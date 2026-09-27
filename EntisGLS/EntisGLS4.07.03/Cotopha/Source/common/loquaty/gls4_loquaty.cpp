
#include <loquaty/gls4_loquaty.h>
#include <sakuraglx/sprite/sglx_sprite_rectangle.h>
#include <sakuraglx/sprite/sglx_sprite_edit.h>
#include <sakuraglx/sprite/sglx_sprite_message.h>
#include <sakuraglx/sprite/sglx_sprite_movie.h>
#include <sakuraglx/render/sglx3d_scene_sprite.h>
#include <sakuraglx/sglx_std_app.h>

#include <loquaty/EntisGLS4_Environment.h>
#include <loquaty/EntisGLS4_Image.h>
#include <loquaty/EntisGLS4_PaintContext.h>
#include <loquaty/EntisGLS4_ImageComposition.h>
#include <loquaty/EntisGLS4_ImageComposition_Layer.h>
#include <loquaty/EntisGLS4_Material.h>
#include <loquaty/EntisGLS4_RenderDevice_ShaderDesc.h>
#include <loquaty/EntisGLS4_ShadowMapInfo.h>
#include <loquaty/EntisGLS4_RenderDevice.h>
#include <loquaty/EntisGLS4_SoftwareRenderDevice.h>
#include <loquaty/EntisGLS4_RenderBuffer.h>
#include <loquaty/EntisGLS4_VertexVariantBuffer.h>
#include <loquaty/EntisGLS4_VertexBuffer.h>
#include <loquaty/EntisGLS4_RenderContext.h>
#include <loquaty/EntisGLS4_Window.h>
#include <loquaty/EntisGLS4_SpriteTimer.h>
#include <loquaty/EntisGLS4_SpriteMouseListener.h>
#include <loquaty/EntisGLS4_SpriteKeyListener.h>
#include <loquaty/EntisGLS4_Sprite.h>
#include <loquaty/EntisGLS4_SpriteFilterTransparencyDrawer.h>
#include <loquaty/EntisGLS4_SpriteFilterBlendAlpha.h>
#include <loquaty/EntisGLS4_SpriteFilterTone.h>
#include <loquaty/EntisGLS4_SpriteFilterShadingOff.h>
#include <loquaty/EntisGLS4_WindowSprite.h>
#include <loquaty/EntisGLS4_RectangleSprite.h>
#include <loquaty/EntisGLS4_TextSprite.h>
#include <loquaty/EntisGLS4_EditSprite.h>
#include <loquaty/EntisGLS4_MessageSprite.h>
#include <loquaty/EntisGLS4_MovieSprite.h>
#include <loquaty/EntisGLS4_RenderableSprite.h>
#include <loquaty/EntisGLS4_VirtualInput.h>
#include <loquaty/EntisGLS4_Collider.h>
#include <loquaty/EntisGLS4_Collision.h>
#include <loquaty/EntisGLS4_Scene.h>
#include <loquaty/EntisGLS4_SceneSprite.h>
#include <loquaty/EntisGLS4_TextureLibrary.h>
#include <loquaty/EntisGLS4_MaterialLibrary.h>
#include <loquaty/EntisGLS4_ModelPose.h>
#include <loquaty/EntisGLS4_ModelPoseLibrary.h>
#include <loquaty/EntisGLS4_ModelBone.h>
#include <loquaty/EntisGLS4_ModelMarker.h>
#include <loquaty/EntisGLS4_ModelBuffer.h>
#include <loquaty/EntisGLS4_AudioInputStream.h>
#include <loquaty/EntisGLS4_AudioPlayer.h>
#include <loquaty/EntisGLS4_SceneParameter.h>
#include <loquaty/EntisGLS4_SceneSequencer.h>
#include <loquaty/EntisGLS4_SceneProperty.h>
#include <loquaty/EntisGLS4_SceneController.h>
#include <loquaty/EntisGLS4_SceneItem.h>
#include <loquaty/EntisGLS4_SceneSpace.h>
#include <loquaty/EntisGLS4_SceneComposition.h>
#include <loquaty/EntisGLS4_SceneCommon.h>
#include <loquaty/EntisGLS4_SceneCamera.h>
#include <loquaty/EntisGLS4_SceneLight.h>
#include <loquaty/EntisGLS4_SceneSoundItem.h>
#include <loquaty/EntisGLS4_SceneDynamicModel.h>
#include <loquaty/EntisGLS4_SceneInstancingItem.h>
#include <loquaty/EntisGLS4_SceneItemInstanceRef.h>
#include <loquaty/EntisGLS4_SceneMultiModel.h>
#include <loquaty/EntisGLS4_SceneMeshBuffer.h>
#include <loquaty/EntisGLS4_SceneSubComposition.h>
#include <loquaty/EntisGLS4_SceneCustomItem.h>
#include <loquaty/EntisGLS4_SceneCustomController.h>
#include <loquaty/EntisGLS4_ScenePoseController.h>
#include <loquaty/EntisGLS4_SceneMeshController.h>
#include <loquaty/EntisGLS4_SceneComposer.h>
#include <loquaty/EntisGLS4_SceneManager.h>



//////////////////////////////////////////////////////////////////////////////
// LURLSchemer::Open を EntisGLS4 のファイルシステムに上書きする
//////////////////////////////////////////////////////////////////////////////

static LFilePtr GLS4OpenFileProc( const wchar_t * pwszPath, long nOpenFlags ) ;

void Loquaty::SetEntisGLS4FileSystem( void )
{
	LURLSchemer::s_pfnOpen = &GLS4OpenFileProc ;
}

LFilePtr GLS4OpenFileProc( const wchar_t * pwszPath, long nOpenFlags )
{
	SFileInterface *	pFile =
		SFileOpener::DefaultNewOpenFile
			( pwszPath, LGLS4File::OpenFlagsLoquatyToGLS4(nOpenFlags) ) ;
	if ( pFile != nullptr )
	{
		return	std::make_shared<LGLS4File>( pFile ) ;
	}
	return	LURLSchemer::OpenProc( pwszPath, nOpenFlags ) ;
}


//////////////////////////////////////////////////////////////////////////////
// EntisGLS4 ネイティブ関数を登録
//////////////////////////////////////////////////////////////////////////////

void Loquaty::DefineEntisGLS4NativeFunctions( LVirtualMachine * vm )
{
	DEF_LOQUATY_FUNC_LIST( vm ) ;
}


//////////////////////////////////////////////////////////////////////////////
// 環境変数からインクルードパスを追加
//////////////////////////////////////////////////////////////////////////////

