
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/render/sglx3d_scene_item_curve.h>

using namespace SSystem ;
using namespace SakuraGL ;

//////////////////////////////////////////////////////////////////////////////
// ベジェ曲線・シリアライザ（共通）
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DItemBezierCurveSerializer::S3DItemBezierCurveSerializer( void )
{
	m_flagModified = false ;
}

// インスタンシング・リスト設定
//////////////////////////////////////////////////////////////////////////////
void S3DItemBezierCurveSerializer::SetBezierCurveBase64( const wchar_t * pwszBase64 )
{
	m_strDataBase64 = pwszBase64 ;
	UpdateDataFromBase64() ;
}

void S3DItemBezierCurveSerializer::SetBezierCurve
		( size_t nCount,
			const S3DSceneComposer::BinaryBezierCurvePoint * pbbcp )
{
	EncodeData( m_strDataBase64, nCount, pbbcp ) ;
	UpdateBezierCurve( nCount, pbbcp ) ;
	m_flagModified = false ;
}

// インスタンシング・リスト取得
//////////////////////////////////////////////////////////////////////////////
const SSystem::SString& S3DItemBezierCurveSerializer::GetBezierCurveBase64( void )
{
	if ( m_flagModified )
	{
		EncodeData
			( m_strDataBase64,
				m_aBezierCurve.GetLength(), m_aBezierCurve.GetConstArray() ) ;
		m_flagModified = false ;
	}
	return	m_strDataBase64 ;
}

size_t S3DItemBezierCurveSerializer::GetDataLengthInBytes( void ) const
{
	return	sizeof(S3DSceneComposer::BinaryBezierCurveData)
			+ ((int) m_aBezierCurve.GetLength() - 1)
				* sizeof(S3DSceneComposer::BinaryBezierCurvePoint) ;
}

size_t S3DItemBezierCurveSerializer::GetBezierCurveData( S3DSceneComposer::BinaryBezierCurveData& bbcd ) const
{
	bbcd.count = (uint32_t) m_aBezierCurve.GetLength() ;
	bbcd.type = 0 ;
	bbcd.reserved[0] = 0 ;
	bbcd.reserved[1] = 0 ;
	//
	eslCopyMemory
		( &(bbcd.points[0]),
			m_aBezierCurve.GetConstArray(),
			m_aBezierCurve.GetLength()
				* sizeof(S3DSceneComposer::BinaryBezierCurvePoint) ) ;
	return	sizeof(S3DSceneComposer::BinaryBezierCurveData)
			+ ((int) m_aBezierCurve.GetLength() - 1)
				* sizeof(S3DSceneComposer::BinaryBezierCurvePoint) ;
}

// インスタンス・データ・パース
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::BinaryBezierCurveData *
	S3DItemBezierCurveSerializer::ParseData
		( SSystem::SArray<uint8_t>& aBinary,
				const wchar_t * pwszBase64, ssize_t nStrLenght )
{
	Charset::DecodeBase64( aBinary, pwszBase64, nStrLenght ) ;
	//
	if ( aBinary.GetLength() <
			sizeof(S3DSceneComposer::BinaryHeader)
			+ sizeof(S3DSceneComposer::BinaryBezierCurveData) )
	{
		return	nullptr ;
	}
	const uint8_t *	pBinary = aBinary.GetConstArray() ;
	const S3DSceneComposer::BinaryHeader *
		pBinHdr = (const S3DSceneComposer::BinaryHeader*) pBinary ;
	if ( pBinHdr->nType != S3DSceneComposer::binaryBezierCurve )
	{
		return	nullptr ;
	}
	const S3DSceneComposer::BinaryBezierCurveData *
		pBinBezier =
			(const S3DSceneComposer::BinaryBezierCurveData*)
				(pBinary + sizeof(S3DSceneComposer::BinaryHeader)) ;
	size_t	nCount = pBinBezier->count ;
	size_t	nDataBytes =
		offsetof(S3DSceneComposer::BinaryBezierCurveData,points[nCount]) ;
	if ( sizeof(S3DSceneComposer::BinaryHeader)
						+ nDataBytes > aBinary.GetLength() )
	{
		return	nullptr ;
	}
	return	(S3DSceneComposer::BinaryBezierCurveData*) pBinBezier ;
}

// インスタンス・データ・エンコード
//////////////////////////////////////////////////////////////////////////////
void S3DItemBezierCurveSerializer::EncodeData
	( SSystem::SString& strBase64, size_t nCount,
		const S3DSceneComposer::BinaryBezierCurvePoint * pbbcp )
{
	S3DSceneComposer::BinaryHeader	bhdr ;
	bhdr.nType = S3DSceneComposer::binaryBezierCurve ;
	bhdr.nSubType = 0 ;
	bhdr.nBodyBytes =
		(uint32_t) (sizeof(S3DSceneComposer::BinaryBezierCurveData)
					+ (nCount - 1)
						* sizeof(S3DSceneComposer::BinaryBezierCurvePoint)) ;
	//
	S3DSceneComposer::BinaryBezierCurveData	bbcd ;
	bbcd.count = (uint32_t) nCount ;
	bbcd.type = 0 ;
	bbcd.reserved[0] = 0 ;
	bbcd.reserved[1] = 0 ;
	//
	SArray<uint8_t>	bufTemp ;
	bufTemp.AddArray( (const uint8_t*) &bhdr, sizeof(bhdr) ) ;
	bufTemp.AddArray
		( (const uint8_t*) &bbcd,
			offsetof(S3DSceneComposer::BinaryBezierCurveData,points[0]) ) ;
	bufTemp.AddArray
		( (const uint8_t*) pbbcp,
			nCount * sizeof(S3DSceneComposer::BinaryBezierCurvePoint) ) ;
	//
	Charset::EncodeBase64
		( strBase64, bufTemp.GetConstArray(), bufTemp.GetLength() ) ;
}

void S3DItemBezierCurveSerializer::UpdateDataFromBase64( void )
{
	m_aBezierCurve.RemoveAll() ;
	m_flagModified = false ;
	//
	SArray<uint8_t>	aBinary ;
	S3DSceneComposer::BinaryBezierCurveData *
		pBinBezier =
			ParseData( aBinary, m_strDataBase64,
						(ssize_t) m_strDataBase64.GetLength() ) ;
	if ( pBinBezier == nullptr )
	{
		return ;
	}
	UpdateBezierCurve( pBinBezier->count, &(pBinBezier->points[0]) ) ;
}

void S3DItemBezierCurveSerializer::UpdateBezierCurve
	( size_t nCount, const S3DSceneComposer::BinaryBezierCurvePoint * pbbcp )
{
	m_aBezierCurve.SetLength( nCount ) ;
	eslCopyMemory
		( m_aBezierCurve.GetArray(), pbbcp,
			nCount * sizeof(S3DSceneComposer::BinaryBezierCurvePoint) ) ;
	m_aBezierCurve.FinishArray() ;
}

// 制御点取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DItemBezierCurveSerializer::GetPointCount( void ) const
{
	return	m_aBezierCurve.GetLength() ;
}

const S3DSceneComposer::BinaryBezierCurvePoint *
	S3DItemBezierCurveSerializer::GetPointAt( size_t i ) const
{
	return	m_aBezierCurve.GetAt( i ) ;
}

void S3DItemBezierCurveSerializer::SetPointAt
	( size_t i, const S3DSceneComposer::BinaryBezierCurvePoint& bbcp )
{
	ESLAssert( i < m_aBezierCurve.GetLength() ) ;
	if ( i < m_aBezierCurve.GetLength() )
	{
		m_aBezierCurve.SetAt( i, bbcp ) ;
	}
}

void S3DItemBezierCurveSerializer::InsertPointAt
	( size_t i, const S3DSceneComposer::BinaryBezierCurvePoint& bbcp )
{
	ESLAssert( i <= m_aBezierCurve.GetLength() ) ;
	if ( i <= m_aBezierCurve.GetLength() )
	{
		m_aBezierCurve.InsertAt( i, bbcp ) ;
	}
}

void S3DItemBezierCurveSerializer::RemovePointAt( size_t i )
{
	ESLAssert( i < m_aBezierCurve.GetLength() ) ;
	m_aBezierCurve.RemoveAt( i ) ;
}

// 編集フラグ設定
//////////////////////////////////////////////////////////////////////////////
void S3DItemBezierCurveSerializer::SetModified( void )
{
	m_flagModified = true ;
}



//////////////////////////////////////////////////////////////////////////////
// 周期動的インスタンス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DCyclicInstanceController, Controller, S3DInstancingEntryInterface )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DCyclicInstanceController, cyclic_instance )

const SSystem::SXMLDocument::AttrInteger
	S3DCyclicInstanceController::s_aiRotationAxisType[5] =
{
	{ L"axis_x", rotationAxisX },
	{ L"axis_y", rotationAxisY },
	{ L"axis_z", rotationAxisZ },
	{ L"vector", rotationAxisVector },
	{ nullptr, 0 },
} ;

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DCyclicInstanceController::S3DCyclicInstanceController( void )
	: Controller( m_ItemClassDescriptor.pwszClassID ),
		m_vPosition( 0, 0, 0 ), m_matRotation( 1, 1, 1 ), m_vZoom( 1, 1, 1 ),
		m_matInstance( 1, 1, 1, 1 ), m_clrInstance( 0xFFFFFFFF, 0 ),
		m_rxtType( rotationAxisX ),
		m_vRotAxis( 0, 0, 1 ), m_degRotInit( 0.0 ),
		m_dpsRotSpeed( 0.0 ), m_radRotation( 0.0 ),
		m_flagMoveTurn( true ), m_fpMoveInit( 0.0 ),
		m_secMoveDuration( 10.0 ), m_secMoving( 0.0 )
{
	m_flagsBehavior |= S3DSceneComposer::behaviorOnTimer ;
	//
	ESLVerify( paramPosition ==
		AddParameterEntry
			( L"position",
				S3DSceneComposer::typePosition,
				S3DSceneComposer::attrNoLocalTransform, L"位置" ) ) ;
	ESLVerify( paramRotation ==
		AddParameterEntry
			( L"rotation",
				S3DSceneComposer::typeRotation, 0, L"回転" ) ) ;
	ESLVerify( paramZoom ==
		AddParameterEntry
			( L"zoom",
				S3DSceneComposer::typeZoom, 0, L"拡大" ) ) ;
	ESLVerify( paramTransparency ==
		AddParameterEntry
			( L"transparency",
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrUIScalarSlider,
				L"透明度", NULL, 0.0, 1.0 ) ) ;
	ESLVerify( paramColorMul ==
		AddParameterEntry
			( L"color_mul",
				S3DSceneComposer::typeColor, 0, L"乗算色" ) ) ;
	ESLVerify( paramColorAdd ==
		AddParameterEntry
			( L"color_add",
				S3DSceneComposer::typeColor, 0, L"加算色" ) ) ;
	ESLVerify( paramRotationAxisType ==
		AddParameterEntry
			( L"rot_axis_type",
				S3DSceneComposer::typeSelector,
				S3DSceneComposer::attrConstant1
				| S3DSceneComposer::attrStringEnumeration
				| S3DSceneComposer::attrUIOnlyEnumeration
				| S3DSceneComposer::attrDynamicValidation, L"回転軸" ) ) ;
	ESLVerify( paramRotationAxisVector ==
		AddParameterEntry
			( L"rot_axis_vec",
				S3DSceneComposer::typeDirection,
				S3DSceneComposer::attrConstant1, L"回転ベクトル" ) ) ;
	ESLVerify( paramRotationInit ==
		AddParameterEntry
			( L"rot_angle0",
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrConstant1
				| S3DSceneComposer::attrUIScalarSlider,
				L"初期回転角", L"回転初期角 [deg]", -180.0, 180.0 ) ) ;
	ESLVerify( paramRotationSpeed ==
		AddParameterEntry
			( L"rot_speed",
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrConstant1
				| S3DSceneComposer::attrUIScalarSlider,
				L"回転速度", L"回転速度 [deg/sec]", -360.0, 360.0 ) ) ;
	ESLVerify( paramMovingLoopTurn ==
		AddParameterEntry
			( L"move_loop_turn",
				S3DSceneComposer::typeBoolean,
				S3DSceneComposer::attrConstant2, L"移動反転ループ" ) ) ;
	ESLVerify( paramMovingInit ==
		AddParameterEntry
			( L"move_init",
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrConstant2
				| S3DSceneComposer::attrUIScalarSlider,
				L"初期位置", nullptr, 0.0, 2.0 ) ) ;
	ESLVerify( paramLoopDuration ==
		AddParameterEntry
			( L"move_duration",
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrConstant2
				| S3DSceneComposer::attrUIScalarSlider,
				L"移動ループ時間", L"移動ループ時間 [sec]", 0.0, 60.0 ) ) ;
	ESLVerify( paramBezierCurve ==
		AddParameterEntry
			( L"path_curve",
				S3DSceneComposer::typeBinary,
				S3DSceneComposer::attrConstant2, L"移動曲線" ) ) ;
}

// インスタンシング・リスト取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DCyclicInstanceController::GetInstancingArray
	( const S4DMatrix*& pMatrixs, const S3DColor*& pColors ) const
{
	pMatrixs = &m_matInstance ;
	pColors = &m_clrInstance ;
	return	1 ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DCyclicInstanceController::GetMatrixParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRotation:
		return	m_matRotation ;
	}
	return	S3DDMatrix( 1, 1, 1 ) ;
}

S3DDVector S3DCyclicInstanceController::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramPosition:
		return	m_vPosition ;
	case	paramZoom:
		return	m_vZoom ;
	case	paramColorMul:
		return	VectorFromColor( m_clrInstance.rgbMul ) ;
	case	paramColorAdd:
		return	VectorFromColor( m_clrInstance.rgbAdd ) ;
	case	paramRotationAxisVector:
		return	m_vRotAxis ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DCyclicInstanceController::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramTransparency:
		return	1.0 - m_clrInstance.rgbMul.argb.Alpha / 255.0 ;
	case	paramRotationInit:
		return	m_degRotInit ;
	case	paramRotationSpeed:
		return	m_dpsRotSpeed ;
	case	paramMovingInit:
		return	m_fpMoveInit ;
	case	paramLoopDuration:
		return	m_secMoveDuration ;
	}
	return	0.0 ;
}

bool S3DCyclicInstanceController::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramMovingLoopTurn:
		return	m_flagMoveTurn;
	}
	return	false ;
}

const wchar_t * S3DCyclicInstanceController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRotationAxisType:
		return	SXMLDocument::GetSymbolAsIntegerOf( s_aiRotationAxisType, m_rxtType ) ;

	case	paramBezierCurve:
		return	((S3DItemBezierCurveSerializer*)&m_bezierPath)->GetBezierCurveBase64() ;
	}
	return	nullptr ;
}

