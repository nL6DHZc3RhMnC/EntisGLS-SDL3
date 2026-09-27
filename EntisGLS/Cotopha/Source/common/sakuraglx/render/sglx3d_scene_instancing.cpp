
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/render/sglx3d_scene_item.h>

using namespace SSystem ;
using namespace SakuraGL ;
using namespace Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// インスタンスエントリ・コントローラー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DInstancingEntryInterface, ESLObject )
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DInstancingEntryEditInterface, S3DInstancingEntryInterface )
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DInstanceEntryController,
				Controller, S3DInstancingEntryInterface )
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DItemInstancingSerializer::RefIndex, SObject )
S3D_IMPLEMENT_COMPOSER_ITEM( S3DInstanceEntryController, instance_entry )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DInstanceEntryController::S3DInstanceEntryController( void )
	: Controller( m_ItemClassDescriptor.pwszClassID ),
		m_vPosition( 0, 0, 0 ), m_matRotation( 1, 1, 1 ), m_vZoom( 1, 1, 1 ),
		m_matInstance( 1, 1, 1, 1 ), m_clrInstance( 0xFFFFFFFF, 0 )
{
	m_iParamPosition =
		AddParameterEntry
			( L"position",
				S3DSceneComposer::typePosition,
				S3DSceneComposer::attrNoLocalTransform, L"位置" ) ;
	m_iParamRotation =
		AddParameterEntry
			( L"rotation",
				S3DSceneComposer::typeRotation, 0, L"回転" ) ;
	m_iParamZoom =
		AddParameterEntry
			( L"zoom",
				S3DSceneComposer::typeZoom, 0, L"拡大" ) ;
	m_iParamTransparency =
		AddParameterEntry
			( L"transparency",
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrUIScalarSlider,
				L"透明度", NULL, 0.0, 1.0 ) ;
	m_iParamColorMul =
		AddParameterEntry
			( L"color_mul",
				S3DSceneComposer::typeColor, 0, L"乗算色" ) ;
	m_iParamColorAdd =
		AddParameterEntry
			( L"color_add",
				S3DSceneComposer::typeColor, 0, L"加算色" ) ;
}

// 座標
//////////////////////////////////////////////////////////////////////////////
const S3DVector& S3DInstanceEntryController::GetPosition( void ) const
{
	return	m_vPosition ;
}

void S3DInstanceEntryController::SetPosition( const S3DVector& vPos )
{
	m_vPosition = vPos ;
}

// 回転
//////////////////////////////////////////////////////////////////////////////
const S3DMatrix& S3DInstanceEntryController::GetRotation( void ) const
{
	return	m_matRotation ;
}

void S3DInstanceEntryController::SetRotation( const S3DMatrix& matRot )
{
	m_matRotation = matRot ;
	CalcInstanceMatrix() ;
}

// 拡大
//////////////////////////////////////////////////////////////////////////////
const S3DVector& S3DInstanceEntryController::GetZoom( void ) const
{
	return	m_vZoom ;
}

void S3DInstanceEntryController::SetZoom( const S3DVector& vZoom )
{
	m_vZoom = vZoom ;
	CalcInstanceMatrix() ;
}

// 色効果・α
//////////////////////////////////////////////////////////////////////////////
const S3DColor& S3DInstanceEntryController::GetColorEffect( void ) const
{
	return	m_clrInstance ;
}

void S3DInstanceEntryController::SetColorEffect( const S3DColor& clrEffect )
{
	m_clrInstance = clrEffect ;
}

// インスタンス
//////////////////////////////////////////////////////////////////////////////
void S3DInstanceEntryController::GetInstanceEntry( S4DMatrix& mat, S3DColor& clr ) const
{
	mat = m_matInstance ;
	clr = m_clrInstance ;
}

void S3DInstanceEntryController::CalcInstanceMatrix( void )
{
	S3DMatrix	mat = m_matRotation ;
	mat.MagnifyByVector( m_vZoom ) ;
	//
	Matrix4x4From3x3
		<float32_t,S3DMatrix,S3DVector>
			( m_matInstance, mat, m_vPosition ) ;
}

