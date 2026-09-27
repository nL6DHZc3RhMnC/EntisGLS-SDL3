
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <sakuragl/sgl2d_image.h>
#include <glscs/glscs_sakura2_obj_font.h>

using	namespace SSystem ;
using	namespace SakuraGL ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// Font オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( ECSSakura2::FontObject, Object, SGLFont )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
FontObject::FontObject( void )
{
	m_fFontStyle = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
FontObject::~FontObject( void )
{
}

// スタイル設定
//////////////////////////////////////////////////////////////////////////////
SGLError FontObject::SetStyle( const SakuraGL::SGLFontStyle& style )
{
	m_fFontStyle = true ;
	m_styleFont = style ;
	m_strFontFace = style.pszFace ;
	m_styleFont.pszFace = m_strFontFace ;
	//
	return	SGLFont::SetStyle( style ) ;
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * FontObject::GetTypeName( void ) const
{
	return	L"SakuraGL::Font" ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SError FontObject::SaveStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	uint32_t	nFlags = m_fFontStyle ? 1 : 0 ;
	file->Write( &nFlags, sizeof(uint32_t) ) ;
	file->Write( &m_styleFont.nStyles, sizeof(uint32_t) ) ;
	file->Write( &m_styleFont.nSize, sizeof(uint32_t) ) ;
	file->WriteString( m_strFontFace ) ;
	return	errSuccess ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SError FontObject::LoadStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	uint32_t		nFlags ;
	SGLFontStyle	styleFont ;
	SString			strFontFace ;
	file->Read( &nFlags, sizeof(uint32_t) ) ;
	file->Read( &styleFont.nStyles, sizeof(uint32_t) ) ;
	file->Read( &styleFont.nSize, sizeof(uint32_t) ) ;
	file->ReadString( strFontFace ) ;
	styleFont.pszFace = strFontFace ;
	//
	if ( nFlags & 0x01 )
	{
		SetStyle( styleFont ) ;
	}
	return	errSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// Font スタブ
//////////////////////////////////////////////////////////////////////////////

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SakuraGL::Font
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT(SakuraGL_Font,context,cls_id)
{
	return	new FontObject ;
}

// SGLError Font::EnumerateFonts
//	( SSystem::SObjectArray<SSystem::SString>& listFonts ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Font_EnumerateFonts,context,arg)
{
	ECS_DECLARE_SYSCALL_VM( context, vm ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SSystem_Array, pArray,
				arg[0].i, listFonts at Font::EnumerateFonts ) ;

	SObjectArray<SString>	listFonts ;
	SGLFont::EnumerateFonts( listFonts ) ;
	//
	const size_t	nFontCount = listFonts.GetLength() ;
	INT64 *	pStrArray =
		(INT64*) pArray->AllocateArray
						( nFontCount, sizeof(INT64), vm ) ;
	if ( pStrArray != NULL )
	{
		for ( size_t i = 0; i < nFontCount; i ++ )
		{
			SString *	pFontName = listFonts.GetAt( i ) ;
			if ( pFontName == NULL )
			{
				pStrArray[i] = 0 ;
				continue ;
			}
			pStrArray[i] = vm->AllocateHeapMemory( sizeof(SSystem_Array) ) ;
			SSystem_Array *	pString =
				(SSystem_Array*) context->AtomicTranslateAddress( pStrArray[i] ) ;
			if ( pString != NULL )
			{
				WORD *	pszStrFile =
					(WORD*) pString->AllocateArray
						( pFontName->GetLength() + 1, sizeof(WORD), vm ) ;
				if ( pszStrFile != NULL )
				{
					::eslMoveMemory
						( pszStrFile,
							pFontName->GetConstArray(),
							(pFontName->GetLength() + 1) * sizeof(WORD) ) ;
					pString->m_nLength = (DWORD) pFontName->GetLength() ;
				}
			}
		}
	}
	return	NULL ;
}

// SGLError SetStyle( const SGLFontStyle& style ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Font_SetStyle,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, FontObject, pFont, arg, Font::SetStyle ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, FontObject::FONT_STYLE, pStyle,
				arg[1].i, style at Font::SetStyle ) ;
	//
	SGLFontStyle	style ;
	SString			strFont ;
	strFont = (const uint16_t*)
				context->AtomicTranslateAddress( pStyle->pszFace ) ;
	//
	style.nStyles = pStyle->nStyles ;
	style.nSize = pStyle->nSize ;
	style.pszFace = strFont ;
	//
	context->m_regset[regAcc].i = pFont->SetStyle( style ) ;
	//
	return	NULL ;
}

// SGLError GetMetrics
//	( uint8_t* pbytRasterized, size_t nBufBytes,
//				SGLFontMetrics& metrics, wchar_t wch ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Font_GetMetrics,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLFont, pFont, arg, Font::GetMetrics ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SGLFontMetrics, pMetrics,
				arg[3].i, metrics at Font::GetMetrics ) ;
	uint8_t *	pbytRasterized =
		(uint8_t*) context->AtomicTranslateAddress
						( arg[1].i, (size_t) arg[2].i ) ;
	//
	context->m_regset[regAcc].i =
		pFont->GetMetrics
			( pbytRasterized, (size_t) arg[2].i,
						*pMetrics, (wchar_t) arg[4].i ) ;
	//
	return	NULL ;
}

#endif