size_t S3DCyclicInstanceController::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t i ) const
{
	if ( i == paramBezierCurve )
	{
		if ( pDst == nullptr )
		{
			return	sizeof(S3DSceneComposer::BinaryHeader)
							+ m_bezierPath.GetDataLengthInBytes() ;
		}
		if ( nBufBytes == sizeof(S3DSceneComposer::BinaryHeader) )
		{
			S3DSceneComposer::BinaryHeader *
				pbh = (S3DSceneComposer::BinaryHeader*) pDst ;
			pbh->nType = S3DSceneComposer::binaryBezierCurve ;
			pbh->nSubType = 0 ;
			pbh->nBodyBytes = (uint32_t) m_bezierPath.GetDataLengthInBytes() ;
			pbh->nReserved = 0 ;
			return	sizeof(S3DSceneComposer::BinaryHeader) ;
		}
		if ( nBufBytes == sizeof(S3DSceneComposer::BinaryHeader)
							+ m_bezierPath.GetDataLengthInBytes() )
		{
			S3DSceneComposer::BinaryHeader *
				pbh = (S3DSceneComposer::BinaryHeader*) pDst ;
			pbh->nType = S3DSceneComposer::binaryBezierCurve ;
			pbh->nSubType = 0 ;
			pbh->nBodyBytes = (uint32_t) m_bezierPath.GetDataLengthInBytes() ;
			pbh->nReserved = 0 ;
			//
			S3DSceneComposer::BinaryBezierCurveData *	pbbcd =
				(S3DSceneComposer::BinaryBezierCurveData*) pbh->GetBodyPtr() ;
			return	sizeof(S3DSceneComposer::BinaryHeader)
							+ m_bezierPath.GetBezierCurveData( *pbbcd ) ;
		}
	}
	return	0 ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DCyclicInstanceController::SetMatrixParameter( size_t i, const S3DDMatrix& mat )
{
	switch ( i )
	{
	case	paramRotation:
		m_matRotation = mat ;
		return ;
	}
}

void S3DCyclicInstanceController::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramPosition:
		m_vPosition = vec ;
		return ;
	case	paramZoom:
		m_vZoom = vec ;
		return ;
	case	paramColorMul:
		m_clrInstance.rgbMul.ui32 =
			(ColorFromVector( vec ).ui32 & 0x00FFFFFF)
				| (m_clrInstance.rgbMul.ui32 & 0xFF000000) ;
		return ;
	case	paramColorAdd:
		m_clrInstance.rgbAdd = ColorFromVector( vec ) ;
		return ;
	case	paramRotationAxisVector:
		m_vRotAxis = vec ;
		return ;
	}
}

void S3DCyclicInstanceController::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramTransparency:
		m_clrInstance.rgbMul.argb.Alpha =
			(uint8_t) esl_clampi( (int) esl_lroundfi( s * 255.0 ), 0, 255 ) ;
		return ;
	case	paramRotationInit:
		m_degRotInit = s ;
		return ;
	case	paramRotationSpeed:
		m_dpsRotSpeed = s ;
		return ;
	case	paramMovingInit:
		m_fpMoveInit = s ;
		return ;
	case	paramLoopDuration:
		m_secMoveDuration = s ;
		return ;
	}
}

void S3DCyclicInstanceController::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramMovingLoopTurn:
		m_flagMoveTurn = b ;
		return ;
	}
}

void S3DCyclicInstanceController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramRotationAxisType:
		m_rxtType = (RotationAxisType)
			SXMLDocument::GetIntegerAsSymbolOf( s_aiRotationAxisType, pwszCmd, m_rxtType ) ;
		return ;

	case	paramBezierCurve:
		m_bezierPath.SetBezierCurveBase64( pwszCmd ) ;
		return ;
	}
}

size_t S3DCyclicInstanceController::SetBinaryParameter
	( size_t i, const void * pSrc, size_t nBufBytes )
{
	if ( i == paramBezierCurve )
	{
		const S3DSceneComposer::BinaryHeader *
			pbh = (const S3DSceneComposer::BinaryHeader*) pSrc ;
		if ( pbh->nType == S3DSceneComposer::binaryBezierCurve )
		{
			const S3DSceneComposer::BinaryBezierCurveData *	pbbcd =
				(const S3DSceneComposer::BinaryBezierCurveData*) pbh->GetBodyPtr() ;
			m_bezierPath.SetBezierCurve( pbbcd->count, &(pbbcd->points[0]) ) ;
			//
			return	sizeof(S3DSceneComposer::BinaryHeader) + pbh->nBodyBytes ;
		}
	}
	return	0 ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DCyclicInstanceController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	size_t	j ;
	switch ( i )
	{
	case	paramRotationAxisType:
		for ( j = 0; s_aiRotationAxisType[j].pszSymbol != nullptr; j ++ )
		{
			aStrSet.Add( new SString( s_aiRotationAxisType[j].pszSymbol ) ) ;
		}
		return	true ;
	}
	return	false ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DCyclicInstanceController::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramRotationAxisVector:
		return	(m_rxtType == rotationAxisVector) ;
	}
	return	true ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DCyclicInstanceController::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本情報" ;
	case	1:
		return	L"回転" ;
	case	2:
		return	L"移動" ;
	}
	return	nullptr ;
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DCyclicInstanceController::OnTimer
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast )
{
	Controller::OnTimer( scene, pItem, msecPast ) ;

	//
	// 時間経過
	//
	double	secPast = (double) msecPast / 1000.0 ;
	//
	double	pi2 = 2.0 * PI ;
	double	rot = m_radRotation / pi2 + m_dpsRotSpeed * secPast / 360.0 ;
	rot -= floor( rot ) ;
	m_radRotation = rot * pi2 ;

	double	secLoop = m_secMoveDuration ;
	if ( m_flagMoveTurn )
	{
		secLoop *= 2.0 ;
	}
	m_secMoving += secPast ;
	if ( m_secMoving > secLoop )
	{
		if ( secLoop > 0.00001 )
		{
			m_secMoving -= floor( m_secMoving / secLoop ) * secLoop ;
		}
		else
		{
			m_secMoving = 0.0 ;
		}
	}

	//
	// 回転反映
	//
	S3DMatrix	matInstance =
					m_matRotation
					* S3DMatrix( m_vZoom.x, m_vZoom.y, m_vZoom.z ) ;
	S3DMatrix	matRot( 1, 1, 1 ) ;
	switch ( m_rxtType )
	{
	case	rotationAxisX:
		matRot.RevolveOnX( sin(m_radRotation), cos(m_radRotation) ) ;
		break ;
	case	rotationAxisY:
		matRot.RevolveOnY( sin(m_radRotation), cos(m_radRotation) ) ;
		break ;
	case	rotationAxisZ:
		matRot.RevolveOnZ( sin(m_radRotation), cos(m_radRotation) ) ;
		break ;
	case	rotationAxisVector:
		matRot.RotationOnVectorOf
			( m_vRotAxis, sin(m_radRotation), cos(m_radRotation) ) ;
		break ;
	}
	m_matInstance.SetMatrix3( matRot * matInstance ) ;

	//
	// 移動反映
	//
	const size_t	nCount = m_bezierPath.GetPointCount() ;
	if ( nCount >= 2 )
	{
		double	t = 0.0 ;
		if ( m_secMoveDuration > 0.00001 )
		{
			t = m_secMoving / m_secMoveDuration ;
			if ( m_flagMoveTurn && (t >= 1.0) )
			{
				t = 2.0 - t ;
			}
		}
		const size_t	nSegments = nCount - 1 ;
		t *= (double) nSegments ;
		//
		const size_t	iSeg = (size_t) floor( t ) ;
		t -= (double) iSeg ;
		//
		const S3DSceneComposer::BinaryBezierCurvePoint *
				pbcp0 = m_bezierPath.GetPointAt( iSeg ) ;
		const S3DSceneComposer::BinaryBezierCurvePoint *
				pbcp1 = m_bezierPath.GetPointAt( iSeg + 1 ) ;
		ESLAssert( pbcp0 != nullptr ) ;
		ESLAssert( pbcp1 != nullptr ) ;
		//
		SGLBezierCurves<S3DVector>	bzCurve ;
		bzCurve.SetLength( 4 ) ;
		bzCurve.SetAt( 0, pbcp0->vPoint ) ;
		bzCurve.SetAt( 1, pbcp0->vHandle[1] ) ;
		bzCurve.SetAt( 2, pbcp1->vHandle[0] ) ;
		bzCurve.SetAt( 3, pbcp1->vPoint ) ;
		//
		m_matInstance.SetTranslation( bzCurve.PointAt( t ) ) ;
	}
	else
	{
		m_matInstance.SetTranslation( m_vPosition ) ;
	}
}

// フレーム（パラメータ）更新後処理
//////////////////////////////////////////////////////////////////////////////
void S3DCyclicInstanceController::OnUpdateFrame
	( S3DSceneComposer::ItemSerializer * pItem,
		double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	Controller::OnUpdateFrame( pItem, fpFrame, seek ) ;

	if ( seek == S3DSceneComposer::seekJumpReset )
	{
		m_radRotation = m_degRotInit * PI / 180.0 ;
		m_secMoving = m_fpMoveInit ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// 周期動的行列コントローラー
//////////////////////////////////////////////////////////////////////////////

const SSystem::SXMLDocument::AttrInteger
	S3DCyclicMatrixController::s_aiRotationAxisType[5] =
{
	{ L"axis_x", rotationAxisX },
	{ L"axis_y", rotationAxisY },
	{ L"axis_z", rotationAxisZ },
	{ L"vector", rotationAxisVector },
	{ nullptr, 0 },
} ;

const SSystem::SXMLDocument::AttrInteger
	S3DCyclicMatrixController::s_aiDynamicRotation[4] =
{
	{ L"no", dynamicNoRotation },
	{ L"by_camera", dynamicRotationByCamera },
	{ L"along_tangent", dynamicRotationAlongTangent },
	{ nullptr, 0 },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DCyclicMatrixController, Controller )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DCyclicMatrixController, cyclic_matrix )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DCyclicMatrixController::S3DCyclicMatrixController( void )
	: Controller( m_ItemClassDescriptor.pwszClassID ),
		m_flagInstanceRotation( false ),
		m_vBasePosition( 0, 0, 0 ),
		m_matBaseRotation( 1, 1, 1 ),
		m_rxtType( rotationAxisY ),
		m_dynamicRotation( dynamicNoRotation ),
		m_vRotAxis( 0, 0, 1 ), m_degRotInit( 0.0 ),
		m_dpsRotSpeed( 0.0 ), m_radRotation( 0.0 ),
		m_flagMoveCurve( false ),
		m_flagMoveTurn( true ), m_flagMoveManual( false ), m_fpMoveInit( 0.0 ),
		m_secMoveDuration( 10.0 ), m_secMoving( 0.0 ), m_vLastTangent( 0, 0, 1 )
{
	m_flagsBehavior |= S3DSceneComposer::behaviorOnTimer ;
	//
	ESLVerify( paramRotationAxisType ==
		AddParameterEntry
			( L"rot_axis_type",
				S3DSceneComposer::typeSelector,
				S3DSceneComposer::attrConstant1
				| S3DSceneComposer::attrStringEnumeration
				| S3DSceneComposer::attrUIOnlyEnumeration
				| S3DSceneComposer::attrDynamicValidation, L"回転軸" ) ) ;
	ESLVerify( paramDynamicRotation ==
		AddParameterEntry
			( L"dynamic_rotation",
				S3DSceneComposer::typeSelector,
				S3DSceneComposer::attrConstant1
				| S3DSceneComposer::attrStringEnumeration
				| S3DSceneComposer::attrUIOnlyEnumeration, L"補助回転" ) ) ;
	ESLVerify( paramRotationAxisVector ==
		AddParameterEntry
			( L"rot_axis_vec",
				S3DSceneComposer::typeDirection,
				S3DSceneComposer::attrConstant1, L"回転ベクトル" ) ) ;
	ESLVerify( paramBaseRotation ==
		AddParameterEntry
			( L"base_rotate",
				S3DSceneComposer::typeRotation,
				S3DSceneComposer::attrCategory1
				| S3DSceneComposer::attrNoLocalTransform, L"ベース回転" ) ) ;
	ESLVerify( paramInstanceRotation ==
		AddParameterEntry
			( L"instance_rot",
				S3DSceneComposer::typeBoolean,
				S3DSceneComposer::attrConstant1, L"インスタンス回転" ) ) ;
	ESLVerify( paramRotationInit ==
		AddParameterEntry
			( L"rot_angle0",
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrConstant1
				| S3DSceneComposer::attrUIScalarSlider,
				L"初期回転角", L"回転初期角 [deg]", -180.0, 180.0 ) ) ;
	ESLVerify( paramRotationSpeed ==
		AddParameterEntry
			( L"rot_speed",
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrConstant1
				| S3DSceneComposer::attrUIScalarSlider,
				L"回転速度", L"回転速度 [deg/sec]", -360.0, 360.0 ) ) ;
	ESLVerify( paramBasePosition ==
		AddParameterEntry
			( L"base_position",
				S3DSceneComposer::typePosition,
				S3DSceneComposer::attrCategory2
				| S3DSceneComposer::attrNoLocalTransform, L"ベース座標" ) ) ;
	ESLVerify( paramMovingCurve ==
		AddParameterEntry
			( L"move_curve",
				S3DSceneComposer::typeBoolean,
				S3DSceneComposer::attrConstant2
				| S3DSceneComposer::attrDynamicValidation, L"移動曲線指定" ) ) ;
	ESLVerify( paramMovingLoopTurn ==
		AddParameterEntry
			( L"move_loop_turn",
				S3DSceneComposer::typeBoolean,
				S3DSceneComposer::attrConstant2, L"移動反転ループ" ) ) ;
	ESLVerify( paramMovingManual ==
		AddParameterEntry
			( L"move_manual",
				S3DSceneComposer::typeBoolean,
				S3DSceneComposer::attrConstant2, L"移動位置指定" ) ) ;
	ESLVerify( paramMovingInit ==
		AddParameterEntry
			( L"move_pos",
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrUIScalarSlider,
				L"初期位置",
				L"位置（0.0～1.0）。反転ループの場合には 0.0～2.0 範囲。\n"
				L"マニュアル移動でない場合には初期位置。", 0.0, 2.0 ) ) ;
	ESLVerify( paramLoopDuration ==
		AddParameterEntry
			( L"move_duration",
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrConstant2
				| S3DSceneComposer::attrUIScalarSlider,
				L"移動ループ時間", L"移動ループ時間 [sec]", 0.0, 60.0 ) ) ;
	ESLVerify( paramBezierCurve ==
		AddParameterEntry
			( L"path_curve",
				S3DSceneComposer::typeBinary,
				S3DSceneComposer::attrConstant2
				| S3DSceneComposer::attrNoLocalTransform, L"移動曲線" ) ) ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DCyclicMatrixController::GetMatrixParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramBaseRotation:
		return	m_matBaseRotation ;
	}
	return	S3DDMatrix( 1, 1, 1 ) ;
}

