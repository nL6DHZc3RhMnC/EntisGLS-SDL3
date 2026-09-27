
#include <sakuraglx/sakuraglx.h>
#include <loquaty/gls4_loquaty.h>
#include <sakuraglx/render/sglx3d_scene_item.h>

using namespace SSystem ;
using namespace SakuraGL ;
using namespace Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// コンポジション・プレイヤー・コントローラー
//////////////////////////////////////////////////////////////////////////////

const SSystem::SXMLDocument::AttrInteger
	S3DCompositionPlayerController::m_aiExpressionType[3] =
{
	{ L"rosetta", S3DQuickVisibleFlagController::vmRosetta },
	{ L"loquaty", S3DQuickVisibleFlagController::vmLoquaty },
	{ nullptr, 0 },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DCompositionPlayerController, Controller )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DCompositionPlayerController, timeline_controller )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DCompositionPlayerController::S3DCompositionPlayerController( void )
	: Controller( m_ItemClassDescriptor.pwszClassID ),
		m_flagPaused( false ),
		m_flagPauseCondition( false ),
		m_vmExprType( vmLoquaty ),
		m_pEditor( nullptr )
{
	AddParameterEntries() ;
}

S3DCompositionPlayerController::S3DCompositionPlayerController( const wchar_t * pwszClassID )
	: Controller( pwszClassID ),
		m_flagPaused( false ),
		m_flagPauseCondition( false ),
		m_vmExprType( vmLoquaty ),
		m_pEditor( nullptr )
{
	AddParameterEntries() ;
}

// パラメータ追加
//////////////////////////////////////////////////////////////////////////////
void S3DCompositionPlayerController::AddParameterEntries( void )
{
	ESLVerify( AddParameterEntry
		( L"time_label", S3DSceneComposer::typeCommand, 0,
			L"時間ラベル", L"タイムライン上に名前を付けます" ) == paramTimeLabel ) ;
	ESLVerify( AddParameterEntry
		( L"expr_lang", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration,
			L"式の言語タイプ" ) == paramExpressionType ) ;
	ESLVerify( AddParameterEntry
		( L"player_command", S3DSceneComposer::typeCommand,
			S3DSceneComposer::attrStringEnumeration,
			L"制御コマンド",
			L"pause [if <条件式>]\n"
			L"jump {<フレーム数> | <ラベル名>} [if <条件式>]\n"
			L"let <式>" ) == paramPlayerCommand ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DCompositionPlayerController::~S3DCompositionPlayerController( void )
{
}

// ラベルフレーム取得
//////////////////////////////////////////////////////////////////////////////
int32_t S3DCompositionPlayerController::GetLabelFrameAs
	( const wchar_t * pwszLabel, bool * pFoundFrame ) const
{
	S3DSceneComposer::CommandSequencer *
			pSeq = ESLTypeCast<S3DSceneComposer::CommandSequencer>
								( GetParameterSequencer( paramTimeLabel ) ) ;
	if ( pSeq != nullptr )
	{
		size_t	nCount = pSeq->GetKeyFrameCount() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			if ( SString::Compare
				( pSeq->GetCommandParameter(i), pwszLabel ) == 0 )
			{
				S3DSceneComposer::KeyFrameParam	kfp ;
				if ( pSeq->GetKeyFrameParameter( i, kfp ) )
				{
					if ( pFoundFrame != nullptr )
					{
						*pFoundFrame = true ;
					}
					return	kfp.iFrame ;
				}
			}
		}
	}
	if ( pFoundFrame != nullptr )
	{
		*pFoundFrame = false ;
	}
	return	0 ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DCompositionPlayerController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramTimeLabel:
		return	m_strLabel ;

	case	paramExpressionType:
		return	SXMLDocument::GetSymbolAsIntegerOf( m_aiExpressionType, m_vmExprType ) ;

	case	paramPlayerCommand:
		return	m_strCommand ;
	}
	return	nullptr ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DCompositionPlayerController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramTimeLabel:
		m_strLabel = pwszCmd ;
		break ;

	case	paramExpressionType:
		m_vmExprType = (ExpressionVM)
			SXMLDocument::GetIntegerAsSymbolOf
				( m_aiExpressionType, pwszCmd, m_vmExprType ) ;
		return ;

	case	paramPlayerCommand:
		if ( (pwszCmd != nullptr) && (pwszCmd[0] != 0) )
		{
			if ( m_sparsCommands.IsIndexOverflow() )
			{
				m_strCommand = pwszCmd ;
			}
			else
			{
				m_strCommand = m_sparsCommands.SubString( m_sparsCommands.GetIndex() ) ;
				m_strCommand += L"\n" ;
				m_strCommand += pwszCmd ;
			}
			m_sparsCommands.AttachString( m_strCommand ) ;
		}
		break ;
	}
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DCompositionPlayerController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	if ( i == paramPlayerCommand )
	{
		aStrSet.Add( new SString( L"pause" ) ) ;
		//
		S3DSceneComposer::CommandSequencer *
				pSeq = ESLTypeCast<S3DSceneComposer::CommandSequencer>
									( GetParameterSequencer( paramTimeLabel ) ) ;
		if ( pSeq != nullptr )
		{
			size_t	nCount = pSeq->GetKeyFrameCount() ;
			for ( size_t i = 0; i < nCount; i ++ )
			{
				SString *	pstrJump = new SString( L"jump " ) ;
				*pstrJump += pSeq->GetCommandParameter(i) ;
				aStrSet.Add( pstrJump ) ;
			}
		}
		return	true ;
	}
	else if ( i == paramExpressionType )
	{
		for ( i = 0; m_aiExpressionType[i].pszSymbol != nullptr; i ++ )
		{
			aStrSet.Add( new SString(m_aiExpressionType[i].pszSymbol) ) ;
		}
		return	true ;
	}
	return	false ;
}

// フレーム（パラメータ）更新後処理
//////////////////////////////////////////////////////////////////////////////
void S3DCompositionPlayerController::OnUpdateFrame
	( S3DSceneComposer::ItemSerializer * pItem,
		double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	Controller::OnUpdateFrame( pItem, fpFrame, seek ) ;
	//
	if ( (seek == S3DSceneComposer::seekJump)
		|| (seek == S3DSceneComposer::seekJumpReset) )
	{
		m_flagPaused = false ;
	}
	if ( /*((seek == S3DSceneComposer::seekStream)
			|| (seek == S3DSceneComposer::seekStreamPaused))
		&&*/ !m_sparsCommands.IsIndexOverflow() && !m_flagPaused )
	{
		S3DSceneComposer::Composition *	pComp = pItem->GetComposition() ;
		if ( (pComp != nullptr) && !pComp->IsEditMode() )
		{
			FetchNextCommand( pComp ) ;
		}
		else
		{
			m_sparsCommands.SeekIndex( m_sparsCommands.GetLength() ) ;
		}
	}
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DCompositionPlayerController::OnTimer
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast )
{
	if ( m_flagPaused )
	{
		if ( !EvalConditionExpression() )
		{
			ESLTrace( "S3DCompositionPlayerController restart.\n" ) ;
			S3DSceneComposer::Composition *	pComp = pItem->GetComposition() ;
			if ( pComp != nullptr )
			{
				pComp->RestartComposition() ;
			}
			m_flagPaused = false ;
			//
			if ( pComp != nullptr )
			{
				FetchNextCommand( pComp ) ;
			}
		}
	}
	if ( !m_flagPaused )
	{
		m_flagsBehavior &= ~S3DSceneComposer::behaviorOnTimer ;
	}
}

// 次のコマンドをフェッチ
//////////////////////////////////////////////////////////////////////////////
bool S3DCompositionPlayerController::FetchNextCommand
			( S3DSceneComposer::Composition * pComp )
{
	SString	strCmdLine ;
	SString	strCmdCode ;
	SString	strParam ;
	while ( !m_sparsCommands.IsIndexOverflow() )
	{
		SStringParser	sparsCmdLine ;
		m_sparsCommands.NextLine( strCmdLine ) ;
		sparsCmdLine.AttachString( strCmdLine ) ;
		//
		sparsCmdLine.NextString( strCmdCode ) ;
		if ( strCmdCode == L"pause" )
		{
			ESLAssert( !m_flagPaused ) ;
			bool	flagLastPaused = m_flagPaused ;
			if ( ParseIfExpression( pComp, sparsCmdLine ) )
			{
				if ( !EvalConditionExpression() )
				{
					// 条件 false なので次へ進む
					ESLTrace( "S3DCompositionPlayerController pause condition is false.\n" ) ;
					continue ;
				}
				m_flagPaused = true ;
				m_flagsBehavior |= S3DSceneComposer::behaviorOnTimer ;
			}
			ESLTrace( "S3DCompositionPlayerController pause.\n" ) ;
			if ( !flagLastPaused )
			{
				pComp->PauseCompositoin() ;
			}
			return	true ;
		}
		else if ( strCmdCode == L"jump" )
		{
			bool	flagLabel ;
			sparsCmdLine.NextString( strParam ) ;
			double	nFrame = GetLabelFrameAs( strParam, &flagLabel ) ;
			if ( !flagLabel )
			{
				nFrame = strParam.AsReal() ;
			}
			if ( ParseIfExpression( pComp, sparsCmdLine ) )
			{
				if ( !EvalConditionExpression() )
				{
					// 条件 false なので jump せずに次へ進む
					continue ;
				}
			}
			m_sparsCommands.SeekIndex( m_sparsCommands.GetLength() ) ;
			pComp->PostTimelineFrame( nFrame ) ;
			break ;
		}
		else if ( strCmdCode == L"let" )
		{
			SetConditionExpression( pComp, sparsCmdLine ) ;
			EvalConditionExpression() ;
		}
	}
	return	false ;
}

// if 文解釈
//////////////////////////////////////////////////////////////////////////////
bool S3DCompositionPlayerController::ParseIfExpression
	( S3DSceneComposer::Composition * pComp,
		SSystem::SStringParser& sparsCmdLine )
{
	if ( sparsCmdLine.HasToComeToken( L"if" ) )
	{
		SetConditionExpression
			( pComp, sparsCmdLine.SubString( sparsCmdLine.GetIndex() ) ) ;
		return	true ;
	}
	m_flagPauseCondition = false ;
	return	false ;
}

// 条件式評価
//////////////////////////////////////////////////////////////////////////////
bool S3DCompositionPlayerController::EvalConditionExpression( void )
{
	if ( !m_flagPauseCondition )
	{
		return	true ;
	}
	S3DSceneDebugTracer	tracer( m_pEditor ) ;
	if ( m_vmExprType == vmRosetta )
	{
		RSSmartPtr	pObj
			( m_exprCondition.EvalExpressionAsRosetta( &tracer ) ) ;
		return	(pObj != nullptr) ? pObj->AsBoolean() : false ;
	}
	else
	{
		Loquaty::LValue	value
			( m_exprCondition.EvalExpressionAsLoquaty( &tracer ) ) ;
		return	value.AsBoolean() ;
	}
}

// 条件式設定
//////////////////////////////////////////////////////////////////////////////
void S3DCompositionPlayerController::SetConditionExpression
	( S3DSceneComposer::Composition * pComp, const wchar_t * pwszExpr )
{
	m_flagPauseCondition = true ;
	m_pEditor = GetEditor() ;

	S3DSceneDebugTracer	tracer( m_pEditor ) ;
	if ( m_vmExprType == vmRosetta )
	{
		pComp->GetManager()->MakeExpressionAsRosetta
						( m_exprCondition, pwszExpr, &tracer ) ;
	}
	else
	{
		m_flagPauseCondition =
			pComp->GetManager()->MakeExpressionAsLoquaty
						( m_exprCondition, pwszExpr, &tracer ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// フラグ表示制御コントローラー
//////////////////////////////////////////////////////////////////////////////

const SSystem::SXMLDocument::AttrInteger
	S3DQuickVisibleFlagController::m_aiCombinationLogic[3] =
{
	{ L"and", S3DQuickVisibleFlagController::combineAnd },
	{ L"or", S3DQuickVisibleFlagController::combineOr },
	{ nullptr, 0 },
} ;

const SSystem::SXMLDocument::AttrInteger
	S3DQuickVisibleFlagController::m_aiExpressionType[3] =
{
	{ L"rosetta", S3DQuickVisibleFlagController::vmRosetta },
	{ L"loquaty", S3DQuickVisibleFlagController::vmLoquaty },
	{ nullptr, 0 },
} ;

const SSystem::SXMLDocument::AttrInteger
	S3DQuickVisibleFlagController::m_aiDefaultVisibleFlag[4] =
{
	{ L"invisible", S3DQuickVisibleFlagController::defaultInvisible },
	{ L"visible", S3DQuickVisibleFlagController::defaultVisible },
	{ L"timeline", S3DQuickVisibleFlagController::defaultTimeline },
	{ nullptr, 0 },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DQuickVisibleFlagController, Controller )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DQuickVisibleFlagController, quick_vis_flag )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DQuickVisibleFlagController::S3DQuickVisibleFlagController( void )
	: Controller( m_ItemClassDescriptor.pwszClassID ),
		m_flagEditMode( false ),
		m_flagUnderControl( false ),
		m_flagResetVisible( true ),
		m_flagNegativeLogic( false ),
		m_logicCombination( combineAnd ),
		m_defaultVisibleFlag( defaultVisible )
{
	m_flagsBehavior |= S3DSceneComposer::behaviorOnTimer ;

	ESLVerify( AddParameterEntry
		( L"neg_logic", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"不論理", L"評価を負論理で行うか？\n全ての条件の合成の評価の論理否定" ) == paramNegativeLogic ) ;
	ESLVerify( AddParameterEntry
		( L"comb_logic", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration,
			L"結合論理",
			L"複数のフラグを指定した場合の結合方法" ) == paramCombinationLogic ) ;
	ESLVerify( AddParameterEntry
		( L"expr_lang", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration,
			L"式の言語タイプ" ) == paramExpressionType ) ;
	ESLVerify( AddParameterEntry
		( L"default_flag", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration,
			L"デフォルト値" ) == paramDefaultFlag ) ;
	ESLVerify( AddParameterEntry
		( L"flag_expr0", S3DSceneComposer::typeCommand,
			S3DSceneComposer::attrConstant,
			L"フラグ式1",
			L"Rosetta 変数を参照する式\n"
			L"※式は毎回評価されず、参照先の変数の内容を評価します" ) == paramFlagExpr0 ) ;
	ESLVerify( AddParameterEntry
		( L"flag_expr1", S3DSceneComposer::typeCommand,
			S3DSceneComposer::attrConstant,
			L"フラグ式2",
			L"Rosetta 変数を参照する式\n"
			L"※式は毎回評価されず、参照先の変数の内容を評価します" ) == paramFlagExpr1 ) ;
	ESLVerify( AddParameterEntry
		( L"flag_expr2", S3DSceneComposer::typeCommand,
			S3DSceneComposer::attrConstant,
			L"フラグ式3",
			L"Rosetta 変数を参照する式\n"
			L"※式は毎回評価されず、参照先の変数の内容を評価します" ) == paramFlagExpr2 ) ;
	ESLVerify( AddParameterEntry
		( L"flag_expr3", S3DSceneComposer::typeCommand,
			S3DSceneComposer::attrConstant,
			L"フラグ式4",
			L"Rosetta 変数を参照する式\n"
			L"※式は毎回評価されず、参照先の変数の内容を評価します" ) == paramFlagExpr3 ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DQuickVisibleFlagController::~S3DQuickVisibleFlagController( void )
{
}

// フラグ変数参照
//////////////////////////////////////////////////////////////////////////////
void S3DQuickVisibleFlagController::UpdateFlagReflections( void )
{
	for ( size_t i = 0; i < paramFlagExprCount; i ++ )
	{
		UpdateFlagReflectionAt( i ) ;
	}
}

void S3DQuickVisibleFlagController::UpdateFlagReflectionAt( size_t iFlag )
{
	if ( m_strFlagExpr[iFlag].IsEmpty() )
	{
		m_pFlagExpr[iFlag] = nullptr ;
		return ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp == nullptr )
	{
		m_pFlagExpr[iFlag] = nullptr ;
		return ;
	}
	m_flagEditMode = pComp->IsEditMode() ;
	//
	S3DSceneDebugTracer	tracer( GetEditor() ) ;
	if ( m_vmExprType == vmRosetta )
	{
		m_pFlagExpr[iFlag] =
			pComp->GetManager()->
				EvalExpressionAsRosetta( m_strFlagExpr[iFlag], &tracer ) ;
	}
	else
	{
		m_pFlagExpr[iFlag] =
			pComp->GetManager()->
				EvalRefExpressionAsLoquaty( m_strFlagExpr[iFlag] ) ;
	}
}

// フラグ評価
//////////////////////////////////////////////////////////////////////////////
bool S3DQuickVisibleFlagController::EvaluateFlags( bool flagCurVisible, bool& flagController )
{
	bool	flagVisible = true ;
	bool	flagAnyFlags = false ;
	switch ( m_logicCombination )
	{
	case	combineAnd:
		flagVisible = true ;
		break ;
	case	combineOr:
		flagVisible = false ;
		break ;
	}
	for ( size_t i = 0; i < paramFlagExprCount; i ++ )
	{
		if ( !m_pFlagExpr[i].IsNull() )
		{
			bool	b = (m_vmExprType == vmRosetta)
							? m_pFlagExpr[i].GetRosetta()->AsBoolean()
							: m_pFlagExpr[i].GetLoquaty().AsBoolean() ;
			switch ( m_logicCombination )
			{
			case	combineAnd:
				flagVisible = (flagVisible && b) ;
				break ;
			case	combineOr:
				flagVisible = (flagVisible || b) ;
				break ;
			}
			flagAnyFlags = true ;
		}
	}
	if ( m_flagNegativeLogic )
	{
		flagVisible = !flagVisible ;
	}
	flagController = flagAnyFlags ;
	if ( flagAnyFlags )
	{
		return	flagVisible ;
	}
	switch ( m_defaultVisibleFlag )
	{
	case	defaultInvisible:
		return	false ;
	case	defaultVisible:
		return	true ;
	default:
		break ;
	}
	return	flagCurVisible ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
bool S3DQuickVisibleFlagController::GetBooleanParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramNegativeLogic:
		return	m_flagNegativeLogic ;
	}
	return	false ;
}

const wchar_t * S3DQuickVisibleFlagController::GetCommandParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramCombinationLogic:
		return	SXMLDocument::GetSymbolAsIntegerOf( m_aiCombinationLogic, m_logicCombination ) ;

	case	paramExpressionType:
		return	SXMLDocument::GetSymbolAsIntegerOf( m_aiExpressionType, m_vmExprType ) ;

	case	paramDefaultFlag:
		return	SXMLDocument::GetSymbolAsIntegerOf( m_aiDefaultVisibleFlag, m_defaultVisibleFlag ) ;

	case	paramFlagExpr0:
	case	paramFlagExpr1:
	case	paramFlagExpr2:
	case	paramFlagExpr3:
		return	m_strFlagExpr[iParam - paramFlagExpr0] ;
	}
	return	nullptr ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DQuickVisibleFlagController::SetBooleanParameter( size_t iParam, bool b )
{
	switch ( iParam )
	{
	case	paramNegativeLogic:
		m_flagNegativeLogic = b ;
		return ;
	}
}

void S3DQuickVisibleFlagController::SetCommandParameter( size_t iParam, const wchar_t * pwszCmd )
{
	switch ( iParam )
	{
	case	paramCombinationLogic:
		m_logicCombination = (CombinationLogicType)
			SXMLDocument::GetIntegerAsSymbolOf
				( m_aiCombinationLogic, pwszCmd, m_logicCombination ) ;
		return ;

	case	paramExpressionType:
		m_vmExprType = (ExpressionVM)
			SXMLDocument::GetIntegerAsSymbolOf
				( m_aiExpressionType, pwszCmd, m_vmExprType ) ;
		return ;

	case	paramDefaultFlag:
		m_defaultVisibleFlag = (DefaultVisibleFlag)
			SXMLDocument::GetIntegerAsSymbolOf
				( m_aiDefaultVisibleFlag, pwszCmd, m_defaultVisibleFlag ) ;
		return ;

	case	paramFlagExpr0:
	case	paramFlagExpr1:
	case	paramFlagExpr2:
	case	paramFlagExpr3:
		if ( m_strFlagExpr[iParam - paramFlagExpr0] != pwszCmd )
		{
			m_strFlagExpr[iParam - paramFlagExpr0] = pwszCmd ;
			UpdateFlagReflectionAt( iParam - paramFlagExpr0 ) ;
		}
		return ;
	}
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DQuickVisibleFlagController::EnumerateStringSet
	( size_t iParam, SSystem::SStringArray& aStrSet )
{
	size_t	i ;
	switch ( iParam )
	{
	case	paramCombinationLogic:
		for ( i = 0; m_aiCombinationLogic[i].pszSymbol != nullptr; i ++ )
		{
			aStrSet.Add( new SString(m_aiCombinationLogic[i].pszSymbol) ) ;
		}
		return	true ;

	case	paramExpressionType:
		for ( i = 0; m_aiExpressionType[i].pszSymbol != nullptr; i ++ )
		{
			aStrSet.Add( new SString(m_aiExpressionType[i].pszSymbol) ) ;
		}
		return	true ;
	}
	return	false ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DQuickVisibleFlagController::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags )
{
	uint32_t	nResFlags =
		Controller::UpdatePropertyReference( comp, pItem, nFlags ) ;

	if ( nFlags & S3DSceneComposer::updateRefScriptObject )
	{
		UpdateFlagReflections() ;
	}
	return	nResFlags ;
}

// フレーム（パラメータ）更新後処理
//////////////////////////////////////////////////////////////////////////////
void S3DQuickVisibleFlagController::OnUpdateFrame
	( S3DSceneComposer::ItemSerializer * pItem,
		double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	Controller::OnUpdateFrame( pItem, fpFrame, seek ) ;

	if ( seek == S3DSceneComposer::seekJumpReset )
	{
		S3DSceneComposer::CommonSerializer *	pCmnSer =
			ESLTypeCast<S3DSceneComposer::CommonSerializer>( pItem ) ;
		if ( pCmnSer != nullptr )
		{
			if ( m_flagUnderControl )
			{
				if ( m_flagEditMode )
				{
					pCmnSer->SetVisibleParameter( m_flagResetVisible ) ;
					m_flagUnderControl = false ;
				}
			}
			else
			{
				m_flagResetVisible = pCmnSer->GetVisibleParameter() ;
			}
		}
	}
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DQuickVisibleFlagController::OnTimer
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast )
{
	Controller::OnTimer( scene, pItem, msecPast ) ;

	S3DSceneComposer::CommonSerializer *	pCmnSer =
		ESLTypeCast<S3DSceneComposer::CommonSerializer>( pItem ) ;
	if ( pCmnSer != nullptr )
	{
		pCmnSer->SetVisibleParameter
			( EvaluateFlags
				( pCmnSer->GetVisibleParameter(), m_flagUnderControl ) ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// カメラ連動空間
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DCameraRelativeSpace::m_paramEntries
			[S3DCameraRelativeSpace::paramCameraRelSpaceCount] =
{
	{ L"reference_camera",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrStringEnumeration,	L"参照カメラ", nullptr },
	{ L"modified_camera_space",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant,	L"補正後カメラ空間", nullptr },
} ;

const S3DSceneComposer::ParamSetClass	S3DCameraRelativeSpace::m_pscClass =
{
	&S3DSceneComposer::SpaceSerializer::m_pscClass,
	S3DCameraRelativeSpace::paramCameraRelSpaceCount,
	&S3DCameraRelativeSpace::m_paramEntries[0]
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DCameraRelativeSpace, SpaceSerializer )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DCameraRelativeSpace, rel_camera_space )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DCameraRelativeSpace::S3DCameraRelativeSpace( void )
	: SpaceSerializer
		( m_ItemClassDescriptor.pwszClassID,
			&S3DCameraRelativeSpace::m_pscClass ),
		m_matLocalSpace( 1, 0, 0,  0, 1, 0,  0, 0, 1 ),
		m_vLocalSpace( 0, 0, 0 ), m_flagModifiedCamera( false )
{
	m_flagsSpaceBehavior |= S3DScene::itemGlobalSpace ;
}

// 参照カメラ設定
//////////////////////////////////////////////////////////////////////////////
void S3DCameraRelativeSpace::SetRelativeCamera
	( S3DScene::Camera * pCamera,
		const wchar_t * pwszCameraID, bool flagModifiedCameraSpace )
{
	m_refCamera.SetReference( pCamera ) ;
	m_strRelCameraID = pwszCameraID ;
	m_flagModifiedCamera = flagModifiedCameraSpace ;
}

// 座標更新
//////////////////////////////////////////////////////////////////////////////
void S3DCameraRelativeSpace::UpdateLocalTransformation( void )
{
	S3DScene::Camera *	pCamera = m_refCamera.GetReference() ;
	if ( pCamera != nullptr )
	{
		S3DDMatrix	matCamera ;
		S3DDVector	vCamera ;
		GetReferenceCameraTransformation( matCamera, vCamera, pCamera ) ;
		//
		S3DDMatrix	matICamera ;
		S3DDVector	vICamera ;
		matICamera.InverseOf( matCamera ) ;
		vICamera = matICamera * vCamera ;
		//
		m_matTransformation = matICamera * m_matLocalSpace ;
		m_vCenter = matICamera * m_vLocalSpace + vICamera ;
	}
}

void S3DCameraRelativeSpace::GetReferenceCameraTransformation
	( S3DDMatrix& matCamera, S3DDVector& vCamera,
					const S3DScene::Camera * pCamera )
{
	if ( m_flagModifiedCamera )
	{
		S3DScene::CalcCameraTransformation( matCamera, vCamera, pCamera ) ;
	}
	else
	{
		S3DScene::CalcUnmodifiedCameraTransformation( matCamera, vCamera, pCamera ) ;
	}
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
bool S3DCameraRelativeSpace::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramModifiedCameraTrans:
		return	m_flagModifiedCamera ;
	}
	return	SpaceSerializer::GetBooleanParameter( i ) ;
}

const wchar_t * S3DCameraRelativeSpace::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRelativeCamera:
		return	m_strRelCameraID ;
	}
	return	SpaceSerializer::GetCommandParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DCameraRelativeSpace::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramModifiedCameraTrans:
		m_flagModifiedCamera = b ;
		return ;
	}
	SpaceSerializer::SetBooleanParameter( i, b ) ;
}

void S3DCameraRelativeSpace::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramRelativeCamera:
		if ( m_strRelCameraID != pwszCmd )
		{
			S3DScene::Camera *	pCamera =
				ESLTypeCast<S3DScene::Camera>
						( GetSceneItemAs( pwszCmd ) ) ;
			SetRelativeCamera( pCamera, pwszCmd ) ;
		}
		return ;
	}
	SpaceSerializer::SetCommandParameter( i, pwszCmd ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DCameraRelativeSpace::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	switch ( i )
	{
	case	paramRelativeCamera:
		{
			S3DSceneComposer::Composition *	pComp = GetComposition() ;
			if ( pComp != nullptr )
			{
				pComp->EnumerateItemIDsAs
					( aStrSet, ESL_RUNTIME_CLASS(S3DScene::Camera) ) ;
			}
		}
		return	true ;
	}
	return	SpaceSerializer::EnumerateStringSet( i, aStrSet ) ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DCameraRelativeSpace::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramGlobalSpace:
	case	paramCameraShift:
		return	false ;
	}
	return	SpaceSerializer::IsParameterValidation( i ) ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DCameraRelativeSpace::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp, uint32_t nFlags )
{
	uint32_t	nResFlags =
		SpaceSerializer::UpdatePropertyReference( comp, nFlags ) ;
	//
	if ( (nFlags & S3DSceneComposer::updateRefItem)
		&& !m_strRelCameraID.IsEmpty() )
	{
		S3DScene::Camera *	pCamera =
			ESLTypeCast<S3DScene::Camera>
					( GetSceneItemAs( m_strRelCameraID ) ) ;
		SetRelativeCamera( pCamera, m_strRelCameraID ) ;
		//
		if ( nResFlags == 0 )
		{
			nResFlags |= S3DSceneComposer::updateRefItem ;
		}
	}
	//
	return	nResFlags ;
}

// ローカル変換行列を取得
//////////////////////////////////////////////////////////////////////////////
const S3DDMatrix&
	S3DCameraRelativeSpace::GetLocalTransformation( S3DDMatrix& matLocal ) const
{
	matLocal = m_matLocalSpace ;
	return	matLocal ;
}

// ローカル変換行列を設定
//////////////////////////////////////////////////////////////////////////////
void S3DCameraRelativeSpace::SetLocalTransformation( const S3DDMatrix& matLocal )
{
	m_matLocalSpace = matLocal ;
}

// ローカル座標を取得
//////////////////////////////////////////////////////////////////////////////
const S3DDVector&
	S3DCameraRelativeSpace::GetLocalSpacePosition( S3DDVector& vLocal ) const
{
	vLocal = m_vLocalSpace ;
	return	vLocal ;
}

// ローカル変換行列を設定
//////////////////////////////////////////////////////////////////////////////
void S3DCameraRelativeSpace::SetLocalSpacePosition( const S3DDVector& vLocal )
{
	m_vLocalSpace = vLocal ;
}

// タイマ処理
//////////////////////////////////////////////////////////////////////////////
void S3DCameraRelativeSpace::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	UpdateLocalTransformation() ;
	//
	SpaceSerializer::OnTimer( scene, msecPast ) ;
}



//////////////////////////////////////////////////////////////////////////////
// カメラ連動コントローラー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DCameraRelativeController, Controller )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DCameraRelativeController, rel_camera_controller )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DCameraRelativeController::S3DCameraRelativeController( void )
	: S3DCameraRelativeController( m_ItemClassDescriptor.pwszClassID )
{
}

S3DCameraRelativeController::S3DCameraRelativeController( const wchar_t * pwszClassID )
	: Controller( pwszClassID ),
		m_vBasePos( 0, 0, 0 ), m_vMoveUnit( 1, 1, 1 ),
		m_flagRelativeX( true ), m_flagRelativeY( false ), m_flagRelativeZ( true )
{
	ESLVerify( paramBasePosition == AddParameterEntry
		( L"position",
			S3DSceneComposer::typePosition,
			S3DSceneComposer::attrNoLocalTransform,
			L"基準座標" ) ) ;
	ESLVerify( paramMoveUnit == AddParameterEntry
		( L"move_unit",
			S3DSceneComposer::typePosition, 0,
			L"移動単位",
			L"カメラ座標に従って、基準座標 + 移動単位 * N（整数）と"
			L"なる座標がアイテムに適用されます。\n"
			L"0.0 の場合には 基準座標 + カメラ座標 が適用されます。" ) ) ;
	ESLVerify( paramRelativeX == AddParameterEntry
		( L"relative_x",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"ｘ座標追従" ) ) ;
	ESLVerify( paramRelativeY == AddParameterEntry
		( L"relative_y",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"ｙ座標追従" ) ) ;
	ESLVerify( paramRelativeZ == AddParameterEntry
		( L"relative_z",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"ｚ座標追従" ) ) ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DCameraRelativeController::GetVectorParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramBasePosition:
		return	m_vBasePos ;
	case	paramMoveUnit:
		return	m_vMoveUnit ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

bool S3DCameraRelativeController::GetBooleanParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramRelativeX:
		return	m_flagRelativeX ;
	case	paramRelativeY:
		return	m_flagRelativeY ;
	case	paramRelativeZ:
		return	m_flagRelativeZ ;
	}
	return	false ;
}

//////////////////////////////////////////////////////////////////////////////
// パラメータ値設定
void S3DCameraRelativeController::SetVectorParameter( size_t iParam, const S3DDVector& vec )
{
	switch ( iParam )
	{
	case	paramBasePosition:
		m_vBasePos = vec ;
		return ;
	case	paramMoveUnit:
		m_vMoveUnit = vec ;
		return ;
	}
}

void S3DCameraRelativeController::SetBooleanParameter( size_t iParam, bool b )
{
	switch ( iParam )
	{
	case	paramRelativeX:
		m_flagRelativeX = b ;
		return ;
	case	paramRelativeY:
		m_flagRelativeY = b ;
		return ;
	case	paramRelativeZ:
		m_flagRelativeZ = b ;
		return ;
	}
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DCameraRelativeController::OnTimer
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast )
{
	Controller::OnTimer( scene, pItem, msecPast ) ;

	S3DSceneComposer::CommonSerializer *
		pCmdItem = ESLTypeCast<S3DSceneComposer::CommonSerializer>( pItem ) ;
	S3DScene::Camera *	pCamera = scene.GetMainCamera() ;
	if ( (pCmdItem != nullptr)
		&& (pCamera != nullptr)
		&& (m_flagRelativeX || m_flagRelativeY || m_flagRelativeZ) )
	{
		S3DDMatrix	matCamera ;
		S3DDVector	vCamera ;
		pCamera->CalcGlobalTransformation( matCamera, vCamera ) ;
		//
		S3DDMatrix	matItem ;
		S3DDVector	vItem ;
		pItem->GetGlobalTransformation( matItem, vItem ) ;
		//
		S3DDVector	vGlobalPos = vItem ;
		if ( m_flagRelativeX )
		{
			vGlobalPos.x = m_vBasePos.x ;
			if ( fabs(m_vMoveUnit.x) > 1.0e-7 )
			{
				vGlobalPos.x +=
					floor( vCamera.x / m_vMoveUnit.x ) * m_vMoveUnit.x ;
			}
			else
			{
				vGlobalPos.x += vCamera.x ;
			}
		}
		if ( m_flagRelativeY )
		{
			vGlobalPos.y = m_vBasePos.y ;
			if ( fabs(m_vMoveUnit.y) > 1.0e-7 )
			{
				vGlobalPos.y +=
					floor( vCamera.y / m_vMoveUnit.y ) * m_vMoveUnit.y ;
			}
			else
			{
				vGlobalPos.y += vCamera.y ;
			}
		}
		if ( m_flagRelativeZ )
		{
			vGlobalPos.z = m_vBasePos.z ;
			if ( fabs(m_vMoveUnit.z) > 1.0e-7 )
			{
				vGlobalPos.z +=
					floor( vCamera.z / m_vMoveUnit.z ) * m_vMoveUnit.z ;
			}
			else
			{
				vGlobalPos.z += vCamera.z ;
			}
		}
		S3DDMatrix	matSpace ;
		S3DDVector	vSpace ;
		pItem->GetItemLinkTransformation( matSpace, vSpace ) ;
		pCmdItem->SetItemPositioin
			( matSpace.Inverse() * (vGlobalPos - vSpace) ) ;
	}
}




//////////////////////////////////////////////////////////////////////////////
// 動的カメラコントローラー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DDynamicCamera::CameraController, Controller )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DDynamicCamera::CameraController::CameraController( const wchar_t * pwszClassID )
	: Controller( pwszClassID )
{
}



//////////////////////////////////////////////////////////////////////////////
// 動的カメラアイテム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DDynamicCamera, CameraSerializer )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DDynamicCamera, dynamic_camera )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DDynamicCamera::S3DDynamicCamera( void )
	: m_vCameraPos( 0, 0, 0 ),
		m_vCameraTarget( 0, 0, 1 ), m_vCameraTop( 0, -1, 0 )
{
	m_flagsBehavior |= S3DScene::itemTimer | S3DScene::itemOwnerBehavior ;
	m_pwszClassID = m_ItemClassDescriptor.pwszClassID ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DDynamicCamera::~S3DDynamicCamera( void )
{
}

// 修正前カメラ座標
//////////////////////////////////////////////////////////////////////////////
const S3DDVector& S3DDynamicCamera::GetCameraPosition( void ) const
{
	return	m_vCameraPos ;
}

const S3DDVector& S3DDynamicCamera::GetCameraTarget( void ) const
{
	return	m_vCameraTarget ;
}

const S3DDVector& S3DDynamicCamera::GetCameraTop( void ) const
{
	return	m_vCameraTop ;
}

void S3DDynamicCamera::SetCameraPosition( const S3DDVector& vPos )
{
	m_vCameraPos = vPos ;
	SpaceParameter().m_vCenter = vPos ;
}

void S3DDynamicCamera::SetCameraTarget( const S3DDVector& vTarget )
{
	m_vCameraTarget = vTarget ;
	m_vTarget = vTarget ;
}

void S3DDynamicCamera::SetCameraTop( const S3DDVector& vTop )
{
	m_vCameraTop = vTop ;
	m_vTop = vTop ;
}

// アイテム作用の追加処理
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicCamera::OnUpdateBehavior( S3DScene& scene )
{
	m_space.m_vCenter = m_vCameraPos ;
	m_vTarget = m_vCameraTarget ;
	m_vTop = m_vCameraTop ;
	//
	S3DSceneComposer::Controller *const*
					ppControllers = m_arrControllers.GetConstArray() ;
	const size_t	nCount = m_arrControllers.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		CameraController *	pController =
				ESLTypeCast<CameraController>( ppControllers[i] ) ;
		if ( (pController != nullptr)
			&& !pController->IsControllerDisabled() )
		{
			pController->OnBeforeRender( scene, this ) ;
		}
	}
	CameraSerializer::OnUpdateBehavior( scene ) ;
}

// 視点修正
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicCamera::ModifyCameraPosture
	( const S3DDMatrix& matCameraRotation,
		const S3DDVector& vCameraOffset,
		bool fAngleLevelMatch, bool fOffsetLevelMatch )
{
	CalcModifiedCamera
		( m_space.m_vCenter, m_vTarget, m_vTop,
			matCameraRotation, vCameraOffset,
			fAngleLevelMatch, fOffsetLevelMatch ) ;
/*
	S3DDMatrix	matCamera( 1, 1, 1 ) ;
	matCamera.CameraAngleOf( m_vTarget, m_space.m_vCenter, m_vTop ) ;
	//
	S3DDMatrix	matICamera ;
	matICamera.InverseOf( matCamera ) ;
	//
	S3DDMatrix	matRotation = matICamera * matCameraRotation * matCamera ;
	S3DDMatrix	matOffset = matICamera ;
	if ( fAngleLevelMatch || fOffsetLevelMatch )
	{
		S3DDVector	vTarget = m_vTarget ;
		S3DDVector	vTop( 0, -1, 0 ) ;
		vTarget.y = m_space.m_vCenter.y ;
		//
		matCamera.CameraAngleOf( vTarget, m_space.m_vCenter, vTop ) ;
		matICamera.InverseOf( matCamera ) ;
		//
		if ( fAngleLevelMatch )
		{
			matRotation = matICamera * matCameraRotation * matCamera ;
			m_vTarget= vTarget ;
			m_vTop = vTop ;
		}
		if ( fOffsetLevelMatch )
		{
			matOffset = matICamera ;
		}
	}
	//
	S3DDVector	vViewAngle = m_vTarget - m_space.m_vCenter ;
	m_space.m_vCenter += matOffset * vCameraOffset ;
	m_vTarget = m_space.m_vCenter + matRotation * vViewAngle ;
	m_vTop = matRotation * m_vTop ;
*/
}

// 修正カメラ計算
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicCamera::CalcModifiedCamera
	( S3DDVector& vModifiedPos,
		S3DDVector& vModifiedTarget,
		S3DDVector& vModifiedTop,
		const S3DDMatrix& matCameraRotation,
		const S3DDVector& vCameraOffset,
		bool fAngleLevelMatch, bool fOffsetLevelMatch ) const
{
	S3DDMatrix	matCamera( 1, 1, 1 ) ;
	matCamera.CameraAngleOf( vModifiedTarget, vModifiedPos, vModifiedTop ) ;
	//
	S3DDMatrix	matICamera ;
	matICamera.InverseOf( matCamera ) ;
	//
	S3DDMatrix	matRotation = matICamera * matCameraRotation * matCamera ;
	S3DDMatrix	matOffset = matICamera ;
	if ( fAngleLevelMatch || fOffsetLevelMatch )
	{
		S3DDVector	vTarget = vModifiedTarget ;
		S3DDVector	vTop( 0, -1, 0 ) ;
		vTarget.y = vModifiedPos.y ;
		//
		matCamera.CameraAngleOf( vTarget, vModifiedPos, vTop ) ;
		matICamera.InverseOf( matCamera ) ;
		//
		if ( fAngleLevelMatch )
		{
			matRotation = matICamera * matCameraRotation * matCamera ;
			vModifiedTarget = vTarget ;
			vModifiedTop = vTop ;
		}
		if ( fOffsetLevelMatch )
		{
			matOffset = matICamera ;
		}
	}
	//
	S3DDVector	vViewAngle = vModifiedTarget - vModifiedPos ;
	vModifiedPos += matOffset * vCameraOffset ;
	vModifiedTarget = vModifiedPos + matRotation * vViewAngle ;
	vModifiedTop = matRotation * vModifiedTop ;
}

// カメラ座標修正空間行列計算
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DDynamicCamera::CalcModifiedCameraRotation
	( S3DDMatrix& matCameraRotation,
		const S3DDVector& vCameraPos,
		const S3DDVector& vCameraTarget,
		const S3DDVector& vCameraTop,
		bool fAngleLevelMatch, bool fOffsetLevelMatch )
{
	S3DDMatrix	matCamera( 1, 1, 1 ) ;
	matCamera.CameraAngleOf( vCameraTarget, vCameraPos, vCameraTop ) ;
	//
	S3DDMatrix	matICamera ;
	matICamera.InverseOf( matCamera ) ;
	//
	S3DDMatrix	matRotation = matICamera * matCameraRotation * matCamera ;
	S3DDMatrix	matOffset = matICamera ;
	if ( fAngleLevelMatch || fOffsetLevelMatch )
	{
		S3DDVector	vTarget = vCameraTarget ;
		S3DDVector	vTop( 0, -1, 0 ) ;
		vTarget.y = vCameraPos.y ;
		//
		matCamera.CameraAngleOf( vTarget, vCameraPos, vTop ) ;
		matICamera.InverseOf( matCamera ) ;
		//
		if ( fAngleLevelMatch )
		{
			matRotation = matICamera * matCameraRotation * matCamera ;
		}
		if ( fOffsetLevelMatch )
		{
			matOffset = matICamera ;
		}
	}
	matCameraRotation = matRotation ;
	return	matOffset ;
}



//////////////////////////////////////////////////////////////////////////////
// 画面揺れ効果
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DCameraWiggleController, CameraController )
S3D_IMPLEMENT_COMPOSER_ITEM( S3DCameraWiggleController, camera_wiggle )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DCameraWiggleController::S3DCameraWiggleController( void )
	: CameraController( m_ItemClassDescriptor.pwszClassID ),
		m_vAmplitude( 0, 0 ),
			m_vFrequency( 10, 10 ), m_radPhase( 0, 0 )
{
	m_flagsBehavior |= S3DSceneComposer::behaviorOnTimer ;
	//
	m_random.InitializeSeed() ;
	//
	AddParameterEntry
		( L"amplitude_x", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"水平振幅", L"水平方向の振幅[deg]", 0.0, 10.0 ) ;
	AddParameterEntry
		( L"frequency_x", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"水平振動数", L"水平方向の振動周波数[Hz]", 0.0, 100.0 ) ;
	AddParameterEntry
		( L"amplitude_y", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"垂直振幅", L"垂直方向の振幅[deg]", 0.0, 10.0 ) ;
	AddParameterEntry
		( L"frequency_y", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"垂直振動数", L"垂直方向の振動周波数[Hz]", 0.0, 100.0 ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DCameraWiggleController::~S3DCameraWiggleController( void )
{
}

// 振動追加
//////////////////////////////////////////////////////////////////////////////
void S3DCameraWiggleController::AddWiggle
		( const S3DCameraWiggleController::WiggleParam& wp )
{
	Lock() ;
	m_wiggles.Add( new Wiggle( wp ) ) ;
	Unlock() ;
}

// 全ての振動を削除
//////////////////////////////////////////////////////////////////////////////
void S3DCameraWiggleController::RemoveAllWiggles( void )
{
	Lock() ;
	m_wiggles.RemoveAll() ;
	Unlock() ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
double S3DCameraWiggleController::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramAmplitudeX:
		return	m_vAmplitude.x ;
	case	paramFrequencyX:
		return	m_vFrequency.x ;
	case	paramAmplitudeY:
		return	m_vAmplitude.y ;
	case	paramFrequencyY:
		return	m_vFrequency.y ;
	}
	return	0 ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DCameraWiggleController::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramAmplitudeX:
		m_vAmplitude.x = s ;
		return ;
	case	paramFrequencyX:
		m_vFrequency.x = s ;
		return ;
	case	paramAmplitudeY:
		m_vAmplitude.y = s ;
		return ;
	case	paramFrequencyY:
		m_vFrequency.y = s ;
		return ;
	}
}

// フレーム描画前処理（全視点・ビュー共通処理）
//////////////////////////////////////////////////////////////////////////////
void S3DCameraWiggleController::OnBeforeRender
	( S3DScene& scene, S3DDynamicCamera * pItem )
{
	S2DVector	vAmpAcc( 0, 0 ) ;
	for ( size_t i = 0; i < m_wiggles.GetLength(); i ++ )
	{
		Wiggle *	pWiggle = m_wiggles.GetAt( i ) ;
		if ( pWiggle != nullptr )
		{
			if ( pWiggle->m_nFlags & flagFreqByRenderFrame )
			{
				pWiggle->m_radPhase += pWiggle->m_vFrequency * (float32_t) PI ;
			}
			S2DVector	vAmp =
				pWiggle->m_degAmplitude
					* (float32_t) (1.0 - (pWiggle->m_secTime / pWiggle->m_secFadeout)) ;
			if ( pWiggle->m_nFlags & flagHorizontalRandom )
			{
				vAmpAcc.x += vAmp.x
					* (float32_t) m_random.QuickRandomDouble( 1.0 ) ;
			}
			else
			{
				vAmpAcc.x += vAmp.x * (float32_t) cos( pWiggle->m_radPhase.x ) ;
			}
			if ( pWiggle->m_nFlags & flagVerticalRandom )
			{
				vAmpAcc.y += vAmp.y
					* (float32_t) m_random.QuickRandomDouble( 1.0 ) ;
			}
			else
			{
				vAmpAcc.y += vAmp.y * (float32_t) cos( pWiggle->m_radPhase.y ) ;
			}
		}
	}
	//
	vAmpAcc.x += (float32_t) (m_vAmplitude.x * cos( m_radPhase.x )) ;
	vAmpAcc.y += (float32_t) (m_vAmplitude.y * cos( m_radPhase.y )) ;
	//
	if ( (fabs(vAmpAcc.x) > 0.001) || (fabs(vAmpAcc.y) > 0.001) )
	{
		S3DDVector	vTargetDelta = pItem->m_vTarget - pItem->m_space.m_vCenter ;
		S3DDVector	vCameraTop = pItem->m_vTop ;
		S3DDVector	vCross = vTargetDelta * vCameraTop ;
		S3DDMatrix	matWiggle( 1, 1, 1 ) ;
		S3DDMatrix	matX( 1, 1, 1 ) ;
		S3DDMatrix	matY( 1, 1, 1 ) ;
		double	rad ;
		rad = vAmpAcc.y * PI / 180.0 ;
		matY.RotationOnVectorOf( vCross, sin(rad), cos(rad) ) ;
		rad = vAmpAcc.x * PI / 180.0 ;
		matX.RotationOnVectorOf( vCameraTop, sin(rad), cos(rad) ) ;
		matWiggle = matY * matX ;
		//
		pItem->m_vTarget = pItem->m_space.m_vCenter + matWiggle * vTargetDelta ;
		pItem->m_vTop = matWiggle * vCameraTop ;
	}
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DCameraWiggleController::OnTimer
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast )
{
	bool	fUpdate = false ;
	double	secPast = (double) msecPast * 0.001 ;
	for ( size_t i = 0; i < m_wiggles.GetLength(); i ++ )
	{
		Wiggle *	pWiggle = m_wiggles.GetAt( i ) ;
		if ( pWiggle != nullptr )
		{
			pWiggle->m_secTime += secPast ;
			if ( pWiggle->m_secTime < pWiggle->m_secFadeout )
			{
				if ( !(pWiggle->m_nFlags & flagFreqByRenderFrame) )
				{
					pWiggle->m_radPhase +=
						pWiggle->m_vFrequency * (float32_t) (secPast * 2.0 * PI) ;
				}
			}
			else
			{
				m_wiggles.SetAt( i, nullptr ) ;
			}
			fUpdate = true ;
		}
	}
	m_wiggles.TrimEmpty() ;
	//
	m_radPhase += m_vFrequency * (secPast * 2.0 * PI) ;
	//
	if ( fUpdate )
	{
		scene.PostSceneUpdate() ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// カメラ・オフセット制御
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DCameraOffsetController, CameraController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DCameraOffsetController, camera_offset )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DCameraOffsetController::S3DCameraOffsetController( void )
	: CameraController( m_ItemClassDescriptor.pwszClassID ),
		m_nCtrlFlags( 0 ),
		m_matRotate( 1, 0, 0,  0, 1, 0,  0, 0, 1 ), m_vOffset( 0, 0, 0 )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DCameraOffsetController::~S3DCameraOffsetController( void )
{
}

// 制御フラグ
//////////////////////////////////////////////////////////////////////////////
void S3DCameraOffsetController::SetControlFlags( uint32_t nFlags )
{
	m_nCtrlFlags = nFlags ;
}

// オフセット座標
//////////////////////////////////////////////////////////////////////////////
void S3DCameraOffsetController::SetOffset( const S3DDVector& vOffset )
{
	m_vOffset = vOffset ;
}

// 回転
//////////////////////////////////////////////////////////////////////////////
void S3DCameraOffsetController::SetRotation( const S3DDMatrix& matRotate )
{
	m_matRotate = matRotate ;
}

// オフセット後カメラ座標計算
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DCameraOffsetController::CalcOffsetCameraPos
	( const S3DDynamicCamera& camera ) const
{
	return	CalcOffsetCameraPos( camera, m_vOffset ) ;
}

S3DDVector S3DCameraOffsetController::CalcOffsetCameraPos
	( const S3DDynamicCamera& camera, const S3DDVector& vOffset ) const
{
	S3DDVector	vModifiedPos = camera.GetCameraPosition() ;
	S3DDVector	vModifiedTarget = camera.GetCameraTarget() ;
	S3DDVector	vModifiedTop = camera.GetCameraTop() ;
	camera.CalcModifiedCamera
		( vModifiedPos, vModifiedTarget, vModifiedTop,
			m_matRotate, vOffset, false,
			(m_nCtrlFlags & flagOffsetLevelMatching) != 0 ) ;
	return	vModifiedPos ;
}

// オフセット座標計算
//////////////////////////////////////////////////////////////////////////////
S3DVector S3DCameraOffsetController::CalcOffsetForCameraPos
	( const S3DDynamicCamera& camera, const S3DDVector& vCameraPos ) const
{
	S3DDMatrix	matRotate = m_matRotate ;
	S3DDMatrix	matOffset =
		camera.CalcModifiedCameraRotation
			( matRotate,
				camera.GetCameraPosition(),
				camera.GetCameraTarget(),
				camera.GetCameraTop(),
				false, (m_nCtrlFlags & flagOffsetLevelMatching) != 0 ) ;
	return	matOffset.Inverse() * (vCameraPos - camera.GetCameraPosition()) ;
}

// フレーム描画前処理（全視点・ビュー共通処理）
//////////////////////////////////////////////////////////////////////////////
void S3DCameraOffsetController::OnBeforeRender
	( S3DScene& scene, S3DDynamicCamera * pItem )
{
	pItem->ModifyCameraPosture
		( m_matRotate, m_vOffset, false,
			(m_nCtrlFlags & flagOffsetLevelMatching) != 0 ) ;
}



//////////////////////////////////////////////////////////////////////////
// カメラ連動通知コントローラー
//////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DCameraRelationController, CameraController )
S3D_IMPLEMENT_COMPOSER_ITEM
	( SakuraGL::S3DCameraRelationController, camera_passive_relation )

// 構築関数
//////////////////////////////////////////////////////////////////////////
S3DCameraRelationController::S3DCameraRelationController
							( S3DCameraRelativeSpace * pRelSpace )
	: CameraController( m_ItemClassDescriptor.pwszClassID )
{
	m_iParamRelCameraSpace =
		AddParameterEntry
			( L"rel_camera_space",
				S3DSceneComposer::typeSelector,
				S3DSceneComposer::attrStringEnumeration,
				L"連動空間" ) ;
}

// S3DCameraRelativeSpace 設定
//////////////////////////////////////////////////////////////////////////
void S3DCameraRelationController::AttachCameraRelativeSpace
	( S3DCameraRelativeSpace * pcrs, const wchar_t * pwszSpaceID )
{
	m_refRelSpace.SetReference( (S3DScene::Space*) pcrs ) ;
	m_strRelSpaceID = pwszSpaceID ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////
const wchar_t * S3DCameraRelationController::GetCommandParameter( size_t i ) const
{
	if ( i == m_iParamRelCameraSpace )
	{
		return	m_strRelSpaceID ;
	}
	return	CameraController::GetCommandParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////
void S3DCameraRelationController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	if ( i == m_iParamRelCameraSpace )
	{
		S3DSceneComposer::Composition *	pComp = GetComposition() ;
		if ( pComp != nullptr )
		{
			S3DCameraRelativeSpace *	pcrs =
				ESLTypeCast<S3DCameraRelativeSpace>
					( pComp->GetSceneItemAs( pwszCmd ) ) ;
			AttachCameraRelativeSpace( pcrs, pwszCmd ) ;
		}
		else
		{
			AttachCameraRelativeSpace( nullptr, pwszCmd ) ;
		}
		return ;
	}
	CameraController::SetCommandParameter( i, pwszCmd ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////
bool S3DCameraRelationController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	if ( i == m_iParamRelCameraSpace )
	{
		S3DSceneComposer::Composition *	pComp = GetComposition() ;
		if ( pComp != nullptr )
		{
			pComp->EnumerateItemIDsAs
				( aStrSet, ESL_RUNTIME_CLASS(S3DCameraRelativeSpace) ) ;
		}
		return	true ;
	}
	return	CameraController::EnumerateStringSet( i, aStrSet ) ;
}

// フレーム描画前処理（全視点・ビュー共通処理）
//////////////////////////////////////////////////////////////////////////
void S3DCameraRelationController::OnBeforeRender
			( S3DScene& scene, S3DDynamicCamera * pItem )
{
	S3DCameraRelativeSpace *	pcrs =
			ESLTypeCast<S3DCameraRelativeSpace>
				( m_refRelSpace.GetReference() ) ;
	if ( pcrs != nullptr )
	{
		pcrs->UpdateLocalTransformation() ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// 参照空間コントローラー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DSpaceReferenceController, Controller )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DSpaceReferenceController, space_referencer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSpaceReferenceController::S3DSpaceReferenceController( void )
	: Controller( m_ItemClassDescriptor.pwszClassID )
{
	m_iParamRefSpace =
		AddParameterEntry
			( L"ref_space",
				S3DSceneComposer::typeSelector,
				S3DSceneComposer::attrStringEnumeration,
				L"参照空間", nullptr ) ;
	m_iParamTargetBone =
		AddParameterEntry
			( L"target_bone",
				S3DSceneComposer::typeSelector,
				S3DSceneComposer::attrConstant
				| S3DSceneComposer::attrStringEnumeration,
				L"対象ボーン", nullptr ) ;
}

// 参照空間設定
//////////////////////////////////////////////////////////////////////////////
void S3DSpaceReferenceController::AttachReferenceSpace
		( S3DScene::Space * pSpace, const wchar_t * pwszSpaceID )
{
	m_strRefSpaceID = pwszSpaceID ;
	//
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		S3DScene::Space *	pTargetSpace =
			ESLTypeCast<S3DScene::Space>( m_refTargetItem.GetReference() ) ;
		if ( pTargetSpace != nullptr )
		{
			pTargetSpace->SwitchReferenceSpace( pSpace ) ;
		}
		else
		{
			S3DScene::Item *	pTargetItem =
				ESLTypeCast<S3DScene::Item>( m_refTargetItem.GetReference() ) ;
			if ( pTargetItem != nullptr )
			{
				pTargetItem->SwitchReferenceSpace( pSpace ) ;
			}
			else
			{
				SwitchReferenceSpace( *pComp ) ;
			}
		}
	}
}

// ターゲット空間設定
//////////////////////////////////////////////////////////////////////////////
void S3DSpaceReferenceController::AttachTargetSpace
		( S3DScene::Space * pSpace, const wchar_t * pwszTargetBoneID )
{
	m_strTargetBoneID = pwszTargetBoneID ;
	//
	if ( m_refTargetItem.GetReference() != pSpace )
	{
		S3DScene::Item *	pLastItem =
			ESLTypeCast<S3DScene::Item>
				( m_refTargetItem.GetReference() ) ;
		if ( pLastItem != nullptr )
		{
			pLastItem->SwitchReferenceSpace( nullptr ) ;
		}
		else
		{
			S3DScene::Space *	pLastSpace =
				ESLTypeCast<S3DScene::Space>
					( m_refTargetItem.GetReference() ) ;
			if ( pLastSpace != nullptr )
			{
				pLastSpace->SwitchReferenceSpace( nullptr ) ;
			}
		}
		m_refTargetItem.SetReference( pSpace ) ;
		//
		S3DSceneComposer::Composition *	pComp = GetComposition() ;
		if ( pComp != nullptr )
		{
			SwitchReferenceSpace( *pComp ) ;
		}
	}
}

// 参照空間切り替え
//////////////////////////////////////////////////////////////////////////////
void S3DSpaceReferenceController::SwitchReferenceSpace
						( S3DSceneComposer::Composition& comp )
{
	if ( m_refTargetItem.GetReference() == nullptr )
	{
		S3DSceneComposer::ItemSerializer *	pItem = GetOwnerItem() ;
		if ( pItem != nullptr )
		{
			if ( m_strTargetBoneID.IsEmpty() )
			{
				S3DSceneComposer::SpaceSerializer *	pSpaceSrlz =
						ESLTypeCast<S3DSceneComposer::SpaceSerializer>( pItem ) ;
				if ( pSpaceSrlz != nullptr )
				{
					m_refTargetItem.SetReference
							( pSpaceSrlz->GetSceneSpace() ) ;
				}
				else
				{
					S3DSceneComposer::ItemCommonSerializer *	pItemSrlz =
						ESLTypeCast<S3DSceneComposer::ItemCommonSerializer>( pItem ) ;
					if ( pItemSrlz != nullptr )
					{
						m_refTargetItem.SetReference
								( pItemSrlz->GetSceneItem() ) ;
					}
				}
			}
			else
			{
				S3DSceneComposer::ItemCommonSerializer *	pItemSrlz =
					ESLTypeCast<S3DSceneComposer::ItemCommonSerializer>( pItem ) ;
				if ( pItemSrlz != nullptr )
				{
					S3DScene::ModelItem *	pModelItem =
							ESLTypeCast<S3DScene::ModelItem>
									( pItemSrlz->GetSceneItem() ) ;
					if ( pModelItem != nullptr )
					{
						S3DModelBuffer *	pModel =
							ESLTypeCast<S3DModelBuffer>
									( pModelItem->GetModel() ) ;
						if ( pModel != nullptr )
						{
							m_refTargetItem.SetReference
								( pModel->GetBonePropertyAs( m_strTargetBoneID ) ) ;
						}
					}
				}
			}
		}
	}
	S3DScene::Space *	pRefSpace = comp.GetSceneSpaceAs( m_strRefSpaceID ) ;
	S3DScene::Space *	pTargetSpace =
		ESLTypeCast<S3DScene::Space>( m_refTargetItem.GetReference() ) ;
	if ( pTargetSpace != nullptr )
	{
		pTargetSpace->SwitchReferenceSpace( pRefSpace ) ;
	}
	else
	{
		S3DScene::Item *	pTargetItem =
			ESLTypeCast<S3DScene::Item>( m_refTargetItem.GetReference() ) ;
		if ( pTargetItem != nullptr )
		{
			pTargetItem->SwitchReferenceSpace( pRefSpace ) ;
		}
	}
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSpaceReferenceController::GetCommandParameter( size_t i ) const
{
	if ( i == m_iParamRefSpace )
	{
		return	m_strRefSpaceID ;
	}
	else if ( i == m_iParamTargetBone )
	{
		return	m_strTargetBoneID ;
	}
	return	nullptr ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DSpaceReferenceController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	if ( i == m_iParamRefSpace )
	{
		S3DSceneComposer::Composition *	pComp = GetComposition() ;
		if ( pComp != nullptr )
		{
			pComp->EnumerateItemIDsAs
				( aStrSet,  ESL_RUNTIME_CLASS
								(S3DSceneComposer::SpaceSerializer) ) ;
			//
			SPointerArray<S3DSceneComposer::ItemSerializer>	aItems ;
			pComp->EnumerateItemsAs
				( aItems, ESL_RUNTIME_CLASS(S3DDynamicModelSerializer) ) ;
			for ( size_t j = 0; j < aItems.GetLength(); j ++ )
			{
				S3DDynamicModelSerializer *	pdms =
					ESLTypeCast<S3DDynamicModelSerializer>( aItems.GetAt( j ) ) ;
				if ( pdms == nullptr )
				{
					continue ;
				}
				S3DModelBuffer *	pModel = pdms->GetModelAlias() ;
				if ( pModel == nullptr )
				{
					continue ;
				}
				SString	strBaseID = pComp->GetSceneItemIDOf( pdms ) ;
				strBaseID += L"@" ;
				//
				for ( size_t k = 0; k < pModel->GetBonePropertyList().GetLength(); k ++ )
				{
					const SString *	pstrID =
							pModel->GetBonePropertyList().GetTagAt( k ) ;
					if ( pstrID != nullptr )
					{
						aStrSet.Add( new SString( strBaseID + *pstrID ) ) ;
					}
				}
			}
			return	true ;
		}
	}
	else if ( i == m_iParamTargetBone )
	{
		S3DDynamicModelSerializer *	pdms =
			ESLTypeCast<S3DDynamicModelSerializer>( GetOwnerItem() ) ;
		if ( pdms != nullptr )
		{
			S3DModelBuffer *	pModel = pdms->GetModelAlias() ;
			if ( pModel == nullptr )
			{
				return	false ;
			}
			for ( size_t j = 0; j < pModel->GetBonePropertyList().GetLength(); j ++ )
			{
				const SString *	pstrID =
						pModel->GetBonePropertyList().GetTagAt( j ) ;
				if ( pstrID != nullptr )
				{
					aStrSet.Add( new SString( *pstrID ) ) ;
				}
			}
			return	true ;
		}
	}
	return	false ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSpaceReferenceController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	if ( i == m_iParamRefSpace )
	{
		if ( m_strRefSpaceID != pwszCmd )
		{
			m_strRefSpaceID = pwszCmd ;
			//
			S3DSceneComposer::Composition *	pComp = GetComposition() ;
			if ( pComp != nullptr )
			{
				SwitchReferenceSpace( *pComp ) ;
			}
		}
	}
	else if ( i == m_iParamTargetBone )
	{
		if ( m_strTargetBoneID != pwszCmd )
		{
			S3DScene::Item *	pLastItem =
				ESLTypeCast<S3DScene::Item>
					( m_refTargetItem.GetReference() ) ;
			if ( pLastItem != nullptr )
			{
				pLastItem->SwitchReferenceSpace( nullptr ) ;
			}
			else
			{
				S3DScene::Space *	pLastSpace =
					ESLTypeCast<S3DScene::Space>
						( m_refTargetItem.GetReference() ) ;
				if ( pLastSpace != nullptr )
				{
					pLastSpace->SwitchReferenceSpace( nullptr ) ;
				}
			}
			m_refTargetItem.SetReference( nullptr ) ;
			//
			m_strTargetBoneID = pwszCmd ;
			//
			S3DSceneComposer::Composition *	pComp = GetComposition() ;
			if ( pComp != nullptr )
			{
				SwitchReferenceSpace( *pComp ) ;
			}
		}
	}
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DSpaceReferenceController::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags )
{
	uint32_t	nResFlags =
		Controller::UpdatePropertyReference( comp, pItem, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefItem )
	{
		S3DSceneComposer::Composition *	pComp = GetComposition() ;
		if ( pComp != nullptr )
		{
			SwitchReferenceSpace( *pComp ) ;
		}
	}
	return	nResFlags ;
}

// フレーム（パラメータ）更新後処理
//////////////////////////////////////////////////////////////////////////////
void S3DSpaceReferenceController::OnUpdateFrame
	( S3DSceneComposer::ItemSerializer * pItem,
		double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	Controller::OnUpdateFrame( pItem, fpFrame, seek ) ;
	//
	if ( seek == S3DSceneComposer::seekJumpReset )
	{
		S3DScene::Item *	pItem =
			ESLTypeCast<S3DScene::Item>
				( m_refTargetItem.GetReference() ) ;
		if ( pItem != nullptr )
		{
			pItem->SetReferenceSpace( pItem->GetReferenceSpace() ) ;
		}
		else
		{
			S3DScene::Space *	pSpace =
				ESLTypeCast<S3DScene::Space>
					( m_refTargetItem.GetReference() ) ;
			if ( pSpace != nullptr )
			{
				pSpace->SetReferenceSpace( pSpace->GetReferenceSpace() ) ;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
// ポーズコントローラー（抽象）
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DDynamicModelSerializer::PoseInterface, ESLObject )
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DDynamicModelSerializer::PoseConntoller, Controller, PoseInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DDynamicModelSerializer::PoseConntoller::PoseConntoller( const wchar_t * pwszClassID )
	: Controller( pwszClassID )
{
}


//////////////////////////////////////////////////////////////////////////////
// ポーズトラック
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DDynamicModelSerializer::PoseTrack, PoseConntoller )
S3D_IMPLEMENT_COMPOSER_ITEM
	( SakuraGL::S3DDynamicModelSerializer::PoseTrack, pose_track )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DDynamicModelSerializer::PoseTrack::PoseTrack( void )
	: PoseConntoller( S3DDynamicModelSerializer::PoseTrack::m_ItemClassDescriptor.pwszClassID )
{
	ESLVerify( paramPose ==
		AddParameterEntry
			( L"pose", S3DSceneComposer::typePose,
				S3DSceneComposer::attrStringEnumeration, L"ポーズ" ) ) ;
	ESLVerify( paramBlend ==
		AddParameterEntry
			( L"blend_weight",
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrUIScalarSlider,
				L"ブレンド", nullptr, 0.0, 1.0 ) ) ;
	//
	m_fpPoseBlend = 1.0 ;
}

// ポーズ適用
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelSerializer::PoseTrack::OnPoseTrack
	( S3DDynamicModelSerializer& item, S3DModelBuffer& model )
{
	S3DSceneComposer::PoseInstance	pose = m_poseCurrent ;
	if ( pose.pPoseBase != nullptr )
	{
		double	w = m_fpPoseBlend ;
		//
		pose.pPoseBase->ApplyPoseTo( model, w, pose.secBasePose ) ;
		//
		if ( pose.pPoseTarget != nullptr )
		{
			pose.pPoseTarget->ApplyPoseTo
					( model, pose.fpTransition * w, pose.secTargetPose ) ;
		}
	}
}

// ポーズ
//////////////////////////////////////////////////////////////////////////////
const S3DSceneComposer::PoseInstance&
		S3DDynamicModelSerializer::PoseTrack::GetCurrentPose( void ) const
{
	return	m_poseCurrent ;
}

void S3DDynamicModelSerializer::PoseTrack::
			SetCurrentPose( const S3DSceneComposer::PoseInstance& pose )
{
	m_poseCurrent = pose ;
	m_strPoseTemp.FreeArray() ;
}

// 適用度
//////////////////////////////////////////////////////////////////////////////
double S3DDynamicModelSerializer::PoseTrack::GetCurrentBlend( void ) const
{
	return	m_fpPoseBlend ;
}

void S3DDynamicModelSerializer::PoseTrack::SetCurrentBlend( double fpBlend )
{
	m_fpPoseBlend = fpBlend ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
double S3DDynamicModelSerializer::PoseTrack::GetScalarParameter( size_t i ) const
{
	if ( i == paramBlend )
	{
		return	GetCurrentBlend() ;
	}
	return	PoseConntoller::GetScalarParameter( i ) ;
}

const wchar_t * S3DDynamicModelSerializer::PoseTrack::GetCommandParameter( size_t i ) const
{
	if ( i == paramPose )
	{
		if ( !m_strPoseTemp.IsEmpty() )
		{
			return	m_strPoseTemp ;
		}
		ItemSerializer *	pOwner = GetOwnerItem() ;
		if ( pOwner != nullptr )
		{
			((PoseTrack*)this)->m_strPoseTemp =
								pOwner->FormatPoseString( m_poseCurrent ) ;
			return	m_strPoseTemp ;
		}
		else
		{
			return	nullptr ;
		}
	}
	return	PoseConntoller::GetCommandParameter( i ) ;
}

S3DSceneComposer::PoseInstance
	S3DDynamicModelSerializer::PoseTrack::GetPoseParameter( size_t i ) const
{
	if ( i == paramPose )
	{
		return	m_poseCurrent ;
	}
	return	PoseConntoller::GetPoseParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelSerializer::PoseTrack::SetScalarParameter( size_t i, double s )
{
	if ( i == paramBlend )
	{
		SetCurrentBlend( s ) ;
		return ;
	}
	PoseConntoller::SetScalarParameter( i, s ) ;
}

void S3DDynamicModelSerializer::PoseTrack::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	if ( i == paramPose )
	{
		ParsePoseString( m_poseCurrent, pwszCmd ) ;
		m_strPoseTemp = pwszCmd ;
		//
//		ItemSerializer *	pOwner = GetOwnerItem() ;
//		if ( pOwner != nullptr )
		{
//			pOwner->ParsePoseString( m_poseCurrent, pwszCmd ) ;
		}
		return ;
	}
	PoseConntoller::SetCommandParameter( i, pwszCmd ) ;
}

void S3DDynamicModelSerializer::PoseTrack::SetPoseParameter
	( size_t i, const S3DSceneComposer::PoseInstance& pose )
{
	if ( i == paramPose )
	{
		SetCurrentPose( pose ) ;
		return ;
	}
	PoseConntoller::SetPoseParameter( i, pose ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DDynamicModelSerializer::PoseTrack::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	if ( paramPose == i )
	{
		S3DDynamicModelSerializer * pdms =
			ESLTypeCast<S3DDynamicModelSerializer>( GetOwnerItem() ) ;
		if ( pdms != nullptr )
		{
			pdms->EnumeratePoseStringSet( aStrSet ) ;
		}
		return	true ;
	}
	return	Controller::EnumerateStringSet( i, aStrSet ) ;
}

// モデル変更時の処理
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelSerializer::PoseTrack::OnChangedModel
	( S3DDynamicModelSerializer& item, S3DModelBuffer * pModel )
{
	S3DSceneComposer::PoseSequencer * pPoseSeq =
			ESLTypeCast<S3DSceneComposer::PoseSequencer>
						( GetParameterSequencer( paramPose ) ) ;
	S3DModelPoseLibrary *	pPoseLib = nullptr ;
	if ( pPoseSeq != nullptr )
	{
		if ( pModel != nullptr )
		{
			pPoseLib = &(pModel->GetPoseLibrary()) ;
		}
		else
		{
			S3DSceneComposer::Composition *	pComp = item.GetComposition() ;
			if ( pComp != nullptr )
			{
				S3DSceneComposer *	pScene = pComp->GetSceneComposer() ;
				if ( pScene != nullptr )
				{
					pPoseLib = &(pScene->Assets().PoseLibrary()) ;
				}
			}
		}
		pPoseSeq->SetReferenceLibrary( pPoseLib ) ;
	}
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DDynamicModelSerializer::PoseTrack::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags )
{
	uint32_t	nResFlags =
		PoseConntoller::UpdatePropertyReference( comp, pItem, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefResource )
	{
		if ( (m_poseCurrent.pPoseBase != nullptr) || !m_strPoseTemp.IsEmpty() )
		{
			if ( m_strPoseTemp.IsEmpty() )
			{
				m_strPoseTemp = FormatPoseString( m_poseCurrent ) ;
			}
			ParsePoseString( m_poseCurrent, m_strPoseTemp ) ;
		}
	}
	return	nResFlags ;
}



//////////////////////////////////////////////////////////////////////////////
// IK ボーントラック
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DDynamicModelSerializer::IKBoneTrack, PoseConntoller )
S3D_IMPLEMENT_COMPOSER_ITEM
	( SakuraGL::S3DDynamicModelSerializer::IKBoneTrack, bone_ik )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DDynamicModelSerializer::IKBoneTrack::IKBoneTrack( void )
	: PoseConntoller
		( S3DDynamicModelSerializer::IKBoneTrack::m_ItemClassDescriptor.pwszClassID ),
		m_vIKPosition( 0, 0, 0 ),
		m_matIKRotate( 1, 1, 1 ),
		m_flagHandleTip( false ),
		m_vTipOffset( 0, 0, 0 ),
		m_fpGimbalWeight( 0.25 ),
		m_fpIKWeight( 1.0 ),
		m_fpTipBoneWeight( 1.0 ),
		m_nEffectJoints( 1 ),
		m_fpBoneStability( 0.5 )
{
	ESLVerify( paramBone ==
		AddParameterEntry
			( L"bone_id", S3DSceneComposer::typeSelector,
				S3DSceneComposer::attrStringEnumeration, L"ボーン" ) ) ;
	ESLVerify( paramPosition ==
		AddParameterEntry
			( L"position", S3DSceneComposer::typePosition,
				S3DSceneComposer::attrNoLocalTransform, L"位置" ) ) ;
	ESLVerify( paramRotation ==
		AddParameterEntry
			( L"rotation", S3DSceneComposer::typeRotation,
				S3DSceneComposer::attrNoLocalTransform, L"回転" ) ) ;
	ESLVerify( paramHandleTip ==
		AddParameterEntry
			( L"handle_tip", S3DSceneComposer::typeBoolean, 0,
				L"ボーン頂点に合わせる", L"IK位置をボーンハンドルの頂点に合わせる" ) ) ;
	ESLVerify( paramTipOffset ==
		AddParameterEntry
			( L"tip_offset", S3DSceneComposer::typePosition,
				S3DSceneComposer::attrNoLocalTransform,
				L"頂点オフセット", L"IK位置を合わせる座標のオフセット（ボーンローカル座標）" ) ) ;
	ESLVerify( paramGimbalWeight ==
		AddParameterEntry
			( L"gimbal_weight",
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrUIScalarSlider,
				L"軸回転影響度", nullptr, 0.0, 1.0 ) ) ;
	ESLVerify( paramIKWeight ==
		AddParameterEntry
			( L"ik_weight",
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrUIScalarSlider,
				L"IK適用度", nullptr, 0.0, 1.0 ) ) ;
	ESLVerify( paramTipBoneWeight ==
		AddParameterEntry
			( L"tip_bone_weight",
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrUIScalarSlider,
				L"先端ボーン回転適用度", nullptr, 0.0, 1.0 ) ) ;
	ESLVerify( paramEffectJoints ==
		AddParameterEntry
			( L"effect_joints",
				S3DSceneComposer::typeInteger,
				S3DSceneComposer::attrConstant,
				L"影響ジョイント数" ) ) ;
	ESLVerify( paramStability ==
		AddParameterEntry
			( L"bone_stability",
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrUIScalarSlider,
				L"ボーン復元力",
				L"フレーム毎の対象ボーンを基準ボーズへの復元率。\n"
				L"0.0 の時には復元しない。", 0.0, 1.0 ) ) ;
	ESLVerify( paramRefPose ==
		AddParameterEntry
			( L"ref_pose", S3DSceneComposer::typePose,
				S3DSceneComposer::attrStringEnumeration,
				L"基準ポーズ", L"ボーンを復元する際の基準ボーズ" ) ) ;
}

// ポーズ
//////////////////////////////////////////////////////////////////////////////
const S3DSceneComposer::PoseInstance&
	S3DDynamicModelSerializer::IKBoneTrack::GetRefPose( void ) const
{
	return	m_poseRef ;
}

void S3DDynamicModelSerializer::IKBoneTrack::SetRefPose( const S3DSceneComposer::PoseInstance& pose )
{
	m_poseRef = pose ;
	m_strPoseTemp.FreeArray() ;
}

// ポーズ適用
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelSerializer::IKBoneTrack::OnPoseTrack
	( S3DDynamicModelSerializer& item, S3DModelBuffer& model )
{
	if ( m_fpIKWeight <= 1.0e-7 )
	{
		return ;
	}
	S3DModelBoneSpace *	pBone = model.GetBonePropertyAs( m_strBone ) ;
	if ( pBone == nullptr )
	{
		return ;
	}
	if ( m_fpBoneStability > 0.001 )
	{
		// ボーンを復元
		S3DModelBoneSpace *	pParent = pBone->GetParentBone() ;
		for ( size_t i = 0; i <= (size_t) m_nEffectJoints; i ++ )
		{
			if ( pParent == nullptr )
			{
				break ;
			}
			S3DDMatrix		matRotBone ;
			S3DDQuaternion	qRotBone = pParent->GetLocalTransformation( matRotBone ) ;
			S3DDQuaternion	qBaseBone( 1, 0, 0, 0 ) ;
			//
			if ( m_poseRef.pPoseBase != nullptr )
			{
				qBaseBone = GetPoseOfBone
					( model, pParent,
						*(m_poseRef.pPoseBase), m_poseRef.secBasePose ) ;
				//
				if ( (m_poseRef.pPoseTarget != nullptr)
					&& (m_poseRef.fpTransition > 0.001) )
				{
					S3DDQuaternion	qTransition = GetPoseOfBone
						( model, pParent,
							*(m_poseRef.pPoseTarget), m_poseRef.secTargetPose ) ;
					//
					qBaseBone.Lerp( qBaseBone, qTransition, m_poseRef.fpTransition ) ;
				}
			}
			//
			S3DDQuaternion	qStability( 1, 0, 0, 0 ) ;
			qStability.Lerp( qRotBone, qBaseBone, m_fpBoneStability ) ;
			qStability.ToMatrix( matRotBone ) ;
			pParent->SetLocalTransformation( matRotBone ) ;
			//
			pParent = pParent->GetParentBone() ;
		}
	}
	S3DDVector	vLocalTip = m_vTipOffset ;
	if ( m_flagHandleTip )
	{
		vLocalTip += pBone->GetBoneHandle() ;
	}
	pBone->OperateInverseKinematics
		( m_vIKPosition, m_matIKRotate, vLocalTip,
			m_fpIKWeight, m_fpGimbalWeight,
			m_fpTipBoneWeight, (size_t) m_nEffectJoints ) ;
	model.PostUpdateBone() ;
}

S3DDQuaternion S3DDynamicModelSerializer::IKBoneTrack::GetPoseOfBone
	( S3DModelBuffer& model,
		S3DModelBoneSpace * pBone, const S3DModelPose & pose, double t )
{
	const SSystem::SString *
				pstrBoneID = model.GetBoneIdentityOf( pBone ) ;
	if ( pstrBoneID != nullptr )
	{
		S3DModelPose::JointAnimation *
						pja = pose.GetJointAs( *pstrBoneID ) ;
		if ( pja != nullptr )
		{
			S3DModelPose::MatrixElement	meBone ;
			S3DDVector					vBoneMove ;
			double						wPhysBlend ;
			pose.CalcJointFrameMatrix
				( meBone, vBoneMove, wPhysBlend, *pja, t ) ;
			//
			if ( meBone.fOrthogonal )
			{
				return	meBone.qRotation ;
			}
			else
			{
				return	S3DDQuaternion( meBone.matTransform ) ;
			}
		}
	}
	return	S3DDQuaternion( 1, 0, 0, 0 ) ;
}

// モデル変更時の処理
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelSerializer::IKBoneTrack::OnChangedModel
	( S3DDynamicModelSerializer& item, S3DModelBuffer * pModel )
{
	S3DSceneComposer::PoseSequencer * pPoseSeq =
			ESLTypeCast<S3DSceneComposer::PoseSequencer>
						( GetParameterSequencer( paramRefPose ) ) ;
	S3DModelPoseLibrary *	pPoseLib = nullptr ;
	if ( pPoseSeq != nullptr )
	{
		if ( pModel != nullptr )
		{
			pPoseLib = &(pModel->GetPoseLibrary()) ;
		}
		else
		{
			S3DSceneComposer::Composition *	pComp = item.GetComposition() ;
			if ( pComp != nullptr )
			{
				S3DSceneComposer *	pScene = pComp->GetSceneComposer() ;
				if ( pScene != nullptr )
				{
					pPoseLib = &(pScene->Assets().PoseLibrary()) ;
				}
			}
		}
		pPoseSeq->SetReferenceLibrary( pPoseLib ) ;
	}
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DDynamicModelSerializer::IKBoneTrack::GetMatrixParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramRotation:
		return	m_matIKRotate ;
	}
	return	S3DDMatrix( 1, 1, 1 ) ;
}

S3DDVector S3DDynamicModelSerializer::IKBoneTrack::GetVectorParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramPosition:
		return	m_vIKPosition ;
	case	paramTipOffset:
		return	m_vTipOffset ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DDynamicModelSerializer::IKBoneTrack::GetScalarParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramGimbalWeight:
		return	m_fpGimbalWeight ;
	case	paramIKWeight:
		return	m_fpIKWeight ;
	case	paramTipBoneWeight:
		return	m_fpTipBoneWeight ;
	case	paramStability:
		return	m_fpBoneStability ;
	}
	return	0.0 ;
}

int32_t S3DDynamicModelSerializer::IKBoneTrack::GetIntegerParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramEffectJoints:
		return	m_nEffectJoints ;
	}
	return	0 ;
}

bool S3DDynamicModelSerializer::IKBoneTrack::GetBooleanParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramHandleTip:
		return	m_flagHandleTip ;
	}
	return	false ;
}

const wchar_t * S3DDynamicModelSerializer::IKBoneTrack::GetCommandParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramBone:
		return	m_strBone ;

	case	paramRefPose:
		if ( !m_strPoseTemp.IsEmpty() )
		{
			return	m_strPoseTemp ;
		}
		ItemSerializer *	pOwner = GetOwnerItem() ;
		if ( pOwner != nullptr )
		{
			((IKBoneTrack*)this)->m_strPoseTemp =
								pOwner->FormatPoseString( m_poseRef ) ;
			return	m_strPoseTemp ;
		}
		else
		{
			return	nullptr ;
		}
	}
	return	nullptr ;
}

S3DSceneComposer::PoseInstance
	S3DDynamicModelSerializer::IKBoneTrack::GetPoseParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramRefPose:
		return	m_poseRef ;
	}
	return	PoseConntoller::GetPoseParameter( iParam ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelSerializer::IKBoneTrack::SetMatrixParameter( size_t iParam, const S3DDMatrix& mat )
{
	switch ( iParam )
	{
	case	paramRotation:
		m_matIKRotate = mat ;
		return ;
	}
}

void S3DDynamicModelSerializer::IKBoneTrack::SetVectorParameter( size_t iParam, const S3DDVector& vec )
{
	switch ( iParam )
	{
	case	paramPosition:
		m_vIKPosition = vec ;
		return ;
	case	paramTipOffset:
		m_vTipOffset = vec ;
		return ;
	}
}

void S3DDynamicModelSerializer::IKBoneTrack::SetScalarParameter( size_t iParam, double s )
{
	switch ( iParam )
	{
	case	paramGimbalWeight:
		m_fpGimbalWeight = s ;
		return ;
	case	paramIKWeight:
		m_fpIKWeight = s ;
		return ;
	case	paramTipBoneWeight:
		m_fpTipBoneWeight = s ;
		return ;
	case	paramStability:
		m_fpBoneStability = s ;
		return ;
	}
}

void S3DDynamicModelSerializer::IKBoneTrack::SetIntegerParameter( size_t iParam, int32_t n )
{
	switch ( iParam )
	{
	case	paramEffectJoints:
		m_nEffectJoints = n ;
		return ;
	}
}

void S3DDynamicModelSerializer::IKBoneTrack::SetBooleanParameter( size_t iParam, bool b )
{
	switch ( iParam )
	{
	case	paramHandleTip:
		m_flagHandleTip = b ;
		return ;
	}
}

void S3DDynamicModelSerializer::IKBoneTrack::SetCommandParameter( size_t iParam, const wchar_t * pwszCmd )
{
	switch ( iParam )
	{
	case	paramBone:
		m_strBone = pwszCmd ;
		return ;

	case	paramRefPose:
		{
			m_strPoseTemp = pwszCmd ;
			//
			ItemSerializer *	pOwner = GetOwnerItem() ;
			if ( pOwner != nullptr )
			{
				pOwner->ParsePoseString( m_poseRef, pwszCmd ) ;
			}
		}
		return ;
	}
}

void S3DDynamicModelSerializer::IKBoneTrack::SetPoseParameter
	( size_t iParam, const S3DSceneComposer::PoseInstance& pose )
{
	if ( iParam == paramRefPose )
	{
		SetRefPose( pose ) ;
		return ;
	}
	PoseConntoller::SetPoseParameter( iParam, pose ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DDynamicModelSerializer::IKBoneTrack::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	switch ( i )
	{
	case	paramBone:
		{
			S3DDynamicModelSerializer *	pdms =
				ESLTypeCast<S3DDynamicModelSerializer>( GetOwnerItem() ) ;
			if ( pdms == nullptr )
			{
				break ;
			}
			S3DModelBuffer *	pModel = pdms->GetModelAlias() ;
			if ( pModel == nullptr )
			{
				break ;
			}
			for ( size_t k = 0; k < pModel->GetBonePropertyList().GetLength(); k ++ )
			{
				const SString *	pstrID =
						pModel->GetBonePropertyList().GetTagAt( k ) ;
				if ( pstrID != nullptr )
				{
					aStrSet.Add( new SString( *pstrID ) ) ;
				}
			}
		}
		return	true ;
	}
	return	false ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DDynamicModelSerializer::IKBoneTrack::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp,
			ItemSerializer * pItem, uint32_t nFlags )
{
	uint32_t	nResFlags =
		PoseConntoller::UpdatePropertyReference( comp, pItem, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefResource )
	{
		if ( (m_poseRef.pPoseBase != nullptr) || !m_strPoseTemp.IsEmpty() )
		{
			if ( m_strPoseTemp.IsEmpty() )
			{
				m_strPoseTemp = FormatPoseString( m_poseRef ) ;
			}
			ParsePoseString( m_poseRef, m_strPoseTemp ) ;
		}
	}
	return	nResFlags ;
}



//////////////////////////////////////////////////////////////////////////////
// 動的モデルアイテム
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
		S3DDynamicModelSerializer::m_paramEntries
				[S3DDynamicModelSerializer::paramModelCount] =
{
	{ L"model",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration,	L"モデル", nullptr },
	{ L"collision",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration,	L"コリジョン", nullptr },
	{ L"collider_flag",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrFlagSetInteger,
		L"衝突フラグ",
		L"当たり判定の対象を判別するためのビット集合を指定する。\n"
		L"システム既定値として 0x01 が形状、0x02 が移動障壁として定義済み。\n"
		L"0x04 は当たり判定（敵）、0x08 は（敵）攻撃当たり判定、"
		L"0x10 はイベント発生判定、0x20 はボーン物理演算当たり判定、"
		L"0x40 は物理演算障壁として推奨。\n"
		L"0x80 は未定義の予約領域で、0x0100 以上がユーザー領域である。" },
	{ L"marker_collision",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration,
		L"マーカー当たり判定", nullptr },
	{ L"dynamic_collision",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"動的なコリジョンモデル",
		L"true の場合、毎フレーム毎にボーン変型を当たり判定モデルに適用します。\n"
		L"false の場合にはボーン変型は無視し、一度だけ衝突判定バッファを作成し流用します。" },
	{ L"pose",
		S3DSceneComposer::typePose,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrStringEnumeration,	L"ポーズ", nullptr },
	{ L"vdraw_target",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrStringEnumeration,	L"バリアント描画", nullptr },
	{ L"border_settings",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant2,	L"固有輪郭設定", nullptr },
	{ L"border_color",
		S3DSceneComposer::typeColor,
		S3DSceneComposer::attrCategory2,	L"輪郭色",nullptr },
	{ L"border_thickness_a",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory2,	L"輪郭太さ（見かけ）", nullptr },
	{ L"border_thickness_b",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory2,	L"輪郭太さ（定数）", nullptr },
} ;

const S3DSceneComposer::ParamSetClass
		S3DDynamicModelSerializer::m_pscClass =
{
	&S3DSceneComposer::ItemCommonSerializer::m_pscClass,
	S3DDynamicModelSerializer::paramModelCount,
	&S3DDynamicModelSerializer::m_paramEntries[0]
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DDynamicModelSerializer,
			S3DDynamicModelItem, ItemCommonSerializer )
S3D_IMPLEMENT_COMPOSER_ITEM( S3DDynamicModelSerializer, dynamic_model )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DDynamicModelSerializer::S3DDynamicModelSerializer( void )
	: ItemCommonSerializer
			( m_ItemClassDescriptor.pwszClassID,
						&S3DDynamicModelSerializer::m_pscClass, nullptr ),
		m_flagDynamicCollision( false )
{
	AttachSceneItem( (S3DDynamicModelItem*) this ) ;
	m_flagsBehavior |= S3DScene::itemTimer ;
	m_flagVisible = ((m_flagsBehavior & S3DScene::itemVisible) != 0) ;
	m_typeMarkerCollision = S3DModelData::MarkerInfo::typeInvalid ;
}

S3DDynamicModelSerializer::S3DDynamicModelSerializer
	( const wchar_t * pwszClassID,
		const S3DSceneComposer::ParamSetClass * pClass )
	: ItemCommonSerializer( pwszClassID, pClass, nullptr ),
		m_flagDynamicCollision( false )
{
	AttachSceneItem( (S3DDynamicModelItem*) this ) ;
	m_flagsBehavior |= S3DScene::itemTimer ;
	m_flagVisible = ((m_flagsBehavior & S3DScene::itemVisible) != 0) ;
	m_typeMarkerCollision = S3DModelData::MarkerInfo::typeInvalid ;
}

// モデル
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelSerializer::AttachModelReference
	( S3DModelBuffer * pModel, S3DSceneComposer * pComposer )
{
	if ( pModel != nullptr )
	{
		m_modelRef.AttachModelReference( *pModel ) ;
		m_modelRef.AttachRelationItem( this ) ;
		AttachModel( &m_modelRef ) ;
		ResetPhysicsParameter() ;
		//
		m_modelRef.GetPoseLibrary().DetachAllReferenceLibrarys() ;
		if ( pComposer != nullptr )
		{
			m_modelRef.GetPoseLibrary().AddReferenceLibrary
					( &(pComposer->Assets().PoseLibrary()) ) ;
		}
		if ( m_maskMarkerCollider != 0 )
		{
			BuildCollisionMesh() ;
		}
		OnChangedModel( &m_modelRef ) ;
	}
	else
	{
		AttachModel( nullptr ) ;
		m_modelRef.ClearBuffer() ;
		m_modelRef.GetPoseLibrary().DetachAllReferenceLibrarys() ;
		//
		OnChangedModel( nullptr ) ;
	}
}

void S3DDynamicModelSerializer::SetModel( const wchar_t * pwszModelID )
{
	if ( m_strModelID != pwszModelID )
	{
		m_strModelID = pwszModelID ;
		UpdateModel() ;
	}
}

void S3DDynamicModelSerializer::UpdateModel( void )
{
	if ( m_strModelID.IsEmpty() )
	{
		if ( m_pModel != nullptr )
		{
			AttachModel( nullptr ) ;
			OnChangedModel( nullptr ) ;
		}
		return ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		S3DSceneComposer *	pSceneComp = pComp->GetSceneComposer() ;
		if ( pSceneComp != nullptr )
		{
			S3DModelBuffer *	pModel =
				pSceneComp->Assets().GetModelAs( m_strModelID ) ;
			if ( m_pModel != pModel )
			{
				AttachModelReference( pModel ) ;
			}
		}
	}
}

const wchar_t * S3DDynamicModelSerializer::GetModelID( void ) const
{
	return	m_strModelID ;
}

// 表示用モデルエイリアス取得
//////////////////////////////////////////////////////////////////////////////
S3DModelBuffer * S3DDynamicModelSerializer::GetModelAlias( void ) const
{
	if ( &m_modelRef == m_pModel )
	{
		return	(S3DModelBuffer*) &m_modelRef ;
	}
	return	nullptr ;
}

// 衝突判定モデル
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelSerializer::SetCollision( const wchar_t * pwszModelID )
{
	if ( m_strCollisionID != pwszModelID )
	{
		m_strCollisionID = pwszModelID ;
		UpdateCollision() ;
	}
}

void S3DDynamicModelSerializer::UpdateCollision( void )
{
	if ( m_strCollisionID.IsEmpty() )
	{
		AttachCollisionModel( nullptr ) ;
		return ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		S3DSceneComposer *	pSceneComp = pComp->GetSceneComposer() ;
		if ( pSceneComp != nullptr )
		{
			AttachCollisionModel
				( pSceneComp->Assets().GetModelAs( m_strCollisionID ) ) ;
		}
	}
}

const wchar_t * S3DDynamicModelSerializer::GetCollisionID( void ) const
{
	return	m_strCollisionID ;
}

// 動的な衝突判定モデルか？
//////////////////////////////////////////////////////////////////////////////
bool S3DDynamicModelSerializer::IsDynamicCollision( void ) const
{
	return	m_flagDynamicCollision ;
}

void S3DDynamicModelSerializer::SetDynamicCollisionFlag( bool flagDynamic )
{
	m_flagDynamicCollision = flagDynamic ;
}

// バリアント描画ターゲット
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelSerializer::SetVariantDrawTaregt( const wchar_t * pwszVDrawTarget )
{
	if ( m_strVDrawtTarget != pwszVDrawTarget )
	{
		m_strVDrawtTarget = pwszVDrawTarget ;
		UpdateVariantDrawTaregt() ;
	}
}

bool S3DDynamicModelSerializer::UpdateVariantDrawTaregt( void )
{
	if ( m_strVDrawtTarget.IsEmpty() )
	{
		m_maskClasses &= ~(1 << S3DScene::classPreRender) ;
		//
		m_refVDrawTarget.SetReference( nullptr ) ;
		return	true ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	bool	flagSuccess = false ;
	if ( pComp != nullptr )
	{
		S3DSceneComposer::ItemSerializer *
			pItem = pComp->GetSceneItemAs( m_strVDrawtTarget ) ;
		m_refVDrawTarget.SetReference( pItem ) ;
		flagSuccess = (pItem != nullptr) ;
	}
	m_maskClasses |= (1 << S3DScene::classPreRender) ;
	return	flagSuccess ;
}

const wchar_t * S3DDynamicModelSerializer::GetVariantDrawTarget( void ) const
{
	return	m_strVDrawtTarget ;
}

// モデル変更時の処理
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelSerializer::OnChangedModel( S3DModelBuffer * pModel )
{
	size_t nCtrls = GetControllerCount() ;
	for ( size_t i = 0; i < nCtrls; i ++ )
	{
		S3DSceneComposer::Controller *	pCtrl = GetControllerAt( i ) ;
		PoseInterface *	pPose = ESLTypeCast<PoseInterface>( pCtrl ) ;
		if ( pPose != nullptr )
		{
			pPose->OnChangedModel( *this, pModel ) ;
		}
		else if ( pCtrl != nullptr )
		{
			S3DSceneComposer::Composition *	pComp = GetComposition() ;
			if ( pComp != nullptr )
			{
				pCtrl->UpdatePropertyReference
					( *pComp, this, S3DSceneComposer::updateRefResource ) ;
			}
		}
	}
}

// モデルデータ関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelSerializer::AttachModel( S3DVertexBufferInterface * pModel )
{
	m_pModel = pModel ;
}

void S3DDynamicModelSerializer::AttachCollisionModel
	( S3DVertexBufferInterface * pColModel, bool fBuildCollision )
{
	m_pCollision = pColModel ;
	//
	if ( fBuildCollision
		&& ((pColModel != nullptr) || (m_maskMarkerCollider != 0)) )
	{
		BuildCollisionMesh() ;
	}
}

// ポーズ設定
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelSerializer::UpdateModelPose( S3DModelBuffer& model )
{
	S3DDynamicModelItem::UpdateModelPose( model ) ;

	if ( m_flagDynamicCollision )
	{
		S3DModelBuffer *	pCollision = ESLTypeCast<S3DModelBuffer>( m_pCollision ) ;
		if ( pCollision != nullptr )
		{
			bool	flagBoneUpdated = false ;
			model.LockModelData() ;
			if ( model.IsUpdateBone() )
			{
				model.UpdateBoneMatrix() ;
				flagBoneUpdated = true ;
			}
			model.UnlockModelData() ;
			//
			if ( flagBoneUpdated )
			{
				if ( pCollision != &model )
				{
					pCollision->LockModelData() ;
					pCollision->DuplicatePoseOf( model ) ;
					pCollision->UpdateBoneMatrix() ;
					pCollision->UnlockModelData() ;
				}
				BuildCollisionMesh() ;
			}
		}
	}
}

// ポーズアニメーション
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelSerializer::OnPoseAnimationTrack( S3DModelBuffer& model )
{
	if ( m_poseCurrent.pPoseBase != nullptr )
	{
		m_poseCurrent.pPoseBase->ApplyPoseTo
				( model, 1.0, m_poseCurrent.secBasePose ) ;
		//
		if ( m_poseCurrent.pPoseTarget != nullptr )
		{
			m_poseCurrent.pPoseTarget->ApplyPoseTo
					( model, m_poseCurrent.fpTransition,
								m_poseCurrent.secTargetPose ) ;
		}
	}
	size_t nCtrls = GetControllerCount() ;
	for ( size_t i = 0; i < nCtrls; i ++ )
	{
		S3DSceneComposer::Controller *	pCtrl = GetControllerAt( i ) ;
		PoseInterface *	pPose = ESLTypeCast<PoseInterface>( pCtrl ) ;
		if ( (pPose != nullptr)
			&& !pCtrl->IsControllerDisabled() )
		{
			pPose->OnPoseTrack( *this, model ) ;
		}
	}
	S3DDynamicModelItem::OnPoseAnimationTrack( model ) ;
}

// 表示モデル追加
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelSerializer::RenderLocalModel
	( const S3DScene& scene,
		S3DRenderContextInterface& render, uint64_t flagsExclusion )
{
	if ( m_strVDrawtTarget.IsEmpty() )
	{
		S3DDynamicModelItem::RenderLocalModel( scene, render, flagsExclusion ) ;
	}
	else
	{
		Item::RenderLocalModel( scene, render, flagsExclusion ) ;
	}
}

// レンダリング前後処理（全視点共通）
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelSerializer::OnRenderEvent
	( S3DScene& scene, S3DScene::ItemClass clsItem )
{
	S3DDynamicModelItem::OnRenderEvent( scene, clsItem ) ;
	//
	if ( (clsItem == S3DScene::classPreRender)
			&& !m_strVDrawtTarget.IsEmpty() && m_flagVisible )
	{
		S3DSceneComposer::ItemSerializer *	pItem =
				ESLTypeCast<S3DSceneComposer::ItemSerializer>
					( m_refVDrawTarget.GetReference() ) ;
		S3DScene::ModelItem *	pModelItem =
				ESLTypeCast<S3DScene::ModelItem>
					( m_refVDrawTarget.GetReference() ) ;
		S3DVertexBufferInterface *	pVariant = m_pModel ;
		if ( (pItem != nullptr) && (pModelItem != nullptr) && (pVariant != nullptr) )
		{
			S3DVertexBufferInterface *	pModel = pModelItem->GetModel() ;
			if ( pModel != nullptr )
			{
				S3DDMatrix	matdModel ;
				S3DDVector	vdModel ;
				GetGlobalTransformation( matdModel, vdModel );
				//
				S3DMultiModelSerializer *	pMultiModel =
					ESLTypeCast<S3DMultiModelSerializer>( pModelItem ) ;
				bool	flagCulling = false ;
				if ( (pMultiModel != nullptr)
					&& (pMultiModel->Instancing().GetCulling()
						!= S3DItemInstancingSerializer::cullingNothing) )
				{
					// カリング判定
					S3DItemInstancingSerializer::FrustumInfo	fi ;
					S3DDMatrix	matdCamera ;
					S3DDVector	vdCamera ;
//					S3DMatrix	matBase ;
					S3DVector	vBase ;
					//
					S3DScene::ProjectionParam	pp ;
					scene.GetProjection( pp ) ;
					fi.vScreen = pp.vScreen ;
					fi.zScale = pp.fpZoom ;
					fi.fpPixelAspect = pp.fpPixelAspect ;
					//
					fi.rectView.x = 0 ;
					fi.rectView.y = 0 ;
					fi.rectView.w = (int32_t) esl_roundfi( pp.vScreen.x * pp.fpZoom * 2.0f ) ;
					fi.rectView.h = (int32_t) esl_roundfi( pp.vScreen.y * pp.fpZoom * 2.0f ) ;
					//
					matdCamera = scene.GetCurrentCameraTransformation( vdCamera ) ;
					//
//					matBase = matdCamera * matdModel ;
					vBase = matdCamera * vdModel - vdCamera ;
					//
					float32_t	sinAngle =
						(float32_t) sin( pMultiModel->GetCullingAngleGap() * PI / 180.0 ) ;
					pMultiModel->Instancing().PrepareCullingInfo( fi, sinAngle, sinAngle ) ;
					//
					S3DVector	vCenter ;
					float32_t	fpInstanceSize =
						(float32_t) pVariant->GetCircumscribedSphere( vCenter )
							+ (float32_t) pMultiModel->GetCullingOffset() ;
					flagCulling =
						pMultiModel->Instancing().IsCullingInstance
								( fi, /*matBase,*/ vBase, fpInstanceSize ) ;
				}
				if ( !flagCulling )
				{
					// バリアント追加
					S3DDMatrix	matdTarget ;
					S3DDVector	vdTarget ;
					pItem->GetTransformationFrom
							( matdTarget, vdTarget, matdModel, vdModel ) ;
					//
					S3DMatrix	matInstance = matdTarget ;
					S3DVector	vInstance = vdTarget ;
					//
					S3DColor	clrInstance = m_space.m_colorEffect ;
					clrInstance.rgbMul.argb.Alpha =
						(uint8_t) esl_clampi
							( 0xFF - (int) m_space.m_nTransparency, 0, 0xFF ) ;
					//
					S3DModelBuffer *	pVarModel = ESLTypeCast<S3DModelBuffer>( pVariant ) ;
					if ( pVarModel != nullptr )
					{
						pVarModel->LockModelData() ;
						if ( pVarModel->IsUpdateBone() )
						{
							pVarModel->UpdateBoneMatrix() ;
						}
						pVarModel->UnlockModelData() ;
					}
					S4DMatrix	mat4Instance( matInstance, vInstance ) ;
					pModel->AddInstanceVariant
						( pVariant, mat4Instance, clrInstance ) ;
				}
			}
		}
	}
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DDynamicModelSerializer::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramBorderColor:
		return	VectorFromColor( m_borderParam.rgbBorder ) ;
	}
	return	ItemCommonSerializer::GetVectorParameter( i ) ;
}

double S3DDynamicModelSerializer::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramBorderThickness1:
		return	m_borderParam.aThickness ;
	case	paramBorderThickness2:
		return	m_borderParam.bThickness ;
	}
	return	ItemCommonSerializer::GetScalarParameter( i ) ;
}

int32_t S3DDynamicModelSerializer::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramColliderFlags:
		return	(int32_t) GetColliderUserFlags() ;
	}
	return	ItemCommonSerializer::GetIntegerParameter( i ) ;
}

bool S3DDynamicModelSerializer::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramVisible:
		return	m_flagVisible ;
	case	paramDynamicCollision:
		return	m_flagDynamicCollision ;
	case	paramBorderParam:
		return	m_flagLocalBorder ;
	}
	return	ItemCommonSerializer::GetBooleanParameter( i ) ;
}

const wchar_t * S3DDynamicModelSerializer::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramModel:
		return	m_strModelID ;
	case	paramCollision:
		return	m_strCollisionID ;
	case	paramMarkerCollision:
		if ( m_typeMarkerCollision != S3DModelData::MarkerInfo::typeInvalid )
		{
			return	S3DModelData::MarkerInfo::m_pwszTypeTags[m_typeMarkerCollision] ;
		}
		return	L"nothing" ;
	case	paramPose:
		if ( m_strPoseTemp.IsEmpty() )
		{
			((S3DDynamicModelSerializer*)this)->m_strPoseTemp =
									FormatPoseString( m_poseCurrent ) ;
		}
		return	m_strPoseTemp ;
	case	paramVDrawTarget:
		return	m_strVDrawtTarget ;
	}
	return	ItemCommonSerializer::GetCommandParameter( i ) ;
}

S3DSceneComposer::PoseInstance
		S3DDynamicModelSerializer::GetPoseParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramPose:
		return	m_poseCurrent ;
	}
	return	ItemCommonSerializer::GetPoseParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelSerializer::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramBorderColor:
		m_borderParam.rgbBorder = ColorFromVector( vec ) ;
		return ;
	}
	ItemCommonSerializer::SetVectorParameter( i, vec ) ;
}

void S3DDynamicModelSerializer::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramBorderThickness1:
		m_borderParam.aThickness = (float32_t) s ;
		return ;
	case	paramBorderThickness2:
		m_borderParam.bThickness = (float32_t) s ;
		return ;
	}
	ItemCommonSerializer::SetScalarParameter( i, s ) ;
}

void S3DDynamicModelSerializer::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramColliderFlags:
		SetColliderUserFlags( (uint32_t) n ) ;
		return ;
	}
	ItemCommonSerializer::SetIntegerParameter( i, n ) ;
}

void S3DDynamicModelSerializer::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramVisible:
		m_flagVisible = b ;
		m_flagsBehavior =
			(m_flagsBehavior & ~S3DScene::itemVisible)
				| ((m_flagVisible && m_strVDrawtTarget.IsEmpty())
										? S3DScene::itemVisible : 0) ;
		return ;
	case	paramDynamicCollision:
		m_flagDynamicCollision = b ;
		return ;
	case	paramBorderParam:
		m_flagLocalBorder = b ;
		return ;
	}
	ItemCommonSerializer::SetBooleanParameter( i, b ) ;
}

void S3DDynamicModelSerializer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramModel:
		SetModel( pwszCmd ) ;
		return ;
	case	paramCollision:
		SetCollision( pwszCmd ) ;
		return ;
	case	paramMarkerCollision:
		{
			m_typeMarkerCollision = S3DModelData::MarkerInfo::typeInvalid ;
			SetModelMarkerForCollider( 0 ) ;
			//
			for ( int j = 0; j < S3DModelData::MarkerInfo::typeCount; j ++ )
			{
				if ( SString::Compare
					( pwszCmd, S3DModelData::MarkerInfo::m_pwszTypeTags[j] ) == 0 )
				{
					m_typeMarkerCollision = (S3DModelData::MarkerInfo::Type) j ;
					SetModelMarkerForCollider( 1 << j ) ;
					break ;
				}
			}
		}
		return ;
	case	paramPose:
		ParsePoseString( m_poseCurrent, pwszCmd ) ;
		m_strPoseTemp = pwszCmd ;
		return ;
	case	paramVDrawTarget:
		SetVariantDrawTaregt( pwszCmd ) ;
		return ;
	}
	ItemCommonSerializer::SetCommandParameter( i, pwszCmd ) ;
}

void S3DDynamicModelSerializer::SetPoseParameter
		( size_t i, const S3DSceneComposer::PoseInstance& pose )
{
	switch ( i )
	{
	case	paramPose:
		m_poseCurrent = pose ;
		m_strPoseTemp.FreeArray() ;
		return ;
	}
	ItemCommonSerializer::SetPoseParameter( i, pose ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DDynamicModelSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	S3DSceneComposer::Composition *	pComp ;
	switch ( i )
	{
	case	paramModel:
	case	paramCollision:
		pComp = GetComposition() ;
		if ( pComp != nullptr )
		{
			S3DSceneComposer *	pSceneComp = pComp->GetSceneComposer() ;
			if ( pSceneComp != nullptr )
			{
				pSceneComp->Assets().EnumerateResourceIDsAs
					( aStrSet, ESL_RUNTIME_CLASS(S3DModelBuffer) ) ;
				pSceneComp->GetAssets().SortStringSet( aStrSet ) ;
			}
		}
		return	true ;

	case	paramMarkerCollision:
		{
			aStrSet.Add( new SString( L"nothing" ) ) ;
			for ( int j = 0; j < S3DModelData::MarkerInfo::typeCount; j ++ )
			{
				aStrSet.Add( new SString(S3DModelData::MarkerInfo::m_pwszTypeTags[j]) ) ;
			}
		}
		return	true ;

	case	paramPose:
		EnumeratePoseStringSet( aStrSet ) ;
		return	true ;

	case	paramVDrawTarget:
		pComp = GetComposition() ;
		if ( pComp != nullptr )
		{
			pComp->EnumerateItemIDsAs
				( aStrSet, ESL_RUNTIME_CLASS(S3DMultiModelSerializer) ) ;
		}
		return	true ;
	}
	return	ItemCommonSerializer::EnumerateStringSet( i, aStrSet ) ;
}

void S3DDynamicModelSerializer::EnumeratePoseStringSet
	( SSystem::SStringArray& aStrSet ) const
{
	const S3DModelPoseLibrary *	pPoseLib = &(m_modelRef.GetPoseLibrary()) ;
	EnumeratePoseStringSetOf( aStrSet, pPoseLib ) ;
	//
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		S3DSceneComposer *	pSceneComp = pComp->GetSceneComposer() ;
		if ( pSceneComp != nullptr )
		{
			EnumeratePoseStringSetOf
				( aStrSet, &(pSceneComp->Assets().GetPoseLibrary()) ) ;
		}
	}
}

void S3DDynamicModelSerializer::EnumeratePoseStringSetOf
	( SSystem::SStringArray& aStrSet, const S3DModelPoseLibrary *	pPoseLib )
{
	S3DSceneComposer::ResourceAssets::EnumeratePoseStringSetOf( aStrSet, pPoseLib ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DDynamicModelSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	if ( iCategory == 1 )
	{
		return	L"モデル設定" ;
	}
	else if ( iCategory == 2 )
	{
		return	L"輪郭描画設定" ;
	}
	return	ItemCommonSerializer::GetParameterCategoryName( iCategory ) ;
}

// タイマ処理 Item::OnTimer / ItemSerializer::OnTimer 実装
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelSerializer::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	S3DDynamicModelItem::OnTimer( scene, msecPast ) ;
	ItemCommonSerializer::OnTimer( scene, msecPast ) ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DDynamicModelSerializer::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramForceToon:
	case	paramForceBorder:
		return	true ;
	}
	return	ItemCommonSerializer::IsParameterValidation( i ) ;
}

// ポーズ取得
//////////////////////////////////////////////////////////////////////////////
S3DModelPose *
	S3DDynamicModelSerializer::GetPoseIdentityAs( const wchar_t * pwszPoseID ) const
{
	S3DModelPose *	pPose =
			m_modelRef.GetPoseLibrary().GetPoseAs( pwszPoseID ) ;
	if ( pPose != nullptr )
	{
		return	pPose ;
	}
	return	ItemCommonSerializer::GetPoseIdentityAs( pwszPoseID ) ;
}

// ポーズID取得
//////////////////////////////////////////////////////////////////////////////
bool S3DDynamicModelSerializer::GetPoseIdentityOf
	( SSystem::SString& strPoseID, S3DModelPose * pPose ) const
{
	if ( pPose == nullptr )
	{
		return	false ;
	}
	const wchar_t *	pwszID =
			m_modelRef.GetPoseLibrary().GetPoseIdentityOf( pPose ) ;
	if ( pwszID != nullptr )
	{
		strPoseID = pwszID ;
		return	true ;
	}
	return	ItemCommonSerializer::GetPoseIdentityOf( strPoseID, pPose ) ;
}

// アイテムのプライマリモデル取得
//////////////////////////////////////////////////////////////////////////////
S3DVertexBufferInterface * S3DDynamicModelSerializer::GetItemPrimaryModel( void )
{
	return	&m_modelRef ;
}

// アイテムのコリジョンバッファ取得
//////////////////////////////////////////////////////////////////////////////
S3DCollider * S3DDynamicModelSerializer::GetItemPrimaryCollider( void )
{
	if ( m_flagCollision )
	{
		return	&m_collisionMesh ;
	}
	return	nullptr ;
}

// ポーズライブラリ取得
//////////////////////////////////////////////////////////////////////////////
S3DModelPoseLibrary * S3DDynamicModelSerializer::GetPoseLibraryChain( void )
{
	return	&(m_modelRef.GetPoseLibrary()) ;
}

// フレーム更新後処理
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelSerializer::OnUpdateFrame
	( double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	ItemCommonSerializer::OnUpdateFrame( fpFrame, seek ) ;
	//
	if ( seek == S3DSceneComposer::seekJumpReset )
	{
		DelayResetPhysicsAndDriveFrames() ;
	}
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DDynamicModelSerializer::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp, uint32_t nFlags )
{
	uint32_t	nResFlags = 0 ;
	if ( nFlags & S3DSceneComposer::updateRefResource )
	{
		UpdateModel() ;
		UpdateCollision() ;
		//
		if ( (m_poseCurrent.pPoseBase != nullptr) || !m_strPoseTemp.IsEmpty() )
		{
			if ( m_strPoseTemp.IsEmpty() )
			{
				m_strPoseTemp = FormatPoseString( m_poseCurrent ) ;
			}
			ParsePoseString( m_poseCurrent, m_strPoseTemp ) ;
		}
	}
	if ( nFlags & S3DSceneComposer::updateRefItem )
	{
		if ( !UpdateVariantDrawTaregt() )
		{
			nResFlags |= S3DSceneComposer::updateRefItem ;
		}
	}
	nResFlags |= ItemCommonSerializer::UpdatePropertyReference( comp, nFlags ) ;
	return	nResFlags ;
}

// シーン取得
//////////////////////////////////////////////////////////////////////////////
S3DScene * S3DDynamicModelSerializer::GetScene( void ) const
{
	return	ItemCommonSerializer::GetScene() ;
}

// Loquaty クラス名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DDynamicModelSerializer::GetLQClassName( void ) const
{
	return	L"EntisGLS4.SceneDynamicModel" ;
}



//////////////////////////////////////////////////////////////////////////////
// 物理演算慣性ポーズ制御
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DBasicInertialPoser::S3DBasicInertialPoser( void )
	: m_degFreeAngle( 5.0 ),
		m_fpAttenuation( 0.2 ), m_fpHardness( 0.05 ), m_fpEffect( 0.3 ),
		m_pModel( nullptr ), m_flagResetPose( true ), m_secPhysical( 0.0 )
{
}

// モデル関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DBasicInertialPoser::AttachTargetModel
	( S3DModelBuffer * pModel, bool flagForceUpdateRef )
{
	if ( (m_pModel != pModel) || flagForceUpdateRef )
	{
		m_pModel = pModel ;
		UpdateModelBoneRef() ;
	}
}

// ボーン参照更新
//////////////////////////////////////////////////////////////////////////////
void S3DBasicInertialPoser::UpdateModelBoneRef( void )
{
	m_aBonePtr.SetLength( m_aTargetBone.GetLength() ) ;
	m_aPhysVertex.SetLength( m_aTargetBone.GetLength() ) ;
	//
	for ( size_t i = 0; i < m_aTargetBone.GetLength(); i ++ )
	{
		SString *	pstrBoneID = m_aTargetBone.GetAt( i ) ;
		if ( pstrBoneID == nullptr )
		{
			m_aBonePtr.SetAt( i, nullptr ) ;
			continue ;
		}
		if ( m_pModel != nullptr )
		{
			m_aBonePtr.SetAt( i, m_pModel->GetBonePropertyAs( *pstrBoneID ) ) ;
		}
		else
		{
			m_aBonePtr.SetAt( i, nullptr ) ;
		}
	}
}

// 関連付けモデル取得
//////////////////////////////////////////////////////////////////////////////
S3DModelBuffer * S3DBasicInertialPoser::GetTargetModel( void ) const
{
	return	m_pModel ;
}

// 対象ボーン数設定
//////////////////////////////////////////////////////////////////////////////
void S3DBasicInertialPoser::SetTargetBoneCount( size_t nCount )
{
	m_aTargetBone.SetLength( nCount ) ;
	m_aBonePtr.SetLength( nCount ) ;
	m_aPhysVertex.SetLength( nCount ) ;
}

// 対象ボーン数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DBasicInertialPoser::GetTargetBoneCount( void ) const
{
	return	m_aTargetBone.GetLength() ;
}

// 対象ボーン設定
//////////////////////////////////////////////////////////////////////////////
void S3DBasicInertialPoser::SetTargetBoneNameAt( size_t i, const wchar_t * pwszBoneID )
{
	ESLAssert( i < m_aTargetBone.GetLength() ) ;
	SString *	pstrBoneID = m_aTargetBone.GetAt( i ) ;
	if ( pstrBoneID == nullptr )
	{
		pstrBoneID = new SString( pwszBoneID ) ;
		m_aTargetBone.SetAt( i, pstrBoneID ) ;
	}
	else
	{
		*pstrBoneID = pwszBoneID ;
	}
	if ( m_pModel != nullptr )
	{
		m_aBonePtr.SetAt( i, m_pModel->GetBonePropertyAs( pwszBoneID ) ) ;
	}
	else
	{
		m_aBonePtr.SetAt( i, nullptr ) ;
	}
}

// 対象ボーン追加
//////////////////////////////////////////////////////////////////////////////
size_t S3DBasicInertialPoser::AddTargetBoneName( const wchar_t * pwszBoneID )
{
	ESLAssert( m_aBonePtr.GetLength() == m_aTargetBone.GetLength() ) ;
	size_t	iEntry = m_aTargetBone.Add( new SString( pwszBoneID ) ) ;
	if ( m_pModel != nullptr )
	{
		m_aBonePtr.Add( m_pModel->GetBonePropertyAs( pwszBoneID ) ) ;
	}
	else
	{
		m_aBonePtr.Add( nullptr ) ;
	}
	m_flagResetPose = true ;
	return	iEntry ;
}

// 対象ボーン名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DBasicInertialPoser::GetTargetBoneNameAt( size_t i ) const
{
	SString *	pstrBoneID = m_aTargetBone.GetAt( i ) ;
	return	(pstrBoneID != nullptr) ? (const wchar_t*) *pstrBoneID : nullptr ;
}

// 角自由度[deg]
//////////////////////////////////////////////////////////////////////////////
double S3DBasicInertialPoser::GetFreeAngle( void ) const
{
	return	m_degFreeAngle ;
}

void S3DBasicInertialPoser::SetFreeAngle( double degAngle )
{
	m_degFreeAngle = degAngle ;
}

// 速度減衰率 [0,1] [/sec]
//////////////////////////////////////////////////////////////////////////////
double S3DBasicInertialPoser::GetAttenuation( void ) const
{
	return	m_fpAttenuation ;
}

void S3DBasicInertialPoser::SetAttenuation( double fpAttenuation )
{
	m_fpAttenuation = fpAttenuation ;
}

// 弾性 [/frame]
//////////////////////////////////////////////////////////////////////////////
double S3DBasicInertialPoser::GetHardness( void ) const
{
	return	m_fpHardness ;
}

void S3DBasicInertialPoser::SetHardness( double fpHardness )
{
	m_fpHardness = fpHardness ;
}

// 慣性影響度
//////////////////////////////////////////////////////////////////////////////
double S3DBasicInertialPoser::GetEffect( void ) const
{
	return	m_fpEffect ;
}

void S3DBasicInertialPoser::SetEffect( double fpEffect )
{
	m_fpEffect = fpEffect ;
}

// 初期化フラグ設定
//////////////////////////////////////////////////////////////////////////////
void S3DBasicInertialPoser::SetResetFlag( void )
{
	m_flagResetPose = true ;
	m_secPhysical = 0.0 ;
}

// 経過時間
//////////////////////////////////////////////////////////////////////////////
void S3DBasicInertialPoser::AddElapsedTime( double secElapsed )
{
	m_secPhysical += secElapsed ;
}

// 物理演算パラメータ初期化
//////////////////////////////////////////////////////////////////////////////
void S3DBasicInertialPoser::ResetPhysicalParameter( S3DSceneComposer::ItemSerializer& item )
{
	m_flagResetPose = false ;
	m_secPhysical = 0.0 ;
	//
	item.GetGlobalTransformation
		( m_physExogenous.matSpace, m_physExogenous.vSpace ) ;
	m_physExogenous.matLastSpace = m_physExogenous.matSpace ;
	m_physExogenous.vLastSpace = m_physExogenous.vSpace ;
	m_physExogenous.vAcceleration = S3DDVector( 0, 0, 0 ) ;
	m_physExogenous.vStream = S3DDVector( 0, 0, 0 ) ;
	//
	const size_t	nTargetCount = GetTargetBoneCount() ;
	m_aPhysVertex.SetLength( nTargetCount ) ;
	//
	for ( size_t i = 0; i < nTargetCount; i ++ )
	{
		InertialVertex *	piv = m_aPhysVertex.GetAt( i ) ;
		S3DModelBoneSpace *	pBone = m_aBonePtr.GetAt( i ) ;
		ESLAssert( piv != nullptr ) ;
		if ( (piv == nullptr) || (pBone == nullptr) )
		{
			continue ;
		}
		pBone->CalcBoneTransformation( piv->matLastSpace, piv->vLastSpace ) ;
		//
		piv->vPos = pBone->GetBoneHandle() ;
		piv->vSpeed = S3DDVector( 0, 0, 0 ) ;
		piv->vLastExSpeed = S3DDVector( 0, 0, 0 ) ;
		piv->vExSpeedLPF = S3DDVector( 0, 0, 0 ) ;
		piv->nHitCollider = 0 ;
		piv->vHitNormal = S3DDVector( 0, 0, 0 ) ;
		piv->matInertialBone = pBone->m_matTransformation ;
	}
}

// 物理演算実行
//////////////////////////////////////////////////////////////////////////////
void S3DBasicInertialPoser::CalculatePhysics
	( S3DSceneComposer::ItemSerializer& item, S3DModelBuffer& model )
{
	if ( m_flagResetPose )
	{
		ResetPhysicalParameter( item ) ;
		m_flagResetPose = false ;
	}
	if ( m_secPhysical < 1.0e-7 )
	{
		for ( size_t i = 0; i < m_aPhysVertex.GetLength(); i ++ )
		{
			InertialVertex *	piv = m_aPhysVertex.GetAt( i ) ;
			S3DModelBoneSpace *	pBone = m_aBonePtr.GetAt( i ) ;
			ESLAssert( piv != nullptr ) ;
			if ( (piv == nullptr) || (pBone == nullptr) )
			{
				continue ;
			}
			SetBoneRotationWithPhysics( *pBone, *piv ) ;
		}
		return ;
	}
	//
	// 空間更新
	//
	m_physExogenous.matLastSpace = m_physExogenous.matSpace ;
	m_physExogenous.vLastSpace = m_physExogenous.vSpace ;
	//
	S3DModelBoneSpace&	boneSpace = model.GetLocalSpaceBone() ;
	item.GetGlobalTransformation
		( m_physExogenous.matSpace, m_physExogenous.vSpace ) ;
	m_physExogenous.vSpace +=
		m_physExogenous.matSpace
			* (boneSpace.m_vCenter + boneSpace.GetBoneOffset()) ;
	m_physExogenous.matSpace *= boneSpace.m_matTransformation ;
	//
	// 各ボーン
	//
	S3DModelBoneSpace::PhysMaterial	mtrl ;
	mtrl.fpAttenuation = m_fpAttenuation ;
	mtrl.fpMinStretch = 1.0 ;
	mtrl.fpMaxStretch = 1.0 ;
	mtrl.fpHardness = m_fpHardness ;
	mtrl.fpEffect = m_fpEffect ;
	mtrl.fpLimitedAngle = 180.0 - m_degFreeAngle ;
	//
	for ( size_t i = 0; i < m_aPhysVertex.GetLength(); i ++ )
	{
		InertialVertex *	piv = m_aPhysVertex.GetAt( i ) ;
		S3DModelBoneSpace *	pBone = m_aBonePtr.GetAt( i ) ;
		ESLAssert( piv != nullptr ) ;
		if ( (piv == nullptr) || (pBone == nullptr) )
		{
			continue ;
		}
		CalculateBonePhysics
			( *pBone, *piv, mtrl, m_physExogenous, m_secPhysical ) ;
		SetBoneRotationWithPhysics( *pBone, *piv ) ;
	}
	m_secPhysical = 0.0 ;
}

void S3DBasicInertialPoser::CalculateBonePhysics
	( S3DModelBoneSpace& bone,
		S3DBasicInertialPoser::InertialVertex& iv,
		const S3DModelBoneSpace::PhysMaterial& mtrl,
		const S3DModelBoneSpace::PhysExogenous& exog, double secElapsed )
{
	const double	frameRate = 60.0 ;
	//
	S3DDMatrix	matBone ;
	S3DDVector	vBone ;
	bone.CalcBoneTransformation( matBone, vBone ) ;
	//
	S3DDMatrix	matLastSpace = exog.matLastSpace * iv.matLastSpace ;
	S3DDVector	vLastSpace = exog.matLastSpace * iv.vLastSpace ;
	S3DDMatrix	matCurSpace = exog.matSpace * matBone ;
	S3DDVector	vCurSpace = exog.matSpace * vBone ;
	S3DDVector	vLastGlobalPos = matLastSpace * iv.vPos + vLastSpace ;
	S3DDVector	vMovedGlobalPos = matCurSpace * iv.vPos + vCurSpace ;
	S3DDVector	vCurHandleGlobalPos = matCurSpace * bone.GetBoneHandle() + vCurSpace ;
	//
	// ボーン速度と加速度×秒
	//
	S3DDVector	vMoveDelta = vMovedGlobalPos - vLastGlobalPos ;
	S3DDVector	vExDeltaSpeed( 0, 0, 0 ) ;
	if ( secElapsed > 0.00001 )
	{
		S3DDVector	vExSpeed = vMoveDelta / secElapsed ;
		double		w = esl_fmin( secElapsed / 0.05, 0.5 ) ;
		iv.vExSpeedLPF = iv.vExSpeedLPF * (1.0 - w) + vExSpeed * w ;
		vExDeltaSpeed = iv.vExSpeedLPF - vExSpeed ;
	}
	//
	// ボーンに伴う加速
	//
	S3DDVector	vInertialDelta = vCurHandleGlobalPos - vMovedGlobalPos ;
	S3DDVector	vAcceleration = vExDeltaSpeed * mtrl.fpEffect
								+ vInertialDelta * (mtrl.fpHardness * frameRate) ;
	//
	// 移動と速度減衰
	//
	vMovedGlobalPos += iv.vSpeed * secElapsed ;
	iv.vSpeed += vAcceleration ;
	iv.vSpeed *= pow( 1.0 - mtrl.fpAttenuation, secElapsed * frameRate ) ;
	//
	// 制限角
	//
	S3DDVector	vMovedLocalPos =
					matCurSpace.Inverse() * (vMovedGlobalPos - vCurSpace) ;
	//
	double		r = vMovedLocalPos.Absolute() ;
	if ( r > 0 )
	{
		S3DDVector	vHandle = bone.GetBoneHandle() ;
		vHandle.Normalize() ;
		//
		S3DDVector	vPos = vMovedLocalPos / r ;
		//
		double	radLim = (180.0 - mtrl.fpLimitedAngle) * PI / 180.0 ;
		double	cosLim = cos( radLim ) ;
		double	cosCross = vHandle.InnerProduct( vPos ) ;
		if ( cosCross < cosLim )
		{
			//
			// 制限処理
			//
			S3DDVector	vOrth = vPos - vHandle * cosCross ;
			vOrth.Normalize() ;
			//
			vMovedLocalPos =
					vHandle * (cosLim * r)
						+ vOrth * (sin(radLim) * r) ;
			ESLAssert( !vMovedLocalPos.IsNaN() ) ;
			//
			vMovedGlobalPos = matCurSpace * vMovedLocalPos + vCurSpace ;
			//
			S3DDVector	vClippedDir = vMovedGlobalPos - vCurHandleGlobalPos ;
			vClippedDir.Normalize() ;
			iv.vSpeed -= vClippedDir * vClippedDir.InnerProduct( iv.vSpeed ) ;
		}
		//
		vMovedLocalPos.Normalize() ;
		vMovedLocalPos *= bone.GetBoneHandle().Absolute() ;
		iv.vPos = vMovedLocalPos ;
	}
	//
	// 空間パラメータ更新
	//
	iv.matLastSpace = matBone ;
	iv.vLastSpace = vBone ;
	//
	S3DDMatrix	matRotate( 1, 1, 1 ) ;
	matRotate.VectorRotationOf( bone.GetBoneHandle(), iv.vPos ) ;
	iv.matInertialBone = bone.m_matTransformation * matRotate ;
}

void S3DBasicInertialPoser::SetBoneRotationWithPhysics
	( S3DModelBoneSpace& bone,
		const S3DBasicInertialPoser::InertialVertex& iv )
{
	bone.m_matTransformation = iv.matInertialBone ;
}



//////////////////////////////////////////////////////////////////////////////
// 物理演算慣性ポーズ・コントローラー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DInertialPoseController, PoseConntoller )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DInertialPoseController, inertial_poser )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DInertialPoseController::S3DInertialPoseController( void )
	: PoseConntoller(m_ItemClassDescriptor.pwszClassID)
{
	ESLVerify( paramFreeAngle == AddParameterEntry
		( L"free_angle", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"角自由度[deg]", nullptr, 0.0, 30.0 ) ) ;
	ESLVerify( paramAttenuation == AddParameterEntry
		( L"attenuation", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"減衰率", L"1/60 秒あたりの減衰率（数値が大きいほど速く減速する）", 0.0, 1.0 ) ) ;
	ESLVerify( paramHardness == AddParameterEntry
		( L"hardness", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"弾性", L"1/60 秒あたりの復元力", 0.0, 1.0 ) ) ;
	ESLVerify( paramEffect == AddParameterEntry
		( L"effect", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"慣性影響度", nullptr, 0.0, 1.0 ) ) ;
	ESLVerify( paramBoneCount == AddParameterEntry
		( L"target_count", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrDynamicValidation,
			L"対象ボーン数", nullptr ) ) ;
}

// ターゲットボーン最大数変更（プロパティ設定）
//////////////////////////////////////////////////////////////////////////////
void S3DInertialPoseController::UpdateTargetBoneProperties( size_t nCount )
{
	const size_t	nLastBoneCount = GetTargetBoneCount() ;
	//
	SetTargetBoneCount( nCount ) ;
	//
	if ( nCount < nLastBoneCount )
	{
		ChopParameterEntryLastAt( paramBoneTarget0 - 1 + nCount ) ;
	}
	else if ( nCount > nLastBoneCount )
	{
		for ( size_t i = nLastBoneCount; i < nCount; i ++ )
		{
			SString *	pstrID = m_aBoneParamIDs.GetAt( i ) ;
			SString *	pstrDisp = m_aBoneParamDisps.GetAt( i ) ;
			if ( pstrID == nullptr )
			{
				pstrID = new SString ;
				pstrID->Format( L"target_bone%d", (int) i ) ;
				m_aBoneParamIDs.SetAt( i, pstrID ) ;
			}
			if ( pstrDisp == nullptr )
			{
				pstrDisp = new SString ;
				pstrDisp->Format( L"ボーン%d", (int) i ) ;
				m_aBoneParamDisps.SetAt( i, pstrDisp ) ;
			}
			ESLVerify( paramBoneTarget0 + i ==
				AddParameterEntry
					( *pstrID, S3DSceneComposer::typeCommand,
						S3DSceneComposer::attrConstant
							| S3DSceneComposer::attrStringEnumeration,
						*pstrDisp, nullptr ) ) ;
		}
	}
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
double S3DInertialPoseController::GetScalarParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramFreeAngle:
		return	m_degFreeAngle ;

	case	paramAttenuation:
		return	m_fpAttenuation ;

	case	paramHardness:
		return	m_fpHardness ;

	case	paramEffect:
		return	m_fpEffect ;
	}
	return	PoseConntoller::GetScalarParameter( iParam ) ;
}

int32_t S3DInertialPoseController::GetIntegerParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramBoneCount:
		return	(int32_t) GetTargetBoneCount() ;
	}
	return	PoseConntoller::GetIntegerParameter( iParam ) ;
}

const wchar_t * S3DInertialPoseController::GetCommandParameter( size_t iParam ) const
{
	if ( iParam >= paramBoneTarget0 )
	{
		return	GetTargetBoneNameAt( iParam - paramBoneTarget0 ) ;
	}
	return	PoseConntoller::GetCommandParameter( iParam ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DInertialPoseController::SetScalarParameter( size_t iParam, double s )
{
	switch ( iParam )
	{
	case	paramFreeAngle:
		m_degFreeAngle = s ;
		return ;

	case	paramAttenuation:
		m_fpAttenuation = s ;
		return ;

	case	paramHardness:
		m_fpHardness = s ;
		return ;

	case	paramEffect:
		m_fpEffect = s ;
		return ;
	}
	PoseConntoller::SetScalarParameter( iParam, s ) ;
}

void S3DInertialPoseController::SetIntegerParameter( size_t iParam, int32_t n )
{
	switch ( iParam )
	{
	case	paramBoneCount:
		UpdateTargetBoneProperties( (size_t) n ) ;
		return ;
	}
	PoseConntoller::SetIntegerParameter( iParam, n ) ;
}

void S3DInertialPoseController::SetCommandParameter( size_t iParam, const wchar_t * pwszCmd )
{
	if ( iParam >= paramBoneTarget0 )
	{
		SetTargetBoneNameAt( iParam - paramBoneTarget0, pwszCmd ) ;
		return ;
	}
	PoseConntoller::SetCommandParameter( iParam, pwszCmd ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DInertialPoseController::EnumerateStringSet
	( size_t iParam, SSystem::SStringArray& aStrSet )
{
	if ( iParam >= paramBoneTarget0 )
	{
		if ( m_pModel != nullptr )
		{
			SStrSortObjectArray<S3DModelBoneSpace>&
					ssoaBones = m_pModel->GetBonePropertyList() ;
			for ( size_t i = 0; i < ssoaBones.GetLength(); i ++ )
			{
				const SString *	pstrID = ssoaBones.GetTagAt( i ) ;
				if ( pstrID != nullptr )
				{
					aStrSet.Add( new SString( *pstrID ) ) ;
				}
			}
		}
		return	true ;
	}
	return	PoseConntoller::EnumerateStringSet( iParam, aStrSet ) ;
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DInertialPoseController::OnTimer
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast )
{
	PoseConntoller::OnTimer( scene, pItem, msecPast ) ;

	AddElapsedTime( (double) msecPast / 1000.0 ) ;
}

// フレーム（パラメータ）更新後処理
//////////////////////////////////////////////////////////////////////////////
void S3DInertialPoseController::OnUpdateFrame
	( S3DSceneComposer::ItemSerializer * pItem,
		double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	PoseConntoller::OnUpdateFrame( pItem, fpFrame, seek ) ;

	if ( seek == S3DSceneComposer::seekJumpReset )
	{
		SetResetFlag() ;
	}
}

// ポーズ適用
//////////////////////////////////////////////////////////////////////////////
void S3DInertialPoseController::OnPoseTrack
	( S3DDynamicModelSerializer& item, S3DModelBuffer& model )
{
	AttachTargetModel( &model ) ;
	CalculatePhysics( item, model ) ;
}

// モデル変更時の処理
//////////////////////////////////////////////////////////////////////////////
void S3DInertialPoseController::OnChangedModel
	( S3DDynamicModelSerializer& item, S3DModelBuffer * pModel )
{
	AttachTargetModel( pModel, true ) ;
}



//////////////////////////////////////////////////////////////////////////////
// 動的モデルアイテム（複数描画対応）
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DMultiModelSerializer::m_paramEntries
		[S3DMultiModelSerializer::paramMultiModelCount] =
{
	{ L"instancing",
		S3DSceneComposer::typeBinary,
		S3DSceneComposer::attrConstant3, L"インスタンス", nullptr },
	{ L"instance_rotate",
		S3DSceneComposer::typeRotation,
		S3DSceneComposer::attrConstant3, L"回転", nullptr },
	{ L"instance_zoom",
		S3DSceneComposer::typeZoom,
		S3DSceneComposer::attrConstant3, L"拡大", nullptr },
	{ L"collision_alpha",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant3, L"当たり判定α閾値",
		L"インスタンスのα値がこの値以上の時に当たり判定を有効にする" },
	{ L"force_face_method",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant3
		| S3DSceneComposer::attrStringEnumeration
		| S3DSceneComposer::attrUIOnlyEnumeration, L"カメラ連動回転", nullptr },
	{ L"sorting_method",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant3
		| S3DSceneComposer::attrStringEnumeration
		| S3DSceneComposer::attrUIOnlyEnumeration, L"ソート", nullptr },
	{ L"culling_method",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant3
		| S3DSceneComposer::attrStringEnumeration
		| S3DSceneComposer::attrUIOnlyEnumeration
		| S3DSceneComposer::attrDynamicValidation, L"カリング", nullptr },
	{ L"culling_near_z",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant3, L"カリング最近ｚ", nullptr },
	{ L"culling_far_z",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant3, L"カリング最遠ｚ", nullptr },
	{ L"culling_offset",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant3,
		L"カリングオフセット", L"カリングを行わない追加の遊び距離" },
	{ L"culling_angle_gap",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant3,
		L"カリング遊び角", L"カリングを行わない追加の遊び視野角[deg]" },
} ;

const S3DSceneComposer::ParamSetClass
	S3DMultiModelSerializer::m_pscClass =
{
	&S3DDynamicModelSerializer::m_pscClass,
	S3DMultiModelSerializer::paramMultiModelCount,
	&S3DMultiModelSerializer::m_paramEntries[0]
} ;

const SXMLDocument::AttrInteger
			S3DMultiModelSerializer::m_aiFaceMethod
				[S3DMultiModelSerializer::faceMethodCount+1] =
{
	{	L"no_operation", faceNoOperation	},
	{	L"rotation_y", faceRotateOnY	},
	{	L"billboard", faceBillboard	},
	{	nullptr, 0	},
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO3
	( SakuraGL::S3DMultiModelSerializer,
		S3DDynamicModelSerializer, RenderTarget, S3DInstancingItemInterface )
S3D_IMPLEMENT_COMPOSER_ITEM( S3DMultiModelSerializer, multi_model )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DMultiModelSerializer::S3DMultiModelSerializer( void )
	: S3DDynamicModelSerializer
		( m_ItemClassDescriptor.pwszClassID,
			&S3DMultiModelSerializer::m_pscClass )
{
	m_flagsBehavior |= S3DScene::itemOwnerBehavior ;
	m_maskClasses |= (1 << S3DScene::classPreRender2) ;
	m_nCollisionAlpha = 0xFF ;
	m_forceFace = faceNoOperation ;
	m_fpCullingOffset = 0.0 ;
	m_fpCullingAngle = 0.0 ;

	S4DMatrix	mat4Instance( 1, 1, 1, 1 ) ;
	S3DColor	clrInstance( 0xFFFFFFFF, 0 ) ;
	m_instancing.InsertStaticInstanceAt( 0, mat4Instance, clrInstance, this ) ;
	m_instancing.UpdateStaticInstancingEntriesBase64() ;
}

// 動的インスタンス追加（classPreRender で呼び出す）
//////////////////////////////////////////////////////////////////////////////
void S3DMultiModelSerializer::AddDynamicInstancingEntries
	( const S4DMatrix * pMatrixs,
		const S3DColor * pColors, size_t nCount, ESLObject * pSourceItem )
{
	m_instancing.Lock() ;
	m_instancing.AddDynamicInstancingEntries
				( pMatrixs, pColors, nCount, pSourceItem ) ;
	m_instancing.Unlock() ;
}

// S3DItemInstancingSerializer 取得
//////////////////////////////////////////////////////////////////////////////
S3DItemInstancingSerializer * S3DMultiModelSerializer::GetInstancing( void )
{
	return	&m_instancing ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DMultiModelSerializer::GetMatrixParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramInstanceRotate:
		return	S3DDMatrix( m_instancing.GetBaseRotation() ) ;
	}
	return	S3DDynamicModelSerializer::GetMatrixParameter( i ) ;
}

S3DDVector S3DMultiModelSerializer::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramInstanceZoom:
		return	m_instancing.GetBaseZoom() ;
	}
	return	S3DDynamicModelSerializer::GetVectorParameter( i ) ;
}

double S3DMultiModelSerializer::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramCullingNearZ:
		return	m_instancing.GetCullingNearZ() ;
	case	paramCullingFarZ:
		return	m_instancing.GetCullingFarZ() ;
	case	paramCullingOffset:
		return	m_fpCullingOffset ;
	case	paramCullingAngleGap:
		return	m_fpCullingAngle ;
	}
	return	S3DDynamicModelSerializer::GetScalarParameter( i ) ;
}

int32_t S3DMultiModelSerializer::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramCollisionAlpha:
		return	m_nCollisionAlpha ;
	}
	return	S3DDynamicModelSerializer::GetIntegerParameter( i ) ;
}

const wchar_t * S3DMultiModelSerializer::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramInstancing:
		return	m_instancing.GetInstancingEntriesBase64() ;
	case	paramForceFaceMethod:
		return	SXMLDocument::GetSymbolAsIntegerOf( m_aiFaceMethod, m_forceFace ) ;
	case	paramSortingMethod:
		return	m_instancing.GetSortingName() ;
	case	paramCullingMethod:
		return	m_instancing.GetCullingName() ;
	}
	return	S3DDynamicModelSerializer::GetCommandParameter( i ) ;
}

size_t S3DMultiModelSerializer::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t i ) const
{
	switch ( i )
	{
	case	paramInstancing:
		if ( pDst == nullptr )
		{
			return	sizeof(S3DSceneComposer::BinaryHeader)
					+ m_instancing.GetInstancingDataLengthInBytes() ;
		}
		if ( nBufBytes == sizeof(S3DSceneComposer::BinaryHeader) )
		{
			S3DSceneComposer::BinaryHeader *
				pbh = (S3DSceneComposer::BinaryHeader*) pDst ;
			pbh->nType = S3DSceneComposer::binaryInstancing ;
			pbh->nSubType = 0 ;
			pbh->nBodyBytes = (uint32_t) m_instancing.GetInstancingDataLengthInBytes() ;
			pbh->nReserved = 0 ;
			return	sizeof(S3DSceneComposer::BinaryHeader) ;
		}
		if ( nBufBytes == sizeof(S3DSceneComposer::BinaryHeader)
						+ m_instancing.GetInstancingDataLengthInBytes() )
		{
			S3DSceneComposer::BinaryHeader *
				pbh = (S3DSceneComposer::BinaryHeader*) pDst ;
			pbh->nType = S3DSceneComposer::binaryInstancing ;
			pbh->nSubType = 0 ;
			pbh->nBodyBytes = (uint32_t) m_instancing.GetInstancingDataLengthInBytes() ;
			pbh->nReserved = 0 ;
			//
			S3DSceneComposer::BinaryInstancingData *	pid =
				(S3DSceneComposer::BinaryInstancingData*) pbh->GetBodyPtr() ;
			return	sizeof(S3DSceneComposer::BinaryHeader)
						+ m_instancing.GetInstancingData( *pid ) ;
		}
		return	0 ;
	}
	return	S3DDynamicModelSerializer::GetBinaryParameter( pDst, nBufBytes, i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DMultiModelSerializer::SetMatrixParameter( size_t i, const S3DDMatrix& mat )
{
	switch ( i )
	{
	case	paramInstanceRotate:
		m_instancing.SetBaseRotation( S3DDQuaternion( mat ) ) ;
		return ;
	}
	S3DDynamicModelSerializer::SetMatrixParameter( i, mat ) ;
}

void S3DMultiModelSerializer::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramInstanceZoom:
		m_instancing.SetBaseZoom( vec ) ;
		return ;
	}
	S3DDynamicModelSerializer::SetVectorParameter( i, vec ) ;
}

void S3DMultiModelSerializer::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramCullingNearZ:
		m_instancing.SetCullingNearZ( (float32_t) s ) ;
		return ;
	case	paramCullingFarZ:
		m_instancing.SetCullingFarZ( (float32_t) s ) ;
		return ;
	case	paramCullingOffset:
		m_fpCullingOffset = s ;
		return ;
	case	paramCullingAngleGap:
		m_fpCullingAngle = s ;
		return ;
	}
	S3DDynamicModelSerializer::SetScalarParameter( i, s ) ;
}

void S3DMultiModelSerializer::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramCollisionAlpha:
		m_nCollisionAlpha = n ;
		return ;
	}
	S3DDynamicModelSerializer::SetIntegerParameter( i, n ) ;
}

void S3DMultiModelSerializer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramInstancing:
		m_instancing.SetInstancingEntriesBase64( pwszCmd ) ;
		return ;
	case	paramForceFaceMethod:
		m_forceFace = (ForceFaceMethod) SXMLDocument::GetIntegerAsSymbolOf
										( m_aiFaceMethod, pwszCmd, m_forceFace ) ;
		return ;
	case	paramSortingMethod:
		m_instancing.SetSortingByName( pwszCmd ) ;
		return ;
	case	paramCullingMethod:
		m_instancing.SetCullingByName( pwszCmd ) ;
		return ;
	}
	S3DDynamicModelSerializer::SetCommandParameter( i, pwszCmd ) ;
}

size_t S3DMultiModelSerializer::SetBinaryParameter
	( size_t i, const void * pSrc, size_t nBufBytes )
{
	switch ( i )
	{
	case	paramInstancing:
		{
			const S3DSceneComposer::BinaryHeader *
				pbh = (const S3DSceneComposer::BinaryHeader*) pSrc ;
			if ( pbh->nType == S3DSceneComposer::binaryInstancing )
			{
				const S3DSceneComposer::BinaryInstancingData *	pid =
					(const S3DSceneComposer::BinaryInstancingData*) pbh->GetBodyPtr() ;
				m_instancing.SetInstancingEntries( *pid ) ;
				//
				return	sizeof(S3DSceneComposer::BinaryHeader) + pbh->nBodyBytes ;
			}
		}
		return	0 ;
	}
	return	S3DDynamicModelSerializer::SetBinaryParameter( i, pSrc, nBufBytes ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DMultiModelSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	int	j ;
	switch ( i )
	{
	case	paramForceFaceMethod:
		for ( j = 0; j < faceMethodCount; j ++ )
		{
			aStrSet.Add( new SString( m_aiFaceMethod[j].pszSymbol ) ) ;
		}
		return	true ;
	case	paramSortingMethod:
		for ( j = 0; j < S3DItemInstancingSerializer::sortMethodCount; j ++ )
		{
			aStrSet.Add( new SString( S3DItemInstancingSerializer::m_pwszSortingMethodName[j] ) ) ;
		}
		return	true ;
	case	paramCullingMethod:
		for ( j = 0; j < S3DItemInstancingSerializer::CullingMethodCount; j ++ )
		{
			aStrSet.Add( new SString( S3DItemInstancingSerializer::m_pwszCullingMethodName[j] ) ) ;
		}
		return	true ;
	}
	return	S3DDynamicModelSerializer::EnumerateStringSet( i, aStrSet ) ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DMultiModelSerializer::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramVDrawTarget:
		return	false ;
	}
	return	S3DDynamicModelSerializer::IsParameterValidation( i ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DMultiModelSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	3:
		return	L"複数描画" ;
	}
	return	S3DDynamicModelSerializer::GetParameterCategoryName( iCategory ) ;
}

// 当たり判定追加
//////////////////////////////////////////////////////////////////////////////
void S3DMultiModelSerializer::RenderLocalCollision
	( const S3DScene& scene, S3DCollision& render )
{
	if ( m_maskColliderFlags == 0 )
	{
		return ;
	}
	const S4DMatrix *	pmatInstancing ;
	const S3DColor *	pclrInstancing ;
	if ( m_flagCollision )
	{
		render.SetUserClassesMask( m_maskColliderFlags ) ;
		render.BeginBatchBuild() ;
		//
		size_t	i ;
		size_t	nStaticInstanceCount =
			m_instancing.GetStaticInstancingArray
					( pmatInstancing, pclrInstancing ) ;
		for ( i = 0; i < nStaticInstanceCount; i ++ )
		{
			if ( (int32_t) pclrInstancing[i].rgbMul.argb.Alpha >= m_nCollisionAlpha )
			{
				render.AddColliderObject
					( &m_collisionMesh, pmatInstancing + i, i ) ;
			}
		}
		size_t	nDynamicInstanceCount =
			m_instancing.GetDynamicInstancingArray
					( pmatInstancing, pclrInstancing ) ;
		for ( i = 0; i < nDynamicInstanceCount; i ++ )
		{
			if ( (int32_t) pclrInstancing[i].rgbMul.argb.Alpha >= m_nCollisionAlpha )
			{
				render.AddColliderObject
					( &m_collisionMesh,
						pmatInstancing + i, nStaticInstanceCount + i ) ;
			}
		}
		render.EndBatchBuild() ;
	}
	else if ( m_pCollision != nullptr )
	{
		render.SetUserClassesMask( m_maskColliderFlags ) ;
		render.BeginBatchBuild() ;
		//
		size_t	nInstanceCount ;
		nInstanceCount =
			m_instancing.GetStaticInstancingArray
					( pmatInstancing, pclrInstancing ) ;
		if ( nInstanceCount > 0 )
		{
			m_pCollision->RenderBufferTo
				( &render, 0, 0, -1,
					nInstanceCount,
					pmatInstancing, pclrInstancing ) ;
		}
		nInstanceCount =
			m_instancing.GetDynamicInstancingArray
					( pmatInstancing, pclrInstancing ) ;
		if ( nInstanceCount > 0 )
		{
			m_pCollision->RenderBufferTo
				( &render, 0, 0, -1,
					nInstanceCount,
					pmatInstancing, pclrInstancing ) ;
		}
		render.EndBatchBuild() ;
	}
}

// 表示モデル追加
//////////////////////////////////////////////////////////////////////////////
void S3DMultiModelSerializer::RenderLocalModel
	( const S3DScene& scene,
		S3DRenderContextInterface& render, uint64_t flagsExclusion )
{
	if ( m_pModel == nullptr )
	{
		Item::RenderLocalModel( scene, render, flagsExclusion ) ;
		return ;
	}
	const S4DMatrix *			pmatStatic = nullptr ;
	const S4DMatrix *			pmatDynamic = nullptr ;
	const S3DColor *			pclrStatic = nullptr ;
	const S3DColor *			pclrDynamic = nullptr ;
	const S3DItemInstancingSerializer::SortIndex *
								pSortIndex = nullptr ;
	size_t						nStaticCount = 0,
								nDynamicCount = 0,
								nOpaqueCount = 0 ;
	bool						flagVDraw = (m_pModel->GetInstancingCount() > 0) ;
	//
	if ( m_instancing.IsNeededDynamicProcess() || flagVDraw )
	{
		pSortIndex = m_instancing.GetProcessedInstanceSortIndexArray
									( nDynamicCount, nOpaqueCount ) ;
		nDynamicCount = m_instancing.GetProcessedInstancingArray
									( pmatDynamic, pclrDynamic ) ;
	}
	else
	{
		nStaticCount = m_instancing.GetStaticInstancingArray
									( pmatStatic, pclrStatic ) ;
		nDynamicCount = m_instancing.GetDynamicInstancingArray
									( pmatDynamic, pclrDynamic ) ;
	}
	if ( (nStaticCount + nDynamicCount == 0) && !flagVDraw )
	{
		Item::RenderLocalModel( scene, render, flagsExclusion ) ;
		return ;
	}
	ShaderSaver	ss ;
	PrepareShaderSettings( scene, render, ss ) ;
	//
	if ( m_flagsModelRequest == 0 )
	{
		if ( nStaticCount > 0 )
		{
			RenderMultiInstance
				( scene, render,
					S3DItemInstancingSerializer::renderStaticInstance,
					flagsExclusion | m_flagsModelExclusion,
					m_iModelViewFirst, m_iModelViewEnd,
					nStaticCount, pmatStatic, pclrStatic ) ;
		}
		if ( nDynamicCount > 0 )
		{
			RenderMultiInstance
				( scene, render,
					S3DItemInstancingSerializer::renderDynamicInstance,
					flagsExclusion | m_flagsModelExclusion,
					m_iModelViewFirst, m_iModelViewEnd,
					nDynamicCount, pmatDynamic, pclrDynamic,
					pSortIndex, nOpaqueCount ) ;
		}
		if ( (nStaticCount + nDynamicCount == 0) && flagVDraw )
		{
			RenderMultiInstance
				( scene, render,
					S3DItemInstancingSerializer::renderVariantInstance,
					flagsExclusion | m_flagsModelExclusion ) ;
		}
	}
	else
	do
	{
		size_t	nMeshCount = m_pModel->GetMeshCount() ;
		if ( m_iModelViewFirst >= nMeshCount )
		{
			break ;
		}
		if ( (m_iModelViewEnd < (ssize_t) m_iModelViewFirst)
			|| (m_iModelViewEnd >= (ssize_t) nMeshCount) )
		{
			nMeshCount -= m_iModelViewFirst ;
		}
		else
		{
			nMeshCount =
				(size_t) m_iModelViewEnd - m_iModelViewFirst ;
		}
		S3DVertexBufferInterface::MeshInfo	inf ;
		eslFillMemory
			( &inf, 0, sizeof(S3DVertexBufferInterface::MeshInfo) ) ;
		size_t	iLast = m_iModelViewFirst ;
		for ( size_t i = 0; i < nMeshCount; i ++ )
		{
			size_t	iMesh = m_iModelViewFirst + i ;
			if ( m_pModel->GetMeshInfoAt( inf, iMesh, 0 )
				|| (inf.pMaterial == nullptr)
				|| !(inf.pMaterial->m_attrSurface.flagsShading
											& m_flagsModelRequest) )
			{
				if ( iLast < iMesh )
				{
					if ( nStaticCount > 0 )
					{
						RenderMultiInstance
							( scene, render,
								S3DItemInstancingSerializer::renderStaticInstance,
								flagsExclusion | m_flagsModelExclusion,
								iLast, (ssize_t) iMesh,
								nStaticCount, pmatStatic, pclrStatic ) ;
					}
					if ( nDynamicCount > 0 )
					{
						RenderMultiInstance
							( scene, render,
								S3DItemInstancingSerializer::renderDynamicInstance,
								flagsExclusion | m_flagsModelExclusion,
								iLast, (ssize_t) iMesh,
								nDynamicCount, pmatDynamic, pclrDynamic,
								pSortIndex, nOpaqueCount ) ;
					}
					if ( (nStaticCount + nDynamicCount == 0) && flagVDraw )
					{
						RenderMultiInstance
							( scene, render,
								S3DItemInstancingSerializer::renderVariantInstance,
								flagsExclusion | m_flagsModelExclusion,
								iLast, (ssize_t) iMesh ) ;
					}
				}
				iLast = iMesh + 1 ;
			}
		}
		size_t	iEndMesh = m_iModelViewFirst + nMeshCount ;
		if ( iLast < iEndMesh )
		{
			if ( nStaticCount > 0 )
			{
				RenderMultiInstance
					( scene, render,
						S3DItemInstancingSerializer::renderStaticInstance,
						flagsExclusion | m_flagsModelExclusion,
						iLast, (ssize_t) iEndMesh,
						nStaticCount, pmatStatic, pclrStatic ) ;
			}
			if ( nDynamicCount > 0 )
			{
				RenderMultiInstance
					( scene, render,
						S3DItemInstancingSerializer::renderDynamicInstance,
						flagsExclusion | m_flagsModelExclusion,
						iLast, (ssize_t) iEndMesh,
						nDynamicCount, pmatDynamic, pclrDynamic,
						pSortIndex, nOpaqueCount ) ;
			}
			if ( (nStaticCount + nDynamicCount == 0) && flagVDraw )
			{
				RenderMultiInstance
					( scene, render,
						S3DItemInstancingSerializer::renderVariantInstance,
						flagsExclusion | m_flagsModelExclusion,
						iLast, (ssize_t) iEndMesh ) ;
			}
		}
	}
	while ( false ) ;
	//
	RestoreShaderSettings( scene, render, ss ) ;
	//
	Item::RenderLocalModel( scene, render, flagsExclusion ) ;
}

void S3DMultiModelSerializer::RenderMultiInstance
	( const S3DScene& scene,
		S3DRenderContextInterface& render,
		S3DItemInstancingSerializer::RenderingType type,
		uint64_t flagsExclusion ,
		size_t iFirst, ssize_t iEnd, size_t nInstancing,
		const S4DMatrix * pmatInstancing,
		const S3DColor * pColorInstancing,
		const S3DItemInstancingSerializer::SortIndex * pSortIndexes, size_t nOpaqueCount )
{
	m_instancing.RenderMultiInstance
		( scene, render, *this, m_pModel, type,
			flagsExclusion, iFirst, iEnd,
			nInstancing, pmatInstancing, pColorInstancing,
			pSortIndexes, nOpaqueCount ) ;
}

// フレーム更新後処理
//////////////////////////////////////////////////////////////////////////////
void S3DMultiModelSerializer::OnUpdateFrame
			( double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	S3DDynamicModelSerializer::OnUpdateFrame( fpFrame, seek ) ;
	//
	if ( seek == S3DSceneComposer::seekJumpReset )
	{
		m_instancing.Lock() ;
		m_instancing.ResetDynamicInstancingEntries() ;
		m_instancing.UpdateDynamicInstancingEntries( this ) ;
		m_instancing.NotifyResetPotentialInstance( this ) ;
		m_instancing.Unlock() ;
	}
}

// アイテム作用の追加処理
//////////////////////////////////////////////////////////////////////////////
void S3DMultiModelSerializer::OnUpdateBehavior( S3DScene& scene )
{
	S3DDynamicModelSerializer::OnUpdateBehavior( scene ) ;
	//
	m_instancing.Lock() ;
	m_instancing.ResetDynamicInstancingEntries() ;
	m_instancing.UpdateDynamicInstancingEntries( this ) ;
	//
	if ( m_pModel != nullptr )
	{
		m_pModel->EnableMultiInstancingMode( true ) ;
		m_pModel->ClearAllInstance() ;
	}
	m_instancing.Unlock() ;
}

// レンダリング前後処理（全視点共通）
//////////////////////////////////////////////////////////////////////////////
void S3DMultiModelSerializer::OnRenderEvent
	( S3DScene& scene, S3DScene::ItemClass clsItem )
{
	S3DDynamicModelSerializer::OnRenderEvent( scene, clsItem ) ;
	//
	if ( (clsItem == S3DScene::classPreRender2) && (m_pModel != nullptr) )
	{
		bool	flagVDraw = (m_pModel->GetInstancingCount() > 0) ;
		m_instancing.Lock() ;
		if ( m_forceFace != faceNoOperation )
		{
			S3DMatrix	matICamera = scene.GetCurrentCameraIMatrix() ;
			m_instancing.ForceFaceDirection( true ) ;
			if ( m_forceFace == faceRotateOnY )
			{
				float32_t	r =
					(float32_t) sqrt( matICamera.m[0][2] * matICamera.m[0][2]
									+ matICamera.m[2][2] * matICamera.m[2][2] ) ;
				float32_t	s = matICamera.m[0][2] / r ;
				float32_t	c = matICamera.m[2][2] / r ;
				S3DMatrix	matDir( c, 0, s,  0, 1, 0,  -s, 0, c ) ;
				m_instancing.SetFaceDirection( matDir ) ;
			}
			else
			{
				m_instancing.SetFaceDirection( matICamera ) ;
			}
		}
		else
		{
			m_instancing.ForceFaceDirection( false ) ;
		}
		m_instancing.ResetInstanceIndex() ;
		//
		if ( m_instancing.IsNeededDynamicProcess() || flagVDraw )
		{
			S3DDMatrix	matdModel ;
			S3DDVector	vdModel ;
			GetGlobalTransformation( matdModel, vdModel );
			//
			S3DVector	vCenter ;
			float32_t	fpInstanceSize =
				(float32_t) m_pModel->GetCircumscribedSphere( vCenter ) ;
			//
			S3DVector	vZoom = m_instancing.GetBaseZoom() ;
			float32_t	zoom = esl_fmaxf( vZoom.x, esl_fmaxf( vZoom.y, vZoom.z ) ) ;
			//
			m_instancing.DoDynamicProcess
				( scene, matdModel, vdModel,
					(float32_t) m_fpCullingOffset
								+ fpInstanceSize * zoom * 2.0f,
					(float32_t) (m_fpCullingAngle * PI / 180.0) ) ;
		}
		m_instancing.Unlock() ;
	}
}

// Loquaty クラス名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DMultiModelSerializer::GetLQClassName( void ) const
{
	return	L"EntisGLS4.SceneMultiModel" ;
}

// アニメーション長取得
//////////////////////////////////////////////////////////////////////////////
bool S3DMultiModelSerializer::GetTargetAnimationLength( double& secLength ) const
{
	secLength = 0.0 ;
	return	false ;
}

// 全フレーム数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DMultiModelSerializer::GetTargetAnimationFrames( void ) const
{
	return	1 ;
}

// ターゲット空間（逆変換用）
//////////////////////////////////////////////////////////////////////////////
void S3DMultiModelSerializer::GetTargetSpaceTransformation
		( S3DDMatrix& matITarget, S3DDVector& vITarget )
{
	S3DDMatrix	matItem ;
	S3DDVector	vItem ;
	CalcGlobalTransformation( matItem, vItem ) ;
	//
	matITarget.InverseOf( matItem ) ;
	vITarget = matITarget * - vItem ;
}

// パーティクル追加
//////////////////////////////////////////////////////////////////////////////
void S3DMultiModelSerializer::AddParticles
	( size_t nCount,
		const S3DVector4 * pvPoints,
		const size_t * pFrames,
		const S3DColor * pSrcColors,
		const float32_t * pZooms,
		const S4DVector * pFaceDirs,
		const float32_t * pxAspect )
{
	if ( nCount == 0 )
	{
		return ;
	}
	m_instancing.Lock() ;
	//
	S4DMatrix *	pMatrix = m_aTempMatrixs.GetArray( nCount ) ;
	S3DColor *	pColor = m_aTempColors.GetArray( nCount ) ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		float32_t	zx = 1.0f, zy = 1.0f ;
		if ( pZooms != nullptr )
		{
			zx = pZooms[i] ;
			zy = zx ;
		}
		if ( pxAspect != nullptr )
		{
			zx *= pxAspect[i] ;
		}
		S3DMatrix	mat3( zx, zy, zy ) ;
		if ( pFaceDirs != nullptr )
		{
			mat3.InitializeMatrix( S3DVector( 1, 1, 1 ) ) ;
			mat3.RevolveForAngle( pFaceDirs[i] ) ;
			mat3.RevolveOnZ( sin(pFaceDirs[i].w), cos(pFaceDirs[i].w) ) ;
			mat3.MagnifyByVector( S3DVector( zx, zy, zy ) ) ;
		}
		S4DMatrix	mat4( 1, 1, 1, 1 ) ;
		mat4.SetMatrix3( mat3 ) ;
		mat4.SetTranslation( pvPoints[i] ) ;
		pMatrix[i] = mat4 ;
		//
		S3DColor	color( 0xFFFFFFFF, 0 ) ;
		if ( pSrcColors != nullptr )
		{
			color = pSrcColors[i] ;
		}
		pColor[i] = color ;
	}
	m_instancing.AddDynamicInstancingEntries
			( pMatrix, pColor, nCount, (ParameterProperty*) this ) ;
	//
	m_aTempMatrixs.FinishArray() ;
	m_aTempColors.FinishArray() ;
	m_instancing.Unlock() ;
}

// AddIndexedParticles を使うか？
//////////////////////////////////////////////////////////////////////////////
bool S3DMultiModelSerializer::IsUsingIndexedParticles( void )
{
	return	true ;
}

// パーティクル追加（高機能）（classPreRender で呼び出す）
//////////////////////////////////////////////////////////////////////////////
void S3DMultiModelSerializer::AddIndexedParticles
	( size_t nCount,
		const S3DVector4 * pvPoints,
		const S3DParticleSerializer::ParticleIndex * pIndexes,
		const size_t * pFrames,
		const S3DColor * pSrcColors,
		const S3DMatrix * pFaceDirs )
{
	if ( nCount == 0 )
	{
		return ;
	}
	m_instancing.Lock() ;
	//
	S4DMatrix *	pMatrix = m_aTempMatrixs.GetArray( nCount ) ;
	S3DColor *	pColor = m_aTempColors.GetArray( nCount ) ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		S4DMatrix	mat4( 1, 1, 1, 1 ) ;
		if ( pFaceDirs != nullptr )
		{
			mat4.SetMatrix3( pFaceDirs[i] ) ;
		}
		mat4.SetTranslation( pvPoints[pIndexes[i].nIndex] ) ;
		pMatrix[i] = mat4 ;
		//
		S3DColor	color( 0xFFFFFFFF, 0 ) ;
		if ( pSrcColors != nullptr )
		{
			color = pSrcColors[i] ;
		}
		pColor[i] = color ;
	}
	m_instancing.AddDynamicInstancingEntries
		( pMatrix, pColor, nCount, (ParameterProperty*) this ) ;
	//
	m_aTempMatrixs.FinishArray() ;
	m_aTempColors.FinishArray() ;
	m_instancing.Unlock() ;
}



//////////////////////////////////////////////////////////////////////////////
// LOD モデルインスタンス描画コントローラー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2( SakuraGL::S3DLODModelInstanceController, Controller, MultiRenderer )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DLODModelInstanceController, lod_instance_renderer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DLODModelInstanceController::S3DLODModelInstanceController( void )
	: Controller( m_ItemClassDescriptor.pwszClassID ),
		m_vModelScale( 1, 1, 1 ),
		m_fpLODDistance( 100.0 ),
		m_fpLODBothBand( 0.0 ),
		m_flagBillboard( false ),
		m_flagBillboardRotAlpha( false ),
		m_vBillboardTop( 0, -1, 0 ),
		m_vBillboardFront( 0, 0, -1 ),
		m_degBillboardDip( 45.0 )
{
	InitProperties() ;
}

S3DLODModelInstanceController::S3DLODModelInstanceController( const wchar_t * pwszClassID )
	: Controller( pwszClassID ),
		m_vModelScale( 1, 1, 1 ),
		m_fpLODDistance( 100.0 ),
		m_fpLODBothBand( 0.0 ),
		m_flagBillboard( false ),
		m_flagBillboardRotAlpha( false ),
		m_vBillboardTop( 0, -1, 0 ),
		m_vBillboardFront( 0, 0, -1 ),
		m_degBillboardDip( 45.0 )
{
	InitProperties() ;
}

// プロパティ設定
//////////////////////////////////////////////////////////////////////////////
void S3DLODModelInstanceController::InitProperties( void )
{
	ESLVerify( paramLODModel ==
		AddParameterEntry
			( L"lod_model",
				S3DSceneComposer::typeCommand,
				S3DSceneComposer::attrConstant
				| S3DSceneComposer::attrStringEnumeration, L"LOD モデル" ) ) ;
	ESLVerify( paramLODScale ==
		AddParameterEntry
			( L"lod_scale",
				S3DSceneComposer::typeZoom,
				S3DSceneComposer::attrConstant, L"LOD スケール" ) ) ;
	ESLVerify( paramDistance ==
		AddParameterEntry
			( L"lod_distance",
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrConstant, L"LOD 切り替え距離" ) ) ;
	ESLVerify( paramBothBand ==
		AddParameterEntry
			( L"lod_both_band",
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrConstant, L"LOD 切り替え猶予幅",
				L"LOD 切り替え距離以上でも通常表示と LOD 表示の両方を行う区間の幅を指定します" ) ) ;
	ESLVerify( paramBillboard ==
		AddParameterEntry
			( L"lod_billboard",
				S3DSceneComposer::typeBoolean,
				S3DSceneComposer::attrConstant, L"ビルボード" ) ) ;
	ESLVerify( paramBillboardRotAlpha ==
		AddParameterEntry
			( L"billboard_rot_alpha",
				S3DSceneComposer::typeBoolean,
				S3DSceneComposer::attrConstant,
				L"回転を加算色αへ反映",
				L"インスタンスの回転行列をインスタンス加算色α成分として設定\n"
				L"（回転に対応したビルボード・シェーダー等で使用）" ) ) ;
	ESLVerify( paramBillboardTop ==
		AddParameterEntry
			( L"billboard_top",
				S3DSceneComposer::typeDirection,
				S3DSceneComposer::attrConstant, L"ビルボード軸" ) ) ;
	ESLVerify( paramBillboardFront ==
		AddParameterEntry
			( L"billboard_front",
				S3DSceneComposer::typeDirection,
				S3DSceneComposer::attrConstant
				| S3DSceneComposer::attrGlobalTransform,
				L"ビルボード正面", L"ビルボードモデルのモデル座標空間での正面方向" ) ) ;
	ESLVerify( paramBillboardDip ==
		AddParameterEntry
			( L"billboard_dip",
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrConstant
				| S3DSceneComposer::attrUIScalarSlider,
				L"ビルボード有効俯角[deg]" ) ) ;
}

// モデル参照更新
//////////////////////////////////////////////////////////////////////////////
void S3DLODModelInstanceController::UpdateModelRef( void )
{
	S3DSceneComposer *	pComposer = GetComposer() ;
	S3DModelBuffer *	pModel = nullptr ;
	if ( !m_strLODModel.IsEmpty() )
	{
		if ( pComposer != nullptr )
		{
			pModel = pComposer->Assets().GetModelAs( m_strLODModel ) ;
		}
	}
	if ( pModel != nullptr )
	{
		m_modelRef.AttachModelReference( *pModel ) ;
		m_modelRef.AttachRelationItem( ESLTypeCast<S3DScene::Item>( GetOwnerItem() ) ) ;
		//
		m_modelRef.GetPoseLibrary().DetachAllReferenceLibrarys() ;
		if ( pComposer != nullptr )
		{
			m_modelRef.GetPoseLibrary().AddReferenceLibrary
					( &(pComposer->Assets().PoseLibrary()) ) ;
		}
	}
	else
	{
		m_modelRef.ClearBuffer() ;
		m_modelRef.GetPoseLibrary().DetachAllReferenceLibrarys() ;
	}
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DLODModelInstanceController::GetVectorParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramLODScale:
		return	m_vModelScale ;

	case	paramBillboardTop:
		return	m_vBillboardTop ;

	case	paramBillboardFront:
		return	m_vBillboardFront ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DLODModelInstanceController::GetScalarParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramDistance:
		return	m_fpLODDistance ;

	case	paramBothBand:
		return	m_fpLODBothBand ;

	case	paramBillboardDip:
		return	m_degBillboardDip ;
	}
	return	0.0 ;
}

bool S3DLODModelInstanceController::GetBooleanParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramBillboard:
		return	m_flagBillboard ;

	case	paramBillboardRotAlpha:
		return	m_flagBillboardRotAlpha ;
	}
	return	false ;
}

const wchar_t * S3DLODModelInstanceController::GetCommandParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramLODModel:
		return	m_strLODModel ;
	}
	return	nullptr ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DLODModelInstanceController::SetVectorParameter( size_t iParam, const S3DDVector& vec )
{
	switch ( iParam )
	{
	case	paramLODScale:
		m_vModelScale = vec ;
		return ;

	case	paramBillboardTop:
		m_vBillboardTop = vec ;
		return ;

	case	paramBillboardFront:
		m_vBillboardFront = vec ;
		return ;
	}
}

void S3DLODModelInstanceController::SetScalarParameter( size_t iParam, double s )
{
	switch ( iParam )
	{
	case	paramDistance:
		m_fpLODDistance = s ;
		return ;

	case	paramBothBand:
		m_fpLODBothBand = s ;
		return ;

	case	paramBillboardDip:
		m_degBillboardDip = s ;
		return ;
	}
}

void S3DLODModelInstanceController::SetBooleanParameter( size_t iParam, bool b )
{
	switch ( iParam )
	{
	case	paramBillboard:
		m_flagBillboard = b ;
		return ;

	case	paramBillboardRotAlpha:
		m_flagBillboardRotAlpha = b ;
		return ;
	}
}

void S3DLODModelInstanceController::SetCommandParameter( size_t iParam, const wchar_t * pwszCmd )
{
	switch ( iParam )
	{
	case	paramLODModel:
		if ( m_strLODModel != pwszCmd )
		{
			m_strLODModel = pwszCmd ;
			UpdateModelRef() ;
		}
		return ;
	}
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DLODModelInstanceController::EnumerateStringSet
	( size_t iParam, SSystem::SStringArray& aStrSet )
{
	switch ( iParam )
	{
	case	paramLODModel:
		{
			S3DSceneComposer *	pComposer = GetComposer() ;
			if ( pComposer != nullptr )
			{
				pComposer->GetAssets().EnumerateResourceIDsAs
					( aStrSet, ESL_RUNTIME_CLASS(S3DModelBuffer) ) ;
				pComposer->GetAssets().SortStringSet( aStrSet ) ;
			}
		}
		return	true ;
	}
	return	false ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DLODModelInstanceController::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags )
{
	uint32_t	nResFlags = 0 ;
	if ( nFlags & S3DSceneComposer::updateRefResource )
	{
		UpdateModelRef() ;
	}
	nResFlags |= Controller::UpdatePropertyReference( comp, pItem, nFlags ) ;
	return	nResFlags ;
}

// 描画処理
//////////////////////////////////////////////////////////////////////////////
S3DItemInstancingSerializer::RenderResult
	S3DLODModelInstanceController::RenderMultiInstance
		( const S3DScene& scene,
			S3DRenderContextInterface& render,
			const S3DItemInstancingSerializer& instancing,
			S3DItemInstancingSerializer::RenderingType type,
			S3DItemInstancingSerializer::RenderInfo& info )
{
	//
	// バッファ準備
	//
	m_aRenderMatrices.RemoveAll() ;
	m_aRenderColors.RemoveAll() ;
	m_aTempMatrices.RemoveAll() ;
	m_aTempColors.RemoveAll() ;
	m_aTempIndexes.RemoveAll() ;
	m_aRenderMatrices.SetLimit( info.nInstanceCount ) ;
	m_aRenderColors.SetLimit( info.nInstanceCount ) ;
	m_aTempMatrices.SetLimit( info.nInstanceCount ) ;
	m_aTempColors.SetLimit( info.nInstanceCount ) ;
	m_aTempIndexes.SetLimit( info.nInstanceCount ) ;

	//
	// LOD 判定のための準備
	//
	S3DDMatrix	matdItem( 1, 1, 1 ) ;
	S3DDVector	vdItem( 0, 0, 0 ) ;
	S3DSceneComposer::ItemSerializer *	pItem = GetOwnerItem() ;
	if ( pItem != nullptr )
	{
		pItem->GetGlobalTransformation( matdItem, vdItem ) ;
	}
	S3DMatrix	matItem = matdItem ;
	S3DMatrix	matIItem = matItem.Inverse() ;
	S3DVector	vItem = vdItem ;
	S3DVector	vCamera = scene.GetCurrentCameraPosition() ;
	S3DVector	vBillboardTop = (matdItem * m_vBillboardTop).Normalized() ;
	S3DVector	vBillboardFront = m_vBillboardFront.Normalized() ;

	//
	// LOD 判定
	//
	const S4DMatrix *	pInstanceMatrices = info.pInstanceMatrices ;
	const S3DColor *	pInstanceColors = info.pInstanceColors ;
	const S3DItemInstancingSerializer::SortIndex *
						pInstanceSortIndexes = info.pInstanceSortIndexes ;
	const float32_t		fpLOD2 = (float32_t) (m_fpLODDistance * m_fpLODDistance) ;
	const float32_t		fpNoLOD2 = (float32_t) ((m_fpLODDistance + m_fpLODBothBand)
												* (m_fpLODDistance + m_fpLODBothBand)) ;
	for ( size_t i = 0; i < info.nRangeCount; i ++ )
	{
		S3DItemInstancingSerializer::InstanceRange	range = info.pInstanceRanges[i] ;
		for ( size_t j = 0; j < range.nCount; j ++ )
		{
			S3DVector	vInstance =
				matItem * pInstanceMatrices[range.nIndex + j].GetTranslation() + vItem ;
			vInstance -= vCamera ;
			//
			bool	flagLOD = false ;
			float32_t	d2 = vInstance.InnerProduct( vInstance ) ;
			if ( d2 > fpLOD2 )
			{
				if ( m_flagBillboard )
				{
					if ( fabs( vBillboardTop.InnerProduct( vInstance ) / sqrt(d2) )
											< cos( m_degBillboardDip * PI / 180.0 ) )
					{
						flagLOD = true ;
					}
				}
				else
				{
					flagLOD = true ;
				}
			}
			if ( flagLOD )
			{
				m_aRenderMatrices.Add( pInstanceMatrices[range.nIndex + j] ) ;
				m_aRenderColors.Add( pInstanceColors[range.nIndex + j] ) ;
			}
			if ( !flagLOD || (d2 < fpNoLOD2) )
			{
				m_aTempMatrices.Add( pInstanceMatrices[range.nIndex + j] ) ;
				m_aTempColors.Add( pInstanceColors[range.nIndex + j] ) ;
				if ( pInstanceSortIndexes != nullptr )
				{
					m_aTempIndexes.Add( pInstanceSortIndexes[range.nIndex + j] ) ;
				}
			}
		}
	}
	if ( m_aRenderMatrices.GetLength() > 0 )
	{
		//
		// LOD 描画
		//
		const size_t	nInstanceCount = m_aRenderMatrices.GetLength() ;
		ESLAssert( nInstanceCount == m_aRenderColors.GetLength() ) ;
		S4DMatrix *	pMatrices = m_aRenderMatrices.GetArray() ;
		S3DColor *	pColors = m_aRenderColors.GetArray() ;
		for ( size_t i = 0; i < nInstanceCount; i ++ )
		{
			if ( m_flagBillboard )
			{
				S3DMatrix	matInstance = pMatrices[i].GetMatrix3() ;
				//
				// 軸周りの回転を頂点加算色のα要素に設定
				//
				if ( m_flagBillboardRotAlpha )
				{
					S3DMatrix	matNormalDelta ;
					matNormalDelta.VectorRotationOf
						( matInstance * S3DVector( 0, -1, 0 ), S3DVector( 0, -1, 0 ) ) ;
					S3DMatrix	matNormalBillboard = matNormalDelta * matInstance ;
					S3DVector	vNormal = matNormalBillboard * vBillboardFront ;
					float32_t	rad = (float32_t) - atan2( vNormal.x, vNormal.z ) ;
					const int	alpha =
						esl_clampi( esl_roundfi
							( (rad / (float32_t) PI * 0.5f + 0.5f ) * 255.0f ), 0, 255 ) ;
					pColors[i].rgbAdd.argb.Alpha = (uint8_t) alpha ;
				}
				//
				// 行列の拡大率を反映したビルボード行列を設定
				//
				float32_t	zx =
					(float32_t) sqrt( matInstance.m[0][0] * matInstance.m[0][0]
									+ matInstance.m[1][0] * matInstance.m[1][0]
									+ matInstance.m[2][0] * matInstance.m[2][0] ) ;
				float32_t	zy =
					(float32_t) sqrt( matInstance.m[0][1] * matInstance.m[0][1]
									+ matInstance.m[1][1] * matInstance.m[1][1]
									+ matInstance.m[2][1] * matInstance.m[2][1] ) ;
				float32_t	zz =
					(float32_t) sqrt( matInstance.m[0][2] * matInstance.m[0][2]
									+ matInstance.m[1][2] * matInstance.m[1][2]
									+ matInstance.m[2][2] * matInstance.m[2][2] ) ;
				//
				S3DVector	vCameraFront = vCamera - matItem * pMatrices[i].GetTranslation() ;
				S3DVector	vModelFront = vBillboardFront ;
				vModelFront -= vBillboardTop * vBillboardTop.InnerProduct( vModelFront ) ;
				vCameraFront -= vBillboardTop * vBillboardTop.InnerProduct( vCameraFront ) ;
				//
				S3DMatrix	matBillboardFace ;
				matBillboardFace.VectorRotationOf( vModelFront, vCameraFront ) ;
				//
				pMatrices[i].SetMatrix3( matBillboardFace ) ;
				pMatrices[i].Scale
					( m_vModelScale.x * zx, m_vModelScale.y * zy, m_vModelScale.z * zz ) ;
			}
			else
			{
				pMatrices[i].Scale( m_vModelScale.x, m_vModelScale.y, m_vModelScale.z ) ;
			}
		}
		m_aRenderMatrices.FinishArray() ;
		m_aRenderColors.FinishArray() ;
		//
		OnRenderLODInstance
			( scene, render, info.pModel,
				info.flagsExclusion,
				info.iMeshFirst, info.iMeshEnd,
				m_aRenderMatrices.GetLength(),
				m_aRenderMatrices.GetConstArray(),
				m_aRenderColors.GetConstArray() ) ;
	}
	//
	// 以降の描画のための情報更新
	//
	info.nInstanceCount = m_aTempMatrices.GetLength() ;
	info.pInstanceMatrices = m_aTempMatrices.GetConstArray() ;
	info.pInstanceColors = m_aTempColors.GetConstArray() ;
	if ( pInstanceSortIndexes != nullptr )
	{
		info.pInstanceSortIndexes = m_aTempIndexes.GetConstArray() ;
	}
	info.nRangeCount = 1 ;
	info.pInstanceRanges[0].nIndex = 0 ;
	info.pInstanceRanges[0].nCount = info.nInstanceCount ;
	//
	if ( info.nInstanceCount == 0 )
	{
		return	S3DItemInstancingSerializer::renderFinished ;
	}
	return	S3DItemInstancingSerializer::renderContinue ;
}

// LOD 描画
//////////////////////////////////////////////////////////////////////////////
void S3DLODModelInstanceController::OnRenderLODInstance
	( const S3DScene& scene,
		S3DRenderContextInterface& render,
		S3DVertexBufferInterface * pOrgModel,
		uint64_t flagsExclusion,
		size_t iMeshFirst, ssize_t iMeshEnd,
		size_t nInstanceCount,
		const S4DMatrix * pInstanceMatrices,
		const S3DColor * pInstanceColors )
{
	m_modelRef.RenderBufferTo
		( &render, flagsExclusion, iMeshFirst, iMeshEnd,
			nInstanceCount, pInstanceMatrices, pInstanceColors ) ;
}



//////////////////////////////////////////////////////////////////////////////
// LOD モデル用ユーザーシェーダー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DLODModelShaderController, S3DLODModelInstanceController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DLODModelShaderController, lod_user_shader )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DLODModelShaderController::S3DLODModelShaderController( void )
	: S3DLODModelInstanceController( m_ItemClassDescriptor.pwszClassID ),
		S3DUserShaderSerializer( paramFirstUniform )
{
	ESLVerify( paramShaderID == AddParameterEntry
		( L"shader_id", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration
			| S3DSceneComposer::attrDynamicValidation,
			L"シェーダ―", nullptr ) ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DLODModelShaderController::~S3DLODModelShaderController( void )
{
}

// シェーダ―更新
//////////////////////////////////////////////////////////////////////////////
void S3DLODModelShaderController::UpdateShaderReference( bool flagSwitchShader )
{
	S3DSceneComposer *				pComposer = GetComposer() ;
	S3DSceneComposer::UserShader *	pUserShader = nullptr ;
	if ( (pComposer != nullptr) && !m_strPropShaderID.IsEmpty() )
	{
		pUserShader = pComposer->GetAssets().GetUserShaderAs( m_strPropShaderID ) ;
	}
	UpdateShaderProperty
		( *this, pComposer, m_strPropShaderID,
			pUserShader, S3DSceneComposer::attrCategory1, flagSwitchShader ) ;
}

// パラメータ保存
//////////////////////////////////////////////////////////////////////////////
void S3DLODModelShaderController::SaveShaderParameters
	( SSystem::SStrSortObjectArray
			<S3DSceneComposer::ParameterEntryStorage>& ssoaParam )
{
	S3DUserShaderSerializer::SaveShaderParameters( *this, ssoaParam ) ;
}

// パラメータ復帰
//////////////////////////////////////////////////////////////////////////////
void S3DLODModelShaderController::ResotreShaderParameters
	( const SSystem::SStrSortObjectArray
			<S3DSceneComposer::ParameterEntryStorage>& ssoaParam )
{
	S3DUserShaderSerializer::ResotreShaderParameters( *this, ssoaParam ) ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DLODModelShaderController::GetMatrixParameter( size_t iParam ) const
{
	if ( iParam < S3DLODModelInstanceController::paramCount )
	{
		return	S3DLODModelInstanceController::GetMatrixParameter( iParam ) ;
	}
	return	S3DUserShaderSerializer::GetMatrixParameter( iParam ) ;
}

S3DDVector S3DLODModelShaderController::GetVectorParameter( size_t iParam ) const
{
	if ( iParam < S3DLODModelInstanceController::paramCount )
	{
		return	S3DLODModelInstanceController::GetVectorParameter( iParam ) ;
	}
	return	S3DUserShaderSerializer::GetVectorParameter( iParam ) ;
}

double S3DLODModelShaderController::GetScalarParameter( size_t iParam ) const
{
	if ( iParam < S3DLODModelInstanceController::paramCount )
	{
		return	S3DLODModelInstanceController::GetScalarParameter( iParam ) ;
	}
	return	S3DUserShaderSerializer::GetScalarParameter( iParam ) ;
}

int32_t S3DLODModelShaderController::GetIntegerParameter( size_t iParam ) const
{
	if ( iParam < S3DLODModelInstanceController::paramCount )
	{
		return	S3DLODModelInstanceController::GetIntegerParameter( iParam ) ;
	}
	return	S3DUserShaderSerializer::GetIntegerParameter( iParam ) ;
}

const wchar_t * S3DLODModelShaderController::GetCommandParameter( size_t iParam ) const
{
	if ( iParam < S3DLODModelInstanceController::paramCount )
	{
		return	S3DLODModelInstanceController::GetCommandParameter( iParam ) ;
	}
	else if ( iParam == paramShaderID )
	{
		return	m_strPropShaderID ;
	}
	return	S3DUserShaderSerializer::GetCommandParameter( iParam ) ;
}

size_t S3DLODModelShaderController::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t iParam ) const
{
	if ( iParam < S3DLODModelInstanceController::paramCount )
	{
		return	S3DLODModelInstanceController::GetBinaryParameter( pDst, nBufBytes, iParam ) ;
	}
	return	S3DUserShaderSerializer::GetBinaryParameter( pDst, nBufBytes, iParam ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DLODModelShaderController::SetMatrixParameter( size_t iParam, const S3DDMatrix& mat )
{
	if ( iParam < S3DLODModelInstanceController::paramCount )
	{
		S3DLODModelInstanceController::SetMatrixParameter( iParam, mat ) ;
		return ;
	}
	S3DUserShaderSerializer::SetMatrixParameter( iParam, mat ) ;
}

void S3DLODModelShaderController::SetVectorParameter( size_t iParam, const S3DDVector& vec )
{
	if ( iParam < S3DLODModelInstanceController::paramCount )
	{
		S3DLODModelInstanceController::SetVectorParameter( iParam, vec ) ;
		return ;
	}
	S3DUserShaderSerializer::SetVectorParameter( iParam, vec ) ;
}

void S3DLODModelShaderController::SetScalarParameter( size_t iParam, double s )
{
	if ( iParam < S3DLODModelInstanceController::paramCount )
	{
		S3DLODModelInstanceController::SetScalarParameter( iParam, s ) ;
		return ;
	}
	S3DUserShaderSerializer::SetScalarParameter( iParam, s ) ;
}

void S3DLODModelShaderController::SetIntegerParameter( size_t iParam, int32_t n )
{
	if ( iParam < S3DLODModelInstanceController::paramCount )
	{
		S3DLODModelInstanceController::SetIntegerParameter( iParam, n ) ;
		return ;
	}
	S3DUserShaderSerializer::SetIntegerParameter( iParam, n ) ;
}

void S3DLODModelShaderController::SetCommandParameter( size_t iParam, const wchar_t * pwszCmd )
{
	if ( iParam < S3DLODModelInstanceController::paramCount )
	{
		S3DLODModelInstanceController::SetCommandParameter( iParam, pwszCmd ) ;
		return ;
	}
	else if ( iParam == paramShaderID )
	{
		if ( m_strPropShaderID != pwszCmd )
		{
			m_strPropShaderID = pwszCmd ;
			UpdateShaderReference( true ) ;
		}
		return ;
	}
	S3DUserShaderSerializer::SetCommandParameter( GetComposer(), iParam, pwszCmd ) ;
}

size_t S3DLODModelShaderController::SetBinaryParameter
	( size_t iParam, const void * pSrc, size_t nBufBytes )
{
	if ( iParam < S3DLODModelInstanceController::paramCount )
	{
		return	S3DLODModelInstanceController::SetBinaryParameter( iParam, pSrc, nBufBytes ) ;
	}
	return	S3DUserShaderSerializer::SetBinaryParameter( iParam, pSrc, nBufBytes ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DLODModelShaderController::EnumerateStringSet
	( size_t iParam, SSystem::SStringArray& aStrSet )
{
	if ( iParam < S3DLODModelInstanceController::paramCount )
	{
		return	S3DLODModelInstanceController::EnumerateStringSet( iParam, aStrSet ) ;
	}
	if ( iParam == paramShaderID )
	{
		S3DSceneComposer *	pComposer = GetComposer() ;
		if ( pComposer != nullptr )
		{
			pComposer->GetAssets().EnumerateResourceIDsAs
				( aStrSet, S3DSceneComposer::resourceTypeShaderDef ) ;
		}
		return	true ;
	}
	return	S3DUserShaderSerializer::EnumerateStringSet( GetComposer(), iParam, aStrSet ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DLODModelShaderController::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
	default:
		return	S3DLODModelInstanceController::GetParameterCategoryName( iCategory ) ;
	case	1:
		return	L"シェーダ―設定" ;
	}
	return	nullptr ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DLODModelShaderController::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags )
{
	uint32_t	nResFlags =
		S3DLODModelInstanceController::UpdatePropertyReference( comp, pItem, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefResource )
	{
		SStrSortObjectArray<S3DSceneComposer::ParameterEntryStorage>	ssoaParam ;
		SaveShaderParameters( ssoaParam ) ;
		//
		UpdateShaderReference( false ) ;
		//
		ResotreShaderParameters( ssoaParam ) ;
	}
	return	nResFlags ;
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DLODModelShaderController::OnTimer
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast )
{
	S3DLODModelInstanceController::OnTimer( scene, pItem, msecPast ) ;

	S3DUserShaderSerializer::OnTimer( (double) msecPast / 1000.0 ) ;
}

// LOD 描画
//////////////////////////////////////////////////////////////////////////////
void S3DLODModelShaderController::OnRenderLODInstance
	( const S3DScene& scene,
		S3DRenderContextInterface& render,
		S3DVertexBufferInterface * pOrgModel,
		uint64_t flagsExclusion,
		size_t iMeshFirst, ssize_t iMeshEnd,
		size_t nInstanceCount,
		const S4DMatrix * pInstanceMatrices,
		const S3DColor * pInstanceColors )
{
	render.PushTransformation() ;
	AttachShaderParamTo( render, GetComposer() ) ;
	S3DLODModelInstanceController::OnRenderLODInstance
		( scene, render, pOrgModel, flagsExclusion,
			iMeshFirst, iMeshEnd,
			nInstanceCount, pInstanceMatrices, pInstanceColors ) ;
	render.PopTransformation() ;
}




//////////////////////////////////////////////////////////////////////////////
// モデル描画順序制御コントローラー
//////////////////////////////////////////////////////////////////////////////

const SXMLDocument::AttrInteger
	S3DModelPartialRenderController::m_xaiFlagPairs[14] =
{
	{ L"nothing", 0 },
	{ L"no_zbuffer", shadingNoZBuffer },
	{ L"no_zbuffer", shadingNoZBuffer },
	{ L"no_write_zbuffer", shadingZBufferNoWrite },
	{ L"no_shadow_object", shadingNoShadowObject },
	{ L"no_drop_shadow", shadingNoDropShadow },
	{ L"no_reflect_object", shadingNoReflectObject },
	{ L"global_reflect_object", shadingGlobalReflectObject },
	{ L"no_fog_effect", shadingNoFogEffect },
	{ L"app_extension1", (int64_t) shadingAppExtension1 },
	{ L"app_extension2", (int64_t) shadingAppExtension2 },
	{ L"app_extension3", (int64_t) shadingAppExtension3 },
	{ L"app_extension4", (int64_t) shadingAppExtension4 },
	{ nullptr, 0 },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DModelPartialRenderController, Controller )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DModelPartialRenderController, partial_renderer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DModelPartialRenderController::S3DModelPartialRenderController( void )
	: Controller( m_ItemClassDescriptor.pwszClassID ),
		m_clsRender( S3DScene::classEffectItem ),
		m_nPriority( 5 )
{
	m_flagsBehavior |= S3DSceneComposer::behaviorRender
							| S3DSceneComposer::behaviorRenderContext ;
	m_flagsEventClasses = (1 << m_clsRender) ;
	//
	m_flagPartial[0] = 0 ;
	m_flagPartial[1] = 0 ;
	m_flagPartial[2] = 0 ;
	m_flagPartial[3] = 0 ;
	m_flagsSaveExclusion = 0 ;
	m_flagsSaveRequest = 0 ;
	//
	ESLVerify( paramRenderClass ==
		AddParameterEntry
			( L"render_class", S3DSceneComposer::typeSelector,
				S3DSceneComposer::attrConstant
				| S3DSceneComposer::attrStringEnumeration
				| S3DSceneComposer::attrUIOnlyEnumeration, L"アイテムクラス" ) ) ;
	ESLVerify( paramRenderPriority ==
		AddParameterEntry
			( L"render_priority", S3DSceneComposer::typeInteger,
				S3DSceneComposer::attrConstant, L"プライオリティ" ) ) ;
	ESLVerify( paramPartialFlag1 ==
		AddParameterEntry
			( L"partial_flag0", S3DSceneComposer::typeSelector,
				S3DSceneComposer::attrConstant
				| S3DSceneComposer::attrStringEnumeration
				| S3DSceneComposer::attrUIOnlyEnumeration, L"対象フラグ[0]" ) ) ;
	ESLVerify( paramPartialFlag2 ==
		AddParameterEntry
			( L"partial_flag1", S3DSceneComposer::typeSelector,
				S3DSceneComposer::attrConstant
				| S3DSceneComposer::attrStringEnumeration
				| S3DSceneComposer::attrUIOnlyEnumeration, L"対象フラグ[1]" ) ) ;
	ESLVerify( paramPartialFlag3 ==
		AddParameterEntry
			( L"partial_flag2", S3DSceneComposer::typeSelector,
				S3DSceneComposer::attrConstant
				| S3DSceneComposer::attrStringEnumeration
				| S3DSceneComposer::attrUIOnlyEnumeration, L"対象フラグ[2]" ) ) ;
	ESLVerify( paramPartialFlag4 ==
		AddParameterEntry
			( L"partial_flag3", S3DSceneComposer::typeSelector,
				S3DSceneComposer::attrConstant
				| S3DSceneComposer::attrStringEnumeration
				| S3DSceneComposer::attrUIOnlyEnumeration, L"対象フラグ[3]" ) ) ;
}

// アイテムクラス
//////////////////////////////////////////////////////////////////////////////
S3DScene::ItemClass S3DModelPartialRenderController::GetRenderItemClass( void ) const
{
	return	m_clsRender ;
}

void S3DModelPartialRenderController::SetRenderItemClass( S3DScene::ItemClass clsItem )
{
	m_clsRender = clsItem ;
	m_flagsEventClasses = (1 << m_clsRender) ;
}

// 描画プライオリティ
//////////////////////////////////////////////////////////////////////////////
int32_t S3DModelPartialRenderController::GetRenderPriority( void ) const
{
	return	m_nPriority ;
}

void S3DModelPartialRenderController::SetRenderPriority( int32_t nPriority )
{
	m_nPriority = (int32_t) esl_clampi( nPriority, 0, 15 ) ;
}

// 選択フラグ
//////////////////////////////////////////////////////////////////////////////
uint64_t S3DModelPartialRenderController::GetPartialFlagAt( size_t i ) const
{
	ESLAssert( i < 4 ) ;
	return	m_flagPartial[i] ;
}

void S3DModelPartialRenderController::SetPartialFlagAt( size_t i, uint64_t flag )
{
	ESLAssert( i < 4 ) ;
	m_flagPartial[i] = flag ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
int32_t S3DModelPartialRenderController::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRenderPriority:
		return	m_nPriority ;
	}
	return	0 ;
}

const wchar_t * S3DModelPartialRenderController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRenderClass:
		return	S3DSceneComposer::ItemCommonSerializer::m_pwszItemClassIDs[m_clsRender] ;

	case	paramPartialFlag1:
		return	SXMLDocument::GetSymbolAsIntegerOf( m_xaiFlagPairs, m_flagPartial[0] ) ;

	case	paramPartialFlag2:
		return	SXMLDocument::GetSymbolAsIntegerOf( m_xaiFlagPairs, m_flagPartial[1] ) ;

	case	paramPartialFlag3:
		return	SXMLDocument::GetSymbolAsIntegerOf( m_xaiFlagPairs, m_flagPartial[2] ) ;

	case	paramPartialFlag4:
		return	SXMLDocument::GetSymbolAsIntegerOf( m_xaiFlagPairs, m_flagPartial[3] ) ;
	}
	return	nullptr ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DModelPartialRenderController::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramRenderPriority:
		m_nPriority = n ;
		return ;
	}
}

void S3DModelPartialRenderController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	size_t	j ;
	switch ( i )
	{
	case	paramRenderClass:
		for ( j = 0; j < S3DScene::classCount; j ++ )
		{
			if ( SString::Compare
				( pwszCmd, S3DSceneComposer::ItemCommonSerializer::m_pwszItemClassIDs[j] ) == 0 )
			{
				SetRenderItemClass( (S3DScene::ItemClass) j ) ;
				break ;
			}
		}
		return ;

	case	paramPartialFlag1:
		m_flagPartial[0] =
			SXMLDocument::GetIntegerAsSymbolOf
				( m_xaiFlagPairs, pwszCmd, m_flagPartial[0] ) ;
		return ;

	case	paramPartialFlag2:
		m_flagPartial[1] =
			SXMLDocument::GetIntegerAsSymbolOf
				( m_xaiFlagPairs, pwszCmd, m_flagPartial[1] ) ;
		return ;

	case	paramPartialFlag3:
		m_flagPartial[2] =
			SXMLDocument::GetIntegerAsSymbolOf
				( m_xaiFlagPairs, pwszCmd, m_flagPartial[2] ) ;
		return ;

	case	paramPartialFlag4:
		m_flagPartial[3] =
			SXMLDocument::GetIntegerAsSymbolOf
				( m_xaiFlagPairs, pwszCmd, m_flagPartial[3] ) ;
		return ;
	}
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DModelPartialRenderController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	size_t	j ;
	switch ( i )
	{
	case	paramRenderClass:
		for ( j = 0; j < S3DScene::classCount; j ++ )
		{
			aStrSet.Add( new SString
				( S3DSceneComposer::ItemCommonSerializer::m_pwszItemClassIDs[j] ) ) ;
		}
		return	true ;

	case	paramPartialFlag1:
	case	paramPartialFlag2:
	case	paramPartialFlag3:
	case	paramPartialFlag4:
		for ( j = 0; m_xaiFlagPairs[j].pszSymbol != nullptr; j ++ )
		{
			aStrSet.Add( new SString( m_xaiFlagPairs[j].pszSymbol ) ) ;
		}
		return	true ;
	}
	return	false ;
}

// 描画前処理
//////////////////////////////////////////////////////////////////////////////
void S3DModelPartialRenderController::BeforeRenderModel
	( const S3DScene& scene,
		S3DScene::ItemClass clsItem,
		S3DSceneComposer::ItemSerializer * pItem,
		S3DRenderContextInterface& render, uint64_t flagsExclusion )
{
	S3DScene::ModelItem *
		pModelItem = ESLTypeCast<S3DScene::ModelItem>( pItem ) ;
	ESLAssert( pModelItem != nullptr ) ;
	if ( pModelItem != nullptr )
	{
		m_flagsSaveExclusion = pModelItem->GetModelExclusionFlags() ;
		m_flagsSaveRequest = pModelItem->GetModelRequestFlags() ;
		//
		if ( clsItem == m_clsRender )
		{
			pModelItem->SetModelRequestFlags
				( m_flagPartial[0] | m_flagPartial[1]
					| m_flagPartial[2] | m_flagPartial[3] ) ;
		}
		else
		{
			pModelItem->SetModelExclusionFlags
				( m_flagsSaveExclusion
					| m_flagPartial[0] | m_flagPartial[1]
					| m_flagPartial[2] | m_flagPartial[3] ) ;
		}
	}
}

// 描画後処理
//////////////////////////////////////////////////////////////////////////////
void S3DModelPartialRenderController::AfterRenderModel
	( const S3DScene& scene,
		S3DScene::ItemClass clsItem,
		S3DSceneComposer::ItemSerializer * pItem,
		S3DRenderContextInterface& render, uint64_t flagsExclusion )
{
	S3DScene::ModelItem *
		pModelItem = ESLTypeCast<S3DScene::ModelItem>( pItem ) ;
	if ( pModelItem != nullptr )
	{
		pModelItem->SetModelExclusionFlags( m_flagsSaveExclusion ) ;
		pModelItem->SetModelRequestFlags( m_flagsSaveRequest ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// ビルボード表示アイテム
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DBillboardSerializer::m_paramEntries
		[S3DBillboardSerializer::paramBillboardCount] =
{
	{ L"image",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrCategory1,	L"画像", nullptr },
	{ L"mesh_target",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrConstant1,	L"メッシュ出力先", nullptr },
	{ L"billboard_center_x",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrUIScalarSlider,
		L"画像中心ｘ座標", L"0.0～1.0 範囲のｘ座標", 0.0, 1.0 },
	{ L"billboard_center_y",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrUIScalarSlider,
		L"画像中心ｙ座標", L"0.0～1.0 範囲のｙ座標", 0.0, 1.0 },
	{ L"billboard_zoom_x",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1, L"画像ｘ拡大率", nullptr },
	{ L"billboard_zoom_y",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,	L"画像ｙ拡大率", nullptr },
	{ L"billboard_angle",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,	L"画像回転角", nullptr },
	{ L"billboard_z_bias",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,	L"ｚバイアス", nullptr },
	{ L"billboard_z_operation",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration
			| S3DSceneComposer::attrCategory1,	L"ｚバッファ操作", nullptr },
	{ L"alpha_dithering",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,	L"αディザリング", nullptr },
	{ L"billboard_visible",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,	L"ビルボード表示", nullptr },
	{ L"billboard_auto_dir",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,	L"ビルボード常時正面", nullptr },
	{ L"billboard_normal",
		S3DSceneComposer::typeDirection,
		S3DSceneComposer::attrCategory1,	L"ビルボード向き", nullptr },
} ;

const S3DSceneComposer::ParamSetClass
	S3DBillboardSerializer::m_pscClass =
{
	&S3DSceneComposer::ItemCommonSerializer::m_pscClass,
	S3DBillboardSerializer::paramBillboardCount,
	&S3DBillboardSerializer::m_paramEntries[0]
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO3
	( SakuraGL::S3DBillboardSerializer,
		BillboardItem, ItemCommonSerializer, RenderTarget )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DBillboardSerializer, billboard )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DBillboardSerializer::S3DBillboardSerializer( void )
	: ItemCommonSerializer
			( m_ItemClassDescriptor.pwszClassID, &m_pscClass, nullptr ),
		m_vImageCenter( 0.5, 0.5 ),
		m_vImageZoom( 1.0, 1.0 )
{
	m_flagsBehavior |= S3DScene::itemOwnerBehavior ;
	//
	AttachSceneItem( (S3DScene::BillboardItem*) this ) ;
	//
	m_flagsZBuf = shadingZBufferNoWrite ;
	//
	m_visibleBillboard = true ;
	m_flagAutoNormal = true ;
	m_vBillboardNormal.x = 0 ;
	m_vBillboardNormal.y = 0 ;
	m_vBillboardNormal.z = -1 ;
}

// 出力先設定
//////////////////////////////////////////////////////////////////////////////
void S3DBillboardSerializer::AttachMeshTarget
	( S3DMeshBufferItemSerializer * pMesh, const wchar_t * pwszMeshID )
{
	m_strMeshTarget = pwszMeshID ;
	m_refMeshTarget.SetReference( (S3DScene::Item*) pMesh ) ;
	//
	if ( pMesh == nullptr )
	{
		m_maskClasses &= ~(1 << S3DScene::classPreRender2) ;
	}
	else
	{
		m_maskClasses |= (1 << S3DScene::classPreRender2) ;
	}
}

bool S3DBillboardSerializer::UpdateMeshTarget( void )
{
	if ( m_strMeshTarget.IsEmpty() )
	{
		m_refMeshTarget.SetReference( nullptr ) ;
		m_maskClasses &= ~(1 << S3DScene::classPreRender2) ;
		return	true ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		S3DSceneComposer::ItemSerializer *
			pItem = pComp->GetSceneItemAs( m_strMeshTarget ) ;
		if ( pItem != nullptr )
		{
			m_refMeshTarget.SetReference( pItem ) ;
			m_maskClasses |= (1 << S3DScene::classPreRender2) ;
			return	true ;
		}
	}
	return	false ;
}

// 画像設定
//////////////////////////////////////////////////////////////////////////////
void S3DBillboardSerializer::AttachBillboardImage
		( SGLImageObject * pImage, const wchar_t * pwszID )
{
	m_bp.pImage = pImage ;
	m_strImageID = pwszID ;
}

// 画像基準座標
//////////////////////////////////////////////////////////////////////////////
void S3DBillboardSerializer::SetBillboardCenter( const S2DVector& vCenter )
{
	m_vImageCenter = vCenter ;
}

// 画像拡大率
//////////////////////////////////////////////////////////////////////////////
void S3DBillboardSerializer::SetBillboardZoom( const S2DVector& vZoom )
{
	m_vImageZoom = vZoom ;
}

// 画像回転角 [deg]
//////////////////////////////////////////////////////////////////////////////
void S3DBillboardSerializer::SetBillboardAngle( float32_t degAngle )
{
	m_bp.zAngle = degAngle ;
}

// ｚバイアス
//////////////////////////////////////////////////////////////////////////////
void S3DBillboardSerializer::SetBillboardBiasZ( float32_t zBias )
{
	m_bp.zBias = zBias ;
}

// ビルボード表示
//////////////////////////////////////////////////////////////////////////////
void S3DBillboardSerializer::SetBillboardVisible( bool fVisible )
{
	m_visibleBillboard = fVisible ;
}

bool S3DBillboardSerializer::IsBillboardVisible( void ) const
{
	return	m_visibleBillboard ;
}

// ビルボード向き
//////////////////////////////////////////////////////////////////////////////
void S3DBillboardSerializer::SetBillboardAutoNormal( bool flagAuto )
{
	m_flagAutoNormal = flagAuto ;
}

bool S3DBillboardSerializer::IsBillboardAutoNormal( void ) const
{
	return	m_flagAutoNormal ;
}

void S3DBillboardSerializer::SetBillboardNormal( const S3DDVector& vNormal )
{
	m_vBillboardNormal = vNormal ;
}

const S3DDVector& S3DBillboardSerializer::GetBillboardNormal( void ) const
{
	return	m_vBillboardNormal ;
}

// 拡大率と画像中心座標を反映
//////////////////////////////////////////////////////////////////////////////
void S3DBillboardSerializer::UpdateBillboardCenterAndZoom( void )
{
	if ( m_bp.pImage != nullptr )
	{
		SGLSize	sizeImage = m_bp.pImage->GetImageSize() ;
		int	nImageSize = esl_max( sizeImage.w, sizeImage.h ) ;
		m_bp.vCenter.x = (float32_t) (m_vImageCenter.x * sizeImage.w) ;
		m_bp.vCenter.y = (float32_t) (m_vImageCenter.y * sizeImage.h) ;
		m_bp.vZoom.x = (float32_t) (m_vImageZoom.x / nImageSize) ;
		m_bp.vZoom.y = (float32_t) (m_vImageZoom.y / nImageSize) ;
	}
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DBillboardSerializer::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramBillboardNormal:
		return	m_vBillboardNormal ;
	}
	return	ItemCommonSerializer::GetVectorParameter( i ) ;
}

double S3DBillboardSerializer::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramBillboardCenterX:
		return	m_vImageCenter.x ;

	case	paramBillboardCenterY:
		return	m_vImageCenter.y ;

	case	paramBillboardZoomX:
		return	m_vImageZoom.x ;

	case	paramBillboardZoomY:
		return	m_vImageZoom.y ;

	case	paramBillboardAngle:
		return	m_bp.zAngle ;

	case	paramZBias:
		return	m_bp.zBias ;
	}
	return	ItemCommonSerializer::GetScalarParameter( i ) ;
}

bool S3DBillboardSerializer::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramAlphaDither:
		return	(m_flagsZBuf & shadingTextureDithering) != 0 ;

	case	paramBillboardVisile:
		return	m_visibleBillboard ;

	case	paramBillboardAutoNormal:
		return	m_flagAutoNormal ;
	}
	return	ItemCommonSerializer::GetBooleanParameter( i ) ;
}

const wchar_t * S3DBillboardSerializer::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramImage:
		return	m_strImageID ;

	case	paramMeshTarget:
		return	m_strMeshTarget ;

	case	paramZOperation:
		if ( m_flagsZBuf & shadingNoZBuffer )
		{
			return	L"no_z_operation" ;
		}
		else if ( m_flagsZBuf & shadingZBufferNoWrite )
		{
			return	L"no_z_write" ;
		}
		return	L"z_write" ;
	}
	return	ItemCommonSerializer::GetCommandParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DBillboardSerializer::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramBillboardNormal:
		m_vBillboardNormal = vec ;
		return ;
	}
	ItemCommonSerializer::SetVectorParameter( i, vec ) ;
}

void S3DBillboardSerializer::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramBillboardCenterX:
		m_vImageCenter.x = s ;
		return ;

	case	paramBillboardCenterY:
		m_vImageCenter.y = s ;
		return ;

	case	paramBillboardZoomX:
		m_vImageZoom.x = s ;
		return ;

	case	paramBillboardZoomY:
		m_vImageZoom.y = s ;
		return ;

	case	paramBillboardAngle:
		m_bp.zAngle = (float32_t) s ;
		return ;

	case	paramZBias:
		m_bp.zBias = (float32_t) s ;
		return ;
	}
	ItemCommonSerializer::SetScalarParameter( i, s ) ;
}

void S3DBillboardSerializer::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramAlphaDither:
		m_flagsZBuf &= ~shadingTextureDithering ;
		m_flagsZBuf |= (b ? shadingTextureDithering : 0) ;
		return ;

	case	paramBillboardVisile:
		m_visibleBillboard = b ;
		return ;

	case	paramBillboardAutoNormal:
		m_flagAutoNormal = b ;
		return ;
	}
	ItemCommonSerializer::SetBooleanParameter( i, b ) ;
}

void S3DBillboardSerializer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramImage:
		if ( m_strImageID != pwszCmd )
		{
			S3DSceneComposer::Composition *	pComp = GetComposition() ;
			SGLImageObject *	pImage = nullptr ;
			if ( pComp != nullptr )
			{
				S3DSceneComposer *	pSceneComp = pComp->GetSceneComposer() ;
				if ( pSceneComp != nullptr )
				{
					pImage = pSceneComp->Assets().GetImageAs( pwszCmd ) ;
				}
			}
			AttachBillboardImage( pImage, pwszCmd ) ;
		}
		return ;

	case	paramMeshTarget:
		if ( m_strMeshTarget != pwszCmd )
		{
			m_strMeshTarget = pwszCmd ;
			UpdateMeshTarget() ;
		}
		return ;

	case	paramZOperation:
		m_flagsZBuf &= ~(shadingNoZBuffer | shadingZBufferNoWrite) ;
		if ( SString::Compare( pwszCmd, L"no_z_operation" ) == 0 )
		{
			m_flagsZBuf |= shadingNoZBuffer ;
		}
		else if ( SString::Compare( pwszCmd, L"no_z_write" ) == 0 )
		{
			m_flagsZBuf |= shadingZBufferNoWrite ;
		}
		return ;
	}
	ItemCommonSerializer::SetCommandParameter( i, pwszCmd ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DBillboardSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	S3DSceneComposer::Composition *	pComp ;
	switch ( i )
	{
	case	paramImage:
		pComp = GetComposition() ;
		if ( pComp != nullptr )
		{
			S3DSceneComposer *	pSceneComp = pComp->GetSceneComposer() ;
			if ( pSceneComp != nullptr )
			{
				pSceneComp->Assets().EnumerateResourceIDsAs
					( aStrSet, ESL_RUNTIME_CLASS(SGLImageObject) ) ;
				pSceneComp->GetAssets().SortStringSet( aStrSet ) ;
			}
		}
		return	true ;

	case	paramMeshTarget:
		{
			S3DSceneComposer::Composition *	pComp = GetComposition() ;
			if ( pComp != nullptr )
			{
				pComp->EnumerateItemIDsAs
					( aStrSet, ESL_RUNTIME_CLASS(S3DMeshBufferItemSerializer) ) ;
			}
		}
		return	true ;

	case	paramZOperation:
		aStrSet.Add( new SString( L"no_z_operation" ) ) ;
		aStrSet.Add( new SString( L"no_z_write" ) ) ;
		aStrSet.Add( new SString( L"z_write" ) ) ;
		return	true ;
	}
	return	ItemCommonSerializer::EnumerateStringSet( i, aStrSet ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DBillboardSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"ビルボード設定" ;
	}
	return	ItemCommonSerializer::GetParameterCategoryName( iCategory ) ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DBillboardSerializer::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramZoom:
	case	paramColorMul:
	case	paramColorAdd:
	case	paramForceToon:
	case	paramForceBorder:
	case	paramFreeToon:
	case	paramFreeBorder:
	case	paramUseCollision:
	case	paramHideNear:
	case	paramHideFar:
		return	false ;
	}
	return	ItemCommonSerializer::IsParameterValidation( i ) ;
}

// レンダリング前後処理（全視点共通）
//////////////////////////////////////////////////////////////////////////////
void S3DBillboardSerializer::OnItemRenderEvent
	( S3DScene& scene, S3DScene::ItemClass clsItem )
{
	if ( (clsItem == S3DScene::classPreRender2)
		&& (m_bp.pImage != nullptr)
		&& !m_strMeshTarget.IsEmpty() )
	{
		S3DMeshBufferItemSerializer *	pMesh =
				ESLTypeCast<S3DMeshBufferItemSerializer>
							( m_refMeshTarget.GetReference() ) ;
		if ( pMesh != nullptr )
		{
			S3DVertexBufferInterface *	pVB = pMesh->GetVertexBuffer() ;
			if ( pVB != nullptr )
			{
				AddBillboardParticleMesh( scene, *pVB ) ;
			}
		}
	}
	ItemCommonSerializer::OnItemRenderEvent( scene, clsItem ) ;
}

void S3DBillboardSerializer::AddBillboardParticleMesh
			( S3DScene& scene, S3DVertexBufferInterface& vb )
{
	if ( (m_bp.pImage == nullptr)
		|| (m_bufPoints.GetLength() == 0) )
	{
		return ;
	}
	UpdateBillboardCenterAndZoom() ;
	//
	const size_t	nFrameCount = m_bp.pImage->GetFrameCount() ;
	SGLImageRect *	pFrameRect = m_meshShaper.GetImageRectBuffer( nFrameCount ) ;
	SGLImageObject *	pAnime =
			m_bp.pImage->NewAnimationReference( pFrameRect, nFrameCount ) ;
	if ( pAnime == nullptr )
	{
		SGLSize	sizeImage = m_bp.pImage->GetImageSize() ;
		for ( size_t i = 0; i < nFrameCount; i ++ )
		{
			pFrameRect[i].x = 0 ;
			pFrameRect[i].y = 0 ;
			pFrameRect[i].w = sizeImage.w ;
			pFrameRect[i].h = sizeImage.h ;
		}
	}
	else
	{
		delete	pAnime ;
	}
	const size_t	nPoints = m_bufPoints.GetLength() ;
	//
	S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
	vb.AllocatePrimitiveBuffer
		( prmbuf, primitiveTriangle, nPoints * 6, nPoints * 4 ) ;
	//
	S3DDMatrix	matSpace ;
	S3DDVector	vSpace ;
	CalcGlobalTransformation( matSpace, vSpace ) ;
	//
	S3DDMatrix	matISpace ;
	matISpace.InverseOf( matSpace ) ;
	//
	S3DMatrix	matICamera = matISpace * scene.GetCurrentCameraIMatrix() ;
	S3DVector	vCameraPos = scene.GetCurrentCameraPosition() ;
	//
	const S3DColor *	pColors = nullptr ;
	const float32_t *	pZooms = nullptr ;
	const S4DVector *	pFaceDirs = nullptr ;
	const float32_t *	pxAspects = nullptr ;
	if ( m_bufColors.GetLength() >= nPoints )
	{
		pColors = m_bufColors.GetConstArray() ;
	}
	if ( m_bufZooms.GetLength() >= nPoints )
	{
		pZooms = m_bufZooms.GetConstArray() ;
	}
	if ( m_bufFaceDirs.GetLength() >= nPoints )
	{
		pFaceDirs = m_bufFaceDirs.GetConstArray() ;
	}
	if ( m_bufXAspects.GetLength() >= nPoints )
	{
		pxAspects = m_bufXAspects.GetConstArray() ;
	}
	S3DMeshShaper::MakeAnimationBillboardParticle
		( prmbuf, matICamera, vCameraPos, m_bp,
			pFrameRect, nFrameCount, nPoints,
			m_bufPoints.GetConstArray(),
			m_bufFrames.GetConstArray(),
			pColors, pZooms, pFaceDirs, pxAspects ) ;
	//
	vb.AddPrimitiveBuffer
		( nullptr, 0, primitiveTriangle, prmbuf, nPoints * 6, nPoints * 4 ) ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DBillboardSerializer::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp, uint32_t nFlags )
{
	uint32_t	nResFlags =
		ItemCommonSerializer::UpdatePropertyReference( comp, nFlags ) ;
	//
	if ( (nFlags & S3DSceneComposer::updateRefResource)
		&& !m_strImageID.IsEmpty() )
	{
		S3DSceneComposer *	pSceneComp = comp.GetSceneComposer() ;
		SGLImageObject *	pImage = nullptr ;
		if ( pSceneComp != nullptr )
		{
			pImage = pSceneComp->Assets().GetImageAs( m_strImageID ) ;
		}
		AttachBillboardImage( pImage, m_strImageID ) ;
	}
	if ( nFlags & S3DSceneComposer::updateRefItem )
	{
		if ( !UpdateMeshTarget() )
		{
			nResFlags |= S3DSceneComposer::updateRefItem ;
		}
	}
	return	nResFlags ;
}

// シーン取得
//////////////////////////////////////////////////////////////////////////////
S3DScene * S3DBillboardSerializer::GetScene( void ) const
{
	return	ItemCommonSerializer::GetScene() ;
}

// タイマ処理 Item::OnTimer / ItemSerializer::OnTimer 実装
//////////////////////////////////////////////////////////////////////////////
void S3DBillboardSerializer::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	BillboardItem::OnTimer( scene, msecPast ) ;
	ItemCommonSerializer::OnTimer( scene, msecPast ) ;
}

// アイテム作用の追加処理
//////////////////////////////////////////////////////////////////////////////
void S3DBillboardSerializer::OnUpdateBehavior( S3DScene& scene )
{
	m_bufPoints.RemoveAll() ;
	m_bufFrames.RemoveAll() ;
	m_bufColors.RemoveAll() ;
	m_bufZooms.RemoveAll() ;
	m_bufXAspects.RemoveAll() ;
	m_bufFaceDirs.RemoveAll() ;
	//
	if ( m_visibleBillboard )
	{
		S3DVector4	vPos( 0, 0, 0 ) ;
		AddParticles( 1, &vPos ) ;
	}
}

// 表示モデル追加
//////////////////////////////////////////////////////////////////////////////
void S3DBillboardSerializer::RenderLocalModel
	( const S3DScene& scene,
		S3DRenderContextInterface& render,
		uint64_t flagsExclusion )
{
	if ( (m_bp.pImage != nullptr) && (m_strMeshTarget.IsEmpty()) )
	{
		UpdateBillboardCenterAndZoom() ;
		//
		BillboardItem::RenderLocalModel( scene, render, flagsExclusion ) ;
	}
}

// アニメーション長取得
//////////////////////////////////////////////////////////////////////////////
bool S3DBillboardSerializer::GetTargetAnimationLength( double& secLength ) const
{
	if ( m_bp.pImage == nullptr )
	{
		return	false ;
	}
	secLength = (double) m_bp.pImage->GetTotalTime() / 1000.0 ;
	return	true ;
}

// 全フレーム数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DBillboardSerializer::GetTargetAnimationFrames( void ) const
{
	if ( m_bp.pImage == nullptr )
	{
		return	0 ;
	}
	return	m_bp.pImage->GetFrameCount() ;
}

// ターゲット空間（逆変換用）
//////////////////////////////////////////////////////////////////////////////
void S3DBillboardSerializer::GetTargetSpaceTransformation
		( S3DDMatrix& matITarget, S3DDVector& vITarget )
{
	S3DDMatrix	matItem ;
	S3DDVector	vItem ;
	CalcGlobalTransformation( matItem, vItem ) ;
	//
	matITarget.InverseOf( matItem ) ;
	vITarget = matITarget * - vItem ;
}

// パーティクル追加
//////////////////////////////////////////////////////////////////////////////
void S3DBillboardSerializer::AddParticles
	( size_t nCount,
		const S3DVector4 * pvPoints,
		const size_t * pFrames,
		const S3DColor * pColors,
		const float32_t * pZooms,
		const S4DVector * pFaceDirs,
		const float32_t * pxAspect )
{
	if ( nCount == 0 )
	{
		return ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	//
	size_t	nBaseLength = m_bufPoints.GetLength() ;
	//
	ESLAssert( pvPoints != nullptr ) ;
	#if	defined(__DEBUG__)
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ESLAssert( !pvPoints[i].IsNaN() ) ;
	}
	#endif
	m_bufPoints.AddArray( pvPoints, nCount ) ;
	//
	if ( pFrames != nullptr )
	{
		m_bufFrames.SetLength( nBaseLength ) ;
		m_bufFrames.AddArray( pFrames, nCount ) ;
	}
	else
	{
		m_bufFrames.SetLength( nBaseLength + nCount ) ;
	}
	if ( pColors != nullptr )
	{
		if ( m_bufColors.GetLength() < nBaseLength )
		{
			FitColorArray( nBaseLength ) ;
		}
		m_bufColors.AddArray( pColors, nCount ) ;
	}
	else
	{
		FitColorArray( nBaseLength + nCount ) ;
	}
	if ( pZooms != nullptr )
	{
		if ( m_bufZooms.GetLength() < nBaseLength )
		{
			FitZoomArray( nBaseLength ) ;
		}
		m_bufZooms.AddArray( pZooms, nCount ) ;
	}
	else
	{
		FitZoomArray( nBaseLength + nCount ) ;
	}
	if ( pxAspect != nullptr )
	{
		if ( m_bufXAspects.GetLength() < nBaseLength )
		{
			FitAspectArray( nBaseLength ) ;
		}
		m_bufXAspects.AddArray( pxAspect, nCount ) ;
	}
	else if ( m_bufXAspects.GetLength() > 0 )
	{
		FitAspectArray( nBaseLength + nCount ) ;
	}
	if ( pFaceDirs != nullptr )
	{
		#if	defined(__DEBUG__)
		for ( size_t i = 0; i < nCount; i ++ )
		{
			ESLAssert( !pFaceDirs[i].IsNaN() ) ;
		}
		#endif
		if ( m_bufFaceDirs.GetLength() < nBaseLength )
		{
			FitFaceDirArray( nBaseLength ) ;
		}
		m_bufFaceDirs.AddArray( pFaceDirs, nCount ) ;
	}
	else
	{
		FitFaceDirArray( nBaseLength + nCount ) ;
	}
}

void S3DBillboardSerializer::FitFaceDirArray( size_t nLength )
{
	S4DVector	vFaceDir ;
	vFaceDir.x = (float32_t) m_vBillboardNormal.x ;
	vFaceDir.y = (float32_t) m_vBillboardNormal.y ;
	vFaceDir.z = (float32_t) m_vBillboardNormal.z ;
	vFaceDir.w = 0.0f ;
	//
	if ( m_flagAutoNormal )
	{
		S3DScene::Camera *	pCamera = nullptr ;
		S3DScene *	pScene = GetScene() ;
		if ( pScene != nullptr )
		{
			pCamera = pScene->GetCurrentCamera() ;
			if ( pCamera == nullptr )
			{
				pCamera = pScene->GetMainCamera() ;
			}
		}
		if ( pCamera != nullptr )
		{
			S3DDMatrix	matLinkCamera ;
			S3DDVector	vLinkCamera ;
			pCamera->CalcItemLinkTransformation
							( matLinkCamera, vLinkCamera ) ;
			//
			// ※GetCameraPosition, GetCameraTarget は S3DDynamicCamera による効果を考慮しない
			// 　VR HMD 等のカメラ操作を含んだカラメ向きを反映するために直接値を取得する
			S3DDVector	vCamera =
//					matLinkCamera * pCamera->GetCameraPosition() ;
					matLinkCamera * pCamera->m_space.m_vCenter ;
			S3DDVector	vTarget =
//					matLinkCamera * pCamera->GetCameraTarget() ;
					matLinkCamera * pCamera->m_vTarget ;
			//
			S3DDVector	vViewDir = vTarget - vCamera ;
			vViewDir.Normalize() ;
			//
			vFaceDir = vViewDir ;
			vFaceDir.w = 0.0f ;
		}
		S3DDMatrix	matGlobal ;
		S3DDVector	vGlobal ;
		GetGlobalTransformation( matGlobal, vGlobal ) ;
		//
		S3DMatrix	matISpace( 1, 1, 1 ) ;
		matISpace.InverseOf( S3DMatrix( matGlobal ) ) ;
		matISpace.RevolveVector( vFaceDir ) ;
	}
	size_t		nLastLen = m_bufFaceDirs.GetLength() ;
	S4DVector *	pBufFaceDir = m_bufFaceDirs.GetArray( nLength ) ;
	for ( size_t i = nLastLen; i < nLength; i ++ )
	{
		pBufFaceDir[i] = vFaceDir ;
	}
	m_bufFaceDirs.FinishArray() ;
}

// AddIndexedParticles を使うか？
//////////////////////////////////////////////////////////////////////////////
bool S3DBillboardSerializer::IsUsingIndexedParticles( void )
{
	return	false ;
}

// パーティクル追加（高機能）（classPreRender で呼び出す）
//////////////////////////////////////////////////////////////////////////////
void S3DBillboardSerializer::AddIndexedParticles
	( size_t nCount,
		const S3DVector4 * pvPoints,
		const S3DParticleSerializer::ParticleIndex * pIndexes,
		const size_t * pFrames,
		const S3DColor * pColors,
		const S3DMatrix * pFaceDirs )
{
}



//////////////////////////////////////////////////////////////////////////////
// 文字列ビルボード・表示インスタンス S3DStringBillboardRenderer::EntryList
//////////////////////////////////////////////////////////////////////////////

// 座標設定
//////////////////////////////////////////////////////////////////////////////
void S3DStringBillboardRenderer::EntryList::SetPosition( const S3DVector& vPosition )
{
	for ( size_t i = 0; i < GetLength(); i ++ )
	{
		At(i).vZoom = vPosition ;
	}
}

// 中心座標加算
//////////////////////////////////////////////////////////////////////////////
void S3DStringBillboardRenderer::EntryList::AddOffsetPosition( const S2DVector& vOffset )
{
	for ( size_t i = 0; i < GetLength(); i ++ )
	{
		At(i).vOffset += vOffset ;
	}
}

// 拡大率設定
//////////////////////////////////////////////////////////////////////////////
void S3DStringBillboardRenderer::EntryList::SetZoom( const S2DVector& vZoom )
{
	for ( size_t i = 0; i < GetLength(); i ++ )
	{
		At(i).vZoom = vZoom ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// 文字列ビルボード・描画用
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DStringBillboardRenderer::S3DStringBillboardRenderer( void )
	: m_nKerning( 0 ), m_nLinePitch( 32 )
{
}

// カーニング（文字間幅）[pixel]
//////////////////////////////////////////////////////////////////////////////
int S3DStringBillboardRenderer::GetKerning( void ) const
{
	return	m_nKerning ;
}

void S3DStringBillboardRenderer::SetKerning( int nKerning )
{
	m_nKerning = nKerning ;
}

// 行間 [pixel]
//////////////////////////////////////////////////////////////////////////////
int S3DStringBillboardRenderer::GetLinePitch( void ) const
{
	return	m_nLinePitch ;
}

void S3DStringBillboardRenderer::SetLinePitch( int nPitch )
{
	m_nLinePitch = nPitch ;
}

// 全画像設定クリア
//////////////////////////////////////////////////////////////////////////////
void S3DStringBillboardRenderer::ClearAllCharImages( void )
{
	m_mapCharImage.RemoveAll() ;
}

// 文字画像設定
//////////////////////////////////////////////////////////////////////////////
void S3DStringBillboardRenderer::SetCharImage
	( wchar_t wch,
		SGLImageObject * pImage,
		const SGLImageRect * pRect, ssize_t iFrame )
{
	ESLAssert( pImage != nullptr ) ;
	if ( pImage == nullptr )
	{
		return ;
	}
	ImageEntry	ie ;
	ie.pImage = pImage ;
	ie.iFrame = iFrame ;
	if ( pRect != nullptr )
	{
		ie.rect = *pRect ;
	}
	else
	{
		ie.rect.x = 0 ;
		ie.rect.y = 0 ;
		ie.rect.SetSize( pImage->GetImageSize() ) ;
	}
	m_mapCharImage.SetAs( wch, ie ) ;
}

// 文字画像検索
//////////////////////////////////////////////////////////////////////////////
const S3DStringBillboardRenderer::ImageEntry *
	S3DStringBillboardRenderer::GetCharImage( wchar_t wch ) const
{
	return	m_mapCharImage.GetAs( wch ) ;
}

// 表示インスタンス生成
//////////////////////////////////////////////////////////////////////////////
void S3DStringBillboardRenderer::MakeBillboard
	( S3DStringBillboardRenderer::EntryList& billboard,
		const wchar_t * pwszString,
		const S3DVector& vPosition,
		const S2DVector& vZoom, const S3DColor& clrEffect )
{
	billboard.RemoveAll() ;
	billboard.m_sizeExt.w = 0 ;
	billboard.m_sizeExt.h = 0 ;
	//
	if ( pwszString == nullptr )
	{
		return ;
	}
	int		yNext = 0 ;
	int		xNext = 0 ;
	size_t	iStrNext = 0 ;
	while ( pwszString[iStrNext] != 0 )
	{
		wchar_t	wch = pwszString[iStrNext ++] ;
		if ( wch == L'\n' )
		{
			xNext = 0 ;
			yNext += m_nLinePitch ;
			continue ;
		}
		const ImageEntry *	pie = m_mapCharImage.GetAs( wch ) ;
		if ( (pie == nullptr)
			|| (pie->pImage == nullptr) )
		{
			xNext += m_nKerning ;
			continue ;
		}
		S3DMeshShaper::BillboardEntry	be ;
		be.pImage = pie->pImage ;
		be.iFrame = pie->iFrame ;
		be.rectImage = pie->rect ;
		be.vOffset.x = (float32_t) - xNext ;
		be.vOffset.y = (float32_t) - yNext ;
		be.vPosition = vPosition ;
		be.clrEffect = clrEffect ;
		be.vZoom = vZoom ;
		billboard.Add( be ) ;
		//
		billboard.m_sizeExt.w =
			(int32_t) esl_max( xNext + pie->rect.w, billboard.m_sizeExt.w ) ;
		billboard.m_sizeExt.h =
			(int32_t) esl_max( yNext + pie->rect.h, billboard.m_sizeExt.h ) ;
		//
		xNext += pie->rect.w + m_nKerning ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// 文字列ビルボード
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DStringBillboardSerializer::m_paramEntries
		[S3DStringBillboardSerializer::paramStringBillboardCount] =
{
	{ L"horz_align",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration
		| S3DSceneComposer::attrUIOnlyEnumeration,
		L"水平アライメント", nullptr },
	{ L"vert_align",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration
		| S3DSceneComposer::attrUIOnlyEnumeration,
		L"垂直アライメント", nullptr },
	{ L"kerning",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1,
		L"カーニング", L"文字画像間の幅[pixel]" },
	{ L"line_pitch",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1,
		L"行間", L"改行を行う場合の行間[pixel]。改行は \\n" },
	{ L"billboard_string",
		S3DSceneComposer::typeCommand,
		S3DSceneComposer::attrConstant1,
		L"ビルボード文字列", nullptr },
	{ L"pixel_density",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1,
		L"表示スケール",
		L"3D 空間上の 1.0 の距離に何ピクセル分の表示を行うかを指定します。" },
	{ L"z_bias",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1,
		L"ｚバイアス",
		L"実際にビルボードを表示する座標を修正します。\n"
		L"プラスは奥へ、マイナスは手前へオフセットします。" },
	{ L"zbuf_operation",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration
		| S3DSceneComposer::attrUIOnlyEnumeration,
		L"ｚバッファ処理", nullptr },
	{ L"charset_expr",
		S3DSceneComposer::typeCommand,
		S3DSceneComposer::attrConstant1,
		L"文字集合",
		L"画像を割り当てる文字集合（文字列）。\n"
		L"例えば 0-9A-F は 0123456789ABCDEF と同じ。\n"
		L"\\ と - 記号は \\\\ \\- のようにエスケープする。" },
	{ L"tile_font_image",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration,
		L"格子状文字画像",
		L"文字画像を格子状に並べた画像を指定します。\n"
		L"割り当てられる画像は、左から右、上から下の順序でインデックスされます。" },
	{ L"tile_width",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1,
		L"格子状文字画像幅 [pixel]" },
	{ L"tile_height",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1,
		L"格子状文字画像高 [pixel]" },
	{ L"char_image_count",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrDynamicValidation,
		L"文字画像数",
		L"格子状に画像を切り出すのとば別に、追加で設定する画像数を指定します。" },
} ;

const S3DSceneComposer::ParamSetClass	S3DStringBillboardSerializer::m_pscClass =
{
	&S3DSceneComposer::ItemBasicSerializer::m_pscClass,
	S3DStringBillboardSerializer::paramStringBillboardCount,
	&S3DStringBillboardSerializer::m_paramEntries[0]
} ;

const SSystem::SXMLDocument::AttrInteger
	S3DStringBillboardSerializer::s_aiHorzAlignPairs[4] =
{
	{ L"center", horzAlignCenter },
	{ L"left", horzAlignLeft },
	{ L"right", horzAlignRight },
	{ nullptr, 0 },
} ;

const SSystem::SXMLDocument::AttrInteger
	S3DStringBillboardSerializer::s_aiVertAlignPairs[4] =
{
	{ L"center", horzAlignCenter },
	{ L"top", vertAlignTop },
	{ L"bottom", vertAlignBottom },
	{ nullptr, 0 },
} ;

const SSystem::SXMLDocument::AttrInteger
	S3DStringBillboardSerializer::s_aiZBufOperationPairs[4] =
{
	{ L"write_z", zbufferWrite },
	{ L"no_write_z", zbufferNoWrite },
	{ L"no_zbuf", zbufferNoCompare },
	{ nullptr, 0 },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DStringBillboardSerializer::Particle, SObject )
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DStringBillboardSerializer, ItemBasicSerializer )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DStringBillboardSerializer, string_billboard )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DStringBillboardSerializer::S3DStringBillboardSerializer( void )
	: ItemBasicSerializer
		( m_ItemClassDescriptor.pwszClassID,
			&S3DStringBillboardSerializer::m_pscClass ),
		m_horzAlign( horzAlignCenter ),
		m_vertAlign( vertAlignCenter ),
		m_fpPixelDensity( 100.0 ),
		m_zBias( 0.0 ),
		m_zbufOperation( zbufferNoWrite ),
		m_flagUpdateCharImageRef( true ),
		m_flagUpdateCharImage( true ),
		m_pTileFontImage( nullptr ),
		m_sizeTileFont( 0, 0 ),
		m_nCharImageCount( 0 )
{
	m_classItem = S3DScene::classEffectItem ;
	m_flagsBehavior = S3DScene::itemVisible | S3DScene::itemTimer ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DStringBillboardSerializer::~S3DStringBillboardSerializer( void )
{
}

// 文字画像数に応じたプロパティ設定
//////////////////////////////////////////////////////////////////////////////
void S3DStringBillboardSerializer::UpdateCharImageCountProps( void )
{
	m_aCharImageProps.SetLimit( m_nCharImageCount ) ;
	for ( size_t i = m_aCharImageProps.GetLength(); i < m_nCharImageCount; i ++ )
	{
		SString *	pstrID = new SString ;
		pstrID->Format( L"char_image%d", i ) ;
		m_aCharImagePropIDs.SetAt( i, pstrID ) ;
		//
		SString *	pstrName = new SString ;
		pstrName->Format( L"文字画像 [%d]", i ) ;
		m_aCharImagePropNames.SetAt( i, pstrName ) ;
		//
		S3DSceneComposer::ParamEntry	pe ;
		pe.id = *pstrID ;
		pe.type = S3DSceneComposer::typeSelector ;
		pe.attr = S3DSceneComposer::attrConstant1
				| S3DSceneComposer::attrStringEnumeration ;
		pe.name = *pstrName ;
		pe.desc = nullptr ;
		pe.minRange = 0 ;
		pe.maxRange = 0 ;
		m_aCharImageProps.Add( pe ) ;
	}
}

// パーティクル追加
//////////////////////////////////////////////////////////////////////////////
void S3DStringBillboardSerializer::AddParticle( Particle * pParticle )
{
	m_csParticle.Lock() ;
	m_aParticles.Add( pParticle ) ;
	m_csParticle.Unlock() ;
}

// アライメント
//////////////////////////////////////////////////////////////////////////////
S3DStringBillboardSerializer::HorizontalAlignment
	S3DStringBillboardSerializer::GetHorizontalAlignment( void ) const
{
	return	m_horzAlign ;
}

S3DStringBillboardSerializer::VerticalAlignment
	S3DStringBillboardSerializer::GetVerticalAlignment( void ) const
{
	return	m_vertAlign ;
}

void S3DStringBillboardSerializer::SetHorizontalAlignment
	( S3DStringBillboardSerializer::HorizontalAlignment align )
{
	m_horzAlign = align ;
}

void S3DStringBillboardSerializer::SetVerticalAlignment
	( S3DStringBillboardSerializer::VerticalAlignment align )
{
	m_vertAlign = align ;
}

// ピクセル密度（拡大率^-1）
//////////////////////////////////////////////////////////////////////////////
double S3DStringBillboardSerializer::GetPixelDensity( void ) const
{
	return	m_fpPixelDensity ;
}

void S3DStringBillboardSerializer::SetPixelDensity( double density )
{
	m_fpPixelDensity = density ;
}

// ｚバイアス
//////////////////////////////////////////////////////////////////////////////
double S3DStringBillboardSerializer::GetZBias( void ) const
{
	return	m_zBias ;
}

void S3DStringBillboardSerializer::SetZBias( double zBias )
{
	m_zBias = zBias ;
}

// ｚバッファ操作
//////////////////////////////////////////////////////////////////////////////
S3DStringBillboardSerializer::ZBufferOperation
	S3DStringBillboardSerializer::GetZBufferOperation( void ) const
{
	return	m_zbufOperation ;
}

void S3DStringBillboardSerializer::SetZBufferOperation
	( S3DStringBillboardSerializer::ZBufferOperation zbufOp )
{
	m_zbufOperation = zbufOp ;
}

// 文字画像参照再設定
//////////////////////////////////////////////////////////////////////////////
void S3DStringBillboardSerializer::UpdateCharImagesRef( void )
{
	m_pTileFontImage = nullptr ;
	m_aCharImages.RemoveAll() ;

	S3DSceneComposer *	pComposer = GetComposer() ;
	if ( pComposer != nullptr )
	{
		if ( !m_strTileFontImage.IsEmpty() )
		{
			m_pTileFontImage =
				pComposer->GetAssets().GetImageAs( m_strTileFontImage ) ;
		}
		for ( size_t i = 0; i < m_nCharImageCount; i ++ )
		{
			SString *	pstrImageID = m_aCharImageIDs.GetAt( i ) ;
			if ( pstrImageID != nullptr )
			{
				m_aCharImages.SetAt
					( i, pComposer->GetAssets().GetImageAs( *pstrImageID ) ) ;
			}
		}
	}
	m_flagUpdateCharImageRef = false ;

	UpdateCharImages() ;
}

// 文字画像再設定
//////////////////////////////////////////////////////////////////////////////
void S3DStringBillboardSerializer::UpdateCharImages( void )
{
	ClearAllCharImages() ;

	// 文字集合を展開
	m_strCharSetArray = L"" ;
	size_t	iLastChar = 0 ;
	wchar_t	wchLastChar = 0 ;
	for ( size_t i = 0; i < m_strCharSetExpr.GetLength(); i ++ )
	{
		wchar_t	wch = m_strCharSetExpr.GetAt( i ) ;
		if ( wch == L'\\' )
		{
			if ( iLastChar < i )
			{
				m_strCharSetArray +=
					m_strCharSetExpr.Middle( iLastChar, (ssize_t) (i - iLastChar) ) ;
			}
			if ( ++ i < m_strCharSetExpr.GetLength() )
			{
				wchLastChar = m_strCharSetExpr.GetAt( i ) ;
				m_strCharSetArray += wchLastChar ;
			}
			iLastChar = i + 1 ;
		}
		else if ( wch == L'-' )
		{
			if ( iLastChar < i )
			{
				m_strCharSetArray +=
					m_strCharSetExpr.Middle( iLastChar, (ssize_t) (i - iLastChar) ) ;
			}
			if ( ++ i < m_strCharSetExpr.GetLength() )
			{
				wch = m_strCharSetExpr.GetAt( i ) ;
				//
				for ( wchar_t j = wchLastChar + 1; j <= wch; j ++ )
				{
					m_strCharSetArray += j ;
				}
				wchLastChar = wch ;
			}
			iLastChar = i + 1 ;
		}
		else
		{
			wchLastChar = wch ;
		}
	}
	if ( iLastChar < m_strCharSetExpr.GetLength() )
	{
		m_strCharSetArray += m_strCharSetExpr.Middle( iLastChar ) ;
	}

	// 格子状画像切り出し
	size_t	iNextChar = 0 ;
	if ( (m_pTileFontImage != nullptr)
		&& !m_sizeTileFont.IsEmpty() )
	{
		SGLSize	sizeTileFont = m_pTileFontImage->GetImageSize() ;
		for ( int y = 0; y + m_sizeTileFont.h <= sizeTileFont.h; y += m_sizeTileFont.h )
		{
			for ( int x = 0; x + m_sizeTileFont.w <= sizeTileFont.w; x += m_sizeTileFont.w )
			{
				if ( iNextChar >= m_strCharSetArray.GetLength() )
				{
					break ;
				}
				SGLImageRect	rect( x, y, m_sizeTileFont.w, m_sizeTileFont.h ) ;
				SetCharImage( m_strCharSetArray.GetAt(iNextChar++), m_pTileFontImage, &rect ) ;
			}
		}
	}

	// 画像配列
	for ( size_t i = 0; i < m_aCharImages.GetLength(); i ++ )
	{
		if ( iNextChar >= m_strCharSetArray.GetLength() )
		{
			break ;
		}
		SGLImageObject *	pImage = m_aCharImages.GetAt( i ) ;
		if ( pImage != nullptr )
		{
			SetCharImage( m_strCharSetArray.GetAt(iNextChar++), pImage ) ;
		}
	}
	m_flagUpdateCharImage = false ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
double S3DStringBillboardSerializer::GetScalarParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramPixelDensity:
		return	m_fpPixelDensity ;

	case	paramBiasZ:
		return	m_zBias ;
	}
	return	ItemBasicSerializer::GetScalarParameter( iParam ) ;
}

int32_t S3DStringBillboardSerializer::GetIntegerParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramKerning:
		return	(int32_t) GetKerning() ;

	case	paramLinePitch:
		return	(int32_t) GetLinePitch() ;

	case	paramTileWidth:
		return	m_sizeTileFont.w ;

	case	paramTileHeight:
		return	m_sizeTileFont.h ;

	case	paramCharImageCount:
		return	(int32_t) m_nCharImageCount ;
	}
	return	ItemBasicSerializer::GetIntegerParameter( iParam ) ;
}

const wchar_t * S3DStringBillboardSerializer::GetCommandParameter( size_t iParam ) const
{
	if ( iParam >= paramCharImage0 )
	{
		SString *	pstrImageID = m_aCharImageIDs.GetAt( iParam - paramCharImage0 ) ;
		return	(pstrImageID == nullptr) ? nullptr : (const wchar_t*) *pstrImageID ;
	}
	switch ( iParam )
	{
	case	paramHorzAlign:
		return	SXMLDocument::GetSymbolAsIntegerOf
						( s_aiHorzAlignPairs, m_horzAlign ) ;

	case	paramVertAlign:
		return	SXMLDocument::GetSymbolAsIntegerOf
						( s_aiVertAlignPairs, m_vertAlign ) ;

	case	paramString:
		return	m_strBillboard ;

	case	paramZBufOperation:
		return	SXMLDocument::GetSymbolAsIntegerOf
						( s_aiZBufOperationPairs, m_zbufOperation ) ;

	case	paramCharSet:
		return	m_strCharSetExpr ;

	case	paramTileFontImage:
		return	m_strTileFontImage ;
	}
	return	ItemBasicSerializer::GetCommandParameter( iParam ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DStringBillboardSerializer::SetScalarParameter( size_t iParam, double s )
{
	switch ( iParam )
	{
	case	paramPixelDensity:
		m_fpPixelDensity = s ;
		return ;

	case	paramBiasZ:
		m_zBias = s ;
		return ;
	}
	ItemBasicSerializer::SetScalarParameter( iParam, s ) ;
}

void S3DStringBillboardSerializer::SetIntegerParameter( size_t iParam, int32_t n )
{
	switch ( iParam )
	{
	case	paramKerning:
		SetKerning( n ) ;
		return ;

	case	paramLinePitch:
		SetLinePitch( n ) ;
		return ;

	case	paramTileWidth:
		if ( m_sizeTileFont.w != n )
		{
			m_sizeTileFont.w = n ;
			m_flagUpdateCharImage = true ;
		}
		return ;

	case	paramTileHeight:
		if ( m_sizeTileFont.h != n )
		{
			m_sizeTileFont.h = n ;
			m_flagUpdateCharImage = true ;
		}
		return ;

	case	paramCharImageCount:
		if ( m_nCharImageCount != (size_t) n )
		{
			m_nCharImageCount = (size_t) n ;
			UpdateCharImageCountProps() ;
		}
		return ;
	}
	ItemBasicSerializer::SetIntegerParameter( iParam, n ) ;
}

void S3DStringBillboardSerializer::SetCommandParameter( size_t iParam, const wchar_t * pwszCmd )
{
	if ( iParam >= paramCharImage0 )
	{
		SString *	pstrImageID = m_aCharImageIDs.GetAt( iParam - paramCharImage0 ) ;
		if ( pstrImageID != nullptr )
		{
			*pstrImageID = pwszCmd ;
		}
		else
		{
			m_aCharImageIDs.SetAt( iParam - paramCharImage0, new SString( pwszCmd ) ) ;
		}
		m_flagUpdateCharImageRef = true ;
		return ;
	}
	switch ( iParam )
	{
	case	paramHorzAlign:
		m_horzAlign = (HorizontalAlignment)
			SXMLDocument::GetIntegerAsSymbolOf
				( s_aiHorzAlignPairs, pwszCmd, m_horzAlign ) ;
		return ;

	case	paramVertAlign:
		m_vertAlign = (VerticalAlignment)
			SXMLDocument::GetIntegerAsSymbolOf
				( s_aiVertAlignPairs, pwszCmd, m_vertAlign ) ;
		return ;

	case	paramString:
		m_strBillboard = pwszCmd ;
		return ;

	case	paramZBufOperation:
		m_zbufOperation = (ZBufferOperation)
			SXMLDocument::GetIntegerAsSymbolOf
				( s_aiZBufOperationPairs, pwszCmd, m_zbufOperation ) ;
		return ;

	case	paramCharSet:
		if ( m_strCharSetExpr != pwszCmd )
		{
			m_strCharSetExpr = pwszCmd ;
			m_flagUpdateCharImage = true ;
		}
		return ;

	case	paramTileFontImage:
		if ( m_strTileFontImage != pwszCmd )
		{
			m_strTileFontImage = pwszCmd ;
			m_flagUpdateCharImageRef = true ;
		}
		return ;
	}
	ItemBasicSerializer::SetCommandParameter( iParam, pwszCmd ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DStringBillboardSerializer::EnumerateStringSet
	( size_t iParam, SSystem::SStringArray& aStrSet )
{
	if ( iParam >= paramCharImage0 )
	{
		S3DSceneComposer *	pComposer = GetComposer() ;
		if ( pComposer != nullptr )
		{
			pComposer->GetAssets().EnumerateTextureStringSet( aStrSet ) ;
		}
		return	true ;
	}
	size_t	i ;
	switch ( iParam )
	{
	case	paramHorzAlign:
		for ( i = 0; s_aiHorzAlignPairs[i].pszSymbol != nullptr; i ++ )
		{
			aStrSet.Add( new SString(s_aiHorzAlignPairs[i].pszSymbol) ) ;
		}
		return	true ;

	case	paramVertAlign:
		for ( i = 0; s_aiVertAlignPairs[i].pszSymbol != nullptr; i ++ )
		{
			aStrSet.Add( new SString(s_aiVertAlignPairs[i].pszSymbol) ) ;
		}
		return	true ;

	case	paramZBufOperation:
		for ( i = 0; s_aiZBufOperationPairs[i].pszSymbol != nullptr; i ++ )
		{
			aStrSet.Add( new SString(s_aiZBufOperationPairs[i].pszSymbol) ) ;
		}
		return	true ;

	case	paramTileFontImage:
		S3DSceneComposer *	pComposer = GetComposer() ;
		if ( pComposer != nullptr )
		{
			pComposer->GetAssets().EnumerateTextureStringSet( aStrSet ) ;
		}
		return	true ;
	}
	return	ItemBasicSerializer::EnumerateStringSet( iParam, aStrSet ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DStringBillboardSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"ビルボード" ;
	}
	return	ItemBasicSerializer::GetParameterCategoryName( iCategory ) ;
}

// パラメータエントリ取得
//////////////////////////////////////////////////////////////////////////////
const S3DSceneComposer::ParamEntry *
	S3DStringBillboardSerializer::GetParameterEntryAt( size_t iParam ) const
{
	if ( iParam >= paramCharImage0 )
	{
		return	m_aCharImageProps.GetAt( iParam - paramCharImage0 ) ;
	}
	return	ItemBasicSerializer::GetParameterEntryAt( iParam ) ;
}

// パラメータ総数
//////////////////////////////////////////////////////////////////////////////
size_t S3DStringBillboardSerializer::GetParameterCount( void ) const
{
	return	ItemBasicSerializer::GetParameterCount() + m_nCharImageCount ;
}

// パラメータ指標検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DStringBillboardSerializer::FindParameterID( const wchar_t * pwszID ) const
{
	ssize_t	iParam = ItemBasicSerializer::FindParameterID( pwszID ) ;
	if ( iParam >= 0 )
	{
		return	iParam ;
	}
	for ( size_t i = 0; i < m_nCharImageCount; i ++ )
	{
		SString *	pstrID = m_aCharImagePropIDs.GetAt( i ) ;
		if ( (pstrID != nullptr) && (*pstrID == pwszID) )
		{
			return	(ssize_t) (paramCharImage0 + i) ;
		}
	}
	return	-1 ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DStringBillboardSerializer::IsParameterValidation( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramForceToon:
	case	paramForceBorder:
	case	paramFreeToon:
	case	paramFreeBorder:
	case	paramUseCollision:
		return	false ;
	}
	return	ItemBasicSerializer::IsParameterValidation( iParam ) ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DStringBillboardSerializer::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp, uint32_t nFlags )
{
	uint32_t	nResFlags =
		ItemBasicSerializer::UpdatePropertyReference( comp, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefResource )
	{
		UpdateCharImagesRef() ;
	}
	return	nResFlags ;
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DStringBillboardSerializer::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	ItemBasicSerializer::OnTimer( scene, msecPast ) ;

	SSmartLock<SCriticalSection>	lock( &m_csParticle ) ;
	for ( size_t i = 0; i < m_aParticles.GetLength(); i ++ )
	{
		Particle *	pParticle = m_aParticles.GetAt( i ) ;
		if ( pParticle != nullptr )
		{
			pParticle->OnTimer( scene, *this, msecPast ) ;
			if ( !pParticle->IsLiving() )
			{
				m_aParticles.SetAt( i, nullptr ) ;
			}
		}
	}
	m_aParticles.TrimEmpty() ;
}

// 表示モデル追加
//////////////////////////////////////////////////////////////////////////////
void S3DStringBillboardSerializer::OnItemRenderModel
	( const S3DScene& scene,
		S3DRenderContextInterface& render, uint64_t flagsExclusion )
{
	if ( m_flagUpdateCharImageRef )
	{
		UpdateCharImagesRef() ;
		m_flagUpdateCharImageRef = false ;
	}
	if ( m_flagUpdateCharImage )
	{
		UpdateCharImages() ;
		m_flagUpdateCharImage = false ;
	}
	m_aBillboardEntryBuf.RemoveAll() ;

	const double	s = 1.0 / m_fpPixelDensity ;
	S3DVector		vPosition( 0, 0, 0 ) ;
	S2DVector		vZoom( s, s ) ;
	S3DColor		clrEffect ;
	GetGlobalColorEffect( clrEffect ) ;
	//
	if ( !m_strBillboard.IsEmpty() )
	{
		MakeBillboard( m_billboardTemp, m_strBillboard, vPosition, vZoom, clrEffect ) ;
		m_aBillboardEntryBuf.AddArray
			( m_billboardTemp.GetConstArray(), m_billboardTemp.GetLength() ) ;
	}
	//
	m_csParticle.Lock() ;
	for ( size_t i = 0; i < m_aParticles.GetLength(); i ++ )
	{
		Particle *	pParticle = m_aParticles.GetAt( i ) ;
		if ( (pParticle != nullptr)
			&& !pParticle->m_strBillboard.IsEmpty() )
		{
			S2DVector	vParticleZoom = pParticle->m_vZoom * s ;
			S3DColor	clrParticle = clrEffect * pParticle->m_clrEffect ;
			MakeBillboard
				( m_billboardTemp,
					pParticle->m_strBillboard,
					pParticle->m_vPosition,
					vParticleZoom, clrParticle ) ;
			m_aBillboardEntryBuf.AddArray
				( m_billboardTemp.GetConstArray(), m_billboardTemp.GetLength() ) ;
		}
	}
	m_csParticle.Unlock() ;
	//
	S3DMeshShaper::BillboardRenderParam	brp ;
	brp.matICamera = scene.GetCurrentCameraIMatrix() ;
	brp.vCameraPos = scene.GetCurrentCameraPosition() ;
	brp.vZoom.x = 1.0f ;
	brp.vZoom.y = 1.0f ;
	brp.zAngle = 0.0f ;
	brp.zBias = (float32_t) m_zBias ;
	brp.flagsBillboard = 0 ;
	brp.flagsShadingOpt = shadingTextureSmoothing | shadingVertexAlpha ;
	//
	switch ( m_zbufOperation )
	{
	case	zbufferNoWrite:
		brp.flagsBillboard |= shadingZBufferNoWrite ;
		break ;
	case	zbufferNoCompare:
		brp.flagsBillboard |= shadingNoZBuffer ;
		break ;
	default:
		break ;
	}
	//
	m_meshShaper.RenderMultiBillboards
		( render, brp, m_aBillboardEntryBuf.GetLength(),
						m_aBillboardEntryBuf.GetConstArray() ) ;
}

// 表示インスタンス生成
//////////////////////////////////////////////////////////////////////////////
void S3DStringBillboardSerializer::MakeBillboard
	( EntryList& billboard,
		const wchar_t * pwszString,
		const S3DVector& vPosition,
		const S2DVector& vZoom, const S3DColor& clrEffect )
{
	S3DStringBillboardRenderer::MakeBillboard
		( billboard, pwszString, vPosition, vZoom, clrEffect ) ;
	//
	S2DVector	vOffset( 0, 0 ) ;
	SGLSize		sizeExt = billboard.GetExternalSize() ;
	switch ( m_horzAlign )
	{
	case	horzAlignCenter:
		vOffset.x = (float32_t) sizeExt.w * 0.5f ;
		break ;
	case	horzAlignLeft:
		break ;
	case	horzAlignRight:
		vOffset.x = (float32_t) sizeExt.w ;
		break ;
	}
	switch ( m_vertAlign )
	{
	case	vertAlignCenter:
		vOffset.y = (float32_t) sizeExt.h * 0.5f ;
		break ;
	case	vertAlignTop:
		break ;
	case	vertAlignBottom:
		vOffset.y = (float32_t) sizeExt.h ;
		break ;
	}
	billboard.AddOffsetPosition( vOffset ) ;
}


//////////////////////////////////////////////////////////////////////////////
// 単純な文字列ビルボード・パーティクル
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DSimpleStringBillboardParticle, Particle )
// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSimpleStringBillboardParticle::S3DSimpleStringBillboardParticle( void )
	: m_msecLife( 1000 ), m_msecFadeout( 500 ),
		m_msecElapsed( 0 ), m_argbMulColor( 0xFFFFFFFF )
{
	m_vZoom.x = 1 ;
	m_vZoom.y = 1 ;
	m_clrEffect.rgbMul.ui32 = 0xFFFFFFFF ;
	m_clrEffect.rgbAdd.ui32 = 0 ;
}

S3DSimpleStringBillboardParticle::S3DSimpleStringBillboardParticle
	( const wchar_t * pwszBillboard,
		const S3DVector& vPosition, const S3DVector& vSpeed,
		const S2DVector& vZoom, uint32_t msecLife, uint32_t msecFadeout )
	: S3DSimpleStringBillboardParticle()
{
	SetString( pwszBillboard ) ;
	SetMovingParameter( vPosition, vSpeed, msecLife ) ;
	m_vZoom = vZoom ;
	m_msecFadeout = msecFadeout ;
}

// 文字列設定
//////////////////////////////////////////////////////////////////////////////
void S3DSimpleStringBillboardParticle::SetString( const wchar_t * pwszBillboard )
{
	m_strBillboard = pwszBillboard ;
}

// 色効果（乗算 RGB 及び α）設定
//////////////////////////////////////////////////////////////////////////////
void S3DSimpleStringBillboardParticle::SetBaseColor( uint32_t argbMul )
{
	m_argbMulColor = argbMul ;
	m_clrEffect.rgbMul.ui32 = argbMul ;
}

// 移動パラメータ設定
//////////////////////////////////////////////////////////////////////////////
void S3DSimpleStringBillboardParticle::SetMovingParameter
	( const S3DVector& vPosition, const S3DVector& vSpeed, uint32_t msecLife )
{
	const double	secLife = (double) msecLife * 0.001 ;
	m_bzCurve.SetLine( vPosition, vPosition + vSpeed * secLife, 1.0, 1.0 ) ;
	m_vPosition = vPosition ;
	m_msecLife = msecLife ;
}

// 寿命設定
//////////////////////////////////////////////////////////////////////////////
void S3DSimpleStringBillboardParticle::SetLifeTime( uint32_t msecLife, uint32_t msecFadeout )
{
	m_msecLife = msecLife ;
	m_msecFadeout = msecFadeout ;
}

// 移動曲線設定
//////////////////////////////////////////////////////////////////////////////
void S3DSimpleStringBillboardParticle::SetMoveCurve
		( const SGLBezierCurves<S3DVector>& bzCurve )
{
	m_bzCurve = bzCurve ;
	if ( bzCurve.GetLength() >= 1 )
	{
		m_vPosition = bzCurve.At(0) ;
	}
}

// 拡大率曲線設定
//////////////////////////////////////////////////////////////////////////////
void S3DSimpleStringBillboardParticle::SetZoomCurve
		( const SGLBezierCurves<S2DVector>& bzZoom )
{
	m_bzZoom = bzZoom ;
	if ( bzZoom.GetLength() >= 1 )
	{
		m_vZoom = bzZoom.At(0) ;
	}
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DSimpleStringBillboardParticle::OnTimer
	( S3DScene& scene,
		S3DStringBillboardSerializer& item, uint32_t msecPast )
{
	m_msecElapsed += msecPast ;
	if ( m_msecElapsed >= m_msecLife )
	{
		m_msecElapsed = m_msecLife ;
	}
	//
	double	t = (m_msecLife > 0) ? (double) m_msecElapsed / m_msecLife : 1.0 ;
	if ( m_bzCurve.GetLength() >= 4 )
	{
		m_vPosition = m_bzCurve.PointAt( t ) ;
	}
	if ( m_bzZoom.GetLength() >= 4 )
	{
		m_vZoom = m_bzZoom.PointAt( t ) ;
	}
	if ( (m_msecFadeout > 0)
		&& (m_msecElapsed + m_msecFadeout > m_msecLife) )
	{
		uint32_t	na = (m_msecElapsed - (m_msecLife - m_msecFadeout))
												* 0xFF / m_msecElapsed ;
		m_clrEffect.rgbMul.argb.Alpha = (uint8_t) (na ^ 0xFF) ;
	}
}

// 寿命判定
//////////////////////////////////////////////////////////////////////////////
bool S3DSimpleStringBillboardParticle::IsLiving( void )
{
	return	(m_msecElapsed < m_msecLife) ;
}



//////////////////////////////////////////////////////////////////////////////
// サブコンポジション
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry	
	S3DSubCompositionSerializer::m_paramEntries
		[S3DSubCompositionSerializer::paramSubCompositionCount] =
{
	{ L"ref_composition",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration,
		L"参照コンポジション", nullptr },
	{ L"comp_visible",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrDynamicValidation,
		L"コンポジション表示", nullptr },
	{ L"pool_instance",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory1,
		L"インスタンスプール", L"コンポジションを再利用します" },
	{ L"timemap_method",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrDynamicValidation
		| S3DSceneComposer::attrStringEnumeration,
		L"タイムマップ", nullptr },
	{ L"auto_stop",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"自動停止", L"コンポジションのタイムラインの末尾まで到達すると自動的に停止する" },
	{ L"auto_release",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"自動解放",
		L"コンポジションのタイムラインの末尾まで到達すると"
		L"自動的に解放する（※動的インスタンスのみ）" },
	{ L"timemap_loop",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"ループ", nullptr },
	{ L"timemap_offset",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1,
		L"フレームオフセット", nullptr },
	{ L"timemap_frame",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,
		L"再生フレーム", nullptr },
} ;

const S3DSceneComposer::ParamSetClass
	S3DSubCompositionSerializer::m_pscClass =
{
	&S3DSceneComposer::ItemBasicSerializer::m_pscClass,
	S3DSubCompositionSerializer::paramSubCompositionCount,
	&S3DSubCompositionSerializer::m_paramEntries[0]
} ;

const wchar_t *	S3DSubCompositionSerializer::m_pwszTimemapMethod
					[S3DSubCompositionSerializer::timemapMethodCount] =
{
	L"frame",
	L"timer",
	L"timemap",
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO3
	( SakuraGL::S3DSubCompositionSerializer,
		ItemBasicSerializer, RenderTarget, S3DInstancingItemInterface )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DSubCompositionSerializer, sub_composition )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSubCompositionSerializer::S3DSubCompositionSerializer( void )
	: ItemBasicSerializer
		( m_ItemClassDescriptor.pwszClassID,
				&S3DSubCompositionSerializer::m_pscClass ),
		m_flagEditMode( false ), m_vParticleBase( 0, 0, 0 )
{
	m_classItem = S3DScene::classDynamicItem1 ;
	m_flagsBehavior =
		S3DScene::itemVisible | S3DScene::itemCollision
		| S3DScene::itemTimer | S3DScene::itemOwnerBehavior ;
	m_maskClasses |= (1 << S3DScene::classPreRender) ;
	//
	m_pciCompInfo = nullptr ;
	m_fpTimemapFrame = 0.0 ;
	m_fpStartOwnerFrame = 0.0 ;
	m_fpLastOwnerFrame = 0.0 ;
	m_timemapMethod = timemapOwnerFrame ;
	m_flagAutoStop = true ;
	m_flagAutoRelease = false ;
	m_flagCompVisible = true ;
	m_flagPoolInstance = true ;
	m_flagLoopTimemap = false ;
	m_flagInitializeItem = false ;
	m_flagStartItem = false ;
	m_pComposition = nullptr ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSubCompositionSerializer::~S3DSubCompositionSerializer( void )
{
	if ( m_pComposition != nullptr )
	{
		delete	m_pComposition ;
		m_pComposition = nullptr ;
	}
	for ( size_t i = 0; i < m_aInstance.GetLength(); i ++ )
	{
		delete	m_aInstance.GetAt( i ) ;
	}
	for ( size_t i = 0; i < m_aCompositions.GetLength(); i ++ )
	{
		delete	m_aCompositions.GetAt( i ) ;
	}
	m_aInstance.RemoveAll() ;
	m_aCompositions.RemoveAll() ;
}

// コンポジション設定
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompositionSerializer::SetCompositionInfo
	( const S3DSceneComposer::CompositionInfo * pci, const wchar_t * pwszCompID )
{
	m_strCompID = pwszCompID ;
	if ( m_pciCompInfo == pci )
	{
		return ;
	}
	m_pciCompInfo = pci ;
	//
	// 現在のコンポジションを削除
	//
	m_csLock.Lock() ;
	if ( m_pComposition != nullptr )
	{
		ShoutdownComposition( *m_pComposition ) ;
		delete	m_pComposition ;
		m_pComposition = nullptr ;
	}
	for ( size_t i = 0; i < m_aInstance.GetLength(); i ++ )
	{
		S3DSceneComposer::Composition *	pComp = m_aInstance.GetAt( i ) ;
		if ( pComp != nullptr )
		{
			ShoutdownComposition( *pComp ) ;
			delete	pComp ;
		}
	}
	m_aInstance.RemoveAll() ;
	//
	for ( size_t i = 0; i < m_aCompositions.GetLength(); i ++ )
	{
		S3DSceneComposer::Composition *	pComp = m_aCompositions.GetAt( i ) ;
		if ( pComp != nullptr )
		{
			ShoutdownComposition( *pComp ) ;
			delete	pComp ;
		}
	}
	m_aCompositions.RemoveAll() ;
	//
	for ( size_t i = 0; i < m_aCompStock.GetLength(); i ++ )
	{
		S3DSceneComposer::Composition *	pComp = m_aCompStock.GetAt( i ) ;
		if ( pComp != nullptr )
		{
			ShoutdownComposition( *pComp ) ;
		}
	}
	m_aCompStock.RemoveAll() ;
	//
	// 新しいコンポジションの準備
	//
	if ( m_flagCompVisible )
	{
		m_pComposition = LoadCompositionInstance( S3DSceneComposer::ScriptObject() ) ;
		if ( m_pComposition != nullptr )
		{
			if ( m_pComposition->GetCurrentPlayingFrame() != m_fpTimemapFrame )
			{
				m_pComposition->SetAllItemsFrame
					( m_fpTimemapFrame, S3DSceneComposer::seekJump ) ;
			}
			m_pComposition->RestartComposition() ;
		}
	}
	m_csLock.Unlock() ;
}

// コンポジション表示設定
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompositionSerializer::SetCompositionVisible( bool flagVisible )
{
	m_flagCompVisible = flagVisible ;
	//
	if ( flagVisible )
	{
		if ( m_pComposition == nullptr )
		{
			m_pComposition = LoadCompositionInstance( S3DSceneComposer::ScriptObject() ) ;
			//
			if ( (m_pComposition != nullptr)
				&& (m_pComposition->GetCurrentPlayingFrame() != m_fpTimemapFrame) )
			{
				m_pComposition->SetAllItemsFrame
					( m_fpTimemapFrame, S3DSceneComposer::seekJump ) ;
			}
			if ( m_pComposition != nullptr )
			{
				m_pComposition->RestartComposition() ;
			}
		}
	}
	else
	{
		m_csLock.Lock() ;
		S3DSceneComposer::Composition *	pComp = m_pComposition ;
		if ( pComp != nullptr )
		{
			m_pComposition = nullptr ;
			//
			if ( m_flagPoolInstance )
			{
				m_aCompStock.Push( pComp ) ;
			}
			else
			{
				ShoutdownComposition( *pComp ) ;
				delete	pComp ;
			}
		}
		m_csLock.Unlock() ;
	}
}

// インスタンス生成
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Composition *
	S3DSubCompositionSerializer::CreateInstance( void )
{
	return	CreateInstance( RSSmartPtr() ) ;
}

S3DSceneComposer::Composition *
	S3DSubCompositionSerializer::CreateInstance
			( const Rosetta::RSSmartPtr& ptrUserInstance )
{
	return	CreateInstance
				( S3DSceneComposer::ScriptObject( ptrUserInstance ) ) ;
}

S3DSceneComposer::Composition *
	S3DSubCompositionSerializer::CreateInstance
			( const S3DSceneComposer::ScriptObject& instance )
{
	S3DSceneComposer::Composition *	pComp = LoadCompositionInstance( instance ) ;
	if ( pComp != nullptr )
	{
		pComp->SetAllItemsFrame( 0.0, S3DSceneComposer::seekJumpReset ) ;
		pComp->RestartComposition() ;
		//
		m_csLock.Lock() ;
		m_aInstance.Add( pComp ) ;
		m_csLock.Unlock() ;
	}
	return	pComp ;
}

// インスタンス削除（リサイクル）
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompositionSerializer::ReleaseInstance( S3DSceneComposer::Composition * pComp )
{
	m_csLock.Lock() ;
	ssize_t	iComp = m_aInstance.FindPtr( pComp ) ;
	if ( iComp >= 0 )
	{
		m_aInstance.RemoveAt( (size_t) iComp ) ;
		//
		if ( m_flagPoolInstance )
		{
			m_csLock.Unlock() ;
			pComp->StopCompositoin() ;
			m_csLock.Lock() ;
			m_aCompStock.Push( pComp ) ;
		}
		else
		{
			m_csLock.Unlock() ;
			ShoutdownComposition( *pComp ) ;
			m_csLock.Lock() ;
			m_aDelayRemove.Add( pComp ) ;
		}
	}
	m_csLock.Unlock() ;
}

// インスタンスが有効か？
//////////////////////////////////////////////////////////////////////////////
bool S3DSubCompositionSerializer::IsValidInstance
			( S3DSceneComposer::Composition * pComp ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csLock ) ;
	return	(m_aInstance.FindPtr( pComp ) >= 0) ;
}

// 有効インスタンス数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DSubCompositionSerializer::GetInstanceCount( void ) const
{
	return	m_aInstance.GetLength() ;
}

// 有効インスタンス取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Composition *
	S3DSubCompositionSerializer::GetInstanceAt( size_t iInstance ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csLock ) ;
	return	m_aInstance.GetAt( iInstance ) ;
}

// 遅延削除（リサイクル）設定
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompositionSerializer::DelayReleaseInstance
	( S3DSceneComposer::Composition * pComp )
{
	ESLAssert( pComp != nullptr ) ;
	m_csLock.Lock() ;
	if ( m_aDelayRelease.FindPtr( pComp ) < 0 )
	{
		m_aDelayRelease.Push( pComp ) ;
	}
	m_csLock.Unlock() ;
}

// 遅延削除インスタンスを削除（リサイクル）
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompositionSerializer::FlushDelayReleaseInstance( void )
{
	m_csLock.Lock() ;
	while ( m_aDelayRelease.GetLength() > 0 )
	{
		S3DSceneComposer::Composition * pComp = m_aDelayRelease.Pop() ;
		m_csLock.Unlock() ;
		ReleaseInstance( pComp ) ;
		m_csLock.Lock() ;
	}
	m_aDelayRemove.RemoveAll() ;
	m_csLock.Unlock() ;
}

// FlushDelayReleaseInstance の非同期呼び出し
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompositionSerializer::AsyncFlushDelayReleaseInstance( S3DScene& scene )
{
	m_csLock.Lock() ;
	while ( m_aDelayRelease.GetLength() > 0 )
	{
		S3DSceneComposer::Composition * pComp = m_aDelayRelease.Pop() ;
		m_csLock.Unlock() ;
		ReleaseInstance( pComp ) ;
		m_csLock.Lock() ;
	}
	if ( m_aDelayRemove.GetLength() > 0 )
	{
		scene.PostAsyncProcedure
			( new DelayRemoveInstanceProc( m_aDelayRemove ), nullptr, true ) ;
	}
	m_csLock.Unlock() ;
}

ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DSubCompositionSerializer::DelayRemoveInstanceProc, ESLObject, SProcedure )

S3DSubCompositionSerializer::DelayRemoveInstanceProc::DelayRemoveInstanceProc
		( SSystem::SObjectArray<S3DSceneComposer::Composition>& aDelayRemove )
{
	m_aDelayRemove.MoveArrayFrom( 0, aDelayRemove ) ;
}

void S3DSubCompositionSerializer::DelayRemoveInstanceProc::Run( void )
{
	m_aDelayRemove.RemoveAll() ;
}

// SceneSettingsInfo 検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DSubCompositionSerializer::FindSceneSettingsInfo
						( SGLSecondaryViewProducer * psvp ) const
{
	m_csLock.Lock() ;
	for ( size_t i = 0; i < m_aSettings.GetLength(); i ++ )
	{
		SceneSettingsInfo *	pssi = m_aSettings.GetAt( i ) ;
		if ( pssi && (pssi->refSVP.GetReference() == psvp) )
		{
			m_csLock.Unlock() ;
			return	(ssize_t) i ;
		}
	}
	m_csLock.Unlock() ;
	return	-1 ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
double S3DSubCompositionSerializer::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramTimemapOffset:
		return	m_fpStartOwnerFrame ;
	case	paramTimemapFrame:
		return	m_fpTimemapFrame ;
	}
	return	ItemBasicSerializer::GetScalarParameter( i ) ;
}

bool S3DSubCompositionSerializer::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramVisibleComp:
		return	m_flagCompVisible ;
	case	paramPoolInstance:
		return	m_flagPoolInstance ;
	case	paramAutoStop:
		return	m_flagAutoStop ;
	case	paramAutoRelease:
		return	m_flagAutoRelease ;
	case	paramTimemapLoop:
		return	m_flagLoopTimemap ;
	}
	return	ItemBasicSerializer::GetBooleanParameter( i ) ;
}

const wchar_t * S3DSubCompositionSerializer::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramComposition:
		return	m_strCompID ;
	case	paramTimemapMethod:
		return	m_pwszTimemapMethod[m_timemapMethod] ;
	}
	return	ItemBasicSerializer::GetCommandParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompositionSerializer::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramTimemapOffset:
		m_fpStartOwnerFrame = s ;
		return ;
	case	paramTimemapFrame:
		m_fpTimemapFrame = s ;
		return ;
	}
	ItemBasicSerializer::SetScalarParameter( i, s ) ;
}

void S3DSubCompositionSerializer::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramVisibleComp:
		SetCompositionVisible( b ) ;
		return ;
	case	paramPoolInstance:
		m_flagPoolInstance = b ;
		return ;
	case	paramAutoStop:
		m_flagAutoStop = b ;
		return ;
	case	paramAutoRelease:
		m_flagAutoRelease = b ;
		return ;
	case	paramTimemapLoop:
		m_flagLoopTimemap = b ;
		return ;
	}
	ItemBasicSerializer::SetBooleanParameter( i, b ) ;
}

void S3DSubCompositionSerializer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramComposition:
		{
			S3DSceneComposer::CompositionInfo *	pci = nullptr ;
			if ( pwszCmd && pwszCmd[0] )
			{
				S3DSceneComposer *	pComposer = GetComposer() ;
				if ( pComposer != nullptr )
				{
					pci = pComposer->GetCompositionAs( pwszCmd, true ) ;
				}
			}
			SetCompositionInfo( pci, pwszCmd ) ;
		}
		return ;

	case	paramTimemapMethod:
		{
			for ( size_t j = 0; j < timemapMethodCount; j ++ )
			{
				if ( SString::Compare( m_pwszTimemapMethod[j], pwszCmd ) == 0 )
				{
					m_timemapMethod = (TimemapMethod) j ;
					break ;
				}
			}
		}
		return ;
	}
	ItemBasicSerializer::SetCommandParameter( i, pwszCmd ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DSubCompositionSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	switch ( i )
	{
	case	paramComposition:
		{
			S3DSceneComposer *	pComposer = GetComposer() ;
			if ( pComposer != nullptr )
			{
				pComposer->EnumerateCompositionIDs( aStrSet ) ;
			}
		}
		return	true ;

	case	paramTimemapMethod:
		{
			for ( size_t j = 0; j < timemapMethodCount; j ++ )
			{
				aStrSet.Add( new SString( m_pwszTimemapMethod[j] ) ) ;
			}
		}
		return	true ;
	}
	return	ItemBasicSerializer::EnumerateStringSet( i, aStrSet ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSubCompositionSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"サブコンポジション" ;
	}
	return	ItemBasicSerializer::GetParameterCategoryName( iCategory ) ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DSubCompositionSerializer::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramVisible:
	case	paramForceToon:
	case	paramForceBorder:
	case	paramFreeToon:
	case	paramFreeBorder:
	case	paramUseCollision:
	case	paramItemClass:
		return	false ;
	case	paramTimemapMethod:
		return	m_flagCompVisible ;
	case	paramTimemapLoop:
		return	m_flagCompVisible && (m_timemapMethod != timemapSequence) ;
	case	paramTimemapOffset:
		return	m_flagCompVisible && (m_timemapMethod == timemapOwnerFrame) ;
	case	paramTimemapFrame:
		return	m_flagCompVisible && (m_timemapMethod == timemapSequence) ;
	}
	return	ItemBasicSerializer::IsParameterValidation( i ) ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DSubCompositionSerializer::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp, uint32_t nFlags )
{
	uint32_t	nResFlags =
		ItemBasicSerializer::UpdatePropertyReference( comp, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefSubComposition )
	{
		SString	strCompID = m_strCompID ;
		SetCompositionInfo( nullptr, nullptr ) ;
		if ( !strCompID.IsEmpty() )
		{
			S3DSceneComposer *	pComposer = GetComposer() ;
			if ( pComposer != nullptr )
			{
				const S3DSceneComposer::CompositionInfo *
					pci = pComposer->GetCompositionAs( strCompID, true ) ;
				SetCompositionInfo( pci, strCompID ) ;
			}
		}
	}
	if ( m_pComposition != nullptr )
	{
		nResFlags |=
			m_pComposition->UpdatePropertyReference( *m_pComposition, nFlags ) ;
	}
	for ( size_t i = 0; i < m_aCompositions.GetLength(); i ++ )
	{
		S3DSceneComposer::Composition *	pComp = m_aCompositions.GetAt( i ) ;
		if ( pComp != nullptr )
		{
			nResFlags |= pComp->UpdatePropertyReference( *pComp, nFlags ) ;
		}
	}
	for ( size_t i = 0; i < m_aCompStock.GetLength(); i ++ )
	{
		S3DSceneComposer::Composition *	pComp = m_aCompStock.GetAt( i ) ;
		if ( pComp != nullptr )
		{
			nResFlags |= pComp->UpdatePropertyReference( *pComp, nFlags ) ;
		}
	}
	return	nResFlags ;
}

// Loquaty クラス名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSubCompositionSerializer::GetLQClassName( void ) const
{
	return	L"EntisGLS4.SceneSubComposition" ;
}

// レンダリング設定
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompositionSerializer::OnSetupSceneSettings
	( S3DScene& scene,
		const SGLSize& sizeFrame,
		SGLSecondaryViewProducer * psvp )
{
	ItemBasicSerializer::OnSetupSceneSettings( scene, sizeFrame, psvp ) ;
	//
	if ( m_pComposition != nullptr )
	{
		m_pComposition->OnSetupSceneSettings( scene, sizeFrame, psvp ) ;
	}
	for ( size_t i = 0; i < m_aInstance.GetLength(); i ++ )
	{
		S3DSceneComposer::Composition *	pComp = m_aInstance.GetAt( i ) ;
		if ( pComp != nullptr )
		{
			pComp->OnSetupSceneSettings( scene, sizeFrame, psvp ) ;
		}
	}
	for ( size_t i = 0; i < m_aCompositions.GetLength(); i ++ )
	{
		S3DSceneComposer::Composition *	pComp = m_aCompositions.GetAt( i ) ;
		if ( pComp != nullptr )
		{
			pComp->OnSetupSceneSettings( scene, sizeFrame, psvp ) ;
		}
	}
	for ( size_t i = 0; i < m_aCompStock.GetLength(); i ++ )
	{
		S3DSceneComposer::Composition *	pComp = m_aCompStock.GetAt( i ) ;
		if ( pComp != nullptr )
		{
			pComp->OnSetupSceneSettings( scene, sizeFrame, psvp ) ;
		}
	}
	//
	SceneSettingsInfo *	pssi = new SceneSettingsInfo ;
	pssi->sizeFrame = sizeFrame ;
	pssi->refSVP = psvp ;
	pssi->refScene = &scene ;
	//
	ssize_t	iSVP = FindSceneSettingsInfo( psvp ) ;
	if ( iSVP < 0 )
	{
		m_aSettings.Add( pssi ) ;
	}
	else
	{
		m_aSettings.SetAt( (size_t) iSVP, pssi ) ;
	}
}

// レンダリング後始末
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompositionSerializer::OnShoutdownSceneSettings
	( S3DScene& scene, SGLSecondaryViewProducer * psvp )
{
	ItemBasicSerializer::OnShoutdownSceneSettings( scene, psvp ) ;
	//
	if ( m_pComposition != nullptr )
	{
		m_pComposition->OnShoutdownSceneSettings( scene, psvp ) ;
	}
	for ( size_t i = 0; i < m_aInstance.GetLength(); i ++ )
	{
		S3DSceneComposer::Composition *	pComp = m_aInstance.GetAt( i ) ;
		if ( pComp != nullptr )
		{
			pComp->OnShoutdownSceneSettings( scene, psvp ) ;
		}
	}
	for ( size_t i = 0; i < m_aCompositions.GetLength(); i ++ )
	{
		S3DSceneComposer::Composition *	pComp = m_aCompositions.GetAt( i ) ;
		if ( pComp != nullptr )
		{
			pComp->OnShoutdownSceneSettings( scene, psvp ) ;
		}
	}
	for ( size_t i = 0; i < m_aCompStock.GetLength(); i ++ )
	{
		S3DSceneComposer::Composition *	pComp = m_aCompStock.GetAt( i ) ;
		if ( pComp != nullptr )
		{
			pComp->OnShoutdownSceneSettings( scene, psvp ) ;
		}
	}
	//
	ssize_t	iSVP = FindSceneSettingsInfo( psvp ) ;
	if ( iSVP >= 0 )
	{
		m_aSettings.RemoveAt( (size_t) iSVP ) ;
	}
}

// 拡張的な処理の通知
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompositionSerializer::OnExtendNotify
	( const wchar_t * pwszCmd, const wchar_t * pwszParam,
		const void * pExParam, size_t nExParamBytes )
{
	ItemBasicSerializer::OnExtendNotify
			( pwszCmd, pwszParam, pExParam, nExParamBytes ) ;
	//
	if ( SString::Compare( pwszCmd, S3DSceneComposer::CmdInitializeItem ) == 0 )
	{
		m_flagInitializeItem = true ;
		//
		S3DSceneComposer::Composition *	pSceneComp = GetComposition() ;
		m_flagEditMode = (pSceneComp != nullptr) && pSceneComp->IsEditMode() ;
	}
	else if ( SString::Compare( pwszCmd, S3DSceneComposer::CmdFinishItem ) == 0 )
	{
		m_flagInitializeItem = false ;
	}
	else if ( SString::Compare( pwszCmd, S3DSceneComposer::CmdStartItem ) == 0 )
	{
		m_flagStartItem = true ;
	}
	else if ( SString::Compare( pwszCmd, S3DSceneComposer::CmdStopItem ) == 0 )
	{
		m_flagStartItem = false ;
	}
	//
	if ( m_pComposition != nullptr )
	{
		m_pComposition->OnExtendNotify
			( pwszCmd, pwszParam, pExParam, nExParamBytes ) ;
	}
	for ( size_t i = 0; i < m_aInstance.GetLength(); i ++ )
	{
		S3DSceneComposer::Composition *	pComp = m_aInstance.GetAt( i ) ;
		if ( pComp != nullptr )
		{
			pComp->OnExtendNotify
				( pwszCmd, pwszParam, pExParam, nExParamBytes ) ;
		}
	}
	for ( size_t i = 0; i < m_aCompositions.GetLength(); i ++ )
	{
		S3DSceneComposer::Composition *	pComp = m_aCompositions.GetAt( i ) ;
		if ( pComp != nullptr )
		{
			pComp->OnExtendNotify
				( pwszCmd, pwszParam, pExParam, nExParamBytes ) ;
		}
	}
	if ( (SString::Compare( pwszCmd, S3DSceneComposer::CmdInitializeItem ) == 0)
		|| (SString::Compare( pwszCmd, S3DSceneComposer::CmdFinishItem ) == 0) )
	{
		for ( size_t i = 0; i < m_aCompStock.GetLength(); i ++ )
		{
			S3DSceneComposer::Composition *	pComp = m_aCompStock.GetAt( i ) ;
			if ( pComp != nullptr )
			{
				pComp->OnExtendNotify
					( pwszCmd, pwszParam, pExParam, nExParamBytes ) ;
			}
		}
	}
}

// タイマ処理
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompositionSerializer::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	ItemBasicSerializer::OnTimer( scene, msecPast ) ;
	//
	if ( msecPast == 0 )
	{
		return ;
	}
	AsyncFlushDelayReleaseInstance( scene ) ;
	//
	double	secPast = (double) msecPast / 1000.0 ;
	if ( m_pComposition != nullptr )
	{
		if ( (m_timemapMethod == timemapOwnerTimer)
			&& m_pComposition->IsPlayingComposition() )
		{
			ESLAssert( m_pciCompInfo != nullptr ) ;
			m_fpTimemapFrame +=
					m_pciCompInfo->FrameIndexFromSecond( secPast ) ;
			UpdateCompositionFrame
				( *m_pComposition, m_fpTimemapFrame,
								S3DSceneComposer::seekStream ) ;
		}
		m_pComposition->OnTimer( scene, msecPast ) ;
	}
	for ( size_t i = 0; i < m_aInstance.GetLength(); i ++ )
	{
		m_csLock.Lock() ;
		S3DSceneComposer::Composition *	pComposition = m_aInstance.GetAt( i ) ;
		m_csLock.Unlock() ;
		if ( pComposition != nullptr )
		{
			if ( pComposition->IsPlayingComposition() )
			{
				ESLAssert( m_pciCompInfo != nullptr ) ;
				double	fpFrame =
					pComposition->GetCurrentPlayingFrame() + 
						m_pciCompInfo->FrameIndexFromSecond( secPast ) ;
				if ( UpdateCompositionFrame
					( *pComposition, fpFrame, S3DSceneComposer::seekStream ) )
				{
					if ( m_flagAutoRelease )
					{
						DelayReleaseInstance( pComposition ) ;
					}
				}
			}
			pComposition->OnTimer( scene, msecPast ) ;
		}
	}
}

// フレーム反映
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompositionSerializer::SetFrameParameters
		( double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	ItemBasicSerializer::SetFrameParameters( fpFrame, seek ) ;
	//
	if ( (m_pComposition != nullptr)
		&& m_pComposition->IsPlayingComposition() )
	{
		if ( m_timemapMethod == timemapOwnerFrame )
		{
			if ( (seek == S3DSceneComposer::seekStream)
				|| (seek == S3DSceneComposer::seekStreamPaused) )
			{
				m_fpTimemapFrame += fpFrame - m_fpLastOwnerFrame ;
			}
			else
			{
				m_fpTimemapFrame = fpFrame - m_fpStartOwnerFrame ;
			}
			UpdateCompositionFrame
				( *m_pComposition, m_fpTimemapFrame, seek ) ;
		}
		else if ( m_timemapMethod == timemapSequence )
		{
			UpdateCompositionFrame
				( *m_pComposition,
					m_fpTimemapFrame, S3DSceneComposer::seekStream ) ;
		}
	}
	//
	m_fpLastOwnerFrame = fpFrame ;
}

// アイテム作用の追加処理
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompositionSerializer::OnUpdateBehavior( S3DScene& scene )
{
	ItemBasicSerializer::OnUpdateBehavior( scene ) ;
	//
	AsyncFlushDelayReleaseInstance( scene ) ;
	//
	// コンポジション描画準備
	//
	m_maskClasses = 0 ;
	//
	bool	flagItemVisible =
				(m_flagsBehavior & S3DScene::itemVisible)
				&& !(m_flagsBehavior & S3DScene::itemIgnore) ;
	if ( m_pComposition != nullptr )
	{
		InvokeUpdateBehaviorFlags
			( scene, m_pComposition,
				m_flagCompVisible && flagItemVisible ) ;
		m_maskClasses |= m_pComposition->GetItemClassesMask() ;
	}
	for ( size_t i = 0; i < m_aInstance.GetLength(); i ++ )
	{
		m_csLock.Lock() ;
		S3DSceneComposer::Composition *	pComp = m_aInstance.GetAt( i ) ;
		m_csLock.Unlock() ;
		if ( pComp != nullptr )
		{
			InvokeUpdateBehaviorFlags( scene, pComp, flagItemVisible ) ;
			m_maskClasses |= pComp->GetItemClassesMask() ;
		}
	}
	//
	m_csLock.Lock() ;
	//
	S3DDMatrix	matItem, matITarget ;
	S3DDVector	vItem, vITarget ;
	CalcGlobalTransformation( matItem, vItem ) ;
	//
	if ( flagItemVisible )
	{
		//
		// パーティクルコンポジション準備
		//
		matITarget.InverseOf( matItem ) ;
		vITarget = matITarget * (m_vParticleBase - vItem) ;
		//
		size_t	nCount = m_aParticleMatrics.GetLength() ;
		ESLAssert( m_aParticleColors.GetLength() == nCount ) ;
		ESLAssert( m_aParticleFrames.GetLength() == nCount ) ;
		const S4DMatrix *	pMatrics = m_aParticleMatrics.GetConstArray() ;
		const S3DColor *	pColors = m_aParticleColors.GetConstArray() ;
		const size_t *		pFrames = m_aParticleFrames.GetConstArray() ;
		//
		for ( size_t i = 0; i < nCount; i ++ )
		{
			S3DSceneComposer::Composition *	pComp = m_aCompositions.GetAt( i ) ;
			if ( pComp == nullptr )
			{
				pComp = LoadCompositionInstance( S3DSceneComposer::ScriptObject() ) ;
				if ( pComp == nullptr )
				{
					continue ;
				}
				m_aCompositions.SetAt( i, pComp ) ;
			}
			pComp->m_matTransformation =
				matITarget * S3DDMatrix( pMatrics[i].GetMatrix3() ) ;
			pComp->m_vCenter =
				matITarget * S3DDVector( pMatrics[i].GetTranslation() ) + vITarget ;
			pComp->m_nTransparency = 0xFF - pColors[i].rgbMul.argb.Alpha ;
			pComp->m_colorEffect = pColors[i] ;
			//
			if ( pComp->GetCurrentPlayingFrame() < pFrames[i] )
			{
				pComp->SetAllItemsFrame
					( (double) pFrames[i], S3DSceneComposer::seekStream ) ;
			}
			else if ( pComp->GetCurrentPlayingFrame() > pFrames[i] )
			{
				pComp->SetAllItemsFrame
					( (double) pFrames[i], S3DSceneComposer::seekJump ) ;
			}
			//
			InvokeUpdateBehaviorFlags( scene, pComp, flagItemVisible ) ;
			m_maskClasses |= pComp->GetItemClassesMask() ;
		}
		for ( size_t i = nCount; i < m_aCompositions.GetLength(); i ++ )
		{
			S3DSceneComposer::Composition *	pComp = m_aCompositions.GetAt( i ) ;
			if ( pComp != nullptr )
			{
				pComp->StopCompositoin() ;
				m_aCompStock.Push( pComp ) ;
			}
		}
		m_aCompositions.SetLength( nCount ) ;
	}
	m_aParticleMatrics.RemoveAll() ;
	m_aParticleColors.RemoveAll() ;
	m_aParticleFrames.RemoveAll() ;
	m_vParticleBase = vItem ;
	//
	m_csLock.Unlock() ;
}

// レンダリング前後処理（全視点共通）
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompositionSerializer::OnRenderEvent
	( S3DScene& scene, S3DScene::ItemClass clsItem )
{
	ItemBasicSerializer::OnRenderEvent( scene, clsItem ) ;
	//
	if ( clsItem == S3DScene::classPreRender )
	{
		size_t		nCtrls = GetControllerCount() ;
		for ( size_t i = 0; i < nCtrls; i ++ )
		{
			S3DSceneComposer::Controller *	pCtrl = GetControllerAt( i ) ;
			if ( (pCtrl == nullptr)
				|| pCtrl->IsControllerDisabled() )
			{
				continue ;
			}
			S3DInstancingEntryInterface *	pInstancing =
				ESLTypeCast<S3DInstancingEntryInterface>( pCtrl ) ;
			if ( pInstancing != nullptr )
			{
				const S4DMatrix *	pMatrixs ;
				const S3DColor	*	pColors ;
				size_t	nCount =
					pInstancing->GetInstancingArray( pMatrixs, pColors ) ;
				//
				AddDynamicInstancingEntries
					( pMatrixs, pColors, nCount, (ParameterProperty*) this ) ;
			}
		}
	}
}

// コンポジションのフレーム更新
//////////////////////////////////////////////////////////////////////////////
bool S3DSubCompositionSerializer::UpdateCompositionFrame
	( S3DSceneComposer::Composition& comp,
		double& fpFrame, S3DSceneComposer::SeekMethod seek ) const
{
	if ( m_pciCompInfo == nullptr )
	{
		return	false ;
	}
	double	fpMaxFrame = (double) m_pciCompInfo->GetTotalFrameCount() ;
	double	fpActualFrame = fpFrame ;
	bool	flagFinished = false ;
	if ( m_flagLoopTimemap && (fpMaxFrame > 0.0) )
	{
		if ( (fpActualFrame < 0.0) || (fpActualFrame >= fpMaxFrame) )
		{
			double	fpBase = floor( fpActualFrame / fpMaxFrame ) ;
			fpActualFrame -= fpBase * fpMaxFrame ;
		}
	}
	else
	{
		fpActualFrame = esl_fclamp( fpActualFrame, 0.0, fpMaxFrame ) ;
		flagFinished = (fpActualFrame >= fpMaxFrame) ;
	}
	if ( fpActualFrame != comp.GetCurrentPlayingFrame() )
	{
		if ( ((seek == S3DSceneComposer::seekStream)
				|| (seek == S3DSceneComposer::seekStreamPaused))
			&& (fpActualFrame < comp.GetCurrentPlayingFrame()) )
		{
			seek = S3DSceneComposer::seekJump ;
		}
		comp.SetAllItemsFrame( fpActualFrame, seek ) ;
		//
		double	fpJumpFrame ;
		while ( comp.GetPostTimelineFrame( fpJumpFrame ) )
		{
			comp.SetAllItemsFrame
				( fpJumpFrame, S3DSceneComposer::seekJump ) ;
			fpFrame = fpJumpFrame ;
			flagFinished = false ;
		}
	}
	else
	{
		comp.OnUpdateFrame( fpActualFrame, seek ) ;
	}
	if ( flagFinished && m_flagAutoStop )
	{
		comp.StopCompositoin() ;
	}
	return	flagFinished ;
}

// コンポジション・インスタンス生成
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Composition *
	S3DSubCompositionSerializer::LoadCompositionInstance
				( const S3DSceneComposer::ScriptObject& instance )
{
	m_csLock.Lock() ;
	if ( m_aCompStock.GetLength() > 0 )
	{
		S3DSceneComposer::Composition *	pComp = m_aCompStock.Pop() ;
		ESLAssert( pComp != nullptr ) ;
		pComp->SetScriptInstance( instance ) ;
		if ( m_flagStartItem )
		{
			pComp->PlayComposition() ;
			pComp->PauseCompositoin() ;
		}
		m_csLock.Unlock() ;
		return	pComp ;
	}
	m_csLock.Unlock() ;
	//
	if ( m_pciCompInfo == nullptr )
	{
		return	nullptr ;
	}
	S3DSceneComposer::Composition *	pOwner = GetComposition() ;
	if ( pOwner == nullptr )
	{
		return	nullptr ;
	}
	S3DSceneComposer *	pComposer = pOwner->GetSceneComposer() ;
	if ( pComposer == nullptr )
	{
		return	nullptr ;
	}
	S3DSceneComposer::Composition *
		pComp = pComposer->CreateComposition( *m_pciCompInfo, pOwner ) ;
	if ( pComp == nullptr )
	{
		return	nullptr ;
	}
	pComp->SetScriptInstance( instance ) ;
	pComp->m_matTransformation.InitializeMatrix( S3DDVector( 1, 1, 1 ) ) ;
	pComp->m_vCenter = S3DDVector( 0, 0, 0 ) ;
	pComp->AttachOwnerItem( this ) ;
	pComp->SetEditMode( m_flagEditMode ) ;
	pComp->SetFixTimerInterval( false ) ;
	pComp->EnableTimerEvent( true, false ) ;
	pComp->EnableFrameOnTimer( false ) ;
	pComp->InitializeFrameParameters() ;
	//
	if ( m_flagInitializeItem )
	{
		S3DScene *	pScene = GetScene() ;
		if ( (pScene == nullptr) && (m_aSettings.GetLength() >= 1) )
		{
			pScene = m_aSettings.At(0).refScene ;
		}
		pComp->InitializeItems( pScene ) ;
	}
	for ( size_t i = 0; i < m_aSettings.GetLength(); i ++ )
	{
		SceneSettingsInfo *	pssi = m_aSettings.GetAt( i ) ;
		ESLAssert( pssi != nullptr ) ;
		if ( pssi->refScene != nullptr )
		{
			pComp->OnSetupSceneSettings
				( *(pssi->refScene.GetReference()),
					pssi->sizeFrame, pssi->refSVP.GetReference() ) ;
		}
	}
	if ( m_flagStartItem )
	{
		pComp->PlayComposition() ;
		pComp->PauseCompositoin() ;
	}
	return	pComp ;
}

// コンポジション・シャットダウン処理
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompositionSerializer::ShoutdownComposition
	( S3DSceneComposer::Composition& comp )
{
	if ( m_flagStartItem )
	{
		comp.StopCompositoin() ;
	}
	if ( m_flagInitializeItem )
	{
		comp.FinishComposition() ;
	}
	for ( size_t i = 0; i < m_aSettings.GetLength(); i ++ )
	{
		SceneSettingsInfo *	pssi = m_aSettings.GetAt( i ) ;
		ESLAssert( pssi != nullptr ) ;
		if ( pssi->refScene != nullptr )
		{
			comp.OnShoutdownSceneSettings
				( *(pssi->refScene.GetReference()),
					pssi->refSVP.GetReference() ) ;
		}
	}
}

// 規定のレンダリングデバイス設定
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompositionSerializer::OnSetRenderDevice( S3DRenderDevice * pDevice )
{
	ItemBasicSerializer::OnSetRenderDevice( pDevice ) ;
	//
	if ( m_pComposition != nullptr )
	{
		m_pComposition->SetRenderDevice( pDevice ) ;
	}
	for ( size_t i = 0; i < m_aInstance.GetLength(); i ++ )
	{
		m_csLock.Lock() ;
		S3DSceneComposer::Composition *	pComp = m_aInstance.GetAt( i ) ;
		m_csLock.Unlock() ;
		if ( pComp != nullptr )
		{
			pComp->SetRenderDevice( pDevice ) ;
		}
	}
	for ( size_t i = 0; i < m_aCompositions.GetLength(); i ++ )
	{
		m_csLock.Lock() ;
		S3DSceneComposer::Composition *	pComp = m_aCompositions.GetAt( i ) ;
		m_csLock.Unlock() ;
		if ( pComp != nullptr )
		{
			pComp->SetRenderDevice( pDevice ) ;
		}
	}
}

// レンダリングの為のデバイスリソース準備
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompositionSerializer::OnPrepareToRender
	( S3DRenderDevice * pDevice, uint32_t nFlags )
{
	ItemBasicSerializer::OnPrepareToRender( pDevice, nFlags ) ;
	//
	S3DScene *	pScene = GetScene() ;
	//
	if ( !m_flagCompVisible && m_flagPoolInstance )
	{
		S3DSceneComposer::Composition *	pComp = CreateInstance() ;
		ESLAssert( pScene != nullptr ) ;
		if ( pComp != nullptr )
		{
			pComp->PrepareToRender( pDevice, pScene, nFlags ) ;
			ReleaseInstance( pComp ) ;
		}
	}
	//
	if ( m_pComposition != nullptr )
	{
		ESLAssert( pScene != nullptr ) ;
		m_pComposition->PrepareToRender( pDevice, pScene, nFlags ) ;
	}
	for ( size_t i = 0; i < m_aInstance.GetLength(); i ++ )
	{
		m_csLock.Lock() ;
		S3DSceneComposer::Composition *	pComp = m_aInstance.GetAt( i ) ;
		m_csLock.Unlock() ;
		if ( pComp != nullptr )
		{
			ESLAssert( pScene != nullptr ) ;
			pComp->PrepareToRender( pDevice, pScene, nFlags ) ;
		}
	}
	for ( size_t i = 0; i < m_aCompositions.GetLength(); i ++ )
	{
		m_csLock.Lock() ;
		S3DSceneComposer::Composition *	pComp = m_aCompositions.GetAt( i ) ;
		m_csLock.Unlock() ;
		if ( pComp != nullptr )
		{
			ESLAssert( pScene != nullptr ) ;
			pComp->PrepareToRender( pDevice, pScene, nFlags ) ;
		}
	}
}

// レンダリング前後処理（全視点共通）
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompositionSerializer::OnItemRenderEvent
	( S3DScene& scene, S3DScene::ItemClass clsItem )
{
	ItemBasicSerializer::OnItemRenderEvent( scene, clsItem ) ;
	//
	if ( m_pComposition != nullptr )
	{
		InvokeRenderSceneEventItems( scene, m_pComposition, clsItem ) ;
	}
	for ( size_t i = 0; i < m_aInstance.GetLength(); i ++ )
	{
		m_csLock.Lock() ;
		S3DSceneComposer::Composition *	pComp = m_aInstance.GetAt( i ) ;
		m_csLock.Unlock() ;
		if ( pComp != nullptr )
		{
			InvokeRenderSceneEventItems( scene, pComp, clsItem ) ;
		}
	}
	for ( size_t i = 0; i < m_aCompositions.GetLength(); i ++ )
	{
		m_csLock.Lock() ;
		S3DSceneComposer::Composition *	pComp = m_aCompositions.GetAt( i ) ;
		m_csLock.Unlock() ;
		if ( pComp != nullptr )
		{
			InvokeRenderSceneEventItems( scene, pComp, clsItem ) ;
		}
	}
}

// 当たり判定追加
//（必要であれば scene.PhysicsScene() へ Actor の追加も行う）
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompositionSerializer::OnItemRenderCollision
	( const S3DScene& scene, S3DCollision& render )
{
	ItemBasicSerializer::OnItemRenderCollision( scene, render ) ;
	//
	if ( m_pComposition != nullptr )
	{
		InvokeRenderSceneCollision
			( scene, render,
				m_pComposition, scene.GetCurrentCollisionMask() ) ;
	}
	for ( size_t i = 0; i < m_aInstance.GetLength(); i ++ )
	{
		S3DSceneComposer::Composition *	pComp = m_aInstance.GetAt( i ) ;
		if ( pComp != nullptr )
		{
			InvokeRenderSceneCollision
				( scene, render, pComp, scene.GetCurrentCollisionMask() ) ;
		}
	}
	for ( size_t i = 0; i < m_aCompositions.GetLength(); i ++ )
	{
		S3DSceneComposer::Composition *	pComp = m_aCompositions.GetAt( i ) ;
		if ( pComp != nullptr )
		{
			InvokeRenderSceneCollision
				( scene, render, pComp, scene.GetCurrentCollisionMask() ) ;
		}
	}
}

// 表示モデル追加
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompositionSerializer::OnItemRenderModel
	( const S3DScene& scene,
		S3DRenderContextInterface& render, uint64_t flagsExclusion )
{
	ItemBasicSerializer::OnItemRenderModel( scene, render, flagsExclusion ) ;
	//
	if ( m_pComposition != nullptr )
	{
		InvokeRenderSceneItems
			( scene, render, m_pComposition, flagsExclusion ) ;
	}
	for ( size_t i = 0; i < m_aInstance.GetLength(); i ++ )
	{
		S3DSceneComposer::Composition *	pComp = m_aInstance.GetAt( i ) ;
		if ( pComp != nullptr )
		{
			InvokeRenderSceneItems
				( scene, render, pComp, flagsExclusion ) ;
		}
	}
	for ( size_t i = 0; i < m_aCompositions.GetLength(); i ++ )
	{
		S3DSceneComposer::Composition *	pComp = m_aCompositions.GetAt( i ) ;
		if ( pComp != nullptr )
		{
			InvokeRenderSceneItems
				( scene, render, pComp, flagsExclusion ) ;
		}
	}
}

// アニメーション長取得
//////////////////////////////////////////////////////////////////////////////
bool S3DSubCompositionSerializer::GetTargetAnimationLength( double& secLength ) const
{
	if ( m_pciCompInfo == nullptr )
	{
		return	false ;
	}
	secLength = m_pciCompInfo->FrameIndexToSecond
				( (double) m_pciCompInfo->GetTotalFrameCount() ) ;
	return	true ;
}

// 全フレーム数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DSubCompositionSerializer::GetTargetAnimationFrames( void ) const
{
	if ( m_pciCompInfo == nullptr )
	{
		return	0 ;
	}
	return	(size_t) m_pciCompInfo->GetTotalFrameCount() ;
}

// ターゲット空間（逆変換用）
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompositionSerializer::GetTargetSpaceTransformation
		( S3DDMatrix& matITarget, S3DDVector& vITarget )
{
	matITarget = S3DDMatrix( 1, 1, 1 ) ;
	vITarget = - m_vParticleBase ;
}

// パーティクル追加
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompositionSerializer::AddParticles
	( size_t nCount,
		const S3DVector4 * pvPoints,
		const size_t * pFrames,
		const S3DColor * pColors,
		const float32_t * pZooms,
		const S4DVector * pFaceDirs,
		const float32_t * pxAspect )
{
	if ( nCount == 0 )
	{
		return ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	//
	const size_t	iBase = m_aParticleMatrics.GetLength() ;
	m_aParticleMatrics.SetLength( iBase + nCount ) ;
	m_aParticleColors.SetLength( iBase + nCount ) ;
	m_aParticleFrames.SetLength( iBase + nCount ) ;
	//
	S4DMatrix *	pBufMatrics = m_aParticleMatrics.GetAt( iBase ) ;
	S3DColor *	pBufColors = m_aParticleColors.GetAt( iBase ) ;
	size_t *	pBufFrames = m_aParticleFrames.GetAt( iBase ) ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		float32_t	zx = 1.0f, zy = 1.0f ;
		if ( pZooms != nullptr )
		{
			zx = pZooms[i] ;
			zy = zx ;
		}
		if ( pxAspect != nullptr )
		{
			zx *= pxAspect[i] ;
		}
		S3DMatrix	mat3( zx, zy, zy ) ;
		if ( pFaceDirs != nullptr )
		{
			mat3.InitializeMatrix( S3DVector( 1, 1, 1 ) ) ;
			mat3.RevolveForAngle( pFaceDirs[i] ) ;
			mat3.RevolveOnZ( sin(pFaceDirs[i].w), cos(pFaceDirs[i].w) ) ;
			mat3.MagnifyByVector( S3DVector( zx, zy, zy ) ) ;
		}
		S4DMatrix	mat4( 1, 1, 1, 1 ) ;
		mat4.SetMatrix3( mat3 ) ;
		mat4.SetTranslation( pvPoints[i] ) ;
		pBufMatrics[i] = mat4 ;
		//
		S3DColor	color( 0xFFFFFFFF, 0 ) ;
		if ( pColors != nullptr )
		{
			color = pColors[i] ;
		}
		pBufColors[i] = color ;
		//
		if ( pFrames != nullptr )
		{
			pBufFrames[i] = pFrames[i] ;
		}
		else
		{
			pBufFrames[i] = 0 ;
		}
	}
}

// AddIndexedParticles を使うか？
//////////////////////////////////////////////////////////////////////////////
bool S3DSubCompositionSerializer::IsUsingIndexedParticles( void )
{
	return	true ;
}

// パーティクル追加（高機能）（classPreRender で呼び出す）
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompositionSerializer::AddIndexedParticles
	( size_t nCount,
		const S3DVector4 * pvPoints,
		const S3DParticleSerializer::ParticleIndex * pIndexes,
		const size_t * pFrames,
		const S3DColor * pColors,
		const S3DMatrix * pFaceDirs )
{
	if ( nCount == 0 )
	{
		return ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	//
	const size_t	iBase = m_aParticleMatrics.GetLength() ;
	m_aParticleMatrics.SetLength( iBase + nCount ) ;
	m_aParticleColors.SetLength( iBase + nCount ) ;
	m_aParticleFrames.SetLength( iBase + nCount ) ;
	//
	S4DMatrix *	pBufMatrics = m_aParticleMatrics.GetAt( iBase ) ;
	S3DColor *	pBufColors = m_aParticleColors.GetAt( iBase ) ;
	size_t *	pBufFrames = m_aParticleFrames.GetAt( iBase ) ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		S4DMatrix	mat4( 1, 1, 1, 1 ) ;
		if ( pFaceDirs != nullptr )
		{
			mat4.SetMatrix3( pFaceDirs[i] ) ;
		}
		mat4.SetTranslation( pvPoints[pIndexes[i].nIndex] ) ;
		pBufMatrics[i] = mat4 ;
		//
		S3DColor	color( 0xFFFFFFFF, 0 ) ;
		if ( pColors != nullptr )
		{
			color = pColors[i] ;
		}
		pBufColors[i] = color ;
		//
		if ( pFrames != nullptr )
		{
			pBufFrames[i] = pFrames[i] ;
		}
		else
		{
			pBufFrames[i] = 0 ;
		}
	}
}

// 動的インスタンス追加（classPreRender で呼び出す）
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompositionSerializer::AddDynamicInstancingEntries
	( const S4DMatrix * pMatrixs,
			const S3DColor * pColors, size_t nCount, ESLObject * pSrcItem )
{
	if ( nCount == 0 )
	{
		return ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	//
	const size_t	iBase = m_aParticleMatrics.GetLength() ;
	m_aParticleMatrics.SetLength( iBase + nCount ) ;
	m_aParticleColors.SetLength( iBase + nCount ) ;
	m_aParticleFrames.SetLength( iBase + nCount ) ;
	//
	S4DMatrix *	pBufMatrics = m_aParticleMatrics.GetAt( iBase ) ;
	S3DColor *	pBufColors = m_aParticleColors.GetAt( iBase ) ;
	size_t *	pBufFrames = m_aParticleFrames.GetAt( iBase ) ;
	size_t		iFrame = (size_t) m_fpTimemapFrame ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pBufMatrics[i] = pMatrixs[i] ;
		pBufColors[i] = pColors[i] ;
		pBufFrames[i] = iFrame ;
	}
}

// S3DItemInstancingSerializer 取得
//////////////////////////////////////////////////////////////////////////////
S3DItemInstancingSerializer * S3DSubCompositionSerializer::GetInstancing( void )
{
	return	nullptr ;
}



//////////////////////////////////////////////////////////////////////////////
// サブコンポジション・インスタンス
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DSubCompInstanceSerializer::m_paramEntries[S3DSubCompInstanceSerializer::paramCompInstanceCount] =
{
	{ L"sub_comp",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration,	L"サブコンポジション", nullptr },
	{ L"create",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory1, L"生成", nullptr },
	{ L"force_release",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"強制解放", L"生成可能区間以外ではインスタンスを強制破棄する" },
	{ L"track_pos",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory1, L"位置・行列反映", nullptr },
	{ L"recreate_limit",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1, L"自動再生成回数", nullptr },
} ;

const S3DSceneComposer::ParamSetClass	S3DSubCompInstanceSerializer::m_pscClass =
{
	&S3DSubCompInstanceSerializer::ItemCommonSerializer::m_pscClass,
	S3DSubCompInstanceSerializer::paramCompInstanceCount,
	&S3DSubCompInstanceSerializer::m_paramEntries[0]
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSubCompInstanceSerializer, ItemBasicSerializer )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DSubCompInstanceSerializer, subcomp_instance )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSubCompInstanceSerializer::S3DSubCompInstanceSerializer( void )
	: ItemBasicSerializer
		( m_ItemClassDescriptor.pwszClassID,
				&S3DSubCompInstanceSerializer::m_pscClass ),
		m_pInstance( nullptr ),
		m_flagCreateComp( true ),
		m_flagForceRelease( false ),
		m_flagTrackPos( true ),
		m_nAutoRecreateLimit( 0 ),
		m_nCreatedCount( 0 )
{
	m_classItem = S3DScene::classDynamicItem1 ;
	m_flagsBehavior = S3DScene::itemVisible | S3DScene::itemTimer ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSubCompInstanceSerializer::~S3DSubCompInstanceSerializer( void )
{
}

// サブコンポジション参照
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompInstanceSerializer::UpdateSubCompRef( void )
{
	ReleaseInstance() ;

	m_refSubComp = nullptr ;
	if ( m_strSubComp.IsEmpty() )
	{
		return ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		m_refSubComp = pComp->GetSceneItemAs( m_strSubComp ) ;
	}
}

// インスタンス解放
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompInstanceSerializer::ReleaseInstance( void )
{
	if ( m_pInstance != nullptr )
	{
		S3DSubCompositionSerializer *	pSubComp =
				m_refSubComp.GetRef<S3DSubCompositionSerializer>() ;
		if ( pSubComp != nullptr )
		{
			pSubComp->ReleaseInstance( m_pInstance ) ;
		}
		m_pInstance = nullptr ;
	}
}

// 有効なインスタンス取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Composition * S3DSubCompInstanceSerializer::GetValidInstance( void ) const
{
	if ( m_pInstance != nullptr )
	{
		S3DSubCompositionSerializer *	pSubComp =
				m_refSubComp.GetRef<S3DSubCompositionSerializer>() ;
		if ( (pSubComp != nullptr) && pSubComp->IsValidInstance( m_pInstance ) )
		{
			return	m_pInstance ;
		}
	}
	return	nullptr ;
}

// インスタンス生成
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Composition * S3DSubCompInstanceSerializer::CreateInstance( void )
{
	ReleaseInstance() ;

	S3DSubCompositionSerializer *	pSubComp =
			m_refSubComp.GetRef<S3DSubCompositionSerializer>() ;
	if ( pSubComp != nullptr )
	{
		m_pInstance = pSubComp->CreateInstance() ;
		if ( m_pInstance != nullptr )
		{
			ReflectInstanceParameter( *m_pInstance ) ;
		}
	}
	return	m_pInstance ;
}

// インスタンスに行列反映
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompInstanceSerializer::ReflectInstanceParameter( S3DSceneComposer::Composition& comp )
{
	S3DDMatrix	matOffset( 1, 1, 1 ) ;
	S3DDVector	vOffset( 0, 0, 0 ) ;

	S3DSubCompositionSerializer *	pSubComp =
			m_refSubComp.GetRef<S3DSubCompositionSerializer>() ;
	if ( pSubComp != nullptr )
	{
		pSubComp->GetTransformationFrom( matOffset, vOffset, this ) ;
	}

	S3DColor	clrEffect ;
	GetGlobalColorEffect( clrEffect ) ;

	comp.SetItemPositioin( vOffset ) ;
	comp.SetItemMatrix( matOffset ) ;
	comp.SpaceParameter().m_colorEffect = clrEffect ;
	comp.SpaceParameter().m_nTransparency = 0xFF - clrEffect.rgbMul.argb.Alpha ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
int32_t S3DSubCompInstanceSerializer::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramAutoRecreateCount:
		return	(int32_t) m_nAutoRecreateLimit ;
	}
	return	ItemBasicSerializer::GetIntegerParameter( i ) ;
}

bool S3DSubCompInstanceSerializer::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramCreateComp:
		return	m_flagCreateComp ;
	case	paramForceReleaseComp:
		return	m_flagForceRelease ;
	case	paramTrackPosition:
		return	m_flagTrackPos ;
	}
	return	ItemBasicSerializer::GetBooleanParameter( i ) ;
}

const wchar_t * S3DSubCompInstanceSerializer::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramComposition:
		return	m_strSubComp ;
	}
	return	ItemBasicSerializer::GetCommandParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompInstanceSerializer::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramAutoRecreateCount:
		m_nAutoRecreateLimit = (size_t) n ;
		return ;
	}
	ItemBasicSerializer::SetIntegerParameter( i, n ) ;
}

void S3DSubCompInstanceSerializer::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramCreateComp:
		m_flagCreateComp = b ;
		return ;
	case	paramForceReleaseComp:
		m_flagForceRelease = b ;
		return ;
	case	paramTrackPosition:
		m_flagTrackPos = b ;
		return ;
	}
	ItemBasicSerializer::SetBooleanParameter( i, b ) ;
}

void S3DSubCompInstanceSerializer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramComposition:
		if ( m_strSubComp != pwszCmd )
		{
			m_strSubComp = pwszCmd ;
			UpdateSubCompRef() ;
		}
		return ;
	}
	ItemBasicSerializer::SetCommandParameter( i, pwszCmd ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DSubCompInstanceSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	S3DSceneComposer::Composition *	pComp ;
	switch ( i )
	{
	case	paramComposition:
		pComp = GetComposition() ;
		if ( pComp != nullptr )
		{
			pComp->EnumerateItemIDsAs
				( aStrSet, ESL_RUNTIME_CLASS(S3DSubCompositionSerializer) ) ;
		}
		return	true ;
	}
	return	ItemBasicSerializer::EnumerateStringSet( i, aStrSet ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSubCompInstanceSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"サブコンポジション" ;
	}
	return	ItemBasicSerializer::GetParameterCategoryName( iCategory ) ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DSubCompInstanceSerializer::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramForceToon:
	case	paramForceBorder:
	case	paramFreeToon:
	case	paramFreeBorder:
	case	paramUseCollision:
	case	paramGlobalSpace:
	case	paramCameraShift:
	case	paramCameraSpace:
	case	paramHideNear:
	case	paramHideFar:
	case	paramItemClass:
	case	paramItemPriority:
		return	false ;
	}
	return	ItemBasicSerializer::IsParameterValidation( i ) ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DSubCompInstanceSerializer::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp, uint32_t nFlags )
{
	uint32_t	nResFlags =
		ItemBasicSerializer::UpdatePropertyReference( comp, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefItem )
	{
		UpdateSubCompRef() ;
	}
	return	nResFlags ;
}

// 拡張的な処理の通知
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompInstanceSerializer::OnExtendNotify
	( const wchar_t * pwszCmd, const wchar_t * pwszParam,
		const void * pExParam, size_t nExParamBytes )
{
	ItemBasicSerializer::OnExtendNotify
			( pwszCmd, pwszParam, pExParam, nExParamBytes ) ;
	//
	if ( SString::Compare( pwszCmd, S3DSceneComposer::CmdInitializeItem ) == 0 )
	{
	}
	else if ( SString::Compare( pwszCmd, S3DSceneComposer::CmdFinishItem ) == 0 )
	{
		ReleaseInstance() ;
	}
	else if ( SString::Compare( pwszCmd, S3DSceneComposer::CmdStartItem ) == 0 )
	{
	}
	else if ( SString::Compare( pwszCmd, S3DSceneComposer::CmdStopItem ) == 0 )
	{
		ReleaseInstance() ;
	}
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompInstanceSerializer::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	ItemBasicSerializer::OnTimer( scene, msecPast ) ;

	S3DSceneComposer::Composition *	pComp = GetValidInstance() ;
	if ( pComp != nullptr )
	{
		if ( !pComp->IsPlayingComposition() )
		{
			ReleaseInstance() ;
		}
	}
}

// フレーム更新後処理
//////////////////////////////////////////////////////////////////////////////
void S3DSubCompInstanceSerializer::OnUpdateFrame
	( double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	ItemBasicSerializer::OnUpdateFrame( fpFrame, seek ) ;

	if ( seek == S3DSceneComposer::seekJumpReset )
	{
		m_nCreatedCount = 0 ;
	}

	S3DSceneComposer::Composition *	pComp = GetValidInstance() ;
	if ( m_flagCreateComp )
	{
		if ( (pComp == nullptr)
			&& (m_nCreatedCount < m_nAutoRecreateLimit + 1) )
		{
			pComp = CreateInstance() ;
			m_nCreatedCount ++ ;
		}
	}
	else
	{
		if ( m_flagForceRelease && (pComp != nullptr) )
		{
			ReleaseInstance() ;
			pComp = nullptr ;
		}
		m_nCreatedCount = 0 ;
	}
	if ( m_flagTrackPos )
	{
		if ( pComp != nullptr )
		{
			ReflectInstanceParameter( *pComp ) ;
		}
	}
}



//////////////////////////////////////////////////////////////////////////////
// サウンドアイテム
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DSoundItemSerializer::m_paramEntries[S3DSoundItemSerializer::paramSoundCount] =
{
	{ L"sound",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration,	L"サウンド", nullptr },
	{ L"volume",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrUIScalarSlider,	L"音量", nullptr, 0.0, 1.0 },
	{ L"volume_line",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration
		| S3DSceneComposer::attrUIOnlyEnumeration,	L"音量ライン", nullptr },
	{ L"loop",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory1, L"ループ", nullptr },
	{ L"play",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory1, L"再生", nullptr },
	{ L"seek",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,
		L"再生位置", L"再生開始時の時間[秒]" },
	{ L"max_instance",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1,
		L"同時再生可能数", L"動的な同時再生可能最大数" },
	{ L"fade_volume",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrDynamicValidation,
		L"有効距離指定", L"音源から有効距離範囲外（有効距離＋フェード距離）の時に音量を 0.0 にします。" },
	{ L"auto_play",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"自動再生停止",
		L"音源から有効距離範囲内（有効距離＋フェード距離）で自動的に再生を開始し、"
		L"範囲外で自動的に停止します。" },
	{ L"fade_reach",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1, L"有効距離" },
	{ L"fade_latitude",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1, L"有効フェード距離",
		L"音源から有効距離以上離れた場合にフェード距離の区間で音量をフェードします。" },
	{ L"env_volume",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrDynamicValidation,
		L"距離効果無効", L"距離と方角に応じた音量効果を無効化します" },
	{ L"attenuation_power",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrUIScalarSlider,
		L"減衰指数", L"距離に対する減衰空間の次元。\n"
		L"3次元空間では通常 2.0。\n0.0 の時は減衰しない。", 0.0, 2.0 },
	{ L"attenuation_distance",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1,
		L"減衰基準距離", L"マイクからこの距離離れた時に音量が x1.0 倍になる距離。" },
	{ L"directional",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrDynamicValidation, L"指向性音源" },
	{ L"direction",
		S3DSceneComposer::typeDirection,
		S3DSceneComposer::attrConstant1, L"指向性ベクトル", nullptr },
	{ L"cone_angle",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrUIScalarSlider,
		L"指向性範囲角", L"指向性音源の範囲角 [deg]", 0.0, 180.0 },
	{ L"angle_gradation",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrUIScalarSlider,
		L"指向性ぼかし角",
		L"指向性音源の範囲角外のグラデーション角 [deg]", 0.0, 180.0 },
	{ L"base_volume",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrUIScalarSlider,
		L"指向性外音量", L"指向性音源の範囲外の音量", 0.0, 1.0 },
} ;

const S3DSceneComposer::ParamSetClass	S3DSoundItemSerializer::m_pscClass =
{
	&S3DSceneComposer::ItemCommonSerializer::m_pscClass,
	S3DSoundItemSerializer::paramSoundCount,
	&S3DSoundItemSerializer::m_paramEntries[0]
} ;

const SSystem::SXMLDocument::AttrInteger	S3DSoundItemSerializer::m_aiVolumeLine[32] =
{
	{ L"default", -1 },
	{ L"composition", SGLAudioPlayer::lineComposition },
	{ L"system", SGLAudioPlayer::lineSystem },
	{ L"music", SGLAudioPlayer::lineMusic },
	{ L"sound", SGLAudioPlayer::lineSound },
	{ L"voice", SGLAudioPlayer::lineVoice },
	{ L"user0", SGLAudioPlayer::lineUserFirst },
	{ L"user1", SGLAudioPlayer::lineUserFirst+1 },
	{ L"user2", SGLAudioPlayer::lineUserFirst+2 },
	{ L"user3", SGLAudioPlayer::lineUserFirst+3 },
	{ L"user4", SGLAudioPlayer::lineUserFirst+4 },
	{ L"user5", SGLAudioPlayer::lineUserFirst+5 },
	{ L"user6", SGLAudioPlayer::lineUserFirst+6 },
	{ L"user7", SGLAudioPlayer::lineUserFirst+7 },
	{ L"user8", SGLAudioPlayer::lineUserFirst+8 },
	{ L"user9", SGLAudioPlayer::lineUserFirst+9 },
	{ L"user10", SGLAudioPlayer::lineUserFirst+10 },
	{ L"user11", SGLAudioPlayer::lineUserFirst+11 },
	{ L"user12", SGLAudioPlayer::lineUserFirst+12 },
	{ L"user13", SGLAudioPlayer::lineUserFirst+13 },
	{ L"user14", SGLAudioPlayer::lineUserFirst+14 },
	{ L"user15", SGLAudioPlayer::lineUserFirst+15 },
	{ nullptr, 0 },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DSoundItemSerializer::Instance, SObject )
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DSoundItemSerializer, SoundItem, ItemCommonSerializer )
S3D_IMPLEMENT_COMPOSER_ITEM( S3DSoundItemSerializer, sound_item )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSoundItemSerializer::S3DSoundItemSerializer( void )
	: ItemCommonSerializer
			( m_ItemClassDescriptor.pwszClassID,
						&S3DSoundItemSerializer::m_pscClass, nullptr ),
		m_pAudioRsrc( nullptr ), m_iVolumeLine( -1 ),
		m_flagScenePlaying( false ),
		m_flagPlay( false ), m_flagPlaying( false ),
		m_flagLoop( false ), m_flagAutoPlay( false ),
		m_secSeek( 0.0 ), m_nMaxInstance( 4 ),
		m_degConeAngle( 90.0 ), m_degAngleGradation( 90.0 )
{
	AttachSceneItem( (SoundItem*) this ) ;
	SetDirection
		( S3DDVector( 0, 0, 1 ),
			m_degConeAngle, m_degAngleGradation, m_fpBaseVolume ) ;
}

S3DSoundItemSerializer::S3DSoundItemSerializer
	( const wchar_t * pwszClassID,
		const S3DSceneComposer::ParamSetClass * pClass )
	: ItemCommonSerializer( pwszClassID, pClass, nullptr ),
		m_pAudioRsrc( nullptr ), m_iVolumeLine( -1 ),
		m_flagScenePlaying( false ),
		m_flagPlay( false ), m_flagPlaying( false ),
		m_flagLoop( false ), m_flagAutoPlay( false ),
		m_secSeek( 0.0 ), m_nMaxInstance( 4 ),
		m_degConeAngle( 90.0 ), m_degAngleGradation( 90.0 )
{
	AttachSceneItem( (SoundItem*) this ) ;
	SetDirection
		( S3DDVector( 0, 0, 1 ),
			m_degConeAngle, m_degAngleGradation, m_fpBaseVolume ) ;
}

// サウンド
//////////////////////////////////////////////////////////////////////////////
void S3DSoundItemSerializer::SetSound( const wchar_t * pwszSoundID )
{
	if ( m_strSoundID != pwszSoundID )
	{
		m_strSoundID = pwszSoundID ;
		UpdateSoundRef() ;
	}
}

void S3DSoundItemSerializer::UpdateSoundRef( void )
{
	if ( m_strSoundID.IsEmpty() )
	{
		AttachAudioPlayer( nullptr ) ;
		m_pAudioRsrc = nullptr ;
		return ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		S3DSceneComposer *	pSceneComp = pComp->GetSceneComposer() ;
		if ( pSceneComp != nullptr )
		{
			SGLAudioPlayer *
				pAudio = pSceneComp->Assets().GetAudioAs( m_strSoundID ) ;
			if ( pAudio != m_pAudioRsrc )
			{
				m_csSync.Lock() ;
				m_pAudioRsrc = pAudio ;
				m_aPlayerStock.RemoveAll() ;
				m_aInstance.RemoveAll() ;
				AttachAudioPlayer( nullptr ) ;
				m_flagPlaying = false ;
				m_csSync.Unlock() ;
			}
		}
	}
}

const wchar_t * S3DSoundItemSerializer::GetSoundID( void ) const
{
	return	m_strSoundID ;
}

SGLAudioPlayer * S3DSoundItemSerializer::GetAudioResource( void ) const
{
	return	m_pAudioRsrc ;
}

// SGLAudioPlayerInterface 参照プレーヤー生成
//////////////////////////////////////////////////////////////////////////////
SGLAudioPlayerInterface * S3DSoundItemSerializer::CreateAudioPlayer( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	SGLAudioPlayerInterface *	pPlayer = m_aPlayerStock.Pop() ;
	if ( pPlayer != nullptr )
	{
		return	pPlayer ;
	}
	if ( m_pAudioRsrc != nullptr )
	{
		return	m_pAudioRsrc->ClonePlayer() ;
	}
	return	nullptr ;
}

// SGLAudioPlayerInterface 参照プレーヤー破棄
//////////////////////////////////////////////////////////////////////////////
void S3DSoundItemSerializer::ReleaseAudioPlayer( SGLAudioPlayerInterface * pPlayer )
{
	if ( pPlayer != nullptr )
	{
		pPlayer->Stop() ;

		m_csSync.Lock() ;
		m_aPlayerStock.Push( pPlayer ) ;
		m_csSync.Unlock() ;
	}
}

// 自動削除インスタンス再生
//////////////////////////////////////////////////////////////////////////////
S3DSoundItemSerializer::Instance *
	S3DSoundItemSerializer::PlayTemporary
		( double fpSubVolume,
			S3DSceneComposer::ItemSerializer * pRefItem,
			const S3DDMatrix * pMatrix, const S3DDVector * pPos )
{
	uint32_t	nFlags = instanceAutoDeleteOnEnd ;
	if ( pRefItem != nullptr )
	{
		nFlags |= instanceAutoDeleteWithItem ;
	}
	LockInstance() ;

	Instance *	pInstance =
		CreateInstance( nFlags, fpSubVolume, pRefItem, pMatrix, pPos ) ;
	PlayInstance( pInstance ) ;

	UnlockInstance() ;
	return	pInstance ;
}

// インスタンス生成
//////////////////////////////////////////////////////////////////////////////
S3DSoundItemSerializer::Instance *
	S3DSoundItemSerializer::CreateInstance
		( uint32_t nFlags, double fpSubVolume,
			S3DSceneComposer::ItemSerializer * pRefItem,
			const S3DDMatrix * pMatrix, const S3DDVector * pPos )
{
	Instance *	pInstance = new Instance ;
	pInstance->m_fpVolume = fpSubVolume ;
	pInstance->m_nFlags = nFlags ;
	pInstance->m_pPlayer = CreateAudioPlayer() ;
	pInstance->m_refItem = pRefItem ;
	if ( pMatrix != nullptr )
	{
		pInstance->m_matRotate = *pMatrix ;
	}
	if ( pPos != nullptr )
	{
		pInstance->m_vPos = *pPos ;
	}
	m_csSync.Lock() ;
	m_aInstance.InsertAt( 0, pInstance ) ;
	if ( (m_aInstance.GetLength() > m_nMaxInstance) && (m_nMaxInstance >= 1) )
	{
		RemoveInstance( m_aInstance.GetAt( m_nMaxInstance ) ) ;
		m_aInstance.SetLength( m_nMaxInstance ) ;
	}
	m_csSync.Unlock() ;
	return	pInstance ;
}

// インスタンス削除
//////////////////////////////////////////////////////////////////////////////
void S3DSoundItemSerializer::RemoveInstance
	( S3DSoundItemSerializer::Instance * pInstance )
{
	m_csSync.Lock() ;
	ssize_t	i = m_aInstance.FindPtr( pInstance ) ;
	if ( i >= 0 )
	{
		ReleaseAudioPlayer( pInstance->m_pPlayer.Detach() ) ;
		m_aInstance.RemoveAt( (size_t) i ) ;
	}
	m_csSync.Unlock() ;
}

// 再生開始
//////////////////////////////////////////////////////////////////////////////
void S3DSoundItemSerializer::PlayInstance
	( S3DSoundItemSerializer::Instance * pInstance )
{
	m_csSync.Lock() ;
	if ( m_aInstance.FindPtr( pInstance ) >= 0 )
	{
		S3DDMatrix	matItem( 1, 1, 1 ) ;
		S3DDVector	vItem( 0, 0, 0 ) ;
		S3DSceneComposer::ItemSerializer *
				pItem = pInstance->m_refItem.GetReference() ;
		if ( pItem != nullptr )
		{
			pItem->GetGlobalTransformation( matItem, vItem ) ;
		}
		else
		{
			GetGlobalTransformation( matItem, vItem ) ;
		}
		S3DDMatrix	matInstance = matItem * pInstance->m_matRotate ;
		S3DDVector	vInstance = matItem * pInstance->m_vPos + vItem ;
		SGLAudioPlayerInterface *
					pPlayer = pInstance->m_pPlayer ;
		if ( pPlayer == nullptr )
		{
			pPlayer = CreateAudioPlayer() ;
			pInstance->m_pPlayer = pPlayer ;
		}
		if ( pPlayer != nullptr )
		{
			if ( m_iVolumeLine >= 0 )
			{
				SGLAudioPlayer::SetAudioLineMask( pPlayer, (1 << m_iVolumeLine) ) ;
			}
			ApplySoundInstanceVolume
				( matInstance, vInstance, pPlayer, pInstance->m_fpVolume ) ;
			//
			pPlayer->SetLoop( m_flagLoop ) ;
			pPlayer->SeekPosition
				( (uint64_t) (pPlayer->GetSampleFrequency() * m_secSeek) ) ;
			pPlayer->Play() ;
		}
	}
	m_csSync.Unlock() ;
}

// 再生開始
//////////////////////////////////////////////////////////////////////////////
void S3DSoundItemSerializer::OnPlayStart( void )
{
	SGLAudioPlayerInterface *	pPlayer = GetAudioPlayer() ;
	if ( pPlayer == nullptr )
	{
		pPlayer = CreateAudioPlayer() ;
		SetSmartAudioPlayer( pPlayer ) ;
	}
	if ( pPlayer != nullptr )
	{
		S3DDMatrix	matItem ;
		S3DDVector	vItem ;
		CalcGlobalTransformation( matItem, vItem ) ;
		//
		ApplySoundInstanceVolume( matItem, vItem, pPlayer, 1.0 ) ;
		//
		pPlayer->SetLoop( m_flagLoop ) ;
		pPlayer->SeekPosition
			( (uint64_t) (pPlayer->GetSampleFrequency() * m_secSeek) ) ;
		pPlayer->Play() ;
	}
}

// 単体音量反映
//////////////////////////////////////////////////////////////////////////////
void S3DSoundItemSerializer::ApplySoundInstanceVolume
	( const S3DDMatrix& matInstance,
		const S3DDVector& vInstance,
		SGLAudioPlayerInterface * pPlayer, double fpSubVolume ) const
{
	S3DScene *			pScene = ItemCommonSerializer::GetScene() ;
	S3DScene::Camera *	pCamera = nullptr ;
	if ( pScene != nullptr )
	{
		pCamera = pScene->GetSoundCamera() ;
	}
	if ( pCamera != nullptr )
	{
		S3DDMatrix	matCamera ;
		S3DDVector	vCamera ;
		S3DScene::CalcCameraTransformation
					( matCamera, vCamera, pCamera ) ;
		//
		ApplySoundVolume
			( matCamera, vCamera,
				matInstance, vInstance,
				pCamera, fpSubVolume, pPlayer ) ;
	}
	else
	{
		float32_t	fpVol[2] ;
		fpVol[0] = (float32_t) (m_fpVolume * fpSubVolume) ;
		fpVol[1] = fpVol[0] ;
		pPlayer->SetVolume( fpVol, 2 ) ;
	}
}

// インスタンスが存在するか？
//////////////////////////////////////////////////////////////////////////////
bool S3DSoundItemSerializer::IsValidInstance
	( S3DSoundItemSerializer::Instance * pInstance ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csSync ) ;
	return	(m_aInstance.FindPtr( pInstance ) >= 0) ;
}

// インスタンス座標変更
//////////////////////////////////////////////////////////////////////////////
void S3DSoundItemSerializer::SetInstancePosition
	( S3DSoundItemSerializer::Instance * pInstance, const S3DDVector& vPos ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csSync ) ;
	if ( m_aInstance.FindPtr( pInstance ) >= 0 )
	{
		pInstance->m_vPos = vPos ;
	}
}

// インスタンス再生時間と再生中か照会
//////////////////////////////////////////////////////////////////////////////
bool S3DSoundItemSerializer::GetPlayingTimeOfInstance
	( double& secPlaying, S3DSoundItemSerializer::Instance * pInstance ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csSync ) ;
	if ( m_aInstance.FindPtr( pInstance ) >= 0 )
	{
		SGLAudioPlayerInterface *	pPlayer = pInstance->m_pPlayer ;
		if ( pPlayer != nullptr )
		{
			uint64_t	nPos = pPlayer->GetPosition() ;
			uint32_t	nFreq = pPlayer->GetSampleFrequency() ;
			secPlaying = (double) nPos / nFreq ;
			return	pPlayer->IsPlaying() ;
		}
	}
	return	false ;
}

// インスタンス操作同期用
//////////////////////////////////////////////////////////////////////////////
void S3DSoundItemSerializer::LockInstance( void ) const
{
	m_csSync.Lock() ;
}

void S3DSoundItemSerializer::UnlockInstance( void ) const
{
	m_csSync.Unlock() ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DSoundItemSerializer::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramDirection:
		return	m_vDirection ;
	}
	return	ItemCommonSerializer::GetVectorParameter( i ) ;
}

double S3DSoundItemSerializer::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramVolume:
		return	m_fpVolume ;

	case	paramSeek:
		return	m_secSeek ;

	case	paramFadeReach:
		return	m_fpFadeReach ;

	case	paramFadeLatitude:
		return	m_fpFadeLatitude ;

	case	paramAttenuationPower:
		return	m_fpAttenuationPower ;

	case	paramAttenuationDistance:
		return	m_fpAttenuationDistance ;

	case	paramConeAngle:
		return	m_degConeAngle ;

	case	paramAngleGradation:
		return	m_degAngleGradation ;

	case	paramBaseVolume:
		return	m_fpBaseVolume ;
	}
	return	ItemCommonSerializer::GetScalarParameter( i ) ;
}

int32_t S3DSoundItemSerializer::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramMaxInstance:
		return	(int32_t) m_nMaxInstance ;
	}
	return	ItemCommonSerializer::GetIntegerParameter( i ) ;
}

bool S3DSoundItemSerializer::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramLoop:
		return	m_flagLoop ;

	case	paramPlay:
		return	m_flagPlay ;

	case	paramFadeVolume:
		return	(m_nSoundFlags & soundFadeReach) != 0 ;

	case	paramFadeAutoPlay:
		return	m_flagAutoPlay ;

	case	paramEnvVolume:
		return	(m_nSoundFlags & soundEnvironment) != 0 ;

	case	paramDirectional:
		return	(m_nSoundFlags & soundDirectional) != 0 ;
	}
	return	ItemCommonSerializer::GetBooleanParameter( i ) ;
}

const wchar_t * S3DSoundItemSerializer::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramSound:
		return	m_strSoundID ;

	case	paramVolumeLine:
		return	SXMLDocument::GetSymbolAsIntegerOf( m_aiVolumeLine, m_iVolumeLine ) ;
	}
	return	ItemCommonSerializer::GetCommandParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSoundItemSerializer::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramDirection:
		m_vDirection = vec ;
		return ;
	}
	ItemCommonSerializer::SetVectorParameter( i, vec ) ;
}

void S3DSoundItemSerializer::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramVolume:
		m_fpVolume  = s ;
		return ;

	case	paramSeek:
		m_secSeek = s ;
		return ;

	case	paramFadeReach:
		m_fpFadeReach = s ;
		return ;

	case	paramFadeLatitude:
		m_fpFadeLatitude = s ;
		return ;

	case	paramAttenuationPower:
		m_fpAttenuationPower = s ;
		return ;

	case	paramAttenuationDistance:
		m_fpAttenuationDistance = s ;
		return ;

	case	paramConeAngle:
		m_degConeAngle = s ;
		SetDirection
			( m_vDirection,
				m_degConeAngle, m_degAngleGradation, m_fpBaseVolume ) ;
		return ;

	case	paramAngleGradation:
		m_degAngleGradation = s ;
		SetDirection
			( m_vDirection,
				m_degConeAngle, m_degAngleGradation, m_fpBaseVolume ) ;
		return ;

	case	paramBaseVolume:
		m_fpBaseVolume = s ;
		return ;
	}
	ItemCommonSerializer::SetScalarParameter( i, s ) ;
}

void S3DSoundItemSerializer::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramMaxInstance:
		m_nMaxInstance = (size_t) n ;
		return ;
	}
	ItemCommonSerializer::SetIntegerParameter( i, n ) ;
}

void S3DSoundItemSerializer::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramLoop:
		m_flagLoop = b ;
		return ;

	case	paramPlay:
		m_flagPlay = b ;
		return ;

	case	paramFadeVolume:
		if ( b )
		{
			m_nSoundFlags |= soundFadeReach ;
		}
		else
		{
			m_nSoundFlags &= ~soundFadeReach ;
		}
		return ;

	case	paramFadeAutoPlay:
		m_flagAutoPlay = b ;
		return ;

	case	paramEnvVolume:
		if ( b )
		{
			m_nSoundFlags |= soundEnvironment ;
		}
		else
		{
			m_nSoundFlags &= ~soundEnvironment ;
		}
		return ;

	case	paramDirectional:
		if ( b )
		{
			m_nSoundFlags |= soundDirectional ;
		}
		else
		{
			m_nSoundFlags &= ~soundDirectional ;
		}
		return ;
	}
	ItemCommonSerializer::SetBooleanParameter( i, b ) ;
}

void S3DSoundItemSerializer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramSound:
		SetSound( pwszCmd ) ;
		return ;

	case	paramVolumeLine:
		m_iVolumeLine =
			(int32_t) SXMLDocument::GetIntegerAsSymbolOf
							( m_aiVolumeLine, pwszCmd, m_iVolumeLine ) ;
		return ;
	}
	ItemCommonSerializer::SetCommandParameter( i, pwszCmd ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DSoundItemSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	S3DSceneComposer::Composition *	pComp ;
	switch ( i )
	{
	case	paramSound:
		pComp = GetComposition() ;
		if ( pComp != nullptr )
		{
			S3DSceneComposer *	pSceneComp = pComp->GetSceneComposer() ;
			if ( pSceneComp != nullptr )
			{
				pSceneComp->Assets().EnumerateResourceIDsAs
					( aStrSet, ESL_RUNTIME_CLASS(SGLAudioPlayer) ) ;
			}
		}
		return	true ;

	case	paramVolumeLine:
		for ( int j = 0; m_aiVolumeLine[j].pszSymbol != nullptr; j ++ )
		{
			aStrSet.Add( new SString( m_aiVolumeLine[j].pszSymbol ) ) ;
		}
		return	true ;
	}
	return	ItemCommonSerializer::EnumerateStringSet( i, aStrSet ) ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DSoundItemSerializer::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramZoom:
	case	paramTransparency:
	case	paramColorMul:
	case	paramColorAdd:
	case	paramVisible:
	case	paramForceToon:
	case	paramForceBorder:
	case	paramFreeToon:
	case	paramFreeBorder:
	case	paramUseCollision:
	case	paramHideNear:
	case	paramHideFar:
		return	false ;

	case	paramFadeAutoPlay:
	case	paramFadeReach:
	case	paramFadeLatitude:
		return	(m_nSoundFlags & soundFadeReach) != 0 ;

	case	paramAttenuationPower:
	case	paramAttenuationDistance:
	case	paramDirectional:
		return	(m_nSoundFlags & soundEnvironment) == 0 ;

	case	paramDirection:
	case	paramConeAngle:
	case	paramAngleGradation:
	case	paramBaseVolume:
		return	((m_nSoundFlags & soundEnvironment) == 0)
				&& ((m_nSoundFlags & soundDirectional) != 0) ;
	}
	return	ItemCommonSerializer::IsParameterValidation( i ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSoundItemSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	if ( iCategory == 1 )
	{
		return	L"サウンド設定" ;
	}
	return	ItemCommonSerializer::GetParameterCategoryName( iCategory ) ;
}

// タイマ処理 Item::OnTimer / ItemSerializer::OnTimer 実装
//////////////////////////////////////////////////////////////////////////////
void S3DSoundItemSerializer::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	S3DDMatrix	matCamera( 1, 1, 1 ) ;
	S3DDVector	vCamera( 0, 0, 0 ) ;
	S3DScene::Camera *	pCamera = scene.GetSoundCamera() ;
	if ( pCamera != nullptr )
	{
		S3DScene::CalcCameraTransformation
					( matCamera, vCamera, pCamera ) ;
	}
	S3DDMatrix	matItem ;
	S3DDVector	vItem ;
	CalcGlobalTransformation( matItem, vItem ) ;

	if ( m_flagScenePlaying )
	{
		if ( (m_nSoundFlags & soundFadeReach) && m_flagAutoPlay )
		{
			S3DDVector	vViewItem = matCamera * vItem - vCamera ;
			double	r = vViewItem.Absolute() ;
			if ( r < m_fpFadeReach + m_fpFadeLatitude )
			{
				SGLAudioPlayerInterface *	pPlayer = GetAudioPlayer() ;
				if ( (pPlayer == nullptr) || !pPlayer->IsPlaying() )
				{
					OnPlayStart() ;
				}
			}
			else if ( r > m_fpFadeReach + m_fpFadeLatitude + 1.0 )
			{
				SGLAudioPlayerInterface *	pPlayer = GetAudioPlayer() ;
				if ( (pPlayer != nullptr) && pPlayer->IsPlaying() )
				{
					pPlayer->Stop() ;
				}
			}
		}
		else
		{
			ReflectSoundPlayState() ;
		}
	}

	SoundItem::OnTimer( scene, msecPast ) ;
	ItemCommonSerializer::OnTimer( scene, msecPast ) ;

	m_csSync.Lock() ;
	if ( m_aInstance.GetLength() > 0 )
	{
		for ( size_t i = 0; i < m_aInstance.GetLength(); i ++ )
		{
			Instance *	pInstance = m_aInstance.GetAt( i ) ;
			if ( pInstance == nullptr )
			{
				continue ;
			}
			//
			// 自動削除判定
			//
			bool	flagDelete = false ;
			if ( pInstance->m_nFlags & instanceAutoDeleteOnEnd )
			{
				if ( (pInstance->m_pPlayer == nullptr)
					|| !(pInstance->m_pPlayer->IsPlaying()) )
				{
					flagDelete = true ;
				}
			}
			if ( pInstance->m_nFlags & instanceAutoDeleteWithItem )
			{
				if ( pInstance->m_refItem.GetReference() == nullptr )
				{
					flagDelete = true ;
				}
			}
			if ( flagDelete )
			{
				// 削除
				ReleaseAudioPlayer( pInstance->m_pPlayer.Detach() ) ;
				m_aInstance.SetAt( i, nullptr ) ;
				continue ;
			}
			if ( pInstance->m_pPlayer == nullptr )
			{
				continue ;
			}
			if ( (m_nSoundFlags & soundEnvironment)
				&& !(m_nSoundFlags & soundFadeReach) )
			{
				// 音量の減衰はないので音量の反映はしない
				continue ;
			}
			//
			// インスタンス行列計算
			//
			S3DDMatrix	matInstance ;
			S3DDVector	vInstance ;
			//
			S3DSceneComposer::ItemSerializer *
				pRefItem = pInstance->m_refItem.GetReference() ;
			if ( pRefItem != nullptr )
			{
				S3DDMatrix	matRefItem ;
				S3DDVector	vRefItem ;
				pRefItem->GetGlobalTransformation( matRefItem, vRefItem ) ;
				//
				matInstance = matRefItem * pInstance->m_matRotate ;
				vInstance = matRefItem * pInstance->m_vPos + vRefItem ;
			}
			else
			{
				matInstance = matItem * pInstance->m_matRotate ;
				vInstance = matItem * pInstance->m_vPos + vItem ;
			}
			//
			// 音量反映
			//
			ApplySoundVolume
				( matCamera, vCamera, matInstance, vInstance,
					pCamera, pInstance->m_fpVolume, pInstance->m_pPlayer ) ;
		}
		m_aInstance.TrimEmpty() ;
	}
	m_csSync.Unlock() ;
}

// フレーム更新後処理
//////////////////////////////////////////////////////////////////////////////
void S3DSoundItemSerializer::OnUpdateFrame
	( double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	ItemCommonSerializer::OnUpdateFrame( fpFrame, seek ) ;
	//
	if ( seek == S3DSceneComposer::seekJumpReset )
	{
		OnScenePlaying( false ) ;
	}
	else if ( seek == S3DSceneComposer::seekStream )
	{
		OnScenePlaying( true ) ;
	}
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DSoundItemSerializer::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp, uint32_t nFlags )
{
	uint32_t	nResFlags =
		ItemCommonSerializer::UpdatePropertyReference( comp, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefResource )
	{
		UpdateSoundRef() ;
	}
	return	nResFlags ;
}

// 拡張的な処理の通知
//////////////////////////////////////////////////////////////////////////////
void S3DSoundItemSerializer::OnExtendNotify
	( const wchar_t * pwszCmd, const wchar_t * pwszParam,
		const void * pExParam, size_t nExParamBytes )
{
	ItemCommonSerializer::OnExtendNotify
		( pwszCmd, pwszParam, pExParam, nExParamBytes ) ;
	//
	if ( SString::Compare( pwszCmd, S3DSceneComposer::CmdStartItem ) == 0 )
	{
		OnScenePlaying( true ) ;
	}
	else if ( SString::Compare( pwszCmd, S3DSceneComposer::CmdStopItem ) == 0 )
	{
		OnScenePlaying( false ) ;
	}
}

// シーン再生モード
//////////////////////////////////////////////////////////////////////////////
void S3DSoundItemSerializer::OnScenePlaying( bool flagScenePlaying )
{
	if ( flagScenePlaying )
	{
		ReflectSoundPlayState() ;
		m_flagScenePlaying = true ;
	}
	else
	{
		if ( m_flagPlaying )
		{
			SGLAudioPlayerInterface *	pPlayer = GetAudioPlayer() ;
			if ( pPlayer != nullptr )
			{
				pPlayer->Stop() ;
			}
			m_flagPlaying = false ;
		}
		m_flagScenePlaying = false ;
	}
}

// サウンド再生状態反映
//////////////////////////////////////////////////////////////////////////////
void S3DSoundItemSerializer::ReflectSoundPlayState( void )
{
	if ( m_flagPlaying != m_flagPlay )
	{
		if ( m_flagPlay )
		{
			if ( !(m_nSoundFlags & soundFadeReach) || !m_flagAutoPlay )
			{
				OnPlayStart() ;
			}
			m_flagPlaying = true ;
		}
		else
		{
			SGLAudioPlayerInterface *	pPlayer = GetAudioPlayer() ;
			if ( pPlayer != nullptr )
			{
				pPlayer->Stop() ;
			}
			m_flagPlaying = false ;
		}
	}
}

// Loquaty クラス名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSoundItemSerializer::GetLQClassName( void ) const
{
	return	L"EntisGLS4.SceneSoundItem" ;
}



//////////////////////////////////////////////////////////////////////////////
// レンズフレア
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DLensFlareSerializer::m_paramEntries[S3DLensFlareSerializer::paramLensFlareCount] =
{
	{ L"flare_count",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrDynamicValidation,	L"フレア数", nullptr },
	{ L"flare_fade_radius",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrDynamicValidation,
		L"フレア半径", L"フレアが陰に隠れる際のフェード処理用半径" },
} ;

const S3DSceneComposer::ParamSetClass
	S3DLensFlareSerializer::m_pscClass =
{
	&S3DSceneComposer::ItemBasicSerializer::m_pscClass,
	S3DLensFlareSerializer::paramLensFlareCount,
	&S3DLensFlareSerializer::m_paramEntries[0]
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DLensFlareSerializer, ItemBasicSerializer )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DLensFlareSerializer, lens_flare )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DLensFlareSerializer::S3DLensFlareSerializer( void )
	: ItemBasicSerializer
		( m_ItemClassDescriptor.pwszClassID, &m_pscExClass )
{
	m_classItem = S3DScene::classEffect2 ;
	//
	m_pscExClass.pParent = &S3DLensFlareSerializer::m_pscClass ;
	m_pscExClass.nCount = 0 ;
	m_pscExClass.pEntries = nullptr ;
	//
	m_fpFadeRadius = 0.0 ;
}

// プロパティ追加数
//////////////////////////////////////////////////////////////////////////////
size_t S3DLensFlareSerializer::GetPropExtensionCount( void ) const
{
	return	m_aExImages.GetLength() ;
}

void S3DLensFlareSerializer::SetPropExtensionCount( size_t nCount )
{
	if ( nCount != m_aExImages.GetLength() )
	{
		size_t	nLastCount = m_aExImages.GetLength() ;
		//
		m_aExImages.SetLength( nCount ) ;
		m_aExImageIDs.SetLength( nCount ) ;
		//
		for ( size_t i = nLastCount; i < nCount; i ++ )
		{
			ImageEntry *	pie = m_aExImages.GetAt( i ) ;
			if ( pie != nullptr )
			{
				pie->pImage = nullptr ;
				pie->fpPosition = 0.0f ;
				pie->fpZoom = 1.0f ;
				pie->vCenter.x = 0.0f ;
				pie->vCenter.y = 0.0f ;
			}
		}
		UpdateExtensionProperties() ;
	}
}

// プロパティ項目更新
//////////////////////////////////////////////////////////////////////////////
void S3DLensFlareSerializer::UpdateExtensionProperties( void )
{
	size_t	nLastEntryCount =
				m_aParamEntries.GetLength() / paramExEntryCount ;
	for ( size_t i = nLastEntryCount; i < m_aExImages.GetLength(); i ++ )
	{
		SString	strID ;
		SString	strName ;
		//
		strID.Format( L"flare_image%d", (i + 1) ) ;
		strName.Format( L"画像 [%d]", (i + 1) ) ;
		AddExPropertyEntry
			( strID, S3DSceneComposer::typeSelector,
				S3DSceneComposer::attrConstant1
				| S3DSceneComposer::attrStringEnumeration,
				strName, L"表示するフレア画像" ) ;
		//
		strID.Format( L"flare_position%d", (i + 1) ) ;
		strName.Format( L"位置 [%d]", (i + 1) ) ;
		AddExPropertyEntry
			( strID, S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrConstant1,
				strName, L"フレア画像の位置を 1.0 を基準とする比率で指定" ) ;
		//
		strID.Format( L"flare_zoom%d", (i + 1) ) ;
		strName.Format( L"拡大率 [%d]", (i + 1) ) ;
		AddExPropertyEntry
			( strID, S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrConstant1,
				strName, L"フレア画像の拡大率を指定" ) ;
		//
		strID.Format( L"flare_offset%d", (i + 1) ) ;
		strName.Format( L"表示オフセット [%d]", (i + 1) ) ;
		AddExPropertyEntry
			( strID, S3DSceneComposer::typeVector2,
				S3DSceneComposer::attrConstant1,
				strName, L"フレア画像の表示中心座標のオフセットを指定" ) ;
	}
	m_pscExClass.nCount = m_aParamEntries.GetLength() ;
	m_pscExClass.pEntries = m_aParamEntries.GetConstArray() ;
}

void S3DLensFlareSerializer::AddExPropertyEntry
	( const wchar_t * id,
		S3DSceneComposer::ParameterType type, uint32_t attr,
		const wchar_t * name, const wchar_t * desc,
		double minRange, double maxRange )
{
	SString *	pstrID = new SString( id ) ;
	SString *	pstrName = new SString( name ) ;
	SString *	pstrDesc = new SString( desc ) ;
	//
	m_aParamStrings.Add( pstrID ) ;
	m_aParamStrings.Add( pstrName ) ;
	m_aParamStrings.Add( pstrDesc ) ;
	//
	S3DSceneComposer::ParamEntry	pe ;
	pe.id = *pstrID ;
	pe.type = type ;
	pe.attr = attr ;
	pe.name = *pstrName ;
	pe.desc = *pstrDesc ;
	pe.minRange = minRange ;
	pe.maxRange = maxRange ;
	//
	m_aParamEntries.Add( pe ) ;
}

// 画像参照更新
//////////////////////////////////////////////////////////////////////////////
void S3DLensFlareSerializer::UpdateImageReference
	( S3DSceneComposer::Composition& comp, bool flagForceUpdate )
{
	S3DSceneComposer *	pSceneComp = comp.GetSceneComposer() ;
	if ( pSceneComp == nullptr )
	{
		return ;
	}
	for ( size_t i = 0; i < m_aExImages.GetLength(); i ++ )
	{
		SString *		pstrID = m_aExImageIDs.GetAt( i ) ;
		ImageEntry *	pie = m_aExImages.GetAt( i ) ;
		ESLAssert( pie != nullptr ) ;
		if ( pie == nullptr )
		{
			continue ;
		}
		if ( (pstrID != nullptr) && !pstrID->IsEmpty() )
		{
			if ( flagForceUpdate || (pie->pImage == nullptr) )
			{
				pie->pImage = pSceneComp->Assets().GetImageAs( *pstrID ) ;
			}
		}
		else
		{
			pie->pImage = nullptr ;
		}
	}
}

// 表示画像設定
//////////////////////////////////////////////////////////////////////////////
void S3DLensFlareSerializer::SetImageEntries( const ImageEntry * pImages, size_t nCount )
{
	m_aImages.RemoveAll() ;
	m_aImages.AddArray( pImages, nCount ) ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
double S3DLensFlareSerializer::GetScalarParameter( size_t i ) const
{
	if ( i < paramLensFrareCount )
	{
		return	ItemBasicSerializer::GetScalarParameter( i ) ;
	}
	else if ( i == paramLensFrareCount )
	{
		return	0.0 ;
	}
	else if ( i == paramFadeRadius )
	{
		return	m_fpFadeRadius ;
	}
	const size_t	iExEntry = (i - paramLensFlareTotlaCount) / paramExEntryCount ;
	ImageEntry *	pie = m_aExImages.GetAt( iExEntry ) ;
	switch ( (i - paramLensFlareTotlaCount) % paramExEntryCount )
	{
	case	paramFlarePosition:
		if ( pie != nullptr )
		{
			return	pie->fpPosition ;
		}
		break ;
	case	paramFlareZoom:
		if ( pie != nullptr )
		{
			return	pie->fpZoom ;
		}
		break ;
	}
	return	0.0 ;
}

int32_t S3DLensFlareSerializer::GetIntegerParameter( size_t i ) const
{
	if ( i < paramLensFrareCount )
	{
		return	ItemBasicSerializer::GetIntegerParameter( i ) ;
	}
	else if ( i == paramLensFrareCount )
	{
		return	(int32_t) GetPropExtensionCount() ;
	}
	return	0 ;
}

const wchar_t * S3DLensFlareSerializer::GetCommandParameter( size_t i ) const
{
	if ( i < paramLensFrareCount )
	{
		return	ItemBasicSerializer::GetCommandParameter( i ) ;
	}
	else if ( i == paramLensFrareCount )
	{
		return	nullptr ;
	}
	if ( ((i - paramLensFlareTotlaCount)
				% paramExEntryCount) == paramFlareImage )
	{
		size_t	iExEntry = (i - paramLensFlareTotlaCount) / paramExEntryCount ;
		SString *	pstrImage = m_aExImageIDs.GetAt( iExEntry ) ;
		if ( pstrImage != nullptr )
		{
			return	*pstrImage ;
		}
		return	nullptr ;
	}
	return	nullptr ;
}

size_t S3DLensFlareSerializer::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t i ) const
{
	if ( i < paramLensFrareCount )
	{
		return	ItemBasicSerializer::GetBinaryParameter( pDst, nBufBytes, i ) ;
	}
	else if ( i == paramLensFrareCount )
	{
		return	0 ;
	}
	const size_t	iExEntry = (i - paramLensFlareTotlaCount) / paramExEntryCount ;
	ImageEntry *	pie = m_aExImages.GetAt( iExEntry ) ;
	switch ( (i - paramLensFlareTotlaCount) % paramExEntryCount )
	{
	case	paramFlareCenterOffset:
		if ( (pie != nullptr) && (nBufBytes == sizeof(S2DDVector)) )
		{
			*((S2DDVector*) pDst) = pie->vCenter ;
			return	sizeof(S2DDVector) ;
		}
		break ;
	}
	return	0 ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DLensFlareSerializer::SetScalarParameter( size_t i, double s )
{
	if ( i < paramLensFrareCount )
	{
		ItemBasicSerializer::SetScalarParameter( i, s ) ;
		return ;
	}
	else if ( i == paramLensFrareCount )
	{
		return ;
	}
	else if ( i == paramFadeRadius )
	{
		m_fpFadeRadius = s ;
		return ;
	}
	const size_t	iExEntry = (i - paramLensFlareTotlaCount) / paramExEntryCount ;
	ImageEntry *	pie = m_aExImages.GetAt( iExEntry ) ;
	switch ( (i - paramLensFlareTotlaCount) % paramExEntryCount )
	{
	case	paramFlarePosition:
		if ( pie != nullptr )
		{
			pie->fpPosition = (float32_t) s ;
		}
		break ;
	case	paramFlareZoom:
		if ( pie != nullptr )
		{
			pie->fpZoom = (float32_t) s ;
		}
		break ;
	}
}

void S3DLensFlareSerializer::SetIntegerParameter( size_t i, int32_t n )
{
	if ( i < paramLensFrareCount )
	{
		ItemBasicSerializer::GetIntegerParameter( i ) ;
		return ;
	}
	else if ( i == paramLensFrareCount )
	{
		SetPropExtensionCount( n ) ;
		return ;
	}
}

void S3DLensFlareSerializer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	if ( i < paramLensFrareCount )
	{
		ItemBasicSerializer::SetCommandParameter( i, pwszCmd ) ;
		return ;
	}
	else if ( i == paramLensFrareCount )
	{
		return ;
	}
	if ( ((i - paramLensFlareTotlaCount)
				% paramExEntryCount) == paramFlareImage )
	{
		size_t	iExEntry = (i - paramLensFlareTotlaCount) / paramExEntryCount ;
		m_aExImageIDs.SetAt( iExEntry, new SString( pwszCmd ) ) ;
		//
		ImageEntry *	pie = m_aExImages.GetAt( iExEntry ) ;
		if ( pie != nullptr )
		{
			pie->pImage = nullptr ;
		}
		S3DSceneComposer::Composition *	pComp = GetComposition() ;
		if ( pComp != nullptr )
		{
			UpdateImageReference( *pComp, false ) ;
		}
		return ;
	}
}

size_t S3DLensFlareSerializer::SetBinaryParameter
	( size_t i, const void * pSrc, size_t nBufBytes )
{
	if ( i < paramLensFrareCount )
	{
		return	ItemBasicSerializer::SetBinaryParameter( i, pSrc, nBufBytes ) ;
	}
	else if ( i == paramLensFrareCount )
	{
		return	0 ;
	}
	const size_t	iExEntry = (i - paramLensFlareTotlaCount) / paramExEntryCount ;
	ImageEntry *	pie = m_aExImages.GetAt( iExEntry ) ;
	switch ( (i - paramLensFlareTotlaCount) % paramExEntryCount )
	{
	case	paramFlareCenterOffset:
		if ( (pie != nullptr) && (nBufBytes == sizeof(S2DDVector)) )
		{
			pie->vCenter = *((S2DDVector*) pSrc) ;
			return	sizeof(S2DDVector) ;
		}
		break ;
	}
	return	0 ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DLensFlareSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	if ( i < paramLensFrareCount )
	{
		return	ItemBasicSerializer::EnumerateStringSet( i, aStrSet ) ;
	}
	else if ( i == paramLensFrareCount )
	{
		return	false ;
	}
	const size_t	iExEntry = (i - paramLensFlareTotlaCount) / paramExEntryCount ;
	ImageEntry *	pie = m_aExImages.GetAt( iExEntry ) ;
	if ( ((i - paramLensFlareTotlaCount)
			% paramExEntryCount) == paramFlareImage )
	{
		S3DSceneComposer *	pComp = GetComposer() ;
		if ( pComp != nullptr )
		{
			pComp->Assets().EnumerateResourceIDsAs
				( aStrSet, ESL_RUNTIME_CLASS(SGLImageObject) ) ;
		}
		return	true ;
	}
	return	false ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DLensFlareSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"レンズフレア" ;
	}
	return	ItemBasicSerializer::GetParameterCategoryName( iCategory ) ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DLensFlareSerializer::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp, uint32_t nFlags )
{
	uint32_t	nResFlags =
		ItemBasicSerializer::UpdatePropertyReference( comp, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefResource )
	{
		UpdateImageReference( comp, true ) ;
	}
	return	nResFlags ;
}

// 表示モデル追加
//////////////////////////////////////////////////////////////////////////////
void S3DLensFlareSerializer::RenderModel
	( const S3DScene& scene,
		S3DRenderContextInterface& render, uint64_t flagsExclusion )
{
	ItemBasicSerializer::RenderModel( scene, render, flagsExclusion ) ;
	//
	S3DColor	clrEffect ;
	GetGlobalColorEffect( clrEffect ) ;
	if ( clrEffect.rgbMul.argb.Alpha <= 1 )
	{
		return ;
	}
	S3DScene::Camera *	pCamera = scene.GetCurrentCamera() ;
	if ( pCamera == nullptr )
	{
		return ;
	}
	S3DDMatrix	matTemp ;
	S3DDVector	vCamera ;
	S3DDVector	vTarget ;
	pCamera->CalcGlobalTransformation( matTemp, vCamera ) ;
	CalcGlobalTransformation( matTemp, vTarget ) ;
	if ( m_flagsBehavior & (S3DScene::itemCameraShift | S3DScene::itemCameraSpace) )
	{
		vTarget += vCamera ;
	}
	//
	float32_t	fpError = (float32_t) (vTarget - vCamera).Absolute() * 1.0e-8f ;
	S3DDVector	vVTarget = vTarget ;
	scene.TransformByCurrentCamera( vVTarget ) ;
	if ( vVTarget.z <= fpError )
	{
		return ;
	}
	static const int	nDivCounts[4] =
	{
		1, 6, 8, 12
	} ;
	static const double	fpWeight[4] =
	{
		0.25, 0.25, 0.25, 0.25
	} ;
	double		fpHidden = 0.0 ;
	S3DDMatrix	matICamera = scene.GetCurrentCameraIMatrix() ;
	//
	for ( int i = 0; i < 4; i ++ )
	{
		for ( int j = 0; j < nDivCounts[i]; j ++ )
		{
			S3DDVector	vTestTarget = vTarget ;
			double		rad = PI * 2.0 * j / nDivCounts[i] ;
			double		r = m_fpFadeRadius * i * (1.0 / 3.0) ;
			//
			vTestTarget += matICamera
							* S3DDVector( r * cos(rad), r * sin(rad), 0.0 ) ;
			//
			S3DCollision::Result	rsHit ;
			rsHit.SetInclusionSceneFlags( 0 ) ;
			rsHit.SetInclusionUserFlags( S3DCollision::colliderShape ) ;
			rsHit.fpDistance = (float32_t) (vTestTarget - vCamera).Absolute() ;
			//
			S3DScene::Item *
				pHitItem = scene.IsItemSegmentCrossing
							( vCamera, vTestTarget, fpError, rsHit ) ;
			if ( pHitItem != nullptr )
			{
				fpHidden += fpWeight[i] / nDivCounts[i] ;
				if ( (m_fpFadeRadius == 0.0) && (i == 0) )
				{
					return ;
				}
			}
		}
	}
	S2DDVector	vTarget2D ;
	scene.ViewProjectionOf( vTarget2D, vVTarget ) ;
	//
	S3DScene::ProjectionParam	pp ;
	scene.GetCurrentProjection( pp ) ;
	vTarget2D.x -= pp.vScreen.x ;
	vTarget2D.y -= pp.vScreen.y ;
	//
	double		vl = sqrt( pp.vScreen.x * pp.vScreen.x
							+ pp.vScreen.y * pp.vScreen.y ) ;
	double		tl = vTarget2D.Absolute() ;
	uint32_t	nTransparency = 0 ;
	if ( tl > vl )
	{
		double	t = esl_fmin( 1.0, tl / vl - 1.0 ) ;
		nTransparency = (uint32_t) eslRoundR64ToLInt( t * 0x100 ) ;
	}
	if ( fpHidden > 0.0 )
	{
		fpHidden *= fpHidden ;
		fpHidden *= fpHidden ;
		nTransparency =
			(uint32_t) eslRoundR64ToLInt
				( 0x100 - (0x100 - nTransparency) * (1.0 - fpHidden) ) ;
	}
	if ( nTransparency >= 0x100 )
	{
		return ;
	}
	nTransparency = 0x100 - (0x100 - nTransparency)
							* (clrEffect.rgbMul.argb.Alpha + 1) / 0x100 ;
	//
	for ( size_t i = 0; i < m_aImages.GetLength(); i ++ )
	{
		ImageEntry *	pie = m_aImages.GetAt( i ) ;
		ESLAssert( pie != nullptr ) ;
		if ( pie && pie->pImage )
		{
			float32_t	p = 1.0f - pie->fpPosition ;
			float32_t	x = pp.vScreen.x
							+ (float32_t) vTarget2D.x * p ;
			float32_t	y = pp.vScreen.y
							+ (float32_t) vTarget2D.y * p ;
			float32_t	z = pp.fpZoom * pie->fpZoom ;
			x -= pie->vCenter.x * z ;
			y -= pie->vCenter.y * z ;
			//
			SGLPaintParam	pprm ;
			SGLAffine	affine( z, 0.0f, x,  0.0f, z, y ) ;
			pprm.nFlags = paintFunctionAdd | paintSmoothStretch | paintDelayable ;
			pprm.pAffine = &affine ;
			pprm.nTransparency = nTransparency ;
			//
			render.DrawImage( pprm, pie->pImage ) ;
		}
	}
	for ( size_t i = 0; i < m_aExImages.GetLength(); i ++ )
	{
		ImageEntry *	pie = m_aExImages.GetAt( i ) ;
		ESLAssert( pie != nullptr ) ;
		if ( pie && pie->pImage )
		{
			float32_t	p = 1.0f - pie->fpPosition ;
			float32_t	x = pp.vScreen.x
							+ (float32_t) vTarget2D.x * p ;
			float32_t	y = pp.vScreen.y
							+ (float32_t) vTarget2D.y * p ;
			float32_t	z = pp.fpZoom * pie->fpZoom ;
			SGLSize		sizeImage = pie->pImage->GetImageSize() ;
			x -= (pie->vCenter.x + (float32_t) sizeImage.w * 0.5f) * z ;
			y -= (pie->vCenter.y + (float32_t) sizeImage.h * 0.5f) * z ;
			//
			SGLPaintParam	pprm ;
			SGLAffine	affine( z, 0.0f, x,  0.0f, z, y ) ;
			pprm.nFlags = paintFunctionAdd | paintSmoothStretch | paintDelayable ;
			pprm.pAffine = &affine ;
			pprm.nTransparency = nTransparency ;
			//
			render.DrawImage( pprm, pie->pImage ) ;
		}
	}
}



//////////////////////////////////////////////////////////////////////////////
// Rosetta インスタンス・コンテナ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DSceneScriptInstance, Controller, S3DSceneCustomProperty )
S3D_IMPLEMENT_COMPOSER_ITEM
	( SakuraGL::S3DSceneScriptInstance, script_obj_instance )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneScriptInstance::S3DSceneScriptInstance( void )
	: Controller( m_ItemClassDescriptor.pwszClassID )
{
	m_pRSClass = nullptr ;
	m_pInstance = nullptr ;
	//
	m_iParamRSClass = 
		AddParameterEntry
			( L"prototype", S3DSceneComposer::typeSelector,
				S3DSceneComposer::attrConstant
				| S3DSceneComposer::attrStringEnumeration
				| S3DSceneComposer::attrDynamicValidation,
				L"クラス", L"インスタンスのクラスを指定します" ) ;
	m_iParamBase = GetParameterCount() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneScriptInstance::~S3DSceneScriptInstance( void )
{
	if ( m_pInstance != nullptr )
	{
		m_pInstance->ReleaseRef() ;
	}
	if ( m_pRSClass != nullptr )
	{
		m_pRSClass->ReleaseRef() ;
	}
}

// インスタンス取得
//////////////////////////////////////////////////////////////////////////////
Rosetta::RSObject *
	S3DSceneScriptInstance::GetRSInstance( void ) const
{
	if ( m_pInstance != nullptr )
	{
		m_pInstance->AddRef() ;
	}
	return	m_pInstance ;
}

const Loquaty::LObjPtr&
	S3DSceneScriptInstance::GetLoquatyInstance( void ) const
{
	return	m_pLObject ;
}

// クラス参照を更新する
//////////////////////////////////////////////////////////////////////////////
void S3DSceneScriptInstance::UpdateClassRef( void )
{
	if ( m_pRSClass != nullptr )
	{
		m_pRSClass->ReleaseRef() ;
		m_pRSClass = nullptr ;
	}
	m_pLClass.Release() ;

	S3DCompositionManager *	pManager = GetManager() ;
	if ( pManager != nullptr )
	{
		LClass *	pLClass = pManager->LoquatyVM()->GetClassPathAs( m_strRSClass ) ;
		if ( pLClass != nullptr )
		{
			pLClass->AddRef() ;
			m_pLClass.SetPtr( pLClass ) ;
		}
		else
		{
			m_pRSClass = pManager->VM().GetClassAs( m_strRSClass ) ;
			if ( m_pRSClass != nullptr )
			{
				m_pRSClass->AddRef() ;
			}
		}
	}
	UpdateClassMember() ;
}

// クラスメンバのパラメータに更新
//////////////////////////////////////////////////////////////////////////////
void S3DSceneScriptInstance::UpdateClassMember( void )
{
	if ( m_pLClass != nullptr )
	{
		UpdateLoquatyClassMember() ;
	}
	else if ( m_pRSClass != nullptr )
	{
		UpdateRosettaClassMember() ;
	}
}

void S3DSceneScriptInstance::UpdateRosettaClassMember( void )
{
	ChopParameterEntryLastAt( m_iParamRSClass ) ;
	m_aOptions.RemoveAll() ;
	//
	if ( m_pInstance != nullptr )
	{
		m_pInstance->ReleaseRef() ;
		m_pInstance = nullptr ;
	}
	m_pLObject.Release() ;
	//
	if ( m_pRSClass == nullptr )
	{
		return ;
	}
	RSObject *	pProto = m_pRSClass->m_pPrototype ;
	if ( pProto == nullptr )
	{
		return ;
	}
	S3DCompositionManager *	pManager = GetManager() ;
	if ( pManager == nullptr )
	{
		return ;
	}
	//
	// パラメータリスト
	//
	if ( (m_pContext == nullptr)
		|| (m_pContext->GetVM() != &(pManager->GetVM())) )
	{
		m_pContext = new Rosetta::RSContext( &(pManager->VM()) ) ;
	}
	//
	const size_t *	pIndex = nullptr ;
	size_t			nCount = 0 ;
	RSStructuredPointerClass *
		pStruct = ESLTypeCast<RSStructuredPointerClass>( m_pRSClass ) ;
	if ( pStruct != nullptr )
	{
		nCount = pStruct->GetArrayMemberCount() ;
		pIndex = pStruct->GetOrderedMemberIndex() ;
	}
	else
	{
		nCount = pProto->GetElementCount() ;
	}
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RSStructuredPointerClass::ElementInfo *	peiElement = nullptr ;
		RSSmartPtr		pElement( nullptr, m_pContext ) ;
		const wchar_t *	pwszName = nullptr ;
		RSClass *		pElementType = nullptr ;
		RSCodeComment *	pComment = nullptr ;
		if ( pStruct != nullptr )
		{
			pwszName = pStruct->GetArrayMemberNameAt( pIndex[i] ) ;
			peiElement = pStruct->GetArrayMemberAt( pIndex[i] ) ;
			if ( peiElement == nullptr )
			{
				continue ;
			}
			pElementType = peiElement->m_pClass ;
			pComment = peiElement->m_pComment ;
		}
		else
		{
			pwszName = pProto->GetElementNameAt( (int) i ) ;
			pElement = pProto->GetElementAt( *m_pContext, (int) i ) ;
			if ( pElement == nullptr )
			{
				continue ;
			}
			RSObject *	pObj = pElement.GetEntity() ;
			if ( pObj == nullptr )
			{
				continue ;
			}
			pElementType = pObj->GetRSClass() ;
			if ( pElement->GetDefinitionComment() != nullptr )
			{
				pComment = pElement->GetDefinitionComment() ;
			}
			else if ( pObj->GetDefinitionComment() != nullptr )
			{
				pComment = pObj->GetDefinitionComment() ;
			}
		}
		S3DSceneComposer::ParameterType
						type = S3DSceneComposer::typeInvalid ;
		const wchar_t *	pwszDispName = pwszName ;
		const wchar_t *	pwszComment = nullptr ;
		uint32_t		nAttrFlags = S3DSceneComposer::attrCategory1 ;
		double			minRange = 0.0, maxRange = 1.0 ;
		//
		RSStructuredPointerClass *	pType =
			ESLTypeCast<RSStructuredPointerClass>( pElementType ) ;
		if ( pType != nullptr )
		{
			const wchar_t *	pwszClassName = pType->GetRSClassName() ;
			if ( SString::Compare( pwszClassName, L"Vector3D4" ) == 0 )
			{
				type = S3DSceneComposer::typePosition ;
			}
			else if ( SString::Compare( pwszClassName, L"Vector3D" ) == 0 )
			{
				type = S3DSceneComposer::typeDirection ;
			}
			else if ( SString::Compare( pwszClassName, L"Quaternion" ) == 0 )
			{
				type = S3DSceneComposer::typeRotation ;
			}
			else if ( SString::Compare( pwszClassName, L"Matrix3D" ) == 0 )
			{
				type = S3DSceneComposer::typeMatrix ;
			}
			else if ( SString::Compare( pwszClassName, L"RGBColor" ) == 0 )
			{
				type = S3DSceneComposer::typeColor ;
			}
			else if ( SString::Compare( pwszClassName, L"Vector2D" ) == 0 )
			{
				type = S3DSceneComposer::typeVector2 ;
			}
			else if ( SString::Compare( pwszClassName, L"Matrix4D" ) == 0 )
			{
				type = S3DSceneComposer::typeMatrix4 ;
			}
		}
		else
		{
			if ( m_pContext->GetBasicTypeClass
							( RSCodeControl::wiBoolean ) == pElementType )
			{
				type = S3DSceneComposer::typeBoolean ;
			}
			else if ( pElementType->IsKindOf
							( ESL_RUNTIME_CLASS(RSNumberClass) ) )
			{
				type = S3DSceneComposer::typeScalar ;
			}
			else if ( pElementType->IsKindOf
							( ESL_RUNTIME_CLASS(RSIntegerClass) ) )
			{
				type = S3DSceneComposer::typeInteger ;
				//
				const SXMLDocument *	pxmlSelector =
					pComment->GetXMLDocument().GetElementTagAs( L"selector" ) ;
				if ( pxmlSelector != nullptr )
				{
					type = S3DSceneComposer::typeSelector ;
					nAttrFlags |= S3DSceneComposer::attrStringEnumeration
								| S3DSceneComposer::attrUIOnlyEnumeration ;
					//
					ParamOption *	pOption = new ParamOption ;
					m_aOptions.SetAt( GetParameterCount(), pOption ) ;
					//
					for ( size_t iOpt = 0;
						iOpt < pxmlSelector->GetElementsCount(); iOpt ++ )
					{
						const SXMLDocument *
							pxmlEntry = pxmlSelector->GetElementAt( iOpt ) ;
						if ( (pxmlEntry == nullptr)
							|| (pxmlEntry->GetTag() != L"enum") )
						{
							continue ;
						}
						const SString *	pstrName = pxmlEntry->GetAttributeAs( L"name" ) ;
						if ( pstrName != nullptr )
						{
							SString *	pstrNameBuf = new SString( *pstrName ) ;
							SXMLDocument::AttrInteger	ai ;
							ai.nValue = pxmlEntry->GetAttrIntegerAs( L"num" ) ;
							ai.pszSymbol = *pstrNameBuf ;
							pOption->m_aStrIntPairs.Add( ai ) ;
							pOption->m_aStrBuffers.Add( pstrNameBuf ) ;
						}
					}
					SXMLDocument::AttrInteger	aiNull ;
					aiNull.nValue = 0 ;
					aiNull.pszSymbol = nullptr ;
					pOption->m_aStrIntPairs.Add( aiNull ) ;
				}
			}
			else if ( pElementType->IsKindOf
							( ESL_RUNTIME_CLASS(RSStringClass) ) )
			{
				type = S3DSceneComposer::typeSelector ;
			}
		}
		if ( type != S3DSceneComposer::typeInvalid )
		{
			if ( pComment != nullptr )
			{
				const SXMLDocument&	xmlDoc = pComment->GetXMLDocument() ;
				const SString *	pstrName = xmlDoc.GetTextElementAs( L"name" ) ;
				if ( pstrName != nullptr )
				{
					pwszDispName = *pstrName ;
				}
				const SString *	pstrDesc = xmlDoc.GetTextElementAs( L"desc" ) ;
				if ( pstrDesc != nullptr )
				{
					pwszComment = *pstrDesc ;
				}
				const SXMLDocument *	pxmlRange = xmlDoc.GetElementTagAs( L"range" ) ;
				if ( pxmlRange != nullptr )
				{
					nAttrFlags |= S3DSceneComposer::attrUIScalarSlider ;
					minRange = pxmlRange->GetAttrRealAs( L"min", 0.0 ) ;
					maxRange = pxmlRange->GetAttrRealAs( L"max", 0.0 ) ;
				}
			}
			AddParameterEntry
				( pwszName, type,
					nAttrFlags, pwszDispName,
					pwszComment, minRange, maxRange ) ;
		}
	}
	//
	// インスタンス生成
	//
	RSObject *	pArg = m_pContext->new_Array() ;
	m_pInstance = m_pRSClass->NewInstance( *m_pContext, pArg ) ;
	m_pContext->ReleaseObjectRef( pArg ) ;
}

void S3DSceneScriptInstance::UpdateLoquatyClassMember( void )
{
	ChopParameterEntryLastAt( m_iParamRSClass ) ;
	m_aOptions.RemoveAll() ;
	//
	if ( m_pInstance != nullptr )
	{
		m_pInstance->ReleaseRef() ;
		m_pInstance = nullptr ;
	}
	m_pLObject.Release() ;
	//
	if ( m_pLClass == nullptr )
	{
		return ;
	}
	m_pLObject.SetPtr( m_pLClass->CreateInstance() ) ;
	if ( m_pLObject != nullptr )
	{
		S3DSceneCustomProperty::AttachObject( m_pLClass->VM(), m_pLObject ) ;

		for ( size_t i = 0; i < m_params.GetLength(); i ++ )
		{
			AddParameterEntry( m_params.At(i) ) ;
		}
	}
}

// パラメータ保存
//////////////////////////////////////////////////////////////////////////////
void S3DSceneScriptInstance::SaveInstanceParameters
	( SSystem::SStrSortObjectArray
			<S3DSceneComposer::ParameterEntryStorage>& ssoaParam )
{
	const size_t	nParamCount = m_arrParamClass.GetLength() ;
	const S3DSceneComposer::ParamEntry *
					pParamEntries = m_arrParamClass.GetConstArray() ;
	for ( size_t iParam = m_iParamBase; iParam < nParamCount; iParam ++ )
	{
		const S3DSceneComposer::ParamEntry&	pe = pParamEntries[iParam] ;
		//
		S3DSceneComposer::ParameterEntryStorage *
					ppes = new S3DSceneComposer::ParameterEntryStorage ;
		SaveParameterEntryAt( *ppes, iParam ) ;
		//
		ssoaParam.SetAs( pe.id, ppes ) ;
	}
}

// パラメータ復帰
//////////////////////////////////////////////////////////////////////////////
void S3DSceneScriptInstance::ResotreInstanceParameters
	( const SSystem::SStrSortObjectArray
			<S3DSceneComposer::ParameterEntryStorage>& ssoaParam )
{
	for ( size_t i = 0; i < ssoaParam.GetLength(); i ++ )
	{
		const SString *	pstrTag = ssoaParam.GetTagAt( i ) ;
		ESLAssert( pstrTag != nullptr ) ;
		//
		ssize_t	iParam = FindParameterID( *pstrTag ) ;
		if ( iParam < 0 )
		{
			continue ;
		}
		S3DSceneComposer::ParameterEntryStorage *
					ppes = ssoaParam.GetAt( i ) ;
		if ( ppes != nullptr )
		{
			RestoreParameterEntryAt( iParam, *ppes ) ;
		}
	}
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DSceneScriptInstance::GetMatrixParameter( size_t i ) const
{
	if ( (i > m_iParamRSClass) && (m_pLObject != nullptr) )
	{
		return	S3DSceneCustomProperty::GetMatrixParameter( i - m_iParamBase ) ;
	}
	else if ( (i > m_iParamRSClass)
		&& (m_pInstance != nullptr) && (m_pContext != nullptr) )
	{
		RSSmartPtr	pElement
			( m_pInstance->GetMemberAs
				( *m_pContext, GetParameterID( i ) ), m_pContext ) ;
		RSStructuredPointer *	pStruct =
			ESLTypeCast<RSStructuredPointer>( pElement.GetEntity().Ptr() ) ;
		if ( (pStruct != nullptr) && (pStruct->GetRSClass() != nullptr) )
		{
			S3DDMatrix	matd ;
			const wchar_t *	pwszClassName =
								pStruct->GetRSClass()->GetRSClassName() ;
			if ( SString::Compare( pwszClassName, L"Matrix3D" ) == 0 )
			{
				SGL3DMatrix<float32_t,3> *	pmatBuf =
					(SGL3DMatrix<float32_t,3>*)
						pStruct->GetPointer( sizeof(SGL3DMatrix<float32_t,3>) ) ;
				if ( pmatBuf != nullptr )
				{
					for ( int i = 0; i < 3; i ++ )
					{
						matd.m[i][0] = pmatBuf->m[i][0] ;
						matd.m[i][1] = pmatBuf->m[i][1] ;
						matd.m[i][2] = pmatBuf->m[i][2] ;
					}
				}
				return	matd ;
			}
			else if ( SString::Compare( pwszClassName, L"Quaternion" ) == 0 )
			{
				S3DQuaternion *	pqBuf =
					(S3DQuaternion*) pStruct->GetPointer( sizeof(S3DQuaternion) ) ;
				if ( pqBuf != nullptr )
				{
					S3DMatrix	matf ;
					pqBuf->ToMatrix( matf ) ;
					matd = matf ;
				}
				return	matd ;
			}
		}
	}
	return	Controller::GetMatrixParameter( i ) ;
}

S3DDVector S3DSceneScriptInstance::GetVectorParameter( size_t i ) const
{
	if ( (i > m_iParamRSClass) && (m_pLObject != nullptr) )
	{
		return	S3DSceneCustomProperty::GetVectorParameter( i - m_iParamBase ) ;
	}
	else if ( (i > m_iParamRSClass)
		&& (m_pInstance != nullptr) && (m_pContext != nullptr) )
	{
		RSSmartPtr	pElement
			( m_pInstance->GetMemberAs
				( *m_pContext, GetParameterID( i ) ), m_pContext ) ;
		RSStructuredPointer *	pStruct =
			ESLTypeCast<RSStructuredPointer>( pElement.GetEntity().Ptr() ) ;
		if ( (pStruct != nullptr) && (pStruct->GetRSClass() != nullptr) )
		{
			S3DDVector	vecd ;
			const wchar_t *	pwszClassName =
								pStruct->GetRSClass()->GetRSClassName() ;
			if ( (SString::Compare( pwszClassName, L"Vector3D" ) == 0)
				|| (SString::Compare( pwszClassName, L"Vector3D4" ) == 0) )
			{
				S3DVector *	pvBuf =
					(S3DVector*) pStruct->GetPointer( sizeof(S3DVector) ) ;
				if ( pvBuf != nullptr )
				{
					vecd = *pvBuf ;
					return	vecd ;
				}
			}
			else if ( SString::Compare( pwszClassName, L"RGBColor" ) == 0 )
			{
				SGLPalette *	ppBuf =
					(SGLPalette*) pStruct->GetPointer( sizeof(SGLPalette) ) ;
				if ( ppBuf != nullptr )
				{
					vecd = VectorFromColor( *ppBuf ) ;
					return	vecd ;
				}
			}
		}
	}
	return	Controller::GetVectorParameter( i ) ;
}

double S3DSceneScriptInstance::GetScalarParameter( size_t i ) const
{
	if ( (i > m_iParamRSClass) && (m_pLObject != nullptr) )
	{
		return	S3DSceneCustomProperty::GetScalarParameter( i - m_iParamBase ) ;
	}
	else if ( (i > m_iParamRSClass)
		&& (m_pInstance != nullptr) && (m_pContext != nullptr) )
	{
		return	m_pInstance->GetMemberNumberAs
					( *m_pContext, GetParameterID( i ) ) ;
	}
	return	Controller::GetScalarParameter( i ) ;
}

int32_t S3DSceneScriptInstance::GetIntegerParameter( size_t i ) const
{
	if ( (i > m_iParamRSClass) && (m_pLObject != nullptr) )
	{
		return	(int32_t) S3DSceneCustomProperty::GetIntegerParameter( i - m_iParamBase ) ;
	}
	else if ( (i > m_iParamRSClass)
		&& (m_pInstance != nullptr) && (m_pContext != nullptr) )
	{
		return	(int32_t) m_pInstance->GetMemberIntegerAs
								( *m_pContext, GetParameterID( i ) ) ;
	}
	return	Controller::GetIntegerParameter( i ) ;
}

bool S3DSceneScriptInstance::GetBooleanParameter( size_t i ) const
{
	if ( (i > m_iParamRSClass) && (m_pLObject != nullptr) )
	{
		return	S3DSceneCustomProperty::GetBooleanParameter( i - m_iParamBase ) ;
	}
	else if ( (i > m_iParamRSClass)
		&& (m_pInstance != nullptr) && (m_pContext != nullptr) )
	{
		return	m_pInstance->GetMemberIntegerAs
					( *m_pContext, GetParameterID( i ) ) != 0 ;
	}
	return	Controller::GetBooleanParameter( i ) ;
}

const wchar_t * S3DSceneScriptInstance::GetCommandParameter( size_t i ) const
{
	if ( i == m_iParamRSClass )
	{
		return	m_strRSClass ;
	}
	else if ( (i > m_iParamRSClass) && (m_pLObject != nullptr) )
	{
		return	S3DSceneCustomProperty::GetCommandParameter( i - m_iParamBase ) ;
	}
	else if ( (m_pInstance != nullptr) && (m_pContext != nullptr) )
	{
		ParamOption *	pOption = m_aOptions.GetAt( i ) ;
		if ( pOption != nullptr )
		{
			int64_t	num = m_pInstance->GetMemberIntegerAs
								( *m_pContext, GetParameterID( i ) ) ;
			return	SXMLDocument::GetSymbolAsIntegerOf
						( pOption->m_aStrIntPairs.GetConstArray(), num ) ;
		}
	}
	return	Controller::GetCommandParameter( i ) ;
}

size_t S3DSceneScriptInstance::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t i ) const
{
	if ( (i > m_iParamRSClass) && (m_pLObject != nullptr) )
	{
		return	S3DSceneCustomProperty::GetBinaryParameter
							( pDst, nBufBytes, i - m_iParamBase ) ;
	}
	else if ( (i > m_iParamRSClass)
		&& (m_pInstance != nullptr) && (m_pContext != nullptr) )
	{
		RSSmartPtr	pElement
			( m_pInstance->GetMemberAs
				( *m_pContext, GetParameterID( i ) ), m_pContext ) ;
		RSStructuredPointer *	pStruct =
			ESLTypeCast<RSStructuredPointer>( pElement.GetEntity().Ptr() ) ;
		if ( (pStruct != nullptr) && (pStruct->GetRSClass() != nullptr) )
		{
			const wchar_t *	pwszClassName =
								pStruct->GetRSClass()->GetRSClassName() ;
			if ( SString::Compare( pwszClassName, L"Vector2D" ) == 0 )
			{
				S2DVector *	pvBuf =
					(S2DVector*) pStruct->GetPointer( sizeof(S2DVector) ) ;
				if ( (pvBuf != nullptr) && (nBufBytes == sizeof(S2DDVector)) )
				{
					*((S2DDVector*)pDst) = *pvBuf ;
					return	sizeof(S2DDVector) ;
				}
			}
			else if ( SString::Compare( pwszClassName, L"Matrix4D" ) == 0 )
			{
				S4DMatrix *	pmatBuf =
					(S4DMatrix*) pStruct->GetPointer( sizeof(S4DMatrix) ) ;
				if ( (pmatBuf != nullptr) && (nBufBytes == sizeof(S4DDMatrix)) )
				{
					*((S4DDMatrix*)pDst) = *pmatBuf ;
					return	sizeof(S4DDMatrix) ;
				}
			}
		}
	}
	return	Controller::GetBinaryParameter( pDst, nBufBytes, i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneScriptInstance::SetMatrixParameter
	( size_t i, const S3DDMatrix& mat )
{
	if ( (i > m_iParamRSClass) && (m_pLObject != nullptr) )
	{
		S3DSceneCustomProperty::SetMatrixParameter( i - m_iParamBase, mat ) ;
		return ;
	}
	else if ( (i > m_iParamRSClass)
		&& (m_pInstance != nullptr) && (m_pContext != nullptr) )
	{
		RSSmartPtr	pElement
			( m_pInstance->GetMemberAs
				( *m_pContext, GetParameterID( i ) ), m_pContext ) ;
		RSStructuredPointer *	pStruct =
			ESLTypeCast<RSStructuredPointer>( pElement.GetEntity().Ptr() ) ;
		if ( (pStruct != nullptr) && (pStruct->GetRSClass() != nullptr) )
		{
			const wchar_t *	pwszClassName =
								pStruct->GetRSClass()->GetRSClassName() ;
			if ( SString::Compare( pwszClassName, L"Matrix3D" ) == 0 )
			{
				SGL3DMatrix<float32_t,3> *	pmatBuf =
					(SGL3DMatrix<float32_t,3>*)
						pStruct->GetPointer( sizeof(SGL3DMatrix<float32_t,3>) ) ;
				if ( pmatBuf != nullptr )
				{
					for ( int i = 0; i < 3; i ++ )
					{
						pmatBuf->m[i][0] = (float32_t) mat.m[i][0] ;
						pmatBuf->m[i][1] = (float32_t) mat.m[i][1] ;
						pmatBuf->m[i][2] = (float32_t) mat.m[i][2] ;
					}
				}
			}
			else if ( SString::Compare( pwszClassName, L"Quaternion" ) == 0 )
			{
				S3DQuaternion *	pqBuf =
					(S3DQuaternion*) pStruct->GetPointer( sizeof(S3DQuaternion) ) ;
				if ( pqBuf != nullptr )
				{
					*pqBuf = S3DDQuaternion( mat ) ;
				}
			}
		}
	}
	Controller::SetMatrixParameter( i, mat ) ;
}

void S3DSceneScriptInstance::SetVectorParameter
	( size_t i, const S3DDVector& vec )
{
	if ( (i > m_iParamRSClass) && (m_pLObject != nullptr) )
	{
		S3DSceneCustomProperty::SetVectorParameter( i - m_iParamBase, vec ) ;
		return ;
	}
	else if ( (i > m_iParamRSClass)
		&& (m_pInstance != nullptr) && (m_pContext != nullptr) )
	{
		RSSmartPtr	pElement
			( m_pInstance->GetMemberAs
				( *m_pContext, GetParameterID( i ) ), m_pContext ) ;
		RSStructuredPointer *	pStruct =
			ESLTypeCast<RSStructuredPointer>( pElement.GetEntity().Ptr() ) ;
		if ( (pStruct != nullptr) && (pStruct->GetRSClass() != nullptr) )
		{
			const wchar_t *	pwszClassName =
								pStruct->GetRSClass()->GetRSClassName() ;
			if ( (SString::Compare( pwszClassName, L"Vector3D" ) == 0)
				|| (SString::Compare( pwszClassName, L"Vector3D4" ) == 0) )
			{
				S3DVector *	pvBuf =
					(S3DVector*) pStruct->GetPointer( sizeof(S3DVector) ) ;
				if ( pvBuf != nullptr )
				{
					*pvBuf = vec ;
				}
			}
			else if ( SString::Compare( pwszClassName, L"RGBColor" ) == 0 )
			{
				SGLPalette *	ppBuf =
					(SGLPalette*) pStruct->GetPointer( sizeof(SGLPalette) ) ;
				if ( ppBuf != nullptr )
				{
					*ppBuf = ColorFromVector( vec ) ;
				}
			}
		}
	}
	Controller::SetVectorParameter( i, vec ) ;
}

void S3DSceneScriptInstance::SetScalarParameter
	( size_t i, double s )
{
	if ( (i > m_iParamRSClass) && (m_pLObject != nullptr) )
	{
		S3DSceneCustomProperty::SetScalarParameter( i - m_iParamBase, s ) ;
		return ;
	}
	else if ( (i > m_iParamRSClass)
		&& (m_pInstance != nullptr) && (m_pContext != nullptr) )
	{
		m_pInstance->SetMemberNumberAs
			( *m_pContext, GetParameterID( i ), s ) ;
	}
	Controller::SetScalarParameter( i, s ) ;
}

void S3DSceneScriptInstance::SetIntegerParameter
	( size_t i, int32_t n )
{
	if ( (i > m_iParamRSClass) && (m_pLObject != nullptr) )
	{
		S3DSceneCustomProperty::SetIntegerParameter( i - m_iParamBase, n ) ;
		return ;
	}
	else if ( (i > m_iParamRSClass)
		&& (m_pInstance != nullptr) && (m_pContext != nullptr) )
	{
		m_pInstance->SetMemberIntegerAs
			( *m_pContext, GetParameterID( i ), n ) ;
	}
	Controller::SetIntegerParameter( i, n ) ;
}

void S3DSceneScriptInstance::SetBooleanParameter
	( size_t i, bool b )
{
	if ( (i > m_iParamRSClass) && (m_pLObject != nullptr) )
	{
		S3DSceneCustomProperty::SetBooleanParameter( i - m_iParamBase, b ) ;
		return ;
	}
	else if ( (i > m_iParamRSClass)
		&& (m_pInstance != nullptr) && (m_pContext != nullptr) )
	{
		m_pInstance->SetMemberIntegerAs
			( *m_pContext, GetParameterID( i ), b ) ;
	}
	Controller::SetBooleanParameter( i, b ) ;
}

void S3DSceneScriptInstance::SetCommandParameter
	( size_t i, const wchar_t * pwszCmd )
{
	if ( i == m_iParamRSClass )
	{
		if ( (m_strRSClass != pwszCmd) || (m_pRSClass == nullptr) )
		{
			m_strRSClass = pwszCmd ;
			UpdateClassRef() ;
		}
		return ;
	}
	else if ( (i > m_iParamRSClass) && (m_pLObject != nullptr) )
	{
		S3DSceneCustomProperty::SetCommandParameter( i - m_iParamBase, pwszCmd ) ;
		return ;
	}
	else if ( (i > m_iParamRSClass)
			&& (m_pInstance != nullptr) && (m_pContext != nullptr) )
	{
		ParamOption *	pOption = m_aOptions.GetAt( i ) ;
		if ( pOption != nullptr )
		{
			ssize_t	iIndex = -1 ;
			int64_t	num =
				SXMLDocument::GetIntegerAsSymbolOf
					( pOption->m_aStrIntPairs.GetConstArray(), pwszCmd, 0, &iIndex ) ;
			if ( iIndex >= 0 )
			{
				m_pInstance->SetMemberIntegerAs
					( *m_pContext, GetParameterID( i ), num ) ;
			}
			return ;
		}
		m_pInstance->SetMemberStringAs
			( *m_pContext, GetParameterID( i ), pwszCmd ) ;
		CreateParameterSequencer( i ) ;
	}
	Controller::SetCommandParameter( i, pwszCmd ) ;
}

size_t S3DSceneScriptInstance::SetBinaryParameter
			( size_t i, const void * pSrc, size_t nBufBytes )
{
	if ( (i > m_iParamRSClass) && (m_pLObject != nullptr) )
	{
		return	S3DSceneCustomProperty::SetBinaryParameter
								( i - m_iParamBase, pSrc, nBufBytes ) ;
	}
	if ( (i > m_iParamRSClass)
		&& (m_pInstance != nullptr) && (m_pContext != nullptr) )
	{
		RSSmartPtr	pElement
			( m_pInstance->GetMemberAs
				( *m_pContext, GetParameterID( i ) ), m_pContext ) ;
		RSStructuredPointer *	pStruct =
			ESLTypeCast<RSStructuredPointer>( pElement.GetEntity().Ptr() ) ;
		if ( (pStruct != nullptr) && (pStruct->GetRSClass() != nullptr) )
		{
			const wchar_t *	pwszClassName =
								pStruct->GetRSClass()->GetRSClassName() ;
			if ( SString::Compare( pwszClassName, L"Vector2D" ) == 0 )
			{
				S2DVector *	pvBuf =
					(S2DVector*) pStruct->GetPointer( sizeof(S2DVector) ) ;
				if ( (pvBuf != nullptr) && (nBufBytes == sizeof(S2DDVector)) )
				{
					*pvBuf = *((S2DDVector*)pSrc) ;
					return	sizeof(S2DDVector) ;
				}
			}
			else if ( SString::Compare( pwszClassName, L"Matrix4D" ) == 0 )
			{
				S4DMatrix *	pmatBuf =
					(S4DMatrix*) pStruct->GetPointer( sizeof(S4DMatrix) ) ;
				if ( (pmatBuf != nullptr) && (nBufBytes == sizeof(S4DDMatrix)) )
				{
					*pmatBuf = *((S4DDMatrix*)pSrc) ;
					return	sizeof(S4DDMatrix) ;
				}
			}
		}
	}
	return	Controller::SetBinaryParameter( i, pSrc, nBufBytes ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneScriptInstance::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	if ( i == m_iParamRSClass )
	{
		S3DCompositionManager *	pManager = GetManager() ;
		if ( pManager != nullptr )
		{
			size_t	nCount = pManager->VM().GetElementCount() ;
			for ( size_t i = 0; i < nCount; i ++ )
			{
				RSObject *	pObj =
					pManager->VM().GetElementAt( pManager->Context(), (int) i ) ;
				if ( pObj == nullptr )
				{
					continue ;
				}
				RSStructuredPointerClass *	pStructClass =
					ESLTypeCast<RSStructuredPointerClass>( pObj ) ;
				if ( pStructClass != nullptr )
				{
					aStrSet.Add( new SString
						( pManager->VM().GetElementNameAt( (int) i ) ) ) ;
				}
				pObj->ReleaseRef() ;
			}
			const std::map< std::wstring, LPtr<LNamespace> >&
				vNamespaces = pManager->LoquatyVM()->Global()->GetNamespaceList() ;
			for ( auto iter = vNamespaces.begin(); iter != vNamespaces.end(); iter ++ )
			{
				LPtr<LNamespace>	pNamespace( iter->second ) ;
				if ( pNamespace == nullptr )
				{
					continue ;
				}
				LStructureClass *	pStructClass =
					dynamic_cast<LStructureClass*>( pNamespace.Ptr() ) ;
				if ( pStructClass != nullptr )
				{
					aStrSet.Add( new SString( iter->first.c_str() ) ) ;
				}
			}
			return	true ;
		}
	}
	else if ( (i > m_iParamRSClass) && (m_pLObject != nullptr) )
	{
		return	S3DSceneCustomProperty::EnumerateStringSet
									( i - m_iParamBase, aStrSet ) ;
	}
	else
	{
		ParamOption *	pOption = m_aOptions.GetAt( i ) ;
		if ( pOption != nullptr )
		{
			const SXMLDocument::AttrInteger *
				pPairs = pOption->m_aStrIntPairs.GetConstArray() ;
			for ( size_t j = 0; pPairs[j].pszSymbol != nullptr; j ++ )
			{
				aStrSet.Add( new SString( pPairs[j].pszSymbol ) ) ;
			}
			return	true ;
		}
	}
	return	false ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneScriptInstance::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	1:
	case	7:
		return	L"メンバ変数" ;
	}
	return	Controller::GetParameterCategoryName( iCategory ) ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DSceneScriptInstance::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags )
{
	uint32_t	nResFlags =
					Controller::UpdatePropertyReference( comp, pItem, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefScriptObject )
	{
		S3DCompositionManager *	pManager = GetManager() ;
		m_pContext = nullptr ;
		if ( (pManager != nullptr) && (m_pRSClass != nullptr) )
		{
			m_pContext = new Rosetta::RSContext( &(pManager->VM()) ) ;
		}
		SStrSortObjectArray
			<S3DSceneComposer::ParameterEntryStorage>	ssoaParam ;
		SaveInstanceParameters( ssoaParam ) ;
		//
		UpdateClassRef() ;
		//
		ResotreInstanceParameters( ssoaParam ) ;
	}
	//
	return	nResFlags ;
}

// コンポジション取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DSceneComposer::Composition *
	S3DSceneScriptInstance::GetComposition( void ) const
{
	return	Controller::GetComposition() ;
}