// インスタンシング・リスト取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DInstanceEntryController::GetInstancingArray
	( const S4DMatrix*& pMatrixs, const S3DColor*& pColors ) const
{
	pMatrixs = &m_matInstance ;
	pColors = &m_clrInstance ;
	return	1 ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DInstanceEntryController::GetMatrixParameter( size_t i ) const
{
	if ( i == m_iParamRotation )
	{
		return	S3DDMatrix( m_matRotation ) ;
	}
	return	Controller::GetMatrixParameter( i ) ;
}

S3DDVector S3DInstanceEntryController::GetVectorParameter( size_t i ) const
{
	if ( i == m_iParamPosition )
	{
		return	S3DDVector( m_vPosition ) ;
	}
	else if ( i == m_iParamZoom )
	{
		return	S3DDVector( m_vZoom ) ;
	}
	else if ( i == m_iParamColorMul )
	{
		return	VectorFromColor( m_clrInstance.rgbMul ) ;
	}
	else if ( i == m_iParamColorAdd )
	{
		return	VectorFromColor( m_clrInstance.rgbAdd ) ;
	}
	return	Controller::GetVectorParameter( i ) ;
}

double S3DInstanceEntryController::GetScalarParameter( size_t i ) const
{
	if ( i == m_iParamTransparency )
	{
		return	1.0 - (double) m_clrInstance.rgbMul.argb.Alpha / 255.0 ;
	}
	return	Controller::GetScalarParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DInstanceEntryController::SetMatrixParameter( size_t i, const S3DDMatrix& mat )
{
	if ( i == m_iParamRotation )
	{
		m_matRotation = mat ;
		CalcInstanceMatrix() ;
		return ;
	}
}

void S3DInstanceEntryController::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	if ( i == m_iParamPosition )
	{
		m_vPosition = vec ;
		CalcInstanceMatrix() ;
		return ;
	}
	else if ( i == m_iParamZoom )
	{
		m_vZoom = vec ;
		CalcInstanceMatrix() ;
		return ;
	}
	else if ( i == m_iParamColorMul )
	{
		m_clrInstance.rgbMul.ui32 =
			(m_clrInstance.rgbMul.ui32 & 0xFF000000)
			| (ColorFromVector(vec).ui32 & 0x00FFFFFF) ;
		return ;
	}
	else if ( i == m_iParamColorAdd )
	{
		m_clrInstance.rgbAdd = ColorFromVector(vec) ;
		return ;
	}
}

void S3DInstanceEntryController::SetScalarParameter( size_t i, double s )
{
	if ( i == m_iParamTransparency )
	{
		m_clrInstance.rgbMul.argb.Alpha =
			(uint8_t) eslRoundR64ToLInt
						( 255.0 - esl_fclamp( s, 0.0, 1.0 ) * 255.0 ) ;
		return ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// インスタンシング・シリアライザ（共通処理）
//////////////////////////////////////////////////////////////////////////////

ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DItemInstancingSerializer::MultiRenderer, ESLObject ) ;

const wchar_t *	S3DItemInstancingSerializer::m_pwszSortingMethodName
					[S3DItemInstancingSerializer::sortMethodCount] =
{
	L"no_sorting",
	L"only_transparenct",
	L"all_items",
} ;

const wchar_t *	S3DItemInstancingSerializer::m_pwszCullingMethodName
					[S3DItemInstancingSerializer::CullingMethodCount] =
{
	L"no_culling",
	L"by_z",
	L"by_frustum",
} ;

// InstanceRef 代入
//////////////////////////////////////////////////////////////////////////////
const S3DItemInstancingSerializer::InstanceRef&
	S3DItemInstancingSerializer::InstanceRef::operator =
		( const S3DItemInstancingSerializer::InstanceRef& ref )
{
	m_refInstance = ref.m_refInstance ;
	m_refItem = ref.m_refItem ;
	return	*this ;
}

// InstanceRef 空判定
//////////////////////////////////////////////////////////////////////////////
bool S3DItemInstancingSerializer::InstanceRef::IsEmpty( void ) const
{
	return	(m_refInstance.GetReference() == nullptr) ;
}

// 同値判定
//////////////////////////////////////////////////////////////////////////////
bool S3DItemInstancingSerializer::InstanceRef::IsEqual
		( const S3DItemInstancingSerializer::InstanceRef& ref ) const
{
	return	(m_refInstance.GetReference() == ref.m_refInstance.GetReference()) ;
}

// 参照解放
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::InstanceRef::ReleaseRef( void )
{
	m_refInstance = nullptr ;
	m_refItem = nullptr ;
}

// 参照先アイテム
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ItemSerializer *
	S3DItemInstancingSerializer::InstanceRef::GetRefItem( void ) const
{
	return	m_refItem.Ref() ;
}

// インスタンス参照か？
//////////////////////////////////////////////////////////////////////////////
bool S3DItemInstancingSerializer::InstanceRef::IsInstance( void ) const
{
	return	(m_refInstance.GetRef<RefIndex>() != nullptr) ;
}

// インスタンス指標取得
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DItemInstancingSerializer::InstanceRef::GetInstanceIndex( void ) const
{
	RefIndex *	pRefIndex = m_refInstance.GetRef<RefIndex>() ;
	return	(pRefIndex != nullptr) ? (ssize_t) pRefIndex->m_iInstanceIndex : -1 ;
}

// InstanceInfo 代入
//////////////////////////////////////////////////////////////////////////////
const S3DItemInstancingSerializer::InstanceInfo&
	S3DItemInstancingSerializer::InstanceInfo::operator =
		( const S3DItemInstancingSerializer::InstanceInfo& info )
{
	type = info.type ;
	pMeshCol = info.pMeshCol ;
	pInstancing = info.pInstancing ;
	pItem = info.pItem ;
	pEntry = info.pEntry ;
	iController = info.iController ;
	iInstance = info.iInstance ;
	iAbsInstance = info.iAbsInstance ;
	mat4Instance = info.mat4Instance ;
	clrInstance = info.clrInstance ;
	return	*this ;
}

// InstanceInfo 比較
//////////////////////////////////////////////////////////////////////////////
bool S3DItemInstancingSerializer::InstanceInfo::IsEqual
	( const S3DItemInstancingSerializer::InstanceInfo& info ) const
{
	if ( type != info.type )
	{
		return	false ;
	}
	if ( type == S3DItemInstancingSerializer::typeNothing )
	{
		return	true ;
	}
	if ( pItem != info.pItem )
	{
		return	false ;
	}
	if ( type == S3DItemInstancingSerializer::typeItem )
	{
		return	true ;
	}
	if ( type == S3DItemInstancingSerializer::typeController )
	{
		if ( iController != info.iController )
		{
			return	false ;
		}
	}
	return	(iInstance == info.iInstance) ;
}

bool S3DItemInstancingSerializer::InstanceInfo::IsEmpty( void ) const
{
	return	(type == S3DItemInstancingSerializer::typeNothing)
			|| (pItem == NULL) ;
}

bool S3DItemInstancingSerializer::InstanceInfo::IsInstance( void ) const
{
	return	(pItem != NULL)
		&& ((type == S3DItemInstancingSerializer::typeController)
			|| (type == S3DItemInstancingSerializer::typeStaticInstance)) ;
}

// 参照取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SObject * S3DItemInstancingSerializer::InstanceInfo::GetRefObject( void ) const
{
	if ( type == S3DItemInstancingSerializer::typeStaticInstance )
	{
		ESLAssert( pInstancing != nullptr ) ;
		return	pInstancing->GetIndexReference( iInstance ) ;
	}
	if ( type == S3DItemInstancingSerializer::typeController )
	{
		S3DSceneComposer::Controller *	pCtrl =
			ESLTypeCast<S3DSceneComposer::Controller>( pEntry ) ;
		ESLAssert( pCtrl != nullptr ) ;
		if ( pCtrl != nullptr )
		{
			if ( ESLTypeCast<S3DInstancingEntryEditInterface>( pEntry ) == nullptr )
			{
				// S3DInstancingEntryEditInterface ではない場合
				// 削除はコントローラー毎削除される前提
				// ex. S3DInstanceEntryController 等
				return	pCtrl ;
			}
			// S3DInstancingEntryEditInterface 派生型の場合には
			// RemoveInstancingEntryAt で削除する前提
			// ex. S3DOPhysicsDynamicInstancingInterface 等
		}
		ESLAssert( pInstancing != nullptr ) ;
		return	pInstancing->GetIndexReference
					( pInstancing->InstanceIndexOfAbsInstance( iAbsInstance ) ) ;
	}
	if ( type == S3DItemInstancingSerializer::typeItem )
	{
		ESLAssert( pItem != nullptr ) ;
		return	pItem ;
	}
	return	nullptr ;
}

void S3DItemInstancingSerializer::InstanceInfo::GetInstanceRef( InstanceRef& ref ) const
{
	SaveInstanceReference( ref, *this ) ;
}

// インスタンス行列取得
//////////////////////////////////////////////////////////////////////////////
bool S3DItemInstancingSerializer::InstanceInfo::GetSafeInstanceMatrix
	( S4DMatrix& matrix, S3DColor& color,
		const S3DItemInstancingSerializer::InstanceRef& ref ) const
{
	return	GetSafeInstanceMatrixOf( matrix, color, *this, ref ) ;
}

bool S3DItemInstancingSerializer::InstanceInfo::GetSafeGlobalMatrix
	( S3DDMatrix& matrix, S3DDVector& pos, S3DColor& color,
		const S3DItemInstancingSerializer::InstanceRef& ref ) const
{
	return	GetSafeInstanceGlobalMatrixOf( matrix, pos, color, *this, ref ) ;
}


// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DItemInstancingSerializer::S3DItemInstancingSerializer( void )
	: m_qRotation( 1, 0, 0, 0 ),
		m_vZoom( 1, 1, 1 ),
		m_matInstanceBase( 1, 1, 1 ),
		m_matFaceDirection( 1, 1, 1 ),
		m_flagFaceDirection( false ),
		m_flagPotentialMode( false ),
		m_sorting( sortNothing ),
		m_culling( cullingNothing ),
		m_zNear( 0.0f ), m_zFar( 10000.0f ),
		m_nOpaqueCount( 0 )
{
}

// モデル基本回転
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::SetBaseRotation( const S3DDQuaternion& qRot )
{
	if ( m_qRotation != qRot )
	{
		m_qRotation = qRot ;
		UpdateBaseMatrix() ;
	}
}

// モデル基本拡大率
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::SetBaseZoom( const S3DDVector& vZoom )
{
	if ( m_vZoom != vZoom )
	{
		m_vZoom = vZoom ;
		UpdateBaseMatrix() ;
	}
}

// 面の方向を固定する
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::ForceFaceDirection( bool flagEnable )
{
	m_flagFaceDirection = flagEnable ;
}

void S3DItemInstancingSerializer::SetFaceDirection( const S3DMatrix& matFaceDir )
{
	m_matFaceDirection = matFaceDir ;
}

// ソート方式
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::SetSorting( S3DItemInstancingSerializer::SortingMethod sort )
{
	m_sorting = sort ;
}

void S3DItemInstancingSerializer::SetSortingByName( const wchar_t * pwszName )
{
	for ( int i = 0; i < sortMethodCount; i ++ )
	{
		if ( SString::Compare( m_pwszSortingMethodName[i], pwszName ) == 0 )
		{
			m_sorting = (SortingMethod) i ;
			break ;
		}
	}
}

// カリング方式
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::SetCulling( S3DItemInstancingSerializer::CullingMethod culling )
{
	m_culling = culling ;
}

void S3DItemInstancingSerializer::SetCullingByName( const wchar_t * pwszName )
{
	for ( int i = 0; i < CullingMethodCount; i ++ )
	{
		if ( SString::Compare( m_pwszCullingMethodName[i], pwszName ) == 0 )
		{
			m_culling = (CullingMethod) i ;
			break ;
		}
	}
}

// ｚ範囲
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::SetCullingNearZ( float32_t zNear )
{
	m_zNear = zNear ;
}

void S3DItemInstancingSerializer::SetCullingFarZ( float32_t zFar )
{
	m_zFar = zFar ;
}

// 基本変換行列反映
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::UpdateBaseMatrix( void )
{
	S3DDMatrix	matd ;
	m_qRotation.ToMatrix( matd ) ;
	matd.MagnifyByVector( m_vZoom ) ;
	//
	S3DMatrix	matBase = matd ;
	m_matInstanceBase = matBase ;
	//
	S4DMatrix *			pmat4Dst = m_aInstanceMatrixs.GetArray() ;
	const S3DMatrix *	pmat3Src = m_aInstanceOrgMatrixs.GetConstArray() ;
	size_t				nCount = m_aInstanceMatrixs.GetLength() ;
	ESLAssert( nCount <= m_aInstanceOrgMatrixs.GetLength() ) ;
	if ( nCount > m_aInstanceOrgMatrixs.GetLength() )
	{
		nCount = m_aInstanceOrgMatrixs.GetLength() ;
	}
	for ( size_t i = 0; i < nCount; i++ )
	{
		S3DMatrix	mat3 = *pmat3Src * matBase ;
		pmat4Dst->SetMatrix3( mat3 ) ;
		pmat3Src ++ ;
		pmat4Dst ++ ;
	}
	m_aInstanceMatrixs.FinishArray() ;
}

// インスタンシング・リスト設定
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::SetInstancingEntriesBase64( const wchar_t * pwszBase64 )
{
	m_strInstancingBase64 = pwszBase64 ;
	UpdateInstancingEntriesFromBase64() ;
}

void S3DItemInstancingSerializer::SetInstancingEntries
		( const S3DSceneComposer::BinaryInstancingData& bid )
{
	EncodeData( m_strInstancingBase64, bid ) ;
	UpdateInstancingEntries( bid ) ;
}

// インスタンシング・リスト取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DItemInstancingSerializer::GetInstancingDataLengthInBytes( void ) const
{
	return	sizeof(S3DSceneComposer::BinaryInstancingData)
			+ ((int) m_aInstanceMatrixs.GetLength() - 1)
				* sizeof(S3DSceneComposer::BinaryInstancingEntry) ;
}

size_t S3DItemInstancingSerializer::GetInstancingData
			( S3DSceneComposer::BinaryInstancingData& bid ) const
{
	const S4DMatrix *	pMatrixs = m_aInstanceMatrixs.GetConstArray() ;
	const S3DColor *	pColors = m_aInstanceColors.GetConstArray() ;
	const S3DMatrix *	pOrgMatrix = m_aInstanceOrgMatrixs.GetConstArray() ;
	bid.count = (uint32_t) m_aInstanceOrgMatrixs.GetLength() ;
	for ( size_t i = 0; i < bid.count; i ++ )
	{
		S3DSceneComposer::BinaryInstancingEntry&
									entry = bid.entries[i] ;
		entry.matrix = pMatrixs[i] ;
		entry.matrix.SetMatrix3( pOrgMatrix[i] ) ;
		entry.color = pColors[i] ;
	}
	return	sizeof(S3DSceneComposer::BinaryInstancingData)
			+ ((int) bid.count - 1)
				* sizeof(S3DSceneComposer::BinaryInstancingEntry) ;
}

// インスタンス・データ・パース
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::BinaryInstancingData *
	S3DItemInstancingSerializer::ParseData
		( SArray<uint8_t>& aBinary,
				const wchar_t * pwszBase64, ssize_t nStrLenght )
{
	Charset::DecodeBase64( aBinary, pwszBase64, nStrLenght ) ;
	//
	if ( aBinary.GetLength() <
			sizeof(S3DSceneComposer::BinaryHeader)
			+ sizeof(S3DSceneComposer::BinaryInstancingData) )
	{
		return	NULL ;
	}
	const uint8_t *	pBinary = aBinary.GetConstArray() ;
	const S3DSceneComposer::BinaryHeader *
		pBinHdr = (const S3DSceneComposer::BinaryHeader*) pBinary ;
	if ( pBinHdr->nType != S3DSceneComposer::binaryInstancing )
	{
		return	NULL ;
	}
	const S3DSceneComposer::BinaryInstancingData *
		pBinInstancing =
			(const S3DSceneComposer::BinaryInstancingData*)
				(pBinary + sizeof(S3DSceneComposer::BinaryHeader)) ;
	size_t	nCount = pBinInstancing->count ;
	size_t	nDataBytes =
		offsetof(S3DSceneComposer::BinaryInstancingData,entries[nCount]) ;
	if ( sizeof(S3DSceneComposer::BinaryHeader)
						+ nDataBytes > aBinary.GetLength() )
	{
		return	NULL ;
	}
	return	(S3DSceneComposer::BinaryInstancingData*) pBinInstancing ;
}

// インスタンス・データ・エンコード
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::EncodeData
	( SSystem::SString& strBase64,
		const S3DSceneComposer::BinaryInstancingData& bid )
{
	S3DSceneComposer::BinaryHeader	bhdr ;
	bhdr.nType = S3DSceneComposer::binaryInstancing ;
	bhdr.nSubType = 0 ;
	bhdr.nBodyBytes =
		sizeof(S3DSceneComposer::BinaryInstancingData)
			+ ((int) bid.count - 1)
				* sizeof(S3DSceneComposer::BinaryInstancingEntry) ;
	//
	SArray<uint8_t>	bufTemp ;
	bufTemp.AddArray( (const uint8_t*) &bhdr, sizeof(bhdr) ) ;
	bufTemp.AddArray
		( (const uint8_t*) &bid,
			sizeof(S3DSceneComposer::BinaryInstancingData)
			+ ((int) bid.count - 1)
				* sizeof(S3DSceneComposer::BinaryInstancingEntry) ) ;
	//
	Charset::EncodeBase64
		( strBase64, bufTemp.GetConstArray(), bufTemp.GetLength() ) ;
}

void S3DItemInstancingSerializer::UpdateInstancingEntriesFromBase64( void )
{
	m_aInstanceOrgMatrixs.RemoveAll() ;
	m_aInstanceMatrixs.RemoveAll() ;
	m_aInstanceColors.RemoveAll() ;
	//
	SArray<uint8_t>	aBinary ;
	S3DSceneComposer::BinaryInstancingData *
		pBinInstancing =
			ParseData( aBinary, m_strInstancingBase64,
						(ssize_t) m_strInstancingBase64.GetLength() ) ;
	if ( pBinInstancing == NULL )
	{
		return ;
	}
	UpdateInstancingEntries( *pBinInstancing ) ;
}

void S3DItemInstancingSerializer::UpdateInstancingEntries
		( const S3DSceneComposer::BinaryInstancingData& bid )
{
	m_aInstanceOrgMatrixs.SetLength( bid.count ) ;
	m_aInstanceMatrixs.SetLength( bid.count ) ;
	m_aInstanceColors.SetLength( bid.count ) ;
	//
	S3DMatrix *	pOrgMat = m_aInstanceOrgMatrixs.GetArray() ;
	S4DMatrix *	pMatrix = m_aInstanceMatrixs.GetArray() ;
	S3DColor *	pColor = m_aInstanceColors.GetArray() ;
	S3DMatrix	mat3Base = m_matInstanceBase ;
	for ( size_t i = 0; i < bid.count; i ++ )
	{
		S4DMatrix	mat4 = bid.entries[i].matrix ;
		S3DMatrix	mat3 = mat4.GetMatrix3() ;
		mat4.SetMatrix3( mat3 * mat3Base ) ;
		pOrgMat[i] = mat3 ;
		pMatrix[i] = mat4 ;
		pColor[i] = bid.entries[i].color ;
	}
	m_aInstanceOrgMatrixs.FinishArray() ;
	m_aInstanceMatrixs.FinishArray() ;
	m_aInstanceColors.FinishArray() ;
}

// 静インスタンス数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DItemInstancingSerializer::GetStaticInstanceCount( void ) const
{
	ESLAssert( m_aInstanceMatrixs.GetLength() == m_aInstanceColors.GetLength() ) ;
	return	m_aInstanceMatrixs.GetLength() ;
}

// 静インスタンス取得
//////////////////////////////////////////////////////////////////////////////
bool S3DItemInstancingSerializer::GetStaticInstanceAt
	( size_t i, S4DMatrix& matrix, S3DColor& color ) const
{
	if ( m_flagPotentialMode )
	{
		ESLAssert( m_aTempMatrixs.GetLength() == m_aTempColors.GetLength() ) ;
		if ( i < m_aTempMatrixs.GetLength() )
		{
			matrix = m_aTempMatrixs.At(i) ;
			color = m_aTempColors.At(i) ;
			return	true ;
		}
	}
	ESLAssert( m_aInstanceMatrixs.GetLength() == m_aInstanceColors.GetLength() ) ;
	if ( i < m_aInstanceMatrixs.GetLength() )
	{
		matrix = m_aInstanceMatrixs.At(i) ;
		color = m_aInstanceColors.At(i) ;
		return	true ;
	}
	return	false ;
}

// 静インスタンス挿入
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::InsertStaticInstanceAt
	( size_t nIndex, const S4DMatrix& matrix, const S3DColor& color,
		S3DSceneComposer::ItemSerializer * pNotifyItem )
{
	ESLAssert( m_aInstanceMatrixs.GetLength() == m_aInstanceColors.GetLength() ) ;
	m_aInstanceMatrixs.InsertAt( nIndex, matrix ) ;
	m_aInstanceColors.InsertAt( nIndex, color ) ;
	//
	InsertIndexByInstanceIndex( nIndex ) ;
	m_aDynamicIndex.InsertAt( nIndex, nIndex ) ;
	//
	if ( pNotifyItem != NULL )
	{
		NotifyOnInsertedInstance( nIndex, matrix, color, pNotifyItem ) ;
	}
}

void S3DItemInstancingSerializer::NotifyOnInsertedInstance
	( size_t nIndex, const S4DMatrix& matrix, const S3DColor& color,
		S3DSceneComposer::ItemSerializer * pNotifyItem )
{
	size_t	nCtrl = pNotifyItem->GetControllerCount() ;
	for ( size_t i = 0; i < nCtrl; i ++ )
	{
		S3DPotentialInstancingInterface *	pCtrl =
			ESLTypeCast<S3DPotentialInstancingInterface>
					( pNotifyItem->GetControllerAt( i ) ) ;
		if ( pCtrl != NULL )
		{
			pCtrl->OnInsertedInstance( this, nIndex, matrix, color ) ;
		}
	}
}

// 静インスタンス削除
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::RemoveStaticInstanceAt
	( size_t nIndex, S3DSceneComposer::ItemSerializer * pNotifyItem )
{
	DeleteIndexByInstanceIndex( nIndex ) ;
	//
	ESLAssert( m_aInstanceMatrixs.GetLength() == m_aInstanceColors.GetLength() ) ;
	m_aInstanceMatrixs.RemoveAt( nIndex ) ;
	m_aInstanceColors.RemoveAt( nIndex ) ;
	//
	if ( pNotifyItem != NULL )
	{
		NotifyOnRemovedInstance( nIndex, pNotifyItem ) ;
	}
}

void S3DItemInstancingSerializer::NotifyOnRemovedInstance
	( size_t nIndex, S3DSceneComposer::ItemSerializer * pNotifyItem )
{
	size_t	nCtrl = pNotifyItem->GetControllerCount() ;
	for ( size_t i = 0; i < nCtrl; i ++ )
	{
		S3DPotentialInstancingInterface *	pCtrl =
			ESLTypeCast<S3DPotentialInstancingInterface>
					( pNotifyItem->GetControllerAt( i ) ) ;
		if ( pCtrl != NULL )
		{
			// ※S3DPotentialInstancingInterface は全ての静的・動的インスタンスを
			//   後処理するため、アイテム配下の通しインデックスを通知する
			pCtrl->OnRemovedInstance( this, nIndex ) ;
		}
	}
}

// 静インスタンスを m_aInstanceOrgMatrixs にも複製して base64 文字列更新
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::UpdateStaticInstancingEntriesBase64( void )
{
	size_t	nCount = m_aInstanceMatrixs.GetLength() ;
	m_aInstanceOrgMatrixs.SetLength( nCount ) ;
	//
	const S4DMatrix *	pMatrixs = m_aInstanceMatrixs.GetConstArray() ;
	S3DMatrix *			pOrgMatrix = m_aInstanceOrgMatrixs.GetArray() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pOrgMatrix[i] = pMatrixs[i].GetMatrix3() ;
	}
	m_aInstanceOrgMatrixs.FinishArray() ;
	//
	UpdateInstancingEntriesBase64() ;
}

// base64 文字列更新
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::UpdateInstancingEntriesBase64( void )
{
	SArray<uint8_t>	bufData ;
	const size_t	nDataBytes = GetInstancingDataLengthInBytes() ;
	S3DSceneComposer::BinaryInstancingData *
		pbid = (S3DSceneComposer::BinaryInstancingData*) bufData.GetArray( nDataBytes ) ;
	//
	GetInstancingData( *pbid ) ;
	//
	EncodeData( m_strInstancingBase64, *pbid ) ;
	//
	bufData.FinishArray() ;
}

// ダイナミック・インスタンス・リセット
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::ResetDynamicInstancingEntries( void )
{
	m_aTempMatrixs.RemoveAll() ;
	m_aTempColors.RemoveAll() ;
	m_aTempSourceItems.RemoveAll() ;
	m_aTempSourceIndexes.RemoveAll() ;
}

// インスタンシング・エントリ・コントローラー反映更新
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::UpdateDynamicInstancingEntries
		( const S3DSceneComposer::ItemSerializer * pItem )
{
	m_flagPotentialMode = false ;
	//
	const S4DMatrix*	pPotentialMatrixs = NULL ;
	const S3DColor*		pPotentialColors = NULL ;
	size_t				nPotentialCount = 0 ;
	//
	size_t		nCtrls = pItem->GetControllerCount() ;
	for ( size_t i = 0; i < nCtrls; i ++ )
	{
		S3DSceneComposer::Controller *	pCtrl = pItem->GetControllerAt( i ) ;
		if ( (pCtrl == NULL)
			|| pCtrl->IsControllerDisabled() )
		{
			continue ;
		}
		S3DInstancingEntryInterface *	pInstancing =
			ESLTypeCast<S3DInstancingEntryInterface>( pCtrl ) ;
		if ( pInstancing != NULL )
		{
			if ( pPotentialMatrixs != NULL )
			{
				UpdatePotentialInstance
					( pPotentialMatrixs, pPotentialColors, nPotentialCount ) ;
				//
				pPotentialMatrixs = NULL ;
				pPotentialColors = NULL ;
				nPotentialCount = 0 ;
			}
			const S4DMatrix*	pMatrixs = NULL ;
			const S3DColor*		pColors = NULL ;
			size_t	nCount =
				pInstancing->GetInstancingArray( pMatrixs, pColors ) ;
			//
			AddDynamicInstancingEntries
				( pMatrixs, pColors, nCount,
					(S3DSceneComposer::ParameterProperty*) pItem ) ;
		}
		else
		{
			S3DPotentialInstancingInterface *	pPotential =
				ESLTypeCast<S3DPotentialInstancingInterface>( pCtrl ) ;
			if ( pPotential != NULL )
			{
				if ( !m_flagPotentialMode )
				{
					MakeStaticInstancePotentialMode( pItem ) ;
					//
					pPotentialMatrixs = NULL ;
					pPotentialColors = NULL ;
					nPotentialCount = 0 ;
				}
				if ( pPotentialMatrixs == NULL )
				{
					pPotentialMatrixs = m_aTempMatrixs.GetConstArray() ;
					pPotentialColors = m_aTempColors.GetConstArray() ;
					nPotentialCount = m_aTempMatrixs.GetLength() ;
				}
				if ( pPotential->ProcessInstance
					( pPotentialMatrixs,
						pPotentialColors, nPotentialCount ) )
				{
					nPotentialCount =
						pPotential->GetPotentialInstancingArray
								( pPotentialMatrixs, pPotentialColors ) ;
				}
			}
		}
	}
	if ( pPotentialMatrixs != NULL )
	{
		UpdatePotentialInstance
			( pPotentialMatrixs, pPotentialColors, nPotentialCount ) ;
	}
}

// PotentialMode 用の初期化呼び出し
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::NotifyResetPotentialInstance
				( const S3DSceneComposer::ItemSerializer * pItem )
{
	if ( m_flagPotentialMode )
	{
		size_t		nCtrls = pItem->GetControllerCount() ;
		for ( size_t i = 0; i < nCtrls; i ++ )
		{
			S3DSceneComposer::Controller *	pCtrl = pItem->GetControllerAt( i ) ;
			S3DPotentialInstancingInterface *	pPotential =
				ESLTypeCast<S3DPotentialInstancingInterface>( pCtrl ) ;
			if ( pPotential != NULL )
			{
				pPotential->ResetInstance
					( m_aTempMatrixs.GetConstArray(),
						m_aTempColors.GetConstArray(),
						m_aTempMatrixs.GetLength() ) ;
			}
		}
	}
}

// 追加用バッファ処理
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::AddDynamicInstancingEntries
	( const S4DMatrix * pMatrixs,
		const S3DColor * pColors, size_t nCount, ESLObject * pSourceItem )
{
	if ( nCount >= 1 )
	{
		size_t	nLast = m_aTempMatrixs.GetLength() ;
		m_aTempMatrixs.SetLength( nLast + nCount ) ;
		m_aTempColors.AddArray( pColors, nCount ) ;
		m_aTempSourceItems.SetLength( nLast + nCount ) ;
		m_aTempSourceIndexes.SetLength( nLast + nCount ) ;
		//
		S3DMatrix	mat3Base = m_matInstanceBase ;
		S4DMatrix *	pmat4Dst = m_aTempMatrixs.GetAt( nLast ) ;
		ESLObject**	ppSrcItem = m_aTempSourceItems.GetArray() + nLast ;
		size_t *	pSrcIndex = m_aTempSourceIndexes.GetArray() + nLast ;
		for ( size_t j = 0; j < nCount; j ++ )
		{
			S4DMatrix	mat4 = pMatrixs[j] ;
			mat4.SetMatrix3( mat4.GetMatrix3() * mat3Base ) ;
			pmat4Dst[j] = mat4 ;
			ppSrcItem[j] = pSourceItem ;
			pSrcIndex[j] = j ;
		}
		m_aTempSourceItems.FinishArray() ;
		m_aTempSourceIndexes.FinishArray() ;
	}
}

// ダイナミック・インスタンス（ソート前の静インスタンスを含む指標）取得
//////////////////////////////////////////////////////////////////////////////
bool S3DItemInstancingSerializer::GetDynamicInstanceAt
	( size_t i, S4DMatrix& matrix, S3DColor& color ) const
{
	ESLAssert( m_aTempMatrixs.GetLength() == m_aTempColors.GetLength() ) ;
	if ( i < m_aTempMatrixs.GetLength() )
	{
		matrix = m_aTempMatrixs.At(i) ;
		color = m_aTempColors.At(i) ;
		return	true ;
	}
	if ( i < m_aInstanceMatrixs.GetLength() )
	{
		matrix = m_aInstanceMatrixs.At(i) ;
		color = m_aInstanceColors.At(i) ;
		return	true ;
	}
	return	false ;
}

// PotentialMode へ静的インスタンスを移行
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::MakeStaticInstancePotentialMode
	( const S3DSceneComposer::ItemSerializer * pItem )
{
	size_t	nStaticCount = m_aInstanceMatrixs.GetLength() ;
	ESLAssert( m_aInstanceColors.GetLength() == nStaticCount ) ;
	//
	m_aTempMatrixs.Insert( 0, nStaticCount ) ;
	m_aTempColors.Insert( 0, nStaticCount ) ;
	m_aTempSourceItems.Insert( 0, nStaticCount ) ;
	m_aTempSourceIndexes.Insert( 0, nStaticCount ) ;
	//
	const S4DMatrix *	pmat4Src = m_aInstanceMatrixs.GetConstArray() ;
	const S3DColor *	pColorSrc = m_aInstanceColors.GetConstArray() ;
	S4DMatrix *			pmat4Dst = m_aTempMatrixs.GetArray() ;
	S3DColor *			pColorDst = m_aTempColors.GetArray() ;
	ESLObject**			ppSrcItem = m_aTempSourceItems.GetArray() ;
	size_t *			pSrcIndex = m_aTempSourceIndexes.GetArray() ;
	//
	for ( size_t i = 0; i < nStaticCount; i ++ )
	{
		pmat4Dst[i] = pmat4Src[i] ;
		pColorDst[i] = pColorSrc[i] ;
		ppSrcItem[i] = (S3DSceneComposer::ParameterProperty*) pItem ;
		pSrcIndex[i] = i ;
	}
	m_aTempMatrixs.FinishArray() ;
	m_aTempColors.FinishArray() ;
	m_aTempSourceItems.FinishArray() ;
	m_aTempSourceIndexes.FinishArray() ;
	//
	m_flagPotentialMode = true ;
}

// Potential なインスタンスを更新
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::UpdatePotentialInstance
	( const S4DMatrix* pMatrixs, const S3DColor * pColors, size_t nCount )
{
	if ( nCount != m_aTempMatrixs.GetLength() )
	{
		m_aTempMatrixs.SetLength( nCount ) ;
		m_aTempColors.SetLength( nCount ) ;
		m_aTempSourceItems.SetLength( nCount ) ;
		m_aTempSourceIndexes.SetLength( nCount ) ;
	}
	S4DMatrix *	pmat4Dst = m_aTempMatrixs.GetArray() ;
	S3DColor *	pColorDst = m_aTempColors.GetArray() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pmat4Dst[i] = pMatrixs[i] ;
		pColorDst[i] = pColors[i] ;
	}
	m_aTempMatrixs.FinishArray() ;
	m_aTempColors.FinishArray() ;
}

// インスタンスの参照取得
//////////////////////////////////////////////////////////////////////////////
S3DItemInstancingSerializer::InstanceRef
	S3DItemInstancingSerializer::GetInstanceRefAt
		( S3DSceneComposer::ItemSerializer * pItem, size_t iInsyanceIndex )
{
	RefIndex *	pRefIndex = GetIndexReference( iInsyanceIndex ) ;
	if ( pRefIndex == nullptr )
	{
		return	InstanceRef() ;
	}
	return	InstanceRef( pItem, pRefIndex ) ;
}

// インスタンス指標への参照オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
S3DItemInstancingSerializer::RefIndex *
	S3DItemInstancingSerializer::GetIndexReference( size_t iInstanceIndex )
{
	RefIndex *	pRefIndex = nullptr ;
	m_csLock.Lock() ;
	for ( size_t i = 0; i < m_aRefIndexes.GetLength(); i ++ )
	{
		RefIndex *	pfi = m_aRefIndexes.GetAt(i) ;
		ESLAssert( pfi != nullptr ) ;
		if ( (pfi != nullptr) && (pfi->m_iInstanceIndex == iInstanceIndex) )
		{
			pRefIndex = pfi ;
			break ;
		}
	}
	if ( pRefIndex == nullptr )
	{
		pRefIndex = new RefIndex ;
		pRefIndex->m_iInstanceIndex = iInstanceIndex ;
		m_aRefIndexes.Add( pRefIndex ) ;
	}
	m_csLock.Unlock() ;
	return	pRefIndex ;
}

// インスタンス指標取得（存在しない場合には -1）
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DItemInstancingSerializer::GetReferenceIndexOf
		( S3DItemInstancingSerializer::RefIndex * pRefIndex ) const
{
	ssize_t	iIndex = -1 ;
	m_csLock.Lock() ;
	if ( pRefIndex != nullptr )
	{
		if ( m_aRefIndexes.FindPtr( pRefIndex ) >= 0 )
		{
			iIndex = (ssize_t) pRefIndex->m_iInstanceIndex ;
		}
	}
	m_csLock.Unlock() ;
	return	iIndex ;
}

// インスタンス・インデックス・リセット
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::ResetInstanceIndex( void )
{
	const size_t	nTotalCount =
		m_aTempMatrixs.GetLength()
			+ (m_flagPotentialMode ? 0 : m_aInstanceMatrixs.GetLength()) ;
	m_aDynamicIndex.SetLength( nTotalCount ) ;
	//
	size_t *	pIndexArray = m_aDynamicIndex.GetArray() ;
	for ( size_t i = 0; i < nTotalCount; i ++ )
	{
		pIndexArray[i] = i ;
	}
	m_aDynamicIndex.FinishArray() ;
}