S3DDVector S3DCyclicMatrixController::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRotationAxisVector:
		return	m_vRotAxis ;
	case	paramBasePosition:
		return	m_vBasePosition ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DCyclicMatrixController::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRotationInit:
		return	m_degRotInit ;
	case	paramRotationSpeed:
		return	m_dpsRotSpeed ;
	case	paramMovingInit:
		return	m_fpMoveInit ;
	case	paramLoopDuration:
		return	m_secMoveDuration ;
	}
	return	0.0 ;
}

bool S3DCyclicMatrixController::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramInstanceRotation:
		return	m_flagInstanceRotation ;
	case	paramMovingCurve:
		return	m_flagMoveCurve ;
	case	paramMovingLoopTurn:
		return	m_flagMoveTurn;
	case	paramMovingManual:
		return	m_flagMoveManual ;
	}
	return	false ;
}

const wchar_t * S3DCyclicMatrixController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRotationAxisType:
		return	SXMLDocument::GetSymbolAsIntegerOf( s_aiRotationAxisType, m_rxtType ) ;

	case	paramDynamicRotation:
		return	SXMLDocument::GetSymbolAsIntegerOf( s_aiDynamicRotation, m_dynamicRotation ) ;

	case	paramBezierCurve:
		return	((S3DItemBezierCurveSerializer*)&m_bezierPath)->GetBezierCurveBase64() ;
	}
	return	nullptr ;
}

size_t S3DCyclicMatrixController::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t i ) const
{
	if ( i == paramBezierCurve )
	{
		if ( pDst == nullptr )
		{
			return	sizeof(S3DSceneComposer::BinaryHeader)
							+ m_bezierPath.GetDataLengthInBytes() ;
		}
		if ( nBufBytes == sizeof(S3DSceneComposer::BinaryHeader) )
		{
			S3DSceneComposer::BinaryHeader *
				pbh = (S3DSceneComposer::BinaryHeader*) pDst ;
			pbh->nType = S3DSceneComposer::binaryBezierCurve ;
			pbh->nSubType = 0 ;
			pbh->nBodyBytes = (uint32_t) m_bezierPath.GetDataLengthInBytes() ;
			pbh->nReserved = 0 ;
			return	sizeof(S3DSceneComposer::BinaryHeader) ;
		}
		if ( nBufBytes == sizeof(S3DSceneComposer::BinaryHeader)
							+ m_bezierPath.GetDataLengthInBytes() )
		{
			S3DSceneComposer::BinaryHeader *
				pbh = (S3DSceneComposer::BinaryHeader*) pDst ;
			pbh->nType = S3DSceneComposer::binaryBezierCurve ;
			pbh->nSubType = 0 ;
			pbh->nBodyBytes = (uint32_t) m_bezierPath.GetDataLengthInBytes() ;
			pbh->nReserved = 0 ;
			//
			S3DSceneComposer::BinaryBezierCurveData *	pbbcd =
				(S3DSceneComposer::BinaryBezierCurveData*) pbh->GetBodyPtr() ;
			return	sizeof(S3DSceneComposer::BinaryHeader)
							+ m_bezierPath.GetBezierCurveData( *pbbcd ) ;
		}
	}
	return	0 ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DCyclicMatrixController::SetMatrixParameter( size_t i, const S3DDMatrix& mat )
{
	switch ( i )
	{
	case	paramBaseRotation:
		m_matBaseRotation = mat ;
		return ;
	}
}

void S3DCyclicMatrixController::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramRotationAxisVector:
		m_vRotAxis = vec ;
		return ;
	case	paramBasePosition:
		m_vBasePosition = vec ;
		return ;
	}
}

void S3DCyclicMatrixController::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramRotationInit:
		m_degRotInit = s ;
		return ;
	case	paramRotationSpeed:
		m_dpsRotSpeed = s ;
		return ;
	case	paramMovingInit:
		m_fpMoveInit = s ;
		return ;
	case	paramLoopDuration:
		m_secMoveDuration = s ;
		return ;
	}
}

void S3DCyclicMatrixController::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramInstanceRotation:
		m_flagInstanceRotation = b ;
		return ;
	case	paramMovingCurve:
		m_flagMoveCurve = b ;
		return ;
	case	paramMovingLoopTurn:
		m_flagMoveTurn = b ;
		return ;
	case	paramMovingManual:
		m_flagMoveManual = b ;
		return ;
	}
}

void S3DCyclicMatrixController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramRotationAxisType:
		m_rxtType = (RotationAxisType)
			SXMLDocument::GetIntegerAsSymbolOf
				( s_aiRotationAxisType, pwszCmd, m_rxtType ) ;
		return ;

	case	paramDynamicRotation:
		m_dynamicRotation = (DynamicRotation)
			SXMLDocument::GetIntegerAsSymbolOf
				( s_aiDynamicRotation, pwszCmd, m_dynamicRotation ) ;
		return ;

	case	paramBezierCurve:
		m_bezierPath.SetBezierCurveBase64( pwszCmd ) ;
		return ;
	}
}

size_t S3DCyclicMatrixController::SetBinaryParameter
	( size_t i, const void * pSrc, size_t nBufBytes )
{
	if ( i == paramBezierCurve )
	{
		const S3DSceneComposer::BinaryHeader *
			pbh = (const S3DSceneComposer::BinaryHeader*) pSrc ;
		if ( pbh->nType == S3DSceneComposer::binaryBezierCurve )
		{
			const S3DSceneComposer::BinaryBezierCurveData *	pbbcd =
				(const S3DSceneComposer::BinaryBezierCurveData*) pbh->GetBodyPtr() ;
			m_bezierPath.SetBezierCurve( pbbcd->count, &(pbbcd->points[0]) ) ;
			//
			return	sizeof(S3DSceneComposer::BinaryHeader) + pbh->nBodyBytes ;
		}
	}
	return	0 ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DCyclicMatrixController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	size_t	j ;
	switch ( i )
	{
	case	paramRotationAxisType:
		for ( j = 0; s_aiRotationAxisType[j].pszSymbol != nullptr; j ++ )
		{
			aStrSet.Add( new SString( s_aiRotationAxisType[j].pszSymbol ) ) ;
		}
		return	true ;

	case	paramDynamicRotation:
		for ( j = 0; s_aiDynamicRotation[j].pszSymbol != nullptr; j ++ )
		{
			aStrSet.Add( new SString( s_aiDynamicRotation[j].pszSymbol ) ) ;
		}
		return	true ;
	}
	return	false ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DCyclicMatrixController::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramRotationAxisVector:
		return	(m_rxtType == rotationAxisVector) ;

	case	paramMovingLoopTurn:
	case	paramMovingInit:
	case	paramLoopDuration:
	case	paramBezierCurve:
		return	m_flagMoveCurve ;
	}
	return	true ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DCyclicMatrixController::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本情報" ;
	case	1:
		return	L"回転" ;
	case	2:
		return	L"移動" ;
	}
	return	nullptr ;
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DCyclicMatrixController::OnTimer
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast )
{
	Controller::OnTimer( scene, pItem, msecPast ) ;

	//
	// 時間経過
	//
	double	secPast = (double) msecPast / 1000.0 ;
	//
	double	pi2 = 2.0 * PI ;
	double	rot = m_radRotation / pi2 + m_dpsRotSpeed * secPast / 360.0 ;
	rot -= floor( rot ) ;
	m_radRotation = rot * pi2 ;

	if ( !m_flagMoveManual )
	{
		double	secLoop = m_secMoveDuration ;
		if ( m_flagMoveTurn )
		{
			secLoop *= 2.0 ;
		}
		m_secMoving += secPast ;
		if ( m_secMoving > secLoop )
		{
			if ( secLoop > 0.00001 )
			{
				m_secMoving -= floor( m_secMoving / secLoop ) * secLoop ;
			}
			else
			{
				m_secMoving = 0.0 ;
			}
		}
	}

	//
	// パラメータ反映
	//
	S3DSceneComposer::CommonSerializer *
		pCmnSer = ESLTypeCast<S3DSceneComposer::CommonSerializer>( pItem ) ;
	if ( pCmnSer != nullptr )
	{
		ApplyParameters( &scene, pCmnSer ) ;
	}
}

// フレーム（パラメータ）更新後処理
//////////////////////////////////////////////////////////////////////////////
void S3DCyclicMatrixController::OnUpdateFrame
	( S3DSceneComposer::ItemSerializer * pItem,
		double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	Controller::OnUpdateFrame( pItem, fpFrame, seek ) ;

	if ( seek == S3DSceneComposer::seekJumpReset )
	{
		m_radRotation = m_degRotInit * PI / 180.0 ;
		m_secMoving = m_secMoveDuration * m_fpMoveInit ;

		S3DSceneComposer::CommonSerializer *
			pCmnSer = ESLTypeCast<S3DSceneComposer::CommonSerializer>( pItem ) ;
		if ( pCmnSer != nullptr )
		{
//			m_vBasePosition = pCmnSer->GetItemPosition() ;
//			m_matBaseRotation = pCmnSer->GetItemRotation() ;
			//
			ApplyParameters( pItem->GetScene(), pCmnSer ) ;
		}
	}
	else if ( m_flagMoveManual )
	{
		m_secMoving = m_secMoveDuration * m_fpMoveInit ;
	}
}

