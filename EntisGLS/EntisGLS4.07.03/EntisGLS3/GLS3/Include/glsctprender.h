
/*****************************************************************************
                          Entis Graphic Library
 -----------------------------------------------------------------------------
    Copyright (C) 2004-2007 Leshade Entis, Entis-soft. Al rights reserved.
 *****************************************************************************/


#if	!defined(__GLSCTPRENDER_H__)
#define	__GLSCTPRENDER_H__


//////////////////////////////////////////////////////////////////////////////
// クラス
//////////////////////////////////////////////////////////////////////////////

class	ECSPolygonModel ;
class	ECSModelJoint ;
class	ECSRenderSprite ;


//////////////////////////////////////////////////////////////////////////////
// モデルデータ・スクリプト・インターフェース
//////////////////////////////////////////////////////////////////////////////

#include <glscsobj_polygon_model.h>


//////////////////////////////////////////////////////////////////////////////
// モデルジョイント・スクリプト・インターフェース
//////////////////////////////////////////////////////////////////////////////

#include <glscsobj_model_joint.h>


//////////////////////////////////////////////////////////////////////////////
// パーティクル付加モデルジョイント・スクリプト・インターフェース
//////////////////////////////////////////////////////////////////////////////

class	ECSParticleModel	: public ECSModelJoint
{
public:
	// 構築関数
	ECSParticleModel( void ) ;
	// 消滅関数
	virtual ~ECSParticleModel( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSParticleModel, ECSModelJoint )

public:
	enum	ParticleFlag
	{
		pfAnimationLoop	= 0x0001,	// 画像のアニメーションをループさせる
	} ;
	struct	PARTICLE_FLICK
	{
		double			rAmplitude ;		// 揺らぎ幅 [pixel]
		double			rAmplitudeRange ;	// 揺らぎ幅（乱数） [pixel]
		double			rFrequency ;		// 揺らぎ周期 [sec]
		double			rFrequencyRange ;	// 揺らぎ周期（乱数） [sec]
	} ;
	struct	PARTICLE_PARAM
	{
		unsigned int	nFlags ;			// フラグ
		unsigned int	nDuration ;			// 寿命 [ms]
		unsigned int	nAnimationSpeed ;	// アニメーション速度比 x100H
											//（アニメ画像パーティクル用）
		unsigned int	nFadein ;			// フェードイン時間 [ms]
		unsigned int	nFadeout ;			// フェードアウト時間 [ms]
		unsigned int	nFadeTransparency ;	// フェードアウト透明度
		double			rFadeZoom ;			// フェードアウト時の拡大比率
		E3D_VECTOR		vGenWidth ;			// 発生幅
		E3D_VECTOR		vGenAngle ;			// 初速基底（rGenAngleRange の基準）
		double			rGenAngleRange ;	// 発生角の幅（乱数） [deg]
		double			rGenVelocity ;		// （中心から遠ざかる）初速 [/sec]
		double			rGenVelocityRange ;	// 初速の幅（乱数） [/sec]
		double			rShrink ;			// 初速減速率 [/sec]
		E3D_VECTOR		vRevBaseAxis ;		// 回転軸
		double			rRevSpeed ;			// 回転速度 [deg/sec]
		double			rRevSpeedRange ;	// 回転速度の幅（乱数）[deg/sec]
		E3D_VECTOR		vRevRevAxis ;		// 回転軸を回転させる基底（ランダム用）
		double			rRevRevRange ;		// 回転軸の回転幅（乱数）[deg]
		double			rZoom ;				// 拡大率
		double			rZoomRange ;		// 拡大率の幅（乱数）
		PARTICLE_FLICK	pfFlickness[2] ;	// 揺らぎ
		E3D_VECTOR		vGenSpeed ;			// 初速ベクトル [pixel/sec]
		double			rGenSpeedRange ;	// 初速の幅（乱数）[pixel/sec]
		E3D_VECTOR		vStream ;			// 流速 [pixel/sec]
		E3D_VECTOR		vGravity ;			// 重力加速度 [pixel/sec/sec]
	} ;
	struct	PARTICLE
	{
		unsigned int	iParticleImage ;	// 画像番号
		unsigned int	nPastTime ;			// 経過時間 [ms]
		unsigned int	nAnimeTime ;		// アニメーション用時間 [ms]
		E3D_VECTOR		vShow ;				// 表示座標
		E3D_VECTOR		vPos ;				// 座標
		E3D_VECTOR		vVelocity ;			// （初）速度 [pixel/sec]
		E3D_VECTOR		vAcceleration ;		// 現在加速度 [pixel/sec/sec]
											// 実速度 = vVelocity + vAcceleration
		E3D_VECTOR		vRevAxis ;			// 回転軸
		double			rRevAngle ;			// 回転角度 [deg]
		double			rRevSpeed ;			// 回転速度 [deg/sec]
		double			rZoom ;				// 拡大率
		PARTICLE_FLICK	pfFlickness[2] ;	// 揺らぎ
		E3D_VECTOR		vFlickUnit[2] ;		// 揺らぎ基底ベクトル
	} ;

protected:
	unsigned int		m_nGenCount ;		// パーティクル生成数 [/100sec]
	DWORD				m_dwRandom ;		// 乱数の種