// 動的処理実行
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::DoDynamicProcess
	( S3DScene& scene,
		const S3DDMatrix & matdModel, const S3DDVector& vdModel,
		float32_t fpInstanceSize, float32_t radPlayOfAngle )
{
	const size_t	nTotalCount =
		m_aTempMatrixs.GetLength()
			+ (m_flagPotentialMode ? 0 : m_aInstanceMatrixs.GetLength()) ;
	m_aDynamicMatrixs.SetLength( nTotalCount ) ;
	m_aDynamicColors.SetLength( nTotalCount ) ;
	m_aDynamicSort.SetLength( nTotalCount ) ;
	m_nOpaqueCount = nTotalCount ;
	//
	S4DMatrix *	pDstMatrics = m_aDynamicMatrixs.GetArray() ;
	S3DColor *	pDstColors = m_aDynamicColors.GetArray() ;
	SortIndex *	pSortArray = m_aDynamicSort.GetArray() ;
	size_t		iDst = 0 ;
	//
	// 視錐台情報取得
	//
	FrustumInfo	fi ;
	S3DDMatrix	matdCamera ;//, matdModel ;
	S3DDVector	vdCamera ;//, vdModel ;
	S3DMatrix	matBase ;
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
	fi.rectView.w = (int32_t) esl_roundfi(pp.vScreen.x * 2.0f) ;
	fi.rectView.h = (int32_t) esl_roundfi(pp.vScreen.y * 2.0f) ;
	//
	matdCamera = scene.GetCurrentCameraTransformation( vdCamera ) ;
	//
//	render.GetViewPort( fi.rectView ) ;
//	render.GetProjectionScreen( fi.vScreen, fi.zScale, fi.fpPixelAspect ) ;
//	render.GetCamera( matdCamera, vdCamera ) ;
//	render.GetMatrixTransformation( matdModel, vdModel ) ;
	//
	matBase = matdCamera * matdModel ;
	vBase = matdCamera * vdModel + vdCamera ;
	//
	float32_t	sinPlayOfAngle = (float32_t) sin(radPlayOfAngle) ;
	PrepareCullingInfo( fi, sinPlayOfAngle, sinPlayOfAngle ) ;
	//
	// 方向強制用
	//
	S3DMatrix	matFaceDir ;
//	S3DVector	vCameraFaceDir ;
	if ( m_flagFaceDirection )
	{
		S3DMatrix	matBaseRot ;
		S3DQuaternion(m_qRotation).ToMatrix( matBaseRot ) ;
		matFaceDir = m_matFaceDirection * matBaseRot ;
		//
		/*
		S3DDVector	vdCameraFaceDir =
			scene.GetCurrentCameraIMatrix() * S3DDVector( 0, 0, 1 ) ;
		vCameraFaceDir = matdModel.Inverse() * vdCameraFaceDir ;
		vCameraFaceDir.Normalize() ;
		*/
	}
	//
	// 静的インスタンス
	//
	const S4DMatrix *	pSrcMatrics = m_aInstanceMatrixs.GetConstArray() ;
	const S3DColor *	pSrcColors = m_aInstanceColors.GetConstArray() ;
	size_t	nSrcCount = m_aInstanceMatrixs.GetLength() ;
	size_t	i ;
	//
	if ( !m_flagPotentialMode )
	{
		for ( i = 0; i < nSrcCount; i ++ )
		{
//			S3DMatrix	mat3 = matBase * pSrcMatrics[i].GetMatrix3() ;
			S3DVector	pos = matBase * pSrcMatrics[i].GetTranslation() + vBase ;
			//
			if ( !IsCullingInstance( fi, /*mat3,*/ pos, fpInstanceSize ) )
			{
				SortIndex&	si = pSortArray[iDst] ;
				si.z = pos.z ;
				si.i = (uint32_t) iDst ;
				//
				pDstMatrics[iDst] = pSrcMatrics[i] ;
				pDstColors[iDst] = pSrcColors[i] ;
				//
				if ( m_flagFaceDirection )
				{
					ForceInstanceFaceDirection
						( pDstMatrics[iDst], matFaceDir /*, vCameraFaceDir*/ ) ;
				}
				iDst ++ ;
			}
		}
	}
	//
	// 動的インスタンス
	//
	pSrcMatrics = m_aTempMatrixs.GetConstArray() ;
	pSrcColors = m_aTempColors.GetConstArray() ;
	nSrcCount = m_aTempMatrixs.GetLength() ;
	//
	for ( i = 0; i < nSrcCount; i ++ )
	{
//		S3DMatrix	mat3 = matBase * pSrcMatrics[i].GetMatrix3() ;
		S3DVector	pos = matBase * pSrcMatrics[i].GetTranslation() + vBase ;
		//
		if ( !IsCullingInstance( fi, /*mat3,*/ pos, fpInstanceSize ) )
		{
			SortIndex&	si = pSortArray[iDst] ;
			si.z = pos.z ;
			si.i = (uint32_t) iDst ;
			//
			pDstMatrics[iDst] = pSrcMatrics[i] ;
			pDstColors[iDst] = pSrcColors[i] ;
			//
			if ( m_flagFaceDirection )
			{
				ForceInstanceFaceDirection
					( pDstMatrics[iDst], matFaceDir /*, vCameraFaceDir*/ ) ;
			}
			iDst ++ ;
		}
	}
	m_aDynamicMatrixs.FinishArray() ;
	m_aDynamicColors.FinishArray() ;
	//
	ESLAssert( iDst <= nTotalCount ) ;
	m_aDynamicMatrixs.SetLength( iDst ) ;
	m_aDynamicColors.SetLength( iDst ) ;
	pDstMatrics = m_aDynamicMatrixs.GetArray() ;
	pDstColors = m_aDynamicColors.GetArray() ;
	//
	// ソート判定
	//
	if ( (m_sorting == sortNothing) || (iDst < 2) )
	{
		return ;
	}
	size_t	nCount = iDst ;
	if ( m_sorting == sortAllItems )
	{
		DoSortIndex( pSortArray, nCount ) ;
	}
	else
	{
		//
		// 半透明インスタンスを分離
		//
		ssize_t	iOpaque = 0 ;
		ssize_t	iTransparent = (ssize_t) nCount - 1 ;
		//
		while ( iOpaque < iTransparent )
		{
			while ( iOpaque < iTransparent )
			{
				if ( pDstColors[iOpaque].rgbMul.argb.Alpha < 0xFE )
				{
					// 半透明発見
					break ;
				}
				iOpaque ++ ;
			}
			while ( iOpaque < iTransparent )
			{
				if ( pDstColors[iTransparent].rgbMul.argb.Alpha >= 0xFE )
				{
					// 不透明発見
					break ;
				}
				iTransparent -- ;
			}
			if ( iOpaque < iTransparent )
			{
				SortIndex	si = pSortArray[iOpaque] ;
				pSortArray[iOpaque] = pSortArray[iTransparent] ;
				pSortArray[iTransparent] = si ;
				iOpaque ++ ;
				iTransparent -- ;
			}
		}
		if ( (iOpaque == iTransparent)
			&& (pDstColors[iOpaque].rgbMul.argb.Alpha >= 0xFE) )
		{
			iOpaque ++ ;
		}
		ESLAssert( iOpaque <= (ssize_t) nCount ) ;
		//
		DoSortIndex( pSortArray + iOpaque, nCount - iOpaque ) ;
		m_nOpaqueCount = iOpaque ;
	}
	//
	// ソートしたインデックスで並び替え
	//
	S4DMatrix *	pSrcTempMatrics = m_aDynamicMatrixs2.GetArray( nCount ) ;
	S3DColor *	pSrcTempColors = m_aDynamicColors2.GetArray( nCount ) ;
	eslCopyMemory
		( pSrcTempMatrics, pDstMatrics, nCount * sizeof(S4DMatrix) ) ;
	eslCopyMemory
		( pSrcTempColors, pDstColors, nCount * sizeof(S3DColor) ) ;
	//
	S4DMatrix	mat4Temp ;
	S3DColor	clrTemp ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		size_t	j = pSortArray[i].i ;
		pDstMatrics[i] = pSrcTempMatrics[j] ;
		pDstColors[i] = pSrcTempColors[j] ;
	}
	//
	m_aDynamicMatrixs2.FinishArray() ;
	m_aDynamicColors2.FinishArray() ;
	//
	m_aDynamicMatrixs.FinishArray() ;
	m_aDynamicColors.FinishArray() ;
	m_aDynamicSort.FinishArray() ;
}

// 絶対インデックス（フレーム開始時のアイテム削除の影響のない）から
// 削除の影響を受けた実際のインスタンスのインデックスを取得
// （静的インスタンス＋動的インスタンス統合順）
//////////////////////////////////////////////////////////////////////////////
size_t S3DItemInstancingSerializer::InstanceIndexOfAbsInstance( size_t iAbsInstance ) const
{
	size_t *	pIndex = m_aDynamicIndex.GetAt( iAbsInstance ) ;
	if ( pIndex == NULL )
	{
		return	(size_t) -1 ;
	}
	return	*pIndex ;
}

// 描画されたインデックスからソート前のインデックスを取得
// （静的インスタンス＋動的インスタンス統合順）
//////////////////////////////////////////////////////////////////////////////
size_t S3DItemInstancingSerializer::InstanceIndexOfSortedInstance( size_t iSortedInstance ) const
{
	SortIndex *	psi = m_aDynamicSort.GetAt( iSortedInstance ) ;
	if ( (psi == NULL) || (psi->i != (uint32_t) -1) )
	{
		return	(size_t) -1 ;
	}
	return	InstanceIndexOfAbsInstance( psi->i ) ;
}

// インスタンス・インデックスから絶対インデックスを取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DItemInstancingSerializer::AbsInstanceOfInstanceIndex( size_t iInstanceIndex ) const
{
	const size_t *	pIndex = m_aDynamicIndex.GetConstArray() ;
	const size_t	nCount = m_aDynamicIndex.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( pIndex[i] == iInstanceIndex )
		{
			return	i ;
		}
	}
	return	(size_t) -1 ;
}

// インデックスを挿入する
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::InsertIndexByInstanceIndex( size_t iInstanceIndex )
{
	size_t *	pAbsIndex = m_aDynamicIndex.GetArray() ;
	size_t		nAbsCount = m_aDynamicIndex.GetLength() ;
	for ( size_t i = 0; i < nAbsCount; i ++ )
	{
		if ( (pAbsIndex[i] != (size_t) -1)
			&& (pAbsIndex[i] >= iInstanceIndex) )
		{
			pAbsIndex[i] ++ ;
		}
	}
	m_aDynamicIndex.FinishArray() ;

	for ( size_t i = 0; i < m_aRefIndexes.GetLength(); i ++ )
	{
		RefIndex *	pri = m_aRefIndexes.GetAt(i) ;
		if ( pri != nullptr )
		{
			if ( pri->m_iInstanceIndex >= iInstanceIndex )
			{
				pri->m_iInstanceIndex ++ ;
			}
		}
		else
		{
			m_aRefIndexes.RemoveAt( i -- ) ;
		}
	}
}

// インデックスを削除する
//////////////////////////////////////////////////////////////////////////////
bool S3DItemInstancingSerializer::DeleteIndexByAbsInstance( size_t iAbsInstance )
{
	size_t *	pIndex = m_aDynamicIndex.GetAt( iAbsInstance ) ;
	if ( (pIndex == NULL) || (*pIndex == (size_t) -1) )
	{
		return	false ;
	}
	size_t	iInstanceIndex = *pIndex ;
	*pIndex = (size_t) -1 ;
	//
	DeleteIndexByInstanceIndex( iInstanceIndex ) ;
	return	true ;
}

bool S3DItemInstancingSerializer::DeleteIndexByInstanceIndex( size_t iInstanceIndex )
{
	bool		flagDeleted = false ;
	size_t *	pAbsIndex = m_aDynamicIndex.GetArray() ;
	size_t		nAbsCount = m_aDynamicIndex.GetLength() ;
	for ( size_t i = 0; i < nAbsCount; i ++ )
	{
		if ( (pAbsIndex[i] != (size_t) -1)
			&& (pAbsIndex[i] > iInstanceIndex) )
		{
			pAbsIndex[i] -- ;
		}
		else if ( pAbsIndex[i] == iInstanceIndex )
		{
			flagDeleted = true ;
			pAbsIndex[i] = (size_t) -1 ;
		}
	}
	m_aDynamicIndex.FinishArray() ;

	for ( size_t i = 0; i < m_aRefIndexes.GetLength(); i ++ )
	{
		RefIndex *	pri = m_aRefIndexes.GetAt(i) ;
		if ( pri != nullptr )
		{
			if ( pri->m_iInstanceIndex > iInstanceIndex )
			{
				pri->m_iInstanceIndex -- ;
			}
			else if ( pri->m_iInstanceIndex == iInstanceIndex )
			{
				m_aRefIndexes.RemoveAt( i -- ) ;
			}
		}
		else
		{
			m_aRefIndexes.RemoveAt( i -- ) ;
		}
	}

	return	flagDeleted ;
}

// AddDynamicInstancingEntries で設定されたソースを取得する
//////////////////////////////////////////////////////////////////////////////
ESLObject * S3DItemInstancingSerializer::GetSourceItemOfAbsInstance
						( size_t& iSubIndex, size_t iAbsInstance ) const
{
	size_t	iInstance = InstanceIndexOfAbsInstance( iAbsInstance ) ;
	if ( iInstance < m_aInstanceMatrixs.GetLength() )
	{
		return	NULL ;
	}
	if ( !m_flagPotentialMode )
	{
		iInstance -= m_aInstanceMatrixs.GetLength() ;
	}
	ESLAssert( iInstance < m_aTempSourceIndexes.GetLength() ) ;
	iSubIndex = m_aTempSourceIndexes.At( iInstance ) ;
	return	m_aTempSourceItems.GetAt( iInstance ) ;
}

// インスタンス情報取得
//////////////////////////////////////////////////////////////////////////////
S3DItemInstancingSerializer::InstanceType
	S3DItemInstancingSerializer::GetInstanceInfo
		( S3DItemInstancingSerializer::InstanceInfo& info,
			const S3DCollision::MeshCollision * pMeshCol )
{
	if ( pMeshCol == nullptr )
	{
		return	typeNothing ;
	}
	S3DSceneComposer::ItemSerializer *	pHitItem =
		ESLTypeCast<S3DSceneComposer::ItemSerializer>
								( pMeshCol->pUserData ) ;
	if ( pHitItem == nullptr )
	{
		S3DPhysicsScene::Object *	pPhysObj =
			ESLTypeCast<S3DPhysicsScene::Object>( pMeshCol->pUserData ) ;
		if ( pPhysObj != nullptr )
		{
			pHitItem = ESLTypeCast<S3DSceneComposer::ItemSerializer>
										( pPhysObj->GetOwnerObject() ) ;
		}
		if ( pHitItem == nullptr )
		{
			return	typeNothing ;
		}
	}
	return	GetInstanceInfo( info, pHitItem, pMeshCol->iInstance, pMeshCol ) ;
}

S3DItemInstancingSerializer::InstanceType
	S3DItemInstancingSerializer::GetInstanceInfo
		( S3DItemInstancingSerializer::InstanceInfo& info,
			S3DSceneComposer::ItemSerializer * pItem,
			size_t iAbsInstance,
			const S3DCollision::MeshCollision * pMeshCol )
{
	info.type = typeNothing ;
	info.pMeshCol = pMeshCol ;
	info.pInstancing = NULL ;
	info.pItem = NULL ;
	info.pEntry = NULL ;
	info.iController = 0 ;
	info.iInstance = 0 ;
	info.iAbsInstance = 0 ;
	//
	S3DInstancingItemInterface *
		pInstancingItem = ESLTypeCast<S3DInstancingItemInterface>( pItem ) ;
	if ( pInstancingItem != NULL )
	{
		S3DItemInstancingSerializer *
			pInstancing = pInstancingItem->GetInstancing() ;
		while ( pInstancing != NULL )
		{
			SSmartLock<S3DItemInstancingSerializer>	lock( pInstancing ) ;
			size_t	nStaticCount = pInstancing->GetStaticInstanceCount() ;
			size_t	iOrgIndex = pInstancing->InstanceIndexOfAbsInstance( iAbsInstance ) ;
			if ( iOrgIndex == (size_t) -1 )
			{
				return	typeNothing ;
			}
			if ( iOrgIndex < nStaticCount )
			{
				//
				// 静的インスタンス
				//
				info.type = typeStaticInstance ;
				info.pInstancing = pInstancing ;
				info.pItem = pItem ;
				info.iInstance = iOrgIndex ;
				info.iAbsInstance = iAbsInstance ;
				pInstancing->GetStaticInstanceAt
					( iOrgIndex, info.mat4Instance, info.clrInstance ) ;
				return	info.type ;
			}
			S3DSceneComposer::ItemSerializer *
				pSubItem = ESLTypeCast<S3DSceneComposer::ItemSerializer>
						( pInstancing->GetSourceItemOfAbsInstance
												( iAbsInstance, iAbsInstance ) ) ;
			if ( (pSubItem == NULL) || (pSubItem == pItem) )
			{
				//
				// コントローラー検索
				//
				iOrgIndex -= nStaticCount ;
				//
				size_t	nCtrls = pItem->GetControllerCount() ;
				for ( size_t iCtrl = 0; iCtrl < nCtrls; iCtrl ++ )
				{
					S3DSceneComposer::Controller *
						pCtrl = pItem->GetControllerAt( iCtrl ) ;
					if ( (pCtrl == NULL)
						|| pCtrl->IsControllerDisabled() )
					{
						continue ;
					}
					S3DInstancingEntryInterface *	pInstanceCtrl =
						ESLTypeCast<S3DInstancingEntryInterface>( pCtrl ) ;
					if ( pInstanceCtrl == NULL )
					{
						continue ;
					}
					const S4DMatrix *	pMatrixs ;
					const S3DColor	*	pColors ;
					size_t	nCount =
						pInstanceCtrl->GetInstancingArray( pMatrixs, pColors ) ;
					if ( iOrgIndex < nCount )
					{
						//
						// 動的インスタンス
						//
						info.type = typeController ;
						info.pInstancing = pInstancing ;
						info.pItem = pItem ;
						info.pEntry = pInstanceCtrl ;
						info.iController = iCtrl ;
						info.iInstance = iOrgIndex ;
						info.iAbsInstance = iAbsInstance ;
						info.mat4Instance = pMatrixs[iOrgIndex] ;
						info.clrInstance = pColors[iOrgIndex] ;
						return	info.type ;
					}
					iOrgIndex -= nCount ;
				}
				return	typeNothing ;
			}
			pItem = pSubItem ;
			pInstancingItem = ESLTypeCast<S3DInstancingItemInterface>( pItem ) ;
			if ( pInstancingItem == NULL )
			{
				break ;
			}
			pInstancing = pInstancingItem->GetInstancing() ;
		}
	}
	if ( (pItem != NULL) && (pItem->GetComposition() != NULL) )
	{
		info.type = typeItem ;
		info.pItem = pItem ;
		return	info.type ;
	}
	return	typeNothing ;
}

// インスタンス挿入に対応するインスタンス指標を修正
//////////////////////////////////////////////////////////////////////////////
bool S3DItemInstancingSerializer::ModifyOnInsertInstance
	( S3DItemInstancingSerializer::InstanceInfo& info,
		S3DItemInstancingSerializer * pInstancing, size_t iInstance )
{
	ESLAssert( pInstancing != NULL ) ;
	if ( info.pInstancing == pInstancing )
	{
		if ( info.type == typeStaticInstance )
		{
			if ( info.iInstance == (size_t) -1 )
			{
				return	false ;
			}
			if ( info.iInstance >= iInstance )
			{
				info.iInstance ++ ;
			}
			info.iAbsInstance = (size_t) -1 ;
			return	true ;
		}
		else if ( info.type == typeController )
		{
			if ( info.iController == (size_t) -1 )
			{
				return	false ;
			}
			if ( info.pInstancing != NULL )
			{
				const size_t	nStaticCount = info.pInstancing->GetStaticInstanceCount() ;
				if ( iInstance < nStaticCount )
				{
					return	false ;		// 対象外
				}
				iInstance -= nStaticCount ;
			}
			ESLAssert( info.pItem != NULL ) ;
			size_t	nCtrlCount = info.pItem->GetControllerCount() ;
			info.iController = (size_t) -1 ;
			for ( size_t i = 0; i < info.iController; i ++ )
			{
				S3DSceneComposer::Controller *
						pCtrl = info.pItem->GetControllerAt( i ) ;
				if ( (pCtrl == NULL)
					|| pCtrl->IsControllerDisabled() )
				{
					continue ;
				}
				S3DInstancingEntryInterface *	pInstancing =
					ESLTypeCast<S3DInstancingEntryInterface>( pCtrl ) ;
				if ( info.pEntry == pInstancing )
				{
					info.iController = i ;
					break ;
				}
				const S4DMatrix *	pMatrixs ;
				const S3DColor *	pColors ;
				size_t	nCount =
					pInstancing->GetInstancingArray( pMatrixs, pColors ) ;
				//
				if ( iInstance < nCount )
				{
					return	false ;		// 対象外
				}
				iInstance -= nCount ;
			}
			if ( info.iInstance >= iInstance )
			{
				info.iInstance ++ ;
			}
			info.iAbsInstance = (size_t) -1 ;
			return	true ;
		}
	}
	return	false ;
}

