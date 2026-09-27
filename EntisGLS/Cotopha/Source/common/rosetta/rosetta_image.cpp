
#include <rosetta/rosetta.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl2d/sgl_image_filter.h>
#include <sakuragl/sgl2d/sgl_image_conversion.h>
#include <sakuragl/sgl2d/sgl_image_buf_object.h>
#include <rosetta/rosetta_reference.h>
#include <rosetta/rosetta_array.h>
#include <rosetta/rosetta_image.h>

using namespace	SSystem ;
using namespace	SakuraGL ;
using namespace	Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// Point クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSPointClass, RSStructuredPointerClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSPointClass::RSPointClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSStructuredPointerClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSPointClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStructuredPointer( this, NULL ) ;
	//
	AddArrayMemberAs
		( context, L"x", context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"y", context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"int x, int y",
				NULL, &RSPointClass::method_init, NULL ) ;
}

// void <init>( int x, int y )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPointClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Point 構築関数の this が Structure ではありません" ) ;
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
	SGLPoint *	pPtr = (SGLPoint*) pBuf->m_ptrBuf ;
	pPtr->x = arg.IntAt( 0 ) ;
	pPtr->y = arg.IntAt( 1 ) ;
	return	NULL ;
}



//////////////////////////////////////////////////////////////////////////////
// Size クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSizeClass, RSStructuredPointerClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSizeClass::RSSizeClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSStructuredPointerClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSizeClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStructuredPointer( this, NULL ) ;
	//
	AddArrayMemberAs
		( context, L"w", context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"h", context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"int x, int y",
				NULL, &RSSizeClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isEmpty", L"boolean", L"",
				NULL, &RSSizeClass::method_isEmpty,
				NULL, RSFunctionPrototype::flagConstant ) ;
}

// void <init>( int w, int w )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSizeClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Size 構築関数の this が Structure ではありません" ) ;
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
	SGLSize *	pPtr = (SGLSize*) pBuf->m_ptrBuf ;
	pPtr->w = arg.IntAt( 0 ) ;
	pPtr->h = arg.IntAt( 1 ) ;
	return	NULL ;
}

// boolean isEmpty()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSizeClass::method_isEmpty
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	context.new_Boolean
				( (pThis->GetMemberIntegerAs( context, L"w" ) == 0)
					&&  (pThis->GetMemberIntegerAs( context, L"h" ) == 0) ) ;
}



//////////////////////////////////////////////////////////////////////////////
// Rect クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSRectClass, RSStructuredPointerClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSRectClass::RSRectClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSStructuredPointerClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSRectClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStructuredPointer( this, NULL ) ;
	//
	AddArrayMemberAs
		( context, L"x", context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"y", context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"w", context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"h", context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"int x, int y, int w, int h",
				NULL, &RSRectClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getPosition", L"Point", L"",
				NULL, &RSRectClass::method_getPosition,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getSize", L"Size", L"",
				NULL, &RSRectClass::method_getSize,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setPosition", NULL, L"int x, int y",
				NULL, &RSRectClass::method_setPosition, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setSize", NULL, L"int w, int h",
				NULL, &RSRectClass::method_setSize, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isEmpty", L"boolean", L"",
				NULL, &RSRectClass::method_isEmpty,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"clear", NULL, L"",
				NULL, &RSRectClass::method_clear, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isBounds", L"boolean", L"int x, int y",
				NULL, &RSRectClass::method_isBounds,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"test", L"boolean", L"Rect rect",
				NULL, &RSRectClass::method_test,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"and", L"Rect", L"Rect rect",
				NULL, &RSRectClass::method_and,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"or", L"Rect", L"Rect rect",
				NULL, &RSRectClass::method_or,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"clone", L"Rect", L"",
				NULL, &RSRectClass::method_clone,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"copy", L"Rect", L"Rect rect",
				NULL, &RSRectClass::method_copy, NULL ) ;
}

// Object - SGLImageRect 変換
//////////////////////////////////////////////////////////////////////////////
SGLImageRect * RSRectClass::ImageRectFromObject
	( RSContext& context, SGLImageRect& rect, RSObject * pObj )
{
	if ( pObj == NULL )
	{
		return	NULL ;
	}
	pObj = pObj->GetEntityObject() ;
	if ( pObj == NULL )
	{
		return	NULL ;
	}
	RSStructuredPointer *	pPtr = ESLTypeCast<RSStructuredPointer>( pObj ) ;
	if ( pPtr->m_nLimit < sizeof(SGLImageRect) )
	{
		return	NULL ;
	}
	rect = *((SGLImageRect*) pPtr->GetPointer()) ;
	return	&rect ;
}

SakuraGL::SGLImageRect *
	RSRectClass::GetThisRect( RSContext& context, RSObject * pThis )
{
	if ( pThis == NULL )
	{
		return	NULL ;
	}
	pThis = pThis->GetEntityObject() ;
	if ( pThis == NULL )
	{
		return	NULL ;
	}
	RSStructuredPointer *	pPtr = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pPtr->m_nLimit < sizeof(SGLImageRect) )
	{
		return	NULL ;
	}
	return	(SGLImageRect*) pPtr->GetPointer() ;
}

// void <init>( int x, int y, int w, int h )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRectClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Rect 構築関数の this が Structure ではありません" ) ;
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
	SGLImageRect *	pPtr = (SGLImageRect*) pBuf->m_ptrBuf ;
	pPtr->x = arg.IntAt( 0 ) ;
	pPtr->y = arg.IntAt( 1 ) ;
	pPtr->w = arg.IntAt( 2 ) ;
	pPtr->h = arg.IntAt( 3 ) ;
	return	NULL ;
}

// Point getPosition()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRectClass::method_getPosition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSObject *	p = context.new_Object( L"Point" ) ;
	ESLAssert( p != NULL ) ;
	p->SetMemberIntegerAs
		( context, L"x", pThis->GetMemberIntegerAs( context, L"x" ) ) ;
	p->SetMemberIntegerAs
		( context, L"y", pThis->GetMemberIntegerAs( context, L"y" ) ) ;
	return	p ;
}

// Size getSize()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRectClass::method_getSize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSObject *	p = context.new_Object( L"Size" ) ;
	ESLAssert( p != NULL ) ;
	p->SetMemberIntegerAs
		( context, L"w", pThis->GetMemberIntegerAs( context, L"w" ) ) ;
	p->SetMemberIntegerAs
		( context, L"h", pThis->GetMemberIntegerAs( context, L"h" ) ) ;
	return	p ;
}

// void setPosition( int x, int y )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRectClass::method_setPosition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	pThis->SetMemberIntegerAs( context, L"x", arg.LongAt( 0 ) ) ;
	pThis->SetMemberIntegerAs( context, L"y", arg.LongAt( 1 ) ) ;
	return	NULL ;
}

// void setSize( int w, int h )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRectClass::method_setSize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	pThis->SetMemberIntegerAs( context, L"w", arg.LongAt( 0 ) ) ;
	pThis->SetMemberIntegerAs( context, L"h", arg.LongAt( 1 ) ) ;
	return	NULL ;
}

// boolean isEmpty()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRectClass::method_isEmpty
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	context.new_Boolean
				( (pThis->GetMemberIntegerAs( context, L"w" ) == 0)
					&&  (pThis->GetMemberIntegerAs( context, L"h" ) == 0) ) ;
}

// void clear()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRectClass::method_clear
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	pThis->SetMemberIntegerAs( context, L"x", 0 ) ;
	pThis->SetMemberIntegerAs( context, L"y", 0 ) ;
	pThis->SetMemberIntegerAs( context, L"w", 0 ) ;
	pThis->SetMemberIntegerAs( context, L"h", 0 ) ;
	return	NULL ;
}

// boolean isBounds( int x, int y )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRectClass::method_isBounds
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageRect	irctThis ;
	if ( ImageRectFromObject( context, irctThis, pThis ) == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean( irctThis.IsBounds( arg.IntAt(0), arg.IntAt(0) ) ) ;
}

// boolean test( Rect rect )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRectClass::method_test
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageRect	irctThis ;
	if ( ImageRectFromObject( context, irctThis, pThis ) == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageRect	irctObj ;
	if ( ImageRectFromObject( context, irctObj, arg.ObjectAt(0) ) == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SGLRect	rectThis( irctThis ) ;
	SGLRect	rectObj( irctObj ) ;
	return	context.new_Boolean( rectThis &= rectObj ) ;
}

// Rect and( Rect rect )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRectClass::method_and
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageRect	irctThis ;
	if ( ImageRectFromObject( context, irctThis, pThis ) == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageRect	irctObj ;
	if ( ImageRectFromObject( context, irctObj, arg.ObjectAt(0) ) == NULL )
	{
		return	NULL ;
	}
	SGLRect	rectThis( irctThis ) ;
	SGLRect	rectObj( irctObj ) ;
	rectThis &= rectObj ;
	irctThis = rectThis ;
	//
	RSObject *	p = context.new_Object( L"Rect" ) ;
	ESLAssert( p != NULL ) ;
	p->SetMemberIntegerAs( context, L"x", irctThis.x ) ;
	p->SetMemberIntegerAs( context, L"y", irctThis.y ) ;
	p->SetMemberIntegerAs( context, L"w", irctThis.w ) ;
	p->SetMemberIntegerAs( context, L"h", irctThis.h ) ;
	return	p ;
}

// Rect or( Rect rect )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRectClass::method_or
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageRect	irctThis ;
	if ( ImageRectFromObject( context, irctThis, pThis ) == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageRect	irctObj ;
	if ( ImageRectFromObject( context, irctObj, arg.ObjectAt(0) ) == NULL )
	{
		return	NULL ;
	}
	SGLRect	rectThis( irctThis ) ;
	SGLRect	rectObj( irctObj ) ;
	rectThis |= rectObj ;
	irctThis = rectThis ;
	//
	RSObject *	p = context.new_Object( L"Rect" ) ;
	ESLAssert( p != NULL ) ;
	p->SetMemberIntegerAs( context, L"x", irctThis.x ) ;
	p->SetMemberIntegerAs( context, L"y", irctThis.y ) ;
	p->SetMemberIntegerAs( context, L"w", irctThis.w ) ;
	p->SetMemberIntegerAs( context, L"h", irctThis.h ) ;
	return	p ;
}

// Rect clone()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRectClass::method_clone
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageRect	irctThis ;
	if ( ImageRectFromObject( context, irctThis, pThis ) == NULL )
	{
		return	NULL ;
	}
	RSObject *	p = context.new_Object( L"Rect" ) ;
	ESLAssert( p != NULL ) ;
	p->SetMemberIntegerAs( context, L"x", irctThis.x ) ;
	p->SetMemberIntegerAs( context, L"y", irctThis.y ) ;
	p->SetMemberIntegerAs( context, L"w", irctThis.w ) ;
	p->SetMemberIntegerAs( context, L"h", irctThis.h ) ;
	return	p ;
}

// Rect copy( Rect rect )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRectClass::method_copy
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageRect *	pThisRect = GetThisRect( context, pThis ) ;
	if ( pThisRect == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageRect	irctObj ;
	if ( ImageRectFromObject( context, irctObj, arg.ObjectAt(0) ) == NULL )
	{
		return	NULL ;
	}
	*pThisRect = irctObj ;
	//
	RSObject::AddRef( pThis );
	return	pThis ;
}



//////////////////////////////////////////////////////////////////////////////
// RGBColor 構造体
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( Rosetta::RSRGBColorClass, RSStructuredPointerClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSRGBColorClass::RSRGBColorClass
	( RSClass * pClass, const wchar_t * pwszClassName )
	: RSStructuredPointerClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSRGBColorClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStructuredPointer( this, NULL ) ;
	//
	AddArrayMemberAs
		( context, L"blue",
			context.GetBasicTypeClass(RSCodeControl::wiByte),
			0, 0, context.new_Integer(0) ) ;
	AddArrayMemberAs
		( context, L"green",
			context.GetBasicTypeClass(RSCodeControl::wiByte),
			0, 0, context.new_Integer(0) ) ;
	AddArrayMemberAs
		( context, L"red",
			context.GetBasicTypeClass(RSCodeControl::wiByte),
			0, 0, context.new_Integer(0) ) ;
	AddArrayMemberAs
		( context, L"alpha",
			context.GetBasicTypeClass(RSCodeControl::wiByte),
			0, 0, context.new_Integer(0) ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>",
				NULL, L"int red, int green, int blue, int alpha",
				NULL, &RSRGBColorClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getInt",
				L"int", L"",
				NULL, &RSRGBColorClass::method_getInt,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getRed",
				L"int", L"",
				NULL, &RSRGBColorClass::method_getRed,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getGreen",
				L"int", L"",
				NULL, &RSRGBColorClass::method_getGreen,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getBlue",
				L"int", L"",
				NULL, &RSRGBColorClass::method_getBlue,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getAlpha",
				L"int", L"",
				NULL, &RSRGBColorClass::method_getAlpha,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setInt",
				L"RGBColor", L"int argb",
				NULL, &RSRGBColorClass::method_setInt, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"copy",
				L"RGBColor", L"RGBColor color",
				NULL, &RSRGBColorClass::method_copy, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"mul",
				L"RGBColor", L"int alpha",
				NULL, &RSRGBColorClass::method_mul, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"blendTo",
				L"int", L"int argb",
				NULL, &RSRGBColorClass::method_blendTo,
				NULL, RSFunctionPrototype::flagConstant ) ;
}

// this 実体ポインタを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLPalette *
	RSRGBColorClass::GetThisColor( RSContext& context, RSObject* pThis )
{
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"this が RGBColor ではありません" ) ;
		return	NULL ;
	}
	SGLPalette *	pPtr = (SGLPalette*) pObj->GetPointer( sizeof(SGLPalette) ) ;
	if ( pPtr == NULL )
	{
		context.ThrowExceptionError
			( L"RGBColor の this ポインタが有効ではありません" ) ;
	}
	return	pPtr ;
}

// void <init>( int red, int green, int blue, int alpha )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRGBColorClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"RGBColor 構築関数の this が Structure ではありません" ) ;
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
	SGLPalette *	pPtr = (SGLPalette*) pBuf->m_ptrBuf ;
	pPtr->argb.Red = (uint8_t) arg.IntAt( 0 ) ;
	pPtr->argb.Green = (uint8_t) arg.IntAt( 1 ) ;
	pPtr->argb.Blue = (uint8_t) arg.IntAt( 2 ) ;
	pPtr->argb.Alpha = (uint8_t) arg.IntAt( 3 ) ;
	return	NULL ;
}

