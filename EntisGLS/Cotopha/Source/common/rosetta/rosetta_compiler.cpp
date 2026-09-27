
#include <rosetta/rosetta.h>
#include <glscs/glscs_sakura2_module_maker.h>
#include <rosetta/rosetta_compiler.h>
#include <rosetta/rosetta_string.h>
#include <rosetta/rosetta_array.h>

using namespace	SSystem ;
using namespace	Rosetta ;
using namespace ECSSakura2 ;
using namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// コンパイル時型情報
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSCompiler::TypeInfo::TypeInfo( RSCompiler * compiler )
	: m_compiler( compiler ),
		m_ptiNext( NULL ),
		m_typeNum( typeObject ),
		m_accMod( 0 ),
		m_flagPointer( false ),
		m_flagReference( false ),
		m_flagLocal( false ),
		m_flagVirtual( false ),
		m_flagLoaded( false ),
		m_flagLockAddr( false ),
		m_flagRefRosetta( false ),
		m_regLoaded( -1 ), m_regObject( -1 ),
		m_addrOffset( 0 ), m_pLocalVar( NULL ),
		m_pClass( NULL ), m_pImmediate( NULL )
{
	if ( compiler != NULL )
	{
		compiler->AddChainTyepInfo( this ) ;
	}
}