// パラメータ反映
//////////////////////////////////////////////////////////////////////////////
void S3DCyclicMatrixController::ApplyParameters
	( S3DScene * pScene, S3DSceneComposer::CommonSerializer * pItem )
{
	//
	// 移動反映
	//
	const size_t	nCount = m_bezierPath.GetPointCount() ;
	S3DDVector		vTangent = m_vLastTangent ;
	if ( nCount >= 2 )
	{
		double	t = 0.0 ;
		if ( m_secMoveDuration > 0.00001 )
		{
			t = m_secMoving / m_secMoveDuration ;
			if ( m_flagMoveTurn && (t >= 1.0) )
			{
				t = 2.0 - t ;
			}
		}
		const size_t	nSegments = nCount - 1 ;
		t *= (double) nSegments ;
		//
		const size_t	iSeg = (size_t) floor( t ) ;
		t -= (double) iSeg ;
		//
		const S3DSceneComposer::BinaryBezierCurvePoint *
				pbcp0 = m_bezierPath.GetPointAt( iSeg ) ;
		const S3DSceneComposer::BinaryBezierCurvePoint *
				pbcp1 = m_bezierPath.GetPointAt( iSeg + 1 ) ;
		ESLAssert( pbcp0 != nullptr ) ;
		ESLAssert( pbcp1 != nullptr ) ;
		//
		SGLBezierCurves<S3DVector>	bzCurve ;
		bzCurve.SetLength( 4 ) ;
		bzCurve.SetAt( 0, pbcp0->vPoint ) ;
		bzCurve.SetAt( 1, pbcp0->vHandle[1] ) ;
		bzCurve.SetAt( 2, pbcp1->vHandle[0] ) ;
		bzCurve.SetAt( 3, pbcp1->vPoint ) ;
		//
		pItem->SetItemPositioin( m_vBasePosition + S3DDVector(bzCurve.PointAt( t )) ) ;
		//
		if ( m_dynamicRotation == dynamicRotationAlongTangent )
		{
			S3DVector	vT ;
			bzCurve.Tangent( vT, t, 0 ) ;
			if ( vT.Absolute() > 0.00001 )
			{
				vTangent = vT ;
				m_vLastTangent = vTangent ;
			}
		}
	}
	//
	// 回転行列
	//
	S3DDMatrix	matRot( 1, 1, 1 ) ;
	switch ( m_rxtType )
	{
	case	rotationAxisX:
		matRot.RevolveOnX( sin(m_radRotation), cos(m_radRotation) ) ;
		break ;
	case	rotationAxisY:
		matRot.RevolveOnY( sin(m_radRotation), cos(m_radRotation) ) ;
		break ;
	case	rotationAxisZ:
		matRot.RevolveOnZ( sin(m_radRotation), cos(m_radRotation) ) ;
		break ;
	case	rotationAxisVector:
		matRot.RotationOnVectorOf
			( m_vRotAxis, sin(m_radRotation), cos(m_radRotation) ) ;
		break ;
	}
	//
	// 補助回転
	//
	bool	flagTrangent = false ;
	if ( m_dynamicRotation == dynamicRotationByCamera )
	{
		S3DDMatrix	matItem ;
		S3DDVector	vItem ;
		pItem->GetGlobalTransformation( matItem, vItem ) ;
		//
		S3DDVector	vCamera( 0, 0, 0 ) ;
		if ( pScene != nullptr )
		{
			vCamera = pScene->GetCurrentCameraPosition() ;
		}
		//
		S3DDMatrix	matLink ;
		S3DDVector	vLink ;
		pItem->GetItemLinkTransformation( matLink, vLink ) ;
		//
		vTangent = matLink.Inverse() * (vItem - vCamera ) ;
		flagTrangent = true ;
	}
	else if ( m_dynamicRotation == dynamicRotationAlongTangent )
	{
		flagTrangent = true ;
	}
	if ( flagTrangent )
	{
		S3DDVector	vBase( 0, 0, 1 ) ;
		switch ( m_rxtType )
		{
		case	rotationAxisX:
			vTangent.x = 0 ;
			break ;
		case	rotationAxisY:
			vTangent.y = 0 ;
			break ;
		case	rotationAxisZ:
			vTangent.z = 0 ;
			vBase = S3DDVector( 1, 0, 0 ) ;
			break ;
		case	rotationAxisVector:
			{
				S3DDVector	vRotAxis = m_vRotAxis.Normalized() ;
				vTangent -= vRotAxis * vRotAxis.InnerProduct( vTangent ) ;
				vBase -= vRotAxis * vRotAxis.InnerProduct( vBase ) ;
			}
			break ;
		}
		if ( (vTangent.Absolute() > 0.00001) && (vBase.Absolute() > 0.00001) )
		{
			S3DDMatrix	matDynRot( 1, 1, 1 ) ;
			matDynRot.VectorRotationOf( vBase, vTangent ) ;
			matRot = matRot * matDynRot ;
		}
	}
	//
	// 回転反映
	//
	matRot = matRot * m_matBaseRotation ;
	//
	if ( m_flagInstanceRotation )
	{
		S3DInstancingItemInterface *
				pInstancing = ESLTypeCast<S3DInstancingItemInterface>( pItem ) ;
		if ( pInstancing != nullptr )
		{
			S3DItemInstancingSerializer *	pInstancingSer = pInstancing->GetInstancing() ;
			if ( pInstancingSer != nullptr )
			{
				pInstancingSer->Lock() ;
				pInstancingSer->SetBaseRotation( S3DDQuaternion( matRot ) ) ;
				pInstancingSer->Unlock() ;
			}
		}
	}
	else
	{
		pItem->SetItemRotation( matRot ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// パス適用モデル生成
//////////////////////////////////////////////////////////////////////////////

// ベジェ曲線の線分化
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBezierPathBuilder::SegmentCurve::PrepareCruve
	( const S3DItemBezierCurveSerializer& bezier, double fpUnitLength )
{
	const size_t	nDivCount = 64 ;
	size_t	iBezier = 0 ;
	double	zCource = 0.0 ;
	//
	SGLBezierCurves<S3DVector>	bzCurve ;
	bzCurve.SetLength( 4 ) ;
	//
	m_aSegPoints.RemoveAll() ;
	m_aSegPoints.SetLimit( (bezier.GetPointCount() - 1) * nDivCount + 1 ) ;
	//
	m_fpUnitLength = fpUnitLength ;
	//
	S3DMatrix	matLastRot( 1, 1, 1 ) ;
	S3DVector	vLastTangent( 0, 0, 1 ) ;
	//
	for ( size_t iBezier = 0;
			iBezier + 1 < bezier.GetPointCount(); iBezier ++ )
	{
		const S3DSceneComposer::BinaryBezierCurvePoint *
							pbbcp0 = bezier.GetPointAt( iBezier ) ;
		const S3DSceneComposer::BinaryBezierCurvePoint *
							pbbcp1 = bezier.GetPointAt( iBezier + 1 ) ;
		ESLAssert( pbbcp0 != nullptr ) ;
		ESLAssert( pbbcp1 != nullptr ) ;
		//
		bzCurve.At(0) = pbbcp0->vPoint ;
		bzCurve.At(1) = pbbcp0->vHandle[1] ;
		bzCurve.At(2) = pbbcp1->vHandle[0] ;
		bzCurve.At(3) = pbbcp1->vPoint ;
		//
		for ( size_t i = ((iBezier == 0) ? 0 : 1); i <= nDivCount; i ++ )
		{
			SegmentPoint	sp ;
			double	t = (double) i / (double) nDivCount ;
			double	nt = 1.0 - t ;
			bzCurve.PointAt( sp.m_vPoint, t, 0 ) ;
			sp.m_vZoom = pbbcp0->vZoom * nt + pbbcp1->vZoom * t ;
			sp.m_vZoom.z = esl_fmaxf( (float32_t) fabs( sp.m_vZoom.z ), 0.00001f ) ;
			sp.m_radGimbal = (pbbcp0->degGimbal * (float32_t) nt
								+ pbbcp1->degGimbal * (float32_t) t)
							* (float32_t) (PI / 180.0) ;
			sp.m_argbColor = pbbcp0->argbColor * nt + pbbcp1->argbColor * t ;
			sp.m_nExData = (i < nDivCount) ? pbbcp0->nExData : pbbcp1->nExData ;
			//
			if ( m_aSegPoints.GetLength() >= 1 )
			{
				SegmentPoint *	pspLast = m_aSegPoints.GetLastAt() ;
				ESLAssert( pspLast != nullptr ) ;
				S3DVector	vDelta = sp.m_vPoint - pspLast->m_vPoint ;
				pspLast->m_zLength = vDelta.Absolute() ;
				zCource += pspLast->m_zLength ;
				//
				vDelta.Normalize() ;
				pspLast->m_vAxisZ = vDelta ;
			}
			//
			double	unitCourse = zCource / m_fpUnitLength ;
			sp.m_zAccCourse = zCource ;
			sp.m_zLength = 0.0 ;
			sp.m_nUnit = (size_t) floor( unitCourse ) ;
			sp.m_zCourse = unitCourse - (double) sp.m_nUnit * m_fpUnitLength ;
			//
			S3DVector	vTangent ;
			bzCurve.Tangent( vTangent, t, 0 ) ;
			if ( vTangent.Absolute() < 0.00001 )
			{
				vTangent = vLastTangent ;
			}
			//
			S3DMatrix	matRotate( 1, 1, 1 ) ;
			if ( i == 0 )
			{
				matRotate.RevolveForAngle( vTangent ) ;
			}
			else
			{
				S3DMatrix	matDelta( 1, 1, 1 ) ;
				matDelta.VectorRotationOf( vLastTangent, vTangent ) ;
				matRotate = matDelta * matLastRot ;
			}
			vLastTangent = vTangent ;
			matLastRot = matRotate ;
			//
			matRotate.RevolveOnZ( sin(sp.m_radGimbal), cos(sp.m_radGimbal) ) ;
			sp.m_qRotate.FromMatrix( matRotate ) ;
			sp.m_vAxisZ = vTangent.Normalized() ;
			//
			m_aSegPoints.Add( sp ) ;
		}
	}
	m_zCource = zCource ;
}

// 参照元メッシュ情報クリア
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBezierPathBuilder::ClearRefMesh( void )
{
	m_aMeshInfo.RemoveAll() ;
}

// 参照元メッシュ設定
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshBezierPathBuilder::SetupRefMeshAt
	( size_t nIndex, const S3DModelBuffer& model, size_t iMesh )
{
	S3DModelData::MeshObject *	pmo = model.GetMeshObjectAt( iMesh ) ;
	if ( pmo == nullptr )
	{
		return	false ;
	}
	MeshInfo *	pmi = new MeshInfo ;
	pmi->m_pmoMesh = pmo ;
	pmi->m_pvVertex = model.GetVertexBufferAt( pmo->m_iVertex ) ;
	pmi->m_pvNormal = model.GetNormalBufferAt( pmo->m_iNormal ) ;
	pmi->m_zMin = 0.0f ;
	pmi->m_zMax = 0.0f ;
	//
	const S3DVector4 *	pvVertex = pmi->m_pvVertex ;
	const size_t		nCount = pmo->m_countVertex ;
	size_t *			pOrder = pmi->m_aVertexOrder.GetArray( nCount ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pOrder[i] = i ;
	}
	for ( size_t i = 0; i < nCount; i ++ )
	{
		size_t		iMin = i ;
		float32_t	zMin = pvVertex[pOrder[i]].z ;
		for ( size_t j = i + 1; j < nCount; j ++ )
		{
			float32_t	z = pvVertex[pOrder[j]].z ;
			if ( z < zMin )
			{
				iMin = j ;
				zMin = z ;
			}
		}
		size_t	k = pOrder[i] ;
		pOrder[i] = pOrder[iMin] ;
		pOrder[iMin] = k ;
	}
	if ( nCount > 0 )
	{
		pmi->m_zMin = pvVertex[pOrder[0]].z ;
		pmi->m_zMax = pvVertex[pOrder[nCount - 1]].z ;
	}
	pmi->m_aVertexOrder.FinishArray() ;
	//
	m_aMeshInfo.SetAt( nIndex, pmi ) ;
	return	true ;
}

void S3DMeshBezierPathBuilder::ClearRefMeshAt( size_t nIndex )
{
	m_aMeshInfo.SetAt( nIndex, nullptr ) ;
}

// メッシュ生成
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBezierPathBuilder::BuildMesh
	( S3DVertexBufferInterface & vbDst, const SegmentCurve& curve )
{
	const SegmentPoint *	pspPoints = curve.m_aSegPoints.GetConstArray() ;
	const size_t			nPointCount = curve.m_aSegPoints.GetLength() ;
	size_t					iCurPoint = 0 ;
	const SegmentPoint *	pspCurPoint = pspPoints ;
	//
	double	zPos = 0.0 ;
	while ( zPos < curve.m_zCource )
	{
		//
		// コース上の次の位置を検索
		//
		ESLAssert( pspCurPoint != nullptr ) ;
		const SegmentPoint *	pspNextPoint = pspCurPoint ;
		size_t					iNextPoint = iCurPoint ;
		double					zNextPos = zPos ;
		double					zBasePosition = zPos ;
		double					zLeftUnit = curve.m_fpUnitLength ;
		uint32_t				nExData = pspCurPoint->m_nExData ;
		//
		while ( (iNextPoint + 1 < nPointCount)
			&& (pspNextPoint->m_zLength
				- (zNextPos - pspNextPoint->m_zAccCourse)
							< zLeftUnit * pspNextPoint->m_vZoom.z) )
		{
			double	zLeft = pspNextPoint->m_zLength
							- (zNextPos - pspNextPoint->m_zAccCourse) ;
			zNextPos += zLeft ;
			zLeftUnit -= zLeft / pspNextPoint->m_vZoom.z ;
			if ( iNextPoint + 1 >= nPointCount )
			{
				break ;
			}
			pspNextPoint = pspPoints + (++ iNextPoint) ;
			nExData = pspNextPoint->m_nExData ;
		}
		zNextPos += zLeftUnit * pspNextPoint->m_vZoom.z ;
		//
		// メッシュデータ取得
		//
		MeshInfo *	pmi = nullptr ;
		if ( nExData < m_aMeshInfo.GetLength() )
		{
			pmi = m_aMeshInfo.GetAt( nExData ) ;
		}
		/*
		if ( zNextPos < curve.m_zCource )
		{
			if ( pspCurPoint->m_nExData < m_aMeshInfo.GetLength() )
			{
				pmi = m_aMeshInfo.GetAt( pspCurPoint->m_nExData ) ;
			}
		}
		else
		{
			if ( pspNextPoint->m_nExData < m_aMeshInfo.GetLength() )
			{
				pmi = m_aMeshInfo.GetAt( pspNextPoint->m_nExData ) ;
			}
		}
		*/
		if ( pmi == nullptr )
		{
			zPos = zNextPos ;
			iCurPoint = iNextPoint ;
			pspCurPoint = pspNextPoint ;
			continue ;
		}
		S3DModelData::MeshObject *	pmo = pmi->m_pmoMesh ;
		ESLAssert( pmo != nullptr ) ;
		//
		// 開始位置検索
		//
		double	zOffset = pmi->m_zMin ;
		if ( pmi->m_zMin < 0.0 )
		{
			double	zBack = - pmi->m_zMin ;
			while ( (iCurPoint > 0)
				&& (zBasePosition - pspCurPoint->m_zAccCourse
								< zBack * pspCurPoint->m_vZoom.z) )
			{
				zBack -= (zBasePosition - pspCurPoint->m_zAccCourse)
											/ pspCurPoint->m_vZoom.z ;
				zBasePosition = pspCurPoint->m_zAccCourse ;
				pspCurPoint = pspPoints + (-- iCurPoint) ;
			}
			zBasePosition -= zBack * pspCurPoint->m_vZoom.z ;
		}
		else
		{
			zOffset = 0.0 ;
		}
		//
		// バッファ確保
		//
		S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
		vbDst.AllocatePrimitiveBuffer
			( prmbuf, primitiveTriangle,
				pmo->m_countPolygon * 3, pmo->m_countVertex ) ;
		//
		// 頂点と法線処理
		//
		const SegmentPoint *	pspNext = pspCurPoint ;
		if ( iCurPoint + 1 < nPointCount )
		{
			pspNext ++ ;
		}
		S3DMatrix		matRotate ;
		const size_t *	pOrder = pmi->m_aVertexOrder.GetConstArray() ;
		float32_t		zLastVertex = pmi->m_zMin ;
		ESLAssert( pmi->m_aVertexOrder.GetLength() >= pmo->m_countVertex ) ;
		for ( size_t i = 0; i < pmo->m_countVertex; i ++ )
		{
			const size_t		iVert = pOrder[i] ;
			const S3DVector4 *	pvVertex = pmi->m_pvVertex + iVert ;
			const S3DVector4 *	pvNormal = pmi->m_pvNormal + iVert ;
			//
			// 適用区間移動
			//
			double	zVertPosition =
						zBasePosition
							+ (pvVertex->z - zOffset) * pspCurPoint->m_vZoom.z ;
			while ( (pspNext->m_zAccCourse < zVertPosition)
							&& (iCurPoint + 1 < nPointCount) )
			{
				double	zDelta = pspNext->m_zAccCourse - zBasePosition ;
				zOffset += zDelta / pspCurPoint->m_vZoom.z ;
				zBasePosition = pspNext->m_zAccCourse ;
				//
				pspCurPoint = pspPoints + (++ iCurPoint) ;
				pspNext = pspCurPoint ;
				if ( iCurPoint + 1 < nPointCount )
				{
					pspNext ++ ;
				}
				zVertPosition =
					zBasePosition
						+ (pvVertex->z - zOffset) * pspCurPoint->m_vZoom.z ;
			}
			//
			// 適用処理
			//
			double	zDelta = zVertPosition - pspCurPoint->m_zAccCourse ;
			double	t = (pspCurPoint->m_zLength != 0.0)
							? zDelta / pspCurPoint->m_zLength : 0.0 ;
			//
			S3DQuaternion	qRotate ;
			qRotate.Lerp
				( pspCurPoint->m_qRotate,
					pspNext->m_qRotate, (float32_t) t ) ;
			qRotate.ToMatrix( matRotate ) ;
			//
			S3DVector	vZoom = pspCurPoint->m_vZoom * (1.0 - t)
												+ pspNext->m_vZoom * t ;
			//
			S3DVector	vSrcPos( pvVertex->x * vZoom.x, pvVertex->y * vZoom.y, 0.0f ) ;
			S3DVector4	vVertex = matRotate * vSrcPos ;
			S3DVector4	vNormal = matRotate * *pvNormal ;
			vVertex += pspCurPoint->m_vPoint ;
			vVertex += pspCurPoint->m_vAxisZ * zDelta ;
			//
			prmbuf.pvVertex[iVert] = vVertex ;
			prmbuf.pvNormal[iVert] = vNormal ;
		}
		//
		// UVマップ複製
		//
		size_t	nUVMapLength =
					(size_t) esl_min( (int) pmo->m_bufUVMap.GetLength(),
										(int) pmo->m_countVertex ) ; ;
		eslCopyMemory
			( prmbuf.pvUVMap,
				pmo->m_bufUVMap.GetConstArray(),
				nUVMapLength * sizeof(S2DVector) ) ;
		eslFillMemory
			( prmbuf.pvUVMap + nUVMapLength, 0,
				(pmo->m_countVertex - nUVMapLength) * sizeof(S2DVector) ) ;
		//
		// 頂点色複製
		//
		SGLPalette	argbColor = pspCurPoint->m_argbColor ;
		size_t		nColorMapLength =
						(size_t) esl_min( (int) pmo->m_bufColor.GetLength(),
											(int) pmo->m_countVertex ) ; ;
		S3DColor *	pDstColor = prmbuf.pColor ;
		const S3DColor *
					pSrcColor = pmo->m_bufColor.GetConstArray() ;
		//
		for ( size_t i = 0; i < nColorMapLength; i ++ )
		{
			pDstColor[i].rgbMul = pSrcColor[i].rgbMul * argbColor ;
			pDstColor[i].rgbAdd = pSrcColor[i].rgbAdd * argbColor ;
		}
		for ( size_t i = nColorMapLength; i < pmo->m_countVertex; i ++ )
		{
			pDstColor[i].rgbMul = argbColor ;
			pDstColor[i].rgbAdd.ui32 = 0 ;
		}
		//
		// インデックスリスト複製
		//
		size_t	nIndexLength =
					(size_t) esl_min( (int) pmo->m_bufIndex.GetLength(),
										(int) pmo->m_countPolygon * 3 ) ;
		eslCopyMemory
			( prmbuf.pIndexedList,
				pmo->m_bufIndex.GetConstArray(),
				nIndexLength * sizeof(uint32_t) ) ;
		eslFillMemory
			( prmbuf.pIndexedList + nIndexLength, 0,
				(pmo->m_countPolygon * 3 - nIndexLength) * sizeof(uint32_t) ) ;
		//
		// 確定
		//
		vbDst.AddPrimitiveBuffer
			( nullptr, 0, primitiveTriangle, prmbuf,
				pmo->m_countPolygon * 3, pmo->m_countVertex ) ;
		//
		// 次の位置
		//
		zPos = zNextPos ;
		iCurPoint = iNextPoint ;
		pspCurPoint = pspNextPoint ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// パス適用モデル生成
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DMeshBezierPathController, MeshController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DMeshBezierPathController, path_mesh )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DMeshBezierPathController::S3DMeshBezierPathController( void )
	: MeshController( m_ItemClassDescriptor.pwszClassID ),
		m_pRefModel( nullptr ), m_fpUnitLength( 1.0 ),
		m_nRefMeshCount( 0 ), m_nMaterialCount( 1 ), m_nSubMeshCount( 1 )
{
	PrepareParameterEntryCount( paramCount ) ;
	AddParameterEntry
		( L"ref_model", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrDynamicValidation,
			L"参照モデル", nullptr ) ;
	AddParameterEntry
		( L"path_curve", S3DSceneComposer::typeBinary,
			S3DSceneComposer::attrConstant,
			L"パス曲線", nullptr ) ;
	AddParameterEntry
		( L"unit_length", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant,
			L"ユニット長", nullptr ) ;
	AddParameterEntry
		( L"material_count", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrDynamicValidation,
			L"出力マテリアル数", nullptr ) ;
	AddParameterEntry
		( L"mesh_count", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrDynamicValidation,
			L"参照メッシュ数", nullptr ) ;

	UpdateReferenceMeshCount( 1 ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DMeshBezierPathController::~S3DMeshBezierPathController( void )
{
}

// 参照モデル更新
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBezierPathController::UpdateReferenceModel( void )
{
	if ( m_strRefModel.IsEmpty() )
	{
		m_pRefModel = nullptr ;
		return ;
	}
	S3DSceneComposer *	pComposer = GetComposer() ;
	if ( pComposer == nullptr )
	{
		return ;
	}
	m_pRefModel = pComposer->GetAssets().GetModelAs( m_strRefModel ) ;
	m_aBuilders.RemoveAll() ;
	//
	if ( m_pRefModel != nullptr )
	{
		for ( size_t i = 0; i < m_aRefMeshID.GetLength(); i ++ )
		{
			UpdateReferenceMeshIndexAt
				( i / m_nSubMeshCount, i % m_nSubMeshCount ) ;
		}
	}
}

// 参照メッシュ数変更
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBezierPathController::UpdateReferenceMeshCount( size_t nCount )
{
	if ( nCount == m_nRefMeshCount )
	{
		return ;
	}
	m_nRefMeshCount = nCount ;
	m_nSubMeshCount = nCount / m_nMaterialCount ;
	//
	// プロパティ変更
	//
	ChopParameterEntryLastAt( paramMeshCount  ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SString *	pstrPropID = m_aRefMeshPropID.GetAt( i ) ;
		SString *	pstrPropName = m_aRefMeshPropName.GetAt( i ) ;
		if ( pstrPropID == nullptr )
		{
			pstrPropID = new SString ;
			m_aRefMeshPropID.SetAt( i, pstrPropID ) ;
		}
		pstrPropID->Format( L"ref_mesh%d", i ) ;
		//
		if ( pstrPropName == nullptr )
		{
			pstrPropName = new SString ;
			m_aRefMeshPropName.SetAt( i, pstrPropName ) ;
		}
		pstrPropName->Format
			( L"参照メッシュ[%d][%d]",
				(i / m_nSubMeshCount), (i % m_nSubMeshCount) ) ;
		//
		AddParameterEntry
			( *pstrPropID, S3DSceneComposer::typeSelector,
				S3DSceneComposer::attrConstant
				| S3DSceneComposer::attrStringEnumeration,
				*pstrPropName ) ;
	}
}

// 参照メッシュ指標更新
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBezierPathController::UpdateReferenceMeshIndexAt
				( size_t iMaterial, size_t iSubMesh )
{
	if ( (m_pRefModel != nullptr)
		&& (iSubMesh + iMaterial * m_nSubMeshCount < m_aRefMeshID.GetLength()) )
	{
		size_t		iMesh = (size_t) -1 ;
		SString *	pstrMeshID =
			m_aRefMeshID.GetAt( iSubMesh + iMaterial * m_nSubMeshCount ) ;
		if ( pstrMeshID != nullptr )
		{
			S3DModelBuffer::MeshGroup *
				pmg = m_pRefModel->GetMeshGroupAs( *pstrMeshID ) ;
			if ( pmg != nullptr )
			{
				iMesh = pmg->m_iFirstMesh ;
			}
		}
		S3DMeshBezierPathBuilder *	pBuilder = m_aBuilders.GetAt( iMaterial ) ;
		if ( pBuilder == nullptr )
		{
			pBuilder = new S3DMeshBezierPathBuilder ;
			m_aBuilders.SetAt( iMaterial, pBuilder ) ;
		}
		if ( iMesh != (size_t) -1 )
		{
			pBuilder->SetupRefMeshAt( iSubMesh, *m_pRefModel, iMesh ) ;
		}
		else
		{
			pBuilder->ClearRefMeshAt( iSubMesh ) ;
		}
	}
}

// 参照メッシュインターリーブ
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBezierPathController::InterleaveMeshRef
				( size_t nNewSubMesh, size_t nOldSubMesh )
{
	if ( nNewSubMesh > nOldSubMesh )
	{
		for ( size_t i = 0; i < m_nMaterialCount; i ++ )
		{
			size_t	iBase = nNewSubMesh * i ;
			for ( size_t j = nOldSubMesh; j < nNewSubMesh; j ++ )
			{
				m_aRefMeshID.InsertAt( iBase + j, nullptr ) ;
			}
		}
	}
	else if ( nNewSubMesh < nOldSubMesh )
	{
		for ( size_t i = 0; i < m_nMaterialCount; i ++ )
		{
			size_t	iBase = nNewSubMesh * i ;
			for ( size_t j = nOldSubMesh; j > nNewSubMesh; j -- )
			{
				m_aRefMeshID.RemoveAt( iBase + j - 1 ) ;
			}
		}
	}
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
double S3DMeshBezierPathController::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramUnitLength:
		return	m_fpUnitLength ;
	}
	return	0.0 ;
}

int32_t S3DMeshBezierPathController::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramMaterialCount:
		return	(int32_t) m_nMaterialCount ;
	case	paramMeshCount:
		return	(int32_t) m_nSubMeshCount ;
	}
	return	0 ;
}

const wchar_t * S3DMeshBezierPathController::GetCommandParameter( size_t i ) const
{
	if ( i >= paramRefMesh0 )
	{
		SString *	pstrMeshID = m_aRefMeshID.GetAt( i - paramRefMesh0 ) ;
		return	(pstrMeshID != nullptr) ? (const wchar_t*) *pstrMeshID : nullptr ;
	}
	switch ( i )
	{
	case	paramRefModel:
		return	m_strRefModel ;
	case	paramPathBezier:
		return	((S3DItemBezierCurveSerializer*)&m_bezierPath)->GetBezierCurveBase64() ;
	}
	return	nullptr ;
}

size_t S3DMeshBezierPathController::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t i ) const
{
	if ( i == paramPathBezier )
	{
		if ( pDst == nullptr )
		{
			return	sizeof(S3DSceneComposer::BinaryHeader)
							+ m_bezierPath.GetDataLengthInBytes() ;
		}
		if ( nBufBytes == sizeof(S3DSceneComposer::BinaryHeader) )
		{
			S3DSceneComposer::BinaryHeader *
				pbh = (S3DSceneComposer::BinaryHeader*) pDst ;
			pbh->nType = S3DSceneComposer::binaryBezierCurve ;
			pbh->nSubType = 0 ;
			pbh->nBodyBytes = (uint32_t) m_bezierPath.GetDataLengthInBytes() ;
			pbh->nReserved = 0 ;
			return	sizeof(S3DSceneComposer::BinaryHeader) ;
		}
		if ( nBufBytes == sizeof(S3DSceneComposer::BinaryHeader)
							+ m_bezierPath.GetDataLengthInBytes() )
		{
			S3DSceneComposer::BinaryHeader *
				pbh = (S3DSceneComposer::BinaryHeader*) pDst ;
			pbh->nType = S3DSceneComposer::binaryBezierCurve ;
			pbh->nSubType = 0 ;
			pbh->nBodyBytes = (uint32_t) m_bezierPath.GetDataLengthInBytes() ;
			pbh->nReserved = 0 ;
			//
			S3DSceneComposer::BinaryBezierCurveData *	pbbcd =
				(S3DSceneComposer::BinaryBezierCurveData*) pbh->GetBodyPtr() ;
			return	sizeof(S3DSceneComposer::BinaryHeader)
							+ m_bezierPath.GetBezierCurveData( *pbbcd ) ;
		}
	}
	return	0 ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBezierPathController::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramUnitLength:
		m_fpUnitLength = s ;
		return ;
	}
}

void S3DMeshBezierPathController::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramMaterialCount:
		if ( n >= 1 )
		{
			m_nMaterialCount = (size_t) esl_min( n, 4 ) ;
			UpdateReferenceMeshCount( m_nSubMeshCount * m_nMaterialCount ) ;
		}
		return ;

	case	paramMeshCount:
		if ( n >= 0 )
		{
			size_t	nOldSubMeshCount = m_nSubMeshCount ;
			UpdateReferenceMeshCount( (size_t) n * m_nMaterialCount ) ;
			InterleaveMeshRef( (size_t) n, nOldSubMeshCount ) ;
		}
		return ;
	}
}