// const int getInt( void )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRGBColorClass::method_getInt
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPalette *	pPtr = GetThisColor( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pPtr->ui32 ) ;
}

// const int getRed( void )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRGBColorClass::method_getRed
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPalette *	pPtr = GetThisColor( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pPtr->argb.Red ) ;
}

// const int getGreen( void )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRGBColorClass::method_getGreen
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPalette *	pPtr = GetThisColor( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pPtr->argb.Green ) ;
}

// const int getBlue( void )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRGBColorClass::method_getBlue
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPalette *	pPtr = GetThisColor( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pPtr->argb.Blue ) ;
}

// const int getAlpha( void )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRGBColorClass::method_getAlpha
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPalette *	pPtr = GetThisColor( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pPtr->argb.Alpha ) ;
}

// RGBColor setInt( int argb )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRGBColorClass::method_setInt
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPalette *	pPtr = GetThisColor( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pPtr->ui32 = (uint32_t) arg.IntAt( 0 ) ;
	//
	RSObject::AddRef( pThis );
	return	pThis ;
}

// RGBColor copy( RGBColor color )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRGBColorClass::method_copy
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPalette *	pPtr = GetThisColor( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLPalette *	pSrc = (SGLPalette*) arg.PointerAt( 0, sizeof(SGLPalette) ) ;
	if ( pSrc == NULL )
	{
		context.ThrowExceptionError( L"RGBColor.copy 関数の引数が不正です" ) ;
		return	NULL ;
	}
	*pPtr = *pSrc ;
	//
	RSObject::AddRef( pThis );
	return	pThis ;
}

// RGBColor mul( int alpha )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRGBColorClass::method_mul
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPalette *	pPtr = GetThisColor( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	*pPtr *= (unsigned int) esl_clampi( arg.IntAt(0), 0, 0x100 ) ;
	//
	RSObject::AddRef( pThis );
	return	pThis ;
}

// const int blendTo( int argb )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRGBColorClass::method_blendTo
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPalette *	pPtr = GetThisColor( context, pThis ) ;
	if ( pPtr == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLPalette	rgbTemp( (uint32_t) arg.IntAt(0) ) ;
	rgbTemp *= *pPtr ;
	//
	return	context.new_Integer( rgbTemp.ui32 ) ;
}



//////////////////////////////////////////////////////////////////////////////
// Vector2D クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSVector2DClass, RSStructuredPointerClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSVector2DClass::RSVector2DClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSStructuredPointerClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSVector2DClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStructuredPointer( this, NULL ) ;
	//
	AddArrayMemberAs
		( context, L"x", context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"y", context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"double x, double y",
				NULL, &RSVector2DClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"equals", L"boolean", L"Vector2D v",
				NULL, &RSVector2DClass::method_equals,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"clone",
				L"Vector2D", L"",
				NULL, &RSVector2DClass::method_clone,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"copy",
				L"Vector2D", L"Vector2D src",
				NULL, &RSVector2DClass::method_copy, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"add",
				L"Vector2D", L"Vector2D v",
				NULL, &RSVector2DClass::method_add, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"sub",
				L"Vector2D", L"Vector2D v",
				NULL, &RSVector2DClass::method_sub, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"mul",
				L"Vector2D", L"double s",
				NULL, &RSVector2DClass::method_mul, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"div",
				L"Vector2D", L"double s",
				NULL, &RSVector2DClass::method_div, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"innerProduct", L"double", L"Vector2D v",
				NULL, &RSVector2DClass::method_innerProduct,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"absolute", L"double", L"",
				NULL, &RSVector2DClass::method_absolute,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"normalize", L"Vector2D", L"",
				NULL, &RSVector2DClass::method_normalize, NULL ) ;
}

// Object - S2DDVector 変換
//////////////////////////////////////////////////////////////////////////////
S2DVector RSVector2DClass::VectorFromObject( RSContext& context, RSObject * pObj )
{
	RSTypedArrayPointer *
		pPtr = ESLTypeCast<RSTypedArrayPointer>( pObj ) ;
	if ( pPtr != NULL )
	{
		return	*((S2DVector*) pPtr->GetPointer()) ;
	}
	S2DVector	v ;
	v.x = (float32_t) pObj->GetMemberNumberAs( context, L"x" ) ;
	v.y = (float32_t) pObj->GetMemberNumberAs( context, L"y" ) ;
	return	v ;
}

void RSVector2DClass::ObjectFromVector
		( RSContext& context, RSObject * pObj, const S2DVector& v )
{
	RSTypedArrayPointer *
		pPtr = ESLTypeCast<RSTypedArrayPointer>( pObj ) ;
	if ( pPtr != NULL )
	{
		S2DVector*	pv = (S2DVector*) pPtr->GetPointer() ;
		*pv = v ;
	}
	else
	{
		pObj->SetMemberNumberAs( context, L"x", v.x ) ;
		pObj->SetMemberNumberAs( context, L"y", v.y ) ;
	}
}

// void <init>( double x, double y )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector2DClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Vector2D 構築関数の this が Structure ではありません" ) ;
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
	S2DVector *	pPtr = (S2DVector*) pBuf->m_ptrBuf ;
	pPtr->x = (float32_t) arg.DoubleAt( 0 ) ;
	pPtr->y = (float32_t) arg.DoubleAt( 1 ) ;
	return	NULL ;
}

// boolean equals( Vector2D v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector2DClass::method_equals
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	p = arg.ObjectAt( 0 ) ;
	if ( p == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean
				( VectorFromObject( context, pThis ).
					IsEqual( VectorFromObject( context, p ) ) ) ;
}

// const Vector2D clone()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector2DClass::method_clone
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStructuredPointer *	pObj =
		ESLTypeCast<RSStructuredPointer>
			( context.new_StructuredPointer( L"Vector2D" ) ) ;
	ESLAssert( pObj != NULL ) ;
	*((S2DVector*) pObj->GetPointer()) = VectorFromObject( context, pThis ) ;
	return	pObj ;
}

// Vector2D copy( Vector2D v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector2DClass::method_copy
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	p = arg.ObjectAt( 0 ) ;
	if ( p == NULL )
	{
		return	NULL ;
	}
	S2DVector	v = VectorFromObject( context, p ) ;
	ObjectFromVector( context, pThis, v ) ;
	return	RSObject::AddRef( pThis ) ;
}

// Vector2D add( Vector2D v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector2DClass::method_add
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	p = arg.ObjectAt( 0 ) ;
	if ( p == NULL )
	{
		return	NULL ;
	}
	S2DVector	v = VectorFromObject( context, pThis )
					+ VectorFromObject( context, p ) ;
	ObjectFromVector( context, pThis, v ) ;
	return	RSObject::AddRef( pThis ) ;
}

// Vector2D sub( Vector2D v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector2DClass::method_sub
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	p = arg.ObjectAt( 0 ) ;
	if ( p == NULL )
	{
		return	NULL ;
	}
	S2DVector	v = VectorFromObject( context, pThis )
					- VectorFromObject( context, p ) ;
	ObjectFromVector( context, pThis, v ) ;
	return	RSObject::AddRef( pThis ) ;
}

// Vector2D mul( double s )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector2DClass::method_mul
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	S2DVector	v = VectorFromObject( context, pThis ) * arg.DoubleAt(0) ;
	ObjectFromVector( context, pThis, v ) ;
	return	RSObject::AddRef( pThis ) ;
}

// Vector2D div( double s )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector2DClass::method_div
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	S2DVector	v = VectorFromObject( context, pThis ) / arg.DoubleAt(0) ;
	ObjectFromVector( context, pThis, v ) ;
	return	RSObject::AddRef( pThis ) ;
}

// double innerProduct( Vector2D v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector2DClass::method_innerProduct
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	p = arg.ObjectAt( 0 ) ;
	if ( p == NULL )
	{
		return	NULL ;
	}
	return	context.new_Number
			( VectorFromObject( context, pThis ).
				InnerProduct( VectorFromObject( context, p ) ) ) ;
}

// double absolute()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector2DClass::method_absolute
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	context.new_Number
				( VectorFromObject( context, pThis ).Absolute() ) ;
}

// Vector2D normalize()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVector2DClass::method_normalize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S2DVector	v = VectorFromObject( context, pThis ) ;
	v.Normalize() ;
	ObjectFromVector( context, pThis, v ) ;
	return	RSObject::AddRef( pThis ) ;
}



//////////////////////////////////////////////////////////////////////////////
// Affine クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSAffineClass, RSStructuredPointerClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSAffineClass::RSAffineClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSStructuredPointerClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSAffineClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStructuredPointer( this, NULL ) ;
	//
	AddArrayMemberAs
		( context, L"a11", context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"a21", context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"a12", context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"a22", context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"a13", context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"a23", context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	//
	SGLAffine *	pAffine = (SGLAffine*) m_bufInit.GetArray( sizeof(SGLAffine) ) ;
	*pAffine = SGLAffine() ;
	m_bufInit.FinishArray() ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"transformVector",
				L"Vector2D", L"Vector2D v",
				NULL, &RSAffineClass::method_transformVector,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"transformVectors",
				NULL, L"Vector2D vDst, Vector2D vSrc, int count",
				NULL, &RSAffineClass::method_transformVectors,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"inverseOf", NULL, L"Affine af",
				NULL, &RSAffineClass::method_inverseOf, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"mappingOf",
				NULL, L"Vector2D vDst, Vector2D vSrc",
				NULL, &RSAffineClass::method_mappingOf, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getPosition", L"Vector2D", L"",
				NULL, &RSAffineClass::method_getPosition,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setPosition", NULL, L"double x, double y",
				NULL, &RSAffineClass::method_setPosition, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isRotation", L"boolean", L"",
				NULL, &RSAffineClass::method_isRotation,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setRotation", NULL, L"double rad",
				NULL, &RSAffineClass::method_setRotation, NULL ) ;
}

// Object - SGLAffine 変換
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLAffine RSAffineClass::AffineFromObject
	( RSContext& context, RSObject * pObj )
{
	SGLAffine	af ;
	af.a11 = (float32_t) pObj->GetMemberNumberAs( context, L"a11" ) ;
	af.a21 = (float32_t) pObj->GetMemberNumberAs( context, L"a21" ) ;
	af.a12 = (float32_t) pObj->GetMemberNumberAs( context, L"a12" ) ;
	af.a22 = (float32_t) pObj->GetMemberNumberAs( context, L"a22" ) ;
	af.a13 = (float32_t) pObj->GetMemberNumberAs( context, L"a13" ) ;
	af.a23 = (float32_t) pObj->GetMemberNumberAs( context, L"a23" ) ;
	return	af ;
}

void RSAffineClass::ObjectFromAffine
	( RSContext& context, RSObject * pObj, const SakuraGL::SGLAffine& af )
{
	pObj->SetMemberNumberAs( context, L"a11", af.a11 ) ;
	pObj->SetMemberNumberAs( context, L"a21", af.a21 ) ;
	pObj->SetMemberNumberAs( context, L"a12", af.a12 ) ;
	pObj->SetMemberNumberAs( context, L"a22", af.a22 ) ;
	pObj->SetMemberNumberAs( context, L"a13", af.a13 ) ;
	pObj->SetMemberNumberAs( context, L"a23", af.a23 ) ;
}

// Vector2D transformVector( Vector2D v )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAffineClass::method_transformVector
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pSrc = arg.ObjectAt( 0 ) ;
	if ( pSrc == NULL )
	{
		context.ThrowExceptionError
			( L"Affine.transformVector 引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	SGLAffine	afThis = AffineFromObject( context, pThis ) ;
	S2DDVector	vSrc = RSVector2DClass::VectorFromObject( context, pSrc ) ;
	afThis.TransformVectors( &vSrc, &vSrc, 1 ) ;
	//
	RSObject *	pDst = context.new_Object( L"Vector2D" ) ;
	ESLAssert( pDst != NULL ) ;
	RSVector2DClass::ObjectFromVector( context, pDst, vSrc ) ;
	return	pDst ;
}

// void transformVectors( Vector2D vDst, Vector2D vSrc, int count )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAffineClass::method_transformVectors
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t		nCount = (size_t) arg.IntAt( 2 ) ;
	S2DVector *	pvDst =
		(S2DVector*) arg.PointerAt( 0, nCount * sizeof(S2DVector) ) ;
	S2DVector *	pvSrc =
		(S2DVector*) arg.PointerAt( 1, nCount * sizeof(S2DVector) ) ;
	if ( (pvDst == NULL) || (pvSrc == NULL) )
	{
		context.ThrowExceptionError
			( L"Affine.transformVectors 引数が不正です" ) ;
		return	NULL ;
	}
	SGLAffine	afThis = AffineFromObject( context, pThis ) ;
	afThis.TransformVectors( pvDst, pvSrc, nCount ) ;
	return	NULL ;
}

// void inverseOf( Affine af )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAffineClass::method_inverseOf
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pSrc = arg.ObjectAt( 0 ) ;
	if ( pSrc == NULL )
	{
		context.ThrowExceptionError
			( L"Affine.inverseOf 引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	SGLAffine	afSrc = AffineFromObject( context, pSrc ) ;
	SGLAffine	afDst ;
	afDst.InverseOf( afSrc ) ;
	ObjectFromAffine( context, pThis, afDst ) ;
	return	NULL ;
}

// void mappingOf( Vector2D vDst, Vector2D vSrc )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAffineClass::method_mappingOf
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	S2DVector *	pvDst = (S2DVector*) arg.PointerAt( 0, sizeof(S2DVector) * 3 ) ;
	S2DVector *	pvSrc = (S2DVector*) arg.PointerAt( 1, sizeof(S2DVector) * 3 ) ;
	if ( (pvSrc == NULL) || (pvDst == NULL) )
	{
		context.ThrowExceptionError
			( L"Affine.mappingOf 引数が不正です", L"NullPointerException" ) ;
		return	NULL ;
	}
	SGLAffine	afDst ;
	afDst.MappingOf( pvDst, pvSrc ) ;
	ObjectFromAffine( context, pThis, afDst ) ;
	return	NULL ;
}

// Vector2D getPosition()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAffineClass::method_getPosition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSObject *	pDst = context.new_Object( L"Vector2D" ) ;
	ESLAssert( pDst != NULL ) ;
	SGLAffine	afThis = AffineFromObject( context, pThis ) ;
	RSVector2DClass::ObjectFromVector( context, pDst, afThis.GetPosition() ) ;
	return	pDst ;
}