void Loquaty::AddEnvironmentIncludePath
	( LVirtualMachine * vm, const wchar_t * pwszEnvName )
{
	SString	strEnvIncludePath ;
	if ( SGLStdApplication::GetEnvironmentVariable
					( pwszEnvName, strEnvIncludePath ) != nullptr )
	{
		SStringParser	sparsPath ;
		sparsPath.AttachString( strEnvIncludePath ) ;
		while ( !sparsPath.IsIndexOverflow() )
		{
			SString	strPath ;
			sparsPath.NextEnclosedString( strPath, L';' ) ;
			if ( !strPath.IsEmpty() )
			{
				vm->SourceProducer().DirectoryPath().AddDirectory
					( std::make_shared<LSubDirectory>( nullptr, strPath ) ) ;
			}
		}
	}
}



//////////////////////////////////////////////////////////////////////////////
// Loquaty::Object コンテナ
//////////////////////////////////////////////////////////////////////////////

ESL_IMPLEMENT_CLASS_INFO( Loquaty::LObjectPtr, SObject )



//////////////////////////////////////////////////////////////////////////////
// 数式／文・評価コンテキスト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( Loquaty::LInstantEvaluator, SObject )

// 仮想マシン関連付け
void LInstantEvaluator::AttachVM( LVirtualMachine& vm )
{
	m_vm = &vm ;
}

// 数式コンパイル
bool LInstantEvaluator::MakeExpression
	( const wchar_t * pwszExpr,
		LClass * pThisClass, Type type, LClass * pRetTypeClass )
{
	LType *	pRetType = nullptr ;
	LType	typeRet ;
	switch ( type )
	{
	case	typeBoolean:
		typeRet = LType( LType::typeBoolean ) ;
		pRetType = &typeRet ;
		break ;
	case	typeInteger:
		typeRet = LType( LType::typeInt64 ) ;
		pRetType = &typeRet ;
		break ;
	case	typeDouble:
		typeRet = LType( LType::typeDouble ) ;
		pRetType = &typeRet ;
		break ;
	case	typeObject:
		typeRet = LType( pRetTypeClass ) ;
		pRetType = &typeRet ;
		break ;
	default:
		break ;
	}

	ESLAssert( m_vm != nullptr ) ;
	if ( m_vm == nullptr )
	{
		return	false ;
	}
	LString				strErrMsg ;
	LPtr<LFunctionObj>	pFunc =
		LCompiler::CompileExpressionAsFunc
			( *m_vm, pwszExpr, pThisClass, &strErrMsg, pRetType ) ;
	m_strErrMsg = strErrMsg.c_str() ;
	if ( pFunc == nullptr )
	{
		return	false ;
	}
	m_pFunc = pFunc ;
	return	true ;
}

// 文コンパイル
bool LInstantEvaluator::MakeStatement
	( const wchar_t * pwszStatement,
		LClass * pThisClass, Type type, LClass * pRetTypeClass )
{
	LType	typeRet ;
	switch ( type )
	{
	case	typeBoolean:
		typeRet = LType( LType::typeBoolean ) ;
		break ;
	case	typeInteger:
		typeRet = LType( LType::typeInt64 ) ;
		break ;
	case	typeDouble:
		typeRet = LType( LType::typeDouble ) ;
		break ;
	case	typeObject:
		typeRet = LType( pRetTypeClass ) ;
		break ;
	default:
		break ;
	}

	ESLAssert( m_vm != nullptr ) ;
	if ( m_vm == nullptr )
	{
		return	false ;
	}
	LString				strErrMsg ;
	LPtr<LFunctionObj>	pFunc =
		LCompiler::CompileStatementsAsFunc
			( *m_vm, pwszStatement, typeRet, pThisClass, &strErrMsg ) ;
	m_strErrMsg = strErrMsg.c_str() ;
	if ( pFunc == nullptr )
	{
		return	false ;
	}
	m_pFunc = pFunc ;
	return	true ;
}

// エラー出力取得
const SSystem::SString& LInstantEvaluator::GetErrorMessages( void ) const
{
	return	m_strErrMsg ;
}

// コンパイル済みか？
bool LInstantEvaluator::IsCompiled( void ) const
{
	return	(m_pFunc != nullptr) ;
}

// コンパイル済み関数取得
const LPtr<LFunctionObj>& LInstantEvaluator::GetFunction( void ) const
{
	return	m_pFunc ;
}

// this 取得
LClass * LInstantEvaluator::GetThisClass( void ) const
{
	return	(m_pFunc != nullptr) ? m_pFunc->GetThisClass() : nullptr ;
}

// 式／文評価
LValue LInstantEvaluator::EvaluateValue
	( const LPtr<LTaskObj>& pTask, const LObjPtr& pThis )
{
	if ( (m_pFunc == nullptr) || (pTask == nullptr) )
	{
		return	LValue() ;
	}
	LObjPtr	pTempThis ;
	LValue	valArg[1] ;
	size_t	nArgs = 0 ;
	if ( (m_pFunc->GetThisClass() != nullptr)
		&& (pThis != nullptr) )
	{
		pTempThis.SetPtr( pThis->CastClassTo( m_pFunc->GetThisClass() ) ) ;
		valArg[0] = LValue( pTempThis ) ;
		nArgs = 1 ;
	}

	auto [retval, exception] =
		pTask->SyncCallFunction( m_pFunc.Ptr(), valArg, nArgs ) ;

	m_pException.SetPtr( exception.Get() ) ;
	return	retval ;
}

bool LInstantEvaluator::EvaluateAsBool
	( const LPtr<LTaskObj>& pTask, const LObjPtr& pThis )
{
	return	EvaluateValue(pTask, pThis).AsBoolean() ;
}

int64_t LInstantEvaluator::EvaluateAsLong
	( const LPtr<LTaskObj>& pTask, const LObjPtr& pThis )
{
	return	EvaluateValue(pTask, pThis).AsInteger() ;
}

double LInstantEvaluator::EvaluateAsDouble
	( const LPtr<LTaskObj>& pTask, const LObjPtr& pThis )
{
	return	EvaluateValue(pTask, pThis).AsDouble() ;
}

SSystem::SString LInstantEvaluator::EvaluateAsString
	( const LPtr<LTaskObj>& pTask, const LObjPtr& pThis )
{
	LString	str = EvaluateValue(pTask, pThis).AsString() ;
	return	SString( str.c_str() ) ;
}

LObjPtr LInstantEvaluator::EvaluateAsObject
	( const LPtr<LTaskObj>& pTask, const LObjPtr& pThis )
{
	LValue	val = EvaluateValue( pTask, pThis ) ;
	if ( val.GetType().IsObject() )
	{
		return	LObjPtr( val.AddRef() ) ;
	}
	return	LObjPtr() ;
}