	class	EParticleImage
	{
	public:
		ECSReference		m_refImage ;		// 粒子参照先
		EGLAnimation *		m_pParticleImage ;	// 粒子画像
		E3D_VECTOR_2D		m_vImageCenter ;	// 粒子ホットスポット
		E3DPolygonModel *	m_ppmModel ;		// 粒子モデル
		bool				m_fOwnModel ;
	public:
		EParticleImage( void )
			: m_pParticleImage( NULL ),
				m_ppmModel( NULL ), m_fOwnModel( false ) { }
		~EParticleImage( void )
			{
				if ( m_fOwnModel && m_ppmModel )
				{
					delete	m_ppmModel ;
				}
			}
	} ;
	EObjArray<EParticleImage>
						m_lstImages ;		// パーティクル画像リスト

	PARTICLE_PARAM		m_ppParam ;			// パラメータ
	EObjArray<PARTICLE>	m_lstParticles ;	// 粒子配列

public:
	// パーティクル画像を設定
	ESLError SetParticleImageResource
		( ECSResource * pImage,
			const E3D_VECTOR_2D * pHotspot = NULL, int nIndex = 0 ) ;
	ESLError SetParticleImage
		( EGLAnimation * pImage,
			const E3D_VECTOR_2D * pHotspot = NULL, int nIndex = 0 ) ;
	ESLError SetParticleModelObject
		( ECSPolygonModel * pModel, int nIndex = 0 ) ;
	ESLError SetParticleModel
		( E3DPolygonModel * pModel, int nIndex = 0 ) ;
	// パーティクル画像取得
	EGLAnimation * GetParticleImage( int nIndex = 0 ) ;
	E3DPolygonModel * GetParticleModel( int nIndex = 0 ) ;
	// パーティクル画像の最大数を設定する
	void SetParticleImageLimit( int nLimit ) ;
	// パーティクルパラメータ設定
	void SetParticleParameter( const PARTICLE_PARAM & param ) ;
	// パーティクルパラメータ取得
	const PARTICLE_PARAM & GetParticleParameter( void ) const
		{
			return	m_ppParam ;
		}
	// パーティクルを生成する
	void CreateParticle( int nCount ) ;
	// パーティクル生成数を設定する（/100sec）
	void SetParticleGenerator( int nCount ) ;
	// 乱数生成
	long int Random( long int nLimit ) ;
	// 現在のパーティクル数を取得する
	int GetCurrentParticleCount( void ) const
		{
			return	m_lstParticles.GetSize( ) ;
		}

public:
	// ジョイント生成
	virtual E3DModelJoint * CreateJoint( void ) const ;
	// ジョイントアニメーション
	virtual bool OnAdvanceAnimation( unsigned int nTime ) ;
	// パーティクルアニメーション
	bool AdvanceParticleTime( int nPastTime ) ;
	// パーティクルの座標更新
	void AdvanceParticlePosition
		( PARTICLE * pp, int nPastTime ) const ;
	// モデルをレンダリングバッファに追加する
	virtual ESLError AddModelToRender( E3DRenderPolygon & render ) ;

public:		// 通常のオブジェクト処理
	// オブジェクトの型名を取得する
	virtual const wchar_t * GetTypeName( void ) const ;
	// オブジェクトを複製
	virtual ECSObject * Duplicate( void ) ;
	// オブジェクトを代入
	virtual ESLError Move( ECSContext & context, ECSObject * obj ) ;
	// メンバ関数インデックス取得
	virtual ESLError GetFunction
		( ECSContext & context, int & nIndex, const wchar_t * pwszName ) ;
	// メンバ関数呼び出し
	virtual ESLError CallFunction
		( ECSContext & context,
			int nIndex, ECSObjArray<ECSObject> & lstArg ) ;

public:		// シリアル化のための関数（システムによって必要）
	// 全てのメンバ変数にインデックスを振る
	virtual void IndexAllMember( void ) ;
	// 全てのメンバ変数の参照を解消する
	virtual void CleanupAllReference( ECSContext & context ) ;
	// 全てのメンバ変数の参照を解決する
	virtual ESLError CommitAllReference( ECSContext & context ) ;
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;

public:
	// メンバ関数プロトタイプ
	typedef	ESLError (ECSParticleModel::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[8] ;
	static const PFUNC_CALL	m_pfnCallFunc[7] ;

protected:
	static void CopyVector3DofMember
		( E3D_VECTOR & v, ECSStructure * ph, const wchar_t * pwszName ) ;
	// メンバ関数
	ESLError Call_SetParticleImageLimit
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetParticleImage
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetParticleParameter
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CreateParticle
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetParticleGenerator
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_AdvanceParticleTime
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_AddModelToRenderer
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// レンダリングスプライト・スクリプト・インターフェース
//////////////////////////////////////////////////////////////////////////////

#include <glscsobj_render_sprite.h>



#endif