RSCompiler::TypeInfo::TypeInfo( const TypeInfo & ti )
	: m_compiler( ti.m_compiler ),
		m_ptiNext( NULL ),
		m_typeNum( ti.m_typeNum ),
		m_accMod( ti.m_accMod ),
		m_flagPointer( ti.m_flagPointer ),
		m_flagReference( ti.m_flagReference ),
		m_flagLocal( ti.m_flagLocal ),
		m_flagVirtual( ti.m_flagVirtual ),
		m_flagLoaded( ti.m_flagLoaded ),
		m_flagLockAddr( ti.m_flagLockAddr ),
		m_flagRefRosetta( ti.m_flagRefRosetta ),
		m_regLoaded( ti.m_regLoaded ),
		m_regObject( ti.m_regObject ),
		m_addrOffset( ti.m_addrOffset ),
		m_pLocalVar( ti.m_pLocalVar ),
		m_pClass( ti.m_pClass ), m_pImmediate( ti.m_pImmediate )
{
	if ( m_compiler != NULL )
	{
		m_compiler->AddChainTyepInfo( this ) ;
	}
	RSObject::AddRef( m_pImmediate ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSCompiler::TypeInfo::~TypeInfo( void )
{
	if ( m_compiler != NULL )
	{
		m_compiler->DetachChainTyepInfo( this ) ;
	}
	RSObject::ReleaseRef( m_pImmediate ) ;
	m_pImmediate = NULL ;
}

// 代入
//////////////////////////////////////////////////////////////////////////////
RSCompiler::TypeInfo & RSCompiler::TypeInfo::operator = ( const RSCompiler::TypeInfo & ti )
{
	m_typeNum = ti.m_typeNum ;
	m_accMod = ti.m_accMod ;
	m_flagPointer = ti.m_flagPointer ;
	m_flagReference = ti.m_flagReference ;
	m_flagLocal = ti.m_flagLocal ;
	m_flagVirtual = ti.m_flagVirtual ;
	m_flagLoaded = ti.m_flagLoaded ;
	m_flagLockAddr = ti.m_flagLockAddr ;
	m_flagRefRosetta = ti.m_flagRefRosetta ;
	m_regLoaded = ti.m_regLoaded ;
	m_regObject = ti.m_regObject ;
	m_addrOffset = ti.m_addrOffset ;
	m_pLocalVar = ti.m_pLocalVar ;
	m_pClass = ti.m_pClass ;
	RSObject::ReleaseRef( m_pImmediate ) ;
	m_pImmediate = ti.m_pImmediate ;
	RSObject::AddRef( m_pImmediate ) ;
	return	*this ;
}

// Rosetta オブジェクトか？
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::TypeInfo::IsObject( void ) const
{
	return	(m_typeNum == typeObject) && !m_flagPointer ;
}

// 整数型か？
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::TypeInfo::IsInteger( void ) const
{
	return	(m_typeNum <= typeInt64)
			&& (m_typeNum != typeObject) && !m_flagPointer ;
}

// 浮動小数点型か？
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::TypeInfo::IsFloatingPoint( void ) const
{
	return	(m_typeNum >= typeFloat32)
			&& (m_typeNum != typeObject) && !m_flagPointer ;
}

// 型設定
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::TypeInfo::SetType( RSContext& context, RSClass * pClass )
{
	ChangeType( context, pClass ) ;
	//
	m_accMod = 0 ;
	m_flagReference = false ;
	m_flagLocal = false ;
	m_flagLoaded = false ;
	m_flagRefRosetta = false ;
}

void RSCompiler::TypeInfo::ChangeType( RSContext& context, RSClass * pClass )
{
	RSTypedArrayPointerClass *
		pPtrClass = ESLTypeCast<RSTypedArrayPointerClass>( pClass ) ;
	if ( pPtrClass != NULL )
	{
		static const NumberType	s_typeNumTable[] =
		{
			typeUint8, typeInt8,
			typeUint16, typeInt16,
			typeUint32, typeInt32,
			typeInt64,
			typeFloat32, typeFloat64,
		} ;
		if ( pPtrClass->m_typeElement != RSReferenceNumber::typeObject )
		{
			m_typeNum = s_typeNumTable[pPtrClass->m_typeElement] ;
		}
		else
		{
			m_typeNum = typeObject ;
		}
		m_flagPointer = true ;
		m_pClass = pClass ;
	}
	else
	{
		int	nBasicType = RSCodeControl::wiInvalid ;
		for ( int i = RSCodeControl::wiBoolean;
					i <= RSCodeControl::wiDouble; i ++ )
		{
			if ( context.GetBasicTypeClass
					( (RSCodeControl::WordIndex) i ) == pClass )
			{
				nBasicType = i ;
				break ;
			}
		}
		if ( nBasicType != RSCodeControl::wiInvalid )
		{
			static const NumberType	s_typeWordTable[] =
			{
				typeInt8, typeInt8,
				typeInt16, typeUint16,
				typeInt32, typeInt64,
				typeFloat32, typeFloat64,
			} ;
			m_typeNum = s_typeWordTable[nBasicType - RSCodeControl::wiBoolean] ;
		}
		else if ( context.GetIntegerClass() == pClass )
		{
			m_typeNum = typeInt64 ;
		}
		else if ( context.GetNumberClass() == pClass )
		{
			m_typeNum = typeFloat64 ;
		}
		else
		{
			m_typeNum = typeObject ;
		}
		m_flagPointer = false ;
		m_pClass = pClass ;
	}
}

void RSCompiler::TypeInfo::CopyType( const RSCompiler::TypeInfo & ti )
{
	m_typeNum = ti.m_typeNum ;
	m_flagPointer = ti.m_flagPointer ;
	m_flagReference = ti.m_flagReference ;
	m_flagLocal = ti.m_flagLocal ;
	m_pLocalVar = ti.m_pLocalVar ;
	m_pClass = ti.m_pClass ;
	RSObject::ReleaseRef( m_pImmediate ) ;
	m_pImmediate = ti.m_pImmediate ;
	RSObject::AddRef( m_pImmediate ) ;
}

// ローカル変数への参照を設定
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::TypeInfo::SetVarReference
	( RSContext& context, const RSCompiler::LocalVariable * plv )
{
	ESLAssert( plv != NULL ) ;
	SetType( context, plv->pClass ) ;
	//
	m_accMod = plv->accMod ;
	m_flagReference = true ;
	m_flagLocal = true ;
	m_pLocalVar = plv ;
}

// 構造体メンバへの参照を設定
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::TypeInfo::SetMemberReference
	( RSContext& context,
		const RSStructuredPointerClass::ElementInfo * pei )
{
	ESLAssert( pei != NULL ) ;
	SetType( context, pei->m_pClass ) ;
	//
	m_flagReference = !m_flagPointer ;
}

// 即値設定
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::TypeInfo::SetImmediate
	( RSContext& context, RSObject * pObj, RSClass * pClass )
{
	SetType( context, pClass ) ;
	//
	RSObject::ReleaseRef( m_pImmediate ) ;
	m_pImmediate = pObj ;
}

// 即値取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSCompiler::TypeInfo::GetImmediate( void ) const
{
	if ( m_flagLoaded )
	{
		return	NULL ;
	}
	return	m_pImmediate ;
}

// オブジェクトを設定
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::TypeInfo::SetLoadedObject
	( RSContext& context, RSClass * pClass, int regLoaded )
{
	SetType( context, pClass ) ;
	m_typeNum = typeObject ;
	m_flagPointer = false ;
	m_flagRefRosetta = true ;
	m_flagLoaded = true ;
	m_flagLockAddr = false ;
	m_regLoaded = regLoaded ;
	m_regObject = regLoaded ;
}

// ポインタを設定
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::TypeInfo::SetLoadedPointer
	( RSContext& context,
		RSTypedArrayPointerClass * pClass, int regLoaded, int regOwner )
{
	SetType( context, pClass ) ;
	ESLAssert( m_flagPointer ) ;
	m_flagRefRosetta = false ;
	m_flagLoaded = true ;
	m_flagLockAddr = false ;
	m_regLoaded = regLoaded ;
	m_regObject = regOwner ;
}

// 数値を設定
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::TypeInfo::SetLoadedNumber
	( RSContext& context, RSClass * pClass, int regLoaded )
{
	SetType( context, pClass ) ;
	ESLAssert( m_typeNum != typeObject ) ;
	m_flagRefRosetta = false ;
	m_flagLoaded = true ;
	m_flagLockAddr = false ;
	m_regLoaded = regLoaded ;
	m_regObject = -1 ;
}



//////////////////////////////////////////////////////////////////////////////
// テンポラリレジスタ
//////////////////////////////////////////////////////////////////////////////

// リセット
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::ExprRegContext::Reset( void )
{
	nAlloc = 0 ;
	memset( &maskAlloc[0], 0, 128/8 ) ;
}

// コピー
//////////////////////////////////////////////////////////////////////////////
const RSCompiler::ExprRegContext&
	RSCompiler::ExprRegContext::operator =
			( const RSCompiler::ExprRegContext& xrc )
{
	nAlloc = xrc.nAlloc ;
	memmove( &maskAlloc[0], &xrc.maskAlloc[0], 128/8 ) ;
	return	*this ;
}

// レジスタ確保
//////////////////////////////////////////////////////////////////////////////
int RSCompiler::ExprRegContext::Allocate( void )
{
	const int	reg = 16 + (nAlloc ++) ;	// 途中に空きがあっても無視する
											// ※関数呼び出しなどで連続性を保証するため
	if ( reg < 128 )
	{
		ESLAssert( !(maskAlloc[reg >> 3] & (1 << (reg & 0x07))) ) ;
		maskAlloc[reg >> 3] |= (1 << (reg & 0x07)) ;
		return	reg ;
	}
	else
	{
		return	-1 ;
	}
}

// レジスタ解放
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::ExprRegContext::Free( int reg )
{
	ESLAssert( maskAlloc[reg >> 3] & (1 << (reg & 0x07)) ) ;
	maskAlloc[reg >> 3] &= ~(1 << (reg & 0x07)) ;
	//
	if ( nAlloc + 15 == reg )
	{
		int	i = (reg >> 3) ;
		int	j ;
		while ( (maskAlloc[i] == 0) && (i > 2) )
		{
			i -- ;
		}
		uint8_t	mask = maskAlloc[i] ;
		if ( mask == 0 )
		{
			ESLAssert( i == 2 ) ;
			nAlloc = 0 ;
		}
		else
		{
			for ( j = 7; j >= 0; j -- )
			{
				if ( mask & 0x80 )
				{
					break ;
				}
				mask <<= 1 ;
			}
			nAlloc = ((i - 2) << 3) + j + 1 ;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
// Rosetta -> Sakura2 コンパイラ
//////////////////////////////////////////////////////////////////////////////

const RSCompiler::PFUNC_COMPILE_STATEMENT
				RSCompiler::m_pfnCompileStatement[RSCodeControl::wiCount + 1] =
{
	&RSCompiler::CompileStatementImport,			// import
	&RSCompiler::CompileStatementClass,				// class
	&RSCompiler::CompileStatementStruct,			// struct
	&RSCompiler::CompileStatementFunction,			// function
	&RSCompiler::CompileStatementInvalid,			// extends
	&RSCompiler::CompileStatementInvalid,			// implements
	&RSCompiler::CompileStatementFor,				// for
	&RSCompiler::CompileStatementWhile,				// while
	&RSCompiler::CompileStatementDo,				// do
	&RSCompiler::CompileStatementIf,				// if
	&RSCompiler::CompileStatementInvalid,			// else
	&RSCompiler::CompileStatementSwitch,			// switch
	&RSCompiler::CompileStatementCase,				// case
	&RSCompiler::CompileStatementDefault,			// default
	&RSCompiler::CompileStatementBreak,				// break
	&RSCompiler::CompileStatementContinue,			// continue
	&RSCompiler::CompileStatementTry,				// try
	&RSCompiler::CompileStatementInvalid,			// catch
	&RSCompiler::CompileStatementInvalid,			// finally
	&RSCompiler::CompileStatementThrow,				// throw
	&RSCompiler::CompileStatementReturn,			// return
	&RSCompiler::CompileStatementWith,				// with
	&RSCompiler::CompileStatementSynchronized,		// synchronized
	&RSCompiler::CompileStatementAccessModifier,	// static
	&RSCompiler::CompileStatementAccessModifier,	// abstract
	&RSCompiler::CompileStatementAccessModifier,	// native
	&RSCompiler::CompileStatementAccessModifier,	// const
	&RSCompiler::CompileStatementAccessModifier,	// public
	&RSCompiler::CompileStatementAccessModifier,	// protected
	&RSCompiler::CompileStatementAccessModifier,	// private
	&RSCompiler::CompileStatementVar,				// var
	&RSCompiler::CompileStatementVar,				// void
	&RSCompiler::CompileStatementVar,				// boolean
	&RSCompiler::CompileStatementVar,				// byte
	&RSCompiler::CompileStatementVar,				// short
	&RSCompiler::CompileStatementVar,				// char
	&RSCompiler::CompileStatementVar,				// int
	&RSCompiler::CompileStatementVar,				// long
	&RSCompiler::CompileStatementVar,				// float
	&RSCompiler::CompileStatementVar,				// double
	&RSCompiler::CompileStatementExpression,		// this
	&RSCompiler::CompileStatementExpression,		// super
	&RSCompiler::CompileStatementDebugPoint,		// super
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSCompiler, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSCompiler::RSCompiler
	( RSContext * context, ECSSakura2::ExecutableModuleMaker * xmm )
{
	m_context = context ;
	m_xmm = xmm ;
	m_flagsBehaivor = behaviorTruthSymbol ;
	//
	m_nMaxLocalBytes = 0 ;
	m_plvThis = NULL ;
	m_pperr = NULL ;
	m_flagInvalidFunc = false ;
	m_addrAddSP = 0 ;
	m_ptiFirstTemp = NULL ;
	m_ptiExprParentOf = NULL ;
	//
	ResetAllLocalVariableCaches() ;
	m_xrcRegs.Reset() ;
	//
	m_pCurParenthesis = NULL ;
	m_iSrcStatement = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSCompiler::~RSCompiler( void )
{
}

// 関数コンパイル
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::CompileFunction
	( RSClass * pThisClass,
		RSFunctionPrototype& proto,
		SSystem::SParserErrorInterface& perr )
{
	if ( proto.m_methodNative.pfnMethod != NULL )
	{
		return	false ;
	}
	if ( proto.m_pfnFuncAddr != -1 )
	{
		return	false ;
	}
	if ( proto.m_pParenthesis == NULL )
	{
		return	false ;
	}
	RSObject::AddRef( pThisClass ) ;
	m_context->PushNamespace( pThisClass, m_context->new_Namespace() ) ;
	//
	PrepareCompileFunction( pThisClass, proto, perr ) ;
	//
	m_pPrototype = &proto ;
	m_pCurParenthesis = proto.m_pParenthesis ;
	m_context->SetCurrentParenthesis( proto.m_pParenthesis ) ;
	//
	RSCodeStream	cstrm( *(proto.m_pParenthesis) ) ;
	while ( !cstrm.IsEndOfStream() && !m_flagInvalidFunc )
	{
		CompileAStatement( cstrm ) ;
		//
		ESLAssert( m_ptiFirstTemp == NULL ) ;
		ESLAssert( m_xrcRegs.nAlloc == 0 ) ;
	}
	m_pPrototype = NULL ;
	m_pCurParenthesis = NULL ;
	//
	FinishCompileFunction() ;
	//
	m_context->PopNamespace() ;
	return	!m_flagInvalidFunc ;
}

// 関数コンパイル初期化
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::PrepareCompileFunction
	( RSClass * pThisClass,
		RSFunctionPrototype& proto, SSystem::SParserErrorInterface& perr )
{
	//
	// 情報初期化
	//
	m_stackLocals.RemoveAll() ;
	m_nMaxLocalBytes = 0 ;
	m_flagInvalidFunc = false ;
	m_pperr = &perr ;
	ResetAllLocalVariableCaches() ;
	m_xrcRegs.Reset() ;
	//
	// 関数ローカルフレーム確保
	//
	m_xmm->WriteCodePushRegsImm8( regBP, 2 ) ;	// [bp] [tp] [ret ip] [arg0]
	m_xmm->WriteCodeMoveRegReg( regBP, regSP ) ;
	m_addrAddSP = m_xmm->WriteCodeAddSP( 0 ) ;
	//
	// 関数引数受け取り
	//
	PushLocalSpace() ;
	//
	m_plvThis = AllocateLocalVariable( L"this", pThisClass ) ;
	if ( proto.IsConstantModifier() )
	{
		m_plvThis->accMod |= RSObject::modifierConst ;
	}
	InitializeArgument( m_plvThis, regTP ) ;
	//
	for ( size_t i = 0; i < proto.m_aArgTypes.GetLength(); i ++ )
	{
		SString *	pArgName = proto.m_aArgNames.GetAt( i ) ;
		RSClass *	pArgType = proto.m_aArgTypes.GetAt( i ) ;
		const wchar_t *	pwszArgName = NULL ;
		if ( pArgName != NULL )
		{
			pwszArgName = *pArgName ;
		}
		LocalVariable *	plv = AllocateLocalVariable( pwszArgName, pArgType ) ;
		//
		int	regTemp = AllocateTemporaryRegister() ;
		m_xmm->WriteCodeLoad
			( regTemp, dataInt64, regBP, (int) (i + 3) * 8 ) ;
		//
		InitializeArgument( plv, regTemp ) ;
		FreeTemporaryRegister( regTemp ) ;
	}
	FreeAllTemporaryRegister() ;
	//
	CompileCheckException() ;
}

// 関数コンパイル終了
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::FinishCompileFunction( void )
{
	//
	// デフォルトのリターン
	//
	m_xmm->WriteCodeMoveRegReg( regAcc, regIntZero ) ;
	CompileReturn( regAcc ) ;
	//
	// 関数ローカルフレーム確定
	//
	m_xmm->CommitCodeAddSP( m_addrAddSP, - (int) m_nMaxLocalBytes ) ;
	//
	m_stackLocals.RemoveAll() ;
	m_plvThis = NULL ;
	m_nMaxLocalBytes = 0 ;
	m_pperr = NULL ;
}

// コンパイルエラー出力
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::OutputError( const wchar_t * pwszErrMsg )
{
	m_context->ThrowExceptionError( pwszErrMsg ) ;
	m_flagInvalidFunc = true ;
}

// 関数復帰
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileReturn( int regRet )
{
	RegisterAssign	ra ;
	SaveAllLocalVariableCaches( ra ) ;
	//
	m_xmm->WriteCodePushReg( regRet ) ;
	//
	ExprRegContext	xrc ;
	PushAllTemporaryRegisters( xrc ) ;
	ReleaseAllTemporaryOwnerObjects() ;
	FreeAllLocalSpace() ;
	PopAllTemporaryRegisters( xrc ) ;
	//
	m_xmm->WriteCodePopReg( regAcc ) ;
	m_xmm->WriteCodeMoveRegReg( regSP, regBP ) ;
	m_xmm->WriteCodePopRegsImm8( regBP, 2 ) ;
	m_xmm->WriteCodeReturn() ;
	//
	RestoreAllLocalVariableCaches( ra ) ;
}

// Rosetta 例外判定し、例外発生の場合関数復帰
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileCheckException( bool fNoExceptPos )
{
	m_xmm->WriteCodeSyscall( L"__nrs_is_exception" ) ;
	size_t	addrNCJump = m_xmm->WriteCodeNCJump( regAcc ) ;
	//
	if ( !fNoExceptPos && (m_pCurParenthesis != NULL) )
	{
		int	regArg0 = GetFreeTemporaryRegister( 2 ) ;
		int	regArg1 = regArg0 + 1 ;
		m_xmm->WriteCodeMoveRegInt64( regArg0, (ulong_ptr_t) m_pCurParenthesis ) ;
		m_xmm->WriteCodeMoveRegInt64( regArg1, m_iSrcStatement ) ;
		//
		m_xmm->WriteCodePushRegsImm8( regArg0, 2 ) ;
		m_xmm->WriteCodeSyscall( L"__nrs_set_exception_position" ) ;
		m_xmm->WriteCodeAddSP( 16 ) ;
	}
	//
	m_xmm->WriteCodeMoveRegReg( regAcc, regIntZero ) ;
	CompileReturn( regAcc ) ;
	//
	m_xmm->CommitJumpAddress( addrNCJump, m_xmm->GetNextCodeAddress() ) ;
}

// アライメントチェックし、例外発生
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileCheckAlignment( int reg, int nAlign )
{
	if ( nAlign <= 1 )
	{
		return ;
	}
	size_t	addrJump ;
	if ( nAlign == 2 )
	{
		addrJump = m_xmm->WriteCodeNCJump( reg ) ;
	}
	else
	{
		m_xmm->WriteCodeAddRegRegImm32( regAcc, regIntZero, nAlign - 1 ) ;
		m_xmm->WriteCode2OP( codeAndReg, regAcc, reg ) ;
		m_xmm->WriteCodeSIMD64_2OP( simdPcmpnesd, regAcc, regIntZero ) ;
		addrJump = m_xmm->WriteCodeCJump( regAcc ) ;
	}
	//
	int	regArg0 = GetFreeTemporaryRegister( 2 ) ;
	int	regArg1 = regArg0 + 1 ;
	m_xmm->WriteCodeMoveRegInt64( regArg0, (ulong_ptr_t) m_pCurParenthesis ) ;
	m_xmm->WriteCodeMoveRegInt64( regArg1, m_iSrcStatement ) ;
	//
	m_xmm->WriteCodePushRegsImm8( regArg0, 2 ) ;
	m_xmm->WriteCodeSyscall( L"__nrs_set_currrent_position" ) ;
	m_xmm->WriteCodeAddSP( 16 ) ;
	//
	m_xmm->WriteCodeMoveRegInt64( regArg0, (ulong_ptr_t) L"アライメントエラー" ) ;
	m_xmm->WriteCodeMoveRegInt64( regArg1, 0 ) ;
	//
	m_xmm->WriteCodePushRegsImm8( regArg0, 2 ) ;
	m_xmm->WriteCodeSyscall( L"__nrs_throw_exception" ) ;
	m_xmm->WriteCodeAddSP( 16 ) ;
	//
	m_xmm->WriteCodeMoveRegReg( regAcc, regIntZero ) ;
	CompileReturn( regAcc ) ;
	//
	m_xmm->CommitJumpAddress( addrJump, m_xmm->GetNextCodeAddress() ) ;
}

// ローカル空間追加
//////////////////////////////////////////////////////////////////////////////
RSCompiler::LocalSpace * RSCompiler::PushLocalSpace( void )
{
	ssize_t	nLocalBytes = 0 ;
	for ( size_t i = 0; i < m_stackLocals.GetLength(); i ++ )
	{
		LocalSpace *	pls = m_stackLocals.GetAt( i ) ;
		ESLAssert( pls != NULL ) ;
		nLocalBytes += (ssize_t) pls->m_nBytes ;
	}
	LocalSpace *	pls = new LocalSpace ;
	pls->m_bpOffset = - nLocalBytes ;
	m_stackLocals.Push( pls ) ;
	return	pls ;
}

// ローカル空間削除
//////////////////////////////////////////////////////////////////////////////
RSCompiler::LocalSpace * RSCompiler::PopLocalSpace( void )
{
	return	m_stackLocals.Pop() ;
}

// ローカル変数追加
//////////////////////////////////////////////////////////////////////////////
RSCompiler::LocalVariable *
	RSCompiler::AllocateLocalVariable
		( const wchar_t * pwszName, RSClass * pType )
{
	LocalSpace *	pls = m_stackLocals.GetLastAt() ;
	ESLAssert( pls != NULL ) ;
	//
	TypeInfo	ti( NULL ) ;
	ti.SetType( *m_context, pType ) ;
	//
	LocalVariable	lv ;
	lv.typeNumber = ti.m_typeNum ;
	lv.pClass = pType ;
	lv.flagObject = true ;
	lv.flagPointer = ti.m_flagPointer ;
	lv.nBytes = 8 ;
	if ( lv.flagPointer )
	{
		lv.nBytes += 0x18 ;
	}
	else if ( lv.typeNumber != typeObject )
	{
		lv.flagObject = false ;
	}
	else
	{
		lv.nBytes += 0x18 ;
	}
	lv.bpNumOffset = pls->m_bpOffset
						- (ssize_t) (pls->m_nBytes + lv.nBytes) ;
	lv.bpObjOffset = lv.bpNumOffset ;
	if ( lv.flagObject )
	{
		lv.bpObjOffset = lv.bpNumOffset + 0x18 ;
	}
	pls->m_nBytes += lv.nBytes ;
	if ( m_nMaxLocalBytes < (size_t) (pls->m_nBytes - pls->m_bpOffset) )
	{
		m_nMaxLocalBytes = (size_t) (pls->m_nBytes - pls->m_bpOffset) ;
	}
	if ( pwszName && pwszName[0] )
	{
		if ( pls->m_ssaVar.GetAs( pwszName ) != NULL )
		{
			OutputError( SString(pwszName) + L" は二重定義です" ) ;
		}
	}
	size_t	i = pls->m_ssaVar.Add( pwszName, lv ) ;
	return	pls->m_ssaVar.GetAt( i ) ;
}

// ローカル変数検索
//////////////////////////////////////////////////////////////////////////////
RSCompiler::LocalVariable *
	RSCompiler::SearchLocalVariable( const wchar_t * pwszName )
{
	for ( size_t i = 0; i < m_stackLocals.GetLength(); i ++ )
	{
		LocalSpace *	pls = m_stackLocals.GetLastAt( i ) ;
		if ( pls != NULL )
		{
			LocalVariable *	plv = pls->m_ssaVar.GetAs( pwszName ) ;
			if ( plv != NULL )
			{
				return	plv ;
			}
		}
	}
	return	NULL ;
}

// ローカル空間の解放
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::FreeLocalSpace( RSCompiler::LocalSpace * pls )
{
	for ( size_t i = 0; i < pls->m_ssaVar.GetLength(); i ++ )
	{
		LocalVariable *	plv = pls->m_ssaVar.GetAt( i ) ;
		ESLAssert( plv != NULL ) ;
		FreeLocalVariable( plv ) ;
	}
}

// 全ローカル空間の解放（LocalSpace は保持）
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::FreeAllLocalSpace( void )
{
	for ( size_t i = 0; i < m_stackLocals.GetLength(); i ++ )
	{
		LocalSpace *	pls = m_stackLocals.GetLastAt( i ) ;
		if ( pls != NULL )
		{
			FreeLocalSpace( pls ) ;
		}
	}
}

// ローカル変数初期化（オブジェクトを数値やポインタへ実体化）
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::InitializeArgument
	( RSCompiler::LocalVariable * plv, int regInit )
{
	if ( plv->flagPointer )
	{
		ESLAssert( plv->nBytes >= 0x20 ) ;
		m_xmm->WriteCodePushReg( regInit ) ;
		m_xmm->WriteCodeSyscall( L"__nrs_realize_pointer" ) ;
		m_xmm->WriteCodeAddSP( 8 ) ;
		m_xmm->WriteCodeStore
			( regAcc, dataInt64, regBP, (int) plv->bpNumOffset ) ;
		m_xmm->WriteCodeStore
			( regAcc, dataInt64, regBP, (int) plv->bpObjOffset ) ;
		m_xmm->WriteCodeStore
			( regYP, dataInt64, regBP, (int) plv->bpNumOffset + 0x08 ) ;
		m_xmm->WriteCodeStore
			( regIntZero, dataInt64, regBP, (int) plv->bpNumOffset + 0x10 ) ;
		m_xmm->WriteCodeAddRegRegImm32
			( regYP, regBP, (int) plv->bpNumOffset + 0x08 ) ;
	}
	else if ( plv->typeNumber != typeObject )
	{
		m_xmm->WriteCodePushReg( regInit ) ;
		if ( plv->typeNumber <= typeInt64 )
		{
			m_xmm->WriteCodeSyscall( L"__nrs_realize_integer" ) ;
		}
		else
		{
			m_xmm->WriteCodeSyscall( L"__nrs_realize_real_number" ) ;
		}
		m_xmm->WriteCodeAddSP( 8 ) ;
		m_xmm->WriteCodeStore
			( regAcc, dataInt64, (int) regBP, (int) plv->bpNumOffset ) ;
	}
	else
	{
		ESLAssert( plv->nBytes >= 0x20 ) ;
		m_xmm->WriteCodeStore
			( regInit, dataInt64, regBP, (int) plv->bpNumOffset ) ;
		m_xmm->WriteCodeStore
			( regIntZero, dataInt64, regBP, (int) plv->bpObjOffset ) ;
		m_xmm->WriteCodeStore
			( regYP, dataInt64, regBP, (int) plv->bpNumOffset + 0x08 ) ;
		m_xmm->WriteCodeStore
			( regIntOne, dataInt64, regBP, (int) plv->bpNumOffset + 0x10 ) ;
		m_xmm->WriteCodeAddRegRegImm32
			( regYP, regBP, (int) plv->bpNumOffset + 0x08 ) ;
	}
}

// ローカル変数初期化
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::InitializeVariable( LocalVariable * plv )
{
	if ( plv->flagPointer )
	{
		ESLAssert( plv->nBytes >= 16 ) ;
		WriteLocalVariable( plv, regIntZero ) ;
		m_xmm->WriteCodeStore
			( regIntZero, dataInt64, regBP, (int) plv->bpObjOffset ) ;
		m_xmm->WriteCodeStore
			( regYP, dataInt64, regBP, (int) plv->bpNumOffset + 0x08 ) ;
		m_xmm->WriteCodeStore
			( regIntZero, dataInt64, regBP, (int) plv->bpNumOffset + 0x10 ) ;
		m_xmm->WriteCodeAddRegRegImm32
			( regYP, regBP, (int) plv->bpNumOffset + 0x08 ) ;
	}
	else if ( plv->typeNumber != typeObject )
	{
	}
	else
	{
		ESLAssert( plv->nBytes >= 0x20 ) ;
		WriteLocalVariable( plv, regIntZero ) ;
		m_xmm->WriteCodeStore
			( regIntZero, dataInt64, regBP, (int) plv->bpObjOffset ) ;
		m_xmm->WriteCodeStore
			( regYP, dataInt64, regBP, (int) plv->bpNumOffset + 0x08 ) ;
		m_xmm->WriteCodeStore
			( regIntOne, dataInt64, regBP, (int) plv->bpNumOffset + 0x10 ) ;
		m_xmm->WriteCodeAddRegRegImm32
			( regYP, regBP, (int) plv->bpNumOffset + 0x08 ) ;
	}
}

// ローカル変数の解放
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::FreeLocalVariable( const RSCompiler::LocalVariable * plv )
{
	ReleaseRefLocalVariable( plv ) ;
	FreePointerLocalVariable( plv ) ;
	//
	if ( plv->flagObject )
	{
		m_xmm->WriteCodeLoad( regYP, dataInt64, regYP ) ;
	}
	for ( int i = 0; i < 7; i ++ )
	{
		if ( m_raRegs.m_slot[i].m_plv == plv )
		{
			m_raRegs.m_slot[i].m_plv = NULL ;
		}
	}
}

// ローカル変数の解放（Rosetta オブジェクトの参照のみ）
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::ReleaseRefLocalVariable( const RSCompiler::LocalVariable * plv )
{
	if ( !plv->flagPointer && plv->flagObject )
	{
		int	regTemp = AllocateTemporaryRegister() ;
		m_xmm->WriteCodeLoad( regAcc, dataInt64, regBP, (int) plv->bpObjOffset ) ;
		m_xmm->WriteCodeMoveRegReg( regTemp, regAcc ) ;
		m_xmm->WriteCode2OP( codeCmpEqReg, regTemp, regIntZero ) ;
		size_t	addrCJump = m_xmm->WriteCodeCJump( regTemp ) ;
		FreeTemporaryRegister( regTemp ) ;
		//
		WriteBackAllLocalVariables( false ) ;
		m_xmm->WriteCodeStore
			( regIntZero, dataInt64, regBP, (int) plv->bpObjOffset ) ;
		if ( m_xrcRegs.nAlloc > 0 )
		{
			m_xmm->WriteCodePushRegsImm8( 16, m_xrcRegs.nAlloc ) ;
		}
		m_xmm->WriteCodePushReg( regAcc ) ;
		m_xmm->WriteCodeSyscall( L"__nrs_release_ref" ) ;
		m_xmm->WriteCodeAddSP( 8 ) ;
		if ( m_xrcRegs.nAlloc > 0 )
		{
			m_xmm->WriteCodePopRegsImm8( 16, m_xrcRegs.nAlloc ) ;
		}
		ReloadAllLocalVariables() ;
		//
		m_xmm->CommitJumpAddress( addrCJump, m_xmm->GetNextCodeAddress() ) ;
	}
}

// ローカル変数の解放（Sakura2 オブジェクトの解放のみ）
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::FreePointerLocalVariable( const RSCompiler::LocalVariable * plv )
{
	if ( plv->flagPointer )
	{
		int	regTemp = AllocateTemporaryRegister() ;
		m_xmm->WriteCodeLoad( regAcc, dataInt64, regBP, (int) plv->bpObjOffset ) ;
		m_xmm->WriteCodeMoveRegReg( regTemp, regAcc ) ;
		m_xmm->WriteCode2OP( codeCmpEqReg, regTemp, regIntZero ) ;
		size_t	addrCJump = m_xmm->WriteCodeCJump( regTemp ) ;
		FreeTemporaryRegister( regTemp ) ;
		//
		WriteBackAllLocalVariables( false ) ;
		m_xmm->WriteCodeStore
			( regIntZero, dataInt64, regBP, (int) plv->bpObjOffset ) ;
		if ( m_xrcRegs.nAlloc > 0 )
		{
			m_xmm->WriteCodePushRegsImm8( 16, m_xrcRegs.nAlloc ) ;
		}
		m_xmm->WriteCodePushReg( regAcc ) ;
		m_xmm->WriteCodeSyscall( L"__nrs_release_ptr_ref" ) ;
		m_xmm->WriteCodeAddSP( 8 ) ;
		//
		if ( m_xrcRegs.nAlloc > 0 )
		{
			m_xmm->WriteCodePopRegsImm8( 16, m_xrcRegs.nAlloc ) ;
		}
		ReloadAllLocalVariables() ;
		//
		m_xmm->CommitJumpAddress( addrCJump, m_xmm->GetNextCodeAddress() ) ;
	}
}

// 全一時オブジェクト参照を解放する（情報は現在のまま保持）
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::ReleaseAllTemporaryOwnerObjects( void )
{
	ExprRegContext	xrc ;
	WriteBackAllLocalVariables() ;
	PushAllTemporaryRegisters( xrc ) ;
	//
	TypeInfo *	ptiNext = m_ptiFirstTemp ;
	while ( ptiNext != NULL )
	{
		if ( ptiNext->m_flagLoaded
			&& (ptiNext->m_regObject >= 0) )
		{
			m_xmm->WriteCodeLoad
				( regAcc, dataInt64, regSP, (ptiNext->m_regObject - 16) * 8 ) ;
			m_xmm->WriteCodePushReg( regAcc ) ;
			if ( !ptiNext->m_flagRefRosetta )
			{
				m_xmm->WriteCodeSyscall( L"__nrs_release_ptr_ref" ) ;
			}
			else
			{
				m_xmm->WriteCodeSyscall( L"__nrs_release_ref" ) ;
			}
			m_xmm->WriteCodeAddSP( 8 ) ;
		}
		ptiNext = ptiNext->m_ptiNext ;
	}
	//
	PopAllTemporaryRegisters( xrc ) ;
	ReloadAllLocalVariables() ;
}

// TypeInfo をチェーンに追加
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::AddChainTyepInfo( RSCompiler::TypeInfo * ptiTemp )
{
	ESLAssert( ptiTemp->m_ptiNext == NULL ) ;
	ptiTemp->m_ptiNext = m_ptiFirstTemp ;
	m_ptiFirstTemp = ptiTemp ;
}

// TypeInfo をチェーンから削除
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::DetachChainTyepInfo( RSCompiler::TypeInfo * ptiTemp )
{
	TypeInfo *	ptiLast = NULL ;
	TypeInfo *	ptiNext = m_ptiFirstTemp ;
	while ( ptiNext != NULL )
	{
		if ( ptiNext == ptiTemp )
		{
			ESLAssert( (ptiLast == NULL) || (ptiLast->m_ptiNext == ptiNext) ) ;
			if ( ptiLast != NULL )
			{
				ptiLast->m_ptiNext = ptiNext->m_ptiNext ;
			}
			else
			{
				m_ptiFirstTemp = ptiNext->m_ptiNext ;
			}
			ptiTemp->m_ptiNext = NULL ;
			break ;
		}
		ptiLast = ptiNext ;
		ptiNext = ptiNext->m_ptiNext ;
	}
	if ( m_ptiExprParentOf == ptiTemp )
	{
		m_ptiExprParentOf = NULL ;
	}
}

// コンパイル時型情報とオブジェクト参照を移転
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::MoveTemporaryTypeInfo
	( RSCompiler::TypeInfo& tiDst, RSCompiler::TypeInfo& tiSrc )
{
	FreeTemporaryRegister( tiDst ) ;
	//
	tiDst = tiSrc ;
	//
	tiSrc.m_flagLoaded = false ;
	tiSrc.m_regLoaded = -1 ;
	tiSrc.m_regObject = -1 ;
}

// テンポラリレジスタ解放とオブジェクト参照の解放
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::FreeTemporaryRegister( RSCompiler::TypeInfo& ti )
{
	if ( ti.m_flagLoaded )
	{
		if ( ti.m_flagLockAddr )
		{
			UnlockCacheRegister( ti.m_regLoaded ) ;
		}
		else
		{
			FreeTemporaryRegister( ti.m_regLoaded ) ;
		}
		//
		FreeTemporaryRegisterAndOwnerObject( ti ) ;
		//
		ti.m_regLoaded = -1 ;
		ti.m_flagLoaded = false ;
	}
}

// ローカル変数をキャッシュレジスタへ割り当ててロード
//////////////////////////////////////////////////////////////////////////////
int RSCompiler::CommitLocalVariable( const LocalVariable * plv )
{
	int	iEmpty = -1 ;
	int	iMostOldNW = -1 ;
	int	nOldAccessNW = 0x7FFFFFFF ;
	int	iMostOldWB = -1 ;
	int	nOldAccessWB = 0x7FFFFFFF ;
	for ( int i = 0; i < 7; i ++ )
	{
		if ( m_raRegs.m_slot[i].m_plv == plv )
		{
			m_raRegs.m_slot[i].m_lastAccess = m_raRegs.m_access ++ ;
			return	i + 1 ;
		}
		else if ( m_raRegs.m_slot[i].m_plv == NULL )
		{
			iEmpty = i ;
			iMostOldNW = i ;
			nOldAccessNW = 0 ;
			iMostOldWB = i ;
			nOldAccessWB = 0 ;
		}
		else if ( m_raRegs.m_slot[i].m_locked == 0 )
		{
			if ( !m_raRegs.m_slot[i].m_modified )
			{
				if ( m_raRegs.m_slot[i].m_lastAccess < nOldAccessNW )
				{
					nOldAccessNW = m_raRegs.m_slot[i].m_lastAccess ;
					iMostOldNW = i ;
				}
			}
			else
			{
				if ( m_raRegs.m_slot[i].m_lastAccess < nOldAccessWB )
				{
					nOldAccessWB = m_raRegs.m_slot[i].m_lastAccess ;
					iMostOldWB = i ;
				}
			}
		}
	}
	int	regSlot = 0 ;
	if ( iEmpty >= 0 )
	{
		regSlot = iEmpty + 1 ;
		ESLAssert( m_raRegs.m_slot[iEmpty].m_plv == NULL ) ;
		m_raRegs.m_slot[iEmpty].m_plv = plv ;
		m_raRegs.m_slot[iEmpty].m_lastAccess = m_raRegs.m_access ++ ;
		m_raRegs.m_slot[iEmpty].m_locked = 0 ;
		m_raRegs.m_slot[iEmpty].m_modified = false ;
	}
	else if ( iMostOldNW >= 0 )
	{
		regSlot = iMostOldNW + 1 ;
		ESLAssert( !m_raRegs.m_slot[iMostOldNW].m_modified ) ;
		m_raRegs.m_slot[iMostOldNW].m_plv = plv ;
		m_raRegs.m_slot[iMostOldNW].m_lastAccess = m_raRegs.m_access ++ ;
	}
	else if ( iMostOldWB >= 0 )
	{
		regSlot = iMostOldWB + 1 ;
		WriteBackLocalVariable( regSlot ) ;
		//
		ESLAssert( !m_raRegs.m_slot[iMostOldWB].m_modified ) ;
		m_raRegs.m_slot[iMostOldWB].m_plv = plv ;
		m_raRegs.m_slot[iMostOldWB].m_lastAccess = m_raRegs.m_access ++ ;
	}
	else
	{
		return	-1 ;
	}
	m_xmm->WriteCodeLoad
		( regSlot, LocalVariableToDataType( plv ), regBP, (int) plv->bpNumOffset ) ;
	return	regSlot ;
}

// ローカル変数をテンポラリレジスタへロード
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::LoadLocalVariable( const RSCompiler::LocalVariable * plv, int reg )
{
	int	regSlot = CommitLocalVariable( plv ) ;
	if ( regSlot < 0 )
	{
		m_xmm->WriteCodeLoad
			( reg, LocalVariableToDataType( plv ), regBP, (int) plv->bpNumOffset ) ;
		return ;
	}
	m_xmm->WriteCodeMoveRegReg( reg, regSlot ) ;
}

// ローカル変数へテンポラリレジスタからライト
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::WriteLocalVariable( const RSCompiler::LocalVariable * plv, int reg )
{
	int	iEmpty = -1 ;
	int	iMostOldNW = -1 ;
	int	nOldAccessNW = 0x7FFFFFFF ;
	int	iMostOldWB = -1 ;
	int	nOldAccessWB = 0x7FFFFFFF ;
	for ( int i = 0; i < 7; i ++ )
	{
		if ( m_raRegs.m_slot[i].m_plv == plv )
		{
			m_xmm->WriteCodeMoveRegReg( i + 1, reg ) ;
			m_raRegs.m_slot[i].m_lastAccess = m_raRegs.m_access ++ ;
			m_raRegs.m_slot[i].m_modified = true ;
			return ;
		}
		else if ( m_raRegs.m_slot[i].m_plv == NULL )
		{
			iEmpty = i ;
			iMostOldNW = i ;
			nOldAccessNW = 0 ;
			iMostOldWB = i ;
			nOldAccessWB = 0 ;
		}
		else if ( m_raRegs.m_slot[i].m_locked == 0 )
		{
			if ( !m_raRegs.m_slot[i].m_modified )
			{
				if ( m_raRegs.m_slot[i].m_lastAccess < nOldAccessNW )
				{
					nOldAccessNW = m_raRegs.m_slot[i].m_lastAccess ;
					iMostOldNW = i ;
				}
			}
			else
			{
				if ( m_raRegs.m_slot[i].m_lastAccess < nOldAccessWB )
				{
					nOldAccessWB = m_raRegs.m_slot[i].m_lastAccess ;
					iMostOldWB = i ;
				}
			}
		}
	}
	int	regSlot = 0 ;
	if ( iEmpty >= 0 )
	{
		regSlot = iEmpty + 1 ;
		ESLAssert( m_raRegs.m_slot[iEmpty].m_plv == NULL ) ;
		m_raRegs.m_slot[iEmpty].m_plv = plv ;
		m_raRegs.m_slot[iEmpty].m_lastAccess = m_raRegs.m_access ++ ;
		m_raRegs.m_slot[iEmpty].m_locked = 0 ;
		m_raRegs.m_slot[iEmpty].m_modified = true ;
	}
	else if ( iMostOldNW >= 0 )
	{
		regSlot = iMostOldNW + 1 ;
		ESLAssert( !m_raRegs.m_slot[iMostOldNW].m_modified ) ;
		m_raRegs.m_slot[iMostOldNW].m_plv = plv ;
		m_raRegs.m_slot[iMostOldNW].m_lastAccess = m_raRegs.m_access ++ ;
		m_raRegs.m_slot[iMostOldNW].m_modified = true ;
	}
	else if ( iMostOldWB >= 0 )
	{
		regSlot = iMostOldWB + 1 ;
		WriteBackLocalVariable( regSlot ) ;
		//
		ESLAssert( !m_raRegs.m_slot[iMostOldWB].m_modified ) ;
		m_raRegs.m_slot[iMostOldWB].m_plv = plv ;
		m_raRegs.m_slot[iMostOldWB].m_lastAccess = m_raRegs.m_access ++ ;
		m_raRegs.m_slot[iMostOldWB].m_modified = true ;
	}
	else
	{
		m_xmm->WriteCodeStore
			( reg, LocalVariableToDataType( plv ), regBP, (int) plv->bpNumOffset ) ;
		return ;
	}
	m_xmm->WriteCodeMoveRegReg( regSlot, reg ) ;
}

// ローカル変数のレジスタキャッシュをライトバック
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::WriteBackLocalVariable( int reg )
{
	ESLAssert( (reg >= 1) && (reg <= 7) ) ;
	int	iSlot = reg - 1 ;
	const LocalVariable *
		plv = m_raRegs.m_slot[iSlot].m_plv ;
	if ( (plv != NULL) && m_raRegs.m_slot[iSlot].m_modified )
	{
		m_xmm->WriteCodeStore
			( reg, LocalVariableToDataType( plv ), regBP, (int) plv->bpNumOffset ) ;
		m_raRegs.m_slot[iSlot].m_modified = false ;
	}
}

// すべてのローカル変数のレジスタキャッシュをライトバック
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::WriteBackAllLocalVariables( bool fClearModifiedFlag )
{
	for ( int i = 0; i < 7; i ++ )
	{
		const LocalVariable *
			plv = m_raRegs.m_slot[i].m_plv ;
		if ( (plv != NULL) && m_raRegs.m_slot[i].m_modified )
		{
			m_xmm->WriteCodeStore
				( i + 1, LocalVariableToDataType( plv ), regBP, (int) plv->bpNumOffset ) ;
			m_raRegs.m_slot[i].m_modified = !fClearModifiedFlag ;
		}
	}
}

// すべてのローカル変数のレジスタキャッシュを再ロード
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::ReloadAllLocalVariables( void )
{
	for ( int i = 0; i < 7; i ++ )
	{
		const LocalVariable *
			plv = m_raRegs.m_slot[i].m_plv ;
		if ( plv != NULL )
		{
			m_xmm->WriteCodeLoad
				( i + 1, LocalVariableToDataType( plv ), regBP, (int) plv->bpNumOffset ) ;
		}
	}
}

// ローカル変数のレジスタ割り当てを解除する
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::ResetAllLocalVariableCaches( void )
{
	for ( int i = 0; i < 7; i ++ )
	{
		m_raRegs.m_slot[i].m_plv = NULL ;
		m_raRegs.m_slot[i].m_locked = 0 ;
		m_raRegs.m_slot[i].m_modified = false ;
	}
	m_raRegs.m_access = 0 ;
}

// ローカル変数のレジスタキャッシュをライトバックし割り当てを解除する
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::FlushAllLocalVariableCaches( void )
{
	WriteBackAllLocalVariables() ;
	ResetAllLocalVariableCaches() ;
}

// ローカル変数のレジスタキャッシュをロック（解除不可）にする
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::LockCacheRegister( int reg )
{
	ESLAssert( (reg >= 1) && (reg <= 7) ) ;
	if ( (reg >= 1) && (reg <= 7) )
	{
		int	iSlot = reg - 1 ;
		if ( m_raRegs.m_slot[iSlot].m_plv != NULL )
		{
			m_raRegs.m_slot[iSlot].m_locked ++ ;
		}
	}
}

// ローカル変数のレジスタキャッシュをアンロックにする
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::UnlockCacheRegister( int reg )
{
	ESLAssert( (reg >= 1) && (reg <= 7) ) ;
	if ( (reg >= 1) && (reg <= 7) )
	{
		int	iSlot = reg - 1 ;
		if ( m_raRegs.m_slot[iSlot].m_plv != NULL )
		{
			ESLAssert( m_raRegs.m_slot[iSlot].m_locked > 0 ) ;
			m_raRegs.m_slot[iSlot].m_locked -- ;
		}
	}
}

// すべてのローカル変数のレジスタキャッシュがアンロックされているか検証
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::VerifyUnlockAllLocalCache( void )
{
#if	defined(__DEBUG__)
	for ( int i = 0; i < 7; i ++ )
	{
		if ( m_raRegs.m_slot[i].m_plv != NULL )
		{
			ESLAssert( m_raRegs.m_slot[i].m_locked == 0 ) ;
		}
	}
#endif
}

// ローカル変数の割り当てを保存する
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::SaveAllLocalVariableCaches( RSCompiler::RegisterAssign& ra )
{
	ra = m_raRegs ;
}

// ローカル変数の割り当てを復元する
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::RestoreAllLocalVariableCaches( const RSCompiler::RegisterAssign& ra )
{
	m_raRegs = ra ;
}

// テンポラリレジスタを一時 PUSH してコンテキストをリセット
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::PushAllTemporaryRegisters( RSCompiler::ExprRegContext& xrc )
{
	if ( m_xrcRegs.nAlloc > 0 )
	{
		m_xmm->WriteCodePushRegsImm8( 16, m_xrcRegs.nAlloc ) ;
	}
	xrc = m_xrcRegs ;
	m_xrcRegs.Reset() ;
}

// テンポラリレジスタを POP してコンテキストを復元
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::PopAllTemporaryRegisters( const RSCompiler::ExprRegContext& xrc )
{
	m_xrcRegs = xrc ;
	//
	if ( m_xrcRegs.nAlloc > 0 )
	{
		m_xmm->WriteCodePopRegsImm8( 16, m_xrcRegs.nAlloc ) ;
	}
}

// 未確保レジスタ番号取得
//////////////////////////////////////////////////////////////////////////////
int RSCompiler::GetFreeTemporaryRegister( int nCount )
{
	if ( m_xrcRegs.nAlloc + nCount > 128 )
	{
		OutputError( L"数式が複雑すぎます" ) ;
	}
	return	m_xrcRegs.nAlloc + 16 ;
}

// テンポラリレジスタ確保
//////////////////////////////////////////////////////////////////////////////
int RSCompiler::AllocateTemporaryRegister( void )
{
	int	reg = m_xrcRegs.Allocate() ;
	if ( reg < 0 )
	{
		OutputError( L"数式が複雑すぎます" ) ;
	}
	return	reg ;
}

// テンポラリレジスタ解放
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::FreeTemporaryRegister( int reg )
{
	m_xrcRegs.Free( reg ) ;
}

// 所有オブジェクトとレジスタの解放
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::FreeTemporaryRegisterAndOwnerObject( RSCompiler::TypeInfo& ti )
{
	if ( ti.m_flagLoaded )
	{
		if ( ti.m_regObject >= 0 )
		{
			FlushAllLocalVariableCaches() ;
			if ( m_xrcRegs.nAlloc > 0 )
			{
				m_xmm->WriteCodePushRegsImm8( 16, m_xrcRegs.nAlloc ) ;
			}
			m_xmm->WriteCodePushReg( ti.m_regObject ) ;
			if ( !ti.m_flagRefRosetta )
			{
				m_xmm->WriteCodeSyscall( L"__nrs_release_ptr_ref" ) ;
			}
			else
			{
				m_xmm->WriteCodeSyscall( L"__nrs_release_ref" ) ;
			}
			m_xmm->WriteCodeAddSP( 8 ) ;
			if ( m_xrcRegs.nAlloc > 0 )
			{
				m_xmm->WriteCodePopRegsImm8( 16, m_xrcRegs.nAlloc ) ;
			}
			if ( ti.m_regObject != ti.m_regLoaded )
			{
				FreeTemporaryRegister( ti.m_regObject ) ;
			}
			ti.m_regObject = -1 ;
			ti.m_flagRefRosetta = false ;
		}
	}
}

// 全テンポラリレジスタ解放
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::FreeAllTemporaryRegister( void )
{
	m_xrcRegs.Reset() ;
}

// 全テンポラリレジスタの解放検証
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::VerifyFreeAllTemporaryRegister( void )
{
	ESLAssert( m_xrcRegs.nAlloc == 0 ) ;
	m_xrcRegs.Reset() ;
}

// 文コンパイル
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileStatements( RSCodeStream& cstrm )
{
	RSParenthesis *	pPrth =
			cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
	if ( pPrth != NULL )
	{
		RSCodeStream	cs( *pPrth ) ;
		CompileMultiStatements( cs ) ;
	}
	else
	{
		CompileAStatement( cstrm ) ;
	}
}

// 複文コンパイル
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileMultiStatements( RSCodeStream& cstrm )
{
	while ( !cstrm.IsEndOfStream() && !m_flagInvalidFunc )
	{
		CompileAStatement( cstrm ) ;
	}
}

// 一文コンパイル
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileAStatement( RSCodeStream& cstrm )
{
	RSCode *	pcd = cstrm.GetTerm() ;
	if ( pcd != NULL )
	{
		m_iSrcStatement = pcd->m_iSrc ;
		m_context->SetCurrentStatement( pcd ) ;
		m_xmm->AddDebugCodeInfo
			( m_xmm->GetNextCodeAddress(),
					m_pCurParenthesis, m_iSrcStatement ) ;
	}
	RSCodeControl *	pCode = cstrm.NextStatementControlWord() ;
	if ( pCode != NULL )
	{
		// <control-word> statement ;
		m_iSrcStatement = pCode->m_iSrc ;
		m_context->SetCurrentStatement( pCode ) ;
		//
		(this->*m_pfnCompileStatement[pCode->m_word])( cstrm, pCode->m_word ) ;
	}
	else
	{
		RSClass *	pClass = m_context->ParseClassExpression( cstrm ) ;
		m_context->OutputExceptionError( *m_pperr ) ;
		if ( pClass != NULL )
		{
			// <type-expr> <var-name>
			//	{ [= <init-expr>] | ( <arg-list> ) <function-implements> }
			CompileDeclareVariable( cstrm, 0, pClass ) ;
		}
		else if ( cstrm.NextOperator
					( RSCodeOperator::opEndOfStatement ) == NULL )
		{
			// expression ;
			TypeInfo	ti( this ) ;
			CompileExpression
				( ti, cstrm, RSCodeOperator::priorityNothing ) ;
			FreeTemporaryRegister( ti ) ;
			cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) ;
		}
	}
	if ( m_ptiExprParentOf != NULL )
	{
		FreeTemporaryRegister( *m_ptiExprParentOf ) ;
		delete	m_ptiExprParentOf ;
		m_ptiExprParentOf = NULL ;
	}
	VerifyUnlockAllLocalCache() ;
	VerifyFreeAllTemporaryRegister() ;
	m_context->OutputExceptionError( *m_pperr ) ;
}

// 定義文コンパイル
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileDeclareVariable
	( RSCodeStream& cstrm, uint32_t accMod, RSClass * pClass )
{
	RSCodeSymbol *	pSymName = cstrm.NextSymbol() ;
	if ( pSymName == NULL )
	{
		OutputError( L"定義名が記述されていません" ) ;
		return ;
	}
	size_t		iCode = cstrm.GetIndex() ;
	RSCode *	pCode = cstrm.NextTerm() ;
	if ( (pCode != NULL)
		&& (pCode->m_type == RSCode::typeParenthesis)
		&& (((RSParenthesis*)pCode)->m_parenthesis
							== RSParenthesis::ptParenthesis) )
	{
		//
		// 関数定義解釈
		//
		m_flagInvalidFunc = true ;
		return ;
	}
	else if ( pClass != NULL )
	{
		//
		// 変数定義解釈
		//
		cstrm.SeekIndex( iCode ) ;
		while ( !m_context->IsException() )
		{
			LocalVariable *	plv =
				AllocateLocalVariable( pSymName->m_symbol, pClass ) ;
			if ( m_context->IsException() )
			{
				return ;
			}
			plv->accMod = accMod ;
			if ( accMod & RSObject::modifierStatic )
			{
				OutputError( L"static は不正な修飾です" ) ;
				break ;
			}
			InitializeVariable( plv ) ;
			//
			if ( cstrm.NextOperator( RSCodeOperator::opMove ) != NULL )
			{
				TypeInfo	tiInit( this ) ;
				if ( !CompileExpression
					( tiInit, cstrm, RSCodeOperator::priorityList ) )
				{
					return ;
				}
				TypeInfo	tiVar( this ) ;
				ReferenceToLocalVariable( tiVar, plv ) ;
				tiVar.m_accMod &= ~RSObject::modifierConst ;
				//
				OperatorMove( tiVar, tiInit ) ;
				//
				FreeTemporaryRegister( tiVar ) ;
				FreeTemporaryRegister( tiInit ) ;
			}
			else
			{
				WriteLocalVariable( plv, regIntZero ) ;
			}
			if ( cstrm.NextOperator( RSCodeOperator::opSequencing ) == NULL )
			{
				if ( cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) != NULL )
				{
					break ;
				}
				if ( cstrm.IsEndOfStream() )
				{
					break ;
				}
				else
				{
					OutputError
						( L"変数の初期値と次の変数が"
								L" \',\' で区切られていません" ) ;
					return ;
				}
			}
			pSymName = cstrm.NextSymbol() ;
			if ( pSymName == NULL )
			{
				OutputError
					( L"変数定義で、\',\' の後に変数名がありません" ) ;
				return ;
			}
		}
	}
	else
	{
		OutputError( L"void 変数は定義できません" ) ;
	}
}

// 数式コンパイル
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::CompileExpression
		( RSCompiler::TypeInfo& tiExpr, RSCodeStream& cs, int priority )
{
	if ( m_ptiExprParentOf != NULL )
	{
		FreeTemporaryRegister( *m_ptiExprParentOf ) ;
		delete	m_ptiExprParentOf ;
		m_ptiExprParentOf = NULL ;
	}
	//
	RSCode *	pCode = cs.NextTerm() ;
	if ( pCode == NULL )
	{
		OutputError( L"数式の解釈中に文末に到達しました" ) ;
		return	false ;
	}
	m_iSrcStatement = pCode->m_iSrc ;
	//
	RSCode *	pNextCode ;
	RSClass *	pThisClass ;
	RSObject *	pObj ;
	switch ( pCode->m_type )
	{
	case	RSCode::typeLiteral:
		ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeLiteral) ) ) ;
		ESLAssert( ((RSCodeLiteral*)pCode)->m_literal != NULL ) ;
		pObj = ((RSCodeLiteral*)pCode)->m_literal->DuplicateObject( *m_context ) ;
		tiExpr.SetImmediate( *m_context, pObj, pObj->GetRSClass() ) ;
		break ;

	case	RSCode::typeControlCode:
		ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeControl) ) ) ;
		switch ( ((RSCodeControl*)pCode)->m_word )
		{
		case	RSCodeControl::wiThis:
			// this オブジェクト
			ESLAssert( m_plvThis != NULL ) ;
			tiExpr.SetVarReference( *m_context, m_plvThis ) ;
			break ;

		case	RSCodeControl::wiFunction:
			// function オブジェクト
			m_flagInvalidFunc = true ;
			return	false ;

		case	RSCodeControl::wiSuper:
			ESLAssert( m_plvThis != NULL ) ;
			pThisClass = m_plvThis->pClass ;
			if ( (pThisClass == NULL)
				|| (pThisClass->m_pSuperClass == NULL) )
			{
				OutputError( L"super は存在しません" ) ;
				return	false ;
			}
			pNextCode = cs.GetTerm() ;
			if ( (pNextCode != NULL)
				&& (pNextCode->m_type == RSCode::typeParenthesis)
				&& (((RSParenthesis*)pNextCode)->
						m_parenthesis == RSParenthesis::ptParenthesis) )
			{
				// super() コンストラクタ呼び出し
				RSFunctionObject *
					pFunc = pThisClass->m_pSuperClass->GetConstructor() ;
				if ( pFunc == NULL )
				{
					OutputError( L"super クラスにコンストラクタは存在しません" ) ;
					return	false ;
				}
				if ( (m_pPrototype->m_pNamespaceClass == NULL)
					|| (m_pPrototype->m_pNamespaceClass->GetConstructor()
											!= m_pPrototype->m_pFuncGroup) )
				{
					OutputError( L"super() がコンストラクタ外で呼び出されています" ) ;
					return	false ;
				}
				SObjectArray<TypeInfo>	arg ;
				pNextCode = cs.NextTerm() ;
				if ( !CompileArgument( arg, *((RSParenthesis*)pNextCode) ) )
				{
					FreeTemporaryRegisters( arg ) ;
					return	false ;
				}
				tiExpr.SetLoadedObject
					( *m_context, m_plvThis->pClass,
							AllocateTemporaryRegister() ) ;
				tiExpr.m_regObject = -1 ;
				//
				m_xmm->WriteCodeLoad
					( tiExpr.m_regLoaded, dataInt64, regBP, 8 ) ;
				//
				if ( !CompileCallFunction( tiExpr, *pFunc, false, arg, false ) )
				{
					FreeTemporaryRegisters( arg ) ;
					return	false ;
				}
				FlushAllLocalVariableCaches() ;
				//
				if ( ESLTypeCast<RSStructuredPointerClass>( pThisClass ) != NULL )
				{
					m_xmm->WriteCodeLoad
						( regAcc, dataInt64, regBP, (int) m_plvThis->bpNumOffset ) ;
					m_xmm->WriteCodePushReg( regAcc ) ;
					m_xmm->WriteCodeSyscall( L"__nrs_release_ptr_ref" ) ;
					m_xmm->WriteCodeAddSP( 8 ) ;
					//
					m_xmm->WriteCodeLoad( regAcc, dataInt64, regBP, 8 ) ;
					m_xmm->WriteCodePushReg( regAcc ) ;
					m_xmm->WriteCodeSyscall( L"__nrs_realize_pointer" ) ;
					m_xmm->WriteCodeAddSP( 8 ) ;
					m_xmm->WriteCodeStore
						( regAcc, dataInt64, regBP, (int) m_plvThis->bpNumOffset ) ;
					m_xmm->WriteCodeStore
						( regAcc, dataInt64, regBP, (int) m_plvThis->bpObjOffset ) ;
				}
				//
				FreeTemporaryRegisters( arg ) ;
				break ;
			}
			else if ( (pNextCode != NULL)
				&& (pNextCode->m_type == RSCode::typeOperator)
				&& (((RSCodeOperator*)pNextCode)->
						m_operator == RSCodeOperator::opMemberOf) )
			{
				// super.member 親クラスのメンバ関数
				pNextCode = cs.NextTerm( 1 ) ;
				if ( (pNextCode == NULL)
					|| (pNextCode->m_type != RSCode::typeSymbol) )
				{
					OutputError( L"super メンバが指定されていません" ) ;
					return	false ;
				}
				const SString&
					strMember = ((RSCodeSymbol*)pNextCode)->m_symbol ;
				RSClass *	pSuperClass = pThisClass->m_pSuperClass ;
				RSObject *	pMember =
						pSuperClass->GetMemberAs( *m_context, strMember ) ;
				ESLAssert( m_ptiExprParentOf == NULL ) ;
				if ( pMember != NULL )
				{
					RSObject::AddRef( pSuperClass ) ;
					//
					m_ptiExprParentOf = new TypeInfo( this ) ;
					m_ptiExprParentOf->SetImmediate
						( *m_context, pSuperClass, pSuperClass->GetRSClass() ) ;
					//
					tiExpr.SetImmediate
						( *m_context, pMember, pMember->GetRSClass() ) ;
				}
				else
				{
					pMember = pSuperClass->GetVirtualMemberAs( *m_context, strMember ) ;
					if ( pMember != NULL )
					{
						m_ptiExprParentOf = new TypeInfo( this ) ;
						m_ptiExprParentOf->
							SetVarReference( *m_context, m_plvThis ) ;
						tiExpr.SetImmediate
							( *m_context, pMember, pMember->GetRSClass() ) ;
					}
					else
					{
						OutputError( strMember + L" メンバが見つかりません" ) ;
						return	false ;
					}
				}
				break ;
			}

		default:
			if ( (((RSCodeControl*)pCode)->m_word >= RSCodeControl::wiFirstBasicType)
				&& (((RSCodeControl*)pCode)->m_word <= RSCodeControl::wiLastBasicType) )
			{
				// 基本型
				RSCodeControl::WordIndex	wi = ((RSCodeControl*)pCode)->m_word ;
				pObj = m_context->GetBasicTypeClass( wi ) ;
				if ( pObj == NULL )
				{
					OutputError( SString(RSCodeControl::ControlWordAt(wi))
									+ L" は定義されていない基本型です" ) ;
					return	false ;
				}
				pObj->AddRef() ;
				tiExpr.SetImmediate( *m_context, pObj, pObj->GetRSClass() ) ;
			}
			else
			{
				OutputError( L"数式中に不正な予約語が含まれています" ) ;
				return	false ;
			}
			break ;
		}
		break ;

	case	RSCode::typeOperator:
		ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeOperator) ) ) ;
		{
			RSCodeOperator *	pOpCode = (RSCodeOperator*) pCode ;
			const RSCodeOperator::OperatorInfo &
								opinf = pOpCode->GetOperatorInfo() ;
			if ( opinf.flagsRule & RSCodeOperator::ruleUnary )
			{
				if ( pOpCode->m_operator == RSCodeOperator::opNew )
				{
					// new 演算子
					if ( !CompileNewOperator( tiExpr, cs ) )
					{
						return	false ;
					}
				}
				else
				{
					// 単項演算子
					int	nOpPriority = opinf.priorityUnary ;
					if ( !CompileExpression( tiExpr, cs, nOpPriority ) )
					{
						return	false ;
					}
					if ( !CompileUnaryOperator( tiExpr, pOpCode->m_operator ) )
					{
						return	false ;
					}
				}
			}
			else
			{
				OutputError
					( SString(pOpCode->GetOperatorString())
									+ L" 演算子には左辺が必要です" ) ;
				return	false ;
			}
		}
		break ;

	case	RSCode::typeParenthesis:
		ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSParenthesis) ) ) ;
		{
			RSParenthesis *	pPrth = (RSParenthesis*) pCode ;
			switch ( pPrth->m_parenthesis )
			{
			case	RSParenthesis::ptParenthesis:
				// ( expr )
				{
					RSCodeStream	csPrth( *pPrth ) ;
					if ( !CompileExpression( tiExpr, csPrth, 0 ) )
					{
						return	false ;
					}
					RSClass *	pClass = ESLTypeCast<RSClass>( tiExpr.GetImmediate() ) ;
					if ( (tiExpr.m_typeNum == typeObject)
						&& !tiExpr.m_flagPointer && (pClass != NULL) )
					{
						// (type) 型キャスト判定
						TypeInfo	tiTemp( this ) ;
						if ( !CompileExpression
							( tiTemp, cs, RSCodeOperator::priorityUnary ) )
						{
							return	false ;
						}
						if ( !OperatorCast( tiTemp, pClass, true ) )
						{
							return	false ;
						}
						MoveTemporaryTypeInfo( tiExpr, tiTemp ) ;
						FreeTemporaryRegister( tiTemp ) ;
					}
				}
				break ;

			case	RSParenthesis::ptBracket:
				// [ expr, expr, ... ]
				m_flagInvalidFunc = true ;
				return	false ;

			case	RSParenthesis::ptBrace:
				// { id : expr, ... }
				m_flagInvalidFunc = true ;
				return	false ;

			default:
				break ;
			}
		}
		break ;

	case	RSCode::typeSymbol:
		// 任意シンボル
		ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeSymbol) ) ) ;
		if ( !GetVariableAs( tiExpr, ((RSCodeSymbol*)pCode)->m_symbol ) )
		{
			return	false ;
		}
		break ;

	default:
		OutputError( L"内部エラー：未定義コードを解釈できません" ) ;
		return	false ;
	}
	for ( ; ; )
	{
		//
		// 後置演算子／二項演算子判定
		//
		pCode = cs.GetTerm() ;
		if ( pCode == NULL )
		{
			break ;
		}
		if ( pCode->m_type == RSCode::typeOperator )
		{
			ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeOperator) ) ) ;
			RSCodeOperator *	pOpCode = (RSCodeOperator*) pCode ;
			const RSCodeOperator::OperatorInfo &
								opinf = pOpCode->GetOperatorInfo() ;
			if ( opinf.flagsRule & RSCodeOperator::ruleUnaryPost )
			{
				// 後置単項演算子
				if ( opinf.priorityUnaryPost <= priority )
				{
					break ;
				}
				TypeInfo	tiDup( this ) ;
				if ( !CloneObject( tiDup, tiExpr ) )
				{
					return	false ;
				}
				cs.NextTerm() ;
				//
				if ( !CompileUnaryOperator( tiExpr, pOpCode->m_operator ) )
				{
					return	false ;
				}
				MoveTemporaryTypeInfo( tiExpr, tiDup ) ;
			}
			else if ( opinf.flagsRule & RSCodeOperator::ruleBinary )
			{
				// 二項演算子
				if ( opinf.priorityBinary <= priority )
				{
					break ;
				}
				cs.NextTerm() ;
				//
				RSCodeOperator::OperatorIndex	iOp = pOpCode->m_operator ;
				if ( opinf.flagsRule & RSCodeOperator::ruleSpecialRight )
				{
					if ( (iOp == RSCodeOperator::opStaticMemberOf)
								|| (iOp == RSCodeOperator::opMemberOf) )
					{
						// メンバ参照
						if ( !CompileExpressionRefMemberOf( tiExpr, cs, iOp ) )
						{
							return	false ;
						}
					}
					else if ( iOp == RSCodeOperator::opLogicalAnd )
					{
						// expr && expr
						if ( !RealizeToBoolean( tiExpr ) )
						{
							return	false ;
						}
						ESLAssert( tiExpr.m_flagLoaded ) ;
						size_t	addrNCJump =
							m_xmm->WriteCodeNCJump( tiExpr.m_regLoaded ) ;
						//
						TypeInfo	tiExpr2( this ) ;
						if ( !CompileExpression
								( tiExpr2, cs, opinf.priorityBinary ) )
						{
							return	false ;
						}
						if ( !RealizeToBoolean( tiExpr2 ) )
						{
							return	false ;
						}
						m_xmm->WriteCodeMoveRegReg
							( tiExpr.m_regLoaded, tiExpr2.m_regLoaded ) ;
						FreeTemporaryRegister( tiExpr2 ) ;
						//
						m_xmm->CommitJumpAddress
							( addrNCJump, m_xmm->GetNextCodeAddress() ) ;
					}
					else if ( iOp == RSCodeOperator::opLogicalOr )
					{
						// expr || expr
						if ( !RealizeToBoolean( tiExpr ) )
						{
							return	false ;
						}
						ESLAssert( tiExpr.m_flagLoaded ) ;
						size_t	addrCJump =
							m_xmm->WriteCodeCJump( tiExpr.m_regLoaded ) ;
						//
						TypeInfo	tiExpr2( this ) ;
						if ( !CompileExpression
								( tiExpr2, cs, opinf.priorityBinary ) )
						{
							return	false ;
						}
						if ( !RealizeToBoolean( tiExpr2 ) )
						{
							return	false ;
						}
						m_xmm->WriteCodeMoveRegReg
							( tiExpr.m_regLoaded, tiExpr2.m_regLoaded ) ;
						FreeTemporaryRegister( tiExpr2 ) ;
						//
						m_xmm->CommitJumpAddress
							( addrCJump, m_xmm->GetNextCodeAddress() ) ;
					}
					else if ( iOp == RSCodeOperator::opConditional )
					{
						// expr ? expr : expr
						if ( !CompileExpressionSelector( tiExpr, cs ) )
						{
							return	false ;
						}
					}
					else
					{
						OutputError
							( SString(L"内部エラー：")
								+ RSCodeOperator::m_pwszOperators[iOp]
								+ L" は未定義の特殊演算子です" ) ;
						return	false ;
					}
				}
				else
				{
					// 一般的な二項演算子
					if ( m_ptiExprParentOf != NULL )
					{
						FreeTemporaryRegister( *m_ptiExprParentOf ) ;
						delete	m_ptiExprParentOf ;
						m_ptiExprParentOf = NULL ;
					}
					//
					int	nOpPriority = opinf.priorityBinary ;
					if ( opinf.flagsRule & RSCodeOperator::ruleRightToLeft )
					{
						nOpPriority -- ;
					}
					TypeInfo	tiRight( this ) ;
					if ( !CompileExpression( tiRight, cs, nOpPriority ) )
					{
						FreeTemporaryRegister( tiRight ) ;
						return	false ;
					}
					if ( !CompileBinaryOperator
							( tiExpr, tiRight, pOpCode->m_operator ) )
					{
						FreeTemporaryRegister( tiRight ) ;
						return	false ;
					}
					FreeTemporaryRegister( tiRight ) ;
				}
			}
			else
			{
				OutputError( L"数式の書式エラー：単項演算子の用法が不正です" ) ;
				return	false ;
			}
		}
		else if ( pCode->m_type == RSCode::typeParenthesis )
		{
			ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSParenthesis) ) ) ;
			RSParenthesis *	pPrth = (RSParenthesis*) pCode ;
			if ( pPrth->m_parenthesis == RSParenthesis::ptParenthesis )
			{
				//
				// expr(...)
				//
				RSFunctionClass *	pFuncClass =
					ESLTypeCast<RSFunctionClass>( tiExpr.m_pClass ) ;
				if ( pFuncClass == NULL )
				{
					OutputError( L"関数ではないオブジェクトの関数呼び出しです" ) ;
					return	false ;
				}
				cs.NextTerm() ;
				//
				TypeInfo	tiThisObj( this ) ;
				if ( m_ptiExprParentOf != NULL )
				{
					MoveTemporaryTypeInfo( tiThisObj, *m_ptiExprParentOf ) ;
				}
				else
				{
					tiThisObj.SetImmediate
						( *m_context, m_context->new_Pointer( NULL ), NULL ) ;
				}
				//
				SObjectArray<TypeInfo>	arg ;
				if ( !CompileArgument( arg, *pPrth ) )
				{
					FreeTemporaryRegisters( arg ) ;
					FreeTemporaryRegister( tiThisObj ) ;
					return	false ;
				}
				RSFunctionObject *	pFuncObj = NULL ;
				RSObject *	pImmObj = tiExpr.GetImmediate() ;
				if ( pImmObj != NULL )
				{
					pFuncObj = ESLTypeCast<RSFunctionObject>
									( pImmObj->GetEntityObject() ) ;
				}
				if ( pFuncObj != NULL )
				{
					if ( !CompileCallFunction
						( tiThisObj,
							*pFuncObj, tiExpr.m_flagVirtual, arg, true ) )
					{
						FreeTemporaryRegisters( arg ) ;
						FreeTemporaryRegister( tiThisObj ) ;
						return	false ;
					}
				}
				else
				{
					if ( !CompileCallFunction
						( tiThisObj, tiExpr, arg ) )
					{
						FreeTemporaryRegisters( arg ) ;
						FreeTemporaryRegister( tiThisObj ) ;
						return	false ;
					}
				}
				MoveTemporaryTypeInfo( tiExpr, tiThisObj ) ;
				FreeTemporaryRegister( tiThisObj ) ;
				//
				if ( m_ptiExprParentOf != NULL )
				{
					FreeTemporaryRegister( *m_ptiExprParentOf ) ;
					delete	m_ptiExprParentOf ;
					m_ptiExprParentOf = NULL ;
				}
			}
			else if ( pPrth->m_parenthesis == RSParenthesis::ptBracket )
			{
				cs.NextTerm() ;
				//
				RSClass *	pClass = ESLTypeCast<RSClass>( tiExpr.GetImmediate() ) ;
				if ( (pClass) && (pPrth->m_terms.GetLength() == 0) )
				{
					//
					// type[]
					//
					pClass = m_context->GetVM()->GetArrayClassAs( pClass, 1 ) ;
					pClass->AddRef() ;
					tiExpr.SetImmediate
						( *m_context, pClass, pClass->GetRSClass() ) ;
				}
				else
				{
					//
					// expr[expr]
					//
					RSCodeStream	csPrth( *pPrth ) ;
					TypeInfo		tiIndex( this ) ;
					if ( !CompileExpression( tiIndex, csPrth ) )
					{
						FreeTemporaryRegister( tiIndex ) ;
						return	false ;
					}
					if ( !CompileReferenceElement( tiExpr, tiIndex ) )
					{
						FreeTemporaryRegister( tiIndex ) ;
						FreeTemporaryRegister( tiExpr ) ;
						return	false ;
					}
					FreeTemporaryRegister( tiIndex ) ;
				}
			}
			else
			{
				OutputError( L"数式の書式エラー：{} 括弧が不正です" ) ;
				return	false ;
			}
		}
		else
		{
			OutputError( L"数式の書式エラー：項間に演算子がありません" ) ;
			return	false ;
		}
	}
	return	true ;
}