// void setPosition( double x, double y )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAffineClass::method_setPosition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLAffine	afThis = AffineFromObject( context, pThis ) ;
	afThis.SetPosition( arg.DoubleAt(0), arg.DoubleAt(1) ) ;
	ObjectFromAffine( context, pThis, afThis ) ;
	return	NULL ;
}

// boolean isRotation()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAffineClass::method_isRotation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAffine	afThis = AffineFromObject( context, pThis ) ;
	return	context.new_Boolean( afThis.IsRotation() ) ;
}

// void setRotation( double rad )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAffineClass::method_setRotation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLAffine	afThis = AffineFromObject( context, pThis ) ;
	afThis.SetRotation( arg.DoubleAt( 0 ) ) ;
	ObjectFromAffine( context, pThis, afThis ) ;
	return	NULL ;
}



//////////////////////////////////////////////////////////////////////////////
// Image.BufferInfo クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSImageClass::BufferInfoClass, RSStructuredPointerClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSImageClass::BufferInfoClass::BufferInfoClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSStructuredPointerClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSImageClass::BufferInfoClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStructuredPointer( this, NULL ) ;
	//
	AddArrayMemberAs
		( context, L"format",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"depth",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"width",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"height",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"ptOrigin", context.GetClassAs( L"Point" ) ) ;
	AddArrayMemberAs
		( context, L"colorClip",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"pitchPixel",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"pitchLine",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"reserved",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
}


//////////////////////////////////////////////////////////////////////////////
// Image.BuildAtlasParam クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( Rosetta::RSImageClass::BuildAtlasParamClass, RSStructuredPointerClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSImageClass::BuildAtlasParamClass::BuildAtlasParamClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSStructuredPointerClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSImageClass::BuildAtlasParamClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStructuredPointer( this, NULL ) ;
	//
	AddArrayMemberAs
		( context, L"nFlags",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"nMargin",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"sizeAtlasMin", context.GetClassAs( L"Size" ) ) ;
	AddArrayMemberAs
		( context, L"sizeAtlasMax", context.GetClassAs( L"Size" ) ) ;
}



//////////////////////////////////////////////////////////////////////////////
// Image クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSImageClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSImageClass::RSImageClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSImageClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	BufferInfoClass *
		pBufInfoClass = new BufferInfoClass
							( context.GetClassClass(), L"BufferInfo" ) ;
	pBufInfoClass->Initialize( context ) ;
	pBufInfoClass->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"BufferInfo", pBufInfoClass ) ) ;
	//
	BuildAtlasParamClass *
		pBuildAtlasParam = new BuildAtlasParamClass
							( context.GetClassClass(), L"BuildAtlasParam" ) ;
	pBuildAtlasParam->Initialize( context ) ;
	pBuildAtlasParam->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"BuildAtlasParam", pBuildAtlasParam ) ) ;
	//
	CreateMemberIntegerAs
		( context, L"stereoBoth", stereoImageBoth, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"stereoRight", stereoImageRight, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"stereoLeft", stereoImageLeft, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"formatDefaultRGBA", formatImageDefaultRGBA, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"formatRGB", formatImageRGB, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"formatGray", formatImageGray, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"formatBGR", formatImageBGR, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"formatYUV", formatImageYUV, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"formatHSB", formatImageHSB, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"formatZ", formatImageZ, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"formatDepth", formatImageDepth, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"formatARGB", formatImageARGB, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"formatABGR", formatImageABGR, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"formatTypeMask", formatImageTypeMask, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"formatFlagDepth", formatImageFlagDepth, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"formatFlagPalette", formatImageFlagPalette, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"formatFlagClipping", formatImageFlagClipping, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"formatFlagAlpha", formatImageFlagAlpha, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"formatFlagNoProductOfAlpha", formatImageFlagNoProductOfAlpha, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"formatFlagSideBySide", formatImageFlagSideBySide, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"lockRead", SGLImageObject::lockRead, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"lockWrite", SGLImageObject::lockWrite, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"lockReadWrite", SGLImageObject::lockReadWrite, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"bufferNoWritable", SGLImageObject::bufferNoWritable, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"bufferNoReadable", SGLImageObject::bufferNoReadable, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"bufferNonPowerOf2", SGLImageObject::bufferNonPowerOf2, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"bufferForTexture", SGLImageObject::bufferForTexture, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"bufferForMipmapTexture", SGLImageObject::bufferForMipmapTexture, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"bufferForRenderTarget", SGLImageObject::bufferForRenderTarget, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"bufferCubeMapTexture", SGLImageObject::bufferCubeMapTexture, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"bufferDeviceRenderBuffer", SGLImageObject::bufferDeviceRenderBuffer, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"bufferMultisample", SGLImageObject::bufferMultisample, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"bufferCompressedTexture", SGLImageObject::bufferCompressedTexture, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"bufferTexture3D", SGLImageObject::bufferTexture3D, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"bufferSampleNoSmooth", SGLImageObject::bufferSampleNoSmooth, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"bufferSampleTiling", SGLImageObject::bufferSampleTiling, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"bufferOnMemory", SGLImageObject::bufferOnMemory, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"bufferOnDeviceOnly", SGLImageObject::bufferOnDeviceOnly, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"normalizeFormatMergeAnimation",
				SGLImageObject::formatMergeAnimation, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"normalizeFormatCastSize",
				SGLImageObject::formatCastSize, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"normalizeFormatCastSizeBottom",
				SGLImageObject::formatCastSizeBottom, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"atlasFlagExpandPOT",
				SGLImageObject::atlasFlagExpandPOT, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"toneBrightness", toneBrightness, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"toneMultiple", toneMultiple, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"toneAdditional", toneAdditional, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"toneOffsetMultiple", toneOffsetMultiple, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"toneGamma", toneGamma, modifierConst ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &RSImageClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getFrameCount", L"int", L"",
				NULL, &RSImageClass::method_getFrameCount,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getSequenceTable", L"int[]", L"",
				NULL, &RSImageClass::method_getSequenceTable,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getTotalTime", L"long", L"",
				NULL, &RSImageClass::method_getTotalTime,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"frameFromMilliSec", L"int", L"long msec",
				NULL, &RSImageClass::method_frameFromMilliSec,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"selectFrame",
				L"boolean", L"int iFrame, int iSide = Image.stereoRight",
				NULL, &RSImageClass::method_selectFrame, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getSelectedFrame", L"int", L"int[] side = null",
				NULL, &RSImageClass::method_getSelectedFrame,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getImageSize", L"Size", L"",
				NULL, &RSImageClass::method_getImageSize,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getImageInfo",
			L"boolean", L"Image.BufferInfo imginf",
				NULL, &RSImageClass::method_getImageInfo,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getPaletteTable", L"int[]", L"",
				NULL, &RSImageClass::method_getPaletteTable,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"lockBuffer",
			L"Uint8Pointer", L"Image.BufferInfo imginf, "
				L"int flags = Image.lockReadWrite, Rect rect = null",
				NULL, &RSImageClass::method_lockBuffer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"unlockBuffer",
			L"boolean", L"int flags = Image.lockReadWrite",
				NULL, &RSImageClass::method_unlockBuffer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"newReference",
			L"Image", L"Rect rectClip = null, int iFrame = -1, int iSide = Image.stereoRight",
				NULL, &RSImageClass::method_newReference, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"normalizeFormat", NULL,
			L"int format = 0, int depth = 0, "
			L"int flags = 0, int width = 0, int height = 0",
				NULL, &RSImageClass::method_normalizeFormat, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getReferenceRectOfAtlas",
			L"Image", L"Rect rectRef, int iFrame = -1",
				NULL, &RSImageClass::method_getReferenceRectOfAtlas, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"buildAtlas", L"Image",
			L"Image[] images, Image.BuildAtlasParam param",
				NULL, &RSImageClass::method_buildAtlas, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"estimateAtlasSize", L"int",
			L"Size sizeEstimated, "
				L"Image[] images, boolean flagMakePOT = false, int nMargin = 1, "
				L"int nSizeLimit = 0, Uint32Pointer pUsedIndex = null",
			nullptr, &RSImageClass::method_estimateAtlasSize, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"normalizeToTexture", NULL, L"int nFlags = 0",
				NULL, &RSImageClass::method_normalizeToTexture, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"normalizeToMipmapTexture", NULL, L"int nFlags = 0",
				NULL, &RSImageClass::method_normalizeToMipmapTexture, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"normalizeToRenderTarget", NULL, L"int nFlags = 0",
				NULL, &RSImageClass::method_normalizeToRenderTarget, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"createBuffer", L"boolean",
				L"Image.BufferInfo imginf, long nFlags = Image.bufferOnMemory, "
				L"int countFrame = 1, long msecLong = 0",
				NULL, &RSImageClass::method_createBuffer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"createImage", L"boolean",
				L"int width, int height, int format = Image.formatDefaultRGBA, "
				L"int depth = 32, long nFlags = Image.bufferOnMemory, "
				L"int countFrame = 1, long msecLong = 0",
				NULL, &RSImageClass::method_createImage, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"createCloneBuffer",
				L"boolean", L"Image src, long nFlags = Image.bufferOnMemory",
				NULL, &RSImageClass::method_createCloneBuffer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"releaseBuffer", NULL, L"",
				NULL, &RSImageClass::method_releaseBuffer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getBufferFlags", L"long", L"",
				NULL, &RSImageClass::method_getBufferFlags,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setPaletteTable", NULL, L"int[] palette",
				NULL, &RSImageClass::method_setPaletteTable, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setSequenceTable", NULL, L"int[] seq",
				NULL, &RSImageClass::method_setSequenceTable, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setImageOrigin", NULL, L"int x, int y",
				NULL, &RSImageClass::method_setImageOrigin, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setAnimationDuration", NULL, L"int duration",
				NULL, &RSImageClass::method_setAnimationDuration, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"loadImage",
				L"boolean", L"String file, String mime = null",
				NULL, &RSImageClass::method_loadImage, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readImage",
				L"boolean", L"InputStream is, String mime = null",
				NULL, &RSImageClass::method_readImage, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readImage",
				L"boolean", L"RandomAccessFile file, String mime = null",
				NULL, &RSImageClass::method_readImage, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"saveImage",
				L"boolean",
				L"String file, String mime = null, int quality = 0x100",
				NULL, &RSImageClass::method_saveImage,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeImage",
				L"boolean",
				L"RandomAccessFile file, "
				L"String mime = null, int quality = 0x100",
				NULL, &RSImageClass::method_writeImage,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"fillImage",
				L"boolean",
				L"int argb, Rect rect = null",
				NULL, &RSImageClass::method_fillImage, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"copyImage",
				L"boolean",
				L"Image imgSrc, int x = 0, int y = 0, Rect rctSrc = null",
				NULL, &RSImageClass::method_copyImage, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"blendImage",
				L"boolean",
				L"Image imgSrc, int x = 0, int y = 0, Rect rctSrc = null",
				NULL, &RSImageClass::method_blendImage, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"halfBlendImage",
				L"boolean",
				L"Image imgSrc, int x = 0, int y = 0, Rect rctSrc = null",
				NULL, &RSImageClass::method_halfBlendImage, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"convertImage",
				L"boolean",
				L"Image imgSrc, int x = 0, int y = 0, Rect rctSrc = null",
				NULL, &RSImageClass::method_convertImage, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"multiplyImageRGBAlpha",
				L"boolean", L"",
				NULL, &RSImageClass::method_multiplyImageRGBAlpha, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"blendWithAlphaChannel",
				L"boolean",
				L"Image imgAlpha, int fxCoefficient = 0x100, "
				L"int fxIntercept = 0, int x = 0, int y = 0, Rect rctSrc = null",
				NULL, &RSImageClass::method_blendWithAlphaChannel, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"filterToneTable",
				L"boolean",
				L"Uint8Pointer pRedTone, Uint8Pointer pGreenTone, "
				L"Uint8Pointer pBlueTone, Uint8Pointer pAlphaTone, Rect rect = null",
				NULL, &RSImageClass::method_filterToneTable, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"makeToneFilter",
				NULL, L"Uint8Pointer pTone, int nValue, int nType",
				NULL, &RSImageClass::method_makeToneFilter, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"enlargeHalfImage",
				L"boolean", L"Image imgSrc",
				NULL, &RSImageClass::method_enlargeHalfImage, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"orthogonalRotate",
				L"boolean", L"Image imgSrc, int degAngle",
				NULL, &RSImageClass::method_orthogonalRotate, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSImageClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<SGLImageObject>(pObj) != nullptr) ;
}

// this オブジェクトの画像を取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLImageObject *
	RSImageClass::GetThisImage( RSContext& context, RSObject* pThis )
{
	SGLImageObject *	pImage = ImageFromObject( context, pThis ) ;
	if ( pImage == NULL )
	{
		context.ThrowExceptionError( L"this が Image ではありません" ) ;
	}
	return	pImage ;
}

// Object -> SGLImageObject 変換
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLImageObject *
	RSImageClass::ImageFromObject( RSContext& context, RSObject* pObject )
{
	SGLImageObject *	pImage = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pObject ) ;
	if ( pNativeObj != NULL )
	{
		pImage = ESLTypeCast<SGLImageObject>( pNativeObj->GetObject() ) ;
	}
	return	pImage ;
}

// 新規 Image 生成
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::NewImage( RSContext& context, SGLImageObject * pImage )
{
	ESLAssert( pImage != NULL ) ;
	return	new RSNativeObject
				( new SSmartObject( pImage ),
					context.GetClassAs( L"Image" ) ) ;
}

// Image[] -> SPointerArray<SGLImageObject>
//////////////////////////////////////////////////////////////////////////////
void RSImageClass::GetImageArray
	( RSContext& context,
		SPointerArray<SGLImageObject>& aImages,
		RSObject * pImageArray )
{
	if ( pImageArray == nullptr )
	{
		return ;
	}
	size_t	nCount = pImageArray->GetElementCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RSSmartPtr	prsImage = pImageArray->GetElementAt( context, (int) i ) ;
		if ( prsImage == nullptr )
		{
			continue ;
		}
		SGLImageObject *	pImage =
			RSNativeObject::GetNative<SGLImageObject>( prsImage.GetEntity().Ptr() ) ;
		if ( pImage == nullptr )
		{
			continue ;
		}
		aImages.Add( pImage ) ;
	}
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"Image.<init> の this が Image ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( new SGLImage ) ;
	return	NULL ;
}