// 文実行 (void)
void LInstantEvaluator::Execute
	( const LPtr<LTaskObj>& pTask, const LObjPtr& pThis )
{
	EvaluateValue( pTask, pThis ) ;
}

// 非同期実行開始
bool LInstantEvaluator::BeginAsync
	( const LPtr<LTaskObj>& pTask, const LObjPtr& pThis )
{
	if ( (m_pFunc == nullptr) || (pTask == nullptr) )
	{
		return	true ;
	}
	LValue	valArg[1] ;
	size_t	nArgs = 0 ;
	if ( (m_pFunc->GetThisClass() != nullptr)
		&& (pThis != nullptr) )
	{
		valArg[0] = LValue( pThis ) ;
		nArgs = 1 ;
	}
	return	pTask->BeginAsync( m_pFunc, valArg, nArgs ) ;
}

// 例外取得
const LObjPtr& LInstantEvaluator::GetException( void ) const
{
	return	m_pException ;
}



//////////////////////////////////////////////////////////////////////////////
// EntisGLS4.PaintParam -> SGLPaintParam 変換
//////////////////////////////////////////////////////////////////////////////

void Loquaty::GetLPaintParam
	( SGLPaintParam& param,
		SGLAffine& affine, LPtr<LEntisGLS4_PaintParam> pParam )
{
	param.nFlags = (uint32_t) pParam->GetElementLongAs( L"nFlags" ) ;
	param.ptPaint.x = (int32_t) pParam->GetElementLongAs( L"xPaint" ) ;
	param.ptPaint.y = (int32_t) pParam->GetElementLongAs( L"yPaint" ) ;
	param.nTransparency = (uint32_t) pParam->GetElementLongAs( L"nTransparency" ) ;
	param.zOrder = (float32_t) pParam->GetElementDoubleAs( L"zOrder" ) ;
	param.rgbColorParam = (uint32_t) pParam->GetElementLongAs( L"rgbColorParam" ) ;
	param.pAffine = nullptr ;
	param.pVertices = nullptr ;
	param.countVertex = (uint32_t) pParam->GetElementLongAs( L"countVertex" ) ;

	LAffine*	pAffine =
		(LAffine*) pParam->GetElementPointerAs( L"pAffine", sizeof(LAffine) ) ;
	if ( pAffine != nullptr )
	{
		affine = pAffine->ToSGLAffine() ;
		param.pAffine = &affine ;
	}
	param.pVertices =
		(const S2DVector*) pParam->GetElementPointerAs
			( L"pVertices", param.countVertex * sizeof(LVector2) ) ;
}


//////////////////////////////////////////////////////////////////////////////
// EntisGLS4.FontStyle -> SGLFontStyle 変換
//////////////////////////////////////////////////////////////////////////////

void Loquaty::GetLFontStyle
	( SGLFontStyle& fontStyle,
		LString& strFontFace, LPtr<LEntisGLS4_FontStyle> pFontStyle )
{
	fontStyle.nStyles = (uint32_t) pFontStyle->GetElementLongAs( L"styles" ) ;
	fontStyle.nSize = (uint32_t) pFontStyle->GetElementLongAs( L"size" ) ;
	strFontFace = pFontStyle->GetElementStringAs( L"face" ) ;
	fontStyle.pszFace = strFontFace.c_str() ;
}


//////////////////////////////////////////////////////////////////////////////
// SGLFontStyle -> EntisGLS4.FontStyle 変換
//////////////////////////////////////////////////////////////////////////////

void Loquaty::SetLFontStyle
	( LPtr<LEntisGLS4_FontStyle> pFontStyle, const SGLFontStyle& fontStyle )
{
	pFontStyle->SetElementLongAs( L"styles", fontStyle.nStyles ) ;
	pFontStyle->SetElementLongAs( L"size", fontStyle.nSize ) ;
	pFontStyle->SetElementStringAs( L"face", fontStyle.pszFace ) ;
}


//////////////////////////////////////////////////////////////////////////////
// SXMLDocument -> LXMLDocPtr 変換
//////////////////////////////////////////////////////////////////////////////

LXMLDocPtr Loquaty::GetLXMLDocPtr( const SXMLDocument& xmlDoc )
{
	LXMLDocPtr	xmlDstPtr = std::make_shared<LXMLDocument>() ;

	LXMLDocument::ElementType	type = LXMLDocument::typeRoot ;
	switch ( xmlDoc.GetType() )
	{
	case	SXMLDocument::typeRoot:
	default:
		break ;
	case	SXMLDocument::typeTag:
		type = LXMLDocument::typeTag ;
		xmlDstPtr->SetTag( xmlDoc.GetTag(), type ) ;
		break ;
	case	SXMLDocument::typeText:
		type = LXMLDocument::typeText ;
		xmlDstPtr->SetText( xmlDoc.GetText(), type ) ;
		break ;
	case	SXMLDocument::typeCDATA:
		type = LXMLDocument::typeCDATA ;
		xmlDstPtr->SetText( xmlDoc.GetText(), type ) ;
		break ;
	case	SXMLDocument::typeComment:
		type = LXMLDocument::typeComment ;
		xmlDstPtr->SetText( xmlDoc.GetText(), type ) ;
		break ;
	}
	for ( size_t i = 0; i < xmlDoc.GetAttributeCount(); i ++ )
	{
		const SString *	pstrName = xmlDoc.GetAttributeNameAt( i ) ;
		SString *		pstrAttr = xmlDoc.GetAttributeValueAt( i ) ;
		if ( pstrName && pstrAttr )
		{
			xmlDstPtr->SetAttrString( *pstrName, *pstrAttr ) ;
		}
	}
	for ( size_t i = 0; i < xmlDoc.GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlTag = xmlDoc.GetElementAt( i ) ;
		if ( pxmlTag != nullptr )
		{
			LXMLDocPtr	xmlTagPtr = GetLXMLDocPtr( *pxmlTag ) ;
			if ( xmlTagPtr != nullptr )
			{
				xmlDstPtr->AddElement( xmlTagPtr ) ;
			}
		}
	}
	return	xmlDstPtr ;
}


//////////////////////////////////////////////////////////////////////////////
// EntisGLS4.TextSprite.TextStyle -> SGLSpriteText::TextStyle 変換
//////////////////////////////////////////////////////////////////////////////