// インスタンス削除に対応するインスタンス指標を修正
//////////////////////////////////////////////////////////////////////////////
bool S3DItemInstancingSerializer::ModifyOnRemoveInstance
	( S3DItemInstancingSerializer::InstanceInfo& info,
		S3DItemInstancingSerializer * pInstancing, size_t iInstance )
{
	ESLAssert( pInstancing != NULL ) ;
	if ( info.pInstancing == pInstancing )
	{
		if ( info.type == typeStaticInstance )
		{
			if ( info.iInstance == (size_t) -1 )
			{
				return	false ;
			}
			if ( info.iInstance > iInstance )
			{
				info.iInstance -- ;
			}
			else if ( info.iInstance == iInstance )
			{
				info.iInstance = (size_t) -1 ;
			}
			info.iAbsInstance = (size_t) -1 ;
			return	true ;
		}
		else if ( info.type == typeController )
		{
			if ( info.iController == (size_t) -1 )
			{
				return	false ;
			}
			if ( info.pInstancing != NULL )
			{
				const size_t	nStaticCount = info.pInstancing->GetStaticInstanceCount() ;
				if ( iInstance < nStaticCount )
				{
					return	false ;		// 対象外
				}
				iInstance -= nStaticCount ;
			}
			ESLAssert( info.pItem != NULL ) ;
			size_t	nCtrlCount = info.pItem->GetControllerCount() ;
			info.iController = (size_t) -1 ;
			for ( size_t i = 0; i < info.iController; i ++ )
			{
				S3DSceneComposer::Controller *
						pCtrl = info.pItem->GetControllerAt( i ) ;
				if ( (pCtrl == NULL)
					|| pCtrl->IsControllerDisabled() )
				{
					continue ;
				}
				S3DInstancingEntryInterface *	pInstancing =
					ESLTypeCast<S3DInstancingEntryInterface>( pCtrl ) ;
				if ( info.pEntry == pInstancing )
				{
					info.iController = i ;
					break ;
				}
				const S4DMatrix *	pMatrixs ;
				const S3DColor *	pColors ;
				size_t	nCount =
					pInstancing->GetInstancingArray( pMatrixs, pColors ) ;
				//
				if ( iInstance < nCount )
				{
					return	false ;		// 対象外
				}
				if ( iInstance >= nCount )
				{
					iInstance -= nCount ;
				}
				else
				{
					iInstance = 0 ;
					break ;
				}
			}
			if ( info.iInstance > iInstance )
			{
				info.iInstance -- ;
			}
			else if ( info.iInstance == iInstance )
			{
				info.iInstance = (size_t) -1 ;
			}
			info.iAbsInstance = (size_t) -1 ;
		}
	}
	return	false ;
}

// 参照先を安全に保管する
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::SaveInstanceReference
	( S3DItemInstancingSerializer::InstanceRef& ref,
		const S3DItemInstancingSerializer::InstanceInfo& info )
{
	ref.m_refInstance.SetReference( info.GetRefObject() ) ;
	ref.m_refItem.SetRef( info.pItem ) ;
}

// 異なるフレームで GetInstanceInfo した InstanceInfo を検証して現在のフレーム用に修正
// InstanceInfo は ModifyOnRemoveInstance の操作を含む
//////////////////////////////////////////////////////////////////////////////
bool S3DItemInstancingSerializer::CorrectInstanceInfo
	( S3DItemInstancingSerializer::InstanceInfo& info,
		const S3DItemInstancingSerializer::InstanceRef& ref )
{
	//
	// アイテムの有効性検証
	//
	S3DSceneComposer::ItemSerializer *	pRefItem = ref.m_refItem.Ref() ;
	if ( (ref.m_refInstance.GetReference() == nullptr)
		|| (pRefItem == nullptr)
		|| (pRefItem != info.pItem) )
	{
		return	false ;
	}
	if ( pRefItem->GetParentSpaceItem() == nullptr )
	{
		//
		// シーン外アイテムか？
		//
		S3DSceneComposer::Composition *	pComp =
			ESLTypeCast<S3DSceneComposer::Composition>( pRefItem ) ;
		if ( pComp == nullptr )
		{
			return	false ;
		}
		S3DSubCompositionSerializer *	pSubComp =
			ESLTypeCast<S3DSubCompositionSerializer>( pComp->GetOwnerItem() ) ;
		if ( pSubComp == nullptr )
		{
			return	false ;
		}
		if ( !pSubComp->IsValidInstance( pComp )
			&& (pSubComp->GetSubComposition() != pComp) )
		{
			return	false ;
		}
	}
	//
	// インスタンス指標正常化
	//
	if ( info.type == typeStaticInstance )
	{
		ESLAssert( info.pInstancing != nullptr ) ;
		if ( info.iInstance == (size_t) -1 )
		{
			return	false ;
		}
		info.pMeshCol = nullptr ;
		info.iAbsInstance = (size_t) -1 ;
		//
		RefIndex *	pRefIndex = ref.m_refInstance.GetRef<RefIndex>() ;
		if ( pRefIndex != nullptr )
		{
			info.iInstance = pRefIndex->m_iInstanceIndex ;
			//info.iAbsInstance =
			//	info.pInstancing->AbsInstanceOfInstanceIndex( info.iInstance ) ;
			//
			info.pInstancing->GetStaticInstanceAt
				( info.iInstance, info.mat4Instance, info.clrInstance ) ;
		}
		return	true ;
	}
	else if ( info.type == typeController )
	{
		ESLAssert( info.pInstancing != nullptr ) ;
		if ( (info.iController == (size_t) -1)
			|| (info.iInstance == (size_t) -1) )
		{
			return	false ;
		}
		const size_t	nCtrls = pRefItem->GetControllerCount() ;
		bool			flagValid = false ;
		for ( size_t iCtrl = 0; iCtrl < nCtrls; iCtrl ++ )
		{
			S3DSceneComposer::Controller *
				pCtrl = pRefItem->GetControllerAt( iCtrl ) ;
			if ( pCtrl == nullptr )
			{
				continue ;
			}
			S3DInstancingEntryInterface *	pInstanceCtrl =
				ESLTypeCast<S3DInstancingEntryInterface>( pCtrl ) ;
			if ( pInstanceCtrl == nullptr )
			{
				continue ;
			}
			if ( pInstanceCtrl == info.pEntry )
			{
				flagValid = true ;
				info.iController = iCtrl ;
				break ;
			}
		}
		if ( !flagValid )
		{
			return	false ;
		}
		info.pMeshCol = nullptr ;
		info.iAbsInstance = (size_t) -1 ;
		//
		RefIndex *	pRefIndex = ref.m_refInstance.GetRef<RefIndex>() ;
		if ( pRefIndex != nullptr )
		{
			info.iInstance = pRefIndex->m_iInstanceIndex ;
			//info.iAbsInstance =
			//	info.pInstancing->AbsInstanceOfInstanceIndex( info.iInstance ) ;
			//
			const S4DMatrix *	pMatrixs ;
			const S3DColor	*	pColors ;
			size_t	nCount =
				info.pEntry->GetInstancingArray( pMatrixs, pColors ) ;
			if ( info.iInstance < nCount )
			{
				info.mat4Instance = pMatrixs[info.iInstance] ;
				info.clrInstance = pColors[info.iInstance] ;
			}
		}
		return	true ;
	}
	else
	{
		info.pMeshCol = nullptr ;
	}
	return	true ;
}

// 異なるフレームで GetInstanceInfo した InstanceInfo を検証して
// 安全に現在のフレームでのインスタンス行列を取得
//////////////////////////////////////////////////////////////////////////////
bool S3DItemInstancingSerializer::GetSafeInstanceMatrixOf
	( S4DMatrix& matrix, S3DColor& color,
		const S3DItemInstancingSerializer::InstanceInfo& info,
		const S3DItemInstancingSerializer::InstanceRef& ref )
{
	//
	// アイテムの有効性検証
	//
	S3DSceneComposer::ItemSerializer *	pRefItem = ref.m_refItem.Ref() ;
	if ( (ref.m_refInstance.GetReference() == nullptr)
		|| (pRefItem == nullptr)
		|| (pRefItem != info.pItem) )
	{
		return	false ;
	}
	RefIndex *	pRefIndex = ref.m_refInstance.GetRef<RefIndex>() ;
	if ( pRefIndex != nullptr )
	{
		ESLAssert( info.pInstancing != nullptr ) ;
		return	info.pInstancing->GetDynamicInstanceAt
						( pRefIndex->m_iInstanceIndex, matrix, color ) ;
	}
	if ( info.type == typeController )
	{
		S3DInstancingEntryInterface *	pInstanceCtrl =
			ref.m_refInstance.GetRef<S3DInstancingEntryInterface>() ;
		if ( pInstanceCtrl != nullptr )
		{
			const S4DMatrix *	pMatrixs = nullptr ;
			const S3DColor *	pColors = nullptr ;
			const size_t		nCount =
				pInstanceCtrl->GetInstancingArray( pMatrixs, pColors ) ;
			if ( nCount >= 0 )
			{
				matrix = pMatrixs[0] ;
				color = pColors[0] ;
				return	true ;
			}
		}
		return	false ;
	}
	if ( info.type == typeItem )
	{
		matrix = S4DMatrix( 1, 1, 1, 1 ) ;
		color = S3DColor( 0xFFFFFFFF, 0 ) ;
		return	true ;
	}
	return	false ;
}

// 異なるフレームで GetInstanceInfo した InstanceInfo を検証して
// 安全に現在のフレームでのインスタンス行列（グローバル）を取得
//////////////////////////////////////////////////////////////////////////////
bool S3DItemInstancingSerializer::GetSafeInstanceGlobalMatrixOf
	( S3DDMatrix& matrix, S3DDVector& pos, S3DColor& color,
		const InstanceInfo& info, const InstanceRef& ref )
{
	S4DMatrix	mat4Instance ;
	S3DColor	clrInstance ;
	if ( !GetSafeInstanceMatrixOf( mat4Instance, clrInstance, info, ref ) )
	{
		return	false ;
	}
	S3DSceneComposer::ItemSerializer *	pRefItem = ref.m_refItem.Ref() ;
	if ( pRefItem == nullptr )
	{
		return	false ;
	}
	S3DDMatrix	matItem ;
	S3DDVector	vItem ;
	S3DColor	clrItem ;
	pRefItem->GetGlobalTransformation( matItem, vItem ) ;
	pRefItem->GetGlobalColorEffect( clrItem ) ;
	//
	matrix = matItem * S3DDMatrix(mat4Instance.GetMatrix3()) ;
	pos = matItem * S3DDVector(mat4Instance.GetTranslation()) + vItem ;
	color = clrItem * clrInstance ;
	return	true ;
}

// アイテムに対するインスタンス指標を取得
//（コントローラーの場合、コントローラー内のインスタンス番号になるため）
//////////////////////////////////////////////////////////////////////////////
size_t S3DItemInstancingSerializer::GetInstanceIndexOfItem
	( const S3DItemInstancingSerializer::InstanceInfo& info, size_t	iInstance )
{
//	size_t	iInstance = info.iInstance ;
	if ( info.type == S3DItemInstancingSerializer::typeController )
	{
		if ( info.pInstancing != NULL )
		{
			iInstance += info.pInstancing->GetStaticInstanceCount() ;
		}
		for ( size_t i = 0; i < info.iController; i ++ )
		{
			S3DSceneComposer::Controller *
					pCtrl = info.pItem->GetControllerAt( i ) ;
			if ( (pCtrl == NULL)
				|| pCtrl->IsControllerDisabled() )
			{
				continue ;
			}
			S3DInstancingEntryInterface *	pInstancing =
				ESLTypeCast<S3DInstancingEntryInterface>( pCtrl ) ;
			if ( info.pEntry == pInstancing )
			{
				break ;
			}
			if ( pInstancing != NULL )
			{
				const S4DMatrix *	pMatrixs ;
				const S3DColor *	pColors ;
				size_t	nCount =
					pInstancing->GetInstancingArray( pMatrixs, pColors ) ;
				//
				iInstance += nCount ;
			}
		}
	}
	else if ( (info.type == S3DItemInstancingSerializer::typeItem)
			|| (info.type == S3DItemInstancingSerializer::typeNothing) )
	{
		iInstance = 0 ;
	}
	return	iInstance ;
}

// インスタンス空間取得
//////////////////////////////////////////////////////////////////////////////
bool S3DItemInstancingSerializer::GetInstanceSpace
	( S3DDMatrix& matrix, S3DDVector& vPos,
		const S3DItemInstancingSerializer::InstanceInfo& info )
{
	if ( info.type == typeNothing )
	{
		return	false ;
	}
	ESLAssert( info.pItem != NULL ) ;
	if ( info.pItem == NULL )
	{
		return	false ;
	}
	info.pItem->GetGlobalTransformation( matrix, vPos ) ;
	//
	if ( (info.type == typeController)
		|| (info.type == typeStaticInstance) )
	{
		vPos += matrix * S3DDVector( info.mat4Instance.GetTranslation() ) ;
		matrix *= S3DDMatrix( info.mat4Instance.GetMatrix3() ) ;
	}
	//
	return	true ;
}

// カリング判定準備
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::PrepareCullingInfo
		( S3DItemInstancingSerializer::FrustumInfo& fi,
			float32_t sinPlayOfHAngle, float32_t sinPlayOfVAngle ) const
{
	if ( m_culling == cullingByFrustum )
	{
		float32_t	z = fi.vScreen.z * (float32_t) fi.zScale ;
		fi.vFrame[0] =
			S3DVector( 0, - z, (fi.rectView.y - fi.vScreen.y
										- sinPlayOfVAngle * z) ) ;
		fi.vFrame[1] =
			S3DVector( 0, z, - (fi.rectView.y + fi.rectView.h - fi.vScreen.y
										+ sinPlayOfVAngle * z) ) ;
		fi.vFrame[2] =
			S3DVector( - z, 0, (fi.rectView.x - fi.vScreen.x
										- sinPlayOfHAngle * z) * fi.fpPixelAspect ) ;
		fi.vFrame[3] =
			S3DVector( z, 0, - (fi.rectView.x + fi.rectView.w - fi.vScreen.x
										+ sinPlayOfHAngle * z) * fi.fpPixelAspect ) ;
		//
		fi.vFrame[0].Normalize() ;
		fi.vFrame[1].Normalize() ;
		fi.vFrame[2].Normalize() ;
		fi.vFrame[3].Normalize() ;
	}
}

// カリング判定
//////////////////////////////////////////////////////////////////////////////
bool S3DItemInstancingSerializer::IsCullingInstance
	( const S3DItemInstancingSerializer::FrustumInfo& fi,
		/*const S3DMatrix& mat3,*/ const S3DVector& pos, float32_t fpInstanceSize ) const
{
	if ( (pos.z < m_zNear - fpInstanceSize)
		|| (pos.z > m_zFar + fpInstanceSize) )
	{
		return	true ;
	}
	if ( m_culling == cullingByFrustum )
	{
		if ( fi.vScreen.z != 0.0f )
		{
			for ( int i = 0; i < 4; i ++ )
			{
				if ( fi.vFrame[i].InnerProduct( pos ) > fpInstanceSize )
				{
					// 画面外
					return	true ;
				}
			}
		}
		else
		{
			double	xProj = pos.x * fi.zScale / fi.fpPixelAspect + fi.vScreen.x ;
			double	yProj = pos.y * fi.zScale + fi.vScreen.y ;
			double	wRadius = fpInstanceSize * fi.zScale / fi.fpPixelAspect ;
			double	hRadius = fpInstanceSize * fi.zScale ;
			if ( (xProj + wRadius < fi.rectView.x)
				|| (fi.rectView.x + fi.rectView.w < xProj - wRadius)
				|| (yProj + hRadius < fi.rectView.y)
				|| (fi.rectView.y + fi.rectView.h < yProj - hRadius) )
			{
				// 画面外
				return	true ;
			}
		}
	}
	return	false ;
}

// 面の方向を強制する
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::ForceInstanceFaceDirection
	( S4DMatrix& mat4, const S3DMatrix& mat3Face /*, const S3DVector& vCameraDir*/ ) const
{
	float32_t	zz =
		(float32_t) sqrt( mat4.m[0][2] * mat4.m[0][2]
						+ mat4.m[1][2] * mat4.m[1][2]
						+ mat4.m[2][2] * mat4.m[2][2] ) ;
	float32_t	zx =
		(float32_t) sqrt( mat4.m[0][0] * mat4.m[0][0]
						+ mat4.m[1][0] * mat4.m[1][0]
						+ mat4.m[2][0] * mat4.m[2][0] ) ;
	float32_t	zy =
		(float32_t) sqrt( mat4.m[0][1] * mat4.m[0][1]
						+ mat4.m[1][1] * mat4.m[1][1]
						+ mat4.m[2][1] * mat4.m[2][1] ) ;
/*
	float32_t	a = mat4.m[0][2] * vCameraDir.x
					+ mat4.m[1][2] * vCameraDir.y
					+ mat4.m[2][2] * vCameraDir.z ;
	if ( a < 0.0f )
	{
		zx = - zx ;
		zy = - zy ;
		zz = - zz ;
	}
*/
	//
	mat4.m[0][0] = mat3Face.m[0][0] * zx ;
	mat4.m[0][1] = mat3Face.m[0][1] * zy ;
	mat4.m[0][2] = mat3Face.m[0][2] * zy ;
	mat4.m[1][0] = mat3Face.m[1][0] * zx ;
	mat4.m[1][1] = mat3Face.m[1][1] * zy ;
	mat4.m[1][2] = mat3Face.m[1][2] * zy ;
	mat4.m[2][0] = mat3Face.m[2][0] * zx ;
	mat4.m[2][1] = mat3Face.m[2][1] * zy ;
	mat4.m[2][2] = mat3Face.m[2][2] * zy ;
}

// ソート実行
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::DoSortIndex
	( S3DItemInstancingSerializer::SortIndex * psi, size_t nCount )
{
	if ( nCount < 2 )
	{
		return ;
	}
	for ( size_t i = 0; i < nCount - 1; i ++ )
	{
		SortIndex	si = psi[i] ;
		float32_t	zMax = si.z ;
		size_t		iMax = i ;
		for ( size_t j = i + 1; j < nCount; j ++ )
		{
			if ( zMax < psi[j].z )
			{
				zMax = psi[j].z ;
				iMax = j ;
			}
		}
		psi[i] = psi[iMax] ;
		psi[iMax] = si ;
	}
}

// インスタンシング・リスト取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DItemInstancingSerializer::GetStaticInstancingArray
	( const S4DMatrix*& pMatrixs, const S3DColor*& pColors ) const
{
	pMatrixs = m_aInstanceMatrixs.GetConstArray() ;
	pColors = m_aInstanceColors.GetConstArray() ;
	if ( m_flagPotentialMode )
	{
		return	0 ;
	}
	return	m_aInstanceMatrixs.GetLength() ;
}

size_t S3DItemInstancingSerializer::GetDynamicInstancingArray
	( const S4DMatrix*& pMatrixs, const S3DColor*& pColors ) const
{
	pMatrixs = m_aTempMatrixs.GetConstArray() ;
	pColors = m_aTempColors.GetConstArray() ;
	return	m_aTempMatrixs.GetLength() ;
}

size_t S3DItemInstancingSerializer::GetProcessedInstancingArray
	( const S4DMatrix*& pMatrixs, const S3DColor*& pColors ) const
{
	pMatrixs = m_aDynamicMatrixs.GetConstArray() ;
	pColors = m_aDynamicColors.GetConstArray() ;
	return	m_aDynamicMatrixs.GetLength() ;
}

// ソート情報取得
//////////////////////////////////////////////////////////////////////////////
const S3DItemInstancingSerializer::SortIndex *
	S3DItemInstancingSerializer::GetProcessedInstanceSortIndexArray
			( size_t& nTotalCount, size_t& nOpaqueCount ) const
{
	nTotalCount = m_aDynamicSort.GetLength() ;
	nOpaqueCount = m_nOpaqueCount ;
	return	m_aDynamicSort.GetConstArray() ;
}