// int getFrameCount()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_getFrameCount
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pImage->GetFrameCount() ) ;
}

// int[] getSequenceTable()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_getSequenceTable
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	SArray<uint32_t>	aSeq ;
	size_t		nLength = pImage->GetSequenceLength() ;
	uint32_t *	pSeq = aSeq.GetArray( nLength ) ;
	nLength = pImage->GetSequenceTable( pSeq, nLength ) ;
	//
	RSObject *	pObj =
		context.new_Array( nLength, context.GetIntegerClass() ) ;
	for ( size_t i = 0; i < nLength; i ++ )
	{
		context.ReleaseObjectRef
			( pObj->SetElementAt
				( context, (int) i, context.new_Integer( pSeq[i] ) ) ) ;
	}
	aSeq.FinishArray() ;
	return	pObj ;
}

// long getTotalTime()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_getTotalTime
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pImage->GetTotalTime() ) ;
}

// int frameFromMilliSec( long msec )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_frameFromMilliSec
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
				( pImage->FrameFromMilliSec( arg.LongAt(0) ) ) ;
}

// boolean selectFrame( int iFrame, int iSide = Image.stereoRight )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_selectFrame
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
				( pImage->SelectFrame
					( arg.IntAt(0), arg.IntAt(1) ) == sglErrSuccess ) ;
}

// int getSelectedFrame( int[] side = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_getSelectedFrame
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	int		iSide = 0 ;
	size_t	iFrame = pImage->GetSelectedFrame( &iSide ) ;
	//
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObj = arg.ObjectAt( 0 ) ;
	if ( pObj != NULL )
	{
		pObj->SetElementIntegerAt( context, 0, iSide ) ;
	}
	return	context.new_Integer( iFrame ) ;
}

// Size getImageSize()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_getImageSize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	SGLSize	sizeImage = pImage->GetImageSize() ;
	//
	RSObject *	pObj = context.new_Object( L"Size" ) ;
	if ( pObj == NULL )
	{
		return	NULL ;
	}
	pObj->SetMemberIntegerAs( context, L"w", sizeImage.w ) ;
	pObj->SetMemberIntegerAs( context, L"h", sizeImage.h ) ;
	return	pObj ;
}

// boolean getImageInfo( Image.BufferInfo imginf )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_getImageInfo
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageInfo *
		pImgInf =
			(SGLImageInfo*) arg.PointerAt( 0, sizeof(SGLImageInfo) ) ;
	if ( pImgInf == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	if ( pImage->GetImageInfo( *pImgInf ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// int[] getPaletteTable()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_getPaletteTable
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	SArray<SGLPalette>	aPalette ;
	SGLPalette *	pPalette = aPalette.GetArray( 0x100 ) ;
	size_t			nLength = pImage->GetPaletteTable( pPalette, 0x100 ) ;
	//
	RSObject *	pObj =
		context.new_Array( nLength, context.GetIntegerClass() ) ;
	for ( size_t i = 0; i < nLength; i ++ )
	{
		context.ReleaseObjectRef
			( pObj->SetElementAt
				( context, (int) i,
					context.new_Integer( pPalette[i].ui32 ) ) ) ;
	}
	aPalette.FinishArray() ;
	return	pObj ;
}

// Uint8Pointer lockBuffer
//	( Image.BufferInfo imginf,
//		int flags = Image.lockReadWrite, Rect rect = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_lockBuffer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageInfo *
		pImgInf =
			(SGLImageInfo*) arg.PointerAt( 0, sizeof(SGLImageInfo) ) ;
	if ( pImgInf == NULL )
	{
		return	NULL ;
	}
	SGLImageRect *
		pRectClip =
			(SGLImageRect*) arg.PointerAt( 2, sizeof(SGLImageRect) ) ;
	uint8_t *
		pbytBuf = pImage->LockBuffer( *pImgInf, arg.IntAt(1), pRectClip ) ;
	if ( pbytBuf == NULL )
	{
		return	NULL ;
	}
	RSArrayBuffer *	pBuf =
		new RSArrayBuffer( context.GetArrayBufferClass() ) ;
	ssize_t	iOffset = 0 ;
	size_t	nHeight = (size_t) pImgInf->height ;
	if ( pRectClip != NULL )
	{
		nHeight = (size_t) pRectClip->h ;
	}
	if ( pImgInf->pitchLine >= 0 )
	{
		pBuf->AttachBuffer
			( pbytBuf, pImgInf->pitchLine * nHeight ) ;
	}
	else
	{
		iOffset = (ssize_t) (- pImgInf->pitchLine * (nHeight - 1)) ;
		pBuf->AttachBuffer
			( pbytBuf - iOffset, - pImgInf->pitchLine * nHeight ) ;
	}
	return	context.new_PointerNumber
				( pBuf, RSReferenceNumber::typeUint8, iOffset ) ;
}

// boolean unlockBuffer( int flags = Image.lockReadWrite )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_unlockBuffer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLError	err = pImage->UnlockBuffer( arg.IntAt(0) ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// Image newReference
//	( Rect rectClip = null,
//		int iFrame = -1, int iSide = Image.stereoRight )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_newReference
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *		pObjRectClip = arg.ObjectAt( 0 ) ;
	SGLImageRect *	pRectClip = NULL ;
	SGLImageRect	rectClip ;
	if ( pObjRectClip != NULL )
	{
		pRectClip = &rectClip ;
		rectClip.x = (int) pObjRectClip->GetMemberIntegerAs( context, L"x" ) ;
		rectClip.y = (int) pObjRectClip->GetMemberIntegerAs( context, L"y" ) ;
		rectClip.w = (int) pObjRectClip->GetMemberIntegerAs( context, L"w" ) ;
		rectClip.h = (int) pObjRectClip->GetMemberIntegerAs( context, L"h" ) ;
	}
	ssize_t	iFrame = (ssize_t) arg.IntAt( 1, -1 ) ;
	int		iSide = arg.IntAt( 2, stereoImageRight ) ;
	//
	SGLImageObject *	pRefImage =
			pImage->NewReference( pRectClip, iFrame, iSide ) ;
	if ( pRefImage == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject
		( new SSmartObject( pRefImage ), context.GetClassAs(L"Image") ) ;
}

// void normalizeFormat
//	( int format = 0, int depth = 0,
//		int flags = 0, int width, int height )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_normalizeFormat
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pImage->NormalizeFormat
		( (uint32_t) arg.IntAt(0), (uint32_t) arg.IntAt(1),
			(uint32_t) arg.IntAt(2),
			(uint32_t) arg.IntAt(3), (uint32_t) arg.IntAt(4) ) ;
	return	nullptr ;
}

// Image getReferenceRectOfAtlas( Rect rectRef, int iFrame = -1 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_getReferenceRectOfAtlas
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageRect *		pRectRef =
		(SGLImageRect*) arg.PointerAt( 0, sizeof(SGLImageRect) ) ;
	if ( pRectRef == nullptr )
	{
		context.ThrowExceptionError
			( L"Image.getReferenceRectOfAtlas の引数が null です",
										L"NullPointerException" ) ;
		return	nullptr ;
	}
	SGLImageObject *	pAtlas =
		pImage->GetImageReference( *pRectRef, (ssize_t) arg.IntAt( 1, -1 ) ) ;
	if ( pAtlas == nullptr )
	{
		return	nullptr ;
	}
	return	new RSNativeObject( pAtlas, context.GetClassAs(L"Image") ) ;
}

// static Image buildAtlas
//	( Image[] images, Image.BuildAtlasParam param )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_buildAtlas
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	SPointerArray<SGLImageObject>	aImages ;
	GetImageArray( context, aImages, arg.ObjectAt( 0 ) ) ;
	if ( aImages.GetLength() == 0 )
	{
		return	nullptr ;
	}
	SGLImageObject::BuildAtlasParam *	pParam =
		(SGLImageObject::BuildAtlasParam*)
			arg.PointerAt( 1, sizeof(SGLImageObject::BuildAtlasParam) ) ;
	if ( pParam == nullptr )
	{
		context.ThrowExceptionError
			( L"Image.buildAtlas の引数が null です",
							L"NullPointerException" ) ;
		return	nullptr ;
	}
	SGLImageObject *	pAtlas =
		SGLImageObject::BuildAtlas
			( aImages.GetConstArray(), aImages.GetLength(), *pParam ) ;
	if ( pAtlas == nullptr )
	{
		return	nullptr ;
	}
	return	new RSNativeObject
		( new SSmartObject( pAtlas ), context.GetClassAs(L"Image") ) ;
}

// static int estimateAtlasSize
//	( Size sizeEstimated,
//		Image[] images, boolean flagMakePOT = false, int nMargin = 1,
//		int nSizeLimit = 0, Uint32Pointer pUsedIndexes = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_estimateAtlasSize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLSize* pSizeEstimated = (SGLSize*) arg.PointerAt( 0, sizeof(SGLSize) ) ;
	if ( pSizeEstimated == nullptr )
	{
		context.ThrowExceptionError
			( L"Image.estimateAtlasSize の引数が不正です",
									L"IllegalArgumentException" ) ;
		return	nullptr ;
	}
	SPointerArray<SGLImageObject>	aImages ;
	GetImageArray( context, aImages, arg.ObjectAt( 1 ) ) ;
	if ( aImages.GetLength() == 0 )
	{
		context.ThrowExceptionError
			( L"Image.estimateAtlasSize の引数が不正です",
									L"IllegalArgumentException" ) ;
		return	nullptr ;
	}
	size_t			nSizeLimit = (size_t) arg.IntAt( 4, 0 ) ;
	const size_t	nImageCount = aImages.GetLength() ;
	uint32_t *		pUsedIndexes =
			(uint32_t*) arg.PointerAt( 5, sizeof(uint32_t) * nImageCount ) ;
	//
	SArray<size_t>	aUsedIndexes ;
	size_t *		pTempIndexes = aUsedIndexes.GetArray(nImageCount) ;
	size_t			nUsedCount =
		SGLImageObject::EstimateAtlasSize
			( *pSizeEstimated, aImages.GetConstArray(), aImages.GetLength(),
				arg.BooleanAt( 2, false ), (uint32_t) arg.IntAt( 3, 1 ),
				(uint32_t) nSizeLimit, pTempIndexes ) ;
	//
	if ( pUsedIndexes != nullptr )
	{
		for ( size_t i = 0; i < nImageCount; i ++ )
		{
			pUsedIndexes[i] = (uint32_t) pTempIndexes[i] ;
		}
	}
	aUsedIndexes.FinishArray() ;
	//
	return	context.new_Integer( nUsedCount ) ;
}

// void normalizeToTexture( int nFlags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_normalizeToTexture
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pImage->NormalizeToTexture( arg.IntAt(0) ) ;
	return	NULL ;
}

// void normalizeToMipmapTexture( int nFlags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_normalizeToMipmapTexture
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pImage->NormalizeToMipmapTexture( arg.IntAt(0) ) ;
	return	NULL ;
}

// void normalizeToRenderTarget( int nFlags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_normalizeToRenderTarget
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pImage->NormalizeToRenderTarget( arg.IntAt(0) ) ;
	return	NULL ;
}

// boolean createBuffer
//	( Image.BufferInfo imginf,
//		long nFlags = Image.bufferOnMemory,
//		int countFrame = 1, long msecLong = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_createBuffer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageInfo *
		pImgInf = (SGLImageInfo*) arg.PointerAt( 0, sizeof(SGLImageInfo) ) ;
	if ( pImgInf == NULL )
	{
		context.ThrowExceptionError
			( L"Image.createBuffer の引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	uint64_t	nFlags = arg.LongAt( 1, SGLImageObject::bufferOnMemory ) ;
	size_t		countFrame = arg.IntAt( 2, 1 ) ;
	uint64_t	msecLong = arg.LongAt( 3, 0 ) ;
	//
	SGLError	err =
		pImage->CreateBuffer( *pImgInf, nFlags, countFrame, msecLong ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean createImage
//	( int width, int height, int format = Image.formatDefaultRGBA,
//		int depth = 32, long nFlags = Image.bufferOnMemory,
//		int countFrame = 1, long msecLong = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_createImage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	uint32_t	width = (uint32_t) arg.IntAt( 0 ) ;
	uint32_t	height = (uint32_t) arg.IntAt( 1 ) ;
	uint32_t	format = (uint32_t) arg.IntAt( 2, formatImageDefaultRGBA ) ;
	uint32_t	depth = (uint32_t) arg.IntAt( 3, 32 ) ;
	uint64_t	nFlags = arg.LongAt( 4, SGLImageObject::bufferOnMemory ) ;
	size_t		countFrame = arg.IntAt( 5, 1 ) ;
	uint64_t	msecLong = arg.LongAt( 6, 0 ) ;
	//
	SGLError	err = pImage->CreateImage
		( width, height, format, depth, nFlags, countFrame, msecLong ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean createCloneBuffer( Image src, long nFlags = Image.bufferOnMemory )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_createCloneBuffer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageObject *
		pSrcImage = ESLTypeCast<SGLImageObject>( arg.NativeObjectAt( 0 ) ) ;
	if ( pSrcImage == NULL )
	{
		context.ThrowExceptionError
			( L"createCloneBuffer に複製元 Image が指定されていません" ) ;
		return	NULL ;
	}
	uint64_t	nFlags = arg.LongAt( 1, SGLImageObject::bufferOnMemory ) ;
	//
	SGLImageBuffer	imgbuf ;
	SGLPalette		tblPalette[0x100] ;
	imgbuf.ptrPalette = &tblPalette[0] ;
	if ( pSrcImage->GetPaletteTable( imgbuf.ptrPalette, 0x100 ) == 0 )
	{
		imgbuf.ptrPalette = NULL ;
	}
	imgbuf.ptrBuffer = pSrcImage->LockBuffer( imgbuf ) ;
	//
	SGLError	err = pImage->CreateCloneBuffer( imgbuf, nFlags ) ;
	//
	pSrcImage->UnlockBuffer() ;
	//
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// void releaseBuffer()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_releaseBuffer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	pImage->ReleaseBuffer() ;
	return	NULL ;
}

// long getBufferFlags()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_getBufferFlags
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == nullptr )
	{
		return	nullptr ;
	}
	return	context.new_Integer( pImage->GetBufferFlags() ) ;
}

// void setPaletteTable( int[] palette )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_setPaletteTable
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObjPal = arg.ObjectAt( 0 ) ;
	if ( pObjPal == NULL )
	{
		context.ThrowExceptionError
				( L"Image.setPaletteTable の引数が null です" ) ;
		return	NULL ;
	}
	SArray<SGLPalette>	aPalette ;
	size_t	nCount = pObjPal->GetElementCount() ;
	aPalette.SetLimit( nCount ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLPalette	pal ;
		pal = (uint32_t) pObjPal->GetElementIntegerAt( context, (int) i ) ;
		aPalette.Add( pal ) ;
	}
	pImage->SetPaletteTable( aPalette.GetConstArray(), nCount ) ;
	return	NULL ;
}

