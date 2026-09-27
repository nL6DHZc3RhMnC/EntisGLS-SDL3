
#include <rosetta/rosetta.h>
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <rosetta/rosetta_file.h>
#include <rosetta/rosetta_reference.h>
#include <rosetta/rosetta_thread.h>
#include <rosetta/rosetta_image.h>
#include <rosetta/rosetta_model.h>

using namespace	SSystem ;
using namespace	SakuraGL ;
using namespace	Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// Vector3D 構造体
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSVector3DClass, RSStructuredPointerClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSVector3DClass::RSVector3DClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSStructuredPointerClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSVector3DClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStructuredPointer( this, NULL ) ;
	//
	AddArrayMemberAs
		( context, L"x",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"y",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"z",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>",
				NULL, L"double x, double y, double z",
				NULL, &RSVector3DClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"equals", L"boolean", L"Vector3D v",
				NULL, &RSVector3DClass::method_equals,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"clone",
				L"Vector3D", L"",
				NULL, &RSVector3DClass::method_clone,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"copy",
				L"Vector3D", L"Vector3D src",
				NULL, &RSVector3DClass::method_copy, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"add",
				L"Vector3D", L"Vector3D v",
				NULL, &RSVector3DClass::method_add, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"sub",
				L"Vector3D", L"Vector3D v",
				NULL, &RSVector3DClass::method_sub, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"mul",
				L"Vector3D", L"double s",
				NULL, &RSVector3DClass::method_mul, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"div",
				L"Vector3D", L"double s",
				NULL, &RSVector3DClass::method_div, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"absolute",
				L"double", L"",
				NULL, &RSVector3DClass::method_absolute,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"normalize",
				L"Vector3D", L"",
				NULL, &RSVector3DClass::method_normalize, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"innerProduct",
				L"double", L"Vector3D v",
				NULL, &RSVector3DClass::method_innerProduct,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"exteriorProduct",
				L"Vector3D", L"Vector3D v",
				NULL, &RSVector3DClass::method_exteriorProduct,
				NULL, RSFunctionPrototype::flagConstant ) ;
}

// this 実体ポインタを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DVector *
	RSVector3DClass::GetThisVector( RSContext& context, RSObject* pThis )
{
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"this が Vector3D ではありません" ) ;
		return	NULL ;
	}
	S3DVector *	pPtr = (S3DVector*) pObj->GetPointer( sizeof(S3DVector) ) ;
	if ( pPtr == NULL )
	{
		context.ThrowExceptionError
			( L"Vector3D の this ポインタが有効ではありません" ) ;
	}
	return	pPtr ;
}

// void <init>( double x, double y, double z )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector3DClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Vector3D 構築関数の this が Structure ではありません" ) ;
		return	NULL ;
	}
	RSStructuredPointerClass *
		pStructType = ESLTypeCast<RSStructuredPointerClass>( pThis->GetRSClass() ) ;
	if ( pStructType == NULL )
	{
		context.ThrowExceptionError( L"構造体情報がありません" ) ;
		return	NULL ;
	}
	//
	RSArrayBuffer *	pBuf = pStructType->NewBuffer( context ) ;
	pObj->SetPointer( pBuf, pStructType ) ;
	//
	S3DVector *	pPtr = (S3DVector*) pBuf->m_ptrBuf ;
	pPtr->x = (float32_t) arg.DoubleAt( 0 ) ;
	pPtr->y = (float32_t) arg.DoubleAt( 1 ) ;
	pPtr->z = (float32_t) arg.DoubleAt( 2 ) ;
	return	NULL ;
}

// boolean equals( Vector3D v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector3DClass::method_equals
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVector *	pPtr = GetThisVector( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pvSrc = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	if ( pvSrc == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( *pPtr == *pvSrc ) ;
}

// const Vector3D clone()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector3DClass::method_clone
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVector *	pPtr = GetThisVector( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSStructuredPointer *	pObj =
		ESLTypeCast<RSStructuredPointer>
			( context.new_StructuredPointer( L"Vector3D" ) ) ;
	ESLAssert( pObj != NULL ) ;
	*((S3DVector*) pObj->GetPointer()) = *pPtr ;
	return	pObj ;
}

// Vector3D copy( Vector3D v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector3DClass::method_copy
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVector *	pPtr = GetThisVector( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pvSrc = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	if ( pvSrc == NULL )
	{
		context.ThrowExceptionError( L"Vector3D.copy 関数の引数が不正です" ) ;
		return	NULL ;
	}
	*pPtr = *pvSrc ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Vector3D add( Vector3D v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector3DClass::method_add
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVector *	pPtr = GetThisVector( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pvSrc = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	if ( pvSrc == NULL )
	{
		context.ThrowExceptionError( L"Vector3D.add 関数の引数が不正です" ) ;
		return	NULL ;
	}
	*pPtr += *pvSrc ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Vector3D sub( Vector3D v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector3DClass::method_sub
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVector *	pPtr = GetThisVector( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pvSrc = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	if ( pvSrc == NULL )
	{
		context.ThrowExceptionError( L"Vector3D.sub 関数の引数が不正です" ) ;
		return	NULL ;
	}
	*pPtr -= *pvSrc ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Vector3D mul( double v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector3DClass::method_mul
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVector *	pPtr = GetThisVector( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	*pPtr *= (float32_t) arg.DoubleAt( 0 ) ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Vector3D div( double v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector3DClass::method_div
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVector *	pPtr = GetThisVector( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	*pPtr /= (float32_t) arg.DoubleAt( 0 ) ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// const double absolute()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector3DClass::method_absolute
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVector *	pPtr = GetThisVector( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	return	context.new_Number( pPtr->Absolute() ) ;
}

// Vector3D normalize()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector3DClass::method_normalize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVector *	pPtr = GetThisVector( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	pPtr->Normalize() ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// const double innerProduct( Vector3D v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector3DClass::method_innerProduct
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVector *	pPtr = GetThisVector( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pvSrc = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	if ( pvSrc == NULL )
	{
		context.ThrowExceptionError
				( L"Vector3D.innerProduct 関数の引数が不正です" ) ;
		return	NULL ;
	}
	return	context.new_Number( pPtr->InnerProduct( *pvSrc ) ) ;
}

// const Vector3D exteriorProduct( Vector3D v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector3DClass::method_exteriorProduct
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVector *	pPtr = GetThisVector( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pvSrc = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	if ( pvSrc == NULL )
	{
		context.ThrowExceptionError
				( L"Vector3D.exteriorProduct 関数の引数が不正です" ) ;
		return	NULL ;
	}
	RSStructuredPointer *	pObj =
		ESLTypeCast<RSStructuredPointer>
			( context.new_StructuredPointer( L"Vector3D" ) ) ;
	ESLAssert( pObj != NULL ) ;
	*((S3DVector*) pObj->GetPointer()) = *pPtr * *pvSrc ;
	return	pObj ;
}



//////////////////////////////////////////////////////////////////////////////
// Vector3D4 構造体
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSVector3D4Class, RSStructuredPointerClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSVector3D4Class::RSVector3D4Class
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSStructuredPointerClass( pClass, pwszClassName )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSVector3D4Class::Initialize( RSContext& context )
{
	ESLAssert( m_pSuperClass == NULL ) ;
	AddSuperClass( context, context.GetClassAs( L"Vector3D" ) ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSVector3D4Class::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStructuredPointer( this, NULL ) ;
	//
	RemoveVirtualMemberAs( context, L"copy" ) ;
	RemoveVirtualMemberAs( context, L"add" ) ;
	RemoveVirtualMemberAs( context, L"sub" ) ;
	//
	AddArrayMemberAs
		( context, L"w",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>",
				NULL, L"double x, double y, double z, double w",
				NULL, &RSVector3D4Class::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"clone",
				L"Vector3D4", L"",
				NULL, &RSVector3D4Class::method_clone,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"copy",
				L"Vector3D4", L"Vector3D4 src",
				NULL, &RSVector3D4Class::method_copy, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"add",
				L"Vector3D4", L"Vector3D4 v",
				NULL, &RSVector3D4Class::method_add, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"sub",
				L"Vector3D4", L"Vector3D4 v",
				NULL, &RSVector3D4Class::method_sub, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"mul",
				L"Vector3D4", L"double s",
				NULL, &RSVector3D4Class::method_mul, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"div",
				L"Vector3D4", L"double s",
				NULL, &RSVector3D4Class::method_div, NULL ) ;
}

// this 実体ポインタを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S4DVector *
	RSVector3D4Class::GetThisVector( RSContext& context, RSObject* pThis )
{
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"this が Vector3D4 ではありません" ) ;
		return	NULL ;
	}
	S4DVector *	pPtr = (S4DVector*) pObj->GetPointer( sizeof(S4DVector) ) ;
	if ( pPtr == NULL )
	{
		context.ThrowExceptionError
			( L"Vector3D4 の this ポインタが有効ではありません" ) ;
	}
	return	pPtr ;
}

// void <init>( double x, double y, double z, double w )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector3D4Class::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Vector3D4 構築関数の this が Structure ではありません" ) ;
		return	NULL ;
	}
	RSStructuredPointerClass *
		pStructType = ESLTypeCast<RSStructuredPointerClass>( pThis->GetRSClass() ) ;
	if ( pStructType == NULL )
	{
		context.ThrowExceptionError( L"構造体情報がありません" ) ;
		return	NULL ;
	}
	//
	RSArrayBuffer *	pBuf = pStructType->NewBuffer( context ) ;
	pObj->SetPointer( pBuf, pStructType ) ;
	//
	S4DVector *	pPtr = (S4DVector*) pBuf->m_ptrBuf ;
	pPtr->x = (float32_t) arg.DoubleAt( 0 ) ;
	pPtr->y = (float32_t) arg.DoubleAt( 1 ) ;
	pPtr->z = (float32_t) arg.DoubleAt( 2 ) ;
	pPtr->w = (float32_t) arg.DoubleAt( 3 ) ;
	return	NULL ;
}

// const Vector3D4 clone()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector3D4Class::method_clone
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S4DVector *	pPtr = GetThisVector( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSStructuredPointer *	pObj =
		ESLTypeCast<RSStructuredPointer>
			( context.new_StructuredPointer( L"Vector3D4" ) ) ;
	ESLAssert( pObj != NULL ) ;
	*((S4DVector*) pObj->GetPointer()) = *pPtr ;
	return	pObj ;
}

// Vector3D4 copy( Vector3D4 v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector3D4Class::method_copy
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S4DVector *	pPtr = GetThisVector( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S4DVector *	pvSrc = (S4DVector*) arg.PointerAt( 0, sizeof(S4DVector) ) ;
	if ( pvSrc == NULL )
	{
		context.ThrowExceptionError( L"Vector3D4.copy 関数の引数が不正です" ) ;
		return	NULL ;
	}
	*pPtr = *pvSrc ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Vector3D4 add( Vector3D v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector3D4Class::method_add
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S4DVector *	pPtr = GetThisVector( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S4DVector *	pvSrc = (S4DVector*) arg.PointerAt( 0, sizeof(S4DVector) ) ;
	if ( pvSrc == NULL )
	{
		context.ThrowExceptionError( L"Vector3D4.add 関数の引数が不正です" ) ;
		return	NULL ;
	}
	*pPtr += *pvSrc ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Vector3D4 sub( Vector3D v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector3D4Class::method_sub
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S4DVector *	pPtr = GetThisVector( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S4DVector *	pvSrc = (S4DVector*) arg.PointerAt( 0, sizeof(S4DVector) ) ;
	if ( pvSrc == NULL )
	{
		context.ThrowExceptionError( L"Vector3D4.add 関数の引数が不正です" ) ;
		return	NULL ;
	}
	*pPtr -= *pvSrc ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Vector3D4 mul( double s )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector3D4Class::method_mul
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S4DVector *	pPtr = GetThisVector( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	*pPtr *= (float32_t) arg.DoubleAt(0) ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Vector3D4 div( double s )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector3D4Class::method_div
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S4DVector *	pPtr = GetThisVector( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	*pPtr /= (float32_t) arg.DoubleAt(0) ;
	RSObject::AddRef( pThis );
	return	pThis ;
}



//////////////////////////////////////////////////////////////////////////////
// Color3D 構造体
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSColor3DClass, RSStructuredPointerClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSColor3DClass::RSColor3DClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSStructuredPointerClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSColor3DClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStructuredPointer( this, NULL ) ;
	//
	AddArrayMemberAs
		( context, L"rgbMul",
			context.GetBasicTypeClass(RSCodeControl::wiInt),
			0, 0, context.new_Integer(0x00FFFFFF) ) ;
	AddArrayMemberAs
		( context, L"rgbAdd",
			context.GetBasicTypeClass(RSCodeControl::wiInt),
			0, 0, context.new_Integer(0x00000000) ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>",
				NULL, L"int rgbMul, int rgbAdd",
				NULL, &RSColor3DClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"set",
				L"Color3D", L"int rgbMul, int rgbAdd",
				NULL, &RSColor3DClass::method_set, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"copy",
				L"Color3D", L"Color3D color",
				NULL, &RSColor3DClass::method_copy, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"blend",
				L"Color3D", L"Color3D color",
				NULL, &RSColor3DClass::method_blend1,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"blend",
				L"int", L"int color",
				NULL, &RSColor3DClass::method_blend2,
				NULL, RSFunctionPrototype::flagConstant ) ;
}

// this 実体ポインタを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DColor *
	RSColor3DClass::GetThisColor( RSContext& context, RSObject* pThis )
{
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"this が Color3D ではありません" ) ;
		return	NULL ;
	}
	S3DColor *	pPtr = (S3DColor*) pObj->GetPointer( sizeof(S3DColor) ) ;
	if ( pPtr == NULL )
	{
		context.ThrowExceptionError
			( L"Color3D の this ポインタが有効ではありません" ) ;
	}
	return	pPtr ;
}

// void <init>( int rgbMul, int rgbAdd )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSColor3DClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Color3D 構築関数の this が Structure ではありません" ) ;
		return	NULL ;
	}
	RSStructuredPointerClass *
		pStructType = ESLTypeCast<RSStructuredPointerClass>( pThis->GetRSClass() ) ;
	if ( pStructType == NULL )
	{
		context.ThrowExceptionError( L"構造体情報がありません" ) ;
		return	NULL ;
	}
	//
	RSArrayBuffer *	pBuf = pStructType->NewBuffer( context ) ;
	pObj->SetPointer( pBuf, pStructType ) ;
	//
	S3DColor *	pPtr = (S3DColor*) pBuf->m_ptrBuf ;
	pPtr->rgbMul = arg.IntAt( 0 ) & 0x00FFFFFF ;
	pPtr->rgbAdd = arg.IntAt( 1 ) & 0x00FFFFFF ;
	return	NULL ;
}

// Color3D set( int rgbMul, int rgbAdd )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSColor3DClass::method_set
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DColor *	pPtr = GetThisColor( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pPtr->rgbMul = arg.IntAt( 0 ) & 0x00FFFFFF ;
	pPtr->rgbAdd = arg.IntAt( 1 ) & 0x00FFFFFF ;
	//
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Color3D copy( Color3D color )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSColor3DClass::method_copy
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DColor *	pPtr = GetThisColor( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DColor *	pSrc = (S3DColor*) arg.PointerAt( 0, sizeof(S3DColor) ) ;
	if ( pSrc == NULL )
	{
		context.ThrowExceptionError( L"Color3D.copy 関数の引数が不正です" ) ;
		return	NULL ;
	}
	*pPtr = *pSrc ;
	//
	RSObject::AddRef( pThis );
	return	pThis ;
}

// const Color3D blend( Color3D color )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSColor3DClass::method_blend1
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DColor *	pPtr = GetThisColor( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DColor *	pSrc = (S3DColor*) arg.PointerAt( 0, sizeof(S3DColor) ) ;
	if ( pSrc == NULL )
	{
		context.ThrowExceptionError( L"Color3D.blend 関数の引数が不正です" ) ;
		return	NULL ;
	}
	RSStructuredPointer *	pObj =
		ESLTypeCast<RSStructuredPointer>
			( context.new_StructuredPointer( L"Vector3D" ) ) ;
	ESLAssert( pObj != NULL ) ;
	//
	*((S3DColor*) pObj->GetPointer()) = *pPtr * *pSrc ;
	//
	return	pObj ;
}

// const int blend( int color )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSColor3DClass::method_blend2
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DColor *	pPtr = GetThisColor( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLPalette	rgbSrc( (uint32_t) arg.IntAt( 0 ) ) ;
	rgbSrc = *pPtr * rgbSrc ;
	//
	return	context.new_Integer( rgbSrc.ui32 ) ;
}


//////////////////////////////////////////////////////////////////////////////
// Quaternion 構造体
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( Rosetta::RSQuaternionClass, RSStructuredPointerClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSQuaternionClass::RSQuaternionClass
	( RSClass * pClass, const wchar_t * pwszClassName )
	: RSStructuredPointerClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSQuaternionClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStructuredPointer( this, NULL ) ;
	//
	AddArrayMemberAs
		( context, L"q0",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"q1",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"q2",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"q3",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>",
				NULL, L"double q0, double q1, double a2, double q3",
				NULL, &RSQuaternionClass::method_init0, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>",
				NULL, L"Quaternion q",
				NULL, &RSQuaternionClass::method_init1, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>",
				NULL, L"Matrix3D mat3",
				NULL, &RSQuaternionClass::method_init2, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"equals", L"boolean", L"Quaternion q",
				NULL, &RSQuaternionClass::method_equals,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"clone",
				L"Quaternion", L"",
				NULL, &RSQuaternionClass::method_clone,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"copy",
				L"Quaternion", L"Quaternion q",
				NULL, &RSQuaternionClass::method_copy, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"add",
				L"Quaternion", L"Quaternion q",
				NULL, &RSQuaternionClass::method_add, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"sub",
				L"Quaternion", L"Quaternion q",
				NULL, &RSQuaternionClass::method_sub, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"mul",
				L"Quaternion", L"Quaternion q",
				NULL, &RSQuaternionClass::method_mul1, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"mul",
				L"Quaternion", L"double s",
				NULL, &RSQuaternionClass::method_mul2, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"div",
				L"Quaternion", L"double s",
				NULL, &RSQuaternionClass::method_div, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"norm",
				L"double", L"",
				NULL, &RSQuaternionClass::method_norm,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"normalize",
				L"Quaternion", L"",
				NULL, &RSQuaternionClass::method_normalize, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"innerProduct",
				L"double", L"Quaternion q",
				NULL, &RSQuaternionClass::method_innerProduct,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"inverse",
				L"Quaternion", L"",
				NULL, &RSQuaternionClass::method_inverse,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"toMatrix",
				L"Matrix3D", L"Matrix3D mat3",
				NULL, &RSQuaternionClass::method_toMatrix,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"fromMatrix",
				L"Quaternion", L"Matrix3D mat3",
				NULL, &RSQuaternionClass::method_fromMatrix, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getRodriguesRotaion",
				L"double", L"Vector3D v",
				NULL, &RSQuaternionClass::method_getRodriguesRotaion,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setRodriguesRotaion",
				L"Quaternion", L"Vector3D v, double rad",
				NULL, &RSQuaternionClass::method_setRodriguesRotaion, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"lerp",
				L"Quaternion", L"Quaternion q0, Quaternion q1, double t",
				NULL, &RSQuaternionClass::method_lerp, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"slerp",
				L"Quaternion", L"Quaternion q0, Quaternion q1, double t",
				NULL, &RSQuaternionClass::method_slerp, NULL ) ;
}

// this 実体ポインタを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DQuaternion *
	RSQuaternionClass::GetThisQuaternion( RSContext& context, RSObject* pThis )
{
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"this が Vector3D ではありません" ) ;
		return	NULL ;
	}
	S3DQuaternion *	pPtr =
		(S3DQuaternion*) pObj->GetPointer( sizeof(S3DQuaternion) ) ;
	if ( pPtr == NULL )
	{
		context.ThrowExceptionError
			( L"Quaternion の this ポインタが有効ではありません" ) ;
	}
	return	pPtr ;
}

// void <init>( double q0, double q1, double a2, double q3 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSQuaternionClass::method_init0
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Quaternion 構築関数の this が Structure ではありません" ) ;
		return	NULL ;
	}
	RSStructuredPointerClass *
		pStructType = ESLTypeCast<RSStructuredPointerClass>( pThis->GetRSClass() ) ;
	if ( pStructType == NULL )
	{
		context.ThrowExceptionError( L"構造体情報がありません" ) ;
		return	NULL ;
	}
	//
	RSArrayBuffer *	pBuf = pStructType->NewBuffer( context ) ;
	pObj->SetPointer( pBuf, pStructType ) ;
	//
	S3DQuaternion *	pPtr = (S3DQuaternion*) pBuf->m_ptrBuf ;
	pPtr->q[0] = (float32_t) arg.DoubleAt( 0 ) ;
	pPtr->q[1] = (float32_t) arg.DoubleAt( 1 ) ;
	pPtr->q[2] = (float32_t) arg.DoubleAt( 2 ) ;
	pPtr->q[3] = (float32_t) arg.DoubleAt( 3 ) ;
	return	NULL ;
}

// void <init>( Quaternion q )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSQuaternionClass::method_init1
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Quaternion 構築関数の this が Structure ではありません" ) ;
		return	NULL ;
	}
	RSStructuredPointerClass *
		pStructType = ESLTypeCast<RSStructuredPointerClass>( pThis->GetRSClass() ) ;
	if ( pStructType == NULL )
	{
		context.ThrowExceptionError( L"構造体情報がありません" ) ;
		return	NULL ;
	}
	S3DQuaternion *	pqSrc =
			(S3DQuaternion*) arg.PointerAt( 0, sizeof(S3DQuaternion) ) ;
	if ( pqSrc == NULL )
	{
		context.ThrowExceptionError( L"Quaternion 構築関数の引数が null です" ) ;
		return	NULL ;
	}
	//
	RSArrayBuffer *	pBuf = pStructType->NewBuffer( context ) ;
	pObj->SetPointer( pBuf, pStructType ) ;
	//
	S3DQuaternion *	pPtr = (S3DQuaternion*) pBuf->m_ptrBuf ;
	*pPtr = *pqSrc ;
	return	NULL ;
}

// void <init>( Matrix3D mat3 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSQuaternionClass::method_init2
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Quaternion 構築関数の this が Structure ではありません" ) ;
		return	NULL ;
	}
	RSStructuredPointerClass *
		pStructType = ESLTypeCast<RSStructuredPointerClass>( pThis->GetRSClass() ) ;
	if ( pStructType == NULL )
	{
		context.ThrowExceptionError( L"構造体情報がありません" ) ;
		return	NULL ;
	}
	SGL3DMatrix<float32_t,3> *	pmatSrc =
		(SGL3DMatrix<float32_t,3>*) arg.PointerAt( 0, sizeof(SGL3DMatrix<float32_t,3>) ) ;
	if ( pmatSrc == NULL )
	{
		context.ThrowExceptionError( L"Quaternion 構築関数の引数が null です" ) ;
		return	NULL ;
	}
	S3DMatrix	matSrc ;
	for ( int i = 0; i < 3; i ++ )
	{
		matSrc.m[i][0] = pmatSrc->m[i][0] ;
		matSrc.m[i][1] = pmatSrc->m[i][1] ;
		matSrc.m[i][2] = pmatSrc->m[i][2] ;
	}
	//
	RSArrayBuffer *	pBuf = pStructType->NewBuffer( context ) ;
	pObj->SetPointer( pBuf, pStructType ) ;
	//
	S3DQuaternion *	pPtr = (S3DQuaternion*) pBuf->m_ptrBuf ;
	pPtr->FromMatrix( matSrc ) ;
	return	NULL ;
}

// boolean equals( Quaternion q )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSQuaternionClass::method_equals
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DQuaternion *	pPtr = GetThisQuaternion( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DQuaternion *	pqSrc =
			(S3DQuaternion*) arg.PointerAt( 0, sizeof(S3DQuaternion) ) ;
	if ( pqSrc == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( *pPtr == *pqSrc ) ;
}

// const Quaternion clone()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSQuaternionClass::method_clone
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DQuaternion *	pPtr = GetThisQuaternion( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSStructuredPointer *	pObj =
		ESLTypeCast<RSStructuredPointer>
			( context.new_StructuredPointer( L"Quaternion" ) ) ;
	ESLAssert( pObj != NULL ) ;
	*((S3DQuaternion*) pObj->GetPointer()) = *pPtr ;
	return	pObj ;
}

// Quaternion copy( Quaternion q )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSQuaternionClass::method_copy
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DQuaternion *	pPtr = GetThisQuaternion( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DQuaternion *	pqSrc =
		(S3DQuaternion*) arg.PointerAt( 0, sizeof(S3DQuaternion) ) ;
	if ( pqSrc == NULL )
	{
		context.ThrowExceptionError( L"Quaternion.copy 関数の引数が不正です" ) ;
		return	NULL ;
	}
	*pPtr = *pqSrc ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Quaternion add( Quaternion q )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSQuaternionClass::method_add
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DQuaternion *	pPtr = GetThisQuaternion( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DQuaternion *	pqSrc =
		(S3DQuaternion*) arg.PointerAt( 0, sizeof(S3DQuaternion) ) ;
	if ( pqSrc == NULL )
	{
		context.ThrowExceptionError( L"Quaternion.add 関数の引数が不正です" ) ;
		return	NULL ;
	}
	*pPtr += *pqSrc ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Quaternion sub( Quaternion q )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSQuaternionClass::method_sub
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DQuaternion *	pPtr = GetThisQuaternion( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DQuaternion *	pqSrc =
		(S3DQuaternion*) arg.PointerAt( 0, sizeof(S3DQuaternion) ) ;
	if ( pqSrc == NULL )
	{
		context.ThrowExceptionError( L"Quaternion.sub 関数の引数が不正です" ) ;
		return	NULL ;
	}
	*pPtr -= *pqSrc ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Quaternion mul( Quaternion q )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSQuaternionClass::method_mul1
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DQuaternion *	pPtr = GetThisQuaternion( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DQuaternion *	pqSrc =
		(S3DQuaternion*) arg.PointerAt( 0, sizeof(S3DQuaternion) ) ;
	if ( pqSrc == NULL )
	{
		context.ThrowExceptionError( L"Quaternion.sub 関数の引数が不正です" ) ;
		return	NULL ;
	}
	*pPtr *= *pqSrc ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Quaternion mul( double s )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSQuaternionClass::method_mul2
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DQuaternion *	pPtr = GetThisQuaternion( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	*pPtr *= (float32_t) arg.DoubleAt( 0 ) ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Quaternion div( double s )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSQuaternionClass::method_div
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DQuaternion *	pPtr = GetThisQuaternion( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	*pPtr *= 1.0f / (float32_t) arg.DoubleAt( 0 ) ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// const double norm()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSQuaternionClass::method_norm
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DQuaternion *	pPtr = GetThisQuaternion( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	return	context.new_Number( pPtr->Norm() ) ;
}

// Quaternion normalize()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSQuaternionClass::method_normalize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DQuaternion *	pPtr = GetThisQuaternion( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	pPtr->Normalize() ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// const double innerProduct( Quaternion q )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSQuaternionClass::method_innerProduct
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DQuaternion *	pPtr = GetThisQuaternion( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DQuaternion *	pqSrc =
		(S3DQuaternion*) arg.PointerAt( 0, sizeof(S3DQuaternion) ) ;
	if ( pqSrc == NULL )
	{
		context.ThrowExceptionError( L"Quaternion.sub 関数の引数が不正です" ) ;
		return	NULL ;
	}
	return	context.new_Number( pPtr->InnerProduct( *pqSrc ) ) ;
}

// Quaternion inverse()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSQuaternionClass::method_inverse
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DQuaternion *	pPtr = GetThisQuaternion( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	*pPtr = pPtr->Inverse() ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Matrix3D toMatrix( Matrix3D mat3 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSQuaternionClass::method_toMatrix
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DQuaternion *	pPtr = GetThisQuaternion( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGL3DMatrix<float32_t,3> *	pmatSrc =
		(SGL3DMatrix<float32_t,3>*) arg.PointerAt( 0, sizeof(SGL3DMatrix<float32_t,3>) ) ;
	if ( pmatSrc == NULL )
	{
		context.ThrowExceptionError( L"Quaternion.toMatrix 関数の引数が不正です" ) ;
		return	NULL ;
	}
	S3DMatrix	matTemp ;
	pPtr->ToMatrix( matTemp ) ;
	//
	for ( int i = 0; i < 3; i ++ )
	{
		pmatSrc->m[i][0] = matTemp.m[i][0] ;
		pmatSrc->m[i][1] = matTemp.m[i][1] ;
		pmatSrc->m[i][2] = matTemp.m[i][2] ;
	}
	RSObject *	pArg = arg.ObjectAt( 0 ) ;
	RSObject::AddRef( pArg ) ;
	return	pArg ;
}

// Quaternion fromMatrix( Matrix3D mat3 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSQuaternionClass::method_fromMatrix
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DQuaternion *	pPtr = GetThisQuaternion( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGL3DMatrix<float32_t,3> *	pmatSrc =
		(SGL3DMatrix<float32_t,3>*) arg.PointerAt( 0, sizeof(SGL3DMatrix<float32_t,3>) ) ;
	if ( pmatSrc == NULL )
	{
		context.ThrowExceptionError( L"Quaternion.fromMatrix 関数の引数が不正です" ) ;
		return	NULL ;
	}
	S3DMatrix	matTemp ;
	for ( int i = 0; i < 3; i ++ )
	{
		matTemp.m[i][0] = pmatSrc->m[i][0] ;
		matTemp.m[i][1] = pmatSrc->m[i][1] ;
		matTemp.m[i][2] = pmatSrc->m[i][2] ;
	}
	pPtr->FromMatrix( matTemp ) ;
	RSObject::AddRef( pThis ) ;
	return	pThis ;
}

// const double getRodriguesRotaion( Vector3D v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSQuaternionClass::method_getRodriguesRotaion
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DQuaternion *	pPtr = GetThisQuaternion( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pvDst = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	if ( pvDst == NULL )
	{
		context.ThrowExceptionError
			( L"Quaternion.getRodriguesRotaion 関数の引数が不正です" ) ;
		return	NULL ;
	}
	return	context.new_Number( pPtr->GetRodriguesRotaion( *pvDst ) ) ;
}

// Quaternion setRodriguesRotaion( Vector3D v, double rad )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSQuaternionClass::method_setRodriguesRotaion
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DQuaternion *	pPtr = GetThisQuaternion( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pvSrc = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	if ( pvSrc == NULL )
	{
		context.ThrowExceptionError
			( L"Quaternion.setRodriguesRotaion 関数の引数が不正です" ) ;
		return	NULL ;
	}
	pPtr->SetRodriguesRotaion( *pvSrc, (float32_t) arg.DoubleAt(1) ) ;
	RSObject::AddRef( pThis ) ;
	return	pThis ;
}

// Quaternion lerp( Quaternion q0, Quaternion q1, double t )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSQuaternionClass::method_lerp
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DQuaternion *	pPtr = GetThisQuaternion( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DQuaternion *	pq0 =
			(S3DQuaternion*) arg.PointerAt( 0, sizeof(S3DQuaternion) ) ;
	S3DQuaternion *	pq1 =
			(S3DQuaternion*) arg.PointerAt( 1, sizeof(S3DQuaternion) ) ;
	if ( (pq0 == NULL) || (pq1 == NULL) )
	{
		context.ThrowExceptionError
			( L"Quaternion.lerp 関数の引数が不正です" ) ;
		return	NULL ;
	}
	pPtr->Lerp( *pq0, *pq1, (float32_t) arg.DoubleAt(2) ) ;
	RSObject::AddRef( pThis ) ;
	return	pThis ;
}

// Quaternion slerp( Quaternion q0, Quaternion q1, double t )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSQuaternionClass::method_slerp
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DQuaternion *	pPtr = GetThisQuaternion( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DQuaternion *	pq0 =
			(S3DQuaternion*) arg.PointerAt( 0, sizeof(S3DQuaternion) ) ;
	S3DQuaternion *	pq1 =
			(S3DQuaternion*) arg.PointerAt( 1, sizeof(S3DQuaternion) ) ;
	if ( (pq0 == NULL) || (pq1 == NULL) )
	{
		context.ThrowExceptionError
			( L"Quaternion.slerp 関数の引数が不正です" ) ;
		return	NULL ;
	}
	pPtr->Slerp( *pq0, *pq1, (float32_t) arg.DoubleAt(2) ) ;
	RSObject::AddRef( pThis ) ;
	return	pThis ;
}


//////////////////////////////////////////////////////////////////////////////
// Matrix3D 構造体
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( Rosetta::RSMatrix3DClass, RSStructuredPointerClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSMatrix3DClass::RSMatrix3DClass
	( RSClass * pClass, const wchar_t * pwszClassName )
	: RSStructuredPointerClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSMatrix3DClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStructuredPointer( this, NULL ) ;
	//
	AddArrayMemberAs
		( context, L"m",
			context.GetBasicTypeClass(RSCodeControl::wiFloat), 0, 9 ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL,
				L"double m11, double m12, double m13, "
				L"double m21, double m22, double m23, "
				L"double m31, double m32, double m33",
				NULL, &RSMatrix3DClass::method_init0, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL,
				L"Vector3D v0, Vector3D v1, Vector3D v2",
				NULL, &RSMatrix3DClass::method_init1, NULL ) ;
//	AddVirtualDescriptiveAs
//		( context, perr, L"<init>", NULL, L"Vector3D v0",
//				NULL, &RSMatrix3DClass::method_init2, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"Quaternion q",
				NULL, &RSMatrix3DClass::method_init3, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"equals", L"boolean", L"Matrix3D m",
				NULL, &RSMatrix3DClass::method_equals,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"clone",
				L"Matrix3D", L"",
				NULL, &RSMatrix3DClass::method_clone,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"copy",
				L"Matrix3D", L"Matrix3D m",
				NULL, &RSMatrix3DClass::method_copy, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"add",
				L"Matrix3D", L"Matrix3D m",
				NULL, &RSMatrix3DClass::method_add, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"sub",
				L"Matrix3D", L"Matrix3D m",
				NULL, &RSMatrix3DClass::method_sub, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"mul",
				L"Matrix3D", L"Matrix3D m",
				NULL, &RSMatrix3DClass::method_mul1, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"mul",
				L"Vector3D", L"Vector3D v",
				NULL, &RSMatrix3DClass::method_mul2,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"div",
				L"Matrix3D", L"double s",
				NULL, &RSMatrix3DClass::method_div, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"transpose",
				L"Matrix3D", L"",
				NULL, &RSMatrix3DClass::method_transpose, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"inverse",
				L"Matrix3D", L"",
				NULL, &RSMatrix3DClass::method_inverse, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"determinant",
				L"double", L"",
				NULL, &RSMatrix3DClass::method_determinant,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"magnifyByVector",
				L"Matrix3D", L"Vector3D v",
				NULL, &RSMatrix3DClass::method_magnifyByVector, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"magnifyOnVectorOf",
				L"Matrix3D", L"Vector3D v, double s",
				NULL, &RSMatrix3DClass::method_magnifyOnVectorOf, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"rotateOnX",
				L"Matrix3D", L"double sin, double cos",
				NULL, &RSMatrix3DClass::method_rotateOnX, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"rotateOnY",
				L"Matrix3D", L"double sin, double cos",
				NULL, &RSMatrix3DClass::method_rotateOnY, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"rotateOnZ",
				L"Matrix3D", L"double sin, double cos",
				NULL, &RSMatrix3DClass::method_rotateOnZ, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"rotateOnVectorOf",
				L"Matrix3D", L"Vector3D v, double sin, double cos",
				NULL, &RSMatrix3DClass::method_rotateOnVectorOf, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"vectorRotationOf",
				L"Matrix3D", L"Vector3D v0, Vector3D v1",
				NULL, &RSMatrix3DClass::method_vectorRotationOf, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"rotateByAngleOn",
				L"Matrix3D", L"Vector3D v",
				NULL, &RSMatrix3DClass::method_rotateByAngleOn, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"rotateForAngle",
				L"Matrix3D", L"Vector3D v",
				NULL, &RSMatrix3DClass::method_rotateForAngle, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"cameraAngleOf", L"Vector3D",
				L"Vector3D vTarget, Vector3D vView, Vector3D vTop",
				NULL, &RSMatrix3DClass::method_cameraAngleOf, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"transformVectors", NULL,
			L"Vector3D vDst, Vector3D vSrc, int count, Vector3D vOffset = null",
			NULL, &RSMatrix3DClass::method_transformVectors, NULL ) ;
}

// this 実体ポインタを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGL3DMatrix<float32_t,3> *
	RSMatrix3DClass::GetThisMatrix( RSContext& context, RSObject* pThis )
{
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"this が Matrix3D ではありません" ) ;
		return	NULL ;
	}
	SGL3DMatrix<float32_t,3> *	pPtr =
		(SGL3DMatrix<float32_t,3>*)
			pObj->GetPointer( sizeof(SGL3DMatrix<float32_t,3>) ) ;
	if ( pPtr == NULL )
	{
		context.ThrowExceptionError
			( L"Matrix3D の this ポインタが有効ではありません" ) ;
	}
	return	pPtr ;
}

// void <init>( double m11, double m12, double m13,
//				double m21, double m22, double m23,
//				double m31, double m32, double m33 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_init0
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Matrix3D 構築関数の this が Structure ではありません" ) ;
		return	NULL ;
	}
	RSStructuredPointerClass *
		pStructType = ESLTypeCast<RSStructuredPointerClass>( pThis->GetRSClass() ) ;
	if ( pStructType == NULL )
	{
		context.ThrowExceptionError( L"構造体情報がありません" ) ;
		return	NULL ;
	}
	//
	RSArrayBuffer *	pBuf = pStructType->NewBuffer( context ) ;
	pObj->SetPointer( pBuf, pStructType ) ;
	//
	SGL3DMatrix<float32_t,3> *
			pPtr = (SGL3DMatrix<float32_t,3>*) pBuf->m_ptrBuf ;
	for ( int i = 0; i < 3; i ++ )
	{
		pPtr->m[i][0] = (float32_t) arg.DoubleAt( i * 3 + 0 ) ;
		pPtr->m[i][1] = (float32_t) arg.DoubleAt( i * 3 + 1 ) ;
		pPtr->m[i][2] = (float32_t) arg.DoubleAt( i * 3 + 2 ) ;
	}
	return	NULL ;
}

// void <init>( Vector3D v0, Vector3D v1, Vector3D v2 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_init1
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Matrix3D 構築関数の this が Structure ではありません" ) ;
		return	NULL ;
	}
	RSStructuredPointerClass *
		pStructType = ESLTypeCast<RSStructuredPointerClass>( pThis->GetRSClass() ) ;
	if ( pStructType == NULL )
	{
		context.ThrowExceptionError( L"構造体情報がありません" ) ;
		return	NULL ;
	}
	//
	RSArrayBuffer *	pBuf = pStructType->NewBuffer( context ) ;
	pObj->SetPointer( pBuf, pStructType ) ;
	//
	SGL3DMatrix<float32_t,3> *
			pPtr = (SGL3DMatrix<float32_t,3>*) pBuf->m_ptrBuf ;
	for ( size_t i = 0; i < 3; i ++ )
	{
		S3DVector *	pvSrc =
				(S3DVector*) arg.PointerAt( i, sizeof(S3DVector) ) ;
		if ( pvSrc == NULL )
		{
			context.ThrowExceptionError
				( L"Matrix3D 構造体情報の引数が不正です" ) ;
			return	NULL ;
		}
		pPtr->SetColumn( i, *pvSrc ) ;
	}
	return	NULL ;
}

// void <init>( Vector3D v0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_init2
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Matrix3D 構築関数の this が Structure ではありません" ) ;
		return	NULL ;
	}
	RSStructuredPointerClass *
		pStructType = ESLTypeCast<RSStructuredPointerClass>( pThis->GetRSClass() ) ;
	if ( pStructType == NULL )
	{
		context.ThrowExceptionError( L"構造体情報がありません" ) ;
		return	NULL ;
	}
	S3DVector *	pvSrc = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	if ( pvSrc == NULL )
	{
		context.ThrowExceptionError( L"Matrix3D 構造体情報の引数が不正です" ) ;
		return	NULL ;
	}
	//
	RSArrayBuffer *	pBuf = pStructType->NewBuffer( context ) ;
	pObj->SetPointer( pBuf, pStructType ) ;
	//
	SGL3DMatrix<float32_t,3> *
			pPtr = (SGL3DMatrix<float32_t,3>*) pBuf->m_ptrBuf ;
	pPtr->InitializeMatrix( *pvSrc ) ;
	return	NULL ;
}

// void <init>( Quaternion q )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_init3
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Matrix3D 構築関数の this が Structure ではありません" ) ;
		return	NULL ;
	}
	RSStructuredPointerClass *
		pStructType = ESLTypeCast<RSStructuredPointerClass>( pThis->GetRSClass() ) ;
	if ( pStructType == NULL )
	{
		context.ThrowExceptionError( L"構造体情報がありません" ) ;
		return	NULL ;
	}
	S3DQuaternion *	pqSrc =
		(S3DQuaternion*) arg.PointerAt( 0, sizeof(S3DQuaternion) ) ;
	if ( pqSrc == NULL )
	{
		context.ThrowExceptionError( L"Matrix3D 構造体情報の引数が不正です" ) ;
		return	NULL ;
	}
	//
	RSArrayBuffer *	pBuf = pStructType->NewBuffer( context ) ;
	pObj->SetPointer( pBuf, pStructType ) ;
	//
	S3DMatrix	matTemp ;
	pqSrc->ToMatrix( matTemp ) ;
	//
	SGL3DMatrix<float32_t,3> *
			pPtr = (SGL3DMatrix<float32_t,3>*) pBuf->m_ptrBuf ;
	for ( int i = 0; i < 3; i ++ )
	{
		pPtr->m[i][0] = matTemp.m[i][0] ;
		pPtr->m[i][1] = matTemp.m[i][1] ;
		pPtr->m[i][2] = matTemp.m[i][2] ;
	}
	return	NULL ;
}