// 数式コンパイル（メンバ参照）
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::CompileExpressionRefMemberOf
	( RSCompiler::TypeInfo& tiExpr, RSCodeStream& cs,
						RSCodeOperator::OperatorIndex iOp )
{
	RSCode *	pNextCode = cs.NextTerm() ;
	if ( (pNextCode == NULL)
		|| (pNextCode->m_type != RSCode::typeSymbol) )
	{
		OutputError( L"数式の書式エラー：メンバが指定されていません" ) ;
		return	false ;
	}
	ESLAssert( pNextCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeSymbol) ) ) ;
	const SString&	strMember = ((RSCodeSymbol*)pNextCode)->m_symbol ;
	if ( m_ptiExprParentOf == NULL )
	{
		m_ptiExprParentOf = new TypeInfo( this ) ;
	}
	MoveTemporaryTypeInfo( *m_ptiExprParentOf, tiExpr ) ;
	//
	RSClass *	pClass = m_ptiExprParentOf->m_pClass ;
	if ( pClass == NULL )
	{
		OutputError( L"抽象オブジェクトのメンバ参照です" ) ;
		return	false ;
	}
	RSStructuredPointerClass *	pStructClass =
		ESLTypeCast<RSStructuredPointerClass>( pClass ) ;
	bool	fFoundMember = false ;
	if ( pStructClass != NULL )
	{
		RSStructuredPointerClass::ElementInfo *
			peiMember = pStructClass->GetArrayMemberAs( strMember ) ;
		if ( peiMember != NULL )
		{
			if ( !ReferenceStructureMember
				( *m_ptiExprParentOf, strMember, peiMember ) )
			{
				return	false ;
			}
			MoveTemporaryTypeInfo( tiExpr, *m_ptiExprParentOf ) ;
			FreeTemporaryRegister( *m_ptiExprParentOf ) ;
			delete	m_ptiExprParentOf ;
			m_ptiExprParentOf = NULL ;
			fFoundMember = true ;
		}
	}
	else if ( ESLTypeCast<RSClass>( m_ptiExprParentOf->GetImmediate() ) != NULL )
	{
		RSObject *	pObj = m_ptiExprParentOf->GetImmediate() ;
		ESLAssert( pObj != NULL ) ;
		RSClass *	pClassObj = ESLTypeCast<RSClass>( pObj ) ;
		ESLAssert( pClassObj != NULL ) ;
		RSObject *	pMember = pObj->GetMemberAs( *m_context, strMember ) ;
		if ( pMember != NULL )
		{
			if ( !VerifyMemberAccessModifier( pClassObj, pMember, strMember ) )
			{
				return	false ;
			}
			if ( (ESLTypeCast<RSClass>( pMember ) != NULL)
				|| (ESLTypeCast<RSFunctionObject>( pMember ) != NULL) )
			{
				tiExpr.SetImmediate
					( *m_context, pMember, pMember->GetRSClass() ) ;
			}
			else if ( (pMember->GetModifiers() & RSObject::modifierConst)
				&& ((ESLTypeCast<RSNumber>( pMember ) != NULL)
					|| (ESLTypeCast<RSInteger>( pMember ) != NULL)
					|| (ESLTypeCast<RSString>( pMember ) != NULL)) )
			{
				tiExpr.SetImmediate
					( *m_context,
						pMember->DuplicateObject( *m_context ),
						pMember->GetRSClass() ) ;
				m_context->ReleaseObjectRef( pMember ) ;
			}
			else
			{
				if ( !ConvertToObject( *m_ptiExprParentOf ) )
				{
					return	false ;
				}
				TypeInfo	tiMember( this ) ;
				pClass = pMember->GetEntityClass() ;
				tiMember.SetLoadedObject
					( *m_context, pClass, AllocateTemporaryRegister() ) ;
				tiMember.m_accMod = pMember->GetModifiers() ;
				//
				int	regArg0 = GetFreeTemporaryRegister() ;
				int	regArg1 = regArg0 + 1 ;
				ESLAssert( m_ptiExprParentOf->m_flagLoaded ) ;
				m_xmm->WriteCodeMoveRegReg
					( regArg0, m_ptiExprParentOf->m_regLoaded ) ;
				m_xmm->WriteCodeMoveRegInt64
					( regArg1, (ulong_ptr_t) ((const wchar_t*) strMember) ) ;
				ESLAssert( !strMember.IsEmpty() ) ;
				m_xmm->WriteCodePushRegsImm8( regArg0, 2 ) ;
				m_xmm->WriteCodeSyscall( L"__nrs_get_member_as" ) ;
				m_xmm->WriteCodeAddSP( 16 ) ;
				m_xmm->WriteCodeMoveRegReg( tiMember.m_regLoaded, regAcc ) ;
				//
				m_context->ReleaseObjectRef( pMember ) ;
				//
				CompileCheckException() ;
				//
				MoveTemporaryTypeInfo( tiExpr, tiMember ) ;
				FreeTemporaryRegister( tiMember ) ;
			}
			fFoundMember = true ;
		}
	}
	else if ( pClass->m_pPrototype != NULL )
	{
		RSObject *	pMember =
			pClass->m_pPrototype->
				GetMemberAs( *m_context, strMember ) ;
		if ( pMember != NULL )
		{
			if ( !VerifyMemberAccessModifier
						( pClass, pMember, strMember )
				|| !ConvertToObject( *m_ptiExprParentOf ) )
			{
				return	false ;
			}
			TypeInfo	tiMember( this ) ;
			pClass = pMember->GetEntityClass() ;
			tiMember.SetLoadedObject
				( *m_context, pClass, AllocateTemporaryRegister() ) ;
			tiMember.m_accMod = pMember->GetModifiers() ;
			//
			int	regArg0 = GetFreeTemporaryRegister() ;
			int	regArg1 = regArg0 + 1 ;
			ESLAssert( m_ptiExprParentOf->m_flagLoaded ) ;
			m_xmm->WriteCodeMoveRegReg
				( regArg0, m_ptiExprParentOf->m_regLoaded ) ;
			m_xmm->WriteCodeMoveRegInt64
				( regArg1, (ulong_ptr_t) ((const wchar_t*) strMember) ) ;
			ESLAssert( !strMember.IsEmpty() ) ;
			m_xmm->WriteCodePushRegsImm8( regArg0, 2 ) ;
			m_xmm->WriteCodeSyscall( L"__nrs_get_member_as" ) ;
			m_xmm->WriteCodeAddSP( 16 ) ;
			m_xmm->WriteCodeMoveRegReg( tiMember.m_regLoaded, regAcc ) ;
			//
			m_context->ReleaseObjectRef( pMember ) ;
			//
			CompileCheckException() ;
			//
			MoveTemporaryTypeInfo( tiExpr, tiMember ) ;
			FreeTemporaryRegister( tiMember ) ;
			fFoundMember = true ;
		}
	}
	if ( !fFoundMember )
	{
		if ( iOp == RSCodeOperator::opStaticMemberOf )
		{
			pClass = ESLTypeCast<RSClass>
				( m_ptiExprParentOf->GetImmediate() ) ;
			if ( pClass == NULL )
			{
				OutputError( L":: 演算子の左辺がクラスではありません" ) ;
				return	false ;
			}
		}
		RSObject *	pMember =
			pClass->GetVirtualMemberAs( *m_context, strMember ) ;
		if ( pMember == NULL )
		{
			OutputError( strMember + L" メンバが見つかりません" ) ;
			return	false ;
		}
		if ( !VerifyMemberAccessModifier( pClass, pMember, strMember ) )
		{
			return	false ;
		}
		tiExpr.SetImmediate
			( *m_context, pMember, pMember->GetEntityClass() ) ;
		tiExpr.m_flagVirtual =
			(ESLTypeCast<RSStructuredPointerClass>( pClass ) == NULL) ;
	}
	return	true ;
}

// 数式コンパイル（expr ? expr : expr）
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::CompileExpressionSelector
	( RSCompiler::TypeInfo& tiExpr, RSCodeStream& cs )
{
	if ( !RealizeToBoolean( tiExpr ) )
	{
		return	false ;
	}
	FlushAllLocalVariableCaches() ;
	//
	ESLAssert( tiExpr.m_flagLoaded ) ;
	size_t	addrCJump =
		m_xmm->WriteCodeNCJump( tiExpr.m_regLoaded ) ;
	//
	int		regObjTemp = AllocateTemporaryRegister() ;
	bool	flagObjectRef = false ;
	//
	// 真条件式評価
	//
	TypeInfo	tiExpr2( this ) ;
	if ( !CompileExpression
		( tiExpr2, cs, RSCodeOperator::prioritySeparator ) )
	{
		return	false ;
	}
	if ( !LoadReferenceTemporary( tiExpr2, true )
		|| !RealizeObjectToNumber( tiExpr2 ) )
	{
		return	false ;
	}
	if ( !TakeObjectReference( tiExpr2 ) )
	{
		return	false ;
	}
	ESLAssert( tiExpr2.m_flagLoaded ) ;
	ESLAssert( !tiExpr.m_flagLockAddr ) ;
	ESLAssert( tiExpr.m_regObject < 0 ) ;
	//
	tiExpr.CopyType( tiExpr2 ) ;
	tiExpr.m_addrOffset = tiExpr2.m_addrOffset ;
	m_xmm->WriteCodeMoveRegReg
		( tiExpr.m_regLoaded, tiExpr2.m_regLoaded ) ;
	//
	if ( tiExpr2.m_regObject >= 0 )
	{
		flagObjectRef = true ;
		tiExpr.m_flagRefRosetta = tiExpr2.m_flagRefRosetta ;
		tiExpr.m_regObject = regObjTemp ;
		m_xmm->WriteCodeMoveRegReg
			( tiExpr.m_regObject, tiExpr2.m_regObject ) ;
		//
		if ( tiExpr2.m_regObject != tiExpr2.m_regLoaded )
		{
			FreeTemporaryRegister( tiExpr2.m_regObject ) ;
		}
		tiExpr2.m_regObject = -1 ;
	}
	//
	FlushAllLocalVariableCaches() ;
	size_t	addrJump = m_xmm->WriteCodeJump() ;
	//
	FreeTemporaryRegister( tiExpr2 ) ;
	//
	m_xmm->CommitJumpAddress
		( addrCJump, m_xmm->GetNextCodeAddress() ) ;
	//
	RSCode *	pNextCode =
		cs.NextOperator( RSCodeOperator::opSeparator ) ;
	if ( pNextCode == NULL )
	{
		OutputError( L"数式の書式エラー：? に対応する : が見つかりません" ) ;
		return	false ;
	}
	//
	// 偽条件式評価
	//
	if ( !CompileExpression
		( tiExpr2, cs, RSCodeOperator::prioritySelector ) )
	{
		return	false ;
	}
	if ( !LoadReferenceTemporary( tiExpr2, true )
		|| !RealizeObjectToNumber( tiExpr2 ) )
	{
		return	false ;
	}
	if ( !TakeObjectReference( tiExpr2 ) )
	{
		return	false ;
	}
	ESLAssert( tiExpr2.m_flagLoaded ) ;
	if ( tiExpr.IsInteger() )
	{
		if ( tiExpr2.IsInteger() && !flagObjectRef )
		{
			NumberType	typeNum = tiExpr.m_typeNum ;
			if ( typeNum != tiExpr2.m_typeNum )
			{
				if ( typeNum < tiExpr2.m_typeNum )
				{
					typeNum = tiExpr2.m_typeNum ;
				}
				if ( typeNum == typeUint8 )
				{
					typeNum = typeInt16 ;
				}
				else if ( typeNum == typeUint16 )
				{
					typeNum = typeInt32 ;
				}
				else if ( typeNum == typeUint32 )
				{
					typeNum = typeInt64 ;
				}
				tiExpr.m_typeNum = typeNum ;
			}
			m_xmm->WriteCodeMoveRegReg
				( tiExpr.m_regLoaded, tiExpr2.m_regLoaded ) ;
		}
		else
		{
			OutputError( L"? : 演算子で各項の評価型が一致しません" ) ;
			return	false ;
		}
	}
	else if ( tiExpr.IsFloatingPoint() )
	{
		if ( tiExpr2.IsInteger() && !flagObjectRef )
		{
			m_xmm->WriteCode2OP
				( codeCvtInt2Float, tiExpr.m_regLoaded, tiExpr2.m_regLoaded ) ;
		}
		else if ( tiExpr2.IsFloatingPoint() && !flagObjectRef )
		{
			m_xmm->WriteCodeMoveRegReg
				( tiExpr.m_regLoaded, tiExpr2.m_regLoaded ) ;
		}
		else
		{
			OutputError( L"? : 演算子で各項の評価型が一致しません" ) ;
			return	false ;
		}
	}
	else if ( tiExpr.m_flagPointer )
	{
		if ( !tiExpr2.m_flagPointer
			|| (tiExpr.m_pClass != tiExpr2.m_pClass) )
		{
			OutputError( L"? : 演算子で各項の評価型が一致しません" ) ;
			return	false ;
		}
		m_xmm->WriteCodeMoveRegReg
			( tiExpr.m_regLoaded, tiExpr2.m_regLoaded ) ;
		//
		ESLAssert( tiExpr.m_regObject >= 0 ) ;
		ESLAssert( tiExpr.m_flagRefRosetta == tiExpr2.m_flagRefRosetta ) ;
		if ( (tiExpr2.m_regObject >= 0)
			&& (tiExpr.m_flagRefRosetta == tiExpr2.m_flagRefRosetta) )
		{
			m_xmm->WriteCodeMoveRegReg
				( tiExpr.m_regObject, tiExpr2.m_regObject ) ;
		}
		else
		{
			OutputError( L"? : 演算子で各項の評価型が一致しません" ) ;
			return	false ;
		}
		tiExpr2.m_regObject = -1 ;
	}
	else
	{
		if ( tiExpr.m_pClass != tiExpr2.m_pClass )
		{
			OutputError( L"? : 演算子で各項の評価型が一致しません" ) ;
			return	false ;
		}
		ESLAssert( tiExpr.IsObject() ) ;
		m_xmm->WriteCodeMoveRegReg
			( tiExpr.m_regLoaded, tiExpr2.m_regLoaded ) ;
		if ( tiExpr2.m_regObject >= 0 )
		{
			m_xmm->WriteCodeMoveRegReg
				( tiExpr.m_regObject, tiExpr2.m_regObject ) ;
		}
		tiExpr2.m_regObject = -1 ;
	}
	FreeTemporaryRegister( tiExpr2 ) ;
	FlushAllLocalVariableCaches() ;
	//
	m_xmm->CommitJumpAddress
		( addrJump, m_xmm->GetNextCodeAddress() ) ;
	//
	if ( !flagObjectRef )
	{
		FreeTemporaryRegister( regObjTemp ) ;
	}
	return	true ;
}

// 関数呼び出し
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::CompileCallFunction
	( RSCompiler::TypeInfo& tiThisRet,
		RSFunctionObject& func, bool flagVirtual,
		SSystem::SObjectArray<RSCompiler::TypeInfo>& arg, bool fStructCast )
{
	size_t	iVirtual ;
	RSFunctionPrototype *	pProto = GetMatchPrototype( iVirtual, func, arg ) ;
	if ( pProto == NULL )
	{
		OutputError( func.m_strFuncName
					+ L" 関数の呼び出しで引数がプロトタイプに一致しません" ) ;
		return	false ;
	}
	if ( tiThisRet.m_accMod & RSObject::modifierConst )
	{
		if ( !pProto->IsConstantModifier() )
		{
			OutputError( L"const ポインタから非 const な関数を呼び出しています" ) ;
			return	false ;
		}
	}
	if ( fStructCast )
	{
		RSStructuredPointerClass *	pThisStructClass =
			ESLTypeCast<RSStructuredPointerClass>( pProto->m_pNamespaceClass ) ;
		if ( pThisStructClass != NULL )
		{
			if ( !OperatorCast( tiThisRet, pThisStructClass ) )
			{
				return	false ;
			}
		}
	}
	//
	// Class の static 関数の呼び出し判定
	//
	RSClass *	pThisClass =
					ESLTypeCast<RSClass>( tiThisRet.GetImmediate() ) ;
	if ( (pThisClass != NULL)
		&& (pProto->m_methodNative.pfnMethod != NULL) )
	{
		if ( CompileCallSystemFunction
				( tiThisRet, pThisClass, pProto, arg ) )
		{
			return	true ;
		}
	}
	//
	// 引数をオブジェクトへ変換
	//
	if ( !ConvertToObject( tiThisRet ) )
	{
		return	false ;
	}
	if ( !ConvertArgumentToObject( arg ) )
	{
		return	false ;
	}
	//
	// ソース位置
	//
	FlushAllLocalVariableCaches() ;
	//
	int	regArg0 = GetFreeTemporaryRegister( 2 ) ;
	int	regArg1 = regArg0 + 1 ;
	m_xmm->WriteCodeMoveRegInt64( regArg0, (ulong_ptr_t) m_pCurParenthesis ) ;
	m_xmm->WriteCodeMoveRegInt64( regArg1, m_iSrcStatement ) ;
	//
	m_xmm->WriteCodePushRegsImm8( regArg0, 2 ) ;
	m_xmm->WriteCodeSyscall( L"__nrs_set_currrent_position" ) ;
	m_xmm->WriteCodeAddSP( 16 ) ;
	//
	ExprRegContext	xrc ;
	PushAllTemporaryRegisters( xrc ) ;
	//
	// 引数プッシュ
	//
	CompilePushArgument( arg ) ;
	//
	ESLAssert( tiThisRet.m_flagLoaded ) ;
	m_xmm->WriteCodePushReg( tiThisRet.m_regLoaded ) ;
	//
	if ( flagVirtual )
	{
		m_xmm->WriteCodeMoveRegInt64( regAcc, iVirtual ) ;
		m_xmm->WriteCodePushReg( regAcc ) ;
		m_xmm->WriteCodeMoveRegInt64
			( regAcc, (ulong_ptr_t) ((const wchar_t*) func.m_strFuncName) ) ;
		m_xmm->WriteCodePushReg( regAcc ) ;
		//
		m_xmm->WriteCodeSyscall( L"__nrs_call_function_virtual" ) ;
		m_xmm->WriteCodeAddSP( ((int) arg.GetLength() + 4) * 8 ) ;
	}
	else
	{
		m_xmm->WriteCodeMoveRegInt64( regAcc, (ulong_ptr_t) pProto ) ;
		m_xmm->WriteCodePushReg( regAcc ) ;
		//
		m_xmm->WriteCodeSyscall( L"__nrs_call_function_proto" ) ;
		m_xmm->WriteCodeAddSP( ((int) arg.GetLength() + 3) * 8 ) ;
	}
	//
	PopAllTemporaryRegisters( xrc ) ;
	//
	// 返り値
	//
	m_xmm->WriteCodePushReg( regAcc ) ;
	//
	FreeTemporaryRegisters( arg ) ;
	FreeTemporaryRegister( tiThisRet ) ;
	//
	tiThisRet.SetLoadedObject
		( *m_context, pProto->m_pReturnType, AllocateTemporaryRegister() ) ;
	//
	m_xmm->WriteCodePopReg( tiThisRet.m_regObject ) ;
	//
	CompileCheckException( true ) ;
	return	true ;
}

bool RSCompiler::CompileCallFunction
	( RSCompiler::TypeInfo& tiThisRet,
		RSCompiler::TypeInfo& tiFunc,
		SSystem::SObjectArray<TypeInfo>& arg )
{
	//
	// 引数をオブジェクトへ変換
	//
	if ( !ConvertToObject( tiThisRet ) )
	{
		return	false ;
	}
	if ( !ConvertToObject( tiFunc ) )
	{
		return	false ;
	}
	if ( !ConvertArgumentToObject( arg ) )
	{
		return	false ;
	}
	//
	// ソース位置
	//
	FlushAllLocalVariableCaches() ;
	//
	int	regArg0 = GetFreeTemporaryRegister( 2 ) ;
	int	regArg1 = regArg0 + 1 ;
	m_xmm->WriteCodeMoveRegInt64( regArg0, (ulong_ptr_t) m_pCurParenthesis ) ;
	m_xmm->WriteCodeMoveRegInt64( regArg1, m_iSrcStatement ) ;
	//
	m_xmm->WriteCodePushRegsImm8( regArg0, 2 ) ;
	m_xmm->WriteCodeSyscall( L"__nrs_set_currrent_position" ) ;
	m_xmm->WriteCodeAddSP( 16 ) ;
	//
	ExprRegContext	xrc ;
	PushAllTemporaryRegisters( xrc ) ;
	//
	// 引数プッシュ
	//
	CompilePushArgument( arg ) ;
	//
	ESLAssert( tiThisRet.m_flagLoaded ) ;
	m_xmm->WriteCodePushReg( tiThisRet.m_regLoaded ) ;
	//
	ESLAssert( tiFunc.m_flagLoaded ) ;
	m_xmm->WriteCodePushReg( tiFunc.m_regLoaded ) ;
	//
	m_xmm->WriteCodeSyscall( L"__nrs_call_function_obj" ) ;
	m_xmm->WriteCodeAddSP( ((int) arg.GetLength() + 3) * 8 ) ;
	//
	PopAllTemporaryRegisters( xrc ) ;
	//
	// 返り値
	//
	m_xmm->WriteCodePushReg( regAcc ) ;
	//
	FreeTemporaryRegisters( arg ) ;
	FreeTemporaryRegister( tiFunc ) ;
	FreeTemporaryRegister( tiThisRet ) ;
	//
	RSFunctionPrototype *	pProto = nullptr ;
	RSFunctionClass *	pFuncClass = ESLTypeCast<RSFunctionClass>( tiFunc.m_pClass ) ;
	if ( pFuncClass != nullptr )
	{
		pProto = pFuncClass->m_pProto ;
		if ( pProto == nullptr )
		{
			RSGenericFunctionClass *
				pGenFuncClass = ESLTypeCast<RSGenericFunctionClass>( pFuncClass ) ;
			if ( pGenFuncClass != nullptr )
			{
				pProto = pGenFuncClass->m_pProtoGen ;
			}
		}
	}
	RSClass *	pRetType = nullptr ;
	if ( pProto != nullptr )
	{
		pRetType = pProto->m_pReturnType ;
	}
	tiThisRet.SetLoadedObject
		( *m_context, pRetType, AllocateTemporaryRegister() ) ;
	//
	m_xmm->WriteCodePopReg( tiThisRet.m_regObject ) ;
	//
	CompileCheckException() ;
	return	true ;
}