// void setSequenceTable( int[] seq )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_setSequenceTable
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObjSeq = arg.ObjectAt( 0 ) ;
	if ( pObjSeq == NULL )
	{
		context.ThrowExceptionError
				( L"Image.setSequenceTable の引数が null です" ) ;
		return	NULL ;
	}
	SArray<uint32_t>	aSeq ;
	size_t	nCount = pObjSeq->GetElementCount() ;
	aSeq.SetLimit( nCount ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		aSeq.Add( (uint32_t) pObjSeq->GetElementIntegerAt( context, (int) i ) ) ;
	}
	pImage->SetSequenceTable( aSeq.GetConstArray(), nCount ) ;
	return	NULL ;
}

// void setImageOrigin( int x, int y )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_setImageOrigin
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pImage->SetImageOrigin( arg.IntAt(0), arg.IntAt(1) ) ;
	return	NULL ;
}

// void setAnimationDuration( int duration )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_setAnimationDuration
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pImage->SetAnimationDuration( arg.LongAt(0) ) ;
	return	NULL ;
}

// boolean loadImage( String file, String mime = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_loadImage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SString	strFile = arg.StringAt( 0 ) ;
	SString	strMime = arg.StringAt( 1 ) ;
	//
	const wchar_t *	pwszMIME = NULL ;
	if ( !strMime.IsEmpty() )
	{
		pwszMIME = strMime ;
	}
	SGLError	err = pImage->LoadImage( strFile, pwszMIME ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean readImage( InputStream is, String mime = null )
// boolean readImage( RandomAccessFile file, String mime = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_readImage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SFileInterface *
			pFile = ESLTypeCast<SFileInterface>( arg.NativeObjectAt(0) ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError( L"Image.readImage の引数が null です" ) ;
		return	NULL ;
	}
	SString	strMime = arg.StringAt( 1 ) ;
	//
	const wchar_t *	pwszMIME = NULL ;
	if ( !strMime.IsEmpty() )
	{
		pwszMIME = strMime ;
	}
	SGLError	err = pImage->ReadImage( pFile, pwszMIME ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean saveImage
//		( String file, String mime = null, int quality = 0x100 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_saveImage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SString	strFile = arg.StringAt( 0 ) ;
	SString	strMime = arg.StringAt( 1 ) ;
	//
	const wchar_t *	pwszMIME = NULL ;
	if ( !strMime.IsEmpty() )
	{
		pwszMIME = strMime ;
	}
	//
	SGLImageEncoderInterface::Options	opt ;
	opt.nFlags = SGLImageEncoderInterface::optionQuality ;
	opt.nQuality = (uint32_t) arg.IntAt( 2, 0x100 ) ;
	//
	SGLError	err = pImage->SaveImage( strFile, pwszMIME, &opt ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean writeImage
//		( RandomAccessFile file,
//				String mime = null, int quality = 0x100 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_writeImage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SFileInterface *
			pFile = ESLTypeCast<SFileInterface>( arg.NativeObjectAt(0) ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError( L"Image.writeImage の引数が null です" ) ;
		return	NULL ;
	}
	SString	strMime = arg.StringAt( 1 ) ;
	//
	const wchar_t *	pwszMIME = NULL ;
	if ( !strMime.IsEmpty() )
	{
		pwszMIME = strMime ;
	}
	//
	SGLImageEncoderInterface::Options	opt ;
	opt.nFlags = SGLImageEncoderInterface::optionQuality ;
	opt.nQuality = (uint32_t) arg.IntAt( 2, 0x100 ) ;
	//
	SGLError	err = pImage->WriteImage( pFile, pwszMIME, &opt ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean fillImage( int argb, Rect rect = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_fillImage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageBuffer		imgDst ;
	SGLImageRect		rectFill ;
	SGLPalette			pxFill( (uint32_t) arg.IntAt( 0 ) ) ;
	SGLImageSmartBuffer	isbDst( imgDst, pImage ) ;
	return	context.new_Boolean
		( sglFillImageBuffer
			( imgDst, pxFill,
				RSRectClass::ImageRectFromObject
					( context, rectFill, arg.ObjectAt( 1 ) ) ) == sglErrSuccess ) ;
}

// boolean copyImage
//	( Image imgSrc, int x = 0, int y = 0, Rect rctSrc = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_copyImage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageObject *
		pSrcImage = ImageFromObject( context, arg.ObjectAt( 0 ) ) ;
	if ( pSrcImage == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SGLImageBuffer		imgDst, imgSrc ;
	SGLImageRect		rectSrc ;
	SGLImageSmartBuffer	isbDst( imgDst, pImage ) ;
	SGLImageSmartBuffer	isbSrc( imgSrc, pSrcImage, SGLImageObject::lockRead ) ;
	return	context.new_Boolean
		( sglCopyImageBuffer
			( imgDst, imgSrc, arg.IntAt(1), arg.IntAt(2),
				RSRectClass::ImageRectFromObject
					( context, rectSrc, arg.ObjectAt(3) ) ) == sglErrSuccess ) ;
}

// boolean blendImage
//	( Image imgSrc, int x = 0, int y = 0, Rect rctSrc = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_blendImage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageObject *
		pSrcImage = ImageFromObject( context, arg.ObjectAt( 0 ) ) ;
	if ( pSrcImage == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SGLImageBuffer		imgDst, imgSrc ;
	SGLImageRect		rectSrc ;
	SGLImageSmartBuffer	isbDst( imgDst, pImage ) ;
	SGLImageSmartBuffer	isbSrc( imgSrc, pSrcImage, SGLImageObject::lockRead ) ;
	return	context.new_Boolean
		( sglBlendImageBuffer
			( imgDst, imgSrc, arg.IntAt(1), arg.IntAt(2),
				RSRectClass::ImageRectFromObject
					( context, rectSrc, arg.ObjectAt(3) ) ) == sglErrSuccess ) ;
}

// boolean halfBlendImage
//	( Image imgSrc, int x = 0, int y = 0, Rect rctSrc = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_halfBlendImage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageObject *
		pSrcImage = ImageFromObject( context, arg.ObjectAt( 0 ) ) ;
	if ( pSrcImage == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SGLImageBuffer		imgDst, imgSrc ;
	SGLImageRect		rectSrc ;
	SGLImageSmartBuffer	isbDst( imgDst, pImage ) ;
	SGLImageSmartBuffer	isbSrc( imgSrc, pSrcImage, SGLImageObject::lockRead ) ;
	return	context.new_Boolean
		( sglHalfBlendImageBuffer
			( imgDst, imgSrc, arg.IntAt(1), arg.IntAt(2),
				RSRectClass::ImageRectFromObject
					( context, rectSrc, arg.ObjectAt(3) ) ) == sglErrSuccess ) ;
}

// boolean convertImage
//	( Image imgSrc, int x = 0, int y = 0, Rect rctSrc = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_convertImage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageObject *
		pSrcImage = ImageFromObject( context, arg.ObjectAt( 0 ) ) ;
	if ( pSrcImage == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SGLImageBuffer		imgDst, imgSrc ;
	SGLImageRect		rectSrc ;
	SGLImageSmartBuffer	isbDst( imgDst, pImage ) ;
	SGLImageSmartBuffer	isbSrc( imgSrc, pSrcImage, SGLImageObject::lockRead ) ;
	return	context.new_Boolean
		( sglConvertImageBuffer
			( imgDst, imgSrc, arg.IntAt(1), arg.IntAt(2),
				RSRectClass::ImageRectFromObject
					( context, rectSrc, arg.ObjectAt(3) ) ) == sglErrSuccess ) ;
}

// boolean multiplyImageRGBAlpha()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_multiplyImageRGBAlpha
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageBuffer		imgDst ;
	SGLImageSmartBuffer	isbDst( imgDst, pImage ) ;
	return	context.new_Boolean
		( sglMultiplyImageRGBAlpha( imgDst ) == sglErrSuccess ) ;
}

// boolean blendWithAlphaChannel
//	( Image imgAlpha, int fxCoefficient = 0x100,
//		int fxIntercept = 0, int x = 0, int y = 0, Rect rctSrc = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_blendWithAlphaChannel
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageObject *
		pAlphaImage = ImageFromObject( context, arg.ObjectAt( 0 ) ) ;
	if ( pAlphaImage == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SGLImageBuffer		imgDst, imgAlpha ;
	SGLImageRect		rectSrc ;
	SGLImageSmartBuffer	isbDst( imgDst, pImage ) ;
	SGLImageSmartBuffer	isbAlpha( imgAlpha, pAlphaImage, SGLImageObject::lockRead ) ;
	return	context.new_Boolean
		( sglBlendWithAlphaChannel
			( imgDst, imgAlpha, arg.IntAt(1), arg.IntAt(2),
				arg.IntAt(3), arg.IntAt(4),
				RSRectClass::ImageRectFromObject
					( context, rectSrc, arg.ObjectAt(5) ) ) == sglErrSuccess ) ;
}

// boolean filterToneTable
//	( Uint8Pointer pRedTone, Uint8Pointer pGreenTone,
//		Uint8Pointer pBlueTone, Uint8Pointer pAlphaTone, Rect rect = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_filterToneTable
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageBuffer		imgDst ;
	SGLImageRect		rectDst ;
	SGLImageSmartBuffer	isbDst( imgDst, pImage ) ;
	return	context.new_Boolean
		( sglApplyToneImageFilter
			( imgDst,
				RSRectClass::ImageRectFromObject
					( context, rectDst, arg.ObjectAt(4) ),
				arg.PointerAt(0), arg.PointerAt(1),
				arg.PointerAt(2), arg.PointerAt(3) ) == sglErrSuccess ) ;
}

// void makeToneFilter( Uint8Pointer pTone, int nValue, int nType )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_makeToneFilter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	uint8_t *	pTone = arg.PointerAt(0) ;
	if ( pTone == NULL )
	{
		context.ThrowExceptionError
			( L"Image.makeToneFilter 引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	sglMakeToneFilter( pTone, arg.IntAt(1), arg.IntAt(2) ) ;
	return	NULL ;
}

// boolean enlargeHalfImage( Image imgSrc )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_enlargeHalfImage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageObject *
		pSrcImage = ImageFromObject( context, arg.ObjectAt( 0 ) ) ;
	if ( pSrcImage == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SGLImageBuffer		imgDst, imgSrc ;
	SGLImageSmartBuffer	isbDst( imgDst, pImage ) ;
	SGLImageSmartBuffer	isbSrc( imgSrc, pSrcImage, SGLImageObject::lockRead ) ;
	return	context.new_Boolean
		( sglEnlargeHalfImageBuffer( imgDst, imgSrc ) == sglErrSuccess ) ;
}

// boolean orthogonalRotate( Image imgSrc, int degAngle )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageClass::method_orthogonalRotate
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageObject *	pImage = GetThisImage( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageObject *
		pSrcImage = ImageFromObject( context, arg.ObjectAt( 0 ) ) ;
	if ( pSrcImage == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SGLImageBuffer		imgDst, imgSrc ;
	SGLImageSmartBuffer	isbDst( imgDst, pImage ) ;
	SGLImageSmartBuffer	isbSrc( imgSrc, pSrcImage, SGLImageObject::lockRead ) ;
	return	context.new_Boolean
		( sglOrthogonalRotateImageBuffer
				( imgDst, imgSrc, arg.IntAt(1) ) == sglErrSuccess ) ;
}


//////////////////////////////////////////////////////////////////////////////
// ImageComposition.Layer クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( Rosetta::RSImageCompositionClass::LayerClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSImageCompositionClass::LayerClass::LayerClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSImageCompositionClass::LayerClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"Image" ) ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSImageCompositionClass::LayerClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &RSImageCompositionClass::LayerClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getParent", L"ImageComposition.Layer", L"",
				NULL, &RSImageCompositionClass::LayerClass::method_getParent,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getName", L"String", L"",
				NULL, &RSImageCompositionClass::LayerClass::method_getName,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setName", NULL, L"String name",
				NULL, &RSImageCompositionClass::LayerClass::method_setName, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getType", L"int", L"",
				NULL, &RSImageCompositionClass::LayerClass::method_getType,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getPosition", L"Point", L"",
				NULL, &RSImageCompositionClass::LayerClass::method_getPosition,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setPosition", NULL, L"int x, int y",
				NULL, &RSImageCompositionClass::LayerClass::method_setPosition, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getBlendMode", L"int", L"",
				NULL, &RSImageCompositionClass::LayerClass::method_getBlendMode,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setBlendMode", NULL, L"int blend",
				NULL, &RSImageCompositionClass::LayerClass::method_setBlendMode, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isVisible", L"boolean", L"",
				NULL, &RSImageCompositionClass::LayerClass::method_isVisible,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getTransparency", L"int", L"",
				NULL, &RSImageCompositionClass::LayerClass::method_getTransparency,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getChildrenCount", L"int", L"",
				NULL, &RSImageCompositionClass::LayerClass::method_getChildrenCount,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getChildAt", L"ImageComposition.Layer", L"int index",
				NULL, &RSImageCompositionClass::LayerClass::method_getChildAt,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"layerSizeOf", L"Rect", L"int nThreshold = 0",
				NULL, &RSImageCompositionClass::LayerClass::method_layerSizeOf,
				NULL, RSFunctionPrototype::flagConstant ) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLImageComposition::Layer *
	RSImageCompositionClass::LayerClass::GetThisLayer( RSContext& context, RSObject* pThis )
{
	SGLImageComposition::Layer *	pLayer = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pLayer = ESLTypeCast<SGLImageComposition::Layer>( pNativeObj->GetObject() ) ;
	}
	if ( pLayer == NULL )
	{
		context.ThrowExceptionError( L"this が ImageComposition.Layer ではありません" ) ;
	}
	return	pLayer ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::LayerClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"ImageComposition.Layer.<init> の this が ImageComposition.Layer ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( new SGLImageComposition::Layer ) ;
	return	NULL ;
}

// ImageComposition.Layer getParent()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::LayerClass::method_getParent
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition::Layer *	pLayer = GetThisLayer( context, pThis ) ;
	if ( pLayer == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject
		( pLayer->m_parent, context.GetClassAs( L"ImageComposition.Layer" ) ) ;
}

// String getName()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::LayerClass::method_getName
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition::Layer *	pLayer = GetThisLayer( context, pThis ) ;
	if ( pLayer == NULL )
	{
		return	NULL ;
	}
	return	context.new_String( pLayer->m_name ) ;
}

// void setName( String name )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::LayerClass::method_setName
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition::Layer *	pLayer = GetThisLayer( context, pThis ) ;
	if ( pLayer == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pLayer->m_name = arg.StringAt(0) ;
	return	nullptr ;
}

// int getType()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::LayerClass::method_getType
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition::Layer *	pLayer = GetThisLayer( context, pThis ) ;
	if ( pLayer == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pLayer->m_type ) ;
}