void S3DMeshBezierPathController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	if ( i >= paramRefMesh0 )
	{
		if ( i - paramRefMesh0 < m_nRefMeshCount )
		{
			SString *	pstrMeshID = m_aRefMeshID.GetAt( i - paramRefMesh0 ) ;
			if ( pstrMeshID != nullptr )
			{
				*pstrMeshID = pwszCmd ;
			}
			else
			{
				m_aRefMeshID.SetAt( i - paramRefMesh0, new SString(pwszCmd) ) ;
			}
			UpdateReferenceMeshIndexAt
				( (i - paramRefMesh0) / m_nSubMeshCount,
					(i - paramRefMesh0) % m_nSubMeshCount ) ;
		}
		return ;
	}
	switch ( i )
	{
	case	paramRefModel:
		if ( m_strRefModel != pwszCmd )
		{
			m_strRefModel = pwszCmd ;
			UpdateReferenceModel() ;
		}
		return ;
	case	paramPathBezier:
		m_bezierPath.SetBezierCurveBase64( pwszCmd ) ;
		return ;
	}
}

size_t S3DMeshBezierPathController::SetBinaryParameter
	( size_t i, const void * pSrc, size_t nBufBytes )
{
	if ( i == paramPathBezier )
	{
		const S3DSceneComposer::BinaryHeader *
			pbh = (const S3DSceneComposer::BinaryHeader*) pSrc ;
		if ( pbh->nType == S3DSceneComposer::binaryBezierCurve )
		{
			const S3DSceneComposer::BinaryBezierCurveData *	pbbcd =
				(const S3DSceneComposer::BinaryBezierCurveData*) pbh->GetBodyPtr() ;
			m_bezierPath.SetBezierCurve( pbbcd->count, &(pbbcd->points[0]) ) ;
			//
			return	sizeof(S3DSceneComposer::BinaryHeader) + pbh->nBodyBytes ;
		}
	}
	return	0 ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshBezierPathController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	if ( i >= paramRefMesh0 )
	{
		if ( m_pRefModel != nullptr )
		{
			for ( size_t i = 0; i < m_pRefModel->GetMeshGroupList().GetLength(); i ++ )
			{
				const SString *
					pstrID = m_pRefModel->GetMeshGroupList().GetTagAt( i ) ;
				if ( pstrID != nullptr )
				{
					aStrSet.Add( new SString( *pstrID ) ) ;
				}
			}
		}
		return	true ;
	}
	if ( i == paramRefModel )
	{
		S3DSceneComposer *	pComposer = GetComposer() ;
		if ( pComposer != nullptr )
		{
			pComposer->GetAssets().EnumerateResourceIDsAs
					( aStrSet, S3DSceneComposer::resourceTypeModel ) ;
		}
		return	true ;
	}
	return	false ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DMeshBezierPathController::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags )
{
	uint32_t	nResFlags =
			MeshController::UpdatePropertyReference( comp, pItem, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefResource )
	{
		UpdateReferenceModel() ;
		//
		nResFlags |= S3DSceneComposer::updateRefResource ;
	}
	return	nResFlags ;
}

// メッシュ追加処理（全視点・ビュー共通処理）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBezierPathController::AddMesh
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem,
		S3DVertexBufferInterface ** ppVBs, size_t nVBCount )
{
	if ( (m_pRefModel == nullptr)
		|| (m_bezierPath.GetPointCount() < 2) || (nVBCount == 0) )
	{
		return ;
	}
	//
	// ベジェ曲線生成
	//
	S3DMeshBezierPathBuilder::SegmentCurve	curve ;
	curve.PrepareCruve( m_bezierPath, m_fpUnitLength ) ;
	//
	// モデルデータ生成
	//
	for ( size_t i = 0; i < m_nMaterialCount; i ++ )
	{
		if ( i >= nVBCount )
		{
			break ;
		}
		if ( ppVBs[i] == nullptr )
		{
			continue ;
		}
		S3DMeshBezierPathBuilder *	pBuilder = m_aBuilders.GetAt( i ) ;
		if ( pBuilder == nullptr )
		{
			continue ;
		}
		pBuilder->BuildMesh( *(ppVBs[i]), curve ) ;
	}
}