// boolean equals( Matrix3D v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_equals
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGL3DMatrix<float32_t,3> *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGL3DMatrix<float32_t,3> *	pmatSrc =
		(SGL3DMatrix<float32_t,3>*)
			arg.PointerAt( 0, sizeof(SGL3DMatrix<float32_t,3>) ) ;
	if ( pmatSrc == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( *pPtr == *pmatSrc ) ;
}

// const Matrix3D clone()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_clone
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGL3DMatrix<float32_t,3> *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSStructuredPointer *	pObj =
		ESLTypeCast<RSStructuredPointer>
			( context.new_StructuredPointer( L"Matrix3D" ) ) ;
	ESLAssert( pObj != NULL ) ;
	*((SGL3DMatrix<float32_t,3>*) pObj->GetPointer()) = *pPtr ;
	return	pObj ;
}

// Matrix3D copy( Matrix3D m )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_copy
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGL3DMatrix<float32_t,3> *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGL3DMatrix<float32_t,3> *	pmatSrc =
		(SGL3DMatrix<float32_t,3>*)
			arg.PointerAt( 0, sizeof(SGL3DMatrix<float32_t,3>) ) ;
	if ( pmatSrc == NULL )
	{
		context.ThrowExceptionError( L"Matrix3D.copy 関数の引数が不正です" ) ;
		return	NULL ;
	}
	*pPtr = *pmatSrc ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Matrix3D add( Matrix3D m )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_add
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGL3DMatrix<float32_t,3> *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGL3DMatrix<float32_t,3> *	pmatSrc =
		(SGL3DMatrix<float32_t,3>*)
			arg.PointerAt( 0, sizeof(SGL3DMatrix<float32_t,3>) ) ;
	if ( pmatSrc == NULL )
	{
		context.ThrowExceptionError( L"Matrix3D.add 関数の引数が不正です" ) ;
		return	NULL ;
	}
	*pPtr += *pmatSrc ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Matrix3D sub( Matrix3D m )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_sub
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGL3DMatrix<float32_t,3> *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGL3DMatrix<float32_t,3> *	pmatSrc =
		(SGL3DMatrix<float32_t,3>*)
			arg.PointerAt( 0, sizeof(SGL3DMatrix<float32_t,3>) ) ;
	if ( pmatSrc == NULL )
	{
		context.ThrowExceptionError( L"Matrix3D.sub 関数の引数が不正です" ) ;
		return	NULL ;
	}
	*pPtr -= *pmatSrc ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Matrix3D mul( Matrix3D m )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_mul1
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGL3DMatrix<float32_t,3> *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGL3DMatrix<float32_t,3> *	pmatSrc =
		(SGL3DMatrix<float32_t,3>*)
			arg.PointerAt( 0, sizeof(SGL3DMatrix<float32_t,3>) ) ;
	if ( pmatSrc == NULL )
	{
		context.ThrowExceptionError( L"Matrix3D.mul 関数の引数が不正です" ) ;
		return	NULL ;
	}
	*pPtr *= *pmatSrc ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// const Vector3D mul( Vector3D v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_mul2
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGL3DMatrix<float32_t,3> *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pvSrc = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	if ( pvSrc == NULL )
	{
		context.ThrowExceptionError( L"Matrix3D.mul 関数の引数が不正です" ) ;
		return	NULL ;
	}
	RSStructuredPointer *	pObj =
		ESLTypeCast<RSStructuredPointer>
			( context.new_StructuredPointer( L"Vector3D" ) ) ;
	ESLAssert( pObj != NULL ) ;
	*((S3DVector*) pObj->GetPointer()) = *pPtr * *pvSrc ;
	return	pObj ;
}

// Matrix3D mul( double s )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_mul3
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGL3DMatrix<float32_t,3> *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	*pPtr *= (float32_t) arg.DoubleAt( 0 ) ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Matrix3D div( double s )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_div
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGL3DMatrix<float32_t,3> *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	*pPtr /= (float32_t) arg.DoubleAt( 0 ) ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Matrix3D transpose()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_transpose
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGL3DMatrix<float32_t,3> *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	*pPtr = pPtr->Transpose() ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Matrix3D inverse()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_inverse
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGL3DMatrix<float32_t,3> *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	*pPtr = pPtr->Inverse() ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// const double determinant()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_determinant
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGL3DMatrix<float32_t,3> *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	return	context.new_Number( pPtr->Determinant() ) ;
}

// Matrix3D magnifyByVector( Vector3D v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_magnifyByVector
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGL3DMatrix<float32_t,3> *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pvSrc = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	if ( pvSrc == NULL )
	{
		context.ThrowExceptionError
			( L"Matrix3D.magnifyByVector 関数の引数が不正です" ) ;
		return	NULL ;
	}
	pPtr->MagnifyByVector( *pvSrc ) ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Matrix3D magnifyOnVectorOf( Vector3D v, double s )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_magnifyOnVectorOf
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGL3DMatrix<float32_t,3> *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pvSrc = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	if ( pvSrc == NULL )
	{
		context.ThrowExceptionError
			( L"Matrix3D.magnifyByVector 関数の引数が不正です" ) ;
		return	NULL ;
	}
	pPtr->MagnifyOnVectorOf( *pvSrc, (float32_t) arg.DoubleAt( 1 ) ) ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Matrix3D rotateOnX( double sin, double cos )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_rotateOnX
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGL3DMatrix<float32_t,3> *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pPtr->RevolveOnX( arg.DoubleAt(0), arg.DoubleAt(1) ) ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Matrix3D rotateOnY( double sin, double cos )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_rotateOnY
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGL3DMatrix<float32_t,3> *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pPtr->RevolveOnY( arg.DoubleAt(0), arg.DoubleAt(1) ) ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Matrix3D rotateOnZ( double sin, double cos )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_rotateOnZ
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGL3DMatrix<float32_t,3> *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pPtr->RevolveOnZ( arg.DoubleAt(0), arg.DoubleAt(1) ) ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Matrix3D rotateOnVectorOf( Vector3D v, double sin, double cos )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_rotateOnVectorOf
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGL3DMatrix<float32_t,3> *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pvSrc = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	if ( pvSrc == NULL )
	{
		context.ThrowExceptionError
			( L"Matrix3D.rotateOnVectorOf 関数の引数が不正です" ) ;
		return	NULL ;
	}
	pPtr->RotationOnVectorOf( *pvSrc, arg.DoubleAt(1), arg.DoubleAt(2) ) ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Matrix3D vectorRotationOf( Vector3D v0, Vector3D v1 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_vectorRotationOf
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGL3DMatrix<float32_t,3> *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pv0 = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	S3DVector *	pv1 = (S3DVector*) arg.PointerAt( 1, sizeof(S3DVector) ) ;
	if ( (pv0 == NULL) || (pv1 == NULL) )
	{
		context.ThrowExceptionError
			( L"Matrix3D.vectorRotationOf 関数の引数が不正です" ) ;
		return	NULL ;
	}
	pPtr->VectorRotationOf( *pv0, *pv1 ) ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Matrix3D rotateByAngleOn( Vector3D v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_rotateByAngleOn
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGL3DMatrix<float32_t,3> *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pvSrc = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	if ( pvSrc == NULL )
	{
		context.ThrowExceptionError
			( L"Matrix3D.rotateByAngleOn 関数の引数が不正です" ) ;
		return	NULL ;
	}
	pPtr->RevolveByAngleOn( *pvSrc ) ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Matrix3D rotateForAngle( Vector3D v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_rotateForAngle
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGL3DMatrix<float32_t,3> *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pvSrc = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	if ( pvSrc == NULL )
	{
		context.ThrowExceptionError
			( L"Matrix3D.rotateForAngle 関数の引数が不正です" ) ;
		return	NULL ;
	}
	pPtr->RevolveForAngle( *pvSrc ) ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Vector3D cameraAngleOf( Vector3D vTarget, Vector3D vView, Vector3D vTop )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_cameraAngleOf
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGL3DMatrix<float32_t,3> *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pvTarget = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	S3DVector *	pvView = (S3DVector*) arg.PointerAt( 1, sizeof(S3DVector) ) ;
	S3DVector *	pvTop = (S3DVector*) arg.PointerAt( 2, sizeof(S3DVector) ) ;
	if ( (pvTarget == NULL) || (pvView == NULL) || (pvTop == NULL) )
	{
		context.ThrowExceptionError
			( L"Matrix3D.cameraAngleOf 関数の引数が不正です" ) ;
		return	NULL ;
	}
	S3DVector	vCamera = pPtr->CameraAngleOf( *pvTarget, *pvView, *pvTop ) ;
	RSStructuredPointer *	pObj =
		ESLTypeCast<RSStructuredPointer>
			( context.new_StructuredPointer( L"Vector3D" ) ) ;
	ESLAssert( pObj != NULL ) ;
	*((S3DVector*) pObj->GetPointer()) = vCamera ;
	return	pObj ;
}

// void transformVectors( Vector3D4 vDst, Vector3D4 vSrc, int count, Vector3D vOffset = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix3DClass::method_transformVectors
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGL3DMatrix<float32_t,3> *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t			nCount = (size_t) arg.IntAt( 2 ) ;
	S3DVector4 *	pvDst = (S3DVector4*) arg.PointerAt( 0, nCount * sizeof(S3DVector4) ) ;
	S3DVector4 *	pvSrc = (S3DVector4*) arg.PointerAt( 1, nCount * sizeof(S3DVector4) ) ;
	S3DVector *		pvOffset = (S3DVector*) arg.PointerAt( 3, sizeof(S3DVector) ) ;
	if ( (pvDst == NULL) || (pvSrc == NULL) )
	{
		context.ThrowExceptionError
			( L"Matrix3D.transformVectors 関数の引数が不正です" ) ;
		return	NULL ;
	}
	if ( pvOffset != NULL )
	{
		for ( size_t i = 0; i < nCount; i ++ )
		{
			pvDst[i] = pvSrc[i] ;
			pPtr->RevolveVector( pvDst[i] ) ;
			pvDst[i] += *pvOffset ;
		}
	}
	else
	{
		for ( size_t i = 0; i < nCount; i ++ )
		{
			pvDst[i] = pvSrc[i] ;
			pPtr->RevolveVector( pvDst[i] ) ;
		}
	}
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// Matrix4D 構造体
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( Rosetta::RSMatrix4DClass, RSStructuredPointerClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSMatrix4DClass::RSMatrix4DClass
	( RSClass * pClass, const wchar_t * pwszClassName )
	: RSStructuredPointerClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSMatrix4DClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStructuredPointer( this, NULL ) ;
	//
	AddArrayMemberAs
		( context, L"m",
			context.GetBasicTypeClass(RSCodeControl::wiFloat), 0, 16 ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL,
				L"double m11, double m12, double m13, double m14,"
				L"double m21, double m22, double m23, double m24,"
				L"double m31, double m32, double m33, double m34,"
				L"double m41, double m42, double m43, double m44,",
				NULL, &RSMatrix4DClass::method_init0, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL,
				L"Matrix3D m, Vector3D v",
				NULL, &RSMatrix4DClass::method_init1, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL,
				L"Vector3D4 v0, Vector3D4 v1, Vector3D4 v2, Vector3D4 v3",
				NULL, &RSMatrix4DClass::method_init2, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"Vector3D4 v",
				NULL, &RSMatrix4DClass::method_init3, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"equals", L"boolean", L"Matrix4D m",
				NULL, &RSMatrix4DClass::method_equals,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"clone",
				L"Matrix4D", L"",
				NULL, &RSMatrix4DClass::method_clone,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"copy",
				L"Matrix4D", L"Matrix4D m",
				NULL, &RSMatrix4DClass::method_copy, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getMatrix3D",
				L"Matrix4D", L"Matrix3D m = null",
				NULL, &RSMatrix4DClass::method_getMatrix3D,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setMatrix3D",
				L"Matrix4D", L"Matrix3D m",
				NULL, &RSMatrix4DClass::method_setMatrix3D, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getTranslation",
				L"Vector3D", L"Vector3D v = null",
				NULL, &RSMatrix4DClass::method_getTranslation,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setTranslation",
				L"Matrix4D", L"Vector3D v",
				NULL, &RSMatrix4DClass::method_setTranslation, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"add",
				L"Matrix4D", L"Matrix4D m",
				NULL, &RSMatrix4DClass::method_add, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"sub",
				L"Matrix4D", L"Matrix4D m",
				NULL, &RSMatrix4DClass::method_sub, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"mul",
				L"Matrix4D", L"Matrix4D m",
				NULL, &RSMatrix4DClass::method_mul1, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"mul",
				L"Vector3D4", L"Vector3D4 v",
				NULL, &RSMatrix4DClass::method_mul2,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"mul",
				L"Matrix4D", L"double s",
				NULL, &RSMatrix4DClass::method_mul3, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"scale",
				L"Matrix4D", L"double x, double y, double z",
				NULL, &RSMatrix4DClass::method_scale, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"translate",
				L"Matrix4D", L"double x, double y, double z",
				NULL, &RSMatrix4DClass::method_translate, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"transpose",
				L"Matrix4D", L"",
				NULL, &RSMatrix4DClass::method_transpose, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"inverse",
				L"Matrix4D", L"",
				NULL, &RSMatrix4DClass::method_inverse, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"determinant",
				L"double", L"",
				NULL, &RSMatrix4DClass::method_determinant,
				NULL, RSFunctionPrototype::flagConstant ) ;
}

// this 実体ポインタを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S4DMatrix *
	RSMatrix4DClass::GetThisMatrix( RSContext& context, RSObject* pThis )
{
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"this が Matrix4D ではありません" ) ;
		return	NULL ;
	}
	S4DMatrix *	pPtr = (S4DMatrix*) pObj->GetPointer( sizeof(S4DMatrix) ) ;
	if ( pPtr == NULL )
	{
		context.ThrowExceptionError
			( L"Matrix3D の this ポインタが有効ではありません" ) ;
	}
	return	pPtr ;
}