// Point getPosition()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::LayerClass::method_getPosition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition::Layer *	pLayer = GetThisLayer( context, pThis ) ;
	if ( pLayer == NULL )
	{
		return	NULL ;
	}
	RSObject *	pObj = context.new_Object( L"Point" ) ;
	pObj->SetMemberIntegerAs( context, L"x", pLayer->m_position.x ) ;
	pObj->SetMemberIntegerAs( context, L"y", pLayer->m_position.y ) ;
	return	pObj ;
}

// void setPosition( int x, int y )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::LayerClass::method_setPosition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition::Layer *	pLayer = GetThisLayer( context, pThis ) ;
	if ( pLayer == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pLayer->m_position.x = arg.IntAt(0) ;
	pLayer->m_position.y = arg.IntAt(1) ;
	return	nullptr ;
}

// int getBlendMode()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::LayerClass::method_getBlendMode
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition::Layer *	pLayer = GetThisLayer( context, pThis ) ;
	if ( pLayer == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pLayer->m_blend ) ;
}

// void setBlendMode( int blend )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::LayerClass::method_setBlendMode
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition::Layer *	pLayer = GetThisLayer( context, pThis ) ;
	if ( pLayer == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pLayer->m_blend = arg.IntAt(0) ;
	return	nullptr ;
}

// boolean isVisible()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::LayerClass::method_isVisible
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition::Layer *	pLayer = GetThisLayer( context, pThis ) ;
	if ( pLayer == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pLayer->m_visible ) ;
}

// int getTransparency()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::LayerClass::method_getTransparency
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition::Layer *	pLayer = GetThisLayer( context, pThis ) ;
	if ( pLayer == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pLayer->m_transparency ) ;
}

// int getChildrenCount()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::LayerClass::method_getChildrenCount
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition::Layer *	pLayer = GetThisLayer( context, pThis ) ;
	if ( pLayer == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pLayer->m_layers.GetLength() ) ;
}

// ImageComposition.Layer getChildAt( int index )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::LayerClass::method_getChildAt
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition::Layer *	pLayer = GetThisLayer( context, pThis ) ;
	if ( pLayer == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageComposition::Layer *
			pChild = pLayer->m_layers.GetAt( arg.IntAt(0) ) ;
	if ( pChild == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject
		( pChild, context.GetClassAs( L"ImageComposition.Layer" ) ) ;
}

// Rect layerSizeOf( int nThreshold = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::LayerClass::method_layerSizeOf
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition::Layer *	pLayer = GetThisLayer( context, pThis ) ;
	if ( pLayer == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLRect	rectLayer ;
	if ( !SGLImageComposition::LayerSizeOf
				( rectLayer, *pLayer, arg.IntAt(0) ) )
	{
		return	NULL ;
	}
	RSObject *	pObj = context.new_Object( L"Rect" ) ;
	pObj->SetMemberIntegerAs( context, L"x", rectLayer.left ) ;
	pObj->SetMemberIntegerAs( context, L"y", rectLayer.top ) ;
	pObj->SetMemberIntegerAs( context, L"w", rectLayer.GetWidth() ) ;
	pObj->SetMemberIntegerAs( context, L"h", rectLayer.GetHeight() ) ;
	return	pObj ;
}



//////////////////////////////////////////////////////////////////////////////
// ImageComposition クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSImageCompositionClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSImageCompositionClass::RSImageCompositionClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSImageCompositionClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	LayerClass *
		pLayerClass = new LayerClass( context.GetClassClass(), L"Layer" ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"Layer", pLayerClass ) ) ;
	pLayerClass->Initialize( context ) ;
	//
	CreateMemberIntegerAs( context, L"blendPass", PSD::blendPass, modifierConst ) ;
	CreateMemberIntegerAs( context, L"blendNormal", PSD::blendNormal, modifierConst ) ;
	CreateMemberIntegerAs( context, L"blendDarken", PSD::blendDarken, modifierConst ) ;
	CreateMemberIntegerAs( context, L"blendLighten", PSD::blendLighten, modifierConst ) ;
	CreateMemberIntegerAs( context, L"blendHue", PSD::blendHue, modifierConst ) ;
	CreateMemberIntegerAs( context, L"blendSat", PSD::blendSat, modifierConst ) ;
	CreateMemberIntegerAs( context, L"blendColor", PSD::blendColor, modifierConst ) ;
	CreateMemberIntegerAs( context, L"blendLuminosity", PSD::blendLuminosity, modifierConst ) ;
	CreateMemberIntegerAs( context, L"blendMultiply", PSD::blendMultiply, modifierConst ) ;
	CreateMemberIntegerAs( context, L"blendDivision", PSD::blendDivision, modifierConst ) ;
	CreateMemberIntegerAs( context, L"blendAddition", PSD::blendAddition, modifierConst ) ;
	CreateMemberIntegerAs( context, L"blendSubtraction", PSD::blendSubtraction, modifierConst ) ;
	CreateMemberIntegerAs( context, L"blendScreen", PSD::blendScreen, modifierConst ) ;
	CreateMemberIntegerAs( context, L"blendDissolve", PSD::blendDissolve, modifierConst ) ;
	CreateMemberIntegerAs( context, L"blendOverlay", PSD::blendOverlay, modifierConst ) ;
	CreateMemberIntegerAs( context, L"blendHardLight", PSD::blendHardLight, modifierConst ) ;
	CreateMemberIntegerAs( context, L"blendSoftLight", PSD::blendSoftLight, modifierConst ) ;
	CreateMemberIntegerAs( context, L"blendDifference", PSD::blendDifference, modifierConst ) ;
	//
	CreateMemberIntegerAs( context, L"layerNormal", PSD::LayerRecord::layerNormal, modifierConst ) ;
	CreateMemberIntegerAs( context, L"layerBackground", PSD::LayerRecord::layerBackground, modifierConst ) ;
	CreateMemberIntegerAs( context, L"layerGroup", PSD::LayerRecord::layerGroup, modifierConst ) ;
	CreateMemberIntegerAs( context, L"layerEndOfGroup", PSD::LayerRecord::layerEndOfGroup, modifierConst ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &RSImageCompositionClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"loadPsdFile", L"boolean", L"String file",
				NULL, &RSImageCompositionClass::method_loadPsdFile, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readPsdFile", L"boolean", L"InputStream is",
				NULL, &RSImageCompositionClass::method_readPsdFile, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readPsdFile", L"boolean", L"RandomAccessFile file",
				NULL, &RSImageCompositionClass::method_readPsdFile, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"savePsdFile", L"boolean", L"String file",
				NULL, &RSImageCompositionClass::method_savePsdFile, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writePsdFile", L"boolean", L"RandomAccessFile file",
				NULL, &RSImageCompositionClass::method_writePsdFile, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setCanvasInfo", NULL, L"int width, int height, int channels = 3",
				NULL, &RSImageCompositionClass::method_setCanvasInfo, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getCanvasSize", L"Size", L"",
				NULL, &RSImageCompositionClass::method_getCanvasSize,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getLayerCount", L"int", L"",
				NULL, &RSImageCompositionClass::method_getLayerCount,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getLayerAt", L"ImageComposition.Layer", L"int index",
				NULL, &RSImageCompositionClass::method_getLayerAt,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getLayerTreeCount", L"int", L"",
				NULL, &RSImageCompositionClass::method_getLayerTreeCount,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getLayerTreeAt", L"ImageComposition.Layer", L"int index",
				NULL, &RSImageCompositionClass::method_getLayerTreeAt,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"appendLayer", L"ImageComposition.Layer",
			L"String name, Image image = null, int x = 0, int y = 0, "
							L"ImageComposition.Layer parent = null",
			NULL, &RSImageCompositionClass::method_appendLayer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"appendGroupLayer", L"ImageComposition.Layer",
			L"String name, ImageComposition.Layer parent = null",
			NULL, &RSImageCompositionClass::method_appendGroupLayer, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSImageCompositionClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<SGLImageComposition>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLImageComposition *
	RSImageCompositionClass::GetThisImageComposition
					( RSContext& context, RSObject* pThis )
{
	SGLImageComposition *	pImage = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pImage = ESLTypeCast<SGLImageComposition>( pNativeObj->GetObject() ) ;
	}
	if ( pImage == NULL )
	{
		context.ThrowExceptionError( L"this が ImageComposition ではありません" ) ;
	}
	return	pImage ;
}

// SGLImageComposition::Layer を取得
//////////////////////////////////////////////////////////////////////////////
SGLImageComposition::Layer *
	RSImageCompositionClass::LayerFromObject( RSContext& context, RSObject* pObject )
{
	SGLImageComposition::Layer *	pLayer = nullptr ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pObject ) ;
	if ( pNativeObj != nullptr )
	{
		pLayer = ESLTypeCast<SGLImageComposition::Layer>( pNativeObj->GetObject() ) ;
	}
	return	pLayer ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"ImageComposition.<init> の this が ImageComposition ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( new SGLImageComposition ) ;
	return	NULL ;
}

// boolean loadPsdFile( String file )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::method_loadPsdFile
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition *	pImage = GetThisImageComposition( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pImage->LoadPSDFile( arg.StringAt(0) ) == sglErrSuccess ) ;
}

// boolean readPsdFile( InputStream file )
// boolean readPsdFile( RandomAccessFile file )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::method_readPsdFile
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition *	pImage = GetThisImageComposition( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SFileInterface *
			pFile = ESLTypeCast<SFileInterface>( arg.NativeObjectAt(0) ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError( L"ImageComposition.readPsdFile の引数が null です" ) ;
		return	NULL ;
	}
	return	context.new_Boolean
		( pImage->ReadPSDFile( *pFile ) == sglErrSuccess ) ;
}

// boolean savePsdFile( String file )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::method_savePsdFile
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition *	pImage = GetThisImageComposition( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pImage->SavePSDFile( arg.StringAt(0) ) == sglErrSuccess ) ;
}

// boolean writePsdFile( RandomAccessFile file )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::method_writePsdFile
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition *	pImage = GetThisImageComposition( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SFileInterface *
			pFile = ESLTypeCast<SFileInterface>( arg.NativeObjectAt(0) ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError( L"ImageComposition.writePsdFile の引数が null です" ) ;
		return	NULL ;
	}
	return	context.new_Boolean
		( pImage->WritePSDFile( *pFile ) == sglErrSuccess ) ;
}

// void setCanvasInfo( int width, int height, int channels = 3 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::method_setCanvasInfo
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition *	pImage = GetThisImageComposition( context, pThis ) ;
	if ( pImage == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLSize	sizeCanvas( arg.IntAt(0), arg.IntAt(1) ) ;
	pImage->SetCanvasInfo( sizeCanvas, (size_t) arg.IntAt(2,3) ) ;
	return	nullptr ;
}

// Size getCanvasSize()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::method_getCanvasSize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition *	pImage = GetThisImageComposition( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSObject *	pObjSize = context.new_Object( L"Size" ) ;
	pObjSize->SetMemberIntegerAs( context, L"w", pImage->m_sizeCanvas.w ) ;
	pObjSize->SetMemberIntegerAs( context, L"h", pImage->m_sizeCanvas.h ) ;
	return	pObjSize ;
}

// int getLayerCount()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::method_getLayerCount
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition *	pImage = GetThisImageComposition( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pImage->m_layers.GetLength() ) ;
}

// ImageComposition.Layer getLayerAt( int index )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::method_getLayerAt
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition *	pImage = GetThisImageComposition( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageComposition::Layer *
			pLayer = pImage->m_layers.GetAt( arg.IntAt(0) ) ;
	if ( pLayer == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject
				( pLayer, context.GetClassAs( L"ImageComposition.Layer" ) ) ;
}

// int getLayerTreeCount()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::method_getLayerTreeCount
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition *	pImage = GetThisImageComposition( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pImage->m_grouped.GetLength() ) ;
}

// ImageComposition.Layer getLayerTreeAt( int index )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::method_getLayerTreeAt
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition *	pImage = GetThisImageComposition( context, pThis ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageComposition::Layer *
			pLayer = pImage->m_grouped.GetAt( arg.IntAt(0) ) ;
	if ( pLayer == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject
				( pLayer, context.GetClassAs( L"ImageComposition.Layer" ) ) ;
}

// ImageComposition.Layer appendLayer
//		( String name, Image image = null, int x = 0, int y = 0,
//			ImageComposition.Layer parent = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::method_appendLayer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition *	pImage = GetThisImageComposition( context, pThis ) ;
	if ( pImage == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageComposition::Layer *
			pLayer = new SGLImageComposition::Layer ;
	pLayer->m_name = arg.StringAt( 0 ) ;
	pLayer->m_position.x = (int32_t) arg.IntAt(2) ;
	pLayer->m_position.y = (int32_t) arg.IntAt(3) ;
	//
	SGLImageObject *	pSrcImage = RSImageClass::ImageFromObject
										( context, arg.ObjectAt( 1 ) ) ;
	if ( pSrcImage != nullptr )
	{
		pLayer->DuplicateOf( pSrcImage ) ;
	}
	SGLImageComposition::Layer *
			pParent = LayerFromObject( context, arg.ObjectAt( 4 ) ) ;
	pImage->AppendLayer( pLayer, pParent ) ;

	return	new RSNativeObject
				( pLayer, context.GetClassAs( L"ImageComposition.Layer" ) ) ;
}

// ImageComposition.Layer appendGroupLayer
//		( String name, ImageComposition.Layer parent = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSImageCompositionClass::method_appendGroupLayer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLImageComposition *	pImage = GetThisImageComposition( context, pThis ) ;
	if ( pImage == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageComposition::Layer *
		pLayer = pImage->AppendGroupLayer
					( arg.StringAt(0),
						LayerFromObject( context, arg.ObjectAt( 1 ) ) ) ;
	return	new RSNativeObject
				( pLayer, context.GetClassAs( L"ImageComposition.Layer" ) ) ;
}



//////////////////////////////////////////////////////////////////////////////
// PaintContext クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSPaintContextClass::PaintParamClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSPaintContextClass::PaintParamClass::PaintParamClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSPaintContextClass::PaintParamClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSGenericObject( this ) ;
	//
	m_pPrototype->CreateMemberIntegerAs( context, L"nFlags", 0 ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"ptPaint", context.new_ObjectPointer( L"Point" ) ) ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"nTransparency", 0 ) ;
	m_pPrototype->CreateMemberNumberAs( context, L"zOrder", 0.0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"rgbColorParam", 0 ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"pAffine",
				context.new_Pointer
					( NULL, context.GetClassAs( L"Affine" ) ) ) ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"pVertices",
				context.new_PointerNumber
					( NULL, RSReferenceNumber::typeFloat32 ) ) ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"countVertex", 0 ) ;
}