// システム定義済み関数判定
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::CompileCallSystemFunction
	( RSCompiler::TypeInfo& tiThisRet,
		RSClass * pThisClass,
		RSFunctionPrototype * pProto,
		SSystem::SObjectArray<TypeInfo>& arg )
{
	RSObject::METHOD_PROC
				pfnMethod = pProto->m_methodNative.pfnMethod ;
	TypeInfo *	ptiArg0 = arg.GetAt( 0 ) ;
	if ( (ESLTypeCast<RSMathClass>( pThisClass ) != NULL)
			&& (pProto->m_methodNative.pfnMethod != NULL)
			&& (ptiArg0 != NULL) )
	{
		RSObject *	pImm0 = ptiArg0->GetImmediate() ;
		TypeInfo *	ptiArg1 = arg.GetAt( 1 ) ;
		RSObject *	pImm1 = NULL ;
		if ( ptiArg1 != NULL )
		{
			pImm1 = ptiArg1->GetImmediate() ;
		}
		if ( (pImm0 != NULL) && ((ptiArg1 == NULL) || (pImm1 != NULL)) )
		{
			double	arg0, arg1 ;
			if ( !pImm0->AsRealNumber( arg0 ) )
			{
				return	false ;
			}
			if ( pImm1 != NULL )
			{
				if ( !pImm1->AsRealNumber( arg1 ) )
				{
					return	false ;
				}
			}
			if ( pfnMethod == &RSMathClass::method_abs )
			{
				arg0 = fabs( arg0 ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_pow )
			{
				arg0 = pow( arg0, arg1 ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_sqrt )
			{
				arg0 = sqrt( arg0 ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_cos )
			{
				arg0 = cos( arg0 ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_sin )
			{
				arg0 = sin( arg0 ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_tan )
			{
				arg0 = tan( arg0 ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_log )
			{
				arg0 = log( arg0 ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_log10 )
			{
				arg0 = log( arg0 ) / log( 10.0 ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_floor )
			{
				arg0 = floor( arg0 ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_rint )
			{
				arg0 = (double) eslRoundR64ToLInt( arg0 ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_round )
			{
				FreeTemporaryRegisters( arg ) ;
				FreeTemporaryRegister( tiThisRet ) ;
				//
				tiThisRet.SetImmediate
					( *m_context,
						m_context->new_Integer( eslRoundR64ToLInt( arg0 ) ),
						m_context->GetIntegerClass() ) ;
				return	true ;
			}
			else if ( pfnMethod == &RSMathClass::method_acos )
			{
				arg0 = acos( arg0 ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_asin )
			{
				arg0 = asin( arg0 ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_atan )
			{
				arg0 = atan( arg0 ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_atan2 )
			{
				arg0 = atan2( arg0, arg1 ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_max )
			{
				arg0 = esl_fmax( arg0, arg1 ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_min )
			{
				arg0 = esl_fmin( arg0, arg1 ) ;
			}
			else
			{
				return	false ;
			}
			FreeTemporaryRegisters( arg ) ;
			FreeTemporaryRegister( tiThisRet ) ;
			//
			tiThisRet.SetImmediate
				( *m_context,
					m_context->new_Number( arg0 ),
					m_context->GetNumberClass() ) ;
			return	true ;
		}
		else
		{
			if ( !LoadReferenceTemporary( *ptiArg0, true )
				|| !RealizeObjectToNumber( *ptiArg0 )
				|| !OperatorCast( *ptiArg0, m_context->GetNumberClass() ) )
			{
				return	false ;
			}
			ESLAssert( ptiArg0->m_flagLoaded ) ;
			//
			if ( ptiArg1 != NULL )
			{
				if ( !LoadReferenceTemporary( *ptiArg1, true )
					|| !RealizeObjectToNumber( *ptiArg1 )
					|| !OperatorCast( *ptiArg1, m_context->GetNumberClass() ))
				{
					return	false ;
				}
				ESLAssert( ptiArg1->m_flagLoaded ) ;
			}
			if ( pfnMethod == &RSMathClass::method_abs )
			{
				m_xmm->WriteCodeFloat64_2OP
					( fcodeFabs, ptiArg0->m_regLoaded, ptiArg0->m_regLoaded ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_pow )
			{
				if ( ptiArg1 == NULL )
				{
					return	false ;
				}
				m_xmm->WriteCodeFloat64_2OP
					( fcodePow, ptiArg0->m_regLoaded, ptiArg1->m_regLoaded ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_sqrt )
			{
				m_xmm->WriteCodeFloat64_2OP
					( fcodeSqrt, ptiArg0->m_regLoaded, ptiArg0->m_regLoaded ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_cos )
			{
				m_xmm->WriteCodeFloat64_2OP
					( fcodeCos, ptiArg0->m_regLoaded, ptiArg0->m_regLoaded ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_sin )
			{
				m_xmm->WriteCodeFloat64_2OP
					( fcodeSin, ptiArg0->m_regLoaded, ptiArg0->m_regLoaded ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_tan )
			{
				m_xmm->WriteCodeFloat64_2OP
					( fcodeTan, ptiArg0->m_regLoaded, ptiArg0->m_regLoaded ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_log )
			{
				m_xmm->WriteCodeFloat64_2OP
					( fcodeLog, ptiArg0->m_regLoaded, ptiArg0->m_regLoaded ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_log10 )
			{
				m_xmm->WriteCodeMoveRegFloat64( regAcc, 1.0 / log( 10.0 ) ) ;
				m_xmm->WriteCodeFloat64_2OP
					( fcodeLog, ptiArg0->m_regLoaded, ptiArg0->m_regLoaded ) ;
				m_xmm->WriteCode2OP
					( codeFMulReg, ptiArg0->m_regLoaded, regAcc ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_floor )
			{
				m_xmm->WriteCodeFloat64_2OP
					( fcodeFloor, ptiArg0->m_regLoaded, ptiArg0->m_regLoaded ) ;
				m_xmm->WriteCode2OP
					( codeCvtInt2Float, ptiArg0->m_regLoaded, ptiArg0->m_regLoaded ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_rint )
			{
				m_xmm->WriteCode2OP
					( codeCvtFloat2Int, ptiArg0->m_regLoaded, ptiArg0->m_regLoaded ) ;
				m_xmm->WriteCode2OP
					( codeCvtInt2Float, ptiArg0->m_regLoaded, ptiArg0->m_regLoaded ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_round )
			{
				m_xmm->WriteCode2OP
					( codeCvtFloat2Int, ptiArg0->m_regLoaded, ptiArg0->m_regLoaded ) ;
				//
				MoveTemporaryTypeInfo( tiThisRet, *ptiArg0 ) ;
				tiThisRet.ChangeType( *m_context, m_context->GetIntegerClass() ) ;
				//
				FreeTemporaryRegisters( arg ) ;
				return	true ;
			}
			else if ( pfnMethod == &RSMathClass::method_acos )
			{
				m_xmm->WriteCodeFloat64_2OP
					( fcodeACos, ptiArg0->m_regLoaded, ptiArg0->m_regLoaded ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_asin )
			{
				m_xmm->WriteCodeFloat64_2OP
					( fcodeASin, ptiArg0->m_regLoaded, ptiArg0->m_regLoaded ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_atan )
			{
				m_xmm->WriteCodeFloat64_2OP
					( fcodeATan, ptiArg0->m_regLoaded, regFloatOne ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_atan2 )
			{
				if ( ptiArg1 == NULL )
				{
					return	false ;
				}
				m_xmm->WriteCodeFloat64_2OP
					( fcodeATan, ptiArg0->m_regLoaded, ptiArg1->m_regLoaded ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_max )
			{
				if ( ptiArg1 == NULL )
				{
					return	false ;
				}
				m_xmm->WriteCodeMoveRegReg
					( regAcc, ptiArg0->m_regLoaded ) ;
				m_xmm->WriteCode2OP
					( codeCmpLtReg, regAcc, ptiArg1->m_regLoaded ) ;
				m_xmm->WriteCode3OP
					( codeMaskMove,
						ptiArg0->m_regLoaded, ptiArg1->m_regLoaded, regAcc ) ;
			}
			else if ( pfnMethod == &RSMathClass::method_min )
			{
				if ( ptiArg1 == NULL )
				{
					return	false ;
				}
				m_xmm->WriteCodeMoveRegReg
					( regAcc, ptiArg0->m_regLoaded ) ;
				m_xmm->WriteCode2OP
					( codeCmpGtReg, regAcc, ptiArg1->m_regLoaded ) ;
				m_xmm->WriteCode3OP
					( codeMaskMove,
						ptiArg0->m_regLoaded, ptiArg1->m_regLoaded, regAcc ) ;
			}
			else
			{
				return	false ;
			}
			//
			MoveTemporaryTypeInfo( tiThisRet, *ptiArg0 ) ;
			tiThisRet.ChangeType( *m_context, m_context->GetNumberClass() ) ;
			//
			FreeTemporaryRegisters( arg ) ;
			return	true ;
		}
	}
	return	false ;
}

// 関数引数解釈
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::CompileArgument
	( SSystem::SObjectArray<RSCompiler::TypeInfo>& arg, RSParenthesis& prth )
{
	RSCodeStream	csArg( prth ) ;
	while ( !csArg.IsEndOfStream() )
	{
		TypeInfo *	ptiArg = new TypeInfo( this ) ;
		arg.Add( ptiArg ) ;
		//
		if ( !CompileExpression
			( *ptiArg, csArg, RSCodeOperator::priorityList ) )
		{
			return	false ;
		}
		if ( csArg.NextOperator( RSCodeOperator::opSequencing ) == NULL )
		{
			if ( csArg.IsEndOfStream() )
			{
				break ;
			}
			OutputError( L"引数が \',\' で区切られていません" ) ;
			return	false ;
		}
	}
	return	true ;
}

// 関数引数をオブジェクトに変換する
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::ConvertArgumentToObject( SSystem::SObjectArray<TypeInfo>& arg )
{
	for ( size_t i = 0; i < arg.GetLength(); i ++ )
	{
		TypeInfo *	pti = arg.GetAt( i ) ;
		ESLAssert( pti != NULL ) ;
		if ( !ConvertToObject( *pti ) )
		{
			return	false ;
		}
	}
	return	true ;
}

// 関数引数プッシュ（個数＋引数）
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompilePushArgument( SSystem::SObjectArray<TypeInfo>& arg )
{
	bool	fArgSeq = true ;
	int		regArg = -1 ;
	for ( size_t i = 0; i < arg.GetLength(); i ++ )
	{
		TypeInfo *	pti = arg.GetAt( i ) ;
		ESLAssert( pti != NULL ) ;
		ESLAssert( pti->m_flagLoaded ) ;
		if ( regArg < 0 )
		{
			regArg = pti->m_regLoaded ;
		}
		else if ( regArg + i != pti->m_regLoaded )
		{
			fArgSeq = false ;
			break ;
		}
	}
	if ( fArgSeq )
	{
		if ( arg.GetLength() > 0 )
		{
			m_xmm->WriteCodePushRegsImm8( regArg, (int) arg.GetLength() ) ;
		}
	}
	else
	{
		for ( size_t i = 0; i < arg.GetLength(); i ++ )
		{
			TypeInfo *	pti = arg.GetLastAt( i ) ;
			ESLAssert( pti != NULL ) ;
			ESLAssert( pti->m_flagLoaded ) ;
			m_xmm->WriteCodePushReg( pti->m_regLoaded ) ;
		}
	}
	//
	m_xmm->WriteCodeMoveRegInt64( regAcc, arg.GetLength() ) ;
	m_xmm->WriteCodePushReg( regAcc ) ;
}

// 関数適合プロトタイプ取得
//////////////////////////////////////////////////////////////////////////////
RSFunctionPrototype * RSCompiler::GetMatchPrototype
	( size_t& iVirtual,
		RSFunctionObject& func, SSystem::SObjectArray<RSCompiler::TypeInfo>& arg )
{
	for ( size_t i = 0; i < func.m_arrPrototypes.GetLength(); i ++ )
	{
		RSFunctionPrototype *	pProto = func.m_arrPrototypes.GetAt( i ) ;
		ESLAssert( pProto != NULL ) ;
		if ( pProto == NULL )
		{
			continue ;
		}
		if ( IsMatchPrototype( pProto, arg ) )
		{
			iVirtual = i ;
			return	pProto ;
		}
	}
	return	NULL ;
}

bool RSCompiler::IsMatchPrototype
	( RSFunctionPrototype * pProto, SSystem::SObjectArray<TypeInfo>& arg )
{
	size_t	nProtoArgs = pProto->m_aArgTypes.GetLength() ;
	if ( (nProtoArgs < arg.GetLength())
		&& !(pProto->m_nFlags & RSFunctionPrototype::flagVarArg) )
	{
		return	false ;
	}
	if ( nProtoArgs > arg.GetLength() )
	{
		size_t	nDefArgs = pProto->m_aArgDefault.GetLength() ;
		if ( nDefArgs < nProtoArgs )
		{
			return	false ;
		}
		for ( size_t i = arg.GetLength(); i < nProtoArgs; i ++ )
		{
			if ( pProto->m_aArgDefault.GetAt( i ) == NULL )
			{
				return	false ;
			}
		}
	}
	for ( size_t i = 0; i < arg.GetLength(); i ++ )
	{
		RSClass *	pArgType = pProto->m_aArgTypes.GetAt( i ) ;
		if ( pArgType == NULL )
		{
			if ( (i < pProto->m_aArgTypes.GetLength())
				|| (pProto->m_nFlags & RSFunctionPrototype::flagVarArg) )
			{
				continue ;
			}
			return	false ;
		}
		TypeInfo *	ptiArg = arg.GetAt( i ) ;
		ESLAssert( ptiArg != NULL ) ;
		if ( ptiArg->m_pClass != NULL )
		{
			if ( !ptiArg->m_pClass->IsInstanceOf( pArgType ) )
			{
				if ( !ptiArg->m_flagPointer
					&& ((ptiArg->m_typeNum != typeObject)
						|| ESLTypeCast<RSNumberClass>( ptiArg->m_pClass )
						|| ESLTypeCast<RSIntegerClass>( ptiArg->m_pClass )) )
				{
					TypeInfo	tiArg( NULL ) ;
					tiArg.SetType( *m_context, pArgType ) ;
					if ( tiArg.m_flagPointer
						|| (tiArg.m_typeNum == typeObject) )
					{
						return	false ;
					}
				}
				else
				{
					return	false ;
				}
			}
		}
	}
	return	true ;
}

// 一時オブジェクト参照の解放
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::FreeTemporaryRegisters( SSystem::SObjectArray<RSCompiler::TypeInfo>& arg )
{
	for ( size_t i = 0; i < arg.GetLength(); i ++ )
	{
		TypeInfo *	ptiArg = arg.GetAt( i ) ;
		ESLAssert( ptiArg != NULL ) ;
		FreeTemporaryRegister( *ptiArg ) ;
	}
}

// シンボル参照
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::GetVariableAs
	( RSCompiler::TypeInfo& tiExpr,
		const wchar_t * pwszName /* must be static */ )
{
	// ローカル変数
	LocalVariable *	plv = SearchLocalVariable( pwszName ) ;
	if ( plv != NULL )
	{
		tiExpr.SetVarReference( *m_context, plv ) ;
		return	true ;
	}
	if ( m_plvThis && m_plvThis->pClass )
	{
		RSStructuredPointerClass *	pStructClass =
			ESLTypeCast<RSStructuredPointerClass>( m_plvThis->pClass ) ;
		if ( pStructClass != NULL )
		{
			// 構造体メンバ
			RSStructuredPointerClass::ElementInfo *
				peiMember = pStructClass->GetArrayMemberAs( pwszName ) ;
			if ( peiMember != NULL )
			{
				ReferenceToLocalVariable( tiExpr, m_plvThis ) ;
				return	ReferenceStructureMember( tiExpr, pwszName, peiMember ) ;
			}
		}
		else if ( m_plvThis->pClass->m_pPrototype )
		{
			// クラスメンバ
			RSObject *	pMember =
				m_plvThis->pClass->m_pPrototype->
							GetMemberAs( *m_context, pwszName ) ;
			if ( pMember != NULL )
			{
				RSClass *	pClass = pMember->GetEntityClass() ;
				tiExpr.SetLoadedObject
					( *m_context, pClass, AllocateTemporaryRegister() ) ;
				tiExpr.m_accMod = pMember->GetModifiers() ;
				if ( m_pPrototype->IsConstantModifier() )
				{
					tiExpr.m_accMod |= RSObject::modifierConst ;
				}
				//
				int	regArg0 = GetFreeTemporaryRegister() ;
				int	regArg1 = regArg0 + 1 ;
				LoadLocalVariable( m_plvThis, regArg0 ) ;
				m_xmm->WriteCodeMoveRegInt64
					( regArg1, (ulong_ptr_t) pwszName ) ;
				ESLAssert( pwszName != NULL ) ;
				m_xmm->WriteCodePushRegsImm8( regArg0, 2 ) ;
				m_xmm->WriteCodeSyscall( L"__nrs_get_member_as" ) ;
				m_xmm->WriteCodeAddSP( 16 ) ;
				m_xmm->WriteCodeMoveRegReg( tiExpr.m_regLoaded, regAcc ) ;
				//
				m_context->ReleaseObjectRef( pMember ) ;
				//
				CompileCheckException() ;
				return	true ;
			}
		}
		// メンバ関数
		RSFunctionObject *	pVirtFunc =
			m_plvThis->pClass->GetVirtualMemberAs( *m_context, pwszName ) ;
		if ( pVirtFunc != NULL )
		{
			if ( m_ptiExprParentOf != NULL )
			{
				FreeTemporaryRegister( *m_ptiExprParentOf ) ;
				delete	m_ptiExprParentOf ;
			}
			m_ptiExprParentOf = new TypeInfo( this ) ;
			ReferenceToLocalVariable( *m_ptiExprParentOf, m_plvThis ) ;
			//
			tiExpr.SetImmediate
				( *m_context, pVirtFunc, pVirtFunc->GetRSClass() ) ;
			return	true ;
		}
		// static メンバ変数
		RSObject *	pMember =
			m_plvThis->pClass->GetMemberAs( *m_context, pwszName ) ;
		if ( pMember != NULL )
		{
			RSClass *	pMemberClass = ESLTypeCast<RSClass>( pMember ) ;
			if ( pMemberClass != NULL )
			{
				tiExpr.SetImmediate
					( *m_context, pMemberClass, pMemberClass->GetRSClass() ) ;
				return	true ;
			}
			RSClass *	pClass = pMember->GetEntityClass() ;
			tiExpr.SetLoadedObject
				( *m_context, pClass, AllocateTemporaryRegister() ) ;
			tiExpr.m_accMod = pMember->GetModifiers() ;
			if ( m_pPrototype->IsConstantModifier() )
			{
				tiExpr.m_accMod |= RSObject::modifierConst ;
			}
			//
			int	regArg0 = GetFreeTemporaryRegister() ;
			int	regArg1 = regArg0 + 1 ;
			m_xmm->WriteCodeMoveRegInt64
				( regArg0, (ulong_ptr_t) (m_plvThis->pClass) ) ;
			m_xmm->WriteCodeMoveRegInt64
				( regArg1, (ulong_ptr_t) pwszName ) ;
			ESLAssert( pwszName != NULL ) ;
			m_xmm->WriteCodePushRegsImm8( regArg0, 2 ) ;
			m_xmm->WriteCodeSyscall( L"__nrs_get_member_as" ) ;
			m_xmm->WriteCodeAddSP( 16 ) ;
			m_xmm->WriteCodeMoveRegReg( tiExpr.m_regLoaded, regAcc ) ;
			//
			m_context->ReleaseObjectRef( pMember ) ;
			//
			CompileCheckException() ;
			return	true ;
		}
	}
	// クラス
	RSClass *	pClass = m_context->GetClassAs( pwszName ) ;
	if ( pClass != NULL )
	{
		pClass->AddRef();
		tiExpr.SetImmediate( *m_context, pClass, pClass->GetRSClass() ) ;
		return	true ;
	}
	// 大域変数
	RSObject *	pObj =
		m_context->GetVM()->GetMemberAs( *m_context, pwszName ) ;
	if ( pObj != NULL )
	{
		if ( pObj->GetModifiers() & RSObject::modifierConst )
		{
			if ( (pObj->GetBasicType() == RSObject::typeNumber)
				|| (pObj->GetBasicType() == RSObject::typeInteger)
				|| (pObj->GetBasicType() == RSObject::typeBoolean) )
			{
				RSObject *	pImm = pObj->CloneObject( *m_context ) ;
				tiExpr.SetImmediate( *m_context, pImm, pObj->GetRSClass() ) ;
				m_context->ReleaseObjectRef( pObj ) ;
				return	true ;
			}
			else if ( pObj->GetBasicType() == RSObject::typeFunction )
			{
				tiExpr.SetImmediate( *m_context, pObj, pObj->GetRSClass() ) ;
				return	true ;
			}
		}
		RSClass *	pClass = pObj->GetEntityClass() ;
		tiExpr.SetLoadedObject
			( *m_context, pClass, AllocateTemporaryRegister() ) ;
		tiExpr.m_accMod = pObj->GetModifiers() ;
		//
		int	regArg0 = GetFreeTemporaryRegister() ;
		int	regArg1 = regArg0 + 1 ;
		m_xmm->WriteCodeMoveRegInt64
			( regArg0, (ulong_ptr_t) m_context->GetVM() ) ;
		m_xmm->WriteCodeMoveRegInt64
			( regArg1, (ulong_ptr_t) pwszName ) ;
		ESLAssert( pwszName != NULL ) ;
		m_xmm->WriteCodePushRegsImm8( regArg0, 2 ) ;
		m_xmm->WriteCodeSyscall( L"__nrs_get_member_as" ) ;
		m_xmm->WriteCodeAddSP( 16 ) ;
		m_xmm->WriteCodeMoveRegReg( tiExpr.m_regLoaded, regAcc ) ;
		//
		m_context->ReleaseObjectRef( pObj ) ;
		//
		CompileCheckException() ;
		return	true ;
	}
	if ( SString::Compare( pwszName, L"null" ) == 0 )
	{
		pObj = m_context->new_Pointer( NULL ) ;
	}
	else if ( SString::Compare( pwszName, L"true" ) == 0 )
	{
		pObj = m_context->new_Boolean( true ) ;
	}
	else if ( SString::Compare( pwszName, L"false" ) == 0 )
	{
		pObj = m_context->new_Boolean( false ) ;
	}
	else if ( SString::Compare( pwszName, L"undefined" ) == 0 )
	{
		pObj = m_context->new_Reference( NULL ) ;
	}
	if ( pObj != NULL )
	{
		tiExpr.SetImmediate
			( *m_context, pObj, pObj->GetRSClass() ) ;
		return	true ;
	}
	OutputError( SString(L"自明でないシンボル \'") + pwszName + L"\'" ) ;
	return	false ;
}

// メンバへのアクセス保護判定と例外のスロー
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::VerifyMemberAccessModifier
	( RSClass * pClass, RSObject * pMember, const wchar_t * pwszName )
{
	if ( pMember == NULL )
	{
		return	true ;
	}
	if ( pMember->IsEnableAccessModifier( RSObject::modifierPublic ) )
	{
		return	true ;
	}
	if ( (m_plvThis != NULL) && (m_plvThis->pClass == pClass) )
	{
		return	true ;
	}
	OutputError( SString(pwszName) + L" は保護されたメンバです" ) ;
	return	false ;
}

// 構造体メンバ参照
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::ReferenceStructureMember
	( RSCompiler::TypeInfo& tiExpr,
		const wchar_t * pwszName,
		RSStructuredPointerClass::ElementInfo * peiMember )
{
	if ( (peiMember->m_accMod & RSObject::accessMask)
									>= RSObject::modifierProtected )
	{
		if ( !m_plvThis || (m_plvThis->pClass != peiMember->m_pClass) )
		{
			OutputError( SString(pwszName) + L" はプライベートなメンバ参照です" ) ;
			return	false ;
		}
	}
	if ( !LoadReferenceTemporary( tiExpr ) )
	{
		return	false ;
	}
	if ( tiExpr.IsObject() )
	{
		if ( !RealizeObjectToNumber( tiExpr ) )
		{
			return	false ;
		}
	}
	RSTypedArrayPointerClass *
		pPtrClass = ESLTypeCast<RSTypedArrayPointerClass>( peiMember->m_pClass ) ;
	if ( pPtrClass != NULL )
	{
		ESLAssert( tiExpr.m_flagLoaded ) ;
		TypeInfo	tiTemp( NULL ) ;
		tiTemp.SetType( *m_context, pPtrClass ) ;
		//
		if ( m_flagsBehaivor & behaviorFlatPointer )
		{
			tiExpr.m_typeNum = tiTemp.m_typeNum ;
			tiExpr.m_accMod =
				peiMember->m_accMod
					| (tiExpr.m_accMod & RSObject::modifierConst) ;
			tiExpr.m_flagPointer = true ;
			tiExpr.m_flagReference = false ;
			tiExpr.m_pClass = pPtrClass ;
			tiExpr.m_addrOffset += (int) peiMember->m_iOffset ;
			return	true ;
		}
		else
		{
			if ( !LoadReferenceTemporary( tiExpr, true ) )
			{
				return	false ;
			}
			int	regArg0 = AllocateTemporaryRegister() ;
			int	regArg1 = AllocateTemporaryRegister() ;
			ESLAssert( regArg1 == regArg0 + 1 ) ;
			//
			m_xmm->WriteCodeAddRegRegImm32
				( regArg0, tiExpr.m_regLoaded,
					tiExpr.m_addrOffset + (int) peiMember->m_iOffset ) ;
			m_xmm->WriteCodeAddRegRegImm32
				( regArg1, regIntZero, (int) peiMember->m_nBytes ) ;
			m_xmm->WriteCodePushRegsImm8( regArg0, 2 ) ;
			m_xmm->WriteCodeSyscall( L"__nrs_bound_pointer" ) ;
			m_xmm->WriteCodeAddSP( 16 ) ;
			//
			m_xmm->WriteCodeMoveRegReg( regArg0, regAcc ) ;
			//
			if ( tiExpr.m_regObject >= 0 )
			{
				if ( tiExpr.m_flagRefRosetta )
				{
					FreeTemporaryRegisterAndOwnerObject( tiExpr ) ;
					tiExpr.m_regObject = regArg0 ;
				}
				else
				{
					m_xmm->WriteCodePushReg( tiExpr.m_regObject ) ;
					m_xmm->WriteCodeSyscall( L"__nrs_release_ptr_ref" ) ;
					m_xmm->WriteCodeAddSP( 8 ) ;
					//
					m_xmm->WriteCodeMoveRegReg( tiExpr.m_regObject, regArg0 ) ;
				}
			}
			else
			{
				tiExpr.m_regObject = regArg0 ;
			}
			tiExpr.m_flagRefRosetta = false ;
			m_xmm->WriteCodeMoveRegReg
					( tiExpr.m_regLoaded, tiExpr.m_regObject ) ;
			//
			FreeTemporaryRegister( regArg1 ) ;
			if ( tiExpr.m_regObject != regArg0 )
			{
				FreeTemporaryRegister( regArg0 ) ;
			}
			tiExpr.m_typeNum = tiTemp.m_typeNum ;
			tiExpr.m_accMod =
				peiMember->m_accMod
					| (tiExpr.m_accMod & RSObject::modifierConst) ;
			tiExpr.m_flagPointer = true ;
			tiExpr.m_flagReference = false ;
			tiExpr.m_pClass = pPtrClass ;
			tiExpr.m_addrOffset = 0 ;
			//
			return	true ;
		}
	}
	else if ( peiMember->m_type != RSReferenceNumber::typeObject )
	{
		if ( !LoadReferenceTemporary( tiExpr ) )
		{
			return	false ;
		}
		tiExpr.m_typeNum = ConvertNumberType( peiMember->m_type ) ;
		tiExpr.m_accMod =
			peiMember->m_accMod
				| (tiExpr.m_accMod & RSObject::modifierConst) ;
		tiExpr.m_flagPointer = false ;
		tiExpr.m_flagReference = true ;
		tiExpr.m_pClass =
			NumberTypeToClass( ConvertNumberType( peiMember->m_type ) ) ;
		tiExpr.m_addrOffset += (int) peiMember->m_iOffset ;
		return	true ;
	}
	return	false ;
}

// new 演算子
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::CompileNewOperator( RSCompiler::TypeInfo& tiExpr, RSCodeStream& cstrm )
{
	//
	// new Type[]...[](...)
	//
	RSObject *	pTypeObj = m_context->ParseTypeExpression( cstrm ) ;
	if ( pTypeObj == NULL )
	{
		OutputError( L"new 演算子の型指定が不正です" ) ;
		return	false ;
	}
	size_t	iArrayDimStart = cstrm.GetIndex() ;
	int		nArrayDimension = 0 ;
	while ( cstrm.NextParenthesis
				( RSParenthesis::ptBracket ) != NULL )
	{
		nArrayDimension ++ ;
	}
	SObjectArray<TypeInfo>	arg ;
	RSObject *	pObj = NULL ;
	RSParenthesis *	pPrthArg =
		cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	if ( pPrthArg != NULL )
	{
		if ( !CompileArgument( arg, *pPrthArg ) )
		{
			m_context->ReleaseObjectRef( pTypeObj ) ;
			FreeTemporaryRegisters( arg ) ;
			return	false ;
		}
	}
	if ( pTypeObj->GetBasicType() != RSObject::typeClass )
	{
		// JavaScript 風インスタンス生成
		m_context->ReleaseObjectRef( pTypeObj ) ;
		FreeTemporaryRegisters( arg ) ;
		return	false ;
	}
	//
	// Java 風インスタンス生成
	//
	ESLAssert( pTypeObj->IsKindOf( ESL_RUNTIME_CLASS(RSClass) ) ) ;
	RSClass *	pClass = (RSClass*) pTypeObj ;
	if ( nArrayDimension > 0 )
	{
		//
		// 配列
		//
		pClass = m_context->GetVM()->
					GetArrayClassAs( pClass, nArrayDimension - 1 ) ;
		//
		size_t	iSaveIndex = cstrm.GetIndex() ;
		cstrm.SeekIndex( iArrayDimStart ) ;
		//
		RSParenthesis *	pBracket =
			cstrm.NextParenthesis( RSParenthesis::ptBracket ) ;
		ESLAssert( pBracket != NULL ) ;
		if ( pBracket->m_terms.GetLength() > 0 )
		{
			RSCodeStream	csLen( *pBracket ) ;
			TypeInfo	tiLen( this ) ;
			if ( !CompileExpression( tiLen, csLen )
				|| !LoadReferenceTemporary( tiLen )
				|| !RealizeObjectToNumber( tiLen ) )
			{
				m_context->ReleaseObjectRef( pTypeObj ) ;
				FreeTemporaryRegister( tiLen ) ;
				FreeTemporaryRegisters( arg ) ;
				return	false ;
			}
			ESLAssert( tiLen.m_flagLoaded ) ;
			m_xmm->WriteCodePushReg( tiLen.m_regLoaded ) ;
			m_xmm->WriteCodeMoveRegInt64( regAcc, (ulong_ptr_t) pClass ) ;
			m_xmm->WriteCodePushReg( regAcc ) ;
			m_xmm->WriteCodeSyscall( L"__nrs_new_Array" ) ;
			m_xmm->WriteCodeAddSP( 16 ) ;
			//
			m_xmm->WriteCodePushReg( regAcc ) ;
			FreeTemporaryRegister( tiLen ) ;
			m_xmm->WriteCodePopReg( regAcc ) ;
		}
		else
		{
			m_xmm->WriteCodeMoveRegInt64( regAcc, 0x7FFFFFFF ) ;
			m_xmm->WriteCodePushReg( regAcc ) ;
			m_xmm->WriteCodeMoveRegInt64( regAcc, (ulong_ptr_t) pClass ) ;
			m_xmm->WriteCodePushReg( regAcc ) ;
			m_xmm->WriteCodeSyscall( L"__nrs_new_Array" ) ;
			m_xmm->WriteCodeAddSP( 16 ) ;
		}
		cstrm.SeekIndex( iSaveIndex ) ;
		//
		tiExpr.SetLoadedObject
			( *m_context, pClass, AllocateTemporaryRegister() ) ;
		m_xmm->WriteCodeMoveRegReg( tiExpr.m_regLoaded, regAcc ) ;
	}
	else if ( (arg.GetLength() <= 1)
			&& pClass->IsKindOf( ESL_RUNTIME_CLASS(RSTypedArrayPointerClass) ) )
	{
		//
		// ポインタ・構造体
		//
		RSFunctionObject *		pConstructor = pClass->GetConstructor() ;
		RSFunctionPrototype *	pProto = NULL ;
		RSObject::METHOD_PROC	pfnMethod = NULL ;
		if ( pConstructor != NULL )
		{
			size_t	iVirtual ;
			pProto = GetMatchPrototype( iVirtual, *pConstructor, arg ) ;
			if ( pProto != NULL )
			{
				pfnMethod = pProto->m_methodNative.pfnMethod ;
			}
		}
		if ( pfnMethod == &RSStructureClass::method_init1 )
		{
			// <init>( int length = 1 )
			RSStructuredPointerClass *
				pPtrClass = ESLTypeCast<RSStructuredPointerClass>( pClass ) ;
			ESLAssert( pPtrClass != NULL ) ;
			//
			TypeInfo *	ptiArg = arg.GetAt( 0 ) ;
			if ( ptiArg != NULL )
			{
				if ( !LoadReferenceTemporary( *ptiArg )
					|| !RealizeObjectToNumber( *ptiArg ) )
				{
					FreeTemporaryRegisters( arg ) ;
					return	false ;
				}
				ESLAssert( ptiArg->m_flagLoaded ) ;
				m_xmm->WriteCodeMulRegRegImm32
					( regAcc, ptiArg->m_regLoaded,
						(int) pPtrClass->GetStructureBytes() ) ;
				m_xmm->WriteCodePushReg( regAcc ) ;
			}
			else
			{
				m_xmm->WriteCodeMoveRegInt64
					( regAcc, pPtrClass->GetStructureBytes() ) ;
				m_xmm->WriteCodePushReg( regAcc ) ;
			}
			m_xmm->WriteCodeSyscall( L"__nrs_create_pointer" ) ;
			m_xmm->WriteCodeAddSP( 8 ) ;
			//
			tiExpr.SetLoadedPointer
				( *m_context, pPtrClass,
					AllocateTemporaryRegister(), AllocateTemporaryRegister() ) ;
			m_xmm->WriteCodeMoveRegReg( tiExpr.m_regLoaded, regAcc ) ;
			m_xmm->WriteCodeMoveRegReg( tiExpr.m_regObject, regAcc ) ;
		}
		else if ( pfnMethod == &RSTypedArrayPointerClass::method_init1 )
		{
			// <init>( int length )
			RSTypedArrayPointerClass *
				pPtrClass = ESLTypeCast<RSTypedArrayPointerClass>( pClass ) ;
			ESLAssert( pPtrClass != NULL ) ;
			ESLAssert( arg.GetLength() == 1 ) ;
			//
			TypeInfo *	ptiArg = arg.GetAt( 0 ) ;
			ESLAssert( ptiArg != NULL ) ;
			if ( !LoadReferenceTemporary( *ptiArg )
				|| !RealizeObjectToNumber( *ptiArg ) )
			{
				FreeTemporaryRegisters( arg ) ;
				return	false ;
			}
			ESLAssert( ptiArg->m_flagLoaded ) ;
			size_t	nElementBytes =
				RSReferenceNumber::GetNumberSizeOf( pPtrClass->m_typeElement ) ;
			if ( nElementBytes > 1 )
			{
				m_xmm->WriteCodeMulRegRegImm32
					( regAcc, ptiArg->m_regLoaded, (int) nElementBytes ) ;
				m_xmm->WriteCodePushReg( regAcc ) ;
			}
			else
			{
				m_xmm->WriteCodePushReg( ptiArg->m_regLoaded ) ;
			}
			m_xmm->WriteCodeSyscall( L"__nrs_create_pointer" ) ;
			m_xmm->WriteCodeAddSP( 8 ) ;
			//
			tiExpr.SetLoadedPointer
				( *m_context, pPtrClass,
					AllocateTemporaryRegister(), AllocateTemporaryRegister() ) ;
			m_xmm->WriteCodeMoveRegReg( tiExpr.m_regLoaded, regAcc ) ;
			m_xmm->WriteCodeMoveRegReg( tiExpr.m_regObject, regAcc ) ;
		}
		else if ( (pfnMethod == &RSTypedArrayPointerClass::method_init2)
				|| (pfnMethod == &RSStructureClass::method_init2) )
		{
			// <init>( Uint8Pointer ptr )
			RSTypedArrayPointerClass *
				pPtrClass = ESLTypeCast<RSTypedArrayPointerClass>( pClass ) ;
			ESLAssert( pPtrClass != NULL ) ;
			ESLAssert( arg.GetLength() == 1 ) ;
			//
			TypeInfo *	ptiArg = arg.GetAt( 0 ) ;
			ESLAssert( ptiArg != NULL ) ;
			if ( !LoadReferenceTemporary( *ptiArg )
				|| !RealizeObjectToNumber( *ptiArg ) )
			{
				FreeTemporaryRegisters( arg ) ;
				return	false ;
			}
			TypeInfo	tiTemp( NULL ) ;
			tiTemp.SetType( *m_context, pPtrClass ) ;
			//
			MoveTemporaryTypeInfo( tiExpr, *ptiArg ) ;
			tiExpr.m_pClass = pPtrClass ;
			tiExpr.m_typeNum = tiTemp.m_typeNum ;
			tiExpr.m_flagPointer = tiTemp.m_flagPointer ;
			//
			ESLAssert( tiExpr.m_flagLoaded ) ;
			CompileCheckAlignment
				( tiExpr.m_regLoaded, (int) pPtrClass->GetAlignment() ) ;
		}
		else
		{
			RSTypedArrayPointerClass *
				pPtrClass = ESLTypeCast<RSTypedArrayPointerClass>( pClass ) ;
			ESLAssert( pPtrClass != NULL ) ;
			//
			if ( !ConvertArgumentToObject( arg ) )
			{
				FreeTemporaryRegisters( arg ) ;
				return	false ;
			}
			FlushAllLocalVariableCaches() ;
			//
			ExprRegContext	xrc ;
			PushAllTemporaryRegisters( xrc ) ;
			//
			CompilePushArgument( arg ) ;
			//
			m_xmm->WriteCodeMoveRegInt64( regAcc, (ulong_ptr_t) pClass ) ;
			m_xmm->WriteCodePushReg( regAcc ) ;
			//
			m_xmm->WriteCodeSyscall( L"__nrs_new_Pointer" ) ;
			m_xmm->WriteCodeAddSP( ((int) arg.GetLength() + 2) * 8 ) ;
			//
			PopAllTemporaryRegisters( xrc ) ;
			//
			tiExpr.SetLoadedPointer
				( *m_context, pPtrClass,
					AllocateTemporaryRegister(), AllocateTemporaryRegister() ) ;
			m_xmm->WriteCodeMoveRegReg( tiExpr.m_regLoaded, regAcc ) ;
			m_xmm->WriteCodeMoveRegReg( tiExpr.m_regObject, regAcc ) ;
			//
			CompileCheckException() ;
		}
	}
	else
	{
		//
		// オブジェクト
		//
		if ( !ConvertArgumentToObject( arg ) )
		{
			FreeTemporaryRegisters( arg ) ;
			return	false ;
		}
		FlushAllLocalVariableCaches() ;
		//
		ExprRegContext	xrc ;
		PushAllTemporaryRegisters( xrc ) ;
		//
		CompilePushArgument( arg ) ;
		//
		m_xmm->WriteCodeMoveRegInt64( regAcc, (ulong_ptr_t) pClass ) ;
		m_xmm->WriteCodePushReg( regAcc ) ;
		//
		m_xmm->WriteCodeSyscall( L"__nrs_new_Object" ) ;
		m_xmm->WriteCodeAddSP( ((int) arg.GetLength() + 2) * 8 ) ;
		//
		PopAllTemporaryRegisters( xrc ) ;
		//
		tiExpr.SetLoadedObject
			( *m_context, pClass, AllocateTemporaryRegister() ) ;
		m_xmm->WriteCodeMoveRegReg( tiExpr.m_regLoaded, regAcc ) ;
		//
		CompileCheckException() ;
	}
	//
	m_context->ReleaseObjectRef( pTypeObj ) ;
	FreeTemporaryRegisters( arg ) ;
	return	true ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::CompileUnaryOperator
	( RSCompiler::TypeInfo& tiExpr, RSCodeOperator::OperatorIndex opIndex )
{
	RSObject *	pObj = tiExpr.GetImmediate() ;
	if ( pObj != NULL )
	{
		pObj->AddRef() ;
		//
		RSObject *	pResult = m_context->ExecuteUnaryOperator( pObj, opIndex ) ;
		if ( (pResult == NULL) || m_context->IsException() )
		{
			pObj->ReleaseRef() ;
			return	false ;
		}
		tiExpr.SetImmediate( *m_context, pResult, pResult->GetRSClass() ) ;
		return	true ;
	}
	if ( tiExpr.IsObject() )
	{
		if ( (opIndex == RSCodeOperator::opIncrement)
			|| (opIndex == RSCodeOperator::opDecrement) )
		{
			if ( !LoadReferenceTemporary( tiExpr ) )
			{
				return	false ;
			}
		}
		else
		{
			if ( !LoadReferenceTemporary( tiExpr )
				|| !RealizeObjectToNumber( tiExpr ) )
			{
				return	false ;
			}
		}
	}
	if ( tiExpr.m_flagPointer )
	{
		//
		// ポインタへの単項演算子
		//
		RSTypedArrayPointerClass *
			pPtrClass = ESLTypeCast<RSTypedArrayPointerClass>( tiExpr.m_pClass ) ;
		ESLAssert( pPtrClass != NULL ) ;
		//
		switch ( opIndex )
		{
		case	RSCodeOperator::opIncrement:
			if ( tiExpr.m_accMod & RSObject::modifierConst )
			{
				OutputError( L"const オブジェクトへのインクリメントです" ) ;
				return	false ;
			}
			else if ( !tiExpr.m_flagReference || !tiExpr.m_flagLocal )
			{
				OutputError( L"左辺式ではないオブジェクトのインクリメントです" ) ;
				return	false ;
			}
			else
			{
				const LocalVariable *	plv = tiExpr.m_pLocalVar ;
				ESLAssert( plv != NULL ) ;
				if ( !LoadReferenceTemporary( tiExpr ) )
				{
					return	false ;
				}
				m_xmm->WriteCodeAddRegRegImm32
					( tiExpr.m_regLoaded, tiExpr.m_regLoaded,
							(int) pPtrClass->GetElementBytes() ) ;
				WriteLocalVariable( plv, tiExpr.m_regLoaded ) ;
			}
			break ;
		case	RSCodeOperator::opDecrement:
			if ( tiExpr.m_accMod & RSObject::modifierConst )
			{
				OutputError( L"const オブジェクトへのデクリメントです" ) ;
				return	false ;
			}
			else if ( !tiExpr.m_flagReference || !tiExpr.m_flagLocal )
			{
				OutputError( L"左辺式ではないオブジェクトのデクリメントです" ) ;
				return	false ;
			}
			else
			{
				const LocalVariable *	plv = tiExpr.m_pLocalVar ;
				ESLAssert( plv != NULL ) ;
				if ( !LoadReferenceTemporary( tiExpr ) )
				{
					return	false ;
				}
				m_xmm->WriteCodeAddRegRegImm32
					( tiExpr.m_regLoaded, tiExpr.m_regLoaded,
							- (int) pPtrClass->GetElementBytes() ) ;
				WriteLocalVariable( plv, tiExpr.m_regLoaded ) ;
			}
			break ;
		case	RSCodeOperator::opLogicalNot:
			if ( !LoadReferenceTemporary( tiExpr, true ) )
			{
				return	false ;
			}
			ESLAssert( tiExpr.m_flagLoaded ) ;
			ESLAssert( !tiExpr.m_flagLockAddr ) ;
			m_xmm->WriteCode2OP
				( codeCmpEqReg, tiExpr.m_regLoaded, regIntZero ) ;
			tiExpr.ChangeType
				( *m_context,
					m_context->GetBasicTypeClass( RSCodeControl::wiBoolean ) ) ;
			break ;
		default:
			OutputError
				( SString(L"ポインタエラー：")
					+ RSCodeOperator::m_pwszOperators[opIndex]
					+ L" は定義されない単項演算子です" ) ;
			return	false ;
		}
	}
	else if ( tiExpr.m_typeNum != typeObject )
	{
		//
		// 数値への単項演算子
		//
		switch ( opIndex )
		{
		case	RSCodeOperator::opAdd:
			break ;
		case	RSCodeOperator::opSub:
			if ( !LoadReferenceTemporary( tiExpr, true ) )
			{
				return	false ;
			}
			if ( tiExpr.IsInteger() )
			{
				m_xmm->WriteCode1OP( codeNegInt, tiExpr.m_regLoaded ) ;
			}
			else
			{
				m_xmm->WriteCode1OP( codeNegFloat, tiExpr.m_regLoaded ) ;
			}
			break ;
		case	RSCodeOperator::opBitNot:
			if ( !LoadReferenceTemporary( tiExpr, true ) )
			{
				return	false ;
			}
			if ( tiExpr.IsFloatingPoint() )
			{
				m_xmm->WriteCode2OP
					( codeCvtFloat2Int,
						tiExpr.m_regLoaded, tiExpr.m_regLoaded ) ;
				tiExpr.ChangeType
					( *m_context,
						m_context->GetBasicTypeClass( RSCodeControl::wiLong ) ) ;
			}
			m_xmm->WriteCode1OP( codeNotInt, tiExpr.m_regLoaded ) ;
			break ;
		case	RSCodeOperator::opIncrement:
		case	RSCodeOperator::opDecrement:
			if ( tiExpr.m_accMod & RSObject::modifierConst )
			{
				if ( opIndex == RSCodeOperator::opIncrement )
				{
					OutputError( L"const オブジェクトへのインクリメントです" ) ;
				}
				else
				{
					OutputError( L"const オブジェクトへのデクリメントです" ) ;
				}
				return	false ;
			}
			else if ( !tiExpr.m_flagReference )
			{
				if ( opIndex == RSCodeOperator::opIncrement )
				{
					OutputError( L"左辺式ではないオブジェクトのインクリメントです" ) ;
				}
				else
				{
					OutputError( L"左辺式ではないオブジェクトのデクリメントです" ) ;
				}
				return	false ;
			}
			else if ( tiExpr.IsFloatingPoint() )
			{
				if ( opIndex == RSCodeOperator::opIncrement )
				{
					OutputError( L"不正な浮動小数点のインクリメントです" ) ;
				}
				else
				{
					OutputError( L"不正な浮動小数点のデクリメントです" ) ;
				}
				return	false ;
			}
			else if ( tiExpr.m_flagLocal )
			{
				const LocalVariable *	plv = tiExpr.m_pLocalVar ;
				ESLAssert( plv != NULL ) ;
				if ( !LoadReferenceTemporary( tiExpr ) )
				{
					return	false ;
				}
				if ( opIndex == RSCodeOperator::opIncrement )
				{
					m_xmm->WriteCode2OP
						( codeAddReg, tiExpr.m_regLoaded, regIntOne ) ;
				}
				else
				{
					m_xmm->WriteCode2OP
						( codeSubReg, tiExpr.m_regLoaded, regIntOne ) ;
				}
				WriteLocalVariable( plv, tiExpr.m_regLoaded ) ;
			}
			else
			{
				int	regAddrBase = tiExpr.m_regLoaded ;
				int	addrOffset = tiExpr.m_addrOffset ;
				if ( tiExpr.m_flagLockAddr )
				{
					UnlockCacheRegister( regAddrBase ) ;
					tiExpr.m_regLoaded = AllocateTemporaryRegister() ;
					tiExpr.m_flagLockAddr = false ;
					//
					m_xmm->WriteCodeLoad
						( tiExpr.m_regLoaded,
							TypeInfoToDataType( tiExpr ),
							regZeroPtr, addrOffset, regAddrBase ) ;
					tiExpr.m_flagReference = false ;
				}
				else
				{
					m_xmm->WriteCodeMoveRegReg( regAcc, tiExpr.m_regLoaded ) ;
					if ( !LoadReferenceTemporary( tiExpr ) )
					{
						return	false ;
					}
					regAddrBase = regAcc ;
				}
				if ( opIndex == RSCodeOperator::opIncrement )
				{
					m_xmm->WriteCode2OP
						( codeAddReg, tiExpr.m_regLoaded, regIntOne ) ;
				}
				else
				{
					m_xmm->WriteCode2OP
						( codeSubReg, tiExpr.m_regLoaded, regIntOne ) ;
				}
				m_xmm->WriteCodeLoad
					( tiExpr.m_regLoaded,
						TypeInfoToDataType( tiExpr ), regAddrBase, addrOffset ) ;
			}
			break ;
		case	RSCodeOperator::opLogicalNot:
			if ( !LoadReferenceTemporary( tiExpr, true ) )
			{
				return	false ;
			}
			ESLAssert( tiExpr.m_flagLoaded ) ;
			ESLAssert( !tiExpr.m_flagLockAddr ) ;
			if ( tiExpr.IsInteger() )
			{
				m_xmm->WriteCode2OP
					( codeCmpEqReg, tiExpr.m_regLoaded, regIntZero ) ;
			}
			else
			{
				m_xmm->WriteCode2OP
					( codeFCmpEqReg, tiExpr.m_regLoaded, regIntZero ) ;
			}
			tiExpr.ChangeType
				( *m_context,
					m_context->GetBasicTypeClass( RSCodeControl::wiBoolean ) ) ;
			break ;
		default:
			OutputError
				( SString(RSCodeOperator::m_pwszOperators[opIndex])
					+ L" は定義されない単項演算子です" ) ;
			return	false ;
		}
	}
	else
	{
		//
		// オブジェクトへの単項演算子
		//
		if ( !LoadReferenceTemporary( tiExpr, true ) )
		{
			return	false ;
		}
		FlushAllLocalVariableCaches() ;
		//
		ExprRegContext	xrc ;
		PushAllTemporaryRegisters( xrc ) ;
		//
		m_xmm->WriteCodePushReg( tiExpr.m_regLoaded ) ;
		//
		RSClass *	pResultType = nullptr ;
		bool		flagResultNum = false ;
		switch ( opIndex )
		{
		case	RSCodeOperator::opAdd:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_plus" ) ;
			break ;
		case	RSCodeOperator::opSub:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_negate" ) ;
			break ;
		case	RSCodeOperator::opBitNot:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_bit_not" ) ;
			break ;
		case	RSCodeOperator::opIncrement:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_increment" ) ;
			break ;
		case	RSCodeOperator::opDecrement:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_decrement" ) ;
			break ;
		case	RSCodeOperator::opLogicalNot:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_logical_not" ) ;
			pResultType = m_context->GetBasicTypeClass( RSCodeControl::wiBoolean ) ;
			flagResultNum = true ;
			break ;
		default:
			OutputError
				( SString(RSCodeOperator::m_pwszOperators[opIndex])
					+ L" は定義されない単項演算子です" ) ;
			PopAllTemporaryRegisters( xrc ) ;
			return	false ;
		}
		m_xmm->WriteCodeAddSP( 8 ) ;
		//
		PopAllTemporaryRegisters( xrc ) ;
		//
		m_xmm->WriteCodePushReg( regAcc ) ;
		FreeTemporaryRegister( tiExpr ) ;
		if ( flagResultNum )
		{
			tiExpr.SetLoadedNumber
				( *m_context, pResultType, AllocateTemporaryRegister() ) ;
		}
		else
		{
			tiExpr.SetLoadedObject
				( *m_context, pResultType, AllocateTemporaryRegister() ) ;
		}
		m_xmm->WriteCodePopReg( tiExpr.m_regLoaded ) ;
		//
		CompileCheckException() ;
	}
	return	true ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::CompileBinaryOperator
	( RSCompiler::TypeInfo& tiExpr,
		RSCompiler::TypeInfo& tiRight,
		RSCodeOperator::OperatorIndex opIndex )
{
	RSObject *	pObjLeft = tiExpr.GetImmediate() ;
	RSObject *	pObjRight = tiRight.GetImmediate() ;
	if ( pObjLeft && pObjRight )
	{
		//
		// 即値同士の演算
		//
		pObjLeft = pObjLeft->CloneObject( *m_context ) ;
		pObjRight = pObjRight->CloneObject( *m_context ) ;
		pObjLeft = m_context->ExecuteBinaryOperator
							( pObjLeft, pObjRight, opIndex ) ;
		if ( pObjLeft != NULL )
		{
			tiExpr.SetImmediate
				( *m_context, pObjLeft, pObjLeft->GetRSClass() ) ;
		}
		else
		{
			tiExpr.SetImmediate( *m_context, NULL, NULL ) ;
		}
		return	!m_context->IsException() ;
	}
	if ( (tiExpr.m_accMod & RSObject::modifierConst)
		&& RSCodeOperator::IsMoveOperator( opIndex ) )
	{
		OutputError( L"const オブジェクトへの代入です" ) ;
		return	false ;
	}
	switch ( opIndex )
	{
	case	RSCodeOperator::opAdd:
	case	RSCodeOperator::opSub:
	case	RSCodeOperator::opMul:
	case	RSCodeOperator::opDiv:
	case	RSCodeOperator::opMod:
	case	RSCodeOperator::opBitAnd:
	case	RSCodeOperator::opBitOr:
	case	RSCodeOperator::opBitXor:
	case	RSCodeOperator::opShiftRight:
	case	RSCodeOperator::opShiftLeft:
	case	RSCodeOperator::opShiftRightArithmetic:
	case	RSCodeOperator::opEqual:
	case	RSCodeOperator::opNotEqual:
	case	RSCodeOperator::opLessEqual:
	case	RSCodeOperator::opLessThan:
	case	RSCodeOperator::opGraterEqual:
	case	RSCodeOperator::opGraterThan:
	case	RSCodeOperator::opPointerEqual:
	case	RSCodeOperator::opPointerNotEqual:
	case	RSCodeOperator::opLogicalAnd:
	case	RSCodeOperator::opLogicalOr:
	case	RSCodeOperator::opMove:
	case	RSCodeOperator::opMoveAdd:
	case	RSCodeOperator::opMoveSub:
	case	RSCodeOperator::opMoveMul:
	case	RSCodeOperator::opMoveDiv:
	case	RSCodeOperator::opMoveMod:
	case	RSCodeOperator::opMoveBitAnd:
	case	RSCodeOperator::opMoveBitOr:
	case	RSCodeOperator::opMoveBitXor:
	case	RSCodeOperator::opMoveShiftRight:
	case	RSCodeOperator::opMoveShiftLeft:
	case	RSCodeOperator::opMoveShiftRightArithmetic:
		break ;
	case	RSCodeOperator::opStaticMemberOf:
	case	RSCodeOperator::opMemberOf:
		OutputError
			( SString(L"内部エラー：")
				+ RSCodeOperator::m_pwszOperators[opIndex]
						+ L" は定義されない二項演算子です" ) ;
		return	false ;
	case	RSCodeOperator::opMemberCallOf:
		if ( m_ptiExprParentOf != NULL )
		{
			FreeTemporaryRegister( *m_ptiExprParentOf ) ;
			delete	m_ptiExprParentOf ;
		}
		m_ptiExprParentOf = new TypeInfo( this ) ;
		MoveTemporaryTypeInfo( *m_ptiExprParentOf, tiExpr ) ;
		MoveTemporaryTypeInfo( tiExpr, tiRight ) ;
		FreeTemporaryRegister( tiRight ) ;
		return	true ;
	case	RSCodeOperator::opInstanceOf:
		if ( ESLTypeCast<RSClass>( tiRight.GetImmediate() ) != NULL )
		{
			if ( !LoadReferenceTemporary( tiExpr )
				|| !ConvertToObject( tiExpr ) )
			{
				return	false ;
			}
			int	regArg0 = GetFreeTemporaryRegister( 2 ) ;
			int	regArg1 = regArg0 + 1 ;
			m_xmm->WriteCodeMoveRegReg( regArg0, tiExpr.m_regLoaded ) ;
			m_xmm->WriteCodeMoveRegInt64
					( regArg1, (ulong_ptr_t) tiRight.GetImmediate() ) ;
			m_xmm->WriteCodePushRegsImm8( regArg0, 2 ) ;
			m_xmm->WriteCodeSyscall( L"__nrs_operator_instance_of" ) ;
			m_xmm->WriteCodeAddSP( 16 ) ;
			m_xmm->WriteCodePushReg( regAcc ) ;
			//
			FreeTemporaryRegister( tiRight ) ;
			FreeTemporaryRegister( tiExpr ) ;
			//
			tiExpr.SetLoadedNumber
				( *m_context,
					m_context->GetBasicTypeClass
						( RSCodeControl::wiBoolean ),
					AllocateTemporaryRegister() ) ;
			m_xmm->WriteCodePopReg( tiExpr.m_regLoaded ) ;
			return	true ;
		}
		else
		{
			if ( ESLTypeCast<RSClassClass>( tiRight.m_pClass ) == NULL )
			{
				OutputError( L"instanceof の右辺が型指定ではありません" ) ;
			}
			return	false ;
		}
	case	RSCodeOperator::opSequencing:
		MoveTemporaryTypeInfo( tiExpr, tiRight ) ;
		FreeTemporaryRegister( tiRight ) ;
		return	true ;
	case	RSCodeOperator::opBitNot:
	case	RSCodeOperator::opIncrement:
	case	RSCodeOperator::opDecrement:
	case	RSCodeOperator::opLogicalNot:
	case	RSCodeOperator::opNew:
	case	RSCodeOperator::opConditional:
	case	RSCodeOperator::opSeparator:
	case	RSCodeOperator::opEndOfStatement:
	default:
		OutputError
			( SString(RSCodeOperator::m_pwszOperators[opIndex])
								+ L" は定義されない二項演算子です" ) ;
		return	false ;
	}
	bool	fMoveOperator = RSCodeOperator::IsMoveOperator( opIndex ) ;
	if ( !fMoveOperator )
	{
		if ( !RealizeObjectToNumber( tiExpr )
			|| !RealizeObjectToNumber( tiRight ) )
		{
			return	false ;
		}
	}
	else if ( tiExpr.IsObject()
			&& tiExpr.m_flagReference
			&& (tiExpr.m_pLocalVar != NULL) )
	{
		//
		// ローカルのオブジェクト変数への代入演算子
		//
		if ( !LoadReferenceTemporary( tiRight )
			|| !ConvertToObject( tiRight ) )
		{
			return	false ;
		}
		FlushAllLocalVariableCaches() ;
		//
		const LocalVariable *	plvLocal = tiExpr.m_pLocalVar ;
		if ( opIndex != RSCodeOperator::opMove )
		{
			if ( !LoadReferenceTemporary( tiExpr )
				|| !ConvertToObject( tiExpr ) )
			{
				return	false ;
			}
			FlushAllLocalVariableCaches() ;
			//
			int	regArg0 = GetFreeTemporaryRegister( 2 ) ;
			int	regArg1 = regArg0 + 1 ;
			ExprRegContext	xrc ;
			PushAllTemporaryRegisters( xrc ) ;
			//
			RSClass *	pLExpClass = tiExpr.m_pClass ;
			RSClass *	pLRxpClass = tiRight.m_pClass ;
			RSClass *	pEvalClass = nullptr ;
			//
			m_xmm->WriteCodeMoveRegReg( regArg0, tiExpr.m_regLoaded ) ;
			m_xmm->WriteCodeMoveRegReg( regArg1, tiRight.m_regLoaded ) ;
			m_xmm->WriteCodePushRegsImm8( regArg0, 2 ) ;
			//
			switch ( opIndex )
			{
			case	RSCodeOperator::opMoveAdd:
				m_xmm->WriteCodeSyscall( L"__nrs_operator_add" ) ;
				pEvalClass = pLExpClass ;
				break ;
			case	RSCodeOperator::opMoveSub:
				m_xmm->WriteCodeSyscall( L"__nrs_operator_sub" ) ;
				break ;
			case	RSCodeOperator::opMoveMul:
				m_xmm->WriteCodeSyscall( L"__nrs_operator_mul" ) ;
				break ;
			case	RSCodeOperator::opMoveDiv:
				m_xmm->WriteCodeSyscall( L"__nrs_operator_div" ) ;
				break ;
			case	RSCodeOperator::opMoveMod:
				m_xmm->WriteCodeSyscall( L"__nrs_operator_mod" ) ;
				break ;
			case	RSCodeOperator::opMoveBitAnd:
				m_xmm->WriteCodeSyscall( L"__nrs_operator_bit_and" ) ;
				break ;
			case	RSCodeOperator::opMoveBitOr:
				m_xmm->WriteCodeSyscall( L"__nrs_operator_bit_or" ) ;
				break ;
			case	RSCodeOperator::opMoveBitXor:
				m_xmm->WriteCodeSyscall( L"__nrs_operator_bit_xor" ) ;
				break ;
			case	RSCodeOperator::opMoveShiftRight:
				m_xmm->WriteCodeSyscall( L"__nrs_operator_shift_right" ) ;
				break ;
			case	RSCodeOperator::opMoveShiftLeft:
				m_xmm->WriteCodeSyscall( L"__nrs_operator_shift_left" ) ;
				break ;
			case	RSCodeOperator::opMoveShiftRightArithmetic:
				m_xmm->WriteCodeSyscall( L"__nrs_operator_shift_left_arithmetic" ) ;
				break ;
			default:
				break ;
			}
			m_xmm->WriteCodeAddSP( 16 ) ;
			//
			PopAllTemporaryRegisters( xrc ) ;
			//
			m_xmm->WriteCodePushReg( regAcc ) ;
			//
			FreeTemporaryRegister( tiRight ) ;
			FreeTemporaryRegister( tiExpr ) ;
			//
			tiRight.SetLoadedObject
				( *m_context, pEvalClass, AllocateTemporaryRegister() ) ;
			m_xmm->WriteCodePopReg( tiRight.m_regLoaded ) ;
		}
		//
		FreeTemporaryRegister( tiExpr ) ;
		ReleaseRefLocalVariable( plvLocal ) ;
		//
		WriteLocalVariable( plvLocal, tiRight.m_regLoaded ) ;
		m_xmm->WriteCodeStore
			( tiRight.m_regLoaded,
				dataInt64, regBP, (int) plvLocal->bpObjOffset ) ;
		//
		m_xmm->WriteCodePushReg( tiRight.m_regLoaded ) ;
		m_xmm->WriteCodeSyscall( L"__nrs_add_ref" ) ;
		m_xmm->WriteCodeAddSP( 8 ) ;
		//
		ReferenceToLocalVariable( tiExpr, plvLocal ) ;
		return	true ;
	}
	if ( tiExpr.IsObject() || (!fMoveOperator && tiRight.IsObject()) )
	{
		//
		// 実行時オブジェクト演算
		//
		if ( !LoadReferenceTemporary( tiExpr )
			|| !ConvertToObject( tiExpr ) )
		{
			return	false ;
		}
		if ( !LoadReferenceTemporary( tiRight )
			|| !ConvertToObject( tiRight ) )
		{
			return	false ;
		}
		FlushAllLocalVariableCaches() ;
		//
		int	regArg0 = GetFreeTemporaryRegister( 2 ) ;
		int	regArg1 = regArg0 + 1 ;
		ExprRegContext	xrc ;
		PushAllTemporaryRegisters( xrc ) ;
		//
		RSClass *	pLExpClass = tiExpr.m_pClass ;
		RSClass *	pLRxpClass = tiRight.m_pClass ;
		RSClass *	pEvalClass = nullptr ;
		//
		m_xmm->WriteCodeMoveRegReg( regArg0, tiExpr.m_regLoaded ) ;
		m_xmm->WriteCodeMoveRegReg( regArg1, tiRight.m_regLoaded ) ;
		m_xmm->WriteCodePushRegsImm8( regArg0, 2 ) ;
		//
		switch ( opIndex )
		{
		case	RSCodeOperator::opAdd:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_add" ) ;
			pEvalClass = pLExpClass ;
			break ;
		case	RSCodeOperator::opSub:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_sub" ) ;
			break ;
		case	RSCodeOperator::opMul:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_mul" ) ;
			break ;
		case	RSCodeOperator::opDiv:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_div" ) ;
			break ;
		case	RSCodeOperator::opMod:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_mod" ) ;
			break ;
		case	RSCodeOperator::opBitAnd:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_bit_and" ) ;
			break ;
		case	RSCodeOperator::opBitOr:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_bit_or" ) ;
			break ;
		case	RSCodeOperator::opBitXor:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_bit_xor" ) ;
			break ;
		case	RSCodeOperator::opShiftRight:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_shift_right" ) ;
			break ;
		case	RSCodeOperator::opShiftLeft:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_shift_left" ) ;
			break ;
		case	RSCodeOperator::opShiftRightArithmetic:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_shift_right_arithmetic" ) ;
			break ;
		case	RSCodeOperator::opEqual:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_equal" ) ;
			break ;
		case	RSCodeOperator::opNotEqual:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_not_equal" ) ;
			break ;
		case	RSCodeOperator::opLessEqual:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_less_equal" ) ;
			break ;
		case	RSCodeOperator::opLessThan:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_less_than" ) ;
			break ;
		case	RSCodeOperator::opGraterEqual:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_grater_equal" ) ;
			break ;
		case	RSCodeOperator::opGraterThan:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_grater_than" ) ;
			break ;
		case	RSCodeOperator::opPointerEqual:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_pointer_equal" ) ;
			break ;
		case	RSCodeOperator::opPointerNotEqual:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_pointer_not_equal" ) ;
			break ;
		case	RSCodeOperator::opLogicalAnd:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_logical_and" ) ;
			break ;
		case	RSCodeOperator::opLogicalOr:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_logical_or" ) ;
			break ;
		case	RSCodeOperator::opMove:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_move" ) ;
			break ;
		case	RSCodeOperator::opMoveAdd:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_move_add" ) ;
			break ;
		case	RSCodeOperator::opMoveSub:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_move_sub" ) ;
			break ;
		case	RSCodeOperator::opMoveMul:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_move_mul" ) ;
			break ;
		case	RSCodeOperator::opMoveDiv:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_move_div" ) ;
			break ;
		case	RSCodeOperator::opMoveMod:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_move_mod" ) ;
			break ;
		case	RSCodeOperator::opMoveBitAnd:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_move_bit_and" ) ;
			break ;
		case	RSCodeOperator::opMoveBitOr:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_move_bit_or" ) ;
			break ;
		case	RSCodeOperator::opMoveBitXor:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_move_bit_xor" ) ;
			break ;
		case	RSCodeOperator::opMoveShiftRight:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_move_shift_right" ) ;
			break ;
		case	RSCodeOperator::opMoveShiftLeft:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_move_shift_left" ) ;
			break ;
		case	RSCodeOperator::opMoveShiftRightArithmetic:
			m_xmm->WriteCodeSyscall( L"__nrs_operator_move_shift_left_arithmetic" ) ;
			break ;
		default:
			break ;
		}
		m_xmm->WriteCodeAddSP( 16 ) ;
		//
		PopAllTemporaryRegisters( xrc ) ;
		//
		m_xmm->WriteCodePushReg( regAcc ) ;
		//
		FreeTemporaryRegister( tiRight ) ;
		FreeTemporaryRegister( tiExpr ) ;
		//
		tiExpr.SetLoadedObject
			( *m_context, pEvalClass, AllocateTemporaryRegister() ) ;
		m_xmm->WriteCodePopReg( tiExpr.m_regLoaded ) ;
		//
		CompileCheckException() ;
		return	true ;
	}
	else if ( tiExpr.m_flagPointer )
	{
		//
		// ポインタ演算
		//
		RSTypedArrayPointerClass *
			pPtrClass = ESLTypeCast<RSTypedArrayPointerClass>( tiExpr.m_pClass ) ;
		ESLAssert( pPtrClass != NULL ) ;
		//
		if ( opIndex == RSCodeOperator::opMove )
		{
			bool	fResult = OperatorMove( tiExpr, tiRight ) ;
			FreeTemporaryRegister( tiRight ) ;
			return	fResult ;
		}
		if ( RSCodeOperator::IsComparator( opIndex ) )
		{
			if ( !LoadReferenceTemporary( tiExpr )
				|| !LoadReferenceTemporary( tiRight ) )
			{
				return	false ;
			}
		}
		else if ( !tiRight.IsInteger() )
		{
			if ( !LoadReferenceTemporary( tiRight )
				|| !RealizeObjectToNumber( tiRight ) )
			{
				return	false ;
			}
			if ( !tiRight.IsInteger() )
			{
				OutputError( L"不正なポインタへの演算です" ) ;
				return	false ;
			}
		}
		if ( RSCodeOperator::IsMoveOperator( opIndex ) )
		{
			if ( !tiExpr.m_flagReference
				|| !tiExpr.m_flagLocal || (tiExpr.m_pLocalVar == NULL) )
			{
				OutputError( L"左辺式ではないポインタへの代入です" ) ;
				return	false ;
			}
		}
		RSObject *	pImmObj ;
		int			regTemp ;
		switch ( opIndex )
		{
		case	RSCodeOperator::opAdd:
		case	RSCodeOperator::opSub:
			if ( !LoadReferenceTemporary( tiExpr ) )
			{
				return	false ;
			}
			ESLAssert( tiExpr.m_flagLoaded ) ;
			//
			pImmObj = tiRight.GetImmediate() ;
			if ( pImmObj != NULL )
			{
				int64_t	num ;
				if ( pImmObj->AsInteger( num ) )
				{
					tiExpr.m_addrOffset +=
						(int) num * (int) pPtrClass->GetElementBytes() ;
				}
				else
				{
					OutputError( L"整数ではないポインタ加算です" ) ;
					return	false ;
				}
			}
			else
			{
				if ( !LoadReferenceTemporary( tiRight, true ) )
				{
					return	false ;
				}
				ESLAssert( tiRight.m_flagLoaded ) ;
				//
				size_t	nBytes = pPtrClass->GetElementBytes() ;
				if ( nBytes != 1 )
				{
					switch ( nBytes )
					{
					case	2:
						m_xmm->WriteCode3OP
							( codeSllImm8,
								tiRight.m_regLoaded,
								tiRight.m_regLoaded, 1 ) ;
						break ;
					case	4:
						m_xmm->WriteCode3OP
							( codeSllImm8,
								tiRight.m_regLoaded,
								tiRight.m_regLoaded, 2 ) ;
						break ;
					case	8:
						m_xmm->WriteCode3OP
							( codeSllImm8,
								tiRight.m_regLoaded,
								tiRight.m_regLoaded, 3 ) ;
						break ;
					case	16:
						m_xmm->WriteCode3OP
							( codeSllImm8,
								tiRight.m_regLoaded,
								tiRight.m_regLoaded, 4 ) ;
						break ;
					default:
						m_xmm->WriteCodeMulRegRegImm32
							( tiRight.m_regLoaded,
								tiRight.m_regLoaded, (int) nBytes ) ;
						break ;
					}
				}
				m_xmm->WriteCode2OP
					( codeAddReg,
						tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
			}
			break ;

		case	RSCodeOperator::opEqual:
		case	RSCodeOperator::opNotEqual:
			ESLAssert( tiExpr.m_flagLoaded ) ;
			ESLAssert( tiRight.m_flagLoaded ) ;
			m_xmm->WriteCodePushReg( tiRight.m_flagLoaded ) ;
			m_xmm->WriteCodePushReg( tiExpr.m_flagLoaded ) ;
			if ( opIndex == RSCodeOperator::opEqual )
			{
				m_xmm->WriteCodeSyscall( L"__nrs_ptr_operator_equal" ) ;
			}
			else
			{
				m_xmm->WriteCodeSyscall( L"__nrs_ptr_operator_not_equal" ) ;
			}
			m_xmm->WriteCodeAddSP( 16 ) ;
			m_xmm->WriteCodePushReg( regAcc ) ;
			//
			FreeTemporaryRegister( tiExpr ) ;
			FreeTemporaryRegister( tiRight ) ;
			//
			tiExpr.SetLoadedNumber
				( *m_context,
					m_context->GetBasicTypeClass( RSCodeControl::wiBoolean ),
					AllocateTemporaryRegister() ) ;
			m_xmm->WriteCodePopReg( tiExpr.m_regLoaded ) ;
			break ;

		case	RSCodeOperator::opMoveAdd:
		case	RSCodeOperator::opMoveSub:
			ESLAssert( tiExpr.m_flagReference ) ;
			ESLAssert( tiExpr.m_pLocalVar != NULL ) ;
			regTemp = AllocateTemporaryRegister() ;
			LoadLocalVariable( tiExpr.m_pLocalVar, regTemp ) ;
			//
			pImmObj = tiRight.GetImmediate() ;
			if ( pImmObj != NULL )
			{
				int64_t	num ;
				if ( pImmObj->AsInteger( num ) )
				{
					m_xmm->WriteCodeAddRegRegImm32
						( regTemp, regTemp,
							(int) num * (int) pPtrClass->GetElementBytes() ) ;
				}
				else
				{
					OutputError( L"整数ではないポインタ加算です" ) ;
					return	false ;
				}
			}
			else
			{
				ESLAssert( tiRight.m_flagLoaded ) ;
				//
				size_t	nBytes = pPtrClass->GetElementBytes() ;
				if ( nBytes != 1 )
				{
					switch ( nBytes )
					{
					case	2:
						m_xmm->WriteCode3OP
							( codeSllImm8,
								tiRight.m_regLoaded,
								tiRight.m_regLoaded, 1 ) ;
						break ;
					case	4:
						m_xmm->WriteCode3OP
							( codeSllImm8,
								tiRight.m_regLoaded,
								tiRight.m_regLoaded, 2 ) ;
						break ;
					case	8:
						m_xmm->WriteCode3OP
							( codeSllImm8,
								tiRight.m_regLoaded,
								tiRight.m_regLoaded, 3 ) ;
						break ;
					case	16:
						m_xmm->WriteCode3OP
							( codeSllImm8,
								tiRight.m_regLoaded,
								tiRight.m_regLoaded, 4 ) ;
						break ;
					default:
						m_xmm->WriteCodeMulRegRegImm32
							( tiRight.m_regLoaded,
								tiRight.m_regLoaded, (int) nBytes ) ;
						break ;
					}
				}
				m_xmm->WriteCode2OP
					( codeAddReg, regTemp, tiRight.m_regLoaded ) ;
			}
			//
			WriteLocalVariable( tiExpr.m_pLocalVar, regTemp ) ;
			FreeTemporaryRegister( regTemp ) ;
			break ;

		default:
			OutputError( L"不正なポインタへの演算です" ) ;
			return	false ;
		}
		FreeTemporaryRegister( tiRight ) ;
		return	true ;
	}
	else
	{
		//
		// 数値演算
		//
		ESLAssert( tiExpr.IsInteger() || tiExpr.IsFloatingPoint() ) ;
		if ( !LoadReferenceTemporary( tiRight )
			|| !RealizeObjectToNumber( tiRight ) )
		{
			return	false ;
		}
		if ( !tiRight.IsInteger() && !tiRight.IsFloatingPoint() )
		{
			OutputError( L"数値演算の不正な右辺式です" ) ;
			return	false ;
		}
		if ( RSCodeOperator::IsMoveOperator( opIndex ) )
		{
			if ( !tiExpr.m_flagReference )
			{
				OutputError( L"左辺式ではない数値の代入です" ) ;
				return	false ;
			}
			// 演算型の整合
			bool	fIntOp = false ;
			switch ( opIndex )
			{
			case	RSCodeOperator::opMoveMod:
			case	RSCodeOperator::opMoveBitAnd:
			case	RSCodeOperator::opMoveBitOr:
			case	RSCodeOperator::opMoveBitXor:
			case	RSCodeOperator::opMoveShiftRight:
			case	RSCodeOperator::opMoveShiftLeft:
			case	RSCodeOperator::opMoveShiftRightArithmetic:
				fIntOp = true ;
				break ;
			default:
				break ;
			}
			ESLAssert( tiRight.m_flagLoaded ) ;
			if ( (tiExpr.IsInteger() || fIntOp) && tiRight.IsFloatingPoint() )
			{
				m_xmm->WriteCode2OP
					( codeCvtFloat2Int,
						tiRight.m_regLoaded, tiRight.m_regLoaded ) ;
				tiRight.m_typeNum = typeInt64 ;
			}
			else if ( (tiExpr.IsFloatingPoint() && !fIntOp) && tiRight.IsInteger() )
			{
				m_xmm->WriteCode2OP
					( codeCvtInt2Float,
						tiRight.m_regLoaded, tiRight.m_regLoaded ) ;
				tiRight.m_typeNum = typeFloat64 ;
			}
			if ( opIndex == RSCodeOperator::opMove )
			{
				// 代入
				if ( tiExpr.m_flagLocal )
				{
					ESLAssert( tiExpr.m_pLocalVar != NULL ) ;
					WriteLocalVariable
						( tiExpr.m_pLocalVar, tiRight.m_regLoaded ) ;
				}
				else
				{
					m_xmm->WriteCodeStore
						( tiRight.m_regLoaded,
							NumberTypeToDataType( tiExpr.m_typeNum ),
							tiExpr.m_regLoaded, tiExpr.m_addrOffset ) ;
				}
				FreeTemporaryRegister( tiRight ) ;
				return	true ;
			}
			// 代入前の値をロード
			int	regTemp = AllocateTemporaryRegister() ;
			if ( tiExpr.m_flagLocal )
			{
				ESLAssert( tiExpr.m_pLocalVar != NULL ) ;
				LoadLocalVariable( tiExpr.m_pLocalVar, regTemp ) ;
			}
			else
			{
				m_xmm->WriteCodeLoad
					( regTemp,
						NumberTypeToDataType( tiExpr.m_typeNum ),
						tiExpr.m_regLoaded, tiExpr.m_addrOffset ) ;
			}
			if ( fIntOp && tiExpr.IsFloatingPoint() )
			{
				m_xmm->WriteCode2OP( codeCvtFloat2Int, regTemp, regTemp ) ;
			}
			// 演算
			switch ( opIndex )
			{
			case	RSCodeOperator::opMoveAdd:
				if ( tiExpr.IsInteger() )
				{
					m_xmm->WriteCode2OP
						( codeAddReg, regTemp, tiRight.m_regLoaded ) ;
				}
				else
				{
					m_xmm->WriteCode2OP
						( codeFAddReg, regTemp, tiRight.m_regLoaded ) ;
				}
				break ;
			case	RSCodeOperator::opMoveSub:
				if ( tiExpr.IsInteger() )
				{
					m_xmm->WriteCode2OP
						( codeSubReg, regTemp, tiRight.m_regLoaded ) ;
				}
				else
				{
					m_xmm->WriteCode2OP
						( codeFSubReg, regTemp, tiRight.m_regLoaded ) ;
				}
				break ;
			case	RSCodeOperator::opMoveMul:
				if ( tiExpr.IsInteger() )
				{
					m_xmm->WriteCode2OP
						( codeMulReg, regTemp, tiRight.m_regLoaded ) ;
				}
				else
				{
					m_xmm->WriteCode2OP
						( codeFMulReg, regTemp, tiRight.m_regLoaded ) ;
				}
				break ;
			case	RSCodeOperator::opMoveDiv:
				if ( tiExpr.IsInteger() )
				{
					m_xmm->WriteCode2OP
						( codeDivReg, regTemp, tiRight.m_regLoaded ) ;
				}
				else
				{
					m_xmm->WriteCode2OP
						( codeFDivReg, regTemp, tiRight.m_regLoaded ) ;
				}
				break ;
			case	RSCodeOperator::opMoveMod:
				ESLAssert( fIntOp ) ;
				m_xmm->WriteCode2OP
					( codeModReg, regTemp, tiRight.m_regLoaded ) ;
				break ;
			case	RSCodeOperator::opMoveBitAnd:
				ESLAssert( fIntOp ) ;
				m_xmm->WriteCode2OP
					( codeAndReg, regTemp, tiRight.m_regLoaded ) ;
				break ;
			case	RSCodeOperator::opMoveBitOr:
				ESLAssert( fIntOp ) ;
				m_xmm->WriteCode2OP
					( codeOrReg, regTemp, tiRight.m_regLoaded ) ;
				break ;
			case	RSCodeOperator::opMoveBitXor:
				ESLAssert( fIntOp ) ;
				m_xmm->WriteCode2OP
					( codeXorReg, regTemp, tiRight.m_regLoaded ) ;
				break ;
			case	RSCodeOperator::opMoveShiftRight:
				ESLAssert( fIntOp ) ;
				m_xmm->WriteCode2OP
					( codeSrlReg, regTemp, tiRight.m_regLoaded ) ;
				break ;
			case	RSCodeOperator::opMoveShiftLeft:
				ESLAssert( fIntOp ) ;
				m_xmm->WriteCode2OP
					( codeSllReg, regTemp, tiRight.m_regLoaded ) ;
				break ;
			case	RSCodeOperator::opMoveShiftRightArithmetic:
				ESLAssert( fIntOp ) ;
				m_xmm->WriteCode2OP
					( codeSraReg, regTemp, tiRight.m_regLoaded ) ;
				break ;
			default:
				OutputError( L"不正な代入演算子です" ) ;
				break ;
			}
			// 結果のストア
			if ( fIntOp && tiExpr.IsFloatingPoint() )
			{
				m_xmm->WriteCode2OP( codeCvtInt2Float, regTemp, regTemp ) ;
			}
			if ( tiExpr.m_flagLocal )
			{
				ESLAssert( tiExpr.m_pLocalVar != NULL ) ;
				WriteLocalVariable
					( tiExpr.m_pLocalVar, regTemp ) ;
			}
			else
			{
				m_xmm->WriteCodeStore
					( regTemp,
						NumberTypeToDataType( tiExpr.m_typeNum ),
						tiExpr.m_regLoaded, tiExpr.m_addrOffset ) ;
			}
			FreeTemporaryRegister( tiRight ) ;
			FreeTemporaryRegister( regTemp ) ;
			return	!m_context->IsException() ;
		}
		else if ( RSCodeOperator::IsComparator( opIndex ) )
		{
			if ( !LoadReferenceTemporary( tiExpr )
				|| !LoadReferenceTemporary( tiRight ) )
			{
				return	false ;
			}
			if ( tiExpr.IsInteger() && tiRight.IsInteger() )
			{
				switch ( opIndex )
				{
				case	RSCodeOperator::opEqual:
				case	RSCodeOperator::opPointerEqual:
					m_xmm->WriteCode2OP
						( codeCmpEqReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
					break ;
				case	RSCodeOperator::opNotEqual:
				case	RSCodeOperator::opPointerNotEqual:
					m_xmm->WriteCode2OP
						( codeCmpNeReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
					break ;
				case	RSCodeOperator::opLessEqual:
					m_xmm->WriteCode2OP
						( codeCmpLeReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
					break ;
				case	RSCodeOperator::opLessThan:
					m_xmm->WriteCode2OP
						( codeCmpLtReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
					break ;
				case	RSCodeOperator::opGraterEqual:
					m_xmm->WriteCode2OP
						( codeCmpGeReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
					break ;
				case	RSCodeOperator::opGraterThan:
					m_xmm->WriteCode2OP
						( codeCmpGtReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
					break ;
				default:
					break ;
				}
			}
			else
			{
				if ( tiExpr.IsInteger() )
				{
					m_xmm->WriteCode2OP
						( codeCvtInt2Float,
							tiExpr.m_regLoaded, tiExpr.m_regLoaded ) ;
				}
				if ( tiRight.IsInteger() )
				{
					m_xmm->WriteCode2OP
						( codeCvtInt2Float,
							tiRight.m_regLoaded, tiRight.m_regLoaded ) ;
				}
				switch ( opIndex )
				{
				case	RSCodeOperator::opEqual:
				case	RSCodeOperator::opPointerEqual:
					m_xmm->WriteCode2OP
						( codeFCmpEqReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
					break ;
				case	RSCodeOperator::opNotEqual:
				case	RSCodeOperator::opPointerNotEqual:
					m_xmm->WriteCode2OP
						( codeFCmpNeReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
					break ;
				case	RSCodeOperator::opLessEqual:
					m_xmm->WriteCode2OP
						( codeFCmpLeReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
					break ;
				case	RSCodeOperator::opLessThan:
					m_xmm->WriteCode2OP
						( codeFCmpLtReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
					break ;
				case	RSCodeOperator::opGraterEqual:
					m_xmm->WriteCode2OP
						( codeFCmpGeReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
					break ;
				case	RSCodeOperator::opGraterThan:
					m_xmm->WriteCode2OP
						( codeFCmpGtReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
					break ;
				default:
					break;
				}
			}
			FreeTemporaryRegister( tiRight ) ;
			tiExpr.ChangeType
				( *m_context,
					m_context->GetBasicTypeClass( RSCodeControl::wiBoolean ) ) ;
			return	true ;
		}
		// 演算型の整合
		bool	fIntOp = false ;
		switch ( opIndex )
		{
		case	RSCodeOperator::opMod:
		case	RSCodeOperator::opBitAnd:
		case	RSCodeOperator::opBitOr:
		case	RSCodeOperator::opBitXor:
		case	RSCodeOperator::opShiftRight:
		case	RSCodeOperator::opShiftLeft:
		case	RSCodeOperator::opShiftRightArithmetic:
			fIntOp = true ;
			break ;
		default:
			break ;
		}
		if ( !LoadReferenceTemporary( tiExpr )
			|| !LoadReferenceTemporary( tiRight ) )
		{
			return	false ;
		}
		if ( fIntOp || (tiExpr.IsInteger() && tiRight.IsInteger()) )
		{
			if ( tiExpr.IsFloatingPoint() )
			{
				m_xmm->WriteCode2OP
					( codeCvtFloat2Int,
						tiExpr.m_regLoaded, tiExpr.m_regLoaded ) ;
				tiExpr.ChangeType
					( *m_context,
						m_context->GetBasicTypeClass( RSCodeControl::wiLong ) ) ;
			}
			if ( tiRight.IsFloatingPoint() )
			{
				m_xmm->WriteCode2OP
					( codeCvtFloat2Int,
						tiRight.m_regLoaded, tiRight.m_regLoaded ) ;
				tiRight.ChangeType
					( *m_context,
						m_context->GetBasicTypeClass( RSCodeControl::wiLong ) ) ;
			}
			switch ( opIndex )
			{
			case	RSCodeOperator::opAdd:
				m_xmm->WriteCode2OP
					( codeAddReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
				break ;
			case	RSCodeOperator::opSub:
				m_xmm->WriteCode2OP
					( codeSubReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
				break ;
			case	RSCodeOperator::opMul:
				m_xmm->WriteCode2OP
					( codeMulReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
				break ;
			case	RSCodeOperator::opDiv:
				m_xmm->WriteCode2OP
					( codeDivReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
				break ;
			case	RSCodeOperator::opMod:
				m_xmm->WriteCode2OP
					( codeModReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
				break ;
			case	RSCodeOperator::opBitAnd:
				m_xmm->WriteCode2OP
					( codeAndReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
				break ;
			case	RSCodeOperator::opBitOr:
				m_xmm->WriteCode2OP
					( codeOrReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
				break ;
			case	RSCodeOperator::opBitXor:
				m_xmm->WriteCode2OP
					( codeXorReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
				break ;
			case	RSCodeOperator::opShiftRight:
				m_xmm->WriteCode2OP
					( codeSrlReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
				break ;
			case	RSCodeOperator::opShiftLeft:
				m_xmm->WriteCode2OP
					( codeSllReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
				break ;
			case	RSCodeOperator::opShiftRightArithmetic:
				m_xmm->WriteCode2OP
					( codeSraReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
				break ;
			default:
				break ;
			}
			if ( tiExpr.m_typeNum < tiRight.m_typeNum )
			{
				switch ( tiRight.m_typeNum )
				{
				case	typeUint8:
				case	typeInt16:
					tiExpr.ChangeType
						( *m_context,
							m_context->GetBasicTypeClass( RSCodeControl::wiShort ) ) ;
					break ;
				case	typeUint16:
				case	typeInt32:
					tiExpr.ChangeType
						( *m_context,
							m_context->GetBasicTypeClass( RSCodeControl::wiInt ) ) ;
					break ;
				case	typeUint32:
				case	typeInt64:
				default:
					tiExpr.ChangeType
						( *m_context,
							m_context->GetBasicTypeClass( RSCodeControl::wiLong ) ) ;
					break ;
				}
			}
		}
		else
		{
			if ( tiExpr.IsInteger() )
			{
				m_xmm->WriteCode2OP
					( codeCvtInt2Float,
						tiExpr.m_regLoaded, tiExpr.m_regLoaded ) ;
			}
			if ( tiRight.IsInteger() )
			{
				m_xmm->WriteCode2OP
					( codeCvtInt2Float,
						tiRight.m_regLoaded, tiRight.m_regLoaded ) ;
			}
			switch ( opIndex )
			{
			case	RSCodeOperator::opAdd:
				m_xmm->WriteCode2OP
					( codeFAddReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
				break ;
			case	RSCodeOperator::opSub:
				m_xmm->WriteCode2OP
					( codeFSubReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
				break ;
			case	RSCodeOperator::opMul:
				m_xmm->WriteCode2OP
					( codeFMulReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
				break ;
			case	RSCodeOperator::opDiv:
				m_xmm->WriteCode2OP
					( codeFDivReg, tiExpr.m_regLoaded, tiRight.m_regLoaded ) ;
				break ;
			default:
				break ;
			}
			tiExpr.ChangeType
				( *m_context,
					m_context->GetBasicTypeClass( RSCodeControl::wiDouble ) ) ;
		}
		FreeTemporaryRegister( tiRight ) ;
		return	true ;
	}
}

// 間接要素参照
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::CompileReferenceElement
	( RSCompiler::TypeInfo& tiExpr, RSCompiler::TypeInfo& tiIndex )
{
	RSTypedArrayPointerClass *
		pPtrClass = ESLTypeCast<RSTypedArrayPointerClass>( tiExpr.m_pClass ) ;
	if ( pPtrClass != NULL )
	{
		//
		// ポインタ参照
		//
		if ( !LoadReferenceTemporary( tiExpr )
			|| !RealizeObjectToNumber( tiExpr ) )
		{
			return	false ;
		}
		size_t	nElementBytes = pPtrClass->GetElementBytes() ;
		//
		RSObject *	pObjImm = tiIndex.GetImmediate() ;
		if ( pObjImm != NULL )
		{
			//
			// 即値添え字
			//
			int64_t	num ;
			if ( !pObjImm->AsInteger( num ) )
			{
				OutputError( L"ポインタの不正な添え字です" ) ;
				return	false ;
			}
			//
			RSStructuredPointerClass *	pStructClass =
				ESLTypeCast<RSStructuredPointerClass>( pPtrClass ) ;
			if ( pStructClass != NULL )
			{
				if ( !(m_flagsBehaivor & behaviorFlatPointer) )
				{
					if ( !LoadReferenceTemporary( tiExpr, true ) )
					{
						return	false ;
					}
					ESLAssert( tiExpr.m_flagLoaded ) ;
					//
					int	regTemp0 = GetFreeTemporaryRegister( 2 ) ;
					int	regTemp1 = regTemp0 + 1 ;
					m_xmm->WriteCodeAddRegRegImm32
						( regTemp0, tiExpr.m_regLoaded,
							(int) tiExpr.m_addrOffset
								+ (int) num * (int) nElementBytes ) ;
					m_xmm->WriteCodeMoveRegInt64
						( regTemp1, nElementBytes ) ;
					m_xmm->WriteCodePushRegsImm8( regTemp0, 2 ) ;
					m_xmm->WriteCodeSyscall( L"__nrs_bound_pointer" ) ;
					m_xmm->WriteCodeAddSP( 16 ) ;
					//
					if ( tiExpr.m_regObject >= 0 )
					{
						m_xmm->WriteCodePushReg( regAcc ) ;
						FreeTemporaryRegisterAndOwnerObject( tiExpr ) ;
						//
						tiExpr.m_regObject = AllocateTemporaryRegister() ;
						tiExpr.m_flagRefRosetta = false ;
						//
						m_xmm->WriteCodePopReg( tiExpr.m_regObject ) ;
						m_xmm->WriteCodeMoveRegReg
							( tiExpr.m_regLoaded, tiExpr.m_regObject ) ;
					}
					else
					{
						tiExpr.m_regObject = AllocateTemporaryRegister() ;
						tiExpr.m_flagRefRosetta = false ;
						//
						m_xmm->WriteCodeMoveRegReg
							( tiExpr.m_regObject, regAcc ) ;
						m_xmm->WriteCodeMoveRegReg
							( tiExpr.m_regLoaded, regAcc ) ;
					}
					tiExpr.m_addrOffset = 0 ;
					//
					return	true ;
				}
				else
				{
					tiExpr.m_addrOffset += (int) num * (int) nElementBytes ;
					return	true ;
				}
			}
			else
			{
				ESLAssert( tiExpr.m_typeNum != typeObject ) ;
				tiExpr.m_flagPointer = false ;
				tiExpr.m_flagReference = true ;
				tiExpr.m_addrOffset += (int) num * (int) nElementBytes ;
				return	true ;
			}
		}
		//
		// 添え字整数判定
		//
		if ( !LoadReferenceTemporary( tiExpr, true )
			|| !LoadReferenceTemporary( tiIndex, true )
			|| !RealizeObjectToNumber( tiIndex ) )
		{
			return	false ;
		}
		if ( tiIndex.IsInteger() )
		{
			ESLAssert( !tiExpr.m_flagLockAddr ) ;
			ESLAssert( tiIndex.m_flagLoaded ) ;
			ESLAssert( !tiIndex.m_flagLockAddr ) ;
			if ( nElementBytes > 1 )
			{
				switch ( nElementBytes )
				{
				case	2:
					m_xmm->WriteCode3OP
						( codeSllImm8,
							tiIndex.m_regLoaded,
							tiIndex.m_regLoaded, 1 ) ;
					break ;
				case	4:
					m_xmm->WriteCode3OP
						( codeSllImm8,
							tiIndex.m_regLoaded,
							tiIndex.m_regLoaded, 2 ) ;
					break ;
				case	8:
					m_xmm->WriteCode3OP
						( codeSllImm8,
							tiIndex.m_regLoaded,
							tiIndex.m_regLoaded, 3 ) ;
					break ;
				case	16:
					m_xmm->WriteCode3OP
						( codeSllImm8,
							tiIndex.m_regLoaded,
							tiIndex.m_regLoaded, 4 ) ;
					break ;
				default:
					m_xmm->WriteCodeMulRegRegImm32
						( tiIndex.m_regLoaded,
							tiIndex.m_regLoaded, (int) nElementBytes ) ;
					break ;
				}
			}
			RSStructuredPointerClass *	pStructClass =
				ESLTypeCast<RSStructuredPointerClass>( pPtrClass ) ;
			if ( pStructClass != NULL )
			{
				if ( !(m_flagsBehaivor & behaviorFlatPointer) )
				{
					if ( !LoadReferenceTemporary( tiExpr, true ) )
					{
						return	false ;
					}
					ESLAssert( tiExpr.m_flagLoaded ) ;
					//
					int	regTemp0 = GetFreeTemporaryRegister( 2 ) ;
					int	regTemp1 = regTemp0 + 1 ;
					m_xmm->WriteCodeAddRegRegImm32
						( regTemp0, tiExpr.m_regLoaded,
										(int) tiExpr.m_addrOffset ) ;
					m_xmm->WriteCode2OP
						( codeAddReg, regTemp0, tiIndex.m_regLoaded ) ;
					m_xmm->WriteCodeMoveRegInt64
						( regTemp1, nElementBytes ) ;
					m_xmm->WriteCodePushRegsImm8( regTemp0, 2 ) ;
					m_xmm->WriteCodeSyscall( L"__nrs_bound_pointer" ) ;
					m_xmm->WriteCodeAddSP( 16 ) ;
					//
					if ( tiExpr.m_regObject >= 0 )
					{
						m_xmm->WriteCodePushReg( regAcc ) ;
						FreeTemporaryRegisterAndOwnerObject( tiExpr ) ;
						//
						tiExpr.m_regObject = AllocateTemporaryRegister() ;
						tiExpr.m_flagRefRosetta = false ;
						//
						m_xmm->WriteCodePopReg( tiExpr.m_regObject ) ;
						m_xmm->WriteCodeMoveRegReg
							( tiExpr.m_regLoaded, tiExpr.m_regObject ) ;
					}
					else
					{
						tiExpr.m_regObject = AllocateTemporaryRegister() ;
						tiExpr.m_flagRefRosetta = false ;
						//
						m_xmm->WriteCodeMoveRegReg
							( tiExpr.m_regObject, regAcc ) ;
						m_xmm->WriteCodeMoveRegReg
							( tiExpr.m_regLoaded, regAcc ) ;
					}
					tiExpr.m_addrOffset = 0 ;
					//
					FreeTemporaryRegister( tiIndex ) ;
					return	true ;
				}
				else
				{
					m_xmm->WriteCode2OP
						( codeAddReg, tiExpr.m_regLoaded, tiIndex.m_regLoaded ) ;
					FreeTemporaryRegister( tiIndex ) ;
					return	true ;
				}
			}
			else
			{
				ESLAssert( tiExpr.m_typeNum != typeObject ) ;
				m_xmm->WriteCode2OP
					( codeAddReg, tiExpr.m_regLoaded, tiIndex.m_regLoaded ) ;
				tiExpr.m_flagPointer = false ;
				tiExpr.m_flagReference = true ;
				FreeTemporaryRegister( tiIndex ) ;
				return	true ;
			}
		}
	}
	//
	// オブジェクトの要素参照
	//
	if ( !LoadReferenceTemporary( tiExpr )
		|| !ConvertToObject( tiExpr )
		|| !LoadReferenceTemporary( tiIndex )
		|| !ConvertToObject( tiIndex ) )
	{
		return	false ;
	}
	RSClass *	pElementClass = NULL ;
	RSGenericArrayClass *
		pArrayClass = ESLTypeCast<RSGenericArrayClass>( tiExpr.m_pClass ) ;
	if ( pArrayClass != NULL )
	{
		pElementClass = pArrayClass->m_pElementClass ;
	}
	else
	{
		RSGenericHashMapClass *
			pHashClass = ESLTypeCast<RSGenericHashMapClass>( tiExpr.m_pClass ) ;
		if ( pHashClass != NULL )
		{
			pElementClass = pHashClass->m_pElementClass ;
		}
	}
	ESLAssert( tiExpr.m_flagLoaded ) ;
	ESLAssert( tiIndex.m_flagLoaded ) ;
	m_xmm->WriteCodePushReg( tiIndex.m_regLoaded ) ;
	m_xmm->WriteCodePushReg( tiExpr.m_regLoaded ) ;
	m_xmm->WriteCodeSyscall( L"__nrs_get_element_as" ) ;
	m_xmm->WriteCodeAddSP( 16 ) ;
	m_xmm->WriteCodePushReg( regAcc ) ;
	//
	FreeTemporaryRegister( tiIndex ) ;
	FreeTemporaryRegister( tiExpr ) ;
	//
	tiExpr.SetLoadedObject
		( *m_context, pElementClass, AllocateTemporaryRegister() ) ;
	m_xmm->WriteCodePopReg( tiExpr.m_regLoaded ) ;
	//
	CompileCheckException() ;
	return	true ;
}

// NumberType -> DataType 変換
//////////////////////////////////////////////////////////////////////////////
ECSSakura2Processor::DataType
		RSCompiler::NumberTypeToDataType( RSCompiler::NumberType type )
{
	switch ( type )
	{
	case	typeInt8:
		return	dataInt8 ;
	case	typeUint8:
		return	dataUint8 ;
	case	typeInt16:
		return	dataInt16 ;
	case	typeUint16:
		return	dataUint16 ;
	case	typeInt32:
		return	dataInt32 ;
	case	typeUint32:
		return	dataUint32 ;
	case	typeFloat32:
		return	dataFloat ;
	case	typeInt64:
	case	typeFloat64:
	default:
		return	dataInt64 ;
	}
	return	dataInt64 ;
}

// TypeInfo -> DataType 変換
//////////////////////////////////////////////////////////////////////////////
ECSSakura2Processor::DataType
		RSCompiler::TypeInfoToDataType( const RSCompiler::TypeInfo& ti )
{
	if ( ti.m_flagPointer )
	{
		return	dataInt64 ;
	}
	return	NumberTypeToDataType( ti.m_typeNum ) ;
}

// LocalVariable -> DataType 変換
//////////////////////////////////////////////////////////////////////////////
ECSSakura2Processor::DataType
		RSCompiler::LocalVariableToDataType
			( const RSCompiler::LocalVariable * plv )
{
	if ( plv->flagPointer )
	{
		return	dataInt64 ;
	}
	return	NumberTypeToDataType( plv->typeNumber ) ;
}

// RSReferenceNumber::NumberType -> NumberType 変換
//////////////////////////////////////////////////////////////////////////////
RSCompiler::NumberType
	RSCompiler::ConvertNumberType( RSReferenceNumber::NumberType type )
{
	static const NumberType	s_typeNumber[] =
	{
		typeUint8,
		typeInt8,
		typeUint16,
		typeInt16,
		typeUint32,
		typeInt32,
		typeInt64,
		typeFloat32,
		typeFloat64,
		typeObject,
	} ;
	return	s_typeNumber[type] ;
}

// ローカル変数への参照を作成
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::ReferenceToLocalVariable
	( RSCompiler::TypeInfo& ti, const RSCompiler::LocalVariable * plv )
{
	ti.m_typeNum = plv->typeNumber ;
	ti.m_accMod = plv->accMod ;
	ti.m_flagPointer = plv->flagPointer ;
	ti.m_flagReference = true ;
	ti.m_flagLocal = true ;
	ti.m_flagLoaded = false ;
	ti.m_flagRefRosetta = true ;
	ti.m_regLoaded = -1 ;
	ti.m_regObject = -1 ;
	ti.m_addrOffset = 0 ;
	ti.m_pLocalVar = plv ;
	ti.m_pClass = plv->pClass ;
	ti.m_pImmediate = NULL ;
}

// NumberType -> RSClass 変換
//////////////////////////////////////////////////////////////////////////////
RSClass * RSCompiler::NumberTypeToClass( RSCompiler::NumberType type ) const
{
	static const RSCodeControl::WordIndex	wiWordType[] =
	{
		RSCodeControl::wiByte,
		RSCodeControl::wiChar,
		RSCodeControl::wiShort,
		RSCodeControl::wiChar,
		RSCodeControl::wiInt,
		RSCodeControl::wiLong,
		RSCodeControl::wiLong,
		RSCodeControl::wiFloat,
		RSCodeControl::wiDouble,
	} ;
	if ( type == typeObject )
	{
		return	m_context->GetGenericObjectClass() ;
	}
	return	m_context->GetBasicTypeClass( wiWordType[type] ) ;
}

// 参照や即値をレジスタにロード
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::LoadReferenceTemporary
		( RSCompiler::TypeInfo& ti, bool fTempReg )
{
	if ( ti.m_flagReference )
	{
		if ( ti.m_flagLocal )
		{
			ESLAssert( !ti.m_flagLoaded ) ;
			ESLAssert( ti.m_pLocalVar != NULL ) ;
			ESLAssert( !ti.m_flagLockAddr ) ;
			if ( !fTempReg && ti.m_pLocalVar->flagPointer )
			{
				ti.m_regLoaded = CommitLocalVariable( ti.m_pLocalVar ) ;
				if ( ti.m_regLoaded >= 0 )
				{
					ti.m_flagLoaded = true ;
					ti.m_flagLockAddr = true ;
					LockCacheRegister( ti.m_regLoaded ) ;
				}
			}
			if ( !ti.m_flagLoaded )
			{
				ti.m_flagLoaded = true ;
				ti.m_flagLockAddr = false ;
				ti.m_regLoaded = AllocateTemporaryRegister() ;
				ti.m_regObject = -1 ;
				ti.m_flagLocal = false ;
				LoadLocalVariable( ti.m_pLocalVar, ti.m_regLoaded ) ;
			}
			ti.m_flagLocal = false ;
		}
		else
		{
			int	regAddrBase = ti.m_regLoaded ;
			if ( ti.m_flagLockAddr )
			{
				UnlockCacheRegister( regAddrBase ) ;
				ti.m_regLoaded = AllocateTemporaryRegister() ;
				ti.m_flagLockAddr = false ;
			}
			ESLAssert( ti.m_typeNum != typeObject ) ;
			ESLAssert( !ti.m_flagPointer ) ;
			ESLAssert( ti.m_flagLoaded ) ;
			m_xmm->WriteCodeLoad
				( ti.m_regLoaded,
					TypeInfoToDataType( ti ),
					regZeroPtr, ti.m_addrOffset, regAddrBase ) ;
		}
		ti.m_addrOffset = 0 ;
		ti.m_flagReference = false ;
	}
	else if ( !ti.m_flagLoaded )
	{
		ESLAssert( ti.m_pImmediate != NULL ) ;
		if ( ti.m_flagPointer )
		{
			RSTypedArrayPointer *
				pPtrObj = ESLTypeCast<RSTypedArrayPointer>( ti.m_pImmediate ) ;
			if ( (pPtrObj != NULL)
				&& (pPtrObj->GetPointer() == NULL) )
			{
				ti.m_flagLoaded = true ;
				ti.m_flagLockAddr = false ;
				ti.m_regLoaded = AllocateTemporaryRegister() ;
				ti.m_regObject = -1 ;
				//
				m_xmm->WriteCodeMoveRegReg( ti.m_regLoaded, regIntZero ) ;
			}
			/*
			else if ( ti.m_pImmediate->GetEntityObject() == NULL )
			{
				ti.m_flagLoaded = true ;
				ti.m_flagLockAddr = false ;
				ti.m_regLoaded = AllocateTemporaryRegister() ;
				ti.m_regObject = -1 ;
				//
				m_xmm->WriteCodeMoveRegReg( ti.m_regLoaded, regIntZero ) ;
			}
			*/
			else
			{
				OutputError( L"不正なポインタ即値です" ) ;
				return	false ;
			}
		}
		else if ( ti.m_typeNum != typeObject )
		{
			ti.m_flagLoaded = true ;
			ti.m_flagLockAddr = false ;
			ti.m_regLoaded = AllocateTemporaryRegister() ;
			ti.m_regObject = -1 ;
			//
			if ( ti.m_typeNum <= typeInt64 )
			{
				int64_t	num ;
				if ( ti.m_pImmediate->AsInteger( num ) )
				{
					m_xmm->WriteCodeMoveRegInt64( ti.m_regLoaded, num ) ;
				}
				else
				{
					OutputError( L"不正な整数即値です" ) ;
					return	false ;
				}
			}
			else
			{
				double	num ;
				if ( ti.m_pImmediate->AsRealNumber( num ) )
				{
					m_xmm->WriteCodeMoveRegFloat64( ti.m_regLoaded, num ) ;
				}
				else
				{
					OutputError( L"不正な数値即値です" ) ;
					return	false ;
				}
			}
		}
		else
		{
			if ( ti.m_pImmediate->GetEntityObject() == NULL )
			{
				ti.m_flagLoaded = true ;
				ti.m_flagLockAddr = false ;
				ti.m_regLoaded = AllocateTemporaryRegister() ;
				ti.m_regObject = -1 ;
				//
				m_xmm->WriteCodeMoveRegReg( ti.m_regLoaded, regIntZero ) ;
			}
			else if ( (ESLTypeCast<RSClass>( ti.m_pImmediate ) != NULL)
				|| (ESLTypeCast<RSFunctionObject>( ti.m_pImmediate ) != NULL) )
			{
				ti.m_flagLoaded = true ;
				ti.m_flagLockAddr = false ;
				ti.m_flagRefRosetta = true ;
				ti.m_regLoaded = AllocateTemporaryRegister() ;
				ti.m_regObject = -1 ;
				//
				m_xmm->WriteCodeMoveRegImm64
					( ti.m_regLoaded, (ulong_ptr_t) ti.m_pImmediate ) ;
				//
				if ( ESLTypeCast<RSClass>( ti.m_pImmediate ) ==  NULL )
				{
					m_xmm->WriteCodePushReg( ti.m_regLoaded ) ;
					m_xmm->WriteCodeSyscall( L"__nrs_add_ref" ) ;
					m_xmm->WriteCodeAddSP( 8 ) ;
				}
			}
			else if ( ESLTypeCast<RSString>( ti.m_pImmediate ) != NULL )
			{
				ti.m_flagLoaded = true ;
				ti.m_flagLockAddr = false ;
				ti.m_flagRefRosetta = true ;
				ti.m_regLoaded = AllocateTemporaryRegister() ;
				ti.m_regObject = ti.m_regLoaded ;
				//
				RSString *	pStrImm = ESLTypeCast<RSString>( ti.m_pImmediate ) ;
				m_xmm->WriteCodeMoveRegConstString
								( regAcc, pStrImm->m_strValue ) ;
				m_xmm->WriteCodePushReg( regAcc ) ;
				m_xmm->WriteCodeSyscall( L"__nrs_new_String" ) ;
				m_xmm->WriteCodeAddSP( 8 ) ;
				m_xmm->WriteCodeMoveRegReg( ti.m_regLoaded, regAcc ) ;
			}
			else
			{
				OutputError( L"不正なオブジェクト即値です" ) ;
				return	false ;
			}
		}
		RSObject::ReleaseRef( ti.m_pImmediate ) ;
		ti.m_pImmediate = NULL ;
	}
	else
	{
		ESLAssert( ti.m_flagLoaded ) ;
		if ( fTempReg && ti.m_flagLockAddr )
		{
			int	regAddr = ti.m_regLoaded ;
			UnlockCacheRegister( regAddr ) ;
			ti.m_regLoaded = AllocateTemporaryRegister() ;
			ti.m_flagLockAddr = false ;
			m_xmm->WriteCodeMoveRegReg( ti.m_regLoaded, regAddr ) ;
		}
	}
	return	true ;
}

// 数値やポインタオブジェクトを数値やポインタへ変換
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::RealizeObjectToNumber( RSCompiler::TypeInfo& ti )
{
	if ( ti.IsObject() )
	{
		TypeInfo	tiTemp( NULL ) ;
		tiTemp.SetType( *m_context, ti.m_pClass ) ;
		if ( tiTemp.m_flagPointer )
		{
			ESLAssert( !ti.m_flagReference ) ;
			ESLAssert( ti.m_flagLoaded ) ;
			m_xmm->WriteCodePushReg( ti.m_regLoaded ) ;
			m_xmm->WriteCodeSyscall( L"__nrs_realize_pointer" ) ;
			m_xmm->WriteCodeAddSP( 8 ) ;
			//
			m_xmm->WriteCodePushReg( regAcc ) ;
			FreeTemporaryRegisterAndOwnerObject( ti ) ;
			//
			ti.m_regObject = AllocateTemporaryRegister() ;
			m_xmm->WriteCodePopReg( ti.m_regObject ) ;
			m_xmm->WriteCodeMoveRegReg( ti.m_regLoaded, ti.m_regObject ) ;
			//
			ti.m_typeNum = tiTemp.m_typeNum ;
			ti.m_flagPointer = true ;
			ti.m_addrOffset = 0 ;
			ESLAssert( !ti.m_flagLocal ) ;
			//
			CompileCheckException() ;
		}
		else if ( tiTemp.m_typeNum != typeObject )
		{
			ESLAssert( !ti.m_flagReference ) ;
			ESLAssert( ti.m_flagLoaded ) ;
			m_xmm->WriteCodePushReg( ti.m_regLoaded ) ;
			if ( tiTemp.m_typeNum <= typeInt64 )
			{
				m_xmm->WriteCodeSyscall( L"__nrs_realize_integer" ) ;
			}
			else
			{
				m_xmm->WriteCodeSyscall( L"__nrs_realize_real_number" ) ;
			}
			m_xmm->WriteCodeAddSP( 8 ) ;
			//
			m_xmm->WriteCodePushReg( regAcc ) ;
			FreeTemporaryRegisterAndOwnerObject( ti ) ;
			m_xmm->WriteCodePopReg( ti.m_regLoaded ) ;
			//
			ti.m_typeNum = tiTemp.m_typeNum ;
			ti.m_flagPointer = false ;
			//
			CompileCheckException() ;
		}
	}
	return	true ;
}

// ブール値へ変換しレジスタにロード
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::RealizeToBoolean( RSCompiler::TypeInfo& ti )
{
	RSClass *	pBooleanClass =
					m_context->GetBasicTypeClass( RSCodeControl::wiBoolean ) ;
	RSObject *	pObj = ti.GetImmediate() ;
	if ( pObj != NULL )
	{
		//
		// 即値
		//
		bool	b = pObj->AsBoolean() ;
		ti.SetLoadedNumber
			( *m_context, pBooleanClass, AllocateTemporaryRegister() ) ;
		if ( b )
		{
			m_xmm->WriteCodeMoveRegReg( ti.m_regLoaded, regFillBit ) ;
		}
		else
		{
			m_xmm->WriteCodeMoveRegReg( ti.m_regLoaded, regIntZero ) ;
		}
		return	true ;
	}
	if ( !LoadReferenceTemporary( ti, true ) )
	{
		return	false ;
	}
	ESLAssert( !ti.m_flagReference ) ;
	ESLAssert( ti.m_flagLoaded ) ;
	ESLAssert( !ti.m_flagLockAddr ) ;
	if ( ti.m_flagPointer || ti.IsInteger() )
	{
		//
		// 整数・ポインタ
		//
		FreeTemporaryRegisterAndOwnerObject( ti ) ;
		//
		if ( ti.m_pClass != pBooleanClass )
		{
			ti.SetLoadedNumber
				( *m_context, pBooleanClass, ti.m_regLoaded ) ;
			//
			m_xmm->WriteCode2OP( codeCmpNeReg, ti.m_regLoaded, regIntZero ) ;
		}
		return	true ;
	}
	else if ( ti.IsFloatingPoint() )
	{
		//
		// 浮動小数点
		//
		FreeTemporaryRegisterAndOwnerObject( ti ) ;
		//
		ti.SetLoadedNumber
			( *m_context, pBooleanClass, ti.m_regLoaded ) ;
		//
		m_xmm->WriteCode2OP( codeFCmpNeReg, ti.m_regLoaded, regIntZero ) ;
		return	true ;
	}
	else
	{
		//
		// オブジェクト
		//
		ESLAssert( ti.IsObject() ) ;
		//
		m_xmm->WriteCodePushReg( ti.m_regLoaded ) ;
		m_xmm->WriteCodeSyscall( L"__nrs_operator_boolean" ) ;
		m_xmm->WriteCodeAddSP( 8 ) ;
		m_xmm->WriteCodePushReg( regAcc ) ;
		//
		FreeTemporaryRegisterAndOwnerObject( ti ) ;
		//
		ti.SetLoadedNumber
			( *m_context, pBooleanClass, ti.m_regLoaded ) ;
		//
		m_xmm->WriteCodePopReg( ti.m_regLoaded ) ;
		return	true ;
	}
}

// オブジェクトやポインタのオブジェクト参照を保持するように正規化
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::TakeObjectReference( RSCompiler::TypeInfo& ti )
{
	if ( !LoadReferenceTemporary( ti, true ) )
	{
		return	false ;
	}
	ESLAssert( ti.m_regLoaded >= 0 ) ;
	if ( ti.m_flagPointer )
	{
		if ( ti.m_regObject < 0 )
		{
			m_xmm->WriteCodePushReg( ti.m_regLoaded ) ;
			m_xmm->WriteCodeSyscall( L"__nrs_add_ptr_ref" ) ;
			m_xmm->WriteCodeAddSP( 8 ) ;
			//
			ti.m_regObject = AllocateTemporaryRegister() ;
			ti.m_flagRefRosetta = false ;
			m_xmm->WriteCodeMoveRegReg( ti.m_regObject, ti.m_regLoaded ) ;
		}
	}
	else if ( ti.IsObject() )
	{
		if ( ti.m_regObject < 0 )
		{
			m_xmm->WriteCodePushReg( ti.m_regLoaded ) ;
			m_xmm->WriteCodeSyscall( L"__nrs_add_ref" ) ;
			m_xmm->WriteCodeAddSP( 8 ) ;
			//
			ti.m_regObject = ti.m_regLoaded ;
			ti.m_flagRefRosetta = true ;
		}
	}
	return	true ;
}

// 代入操作
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::OperatorMove
	( RSCompiler::TypeInfo& tiDst, RSCompiler::TypeInfo& tiSrc )
{
	if ( tiDst.m_accMod & RSObject::modifierConst )
	{
		OutputError( L"代入先が const 修飾されています" ) ;
		return	false ;
	}
	if ( !tiDst.m_flagReference )
	{
		if ( !LoadReferenceTemporary( tiDst ) )
		{
			return	false ;
		}
		if ( (tiDst.m_typeNum == typeObject)
			&& !tiDst.m_flagPointer && tiDst.m_flagLoaded )
		{
			if ( ConvertToObject( tiSrc ) )
			{
				ESLAssert( tiDst.m_flagLoaded ) ;
				ESLAssert( tiSrc.m_flagLoaded ) ;
				//
				m_xmm->WriteCodePushReg( tiSrc.m_regLoaded ) ;
				m_xmm->WriteCodePushReg( tiDst.m_regLoaded ) ;
				m_xmm->WriteCodeSyscall( L"__nrs_operator_move" ) ;
				m_xmm->WriteCodeAddSP( 16 ) ;
				m_xmm->WriteCodePushReg( regAcc ) ;
				//
				FreeTemporaryRegisterAndOwnerObject( tiDst ) ;
				//
				tiDst.m_regObject = tiDst.m_regLoaded ;
				m_xmm->WriteCodePopReg( tiDst.m_regLoaded ) ;
				return	true ;
			}
		}
		OutputError( L"代入先が左辺式ではありません" ) ;
		return	false ;
	}
	ESLAssert( !tiDst.m_flagLoaded ) ;
	if ( tiDst.m_flagPointer )
	{
		//
		// ポインタ変数への代入
		//
		ESLAssert( tiDst.m_flagLocal ) ;
		ESLAssert( tiDst.m_pLocalVar != NULL ) ;
		if ( !tiSrc.m_flagLoaded
			&& (tiSrc.m_pImmediate != NULL)
			&& (tiSrc.m_pImmediate->GetEntityObject() == NULL) )
		{
			FreePointerLocalVariable( tiDst.m_pLocalVar ) ;
			WriteLocalVariable( tiDst.m_pLocalVar, regIntZero ) ;
			return	true ;
		}
		if ( !LoadReferenceTemporary( tiSrc ) )
		{
			return	false ;
		}
		if ( !RealizeObjectToNumber( tiSrc ) )
		{
			return	false ;
		}
		ESLAssert( tiSrc.m_flagLoaded ) ;
		if ( !tiSrc.m_flagPointer )
		{
			OutputError( L"ポインタへの不正な代入です" ) ;
			return	false ;
		}
		RSStructuredPointerClass *
			pDstStruct = ESLTypeCast<RSStructuredPointerClass>( tiDst.m_pClass ) ;
		if ( pDstStruct != NULL )
		{
			//
			// 構造体ポインタの変換
			//
			RSStructuredPointerClass *
				pSrcStruct = ESLTypeCast<RSStructuredPointerClass>( tiSrc.m_pClass ) ;
			if ( pSrcStruct == NULL )
			{
				OutputError
					( SString(tiSrc.m_pClass->GetRSClassName()) + L" を "
						+ pDstStruct->GetRSClassName() + L" へ変換できません" ) ;
				return	false ;
			}
			ssize_t	iSuperOffset = pSrcStruct->OffsetSuperStruct( pDstStruct ) ;
			if ( iSuperOffset < 0 )
			{
				OutputError
					( SString(tiSrc.m_pClass->GetRSClassName()) + L" を "
						+ pDstStruct->GetRSClassName() + L" へ変換できません" ) ;
				return	false ;
			}
			if ( iSuperOffset > 0 )
			{
				// 派生元構造体への変換
				if ( !LoadReferenceTemporary( tiSrc, true ) )
				{
					return	false ;
				}
				ESLAssert( tiSrc.m_flagLoaded ) ;
				ESLAssert( !tiSrc.m_flagLockAddr ) ;
				int	regPtrTemp = AllocateTemporaryRegister() ;
				int	regMaskTemp = AllocateTemporaryRegister() ;
				m_xmm->WriteCodeMoveRegReg
					( regMaskTemp, tiSrc.m_regLoaded ) ;
				m_xmm->WriteCodeAddRegRegImm32
					( regPtrTemp, tiSrc.m_regLoaded, (int) iSuperOffset ) ;
				m_xmm->WriteCode2OP
					( codeCmpNeReg, regMaskTemp, regIntZero ) ;
				m_xmm->WriteCode3OP
					( codeMaskMove,
						tiSrc.m_regLoaded, regPtrTemp, regMaskTemp ) ;
				FreeTemporaryRegister( regPtrTemp ) ;
				FreeTemporaryRegister( regMaskTemp ) ;
				tiSrc.m_pClass = tiDst.m_pClass ;
			}
		}
		else
		{
			RSTypedArrayPointerClass *
				pDstPtrType = ESLTypeCast<RSTypedArrayPointerClass>( tiDst.m_pClass ) ;
			if ( pDstPtrType != NULL )
			{
				RSTypedArrayPointerClass *
					pSrcPtrType = ESLTypeCast<RSTypedArrayPointerClass>( tiSrc.m_pClass ) ;
				if ( (pSrcPtrType == NULL)
					|| ((pDstPtrType->m_typeElement != RSReferenceNumber::typeUint8)
						&& (pDstPtrType->m_typeElement != pSrcPtrType->m_typeElement)) )
				{
					OutputError
						( SString(tiSrc.m_pClass->GetRSClassName()) + L" を "
							+ pDstPtrType->GetRSClassName() + L" へ変換できません" ) ;
					return	false ;
				}
			}
			else
			{
				OutputError( L"不正なポインタへの代入です" ) ;
				return	false ;
			}
		}
		//
		// ポインタ値の代入
		//
		if ( tiSrc.m_addrOffset == 0 )
		{
			WriteLocalVariable( tiDst.m_pLocalVar, tiSrc.m_regLoaded ) ;
		}
		else
		{
			m_xmm->WriteCodeAddRegRegImm32
				( regAcc, tiSrc.m_regLoaded, tiSrc.m_addrOffset ) ;
			WriteLocalVariable( tiDst.m_pLocalVar, regAcc ) ;
		}
		//
		if ( tiSrc.m_regObject >= 0 )
		{
			// 所有オブジェクトの移譲
			ESLAssert( !tiSrc.m_flagRefRosetta ) ;
			FreePointerLocalVariable( tiDst.m_pLocalVar ) ;
			m_xmm->WriteCodeStore
				( tiSrc.m_regObject,
					dataInt64, regBP, (int) tiDst.m_pLocalVar->bpObjOffset ) ;
			FreeTemporaryRegister( tiSrc.m_regObject ) ;
			tiSrc.m_regObject = -1 ;
		}
		else
		{
			// ポインタの所有（異なるオブジェクトの場合）
			int	regPtrTemp = AllocateTemporaryRegister() ;
			m_xmm->WriteCode3OP
				( codeSllImm8, regPtrTemp, regMaskLow32, 32 ) ;
			m_xmm->WriteCodeLoad
				( regAcc,
					dataInt64, regBP, (int) tiDst.m_pLocalVar->bpObjOffset ) ;
			m_xmm->WriteCode2OP
				( codeAndReg, regPtrTemp, tiSrc.m_regLoaded ) ;
			m_xmm->WriteCode2OP
				( codeCmpEqReg, regAcc, regPtrTemp ) ;
			size_t	addrCJump = m_xmm->WriteCodeCJump( regAcc ) ;
			//
			m_xmm->WriteCodePushReg( regPtrTemp ) ;
			m_xmm->WriteCodeSyscall( L"__nrs_add_ptr_ref" ) ;
			m_xmm->WriteCodeAddSP( 8 ) ;
			//
			FreePointerLocalVariable( tiDst.m_pLocalVar ) ;
			//
			m_xmm->WriteCodeStore
				( regPtrTemp,
					dataInt64, regBP, (int) tiDst.m_pLocalVar->bpObjOffset ) ;
			//
			FreeTemporaryRegister( regPtrTemp ) ;
			m_xmm->CommitJumpAddress
				( addrCJump, m_xmm->GetNextCodeAddress() ) ;
		}
	}
	else if ( tiDst.m_typeNum != typeObject )
	{
		//
		// 数値変数への代入
		//
		if ( !LoadReferenceTemporary( tiSrc ) )
		{
			return	false ;
		}
		if ( !RealizeObjectToNumber( tiSrc ) )
		{
			return	false ;
		}
		if ( tiSrc.m_flagPointer )
		{
			ESLAssert( tiDst.m_pClass != NULL ) ;
			OutputError
				( SString(tiSrc.m_pClass->GetRSClassName()) + L" を "
					+ tiDst.m_pClass->GetRSClassName() + L" へ変換できません" ) ;
			return	false ;
		}
		ESLAssert( tiSrc.m_flagLoaded ) ;
		ESLAssert( !tiSrc.m_flagReference ) ;
		//
		if ( tiSrc.m_typeNum == typeObject )
		{
			m_xmm->WriteCodePushReg( tiSrc.m_regLoaded ) ;
			if ( tiDst.m_typeNum <= typeInt64 )
			{
				m_xmm->WriteCodeSyscall( L"__nrs_realize_integer" ) ;
				tiSrc.m_typeNum = typeInt64 ;
				tiSrc.m_pClass =
					m_context->GetBasicTypeClass( RSCodeControl::wiLong ) ;
			}
			else
			{
				m_xmm->WriteCodeSyscall( L"__nrs_realize_real_number" ) ;
				tiSrc.m_typeNum = typeFloat64 ;
				tiSrc.m_pClass =
					m_context->GetBasicTypeClass( RSCodeControl::wiDouble ) ;
			}
			m_xmm->WriteCodeAddSP( 8 ) ;
			//
			CompileCheckException() ;
			//
			m_xmm->WriteCodePushReg( regAcc ) ;
			FreeTemporaryRegisterAndOwnerObject( tiSrc ) ;
			m_xmm->WriteCodePopReg( tiSrc.m_regLoaded ) ;
		}
		int	regSrc = tiSrc.m_regLoaded ;
		if ( tiDst.m_typeNum <= typeInt64 )
		{
			if ( tiSrc.m_typeNum > typeInt64 )
			{
				// float -> int
				m_xmm->WriteCode2OP
					( codeCvtFloat2Int, regAcc, tiSrc.m_regLoaded ) ;
				regSrc = regAcc ;
			}
		}
		else
		{
			if ( tiSrc.m_typeNum <= typeInt64 )
			{
				// int -> float
				m_xmm->WriteCode2OP
					( codeCvtInt2Float, regAcc, tiSrc.m_regLoaded ) ;
				regSrc = regAcc ;
			}
		}
		if ( tiDst.m_flagLocal )
		{
			WriteLocalVariable( tiDst.m_pLocalVar, regSrc ) ;
		}
		else
		{
			m_xmm->WriteCodeStore
				( regSrc,
					NumberTypeToDataType(tiDst.m_typeNum),
					tiDst.m_regLoaded, tiDst.m_addrOffset ) ;
		}
	}
	else
	{
		ESLAssert( tiDst.m_flagLocal ) ;
		ESLAssert( tiDst.m_pLocalVar != NULL ) ;
		if ( tiDst.m_pClass != NULL )
		{
			if ( !OperatorObjectCast( tiSrc, tiDst.m_pClass ) )
			{
				return	false ;
			}
		}
		else
		{
			if ( !ConvertToObject( tiSrc ) )
			{
				return	false ;
			}
		}
		if ( !LoadReferenceTemporary( tiSrc ) )
		{
			return	false ;
		}
		ESLAssert( tiSrc.m_flagLoaded ) ;
		ESLAssert( !tiSrc.m_flagReference ) ;
		//
		ReleaseRefLocalVariable( tiDst.m_pLocalVar ) ;
		//
		WriteLocalVariable( tiDst.m_pLocalVar, tiSrc.m_regLoaded ) ;
		m_xmm->WriteCodeStore
			( tiSrc.m_regLoaded,
				dataInt64, regBP, (int) tiDst.m_pLocalVar->bpObjOffset ) ;
		//
		m_xmm->WriteCodePushReg( tiSrc.m_regLoaded ) ;
		m_xmm->WriteCodeSyscall( L"__nrs_add_ref" ) ;
		m_xmm->WriteCodeAddSP( 8 ) ;
	}
	return	true ;
}

// キャスト
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::OperatorCast
	( RSCompiler::TypeInfo& tiDst, RSClass * pClass, bool fForceCast )
{
	RSObject *	pImmObj = tiDst.GetImmediate() ;
	if ( pImmObj != NULL )
	{
		//
		// 即値のキャスト
		//
		RSObject *	pObj =
			pClass->CastInstance
				( *m_context, pImmObj,
					(fForceCast ? RSClass::castForce : RSClass::castNatural) ) ;
		if ( pObj == NULL )
		{
			OutputError
				( SString(pImmObj->GetTypeName())
					+ L" から " + pClass->GetRSClassName()
					+ L" へキャストできません" ) ;
		}
		if ( m_context->IsException() )
		{
			return	false ;
		}
		tiDst.SetImmediate( *m_context, pObj, pObj->GetRSClass() ) ;
		return	true ;
	}
	RSStructuredPointerClass *
		pStructClass = ESLTypeCast<RSStructuredPointerClass>( pClass ) ;
	if ( pStructClass != NULL )
	{
		//
		// 構造体ポインタのキャスト
		//
		if ( !LoadReferenceTemporary( tiDst, true )
			|| !RealizeObjectToNumber( tiDst ) )
		{
			return	false ;
		}
		RSStructuredPointerClass *
			pSrcStruct = ESLTypeCast<RSStructuredPointerClass>( tiDst.m_pClass ) ;
		if ( !tiDst.m_flagPointer || (pSrcStruct == NULL) )
		{
			OutputError( SString(pClass->GetRSClassName()) + L" へキャストできません" ) ;
			return	false ;
		}
		ssize_t	iCastOffset = pSrcStruct->OffsetSuperStruct( pStructClass ) ;
		if ( iCastOffset < 0 )
		{
			if ( fForceCast )
			{
				iCastOffset = pStructClass->OffsetSuperStruct( pSrcStruct ) ;
			}
			if ( iCastOffset < 0 )
			{
				OutputError( SString(pClass->GetRSClassName()) + L" へキャストできません" ) ;
				return	false ;
			}
			iCastOffset = - iCastOffset ;
		}
		ESLAssert( tiDst.m_flagLoaded ) ;
		ESLAssert( !tiDst.m_flagLockAddr ) ;
		if ( iCastOffset != 0 )
		{
			m_xmm->WriteCodeMoveRegReg( regAcc, tiDst.m_regLoaded ) ;
			m_xmm->WriteCodeAddRegRegImm32
				( tiDst.m_regLoaded, tiDst.m_regLoaded, (int) iCastOffset ) ;
			m_xmm->WriteCode2OP( codeCmpNeReg, regAcc, regIntZero ) ;
			m_xmm->WriteCode2OP( codeAndReg, tiDst.m_regLoaded, regAcc ) ;
		}
		tiDst.ChangeType( *m_context, pStructClass ) ;
		return	true ;
	}
	RSTypedArrayPointerClass *
		pPtrClass = ESLTypeCast<RSTypedArrayPointerClass>( pClass ) ;
	if ( pPtrClass != NULL )
	{
		//
		// ポインタの変換
		//
		if ( !LoadReferenceTemporary( tiDst, true )
			|| !RealizeObjectToNumber( tiDst ) )
		{
			return	false ;
		}
		if ( !tiDst.m_flagPointer
			|| (pPtrClass->m_typeElement != RSReferenceNumber::typeUint8) )
		{
			OutputError( SString(pClass->GetRSClassName()) + L" へキャストできません" ) ;
			return	false ;
		}
		tiDst.ChangeType( *m_context, pPtrClass ) ;
		return	true ;
	}
	RSBooleanClass *	pBoolClass = ESLTypeCast<RSBooleanClass>( pClass ) ;
	if ( pBoolClass != NULL )
	{
		//
		// ブールへのキャスト
		//
		if ( !LoadReferenceTemporary( tiDst, true )
			|| !RealizeObjectToNumber( tiDst ) )
		{
			return	false ;
		}
		ESLAssert( tiDst.m_flagLoaded ) ;
		if ( tiDst.m_pClass
				== m_context->GetBasicTypeClass( RSCodeControl::wiBoolean ) )
		{
			return	true ;
		}
		if ( tiDst.m_flagPointer || tiDst.IsInteger() )
		{
			m_xmm->WriteCode2OP
				( codeCmpNeReg, tiDst.m_regLoaded, regIntZero ) ;
			tiDst.ChangeType( *m_context, pBoolClass ) ;
			return	true ;
		}
		if ( tiDst.IsFloatingPoint() )
		{
			m_xmm->WriteCode2OP
				( codeFCmpNeReg, tiDst.m_regLoaded, regIntZero ) ;
			tiDst.ChangeType( *m_context, pBoolClass ) ;
			return	true ;
		}
		return	OperatorObjectCast( tiDst, pClass, fForceCast ) ;
	}
	RSIntegerClass *	pIntClass = ESLTypeCast<RSIntegerClass>( pClass ) ;
	if ( pIntClass != NULL )
	{
		//
		// 整数へのキャスト
		//
		if ( !LoadReferenceTemporary( tiDst, true )
			|| !RealizeObjectToNumber( tiDst ) )
		{
			return	false ;
		}
		ESLAssert( tiDst.m_flagLoaded ) ;
		if ( tiDst.m_flagPointer )
		{
			OutputError( SString(pClass->GetRSClassName()) + L" へキャストできません" ) ;
			return	false ;
		}
		if ( tiDst.IsFloatingPoint() )
		{
			if ( pIntClass->m_intType != RSInteger::typeBoolean )
			{
				m_xmm->WriteCode2OP
					( codeCvtFloat2Int, tiDst.m_regLoaded, tiDst.m_regLoaded ) ;
			}
			switch ( pIntClass->m_intType )
			{
			case	RSInteger::typeBoolean:
				m_xmm->WriteCode2OP
					( codeFCmpNeReg, tiDst.m_regLoaded, regIntZero ) ;
				break ;
			case	RSInteger::typeInt8:
				m_xmm->WriteCode2OP
					( codeMoveSx8Reg, tiDst.m_regLoaded, tiDst.m_regLoaded ) ;
				break ;
			case	RSInteger::typeUint8:
				m_xmm->WriteCode2OP
					( codeAndReg, tiDst.m_regLoaded, regMaskLow8 ) ;
				break ;
			case	RSInteger::typeInt16:
				m_xmm->WriteCode2OP
					( codeMoveSx16Reg, tiDst.m_regLoaded, tiDst.m_regLoaded ) ;
				break ;
			case	RSInteger::typeUint16:
				m_xmm->WriteCode2OP
					( codeAndReg, tiDst.m_regLoaded, regMaskLow16 ) ;
				break ;
			case	RSInteger::typeInt32:
				m_xmm->WriteCode2OP
					( codeMoveSx32Reg, tiDst.m_regLoaded, tiDst.m_regLoaded ) ;
				break ;
			case	RSInteger::typeUint32:
				m_xmm->WriteCode2OP
					( codeAndReg, tiDst.m_regLoaded, regMaskLow32 ) ;
				break ;
			default:
				break ;
			}
			tiDst.ChangeType( *m_context, pIntClass ) ;
			return	true ;
		}
		else if ( tiDst.IsInteger() )
		{
			switch ( pIntClass->m_intType )
			{
			case	RSInteger::typeBoolean:
				m_xmm->WriteCode2OP
					( codeCmpNeReg, tiDst.m_regLoaded, regIntZero ) ;
				break ;
			case	RSInteger::typeInt8:
				if ( tiDst.m_typeNum != typeInt8 )
				{
					m_xmm->WriteCode2OP
						( codeMoveSx8Reg, tiDst.m_regLoaded, tiDst.m_regLoaded ) ;
				}
				break ;
			case	RSInteger::typeUint8:
				if ( tiDst.m_typeNum != typeUint8 )
				{
					m_xmm->WriteCode2OP
						( codeAndReg, tiDst.m_regLoaded, regMaskLow8 ) ;
				}
				break ;
			case	RSInteger::typeInt16:
				if ( tiDst.m_typeNum >= typeUint16 )
				{
					m_xmm->WriteCode2OP
						( codeMoveSx16Reg, tiDst.m_regLoaded, tiDst.m_regLoaded ) ;
				}
				break ;
			case	RSInteger::typeUint16:
				if ( (tiDst.m_typeNum != typeUint8)
					&& (tiDst.m_typeNum != typeUint16) )
				{
					m_xmm->WriteCode2OP
						( codeAndReg, tiDst.m_regLoaded, regMaskLow16 ) ;
				}
				break ;
			case	RSInteger::typeInt32:
				if ( tiDst.m_typeNum >= typeUint32 )
				{
					m_xmm->WriteCode2OP
						( codeMoveSx32Reg, tiDst.m_regLoaded, tiDst.m_regLoaded ) ;
				}
				break ;
			case	RSInteger::typeUint32:
				if ( (tiDst.m_typeNum != typeUint8)
					&& (tiDst.m_typeNum != typeUint16)
					&& (tiDst.m_typeNum != typeUint32) )
				{
					m_xmm->WriteCode2OP
						( codeAndReg, tiDst.m_regLoaded, regMaskLow32 ) ;
				}
				break ;
			default:
				break ;
			}
			tiDst.ChangeType( *m_context, pIntClass ) ;
			return	true ;
		}
		return	OperatorObjectCast( tiDst, pClass, fForceCast ) ;
	}
	RSNumberClass *	pNumClass = ESLTypeCast<RSNumberClass>( pClass ) ;
	if ( pNumClass != NULL )
	{
		//
		// 浮動小数点へのキャスト
		//
		if ( !LoadReferenceTemporary( tiDst, true )
			|| !RealizeObjectToNumber( tiDst ) )
		{
			return	false ;
		}
		ESLAssert( tiDst.m_flagLoaded ) ;
		if ( tiDst.m_flagPointer )
		{
			OutputError( SString(pClass->GetRSClassName()) + L" へキャストできません" ) ;
			return	false ;
		}
		if ( tiDst.IsFloatingPoint() )
		{
			tiDst.ChangeType( *m_context, pNumClass ) ;
			return	true ;
		}
		if ( tiDst.IsInteger() )
		{
			m_xmm->WriteCode2OP
				( codeCvtInt2Float, tiDst.m_regLoaded, tiDst.m_regLoaded ) ;
			tiDst.ChangeType( *m_context, pNumClass ) ;
			return	true ;
		}
		return	OperatorObjectCast( tiDst, pClass, fForceCast ) ;
	}
	return	OperatorObjectCast( tiDst, pClass, fForceCast ) ;
}

// オブジェクトキャスト
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::OperatorObjectCast
	( RSCompiler::TypeInfo& tiDst, RSClass * pClass, bool fForceCast )
{
	if ( !ConvertToObject( tiDst ) )
	{
		return	false ;
	}
	ESLAssert( !tiDst.m_flagPointer && (tiDst.m_typeNum == typeObject) ) ;
	ESLAssert( tiDst.m_flagLoaded ) ;
	FlushAllLocalVariableCaches() ;
	if ( m_xrcRegs.nAlloc > 0 )
	{
		m_xmm->WriteCodePushRegsImm8( 16, m_xrcRegs.nAlloc ) ;
	}
	m_xmm->WriteCodeMoveRegInt64
		( regAcc, (fForceCast ? RSClass::castForce : RSClass::castNatural) ) ;
	m_xmm->WriteCodePushReg( regAcc ) ;
	m_xmm->WriteCodeMoveRegInt64( regAcc, (long_ptr_t) pClass ) ;
	m_xmm->WriteCodePushReg( tiDst.m_regLoaded ) ;
	m_xmm->WriteCodePushReg( regAcc ) ;
	m_xmm->WriteCodeSyscall( L"__nrs_cast_object" ) ;
		m_xmm->WriteCodeAddSP( 8 * 3 ) ;
	if ( m_xrcRegs.nAlloc > 0 )
	{
		m_xmm->WriteCodePopRegsImm8( 16, m_xrcRegs.nAlloc ) ;
	}
	if ( tiDst.m_regObject >= 0 )
	{
		m_xmm->WriteCodePushReg( regAcc ) ;
		//
		if ( tiDst.m_flagRefRosetta )
		{
			m_xmm->WriteCodePushReg( tiDst.m_regObject ) ;
			m_xmm->WriteCodeSyscall( L"__nrs_release_ref" ) ;
			m_xmm->WriteCodeAddSP( 8 ) ;
		}
		else
		{
			m_xmm->WriteCodePushReg( tiDst.m_regObject ) ;
			m_xmm->WriteCodeSyscall( L"__nrs_release_ptr_ref" ) ;
			m_xmm->WriteCodeAddSP( 8 ) ;
		}
		//
		m_xmm->WriteCodePopReg( tiDst.m_regObject ) ;
	}
	else
	{
		tiDst.m_regObject = tiDst.m_regLoaded ;
		m_xmm->WriteCodeMoveRegReg( tiDst.m_regObject, regAcc ) ;
	}
	if ( tiDst.m_regObject != tiDst.m_regLoaded )
	{
		m_xmm->WriteCodeMoveRegReg( tiDst.m_regLoaded, tiDst.m_regObject ) ;
	}
	tiDst.m_flagRefRosetta = true ;
	tiDst.m_pClass = pClass ;
	//
	CompileCheckException() ;
	//
	return	true ;
}

// Rosetta オブジェクトへ変換
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::ConvertToObject( RSCompiler::TypeInfo& tiDst )
{
	if ( !LoadReferenceTemporary( tiDst ) )
	{
		return	false ;
	}
	if ( tiDst.m_flagPointer )
	{
		ESLAssert( tiDst.m_pClass != NULL ) ;
		ESLAssert( tiDst.m_flagLoaded ) ;
		if ( tiDst.m_addrOffset == 0 )
		{
			m_xmm->WriteCodePushReg( tiDst.m_regLoaded ) ;
		}
		else
		{
			m_xmm->WriteCodeAddRegRegImm32
				( regAcc, tiDst.m_regLoaded, tiDst.m_addrOffset ) ;
			m_xmm->WriteCodePushReg( regAcc ) ;
		}
		m_xmm->WriteCodeMoveRegImm64( regAcc, (long_ptr_t) tiDst.m_pClass ) ;
		m_xmm->WriteCodePushReg( regAcc ) ;
		m_xmm->WriteCodeSyscall( L"__nrs_new_pointer" ) ;
		m_xmm->WriteCodeAddSP( 16 ) ;
		//
		m_xmm->WriteCodePushReg( regAcc ) ;
		//
		if ( tiDst.m_flagLockAddr )
		{
			UnlockCacheRegister( tiDst.m_regLoaded ) ;
			tiDst.m_flagLockAddr = false ;
			tiDst.m_regLoaded = AllocateTemporaryRegister() ;
		}
		FreeTemporaryRegisterAndOwnerObject( tiDst ) ;
		//
		m_xmm->WriteCodePopReg( tiDst.m_regLoaded ) ;
		//
		tiDst.m_typeNum = typeObject ;
		tiDst.m_flagPointer = false ;
		tiDst.m_flagRefRosetta = true ;
		tiDst.m_typeNum = typeObject ;
		ESLAssert( tiDst.m_regObject < 0 ) ;
		tiDst.m_regObject = AllocateTemporaryRegister() ;
		m_xmm->WriteCodeMoveRegReg( tiDst.m_regObject, tiDst.m_regLoaded ) ;
	}
	else if ( tiDst.m_typeNum != typeObject )
	{
		ESLAssert( tiDst.m_flagLoaded ) ;
		m_xmm->WriteCodePushReg( tiDst.m_regLoaded ) ;
		if ( tiDst.m_typeNum <= typeInt64 )
		{
			m_xmm->WriteCodeSyscall( L"__nrs_new_integer" ) ;
		}
		else
		{
			m_xmm->WriteCodeSyscall( L"__nrs_new_real_number" ) ;
		}
		m_xmm->WriteCodeAddSP( 8 ) ;
		m_xmm->WriteCodeMoveRegReg( tiDst.m_regLoaded, regAcc ) ;
		//
		FreeTemporaryRegisterAndOwnerObject( tiDst ) ;
		//
		tiDst.m_typeNum = typeObject ;
		tiDst.m_flagPointer = false ;
		tiDst.m_flagRefRosetta = true ;
		ESLAssert( tiDst.m_regObject < 0 ) ;
		tiDst.m_regObject = AllocateTemporaryRegister() ;
		m_xmm->WriteCodeMoveRegReg( tiDst.m_regObject, tiDst.m_regLoaded ) ;
	}
	return	true ;
}

// 数値やオブジェクトを複製する
//////////////////////////////////////////////////////////////////////////////
bool RSCompiler::CloneObject
	( RSCompiler::TypeInfo& tiDst, RSCompiler::TypeInfo& tiSrc )
{
	tiDst = tiSrc ;
	//
	RSObject *	pObj = tiDst.GetImmediate() ;
	if ( pObj != NULL )
	{
		pObj = pObj->CloneObject( *m_context ) ;
		tiDst.SetImmediate( *m_context, pObj, tiDst.m_pClass ) ;
		return	true ;
	}
	if ( tiDst.m_flagLoaded )
	{
		if ( tiDst.m_flagLockAddr )
		{
			LockCacheRegister( tiDst.m_regLoaded ) ;
		}
		else
		{
			ESLAssert( tiSrc.m_flagLoaded ) ;
			tiDst.m_regLoaded = AllocateTemporaryRegister() ;
			m_xmm->WriteCodeMoveRegReg( tiDst.m_regLoaded, tiSrc.m_regLoaded ) ;
		}
		tiDst.m_regObject = -1 ;
	}
	if ( !LoadReferenceTemporary( tiDst, true ) )
	{
		return	false ;
	}
	if ( tiDst.IsObject() )
	{
		FlushAllLocalVariableCaches() ;
		//
		ExprRegContext	xrc ;
		PushAllTemporaryRegisters( xrc ) ;
		//
		m_xmm->WriteCodePushReg( tiDst.m_regLoaded ) ;
		m_xmm->WriteCodeSyscall( L"__nrs_clone_object" ) ;
		m_xmm->WriteCodeAddSP( 8 ) ;
		//
		PopAllTemporaryRegisters( xrc ) ;
		//
		m_xmm->WriteCodeMoveRegReg( tiDst.m_regLoaded, regAcc ) ;
		//
		tiDst.m_flagRefRosetta = true ;
		tiDst.m_regObject = tiDst.m_regLoaded ;
		return	true ;
	}
	return	true ;
}

// import 文
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileStatementImport
	( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	m_flagInvalidFunc = true ;
}

// class 文
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileStatementClass
	( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	m_flagInvalidFunc = true ;
}

// struct 文
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileStatementStruct
	( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	m_flagInvalidFunc = true ;
}

// function 文
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileStatementFunction
	( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	m_flagInvalidFunc = true ;
}

// for 文
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileStatementFor
	( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	RSParenthesis *	pPrthFor =
			cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	RSParenthesis *	pPrthCode =
			cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
	if ( pPrthFor == NULL )
	{
		OutputError( L"for 文の構文エラーです" ) ;
		return ;
	}
	RSCodeStream	csFor( *pPrthFor ) ;
	RSCodeStream	csCode ;
	if ( pPrthCode != NULL )
	{
		csCode.AttachCode( *pPrthCode ) ;
	}
	else
	{
		size_t	iStart = cstrm.GetIndex() ;
		cstrm.PassAStatement() ;
		csCode.AttachCode( cstrm, iStart, (ssize_t) cstrm.GetIndex() ) ;
	}
	RSCode *		pCodeInSym = csFor.GetTerm( 1 ) ;
	if ( (pCodeInSym != NULL)
		&& (pCodeInSym->m_type == RSCode::typeSymbol)
		&& (((RSCodeSymbol*)pCodeInSym)->m_symbol == L"in") )
	{
		//
		// for ( <var-name> in <obj-expr> ) { ... }
		//
		m_flagInvalidFunc = true ;
		return ;
	}
	FlushAllLocalVariableCaches() ;
	//
	// for ( <init-expr> ; <cond-expr> ; <step-expr> ) { ... }
	//
	LocalSpace *	plsFor = PushLocalSpace() ;
	plsFor->m_flagLoop = true ;
	//
	RSClass *	pIterVarType = m_context->ParseClassExpression( csFor ) ;
	if ( pIterVarType != NULL )
	{
		CompileDeclareVariable( csFor, 0, pIterVarType ) ;
		if ( m_flagInvalidFunc )
		{
			ESLVerify( plsFor == PopLocalSpace() ) ;
			FreeLocalSpace( plsFor ) ;
			delete	plsFor ;
			return ;
		}
	}
	else if ( csFor.NextOperator
				( RSCodeOperator::opEndOfStatement ) == NULL )
	{
		TypeInfo	tiExpr( this ) ;
		if ( !CompileExpression( tiExpr, csFor ) || m_flagInvalidFunc )
		{
			FreeTemporaryRegister( tiExpr ) ;
			ESLVerify( plsFor == PopLocalSpace() ) ;
			FreeLocalSpace( plsFor ) ;
			delete	plsFor ;
			return ;
		}
		FreeTemporaryRegister( tiExpr ) ;
		csFor.NextOperator( RSCodeOperator::opEndOfStatement ) ;
	}
	FlushAllLocalVariableCaches() ;
	//
	// 反復条件式
	//
	size_t	addrLoop = m_xmm->GetNextCodeAddress() ;
	size_t	addrJumpBreak = (size_t) -1 ;
	//
	if ( csFor.NextOperator
			( RSCodeOperator::opEndOfStatement ) == NULL )
	{
		TypeInfo	tiCondition( this ) ;
		if ( !CompileExpression
				( tiCondition, csFor, RSCodeOperator::priorityNothing )
			|| m_flagInvalidFunc )
		{
			FreeTemporaryRegister( tiCondition ) ;
			ESLVerify( plsFor == PopLocalSpace() ) ;
			FreeLocalSpace( plsFor ) ;
			delete	plsFor ;
			return ;
		}
		if ( !RealizeToBoolean( tiCondition ) )
		{
			FreeTemporaryRegister( tiCondition ) ;
			ESLVerify( plsFor == PopLocalSpace() ) ;
			FreeLocalSpace( plsFor ) ;
			delete	plsFor ;
			return ;
		}
		FlushAllLocalVariableCaches() ;
		addrJumpBreak =
			m_xmm->WriteCodeNCJump( tiCondition.m_regLoaded ) ;
		FreeTemporaryRegister( tiCondition ) ;
		csFor.NextOperator( RSCodeOperator::opEndOfStatement ) ;
	}
	size_t	addrJumpEnter = m_xmm->WriteCodeJump() ;
	//
	// 反復子更新
	//
	size_t	addrContinue = m_xmm->GetNextCodeAddress() ;
	if ( !csFor.IsEndOfStream() )
	{
		TypeInfo	tiStep( this ) ;
		if ( !CompileExpression( tiStep, csFor )
			|| m_flagInvalidFunc )
		{
			FreeTemporaryRegister( tiStep ) ;
			ESLVerify( plsFor == PopLocalSpace() ) ;
			FreeLocalSpace( plsFor ) ;
			delete	plsFor ;
			return ;
		}
		FreeTemporaryRegister( tiStep ) ;
	}
	FlushAllLocalVariableCaches() ;
	m_xmm->WriteCodeJump( addrLoop ) ;
	//
	// ループ本体
	//
	m_xmm->CommitJumpAddress( addrJumpEnter, m_xmm->GetNextCodeAddress() ) ;
	//
	LocalSpace *	plsLoop = PushLocalSpace() ;
	plsLoop->m_flagLoop = true ;
	if ( addrJumpBreak != (size_t) -1 )
	{
		plsLoop->m_arrBreakRefAddr.Add( addrJumpBreak ) ;
	}
	//
	CompileMultiStatements( csCode ) ;
	if ( m_flagInvalidFunc )
	{
		ESLVerify( plsLoop == PopLocalSpace() ) ;
		FreeLocalSpace( plsLoop ) ;
		ESLVerify( plsFor == PopLocalSpace() ) ;
		FreeLocalSpace( plsFor ) ;
		delete	plsLoop ;
		delete	plsFor ;
		return ;
	}
	//
	ESLVerify( plsLoop == PopLocalSpace() ) ;
	FreeLocalSpace( plsLoop ) ;
	//
	FlushAllLocalVariableCaches() ;
	m_xmm->WriteCodeJump( addrContinue ) ;
	//
	// ループ脱出
	//
	for ( size_t i = 0; i < plsLoop->m_arrBreakRefAddr.GetLength(); i ++ )
	{
		m_xmm->CommitJumpAddress
			( plsLoop->m_arrBreakRefAddr.At(i), m_xmm->GetNextCodeAddress() ) ;
	}
	for ( size_t i = 0; i < plsLoop->m_arrContinueRefAddr.GetLength(); i ++ )
	{
		m_xmm->CommitJumpAddress
			( plsLoop->m_arrContinueRefAddr.At(i), addrContinue ) ;
	}
	ESLVerify( plsFor == PopLocalSpace() ) ;
	FreeLocalSpace( plsFor ) ;
	//
	delete	plsLoop ;
	delete	plsFor ;
}

// while 文
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileStatementWhile
	( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	//
	// while( <expr> ) { ... }
	//
	RSParenthesis *	pPrthWhile =
			cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	RSParenthesis *	pPrthCode =
			cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
	if ( pPrthWhile == NULL )
	{
		OutputError( L"while 文の構文エラーです" ) ;
		m_flagInvalidFunc = true ;
		return ;
	}
	RSCodeStream	csWhile( *pPrthWhile ) ;
	RSCodeStream	csCode ;
	if ( pPrthCode != NULL )
	{
		csCode.AttachCode( *pPrthCode ) ;
	}
	else
	{
		size_t	iStart = cstrm.GetIndex() ;
		cstrm.PassAStatement() ;
		csCode.AttachCode( cstrm, iStart, (ssize_t) cstrm.GetIndex() ) ;
	}
	//
	// 反復条件判定
	//
	FlushAllLocalVariableCaches() ;
	//
	size_t	addrContinue = m_xmm->GetNextCodeAddress() ;
	//
	TypeInfo	tiCondition( this ) ;
	if ( !CompileExpression( tiCondition, csWhile )
		|| m_flagInvalidFunc )
	{
		FreeTemporaryRegister( tiCondition ) ;
		return ;
	}
	if ( !RealizeToBoolean( tiCondition ) )
	{
		FreeTemporaryRegister( tiCondition ) ;
		return ;
	}
	FlushAllLocalVariableCaches() ;
	//
	size_t	addrJumpBreak =
				m_xmm->WriteCodeNCJump( tiCondition.m_regLoaded ) ;
	FreeTemporaryRegister( tiCondition ) ;
	//
	// 反復実行
	//
	LocalSpace *	plsLoop = PushLocalSpace() ;
	plsLoop->m_flagLoop = true ;
	plsLoop->m_arrBreakRefAddr.Add( addrJumpBreak ) ;
	//
	CompileMultiStatements( csCode ) ;
	//
	ESLVerify( plsLoop == PopLocalSpace() ) ;
	FreeLocalSpace( plsLoop ) ;
	//
	FlushAllLocalVariableCaches() ;
	m_xmm->WriteCodeJump( addrContinue ) ;
	//
	// ループ脱出
	//
	for ( size_t i = 0; i < plsLoop->m_arrBreakRefAddr.GetLength(); i ++ )
	{
		m_xmm->CommitJumpAddress
			( plsLoop->m_arrBreakRefAddr.At(i), m_xmm->GetNextCodeAddress() ) ;
	}
	for ( size_t i = 0; i < plsLoop->m_arrContinueRefAddr.GetLength(); i ++ )
	{
		m_xmm->CommitJumpAddress
			( plsLoop->m_arrContinueRefAddr.At(i), addrContinue ) ;
	}
	delete	plsLoop ;
}

// do 文
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileStatementDo
	( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	//
	// do { ... } while( <expr> ) ;
	//
	RSParenthesis *	pPrthCode =
			cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
	RSCodeStream	csCode ;
	if ( pPrthCode != NULL )
	{
		csCode.AttachCode( *pPrthCode ) ;
	}
	else
	{
		size_t	iStart = cstrm.GetIndex() ;
		cstrm.PassAStatement() ;
		csCode.AttachCode( cstrm, iStart, (ssize_t) cstrm.GetIndex() ) ;
	}
	if ( cstrm.NextControlWord( RSCodeControl::wiWhile ) == NULL )
	{
		OutputError( L"do ～ while 文の構文エラーです" ) ;
		return ;
	}
	RSParenthesis *	pPrthWhile =
			cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	if ( pPrthWhile == NULL )
	{
		OutputError( L"do ～ while 文の反復条件がありません" ) ;
		return ;
	}
	if ( cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) == NULL )
	{
		OutputError( L"do ～ while 文末のセミコロンがありません" ) ;
		return ;
	}
	//
	// 反復実行
	//
	FlushAllLocalVariableCaches() ;
	//
	size_t	addrStart = m_xmm->GetNextCodeAddress() ;
	//
	LocalSpace *	plsLoop = PushLocalSpace() ;
	plsLoop->m_flagLoop = true ;
	//
	CompileMultiStatements( csCode ) ;
	//
	ESLVerify( plsLoop == PopLocalSpace() ) ;
	FreeLocalSpace( plsLoop ) ;
	//
	// 反復条件判定
	//
	FlushAllLocalVariableCaches() ;
	//
	size_t	addrContinue = m_xmm->GetNextCodeAddress() ;
	//
	RSCodeStream	csWhile( *pPrthWhile ) ;
	TypeInfo		tiCondition( this ) ;
	if ( !CompileExpression( tiCondition, csWhile )
		|| m_flagInvalidFunc )
	{
		FreeTemporaryRegister( tiCondition ) ;
		return ;
	}
	if ( !RealizeToBoolean( tiCondition ) )
	{
		FreeTemporaryRegister( tiCondition ) ;
		return ;
	}
	FlushAllLocalVariableCaches() ;
	//
	m_xmm->WriteCodeCJump( tiCondition.m_regLoaded, addrStart ) ;
	FreeTemporaryRegister( tiCondition ) ;
	//
	// ループ脱出
	//
	for ( size_t i = 0; i < plsLoop->m_arrBreakRefAddr.GetLength(); i ++ )
	{
		m_xmm->CommitJumpAddress
			( plsLoop->m_arrBreakRefAddr.At(i), m_xmm->GetNextCodeAddress() ) ;
	}
	for ( size_t i = 0; i < plsLoop->m_arrContinueRefAddr.GetLength(); i ++ )
	{
		m_xmm->CommitJumpAddress
			( plsLoop->m_arrContinueRefAddr.At(i), addrContinue ) ;
	}
	delete	plsLoop ;
}

// if 文
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileStatementIf
	( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	//
	// if ( <expr> ) <statement> [else <statement>]
	//
	RSParenthesis *	pPrthIf =
			cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	if ( pPrthIf == NULL )
	{
		OutputError( L"if 文の条件式がありません" ) ;
		return ;
	}
	RSCodeStream	csIf( *pPrthIf ) ;
	TypeInfo		tiCondition( this ) ;
	if ( !CompileExpression( tiCondition, csIf )
		|| m_flagInvalidFunc )
	{
		FreeTemporaryRegister( tiCondition ) ;
		return ;
	}
	if ( !RealizeToBoolean( tiCondition ) )
	{
		FreeTemporaryRegister( tiCondition ) ;
		return ;
	}
	FlushAllLocalVariableCaches() ;
	//
	size_t	addrIfJump = m_xmm->WriteCodeNCJump( tiCondition.m_regLoaded ) ;
	FreeTemporaryRegister( tiCondition ) ;
	//
	LocalSpace *	plsIf = PushLocalSpace() ;
	CompileStatements( cstrm ) ;
	if ( m_flagInvalidFunc )
	{
		ESLVerify( plsIf == PopLocalSpace() ) ;
		FreeLocalSpace( plsIf ) ;
		delete	plsIf ;
		return ;
	}
	ESLVerify( plsIf == PopLocalSpace() ) ;
	FreeLocalSpace( plsIf ) ;
	delete	plsIf ;
	//
	FlushAllLocalVariableCaches() ;
	//
	if ( cstrm.NextControlWord( RSCodeControl::wiElse ) != NULL )
	{
		size_t	addrElseJump = m_xmm->WriteCodeJump() ;
		m_xmm->CommitJumpAddress
				( addrIfJump, m_xmm->GetNextCodeAddress() ) ;
		//
		LocalSpace *	plsElse = PushLocalSpace() ;
		CompileStatements( cstrm ) ;
		if ( m_flagInvalidFunc )
		{
			ESLVerify( plsElse == PopLocalSpace() ) ;
			FreeLocalSpace( plsElse ) ;
			delete	plsElse ;
			return ;
		}
		ESLVerify( plsElse == PopLocalSpace() ) ;
		FreeLocalSpace( plsElse ) ;
		delete	plsElse ;
		//
		FlushAllLocalVariableCaches() ;
		//
		m_xmm->CommitJumpAddress
			( addrElseJump, m_xmm->GetNextCodeAddress() ) ;
	}
	else
	{
		m_xmm->CommitJumpAddress
			( addrIfJump, m_xmm->GetNextCodeAddress() ) ;
	}
}

// switch 文
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileStatementSwitch
	( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	RSParenthesis *	pPrthSwitch =
			cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	if ( pPrthSwitch == NULL )
	{
		OutputError( L"switch 文評価式がありません" ) ;
		return ;
	}
	RSCodeStream	csSwitch( *pPrthSwitch ) ;
	//
	RSParenthesis *	pPrthCase =
			cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
	if ( pPrthCase == NULL )
	{
		OutputError( L"switch 文ブロックがありません" ) ;
		return ;
	}
	FlushAllLocalVariableCaches() ;
	//
	SObjectArray<RSCodeStream>	arrCaseExpr ;
	SArray<size_t>	arrCaseAddr ;
	size_t			addrDefault = (size_t) -1 ;
	size_t			addrJumpEnter = m_xmm->WriteCodeJump() ;
	//
	RSCodeStream	csCase( *pPrthCase ) ;
	LocalSpace *	plsCase = PushLocalSpace() ;
	plsCase->m_flagLoop = true ;
	//
	while ( !csCase.IsEndOfStream() && !m_flagInvalidFunc )
	{
		if ( csCase.NextControlWord( RSCodeControl::wiCase ) != NULL )
		{
			if ( plsCase->m_ssaVar.GetLength() > 0 )
			{
				OutputError( L"case 文で初期化されない変数があります" ) ;
				break ;
			}
			FlushAllLocalVariableCaches() ;
			arrCaseAddr.Add( m_xmm->GetNextCodeAddress() ) ;
			//
			size_t	iCaseExpr = csCase.GetIndex() ;
			csCase.FindOperator( RSCodeOperator::opSeparator ) ;
			size_t	iEndOfExpr = csCase.GetIndex() ;
			//
			RSCodeStream *	pcsCaseExpr =
				new RSCodeStream
					( *pPrthCase, iCaseExpr, (ssize_t) iEndOfExpr - 1 ) ;
			arrCaseExpr.Add( pcsCaseExpr ) ;
			ESLAssert( arrCaseAddr.GetLength() == arrCaseExpr.GetLength() ) ;
		}
		else if ( csCase.NextControlWord( RSCodeControl::wiDefault ) != NULL )
		{
			if ( plsCase->m_ssaVar.GetLength() > 0 )
			{
				OutputError( L"default 文で初期化されない変数があります" ) ;
				break ;
			}
			FlushAllLocalVariableCaches() ;
			addrDefault = m_xmm->GetNextCodeAddress() ;
		}
		else
		{
			CompileAStatement( csCase ) ;
		}
	}
	ESLVerify( plsCase == PopLocalSpace() ) ;
	FreeLocalSpace( plsCase ) ;
	size_t	addrJumpNext = m_xmm->WriteCodeJump() ;
	//
	// 分岐コード
	//
	m_xmm->CommitJumpAddress( addrJumpEnter, m_xmm->GetNextCodeAddress() ) ;
	//
	for ( size_t i = 0; (i < arrCaseExpr.GetLength()) && !m_flagInvalidFunc; i ++ )
	{
		RSCodeStream *	pcsCase = arrCaseExpr.GetAt( i ) ;
		ESLAssert( pcsCase != NULL ) ;
		csSwitch.SeekIndex( 0 ) ;
		//
		TypeInfo	tiSwitch( this ) ;
		TypeInfo	tiCase( this ) ;
		if ( !CompileExpression( tiSwitch, csSwitch )
			|| !CompileExpression( tiCase, *pcsCase ) )
		{
			FreeTemporaryRegister( tiSwitch ) ;
			FreeTemporaryRegister( tiCase ) ;
			break ;
		}
		if ( !CompileBinaryOperator
			( tiSwitch, tiCase, RSCodeOperator::opEqual ) )
		{
			FreeTemporaryRegister( tiSwitch ) ;
			FreeTemporaryRegister( tiCase ) ;
			break ;
		}
		FreeTemporaryRegister( tiCase ) ;
		if ( !RealizeToBoolean( tiSwitch ) )
		{
			FreeTemporaryRegister( tiSwitch ) ;
			break ;
		}
		WriteBackAllLocalVariables() ;
		m_xmm->WriteCodeCJump( tiSwitch.m_regLoaded, arrCaseAddr.At(i) ) ;
		FreeTemporaryRegister( tiSwitch ) ;
	}
	FlushAllLocalVariableCaches() ;
	//
	if ( addrDefault != (size_t) -1 )
	{
		m_xmm->WriteCodeJump( addrDefault ) ;
	}
	m_xmm->CommitJumpAddress( addrJumpNext, m_xmm->GetNextCodeAddress() ) ;
	//
	// 脱出コード完成
	//
	for ( size_t i = 0; i < plsCase->m_arrBreakRefAddr.GetLength(); i ++ )
	{
		m_xmm->CommitJumpAddress
			( plsCase->m_arrBreakRefAddr.At(i), m_xmm->GetNextCodeAddress() ) ;
	}
	if ( plsCase->m_arrContinueRefAddr.GetLength() > 0 )
	{
		OutputError( L"switch 文ブロックで continue 文は無効です" ) ;
	}
	delete	plsCase ;
}

// case 文
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileStatementCase
	( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	cstrm.FindOperator( RSCodeOperator::opSeparator ) ;
}

// default 文
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileStatementDefault
	( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	cstrm.NextOperator( RSCodeOperator::opSeparator ) ;
}

// break 文
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileStatementBreak
	( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	FlushAllLocalVariableCaches() ;
	//
	LocalSpace *	plsLoop = NULL ;
	for ( size_t i = 0; i < m_stackLocals.GetLength(); i ++ )
	{
		LocalSpace *	pls = m_stackLocals.GetLastAt( i ) ;
		ESLAssert( pls != NULL ) ;
		FreeLocalSpace( pls ) ;
		if ( pls->m_flagLoop )
		{
			plsLoop = pls ;
			break ;
		}
	}
	if ( plsLoop == NULL )
	{
		OutputError( L"break 文に対応する反復文がありません" ) ;
		return ;
	}
	plsLoop->m_arrBreakRefAddr.Add( m_xmm->WriteCodeJump() ) ;
}

// continue 文
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileStatementContinue
	( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	FlushAllLocalVariableCaches() ;
	//
	LocalSpace *	plsLoop = NULL ;
	for ( size_t i = 0; i < m_stackLocals.GetLength(); i ++ )
	{
		LocalSpace *	pls = m_stackLocals.GetLastAt( i ) ;
		ESLAssert( pls != NULL ) ;
		FreeLocalSpace( pls ) ;
		if ( pls->m_flagLoop )
		{
			plsLoop = pls ;
			break ;
		}
	}
	if ( plsLoop == NULL )
	{
		OutputError( L"continue 文に対応する反復文がありません" ) ;
		return ;
	}
	plsLoop->m_arrContinueRefAddr.Add( m_xmm->WriteCodeJump() ) ;
}

// try 文
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileStatementTry
	( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	m_flagInvalidFunc = true ;
}

// throw 文
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileStatementThrow
	( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	m_flagInvalidFunc = true ;
}

// return 文
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileStatementReturn
	( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	if ( cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) != NULL )
	{
		FlushAllLocalVariableCaches() ;
		m_xmm->WriteCodeMoveRegInt64( regAcc, 0 ) ;
		CompileReturn( regAcc ) ;
	}
	else
	{
		TypeInfo	tiExpr( this ) ;
		if ( !CompileExpression( tiExpr, cstrm ) )
		{
			return ;
		}
		if ( m_pPrototype->m_pReturnType != NULL )
		{
			if ( !OperatorCast( tiExpr, m_pPrototype->m_pReturnType ) )
			{
				return ;
			}
		}
		if ( !ConvertToObject( tiExpr ) )
		{
			return ;
		}
		ESLAssert( tiExpr.m_flagLoaded ) ;
		if ( tiExpr.m_regObject < 0 )
		{
			m_xmm->WriteCodePushReg( tiExpr.m_regLoaded ) ;
			m_xmm->WriteCodeSyscall( L"__nrs_add_ref" ) ;
			m_xmm->WriteCodeAddSP( 8 ) ;
		}
		else
		{
			ESLAssert( tiExpr.m_flagRefRosetta ) ;
			if ( tiExpr.m_regObject != tiExpr.m_regLoaded )
			{
				m_xmm->WriteCodeMoveRegReg
					( tiExpr.m_regLoaded, tiExpr.m_regObject ) ;
				FreeTemporaryRegister( tiExpr.m_regObject ) ;
			}
			tiExpr.m_regObject = -1 ;
		}
		FlushAllLocalVariableCaches() ;
		CompileReturn( tiExpr.m_regLoaded ) ;
		FreeTemporaryRegister( tiExpr ) ;
	}
}

// with 文
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileStatementWith
	( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	m_flagInvalidFunc = true ;
}

// synchronized 文
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileStatementSynchronized
	( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	m_flagInvalidFunc = true ;
}

// static|abstract|const|public|protected|private 文
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileStatementAccessModifier
	( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	uint32_t	accMod = 0 ;
	do
	{
		switch ( wiIndex )
		{
		case	RSCodeControl::wiStatic:
			accMod |= RSObject::modifierStatic ;
			break ;
		case	RSCodeControl::wiAbstract:
			accMod |= RSObject::modifierAbstract ;
			break ;
		case	RSCodeControl::wiNative:
			accMod |= RSObject::modifierNative ;
			break ;
		case	RSCodeControl::wiConst:
			accMod |= RSObject::modifierConst ;
			break ;
		case	RSCodeControl::wiSynchronized:
			accMod |= RSObject::modifierSynchronized ;
			break ;
		case	RSCodeControl::wiPublic:
			accMod = (accMod & ~RSObject::accessMask) | RSObject::modifierPublic ;
			break ;
		case	RSCodeControl::wiProtected:
			accMod = (accMod & ~RSObject::accessMask) | RSObject::modifierProtected ;
			break ;
		case	RSCodeControl::wiPrivate:
			accMod = (accMod & ~RSObject::accessMask) | RSObject::modifierPrivate ;
			break ;
		case	RSCodeControl::wiVar:
			CompileDeclareVariable( cstrm, accMod, m_context->GetVariableClass() ) ;
			return ;
		case	RSCodeControl::wiVoid:
			CompileDeclareVariable( cstrm, accMod, NULL ) ;
			return ;
		default:
			if ( (wiIndex >= RSCodeControl::wiFirstBasicType)
				&& (wiIndex <= RSCodeControl::wiLastBasicType) )
			{
				RSClass *	pClass = m_context->GetBasicTypeClass( wiIndex ) ;
				pClass = m_context->ParseClassArrayDecoration( cstrm, pClass ) ;
				CompileDeclareVariable( cstrm, accMod, pClass ) ;
			}
			else
			{
				(this->*m_pfnCompileStatement[wiIndex])( cstrm, wiIndex ) ;
			}
			return ;
		}
		RSCodeControl *	pCode = cstrm.NextControlWord() ;
		if ( pCode == NULL )
		{
			RSClass *	pClass = m_context->ParseClassExpression( cstrm ) ;
			if ( pClass != NULL )
			{
				CompileDeclareVariable( cstrm, accMod, pClass ) ;
			}
			else
			{
				OutputError( L"アクセス修飾子が無効です" ) ;
			}
			return ;
		}
		wiIndex = pCode->m_word ;
	}
	while ( !cstrm.IsEndOfStream() ) ;
}

// var|void|boolean|byte|short|char|int|long|float|double 文
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileStatementVar
	( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	RSClass *	pClass = NULL ;
	if ( (wiIndex >= RSCodeControl::wiFirstBasicType)
				&& (wiIndex <= RSCodeControl::wiLastBasicType) )
	{
		pClass =
			m_context->ParseClassArrayDecoration
				( cstrm, m_context->GetBasicTypeClass( wiIndex ) ) ;
	}
	else if ( wiIndex == RSCodeControl::wiVar )
	{
		pClass = m_context->GetVariableClass() ;
	}
	CompileDeclareVariable( cstrm, 0, pClass ) ;
}

// this|super 文
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileStatementExpression
	( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	ESLAssert( cstrm.GetIndex() > 0 ) ;
	cstrm.SeekIndex( cstrm.GetIndex() - 1 ) ;
	TypeInfo	tiExpr( this ) ;
	CompileExpression
		( tiExpr, cstrm, RSCodeOperator::priorityNothing ) ;
	FreeTemporaryRegister( tiExpr ) ;
	cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) ;
}

// デバッグポイント
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileStatementDebugPoint
	( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	m_flagInvalidFunc = true ;
}

// 不正文
//////////////////////////////////////////////////////////////////////////////
void RSCompiler::CompileStatementInvalid
	( RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	OutputError( L"予約語が無効です" ) ;
}



//////////////////////////////////////////////////////////////////////////////
// ポインタオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSPointerMapper, Object )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSPointerMapper::RSPointerMapper( void )
{
	m_pBuf = NULL ;
	m_iOffset = 0 ;
	m_nLimit = 0 ;
	m_nRef = 1 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSPointerMapper::~RSPointerMapper( void )
{
	RSObject::ReleaseRef( m_pBuf ) ;
	m_pBuf = NULL ;
}

// メモリマッピング
//////////////////////////////////////////////////////////////////////////////
ECSSakura2Processor::LinearAddressCache *
	RSPointerMapper::GetSegmentBuffer
		( ECSSakura2Processor::LinearAddressCache & seg )
{
	if ( m_pBuf != NULL )
	{
		seg.baseOffset = 0 ;
		seg.limitSegment = (DWORD) m_nLimit ;
		seg.pbytBuffer = m_pBuf->m_ptrBuf + m_iOffset ;
		return	&seg ;
	}
	return	NULL ;
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSPointerMapper::GetTypeName( void ) const
{
	return	L"PointerMapper" ;
}



//////////////////////////////////////////////////////////////////////////////
// システム関数
//////////////////////////////////////////////////////////////////////////////

#if	!defined(ENTISGLS4_DLL_IMPORT)

// void __nrs_add_ref( void * obj ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_add_ref, context, arg)
{
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject::AddRef( pObj ) ;
	return	NULL ;
}

// void __nrs_release_ref( void * obj ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_release_ref, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	pContext->ReleaseObjectRef( (RSObject*) arg[0].i ) ;
	return	NULL ;
}

// void __nrs_add_ptr_ref( void * ptr ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_add_ptr_ref, context, arg)
{
	RSPointerMapper *	pPtrMap =
		ESLTypeCast<RSPointerMapper>
			( context->m_pSakura2VM->AtomicObjectFromAddress( arg[0].h32 ) ) ;
	if ( pPtrMap != NULL )
	{
		AtomicAdd( &(pPtrMap->m_nRef), 1 ) ;
	}
	return	NULL ;
}

// void __nrs_release_ptr_ref( void * ptr ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_release_ptr_ref, context, arg)
{
	RSPointerMapper *	pPtrMap =
		ESLTypeCast<RSPointerMapper>
			( context->m_pSakura2VM->AtomicObjectFromAddress( arg[0].h32 ) ) ;
	if ( pPtrMap != NULL )
	{
		if ( AtomicSub( &(pPtrMap->m_nRef), 1 ) == 0 )
		{
			context->m_pSakura2VM->FreeHeapObjectAddress( arg[0].i, context ) ;
		}
	}
	return	NULL ;
}

// void * __nrs_realize_pointer( void * obj ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_realize_pointer, context, arg)
{
	RSObject*	pObj = (RSObject*) arg[0].i ;
	RSTypedArrayPointer *	pPtrObj =
			ESLTypeCast<RSTypedArrayPointer>
				( (pObj != NULL) ? pObj->GetEntityObject() : NULL ) ;
	if ( pPtrObj == NULL )
	{
		context->m_regset[regAcc].i = 0 ;
		return	NULL ;
	}
	RSPointerMapper *	pPtrMap = new RSPointerMapper ;
	pPtrMap->m_pBuf = pPtrObj->m_pRefBuffer ;
	pPtrMap->m_iOffset = pPtrObj->m_iOffset ;
	pPtrMap->m_nLimit = pPtrObj->m_nLimit ;
	RSObject::AddRef( pPtrMap->m_pBuf ) ;
	//
	ECSSakura2Processor::AssertLock() ;
	context->m_regset[0].i =
		context->m_pSakura2VM->AllocateHeapObjectAddress( pPtrMap ) ;
	ECSSakura2Processor::AssertUnlock() ;
	return	NULL ;
}

// int64 __nrs_realize_integer( void * obj ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_realize_integer, context, arg)
{
	RSObject *	pObj = (RSObject*) arg[0].i ;
	int64_t	num = 0 ;
	if ( (pObj == NULL) || !pObj->AsInteger( num ) )
	{
		RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
		pContext->ThrowExceptionError( L"整数へ変換できません" ) ;
	}
	context->m_regset[regAcc].i = num ;
	return	NULL ;
}

// double __nrs_realize_real_number( void * obj ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_realize_real_number, context, arg)
{
	RSObject *	pObj = (RSObject*) arg[0].i ;
	double	num = 0 ;
	if ( (pObj == NULL) || !pObj->AsRealNumber( num ) )
	{
		RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
		pContext->ThrowExceptionError( L"数値へ変換できません" ) ;
	}
	context->m_regset[regAcc].f = num ;
	return	NULL ;
}

// void * __nrs_bound_pointer( void * ptr, int bytes ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_bound_pointer, context, arg)
{
	RSPointerMapper *	pPtrMap =
		ESLTypeCast<RSPointerMapper>
			( context->m_pSakura2VM->AtomicObjectFromAddress( arg[0].h32 ) ) ;
	if ( pPtrMap == NULL )
	{
		context->m_regset[regAcc].i = 0 ;
		return	NULL ;
	}
	size_t	nOffset = (size_t) arg[0].l32 ;
	size_t	nBytes = (size_t) arg[1].i ;
	if ( pPtrMap->m_nLimit < nOffset + nBytes )
	{
		if ( pPtrMap->m_nLimit < nOffset )
		{
			context->m_regset[regAcc].i = 0 ;
			return	NULL ;
		}
		nBytes = pPtrMap->m_nLimit - nOffset ;
	}
	RSPointerMapper *	pPtrMap2 = new RSPointerMapper ;
	pPtrMap2->m_pBuf = pPtrMap->m_pBuf ;
	pPtrMap2->m_iOffset = pPtrMap->m_iOffset + nOffset ;
	pPtrMap2->m_nLimit = nBytes ;
	RSObject::AddRef( pPtrMap2->m_pBuf ) ;
	//
	ECSSakura2Processor::AssertLock() ;
	context->m_regset[0].i =
		context->m_pSakura2VM->AllocateHeapObjectAddress( pPtrMap2 ) ;
	ECSSakura2Processor::AssertUnlock() ;
	return	NULL ;
}

// void * __nrs_create_pointer( int bytes ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_create_pointer, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSArrayBuffer *	pBuf =
			new RSArrayBuffer( pContext->GetArrayBufferClass() ) ;
	pBuf->AllocateBuffer( (size_t) arg[0].i ) ;
	//
	RSPointerMapper *	pPtrMap = new RSPointerMapper ;
	pPtrMap->m_pBuf = pBuf ;
	pPtrMap->m_iOffset = 0 ;
	pPtrMap->m_nLimit = pBuf->m_lenBuf ;
	//
	ECSSakura2Processor::AssertLock() ;
	context->m_regset[0].i =
		context->m_pSakura2VM->AllocateHeapObjectAddress( pPtrMap ) ;
	ECSSakura2Processor::AssertUnlock() ;
	return	NULL ;
}

// void * __nrs_new_Array( void * cls, int limit ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_new_Array, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	context->m_regset[0].i =
		(ulong_ptr_t) pContext->new_Array
							( (size_t) arg[1].i, (RSClass*) arg[0].i ) ;
	return	NULL ;
}

// void * __nrs_new_String( const char * pszInit ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_new_String, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	const wchar_t *	pwszInit = NULL ;
	if ( arg[0].i != 0 )
	{
		pwszInit = (const wchar_t*)
						context->AtomicTranslateAddress( arg[0].i, 2 ) ;
	}
	context->m_regset[0].i =
			(ulong_ptr_t) pContext->new_String( pwszInit ) ;
	return	NULL ;
}

// void * __nrs_new_Object( void * cls, int nArgCount, ... ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_new_Object, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSClass *	pClass = (RSClass*) arg[0].i ;
	size_t		nArgCount = (size_t) arg[1].i ;
	RSObject *	pArgObj = pContext->new_Array() ;
	for ( size_t i = 0; i < nArgCount; i ++ )
	{
		RSObject *	pArg = (RSObject*) arg[i + 2].i ;
		RSObject::AddRef( pArg ) ;
		RSObject::ReleaseRef
			( pArgObj->SetElementAt( *pContext, (int) i, pArg ) ) ;
	}
	context->m_regset[0].i =
		(ulong_ptr_t) pClass->NewInstance( *pContext, pArgObj ) ;
	pContext->ReleaseObjectRef( pArgObj ) ;
	return	NULL ;
}

// void * __nrs_new_Pointer( void * cls, int nArgCount, ... ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_new_Pointer, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSClass *	pClass = (RSClass*) arg[0].i ;
	size_t		nArgCount = (size_t) arg[1].i ;
	RSObject *	pArgObj = pContext->new_Array() ;
	for ( size_t i = 0; i < nArgCount; i ++ )
	{
		RSObject *	pArg = (RSObject*) arg[i + 2].i ;
		RSObject::AddRef( pArg ) ;
		RSObject::ReleaseRef
			( pArgObj->SetElementAt( *pContext, (int) i, pArg ) ) ;
	}
	RSObject *	pObj = pClass->NewInstance( *pContext, pArgObj ) ;
	RSTypedArrayPointer *
				pPtrObj = ESLTypeCast<RSTypedArrayPointer>( pObj ) ;
	if ( pPtrObj != NULL )
	{
		RSArrayBuffer *	pBuf = pPtrObj->m_pRefBuffer ;
		RSObject::AddRef( pBuf ) ;
		//
		RSPointerMapper *	pPtrMap = new RSPointerMapper ;
		pPtrMap->m_pBuf = pBuf ;
		pPtrMap->m_iOffset = 0 ;
		pPtrMap->m_nLimit = pBuf->m_lenBuf ;
		//
		ECSSakura2Processor::AssertLock() ;
		context->m_regset[0].i =
			context->m_pSakura2VM->AllocateHeapObjectAddress( pPtrMap ) ;
		ECSSakura2Processor::AssertUnlock() ;
	}
	else
	{
		if ( !pContext->IsException() )
		{
			pContext->ThrowExceptionError
				( L"バッファポインタの構築に失敗しました" ) ;
		}
		context->m_regset[0].i = 0 ;
	}
	pContext->ReleaseObjectRef( pArgObj ) ;
	pContext->ReleaseObjectRef( pObj ) ;
	return	NULL ;
}

// void * __nrs_new_pointer( void * cls, void * ptr ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_new_pointer, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSClass *	pClass = (RSClass*) arg[0].i ;
	RSObject *	pObj = pClass->NewVariable( *pContext ) ;
	RSTypedArrayPointer *
			pPtrObj = ESLTypeCast<RSTypedArrayPointer>( pObj ) ;
	if ( pPtrObj == NULL )
	{
		if ( !pContext->IsException() )
		{
			pContext->ThrowExceptionError
				( L"ポインタオブジェクトの構築に失敗しました" ) ;
		}
		context->m_regset[0].i = 0 ;
		RSObject::ReleaseRef( pObj ) ;
		return	NULL ;
	}
	RSPointerMapper *	pPtrMap =
		ESLTypeCast<RSPointerMapper>
			( context->m_pSakura2VM->AtomicObjectFromAddress( arg[1].h32 ) ) ;
	if ( pPtrMap != NULL )
	{
		RSObject::ReleaseRef( pPtrObj->m_pRefBuffer ) ;
		pPtrObj->m_pRefBuffer = pPtrMap->m_pBuf ;
		RSObject::AddRef( pPtrObj->m_pRefBuffer ) ;
		pPtrObj->m_iOffset = pPtrMap->m_iOffset + arg[1].l32 ;
		pPtrObj->m_nLimit = 0 ;
		if ( pPtrMap->m_nLimit > arg[1].l32 )
		{
			pPtrObj->m_nLimit = pPtrMap->m_nLimit - arg[1].l32 ;
		}
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pPtrObj ;
	return	NULL ;
}

// void * __nrs_new_integer( int num ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_new_integer, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	context->m_regset[regAcc].i =
			(ulong_ptr_t) pContext->new_Integer( arg[0].i ) ;
	return	NULL ;
}

// void * __nrs_new_real_number( double num ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_new_real_number, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	context->m_regset[regAcc].i =
			(ulong_ptr_t) pContext->new_Number( arg[0].f ) ;
	return	NULL ;
}

// void * __nrs_cast_object( void * cls, void * obj, int castMethod ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_cast_object, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSClass *	pClass = (RSClass*) arg[0].i ;
	RSObject *	pObj = (RSObject*) arg[1].i ;
	RSObject *	pCast = NULL ;
	if ( pObj != NULL )
	{
		pCast = pClass->CastInstance
				( *pContext, pObj, (RSClass::CastMethod) arg[2].i ) ;
		if ( pCast == NULL )
		{
			pContext->ThrowExceptionError
				( SString(pObj->GetTypeName())
					+ L" から " + pClass->GetRSClassName()
					+ L" へキャストできません" ) ;
		}
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pCast ;
	return	NULL ;
}

// void * __nrs_clone_object( void * obj ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_clone_object, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	if ( pObj == NULL )
	{
		context->m_regset[regAcc].i = 0 ;
		return	NULL ;
	}
	context->m_regset[regAcc].i =
				(ulong_ptr_t) pObj->CloneObject( *pContext ) ;
	return	NULL ;
}

// void * __nrs_call_function_virtual
//	( void * strFuncName, int iVirtual, void * objThis, int nArgCount, ... ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_call_function_virtual, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	const wchar_t *
				pwszFuncName = (const wchar_t*) arg[0].i ;
	size_t		iVirtual = (size_t) arg[1].i ;
	RSObject *	pObjThis = (RSObject*) arg[2].i ;
	size_t		nArgCount = (size_t) arg[3].i ;
	RSObject*	pArgs[16] ;
	RSObject**	ppArgs = &pArgs[0] ;
	SPointerArray<RSObject>	arrArgs ;
	if ( nArgCount < 16 )
	{
		for ( size_t i = 0; i < nArgCount; i ++ )
		{
			pArgs[i] = (RSObject*) arg[i + 4].i ;
		}
	}
	else
	{
		for ( size_t i = 0; i < nArgCount; i ++ )
		{
			arrArgs.Add( (RSObject*) arg[i + 4].i ) ;
		}
		ppArgs = arrArgs.GetArray() ;
	}
	RSClass *	pClass = NULL ;
	RSObject *	pMember = NULL ;
	if ( pObjThis != NULL )
	{
		pClass = pObjThis->GetEntityClass() ;
		if ( pClass != NULL )
		{
			pMember = pClass->GetVirtualMemberAs( *pContext, pwszFuncName ) ;
		}
	}
	RSFunctionObject *	pFuncObj = ESLTypeCast<RSFunctionObject>( pMember ) ;
	RSFunctionPrototype *	pProto = NULL ;
	if ( pFuncObj != NULL )
	{
		pProto = pFuncObj->m_arrPrototypes.GetAt( iVirtual ) ;
	}
	if ( pProto != NULL )
	{
		RSObject *	pObjRet =
			pContext->CallFunction( pProto, pObjThis, ppArgs, nArgCount, true ) ;
		context->m_regset[regAcc].i = (ulong_ptr_t) pObjRet ;
	}
	else
	{
		pContext->ThrowExceptionError( L"関数が見つかりません" ) ;
		context->m_regset[regAcc].i = 0 ;
	}
	pContext->ReleaseObjectRef( pMember );
	arrArgs.FinishArray() ;
	return	NULL ;
}

// void * __nrs_call_function_obj
//	( void * objFunc, void * objThis, int nArgCount, ... ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_call_function_obj, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObjFunc = (RSObject*) arg[0].i ;
	RSObject *	pObjThis = (RSObject*) arg[1].i ;
	size_t		nArgCount = (size_t) arg[2].i ;
	RSObject*	pArgs[16] ;
	RSObject**	ppArgs = &pArgs[0] ;
	SPointerArray<RSObject>	arrArgs ;
	if ( nArgCount < 16 )
	{
		for ( size_t i = 0; i < nArgCount; i ++ )
		{
			pArgs[i] = (RSObject*) arg[i + 3].i ;
		}
	}
	else
	{
		for ( size_t i = 0; i < nArgCount; i ++ )
		{
			arrArgs.Add( (RSObject*) arg[i + 3].i ) ;
		}
		ppArgs = arrArgs.GetArray() ;
	}
	if ( pObjFunc != nullptr )
	{
		pObjFunc = pObjFunc->GetEntityObject() ;
	}
	RSFunctionObject *	pFuncObj = ESLTypeCast<RSFunctionObject>( pObjFunc ) ;
	if ( pFuncObj != NULL )
	{
		RSObject *	pObjRet =
			pContext->CallFunction
				( *pFuncObj, pObjThis, ppArgs, nArgCount, true ) ;
		context->m_regset[regAcc].i = (ulong_ptr_t) pObjRet ;
	}
	else
	{
		pContext->ThrowExceptionError( L"関数ではありません" ) ;
		context->m_regset[regAcc].i = 0 ;
	}
	arrArgs.FinishArray() ;
	return	NULL ;
}

// void * __nrs_call_function_proto
//	( void * protoFunc, void * objThis, int nArgCount, ... ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_call_function_proto, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSFunctionPrototype *
				pProto = (RSFunctionPrototype*) arg[0].i ;
	RSObject *	pObjThis = (RSObject*) arg[1].i ;
	size_t		nArgCount = (size_t) arg[2].i ;
	RSObject*	pArgs[16] ;
	RSObject**	ppArgs = &pArgs[0] ;
	SPointerArray<RSObject>	arrArgs ;
	if ( nArgCount < 16 )
	{
		for ( size_t i = 0; i < nArgCount; i ++ )
		{
			pArgs[i] = (RSObject*) arg[i + 3].i ;
		}
	}
	else
	{
		for ( size_t i = 0; i < nArgCount; i ++ )
		{
			arrArgs.Add( (RSObject*) arg[i + 3].i ) ;
		}
		ppArgs = arrArgs.GetArray() ;
	}
	RSObject *	pObjRet =
		pContext->CallFunction( pProto, pObjThis, ppArgs, nArgCount, false ) ;
	context->m_regset[regAcc].i = (ulong_ptr_t) pObjRet ;
	arrArgs.FinishArray() ;
	return	NULL ;
}

// void __nrs_set_currrent_position( void * prth, int index ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_set_currrent_position, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	pContext->SetCurrentParenthesis( (const RSParenthesis*) arg[0].i ) ;
	pContext->SetCurrentStatementIndex( (size_t) arg[1].i ) ;
	return	NULL ;
}

// void __nrs_set_exception_position( void * prth, int index ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_set_exception_position, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	pContext->SetExceptionParenthesis( (const RSParenthesis*) arg[0].i ) ;
	pContext->SetExceptionStatementIndex( (size_t) arg[1].i ) ;
	return	NULL ;
}

// bool __nrs_is_exception() ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_is_exception, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	context->m_regset[regAcc].i = pContext->IsException() ? -1 : 0 ;
	return	NULL ;
}

// void __nrs_throw_exception( void * strMsg, void * strClass ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_throw_exception, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	const wchar_t *	pwszMsg = (const wchar_t *) arg[0].i ;
	const wchar_t *	pwszClass = (const wchar_t *) arg[1].i ;
	pContext->ThrowExceptionError( pwszMsg, pwszClass ) ;
	return	NULL ;
}

// void * __nrs_get_member_as( void * obj, void * strName ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_get_member_as, context, arg)
{
	RSContext *		pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *		pObj = (RSObject*) arg[0].i ;
	const wchar_t *	pwszName = (const wchar_t *) arg[1].i ;
	RSObject *		pMember = nullptr ;
	if ( pObj != nullptr )
	{
		pMember = pObj->GetMemberAs( *pContext, pwszName ) ;
	}
	if ( pMember == nullptr )
	{
		if ( (pObj == nullptr)
			|| (pObj->GetEntityObject() == nullptr) )
		{
			pContext->ThrowExceptionError
				( SString(L"null ポインタに対して ")
						+ pwszName + L" メンバを参照しています",
					L"NullPointerException" ) ;
		}
		else
		{
			pContext->ThrowExceptionError
				( SString(pwszName) + L" メンバが見つかりません" ) ;
		}
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pMember ;
	return	NULL ;
}

// void * __nrs_get_element_as( void * obj, void * objIndex ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_get_element_as, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObjIndex = (RSObject*) arg[1].i ;
	if ( pObjIndex == NULL )
	{
		pContext->ThrowExceptionError( L"要素の参照で指標が null です" ) ;
		context->m_regset[regAcc].i = 0 ;
		return	NULL ;
	}
	if ( pObj == NULL )
	{
		pContext->ThrowExceptionError( L"null への要素参照です" ) ;
		context->m_regset[regAcc].i = 0 ;
		return	NULL ;
	}
	RSObject *	pElementObj = NULL ;
	if ( pObjIndex->IsIntegerType() )
	{
		int64_t	index = 0 ;
		ESLVerify( pObjIndex->AsInteger( index ) ) ;
		if ( (index < 0) || (index > 0x7FFFFFFF)
			|| (index > (int64_t) pObj->GetElementLimit()) )
		{
			pContext->ThrowExceptionError( L"指標が範囲を超えています" ) ;
			context->m_regset[regAcc].i = 0 ;
			return	NULL ;
		}
		pElementObj = pObj->GetElementAt( *pContext, (int) index ) ;
	}
	else
	{
		SString	strElement ;
		if ( pObjIndex->AsString( strElement ) )
		{
			pElementObj = pObj->GetMemberAs( *pContext, strElement ) ;
		}
		else
		{
			pContext->ThrowExceptionError( L"要素の参照で指標型が不正です" ) ;
			context->m_regset[regAcc].i = 0 ;
			return	NULL ;
		}
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pElementObj ;
	return	NULL ;
}

// bool __nrs_operator_boolean( void * obj ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_boolean, context, arg)
{
	RSObject *	pObj = (RSObject*) arg[0].i ;
	if ( pObj != NULL )
	{
		context->m_regset[regAcc].i = pObj->AsBoolean() ? -1 : 0 ;
	}
	else
	{
		context->m_regset[regAcc].i = 0 ;
	}
	return	NULL ;
}

// void * __nrs_operator_plus( void * obj ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_plus, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	if ( pObj != NULL )
	{
		pObj = pObj->OperatorPlus( *pContext ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_negate( void * obj ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_negate, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	if ( pObj != NULL )
	{
		pObj = pObj->OperatorNegate( *pContext ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_bit_not( void * obj ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_bit_not, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	if ( pObj != NULL )
	{
		pObj = pObj->OperatorBitNot( *pContext ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_increment( void * obj ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_increment, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	if ( pObj != NULL )
	{
		pObj = pObj->OperatorIncrement( *pContext ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_decrement( void * obj ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_decrement, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	if ( pObj != NULL )
	{
		pObj = pObj->OperatorDecrement( *pContext ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_logical_not( void * obj ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_logical_not, context, arg)
{
	RSObject *	pObj = (RSObject*) arg[0].i ;
	if ( pObj != NULL )
	{
		context->m_regset[regAcc].i = pObj->AsBoolean() ? 0 : -1 ;
	}
	else
	{
		context->m_regset[regAcc].i = -1 ;
	}
	return	NULL ;
}

// void * __nrs_operator_add( void * obj, void * obj2 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_add, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
		RSObject::AddRef( pObj2 ) ;
		pObj = pObj->OperatorAdd( *pContext, pObjRight ) ;
		pContext->ReleaseObjectRef( pObjRight ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_sub( void * obj, void * obj2 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_sub, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
		RSObject::AddRef( pObj2 ) ;
		pObj = pObj->OperatorSub( *pContext, pObjRight ) ;
		pContext->ReleaseObjectRef( pObjRight ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_mul( void * obj, void * obj2 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_mul, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
		RSObject::AddRef( pObj2 ) ;
		pObj = pObj->OperatorMul( *pContext, pObjRight ) ;
		pContext->ReleaseObjectRef( pObjRight ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_div( void * obj, void * obj2 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_div, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
		RSObject::AddRef( pObj2 ) ;
		pObj = pObj->OperatorDiv( *pContext, pObjRight ) ;
		pContext->ReleaseObjectRef( pObjRight ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_mod( void * obj, void * obj2 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_mod, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
		RSObject::AddRef( pObj2 ) ;
		pObj = pObj->OperatorMod( *pContext, pObjRight ) ;
		pContext->ReleaseObjectRef( pObjRight ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_bit_and( void * obj, void * obj2 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_bit_and, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
		RSObject::AddRef( pObj2 ) ;
		pObj = pObj->OperatorBitAnd( *pContext, pObjRight ) ;
		pContext->ReleaseObjectRef( pObjRight ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_bit_or( void * obj, void * obj2 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_bit_or, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
		RSObject::AddRef( pObj2 ) ;
		pObj = pObj->OperatorBitOr( *pContext, pObjRight ) ;
		pContext->ReleaseObjectRef( pObjRight ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_bit_xor( void * obj, void * obj2 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_bit_xor, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
		RSObject::AddRef( pObj2 ) ;
		pObj = pObj->OperatorBitXor( *pContext, pObjRight ) ;
		pContext->ReleaseObjectRef( pObjRight ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_shift_right( void * obj, void * obj2 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_shift_right, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
		RSObject::AddRef( pObj2 ) ;
		pObj = pObj->OperatorBitShiftRight( *pContext, pObjRight ) ;
		pContext->ReleaseObjectRef( pObjRight ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_shift_left( void * obj, void * obj2 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_shift_left, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
		RSObject::AddRef( pObj2 ) ;
		pObj = pObj->OperatorShiftLeft( *pContext, pObjRight ) ;
		pContext->ReleaseObjectRef( pObjRight ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_shift_right_arithmetic( void * obj, void * obj2 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_shift_right_arithmetic, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
		RSObject::AddRef( pObj2 ) ;
		pObj = pObj->OperatorShiftRight( *pContext, pObjRight ) ;
		pContext->ReleaseObjectRef( pObjRight ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_equal( void * obj, void * obj2 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_equal, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
		RSObject::AddRef( pObj2 ) ;
		pObj = pObj->OperatorCompareEQ( *pContext, pObjRight ) ;
		pContext->ReleaseObjectRef( pObjRight ) ;
	}
	else
	{
		pObj = pContext->new_Boolean
			( (pObj2 == NULL) || (pObj2->GetEntityObject() == NULL) ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_not_equal( void * obj, void * obj2 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_not_equal, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
		RSObject::AddRef( pObj2 ) ;
		pObj = pObj->OperatorCompareNE( *pContext, pObjRight ) ;
		pContext->ReleaseObjectRef( pObjRight ) ;
	}
	else
	{
		pObj = pContext->new_Boolean
			( (pObj2!= NULL) && (pObj2->GetEntityObject() != NULL) ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_less_equal( void * obj, void * obj2 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_less_equal, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
		RSObject::AddRef( pObj2 ) ;
		pObj = pObj->OperatorCompareLE( *pContext, pObjRight ) ;
		pContext->ReleaseObjectRef( pObjRight ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_less_than( void * obj, void * obj2 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_less_than, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
		RSObject::AddRef( pObj2 ) ;
		pObj = pObj->OperatorCompareLT( *pContext, pObjRight ) ;
		pContext->ReleaseObjectRef( pObjRight ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_grater_equal( void * obj, void * obj2 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_grater_equal, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
		RSObject::AddRef( pObj2 ) ;
		pObj = pObj->OperatorCompareGE( *pContext, pObjRight ) ;
		pContext->ReleaseObjectRef( pObjRight ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_grater_than( void * obj, void * obj2 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_grater_than, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
		RSObject::AddRef( pObj2 ) ;
		pObj = pObj->OperatorCompareGT( *pContext, pObjRight ) ;
		pContext->ReleaseObjectRef( pObjRight ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_pointer_equal( void * obj, void * obj2 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_pointer_equal, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	pObj = pObj ? pObj->GetEntityObject() : NULL ;
	pObj2 = pObj2 ? pObj2->GetEntityObject() : NULL ;
	context->m_regset[regAcc].i =
		(ulong_ptr_t) pContext->new_Boolean( pObj == pObj2 ) ;
	return	NULL ;
}

// void * __nrs_operator_pointer_not_equal( void * obj, void * obj2 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_pointer_not_equal, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	pObj = pObj ? pObj->GetEntityObject() : NULL ;
	pObj2 = pObj2 ? pObj2->GetEntityObject() : NULL ;
	context->m_regset[regAcc].i =
		(ulong_ptr_t) pContext->new_Boolean( pObj != pObj2 ) ;
	return	NULL ;
}

// void * __nrs_operator_logical_and( void * obj, void * obj2 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_logical_and, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	context->m_regset[regAcc].i =
		(ulong_ptr_t) pContext->new_Boolean
			( pObj && pObj2 && pObj->AsBoolean() && pObj2->AsBoolean() ) ;
	return	NULL ;
}

// void * __nrs_operator_logical_or( void * obj, void * obj2 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_logical_or, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	context->m_regset[regAcc].i =
		(ulong_ptr_t) pContext->new_Boolean
			( (pObj && pObj->AsBoolean()) || (pObj2 && pObj2->AsBoolean()) ) ;
	return	NULL ;
}

// void * __nrs_operator_move( void * obj, void * src ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_move, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		if ( pObj->GetModifiers() & RSObject::modifierConst )
		{
			pContext->ThrowExceptionError( L"const オブジェクトへの代入です" ) ;
		}
		else
		{
			RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
			RSObject::AddRef( pObj2 ) ;
			pObj = pObj->OperatorMove( *pContext, pObjRight ) ;
			pContext->ReleaseObjectRef( pObjRight ) ;
		}
	}
	else
	{
		pContext->ThrowExceptionError( L"null への代入です" ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_move_add( void * obj, void * src ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_move_add, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		if ( pObj->GetModifiers() & RSObject::modifierConst )
		{
			pContext->ThrowExceptionError( L"const オブジェクトへの代入です" ) ;
		}
		else
		{
			RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
			RSObject::AddRef( pObj2 ) ;
			pObj = pObj->OperatorMoveAdd( *pContext, pObjRight ) ;
			pContext->ReleaseObjectRef( pObjRight ) ;
		}
	}
	else
	{
		pContext->ThrowExceptionError( L"null への代入です" ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_move_sub( void * obj, void * src ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_move_sub, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		if ( pObj->GetModifiers() & RSObject::modifierConst )
		{
			pContext->ThrowExceptionError( L"const オブジェクトへの代入です" ) ;
		}
		else
		{
			RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
			RSObject::AddRef( pObj2 ) ;
			pObj = pObj->OperatorMoveSub( *pContext, pObjRight ) ;
			pContext->ReleaseObjectRef( pObjRight ) ;
		}
	}
	else
	{
		pContext->ThrowExceptionError( L"null への代入です" ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_move_mul( void * obj, void * src ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_move_mul, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		if ( pObj->GetModifiers() & RSObject::modifierConst )
		{
			pContext->ThrowExceptionError( L"const オブジェクトへの代入です" ) ;
		}
		else
		{
			RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
			RSObject::AddRef( pObj2 ) ;
			pObj = pObj->OperatorMoveMul( *pContext, pObjRight ) ;
			pContext->ReleaseObjectRef( pObjRight ) ;
		}
	}
	else
	{
		pContext->ThrowExceptionError( L"null への代入です" ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_move_div( void * obj, void * src ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_move_div, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		if ( pObj->GetModifiers() & RSObject::modifierConst )
		{
			pContext->ThrowExceptionError( L"const オブジェクトへの代入です" ) ;
		}
		else
		{
			RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
			RSObject::AddRef( pObj2 ) ;
			pObj = pObj->OperatorMoveDiv( *pContext, pObjRight ) ;
			pContext->ReleaseObjectRef( pObjRight ) ;
		}
	}
	else
	{
		pContext->ThrowExceptionError( L"null への代入です" ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_move_mod( void * obj, void * src ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_move_mod, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		if ( pObj->GetModifiers() & RSObject::modifierConst )
		{
			pContext->ThrowExceptionError( L"const オブジェクトへの代入です" ) ;
		}
		else
		{
			RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
			RSObject::AddRef( pObj2 ) ;
			pObj = pObj->OperatorMoveMod( *pContext, pObjRight ) ;
			pContext->ReleaseObjectRef( pObjRight ) ;
		}
	}
	else
	{
		pContext->ThrowExceptionError( L"null への代入です" ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_move_bit_and( void * obj, void * src ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_move_bit_and, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		if ( pObj->GetModifiers() & RSObject::modifierConst )
		{
			pContext->ThrowExceptionError( L"const オブジェクトへの代入です" ) ;
		}
		else
		{
			RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
			RSObject::AddRef( pObj2 ) ;
			pObj = pObj->OperatorMoveBitAnd( *pContext, pObjRight ) ;
			pContext->ReleaseObjectRef( pObjRight ) ;
		}
	}
	else
	{
		pContext->ThrowExceptionError( L"null への代入です" ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_move_bit_or( void * obj, void * src ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_move_bit_or, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		if ( pObj->GetModifiers() & RSObject::modifierConst )
		{
			pContext->ThrowExceptionError( L"const オブジェクトへの代入です" ) ;
		}
		else
		{
			RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
			RSObject::AddRef( pObj2 ) ;
			pObj = pObj->OperatorMoveBitOr( *pContext, pObjRight ) ;
			pContext->ReleaseObjectRef( pObjRight ) ;
		}
	}
	else
	{
		pContext->ThrowExceptionError( L"null への代入です" ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_move_bit_xor( void * obj, void * src ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_move_bit_xor, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		if ( pObj->GetModifiers() & RSObject::modifierConst )
		{
			pContext->ThrowExceptionError( L"const オブジェクトへの代入です" ) ;
		}
		else
		{
			RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
			RSObject::AddRef( pObj2 ) ;
			pObj = pObj->OperatorMoveBitXor( *pContext, pObjRight ) ;
			pContext->ReleaseObjectRef( pObjRight ) ;
		}
	}
	else
	{
		pContext->ThrowExceptionError( L"null への代入です" ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_move_shift_right( void * obj, void * src ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_move_shift_right, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		if ( pObj->GetModifiers() & RSObject::modifierConst )
		{
			pContext->ThrowExceptionError( L"const オブジェクトへの代入です" ) ;
		}
		else
		{
			RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
			RSObject::AddRef( pObj2 ) ;
			pObj = pObj->OperatorMoveBitShiftRight( *pContext, pObjRight ) ;
			pContext->ReleaseObjectRef( pObjRight ) ;
		}
	}
	else
	{
		pContext->ThrowExceptionError( L"null への代入です" ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_move_shift_left( void * obj, void * src ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_move_shift_left, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		if ( pObj->GetModifiers() & RSObject::modifierConst )
		{
			pContext->ThrowExceptionError( L"const オブジェクトへの代入です" ) ;
		}
		else
		{
			RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
			RSObject::AddRef( pObj2 ) ;
			pObj = pObj->OperatorMoveShiftLeft( *pContext, pObjRight ) ;
			pContext->ReleaseObjectRef( pObjRight ) ;
		}
	}
	else
	{
		pContext->ThrowExceptionError( L"null への代入です" ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// void * __nrs_operator_move_shift_left_arithmetic( void * obj, void * src ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_move_shift_left_arithmetic, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pObj != NULL )
	{
		if ( pObj->GetModifiers() & RSObject::modifierConst )
		{
			pContext->ThrowExceptionError( L"const オブジェクトへの代入です" ) ;
		}
		else
		{
			RSObject *	pObjRight = pObj2 ? pObj2 : pContext->new_Pointer( NULL ) ;
			RSObject::AddRef( pObj2 ) ;
			pObj = pObj->OperatorMoveShiftRight( *pContext, pObjRight ) ;
			pContext->ReleaseObjectRef( pObjRight ) ;
		}
	}
	else
	{
		pContext->ThrowExceptionError( L"null への代入です" ) ;
	}
	context->m_regset[regAcc].i = (ulong_ptr_t) pObj ;
	return	NULL ;
}

// bool __nrs_operator_instance_of( void * obj, void * cls ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_operator_instance_of, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSObject *	pObj = (RSObject*) arg[0].i ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	RSObject *	pObjRight = pObj2 ? pObj2->GetEntityObject() : NULL ;
	if ( (pObjRight != NULL)
		&& (pObjRight->GetBasicType() == RSObject::typeClass) )
	{
		context->m_regset[regAcc].i =
			(pObj && (pObj->InstanceOf( (RSClass*) pObjRight ) != NULL)) ? -1 : 0 ;
	}
	else
	{
		pContext->ThrowExceptionError( L"instanceof の右辺が型指定ではありません" ) ;
		context->m_regset[regAcc].i = 0 ;
	}
	return	NULL ;
}

// bool __nrs_ptr_operator_equal( void * ptr, void * ptr2 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_ptr_operator_equal, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSTypedArrayPointer *
				pPtrObj = ESLTypeCast<RSTypedArrayPointer>( (RSObject*) arg[0].i ) ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pPtrObj != NULL )
	{
		context->m_regset[regAcc].i = pPtrObj->IsEqualObject( pObj2 ) ? -1 : 0 ;
	}
	else
	{
		context->m_regset[regAcc].i = (pPtrObj == pObj2) ? -1 : 0 ;
	}
	return	NULL ;
}

// bool __nrs_ptr_operator_not_equal( void * ptr, void * ptr2 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(__nrs_ptr_operator_not_equal, context, arg)
{
	RSContext *	pContext = (RSContext*) context->m_ptrReserved[0] ;
	RSTypedArrayPointer *
				pPtrObj = ESLTypeCast<RSTypedArrayPointer>( (RSObject*) arg[0].i ) ;
	RSObject *	pObj2 = (RSObject*) arg[1].i ;
	if ( pPtrObj != NULL )
	{
		context->m_regset[regAcc].i = pPtrObj->IsEqualObject( pObj2 ) ? 0 : -1 ;
	}
	else
	{
		context->m_regset[regAcc].i = (pPtrObj != pObj2) ? -1 : 0 ;
	}
	return	NULL ;
}

#endif