// レンダリング
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::RenderMultiInstance
	( const S3DScene& scene,
		S3DRenderContextInterface& render,
		S3DSceneComposer::ItemSerializer& item,
		S3DVertexBufferInterface *	pModel,
		RenderingType type,
		uint64_t flagsExclusion,
		size_t iFirst, ssize_t iEnd, size_t nInstancing,
		const S4DMatrix * pmatInstancing,
		const S3DColor * pColorInstancing,
		const SortIndex * pSortIndexes, size_t nOpaqueCount ) const
{
	InstanceRange	ranges[16] ;
	RenderInfo		info ;
	info.pModel = pModel ;
	info.flagsExclusion = flagsExclusion ;
	info.iMeshFirst = iFirst ;
	info.iMeshEnd = iEnd ;
	info.nInstanceCount = nInstancing ;
	info.pInstanceMatrices = pmatInstancing ;
	info.pInstanceColors = pColorInstancing ;
	info.pInstanceSortIndexes = pSortIndexes ;
	info.nInstanceOpaqueCount = nOpaqueCount ;
	info.nRangeCount = 1 ;
	info.nRangeBufSize = 16 ;
	info.pInstanceRanges = ranges ;
	ranges[0].nIndex = 0 ;
	ranges[0].nCount = nInstancing ;
	//
	const size_t		nCtrls = item.GetControllerCount() ;
	for ( size_t i = 0; i < nCtrls; i ++ )
	{
		S3DSceneComposer::Controller *	pCtrl = item.GetControllerAt( i ) ;
		if ( (pCtrl == nullptr)
			|| pCtrl->IsControllerDisabled() )
		{
			continue ;
		}
		MultiRenderer *	pRenderer = ESLTypeCast<MultiRenderer>( pCtrl ) ;
		if ( pRenderer == nullptr )
		{
			continue ;
		}
		RenderResult	rr =
			pRenderer->RenderMultiInstance
				( scene, render, *this, type, info ) ;
		if ( rr == renderFinished )
		{
			return ;
		}
	}
	if ( info.pModel != nullptr )
	{
		for ( size_t i = 0; i < info.nRangeCount; i ++ )
		{
			InstanceRange	range = info.pInstanceRanges[i] ;
			if ( (range.nCount > 0)
				|| (info.pModel->GetInstancingCount() > 0) )
			{
				ESLAssert( (range.nIndex < nInstancing) || (range.nIndex == 0) ) ;
				ESLAssert( (info.pInstanceMatrices != nullptr) || (range.nIndex == 0) ) ;
				ESLAssert( (info.pInstanceColors != nullptr) || (range.nIndex == 0) ) ;
				info.pModel->RenderBufferTo
					( &render, info.flagsExclusion,
						info.iMeshFirst, info.iMeshEnd,
						range.nCount,
						info.pInstanceMatrices + range.nIndex,
						info.pInstanceColors + range.nIndex ) ;
			}
		}
	}
}

// スレッド排他処理
//////////////////////////////////////////////////////////////////////////////
void S3DItemInstancingSerializer::Lock( void ) const
{
	m_csLock.Lock() ;
}

void S3DItemInstancingSerializer::Unlock( void ) const
{
	m_csLock.Unlock() ;
}

atomic_int_t S3DItemInstancingSerializer::UnlockAll( void ) const
{
	return	m_csLock.UnlockAll() ;
}

void S3DItemInstancingSerializer::Relock( atomic_int_t nLock ) const
{
	m_csLock.Relock( nLock ) ;
}



//////////////////////////////////////////////////////////////////////////////
// アイテム／インスタンス参照オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DItemInstanceRef, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DItemInstanceRef::S3DItemInstanceRef( void )
{
}

S3DItemInstanceRef::S3DItemInstanceRef( const S3DItemInstanceRef& ref )
	: InstanceRef( ref ),
		m_info( ref.m_info )
{
}

S3DItemInstanceRef::S3DItemInstanceRef
	( const S3DCollision::MeshCollision * pMeshCol )
{
	if ( S3DItemInstancingSerializer::GetInstanceInfo( m_info, pMeshCol )
								!= S3DItemInstancingSerializer::typeNothing )
	{
		S3DItemInstancingSerializer::SaveInstanceReference( *this, m_info ) ;
	}
}

S3DItemInstanceRef::S3DItemInstanceRef
	(  S3DSceneComposer::ItemSerializer * pItem, size_t iAbsInstance )
{
	if ( S3DItemInstancingSerializer::GetInstanceInfo
					( m_info, pItem, iAbsInstance, nullptr )
							!= S3DItemInstancingSerializer::typeNothing )
	{
		S3DItemInstancingSerializer::SaveInstanceReference( *this, m_info ) ;
	}
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const S3DItemInstanceRef& S3DItemInstanceRef::operator = ( const S3DItemInstanceRef& ref )
{
	InstanceRef::operator = ( ref ) ;
	m_info = ref.m_info ;
	return	*this ;
}

// 異なるフレームで InstanceInfo を現在のフレーム用に修正する
//////////////////////////////////////////////////////////////////////////////
bool S3DItemInstanceRef::CorrectInstance( void )
{
	if ( S3DItemInstancingSerializer::CorrectInstanceInfo( m_info, *this ) )
	{
		return	true ;
	}
	ReleaseRef() ;
	return	false ;
}

// 安全に現在のフレームでのインスタンス行列（ローカル）を取得
//////////////////////////////////////////////////////////////////////////////
bool S3DItemInstanceRef::GetInstanceMatrix( S4DMatrix& matrix, S3DColor& color )
{
	return	S3DItemInstancingSerializer::GetSafeInstanceMatrixOf
									( matrix, color, m_info, *this ) ;
}

// インスタンスを削除する
//////////////////////////////////////////////////////////////////////////////
bool S3DItemInstanceRef::DeleteInstance( void )
{
	if ( IsEmpty() )
	{
		return	false ;
	}
	S3DSceneComposer::ItemSerializer *			pRefItem = m_refItem.Ref() ;
	S3DSceneComposer::ItemCommonSerializer *	pCmnItem =
		ESLTypeCast<S3DSceneComposer::ItemCommonSerializer>( pRefItem ) ;
	if ( pCmnItem != nullptr )
	{
		S3DScene::Item *	pItem = pCmnItem->GetSceneItem() ;
		if ( (pItem != nullptr)
			&& (pItem->m_classItem < S3DScene::classDynamicItem1) )
		{
			S3DScene *	pScene = pItem->GetScene() ;
			if ( pScene != nullptr )
			{
				// static なアイテムを削除する場合には当たり判定バッファを更新する
				pScene->SetUpdateFieldCollisionFlag() ;
			}
		}
	}
	if ( m_info.pMeshCol != nullptr )
	{
		// 現在のフレームでの当たり判定無効化
		const_cast<S3DCollision::MeshCollision*>( m_info.pMeshCol )->maskClasses = 0 ;
	}
	if ( m_info.type == S3DItemInstancingSerializer::typeStaticInstance )
	{
		// 静的インスタンス
		ESLAssert( m_info.pInstancing != nullptr ) ;
		ESLAssert( m_info.pItem != nullptr ) ;
		if ( m_info.pInstancing != nullptr )
		{
			m_info.pInstancing->Lock() ;
			//
			ssize_t	iInstance = GetInstanceIndex() ;
			if ( iInstance >= 0 )
			{
				ESLAssert( (size_t) iInstance < m_info.pInstancing->GetStaticInstanceCount() ) ;
				m_info.pInstancing->RemoveStaticInstanceAt( (size_t) iInstance, m_info.pItem ) ;
			}
			//
			m_info.pInstancing->Unlock() ;
			ReleaseRef() ;
			return	true ;
		}
	}
	else if ( m_info.type == S3DItemInstancingSerializer::typeController )
	{
		// 動的インスタンス（コントローラー）
		ESLAssert( m_info.pInstancing != nullptr ) ;
		ESLAssert( m_info.pItem != nullptr ) ;
		if ( (m_info.pInstancing != nullptr) && (m_info.pItem != nullptr) )
		{
			m_info.pInstancing->Lock() ;
			//
			S3DInstancingEntryEditInterface *	pInstancing =
				ESLTypeCast<S3DInstancingEntryEditInterface>
							( m_info.pItem->GetControllerAt( m_info.iController ) ) ;
			if ( pInstancing != nullptr )
			{
				pInstancing->RemoveInstancingEntryAt( m_info.iInstance ) ;
			}
			else
			{
				m_info.pItem->RemoveControllerAt( m_info.iController ) ;
			}
			size_t	iInstance =
				S3DItemInstancingSerializer::GetInstanceIndexOfItem
											( m_info, m_info.iInstance ) ;
			m_info.pInstancing->DeleteIndexByInstanceIndex( iInstance ) ;
			m_info.pInstancing->NotifyOnRemovedInstance( iInstance, m_info.pItem ) ;
			//
			m_info.pInstancing->Unlock() ;
			ReleaseRef() ;
			return	true ;
		}
	}
	else if ( m_info.type == S3DItemInstancingSerializer::typeItem )
	{
		S3DSceneComposer::Composition *
				pComp = m_info.pItem->GetComposition() ;
		if ( pComp != nullptr )
		{
			S3DSubCompositionSerializer *	pSubComp =
				ESLTypeCast<S3DSubCompositionSerializer>( pComp->GetOwnerItem() ) ;
			if ( pSubComp != nullptr )
			{
				ESLAssert( pSubComp->IsValidInstance( pComp ) ) ;
				pSubComp->DelayReleaseInstance( pComp ) ;
				ReleaseRef() ;
				return	true ;
			}
			else
			{
				S3DSceneComposer::SpaceSerializer *	pParent =
						ESLTypeCast<S3DSceneComposer::SpaceSerializer>
									( m_info.pItem->GetParentSpaceItem() ) ;
				if ( pParent != nullptr )
				{
					pComp->PostDelayRemoveItem( *pParent, m_info.pItem ) ;
					ReleaseRef() ;
					return	true ;
				}
			}
		}
	}
	return	false ;
}




//////////////////////////////////////////////////////////////////////////////
// インスタンシング・抽象アイテム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DInstancingItemInterface, SObject )


//////////////////////////////////////////////////////////////////////////////
// マルチインスタンスアイテム
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DMultiInstanceSerializer::m_paramEntries
		[S3DMultiInstanceSerializer::paramMultiInstanceCount] =
{
	{ L"instancing_target",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration,
		L"描画ターゲット", NULL },
	{ L"instancing",
		S3DSceneComposer::typeBinary,
		S3DSceneComposer::attrConstant1, L"インスタンス", NULL },
	{ L"instance_rotate",
		S3DSceneComposer::typeRotation,
		S3DSceneComposer::attrConstant1, L"回転", NULL },
	{ L"instance_zoom",
		S3DSceneComposer::typeZoom,
		S3DSceneComposer::attrConstant1, L"拡大", NULL },
} ;

const S3DSceneComposer::ParamSetClass
	S3DMultiInstanceSerializer::m_pscClass =
{
	&ItemBasicSerializer::m_pscClass,
	paramMultiInstanceCount,
	&S3DMultiInstanceSerializer::m_paramEntries[0]
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DPotentialInstancingInterface, ESLObject ) ;
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DMultiInstanceSerializer,
			ItemBasicSerializer, S3DInstancingItemInterface )
S3D_IMPLEMENT_COMPOSER_ITEM( S3DMultiInstanceSerializer, multi_instance )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DMultiInstanceSerializer::S3DMultiInstanceSerializer( void )
	: ItemBasicSerializer( m_ItemClassDescriptor.pwszClassID, &m_pscClass )
{
	m_flagsBehavior |= S3DScene::itemOwnerBehavior ;
	m_maskClasses |= (1 << S3DScene::classPreRender) ;
}

// ターゲット設定
//////////////////////////////////////////////////////////////////////////////
void S3DMultiInstanceSerializer::AttachInstancingTarget
	( S3DInstancingItemInterface * pTarget, const wchar_t * pwszID )
{
	m_refTargetItem.SetReference( pTarget ) ;
	m_strTargetID = pwszID ;
}

bool S3DMultiInstanceSerializer::UpdateInstancingTarget( void )
{
	if ( m_strTargetID.IsEmpty() )
	{
		m_refTargetItem.SetReference( NULL ) ;
		return	true ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != NULL )
	{
		S3DSceneComposer::ItemSerializer *
				pItem = pComp->GetSceneItemAs( m_strTargetID ) ;
		m_refTargetItem.SetReference( pItem ) ;
		return	(pItem != NULL) ;
	}
	return	false ;
}

// ターゲット取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ItemSerializer *
	S3DMultiInstanceSerializer::GetInstancingTarget( void ) const
{
	return	ESLTypeCast<S3DSceneComposer::ItemSerializer>
							( m_refTargetItem.GetReference() ) ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DMultiInstanceSerializer::GetMatrixParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramInstanceRotate:
		return	S3DDMatrix( m_instancing.GetBaseRotation() ) ;
	}
	return	ItemBasicSerializer::GetMatrixParameter( i ) ;
}

S3DDVector S3DMultiInstanceSerializer::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramInstanceZoom:
		return	m_instancing.GetBaseZoom() ;
	}
	return	ItemBasicSerializer::GetVectorParameter( i ) ;
}

const wchar_t * S3DMultiInstanceSerializer::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramInstanceTarget:
		return	m_strTargetID ;
	case	paramInstancing:
		return	m_instancing.GetInstancingEntriesBase64() ;
	}
	return	ItemBasicSerializer::GetCommandParameter( i ) ;
}

size_t S3DMultiInstanceSerializer::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t i ) const
{
	switch ( i )
	{
	case	paramInstancing:
		if ( pDst == NULL )
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
	return	ItemBasicSerializer::GetBinaryParameter( pDst, nBufBytes, i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DMultiInstanceSerializer::SetMatrixParameter( size_t i, const S3DDMatrix& mat )
{
	switch ( i )
	{
	case	paramInstanceRotate:
		m_instancing.SetBaseRotation( S3DDQuaternion( mat ) ) ;
		return ;
	}
	ItemBasicSerializer::SetMatrixParameter( i, mat ) ;
}

void S3DMultiInstanceSerializer::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramInstanceZoom:
		m_instancing.SetBaseZoom( vec ) ;
		return ;
	}
	ItemBasicSerializer::SetVectorParameter( i, vec ) ;
}

void S3DMultiInstanceSerializer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramInstanceTarget:
		if ( m_strTargetID != pwszCmd )
		{
			m_strTargetID = pwszCmd ;
			UpdateInstancingTarget() ;
		}
		return ;
	case	paramInstancing:
		m_instancing.SetInstancingEntriesBase64( pwszCmd ) ;
		return ;
	}
	ItemBasicSerializer::SetCommandParameter( i, pwszCmd ) ;
}

size_t S3DMultiInstanceSerializer::SetBinaryParameter
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
	return	ItemBasicSerializer::SetBinaryParameter( i, pSrc, nBufBytes ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DMultiInstanceSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	S3DSceneComposer::Composition *	pComp ;
	switch ( i )
	{
	case	paramInstanceTarget:
		pComp = GetComposition() ;
		if ( pComp != NULL )
		{
			pComp->EnumerateItemIDsAs
				( aStrSet, ESL_RUNTIME_CLASS(S3DInstancingItemInterface) ) ;
			pComp->EnumerateItemIDsAs
				( aStrSet, ESL_RUNTIME_CLASS(S3DParticleSerializer::RenderTarget) ) ;
		}
		return	true ;
	}
	return	ItemBasicSerializer::EnumerateStringSet( i, aStrSet ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DMultiInstanceSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	1:
		return	L"複数描画" ;
	}
	return	ItemBasicSerializer::GetParameterCategoryName( iCategory ) ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DMultiInstanceSerializer::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
//	case	paramTransparency:
//	case	paramColorMul:
//	case	paramColorAdd:
	case	paramForceToon:
	case	paramForceBorder:
	case	paramFreeToon:
	case	paramFreeBorder:
	case	paramUseCollision:
	case	paramHideNear:
	case	paramHideFar:
	case	paramItemClass:
		return	false ;
	}
	return	ItemBasicSerializer::IsParameterValidation( i ) ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DMultiInstanceSerializer::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp, uint32_t nFlags )
{
	uint32_t	nResFlags =
		ItemBasicSerializer::UpdatePropertyReference( comp, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefItem )
	{
		if ( !UpdateInstancingTarget() )
		{
			nResFlags |= S3DSceneComposer::updateRefItem ;
		}
	}
	return	nResFlags ;
}

// アイテム作用の追加処理
//////////////////////////////////////////////////////////////////////////////
void S3DMultiInstanceSerializer::OnUpdateBehavior( S3DScene& scene )
{
	ItemBasicSerializer::OnUpdateBehavior( scene ) ;
	//
	m_instancing.Lock() ;
	m_instancing.ResetDynamicInstancingEntries() ;
	m_instancing.UpdateDynamicInstancingEntries( this ) ;
	m_instancing.Unlock() ;
}

// レンダリング前後処理（全視点共通）
//////////////////////////////////////////////////////////////////////////////
void S3DMultiInstanceSerializer::OnRenderEvent
	( S3DScene& scene, S3DScene::ItemClass clsItem )
{
	ItemBasicSerializer::OnRenderEvent( scene, clsItem ) ;
	//
	if ( (clsItem == S3DScene::classPreRender)
		&& (m_flagsBehavior & S3DScene::itemVisible) )
	{
		S3DSceneComposer::ItemSerializer *	pItem =
				ESLTypeCast<S3DSceneComposer::ItemSerializer>
								( m_refTargetItem.GetReference() ) ;
		S3DInstancingItemInterface *	pInstanceTarget =
					ESLTypeCast<S3DInstancingItemInterface>
								( m_refTargetItem.GetReference() ) ;
		S3DParticleSerializer::RenderTarget *	pRenderTarget =
					ESLTypeCast<S3DParticleSerializer::RenderTarget>
								( m_refTargetItem.GetReference() ) ;
		if ( (pItem != NULL) && (pItem != this)
			&& ((pInstanceTarget != NULL) || (pRenderTarget != NULL)) )
		{
			m_instancing.ResetInstanceIndex() ;
			UpdateInstancingArray() ;
			//
			if ( m_bufMatrix.GetLength() > 0 )
			{
				const S4DMatrix*	pMatrixs = m_bufMatrix.GetConstArray() ;
				const S3DColor*		pColors = m_bufColor.GetConstArray() ;
				size_t				nInstanceCount = m_bufMatrix.GetLength() ;
				//
				if ( pInstanceTarget != NULL )
				{
					pInstanceTarget->AddDynamicInstancingEntries
						( TransformInstanceMatrics
								( pItem, pMatrixs, nInstanceCount ),
							EffectInstanceColors( pColors, nInstanceCount ),
							nInstanceCount, (ParameterProperty*) this ) ;
				}
				else if ( pRenderTarget != NULL )
				{
					AddParticleInstances
						( pRenderTarget, pMatrixs, pColors, nInstanceCount ) ;
				}
			}
		}
	}
}

// 動的インスタンス追加（classPreRender で呼び出す／但しこのアイテムより先）
//////////////////////////////////////////////////////////////////////////////
void S3DMultiInstanceSerializer::AddDynamicInstancingEntries
	( const S4DMatrix * pMatrixs,
			const S3DColor * pColors,
			size_t nCount, ESLObject * pSourceItem )
{
	m_instancing.Lock() ;
	m_instancing.AddDynamicInstancingEntries
		( pMatrixs, pColors, nCount, pSourceItem ) ;
	m_instancing.Unlock() ;
}

// S3DItemInstancingSerializer 取得
//////////////////////////////////////////////////////////////////////////////
S3DItemInstancingSerializer * S3DMultiInstanceSerializer::GetInstancing( void )
{
	return	&m_instancing ;
}

// フレーム更新後処理
//////////////////////////////////////////////////////////////////////////////
void S3DMultiInstanceSerializer::OnUpdateFrame
		( double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	ItemBasicSerializer::OnUpdateFrame( fpFrame, seek ) ;
	//
	if ( seek == S3DSceneComposer::seekJumpReset )
	{
		m_instancing.Lock() ;
		m_instancing.ResetDynamicInstancingEntries() ;
		m_instancing.UpdateDynamicInstancingEntries( this ) ;
		//
		m_instancing.ResetInstanceIndex() ;
		UpdateInstancingArray() ;
		//
		size_t	nCtrls = GetControllerCount() ;
		for ( size_t i = 0; i < nCtrls; i ++ )
		{
			S3DPotentialInstancingInterface *	pInstancing =
				ESLTypeCast<S3DPotentialInstancingInterface>
										( GetControllerAt( i ) ) ;
			if ( pInstancing != NULL )
			{
				pInstancing->ResetInstance
					( m_bufMatrix.GetConstArray(),
						m_bufColor.GetConstArray(),
						m_bufMatrix.GetLength() ) ;
			}
		}
		m_instancing.Unlock() ;
	}
}

// Rosetta インスタンス取得
//////////////////////////////////////////////////////////////////////////////
Rosetta::RSObject *
	S3DMultiInstanceSerializer::GetRosettaInstanceOf( const wchar_t * pwszClass ) const
{
	RSObject *	pObj = ItemBasicSerializer::GetRosettaInstanceOf( pwszClass ) ;
	if ( pObj == NULL )
	{
		S3DSceneComposer::ItemSerializer *	pItem =
				ESLTypeCast<S3DSceneComposer::ItemSerializer>
								( m_refTargetItem.GetReference() ) ;
		if ( (pItem != NULL) && (pItem != this) )
		{
			pObj = pItem->GetRosettaInstanceOf( pwszClass ) ;
		}
	}
	return	pObj ;
}