//////////////////////////////////////////////////////////////////////////////
// PaintContext クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSPaintContextClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSPaintContextClass::RSPaintContextClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSPaintContextClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	PaintParamClass *	pParamClass = new PaintParamClass( context.GetClassClass() ) ;
	pParamClass->Initialize( context ) ;
	pParamClass->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"PaintParam", pParamClass ) ) ;
	//
	CreateMemberIntegerAs
		( context, L"paintApplyColorAdd", paintApplyColorAdd, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"paintApplyColorMul", paintApplyColorMul, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"paintApplyAlphaMul", paintApplyAlphaMul, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"paintApplyColorMask", paintApplyColorMask, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"paintMaskApply", paintMaskApply, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"paintApplyShifter", paintApplyShifter, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"paintFunctionAdd", (uint32_t) paintFunctionAdd, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"paintFunctionSub", (uint32_t) paintFunctionSub, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"paintFunctionMul", (uint32_t) paintFunctionMul, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"paintFunctionDiv", (uint32_t) paintFunctionDiv, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"paintFunctionMax", (uint32_t) paintFunctionMax, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"paintFunctionMin", (uint32_t) paintFunctionMin, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"paintFunctionMove", (uint32_t) paintFunctionMove, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"paintMultipleByAlpha", (uint32_t) paintMultipleByAlpha, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"paintDstAlphaMask", (uint32_t) paintDstAlphaMask, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"paintMaskFunction", (uint32_t) paintMaskFunction, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"paintFunctionShifter", (uint32_t) paintFunctionShifter, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"paintNoBlendAlpha", paintNoBlendAlpha, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"paintNoProductOfAlpha", paintNoProductOfAlpha, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"paintWithZOrder", paintWithZOrder, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"paintWithZOrderNoWrite", paintWithZOrderNoWrite, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"paintSmoothStretch", paintSmoothStretch, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"paintUnsmoothStretch", paintUnsmoothStretch, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"paintFixedPosition", paintFixedPosition, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"paintPolygonShaped", paintPolygonShaped, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"paintDelayable", paintDelayable, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"paintOrderNoCare", paintOrderNoCare, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"clearTargetColor", SGLPaintContextInterface::clearTargetColor, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"clearTargetZBuffer", SGLPaintContextInterface::clearTargetZBuffer, modifierConst ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &RSPaintContextClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getTargetImage", L"Image", L"",
				NULL, &RSPaintContextClass::method_getTargetImage,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getTargetZBuffer", L"Image", L"",
				NULL, &RSPaintContextClass::method_getTargetZBuffer,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getViewPort", L"boolean", L"Rect rctView",
				NULL, &RSPaintContextClass::method_getViewPort,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"attachTargetImage",
			L"boolean", L"Image pImage, Image pZBuffer, Rect pView = null",
				NULL, &RSPaintContextClass::method_attachTargetImage, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"detachTargetImage", L"boolean", L"",
				NULL, &RSPaintContextClass::method_detachTargetImage, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"appendTransformation",
				L"boolean", L"Affine af, int nTransparency",
				NULL, &RSPaintContextClass::method_appendTransformation, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setTransformation",
				L"boolean", L"Affine af, int nTransparency",
				NULL, &RSPaintContextClass::method_setTransformation, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"currentAffine",
				L"boolean", L"Affine af",
				NULL, &RSPaintContextClass::method_currentAffine,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"currentTransparency", L"int", L"",
				NULL, &RSPaintContextClass::method_currentTransparency,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"pushTransformation", L"boolean", L"",
				NULL, &RSPaintContextClass::method_pushTransformation, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"popTransformation", L"boolean", L"",
				NULL, &RSPaintContextClass::method_popTransformation, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"resetTransformation", L"boolean", L"",
				NULL, &RSPaintContextClass::method_resetTransformation, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setPaintFlags", NULL, L"long nFlags",
				NULL, &RSPaintContextClass::method_setPaintFlags, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getPaintFlags", L"long", L"",
				NULL, &RSPaintContextClass::method_getPaintFlags, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"fillClearTarget",
				L"boolean", L"int argb = 0xFF000000, long flags = 0",
				NULL, &RSPaintContextClass::method_fillClearTarget, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"drawThinLine", L"boolean",
				L"int x0, int y0, int x1, int y1, "
				L"int argb, double z = 0.0, int flags = 0",
				NULL, &RSPaintContextClass::method_drawThinLine, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"drawThinLines", L"boolean",
				L"Vector2D pLines, int nLines, "
				L"int argb, double z = 0.0, int flags = 0",
				NULL, &RSPaintContextClass::method_drawThinLines, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"drawEllipse", L"boolean",
				L"double xCenter, double yCenter, "
				L"double rWidth, double rHeight, "
				L"int argb, double z = 0.0, int flags = 0",
				NULL, &RSPaintContextClass::method_drawEllipse, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"drawArc", L"boolean",
				L"double xCenter, double yCenter, "
				L"double rWidth, double rHeight, "
				L"double radFirst, double radEnd, "
				L"int argb, double z = 0.0, int flags = 0",
				NULL, &RSPaintContextClass::method_drawArc, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"drawBezier", L"boolean",
				L"Vector2D pPoints, int nPoints, "
				L"int argb, double z = 0.0, int flags = 0",
				NULL, &RSPaintContextClass::method_drawBezier, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"fillRectangle", L"boolean",
				L"int x, int y, int width, int height, "
				L"int argb, double z = 0.0, int flags = 0",
				NULL, &RSPaintContextClass::method_fillRectangle, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"fillPolygon", L"boolean",
				L"Vector2D vertices, int count, "
				L"int argb, double z = 0.0, int flags = 0",
				NULL, &RSPaintContextClass::method_fillPolygon, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"fillEllipse", L"boolean",
				L"double xCenter, double yCenter, "
				L"double rWidth, double rHeight, "
				L"int argb, double z = 0.0, int flags = 0",
				NULL, &RSPaintContextClass::method_fillEllipse, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"fillArc", L"boolean",
				L"double xCenter, double yCenter, "
				L"double rWidth, double rHeight, "
				L"double radFirst, double radEnd, "
				L"int argb, double z = 0.0, int flags = 0",
				NULL, &RSPaintContextClass::method_fillArc, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"drawImage", L"boolean",
				L"PaintContext.PaintParam ppPaint, "
				L"Image pSrcImage, Rect pSrcClip = null",
				NULL, &RSPaintContextClass::method_drawImage, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"drawMesh", L"boolean",
				L"Vector2D pDstMesh, Vector2D pSrcMesh, "
				L"int widthMesh, int heightMesh, "
				L"PaintContext.PaintParam ppPaint, "
				L"Image pSrcImage, Rect pSrcClip = null",
				NULL, &RSPaintContextClass::method_drawMesh, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"flush", L"boolean", L"",
				NULL, &RSPaintContextClass::method_flush, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"finish", L"boolean", L"",
				NULL, &RSPaintContextClass::method_finish, NULL ) ;
}

// Object - SGLPaintParam 変換
//////////////////////////////////////////////////////////////////////////////
void RSPaintContextClass::PaintParamFromObject
	( RSContext& context, SGLPaintParam& ppPaint,
		SGLAffine& af, SArray<S2DVector>& vertices, RSObject * pObj )
{
	ppPaint.nFlags = (uint32_t) pObj->GetMemberIntegerAs( context, L"nFlags" ) ;
	RSSmartPtr	pptPaint( pObj->GetMemberAs( context, L"ptPaint" ), &context ) ;
	if ( pptPaint.GetEntity() != NULL )
	{
		ppPaint.ptPaint.x =
			(int32_t) pptPaint->GetMemberIntegerAs( context, L"x" ) ;
		ppPaint.ptPaint.y =
			(int32_t) pptPaint->GetMemberIntegerAs( context, L"y" ) ;
	}
	ppPaint.nTransparency =
		(uint32_t) pObj->GetMemberIntegerAs( context, L"nTransparency" ) ;
	ppPaint.zOrder =
		(float32_t) pObj->GetMemberNumberAs( context, L"zOrder" ) ;
	ppPaint.rgbColorParam =
		(uint32_t) pObj->GetMemberIntegerAs( context, L"rgbColorParam" ) ;
	RSSmartPtr	pAffine( pObj->GetMemberAs( context, L"pAffine" ), &context ) ;
	ppPaint.pAffine = NULL ;
	if ( pAffine.GetEntity() != NULL )
	{
		af = RSAffineClass::AffineFromObject( context, pAffine ) ;
		ppPaint.pAffine = &af ;
	}
	RSSmartPtr	pVertices( pObj->GetMemberAs( context, L"pVertices" ), &context ) ;
	ppPaint.pVertices = NULL ;
	ppPaint.countVertex =
		(uint32_t) pObj->GetMemberIntegerAs( context, L"countVertex" ) ;
	if ( (pVertices.GetEntity() != NULL) && (ppPaint.countVertex > 0) )
	{
		RSTypedArrayPointer *
			pPtr = ESLTypeCast<RSTypedArrayPointer>( pVertices.Ptr() ) ;
		if ( (pPtr != NULL) && (pPtr->GetPointer() != NULL) )
		{
			ppPaint.pVertices = vertices.GetArray( ppPaint.countVertex ) ;
			eslMoveMemory
				( (void*) ppPaint.pVertices,
					pPtr->GetPointer(),
					ppPaint.countVertex * sizeof(S2DVector) ) ;
			vertices.FinishArray() ;
		}
	}
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSPaintContextClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<SGLPaintContextInterface>(pObj) != nullptr) ;
}

// this オブジェクトの描画オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLPaintContextInterface *
	RSPaintContextClass::GetThisPaintContext( RSContext& context, RSObject* pThis )
{
	SGLPaintContextInterface *	pPaint = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pPaint = ESLTypeCast<SGLPaintContextInterface>( pNativeObj->GetObject() ) ;
	}
	if ( pPaint == NULL )
	{
		context.ThrowExceptionError( L"this が PaintContext ではありません" ) ;
	}
	return	pPaint ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"PaintContext.<init> の this が PaintContext ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( new SGLPaintContext ) ;
	return	NULL ;
}

// Image getTargetImage() ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_getTargetImage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPaintContextInterface *	pPaint = GetThisPaintContext( context, pThis ) ;
	if ( pPaint == NULL )
	{
		return	NULL ;
	}
	SGLImageObject *	pImage = pPaint->GetTargetImage() ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject( pImage, context.GetClassAs( L"Image" ) ) ;
}

// Image getTargetZBuffer() ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_getTargetZBuffer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPaintContextInterface *	pPaint = GetThisPaintContext( context, pThis ) ;
	if ( pPaint == NULL )
	{
		return	NULL ;
	}
	SGLImageObject *	pImage = pPaint->GetTargetZBuffer() ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject( pImage, context.GetClassAs( L"Image" ) ) ;
}

// boolean getViewPort( Rect rctView ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_getViewPort
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPaintContextInterface *	pPaint = GetThisPaintContext( context, pThis ) ;
	if ( pPaint == NULL )
	{
		return	NULL ;
	}
	SGLImageRect	rctView ;
	if ( pPaint->GetViewPort( rctView ) )
	{
		return	context.new_Boolean( false ) ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObjRect = arg.ObjectAt( 0 ) ;
	if ( pObjRect == NULL )
	{
		context.ThrowExceptionError
			( L"PaintContext.getViewPort の引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	pObjRect->SetMemberIntegerAs( context, L"x", rctView.x ) ;
	pObjRect->SetMemberIntegerAs( context, L"y", rctView.y ) ;
	pObjRect->SetMemberIntegerAs( context, L"w", rctView.w ) ;
	pObjRect->SetMemberIntegerAs( context, L"h", rctView.h ) ;
	return	context.new_Boolean( true ) ;
}

// boolean attachTargetImage
//	( Image pImage, Image pZBuffer, Rect pView = null ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_attachTargetImage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPaintContextInterface *	pPaint = GetThisPaintContext( context, pThis ) ;
	if ( pPaint == NULL )
	{
		return	NULL ;
	}
	SGLImageObject *	pImage = NULL ;
	SGLImageObject *	pZBuffer = NULL ;
	SGLImageRect		rctView ;
	SGLImageRect *		prctView = NULL ;
	//
	RSContext::SArgList	arg( ppArg, count ) ;
	RSNativeObject *
		pnoImage = ESLTypeCast<RSNativeObject>( arg.ObjectAt( 0 ) ) ;
	if ( pnoImage != NULL )
	{
		pImage = ESLTypeCast<SGLImageObject>( pnoImage->GetObject() ) ;
	}
	RSNativeObject *
		pnoZBuffer = ESLTypeCast<RSNativeObject>( arg.ObjectAt( 1 ) ) ;
	if ( pnoZBuffer != NULL )
	{
		pZBuffer = ESLTypeCast<SGLImageObject>( pnoZBuffer->GetObject() ) ;
	}
	RSObject *	pObjRect = arg.ObjectAt( 2 ) ;
	if ( pObjRect != NULL )
	{
		rctView.x = (int32_t) pObjRect->GetMemberIntegerAs( context, L"x" ) ;
		rctView.y = (int32_t) pObjRect->GetMemberIntegerAs( context, L"y" ) ;
		rctView.w = (int32_t) pObjRect->GetMemberIntegerAs( context, L"w" ) ;
		rctView.h = (int32_t) pObjRect->GetMemberIntegerAs( context, L"h" ) ;
		prctView = &rctView ;
	}
	return	context.new_Boolean
		( pPaint->AttachTargetImage( pImage, pZBuffer, prctView ) == sglErrSuccess ) ;
}

// boolean detachTargetImage()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_detachTargetImage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPaintContextInterface *	pPaint = GetThisPaintContext( context, pThis ) ;
	if ( pPaint == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pPaint->DetachTargetImage() == sglErrSuccess ) ;
}