// フレーム描画前処理（全視点・ビュー共通処理）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBezierPathController::UpdateMesh
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem,
		S3DVertexBufferInterface ** ppVBs, size_t nVBCount )
{
}



//////////////////////////////////////////////////////////////////////////////
// 稲妻効果メッシュ生成
//////////////////////////////////////////////////////////////////////////////

const wchar_t *	S3DThunderMeshController::s_pwszGenerateShapes
					[S3DThunderMeshController::shapeCount] =
{
	L"sphere",
	L"sphere_v",
	L"sphere_h",
	L"ring",
	L"radial",
	L"curve",
} ;

const SSystem::SXMLDocument::AttrInteger
	S3DThunderMeshController::s_aiGenerateShapes[S3DThunderMeshController::shapeCount+1] =
{
	{ L"sphere", shapeSphere },
	{ L"sphere_v", shapeSphereVert },
	{ L"sphere_h", shapeSphereHorz },
	{ L"ring", shapeRing },
	{ L"radial", shapeRadial },
	{ L"curve", shapeCurve },
	{ nullptr, 0 },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DThunderMeshController, MeshController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DThunderMeshController, thunder_mesh )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DThunderMeshController::S3DThunderMeshController( void )
	: MeshController( m_ItemClassDescriptor.pwszClassID ),
		m_matGenRotation( 1, 1, 1 )
{
	m_flagsBehavior |= S3DSceneComposer::behaviorOnTimer ;

	m_shape = shapeSphere ;
	m_fpGenCount = 1.0 ;
	m_nGenMinCount = 0 ;
	m_nGenMaxCount = 30 ;
	m_fpGenRadius = 1.0 ;
	m_fpGenTipShrink = 1.0 ;
	m_secThunderLife = 0.5 ;
	m_secThunderFadeout = 0.2 ;
	m_fpThunderLength = 1.0 ;
	m_tpParam.fpJointEffect = 0.2 ;
	m_tpParam.fpJointLen = 0.3 ;
	m_tpParam.fpWaveEffect = 0.5 ;
	m_tpParam.fpWaveLen = 1.5 ;
	m_fpThickness = 0.2 ;
	m_fpThicknessRnd = 0.3 ;
	m_fpTipLength = 0.5 ;
	m_fpTipThickness = 0.0 ;
	m_fpTipAlpha = 1.0 ;

	m_tlpThunder.nFlags = 0 ;
	m_tlpThunder.uWidth = 1.0f ;
	m_tlpThunder.vRatio = 1.0f ;
	m_tlpThunder.vOffset = 0.0f ;
	m_tlpThunder.zBais = 0.0f ;
	m_tlpThunder.nColorDiv = 4 ;
	m_tlpThunder.pColors = m_clrThunder ;
	m_tlpThunder.pThickDiv = m_fpColorDiv ;
	m_clrThunder[0].rgbMul = 0xFFFFFFFF ;
	m_clrThunder[0].rgbAdd = 0x00FFFFFF ;
	m_clrThunder[1].rgbMul = 0xFFFFFFFF ;
	m_clrThunder[1].rgbAdd = 0x00FFFF00 ;
	m_clrThunder[2].rgbMul = 0xFFFFFFFF ;
	m_clrThunder[2].rgbAdd = 0x00FF0000 ;
	m_clrThunder[3].rgbMul = 0x00FFFFFF ;
	m_clrThunder[3].rgbAdd = 0x00000000 ;
	m_fpColorDiv[0] = 0.0f ;
	m_fpColorDiv[1] = 0.25f ;
	m_fpColorDiv[2] = 0.5f ;
	m_fpColorDiv[3] = 1.0f ;

	PrepareParameterEntryCount( paramCount ) ;
	ESLVerify( paramGenShape == AddParameterEntry
		( L"gen_shape",
			S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration
			| S3DSceneComposer::attrDynamicValidation,
			L"生成形状", NULL ) ) ;
	ESLVerify( paramGenCount == AddParameterEntry
		( L"gen_count",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"生成数", L"秒間の稲妻生成数", 0.0, 100.0 ) ) ;
	ESLVerify( paramGenMinCount == AddParameterEntry
		( L"gen_min_count",
			S3DSceneComposer::typeInteger, 0,
			L"表示最小数", nullptr ) ) ;
	ESLVerify( paramGenMaxCount == AddParameterEntry
		( L"gen_max_count",
			S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"表示最大数", nullptr ) ) ;
	ESLVerify( paramGenCurve == AddParameterEntry
		( L"gen_curve",
			S3DSceneComposer::typeBinary,
			S3DSceneComposer::attrConstant,
			L"パス曲線", L"稲妻形状" ) ) ;
	ESLVerify( paramGenRadius == AddParameterEntry
		( L"gen_radius",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"生成半径", L"稲妻生成形状の半径", 0.0, 100.0 ) ) ;
	ESLVerify( paramGenRotaion == AddParameterEntry
		( L"gen_rotation",
			S3DSceneComposer::typeRotation, 0,
			L"生成回転", nullptr ) ) ;
	ESLVerify( paramGenTipShrink == AddParameterEntry
		( L"gen_tip_shrink",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"生成先端収縮",
			L"生成形状に沿って生成する場合、稲妻先端・終端部の原点位置に対する比率", 0.0, 2.0 ) ) ;
	ESLVerify( paramThunderLife == AddParameterEntry
		( L"thunder_life",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider
			| S3DSceneComposer::attrCategory1,
			L"稲妻寿命",
			L"生成された稲妻の寿命[秒]", 0.1, 2.0 ) ) ;
	ESLVerify( paramThunderFadeout == AddParameterEntry
		( L"thunder_fadeout",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider
			| S3DSceneComposer::attrCategory1,
			L"稲妻フェードアウト時間", nullptr, 0.0, 1.0 ) ) ;
	ESLVerify( paramThunderLength == AddParameterEntry
		( L"thunder_length",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider
			| S3DSceneComposer::attrCategory1,
			L"稲妻長", L"生成する稲妻の最大長に対する比率", 0.0, 1.0 ) ) ;
	ESLVerify( paramJointAmplitude == AddParameterEntry
		( L"joint_amp",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider
			| S3DSceneComposer::attrCategory1,
			L"雷ブレ幅", L"稲妻の微小ジグザグ振幅を指定します", 0.0, 1.0 ) ) ;
	ESLVerify( paramJointLength == AddParameterEntry
		( L"joint_length",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider
			| S3DSceneComposer::attrCategory1,
			L"雷節長", L"ジグザグの節分割長を指定します", 0.0, 2.0 ) ) ;
	ESLVerify( paramWaveAmplitude == AddParameterEntry
		( L"wave_amp",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider
			| S3DSceneComposer::attrCategory1,
			L"雷振幅", L"稲妻の大きな振幅を指定します", 0.0, 2.0 ) ) ;
	ESLVerify( paramWaveLength == AddParameterEntry
		( L"wave_length",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider
			| S3DSceneComposer::attrCategory1,
			L"雷波長", L"稲妻の大きな揺れの波長を指定します", 0.0, 10.0 ) ) ;
	ESLVerify( paramUScale == AddParameterEntry
		( L"u_scale",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider
			| S3DSceneComposer::attrCategory2,
			L"Ｕスケール",
			L"テクスチャマップｘ座標スケール（幅全体を 1.0 とする）", 0.0, 1.0 ) ) ;
	ESLVerify( paramVScale == AddParameterEntry
		( L"v_scale",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider
			| S3DSceneComposer::attrCategory2,
			L"Ｖスケール",
			L"テクスチャマップｙ座標スケール（空間距離比）", 0.0, 10.0 ) ) ;
	ESLVerify( paramZBais == AddParameterEntry
		( L"z_bias",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider
			| S3DSceneComposer::attrCategory2,
			L"ｚバイアス", nullptr, 0.0, 1.0 ) ) ;
	ESLVerify( paramBodyThickness == AddParameterEntry
		( L"thunder_thickness",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider
			| S3DSceneComposer::attrCategory2,
			L"太さ", nullptr, 0.0, 3.0 ) ) ;
	ESLVerify( paramThicknessRandom == AddParameterEntry
		( L"thickness_random",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider
			| S3DSceneComposer::attrCategory2,
			L"太さランダム比率", nullptr, 0.0, 1.0 ) ) ;
	ESLVerify( paramTipLength == AddParameterEntry
		( L"tip_length",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider
			| S3DSceneComposer::attrConstant2,
			L"先端長さ", L"先端として扱う長さ", 0.0, 5.0 ) ) ;
	ESLVerify( paramTipThickness == AddParameterEntry
		( L"tip_thickness",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider
			| S3DSceneComposer::attrConstant2,
			L"先端太さ比率", nullptr, 0.0, 1.0 ) ) ;
	ESLVerify( paramTipAlpha == AddParameterEntry
		( L"tip_alpha",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider
			| S3DSceneComposer::attrConstant2,
			L"先端α", nullptr, 0.0, 1.0 ) ) ;
	ESLVerify( paramColorAlpha0 == AddParameterEntry
		( L"alpha0",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider
			| S3DSceneComposer::attrConstant3,
			L"中心部α", nullptr, 0.0, 1.0 ) ) ;
	ESLVerify( paramColorMul0 == AddParameterEntry
		( L"mul_color0",
			S3DSceneComposer::typeColor,
			S3DSceneComposer::attrConstant3,
			L"中心部乗算色", nullptr ) ) ;
	ESLVerify( paramColorAdd0 == AddParameterEntry
		( L"add_color0",
			S3DSceneComposer::typeColor,
			S3DSceneComposer::attrConstant3,
			L"中心部加算色", nullptr ) ) ;
	ESLVerify( paramColorAlpha1 == AddParameterEntry
		( L"alpha1",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider
			| S3DSceneComposer::attrConstant3,
			L"α[1]", nullptr, 0.0, 1.0 ) ) ;
	ESLVerify( paramColorMul1 == AddParameterEntry
		( L"mul_color1",
			S3DSceneComposer::typeColor,
			S3DSceneComposer::attrConstant3,
			L"乗算色[1]", nullptr ) ) ;
	ESLVerify( paramColorAdd1 == AddParameterEntry
		( L"add_color1",
			S3DSceneComposer::typeColor,
			S3DSceneComposer::attrConstant3,
			L"加算色[1]", nullptr ) ) ;
	ESLVerify( paramColorDivision1 == AddParameterEntry
		( L"color_div1",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider
			| S3DSceneComposer::attrConstant3,
			L"色分割位置[1]", nullptr, 0.0, 1.0 ) ) ;
	ESLVerify( paramColorAlpha2 == AddParameterEntry
		( L"alpha2",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider
			| S3DSceneComposer::attrConstant3,
			L"α[2]", nullptr, 0.0, 1.0 ) ) ;
	ESLVerify( paramColorMul2 == AddParameterEntry
		( L"mul_color2",
			S3DSceneComposer::typeColor,
			S3DSceneComposer::attrConstant3,
			L"乗算色[2]", nullptr ) ) ;
	ESLVerify( paramColorAdd2 == AddParameterEntry
		( L"add_color2",
			S3DSceneComposer::typeColor,
			S3DSceneComposer::attrConstant3,
			L"加算色[2]", nullptr ) ) ;
	ESLVerify( paramColorDivision2 == AddParameterEntry
		( L"color_div2",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider
			| S3DSceneComposer::attrConstant3,
			L"色分割位置[2]", nullptr, 0.0, 1.0 ) ) ;
	ESLVerify( paramColorAlpha3 == AddParameterEntry
		( L"alpha3",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider
			| S3DSceneComposer::attrConstant3,
			L"外延部α", nullptr, 0.0, 1.0 ) ) ;
	ESLVerify( paramColorMul3 == AddParameterEntry
		( L"mul_color3",
			S3DSceneComposer::typeColor,
			S3DSceneComposer::attrConstant3,
			L"外延部乗算色", nullptr ) ) ;
	ESLVerify( paramColorAdd3 == AddParameterEntry
		( L"add_color3",
			S3DSceneComposer::typeColor,
			S3DSceneComposer::attrConstant3,
			L"外延部加算色", nullptr ) ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DThunderMeshController::~S3DThunderMeshController( void )
{
}

// 稲妻発生
//////////////////////////////////////////////////////////////////////////////
void S3DThunderMeshController::GenerateThunder( size_t nGenCount )
{
	if ( nGenCount == 0 )
	{
		return ;
	}
	switch ( m_shape )
	{
	case	shapeSphere:
		GenerateThunderOnSphere( nGenCount ) ;
		break ;
	case	shapeSphereVert:
		GenerateThunderOnSphereVert( nGenCount ) ;
		break ;
	case	shapeSphereHorz:
		GenerateThunderOnSphereHorz( nGenCount ) ;
		break ;
	case	shapeRing:
		GenerateThunderOnRing( nGenCount ) ;
		break ;
	case	shapeRadial:
		GenerateThunderRadially( nGenCount ) ;
		break ;
	case	shapeCurve:
		GenerateThunderOnCurve( nGenCount ) ;
		break ;
	default:
		break ;
	}
}

void S3DThunderMeshController::GenerateThunderOnSphere( size_t nGenCount )
{
	const int	nDivCount = 16 ;
	S3DDVector	vPoints[nDivCount] ;
	double		radArch = m_fpThunderLength * 2.0 * PI ;

	for ( size_t i = 0; i < nGenCount; i ++ )
	{
		for ( int j = 0; j < nDivCount; j ++ )
		{
			double	t = (double) j / (nDivCount - 1) ;
			double	t1 = (t - 0.5) * 2.0 ;
			double	t2 = 1.0 - (1.0 - m_fpGenTipShrink) * t1 * t1 ;
			double	rad = radArch * t ;
			vPoints[j].x = m_fpGenRadius * t2 * sin(rad) ;
			vPoints[j].y = -m_fpGenRadius * t2 * cos(rad) ;
			vPoints[j].z = 0.0 ;
		}
		S3DDMatrix	matRot = m_matGenRotation ;
		double	radY = m_random.QuickRandomDouble(PI) ;
		matRot.RevolveOnY( sin(radY), cos(radY) ) ;
		double	radX = m_random.QuickRandomDouble(PI) ;
		matRot.RevolveOnX( sin(radX), cos(radX) ) ;
		double	radZ = m_random.QuickRandomDouble(PI) ;
		matRot.RevolveOnZ( sin(radZ), cos(radZ) ) ;
		//
		for ( int j = 0; j < nDivCount; j ++ )
		{
			matRot.RevolveVector( vPoints[j] ) ;
		}

		AddThunder
			( vPoints, nDivCount,
				(size_t) esl_lroundfi( m_secThunderLife * 1000.0 ) ) ;
	}
}

void S3DThunderMeshController::GenerateThunderOnSphereVert( size_t nGenCount )
{
	const int	nDivCount = 16 ;
	S3DDVector	vPoints[nDivCount] ;
	double		radArch = m_fpThunderLength * 2.0 * PI ;

	for ( size_t i = 0; i < nGenCount; i ++ )
	{
		for ( int j = 0; j < nDivCount; j ++ )
		{
			double	t = (double) j / (nDivCount - 1) ;
			double	t1 = (t - 0.5) * 2.0 ;
			double	t2 = 1.0 - (1.0 - m_fpGenTipShrink) * t1 * t1 ;
			double	rad = radArch * t ;
			vPoints[j].x = m_fpGenRadius * t2 * sin(rad) ;
			vPoints[j].y = -m_fpGenRadius * t2 * cos(rad) ;
			vPoints[j].z = 0.0 ;
		}
		S3DDMatrix	matRot = m_matGenRotation ;
		double	radY = m_random.QuickRandomDouble(PI) ;
		matRot.RevolveOnY( sin(radY), cos(radY) ) ;
		double	radZ = m_random.QuickRandomFloat
							( (float32_t) (PI * (1.0 - m_fpThunderLength)) ) ;
		matRot.RevolveOnZ( sin(-radZ), cos(-radZ) ) ;
		//
		for ( int j = 0; j < nDivCount; j ++ )
		{
			matRot.RevolveVector( vPoints[j] ) ;
		}

		AddThunder
			( vPoints, nDivCount,
				(size_t) esl_lroundfi( m_secThunderLife * 1000.0 ) ) ;
	}
}

void S3DThunderMeshController::GenerateThunderOnSphereHorz( size_t nGenCount )
{
	const int	nDivCount = 16 ;
	S3DDVector	vPoints[nDivCount] ;
	double		radArch = m_fpThunderLength * 2.0 * PI ;

	for ( size_t i = 0; i < nGenCount; i ++ )
	{
		double		radZ = m_random.QuickRandomFloat( (float32_t) (PI-0.2) ) + 0.1 ;
		double		radY = m_random.QuickRandomDouble(PI) ;
		double		sinZ = sin(radZ) ;
		double		cosZ = cos(radZ) ;

		for ( int j = 0; j < nDivCount; j ++ )
		{
			double	t = (double) j / (nDivCount - 1) ;
			double	t1 = (t - 0.5) * 2.0 ;
			double	t2 = 1.0 - (1.0 - m_fpGenTipShrink) * t1 * t1 ;
			double	rad = radY + radArch * t ;
			vPoints[j].x = m_fpGenRadius * t2 * cosZ * cos(rad) ;
			vPoints[j].y = m_fpGenRadius * t2 * sinZ ;
			vPoints[j].z = m_fpGenRadius * t2 * cosZ * sin(rad) ;
			//
			m_matGenRotation.RevolveVector( vPoints[j] ) ;
		}

		AddThunder
			( vPoints, nDivCount,
				(size_t) esl_lroundfi( m_secThunderLife * 1000.0 ) ) ;
	}
}

void S3DThunderMeshController::GenerateThunderOnRing( size_t nGenCount )
{
	const int	nDivCount = 16 ;
	S3DDVector	vPoints[nDivCount] ;
	double		radArch = m_fpThunderLength * 2.0 * PI ;

	for ( size_t i = 0; i < nGenCount; i ++ )
	{
		for ( int j = 0; j < nDivCount; j ++ )
		{
			double	t = (double) j / (nDivCount - 1) ;
			double	t1 = (t - 0.5) * 2.0 ;
			double	t2 = 1.0 - (1.0 - m_fpGenTipShrink) * t1 * t1 ;
			double	rad = radArch * t ;
			vPoints[j].x = m_fpGenRadius * t2 * sin(rad) ;
			vPoints[j].y = 0.0 ;
			vPoints[j].z = m_fpGenRadius * t2 * cos(rad) ;
		}
		S3DDMatrix	matRot = m_matGenRotation ;
		double	radY = m_random.QuickRandomDouble(PI) ;
		matRot.RevolveOnY( sin(radY), cos(radY) ) ;
		//
		for ( int j = 0; j < nDivCount; j ++ )
		{
			matRot.RevolveVector( vPoints[j] ) ;
		}

		AddThunder
			( vPoints, nDivCount,
				(size_t) esl_lroundfi( m_secThunderLife * 1000.0 ) ) ;
	}
}

void S3DThunderMeshController::GenerateThunderRadially( size_t nGenCount )
{
	const int	nDivCount = 3 ;
	S3DDVector	vPoints[nDivCount] ;

	for ( size_t i = 0; i < nGenCount; i ++ )
	{
		vPoints[0] = S3DDVector( 0, 0, 0 ) ;
		vPoints[2].x = m_random.QuickRandomDouble(1.0) ;
		vPoints[2].y = m_random.QuickRandomDouble(1.0) ;
		vPoints[2].z = m_random.QuickRandomDouble(1.0) ;
		vPoints[2].Normalize() ;
		//
		vPoints[2] *= (1.0 + m_random.QuickRandomDouble
								( 1.0 - m_fpThunderLength )) * m_fpGenRadius ;
		vPoints[1] = vPoints[2] * 0.5 ;

		AddThunder
			( vPoints, nDivCount,
				(size_t) esl_lroundfi( m_secThunderLife * 1000.0 ) ) ;
	}
}

void S3DThunderMeshController::GenerateThunderOnCurve( size_t nGenCount )
{
	const size_t	nBezierPoints = m_bezierPath.GetPointCount() ;
	if ( nBezierPoints < 2 )
	{
		return ;
	}
	SGLBezierCurves<S3DDVector>	bezier ;
	bezier.SetLength( 4 ) ;
	//
	SArray<S3DDVector>	aPoints ;
	const int			nDivCount =
							(size_t) esl_max( (int) esl_lroundfi
								( (m_fpThunderLength
									* (m_bezierPath.GetPointCount() - 1) + 1.0) * 8.0 ), 8 ) ;
	S3DDVector *		pPoints = aPoints.GetArray( nDivCount ) ;

	for ( size_t i = 0; i < nGenCount; i ++ )
	{
		double	t0 = esl_fmax( m_random.QuickRandomFloat
							( 1.0f - (float32_t) m_fpThunderLength ), 0.0 ) ;
		for ( int j = 0; j < nDivCount; j ++ )
		{
			double	t = esl_fmax( t0 + m_fpThunderLength * j / (nDivCount - 1), 0.0 ) ;
			size_t	ti = (size_t) floor( t * (nBezierPoints - 1) ) ;
			double	dt = t - ti ;
			if ( ti >= nBezierPoints - 1 )
			{
				ti = nBezierPoints - 2 ;
				dt = 1.0 ;
			}
			const S3DSceneComposer::BinaryBezierCurvePoint *
								pbcp0 = m_bezierPath.GetPointAt( ti ) ;
			const S3DSceneComposer::BinaryBezierCurvePoint *
								pbcp1 = m_bezierPath.GetPointAt( ti + 1 ) ;
			ESLAssert( pbcp0 != nullptr ) ;
			ESLAssert( pbcp1 != nullptr ) ;
			if ( (pbcp0 == nullptr) || (pbcp1 == nullptr) )
			{
				continue ;
			}
			bezier.At(0) = pbcp0->vPoint ;
			bezier.At(1) = pbcp0->vHandle[1] ;
			bezier.At(2) = pbcp1->vHandle[0] ;
			bezier.At(3) = pbcp1->vPoint ;
			//
			pPoints[j] = bezier.PointAt( dt ) ;
		}

		AddThunder
			( pPoints, nDivCount,
				(size_t) esl_lroundfi( m_secThunderLife * 1000.0 ) ) ;
	}

	aPoints.FinishArray() ;
}

// 稲妻追加
//////////////////////////////////////////////////////////////////////////////
void S3DThunderMeshController::AddThunder
	( const S3DDVector * pvPoints, size_t nCount, size_t msecLeft )
{
	ThunderInstance *	pThunder = new ThunderInstance ;
	pThunder->m_aPoints.AddArray( pvPoints, nCount ) ;
	pThunder->m_flagFirst = true ;
	pThunder->m_msecLeft = msecLeft ;
	pThunder->m_msecPast = 0 ;
	//
	m_csThunder.Lock() ;
	m_aInstances.Add( pThunder ) ;
	m_csThunder.Unlock() ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DThunderMeshController::GetMatrixParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramGenRotaion:
		return	m_matGenRotation ;
	}
	return	S3DDMatrix( 1, 1, 1 ) ;
}

S3DDVector S3DThunderMeshController::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramColorMul0:
		return	VectorFromColor( m_clrThunder[0].rgbMul ) ;
	case	paramColorAdd0:
		return	VectorFromColor( m_clrThunder[0].rgbAdd ) ;
	case	paramColorMul1:
		return	VectorFromColor( m_clrThunder[1].rgbMul ) ;
	case	paramColorAdd1:
		return	VectorFromColor( m_clrThunder[1].rgbAdd ) ;
	case	paramColorMul2:
		return	VectorFromColor( m_clrThunder[2].rgbMul ) ;
	case	paramColorAdd2:
		return	VectorFromColor( m_clrThunder[2].rgbAdd ) ;
	case	paramColorMul3:
		return	VectorFromColor( m_clrThunder[3].rgbMul ) ;
	case	paramColorAdd3:
		return	VectorFromColor( m_clrThunder[3].rgbAdd ) ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DThunderMeshController::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramGenCount:
		return	m_fpGenCount ;
	case	paramGenRadius:
		return	m_fpGenRadius ;
	case	paramGenTipShrink:
		return	m_fpGenTipShrink ;
	case	paramThunderLife:
		return	m_secThunderLife ;
	case	paramThunderFadeout:
		return	m_secThunderFadeout ;
	case	paramThunderLength:
		return	m_fpThunderLength ;
	case	paramJointAmplitude:
		return	m_tpParam.fpJointEffect ;
	case	paramJointLength:
		return	m_tpParam.fpJointLen ;
	case	paramWaveAmplitude:
		return	m_tpParam.fpWaveEffect ;
	case	paramWaveLength:
		return	m_tpParam.fpWaveLen ;
	case	paramUScale:
		return	m_tlpThunder.uWidth ;
	case	paramVScale:
		return	m_tlpThunder.vRatio ;
	case	paramZBais:
		return	m_tlpThunder.zBais ;
	case	paramBodyThickness:
		return	m_fpThickness ;
	case	paramThicknessRandom:
		return	m_fpThicknessRnd ;
	case	paramTipLength:
		return	m_fpTipLength ;
	case	paramTipThickness:
		return	m_fpTipThickness ;
	case	paramTipAlpha:
		return	m_fpTipAlpha ;
	case	paramColorAlpha0:
		return	m_clrThunder[0].rgbMul.argb.Alpha / 255.0 ;
	case	paramColorAlpha1:
		return	m_clrThunder[1].rgbMul.argb.Alpha / 255.0 ;
	case	paramColorDivision1:
		return	m_fpColorDiv[1] ;
	case	paramColorAlpha2:
		return	m_clrThunder[2].rgbMul.argb.Alpha / 255.0 ;
	case	paramColorDivision2:
		return	m_fpColorDiv[2] ;
	case	paramColorAlpha3:
		return	m_clrThunder[3].rgbMul.argb.Alpha / 255.0 ;
	}
	return	0.0 ;
}