// アイテムのプライマリモデル取得
//////////////////////////////////////////////////////////////////////////////
S3DVertexBufferInterface * S3DMultiInstanceSerializer::GetItemPrimaryModel( void )
{
	S3DSceneComposer::ItemSerializer *	pItem =
			ESLTypeCast<S3DSceneComposer::ItemSerializer>
							( m_refTargetItem.GetReference() ) ;
	if ( (pItem != nullptr) && (pItem != this) )
	{
		return	pItem->GetItemPrimaryModel() ;
	}
	return	ItemBasicSerializer::GetItemPrimaryModel() ;
}

// アイテムのコリジョンバッファ取得
//////////////////////////////////////////////////////////////////////////////
S3DCollider * S3DMultiInstanceSerializer::GetItemPrimaryCollider( void )
{
	S3DSceneComposer::ItemSerializer *	pItem =
			ESLTypeCast<S3DSceneComposer::ItemSerializer>
							( m_refTargetItem.GetReference() ) ;
	if ( (pItem != nullptr) && (pItem != this) )
	{
		return	pItem->GetItemPrimaryCollider() ;
	}
	return	ItemBasicSerializer::GetItemPrimaryCollider() ;
}

// スクリプト・インスタンス取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneScriptInstance *
	S3DMultiInstanceSerializer::GetScriptInstanceOf( const wchar_t * pwszClass ) const
{
	S3DSceneScriptInstance *	pInstance =
		ItemBasicSerializer::GetScriptInstanceOf( pwszClass ) ;
	if ( pInstance != nullptr )
	{
		return	pInstance ;
	}
	S3DSceneComposer::ItemSerializer *	pItem =
			ESLTypeCast<S3DSceneComposer::ItemSerializer>
							( m_refTargetItem.GetReference() ) ;
	if ( pItem != nullptr )
	{
		return	pItem->GetScriptInstanceOf( pwszClass ) ;
	}
	return	nullptr ;
}

// インスタンス収集
//////////////////////////////////////////////////////////////////////////////
void S3DMultiInstanceSerializer::UpdateInstancingArray( void )
{
	m_bufMatrix.RemoveAll() ;
	m_bufColor.RemoveAll() ;
	//
	const S4DMatrix *	pMatrixs ;
	const S3DColor *	pColors ;
	size_t				nCount ;
	nCount = m_instancing.GetStaticInstancingArray( pMatrixs, pColors ) ;
	if ( nCount != 0 )
	{
		m_bufMatrix.AddArray( pMatrixs, nCount ) ;
		m_bufColor.AddArray( pColors, nCount ) ;
	}
	nCount = m_instancing.GetDynamicInstancingArray( pMatrixs, pColors ) ;
	if ( nCount != 0 )
	{
		m_bufMatrix.AddArray( pMatrixs, nCount ) ;
		m_bufColor.AddArray( pColors, nCount ) ;
	}
}

// 座標変換
//////////////////////////////////////////////////////////////////////////////
const S4DMatrix *
	S3DMultiInstanceSerializer::TransformInstanceMatrics
		( S3DSceneComposer::ItemSerializer * pTargetItem,
				const S4DMatrix * pMatrixs, size_t nCount )
{
	S3DDMatrix	matdOffset ;
	S3DDVector	vdOffset ;
	pTargetItem->GetTransformationFrom( matdOffset, vdOffset, this ) ;
	//
	S3DMatrix	matOffset = matdOffset ;
	S3DVector	vOffset = vdOffset ;
	S4DMatrix	mat4Offset( matOffset, vOffset ) ;
	//
	S4DMatrix *	pDstMatrix = m_bufTempMatrix.GetArray( nCount ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pDstMatrix[i] = mat4Offset * pMatrixs[i] ;
	}
	m_bufTempMatrix.FinishArray() ;
	return	pDstMatrix ;
}

void S3DMultiInstanceSerializer::TransformParticleInstances
	( S3DParticleSerializer::RenderTarget * pRenderTarget,
					const S4DMatrix * pMatrixs, size_t nCount )
{
	S3DDMatrix	matdItem ;
	S3DDVector	vdItem ;
	GetGlobalTransformation( matdItem, vdItem ) ;
	//
	S3DDMatrix	matdITarget ;
	S3DDVector	vdITarget ;
	pRenderTarget->GetTargetSpaceTransformation( matdITarget, vdITarget ) ;
	//
	S3DMatrix	matITarget = matdITarget * matdItem ;
	S3DVector	vITarget = matdITarget * vdItem + vdITarget ;
	//
	bool			flagFaceDir = !pRenderTarget->IsUsingIndexedParticles() ;
	S3DMatrix *		pDstFaceMatrix = m_bufFaceMatrix.GetArray( nCount ) ;
	S3DVector4 *	pDstPoint = m_bufPoint.GetArray( nCount ) ;
	S4DVector *		pDstFaceDir = m_bufFaceDir.GetArray( nCount ) ;
	float32_t *		pDstZoom = m_bufZoom.GetArray( nCount ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		S3DMatrix	matSrc = pMatrixs[i].GetMatrix3() ;
		S3DVector	vSrc = pMatrixs[i].GetTranslation() ;
		pDstFaceMatrix[i] = matITarget * matSrc ;
		pDstPoint[i] = matITarget * vSrc + vITarget ;
		//
		S3DVector	vFaceDir = pDstFaceMatrix[i] * S3DVector( 0, 0, 1 ) ;
		float32_t	wFaceDir = 0.0f ;
		if ( flagFaceDir )
		{
			S3DMatrix	matFace( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
			matFace.RevolveForAngle( vFaceDir ) ;
			//
			S3DMatrix	matRZ = matFace.Inverse() * pDstFaceMatrix[i] ;
			wFaceDir = (float32_t) atan2( matRZ.m[1][0], matRZ.m[0][0] ) ;
			//
			pDstZoom[i] =
				(float32_t) sqrt( matRZ.m[1][0] * matRZ.m[1][0]
									+ matRZ.m[0][0] * matRZ.m[0][0] ) ;
		}
		pDstFaceDir[i] = S4DVector( vFaceDir, wFaceDir ) ;
	}
	m_bufFaceMatrix.FinishArray() ;
	m_bufPoint.FinishArray() ;
	m_bufFaceDir.FinishArray() ;
	m_bufZoom.FinishArray() ;
}

// 色効果
//////////////////////////////////////////////////////////////////////////////
const S3DColor * S3DMultiInstanceSerializer::EffectInstanceColors
	( const S3DColor * pColors, size_t nCount )
{
	S3DColor	clrEffect ;
	GetGlobalColorEffect( clrEffect ) ;

	S3DColor *	pDstColor = m_bufTempColor.GetArray( nCount ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		S3DColor	clrDst = clrEffect * pColors[i] ;
		pDstColor[i] = clrDst ;
	}
	m_bufTempColor.FinishArray() ;
	return	pDstColor ;
}

// インスタンス出力
//////////////////////////////////////////////////////////////////////////////
void S3DMultiInstanceSerializer::AddParticleInstances
	( S3DParticleSerializer::RenderTarget * pRenderTarget,
		const S4DMatrix * pMatrixs, const S3DColor * pColors, size_t nCount )
{
	TransformParticleInstances( pRenderTarget, pMatrixs, nCount ) ;
	//
	if ( pRenderTarget->IsUsingIndexedParticles() )
	{
		S3DParticleSerializer::ParticleIndex *
						pIndex = m_bufIndex.GetArray( nCount ) ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			pIndex[i].nIndex = i ;
			pIndex[i].nBlurCount = 1 ;
			pIndex[i].nIdentity = i ;
		}
		pRenderTarget->AddIndexedParticles
			( nCount, m_bufPoint.GetConstArray(),
				pIndex, NULL,
				EffectInstanceColors( pColors, nCount ),
				m_bufFaceMatrix.GetConstArray() ) ;
	}
	else
	{
		pRenderTarget->AddParticles
			( nCount, m_bufPoint.GetConstArray(), NULL,
				EffectInstanceColors( pColors, nCount ),
				m_bufZoom.GetConstArray(), m_bufFaceDir.GetConstArray() ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// インスタンス周期回転コントローラー
//////////////////////////////////////////////////////////////////////////////

const SSystem::SXMLDocument::AttrInteger
	S3DInstanceRotationController::s_aiRotationAxisType[5] =
{
	{ L"axis_x", rotationAxisX },
	{ L"axis_y", rotationAxisY },
	{ L"axis_z", rotationAxisZ },
	{ L"vector", rotationAxisVector },
	{ nullptr, 0 },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DInstanceRotationController, Controller )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DInstanceRotationController, instance_rotator )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DInstanceRotationController::S3DInstanceRotationController( void )
	: Controller( m_ItemClassDescriptor.pwszClassID ),
		m_matBaseRotation( 1, 1, 1 ),
		m_rxtType( rotationAxisY ),
		m_vRotAxis( 0, 0, 1 ),
		m_degRotation( 0.0 ),
		m_degRotCurrent( 0.0 ),
		m_secSpeedPhase( 0.0 ),
		m_dpsRotSpeed( 0.0 ),
		m_dpsRotSpeedAmp( 0.0 ),
		m_dpsRotSpeedCycle( 1.0 )
{
	m_flagsBehavior |= S3DSceneComposer::behaviorOnTimer ;

	ESLVerify( paramBaseRotation ==
		AddParameterEntry
			( L"rotation",
				S3DSceneComposer::typeRotation,
				S3DSceneComposer::attrConstant, L"ベース回転" ) ) ;
	ESLVerify( paramRotationAxisType ==
		AddParameterEntry
			( L"rot_axis_type",
				S3DSceneComposer::typeSelector,
				S3DSceneComposer::attrConstant
				| S3DSceneComposer::attrStringEnumeration
				| S3DSceneComposer::attrUIOnlyEnumeration
				| S3DSceneComposer::attrDynamicValidation, L"回転軸" ) ) ;
	ESLVerify( paramRotationAxisVector ==
		AddParameterEntry
			( L"rot_axis_vec",
				S3DSceneComposer::typeDirection,
				S3DSceneComposer::attrConstant1, L"回転ベクトル" ) ) ;
	ESLVerify( paramRotationOnAxis ==
		AddParameterEntry
			( L"rot_angle",
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrUIScalarSlider,
				L"回転角", L"回転角 [deg]", -180.0, 180.0 ) ) ;
	ESLVerify( paramRotationSpeed ==
		AddParameterEntry
			( L"rot_speed",
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrUIScalarSlider,
				L"回転速度", L"回転速度 [deg/sec]", -360, 360.0 ) ) ;
	ESLVerify( paramRotationSpeedAmp ==
		AddParameterEntry
			( L"rot_speed_amp",
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrUIScalarSlider,
				L"回転速度変分幅", L"回転速度変分幅 [deg/sec]", -360, 360.0 ) ) ;
	ESLVerify( paramRotationSpeedCycle ==
		AddParameterEntry
			( L"rot_speed_cycle",
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrUIScalarSlider,
				L"回転速度変分周期", L"回転速度変分周期 [sec]", 1.0, 10.0 ) ) ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DInstanceRotationController::GetMatrixParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramBaseRotation:
		return	m_matBaseRotation ;
	}
	return	S3DDMatrix( 1, 1, 1 ) ;
}

S3DDVector S3DInstanceRotationController::GetVectorParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramRotationAxisVector:
		return	m_vRotAxis ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DInstanceRotationController::GetScalarParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramRotationOnAxis:
		return	m_degRotation ;
	case	paramRotationSpeed:
		return	m_dpsRotSpeed ;
	case	paramRotationSpeedAmp:
		return	m_dpsRotSpeedAmp ;
	case	paramRotationSpeedCycle:
		return	m_dpsRotSpeedCycle ;
	}
	return	0.0 ;
}

const wchar_t * S3DInstanceRotationController::GetCommandParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramRotationAxisType:
		return	SXMLDocument::GetSymbolAsIntegerOf( s_aiRotationAxisType, m_rxtType ) ;
	}
	return	nullptr ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DInstanceRotationController::SetMatrixParameter( size_t iParam, const S3DDMatrix& mat )
{
	switch ( iParam )
	{
	case	paramBaseRotation:
		m_matBaseRotation = mat ;
		return ;
	}
}

void S3DInstanceRotationController::SetVectorParameter( size_t iParam, const S3DDVector& vec )
{
	switch ( iParam )
	{
	case	paramRotationAxisVector:
		m_vRotAxis = vec ;
		return ;
	}
}

void S3DInstanceRotationController::SetScalarParameter( size_t iParam, double s )
{
	switch ( iParam )
	{
	case	paramRotationOnAxis:
		m_degRotation = s ;
		return ;
	case	paramRotationSpeed:
		m_dpsRotSpeed = s ;
		return ;
	case	paramRotationSpeedAmp:
		m_dpsRotSpeedAmp = s ;
		return ;
	case	paramRotationSpeedCycle:
		m_dpsRotSpeedCycle = s ;
		return ;
	}
}

void S3DInstanceRotationController::SetCommandParameter( size_t iParam, const wchar_t * pwszCmd )
{
	switch ( iParam )
	{
	case	paramRotationAxisType:
		m_rxtType = (RotationAxisType)
			SXMLDocument::GetIntegerAsSymbolOf
				( s_aiRotationAxisType, pwszCmd, m_rxtType ) ;
		return ;
	}
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DInstanceRotationController::EnumerateStringSet
	( size_t iParam, SSystem::SStringArray& aStrSet )
{
	int	i ;
	switch ( iParam )
	{
	case	paramRotationAxisType:
		for ( i = 0; s_aiRotationAxisType[i].pszSymbol != nullptr; i ++ )
		{
			aStrSet.Add( new SString( s_aiRotationAxisType[i].pszSymbol ) ) ;
		}
		return	true ;
	}
	return	false ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DInstanceRotationController::IsParameterValidation( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramRotationAxisVector:
		return	(m_rxtType == rotationAxisVector) ;
	}
	return	true ;
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DInstanceRotationController::OnTimer
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast )
{
	Controller::OnTimer( scene, pItem, msecPast ) ;

	double	secPast = (double) msecPast / 1000.0 ;
	double	dpsSpeed = m_dpsRotSpeed ;
	dpsSpeed += m_dpsRotSpeedAmp
					* sin( m_secSpeedPhase * 2.0 * PI
							/ esl_fmax(m_dpsRotSpeedCycle,1.0e-5) ) ;

	m_secSpeedPhase += secPast ;
	m_degRotCurrent += dpsSpeed * secPast ;

	ReflectInstanceBaseRotation() ;
}

// フレーム（パラメータ）更新後処理
//////////////////////////////////////////////////////////////////////////////
void S3DInstanceRotationController::OnUpdateFrame
	( S3DSceneComposer::ItemSerializer * pItem,
		double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	Controller::OnUpdateFrame( pItem, fpFrame, seek ) ;

	if ( seek == S3DSceneComposer::seekJumpReset )
	{
		m_secSpeedPhase = 0.0 ;
		m_degRotCurrent = 0.0 ;
		ReflectInstanceBaseRotation() ;
	}
}

// 回転反映
//////////////////////////////////////////////////////////////////////////////
void S3DInstanceRotationController::ReflectInstanceBaseRotation( void )
{
	S3DInstancingItemInterface *	pInstancingItem =
			ESLTypeCast<S3DInstancingItemInterface>( GetOwnerItem() ) ;
	if ( pInstancingItem == nullptr )
	{
		return ;
	}
	S3DItemInstancingSerializer *
			pInstancing = pInstancingItem->GetInstancing() ;
	if ( pInstancing == nullptr )
	{
		return ;
	}
	S3DDMatrix	matRotation( 1, 1, 1 ) ;
	double		rad = (m_degRotation + m_degRotCurrent) * PI / 180.0 ;
	switch ( m_rxtType )
	{
	case	rotationAxisX:
		matRotation.RevolveOnX( sin(rad), cos(rad) ) ;
		break ;
	case	rotationAxisY:
		matRotation.RevolveOnY( sin(rad), cos(rad) ) ;
		break ;
	case	rotationAxisZ:
		matRotation.RevolveOnZ( sin(rad), cos(rad) ) ;
		break ;
	case	rotationAxisVector:
		matRotation.RotationOnVectorOf( m_vRotAxis, sin(rad), cos(rad) ) ;
		break ;
	}
	S3DDQuaternion	qRotation ;
	qRotation.FromMatrix( matRotation * m_matBaseRotation ) ;
	pInstancing->SetBaseRotation( qRotation ) ;
}



//////////////////////////////////////////////////////////////////////////////
// 物理演算動的インスタンス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DOPhysicsDynamicInstancingInterface, S3DInstancingEntryEditInterface ) ;

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DOPhysicsDynamicInstancingInterface::S3DOPhysicsDynamicInstancingInterface( void )
	: m_material( S3DPhysicsScene::m_materialDefault ),
		m_matBase( 1, 1, 1 ), m_flagCollider( true ),
		m_maskColliderClasses( S3DCollision::colliderPhysItem )
{
	m_collider.type = S3DCollision::coliderTypeSolidSphere ;
	m_collider.vPos = S3DVector( 0, 0, 0 ) ;
	m_collider.fpRadius = 1.0f ;
	m_collider.vCubeSize = S3DVector( 1, 1, 1 ) ;
	m_collider.pColider = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DOPhysicsDynamicInstancingInterface::~S3DOPhysicsDynamicInstancingInterface( void )
{
}

// アクターをインスタンスに更新
//////////////////////////////////////////////////////////////////////////////
void S3DOPhysicsDynamicInstancingInterface::UpdateInstanceEntries
		( const S3DDMatrix& matSpace, const S3DDVector& vSpace )
{
	S3DPhysicsScene::Actor *const*	ppActors = m_aActors.GetConstArray() ;
	const size_t					nActors = m_aActors.GetLength() ;
	//
	S3DMatrix	matISpace = matSpace.Inverse() ;
	S4DMatrix *	pMatrixs = m_aMatrixs.GetArray( nActors ) ;
	S4DMatrix	mat4Temp( 1, 1, 1, 1 ) ;
	//
	for ( size_t i = 0; i < nActors; i ++ )
	{
		S3DPhysicsScene::Actor *	pActor = ppActors[i] ;
		ESLAssert( pActor != NULL ) ;
		if ( pActor == NULL )
		{
			continue ;
		}
		S3DMatrix	matRot ;
		S3DVector	vPos = pActor->m_vPos - vSpace ;
		pActor->m_qPosture.ToMatrix( matRot ) ;
		matRot.MagnifyByVector( pActor->m_vZoom ) ;
		// 
		mat4Temp.SetMatrix3( matISpace * matRot * m_matBase ) ;
		mat4Temp.SetTranslation( matISpace * vPos ) ;
		pMatrixs[i] = mat4Temp ;
	}
	//
	m_aMatrixs.FinishArray() ;
}

// アクターを S3DPhysicsScene へ追加（コライダも追加）
//////////////////////////////////////////////////////////////////////////////
void S3DOPhysicsDynamicInstancingInterface::AddActorToPhysicsScene( S3DPhysicsScene& scene )
{
	S3DPhysicsScene::Actor *const*	ppActors = m_aActors.GetConstArray() ;
	const size_t					nActors = m_aActors.GetLength() ;
	//
	scene.Lock() ;
	m_aRemoveActors.RemoveAll() ;
	scene.AddActors( ppActors, nActors ) ;
	//
	if ( m_flagCollider && (nActors > 0) )
	{
		S3DCollision&	collision = scene.Collision() ;
		collision.PushTransformation() ;
		collision.SetUserClassesMask( m_maskColliderClasses ) ;
		collision.BeginBatchBuild() ;
		//
		for ( size_t i = 0; i < nActors; i ++ )
		{
			S3DPhysicsScene::Actor *	pActor = ppActors[i] ;
			ESLAssert( pActor != NULL ) ;
			if ( pActor == NULL )
			{
				continue ;
			}
			S3DDMatrix	matdAct ;
			S3DMatrix	matAct ;
			pActor->m_qPosture.ToMatrix( matAct ) ;
			matAct.MagnifyByVector( pActor->m_vZoom ) ;
			matdAct = matAct * m_matBase ;
			//
			collision.AttachMeshUserData( pActor ) ;
			collision.SetMatrixTransformation( matdAct, pActor->m_vPos ) ;
			collision.AddColliderDescription( m_collider, i ) ;
		}
		//
		collision.EndBatchBuild() ;
		collision.PopTransformation() ;
	}
	scene.Unlock() ;
}

// 全インスタンス削除
//////////////////////////////////////////////////////////////////////////////
void S3DOPhysicsDynamicInstancingInterface::RemoveAllInstance( void )
{
	while ( m_aActors.GetLength() > 0 )
	{
		m_aRemoveActors.Add( m_aActors.Pop() ) ;
	}
	m_aMatrixs.RemoveAll() ;
	m_aColors.RemoveAll() ;
}

// インスタンス追加
//////////////////////////////////////////////////////////////////////////////
size_t S3DOPhysicsDynamicInstancingInterface::AddInstance
	( const S4DMatrix * pMatrixs,
		const S3DColor * pColors, size_t nCount,
		const S3DVector * pSpeeds, const S4DVector * pRotSpeeds,
		const void * pExInitData, size_t nExDataStride )
{
	size_t	iAddIndex = m_aActors.GetLength() ;
	ESLAssert( m_aMatrixs.GetLength() == iAddIndex ) ;
	ESLAssert( m_aColors.GetLength() == iAddIndex ) ;
	if ( pMatrixs == NULL )
	{
		ESLAssert( nCount == 0 ) ;
		return	iAddIndex ;
	}
	m_aMatrixs.AddArray( pMatrixs, nCount ) ;
	//
	if ( pColors != NULL )
	{
		m_aColors.AddArray( pColors, nCount ) ;
	}
	else
	{
		S3DColor	clrDummy( 0xFFFFFFFF, 0 ) ;
		m_aColors.SetLimit( iAddIndex + nCount ) ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			m_aColors.Add( clrDummy ) ;
		}
	}
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		S3DPhysicsScene::Actor *	pActor = NewActor( pExInitData ) ;
		pActor->m_pMaterial = &m_material ;
		pActor->m_pHitPoints = m_aHitPoint.GetConstArray() ;
		pActor->m_nHitPointCount = m_aHitPoint.GetLength() ;
		//
		pActor->m_vPos = pMatrixs[i].GetTranslation() ;
		pActor->m_vLastPos = pActor->m_vPos ;
		//
		S3DMatrix	matRotate = pMatrixs[i].GetMatrix3() ;
		S3DVector	vZoom( 1, 1, 1 ) ;
		matRotate.ExtractMagnification( vZoom ) ;
		pActor->m_qPosture.FromMatrix( matRotate ) ;
		pActor->m_qLastPosture = pActor->m_qPosture ;
		pActor->m_vZoom = vZoom ;
		//
		if ( pSpeeds != nullptr )
		{
			pActor->m_vSpeed = pSpeeds[i] ;
			pActor->m_vLastSpeed = pActor->m_vSpeed ;
		}
		if ( pRotSpeeds != nullptr )
		{
			pActor->m_vRotate = pRotSpeeds[i] ;
			pActor->m_fpRotSpeed = pRotSpeeds[i].w ;
		}
		//
		m_aActors.Add( pActor ) ;
		//
		pExInitData = ((const uint8_t*) pExInitData) + nExDataStride ;
	}
	return	iAddIndex ;
}