// boolean appendTransformation( Affine af, int nTransparency )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_appendTransformation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPaintContextInterface *	pPaint = GetThisPaintContext( context, pThis ) ;
	if ( pPaint == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObjAffine = arg.ObjectAt( 0 ) ;
	if ( pObjAffine == NULL )
	{
		context.ThrowExceptionError
			( L"PaintContext.appendTransformation の引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	SGLAffine	af = RSAffineClass::AffineFromObject( context, pObjAffine ) ;
	return	context.new_Boolean
		( pPaint->AppendTransformation( af, arg.IntAt(1) ) == sglErrSuccess ) ;
}

// boolean setTransformation( Affine af, int nTransparency )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_setTransformation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPaintContextInterface *	pPaint = GetThisPaintContext( context, pThis ) ;
	if ( pPaint == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObjAffine = arg.ObjectAt( 0 ) ;
	if ( pObjAffine == NULL )
	{
		context.ThrowExceptionError
			( L"PaintContext.setTransformation の引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	SGLAffine	af = RSAffineClass::AffineFromObject( context, pObjAffine ) ;
	return	context.new_Boolean
		( pPaint->SetTransformation( af, arg.IntAt(1) ) == sglErrSuccess ) ;
}

// boolean currentAffine( Affine af )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_currentAffine
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPaintContextInterface *	pPaint = GetThisPaintContext( context, pThis ) ;
	if ( pPaint == NULL )
	{
		return	NULL ;
	}
	SGLAffine	af ;
	if ( pPaint->CurrentAffine( af ) )
	{
		return	context.new_Boolean( false ) ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObjAffine = arg.ObjectAt( 0 ) ;
	if ( pObjAffine == NULL )
	{
		context.ThrowExceptionError
			( L"PaintContext.currentAffine の引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	RSAffineClass::ObjectFromAffine( context, pObjAffine, af ) ;
	return	context.new_Boolean( true ) ;
}

// int currentTransparency()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_currentTransparency
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPaintContextInterface *	pPaint = GetThisPaintContext( context, pThis ) ;
	if ( pPaint == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pPaint->CurrentTransparency() ) ;
}

// boolean pushTransformation()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_pushTransformation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPaintContextInterface *	pPaint = GetThisPaintContext( context, pThis ) ;
	if ( pPaint == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pPaint->PushTransformation() == sglErrSuccess ) ;
}

// boolean popTransformation()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_popTransformation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPaintContextInterface *	pPaint = GetThisPaintContext( context, pThis ) ;
	if ( pPaint == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pPaint->PopTransformation() == sglErrSuccess ) ;
}

// boolean resetTransformation()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_resetTransformation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPaintContextInterface *	pPaint = GetThisPaintContext( context, pThis ) ;
	if ( pPaint == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pPaint->ResetTransformation() == sglErrSuccess ) ;
}

// void setPaintFlags( long nFlags )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_setPaintFlags
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPaintContextInterface *	pPaint = GetThisPaintContext( context, pThis ) ;
	if ( pPaint == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pPaint->SetPaintFlags( arg.LongAt( 0 ) ) ;
	return	NULL ;
}

// long getPaintFlags()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_getPaintFlags
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPaintContextInterface *	pPaint = GetThisPaintContext( context, pThis ) ;
	if ( pPaint == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pPaint->GetPaintFlags() ) ;
}

// boolean fillClearTarget( int argb = 0xFF000000, long flags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_fillClearTarget
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPaintContextInterface *	pPaint = GetThisPaintContext( context, pThis ) ;
	if ( pPaint == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pPaint->FillClearTarget
			( SGLPalette( arg.IntAt( 0 ) ), arg.LongAt( 1 ) ) == sglErrSuccess ) ;
}

// boolean drawThinLine
//	( int x0, int y0, int x1, int y1, int argb, double z = 0.0, int flags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_drawThinLine
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLDrawContextInterface *	pDraw =
			ESLTypeCast<SGLDrawContextInterface>
				( GetThisPaintContext( context, pThis ) ) ;
	if ( pDraw == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pDraw->DrawThinLine
			( arg.IntAt(0), arg.IntAt(1), arg.IntAt(2), arg.IntAt(3),
				(uint32_t) arg.IntAt(4), arg.DoubleAt(5), (uint32_t) arg.IntAt(6) ) == sglErrSuccess ) ;
}

// boolean drawThinLines
//	( Vector2D pLines, int nLines, int argb, double z = 0.0, int flags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_drawThinLines
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLDrawContextInterface *	pDraw =
			ESLTypeCast<SGLDrawContextInterface>
				( GetThisPaintContext( context, pThis ) ) ;
	if ( pDraw == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t		nLines = (size_t) arg.IntAt(1) ;
	S2DVector *	pvLines =
		(S2DVector*) arg.PointerAt( 0, nLines * sizeof(S2DVector) ) ;
	if ( pvLines == NULL )
	{
		context.ThrowExceptionError
			( L"PaintContext.drawThinLines の引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	return	context.new_Boolean
		( pDraw->DrawThinLines
			( pvLines, nLines,
				(uint32_t) arg.IntAt(2),
				arg.DoubleAt(3), (uint32_t) arg.IntAt(4) ) == sglErrSuccess ) ;
}

// boolean drawEllipse
//	( double xCenter, double yCenter,
//		double rWidth, double rHeight,
//		int argb, double z = 0.0, int flags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_drawEllipse
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLDrawContextInterface *	pDraw =
			ESLTypeCast<SGLDrawContextInterface>
				( GetThisPaintContext( context, pThis ) ) ;
	if ( pDraw == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pDraw->DrawEllipse
			( (float32_t) arg.DoubleAt(0), (float32_t) arg.DoubleAt(1),
				(float32_t) arg.DoubleAt(2), (float32_t) arg.DoubleAt(3),
				(uint32_t) arg.IntAt(4), arg.DoubleAt(5), (uint32_t) arg.IntAt(6) ) == sglErrSuccess ) ;
}

// boolean drawArc
//	( double xCenter, double yCenter,
//		double rWidth, double rHeight,
//		double radFirst, double radEnd, int argb, double z = 0.0, int flags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_drawArc
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLDrawContextInterface *	pDraw =
			ESLTypeCast<SGLDrawContextInterface>
				( GetThisPaintContext( context, pThis ) ) ;
	if ( pDraw == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pDraw->DrawArc
			( (float32_t) arg.DoubleAt(0), (float32_t) arg.DoubleAt(1),
				(float32_t) arg.DoubleAt(2), (float32_t) arg.DoubleAt(3),
				(float32_t) arg.DoubleAt(4), (float32_t) arg.DoubleAt(5),
				(uint32_t) arg.IntAt(6), arg.DoubleAt(7), (uint32_t) arg.IntAt(8) ) == sglErrSuccess ) ;
}

// boolean drawBezier
//	( Vector2D pPoints, int nPoints, int argb, double z = 0.0, int flags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_drawBezier
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLDrawContextInterface *	pDraw =
			ESLTypeCast<SGLDrawContextInterface>
				( GetThisPaintContext( context, pThis ) ) ;
	if ( pDraw == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t		nPoints = (size_t) arg.IntAt(1) ;
	S2DVector *	pvPoints =
		(S2DVector*) arg.PointerAt( 0, nPoints * sizeof(S2DVector) ) ;
	if ( pvPoints == NULL )
	{
		context.ThrowExceptionError
			( L"PaintContext.drawBezier の引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	return	context.new_Boolean
		( pDraw->DrawBezier
			( pvPoints, nPoints,
				(uint32_t) arg.IntAt(2),
				arg.DoubleAt(3), (uint32_t) arg.IntAt(4) ) == sglErrSuccess ) ;
}

// boolean fillRectangle
//	( int x, int y, int width, int height,
//		int argb, double z = 0.0, int flags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_fillRectangle
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPaintContextInterface *	pPaint = GetThisPaintContext( context, pThis ) ;
	if ( pPaint == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pPaint->FillRectangle
			( arg.IntAt(0), arg.IntAt(1), arg.IntAt(2), arg.IntAt(3),
				(uint32_t) arg.IntAt(4), arg.DoubleAt(5), (uint32_t) arg.IntAt(6) ) == sglErrSuccess ) ;
}

// boolean fillPolygon
//	( Vector2D vertices, int count, int argb, double z = 0.0, int flags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_fillPolygon
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPaintContextInterface *	pPaint = GetThisPaintContext( context, pThis ) ;
	if ( pPaint == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t		nCount = (size_t) arg.IntAt(1) ;
	S2DVector *	pvVertices =
		(S2DVector*) arg.PointerAt( 0, nCount * sizeof(S2DVector) ) ;
	if ( pvVertices == NULL )
	{
		context.ThrowExceptionError
			( L"PaintContext.fillPolygon の引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	return	context.new_Boolean
		( pPaint->FillPolygon
			( pvVertices, nCount,
				(uint32_t) arg.IntAt(2),
				arg.DoubleAt(3), (uint32_t) arg.IntAt(4) ) == sglErrSuccess ) ;
}

// boolean fillEllipse
//	( double xCenter, double yCenter,
//		double rWidth, double rHeight,
//		int argb, double z = 0.0, int flags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_fillEllipse
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLDrawContextInterface *	pDraw =
			ESLTypeCast<SGLDrawContextInterface>
				( GetThisPaintContext( context, pThis ) ) ;
	if ( pDraw == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pDraw->FillEllipse
			( (float32_t) arg.DoubleAt(0), (float32_t) arg.DoubleAt(1),
				(float32_t) arg.DoubleAt(2), (float32_t) arg.DoubleAt(3),
				(uint32_t) arg.IntAt(4), arg.DoubleAt(5), (uint32_t) arg.IntAt(6) ) == sglErrSuccess ) ;
}

// boolean fillArc
//	( double xCenter, double yCenter,
//		double rWidth, double rHeight,
//		double radFirst, double radEnd, int argb, double z = 0.0, int flags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_fillArc
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLDrawContextInterface *	pDraw =
			ESLTypeCast<SGLDrawContextInterface>
				( GetThisPaintContext( context, pThis ) ) ;
	if ( pDraw == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pDraw->FillArc
			( (float32_t) arg.DoubleAt(0), (float32_t) arg.DoubleAt(1),
				(float32_t) arg.DoubleAt(2), (float32_t) arg.DoubleAt(3),
				(float32_t) arg.DoubleAt(4), (float32_t) arg.DoubleAt(5),
				(uint32_t) arg.IntAt(6), arg.DoubleAt(7), (uint32_t) arg.IntAt(8) ) == sglErrSuccess ) ;
}

// boolean drawImage
//	( PaintContext.PaintParam ppPaint,
//			Image pSrcImage, Rect pSrcClip = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_drawImage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPaintContextInterface *	pPaint = GetThisPaintContext( context, pThis ) ;
	if ( pPaint == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLAffine			af ;
	SArray<S2DVector>	vertices ;
	SGLPaintParam		ppPaint ;
	RSObject *			pObjPaintParam = arg.ObjectAt( 0 ) ;
	if ( pObjPaintParam == NULL )
	{
		context.ThrowExceptionError
			( L"PaintContext.drawImage の引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	PaintParamFromObject( context, ppPaint, af, vertices, pObjPaintParam ) ;
	//
	SGLImageObject *	pSrcImage =
			RSImageClass::ImageFromObject( context, arg.ObjectAt( 1 ) ) ;
	//
	SGLImageRect *	pSrcClip =
			(SGLImageRect*) arg.PointerAt( 2, sizeof(SGLImageRect) ) ;
	//
	return	context.new_Boolean
		( pPaint->DrawImage( ppPaint, pSrcImage, pSrcClip ) == sglErrSuccess ) ;
}

// boolean drawMesh
//	( Vector2D pDstMesh, Vector2D pSrcMesh,
//		int widthMesh, int heightMesh,
//		PaintContext.PaintParam ppPaint,
//		Image pSrcImage, Rect pSrcClip = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_drawMesh
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPaintContextInterface *	pPaint = GetThisPaintContext( context, pThis ) ;
	if ( pPaint == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t	widthInMesh = (size_t) arg.IntAt( 2 ) ;
	size_t	heightInMesh = (size_t) arg.IntAt( 3 ) ;
	size_t	countVertex = (widthInMesh + 1) * (heightInMesh + 1) ;
	const S2DVector *	pDstMesh =
			(S2DVector*) arg.PointerAt( 0, countVertex * sizeof(S2DVector) ) ;
	const S2DVector *	pSrcMesh =
			(S2DVector*) arg.PointerAt( 1, countVertex * sizeof(S2DVector) ) ;
	if ( pDstMesh == NULL )
	{
		return	NULL ;
	}
	SGLAffine			af ;
	SArray<S2DVector>	vertices ;
	SGLPaintParam		ppPaint ;
	RSObject *			pObjPaintParam = arg.ObjectAt( 4 ) ;
	if ( pObjPaintParam == NULL )
	{
		context.ThrowExceptionError
			( L"PaintContext.drawImage の引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	PaintParamFromObject( context, ppPaint, af, vertices, pObjPaintParam ) ;
	//
	SGLImageObject *	pSrcImage =
			RSImageClass::ImageFromObject( context, arg.ObjectAt( 5 ) ) ;
	//
	SGLImageRect *	pSrcClip =
			(SGLImageRect*) arg.PointerAt( 6, sizeof(SGLImageRect) ) ;
	//
	return	context.new_Boolean
		( pPaint->DrawMesh
			( pDstMesh, pSrcMesh,
				widthInMesh, heightInMesh,
				ppPaint, pSrcImage, pSrcClip ) == sglErrSuccess ) ;
}

// boolean flush()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_flush
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPaintContextInterface *	pPaint = GetThisPaintContext( context, pThis ) ;
	if ( pPaint == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pPaint->Flush() == sglErrSuccess ) ;
}

// boolean finish()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPaintContextClass::method_finish
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLPaintContextInterface *	pPaint = GetThisPaintContext( context, pThis ) ;
	if ( pPaint == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pPaint->Finish() == sglErrSuccess ) ;
}