// void <init>( double m11, double m12, double m13, double m14,
//				double m21, double m22, double m23, double m24,
//				double m31, double m32, double m33, double m34,
//				double m41, double m42, double m43, double m44 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix4DClass::method_init0
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Matrix4D 構築関数の this が Structure ではありません" ) ;
		return	NULL ;
	}
	RSStructuredPointerClass *
		pStructType = ESLTypeCast<RSStructuredPointerClass>( pThis->GetRSClass() ) ;
	if ( pStructType == NULL )
	{
		context.ThrowExceptionError( L"構造体情報がありません" ) ;
		return	NULL ;
	}
	//
	RSArrayBuffer *	pBuf = pStructType->NewBuffer( context ) ;
	pObj->SetPointer( pBuf, pStructType ) ;
	//
	S4DMatrix *	pPtr = (S4DMatrix*) pBuf->m_ptrBuf ;
	for ( int i = 0; i < 4; i ++ )
	{
		pPtr->m[i][0] = (float32_t) arg.DoubleAt( i * 4 + 0 ) ;
		pPtr->m[i][1] = (float32_t) arg.DoubleAt( i * 4 + 1 ) ;
		pPtr->m[i][2] = (float32_t) arg.DoubleAt( i * 4 + 2 ) ;
		pPtr->m[i][3] = (float32_t) arg.DoubleAt( i * 4 + 3 ) ;
	}
	return	NULL ;
}

// void <init>( Matrix3D m, Vector3D v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix4DClass::method_init1
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Matrix4D 構築関数の this が Structure ではありません" ) ;
		return	NULL ;
	}
	RSStructuredPointerClass *
		pStructType = ESLTypeCast<RSStructuredPointerClass>( pThis->GetRSClass() ) ;
	if ( pStructType == NULL )
	{
		context.ThrowExceptionError( L"構造体情報がありません" ) ;
		return	NULL ;
	}
	SGL3DMatrix<float32_t,3> *	pmatSrc =
		(SGL3DMatrix<float32_t,3>*)
			arg.PointerAt( 0, sizeof(SGL3DMatrix<float32_t,3>) ) ;
	S3DVector *	pvSrc = (S3DVector*) arg.PointerAt( 1, sizeof(S3DVector) ) ;
	//
	if ( (pmatSrc == NULL) || (pvSrc == NULL) )
	{
		context.ThrowExceptionError
			( L"Matrix4D 構造体情報の引数が不正です" ) ;
		return	NULL ;
	}
	//
	RSArrayBuffer *	pBuf = pStructType->NewBuffer( context ) ;
	pObj->SetPointer( pBuf, pStructType ) ;
	//
	S4DMatrix *	pPtr = (S4DMatrix*) pBuf->m_ptrBuf ;
	for ( int i = 0; i < 3; i ++ )
	{
		pPtr->m[i][0] = pmatSrc->m[i][0] ;
		pPtr->m[i][1] = pmatSrc->m[i][1] ;
		pPtr->m[i][2] = pmatSrc->m[i][2] ;
	}
	pPtr->SetTranslation( *pvSrc ) ;
	return	NULL ;
}

// void <init>( Vector3D4 v0, Vector3D4 v1, Vector3D4 v2, Vector3D4 v3 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix4DClass::method_init2
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Matrix4D 構築関数の this が Structure ではありません" ) ;
		return	NULL ;
	}
	RSStructuredPointerClass *
		pStructType = ESLTypeCast<RSStructuredPointerClass>( pThis->GetRSClass() ) ;
	if ( pStructType == NULL )
	{
		context.ThrowExceptionError( L"構造体情報がありません" ) ;
		return	NULL ;
	}
	//
	RSArrayBuffer *	pBuf = pStructType->NewBuffer( context ) ;
	pObj->SetPointer( pBuf, pStructType ) ;
	//
	S4DMatrix *	pPtr = (S4DMatrix*) pBuf->m_ptrBuf ;
	for ( size_t i = 0; i < 4; i ++ )
	{
		S4DVector *	pvSrc =
				(S4DVector*) arg.PointerAt( i, sizeof(S4DVector) ) ;
		if ( pvSrc == NULL )
		{
			context.ThrowExceptionError
				( L"Matrix4D 構造体情報の引数が不正です" ) ;
			return	NULL ;
		}
		pPtr->m[i][0] = pvSrc->x ;
		pPtr->m[i][1] = pvSrc->y ;
		pPtr->m[i][2] = pvSrc->z ;
		pPtr->m[i][3] = pvSrc->w ;
	}
	return	NULL ;
}

// void <init>( Vector3D4 v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix4DClass::method_init3
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Matrix4D 構築関数の this が Structure ではありません" ) ;
		return	NULL ;
	}
	RSStructuredPointerClass *
		pStructType = ESLTypeCast<RSStructuredPointerClass>( pThis->GetRSClass() ) ;
	if ( pStructType == NULL )
	{
		context.ThrowExceptionError( L"構造体情報がありません" ) ;
		return	NULL ;
	}
	S4DVector *	pvSrc =
			(S4DVector*) arg.PointerAt( 0, sizeof(S4DVector) ) ;
	if ( pvSrc == NULL )
	{
		context.ThrowExceptionError
			( L"Matrix4D 構造体情報の引数が不正です" ) ;
		return	NULL ;
	}
	//
	RSArrayBuffer *	pBuf = pStructType->NewBuffer( context ) ;
	pObj->SetPointer( pBuf, pStructType ) ;
	//
	S4DMatrix *	pPtr = (S4DMatrix*) pBuf->m_ptrBuf ;
	pPtr->InitializeMatrix( pvSrc->x, pvSrc->y, pvSrc->z, pvSrc->w ) ;
	return	NULL ;
}

// const boolean equals( Matrix3D m )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix4DClass::method_equals
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S4DMatrix *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S4DMatrix *	pmatSrc = (S4DMatrix*) arg.PointerAt( 0, sizeof(S4DMatrix) ) ;
	if ( pmatSrc == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( *pPtr == *pmatSrc ) ;
}

// const Matrix4D clone()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix4DClass::method_clone
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S4DMatrix *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSStructuredPointer *	pObj =
		ESLTypeCast<RSStructuredPointer>
			( context.new_StructuredPointer( L"Matrix4D" ) ) ;
	ESLAssert( pObj != NULL ) ;
	*((S4DMatrix*) pObj->GetPointer()) = *pPtr ;
	return	pObj ;
}

// Matrix4D copy( Matrix4D m )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix4DClass::method_copy
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S4DMatrix *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S4DMatrix *	pmatSrc = (S4DMatrix*) arg.PointerAt( 0, sizeof(S4DMatrix) ) ;
	if ( pmatSrc == NULL )
	{
		context.ThrowExceptionError( L"Matrix4D.copy 関数の引数が不正です" ) ;
		return	NULL ;
	}
	*pPtr = *pmatSrc ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// const Matrix3D getMatrix3D( Matrix3D m = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix4DClass::method_getMatrix3D
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S4DMatrix *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObj = arg.ObjectAt( 0 ) ;
	SGL3DMatrix<float32_t,3> *
		pmatDst = (SGL3DMatrix<float32_t,3>*)
					arg.PointerAt( 0, sizeof(SGL3DMatrix<float32_t,3>) ) ;
	if ( (pObj == NULL) || (pmatDst == NULL) )
	{
		RSStructuredPointer *	pObjTemp =
			ESLTypeCast<RSStructuredPointer>
				( context.new_StructuredPointer( L"Matrix3D" ) ) ;
		pObj = pObjTemp ;
		pmatDst = (SGL3DMatrix<float32_t,3>*) pObjTemp->GetPointer() ;
	}
	else
	{
		RSObject::AddRef( pObj );
	}
	for ( int i = 0; i < 3; i ++ )
	{
		pmatDst->m[i][0] = pPtr->m[i][0] ;
		pmatDst->m[i][1] = pPtr->m[i][1] ;
		pmatDst->m[i][2] = pPtr->m[i][2] ;
	}
	return	pObj ;
}

// Matrix4D setMatrix3D( Matrix3D m )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix4DClass::method_setMatrix3D
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S4DMatrix *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGL3DMatrix<float32_t,3> *
		pmatSrc = (SGL3DMatrix<float32_t,3>*)
					arg.PointerAt( 0, sizeof(SGL3DMatrix<float32_t,3>) ) ;
	if ( pmatSrc == NULL )
	{
		context.ThrowExceptionError
			( L"Matrix4D.setMatrix3D 関数の引数が不正です" ) ;
		return	NULL ;
	}
	for ( int i = 0; i < 3; i ++ )
	{
		pPtr->m[i][0] = pmatSrc->m[i][0] ;
		pPtr->m[i][1] = pmatSrc->m[i][1] ;
		pPtr->m[i][2] = pmatSrc->m[i][2] ;
	}
	RSObject::AddRef( pThis );
	return	pThis ;
}

// const Vector3D getTranslation( Vector3D v = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix4DClass::method_getTranslation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S4DMatrix *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObj = arg.ObjectAt( 0 ) ;
	S3DVector *	pvDst = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	if ( (pObj == NULL) || (pvDst == NULL) )
	{
		RSStructuredPointer *	pObjTemp =
			ESLTypeCast<RSStructuredPointer>
				( context.new_StructuredPointer( L"Vector3D" ) ) ;
		pObj = pObjTemp ;
		pvDst = (S3DVector*) pObjTemp->GetPointer() ;
	}
	else
	{
		RSObject::AddRef( pObj );
	}
	*pvDst = pPtr->GetTranslation() ;
	return	pObj ;
}

// Matrix4D setTranslation( Vector3D v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix4DClass::method_setTranslation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S4DMatrix *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pvSrc = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	if ( pvSrc == NULL )
	{
		context.ThrowExceptionError
			( L"Matrix4D.setTranslation 関数の引数が不正です" ) ;
		return	NULL ;
	}
	pPtr->SetTranslation( *pvSrc ) ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Matrix4D add( Matrix4D m )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix4DClass::method_add
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S4DMatrix *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S4DMatrix *	pmatSrc = (S4DMatrix*) arg.PointerAt( 0, sizeof(S4DMatrix) ) ;
	if ( pmatSrc == NULL )
	{
		context.ThrowExceptionError( L"Matrix4D.add 関数の引数が不正です" ) ;
		return	NULL ;
	}
	*pPtr += *pmatSrc ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Matrix4D sub( Matrix4D m )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix4DClass::method_sub
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S4DMatrix *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S4DMatrix *	pmatSrc = (S4DMatrix*) arg.PointerAt( 0, sizeof(S4DMatrix) ) ;
	if ( pmatSrc == NULL )
	{
		context.ThrowExceptionError( L"Matrix4D.add 関数の引数が不正です" ) ;
		return	NULL ;
	}
	*pPtr -= *pmatSrc ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Matrix4D mul( Matrix4D m )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix4DClass::method_mul1
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S4DMatrix *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S4DMatrix *	pmatSrc = (S4DMatrix*) arg.PointerAt( 0, sizeof(S4DMatrix) ) ;
	if ( pmatSrc == NULL )
	{
		context.ThrowExceptionError( L"Matrix4D.add 関数の引数が不正です" ) ;
		return	NULL ;
	}
	*pPtr *= *pmatSrc ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// const Vector4D mul( Vector3D4 v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix4DClass::method_mul2
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S4DMatrix *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S4DVector *	pvSrc = (S4DVector*) arg.PointerAt( 0, sizeof(S4DVector) ) ;
	if ( pvSrc == NULL )
	{
		context.ThrowExceptionError( L"Matrix4D.mul 関数の引数が不正です" ) ;
		return	NULL ;
	}
	RSStructuredPointer *	pObjDst =
		ESLTypeCast<RSStructuredPointer>
			( context.new_StructuredPointer( L"Vector3D4" ) ) ;
	S4DVector *	pvDst = (S4DVector*) pObjDst->GetPointer() ;
	*pvDst = *pPtr * *pvSrc ;
	return	pObjDst ;
}

// Matrix4D mul( double s )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix4DClass::method_mul3
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S4DMatrix *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	*pPtr *= (float32_t) arg.DoubleAt( 0 ) ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Matrix4D scale( double x, double y, double z )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix4DClass::method_scale
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S4DMatrix *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pPtr->Scale( arg.DoubleAt(0), arg.DoubleAt(1), arg.DoubleAt(2) ) ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Matrix4D translate( double x, double y, double z )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix4DClass::method_translate
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S4DMatrix *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pPtr->Translate( arg.DoubleAt(0), arg.DoubleAt(1), arg.DoubleAt(2) ) ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Matrix4D transpose()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix4DClass::method_transpose
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S4DMatrix *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	S4DMatrix	matTemp ;
	matTemp.TransposeOf( *pPtr ) ;
	*pPtr = matTemp ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// Matrix4D inverse()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix4DClass::method_inverse
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S4DMatrix *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	S4DMatrix	matTemp ;
	matTemp.InverseOf( *pPtr ) ;
	*pPtr = matTemp ;
	RSObject::AddRef( pThis );
	return	pThis ;
}

// const double determinant()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMatrix4DClass::method_determinant
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S4DMatrix *	pPtr = GetThisMatrix( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	return	context.new_Number( pPtr->Determinant() ) ;
}



//////////////////////////////////////////////////////////////////////////////
// TextureLibrary クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSTextureLibraryClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSTextureLibraryClass::RSTextureLibraryClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSTextureLibraryClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &RSTextureLibraryClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getTextureAs", L"Image", L"String id",
				NULL, &RSTextureLibraryClass::method_getTextureAs,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"addTextureAs",
			L"boolean", L"String id, Image texture",
			NULL, &RSTextureLibraryClass::method_addTextureAs, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"removeTextureAs",
			L"boolean", L"String id",
			NULL, &RSTextureLibraryClass::method_removeTextureAs, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"removeAllTexture",
			L"boolean", L"",
			NULL, &RSTextureLibraryClass::method_removeAllTexture, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSTextureLibraryClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<S3DTextureLibrary>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DTextureLibrary *
	RSTextureLibraryClass::GetThisTextureLibrary( RSContext& context, RSObject* pThis )
{
	S3DTextureLibrary *	pTxtLib = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pTxtLib = ESLTypeCast<S3DTextureLibrary>( pNativeObj->GetObject() ) ;
	}
	if ( pTxtLib == NULL )
	{
		context.ThrowExceptionError( L"this が TextureLibrary ではありません" ) ;
	}
	return	pTxtLib ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSTextureLibraryClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"TextureLibrary.<init> の this が TextureLibrary ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( new S3DTextureLibrary ) ;
	return	NULL ;
}