// インスタンス数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DOPhysicsDynamicInstancingInterface::GetInstanceCount( void ) const
{
	return	m_aActors.GetLength() ;
}

// インスタンス・アクター取得
//////////////////////////////////////////////////////////////////////////////
S3DPhysicsScene::Actor * S3DOPhysicsDynamicInstancingInterface::GetInstanceActorAt( size_t i ) const
{
	return	m_aActors.GetAt( i ) ;
}

// インスタンス色
//////////////////////////////////////////////////////////////////////////////
S3DColor * S3DOPhysicsDynamicInstancingInterface::GetInstanceColorAt( size_t i ) const
{
	return	m_aColors.GetAt( i ) ;
}

// アクター生成
//////////////////////////////////////////////////////////////////////////////
S3DPhysicsScene::Actor *
	S3DOPhysicsDynamicInstancingInterface::NewActor( const void * pExInitData ) const
{
	return	new S3DPhysicsScene::Actor
				( const_cast<S3DOPhysicsDynamicInstancingInterface*>( this ) ) ;
}

// インスタンシング・リスト取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DOPhysicsDynamicInstancingInterface::GetInstancingArray
	( const S4DMatrix*& pMatrixs, const S3DColor*& pColors ) const
{
	pMatrixs = m_aMatrixs.GetConstArray() ;
	pColors = m_aColors.GetConstArray() ;
	ESLAssert( m_aMatrixs.GetLength() == m_aColors.GetLength() ) ;
	return	m_aMatrixs.GetLength() ;
}

// インスタンス削除
//////////////////////////////////////////////////////////////////////////////
void S3DOPhysicsDynamicInstancingInterface::RemoveInstancingEntryAt( size_t i )
{
	ESLAssert( i < m_aActors.GetLength() ) ;
	S3DPhysicsScene::Actor *	pActor = m_aActors.DetachAt( i ) ;
	if ( pActor != NULL )
	{
		m_aRemoveActors.Add( pActor ) ;
	}
	m_aMatrixs.RemoveAt( i ) ;
	m_aColors.RemoveAt( i ) ;
}



//////////////////////////////////////////////////////////////////////////////
// 物理演算動的インスタンスコントローラー
//////////////////////////////////////////////////////////////////////////////

const SSystem::SXMLDocument::AttrInteger
	S3DOPhysicsInstancingController::m_aiCollisionShapeType[4] =
{
	{ L"mesh", S3DCollision::coliderTypeBuffer },
	{ L"sphere", S3DCollision::coliderTypeSolidSphere },
	{ L"cube", S3DCollision::coliderTypeSolidCube },
	{ NULL, 0 },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DOPhysicsInstancingController,
			Controller, S3DOPhysicsDynamicInstancingInterface )
S3D_IMPLEMENT_COMPOSER_ITEM
	( SakuraGL::S3DOPhysicsInstancingController, physics_instance )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DOPhysicsInstancingController::S3DOPhysicsInstancingController( void )
	: Controller( m_ItemClassDescriptor.pwszClassID ),
		m_flagReflectInstance( false ),
		m_matBaseRotation( 1, 1, 1 ), m_vBaseZoom( 1, 1, 1 ),
		m_flagUpdateCollision( false ), m_flagUpdateHitPoints( false ),
		m_flagValidBox( false ),
		m_typeColliderMarker( S3DModelData::MarkerInfo::typeInvalid ),
		m_pRefCollider( NULL )
{
	InitPhysicsInstancingController() ;
}

S3DOPhysicsInstancingController::S3DOPhysicsInstancingController( const wchar_t * pwszClassID )
	: Controller( pwszClassID ),
		m_flagReflectInstance( false ),
		m_matBaseRotation( 1, 1, 1 ), m_vBaseZoom( 1, 1, 1 ),
		m_flagUpdateCollision( false ), m_flagUpdateHitPoints( false ),
		m_flagValidBox( false ),
		m_vValidBoxMin( -10000, -1000, -10000 ),
		m_vValidBoxMax( 10000, 1000, 10000 ),
		m_typeColliderMarker( S3DModelData::MarkerInfo::typeInvalid ),
		m_pRefCollider( NULL )
{
	InitPhysicsInstancingController() ;
}

void S3DOPhysicsInstancingController::InitPhysicsInstancingController( void )
{
	m_flagsBehavior |= S3DSceneComposer::behaviorCollision
						| S3DSceneComposer::behaviorOnTimer ;
	//
	PrepareParameterEntryCount( paramCount ) ;
	AddParameterEntry
		( L"instancing",
			S3DSceneComposer::typeBinary,
			S3DSceneComposer::attrConstant
				| S3DSceneComposer::attrEditUpdateFrame,
			L"インスタンス" ) ;
	AddParameterEntry
		( L"rotation",
			S3DSceneComposer::typeRotation,
			S3DSceneComposer::attrConstant,
			L"回転" ) ;
	AddParameterEntry
		( L"zoom",
			S3DSceneComposer::typeZoom,
			S3DSceneComposer::attrConstant,
			L"拡大" ) ;
	AddParameterEntry
		( L"reflect_instance",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"動的インスタンス通知",
			L"他の PotentialInstancing コントローラーに動的に生成したインスタンスを通知する。\n"
			L"※その場合、他の Instancing も動的通知に対応して整合性が取れる必要があるため、"
			L"通常はインスタンスを生成するコントローラーはこのコントローラー単独でのみ使用する。" ) ;
	AddParameterEntry
		( L"no_rotation",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant1,
			L"回転無効", L"物理演算で回転を禁止する" ) ;
	AddParameterEntry
		( L"no_move",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant1,
			L"移動無効", L"物理演算で移動を禁止する" ) ;
	AddParameterEntry
		( L"weight",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant1,
			L"質量", L"物理演算でのオブジェクトの質量" ) ;
	AddParameterEntry
		( L"rot_weight",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant1,
			L"回転質量比", L"物理演算でのトルクに影響する質量比率（大きい方が回転しづらい）" ) ;
	AddParameterEntry
		( L"volume",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant1,
			L"空間比重", L"物理演算での空間媒質流の影響を与えるパラメータ" ) ;
	AddParameterEntry
		( L"stillFriction",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant1,
			L"静止摩擦力",
			L"この力以上でスリップする。\n"
			L"動摩擦係数は(係る力)／(静止摩擦力)×(摩擦抵抗係数)で決定する。" ) ;
	AddParameterEntry
		( L"resistance",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant1,
			L"摩擦抵抗係数", L"動摩擦力を計算するための係数。" ) ;
	AddParameterEntry
		( L"elasticity",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant1,
			L"弾性率" ) ;
	AddParameterEntry
		( L"actor_collision",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant2,
			L"物理当たり判定", L"インスタンスに物理演算用当たり判定を設定する" ) ;
	AddParameterEntry
		( L"collision_shape",
			S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant2
				| S3DSceneComposer::attrStringEnumeration
				| S3DSceneComposer::attrUIOnlyEnumeration
				| S3DSceneComposer::attrDynamicValidation,
			L"当たり判定形状" ) ;
	AddParameterEntry
		( L"collision_center",
			S3DSceneComposer::typePosition,
			S3DSceneComposer::attrConstant2,
			L"当たり判定中心" ) ;
	AddParameterEntry
		( L"collision_radius",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant2,
			L"当たり判定半径" ) ;
	AddParameterEntry
		( L"collision_size",
			S3DSceneComposer::typeZoom,
			S3DSceneComposer::attrConstant2,
			L"当たり判定サイズ" ) ;
	AddParameterEntry
		( L"collider_marker_type",
			S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant3
				| S3DSceneComposer::attrStringEnumeration
				| S3DSceneComposer::attrUIOnlyEnumeration,
			L"コライダ・マーカー" ) ;
	AddParameterEntry
		( L"collider_foot_marker",
			S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant3
				| S3DSceneComposer::attrStringEnumeration,
			L"接地用マーカー" ) ;
	AddParameterEntry
		( L"collider_class_mask",
			S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant3
				| S3DSceneComposer::attrFlagSetInteger,
			L"コライダ・クラスマスク" ) ;
	AddParameterEntry
		( L"valid_box",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant4,
			L"有効領域外で削除", L"有効領域外に出た時に自動的に削除する" ) ;
	AddParameterEntry
		( L"valid_box_min",
			S3DSceneComposer::typePosition,
			S3DSceneComposer::attrConstant4,
			L"有効領域最小値" ) ;
	AddParameterEntry
		( L"valid_box_max",
			S3DSceneComposer::typePosition,
			S3DSceneComposer::attrConstant4,
			L"有効領域最大値" ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DOPhysicsInstancingController::~S3DOPhysicsInstancingController( void )
{
}

// 回転
//////////////////////////////////////////////////////////////////////////////
const S3DMatrix& S3DOPhysicsInstancingController::GetBaseRotation( void ) const
{
	return	m_matBaseRotation ;
}

void S3DOPhysicsInstancingController::SetBaseRotation( const S3DMatrix& matRot )
{
	m_matBaseRotation = matRot ;
}

// 拡大
//////////////////////////////////////////////////////////////////////////////
const S3DVector& S3DOPhysicsInstancingController::GetBaseZoom( void ) const
{
	return	m_vBaseZoom ;
}

void S3DOPhysicsInstancingController::SetBaseZoom( const S3DVector& vZoom )
{
	m_vBaseZoom = vZoom ;
}

// ベース行列更新
//////////////////////////////////////////////////////////////////////////////
void S3DOPhysicsInstancingController::UpdateBaseMatrix( void )
{
	SetBaseMatrix
		( m_matBaseRotation
			* S3DMatrix( m_vBaseZoom.x, m_vBaseZoom.y, m_vBaseZoom.z ) ) ;
}

// コリジョン更新
//////////////////////////////////////////////////////////////////////////////
void S3DOPhysicsInstancingController::UpdateCollision( void )
{
	if ( m_collider.type == S3DCollision::coliderTypeBuffer )
	{
		S3DSceneComposer::ItemSerializer *	pItem = GetOwnerItem() ;
		S3DVertexBufferInterface *			pVBO = NULL ;
		if ( pItem != NULL )
		{
			pVBO = pItem->GetItemPrimaryModel() ;
		}
		if ( m_pRefCollider != pVBO )
		{
			if ( pVBO != NULL )
			{
				m_collisionBuf.ClearBuffer() ;
				m_collisionBuf.BeginBatchBuild() ;
				pVBO->RenderBufferTo( &m_collisionBuf ) ;
				m_collisionBuf.EndBatchBuild() ;
				m_collider.pColider = &m_collisionBuf ;
			}
			else
			{
				m_collider.pColider = NULL ;
			}
			m_pRefCollider = pVBO ;
		}
	}
	m_flagUpdateCollision = false ;
}

// 当たり判定点更新
//////////////////////////////////////////////////////////////////////////////
void S3DOPhysicsInstancingController::UpdateHitPoints( void )
{
	const S3DVector4 *	pvLastHitPoints = m_aHitPoint.GetConstArray() ;
	const size_t		nLastHitPoints = m_aHitPoint.GetLength() ;
	m_aHitPoint.RemoveAll() ;
	//
	// アイテム情報取得
	//
	S3DSceneComposer::ItemSerializer *	pItem = GetOwnerItem() ;
	S3DVertexBufferInterface *			pVBO = NULL ;
	S3DModelBuffer *					pModel = NULL ;
	S3DModelData::MarkerInfo *			pmiFoot = NULL ;
	const S3DDMatrix					matI( 1, 1, 1 ) ;
	const S3DDVector					vZero( 0, 0, 0 ) ;
	//
	if ( pItem != NULL )
	{
		pVBO = pItem->GetItemPrimaryModel() ;
		pModel = ESLTypeCast<S3DModelBuffer>( pVBO ) ;
		if ( (pModel != NULL) && !m_strFootMarkerID.IsEmpty() )
		{
			pmiFoot = pModel->GetMarkerInfoAs( m_strFootMarkerID ) ;
			if ( pmiFoot != NULL )
			{
				S3DDVector	v ;
				float32_t	r ;
				pModel->CalcMarkerPosition( v, r, matI, vZero, *pmiFoot ) ;
				//
				S3DVector4	vMarker( v, r ) ;
				m_aHitPoint.Add( vMarker ) ;
			}
		}
	}
	//
	// 当たり判定点追加
	//
	if ( m_typeColliderMarker == S3DModelData::MarkerInfo::typeInvalid )
	{
		//
		// コリジョン形状から
		//
		if ( m_collider.type == S3DCollision::coliderTypeBuffer )
		{
			if ( pModel != NULL )
			{
				MakeHitPointsFromModel( pModel ) ;
			}
			else if ( pVBO != NULL )
			{
				MakeHitPointsFromVBO( pVBO ) ;
			}
		}
		else if ( m_collider.type == S3DCollision::coliderTypeSolidSphere )
		{
			MakeHitPointsFromSphere( m_collider.vPos, m_collider.fpRadius ) ;
		}
		else if ( m_collider.type == S3DCollision::coliderTypeSolidCube )
		{
			MakeHitPointsFromCube( m_collider.vPos, m_collider.vCubeSize ) ;
		}
	}
	else if ( pModel != NULL )
	{
		//
		// マーカーから
		//
		SStrSortObjectArray<S3DModelData::MarkerInfo>&
								aMarker = pModel->GetMarkerInfoList() ;
		for ( size_t i = 0; i < aMarker.GetLength(); i ++ )
		{
			S3DModelData::MarkerInfo *	pmi = aMarker.GetAt( i ) ;
			ESLAssert( pmi != NULL ) ;
			if ( (pmi != NULL) && (pmi != pmiFoot)
				&& (pmi->m_type == m_typeColliderMarker) )
			{
				S3DDVector	v ;
				float32_t	r ;
				pModel->CalcMarkerPosition( v, r, matI, vZero, *pmi ) ;
				//
				S3DVector4	vMarker( v, r ) ;
				m_aHitPoint.Add( vMarker ) ;
			}
		}
	}
	//
	// アクターへ更新
	//
	if ( (m_aHitPoint.GetConstArray() != pvLastHitPoints)
		|| (nLastHitPoints != m_aHitPoint.GetLength()) )
	{
		S3DPhysicsScene::Actor **	ppActors = m_aActors.GetArray() ;
		size_t						nCount = m_aActors.GetLength() ;
		//
		const S3DVector4 *	pHitPoints = m_aHitPoint.GetConstArray() ;
		size_t				nHitPointCount = m_aHitPoint.GetLength() ;
		float32_t			fpRadius = 0.00001f ;
		//
		for ( size_t i = 0; i < nCount; i ++ )
		{
			S3DPhysicsScene::Actor *	pActor = ppActors[i] ;
			ESLAssert( pActor != NULL ) ;
			if ( pActor != NULL )
			{
				pActor->m_pHitPoints = pHitPoints ;
				pActor->m_nHitPointCount = nHitPointCount ;
			}
		}
		//
		m_aActors.FinishArray() ;
	}
	//
	m_flagUpdateHitPoints = false ;
}

void S3DOPhysicsInstancingController::MakeHitPointsFromModel
	( S3DModelBuffer * pModel )
{
	MakeHitPointsFromVertex
		( pModel->GetVertexBufferAt(0), pModel->GetVertexBufferLength() ) ;
}

void S3DOPhysicsInstancingController::MakeHitPointsFromVBO
	( S3DVertexBufferInterface * pVBO )
{
	SArray<S3DVector4>	bufVertex ;
	//
	S3DVertexBufferInterface::MeshInfo	minf ;
	eslFillMemory( &minf, 0, sizeof(S3DVertexBufferInterface::MeshInfo) ) ;
	//
	const size_t	nMeshCount = pVBO->GetMeshCount() ;
	for ( size_t iMesh = 0; iMesh < nMeshCount; iMesh ++ )
	{
		if ( pVBO->GetMeshInfoAt( minf, iMesh, 0 ) )
		{
			continue ;
		}
		size_t	iBase = bufVertex.GetLength() ;
		bufVertex.SetLength( iBase + minf.countVertex ) ;
		minf.pvVertex = bufVertex.GetArray() + iBase ;
		pVBO->GetMeshInfoAt( minf, iMesh, minf.countVertex, 0 ) ;
		bufVertex.FinishArray() ;
	}
	//
	MakeHitPointsFromVertex
		( bufVertex.GetConstArray(), bufVertex.GetLength() ) ;
}

void S3DOPhysicsInstancingController::MakeHitPointsFromVertex
	( const S3DVector4 * pvVertex, size_t nCount )
{
	struct VertexInfo
	{
		uint32_t	iIndex ;
		float32_t	fpLength ;
		float32_t	fpRcpLength ;
		float32_t	cosMax ;
		float32_t	fpValue ;
	} ;
	if ( nCount == 0 )
	{
		return ;
	}
	//
	// 頂点情報概要
	//
	SArray<VertexInfo>	aVertInfo ;
	VertexInfo *	pviVertex = aVertInfo.GetArray( nCount ) ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		VertexInfo&	vi = pviVertex[i] ;
		vi.iIndex = (uint32_t) i ;
		vi.fpLength = (float32_t) pvVertex[i].Absolute() ;
		vi.fpRcpLength = (vi.fpLength < 1.0e-8) ? 0.0f : (1.0f / vi.fpLength) ;
		vi.cosMax = 0.0f ;
		vi.fpValue = vi.fpLength ;
	}
	//
	// 特徴点を抽出する
	//
	const size_t	nMaxHitPointCount = 24 ;
	size_t			nHitPoints = 0 ;
	for ( size_t i = 0; (i < nCount) && (i < nMaxHitPointCount); i ++ )
	{
		//
		// 最大値を検索
		//
		size_t		iMax = i ;
		float32_t	fpMax = pviVertex[i].fpValue ;
		for ( size_t j = i + 1; j < nCount; j ++ )
		{
			if ( fpMax < pviVertex[j].fpValue )
			{
				iMax = j ;
				fpMax = pviVertex[j].fpValue ;
			}
		}
		VertexInfo	viTemp = pviVertex[i] ;
		if ( viTemp.cosMax > 0.99999f )
		{
			break ;
		}
		nHitPoints = i + 1 ;
		pviVertex[i] = pviVertex[iMax] ;
		pviVertex[iMax] = viTemp ;
		//
		// 評価値更新
		//
		S3DVector	vDir = pvVertex[ pviVertex[i].iIndex ] ;
		vDir *= pviVertex[i].fpRcpLength ;
		//
		for ( size_t j = i + 1; j < nCount; j ++ )
		{
			VertexInfo&	vi = pviVertex[j] ;
			S3DVector	v = pvVertex[ vi.iIndex ] ;
			float32_t	c = v.InnerProduct( vDir ) * vi.fpRcpLength ;
			if ( c > vi.cosMax )
			{
				vi.cosMax = c ;
				vi.fpValue = esl_fminf( 1.0f - c, 1.0f ) * vi.fpLength ;
			}
		}
	}
	//
	// 当たり判定点を設定
	//
	m_aHitPoint.SetLimit( m_aHitPoint.GetLength() + nHitPoints ) ;
	//
	float32_t	r = pviVertex[0].fpLength * 0.001f ;
	for ( size_t i = 0; i < nHitPoints; i ++ )
	{
		S3DVector4	vHitPoint = pvVertex[ pviVertex[i].iIndex ] ;
		vHitPoint.d = r ;
		m_aHitPoint.Add( vHitPoint ) ;
	}
	//
	aVertInfo.FinishArray() ;
}

void S3DOPhysicsInstancingController::MakeHitPointsFromSphere
	( const S3DVector& vCenter, float32_t fpRadius )
{
	S3DVector4	vHitPoint( vCenter, fpRadius ) ;
	m_aHitPoint.Add( vHitPoint ) ;
}

void S3DOPhysicsInstancingController::MakeHitPointsFromCube
	( const S3DVector& vCenter, const S3DVector& vCubeSize )
{
	m_aHitPoint.SetLimit( m_aHitPoint.GetLength() + 9 ) ;
	//
	float32_t	r = (float32_t) vCubeSize.Absolute() * 0.001f ;
	float32_t	c = esl_fminf( esl_fminf( vCubeSize.x, vCubeSize.y ), vCubeSize.z ) ;
	S3DVector4	vHitPoint( vCenter, c ) ;
	m_aHitPoint.Add( vHitPoint ) ;
	//
	static const float32_t	vCube[8][3] =
	{
		{ 1, 1, 1 },
		{ 1, 1, -1 },
		{ -1, 1, 1 },
		{ -1, 1, -1 },
		{ 1, -1, 1 },
		{ 1, -1, -1 },
		{ -1, -1, 1 },
		{ -1, -1, -1 },
	} ;
	for ( size_t i = 0; i < 8; i ++ )
	{
		vHitPoint.x = vCenter.x + (vCubeSize.x - r) * vCube[i][0] ;
		vHitPoint.y = vCenter.y + (vCubeSize.y - r) * vCube[i][1] ;
		vHitPoint.z = vCenter.z + (vCubeSize.z - r) * vCube[i][2] ;
		vHitPoint.d = r ;
		m_aHitPoint.Add( vHitPoint ) ;
	}
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DOPhysicsInstancingController::GetMatrixParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRotation:
		return	m_matBaseRotation ;
	}
	return	S3DDMatrix( 1, 1, 1 ) ;
}