int32_t S3DThunderMeshController::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramGenMinCount:
		return	m_nGenMinCount ;
	case	paramGenMaxCount:
		return	m_nGenMaxCount ;
	}
	return	0 ;
}

const wchar_t * S3DThunderMeshController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramGenShape:
		return	s_pwszGenerateShapes[m_shape] ;

	case	paramGenCurve:
		return	((S3DItemBezierCurveSerializer*)&m_bezierPath)->GetBezierCurveBase64() ;
	}
	return	nullptr ;
}

size_t S3DThunderMeshController::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t i ) const
{
	switch ( i )
	{
	case	paramGenCurve:
		if ( pDst == nullptr )
		{
			return	sizeof(S3DSceneComposer::BinaryHeader)
							+ m_bezierPath.GetDataLengthInBytes() ;
		}
		if ( nBufBytes == sizeof(S3DSceneComposer::BinaryHeader) )
		{
			S3DSceneComposer::BinaryHeader *
				pbh = (S3DSceneComposer::BinaryHeader*) pDst ;
			pbh->nType = S3DSceneComposer::binaryBezierCurve ;
			pbh->nSubType = 0 ;
			pbh->nBodyBytes = (uint32_t) m_bezierPath.GetDataLengthInBytes() ;
			pbh->nReserved = 0 ;
			return	sizeof(S3DSceneComposer::BinaryHeader) ;
		}
		if ( nBufBytes == sizeof(S3DSceneComposer::BinaryHeader)
							+ m_bezierPath.GetDataLengthInBytes() )
		{
			S3DSceneComposer::BinaryHeader *
				pbh = (S3DSceneComposer::BinaryHeader*) pDst ;
			pbh->nType = S3DSceneComposer::binaryBezierCurve ;
			pbh->nSubType = 0 ;
			pbh->nBodyBytes = (uint32_t) m_bezierPath.GetDataLengthInBytes() ;
			pbh->nReserved = 0 ;
			//
			S3DSceneComposer::BinaryBezierCurveData *	pbbcd =
				(S3DSceneComposer::BinaryBezierCurveData*) pbh->GetBodyPtr() ;
			return	sizeof(S3DSceneComposer::BinaryHeader)
							+ m_bezierPath.GetBezierCurveData( *pbbcd ) ;
		}
		break ;
	}
	return	0 ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DThunderMeshController::SetMatrixParameter( size_t i, const S3DDMatrix& mat )
{
	switch ( i )
	{
	case	paramGenRotaion:
		m_matGenRotation = mat ;
		return ;
	}
}

void S3DThunderMeshController::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramColorMul0:
		m_clrThunder[0].rgbMul =
			(ColorFromVector( vec ).ui32 & 0x00FFFFFF)
			| (m_clrThunder[0].rgbMul.ui32 & 0xFF000000) ;
		return ;
	case	paramColorAdd0:
		m_clrThunder[0].rgbAdd = ColorFromVector( vec ) ;
		return ;
	case	paramColorMul1:
		m_clrThunder[1].rgbMul =
			(ColorFromVector( vec ).ui32 & 0x00FFFFFF)
			| (m_clrThunder[1].rgbMul.ui32 & 0xFF000000) ;
		return ;
	case	paramColorAdd1:
		m_clrThunder[1].rgbAdd = ColorFromVector( vec ) ;
		return ;
	case	paramColorMul2:
		m_clrThunder[2].rgbMul =
			(ColorFromVector( vec ).ui32 & 0x00FFFFFF)
			| (m_clrThunder[2].rgbMul.ui32 & 0xFF000000) ;
		return ;
	case	paramColorAdd2:
		m_clrThunder[2].rgbAdd = ColorFromVector( vec ) ;
		return ;
	case	paramColorMul3:
		m_clrThunder[3].rgbMul =
			(ColorFromVector( vec ).ui32 & 0x00FFFFFF)
			| (m_clrThunder[3].rgbMul.ui32 & 0xFF000000) ;
		return ;
	case	paramColorAdd3:
		m_clrThunder[3].rgbAdd = ColorFromVector( vec ) ;
		return ;
	}
}

