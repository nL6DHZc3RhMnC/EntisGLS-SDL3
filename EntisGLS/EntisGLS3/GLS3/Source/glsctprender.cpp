
/*****************************************************************************
                          Entis Graphic Library
 -----------------------------------------------------------------------------
    Copyright (C) 2004-2012 Leshade Entis, Entis-soft. Al rights reserved.
 *****************************************************************************/


#include <gls.h>
#include <math.h>


//////////////////////////////////////////////////////////////////////////////
// パーティクル付加モデルジョイント・スクリプト・インターフェース
//////////////////////////////////////////////////////////////////////////////

ECSStrTagArray *	ECSParticleModel::m_staFuncName = NULL ;
const wchar_t *	ECSParticleModel::m_pwszFuncName[8] =
{
	L"SetParticleImageLimit",
	L"SetParticleImage",
	L"SetParticleParameter",
	L"CreateParticle",
	L"SetParticleGenerator",
	L"AdvanceParticleTime",
	L"AddModelToRenderer",
	NULL
} ;
const ECSParticleModel::PFUNC_CALL	ECSParticleModel::m_pfnCallFunc[7] =
{
	&ECSParticleModel::Call_SetParticleImageLimit,
	&ECSParticleModel::Call_SetParticleImage,
	&ECSParticleModel::Call_SetParticleParameter,
	&ECSParticleModel::Call_CreateParticle,
	&ECSParticleModel::Call_SetParticleGenerator,
	&ECSParticleModel::Call_AdvanceParticleTime,
	&ECSParticleModel::Call_AddModelToRenderer,
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSParticleModel, ECSModelJoint )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSParticleModel::ECSParticleModel( void )
{
	m_nGenCount = 0 ;
	m_dwRandom = ::timeGetTime( ) ;
	SetParticleImageLimit( 1 ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSParticleModel::~ECSParticleModel( void )
{
}

// パーティクル画像を設定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleModel::SetParticleImageResource
	( ECSResource * pImage, const E3D_VECTOR_2D * pHotspot, int nIndex )
{
	if ( (unsigned int) nIndex >= m_lstImages.GetSize() )
	{
		return	eslErrGeneral ;
	}
	EParticleImage *	ppi = m_lstImages.GetAt( nIndex ) ;
	ESLAssert( ppi != NULL ) ;
	ppi->m_refImage.SetReference( pImage ) ;
	//
	if ( pImage != NULL )
	{
		return	SetParticleImage( pImage->GetImage(), pHotspot, nIndex ) ;
	}
	else
	{
		return	SetParticleImage( NULL, pHotspot, nIndex ) ;
	}
}

ESLError ECSParticleModel::SetParticleImage
	( EGLAnimation * pImage, const E3D_VECTOR_2D * pHotspot, int nIndex )
{
	if ( (unsigned int) nIndex >= m_lstImages.GetSize() )
	{
		return	eslErrGeneral ;
	}
	EParticleImage *	ppi = m_lstImages.GetAt( nIndex ) ;
	ESLAssert( ppi != NULL ) ;
	ppi->m_pParticleImage = pImage ;
	//
	E3D_VECTOR_2D	vCenter ;
	if ( pHotspot != NULL )
	{
		vCenter.x = pHotspot->x ;
		vCenter.y = pHotspot->y ;
	}
	else if ( pImage != NULL )
	{
		vCenter.x = (REAL32) (pImage->GetWidth() * 0.5) ;
		vCenter.y = (REAL32) (pImage->GetHeight() * 0.5) ;
	}
	else
	{
		vCenter.x = 0 ;
		vCenter.y = 0 ;
	}
	//
	if ( (ppi->m_ppmModel != NULL) && ppi->m_fOwnModel )
	{
		delete	ppi->m_ppmModel ;
	}
	ppi->m_ppmModel = NULL ;
	ppi->m_fOwnModel = false ;
	//
	if ( (pImage != NULL) && (pImage->GetInfo() != NULL) )
	{
		ppi->m_ppmModel = new E3DPolygonModel ;
		ppi->m_fOwnModel = true ;
		ppi->m_vImageCenter = vCenter ;
		//
		ppi->m_ppmModel->CreateImagePrimitive
			( pImage->GetInfo(), NULL, &vCenter, NULL ) ;
	}
	return	eslErrSuccess ;
}

ESLError ECSParticleModel::SetParticleModelObject
	( ECSPolygonModel * pModel, int nIndex )
{
	if ( (unsigned int) nIndex >= m_lstImages.GetSize() )
	{
		return	eslErrGeneral ;
	}
	EParticleImage *	ppi = m_lstImages.GetAt( nIndex ) ;
	ESLAssert( ppi != NULL ) ;
	ppi->m_refImage.SetReference( pModel ) ;
	//
	return	SetParticleModel( pModel, nIndex ) ;
}

ESLError ECSParticleModel::SetParticleModel
	( E3DPolygonModel * pModel, int nIndex )
{
	if ( (unsigned int) nIndex >= m_lstImages.GetSize() )
	{
		return	eslErrGeneral ;
	}
	EParticleImage *	ppi = m_lstImages.GetAt( nIndex ) ;
	ESLAssert( ppi != NULL ) ;
	//
	if ( (ppi->m_ppmModel != NULL) && ppi->m_fOwnModel )
	{
		delete	ppi->m_ppmModel ;
	}
	ppi->m_pParticleImage = NULL ;
	ppi->m_ppmModel = pModel ;
	ppi->m_fOwnModel = false ;
	//
	return	eslErrSuccess ;
}

// パーティクル画像取得
//////////////////////////////////////////////////////////////////////////////
EGLAnimation * ECSParticleModel::GetParticleImage( int nIndex )
{
	EParticleImage *	ppi = m_lstImages.GetAt( nIndex ) ;
	if ( ppi != NULL )
	{
		return	ppi->m_pParticleImage ;
	}
	return	NULL ;
}

E3DPolygonModel * ECSParticleModel::GetParticleModel( int nIndex )
{
	EParticleImage *	ppi = m_lstImages.GetAt( nIndex ) ;
	if ( ppi != NULL )
	{
		return	ppi->m_ppmModel ;
	}
	return	NULL ;
}

// パーティクル画像の最大数を設定する
//////////////////////////////////////////////////////////////////////////////
void ECSParticleModel::SetParticleImageLimit( int nLimit )
{
	m_lstImages.SetSize( nLimit ) ;
	//
	for ( int i = 0; i < nLimit; i ++ )
	{
		if ( m_lstImages.GetAt( i ) == NULL )
		{
			m_lstImages.SetAt( i, new EParticleImage ) ;
		}
	}
}

// パーティクルパラメータ設定
//////////////////////////////////////////////////////////////////////////////
void ECSParticleModel::SetParticleParameter( const PARTICLE_PARAM & param )
{
	m_ppParam = param ;
}

// パーティクルを生成する
//////////////////////////////////////////////////////////////////////////////
void ECSParticleModel::CreateParticle( int nCount )
{
	const double	pi_by180 = 3.1415926535897932384626433832795 / 180 ;
	E3D_REV_MATRIX	revmat ;
	E3DVector		vUnit( 1, 1, 1 ) ;
	for ( int i = 0; i < nCount; i ++ )
	{
		PARTICLE *	pp = new PARTICLE ;
		m_lstParticles.Add( pp ) ;
		//
		pp->iParticleImage = Random( m_lstImages.GetSize() ) ;
		//
		pp->nPastTime = 0 ;
		pp->nAnimeTime = 0 ;
		pp->vPos.x =
			(REAL32) (m_ppParam.vGenWidth.x
							* (Random(0x8000) - 0x4000) / 0x4000) ;
		pp->vPos.y =
			(REAL32) (m_ppParam.vGenWidth.y
							* (Random(0x8000) - 0x4000) / 0x4000) ;
		pp->vPos.z =
			(REAL32) (m_ppParam.vGenWidth.z
							* (Random(0x8000) - 0x4000) / 0x4000) ;
		pp->vShow = pp->vPos ;
		//
		double	v0 = m_ppParam.rGenVelocity
						+ m_ppParam.rGenVelocityRange * Random(0x1000) / 0x1000 ;
		double	va = m_ppParam.rGenAngleRange * Random(0x1000) / 0x1000 ;
		va *= pi_by180 ;
		//
		pp->vVelocity.x = (REAL32) (v0 * sin( va )) ;
		pp->vVelocity.y = 0 ;
		pp->vVelocity.z = (REAL32) (v0 * cos( va )) ;
		//
		va = (360.0 * pi_by180) * Random(0x1000) / 0x1000 ;
		revmat.InitializeMatrix( vUnit ) ;
		revmat.RevolveForAngle( m_ppParam.vGenAngle ) ;
		revmat.RevolveOnZ( sin( va ), cos( va ) ) ;
		revmat.RevolveVector( pp->vVelocity ) ;
		//
		pp->vVelocity +=
			m_ppParam.vGenSpeed
				* (1 + m_ppParam.rGenSpeedRange * Random(0x1000) / 0x1000) ;
		//
		pp->vAcceleration.x = 0 ;
		pp->vAcceleration.y = 0 ;
		pp->vAcceleration.z = 0 ;
		//
		va = (m_ppParam.rRevRevRange * pi_by180) * Random(0x1000) / 0x1000 ;
		revmat.InitializeMatrix( vUnit ) ;
		revmat.RevolveForAngle( m_ppParam.vRevRevAxis ) ;
		revmat.RevolveOnZ( sin( va ), cos( va ) ) ;
		revmat.RevolveByAngleOn( m_ppParam.vRevRevAxis ) ;
		//
		pp->vRevAxis = m_ppParam.vRevBaseAxis ;
		revmat.RevolveVector( pp->vRevAxis ) ;
		//
		pp->rRevAngle = 0 ;
		pp->rRevSpeed = m_ppParam.rRevSpeed
						+ m_ppParam.rRevSpeedRange * Random(0x1000) / 0x1000 ;
		pp->rZoom = m_ppParam.rZoom
						+ m_ppParam.rZoomRange * Random(0x1000) / 0x1000 ;
		//
		for ( int j = 0; j < 2; j ++ )
		{
			pp->pfFlickness[j].rAmplitude =
				m_ppParam.pfFlickness[j].rAmplitude
					+ m_ppParam.pfFlickness[j].rAmplitudeRange
											* Random(0x1000) / 0x1000 ;
			pp->pfFlickness[j].rFrequency =
				m_ppParam.pfFlickness[j].rFrequency
					+ m_ppParam.pfFlickness[j].rFrequencyRange
											* Random(0x1000) / 0x1000 ;
			//
			va = (360.0 * pi_by180) * Random(0x1000) / 0x1000 ;
			pp->vFlickUnit[j].x = (REAL32) cos( va ) ;
			pp->vFlickUnit[j].y = (REAL32) sin( va ) ;
			pp->vFlickUnit[j].z = 0 ;
			//
			revmat.InitializeMatrix( vUnit ) ;
			revmat.RevolveForAngle( pp->vVelocity ) ;
			revmat.RevolveVector( pp->vFlickUnit[j] ) ;
		}
	}
}

// パーティクル生成数を設定する（/100sec）
//////////////////////////////////////////////////////////////////////////////
void ECSParticleModel::SetParticleGenerator( int nCount )
{
	m_nGenCount = nCount ;
}

// 乱数生成
//////////////////////////////////////////////////////////////////////////////
long int ECSParticleModel::Random( long int nLimit )
{
	m_dwRandom = m_dwRandom * 5 + 0x9A731651 ;
	if ( nLimit <= 0 )
	{
		return	0 ;
	}
	return	(long int) (m_dwRandom >> 8) % nLimit ;
}

// ジョイント生成
//////////////////////////////////////////////////////////////////////////////
E3DModelJoint * ECSParticleModel::CreateJoint( void ) const
{
	return	new ECSParticleModel ;
}

// ジョイントアニメーション
//////////////////////////////////////////////////////////////////////////////
bool ECSParticleModel::OnAdvanceAnimation( unsigned int nTime )
{
	DWORD	dwPastTime = nTime ;
	if ( dwPastTime > 1000 )
	{
		dwPastTime = 1000 ;
	}
	bool	fUpdate = AdvanceParticleTime( nTime ) ;
	//
	fUpdate |= ECSModelJoint::OnAdvanceAnimation( nTime ) ;
	return	fUpdate ;
}

// パーティクルアニメーション
//////////////////////////////////////////////////////////////////////////////
bool ECSParticleModel::AdvanceParticleTime( int nPastTime )
{
	bool	fUpdate = false ;
	//
	if ( nPastTime > 0 )
	{
		int		i, nCount, nAnimeLength = 0 ;
		nCount = m_lstParticles.GetSize() ;
		fUpdate |= (nCount > 0) ;
		//
		// パーティクル生成
		//
		int	nGenCount = (int) ((INT64) m_nGenCount * nPastTime / 100000) ;
		int	nGenOdd = (int) ((INT64) m_nGenCount * nPastTime % 100000) ;
		if ( nGenOdd > Random( 100000 ) )
		{
			nGenCount ++ ;
		}
		if ( nGenCount > 0 )
		{
			CreateParticle( nGenCount ) ;
			fUpdate = true ;
		}
		//
		// パーティクル移動
		//
		for ( i = 0; i < nCount; i ++ )
		{
			PARTICLE *	pp = m_lstParticles.GetAt( i ) ;
			if ( pp != NULL )
			{
				AdvanceParticlePosition( pp, nPastTime ) ;
				//
				EGLAnimation *	pParticleImage =
					GetParticleImage( pp->iParticleImage ) ;
				nAnimeLength = 0 ;
				if ( pParticleImage != NULL )
				{
					nAnimeLength = pParticleImage->GetTotalTime( ) ;
				}
				//
				if ( (pp->nPastTime >= m_ppParam.nDuration)
					&& ((nAnimeLength == 0)
						|| (m_ppParam.nFlags & pfAnimationLoop)) )
				{
					m_lstParticles.SetAt( i, NULL ) ;
				}
				else if ( nAnimeLength != 0 )
				{
					if ( m_ppParam.nFlags & pfAnimationLoop )
					{
						pp->nAnimeTime %= nAnimeLength ;
					}
					else if ( (int) pp->nAnimeTime >= nAnimeLength )
					{
						m_lstParticles.SetAt( i, NULL ) ;
					}
				}
			}
		}
		m_lstParticles.TrimEmpty( ) ;
	}
	return	fUpdate ;
}

// パーティクルの座標更新
//////////////////////////////////////////////////////////////////////////////
void ECSParticleModel::AdvanceParticlePosition
	( PARTICLE * pp, int nPastTime ) const
{
	double	rPastSec = nPastTime * 0.001 ;
	pp->nPastTime += nPastTime ;
	pp->nAnimeTime +=
		(int) ((INT64) nPastTime * m_ppParam.nAnimationSpeed / 0x100) ;
	//
	if ( m_ppParam.rShrink != 0 )
	{
		double	rShrink = 1.0 - m_ppParam.rShrink ;
		pp->vVelocity *= pow( rShrink, rPastSec ) ;
	}
	//
	pp->vAcceleration += m_ppParam.vGravity * rPastSec ;
	//
	pp->vPos += pp->vVelocity * rPastSec ;
	pp->vPos += pp->vAcceleration ;
	pp->vPos += m_ppParam.vStream * rPastSec ;
	//
	pp->rRevAngle += pp->rRevSpeed * rPastSec ;
	//
	const double	pi_x2 = 3.1415926535897932384626433832795 * 2 / 1000.0 ;
	pp->vShow = pp->vPos ;
	for ( int j = 0; j < 2; j ++ )
	{
		if ( (pp->pfFlickness[j].rAmplitude != 0)
			&& (pp->pfFlickness[j].rFrequency != 0) )
		{
			double	amp = pp->pfFlickness[j].rAmplitude
				* sin( pp->nPastTime * pi_x2 / pp->pfFlickness[j].rFrequency ) ;
			pp->vShow += pp->vFlickUnit[j] * amp ;
		}
	}
}

// モデルをレンダリングバッファに追加する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleModel::AddModelToRender( E3DRenderPolygon & render )
{
	const double	pi_by180 = 3.1415926535897932384626433832795 / 180.0 ;
	ESLError	err = ECSModelJoint::AddModelToRender( render ) ;
	if ( !err )
	{
		E3D_COLOR *	pColor = NULL ;
		if ( ((m_clrModelColor.rgbMul.dwPixelCode & 0xFFFFFF) != 0xFFFFFF)
			|| ((m_clrModelColor.rgbAdd.dwPixelCode & 0xFFFFFF) != 0) )
		{
			pColor = &m_clrModelColor ;
		}
		//
		E3DModelJoint		mjParticle ;
		mjParticle.m_parent = this ;
		EPtrObjArray<E3DPolygonModel> &
							mdlist = mjParticle.ModelList( ) ;
		E3DDFVector &		jntpos = mjParticle.Position( ) ;
		E3DDFRevMatrix &	revmat = mjParticle.Matrix( ) ;
		E3DDFVector			vUnit ;
		int	i, nCount ;
		nCount = m_lstParticles.GetSize( ) ;
		for ( i = 0; i < nCount; i ++ )
		{
			PARTICLE *	pp = m_lstParticles.GetAt( i ) ;
			ESLAssert( pp != NULL ) ;
			if ( pp == NULL )
			{
				continue ;
			}
			EParticleImage *
				ppi = m_lstImages.GetAt( pp->iParticleImage ) ;
			if ( (ppi == NULL) || (ppi->m_ppmModel == NULL) )
			{
				continue ;
			}
			if ( ppi->m_pParticleImage != NULL )
			{
				PEGL_IMAGE_INFO	pInfo =
					ppi->m_pParticleImage->GetFrameAt
						( ppi->m_pParticleImage->TimeToSequence( pp->nAnimeTime ) ) ;
				if ( pInfo == NULL )
				{
					continue ;
				}
				ppi->m_ppmModel->AttachImagePolygon
					( pInfo, NULL, &(ppi->m_vImageCenter) ) ;
			}
			mdlist.SetAt( 0, ppi->m_ppmModel ) ;
			//
			double			rZoom = pp->rZoom ;
			unsigned int	nTransparency = m_nTransparency ;
			if ( pp->nPastTime < m_ppParam.nFadein )
			{
				nTransparency =
					m_ppParam.nFadeTransparency
						* (m_ppParam.nFadein - pp->nPastTime) / m_ppParam.nFadein ;
				nTransparency =
					0x100 - (0x100 - nTransparency)
								* (0x100 - m_nTransparency) / 0x100 ;
				rZoom = rZoom + (m_ppParam.rFadeZoom - rZoom)
									* pp->nPastTime / m_ppParam.nFadein ;
			}
			else if ( (m_ppParam.nFadeout > 0)
				&& (pp->nPastTime > (m_ppParam.nDuration - m_ppParam.nFadeout)) )
			{
				int	nFadeout =
					pp->nPastTime - (m_ppParam.nDuration - m_ppParam.nFadeout) ;
				nTransparency =
					m_ppParam.nFadeTransparency * nFadeout / m_ppParam.nFadeout ;
				nTransparency =
					0x100 - (0x100 - nTransparency)
								* (0x100 - m_nTransparency) / 0x100 ;
				rZoom = rZoom + (m_ppParam.rFadeZoom - rZoom)
									* nFadeout / m_ppParam.nFadeout ;
			}
			//
			double	rad = pp->rRevAngle * pi_by180 ;
			vUnit.x = vUnit.y = vUnit.z = rZoom ;
			revmat.InitializeMatrix( vUnit ) ;	// パーティクル回転
			revmat.RevolveForAngle( E3DDFVector( pp->vRevAxis ) ) ;
			revmat.RevolveOnZ( sin( rad ), cos( rad ) ) ;
			revmat.RevolveByAngleOn( E3DDFVector( pp->vRevAxis ) ) ;
			//
			jntpos = pp->vShow ;
			//
//			m_rvmat.RevolveMatrix( revmat ) ;	// 親ジョイント回転反映
//			m_rvmat.RevolveVector( jntpos ) ;
//			jntpos += m_vmove ;
			//
			render.AddModel( mjParticle, pColor, nTransparency ) ;
		}
	}
	return	err ;
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSParticleModel::GetTypeName( void ) const
{
	return	L"ParticleModel" ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSParticleModel::Duplicate( void )
{
	ECSParticleModel *	pJoint = new ECSParticleModel ;
	pJoint->CopyModelJoint( *this ) ;
	return	pJoint ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleModel::Move( ECSContext & context, ECSObject * obj )
{
	ECSModelJoint *	pJoint =
		ESLTypeCast<ECSModelJoint>( ECSObject::GetEntity( obj ) ) ;
	if ( pJoint == NULL )
	{
		return	ESLErrorMsg( "ParticleModel への不正な代入操作です。" ) ;
	}
	CopyModelJoint( *pJoint ) ;
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleModel::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	nIndex = m_staFuncName->FindIndex( pwszName ) ;
	if ( nIndex < 0 )
	{
		if ( ECSModelJoint::GetFunction( context, nIndex, pwszName ) )
		{
			return	ESLErrorMsg
				( "ParticleModel の定義されていない"
					"メンバ関数を呼び出そうとしました。" ) ;
		}
		nIndex += m_staFuncName->GetSize( ) ;
	}
	return	eslErrSuccess ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleModel::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	if ( (unsigned int) nIndex < m_staFuncName->GetSize() )
	{
		return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
	}
	return	ECSModelJoint::CallFunction
		( context, nIndex - m_staFuncName->GetSize(), lstArg ) ;
}

// 全てのメンバ変数にインデックスを振る
//////////////////////////////////////////////////////////////////////////////
void ECSParticleModel::IndexAllMember( void )
{
	ECSModelJoint::IndexAllMember( ) ;
	//
	for ( int i = 0; i < (int) m_lstImages.GetSize(); i ++ )
	{
		m_lstImages[i].m_refImage.IndexAllMember() ;
	}
}

// 全てのメンバ変数の参照を解消する
//////////////////////////////////////////////////////////////////////////////
void ECSParticleModel::CleanupAllReference( ECSContext & context )
{
	ECSModelJoint::CleanupAllReference( context ) ;
	//
	for ( int i = 0; i < (int) m_lstImages.GetSize(); i ++ )
	{
		EParticleImage *	ppi = m_lstImages.GetAt( i ) ;
		if ( ppi != NULL )
		{
			ppi->m_refImage.CleanupAllReference( context ) ;
			ppi->m_pParticleImage = NULL ;
			if ( ppi->m_fOwnModel )
			{
				delete	ppi->m_ppmModel ;
				ppi->m_fOwnModel = false ;
			}
			ppi->m_ppmModel = NULL ;
		}
	}
}

// 全てのメンバ変数の参照を解決する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleModel::CommitAllReference( ECSContext & context )
{
	ESLError	err = ECSModelJoint::CommitAllReference( context ) ;
	if ( err )
	{
//		return	err ;
	}
	//
	for ( int i = 0; i < (int) m_lstImages.GetSize(); i ++ )
	{
		EParticleImage *	ppi = m_lstImages.GetAt( i ) ;
		if ( ppi == NULL )
		{
			continue ;
		}
		err = ppi->m_refImage.CommitAllReference( context ) ;
		if ( err )
		{
			break ;
		}
		ECSResource *	prsImage =
			ESLTypeCast<ECSResource>( ppi->m_refImage.m_pRef ) ;
		if ( prsImage != NULL )
		{
			E3D_VECTOR_2D	vCenter = ppi->m_vImageCenter ;
			SetParticleImage( prsImage->GetImage(), &vCenter, i ) ;
			ppi->m_vImageCenter = vCenter ;
		}
		else
		{
			ECSPolygonModel *	ppmModel =
				ESLTypeCast<ECSPolygonModel>( ppi->m_refImage.m_pRef ) ;
			SetParticleModel( ppmModel, i ) ;
		}
	}
	//
	return	err ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleModel::Save( ESLFileObject & file, ECSContext & context )
{
	ESLError	err ;
	err = ECSModelJoint::Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	int	i ;
	int	nImageCount = m_lstImages.GetSize( ) ;
	file.Write( &nImageCount, sizeof(nImageCount) ) ;
	for ( i = 0; i < nImageCount; i ++ )
	{
		err = m_lstImages[i].m_refImage.Save( file, context ) ;
		if ( err )
		{
			return	err ;
		}
		file.Write( &(m_lstImages[i].m_vImageCenter), sizeof(E3D_VECTOR_2D) ) ;
	}
	//
	file.Write( &m_nGenCount, sizeof(m_nGenCount) ) ;
	file.Write( &m_ppParam, sizeof(m_ppParam) ) ;
	//
	int	nParticles = m_lstParticles.GetSize() ;
	file.Write( &nParticles, sizeof(nParticles) ) ;
	for ( i = 0; i < nParticles; i ++ )
	{
		PARTICLE *	pp = m_lstParticles.GetAt( i ) ;
		ESLAssert( pp != NULL ) ;
		file.Write( pp, sizeof(PARTICLE) ) ;
	}
	//
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleModel::Load( ESLFileObject & file, ECSContext & context )
{
	ESLError	err ;
	err = ECSModelJoint::Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	int	i ;
	int	nImageCount ;
	file.Read( &nImageCount, sizeof(nImageCount) ) ;
	SetParticleImageLimit( nImageCount ) ;
	for ( i = 0; i < nImageCount; i ++ )
	{
		err = m_lstImages[i].m_refImage.Load( file, context ) ;
		if ( err )
		{
			return	err ;
		}
		file.Read( &(m_lstImages[i].m_vImageCenter), sizeof(E3D_VECTOR_2D) ) ;
	}
	//
	file.Read( &m_nGenCount, sizeof(m_nGenCount) ) ;
	file.Read( &m_ppParam, sizeof(m_ppParam) ) ;
	//
	int	nParticles ;
	file.Read( &nParticles, sizeof(nParticles) ) ;
	m_lstParticles.RemoveAll( ) ;
	for ( i = 0; i < nParticles; i ++ )
	{
		PARTICLE *	pp = new PARTICLE ;
		file.Read( pp, sizeof(PARTICLE) ) ;
		m_lstParticles.Add( pp ) ;
	}
	//
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleModel::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	return	ECSModelJoint::DumpObject( buf, nIndent, context ) ;
}

// メンバ関数 : SetParticleImageLimit( Integer nLimit )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleModel::Call_SetParticleImageLimit
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	int	nLimit ;
	err = context.GetArgumentAsInt( nLimit, lstArg, 1, 1 ) ;
	if ( err )
		return	err ;
	//
	SetParticleImageLimit( nLimit ) ;
	//
	return	context.PushObject( context.new_CSInteger( 0 ) ) ;
}

// メンバ関数 : SetParticleImage
//		( Reference rsImage [, Vector2D vHotspot [, Integer nIndex]] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleModel::Call_SetParticleImage
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 4 ) ;
	if ( err )
		return	err ;
	//
	E3D_VECTOR_2D *	pvHotspot = NULL ;
	E3D_VECTOR_2D	vHotspot ;
	int				nIndex ;
	ECSResource *	prsImage =
		ESLTypeCast<ECSResource>
			( context.GetArgumentObjectAs( lstArg, 1, L"Resource" ) ) ;
	ECSPolygonModel *	prsModel =
		ESLTypeCast<ECSPolygonModel>
			( context.GetArgumentObjectAs( lstArg, 1, L"PolygonModel" ) ) ;
	ECSStructureInterface *	pstptHotspot =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 2, L"Vector2D" ) ) ;
	if ( pstptHotspot != NULL )
	{
		vHotspot.x = (REAL32) pstptHotspot->GetMemberAsReal( L"x", 0 ) ;
		vHotspot.y = (REAL32) pstptHotspot->GetMemberAsReal( L"y", 0 ) ;
		pvHotspot = &vHotspot ;
	}
	err = context.GetArgumentAsInt( nIndex, lstArg, 3, 0 ) ;
	if ( err )
		return	err ;
	//
	if ( prsImage != NULL )
	{
		err = SetParticleImageResource( prsImage, pvHotspot, nIndex ) ;
	}
	else if ( prsModel != NULL )
	{
		err = SetParticleModelObject( prsModel, nIndex ) ;
	}
	else
	{
		err = SetParticleImageResource( NULL, pvHotspot, nIndex ) ;
	}
	//
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : SetParticleParameter( ParticleParam3D param )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleModel::Call_SetParticleParameter
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSStructure *	pcpp =
		ESLTypeCast<ECSStructure>
			( context.GetArgumentObjectAs( lstArg, 1, L"ParticleParam3D" ) ) ;
	if ( pcpp != NULL )
	{
		PARTICLE_PARAM	pp ;
		::eslFillMemory( &pp, 0, sizeof(pp) ) ;
		//
		pp.nFlags = pcpp->GetMemberAsInt( L"nFlags", 0 ) ;
		pp.nDuration = pcpp->GetMemberAsInt( L"nDuration", 0 ) ;
		pp.nAnimationSpeed = pcpp->GetMemberAsInt( L"nAnimationSpeed", 0 ) ;
		pp.nFadein = pcpp->GetMemberAsInt( L"nFadein", 0 ) ;
		pp.nFadeout = pcpp->GetMemberAsInt( L"nFadeout", 0 ) ;
		pp.nFadeTransparency = pcpp->GetMemberAsInt( L"nFadeTransparency", 0 ) ;
		pp.rFadeZoom = pcpp->GetMemberAsReal( L"rFadeZoom", 0 ) ;
		//
		CopyVector3DofMember( pp.vGenWidth, pcpp, L"vGenWidth" ) ;
		CopyVector3DofMember( pp.vGenAngle, pcpp, L"vGenAngle" ) ;
		//
		pp.rGenAngleRange = pcpp->GetMemberAsReal( L"rGenAngleRange", 0 ) ;
		pp.rGenVelocity = pcpp->GetMemberAsReal( L"rGenVelocity", 0 ) ;
		pp.rGenVelocityRange = pcpp->GetMemberAsReal( L"rGenVelocityRange", 0 ) ;
		pp.rShrink = pcpp->GetMemberAsReal( L"rShrink", 0 ) ;
		//
		CopyVector3DofMember( pp.vRevBaseAxis, pcpp, L"vRevBaseAxis" ) ;
		//
		pp.rRevSpeed = pcpp->GetMemberAsReal( L"rRevSpeed", 0 ) ;
		pp.rRevSpeedRange = pcpp->GetMemberAsReal( L"rRevSpeedRange", 0 ) ;
		//
		CopyVector3DofMember( pp.vRevRevAxis, pcpp, L"vRevRevAxis" ) ;
		//
		pp.rRevRevRange = pcpp->GetMemberAsReal( L"rRevRevRange", 0 ) ;
		pp.rZoom = pcpp->GetMemberAsReal( L"rZoom", 0 ) ;
		pp.rZoomRange = pcpp->GetMemberAsReal( L"rZoomRange", 0 ) ;
		//
		ECSArray *	pfFlickness2 =
			ESLTypeCast<ECSArray>( pcpp->GetMemberAs( L"pfFlickness" ) ) ;
		if ( pfFlickness2 != NULL )
		{
			for ( int i = 0; i < 2; i ++ )
			{
				ECSStructure *	pcpf =
					ESLTypeCast<ECSStructure>( pfFlickness2->m_varArray.GetAt(i) ) ;
				if ( pcpf != NULL )
				{
					pp.pfFlickness[i].rAmplitude =
						pcpf->GetMemberAsReal( L"rAmplitude", 0 ) ;
					pp.pfFlickness[i].rAmplitudeRange =
						pcpf->GetMemberAsReal( L"rAmplitudeRange", 0 ) ;
					pp.pfFlickness[i].rFrequency =
						pcpf->GetMemberAsReal( L"rFrequency", 0 ) ;
					pp.pfFlickness[i].rFrequencyRange =
						pcpf->GetMemberAsReal( L"rFrequencyRange", 0 ) ;
				}
			}
		}
		//
		CopyVector3DofMember( pp.vGenSpeed, pcpp, L"vGenSpeed" ) ;
		pp.rGenSpeedRange = pcpp->GetMemberAsReal( L"rGenSpeedRange", 0 ) ;
		//
		CopyVector3DofMember( pp.vStream, pcpp, L"vStream" ) ;
		CopyVector3DofMember( pp.vGravity, pcpp, L"vGravity" ) ;
		//
		SetParticleParameter( pp ) ;
	}
	//
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

void ECSParticleModel::CopyVector3DofMember
	( E3D_VECTOR & v, ECSStructure * ph, const wchar_t * pwszName )
{
	ECSStructureInterface *	pcv ;
	pcv = ESLTypeCast<ECSStructureInterface>( ph->GetMemberAs( pwszName ) ) ;
	if ( pcv != NULL )
	{
		v.x = (REAL32) pcv->GetMemberAsReal( L"x", 0 ) ;
		v.y = (REAL32) pcv->GetMemberAsReal( L"y", 0 ) ;
		v.z = (REAL32) pcv->GetMemberAsReal( L"z", 0 ) ;
	}
}

// メンバ関数 : CreateParticle( Integer nCount )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleModel::Call_CreateParticle
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	nCount ;
	err = context.GetArgumentAsInt( nCount, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	CreateParticle( nCount ) ;
	//
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : SetParticleGenerator( Integer nCount )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleModel::Call_SetParticleGenerator
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	nCount ;
	err = context.GetArgumentAsInt( nCount, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	SetParticleGenerator( nCount ) ;
	//
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : AdvanceParticleTime( Integer nPastTime )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleModel::Call_AdvanceParticleTime
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	nPastTime ;
	err = context.GetArgumentAsInt( nPastTime, lstArg, 1, 1 ) ;
	if ( err )
		return	err ;
	//
	AdvanceParticleTime( nPastTime ) ;
	//
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : AddModelToRenderer( Reference render )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSParticleModel::Call_AddModelToRenderer
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	E3DRenderPolygon *	render =
		ESLTypeCast<E3DRenderPolygon>( ECSObject::GetEntity( lstArg.GetAt(1) ) ) ;
	if ( render != NULL )
	{
		err = AddModelToRender( *render ) ;
	}
	else
	{
		err = eslErrGeneral ;
	}
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}