S3DDVector S3DOPhysicsInstancingController::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramZoom:
		return	m_vBaseZoom ;

	case	paramCollisionCenter:
		return	m_collider.vPos ;

	case	paramCollisionCubeSize:
		return	m_collider.vCubeSize ;

	case	paramValidBoxMin:
		return	m_vValidBoxMin ;

	case	paramValidBoxMax:
		return	m_vValidBoxMax ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DOPhysicsInstancingController::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramWeight:
		return	m_material.fpWeight ;

	case	paramRotWeight:
		return	m_material.fpRotWeight ;

	case	paramVolume:
		return	m_material.fpVolume ;

	case	paramStillFriction:
		return	m_material.fpStillFriction ;

	case	paramResistance:
		return	m_material.fpResistance ;

	case	paramElasticity:
		return	m_material.fpElasticity ;

	case	paramCollisionRadius:
		return	m_collider.fpRadius ;
	}
	return	0.0 ;
}

int32_t S3DOPhysicsInstancingController::GetIntegerParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramColliderClassesMask:
		return	(int32_t) m_maskColliderClasses ;
	}
	return	0 ;
}

bool S3DOPhysicsInstancingController::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramReflectInstance:
		return	m_flagReflectInstance ;

	case	paramNoRotation:
		return	(m_material.nFlags & S3DPhysicsScene::actorNoRotation) != 0 ;

	case	paramNoMove:
		return	(m_material.nFlags & S3DPhysicsScene::actorNoMove) != 0 ;

	case	paramActorCollision:
		return	m_flagCollider ;

	case	paramValidBox:
		return	m_flagValidBox ;
	}
	return	false ;
}

const wchar_t * S3DOPhysicsInstancingController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramInstancing:
		return	m_instancing.GetInstancingEntriesBase64() ;

	case	paramCollisionShape:
		return	SXMLDocument::GetSymbolAsIntegerOf
							( m_aiCollisionShapeType, m_collider.type ) ;

	case	paramColliderMarkerType:
		if ( m_typeColliderMarker == S3DModelData::MarkerInfo::typeInvalid )
		{
			return	L"nothing" ;
		}
		return	S3DModelData::MarkerInfo::m_pwszTypeTags[m_typeColliderMarker] ;

	case	paramColliderFootMarkerID:
		return	m_strFootMarkerID ;
	}
	return	NULL ;
}

size_t S3DOPhysicsInstancingController::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t i ) const
{
	switch ( i )
	{
	case	paramInstancing:
		if ( pDst == NULL )
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
	return	0 ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DOPhysicsInstancingController::SetMatrixParameter( size_t i, const S3DDMatrix& mat )
{
	switch ( i )
	{
	case	paramRotation:
		m_matBaseRotation = mat ;
		UpdateBaseMatrix() ;
		return ;
	}
}

void S3DOPhysicsInstancingController::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramZoom:
		m_vBaseZoom = vec ;
		UpdateBaseMatrix() ;
		return ;

	case	paramCollisionCenter:
		m_collider.vPos = vec ;
		m_flagUpdateHitPoints = true ;
		return ;

	case	paramCollisionCubeSize:
		m_collider.vCubeSize = vec ;
		m_flagUpdateHitPoints = true ;
		return ;

	case	paramValidBoxMin:
		m_vValidBoxMin = vec ;
		return ;

	case	paramValidBoxMax:
		m_vValidBoxMax = vec ;
		return ;
	}
}

void S3DOPhysicsInstancingController::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramWeight:
		m_material.fpWeight = (float32_t) s ;
		return ;

	case	paramRotWeight:
		m_material.fpRotWeight = (float32_t) s ;
		return ;

	case	paramVolume:
		m_material.fpVolume = (float32_t) s ;
		return ;

	case	paramStillFriction:
		m_material.fpStillFriction = (float32_t) s ;
		return ;

	case	paramResistance:
		m_material.fpResistance = (float32_t) s ;
		return ;

	case	paramElasticity:
		m_material.fpElasticity = (float32_t) s ;
		return ;

	case	paramCollisionRadius:
		m_collider.fpRadius = (float32_t) s ;
		m_flagUpdateHitPoints = true ;
		return ;
	}
}

void S3DOPhysicsInstancingController::SetIntegerParameter( size_t iParam, int32_t n )
{
	switch ( iParam )
	{
	case	paramColliderClassesMask:
		m_maskColliderClasses = (uint32_t) n ;
		return ;
	}
}

void S3DOPhysicsInstancingController::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramReflectInstance:
		m_flagReflectInstance = b ;
		return ;

	case	paramNoRotation:
		m_material.nFlags =
			(m_material.nFlags & ~S3DPhysicsScene::actorNoRotation)
						| (b ? S3DPhysicsScene::actorNoRotation : 0) ;
		return ;

	case	paramNoMove:
		m_material.nFlags =
			(m_material.nFlags & ~S3DPhysicsScene::actorNoMove)
						| (b ? S3DPhysicsScene::actorNoMove : 0) ;
		return ;

	case	paramActorCollision:
		m_flagCollider = b ;
		return ;

	case	paramValidBox:
		m_flagValidBox = b ;
		return ;
	}
}

void S3DOPhysicsInstancingController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	size_t	j ;
	switch ( i )
	{
	case	paramInstancing:
		m_instancing.SetInstancingEntriesBase64( pwszCmd ) ;
		return ;

	case	paramCollisionShape:
		m_collider.type =
			(S3DCollision::ColliderType)
				SXMLDocument::GetIntegerAsSymbolOf
						( m_aiCollisionShapeType, pwszCmd, m_collider.type ) ;
		m_flagUpdateCollision = true ;
		if ( m_typeColliderMarker == S3DModelData::MarkerInfo::typeInvalid )
		{
			m_flagUpdateHitPoints = true ;
		}
		return ;

	case	paramColliderMarkerType:
		for ( j = 0; j < S3DModelData::MarkerInfo::typeCount; j ++ )
		{
			if ( SString::Compare( pwszCmd, S3DModelData::MarkerInfo::m_pwszTypeTags[j] ) == 0 )
			{
				m_flagUpdateHitPoints |= (m_typeColliderMarker != j) ;
				m_typeColliderMarker = (S3DModelData::MarkerInfo::Type) j ;
				return ;
			}
		}
		m_flagUpdateHitPoints |=
				(m_typeColliderMarker != S3DModelData::MarkerInfo::typeInvalid) ;
		m_typeColliderMarker = S3DModelData::MarkerInfo::typeInvalid ;
		return ;

	case	paramColliderFootMarkerID:
		if ( m_strFootMarkerID != pwszCmd )
		{
			m_strFootMarkerID = pwszCmd ;
			m_flagUpdateHitPoints = true ;
		}
		return ;
	}
}

size_t S3DOPhysicsInstancingController::SetBinaryParameter
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
	return	0 ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DOPhysicsInstancingController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	size_t	j ;
	switch ( i )
	{
	case	paramCollisionShape:
		for ( j = 0; m_aiCollisionShapeType[j].pszSymbol != NULL; j ++ )
		{
			aStrSet.Add( new SString( m_aiCollisionShapeType[j].pszSymbol ) ) ;
		}
		return	true ;

	case	paramColliderMarkerType:
		for ( j = 0; j < S3DModelData::MarkerInfo::typeCount; j ++ )
		{
			aStrSet.Add( new SString( S3DModelData::MarkerInfo::m_pwszTypeTags[j] ) ) ;
		}
		aStrSet.Add( new SString( L"nothing" ) ) ;
		return	true ;

	case	paramColliderFootMarkerID:
		{
			S3DSceneComposer::ItemSerializer *	pItem = GetOwnerItem() ;
			S3DModelData::MarkerInfo *			pmiFoot = NULL ;
			//
			if ( pItem == NULL )
			{
				return	false ;
			}
			S3DModelBuffer *	pModel =
				ESLTypeCast<S3DModelBuffer>( pItem->GetItemPrimaryModel() ) ;
			if ( pModel == NULL )
			{
				return	false ;
			}
			SStrSortObjectArray<S3DModelData::MarkerInfo>&
									ssoaMarker = pModel->GetMarkerInfoList() ;
			for ( j = 0; j < ssoaMarker.GetLength(); j ++ )
			{
				const SString *	pstrTag = ssoaMarker.GetTagAt( j ) ;
				if ( pstrTag != NULL )
				{
					aStrSet.Add( new SString( *pstrTag ) ) ;
				}
			}
		}
		return	true ;
	}
	return	false ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DOPhysicsInstancingController::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramCollisionCenter:
		return	(m_collider.type == S3DCollision::coliderTypeSolidSphere)
				|| (m_collider.type == S3DCollision::coliderTypeSolidCube) ;

	case	paramCollisionRadius:
		return	(m_collider.type == S3DCollision::coliderTypeSolidSphere) ;

	case	paramCollisionCubeSize:
		return	(m_collider.type == S3DCollision::coliderTypeSolidCube) ;
	}
	return	true ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DOPhysicsInstancingController::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
	default:
		return	L"インスタンス" ;

	case	1:
		return	L"物性" ;

	case	2:
		return	L"被当たり判定形状" ;

	case	3:
		return	L"当たり判定点" ;

	case	4:
		return	L"有効領域" ;
	}
	return	NULL ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DOPhysicsInstancingController::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags )
{
	uint32_t	nResFlags =
			Controller::UpdatePropertyReference( comp, pItem, nFlags ) ;
	//
	return	nResFlags ;
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DOPhysicsInstancingController::OnTimer
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast )
{
	Controller::OnTimer( scene, pItem, msecPast ) ;
	//
	S3DDMatrix	matItem ;
	S3DDVector	vItem ;
	pItem->GetGlobalTransformation( matItem, vItem ) ;
	UpdateInstanceEntries( matItem, vItem ) ;
}

// フレーム（パラメータ）更新後処理
//////////////////////////////////////////////////////////////////////////////
void S3DOPhysicsInstancingController::OnUpdateFrame
	( S3DSceneComposer::ItemSerializer * pItem,
			double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	Controller::OnUpdateFrame( pItem, fpFrame, seek ) ;
	//
	if ( seek == S3DSceneComposer::seekJumpReset )
	{
		const S4DMatrix*	pMatrixs ;
		const S3DColor*		pColors ;
		const size_t		nCount =
			m_instancing.GetStaticInstancingArray( pMatrixs, pColors ) ;
		//
		RemoveAllInstance() ;
		AddInstance( pMatrixs, pColors, nCount ) ;
	}
}

// 当たり判定追加
//（必要であれば scene.PhysicsScene() へ Actor の追加も行う）
//////////////////////////////////////////////////////////////////////////////
void S3DOPhysicsInstancingController::RenderCollision
	( const S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem, S3DCollision& render )
{
	Controller::RenderCollision( scene, pItem, render ) ;
	//
	if ( m_flagUpdateCollision )
	{
		UpdateCollision() ;
	}
	if ( m_flagUpdateHitPoints )
	{
		UpdateHitPoints() ;
	}
	if ( m_flagValidBox )
	{
		for ( size_t i = 0; i < GetInstanceCount(); i ++ )
		{
			size_t	j = GetInstanceCount() - i - 1 ;
			S3DPhysicsScene::Actor *	pActor = GetInstanceActorAt( j ) ;
			if ( (pActor == nullptr)
				|| (pActor->m_vPos.x < m_vValidBoxMin.x)
				|| (pActor->m_vPos.y < m_vValidBoxMin.y)
				|| (pActor->m_vPos.z < m_vValidBoxMin.z)
				|| (pActor->m_vPos.x > m_vValidBoxMax.x)
				|| (pActor->m_vPos.y > m_vValidBoxMax.y)
				|| (pActor->m_vPos.z > m_vValidBoxMax.z) )
			{
				RemoveInstanceAt( j ) ;
				i -- ;
				continue ;
			}
		}
	}
	AddActorToPhysicsScene( scene.PhysicsScene() ) ;
}

// インスタンス削除
//////////////////////////////////////////////////////////////////////////////
void S3DOPhysicsInstancingController::RemoveInstanceAt( size_t i )
{
	S3DOPhysicsDynamicInstancingInterface::RemoveInstancingEntryAt( i ) ;

	if ( m_flagReflectInstance )
	{
		S3DSceneComposer::ItemSerializer *	pOwnerItem = GetOwnerItem() ;
		S3DInstancingItemInterface *
			pOwnerInstancing = ESLTypeCast<S3DInstancingItemInterface>( pOwnerItem ) ;
		if ( pOwnerInstancing != nullptr )
		{
			S3DItemInstancingSerializer *
						pInstancing = pOwnerInstancing->GetInstancing() ;
			if ( pInstancing != nullptr )
			{
				SSmartLock<S3DItemInstancingSerializer>	lock( pInstancing ) ;
				size_t	iFirst = GetFirstDynamicInstanceIndex() ;
				pInstancing->DeleteIndexByInstanceIndex( iFirst + i ) ;
				pInstancing->NotifyOnRemovedInstance( iFirst + i, pOwnerItem ) ;
			}
		}
	}
}

// 全インスタンス削除
//////////////////////////////////////////////////////////////////////////////
void S3DOPhysicsInstancingController::RemoveAllInstance( void )
{
	size_t	nCount = GetInstanceCount() ;

	S3DOPhysicsDynamicInstancingInterface::RemoveAllInstance() ;

	if ( m_flagReflectInstance )
	{
		S3DSceneComposer::ItemSerializer *	pOwnerItem = GetOwnerItem() ;
		S3DInstancingItemInterface *
			pOwnerInstancing = ESLTypeCast<S3DInstancingItemInterface>( pOwnerItem ) ;
		if ( pOwnerInstancing != nullptr )
		{
			S3DItemInstancingSerializer *
						pInstancing = pOwnerInstancing->GetInstancing() ;
			if ( pInstancing != nullptr )
			{
				SSmartLock<S3DItemInstancingSerializer>	lock( pInstancing ) ;
				size_t	iFirst = GetFirstDynamicInstanceIndex() ;
				for ( size_t i = 0; i < nCount; i ++ )
				{
					pInstancing->NotifyOnRemovedInstance
							( iFirst + nCount - 1 - i, pOwnerItem ) ;
				}
			}
		}
	}
}

// インスタンス追加
//////////////////////////////////////////////////////////////////////////////
size_t S3DOPhysicsInstancingController::AddInstance
	( const S4DMatrix * pMatrixs,
		const S3DColor * pColors, size_t nCount,
		const S3DVector * pSpeeds,
		const S4DVector * pRotSpeeds,
		const void * pExInitData, size_t nExDataStride )
{
	size_t	iIndex =
		S3DOPhysicsDynamicInstancingInterface::AddInstance
			( pMatrixs, pColors, nCount,
					pSpeeds, pRotSpeeds, pExInitData, nExDataStride ) ;

	if ( m_flagReflectInstance && (nCount > 0) )
	{
		S3DSceneComposer::ItemSerializer *	pOwnerItem = GetOwnerItem() ;
		S3DInstancingItemInterface *
			pOwnerInstancing = ESLTypeCast<S3DInstancingItemInterface>( pOwnerItem ) ;
		if ( pOwnerInstancing != nullptr )
		{
			S3DItemInstancingSerializer *
						pInstancing = pOwnerInstancing->GetInstancing() ;
			if ( pInstancing != nullptr )
			{
				SSmartLock<S3DItemInstancingSerializer>	lock( pInstancing ) ;
				if ( pColors == nullptr )
				{
					pColors = m_aColors.GetAt( iIndex ) ;
					ESLAssert( pColors != nullptr ) ;
				}
				size_t	iFirst = GetFirstDynamicInstanceIndex() ;
				for ( size_t i = 0; i < nCount; i ++ )
				{
					pInstancing->NotifyOnInsertedInstance
						( iFirst + iIndex + i, pMatrixs[i], pColors[i], pOwnerItem ) ;
				}
			}
		}
	}
	return	iIndex ;
}

size_t S3DOPhysicsInstancingController::GetFirstDynamicInstanceIndex( void ) const
{
	size_t	nCount = 0 ;

	S3DSceneComposer::ItemSerializer *	pOwnerItem = GetOwnerItem() ;
	S3DInstancingItemInterface *
		pOwnerInstancing = ESLTypeCast<S3DInstancingItemInterface>( pOwnerItem ) ;
	if ( pOwnerInstancing != nullptr )
	{
		S3DItemInstancingSerializer *
					pInstancing = pOwnerInstancing->GetInstancing() ;
		if ( pInstancing != nullptr )
		{
			nCount += pInstancing->GetStaticInstanceCount() ;
		}
	}
	if ( pOwnerItem != nullptr )
	{
		size_t	nCtrl = pOwnerItem->GetControllerCount() ;
		for ( size_t i = 0; i < nCtrl; i ++ )
		{
			S3DInstancingEntryInterface *	pCtrl =
				ESLTypeCast<S3DInstancingEntryInterface>
						( pOwnerItem->GetControllerAt( i ) ) ;
			if ( pCtrl == this )
			{
				break ;
			}
			if ( pCtrl != nullptr )
			{
				const S4DMatrix *	pMatrixs ;
				const S3DColor *	pColors ;
				nCount += pCtrl->GetInstancingArray( pMatrixs, pColors ) ;
			}
		}
	}

	return	nCount ;
}

// アクター生成
//////////////////////////////////////////////////////////////////////////////
S3DPhysicsScene::Actor *
	S3DOPhysicsInstancingController::NewActor( const void * pExInitData ) const
{
	return	new S3DPhysicsScene::Actor( GetOwnerItem() ) ;
}