void S3DThunderMeshController::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramGenCount:
		m_fpGenCount = s ;
		return ;
	case	paramGenRadius:
		m_fpGenRadius = s ;
		return ;
	case	paramGenTipShrink:
		m_fpGenTipShrink = s ;
		return ;
	case	paramThunderLife:
		m_secThunderLife = s ;
		return ;
	case	paramThunderFadeout:
		m_secThunderFadeout = s ;
		return ;
	case	paramThunderLength:
		m_fpThunderLength = s ;
		return ;
	case	paramJointAmplitude:
		m_tpParam.fpJointEffect = s ;
		return ;
	case	paramJointLength:
		m_tpParam.fpJointLen = s ;
		return ;
	case	paramWaveAmplitude:
		m_tpParam.fpWaveEffect = s ;
		return ;
	case	paramWaveLength:
		m_tpParam.fpWaveLen = s ;
		return ;
	case	paramUScale:
		m_tlpThunder.uWidth = (float32_t) s ;
		return ;
	case	paramVScale:
		m_tlpThunder.vRatio = (float32_t) s ;
		return ;
	case	paramZBais:
		m_tlpThunder.zBais = (float32_t) s ;
		return ;
	case	paramBodyThickness:
		m_fpThickness = s ;
		return ;
	case	paramThicknessRandom:
		m_fpThicknessRnd = s ;
		return ;
	case	paramTipLength:
		m_fpTipLength = s ;
		return ;
	case	paramTipThickness:
		m_fpTipThickness = s ;
		return ;
	case	paramTipAlpha:
		m_fpTipAlpha = s ;
		return ;
	case	paramColorAlpha0:
		m_clrThunder[0].rgbMul.argb.Alpha =
			(uint8_t) esl_clampi( (int) esl_lroundfi( s * 255.0 ), 0, 0xFF ) ;
		return ;
	case	paramColorAlpha1:
		m_clrThunder[1].rgbMul.argb.Alpha =
			(uint8_t) esl_clampi( (int) esl_lroundfi( s * 255.0 ), 0, 0xFF ) ;
		return ;
	case	paramColorDivision1:
		m_fpColorDiv[1] = (float32_t) s ;
		return ;
	case	paramColorAlpha2:
		m_clrThunder[2].rgbMul.argb.Alpha =
			(uint8_t) esl_clampi( (int) esl_lroundfi( s * 255.0 ), 0, 0xFF ) ;
		return ;
	case	paramColorDivision2:
		m_fpColorDiv[2] = (float32_t) s ;
		return ;
	case	paramColorAlpha3:
		m_clrThunder[3].rgbMul.argb.Alpha =
			(uint8_t) esl_clampi( (int) esl_lroundfi( s * 255.0 ), 0, 0xFF ) ;
		return ;
	}
}

void S3DThunderMeshController::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramGenMinCount:
		m_nGenMinCount = n ;
		return ;
	case	paramGenMaxCount:
		m_nGenMaxCount = n ;
		return ;
	}
}

void S3DThunderMeshController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramGenShape:
		m_shape = (GenerationShape)
			SXMLDocument::GetIntegerAsSymbolOf( s_aiGenerateShapes, pwszCmd, m_shape ) ;
		return ;

	case	paramGenCurve:
		m_bezierPath.SetBezierCurveBase64( pwszCmd ) ;
		return ;
	}
}

size_t S3DThunderMeshController::SetBinaryParameter
	( size_t i, const void * pSrc, size_t nBufBytes )
{
	if ( i == paramGenCurve )
	{
		const S3DSceneComposer::BinaryHeader *
			pbh = (const S3DSceneComposer::BinaryHeader*) pSrc ;
		if ( pbh->nType == S3DSceneComposer::binaryBezierCurve )
		{
			const S3DSceneComposer::BinaryBezierCurveData *	pbbcd =
				(const S3DSceneComposer::BinaryBezierCurveData*) pbh->GetBodyPtr() ;
			m_bezierPath.SetBezierCurve( pbbcd->count, &(pbbcd->points[0]) ) ;
			//
			return	sizeof(S3DSceneComposer::BinaryHeader) + pbh->nBodyBytes ;
		}
	}
	return	0 ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DThunderMeshController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	switch ( i )
	{
	case	paramGenShape:
		{
			for ( int j = 0; j < shapeCount; j ++ )
			{
				aStrSet.Add( new SString(s_pwszGenerateShapes[j]) ) ;
			}
		}
		return	true ;

	}
	return	false ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DThunderMeshController::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramGenCurve:
		return	(m_shape == shapeCurve) ;
	case	paramGenRadius:
		return	(m_shape != shapeCurve) ;
	}
	return	true ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DThunderMeshController::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"稲妻生成" ;
	case	1:
		return	L"稲妻形状" ;
	case	2:
		return	L"稲妻表示" ;
	case	3:
		return	L"稲妻色" ;
	}
	return	nullptr ;
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DThunderMeshController::OnTimer
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast )
{
	MeshController::OnTimer( scene, pItem, msecPast ) ;

	SSmartLock<SCriticalSection>	lock( &m_csThunder ) ;
	for ( size_t i = 0; i < m_aInstances.GetLength(); i ++ )
	{
		ThunderInstance *	pThunder = m_aInstances.GetAt( i ) ;
		if ( pThunder == nullptr )
		{
			continue ;
		}
		if ( pThunder->m_msecLeft <= msecPast )
		{
			pThunder->m_msecLeft = 0 ;
			if ( !pThunder->m_flagFirst )
			{
				m_aInstances.SetAt( i, nullptr ) ;
				continue ;
			}
		}
		else
		{
			pThunder->m_msecLeft -= msecPast ;
		}
		pThunder->m_flagFirst = false ;
		pThunder->m_msecPast += msecPast ;
	}
	m_aInstances.TrimEmpty() ;

	double	fpGenCount = m_fpGenCount * msecPast / 1000.0 ;
	size_t	nGenCount = (size_t) floor( fpGenCount ) ;
	if ( fpGenCount - nGenCount > m_random.QuickRandomFloat( 1.0 ) )
	{
		nGenCount ++ ;
	}
	if ( (int32_t) (m_aInstances.GetLength() + nGenCount) < m_nGenMinCount )
	{
		nGenCount = (size_t) m_nGenMinCount - m_aInstances.GetLength() ;
	}
	if ( m_aInstances.GetLength() > (size_t) m_nGenMaxCount )
	{
		nGenCount = 0 ;
	}
	GenerateThunder( nGenCount ) ;
}

// フレーム（パラメータ）更新後処理
//////////////////////////////////////////////////////////////////////////////
void S3DThunderMeshController::OnUpdateFrame
	( S3DSceneComposer::ItemSerializer * pItem,
		double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	MeshController::OnUpdateFrame( pItem, fpFrame, seek ) ;
	//
	if ( seek == S3DSceneComposer::seekJumpReset )
	{
		m_csThunder.Lock() ;
		m_aInstances.RemoveAll() ;
		m_csThunder.Unlock() ;
	}
}

// メッシュ追加処理（全視点・ビュー共通処理）
//////////////////////////////////////////////////////////////////////////////
void S3DThunderMeshController::AddMesh
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem,
		S3DVertexBufferInterface ** ppVBs, size_t nVBCount )
{
	if ( (nVBCount == 0) || (ppVBs[0] == nullptr) )
	{
		return ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csThunder ) ;
	m_matICamera = scene.GetCurrentCameraIMatrix() ;
	m_vCameraRay = m_matICamera * S3DVector( 0, 0, 1 ) ;
	//
	for ( size_t i = 0; i < m_aInstances.GetLength(); i ++ )
	{
		ThunderInstance *	pThunder = m_aInstances.GetAt( i ) ;
		if ( (pThunder == nullptr)
			|| (pThunder->m_aPoints.GetLength() <= 1) )
		{
			continue ;
		}
		MakeThunder( *pThunder ) ;
		//
		ESLAssert( m_aThickBuf.GetLength() >= m_tcContext.GetPointCount() ) ;
		ESLAssert( m_aAlphaBuf.GetLength() >= m_tcContext.GetPointCount() ) ;
		S3DMeshShaper::AddThickLines
			( *(ppVBs[0]), m_vCameraRay, m_tlpThunder,
				m_tcContext.GetPointCount(),
				m_tcContext.GetPointArray(),
				m_aThickBuf.GetConstArray(),
				m_aAlphaBuf.GetConstArray() ) ;
	}
}

// フレーム描画前処理（全視点・ビュー共通処理）
//////////////////////////////////////////////////////////////////////////////
void S3DThunderMeshController::UpdateMesh
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem,
		S3DVertexBufferInterface ** ppVBs, size_t nVBCount )
{
}

// 稲妻生成
//////////////////////////////////////////////////////////////////////////////
void S3DThunderMeshController::MakeThunder( const ThunderInstance& thunder )
{
	//
	// 稲妻生成
	//
	const size_t		nOrgPoints = thunder.m_aPoints.GetLength() ;
	const S3DDVector *	pvOrgPoints = thunder.m_aPoints.GetConstArray() ;
	ESLAssert( nOrgPoints >= 2 ) ;
	//
	m_tcContext.Create
		( m_random, m_tpParam, nOrgPoints, pvOrgPoints ) ;
	//
	// 元線分長さ計算
	//
	double *	pfpLength = m_aLengthBuf.GetArray( nOrgPoints ) ;
	double *	pfpLength0 = m_aLength0Buf.GetArray( nOrgPoints ) ;
	double *	pfpLength1 = m_aLength1Buf.GetArray( nOrgPoints ) ;
	for ( size_t i = 0; i < nOrgPoints - 1; i ++ )
	{
		pfpLength[i] = (pvOrgPoints[i + 1] - pvOrgPoints[i]).Absolute() ;
	}
	double	fpAccLength = 0 ;
	for ( size_t i = 0; i < nOrgPoints; i ++ )
	{
		pfpLength0[i] = fpAccLength ;
		fpAccLength += pfpLength[i] ;
	}
	fpAccLength = 0 ;
	for ( size_t i = 0; i < nOrgPoints; i ++ )
	{
		size_t	j = nOrgPoints - 1 - i ;
		pfpLength1[j] = fpAccLength ;
		fpAccLength += (j >= 1) ? pfpLength[j - 1] : 0.0 ;
	}
	//
	// 太さ揺れ生成
	//
	float32_t *	pfpThick0 = m_aThick0Buf.GetArray( nOrgPoints ) ;
	for ( size_t i = 0; i < nOrgPoints; i ++ )
	{
		pfpThick0[i] =
			(float32_t) (m_fpThickness
						* (1.0 + m_random.QuickRandomDouble(m_fpThicknessRnd))) ;
	}
	//
	// 太さとα決定
	//
	const size_t		nPointCount = m_tcContext.GetPointCount() ;
	const double *		pfpIndex = m_tcContext.GetIndexArray() ;
	float32_t *			pfpThick = m_aThickBuf.GetArray( nPointCount ) ;
	uint32_t *			pAlpha = m_aAlphaBuf.GetArray( nPointCount ) ;
	float32_t			fpTotalAlpha = 1.0f ;
	float32_t			msecFadeStart =
							(float32_t) (m_secThunderLife - m_secThunderFadeout) * 1000.0f ;
	if ( thunder.m_msecPast > msecFadeStart )
	{
		fpTotalAlpha =
			(float32_t) esl_fmax( 1.0 - (thunder.m_msecPast - msecFadeStart)
											/ (m_secThunderFadeout * 1000.0), 0.0 ) ;
	}

	for ( size_t i = 0; i < nPointCount; i ++ )
	{
		double		fpIndex = pfpIndex[i] ;
		ssize_t		iIndex = (ssize_t) floor( esl_fclamp( fpIndex, 0, (double) (nOrgPoints-1) ) ) ;
		float32_t	dIndex = (float32_t) (fpIndex - iIndex) ;
		if ( (size_t) iIndex >= nOrgPoints - 1 )
		{
			iIndex = (ssize_t) nOrgPoints - 2 ;
			dIndex = 1.0f ;
		}
		ESLAssert( iIndex >= 0 ) ;
		ESLAssert( (size_t) iIndex + 1 < nOrgPoints ) ;
		//
		double	fpTip = 0.0 ;
		if ( m_fpTipLength > 0.00001 )
		{
			double	fpAccLen0 =
				pfpLength0[iIndex] * (1.0 - dIndex) + pfpLength0[iIndex+1] * dIndex ;
			double	fpAccLen1 =
				pfpLength1[iIndex] * (1.0 - dIndex) + pfpLength1[iIndex+1] * dIndex ;
			fpTip = esl_fmin( esl_fmin( fpAccLen0, fpAccLen1 ) / m_fpTipLength, 1.0 ) ;
		}
		float32_t	fpThick = pfpThick0[iIndex] * (1.0f - dIndex)
										+ pfpThick0[iIndex+1] * dIndex ;
		pfpThick[i] = fpThick * (float32_t) (m_fpTipThickness * (1.0 - fpTip) + fpTip) ;

		float32_t	fpAlpha =
			(float32_t) (m_fpTipAlpha * fpTip + (1.0 - fpTip)) * fpTotalAlpha ;
		pAlpha[i] = (uint32_t) esl_roundfi( esl_fclampf( fpAlpha * 256.0f, 0.0f, 256.0f ) ) ;
	}

	m_aLengthBuf.FinishArray() ;
	m_aLength0Buf.FinishArray() ;
	m_aLength1Buf.FinishArray() ;
	m_aThick0Buf.FinishArray() ;
	m_aThickBuf.FinishArray() ;
	m_aAlphaBuf.FinishArray() ;
}