// const Image getTextureAs( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSTextureLibraryClass::method_getTextureAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DTextureLibrary *	pTxtLib = GetThisTextureLibrary( context, pThis ) ;
	if ( pTxtLib == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageObject *	pImage = pTxtLib->GetTextureAs( arg.StringAt(0) ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject( pImage, context.GetClassAs( L"Image" ) ) ;
}

// boolean addTextureAs( String id, Image texture )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSTextureLibraryClass::method_addTextureAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DTextureLibrary *	pTxtLib = GetThisTextureLibrary( context, pThis ) ;
	if ( pTxtLib == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageObject *	pImage =
		RSImageClass::ImageFromObject( context, arg.ObjectAt(1) ) ;
	if ( pImage == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean
		( pTxtLib->AddSmartTextureAs
			( arg.StringAt(0), pImage->NewReference() ) == sglErrSuccess ) ;
}

// boolean removeTextureAs( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSTextureLibraryClass::method_removeTextureAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DTextureLibrary *	pTxtLib = GetThisTextureLibrary( context, pThis ) ;
	if ( pTxtLib == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pTxtLib->RemoveTextureAs( arg.StringAt(0) ) == sglErrSuccess ) ;
}

// boolean removeAllTexture()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSTextureLibraryClass::method_removeAllTexture
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DTextureLibrary *	pTxtLib = GetThisTextureLibrary( context, pThis ) ;
	if ( pTxtLib == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean
		( pTxtLib->RemoveAllTexture() == sglErrSuccess ) ;
}


//////////////////////////////////////////////////////////////////////////////
// SurfaceAttribute 構造体
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( Rosetta::RSSurfaceAttributeClass, RSStructuredPointerClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSurfaceAttributeClass::RSSurfaceAttributeClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSStructuredPointerClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSurfaceAttributeClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStructuredPointer( this, NULL ) ;
	//
	RSObject *	pInitShade = context.new_Array() ;
	pInitShade->SetElementIntegerAt( context, 0, 0 ) ;
	pInitShade->SetElementIntegerAt( context, 1, 0 ) ;
	//
	AddArrayMemberAs
		( context, L"flagsShading",
			context.GetBasicTypeClass(RSCodeControl::wiLong) ) ;
	AddArrayMemberAs
		( context, L"colorBase", context.GetClassAs( L"Color3D" ) ) ;
	AddArrayMemberAs
		( context, L"colorShade",
			context.GetClassAs( L"Color3D" ), 0, 0, pInitShade ) ;
	AddArrayMemberAs
		( context, L"nAmbient",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"nDiffusion",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"nSpecular",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"nSpecularSize",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"nTransparency",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"nDeepness",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"nDeepnessPower",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"nReflection",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"fpRefraction",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"nEmission",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"nExFlags",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"cosShadeThreshold",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"fpToonShadeThreshold",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"fpToonShadeBrightness",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"rgbSpecularColor",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"rgbBorderColor",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"fpBorderThicknessA",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"fpBorderThicknessB",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"fpBackLight",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"colorBackLight", context.GetClassAs( L"Color3D" ) ) ;
	AddArrayMemberAs
		( context, L"fpRimLight",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"fpRimLightDeepness",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"rgbRimLightColor",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"nBackDiffusion",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
}


//////////////////////////////////////////////////////////////////////////////
// Material クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSMaterialClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSMaterialClass::RSMaterialClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSMaterialClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	CreateMemberIntegerAs
		( context, L"textureMain", S3DMaterial::textureMain, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"textureSub", S3DMaterial::textureSub, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"textureDiffusion", S3DMaterial::textureDiffusion, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"textureNormal", S3DMaterial::textureNormal, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"textureHeight", S3DMaterial::textureHeight, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"textureLuminous", S3DMaterial::textureLuminous, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"textureEnvironment", S3DMaterial::textureEnvironment, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"textureAlpha", S3DMaterial::textureAlpha, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"textureSpecular", S3DMaterial::textureSpecular, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"textureTypeMask", S3DMaterial::textureTypeMask, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"textureMaxCount", S3DMaterial::textureMaxCount, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"shadingMethodNothing", shadingMethodNothing, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingMethodGouraud", shadingMethodGouraud, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingMethodPhong", shadingMethodPhong, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingMethodToon", shadingMethodToon, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingTextureTiling", shadingTextureTiling, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingTextureTriming", shadingTextureTriming, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingTextureSmoothing", shadingTextureSmoothing, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingTextureMapping", shadingTextureMapping, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingEnvironmentMapping", shadingEnvironmentMapping, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingGEnvironmentMapping", shadingGEnvironmentMapping, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingNormalTexture", shadingNormalTexture, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingLuminousTexture", shadingLuminousTexture, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingAlphaTexture", shadingAlphaTexture, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingHeightTexture", shadingHeightTexture, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingRefractEnvMapping", shadingRefractEnvMapping, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingSpecularMapping", shadingSpecularMapping, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingNormalizedUVScale", shadingNormalizedUVScale, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingAllLocalTextureMask", shadingAllLocalTextureMask, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingSingleSidePlane", shadingSingleSidePlane, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingNoZBuffer", shadingNoZBuffer, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingZBufferNoWrite", shadingZBufferNoWrite, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingDrawOffsetBorder", shadingDrawOffsetBorder, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingMeshSurfaceOffset", shadingMeshSurfaceOffset, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingHintPriority0", shadingHintPriority0, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingHintPriority1", shadingHintPriority1, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingHintPriority2", shadingHintPriority2, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingHintPriority3", shadingHintPriority3, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingHintMask", shadingHintMask, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingHintShifter", shadingHintShifter, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingHintNoZSort", shadingHintNoZSort, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingNoShadowObject", shadingNoShadowObject, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingNoDropShadow", shadingNoDropShadow, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingNoReflectObject", shadingNoReflectObject, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingGlobalReflectObject", shadingGlobalReflectObject, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingNoFogEffect", shadingNoFogEffect, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingVertexAlpha", shadingVertexAlpha, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingMakeBlendAdd", shadingMakeBlendAdd, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingEmisiveTarget", shadingEmisiveTarget, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingNoDrawOffsetBorder", shadingNoDrawOffsetBorder, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingAppExtension1", shadingAppExtension1, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingAppExtension2", shadingAppExtension2, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingAppExtension3", shadingAppExtension3, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingAppExtension4", shadingAppExtension4, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"shadingExVarietyShade", shadingExVarietyShade, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingExBackDiffusion", shadingExBackDiffusion, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingExSpecularColor", shadingExSpecularColor, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingExBorderParam", shadingExBorderParam, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingExBackLight", shadingExBackLight, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shadingExRimLight", shadingExRimLight, modifierConst ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &RSMaterialClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getSurfaceAttribute",
			NULL, L"SurfaceAttribute attr",
			NULL, &RSMaterialClass::method_getSurfaceAttribute,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getBackSurfaceAttribute",
			NULL, L"SurfaceAttribute attr",
			NULL, &RSMaterialClass::method_getBackSurfaceAttribute,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isEnabledBackSurfaceAttribute",
			L"boolean", L"",
			NULL, &RSMaterialClass::method_isEnabledBackSurfaceAttribute,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setSurfaceAttribute",
			NULL, L"SurfaceAttribute attr",
			NULL, &RSMaterialClass::method_setSurfaceAttribute, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setBackSurfaceAttribute",
			NULL, L"SurfaceAttribute attr",
			NULL, &RSMaterialClass::method_setBackSurfaceAttribute, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"enableBackSurfaceAttribute",
			NULL, L"boolean flagBack",
			NULL, &RSMaterialClass::method_enableBackSurfaceAttribute, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getTexture",
			L"Image", L"int iTexture = 0",
			NULL, &RSMaterialClass::method_getTexture,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getBackTexture",
			L"Image", L"int iTexture = 0",
			NULL, &RSMaterialClass::method_getBackTexture,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getTextureFlags",
			L"int", L"int iTexture = 0",
			NULL, &RSMaterialClass::method_getTextureFlags,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getTextureType",
			L"int", L"int iTexture = 0",
			NULL, &RSMaterialClass::method_getTextureType,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getBackTextureFlags",
			L"int", L"int iTexture = 0",
			NULL, &RSMaterialClass::method_getBackTextureFlags,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getBackTextureType",
			L"int", L"int iTexture = 0",
			NULL, &RSMaterialClass::method_getBackTextureType,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getTextureApplication",
			L"float", L"int iTexture = 0",
			NULL, &RSMaterialClass::method_getTextureApplication,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getBackTextureApplication",
			L"float", L"int iTexture = 0",
			NULL, &RSMaterialClass::method_getBackTextureApplication,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getTextureParameter",
			L"float", L"int iTexture = 0",
			NULL, &RSMaterialClass::method_getTextureParameter,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getBackTextureParameter",
			L"float", L"int iTexture = 0",
			NULL, &RSMaterialClass::method_getBackTextureParameter,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"findTextureTypeOf",
			L"int", L"int type",
			NULL, &RSMaterialClass::method_findTextureTypeOf,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"findBackTextureTypeOf",
			L"int", L"int type",
			NULL, &RSMaterialClass::method_findBackTextureTypeOf,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setTexture",
			NULL, L"Image image, int iTexture = 0, "
				L"int nFlags = Material.textureDiffusion, "
				L"float nApply = 1.0, float nParam1 = 0.0",
			NULL, &RSMaterialClass::method_setTexture, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setBackTexture",
			NULL, L"Image image, int iTexture = 0, "
				L"int nFlags = Material.textureDiffusion, "
				L"float nApply = 1.0, float nParam1 = 0.0",
			NULL, &RSMaterialClass::method_setBackTexture, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSMaterialClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<S3DMaterial>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DMaterial *
	RSMaterialClass::GetThisMaterial( RSContext& context, RSObject* pThis )
{
	S3DMaterial *	pMaterial = GetMaterialOf( context, pThis ) ;
	if ( pMaterial == NULL )
	{
		context.ThrowExceptionError( L"this が Material ではありません" ) ;
	}
	return	pMaterial ;
}

SakuraGL::S3DMaterial *
	RSMaterialClass::GetMaterialOf( RSContext& context, RSObject* pObj )
{
	S3DMaterial *		pMaterial = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pObj ) ;
	if ( pNativeObj != NULL )
	{
		pMaterial = ESLTypeCast<S3DMaterial>( pNativeObj->GetObject() ) ;
	}
	return	pMaterial ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"Material.<init> の this が Material ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( new S3DMaterial ) ;
	return	NULL ;
}

// void getSurfaceAttribute( SurfaceAttribute attr )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialClass::method_getSurfaceAttribute
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DMaterial *	pMaterial = GetThisMaterial( context, pThis ) ;
	if ( pMaterial == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t				nBytesAttr ;
	S3DSurfaceAttribute *	psa =
		(S3DSurfaceAttribute*) arg.PointerAt( 0, &nBytesAttr ) ;
	if ( nBytesAttr < sizeof(S3DSurfaceAttribute) )
	{
		context.ThrowExceptionError
			( L"SurfaceAttribute 引数の有効長が不足しています" ) ;
		return	NULL ;
	}
	pMaterial->GetSurfaceAttribute( *psa ) ;
	return	NULL ;
}

// void getBackSurfaceAttribute( SurfaceAttribute attr )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialClass::method_getBackSurfaceAttribute
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DMaterial *	pMaterial = GetThisMaterial( context, pThis ) ;
	if ( pMaterial == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t				nBytesAttr ;
	S3DSurfaceAttribute *	psa =
		(S3DSurfaceAttribute*) arg.PointerAt( 0, &nBytesAttr ) ;
	if ( nBytesAttr < sizeof(S3DSurfaceAttribute) )
	{
		context.ThrowExceptionError
			( L"SurfaceAttribute 引数の有効長が不足しています" ) ;
		return	NULL ;
	}
	pMaterial->GetBackSurfaceAttribute( *psa ) ;
	return	NULL ;
}

// boolean isEnabledBackSurfaceAttribute()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialClass::method_isEnabledBackSurfaceAttribute
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DMaterial *	pMaterial = GetThisMaterial( context, pThis ) ;
	if ( pMaterial == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean
				( pMaterial->IsEnabledBackSurfaceAttribute() ) ;
}

// void setSurfaceAttribute( SurfaceAttribute attr )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialClass::method_setSurfaceAttribute
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DMaterial *	pMaterial = GetThisMaterial( context, pThis ) ;
	if ( pMaterial == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t				nBytesAttr ;
	S3DSurfaceAttribute *	psa =
		(S3DSurfaceAttribute*) arg.PointerAt( 0, &nBytesAttr ) ;
	if ( nBytesAttr < sizeof(S3DSurfaceAttribute) )
	{
		context.ThrowExceptionError
			( L"SurfaceAttribute 引数の有効長が不足しています" ) ;
		return	NULL ;
	}
	pMaterial->SetSurfaceAttribute( *psa ) ;
	return	NULL ;
}

// void setBackSurfaceAttribute( SurfaceAttribute attr )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialClass::method_setBackSurfaceAttribute
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DMaterial *	pMaterial = GetThisMaterial( context, pThis ) ;
	if ( pMaterial == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t				nBytesAttr ;
	S3DSurfaceAttribute *	psa =
		(S3DSurfaceAttribute*) arg.PointerAt( 0, &nBytesAttr ) ;
	if ( nBytesAttr < sizeof(S3DSurfaceAttribute) )
	{
		context.ThrowExceptionError
			( L"SurfaceAttribute 引数の有効長が不足しています" ) ;
		return	NULL ;
	}
	pMaterial->SetBackSurfaceAttribute( *psa ) ;
	return	NULL ;
}

// void enableBackSurfaceAttribute( boolean flagBack )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialClass::method_enableBackSurfaceAttribute
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DMaterial *	pMaterial = GetThisMaterial( context, pThis ) ;
	if ( pMaterial == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pMaterial->EnableBackSurfaceAttribute( arg.BooleanAt( 0 ) ) ;
	return	NULL ;
}

// Image getTexture( int iTexture = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialClass::method_getTexture
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DMaterial *	pMaterial = GetThisMaterial( context, pThis ) ;
	if ( pMaterial == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageObject *	pImage = pMaterial->GetTexture( arg.IntAt(0) ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject( pImage, context.GetClassAs( L"Image" ) ) ;
}

// Image getBackTexture( int iTexture = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialClass::method_getBackTexture
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DMaterial *	pMaterial = GetThisMaterial( context, pThis ) ;
	if ( pMaterial == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageObject *	pImage = pMaterial->GetBackTexture( arg.IntAt(0) ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject( pImage, context.GetClassAs( L"Image" ) ) ;
}

// int getTextureFlags( int iTexture = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialClass::method_getTextureFlags
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DMaterial *	pMaterial = GetThisMaterial( context, pThis ) ;
	if ( pMaterial == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
				( pMaterial->GetTextureFlags( arg.IntAt(0) ) ) ;
}

// int getTextureType( int iTexture = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialClass::method_getTextureType
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DMaterial *	pMaterial = GetThisMaterial( context, pThis ) ;
	if ( pMaterial == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
				( pMaterial->GetTextureType( arg.IntAt(0) ) ) ;
}

// int getBackTextureFlags( int iTexture = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialClass::method_getBackTextureFlags
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DMaterial *	pMaterial = GetThisMaterial( context, pThis ) ;
	if ( pMaterial == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
				( pMaterial->GetBackTextureFlags( arg.IntAt(0) ) ) ;
}

// int getBackTextureType( int iTexture = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialClass::method_getBackTextureType
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DMaterial *	pMaterial = GetThisMaterial( context, pThis ) ;
	if ( pMaterial == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
				( pMaterial->GetBackTextureType( arg.IntAt(0) ) ) ;
}

// float getTextureApplication( int iTexture = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialClass::method_getTextureApplication
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DMaterial *	pMaterial = GetThisMaterial( context, pThis ) ;
	if ( pMaterial == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Number
				( pMaterial->GetTextureApplication( arg.IntAt(0) ) ) ;
}

// float getBackTextureApplication( int iTexture = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialClass::method_getBackTextureApplication
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DMaterial *	pMaterial = GetThisMaterial( context, pThis ) ;
	if ( pMaterial == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Number
				( pMaterial->GetBackTextureApplication( arg.IntAt(0) ) ) ;
}

// float getTextureParameter( int iTexture = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialClass::method_getTextureParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DMaterial *	pMaterial = GetThisMaterial( context, pThis ) ;
	if ( pMaterial == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Number
				( pMaterial->GetTextureParameter( arg.IntAt(0) ) ) ;
}

// float getBackTextureParameter( int iTexture = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialClass::method_getBackTextureParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DMaterial *	pMaterial = GetThisMaterial( context, pThis ) ;
	if ( pMaterial == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Number
				( pMaterial->GetBackTextureParameter( arg.IntAt(0) ) ) ;
}

// int findTextureTypeOf( int type )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialClass::method_findTextureTypeOf
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DMaterial *	pMaterial = GetThisMaterial( context, pThis ) ;
	if ( pMaterial == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
				( pMaterial->FindTextureTypeOf( arg.IntAt(0) ) ) ;
}

// int findBackTextureTypeOf( int type )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialClass::method_findBackTextureTypeOf
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DMaterial *	pMaterial = GetThisMaterial( context, pThis ) ;
	if ( pMaterial == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
				( pMaterial->FindBackTextureTypeOf( arg.IntAt(0) ) ) ;
}

// void setTexture
//	( Image image, int iTexture = 0,
//		int nFlags = Material.textureDiffusion,
//		float nApply = 1.0, float nParam1 = 0.0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialClass::method_setTexture
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DMaterial *	pMaterial = GetThisMaterial( context, pThis ) ;
	if ( pMaterial == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageObject *	pImage =
		RSImageClass::ImageFromObject( context, arg.ObjectAt(0) ) ;
	pMaterial->SetTexture
		( pImage, arg.IntAt(1), (uint32_t) arg.IntAt(2),
			(float32_t) arg.DoubleAt( 3, 1.0 ),
			(float32_t) arg.DoubleAt( 4, 0.0 ) ) ;
	return	NULL ;
}

// void setBackTexture
//	( Image image, int iTexture = 0,
//		int nFlags = Material.textureDiffusion,
//		float nApply = 1.0, float nParam1 = 0.0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialClass::method_setBackTexture
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DMaterial *	pMaterial = GetThisMaterial( context, pThis ) ;
	if ( pMaterial == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageObject *	pImage =
		RSImageClass::ImageFromObject( context, arg.ObjectAt(0) ) ;
	pMaterial->SetBackTexture
		( pImage, arg.IntAt(1), (uint32_t) arg.IntAt(2),
			(float32_t) arg.DoubleAt( 3, 1.0 ),
			(float32_t) arg.DoubleAt( 4, 0.0 ) ) ;
	return	NULL ;
}



//////////////////////////////////////////////////////////////////////////////
// ModelPose クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSModelPoseClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSModelPoseClass::RSModelPoseClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSModelPoseClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( nullptr, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", nullptr, L"",
			nullptr, &RSModelPoseClass::method_init,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getAnimationTime", L"int", L"",
			nullptr, &RSModelPoseClass::method_getAnimationTime,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getFramePerSec", L"double", L"",
			nullptr, &RSModelPoseClass::method_getFramePerSec,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"applyPoseTo", nullptr,
			L"ModelBuffer model, double w = 1.0, double sec = 0.0",
			nullptr, &RSModelPoseClass::method_applyPoseTo,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"productPoseTo", nullptr,
			L"ModelBuffer model, double w = 1.0, double sec = 0.0",
			nullptr, &RSModelPoseClass::method_productPoseTo,
			nullptr, RSFunctionPrototype::flagConstant ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSModelPoseClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<S3DModelPose>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DModelPose *
	RSModelPoseClass::GetThisModelPose( RSContext& context, RSObject* pThis )
{
	return	RSNativeObject::GetNative<S3DModelPose>( pThis ) ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSModelPoseClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"ModelPose.<init> の this が ModelPose ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( new S3DMaterialLibrary ) ;
	return	nullptr ;
}

// int getAnimationTime()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSModelPoseClass::method_getAnimationTime
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DModelPose *	pPose = GetThisModelPose( context, pThis ) ;
	if ( pPose == nullptr )
	{
		return	context.new_Integer( 0 ) ;
	}
	return	context.new_Integer( pPose->GetMetaInfo().msecDuration ) ;
}

// double getFramePerSec()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSModelPoseClass::method_getFramePerSec
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DModelPose *	pPose = GetThisModelPose( context, pThis ) ;
	if ( pPose == nullptr )
	{
		return	context.new_Number( 30.0 ) ;
	}
	return	context.new_Number
				( (double) pPose->GetMetaInfo().fxFrameRatio / 0x10000 ) ;
}

// void applyPoseTo( ModelBuffer model, double w = 1.0, double sec = 0.0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSModelPoseClass::method_applyPoseTo
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DModelPose *	pPose = GetThisModelPose( context, pThis ) ;
	if ( pPose == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DModelBuffer *	pModel =
		ESLTypeCast<S3DModelBuffer>( arg.NativeObjectAt( 0 ) ) ;
	if ( pModel == nullptr )
	{
		return	nullptr ;
	}
	pPose->ApplyPoseTo
		( *pModel, arg.DoubleAt( 1, 1.0 ), arg.DoubleAt( 2, 0.0 ) ) ;
	return	nullptr ;
}

// void productPoseTo( ModelBuffer model, double w = 1.0, double sec = 0.0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSModelPoseClass::method_productPoseTo
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DModelPose *	pPose = GetThisModelPose( context, pThis ) ;
	if ( pPose == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DModelBuffer *	pModel =
		ESLTypeCast<S3DModelBuffer>( arg.NativeObjectAt( 0 ) ) ;
	if ( pModel == nullptr )
	{
		return	nullptr ;
	}
	pPose->ProductPoseTo
		( *pModel, arg.DoubleAt( 1, 1.0 ), arg.DoubleAt( 2, 0.0 ) ) ;
	return	nullptr ;
}



//////////////////////////////////////////////////////////////////////////////
// ModelPoseLibrary クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSModelPoseLibraryClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSModelPoseLibraryClass::RSModelPoseLibraryClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSModelPoseLibraryClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( nullptr, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", nullptr, L"",
			nullptr, &RSModelPoseLibraryClass::method_init, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"loadLibrary", L"boolean", L"String file",
			nullptr, &RSModelPoseLibraryClass::method_loadLibrary, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readLibrary", L"boolean", L"InputStream is",
			nullptr, &RSModelPoseLibraryClass::method_readLibrary, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getPoseAs", L"ModelPose", L"String id",
			nullptr, &RSModelPoseLibraryClass::method_getPoseAs,
			nullptr, RSFunctionPrototype::flagConstant ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSModelPoseLibraryClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<S3DModelPoseLibrary>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DModelPoseLibrary *
	RSModelPoseLibraryClass::GetThisPoseLibrary( RSContext& context, RSObject* pThis )
{
	return	RSNativeObject::GetNative<S3DModelPoseLibrary>( pThis ) ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSModelPoseLibraryClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"ModelPoseLibrary.<init> の this が ModelPoseLibrary ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( new S3DMaterialLibrary ) ;
	return	nullptr ;
}

// boolean loadLibrary( String file )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSModelPoseLibraryClass::method_loadLibrary
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DModelPoseLibrary *	pPoseLib = GetThisPoseLibrary( context, pThis ) ;
	if ( pPoseLib == nullptr )
	{
		context.ThrowExceptionError
			( L"ModelPoseLibrary.loadLibrary の this が null です",
										L"NullPointerException" ) ;
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	if ( pPoseLib->LoadLibrary( arg.StringAt(0) ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean readLibrary( InputStream is )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSModelPoseLibraryClass::method_readLibrary
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DModelPoseLibrary *	pPoseLib = GetThisPoseLibrary( context, pThis ) ;
	if ( pPoseLib == nullptr )
	{
		context.ThrowExceptionError
			( L"ModelPoseLibrary.readLibrary の this が null です",
										L"NullPointerException" ) ;
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SFileInterface *	pFile = ESLTypeCast<SFileInterface>( arg.ObjectAt(0) ) ;
	if ( pFile == nullptr )
	{
		return	context.new_Boolean( false ) ;
	}
	if ( pPoseLib->ReadLibrary( *pFile ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// ModelPose getPoseAs( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSModelPoseLibraryClass::method_getPoseAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DModelPoseLibrary *	pPoseLib = GetThisPoseLibrary( context, pThis ) ;
	if ( pPoseLib == nullptr )
	{
		context.ThrowExceptionError
			( L"ModelPoseLibrary.getPoseAs の this が null です",
										L"NullPointerException" ) ;
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DModelPose *	pPose = pPoseLib->GetPoseAs( arg.StringAt(0) ) ;
	if ( pPose == nullptr )
	{
		return	nullptr ;
	}
	return	new RSNativeObject( pPose, context.GetClassAs( L"ModelPose" ) ) ;
}



//////////////////////////////////////////////////////////////////////////////
// MaterialLibrary クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSMaterialLibraryClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSMaterialLibraryClass::RSMaterialLibraryClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSMaterialLibraryClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &RSMaterialLibraryClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getMaterialAs",
			L"Material", L"String id",
			NULL, &RSMaterialLibraryClass::method_getMaterialAs,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"addMaterialAs",
			L"boolean", L"String id, Material material",
			NULL, &RSMaterialLibraryClass::method_addMaterialAs, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"removeMaterialAs",
			L"boolean", L"String id",
			NULL, &RSMaterialLibraryClass::method_removeMaterialAs, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"removeAllMaterial",
			L"boolean", L"",
			NULL, &RSMaterialLibraryClass::method_removeAllMaterial, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSMaterialLibraryClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<S3DMaterialLibrary>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DMaterialLibrary *
	RSMaterialLibraryClass::GetThisMaterialLibrary( RSContext& context, RSObject* pThis )
{
	S3DMaterialLibrary *	pMatLib = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pMatLib = ESLTypeCast<S3DMaterialLibrary>( pNativeObj->GetObject() ) ;
	}
	if ( pMatLib == NULL )
	{
		context.ThrowExceptionError( L"this が MaterialLibrary ではありません" ) ;
	}
	return	pMatLib ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialLibraryClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"MaterialLibrary.<init> の this が MaterialLibrary ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( new S3DMaterialLibrary ) ;
	return	NULL ;
}

// Material getMaterialAs( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialLibraryClass::method_getMaterialAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DMaterialLibrary *	pMatLib = GetThisMaterialLibrary( context, pThis ) ;
	if ( pMatLib == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DMaterial *	pMaterial = pMatLib->GetMaterialAs( arg.StringAt(0) ) ;
	if ( pMaterial == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject( pMaterial, context.GetClassAs( L"Material" ) ) ;
}

// boolean addMaterialAs( String id, Material material )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialLibraryClass::method_addMaterialAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DMaterialLibrary *	pMatLib = GetThisMaterialLibrary( context, pThis ) ;
	if ( pMatLib == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSNativeObject *
		pNObjMaterial = ESLTypeCast<RSNativeObject>( arg.ObjectAt(1) ) ;
	if ( pNObjMaterial == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	S3DMaterial *
		pMaterial = ESLTypeCast<S3DMaterial>( pNObjMaterial->GetObject() ) ;
	if ( pMaterial == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	if ( pNObjMaterial->IsObjectOwner() )
	{
		pNObjMaterial->DetachObject() ;
		pNObjMaterial->AttachObject( pMaterial ) ;
		return	context.new_Boolean
			( pMatLib->AddSmartMaterialAs
				( arg.StringAt(0), pMaterial ) == sglErrSuccess ) ;
	}
	else
	{
		return	context.new_Boolean
			( pMatLib->AddMaterialAs
				( arg.StringAt(0), pMaterial ) == sglErrSuccess ) ;
	}
}

// boolean removeMaterialAs( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialLibraryClass::method_removeMaterialAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DMaterialLibrary *	pMatLib = GetThisMaterialLibrary( context, pThis ) ;
	if ( pMatLib == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pMatLib->RemoveMaterialAs( arg.StringAt(0) ) == sglErrSuccess ) ;
}

// boolean removeAllMaterial()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMaterialLibraryClass::method_removeAllMaterial
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DMaterialLibrary *	pMatLib = GetThisMaterialLibrary( context, pThis ) ;
	if ( pMatLib == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean
		( pMatLib->RemoveAllMaterial() == sglErrSuccess ) ;
}


//////////////////////////////////////////////////////////////////////////////
// CustomShader クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSCustomShaderClass, RGenericNativeObjectClass )
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSCustomShaderClass::RSCustomShaderClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSCustomShaderClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	CreateMemberIntegerAs
		( context, L"uniformInt",
			S3DCustomShader::uniformInt, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"uniformFloat",
			S3DCustomShader::uniformFloat, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"uniformVector2D",
			S3DCustomShader::uniformVector2D, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"uniformVector3D",
			S3DCustomShader::uniformVector3D, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"uniformVector4D",
			S3DCustomShader::uniformVector4D, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"uniformMatrix2x2",
			S3DCustomShader::uniformMatrix2x2, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"uniformMatrix3x3",
			S3DCustomShader::uniformMatrix3x3, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"uniformMatrix4x4",
			S3DCustomShader::uniformMatrix4x4, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"uniformTexture",
			S3DCustomShader::uniformTexture, modifierConst ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSCustomShaderClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<S3DCustomShader>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DCustomShader *
	RSCustomShaderClass::GetThisCustomShader( RSContext& context, RSObject* pThis )
{
	return	RSNativeObject::GetNative<S3DCustomShader>( pThis ) ;
}



//////////////////////////////////////////////////////////////////////////////
// RenderDevice.ShaderDesc クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( Rosetta::RSRenderDeviceClass::ShaderDescClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSRenderDeviceClass::ShaderDescClass::ShaderDescClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSRenderDeviceClass::ShaderDescClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &ShaderDescClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"loadDescriptor",
			L"boolean", L"String strDescFile",
			NULL, &ShaderDescClass::method_loadDescriptor, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"parseDescriptor",
			L"boolean", L"Uint8Pointer ptrDescXml",
			NULL, &ShaderDescClass::method_parseDescriptor, NULL ) ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderDeviceClass::ShaderDescClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"RenderDeivce.ShaderDesc.<init> の this が ShaderDesc ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( new S3DRenderDevice::ShaderDescriptor ) ;
	return	NULL ;
}

// boolean loadDescriptor( String strDescFile )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderDeviceClass::ShaderDescClass::method_loadDescriptor
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderDevice::ShaderDescriptor *	pShdDesc =
		RSNativeObject::GetNative<S3DRenderDevice::ShaderDescriptor>( pThis ) ;
	if ( pShdDesc == NULL )
	{
		context.ThrowExceptionError
			( L"RenderDeivce.ShaderDesc.loadDescriptor の this が null です" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;

	SXMLDocument	xmlDoc ;
	if ( xmlDoc.LoadDocument( arg.StringAt(0), xmlDoc ) )
	{
		return	context.new_Boolean( false ) ;
	}
	SXMLDocument *	pxmlShader = xmlDoc.GetElementTagAs( L"shader" ) ;
	if ( pxmlShader == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	if ( pShdDesc->ParseDescriptor( *pxmlShader ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean parseDescriptor( Uint8Pointer ptrDescXml )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderDeviceClass::ShaderDescClass::method_parseDescriptor
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderDevice::ShaderDescriptor *	pShdDesc =
		RSNativeObject::GetNative<S3DRenderDevice::ShaderDescriptor>( pThis ) ;
	if ( pShdDesc == NULL )
	{
		context.ThrowExceptionError
			( L"RenderDeivce.ShaderDesc.parseDescriptor の this が null です" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t		nXmlBytes = 0 ;
	uint8_t *	pDescXmlBin = arg.PointerAt( 0, &nXmlBytes ) ;
	if ( pDescXmlBin == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SMemoryReferenceFile	memfile ;
	memfile.AttachMemory( pDescXmlBin, nXmlBytes ) ;

	SXMLDocument	xmlDoc ;
	if ( xmlDoc.ReadDocument( memfile, xmlDoc ) )
	{
		return	context.new_Boolean( false ) ;
	}
	SXMLDocument *	pxmlShader = xmlDoc.GetElementTagAs( L"shader" ) ;
	if ( pxmlShader == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	if ( pShdDesc->ParseDescriptor( *pxmlShader ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}



//////////////////////////////////////////////////////////////////////////////
// RenderDevice.DefaultShaderId クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO
	( Rosetta::RSRenderDeviceClass::DefaultShaderIdClass, RSClass )

// 構築関数
RSRenderDeviceClass::DefaultShaderIdClass::DefaultShaderIdClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
void RSRenderDeviceClass::DefaultShaderIdClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSGenericObject( this ) ;
	//
	CreateMemberStringAs
		( context, L"NonShading",
			S3DRenderDevice::DefaultShaderId::NonShading, modifierConst ) ;
	CreateMemberStringAs
		( context, L"Gouraud",
			S3DRenderDevice::DefaultShaderId::Gouraud, modifierConst ) ;
	CreateMemberStringAs
		( context, L"Phong",
			S3DRenderDevice::DefaultShaderId::Phong, modifierConst ) ;
}



//////////////////////////////////////////////////////////////////////////////
// RenderDevice クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSRenderDeviceClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSRenderDeviceClass::RSRenderDeviceClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSRenderDeviceClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	ShaderDescClass *
		pShaderDescClass =
			new ShaderDescClass( context.GetClassClass(), L"ShaderDesc" ) ;
	pShaderDescClass->Initialize( context ) ;
	pShaderDescClass->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"ShaderDesc", pShaderDescClass ) ) ;
	//
	DefaultShaderIdClass *
		pDefShaderIdClass =
			new DefaultShaderIdClass( context.GetClassClass(), L"DefaultShaderId" ) ;
	pDefShaderIdClass->Initialize( context ) ;
	pDefShaderIdClass->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"DefaultShaderId", pDefShaderIdClass ) ) ;
	//
	RSExceptionClass *
		pShaderException =
			new RSExceptionClass( context.GetClassClass(), L"BuildShaderException" ) ;
	pShaderException->AddSuperClass( context, context.GetExceptionClass() ) ;
	pShaderException->Initialize( context ) ;
	pShaderException->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"BuildShaderException", pShaderException ) ) ;
	//
	CreateMemberIntegerAs
		( context, L"procedureAsync", S3DRenderDevice::procedureAsync, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"procedureSync", S3DRenderDevice::procedureSync, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"procedureDelayable", S3DRenderDevice::procedureDelayable, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"procedureLater", S3DRenderDevice::procedureLater, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"procedureNoRender", S3DRenderDevice::procedureNoRender, modifierConst ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"isOnRenderThread", L"boolean", L"",
			NULL, &RSRenderDeviceClass::method_isOnRenderThread,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"procedure", L"boolean",
			L"Runnable proc, int priority",
			NULL, &RSRenderDeviceClass::method_procedure, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"waitUntilAsyncAllProcedures",
			L"boolean", L"long timeout",
			NULL, &RSRenderDeviceClass::method_waitUntilAsyncAllProcedures,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"newRenderer", L"RenderContext", L"",
			NULL, &RSRenderDeviceClass::method_newRenderer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"commitDeviceImage",
			L"boolean", L"Image img, long timeout = 0",
			NULL, &RSRenderDeviceClass::method_commitDeviceImage, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"commitDeviceVertexBuffer",
			L"boolean", L"VertexBuffer vb, long timeout = 0",
			NULL, &RSRenderDeviceClass::method_commitDeviceVertexBuffer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"releaseDeviceImage",
			L"boolean", L"Image img, long timeout = 0",
			NULL, &RSRenderDeviceClass::method_releaseDeviceImage, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"releaseDeviceVertexBuffer",
			L"boolean", L"VertexBuffer vb, long timeout = 0",
			NULL, &RSRenderDeviceClass::method_releaseDeviceVertexBuffer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getCustomShaderAs",
			L"CustomShader", L"String idShader",
			NULL, &RSRenderDeviceClass::method_getCustomShaderAs, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"buildCustomShader",
			L"CustomShader", L"String idShader, RenderDevice.ShaderDesc shddsc",
			NULL, &RSRenderDeviceClass::method_buildCustomShader, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"removeCustomShaderAs",
			NULL, L"String idShader",
			NULL, &RSRenderDeviceClass::method_removeCustomShaderAs, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSRenderDeviceClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<S3DRenderDevice>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DRenderDevice *
	RSRenderDeviceClass::GetThisRenderDevice( RSContext& context, RSObject* pThis )
{
	return	RSNativeObject::GetNative<S3DRenderDevice>( pThis ) ;
}

// const boolean isOnRenderThread()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderDeviceClass::method_isOnRenderThread
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderDevice *	pDevice = GetThisRenderDevice( context, pThis ) ;
	return	context.new_Boolean( (pDevice != NULL) && pDevice->IsOnRenderThread() ) ;
}

// boolean procedure( Runnable proc, int priority )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderDeviceClass::method_procedure
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderDevice *	pDevice = GetThisRenderDevice( context, pThis ) ;
	if ( pDevice == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;

	RSThread::RunnableProcedure *	pRunProc =
		new RSThread::RunnableProcedure
			( context.GetVM(),
				ESLTypeCast<RSThread>( context.GetThreadObject() ),
				arg.ObjectAt(0), true ) ;
	if ( pDevice->Procedure
		( pRunProc, (S3DRenderDevice::ProcedurePriority) arg.IntAt(1) ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean waitUntilAsyncAllProcedures( long timeout )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderDeviceClass::method_waitUntilAsyncAllProcedures
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderDevice *	pDevice = GetThisRenderDevice( context, pThis ) ;
	if ( pDevice == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	if ( pDevice->WaitUntilAsyncAllProcedures( arg.LongAt(0) ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// RenderContext newRenderer()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderDeviceClass::method_newRenderer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderDevice *	pDevice = GetThisRenderDevice( context, pThis ) ;
	if ( pDevice == NULL )
	{
		return	NULL ;
	}
	S3DRenderContextInterface *	pRender = pDevice->NewRenderer() ;
	if ( pRender == NULL )
	{
		return	NULL ;
	}
	RSNativeObject *	pNativeObj =
		new RSNativeObject( NULL, context.GetClassAs( L"RenderContext" ) ) ;
	pNativeObj->SetObject( (SGLPaintContextInterface*) pRender ) ;
	return	pNativeObj ;
}

// boolean commitDeviceImage( Image img, long timeout = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderDeviceClass::method_commitDeviceImage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderDevice *	pDevice = GetThisRenderDevice( context, pThis ) ;
	if ( pDevice == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageObject *	pImage = ESLTypeCast<SGLImageObject>( arg.NativeObjectAt(0) ) ;
	if ( pImage == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	if ( pDevice->CommitDeviceImage( pImage, arg.LongAt(1) ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean commitDeviceVertexBuffer( VertexBuffer vb, long timeout = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderDeviceClass::method_commitDeviceVertexBuffer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderDevice *	pDevice = GetThisRenderDevice( context, pThis ) ;
	if ( pDevice == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVertexBufferInterface *	pVB =
		ESLTypeCast<S3DVertexBufferInterface>( arg.NativeObjectAt(0) ) ;
	if ( pVB == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	if ( pDevice->CommitDeviceVertexBuffer( pVB, arg.LongAt(1) ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean releaseDeviceImage( Image img, long timeout = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderDeviceClass::method_releaseDeviceImage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderDevice *	pDevice = GetThisRenderDevice( context, pThis ) ;
	if ( pDevice == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageObject *	pImage = ESLTypeCast<SGLImageObject>( arg.NativeObjectAt(0) ) ;
	if ( pImage == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	if ( pDevice->ReleaseDeviceImage( pImage, arg.LongAt(1) ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean releaseDeviceVertexBuffer( VertexBuffer vb, long timeout = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderDeviceClass::method_releaseDeviceVertexBuffer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderDevice *	pDevice = GetThisRenderDevice( context, pThis ) ;
	if ( pDevice == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVertexBufferInterface *	pVB =
		ESLTypeCast<S3DVertexBufferInterface>( arg.NativeObjectAt(0) ) ;
	if ( pVB == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	if ( pDevice->ReleaseDeviceVertexBuffer( pVB, arg.LongAt(1) ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// CustomShader getCustomShaderAs( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderDeviceClass::method_getCustomShaderAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderDevice *	pDevice = GetThisRenderDevice( context, pThis ) ;
	if ( pDevice == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DCustomShader *
		pShader = pDevice->GetShaderProgramAs( arg.StringAt(0) ) ;
	if ( pShader == NULL )
	{
		pShader = pDevice->GetDefaultShaderProgramAs( arg.StringAt(0) ) ;
		if ( pShader == NULL )
		{
			return	NULL ;
		}
	}
	return	new RSNativeObject( pShader, context.GetClassAs( L"CustomShader" ) ) ;
}

// CustomShader buildCustomShader( String id, RenderDevice.ShaderDesc shddsc )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderDeviceClass::method_buildCustomShader
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderDevice *	pDevice = GetThisRenderDevice( context, pThis ) ;
	if ( pDevice == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DRenderDevice::ShaderDescriptor *	pShdDesc =
		ESLTypeCast<S3DRenderDevice::ShaderDescriptor>( arg.NativeObjectAt(1) ) ;
	//
	SString	strErrMsg ;
	S3DCustomShader *
		pShader = pDevice->BuildCustomShader
					( arg.StringAt(0), pShdDesc, nullptr, &strErrMsg, nullptr ) ;
	if ( pShader == nullptr )
	{
		RSObject *	pException =
			context.new_Exception
				( strErrMsg,
					context.GetClassAs( L"RenderDevice.BuildShaderException" ) ) ;
		context.SetException( pException ) ;
		return	nullptr ;
	}
	return	new RSNativeObject( pShader, context.GetClassAs( L"CustomShader" ) ) ;
}

// void removeCustomShaderAs( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderDeviceClass::method_removeCustomShaderAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderDevice *	pDevice = GetThisRenderDevice( context, pThis ) ;
	if ( pDevice == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pDevice->RemoveCustomShaderAs( arg.StringAt(0) ) ;
	return	NULL ;
}



//////////////////////////////////////////////////////////////////////////////
// RenderBuffer クラス
//////////////////////////////////////////////////////////////////////////////

// RenderBuffer.EnvMappingParam クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( Rosetta::RSRenderBufferClass::EnvMappingParamClass, RSStructuredPointerClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSRenderBufferClass::EnvMappingParamClass::EnvMappingParamClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSStructuredPointerClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSRenderBufferClass::EnvMappingParamClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStructuredPointer( this, NULL ) ;
	//
	RenderContext::EnvMappingParam	emp ;
	AddArrayMemberAs
		( context, L"pImage",
			context.GetIntClassSizeOf(sizeof(emp.pImage)) ) ;
	AddArrayMemberAs
		( context, L"typeMap",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"matMap",
			context.GetClassAs(L"Matrix3D") ) ;
}

// Object -> RenderContext::EnvMappingParam 変換
//////////////////////////////////////////////////////////////////////////////
void RSRenderBufferClass::EnvMappingParamClass::FromObject
	( RSContext& context,
		RenderContext::EnvMappingParam& param, RSObject * pObj )
{
	param.pImage = reinterpret_cast<SGLImageObject*>
						( pObj->GetMemberIntegerAs( context, L"pImage" ) ) ;
	param.typeMap = (uint32_t) pObj->GetMemberIntegerAs( context, L"typeMap" ) ;
	//
	float32_t *	pMat3x3 =
		(float32_t*) pObj->GetMemberNativePtrAs
						( context, L"matMap", sizeof(float32_t) * 3 * 3 ) ;
	if ( pMat3x3 != NULL )
	{
		for ( int i = 0; i < 3; i ++ )
		{
			param.matMap.m[i][0] = pMat3x3[i * 3] ;
			param.matMap.m[i][1] = pMat3x3[i * 3 + 1] ;
			param.matMap.m[i][2] = pMat3x3[i * 3 + 2] ;
		}
	}
}

// Object <- RenderContext::EnvMappingParam 変換
//////////////////////////////////////////////////////////////////////////////
void RSRenderBufferClass::EnvMappingParamClass::ToObject
	( RSContext& context,
		RSObject * pObj, const RenderContext::EnvMappingParam& param )
{
	pObj->SetMemberIntegerAs
		( context, L"pImage", reinterpret_cast<int64_t>( param.pImage ) ) ;
	pObj->SetMemberIntegerAs( context, L"typeMap", param.typeMap ) ;
	//
	float32_t *	pMat3x3 =
		(float32_t*) pObj->GetMemberNativePtrAs
						( context, L"matMap", sizeof(float32_t) * 3 * 3 ) ;
	if ( pMat3x3 != NULL )
	{
		for ( int i = 0; i < 3; i ++ )
		{
			pMat3x3[i * 3]     = param.matMap.m[i][0] ;
			pMat3x3[i * 3 + 1] = param.matMap.m[i][1] ;
			pMat3x3[i * 3 + 2] = param.matMap.m[i][2] ;
		}
	}
}



// RenderBuffer.OffsetBorderParam クラス情報
// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSRenderBufferClass::OffsetBorderParamClass, RSStructuredPointerClass )

// RenderBuffer.OffsetBorderParam 構築関数
//////////////////////////////////////////////////////////////////////////////
RSRenderBufferClass::OffsetBorderParamClass::OffsetBorderParamClass
	( RSClass * pClass, const wchar_t * pwszClassName )
	: RSStructuredPointerClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSRenderBufferClass::OffsetBorderParamClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStructuredPointer( this, NULL ) ;
	//
	AddArrayMemberAs
		( context, L"rgbBorder",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"aThickness",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"bThickness",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
}


// RenderBuffer.OptionalContextSetクラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSRenderBufferClass::OptionalContextSetClass, RSStructuredPointerClass )

// RenderBuffer.OptionalContextSet 構築関数
//////////////////////////////////////////////////////////////////////////////
RSRenderBufferClass::OptionalContextSetClass::OptionalContextSetClass
	( RSClass * pClass, const wchar_t * pwszClassName )
	: RSStructuredPointerClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSRenderBufferClass::OptionalContextSetClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStructuredPointer( this, NULL ) ;
	//
	S3DRenderBufferInterface::OptionalContextSet	optcs ;
	AddArrayMemberAs
		( context, L"nOptionMask",
			context.GetIntClassSizeOf(sizeof(optcs.nOptionMask)) ) ;
	AddArrayMemberAs
		( context, L"nShadingFlags",
			context.GetIntClassSizeOf(sizeof(optcs.nShadingFlags)) ) ;
	AddArrayMemberAs
		( context, L"pShader",
			context.GetIntClassSizeOf(sizeof(optcs.pShader)) ) ;
	AddArrayMemberAs
		( context, L"opbBorder",
			context.GetClassAs(L"RenderBuffer::OffsetBorderParam") ) ;
	AddArrayMemberAs
		( context, L"faceCulling",
			context.GetIntClassSizeOf(sizeof(optcs.faceCulling)) ) ;
	AddArrayMemberAs
		( context, L"depthMask",
			context.GetIntClassSizeOf(sizeof(optcs.depthMask)) ) ;
	AddArrayMemberAs
		( context, L"blendOp",
			context.GetIntClassSizeOf(sizeof(optcs.blendOp)) ) ;
	AddArrayMemberAs
		( context, L"pointSize",
			context.GetFloatClassSizeOf(sizeof(optcs.pointSize)) ) ;
	AddArrayMemberAs
		( context, L"lineWidth",
			context.GetFloatClassSizeOf(sizeof(optcs.lineWidth)) ) ;
}


// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSRenderBufferClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSRenderBufferClass::RSRenderBufferClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSRenderBufferClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	EnvMappingParamClass *
		pEndMappingParam =
			new EnvMappingParamClass
				( context.GetClassClass(), L"EnvMappingParam" ) ;
	pEndMappingParam->Initialize( context ) ;
	pEndMappingParam->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"EnvMappingParam", pEndMappingParam ) ) ;
	//
	OffsetBorderParamClass *
		pOffsetBorderParam =
			new OffsetBorderParamClass
				( context.GetClassClass(), L"OffsetBorderParam" ) ;
	pOffsetBorderParam->Initialize( context ) ;
	pOffsetBorderParam->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"OffsetBorderParam", pOffsetBorderParam ) ) ;
	//
	OptionalContextSetClass *
		pOptionalContextSet =
			new OptionalContextSetClass
				( context.GetClassClass(), L"OptionalContextSet" ) ;
	pOptionalContextSet->Initialize( context ) ;
	pOptionalContextSet->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"OptionalContextSet", pOptionalContextSet ) ) ;
	//
	CreateMemberIntegerAs
		( context, L"featureSRGB",
			S3DRenderBufferInterface::featureSRGB, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"featureEnvMap",
			S3DRenderBufferInterface::featureEnvMap, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"featureOffsetBorder",
			S3DRenderBufferInterface::featureOffsetBorder, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"featureFaceCulling",
			S3DRenderBufferInterface::featureFaceCulling, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"featureDepthMask",
			S3DRenderBufferInterface::featureDepthMask, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"featureContextSet",
			S3DRenderBufferInterface::featureContextSet, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"featureOrderPriority",
			S3DRenderBufferInterface::featureOrderPriority, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"faceCullingDefault",
			S3DRenderBufferInterface::faceCullingDefault, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"faceCullingBack",
			S3DRenderBufferInterface::faceCullingBack, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"faceCullingFront",
			S3DRenderBufferInterface::faceCullingFront, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"faceCullingNo",
			S3DRenderBufferInterface::faceCullingNo, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"depthMaskDefault",
			S3DRenderBufferInterface::depthMaskDefault, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"depthMaskEnable",
			S3DRenderBufferInterface::depthMaskEnable, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"depthMaskNoWrite",
			S3DRenderBufferInterface::depthMaskNoWrite, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"depthMaskNoWriteGT",
			S3DRenderBufferInterface::depthMaskNoWriteGT, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"depthMaskNoTest",
			S3DRenderBufferInterface::depthMaskNoTest, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"blendDefault",
			S3DRenderBufferInterface::blendDefault, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"blendProductedSrc",
			S3DRenderBufferInterface::blendProductedSrc, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"blendAdd",
			S3DRenderBufferInterface::blendAdd, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"blendUnproductedSrc",
			S3DRenderBufferInterface::blendUnproductedSrc, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"blendCopy",
			S3DRenderBufferInterface::blendCopy, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"blendDstMasked",
			S3DRenderBufferInterface::blendDstMasked, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"blendMulColor",
			S3DRenderBufferInterface::blendMulColor, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"envMappingHemisphere",
			S3DRenderContextInterface::envMappingHemisphere, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"envMappingSphere",
			S3DRenderContextInterface::envMappingSphere, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"envMappingCube",
			S3DRenderContextInterface::envMappingCube, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"envMappingViewport",
			S3DRenderContextInterface::envMappingViewport, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"envMappingTypeMask",
			S3DRenderContextInterface::envMappingTypeMask, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"envMappingRefraction",
			S3DRenderContextInterface::envMappingRefraction, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"optionShadingFlag",
			S3DRenderBufferInterface::optionShadingFlag, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"optionCustomShader",
			S3DRenderBufferInterface::optionCustomShader, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"optionBorderParam",
			S3DRenderBufferInterface::optionBorderParam, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"optionFaceCulling",
			S3DRenderBufferInterface::optionFaceCulling, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"optionDepthMask",
			S3DRenderBufferInterface::optionDepthMask, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"optionContextAll",
			S3DRenderBufferInterface::optionContextAll, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"primitivePoint", primitivePoint, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"primitiveLine", primitiveLine, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"primitiveLineStrip", primitiveLineStrip, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"primitiveTriangle", primitiveTriangle, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"primitiveTriangleStrip", primitiveTriangleStrip, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"renderFenceOrder",
			S3DRenderBufferInterface::renderFenceOrder, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"renderAutoNormal",
			S3DRenderBuffer::renderAutoNormal, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"renderAutoColor",
			S3DRenderBuffer::renderAutoColor, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"renderAutoTexAxis",
			S3DRenderBuffer::renderAutoTexAxis, modifierConst ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &RSRenderBufferClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setShadingFlag",
			NULL, L"long nShadingMethod",
			NULL, &RSRenderBufferClass::method_setShadingFlag, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getShadingFlag", L"long", L"",
			NULL, &RSRenderBufferClass::method_getShadingFlag,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"appendMatrixTransformation",
			NULL, L"Matrix3D mat, Vector3D pos, "
				L"Color3D color = null, int transparency = 0",
			NULL, &RSRenderBufferClass::method_appendMatrixTransformation, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setMatrixTransformation",
			NULL, L"Matrix3D mat, Vector3D pos, "
				L"Color3D color = null, int transparency = 0",
			NULL, &RSRenderBufferClass::method_setMatrixTransformation, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getMatrixTransformation",
			NULL, L"Matrix3D mat, Vector3D pos, "
				L"Color3D color, Uint32Pointer transparency",
			NULL, &RSRenderBufferClass::method_getMatrixTransformation,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"pushTransformation", NULL, L"",
			NULL, &RSRenderBufferClass::method_pushTransformation, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"popTransformation", NULL, L"",
			NULL, &RSRenderBufferClass::method_popTransformation, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"resetTransformation", NULL, L"",
			NULL, &RSRenderBufferClass::method_resetTransformation, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"attachCustomShader", L"boolean", L"CustomShader shader",
			NULL, &RSRenderBufferClass::method_attachCustomShader, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getCustomShader", L"CustomShader", L"",
			NULL, &RSRenderBufferClass::method_getCustomShader,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setCustomShaderUniform",
			L"boolean", L"String idUniform, int type, Uint8Pointer pData, int count",
			NULL, &RSRenderBufferClass::method_setCustomShaderUniform, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"resetCustomShaderUniform", L"boolean", L"",
			NULL, &RSRenderBufferClass::method_resetCustomShaderUniform, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setOptionalFeature",
			L"boolean", L"int feature, int param1, Uint8Pointer pParam2, int sizeOfParam2",
			NULL, &RSRenderBufferClass::method_setOptionalFeature, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getOptionalFeature",
			L"boolean", L"int feature, int param1, Uint8Pointer pParam2, int sizeOfParam2",
			NULL, &RSRenderBufferClass::method_getOptionalFeature, 
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"addPrimitiveList",
			L"boolean", L"Material material, int nFlags, int typePrimitive, "
					L"int countIndex, int countVertex, "
					L"Vector3D4 vVertex, Vector3D4 vNormal, "
					L"Vector2D vUV, Color3D color, Uint32Pointer pIndex",
			NULL, &RSRenderBufferClass::method_addPrimitiveList, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"addVertexBuffer",
			L"boolean", L"int nFlags, VertexBuffer vb, "
					L"int iFirst = 0, int iEnd = -1, "
					L"int nInstancing = 0, "
					L"Matrix4D pMatrixInstance = null, "
					L"Color3D pColorInstance = null",
			NULL, &RSRenderBufferClass::method_addVertexBuffer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"flush", NULL, L"",
			NULL, &RSRenderBufferClass::method_flush, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSRenderBufferClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<S3DRenderBufferInterface>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DRenderBufferInterface *
	RSRenderBufferClass::GetThisRenderBuffer( RSContext& context, RSObject* pThis )
{
	S3DRenderBufferInterface *
		pRender = RenderBufferFromObject( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError( L"this が RenderBuffer ではありません" ) ;
	}
	return	pRender ;
}

// Object -> S3DRenderBufferInterface 変換
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DRenderBufferInterface *
	RSRenderBufferClass::RenderBufferFromObject( RSContext& context, RSObject* pObject )
{
	S3DRenderBufferInterface *	pRender = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pObject ) ;
	if ( pNativeObj != NULL )
	{
		pRender = ESLTypeCast<S3DRenderBufferInterface>( pNativeObj->GetObject() ) ;
	}
	return	pRender ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderBufferClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"RenderBuffer.<init> の this が RenderBuffer ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( (S3DRenderBufferInterface*) new S3DVertexBuffer ) ;
	return	NULL ;
}

// void setShadingFlag( long nShadingMethod )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderBufferClass::method_setShadingFlag
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderBufferInterface *	pRender = GetThisRenderBuffer( context, pThis ) ;
	if ( pRender == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pRender->SetShadingFlag( arg.LongAt(0) ) ;
	return	NULL ;
}

// const long getShadingFlag()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderBufferClass::method_getShadingFlag
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderBufferInterface *	pRender = GetThisRenderBuffer( context, pThis ) ;
	if ( pRender == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pRender->GetShadingFlag() ) ;
}

// void appendMatrixTransformation
// ( Matrix3D mat, Vector3D pos,
//		Color3D color = null, int transparency = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderBufferClass::method_appendMatrixTransformation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderBufferInterface *	pRender = GetThisRenderBuffer( context, pThis ) ;
	if ( pRender == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DDMatrix	matd3( 1, 1, 1 ) ;
	S3DDVector	posd3( 0, 0, 0 ) ;
	//
	SGL3DMatrix<float32_t,3> *
		pMatrix3 = (SGL3DMatrix<float32_t,3>*)
					arg.PointerAt( 0, sizeof(SGL3DMatrix<float32_t,3>) ) ;
	if ( pMatrix3 != nullptr )
	{
		for ( int i = 0; i < 3; i ++ )
		{
			matd3.m[i][0] = pMatrix3->m[i][0] ;
			matd3.m[i][1] = pMatrix3->m[i][1] ;
			matd3.m[i][2] = pMatrix3->m[i][2] ;
		}
	}
	S3DVector *	pVector3 =
		(S3DVector*) arg.PointerAt( 1, sizeof(S3DVector) ) ;
	if ( pVector3 != nullptr )
	{
		posd3 = *pVector3 ;
	}
	S3DColor *	pColor =
		(S3DColor*) arg.PointerAt( 2, sizeof(S3DColor) ) ;
	//
	pRender->AppendMatrixTransformation
		( matd3, posd3, pColor, (unsigned int) arg.IntAt(3) ) ;
	//
	return	NULL ;
}

// void setMatrixTransformation
// ( Matrix3D mat, Vector3D pos,
//		Color3D color = null, int transparency = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderBufferClass::method_setMatrixTransformation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderBufferInterface *	pRender = GetThisRenderBuffer( context, pThis ) ;
	if ( pRender == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DDMatrix	matd3( 1, 1, 1 ) ;
	S3DDVector	posd3( 0, 0, 0 ) ;
	//
	SGL3DMatrix<float32_t,3> *
		pMatrix3 = (SGL3DMatrix<float32_t,3>*)
					arg.PointerAt( 0, sizeof(SGL3DMatrix<float32_t,3>) ) ;
	if ( pMatrix3 == NULL )
	{
		for ( int i = 0; i < 3; i ++ )
		{
			matd3.m[i][0] = pMatrix3->m[i][0] ;
			matd3.m[i][1] = pMatrix3->m[i][1] ;
			matd3.m[i][2] = pMatrix3->m[i][2] ;
		}
	}
	S3DVector *	pVector3 =
		(S3DVector*) arg.PointerAt( 1, sizeof(S3DVector) ) ;
	if ( pVector3 == NULL )
	{
		posd3 = *pVector3 ;
	}
	S3DColor *	pColor =
		(S3DColor*) arg.PointerAt( 2, sizeof(S3DColor) ) ;
	//
	pRender->SetMatrixTransformation
		( matd3, posd3, pColor, (unsigned int) arg.IntAt(3) ) ;
	//
	return	NULL ;
}

// void getMatrixTransformation
// ( Matrix3D mat, Vector3D pos,
//		Color3D color, Uint32Pointer transparency )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderBufferClass::method_getMatrixTransformation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderBufferInterface *	pRender = GetThisRenderBuffer( context, pThis ) ;
	if ( pRender == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DDMatrix	matd3( 1, 1, 1 ) ;
	S3DDVector	posd3( 0, 0, 0 ) ;
	//
	SGL3DMatrix<float32_t,3> *
		pMatrix3 = (SGL3DMatrix<float32_t,3>*)
					arg.PointerAt( 0, sizeof(SGL3DMatrix<float32_t,3>) ) ;
	S3DVector *	pVector3 =
		(S3DVector*) arg.PointerAt( 1, sizeof(S3DVector) ) ;
	S3DColor *	pColor =
		(S3DColor*) arg.PointerAt( 2, sizeof(S3DColor) ) ;
	uint32_t *	pTransparency =
		(uint32_t*) arg.PointerAt( 3, sizeof(uint32_t) ) ;
	//
	unsigned int	nTransparency = 0 ;
	if ( !pRender->GetMatrixTransformation
		( matd3, posd3, pColor, &nTransparency ) )
	{
		if ( pMatrix3 == NULL )
		{
			for ( int i = 0; i < 3; i ++ )
			{
				pMatrix3->m[i][0] = (float32_t) matd3.m[i][0] ;
				pMatrix3->m[i][1] = (float32_t) matd3.m[i][1] ;
				pMatrix3->m[i][2] = (float32_t) matd3.m[i][2] ;
			}
		}
		if ( pVector3 == NULL )
		{
			*pVector3 = posd3 ;
		}
		if ( pTransparency != NULL )
		{
			*pTransparency = (uint32_t) nTransparency ;
		}
	}
	return	NULL ;
}

// void pushTransformation()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderBufferClass::method_pushTransformation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderBufferInterface *	pRender = GetThisRenderBuffer( context, pThis ) ;
	if ( pRender == NULL )
	{
		return	NULL ;
	}
	pRender->PushTransformation() ;
	return	NULL ;
}

// void popTransformation()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderBufferClass::method_popTransformation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderBufferInterface *	pRender = GetThisRenderBuffer( context, pThis ) ;
	if ( pRender == NULL )
	{
		return	NULL ;
	}
	pRender->PopTransformation() ;
	return	NULL ;
}

// void resetTransformation()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderBufferClass::method_resetTransformation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderBufferInterface *	pRender = GetThisRenderBuffer( context, pThis ) ;
	if ( pRender == NULL )
	{
		return	NULL ;
	}
	pRender->ResetTransformation() ;
	return	NULL ;
}

// boolean attachCustomShader( CustomShader pShader )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderBufferClass::method_attachCustomShader
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderBufferInterface *	pRender = GetThisRenderBuffer( context, pThis ) ;
	if ( pRender == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DCustomShader *
		pShader = ESLTypeCast<S3DCustomShader>( arg.NativeObjectAt( 0 ) ) ;
	if ( pRender->AttachCustomShader( pShader ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// const CustomShader getCustomShader()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderBufferClass::method_getCustomShader
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderBufferInterface *	pRender = GetThisRenderBuffer( context, pThis ) ;
	if ( pRender == NULL )
	{
		return	NULL ;
	}
	S3DCustomShader *	pShader = pRender->GetCustomShader() ;
	if ( pShader == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject( pShader, context.GetClassAs( L"CustomShader" ) ) ;
}

// boolean setCustomShaderUniform
//	( String idUniform, int type, Uint8Pointer pData, int count )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderBufferClass::method_setCustomShaderUniform
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderBufferInterface *	pRender = GetThisRenderBuffer( context, pThis ) ;
	if ( pRender == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DCustomShader::UniformType
						type = (S3DCustomShader::UniformType) arg.IntAt(1) ;
	void *				pData = arg.PointerAt(2) ;
	SGLImageObject *	pTexturePtr = NULL ;
	size_t				nCount = (size_t) arg.IntAt(3) ;
	//
	if ( type == S3DCustomShader::uniformTexture )
	{
		int64_t *	pPtrData = reinterpret_cast<int64_t*>( pData ) ;
		pTexturePtr = reinterpret_cast<SGLImageObject*>( pPtrData[0] ) ;
		pData = &pTexturePtr ;
		nCount = 1 ;
	}
	//
	if ( pRender->SetCustomShaderUniform( arg.StringAt(0), type, pData, nCount ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean resetCustomShaderUniform()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderBufferClass::method_resetCustomShaderUniform
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderBufferInterface *	pRender = GetThisRenderBuffer( context, pThis ) ;
	if ( pRender == NULL )
	{
		return	NULL ;
	}
	if ( pRender->ResetCustomShaderUniform() )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean setOptionalFeature
// ( int feature, int param1, Uint8Pointer pParam2, int sizeOfParam2 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderBufferClass::method_setOptionalFeature
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderBufferInterface *	pRender = GetThisRenderBuffer( context, pThis ) ;
	if ( pRender == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	//
	const S3DRenderBufferInterface::FeatureType
					type = (S3DRenderBufferInterface::FeatureType) arg.IntAt(0) ;
	size_t			nSizeOfParam2 = (size_t) arg.IntAt( 3 ) ;
	const void *	pParam2 = arg.PointerAt( 2, nSizeOfParam2 ) ;
	//
	RenderContext::EnvMappingParam	envMapParam ;
	if ( type == S3DRenderBufferInterface::featureEnvMap )
	{
		EnvMappingParamClass::FromObject
			( context, envMapParam, arg.ObjectAt(2) ) ;
		pParam2 = &envMapParam ;
		nSizeOfParam2 = sizeof(RenderContext::EnvMappingParam) ;
	}
	//
	if ( pRender->SetOptionalFeature
		( type, (int32_t) arg.IntAt(1), pParam2, nSizeOfParam2 ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// const boolean getOptionalFeature
// ( int feature, int param1, Uint8Pointer pParam2, int sizeOfParam2 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderBufferClass::method_getOptionalFeature
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderBufferInterface *	pRender = GetThisRenderBuffer( context, pThis ) ;
	if ( pRender == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	//
	const S3DRenderBufferInterface::FeatureType
			type = (S3DRenderBufferInterface::FeatureType) arg.IntAt(0) ;
	size_t	nSizeOfParam2 = (size_t) arg.IntAt( 3 ) ;
	void *	pParam2 = arg.PointerAt( 2, nSizeOfParam2 )  ;
	//
	RenderContext::EnvMappingParam	envMapParam ;
	if ( type == S3DRenderBufferInterface::featureEnvMap )
	{
		pParam2 = &envMapParam ;
		nSizeOfParam2 = sizeof(RenderContext::EnvMappingParam) ;
	}
	//
	if ( pRender->GetOptionalFeature
		( type, (int32_t) arg.IntAt(1), pParam2, nSizeOfParam2 ) )
	{
		return	context.new_Boolean( false ) ;
	}
	if ( type == S3DRenderBufferInterface::featureEnvMap )
	{
		EnvMappingParamClass::ToObject
			( context, arg.ObjectAt(2), envMapParam ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean addIndexedPrimitiveList
// ( Material material, int nFlags, int typePrimitive,
//	int countIndex, int countVertex,
//	Vector3D4 vVertex, Vector3D4 vNormal,
//	Vector2D vUV, Color3D color, Uint32Pointer pIndex )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderBufferClass::method_addPrimitiveList
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderBufferInterface *	pRender = GetThisRenderBuffer( context, pThis ) ;
	if ( pRender == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	//
	S3DMaterial *		pMaterial = ESLTypeCast<S3DMaterial>( arg.NativeObjectAt(0) ) ;
	const uint32_t		nFlags = (uint32_t) arg.IntAt( 1 ) ;
	const S3DPrimitiveType
						typePrimitive = (S3DPrimitiveType) arg.IntAt( 2 ) ;
	const size_t		countIndex = (size_t) arg.IntAt( 3 ) ;
	const size_t		countVertex = (size_t) arg.IntAt( 4 ) ;
	const S3DVector4 *	pvVertex =
							(const S3DVector4*) arg.PointerAt
									( 5, countVertex * sizeof(S3DVector4) ) ;
	const S3DVector4 *	pvNormal =
							(const S3DVector4*) arg.PointerAt
									( 6, countVertex * sizeof(S3DVector4) ) ;
	const S2DVector *	pvUVMap =
							(const S2DVector*) arg.PointerAt
									( 7, countVertex * sizeof(S2DVector) ) ;
	const S3DColor *	pColor =
							(const S3DColor*) arg.PointerAt
									( 8, countVertex * sizeof(S3DColor) ) ;
	const uint32_t *	pIndex =
							(const uint32_t*) arg.PointerAt
									( 9, countIndex * sizeof(uint32_t) ) ;
	//
	if ( pRender->AddIndexedPrimitiveList
		( pMaterial, nFlags, typePrimitive,
			countIndex, countVertex,
			pvVertex, pvNormal, pvUVMap, pColor, pIndex ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean addVertexBuffer
// ( int nFlags, VertexBuffer vb,
//	int iFirst = 0, int iEnd = -1,
//	int nInstancing = 0,
//	Matrix4D pMatrixInstance = null,
//	Color3D pColorInstance = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderBufferClass::method_addVertexBuffer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderBufferInterface *	pRender = GetThisRenderBuffer( context, pThis ) ;
	if ( pRender == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	//
	const uint32_t		nFlags = (uint32_t) arg.IntAt( 0 ) ;
	S3DVertexBufferInterface *
						pVBO = ESLTypeCast<S3DVertexBufferInterface>
											( arg.NativeObjectAt(1) ) ;
	const size_t		iFirst = (size_t) arg.IntAt( 2 ) ;
	const ssize_t		iEnd = (ssize_t) arg.IntAt( 3, -1 ) ;
	const size_t		nInstancing = (size_t) arg.IntAt( 4 ) ;
	const S4DMatrix *	pMatrix =
							(const S4DMatrix*) arg.PointerAt
									( 5, nInstancing * sizeof(S4DMatrix) ) ;
	const S3DColor *	pColor =
							(const S3DColor*) arg.PointerAt
									( 6, nInstancing * sizeof(S3DColor) ) ;
	//
	if ( pRender->AddVertexBuffer
		( S3DMaterial::GetDefaultMaterial(S3DMaterial::defaultWhite),
			nFlags, pVBO, iFirst, iEnd, nInstancing, pMatrix, pColor ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// vod flush()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderBufferClass::method_flush
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderBufferInterface *	pRender = GetThisRenderBuffer( context, pThis ) ;
	if ( pRender == NULL )
	{
		return	NULL ;
	}
	pRender->Flush() ;
	return	NULL ;
}



//////////////////////////////////////////////////////////////////////////////
// VertexVariantBuffer クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSVertexVariantBufferClass, RGenericNativeObjectClass )
// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSVertexVariantBufferClass::RSVertexVariantBufferClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSVertexVariantBufferClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"setBoneMatrix",
			L"boolean", L"int iMesh, int nCount, Matrix4D pMatrix",
			NULL, &RSVertexVariantBufferClass::method_setBoneMatrix, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getBoneMatrix",
			L"boolean", L"int iMesh, int nCount, Matrix4D pMatrix",
			NULL, &RSVertexVariantBufferClass::method_getBoneMatrix,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setMorphingApplication",
			L"boolean", L"int iMesh, Int32Pointer pTarget, "
						L"Float32Pointer pApplication, int nTargetCount",
			NULL, &RSVertexVariantBufferClass::method_setMorphingApplication, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getMorphingApplication",
			L"boolean", L"int iMesh, Int32Pointer pTarget, "
						L"Float32Pointer pApplication, int iTargetIndex",
			NULL, &RSVertexVariantBufferClass::method_getMorphingApplication,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"enableToRenderMesh",
			L"boolean", L"int iFirst = 0, int iEnd = -1, boolean fEnable = true",
			NULL, &RSVertexVariantBufferClass::method_enableToRenderMesh, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isEnabledToRenderMesh",
			L"boolean", L"int iMesh",
			NULL, &RSVertexVariantBufferClass::method_isEnabledToRenderMesh,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setMaterialToRenderMesh",
			L"boolean", L"int iMesh, Material material",
			NULL, &RSVertexVariantBufferClass::method_setMaterialToRenderMesh, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getMaterialToRenderMesh",
			L"Material", L"int iMesh",
			NULL, &RSVertexVariantBufferClass::method_getMaterialToRenderMesh,
			NULL, RSFunctionPrototype::flagConstant ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSVertexVariantBufferClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<S3DVertexVariantBuffer>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DVertexVariantBuffer *
	RSVertexVariantBufferClass::GetThisVertexVariantBuffer( RSContext& context, RSObject* pThis )
{
	S3DVertexVariantBuffer *
		pVVB = VertexVariantBufferFromObject( context, pThis ) ;
	if ( pVVB == NULL )
	{
		context.ThrowExceptionError( L"this が VertexVariantBuffer ではありません" ) ;
	}
	return	pVVB ;
}

// Object -> S3DVertexVariantBuffer 変換
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DVertexVariantBuffer *
	RSVertexVariantBufferClass::VertexVariantBufferFromObject( RSContext& context, RSObject* pObject )
{
	S3DVertexVariantBuffer *	pVVB = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pObject ) ;
	if ( pNativeObj != NULL )
	{
		pVVB = ESLTypeCast<S3DVertexVariantBuffer>( pNativeObj->GetObject() ) ;
	}
	return	pVVB ;
}

// boolean setBoneMatrix( int iMesh, int nCount, Matrix4D pMatrix )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexVariantBufferClass::method_setBoneMatrix
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexVariantBuffer *	pVVB = GetThisVertexVariantBuffer( context, pThis ) ;
	if ( pVVB == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	//
	const size_t		iMesh = (size_t) arg.IntAt( 0 ) ;
	const size_t		nCount = (size_t) arg.IntAt( 1 ) ;
	const S4DMatrix *	pMatrix4 =
		(const S4DMatrix*) arg.PointerAt( 2, nCount * sizeof(S4DMatrix) ) ;
	if ( pMatrix4 == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SArray<S3DMatrix>	bufMatrix ;
	SArray<S3DVector>	bufTrans ;
	S3DMatrix *	pMatrix3 = bufMatrix.GetArray( nCount ) ;
	S3DVector *	pTrans = bufTrans.GetArray( nCount ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pMatrix3[i] = pMatrix4[i].GetMatrix3() ;
		pTrans[i] = pMatrix4[i].GetTranslation() ;
	}
	//
	if ( pVVB->SetBoneMatrix( iMesh, nCount, pMatrix3, pTrans ) )
	{
		bufMatrix.FinishArray() ;
		bufTrans.FinishArray() ;
		return	context.new_Boolean( false ) ;
	}
	bufMatrix.FinishArray() ;
	bufTrans.FinishArray() ;
	return	context.new_Boolean( true ) ;
}

// boolean getBoneMatrix( int iMesh, int nCount, Matrix4D pMatrix )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexVariantBufferClass::method_getBoneMatrix
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexVariantBuffer *	pVVB = GetThisVertexVariantBuffer( context, pThis ) ;
	if ( pVVB == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	//
	const size_t	iMesh = (size_t) arg.IntAt( 0 ) ;
	const size_t	nCount = (size_t) arg.IntAt( 1 ) ;
	S4DMatrix *		pMatrix4 =
		(S4DMatrix*) arg.PointerAt( 2, nCount * sizeof(S4DMatrix) ) ;
	if ( pMatrix4 == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SArray<S3DMatrix>	bufMatrix ;
	SArray<S3DVector>	bufTrans ;
	S3DMatrix *	pMatrix3 = bufMatrix.GetArray( nCount ) ;
	S3DVector *	pTrans = bufTrans.GetArray( nCount ) ;
	//
	if ( pVVB->GetBoneMatrix( iMesh, nCount, pMatrix3, pTrans ) )
	{
		bufMatrix.FinishArray() ;
		bufTrans.FinishArray() ;
		return	context.new_Boolean( false ) ;
	}
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pMatrix4[i].SetMatrix3( pMatrix3[i] ) ;
		pMatrix4[i].SetTranslation( pTrans[i] ) ;
	}
	bufMatrix.FinishArray() ;
	bufTrans.FinishArray() ;
	return	context.new_Boolean( true ) ;
}

// boolean setMorphingApplication
//	( int iMesh, Int32Pointer pTarget,
//	Float32Pointer pApplication, int nTargetCount )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexVariantBufferClass::method_setMorphingApplication
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexVariantBuffer *	pVVB = GetThisVertexVariantBuffer( context, pThis ) ;
	if ( pVVB == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	//
	const size_t		iMesh = (size_t) arg.IntAt( 0 ) ;
	const size_t		nTargetCount = (size_t) arg.IntAt( 3 ) ;
	const int32_t *		pTargetIndex =
		(const int32_t*) arg.PointerAt( 1, nTargetCount * sizeof(int32_t) ) ;
	const float32_t *	pApplication =
		(const float32_t*) arg.PointerAt( 2, nTargetCount * sizeof(float32_t) ) ;
	if ( (pTargetIndex == NULL) || (pApplication == NULL) )
	{
		return	context.new_Boolean( false ) ;
	}
	SArray<ssize_t>	bufTargetMesh ;
	ssize_t *		pTarget = bufTargetMesh.GetArray( nTargetCount ) ;
	for ( size_t i = 0; i < nTargetCount; i ++ )
	{
		pTarget[i] = (ssize_t) pTargetIndex[i] ;
	}
	//
	if ( pVVB->SetMorphingApplication
		( iMesh, pTarget, pApplication, nTargetCount ) )
	{
		bufTargetMesh.FinishArray() ;
		return	context.new_Boolean( false ) ;
	}
	bufTargetMesh.FinishArray() ;
	return	context.new_Boolean( true ) ;
}

// boolean getMorphingApplication
//	( int iMesh, Int32Pointer pTarget,
//	Float32Pointer pApplication, int iTargetIndex )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexVariantBufferClass::method_getMorphingApplication
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexVariantBuffer *	pVVB = GetThisVertexVariantBuffer( context, pThis ) ;
	if ( pVVB == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	//
	const size_t	iMesh = (size_t) arg.IntAt( 0 ) ;
	int32_t *		pTargetIndex =
		(int32_t*) arg.PointerAt( 1, sizeof(int32_t) ) ;
	float32_t *	pApplication =
		(float32_t*) arg.PointerAt( 2, sizeof(float32_t) ) ;
	const size_t	iTargetMeshIndex = (size_t) arg.IntAt( 3 ) ;
	if ( (pTargetIndex == NULL) || (pApplication == NULL) )
	{
		return	context.new_Boolean( false ) ;
	}
	ssize_t	iTargetMesh = 0 ;
	if ( pVVB->GetMorphingApplication
		( iMesh, iTargetMesh, *pApplication, iTargetMeshIndex ) )
	{
		return	context.new_Boolean( false ) ;
	}
	*pTargetIndex = (int32_t) iTargetMesh ;
	return	context.new_Boolean( true ) ;
}

// boolean enableToRenderMesh
//	( int iFirst = 0, int iEnd = -1, boolean fEnable = true )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexVariantBufferClass::method_enableToRenderMesh
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexVariantBuffer *	pVVB = GetThisVertexVariantBuffer( context, pThis ) ;
	if ( pVVB == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	//
	if ( pVVB->EnableToRenderMesh
		( (size_t) arg.IntAt(0), (ssize_t) arg.IntAt(1,-1), arg.BooleanAt(2,true) ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean isEnabledToRenderMesh( int iMesh )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexVariantBufferClass::method_isEnabledToRenderMesh
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexVariantBuffer *	pVVB = GetThisVertexVariantBuffer( context, pThis ) ;
	if ( pVVB == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	//
	return	context.new_Boolean
				( pVVB->IsEnabledToRenderMesh( (size_t) arg.IntAt(0) ) ) ;
}

// boolean setMaterialToRenderMesh( int iMesh, Material material )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexVariantBufferClass::method_setMaterialToRenderMesh
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexVariantBuffer *	pVVB = GetThisVertexVariantBuffer( context, pThis ) ;
	if ( pVVB == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DMaterial *	pMaterial = ESLTypeCast<S3DMaterial>( arg.NativeObjectAt(1) ) ;
	//
	if ( pVVB->SetMaterialToRenderMesh
			( (size_t) arg.IntAt(0), pMaterial ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// Material getMaterialToRenderMesh( int iMesh )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexVariantBufferClass::method_getMaterialToRenderMesh
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexVariantBuffer *	pVVB = GetThisVertexVariantBuffer( context, pThis ) ;
	if ( pVVB == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DMaterial *	pMaterial =
		pVVB->GetMaterialToRenderMesh( (size_t) arg.IntAt(0) ) ;
	//
	if ( pMaterial == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject( pMaterial, context.GetClassAs( L"Material" ) ) ;
}


//////////////////////////////////////////////////////////////////////////////
// VertexBuffer クラス
//////////////////////////////////////////////////////////////////////////////

// VertexBuffer.MeshInfo クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSVertexBufferClass::MeshInfoClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSVertexBufferClass::MeshInfoClass::MeshInfoClass
	( RSClass * pClass, const wchar_t * pwszClassName )
		: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSVertexBufferClass::MeshInfoClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSGenericObject( this ) ;
	//
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"pMaterial",
				context.new_Pointer
					( NULL, context.GetClassAs(L"Material") ) ) ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"typeMesh", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"countPrimitive", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"countVertex", 0 ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"vCenter",
				context.new_Pointer
					( NULL, context.GetClassAs(L"Vector3D") ) ) ) ;
	m_pPrototype->CreateMemberNumberAs( context, L"fpRadius", 0 ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"pvVertex",
				context.new_Pointer
					( NULL, context.GetClassAs(L"Vector3D4") ) ) ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"pvNormal",
				context.new_Pointer
					( NULL, context.GetClassAs(L"Vector3D4") ) ) ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"pvUVMap",
				context.new_Pointer
					( NULL, context.GetClassAs(L"Vector2D") ) ) ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"pColor",
				context.new_Pointer
					( NULL, context.GetClassAs(L"Color3D") ) ) ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"pIndexedList",
				context.new_Pointer
					( NULL, context.GetTypedPointerClass
								(RSReferenceNumber::typeUint32) ) ) ) ;
}

// VertexBuffer.PrimitiveBuffer クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSVertexBufferClass::PrimitiveBufferClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSVertexBufferClass::PrimitiveBufferClass::PrimitiveBufferClass
	( RSClass * pClass, const wchar_t * pwszClassName )
		: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSVertexBufferClass::PrimitiveBufferClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSGenericObject( this ) ;
	//
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"pvVertex",
				context.new_Pointer
					( NULL, context.GetClassAs(L"Vector3D4") ) ) ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"pvNormal",
				context.new_Pointer
					( NULL, context.GetClassAs(L"Vector3D4") ) ) ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"pvUVMap",
				context.new_Pointer
					( NULL, context.GetClassAs(L"Vector2D") ) ) ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"pColor",
				context.new_Pointer
					( NULL, context.GetClassAs(L"Color3D") ) ) ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"pIndexedList",
				context.new_Pointer
					( NULL, context.GetTypedPointerClass
								(RSReferenceNumber::typeUint32) ) ) ) ;
}


// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSVertexBufferClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSVertexBufferClass::RSVertexBufferClass
	( RSClass * pClass, const wchar_t * pwszClassName )
		: RSClass( pClass, pwszClassName )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSVertexBufferClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"RenderBuffer" ) ) ;
	AddImplementClass
		( context, context.GetClassAs( L"VertexVariantBuffer" ) ) ;
	//
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSVertexBufferClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	MeshInfoClass *	pMeshInfo =
			new MeshInfoClass
				( context.GetClassClass(), L"MeshInfo" ) ;
	pMeshInfo->Initialize( context ) ;
	pMeshInfo->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"MeshInfo", pMeshInfo ) ) ;
	//
	PrimitiveBufferClass *	pPrimitiveBuffer =
			new PrimitiveBufferClass
				( context.GetClassClass(), L"PrimitiveBuffer" ) ;
	pPrimitiveBuffer->Initialize( context ) ;
	pPrimitiveBuffer->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"PrimitiveBuffer", pPrimitiveBuffer ) ) ;
	//
	CreateMemberIntegerAs
		( context, L"bufferAutoMerge",
			S3DVertexBufferInterface::bufferAutoMerge, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"bufferKeepDeviceBuffer",
			S3DVertexBufferInterface::bufferKeepDeviceBuffer, modifierConst ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &RSVertexBufferClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getBufferControlFlags", L"int", L"",
			NULL, &RSVertexBufferClass::method_getBufferControlFlags,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setBufferControlFlags",
			NULL, L"int nFlags",
			NULL, &RSVertexBufferClass::method_setBufferControlFlags, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getDefaultMaterial",
			L"Material", L"",
			NULL, &RSVertexBufferClass::method_getDefaultMaterial,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"attachDefaultMaterial",
			NULL, L"Material material",
			NULL, &RSVertexBufferClass::method_attachDefaultMaterial, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"allocatePrimitiveBuffer",
			L"boolean", L"VertexBuffer.PrimitiveBuffer prmbuf, int typePrimitive, "
						L"int countIndex, int countVertex",
			NULL, &RSVertexBufferClass::method_allocatePrimitiveBuffer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"addPrimitiveBuffer",
			L"boolean", L"Material material, int nFlags, int typePrimitive, "
				L"VertexBuffer.PrimitiveBuffer prmbuf, int countIndex, int countVertex",
			NULL, &RSVertexBufferClass::method_addPrimitiveBuffer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"freePrimitiveBuffer",
			L"boolean", L"VertexBuffer.PrimitiveBuffer prmbuf",
			NULL, &RSVertexBufferClass::method_freePrimitiveBuffer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getMeshCount",
			L"int", L"",
			NULL, &RSVertexBufferClass::method_getMeshCount,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getMeshInfoAt",
			L"boolean", L"VertexBuffer.MeshInfo info, int iMesh, int nCopyVerteics",
			NULL, &RSVertexBufferClass::method_getMeshInfoAt,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"updatePrimitiveList",
			L"boolean", L"int iMesh, int nFlags, int countIndex, int countVertex, "
					L"Vector3D4 vVertex, Vector3D4 vNormal, "
					L"Vector2D vUVMap, Color3D pColor, Uint32Pointer pIndex",
			NULL, &RSVertexBufferClass::method_updatePrimitiveList, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setBoneWeightMap",
			L"boolean", L"int iMesh, int nCount, Float32Pointer pWeightMaps",
			NULL, &RSVertexBufferClass::method_setBoneWeightMap, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setBoneJointMap",
			L"boolean", L"int iMesh, int nBoneCount, "
						L"int nJointCount, Uint32Pointer pJointMaps",
			NULL, &RSVertexBufferClass::method_setBoneJointMap, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"allocateMorphing",
			L"boolean", L"int iMesh, int nTargetCount",
			NULL, &RSVertexBufferClass::method_allocateMorphing, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setMorphingTargetMesh",
			L"boolean", L"int iMesh, int iMorphTarget, int countVertex, "
						L"Vector3D4 vVertex, Vector3D4 vNormal, "
						L"Vector2D vUVMap, Color3D pColor",
			NULL, &RSVertexBufferClass::method_setMorphingTargetMesh, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setMorphingTargetWeight",
			L"boolean", L"int iMesh, int iMorphTarget, "
						L"int countVertex, Float32Pointer pWeight",
			NULL, &RSVertexBufferClass::method_setMorphingTargetWeight, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"createVariantBuffer",
			L"VertexVariantBuffer", L"",
			NULL, &RSVertexBufferClass::method_createVariantBuffer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"newReferenceVariantBuffer",
			L"VertexVariantBuffer", L"",
			NULL, &RSVertexBufferClass::method_newReferenceVariantBuffer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"updateVertexVariant",
			L"boolean", L"VertexVariantBuffer vvb, int iFirst = 0, int iEnd = -1",
			NULL, &RSVertexBufferClass::method_updateVertexVariant, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"clearBuffer", NULL, L"",
			NULL, &RSVertexBufferClass::method_clearBuffer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getCircumscribedParallelepiped",
			L"boolean", L"Vector3D vMin, Vector3D vMax",
			NULL, &RSVertexBufferClass::method_getCircumscribedParallelepiped,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"countOfTotalPolygons", L"int", L"",
			NULL, &RSVertexBufferClass::method_countOfTotalPolygons,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"countOfTotalVertices", L"int", L"",
			NULL, &RSVertexBufferClass::method_countOfTotalVertices,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"enableMultiInstancingMode",
			L"boolean", L"boolean flagEnable",
			NULL, &RSVertexBufferClass::method_enableMultiInstancingMode, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isMultiInstancingMode", L"boolean", L"",
			NULL, &RSVertexBufferClass::method_isMultiInstancingMode,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"clearAllInstance", L"boolean", L"",
			NULL, &RSVertexBufferClass::method_clearAllInstance, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"addInstanceVariant",
			L"boolean", L"VertexVariantBuffer vvb, Matrix4D matrix, Color3D color",
			NULL, &RSVertexBufferClass::method_addInstanceVariant, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSVertexBufferClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<S3DVertexBufferInterface>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DVertexBufferInterface *
	RSVertexBufferClass::GetThisVertexBuffer( RSContext& context, RSObject* pThis )
{
	S3DVertexBufferInterface *
		pVBO = VertexBufferFromObject( context, pThis ) ;
	if ( pVBO == NULL )
	{
		context.ThrowExceptionError( L"this が VertexBuffer ではありません" ) ;
	}
	return	pVBO ;
}

// Object -> S3DVertexBufferInterface 変換
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DVertexBufferInterface *
	RSVertexBufferClass::VertexBufferFromObject( RSContext& context, RSObject* pObject )
{
	S3DVertexBufferInterface *	pVBO = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pObject ) ;
	if ( pNativeObj != NULL )
	{
		pVBO = ESLTypeCast<S3DVertexBufferInterface>( pNativeObj->GetObject() ) ;
	}
	return	pVBO ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"VertexBuffer.<init> の this が VertexBuffer ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( (S3DRenderBufferInterface*) new S3DVertexBuffer ) ;
	return	NULL ;
}

// const int getBufferControlFlags()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_getBufferControlFlags
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pVBO->GetBufferControlFlags() ) ;
}

// void setBufferControlFlags( int nFlags )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_setBufferControlFlags
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pVBO->SetBufferControlFlags( (uint32_t) arg.IntAt(0) ) ;
	return	NULL ;
}

// const Material getDefaultMaterial()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_getDefaultMaterial
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	S3DMaterial *	pMaterial = pVBO->GetDefaultMaterial() ;
	if ( pMaterial == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject( pMaterial, context.GetClassAs( L"Material" ) ) ;
}

// void attachDefaultMaterial( Material material )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_attachDefaultMaterial
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DMaterial *	pMaterial =
		ESLTypeCast<S3DMaterial>( arg.NativeObjectAt(0) ) ;
	pVBO->AttachDefaultMaterial( pMaterial ) ;
	return	NULL ;
}

// boolean allocatePrimitiveBuffer
//	( PrimitiveBuffer prmbuf, int typePrimitive,
//		int countIndex, int countVertex )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_allocatePrimitiveBuffer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pPrmBuf = arg.ObjectAt( 0 ) ;
	if ( pPrmBuf == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
	const size_t	nIndexCount = (size_t) arg.IntAt(2) ;
	const size_t	nVertexCount = (size_t) arg.IntAt(3) ;
	if ( pVBO->AllocatePrimitiveBuffer
		( prmbuf, (S3DPrimitiveType) arg.IntAt(1), nIndexCount, nVertexCount ) )
	{
		return	context.new_Boolean( false ) ;
	}
	RSSmartPtr	psIndexedList( pPrmBuf->GetMemberAs( context, L"pIndexedList" ), &context ) ;
	//
	RSStructuredPointerClass *	pVector3D4Class =
		ESLTypeCast<RSStructuredPointerClass>
						( context.GetClassAs( L"Vector3D4" ) ) ;
	RSStructuredPointerClass *	pVector2DClass =
		ESLTypeCast<RSStructuredPointerClass>
						( context.GetClassAs( L"Vector2D" ) ) ;
	RSStructuredPointerClass *	pColor3DClass =
		ESLTypeCast<RSStructuredPointerClass>
						( context.GetClassAs( L"Color3D" ) ) ;
	//
	pPrmBuf->SetMemberStructPtrRefAs
		( context, L"pvVertex",
			(uint8_t*) prmbuf.pvVertex,
			nVertexCount * sizeof(S3DVector4), pVector3D4Class ) ;
	pPrmBuf->SetMemberStructPtrRefAs
		( context, L"pvNormal",
			(uint8_t*) prmbuf.pvNormal,
			nVertexCount * sizeof(S3DVector4), pVector3D4Class ) ;
	pPrmBuf->SetMemberStructPtrRefAs
		( context, L"pvUVMap",
			(uint8_t*) prmbuf.pvUVMap,
			nVertexCount * sizeof(S2DVector), pVector2DClass ) ;
	pPrmBuf->SetMemberStructPtrRefAs
		( context, L"pColor",
			(uint8_t*) prmbuf.pColor,
			nVertexCount * sizeof(S3DColor), pColor3DClass ) ;
	pPrmBuf->SetMemberNativePtrRefAs
		( context, L"pIndexedList",
			(uint8_t*) prmbuf.pIndexedList,
			nIndexCount * sizeof(uint32_t), typeNumberUint32 ) ;
	//
	return	context.new_Boolean( true ) ;
}

// boolean addPrimitiveBuffer
//	( Material material, int nFlags, int typePrimitive,
//		PrimitiveBuffer prmbuf, int countIndex, int countVertex )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_addPrimitiveBuffer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DMaterial *	pMaterial =
				ESLTypeCast<S3DMaterial>( arg.NativeObjectAt( 0 ) ) ;
	RSObject *	pPrmBuf = arg.ObjectAt( 3 ) ;
	if ( pPrmBuf == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	const size_t	nVertexCount = (size_t) arg.IntAt( 5 ) ;
	const size_t	nIndexCount = (size_t) arg.IntAt( 4 ) ;
	//
	S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
	//
	prmbuf.pvVertex = (S3DVector4*)
		pPrmBuf->GetMemberNativePtrAs
			( context, L"pvVertex", nVertexCount * sizeof(S3DVector4) ) ;
	prmbuf.pvNormal = (S3DVector4*)
		pPrmBuf->GetMemberNativePtrAs
			( context, L"pvNormal", nVertexCount * sizeof(S3DVector4) ) ;
	prmbuf.pvUVMap = (S2DVector*)
		pPrmBuf->GetMemberNativePtrAs
			( context, L"pvUVMap", nVertexCount * sizeof(S2DVector) ) ;
	prmbuf.pColor = (S3DColor*)
		pPrmBuf->GetMemberNativePtrAs
			( context, L"pColor", nVertexCount * sizeof(S3DColor) ) ;
	prmbuf.pIndexedList = (uint32_t*)
		pPrmBuf->GetMemberNativePtrAs
			( context, L"pIndexedList", nIndexCount * sizeof(uint32_t) ) ;
	//
	if ( pVBO->AddPrimitiveBuffer
		( pMaterial, (uint32_t) arg.IntAt(1),
			(S3DPrimitiveType) arg.IntAt(2),
			prmbuf, nIndexCount, nVertexCount ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean freePrimitiveBuffer( PrimitiveBuffer prmbuf )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_freePrimitiveBuffer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pPrmBuf = arg.ObjectAt( 0 ) ;
	if ( pPrmBuf == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
	prmbuf.pvVertex = (S3DVector4*)
		pPrmBuf->GetMemberNativePtrAs( context, L"pvVertex", 0 ) ;
	prmbuf.pvNormal = (S3DVector4*)
		pPrmBuf->GetMemberNativePtrAs( context, L"pvNormal", 0 ) ;
	prmbuf.pvUVMap = (S2DVector*)
		pPrmBuf->GetMemberNativePtrAs( context, L"pvUVMap", 0 ) ;
	prmbuf.pColor = (S3DColor*)
		pPrmBuf->GetMemberNativePtrAs( context, L"pColor", 0 ) ;
	prmbuf.pIndexedList = (uint32_t*)
		pPrmBuf->GetMemberNativePtrAs( context, L"pIndexedList", 0 ) ;
	//
	if ( pVBO->FreePrimitiveBuffer( prmbuf ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// const int getMeshCount()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_getMeshCount
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pVBO->GetMeshCount() ) ;
}

// const boolean getMeshInfoAt
//	( MeshInfo info, int iMesh, int nCopyVerteics )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_getMeshInfoAt
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pInfo = arg.ObjectAt(0) ;
	if ( pInfo == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	const size_t	nCopyVerteics = (size_t) arg.IntAt( 2 ) ;
	//
	S3DVertexBufferInterface::MeshInfo	mi ;
	eslFillMemory( &mi, 0, sizeof(S3DVertexBufferInterface::MeshInfo) ) ;
	//
	mi.pMaterial = ESLTypeCast<S3DMaterial>
				( pInfo->GetMemberNativeObjAs( context, L"pMaterial" ) ) ;
	mi.typeMesh = (S3DPrimitiveType)
					pInfo->GetMemberIntegerAs( context, L"typeMesh" ) ;
	mi.countPrimitive =
		(uint32_t) pInfo->GetMemberIntegerAs( context, L"countPrimitive" ) ;
	mi.countVertex =
		(uint32_t) pInfo->GetMemberIntegerAs( context, L"countVertex" ) ;
	mi.pvVertex = (S3DVector4*) pInfo->GetMemberNativePtrAs
				( context, L"pvVertex", nCopyVerteics * sizeof(S3DVector4) ) ;
	mi.pvNormal = (S3DVector4*) pInfo->GetMemberNativePtrAs
				( context, L"pvNormal", nCopyVerteics * sizeof(S3DVector4) ) ;
	mi.pvUVMap = (S2DVector*) pInfo->GetMemberNativePtrAs
				( context, L"pvUVMap", nCopyVerteics * sizeof(S2DVector) ) ;
	mi.pColor = (S3DColor*) pInfo->GetMemberNativePtrAs
				( context, L"pColor", nCopyVerteics * sizeof(S3DColor) ) ;
	mi.pIndexedList = (uint32_t*) pInfo->GetMemberNativePtrAs
				( context, L"pIndexedList",
					mi.countPrimitive
						* GetPrimitiveVertexCount(mi.typeMesh) * sizeof(uint32_t) ) ;
	//
	if ( pVBO->GetMeshInfoAt( mi, (size_t) arg.IntAt(1), nCopyVerteics ) )
	{
		return	context.new_Boolean( false ) ;
	}
	//
	pInfo->SetMemberNativeObjRefAs
		( context, L"pMaterial",
			mi.pMaterial, context.GetClassAs( L"Material" ) ) ;
	pInfo->SetMemberIntegerAs( context, L"typeMesh", mi.typeMesh ) ;
	pInfo->SetMemberIntegerAs( context, L"countPrimitive", mi.countPrimitive ) ;
	pInfo->SetMemberIntegerAs( context, L"countVertex", mi.countVertex ) ;
	//
	RSSmartPtr	psvCenter( pInfo->GetMemberAs( context, L"vCenter" ), &context ) ;
	if ( psvCenter != NULL )
	{
		psvCenter->SetMemberNumberAs( context, L"x", mi.vCenter.x ) ;
		psvCenter->SetMemberNumberAs( context, L"y", mi.vCenter.y ) ;
		psvCenter->SetMemberNumberAs( context, L"z", mi.vCenter.z ) ;
	}
	pInfo->SetMemberNumberAs( context, L"fpRadius", mi.fpRadius ) ;
	//
	return	context.new_Boolean( true ) ;
}

// boolean updatePrimitiveList
//	( int iMesh, int nFlags, int countIndex, int countVertex,
//		Vector3D4 vVertex, Vector3D4 vNormal,
//		Vector2D vUVMap, Color3D pColor Uint32Pointer pIndex )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_updatePrimitiveList
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	const size_t	countIndex = (size_t) arg.IntAt(2) ;
	const size_t	countVertex = (size_t) arg.IntAt(3) ;
	if ( pVBO->UpdateIndexedPrimitiveList
		( (size_t) arg.IntAt(0), (uint32_t) arg.IntAt(1),
			countIndex, countVertex,
			(S3DVector4*) arg.PointerAt( 4, countVertex * sizeof(S3DVector4) ),
			(S3DVector4*) arg.PointerAt( 5, countVertex * sizeof(S3DVector4) ),
			(S2DVector*) arg.PointerAt( 6, countVertex * sizeof(S2DVector) ),
			(S3DColor*) arg.PointerAt( 7, countVertex * sizeof(S3DColor) ),
			(uint32_t*) arg.PointerAt( 8, countIndex * sizeof(uint32_t) ) ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean setBoneWeightMap
//	( int iMesh, int nCount, Float32Pointer pWeightMaps )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_setBoneWeightMap
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	const size_t	iMesh = (size_t) arg.IntAt( 0 ) ;
	//
	S3DVertexBufferInterface::MeshInfo	mi ;
	eslFillMemory( &mi, 0, sizeof(S3DVertexBufferInterface::MeshInfo) ) ;
	if ( pVBO->GetMeshInfoAt( mi, iMesh, 0 ) )
	{
		return	context.new_Boolean( false ) ;
	}
	const size_t	nBoneCount = (size_t) arg.IntAt( 1 ) ;
	float32_t *		pWeightMap =
		(float32_t*) arg.PointerAt
			( 2, mi.countVertex * nBoneCount * sizeof(float32_t) ) ;
	if ( pWeightMap == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SPointerArray<const float32_t>	aWeightMaps ;
	const float32_t **	ppWeightMaps = aWeightMaps.GetArray( nBoneCount ) ;
	for ( size_t i = 0; i < nBoneCount; i ++ )
	{
		ppWeightMaps[i] = pWeightMap + (mi.countVertex * i) ;
	}
	if ( pVBO->SetBoneWeightMap( iMesh, nBoneCount, ppWeightMaps ) )
	{
		aWeightMaps.FinishArray() ;
		return	context.new_Boolean( false ) ;
	}
	aWeightMaps.FinishArray() ;
	return	context.new_Boolean( true ) ;
}

// boolean setBoneJointMap
//	( int iMesh, int nBoneCount, int nJointCount, Uint32Pointer pJointMaps )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_setBoneJointMap
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	const size_t	iMesh = (size_t) arg.IntAt( 0 ) ;
	//
	S3DVertexBufferInterface::MeshInfo	mi ;
	eslFillMemory( &mi, 0, sizeof(S3DVertexBufferInterface::MeshInfo) ) ;
	if ( pVBO->GetMeshInfoAt( mi, iMesh, 0 ) )
	{
		return	context.new_Boolean( false ) ;
	}
	const size_t	nBoneCount = (size_t) arg.IntAt( 1 ) ;
	const size_t	nJointCount = (size_t) arg.IntAt( 2 ) ;
	uint32_t *		pJointMap =
		(uint32_t*) arg.PointerAt
			( 3, mi.countVertex * nBoneCount
					* nJointCount * sizeof(uint32_t) ) ;
	if ( pJointMap == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SPointerArray<const uint32_t>	aJointMaps ;
	const uint32_t **	ppJointMaps = aJointMaps.GetArray( nJointCount ) ;
	for ( size_t i = 0; i < nJointCount; i ++ )
	{
		ppJointMaps[i] = pJointMap + (mi.countVertex * i) ;
	}
	if ( pVBO->SetBoneJointMap( iMesh, nBoneCount, nJointCount, ppJointMaps ) )
	{
		aJointMaps.FinishArray() ;
		return	context.new_Boolean( false ) ;
	}
	aJointMaps.FinishArray() ;
	return	context.new_Boolean( true ) ;
}

// boolean allocateMorphing( int iMesh, int nTargetCount )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_allocateMorphing
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	if ( pVBO->AllocateMorphing( (size_t) arg.IntAt(0), (size_t) arg.IntAt(1) ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean setMorphingTargetMesh
//	( int iMesh, int iMorphTarget, int countVertex,
//		Vector3D4 vVertex, Vector3D4 vNormal,
//		Vector2D vUVMap, Color3D pColor )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_setMorphingTargetMesh
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	const size_t	iMesh = (size_t) arg.IntAt(0) ;
	const size_t	iMorph = (size_t) arg.IntAt(1) ;
	const size_t	nVertex = (size_t) arg.IntAt(2) ;
	S3DVector4 *	pvVertex =
			(S3DVector4*) arg.PointerAt( 3, nVertex * sizeof(S3DVector4) ) ;
	S3DVector4 *	pvNormal =
			(S3DVector4*) arg.PointerAt( 4, nVertex * sizeof(S3DVector4) ) ;
	S2DVector *		pvUVMap =
			(S2DVector*) arg.PointerAt( 5, nVertex * sizeof(S2DVector) ) ;
	S3DColor *		pColor =
			(S3DColor*) arg.PointerAt( 6, nVertex * sizeof(S3DColor) ) ;
	//
	if ( pVBO->SetMorphingTargetMesh
		( iMesh, iMorph, nVertex, pvVertex, pvNormal, pvUVMap, pColor ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean setMorphingTargetWeight
//	( int iMesh, int iMorphTarget,
//		int countVertex, Float32Pointer pWeight )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_setMorphingTargetWeight
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	const size_t	iMesh = (size_t) arg.IntAt(0) ;
	const size_t	iMorph = (size_t) arg.IntAt(1) ;
	const size_t	nVertex = (size_t) arg.IntAt(2) ;
	float32_t *		pfpWeight =
			(float32_t*) arg.PointerAt( 3, nVertex * sizeof(float32_t) ) ;
	//
	if ( pVBO->SetMorphingTargetWeight
		( iMesh, iMorph, nVertex, pfpWeight ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// VertexVariantBuffer createVariantBuffer()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_createVariantBuffer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	S3DVertexVariantBuffer *	pVVB = pVBO->CreateVariantBuffer() ;
	if ( pVVB == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject
			( pVVB, context.GetClassAs( L"VertexVariantBuffer" ) ) ;
}

// VertexVariantBuffer newReferenceVariantBuffer()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_newReferenceVariantBuffer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	S3DVertexVariantBuffer *	pVVB = pVBO->NewReferenceVariantBuffer() ;
	if ( pVVB == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject
			( pVVB, context.GetClassAs( L"VertexVariantBuffer" ) ) ;
}

// boolean updateVertexVariant
//	( VertexVariantBuffer vvb, int iFirst = 0, int iEnd = -1 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_updateVertexVariant
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVertexVariantBuffer *	pVVB =
		ESLTypeCast<S3DVertexVariantBuffer>( arg.NativeObjectAt(0) ) ;
	if ( pVVB == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	if ( pVBO->UpdateVertexVariant
		( pVVB, (size_t) arg.IntAt(1), (ssize_t) arg.IntAt(2) ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// void clearBuffer()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_clearBuffer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	pVBO->ClearBuffer() ;
	return	NULL ;
}

// const boolean getCircumscribedParallelepiped( Vector3D vMin, Vector3D vMax )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_getCircumscribedParallelepiped
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pvMin = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	S3DVector *	pvMax = (S3DVector*) arg.PointerAt( 1, sizeof(S3DVector) ) ;
	//
	return	context.new_Boolean
			( pVBO->GetCircumscribedParallelepiped( *pvMin, *pvMax ) ) ;
}

// const int countOfTotalPolygons()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_countOfTotalPolygons
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pVBO->CountOfTotalPolygons() ) ;
}

// const int countOfTotalVertices()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_countOfTotalVertices
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pVBO->CountOfTotalVertices() ) ;
}

// boolean enableMultiInstancingMode( boolean flagEnable )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_enableMultiInstancingMode
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	if ( pVBO->EnableMultiInstancingMode( arg.BooleanAt(0) ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// const boolean isMultiInstancingMode()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_isMultiInstancingMode
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pVBO->IsMultiInstancingMode() ) ;
}

// boolean clearAllInstance()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_clearAllInstance
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pVBO->ClearAllInstance() ? false : true ) ;
}

// boolean addInstanceVariant
//	( VertexVariantBuffer vvb, Matrix4D matrix, Color3D color )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVertexBufferClass::method_addInstanceVariant
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DVertexBufferInterface *	pVBO = GetThisVertexBuffer( context, pThis ) ;
	if ( pVBO == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVertexVariantBuffer *	pVVB =
		ESLTypeCast<S3DVertexVariantBuffer>( arg.NativeObjectAt(0) ) ;
	if ( pVVB == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	S4DMatrix *	pMatrix = (S4DMatrix*) arg.PointerAt( 1, sizeof(S4DMatrix) ) ;
	S3DColor *	pColor = (S3DColor*) arg.PointerAt( 2, sizeof(S3DColor) ) ;
	if ( (pMatrix == NULL) || (pColor == NULL) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean
			( pVBO->AddInstanceVariant
				( pVVB, *pMatrix, *pColor ) ? false : true ) ;
}




//////////////////////////////////////////////////////////////////////////////
// ModelBuffer クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSModelBufferClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSModelBufferClass::RSModelBufferClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSModelBufferClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"VertexBuffer" ) ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSModelBufferClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &RSModelBufferClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"loadModel",
			L"boolean", L"String file, String mime = null",
			NULL, &RSModelBufferClass::method_loadModel, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readModel",
			L"boolean", L"InputStream is, String mime = null",
			NULL, &RSModelBufferClass::method_readModel, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readModel",
			L"boolean", L"RandomAccessFile file, String mime = null",
			NULL, &RSModelBufferClass::method_readModel, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"saveModel",
			L"boolean", L"String file, "
					L"String mime = null, String mimeImage = null",
			NULL, &RSModelBufferClass::method_saveModel,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeModel",
			L"boolean", L"RandomAccessFile file, "
					L"String mime = null, String mimeImage = null",
			NULL, &RSModelBufferClass::method_writeModel,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getTextureLibrary",
			L"TextureLibrary", L"",
			NULL, &RSModelBufferClass::method_getTextureLibrary,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getMaterialLibrary",
			L"MaterialLibrary", L"",
			NULL, &RSModelBufferClass::method_getMaterialLibrary,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getPoseLibrary",
			L"ModelPoseLibrary", L"",
			NULL, &RSModelBufferClass::method_getPoseLibrary,
			NULL, RSFunctionPrototype::flagConstant ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSModelBufferClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<S3DModelBuffer>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DModelBuffer *
	RSModelBufferClass::GetThisModelBuffer( RSContext& context, RSObject* pThis )
{
	S3DModelBuffer *	pModel = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pModel = ESLTypeCast<S3DModelBuffer>( pNativeObj->GetObject() ) ;
	}
	if ( pModel == NULL )
	{
		context.ThrowExceptionError( L"this が ModelBuffer ではありません" ) ;
	}
	return	pModel ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSModelBufferClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"ModelBuffer.<init> の this が ModelBuffer ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( (S3DRenderBufferInterface*) new S3DModelBuffer ) ;
	return	NULL ;
}

// boolean loadModel( String file, String mime = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSModelBufferClass::method_loadModel
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DModelBuffer *	pModel = GetThisModelBuffer( context, pThis ) ;
	if ( pModel == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pModel->LoadModel
			( arg.StringAt(0), arg.StringAt(1) ) == sglErrSuccess ) ;
}

// boolean readModel( InputStream is, String mime = null )
// boolean readModel( RandomAccessFile file, String mime = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSModelBufferClass::method_readModel
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DModelBuffer *	pModel = GetThisModelBuffer( context, pThis ) ;
	if ( pModel == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SFileInterface *	pFile =
		RSRandomAccessFileClass::GetFileOf( context, arg.ObjectAt(0) ) ;
	if ( pFile == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean
		( pModel->ReadModel( pFile, arg.StringAt(1) ) == sglErrSuccess ) ;
}

// boolean saveModel
//	( String file, String mime = null, String mimeImage = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSModelBufferClass::method_saveModel
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DModelBuffer *	pModel = GetThisModelBuffer( context, pThis ) ;
	if ( pModel == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pModel->SaveModel
			( arg.StringAt(0), arg.StringAt(1), arg.StringAt(2) ) == sglErrSuccess ) ;
}

// boolean writeModel
//	( RandomAccessFile file,
//		String mime = null, String mimeImage = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSModelBufferClass::method_writeModel
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DModelBuffer *	pModel = GetThisModelBuffer( context, pThis ) ;
	if ( pModel == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SFileInterface *	pFile =
		RSRandomAccessFileClass::GetFileOf( context, arg.ObjectAt(0) ) ;
	if ( pFile == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean
		( pModel->WriteModel
			( pFile, arg.StringAt(1), arg.StringAt(2) ) == sglErrSuccess ) ;
}

// TextureLibrary getTextureLibrary()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSModelBufferClass::method_getTextureLibrary
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DModelBuffer *	pModel = GetThisModelBuffer( context, pThis ) ;
	if ( pModel == NULL )
	{
		return	NULL ;
	}
	S3DTextureLibrary *	pTxtLib = &(pModel->GetTextureLibrary()) ;
	return	new RSNativeObject
				( pTxtLib, context.GetClassAs( L"TextureLibrary" ) ) ;
	
}

// MaterialLibrary getMaterialLibrary()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSModelBufferClass::method_getMaterialLibrary
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DModelBuffer *	pModel = GetThisModelBuffer( context, pThis ) ;
	if ( pModel == NULL )
	{
		return	NULL ;
	}
	S3DMaterialLibrary *	pMatLib = &(pModel->GetMaterialLibrary()) ;
	return	new RSNativeObject
				( pMatLib, context.GetClassAs( L"MaterialLibrary" ) ) ;
}

// ModelPoseLibrary getPoseLibrary()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSModelBufferClass::method_getPoseLibrary
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DModelBuffer *	pModel = GetThisModelBuffer( context, pThis ) ;
	if ( pModel == nullptr )
	{
		return	nullptr ;
	}
	S3DModelPoseLibrary *	pPoseLib = &(pModel->GetPoseLibrary()) ;
	return	new RSNativeObject
				( pPoseLib, context.GetClassAs( L"ModelPoseLibrary" ) ) ;
}



//////////////////////////////////////////////////////////////////////////////
// RenderContext クラス
//////////////////////////////////////////////////////////////////////////////

// RenderContext.Light クラス情報
ESL_IMPLEMENT_CLASS_INFO
	( Rosetta::RSRenderContextClass::RSLightClass, RSStructuredPointerClass )

// RenderContext.Lightクラス構築関数
RSRenderContextClass::RSLightClass::RSLightClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSStructuredPointerClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
void RSRenderContextClass::RSLightClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStructuredPointer( this, NULL ) ;
	//
	AddArrayMemberAs
		( context, L"typeLight",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"rgbColor",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"fpBrightness",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"fpAttenuationPower",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"vecPosition",
			context.GetClassAs(L"Vector3D4") ) ;
	AddArrayMemberAs
		( context, L"vecDirection",
			context.GetClassAs(L"Vector3D4") ) ;
	AddArrayMemberAs
		( context, L"fpAngle",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"fpGradation",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"nReserved2",
			context.GetBasicTypeClass(RSCodeControl::wiInt), 0, 2 ) ;
}


// RenderContext.ShadowMapInfo クラス情報
ESL_IMPLEMENT_CLASS_INFO
	( Rosetta::RSRenderContextClass::RSShadowMapInfoClass, RSStructuredPointerClass )

// RenderContext.ShadowMapInfo クラス構築関数
RSRenderContextClass::RSShadowMapInfoClass::RSShadowMapInfoClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSStructuredPointerClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
void RSRenderContextClass::RSShadowMapInfoClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStructuredPointer( this, NULL ) ;
	//
	AddArrayMemberAs
		( context, L"vLight",
			context.GetClassAs(L"Vector3D4") ) ;
	AddArrayMemberAs
		( context, L"vRay",
			context.GetClassAs(L"Vector3D4") ) ;
	AddArrayMemberAs
		( context, L"vTarget",
			context.GetClassAs(L"Vector3D4") ) ;
	AddArrayMemberAs
		( context, L"vAxisX",
			context.GetClassAs(L"Vector3D4") ) ;
	AddArrayMemberAs
		( context, L"vAxisY",
			context.GetClassAs(L"Vector3D4") ) ;
	AddArrayMemberAs
		( context, L"fpFixErrorGap",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"fpVarErrorGap",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"zPersScreen",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"zPersScale",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"zPersNear",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"zPersFar",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"nReserved",
			context.GetBasicTypeClass(RSCodeControl::wiInt), 0, 4 ) ;
}



// RenderContext クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSRenderContextClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSRenderContextClass::RSRenderContextClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSRenderContextClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"PaintContext" ) ) ;
	AddImplementClass( context, context.GetClassAs( L"RenderBuffer" ) ) ;
	//
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSRenderContextClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	RSLightClass *	pListClass =
		new RSLightClass( context.GetClassClass(), L"Light" ) ;
	pListClass->Initialize( context ) ;
	pListClass->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"Light", pListClass ) ) ;
	//
	RSShadowMapInfoClass *	pShadowMapInfo =
		new RSShadowMapInfoClass( context.GetClassClass(), L"ShadowMapInfo" ) ;
	pShadowMapInfo->Initialize( context ) ;
	pShadowMapInfo->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"ShadowMapInfo", pShadowMapInfo ) ) ;
	//
	CreateMemberIntegerAs
		( context, L"lightTypeAmbient", lightTypeAmbient, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"lightTypeVector", lightTypeVector, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"lightTypePoint", lightTypePoint, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"lightTypeSpot", lightTypeSpot, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"lightTypeFog", lightTypeFog, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"lightTypeAmbientMul", lightTypeAmbientMul, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"lightTypeMask", lightTypeMask, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"lightShadowMapping", (uint32_t) lightShadowMapping, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"stereoViewAuto",
			S3DRenderContextInterface::stereoViewAuto, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"stereoViewRight",
			S3DRenderContextInterface::stereoViewRight, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"stereoViewLeft",
			S3DRenderContextInterface::stereoViewLeft, modifierConst ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
			NULL, &RSRenderContextClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"attachMultiTargetImages",
			L"boolean", L"Image[] targets",
			NULL, &RSRenderContextClass::method_attachMultiTargetImages, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getMultiTargetImages", L"Image[]", L"",
			NULL, &RSRenderContextClass::method_getMultiTargetImages,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setProjectionScreen", L"boolean",
			L"Vector3D vScreen, double zScale = 1.0, double pixelAspect = 1.0",
			NULL, &RSRenderContextClass::method_setProjectionScreen, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getProjectionScreen", L"boolean",
			L"Vector3D vScreen, Float64Pointer zScale, Float64Pointer pixelAspect",
			NULL, &RSRenderContextClass::method_getProjectionScreen,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getPerspectiveMatrix",
			L"boolean", L"Matrix4D matPers",
			NULL, &RSRenderContextClass::method_getPerspectiveMatrix,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setPerspectiveMatrix", NULL,
			L"int stereoViewIndex, Matrix4D matPers, boolean flagPersMatrix = true",
			NULL, &RSRenderContextClass::method_setPerspectiveMatrix, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"enablePerspectiveMatrix",
			NULL, L"boolean flagPersMatrix",
			NULL, &RSRenderContextClass::method_enablePerspectiveMatrix, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setCamera",
			NULL, L"Matrix3D matCamera, Vector3D vCamera",
			NULL, &RSRenderContextClass::method_setCamera, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getCamera",
			NULL, L"Matrix3D matCamera, Vector3D vCamera",
			NULL, &RSRenderContextClass::method_getCamera,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setInverseCameraTransformation",
			NULL, L"Color3D color = null, int transparency = 0",
			NULL, &RSRenderContextClass::method_setInverseCameraTransformation, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isSphereIntoView",
			L"boolean", L"Vector3D vLocalPos, double radius",
			NULL, &RSRenderContextClass::method_isSphereIntoView,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getProjectedPosition", L"boolean",
			L"Vector2D vProjPos, Vector3D vLocalPos, double fpLimit = 100000.0",
			NULL, &RSRenderContextClass::method_getProjectedPosition,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setParallax", NULL,
			L"double xParallax, double zFocusRate, double xScreenDelta",
			NULL, &RSRenderContextClass::method_setParallax, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getParallax", L"double", L"",
			NULL, &RSRenderContextClass::method_getParallax,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setZClipRange", NULL, L"double zMin, double zMax",
			NULL, &RSRenderContextClass::method_setZClipRange, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setLightEntries",
			NULL, L"RenderContext.Light pLights, int countLights",
			NULL, &RSRenderContextClass::method_setLightEntries, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setShadowMap", NULL,
			L"int idLight, Image pShadowDepth, RenderContext.ShadowMapInfo infShadowMap",
			NULL, &RSRenderContextClass::method_setShadowMap, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setFog", NULL,
			L"int rgbFog, double zFogNear, double zFogFar",
			NULL, &RSRenderContextClass::method_setFog, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"enableFog", NULL, L"boolean flagEnable",
			NULL, &RSRenderContextClass::method_enableFog, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"currentParallaxView", L"int", L"",
			NULL, &RSRenderContextClass::method_currentParallaxView,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"selectParallaxView",
			L"boolean", L"int stereoViewIndex",
			NULL, &RSRenderContextClass::method_selectParallaxView, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"begin3DRenderer",
			L"boolean", L"long flags = 0",
			NULL, &RSRenderContextClass::method_begin3DRenderer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"end3DRenderer",
			L"boolean", L"long flags = 0",
			NULL, &RSRenderContextClass::method_end3DRenderer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getRenderDevice",
			L"RenderDevice", L"long flags = 0",
			NULL, &RSRenderContextClass::method_getRenderDevice, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"addTemporaryObject", NULL, L"NativeObject obj",
			NULL, &RSRenderContextClass::method_addTemporaryObject, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSRenderContextClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<S3DRenderContextInterface>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DRenderContextInterface *
	RSRenderContextClass::GetThisRenderContext( RSContext& context, RSObject* pThis )
{
	return	RSNativeObject::GetNative<S3DRenderContextInterface>( pThis ) ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.<init> の this が RenderContext ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( (SGLPaintContextInterface*) new S3DRenderContext ) ;
	return	NULL ;
}

// boolean attachMultiTargetImages( Image[] targets )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_attachMultiTargetImages
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.attachMultiTargetImages の this が null です" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pTargets = arg.ObjectAt(0) ;
	if ( pTargets == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.attachMultiTargetImages の引数が null です",
											L"IllegalArgumentException" ) ;
		return	NULL ;
	}
	SPointerArray<SGLImageObject>	aTargets ;
	for ( size_t i = 0; i < pTargets->GetElementCount(); i ++ )
	{
		RSSmartPtr	prsImage = pTargets->GetElementAt( context, (int) i ) ;
		aTargets.Add( RSNativeObject::GetNative<SGLImageObject>( prsImage ) ) ;
	}
	if ( pRender->AttachMultiTargetImages
			( aTargets.GetConstArray(), aTargets.GetLength() ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// const Image[] getMultiTargetImages()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_getMultiTargetImages
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.getMultiTargetImages の this が null です" ) ;
		return	NULL ;
	}
	size_t	nCount = 0 ;
	SGLImageObject*const*
			ppImages = pRender->GetMultiTargetImages( nCount ) ;
	//
	RSClass *	pImageClass = context.GetClassAs( L"Image" ) ;
	RSArray *	pArray = context.new_Array( nCount, pImageClass ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		context.ReleaseObjectRef
			( pArray->SetElementAt
				( context, (int) i,
					new RSNativeObject( ppImages[i], pImageClass ) ) ) ;
	}
	return	pArray ;
}

// boolean setProjectionScreen
//	( Vector3D vScreen, double zScale = 1.0, double pixelAspect = 1.0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_setProjectionScreen
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.setProjectionScreen の this が null です" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pvScreen = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	if ( pvScreen == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.setProjectionScreen の引数が null です",
											L"IllegalArgumentException" ) ;
		return	NULL ;
	}
	if ( pRender->SetProjectionScreen
		( *pvScreen, arg.DoubleAt( 1, 1.0 ), arg.DoubleAt( 2, 1.0 ) ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean getProjectionScreen
//	( Vector3D vScreen, Float64Pointer zScale, Float64Pointer pixelAspect )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_getProjectionScreen
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.getProjectionScreen の this が null です" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pvScreen = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	float64_t *	pScale = (float64_t*) arg.PointerAt( 1, sizeof(float64_t) ) ;
	float64_t *	pAspect = (float64_t*) arg.PointerAt( 2, sizeof(float64_t) ) ;
	if ( (pvScreen == NULL) || (pScale == NULL) || (pAspect == NULL) )
	{
		context.ThrowExceptionError
			( L"RenderContext.getProjectionScreen の引数が null です",
											L"IllegalArgumentException" ) ;
		return	NULL ;
	}
	if ( pRender->GetProjectionScreen( *pvScreen, *pScale, *pAspect ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean getPerspectiveMatrix( Matrix4D matPers )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_getPerspectiveMatrix
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.getPerspectiveMatrix の this が null です" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S4DMatrix *	pmatPers = (S4DMatrix*) arg.PointerAt( 0, sizeof(S4DMatrix) ) ;
	if ( pmatPers == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.getPerspectiveMatrix の引数が null です",
											L"IllegalArgumentException" ) ;
		return	NULL ;
	}
	if ( pRender->GetPerspectiveMatrix( *pmatPers ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// void setPerspectiveMatrix
//	( int stereoViewIndex, Matrix4D matPers, boolean flagPersMatrix = true )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_setPerspectiveMatrix
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.setPerspectiveMatrix の this が null です" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S4DMatrix *	pmatPers = (S4DMatrix*) arg.PointerAt( 1, sizeof(S4DMatrix) ) ;
	if ( pmatPers == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.setPerspectiveMatrix の引数が null です",
											L"IllegalArgumentException" ) ;
		return	NULL ;
	}
	S3DRenderContextInterface::StereoViewIndex
		svi = (S3DRenderContextInterface::StereoViewIndex) arg.IntAt(0) ;
	pRender->SetPerspectiveMatrix( svi, *pmatPers, arg.BooleanAt( 2, true ) ) ;
	return	NULL ;
}

// void enablePerspectiveMatrix( boolean flagPersMatrix )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_enablePerspectiveMatrix
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.enablePerspectiveMatrix の this が null です" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pRender->EnablePerspectiveMatrix( arg.BooleanAt( 0, true ) ) ;
	return	NULL ;
}

// void setCamera( Matrix3D matCamera, Vector3D vCamera )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_setCamera
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.setCamera の this が null です" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGL3DMatrix<float32_t,3> *	pmatCamera =
		(SGL3DMatrix<float32_t,3>*) arg.PointerAt( 0, sizeof(SGL3DMatrix<float32_t,3>) ) ;
	S3DVector *	pvCamera = (S3DVector*) arg.PointerAt( 1, sizeof(S3DVector) ) ;
	if ( (pmatCamera == NULL) || (pvCamera == NULL) )
	{
		context.ThrowExceptionError
			( L"RenderContext.setCamera の引数が null です",
								L"IllegalArgumentException" ) ;
		return	NULL ;
	}
	S3DDMatrix	matdCamera
		( pmatCamera->m[0][0], pmatCamera->m[0][1], pmatCamera->m[0][2],
			pmatCamera->m[1][0], pmatCamera->m[1][1], pmatCamera->m[1][2],
			pmatCamera->m[2][0], pmatCamera->m[2][1], pmatCamera->m[2][2] ) ;
	S3DDVector	vdCamera = *pvCamera ;
	//
	pRender->SetCamera( matdCamera, vdCamera ) ;
	//
	return	NULL ;
}

// void getCamera( Matrix3D matCamera, Vector3D vCamera )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_getCamera
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.getCamera の this が null です" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGL3DMatrix<float32_t,3> *	pmatCamera =
		(SGL3DMatrix<float32_t,3>*) arg.PointerAt( 0, sizeof(SGL3DMatrix<float32_t,3>) ) ;
	S3DVector *	pvCamera = (S3DVector*) arg.PointerAt( 1, sizeof(S3DVector) ) ;
	if ( (pmatCamera == NULL) || (pvCamera == NULL) )
	{
		context.ThrowExceptionError
			( L"RenderContext.getCamera の引数が null です",
								L"IllegalArgumentException" ) ;
		return	NULL ;
	}
	S3DDMatrix	matdCamera ;
	S3DDVector	vdCamera ;
	//
	pRender->GetCamera( matdCamera, vdCamera ) ;
	//
	for ( int i = 0; i < 3; i ++ )
	{
		pmatCamera->m[i][0] = (float32_t) matdCamera.m[i][0] ;
		pmatCamera->m[i][1] = (float32_t) matdCamera.m[i][1] ;
		pmatCamera->m[i][2] = (float32_t) matdCamera.m[i][2] ;
	}
	*pvCamera = vdCamera ;
	//
	return	NULL ;
}

// void setInverseCameraTransformation
//		( Color3D color = null, int transparency = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_setInverseCameraTransformation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.setInverseCameraTransformation の this が null です" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DColor *	pColor = (S3DColor*) arg.PointerAt( 0, sizeof(S3DColor) ) ;
	//
	pRender->SetInverseCameraTransformation
				( pColor, (unsigned int) arg.IntAt( 2 ) ) ;
	//
	return	NULL ;
}

// const boolean isSphereIntoView( Vector3D vLocalPos, double radius )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_isSphereIntoView
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.isSphereIntoView の this が null です" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pvLocalPos = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	if ( pvLocalPos == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.isSphereIntoView の引数が null です",
								L"IllegalArgumentException" ) ;
		return	NULL ;
	}
	S3DDVector	vLocalPos = *pvLocalPos ;
	//
	return	context.new_Boolean
		( pRender->IsSphereIntoView( vLocalPos, arg.DoubleAt(1) ) ) ;
}

// const boolean getProjectedPosition
//	( Vector2D vProjPos, Vector3D vLocalPos, double fpLimit = 100000.0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_getProjectedPosition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.getProjectedPosition の this が null です" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S2DVector *	pvProjPos = (S2DVector*) arg.PointerAt( 0, sizeof(S2DVector) ) ;
	S3DVector *	pvLocalPos = (S3DVector*) arg.PointerAt( 1, sizeof(S3DVector) ) ;
	if ( pvLocalPos == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.getProjectedPosition の引数が null です",
									L"IllegalArgumentException" ) ;
		return	NULL ;
	}
	S2DDVector	vdProjPos ;
	S3DDVector	vdLocalPos = *pvLocalPos ;
	bool		flagResult =
					pRender->GetProjectedPosition
						( vdProjPos, vdLocalPos, arg.DoubleAt( 2, 100000.0 ) ) ;
	//
	*pvProjPos = vdProjPos ;
	return	context.new_Boolean( flagResult ) ;
}

// void setParallax
//	( double xParallax, double zFocusRate, double xScreenDelta )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_setParallax
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.setParallax の this が null です" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pRender->SetParallax( arg.DoubleAt(0), arg.DoubleAt(1), arg.DoubleAt(2) ) ;
	return	NULL ;
}

// const double getParallax()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_getParallax
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.getParallax の this が null です" ) ;
		return	NULL ;
	}
	return	context.new_Number( pRender->GetParallax() ) ;
}

// void setZClipRange( double zMin, double zMax )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_setZClipRange
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.setZClipRange の this が null です" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pRender->SetZClipRange( arg.DoubleAt(0), arg.DoubleAt(1) ) ;
	return	NULL ;
}

// void setLightEntries( RenderContext.Light pLights, int countLights )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_setLightEntries
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.setLightEntries の this が null です" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	const size_t	nCount = (size_t) arg.IntAt( 1 ) ;
	S3DLightEntry *	pLights =
		(S3DLightEntry*) arg.PointerAt( 0, nCount * sizeof(S3DLightEntry) ) ;
	if ( (pLights == NULL) && (nCount > 0) )
	{
		context.ThrowExceptionError
			( L"RenderContext.setLightEntries の引数が null です",
									L"IllegalArgumentException" ) ;
		return	NULL ;
	}
	pRender->SetLightEntries( pLights, nCount ) ;
	return	NULL ;
}

// void setShadowMap
//	( int idLight, Image * pShadowDepth,
//		RenderContext.ShadowMpaInfo infShadowMap )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_setShadowMap
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.setShadowMap の this が null です" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	const uint32_t		idLight = (uint32_t) arg.IntAt( 0 ) ;
	SGLImageObject *	pDepth = ESLTypeCast<SGLImageObject>( arg.NativeObjectAt( 1 ) ) ;
	S3DShadowMapInfo *	pShadowMapInf =
		(S3DShadowMapInfo*) arg.PointerAt( 2, sizeof(S3DShadowMapInfo) ) ;
	if ( pShadowMapInf == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.setShadowMap の引数が null です",
									L"IllegalArgumentException" ) ;
		return	NULL ;
	}
	pRender->SetShadowMap( idLight, pDepth, *pShadowMapInf ) ;
	return	NULL ;
}

// void setFog( int rgbFog, double zFogNear, double zFogFar )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_setFog
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.setFog の this が null です" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pRender->SetFog
		( (uint32_t) arg.IntAt( 0 ), arg.DoubleAt( 1 ), arg.DoubleAt( 2 ) ) ;
	return	NULL ;
}

// void enableFog( boolean flagEnable )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_enableFog
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.enableFog の this が null です" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pRender->EnableFog( arg.BooleanAt( 0 ) ) ;
	return	NULL ;
}

// const int currentParallaxView()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_currentParallaxView
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.currentParallaxView の this が null です" ) ;
		return	NULL ;
	}
	return	context.new_Integer( pRender->CurrentParallaxView() ) ;
}

// boolean selectParallaxView( int stereoViewIndex )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_selectParallaxView
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.selectParallaxView の this が null です" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	if ( pRender->SelectParallaxView
		( (S3DRenderContextInterface::StereoViewIndex) arg.IntAt(0) ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean begin3DRenderer( long flags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_begin3DRenderer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.begin3DRenderer の this が null です" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	if ( pRender->Begin3DRenderer( arg.LongAt(0) ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean end3DRenderer( long flags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_end3DRenderer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.end3DRenderer の this が null です" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	if ( pRender->End3DRenderer( arg.LongAt(0) ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// RenderDevice getRenderDevice( long flags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_getRenderDevice
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.getRenderDevice の this が null です" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DRenderDevice *	pDevice = pRender->GetRenderDeviceObject( arg.LongAt(0) ) ;
	if ( pDevice == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject( pDevice, context.GetClassAs( L"RenderDevice" ) ) ;
}

// void addTemporaryObject( NativeObject obj )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderContextClass::method_addTemporaryObject
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DRenderContextInterface *	pRender = GetThisRenderContext( context, pThis ) ;
	if ( pRender == NULL )
	{
		context.ThrowExceptionError
			( L"RenderContext.addTemporaryObject の this が null です" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSNativeObject *	pNObj = ESLTypeCast<RSNativeObject>( arg.ObjectAt( 0 ) ) ;
	if ( (pNObj != NULL) && pNObj->IsObjectOwner() )
	{
		SSystem::SObject *	pObj = pNObj->DetachObject() ;
		pNObj->AttachObject( pObj ) ;
		pRender->AddTemporaryObject( pObj ) ;
	}
	return	NULL ;
}