void Loquaty::GetLTextStyle
	( SGLSpriteText::TextStyle& style,
		LString& strFontFace, LString& strProhibition,
		LPtr<LEntisGLS4_TextSprite_TextStyle> pStyle )
{
	style.boxAlign = (uint32_t) pStyle->GetElementLongAs( L"boxAlign" ) ;

	LObjPtr	pFont( pStyle->GetElementAs( L"font" ) ) ;
	assert( pFont != nullptr ) ;
	if ( pFont != nullptr )
	{
		GetLFontStyle( style.font, strFontFace, pFont ) ;
	}

	LEntisGLS4_TextSprite_Lettering *	pLettering =
		(LEntisGLS4_TextSprite_Lettering*)
			pStyle->GetElementPointerAs
				( L"lettering", sizeof(LEntisGLS4_TextSprite_Lettering) ) ;
	assert( pLettering != nullptr ) ;
	if ( pLettering != nullptr )
	{
		pLettering->ToLetteringContext( style.context ) ;
	}

	strProhibition = pStyle->GetElementStringAs( L"prohibition" ) ;
	style.context.pwszProhibition = strProhibition.c_str() ;

	LEntisGLS4_TextSprite_Decoration *	pDecoration =
		(LEntisGLS4_TextSprite_Decoration*)
			pStyle->GetElementPointerAs
				( L"decoration", sizeof(LEntisGLS4_TextSprite_Decoration) ) ;
	assert( pDecoration != nullptr ) ;
	if ( pDecoration != nullptr )
	{
		pDecoration->ToLettererDecoration( style.decoration ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// SGLSpriteText::TextStyle -> EntisGLS4.TextSprite.TextStyle 変換
//////////////////////////////////////////////////////////////////////////////

void Loquaty::SetLTextStyle
	( LPtr<LEntisGLS4_TextSprite_TextStyle> pStyle,
				const SGLSpriteText::TextStyle& style )
{
	pStyle->SetElementLongAs( L"boxAlign", style.boxAlign ) ;

	LObjPtr	pFont( pStyle->GetElementAs( L"font" ) ) ;
	assert( pFont != nullptr ) ;
	if ( pFont != nullptr )
	{
		SetLFontStyle( pFont, style.font ) ;
	}

	LEntisGLS4_TextSprite_Lettering *	pLettering =
		(LEntisGLS4_TextSprite_Lettering*)
			pStyle->GetElementPointerAs
				( L"lettering", sizeof(LEntisGLS4_TextSprite_Lettering) ) ;
	assert( pLettering != nullptr ) ;
	if ( pLettering != nullptr )
	{
		pLettering->FromLetteringContext( style.context ) ;
	}

	pStyle->SetElementStringAs( L"prohibition", style.context.pwszProhibition ) ;

	LEntisGLS4_TextSprite_Decoration *	pDecoration =
		(LEntisGLS4_TextSprite_Decoration*)
			pStyle->GetElementPointerAs
				( L"decoration", sizeof(LEntisGLS4_TextSprite_Decoration) ) ;
	assert( pDecoration != nullptr ) ;
	if ( pDecoration != nullptr )
	{
		pDecoration->FromLettererDecoration( style.decoration ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// EntisGLS4.EditSprite.ExitStyle -> SGLSpriteEdit::EditStyle 変換
//////////////////////////////////////////////////////////////////////////////

void Loquaty::GetLEditStyle
	( SGLSpriteEdit::EditStyle& style,
		LString& strFontFace, LString& strProhibition,
		LString& strIMEFontFace,
		LPtr<LEntisGLS4_EditSprite_ExitStyle> pStyle )
{
	GetLTextStyle( style, strFontFace, strProhibition, pStyle ) ;

	style.nEditFlags = (uint32_t) pStyle->GetElementLongAs( L"nEditFlags" ) ;
	style.nCaretWidth = (uint32_t) pStyle->GetElementLongAs( L"nCaretWidth" ) ;
	style.nCaretBlinkInterval = (uint32_t) pStyle->GetElementLongAs( L"nCaretBlinkInterval" ) ;

	SGLPalette *	pCaretColor =
		(SGLPalette*) pStyle->GetElementPointerAs
						( L"rgbaCaretColor", sizeof(SGLPalette) ) ;
	assert( pCaretColor != nullptr ) ;
	if ( pCaretColor != nullptr )
	{
		style.rgbaCaretColor = *pCaretColor ;
	}
	SGLPalette *	pSelBackColor =
		(SGLPalette*) pStyle->GetElementPointerAs
						( L"rgbaSelBackColor", sizeof(SGLPalette) ) ;
	assert( pSelBackColor != nullptr ) ;
	if ( pSelBackColor != nullptr )
	{
		style.rgbaSelBackColor = *pSelBackColor ;
	}
	LEntisGLS4_TextSprite_Decoration *	pDecoSel =
		(LEntisGLS4_TextSprite_Decoration*)
			pStyle->GetElementPointerAs
				( L"decoSel", sizeof(LEntisGLS4_TextSprite_Decoration) ) ;
	assert( pDecoSel != nullptr ) ;
	if ( pDecoSel != nullptr )
	{
		pDecoSel->ToLettererDecoration( style.decoSel ) ;
	}
	LPtr<LEntisGLS4_FontStyle>	pFontIME( pStyle->GetElementAs( L"fontIME" ) ) ;
	assert( pFontIME != nullptr ) ;
	if ( pFontIME != nullptr )
	{
		GetLFontStyle( style.fontIME, strIMEFontFace, pFontIME ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// SGLSpriteEdit::EditStyle -> EntisGLS4.EditSprite.ExitStyle 変換
//////////////////////////////////////////////////////////////////////////////

void Loquaty::SetLEditStyle
	( LPtr<LEntisGLS4_EditSprite_ExitStyle> pStyle,
				const SGLSpriteEdit::EditStyle& style )
{
	SetLTextStyle( pStyle, style ) ;

	pStyle->SetElementLongAs( L"nEditFlags", style.nEditFlags ) ;
	pStyle->SetElementLongAs( L"nCaretWidth", style.nCaretWidth ) ;
	pStyle->SetElementLongAs( L"nCaretBlinkInterval", style.nCaretBlinkInterval ) ;

	SGLPalette *	pCaretColor =
		(SGLPalette*) pStyle->GetElementPointerAs
						( L"rgbaCaretColor", sizeof(SGLPalette) ) ;
	assert( pCaretColor != nullptr ) ;
	if ( pCaretColor != nullptr )
	{
		*pCaretColor = style.rgbaCaretColor ;
	}
	SGLPalette *	pSelBackColor =
		(SGLPalette*) pStyle->GetElementPointerAs
						( L"rgbaSelBackColor", sizeof(SGLPalette) ) ;
	assert( pSelBackColor != nullptr ) ;
	if ( pSelBackColor != nullptr )
	{
		*pSelBackColor = style.rgbaSelBackColor ;
	}
	LEntisGLS4_TextSprite_Decoration *	pDecoSel =
		(LEntisGLS4_TextSprite_Decoration*)
			pStyle->GetElementPointerAs
				( L"decoSel", sizeof(LEntisGLS4_TextSprite_Decoration) ) ;
	assert( pDecoSel != nullptr ) ;
	if ( pDecoSel != nullptr )
	{
		pDecoSel->FromLettererDecoration( style.decoSel ) ;
	}
	LPtr<LEntisGLS4_FontStyle>	pFontIME( pStyle->GetElementAs( L"fontIME" ) ) ;
	assert( pFontIME != nullptr ) ;
	if ( pFontIME != nullptr )
	{
		SetLFontStyle( pFontIME, style.fontIME ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// EntisGLS4.MessageSprite.RichTextStyle -> SGLSpriteMessage::RichTextStyle 変換
//////////////////////////////////////////////////////////////////////////////

void Loquaty::GetLRichTextStyle
	( SGLSpriteMessage::RichTextStyle& style,
		LString& strFontFace, LString& strProhibition,
		LString& strRubyFontFace,
		LPtr<LEntisGLS4_MessageSprite_RichTextStyle> pStyle )
{
	GetLTextStyle( style, strFontFace, strProhibition, pStyle ) ;

	LPtr<LEntisGLS4_FontStyle>	pFontRuby( pStyle->GetElementAs( L"fontRuby" ) ) ;
	assert( pFontRuby != nullptr ) ;
	if ( pFontRuby != nullptr )
	{
		GetLFontStyle( style.fontRuby, strRubyFontFace, pFontRuby ) ;
	}
	LEntisGLS4_TextSprite_Decoration *	pDecoLink =
		(LEntisGLS4_TextSprite_Decoration*)
			pStyle->GetElementPointerAs
				( L"decoLink", sizeof(LEntisGLS4_TextSprite_Decoration)
								* SGLSpriteMessage::LinkInfo::statusCount ) ;
	assert( pDecoLink != nullptr ) ;
	for ( size_t i = 0; i < SGLSpriteMessage::LinkInfo::statusCount; i ++ )
	{
		pDecoLink[i].ToLettererDecoration( style.decoLink[i] ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// SGLSpriteMessage::RichTextStyle -> EntisGLS4.MessageSprite.RichTextStyle 変換
//////////////////////////////////////////////////////////////////////////////

void Loquaty::SetLRichTextStyle
	( LPtr<LEntisGLS4_MessageSprite_RichTextStyle> pStyle,
				const SGLSpriteMessage::RichTextStyle& style )
{
	SetLTextStyle( pStyle, style ) ;

	LPtr<LEntisGLS4_FontStyle>	pFontRuby( pStyle->GetElementAs( L"fontRuby" ) ) ;
	assert( pFontRuby != nullptr ) ;
	if ( pFontRuby != nullptr )
	{
		SetLFontStyle( pFontRuby, style.fontRuby ) ;
	}
	LEntisGLS4_TextSprite_Decoration *	pDecoLink =
		(LEntisGLS4_TextSprite_Decoration*)
			pStyle->GetElementPointerAs
				( L"decoLink", sizeof(LEntisGLS4_TextSprite_Decoration)
								* SGLSpriteMessage::LinkInfo::statusCount ) ;
	assert( pDecoLink != nullptr ) ;
	for ( size_t i = 0; i < SGLSpriteMessage::LinkInfo::statusCount; i ++ )
	{
		pDecoLink[i].FromLettererDecoration( style.decoLink[i] ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// EntisGLS4.MessageSprite.MessageStyle -> SGLSpriteMessage::MessageStyle 変換
//////////////////////////////////////////////////////////////////////////////

void Loquaty::GetLMessageStyle
	( SGLSpriteMessage::MessageStyle& style,
		LString& strFontFace, LString& strProhibition,
		LString& strRubyFontFace,
		LPtr<LEntisGLS4_MessageSprite_MessageStyle> pStyle )
{
	GetLRichTextStyle
		( style, strFontFace, strProhibition, strRubyFontFace, pStyle ) ;

	SGLSpriteMessage::ViewActionStyle *	pViewAct =
		(SGLSpriteMessage::ViewActionStyle*)
			pStyle->GetElementPointerAs
				( L"viewAction", sizeof(SGLSpriteMessage::ViewActionStyle) ) ;
	assert( pViewAct != nullptr ) ;
	if ( pViewAct != nullptr )
	{
		style.viewAction = *pViewAct ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// SGLSpriteMessage::MessageStyle -> EntisGLS4.MessageSprite.MessageStyle 変換
//////////////////////////////////////////////////////////////////////////////

void Loquaty::SetLMessageStyle
	( LPtr<LEntisGLS4_MessageSprite_MessageStyle> pStyle,
				const SGLSpriteMessage::MessageStyle& style )
{
	SetLRichTextStyle( pStyle, style ) ;

	SGLSpriteMessage::ViewActionStyle *	pViewAct =
		(SGLSpriteMessage::ViewActionStyle*)
			pStyle->GetElementPointerAs
				( L"viewAction", sizeof(SGLSpriteMessage::ViewActionStyle) ) ;
	assert( pViewAct != nullptr ) ;
	if ( pViewAct != nullptr )
	{
		*pViewAct = style.viewAction ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// EntisGLS4.VirtualInput.Event -> SGLVirtualInput::InputEvent 変換
//////////////////////////////////////////////////////////////////////////////

void Loquaty::GetLInputEvent
	( SGLVirtualInput::InputEvent& event,
		LPtr<LEntisGLS4_VirtualInput_Event> pEvent )
{
	event.typeDevice = (SGLVirtualInput::DeviceType) pEvent->GetElementLongAs( L"type" ) ;
	event.numDevice = pEvent->GetElementLongAs( L"device" ) ;
	event.codeKey = pEvent->GetElementLongAs( L"key" ) ;
	event.strCommand = pEvent->GetElementStringAs( L"command" ).c_str() ;
}


//////////////////////////////////////////////////////////////////////////////
// S3DCollisionResult -> EntisGLS4.Collider.Result 変換
//////////////////////////////////////////////////////////////////////////////

bool Loquaty::SetLColliderResult
	( LPtr<LEntisGLS4_Collider_Result> pResult, const S3DCollisionResult& rs )
{
	LEntisGLS4_Collider_HitInfo *	pHitInfo =
		(LEntisGLS4_Collider_HitInfo*)
			pResult->GetElementPointerAs
				( L"htinf", sizeof(LEntisGLS4_Collider_HitInfo) ) ;
	assert( pHitInfo != nullptr ) ;
	pHitInfo->FromCollisionResult( rs ) ;

	LPtr<LNativeObj>	pItem ;
	if ( rs.pMesh != nullptr )
	{
		S3DSceneComposer::Parameter *	pParam =
			ESLTypeCast<S3DSceneComposer::Parameter>( rs.pMesh->pUserData ) ;
		if ( pParam != nullptr )
		{
			LClass *	pItemClass =
				GetSceneItemClass( pResult->GetClass()->VM(), pParam ) ;
			if ( pItemClass != nullptr )
			{
				pItem.SetPtr( new LNativeObj( pItemClass ) ) ;
				pItem->SetNative( std::make_shared<LEntisGLS4_SceneItem>(pParam) ) ;
			}
		}
	}
	pResult->SetElementAs( L"item", pItem.Get() ) ;
	return	(pItem != nullptr) ;
}


//////////////////////////////////////////////////////////////////////////////
// EntisGLS4.SceneManager.Context -> S3DSceneComposerPluginSceneInfo 変換
//////////////////////////////////////////////////////////////////////////////

void Loquaty::GetLSceneManagerContext
	( S3DSceneComposerPluginSceneInfo& sceneInfo,
		LPtr<LEntisGLS4_SceneManager_Context> pContext )
{
	sceneInfo.pComposer = nullptr ;
	sceneInfo.pWindow = nullptr ;
	sceneInfo.pScene = nullptr ;
	sceneInfo.pInput = nullptr ;
	sceneInfo.pVR = nullptr ;

	std::shared_ptr<LEntisGLS4_SceneComposer>
		composer = pContext->GetElementNativeAs
						<LEntisGLS4_SceneComposer>( L"composer" ) ;
	if ( composer != nullptr )
	{
		sceneInfo.pComposer = composer->GetRef<S3DSceneComposer>() ;
	}
	std::shared_ptr<LEntisGLS4_WindowSprite>
		window = pContext->GetElementNativeAs
						<LEntisGLS4_WindowSprite>( L"window" ) ;
	if ( window != nullptr )
	{
		sceneInfo.pWindow = window->GetRef<SGLWindowSprite>() ;
	}
	std::shared_ptr<LEntisGLS4_SceneSprite>
		scene = pContext->GetElementNativeAs
						<LEntisGLS4_SceneSprite>( L"scene" ) ;
	if ( scene != nullptr )
	{
		sceneInfo.pScene = scene->GetRef<S3DSceneSprite>() ;
	}
	std::shared_ptr<LEntisGLS4_VirtualInput>
		input = pContext->GetElementNativeAs
						<LEntisGLS4_VirtualInput>( L"input" ) ;
	if ( input != nullptr )
	{
		sceneInfo.pInput = input->GetRef<SGLVirtualInput>() ;
	}
}

//////////////////////////////////////////////////////////////////////////////
// S3DSceneComposerPluginSceneInfo -> EntisGLS4.SceneManager.Context 変換
//////////////////////////////////////////////////////////////////////////////

void Loquaty::SetLSceneManagerContext
	( LPtr<LEntisGLS4_SceneManager_Context> pContext,
		const S3DSceneComposerPluginSceneInfo& sceneInfo )
{
	LVirtualMachine&	vm = pContext->GetClass()->VM() ;

	LPtr<LNativeObj>	pComposer ;
	if ( sceneInfo.pComposer != nullptr )
	{
		pComposer.SetPtr( new LNativeObj( vm.GetClassPathAs( L"EntisGLS4.SceneComposer" ) ) ) ;
		pComposer->SetNative( std::make_shared<LEntisGLS4_SceneComposer>( sceneInfo.pComposer ) ) ;
	}
	LObject::ReleaseRef( pContext->SetElementAs( L"composer", pComposer.Get() ) ) ;

	LPtr<LNativeObj>	pWindow ;
	if ( sceneInfo.pWindow != nullptr )
	{
		pWindow.SetPtr( new LNativeObj( vm.GetClassPathAs( sceneInfo.pWindow->GetLQClassName() ) ) ) ;
		pWindow->SetNative( std::make_shared<LEntisGLS4_WindowSprite>( (SGLSprite*) sceneInfo.pWindow ) ) ;
	}
	LObject::ReleaseRef( pContext->SetElementAs( L"window", pWindow.Get() ) ) ;

	LPtr<LNativeObj>	pScene ;
	if ( sceneInfo.pScene != nullptr )
	{
		pScene.SetPtr( new LNativeObj( vm.GetClassPathAs( sceneInfo.pScene->GetLQClassName() ) ) ) ;
		pScene->SetNative( std::make_shared<LEntisGLS4_SceneSprite>( (SGLSprite*) sceneInfo.pScene ) ) ;
	}
	LObject::ReleaseRef( pContext->SetElementAs( L"scene", pScene.Get() ) ) ;

	LPtr<LNativeObj>	pInput ;
	if ( sceneInfo.pInput != nullptr )
	{
		pInput.SetPtr( new LNativeObj( vm.GetClassPathAs( L"EntisGLS4.VirtualInput" ) ) ) ;
		pInput->SetNative( std::make_shared<LEntisGLS4_SceneSprite>( sceneInfo.pInput ) ) ;
	}
	LObject::ReleaseRef( pContext->SetElementAs( L"input", pInput.Get() ) ) ;
}


//////////////////////////////////////////////////////////////////////////////
// Sprite の Loquaty クラスを取得する
//////////////////////////////////////////////////////////////////////////////

LClass * Loquaty::GetSpriteClass( LVirtualMachine& vm, const SGLSprite* pSprite )
{
	if ( pSprite != nullptr )
	{
		return	vm.GetClassPathAs( pSprite->GetLQClassName() ) ;
	}
	return	vm.GetClassPathAs( L"EntisGLS4.Sprite" ) ;
}


//////////////////////////////////////////////////////////////////////////////
// SceneParameter の Loquaty クラスを取得する
//////////////////////////////////////////////////////////////////////////////

LClass * Loquaty::GetSceneItemClass
	( LVirtualMachine& vm, const S3DSceneComposer::Parameter* pParam )
{
	if ( pParam != nullptr )
	{
		return	vm.GetClassPathAs( pParam->GetLQClassName() ) ;
	}
	return	vm.GetClassPathAs( L"EntisGLS4.SceneParameter" ) ;
}


//////////////////////////////////////////////////////////////////////////////
// Sprite.Parameter -> SGLSprite::Parameter 変換
//////////////////////////////////////////////////////////////////////////////

void LEntisGLS4_Sprite_Parameter::ToSGLSpriteParameter
				( SGLSprite::Parameter& param ) const
{
	param.nFlags = nFlags ;
	param.nSpriteFlags = nSpriteFlags ;
	param.vDst = vDst ;
	param.vCenter = vCenter ;
	param.vZoom = vZoom ;
	param.zAngle = zAngle ;
	param.xyCross = xyCross ;
	param.nTransparency = nTransparency ;
	param.paramFilter = paramFilter ;
	param.paramFilter2 = paramFilter2 ;
	param.rgbColorParam = rgbColorParam ;
}


//////////////////////////////////////////////////////////////////////////////
// TextSprite.Lettering 構造体
//////////////////////////////////////////////////////////////////////////////

void LEntisGLS4_TextSprite_Lettering::ToLetteringContext( SGLLetteringContext& lc )
{
	lc.ptStartWriting = ptStartWriting ;
	lc.rectWritable = rectWritable ;
	lc.typeAlignment = typeAlignment ;
	lc.flagVertical = flagVertical ;
	lc.pitchChar = pitchChar ;
	lc.offsetChar = offsetChar ;
	lc.scalePitch = scalePitch ;
	lc.offsetInHalf = offsetInHalf ;
	lc.offsetOutHalf = offsetOutHalf ;
	lc.pitchTab = pitchTab ;
	lc.pitchLine = pitchLine ;
	lc.widthIndent = widthIndent ;
	lc.minHyphening = minHyphening ;
	lc.maxProhibition = maxProhibition ;
}

void LEntisGLS4_TextSprite_Lettering::FromLetteringContext( const SGLLetteringContext& lc )
{
	ptStartWriting = lc.ptStartWriting ;
	rectWritable = lc.rectWritable ;
	typeAlignment = lc.typeAlignment ;
	flagVertical = lc.flagVertical ;
	pitchChar = lc.pitchChar ;
	offsetChar = lc.offsetChar ;
	scalePitch = lc.scalePitch ;
	offsetInHalf = lc.offsetInHalf ;
	offsetOutHalf = lc.offsetOutHalf ;
	pitchTab = lc.pitchTab ;
	pitchLine = lc.pitchLine ;
	widthIndent = lc.widthIndent ;
	minHyphening = lc.minHyphening ;
	maxProhibition = lc.maxProhibition ;
}


//////////////////////////////////////////////////////////////////////////////
// TextSprite.Decoration 構造体
//////////////////////////////////////////////////////////////////////////////

void LEntisGLS4_TextSprite_Decoration::ToLettererDecoration( SGLLetterer::Decoration& deco )
{
	deco.nFlags = nFlags ;
	deco.rgbaBody = rgbaBody ;
	deco.widthBorder = widthBorder ;
	deco.rgbaBorder = rgbaBorder ;
	deco.rgbaShadow = rgbaShadow ;
	deco.ptShadow = ptShadow ;
	deco.widthBorder2 = widthBorder2 ;
	deco.rgbaBorder2 = rgbaBorder2 ;
	deco.nGradationCount = nGradationCount ;
	deco.nGradationHeight = nGradationHeight ;
	deco.pGradation = rgbGradation ;
}

void LEntisGLS4_TextSprite_Decoration::FromLettererDecoration( const SGLLetterer::Decoration& deco )
{
	nFlags = deco.nFlags ;
	rgbaBody = deco.rgbaBody ;
	widthBorder = deco.widthBorder ;
	rgbaBorder = deco.rgbaBorder ;
	rgbaShadow = deco.rgbaShadow ;
	ptShadow = deco.ptShadow ;
	widthBorder2 = deco.widthBorder2 ;
	rgbaBorder2 = deco.rgbaBorder2 ;
	nGradationCount = deco.nGradationCount ;
	nGradationHeight = deco.nGradationHeight ;

	if ( deco.pGradation != nullptr )
	{
		size_t	nCount = std::min( (size_t) nGradationCount, (size_t) 16 ) ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			rgbGradation[i] = deco.pGradation[i] ;
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
// 衝突情報
//////////////////////////////////////////////////////////////////////////////

void LEntisGLS4_Collider_HitInfo::FromCollisionResult( const S3DCollisionResult& rs )
{
	vHitLocal = rs.vHitLocal ;
	vHitGlobal = rs.vHitGlobal ;
	vNormalLocal = rs.vNormalLocal ;
	vNormal = rs.vNormal ;
	matLocal = rs.matLocal ;
	vLocalBase = rs.vLocalBase ;
	fpDistance = rs.fpDistance ;
	iInstance = rs.iInstance ;
	iMesh = rs.iMesh ;
	iPolygon = rs.iPolygon ;
	maskClasses = 0 ;
	maskColliders = 0 ;

	if ( rs.pMesh != nullptr )
	{
		maskClasses = rs.pMesh->maskClasses & ((1 << S3DScene::classCount) - 1) ;
		maskColliders = rs.pMesh->maskClasses >> S3DScene::classCount ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// モデル・マーカー情報
//////////////////////////////////////////////////////////////////////////////

void LEntisGLS4_ModelMarker_Info::FromMarkerInfo( const S3DModelData::MarkerInfo& mi )
{
	m_type = mi.m_type ;
	m_shape = mi.m_shape ;
	m_iCollider = mi.m_iCollider ;
	m_qRotation = mi.m_qRotation ;
	m_vPosition = mi.m_vPosition ;
	m_vDirection = mi.m_vDirection ;
	m_vSize = mi.m_vSize ;
	m_fpRadius = mi.m_fpRadius ;
	m_fpLength = mi.m_fpLength ;
}


//////////////////////////////////////////////////////////////////////////////
// EntisGLS4.SceneComposer.getCurrent 用スタック
//////////////////////////////////////////////////////////////////////////////

thread_local S3DSceneComposer *	LSceneComposerCurrent::t_pCurrent = nullptr ;

LSceneComposerCurrent::LSceneComposerCurrent( S3DSceneComposer * pCurrent )
{
	m_pPrev = t_pCurrent ;
	t_pCurrent = pCurrent ;
}

LSceneComposerCurrent::~LSceneComposerCurrent( void )
{
	t_pCurrent = m_pPrev ;
}



//////////////////////////////////////////////////////////////////////////////
// EntisGLS4.SceneManager.getCurrent 用スタック
//////////////////////////////////////////////////////////////////////////////

thread_local S3DCompositionManager *	LSceneManagerCurrent::t_pCurrent = nullptr ;

LSceneManagerCurrent::LSceneManagerCurrent( S3DCompositionManager * pCurrent )
{
	m_pPrev = t_pCurrent ;
	t_pCurrent = pCurrent ;
}

LSceneManagerCurrent::~LSceneManagerCurrent( void )
{
	t_pCurrent = m_pPrev ;
}


//////////////////////////////////////////////////////////////////////////////
// Loquaty クラスの ItemCreator
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Loquaty::LSceneItemCreator, ItemCreator )

// 構築
//////////////////////////////////////////////////////////////////////////
LSceneItemCreator::LSceneItemCreator( LClass * pClass )
	: m_pClass( pClass )
{
	ESLAssert( pClass != nullptr ) ;
	LArgumentListType			argList ;
	const LVirtualFuncVector&	virtFuncs = pClass->GetVirtualVector() ;

	const ssize_t	iInitFunc = virtFuncs.FindFunction
									( LClass::s_Constructor, argList, pClass ) ;
	if ( iInitFunc >= 0 )
	{
		m_pInitFunc = virtFuncs.GetFunctionAt( (size_t) iInitFunc ) ;
		if ( m_pInitFunc != nullptr )
		{
			m_pTask = m_pClass->VM().new_Task() ;
		}
	}
}

// アイテム生成
//////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Parameter * LSceneItemCreator::NewItem( void )
{
	LObjPtr	pObj( m_pClass->CreateInstance() ) ;
	if ( pObj == nullptr )
	{
		return	nullptr ;
	}
	if ( m_pInitFunc != nullptr )
	{
		ESLAssert( m_pTask != nullptr ) ;
		LValue	valArg[1] ;
		valArg[0] = LValue( pObj ) ;

		auto [value, except] =
			m_pTask->SyncCallFunction( m_pInitFunc.Ptr(), valArg, 1 ) ;
		if ( except != nullptr )
		{
			LString	lstr ;
			except->AsString( lstr ) ;

			std::string	str = lstr.ToString() ;
			LTrace( "exception: %s\n", str.c_str() ) ;

			S3DSceneComposer *	pComposer = LSceneComposerCurrent::GetCurrent() ;
			if ( pComposer != nullptr )
			{
				LStringParser::LineInfo	lineInf ;
				LString			strSource ;
				LExceptionObj *	pExObj = dynamic_cast<LExceptionObj*>( except.Ptr() ) ;
				if ( (pExObj != nullptr)
					&& pExObj->GetThrownSourceInfo( strSource, lineInf ) )
				{
					pComposer->OutputError
						( lstr.c_str(), strSource.c_str(), (int) lineInf.iLine ) ;
				}
				else
				{
					pComposer->OutputError( lstr.c_str() ) ;
				}
			}
		}
	}
	std::shared_ptr<LReference>
		pItemRef = LNativeObj::GetNative<LReference>( pObj.Ptr() ) ;
	if ( pItemRef == nullptr )
	{
		return	nullptr ;
	}
	S3DSceneComposer::Parameter *
			pParam = pItemRef->GetRef<S3DSceneComposer::Parameter>() ;
	if ( pParam == nullptr )
	{
		return	nullptr ;
	}
	S3DSceneCustomProperty *
			pCustomItem = pItemRef->GetRef<S3DSceneCustomProperty>() ;
	if ( pCustomItem == nullptr )
	{
		return	nullptr ;
	}
	pCustomItem->OwnObject() ;

	return	pParam ;
}


//////////////////////////////////////////////////////////////////////////
// Loquaty の S3DSceneComposerDynamicPlugin::Descriptor 実装
//////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////
LScenePluginDescriptor::LScenePluginDescriptor
	( S3DSceneComposerPluginDescriptorType type,
			const wchar_t * pwszMenuPath, const wchar_t * pwszID,
			LPtr<LFunctionObj> pFuncCreateItem,
			LPtr<LFunctionObj> pFuncOnUpdateMenu )
	: Descriptor( type, pwszMenuPath, pwszID ),
		m_pFuncCreateItem( pFuncCreateItem ),
		m_pFuncOnUpdateMenu( pFuncOnUpdateMenu )
{
	ESLAssert( pFuncCreateItem != nullptr ) ;
	m_pTask = pFuncCreateItem->GetClass()->VM().new_Task() ;
}

// メニューからアイテム作成
//////////////////////////////////////////////////////////////////////////
ESLObject * LScenePluginDescriptor::CreateItem
		( const S3DSceneComposerPluginDescriptor& desc,
			const S3DSceneComposerEditorEnvironment& env )
{
	LPtr<LPointerObj>	pPtrEnv
		( m_pTask->Context().new_Pointer
			( &env, sizeof(S3DSceneComposerEditorEnvironment) ) ) ;

	LValue	valArg[1] ;
	valArg[0] = LValue( pPtrEnv ) ;

	auto [valRet, except] =
		m_pTask->SyncCallFunction( m_pFuncCreateItem.Ptr(), valArg, 1 ) ;

	std::shared_ptr<LReference>
		pItemRef = LNativeObj::GetNative<LReference>( valRet.GetObject().Ptr() ) ;
	if ( pItemRef == nullptr )
	{
		return	nullptr ;
	}
	S3DSceneComposer::Parameter *
			pParam = pItemRef->GetRef<S3DSceneComposer::Parameter>() ;
	if ( pParam == nullptr )
	{
		return	nullptr ;
	}
	S3DSceneCustomProperty *
			pCustomItem = pItemRef->GetRef<S3DSceneCustomProperty>() ;
	if ( pCustomItem != nullptr )
	{
		pCustomItem->OwnObject() ;
	}
	else
	{
		pItemRef->DetachReference() ;
	}
	return	pParam ;
}

// メニューの表示状態更新
//////////////////////////////////////////////////////////////////////////
uint32_t LScenePluginDescriptor::OnUpdateMenu
		( const S3DSceneComposerPluginDescriptor& desc,
			const S3DSceneComposer::ItemSerializer* pParentItem )
{
	if ( m_pFuncOnUpdateMenu == nullptr )
	{
		return	0 ;
	}
	LPtr<LNativeObj>	pParentObj
		( new LNativeObj( GetSceneItemClass( m_pTask->Context().VM(), pParentItem ) ) ) ;
	pParentObj->SetNative
		( std::make_shared<LEntisGLS4_SceneItem>
			( (S3DSceneComposer::ItemSerializer*) pParentItem ) ) ;

	LValue	valArg[1] ;
	valArg[0] = LValue( pParentObj ) ;

	auto [valRet, except] =
		m_pTask->SyncCallFunction( m_pFuncOnUpdateMenu.Ptr(), valArg, 1 ) ;

	return	(uint32_t) valRet.AsInteger() ;
}



//////////////////////////////////////////////////////////////////////////////
// LSourceFilePtr コンテナ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( Loquaty::LSourceFilePtrObj, SObject )



//////////////////////////////////////////////////////////////////////////////
// S3DSceneComposer 用コンパイラ
//////////////////////////////////////////////////////////////////////////////

LSceneCompiler::LSceneCompiler
	( S3DSceneComposer& composer,
		LVirtualMachine& vm, LStringParser * src )
	: m_composer( composer ), LCompiler( vm, src )
{
}

// 文字列出力
void LSceneCompiler::PrintString( const LString& str )
{
	LCompiler::PrintString( str ) ;

	m_composer.OutputError( str.c_str() ) ;
}

