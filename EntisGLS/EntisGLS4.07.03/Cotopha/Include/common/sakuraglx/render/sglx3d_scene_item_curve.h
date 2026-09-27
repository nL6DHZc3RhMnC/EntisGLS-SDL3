
#if	!defined(__SAKURAGLX3D_SCENE_ITEM_CURVE_H__)
#define	__SAKURAGLX3D_SCENE_ITEM_CURVE_H__	1

#include <sakuraglx/render/sglx3d_scene_composer.h>
#include <sakuraglx/render/sglx3d_scene_item.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// ベジェ曲線・シリアライザ（共通）
	//////////////////////////////////////////////////////////////////////////

	class	S3DItemBezierCurveSerializer
	{
	protected:
		SSystem::SString	m_strDataBase64 ;
		SSystem::SArray
			<S3DSceneComposer::BinaryBezierCurvePoint>
							m_aBezierCurve ;
		bool				m_flagModified ;

	public:
		// 構築関数
		S3DItemBezierCurveSerializer( void ) ;

	public:
		// インスタンシング・リスト設定
		void SetBezierCurveBase64( const wchar_t * pwszBase64 ) ;
		void SetBezierCurve
				( size_t nCount,
					const S3DSceneComposer::BinaryBezierCurvePoint * pbbcp ) ;
		// インスタンシング・リスト取得
		const SSystem::SString& GetBezierCurveBase64( void ) ;
		size_t GetDataLengthInBytes( void ) const ;
		size_t GetBezierCurveData( S3DSceneComposer::BinaryBezierCurveData& bbcd ) const ;
		// インスタンス・データ・パース
		static S3DSceneComposer::BinaryBezierCurveData *
			ParseData( SSystem::SArray<uint8_t>& aBinary,
						const wchar_t * pwszBase64, ssize_t nStrLenght = -1 ) ;
		// インスタンス・データ・エンコード
		static void EncodeData
			( SSystem::SString& strBase64, size_t nCount,
				const S3DSceneComposer::BinaryBezierCurvePoint * pbbcp ) ;
	protected:
		void UpdateDataFromBase64( void ) ;
		void UpdateBezierCurve
				( size_t nCount,
					const S3DSceneComposer::BinaryBezierCurvePoint * pbbcp ) ;

	public:
		// 制御点取得
		size_t GetPointCount( void ) const ;
		const S3DSceneComposer::BinaryBezierCurvePoint * GetPointAt( size_t i ) const ;
		void SetPointAt
			( size_t i, const S3DSceneComposer::BinaryBezierCurvePoint& bbcp ) ;
		void InsertPointAt
			( size_t i, const S3DSceneComposer::BinaryBezierCurvePoint& bbcp ) ;
		void RemovePointAt( size_t i ) ;
		// 編集フラグ設定
		void SetModified( void ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 周期動的インスタンス
	//////////////////////////////////////////////////////////////////////////

	class	S3DCyclicInstanceController
				: public S3DSceneComposer::Controller,
					public S3DInstancingEntryInterface
	{
	public:
		enum	ParameterIndex
		{
			paramPosition,
			paramRotation,
			paramZoom,
			paramTransparency,
			paramColorMul,
			paramColorAdd,
			paramRotationAxisType,
			paramRotationAxisVector,
			paramRotationInit ,
			paramRotationSpeed,
			paramMovingLoopTurn,
			paramMovingInit,
			paramLoopDuration,
			paramBezierCurve,
		} ;

		enum	RotationAxisType
		{
			rotationAxisX,
			rotationAxisY,
			rotationAxisZ,
			rotationAxisVector,
		} ;
		static const SSystem::SXMLDocument::AttrInteger	s_aiRotationAxisType[5] ;

	protected:
		S3DVector			m_vPosition ;
		S3DMatrix			m_matRotation ;
		S3DVector			m_vZoom ;
		S4DMatrix			m_matInstance ;
		S3DColor			m_clrInstance ;

		RotationAxisType	m_rxtType ;
		S3DVector			m_vRotAxis ;
		double				m_degRotInit ;
		double				m_dpsRotSpeed ;
		double				m_radRotation ;

		bool				m_flagMoveTurn ;
		double				m_fpMoveInit ;
		double				m_secMoveDuration ;
		double				m_secMoving ;
		S3DItemBezierCurveSerializer
							m_bezierPath ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2
			( S3DCyclicInstanceController, Controller, S3DInstancingEntryInterface )
		S3D_DECLARE_COMPOSER_ITEM( S3DCyclicInstanceController, cyclic_instance )
		// 構築関数
		S3DCyclicInstanceController( void ) ;

	public:	// S3DInstancingEntryInterface
		// インスタンシング・リスト取得
		virtual size_t GetInstancingArray
			( const S4DMatrix*& pMatrixs, const S3DColor*& pColors ) const ;

	public:	// ParameterProperty
		// パラメータ値取得
		virtual S3DDMatrix GetMatrixParameter( size_t i ) const ;
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		virtual size_t GetBinaryParameter
			( void * pDst, size_t nBufBytes, size_t i ) const ;
		// パラメータ値設定
		virtual void SetMatrixParameter( size_t i, const S3DDMatrix& mat ) ;
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		virtual size_t SetBinaryParameter
			( size_t i, const void * pSrc, size_t nBufBytes ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// Controller
		// タイマー処理
		virtual void OnTimer
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast ) ;
		// フレーム（パラメータ）更新後処理
		virtual void OnUpdateFrame
			( S3DSceneComposer::ItemSerializer * pItem,
				double fpFrame, S3DSceneComposer::SeekMethod seek ) ;

	} ;



	//////////////////////////////////////////////////////////////////////////
	// 周期動的行列コントローラー
	//////////////////////////////////////////////////////////////////////////

	class	S3DCyclicMatrixController
				: public S3DSceneComposer::Controller
	{
	public:
		enum	ParameterIndex
		{
			paramRotationAxisType,
			paramDynamicRotation,
			paramRotationAxisVector,
			paramBaseRotation,
			paramInstanceRotation,
			paramRotationInit,
			paramRotationSpeed,
			paramBasePosition,
			paramMovingCurve,
			paramMovingLoopTurn,
			paramMovingManual,
			paramMovingInit,
			paramLoopDuration,
			paramBezierCurve,
		} ;

		enum	RotationAxisType
		{
			rotationAxisX,
			rotationAxisY,
			rotationAxisZ,
			rotationAxisVector,
		} ;
		enum	DynamicRotation
		{
			dynamicNoRotation,
			dynamicRotationByCamera,		// カメラに正対するように回転する
			dynamicRotationAlongTangent,	// 接線に沿って回転する
		} ;
		static const SSystem::SXMLDocument::AttrInteger	s_aiRotationAxisType[5] ;
		static const SSystem::SXMLDocument::AttrInteger	s_aiDynamicRotation[4] ;

	protected:
		bool				m_flagInstanceRotation ;
		S3DDVector			m_vBasePosition ;
		S3DDMatrix			m_matBaseRotation ;

		RotationAxisType	m_rxtType ;
		DynamicRotation		m_dynamicRotation ;
		S3DDVector			m_vRotAxis ;
		double				m_degRotInit ;
		double				m_dpsRotSpeed ;
		double				m_radRotation ;

		bool				m_flagMoveCurve ;
		bool				m_flagMoveTurn ;
		bool				m_flagMoveManual ;
		double				m_fpMoveInit ;
		double				m_secMoveDuration ;
		double				m_secMoving ;
		S3DItemBezierCurveSerializer
							m_bezierPath ;
		S3DDVector			m_vLastTangent ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DCyclicMatrixController, Controller )
		S3D_DECLARE_COMPOSER_ITEM( S3DCyclicMatrixController, cyclic_matrix )
		// 構築関数
		S3DCyclicMatrixController( void ) ;

	public:	// ParameterProperty
		// パラメータ値取得
		virtual S3DDMatrix GetMatrixParameter( size_t i ) const ;
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		virtual size_t GetBinaryParameter
			( void * pDst, size_t nBufBytes, size_t i ) const ;
		// パラメータ値設定
		virtual void SetMatrixParameter( size_t i, const S3DDMatrix& mat ) ;
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		virtual size_t SetBinaryParameter
			( size_t i, const void * pSrc, size_t nBufBytes ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// Controller
		// タイマー処理
		virtual void OnTimer
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast ) ;
		// フレーム（パラメータ）更新後処理
		virtual void OnUpdateFrame
			( S3DSceneComposer::ItemSerializer * pItem,
				double fpFrame, S3DSceneComposer::SeekMethod seek ) ;

	protected:
		// パラメータ反映
		void ApplyParameters
			( S3DScene * pScene, S3DSceneComposer::CommonSerializer * pItem ) ;
	} ;



	//////////////////////////////////////////////////////////////////////////
	// パス適用モデル生成
	//////////////////////////////////////////////////////////////////////////

	class	S3DMeshBezierPathBuilder
	{
	public:
		// メッシュ情報
		class	MeshInfo
		{
		public:
			S3DModelData::MeshObject *	m_pmoMesh ;
			const S3DVector4 *			m_pvVertex ;
			const S3DVector4 *			m_pvNormal ;
			float32_t					m_zMin ;
			float32_t					m_zMax ;
			SSystem::SArray<size_t>		m_aVertexOrder ;
		public:
			MeshInfo( void )
				: m_pmoMesh(NULL),
					m_pvVertex(NULL), m_pvNormal(NULL),
					m_zMin(0.0f), m_zMax(0.0f) { }
		} ;

		// ベジェ曲線→線分化補完情報
		class	SegmentPoint
		{
		public:
			double			m_zAccCourse ;
			double			m_zLength ;
			double			m_zCourse ;			// decimal of course / unit-length
			size_t			m_nUnit ;			// int-part of course / unit-length
			S3DVector		m_vPoint ;
			S3DVector		m_vZoom ;
			float32_t		m_radGimbal ;
			SGLPalette		m_argbColor ;
			uint32_t		m_nExData ;
			S3DVector		m_vAxisZ ;
			S3DQuaternion	m_qRotate ;
		} ;

		class	SegmentCurve
		{
		public:
			SSystem::SArray<SegmentPoint>	m_aSegPoints ;
			size_t							m_nCorseUnitLen ;
			double							m_zCource ;
			double							m_fpUnitLength ;
		public:
			// ベジェ曲線の線分化
			void PrepareCruve
				( const S3DItemBezierCurveSerializer& bezier, double fpUnitLength ) ;
		} ;

	protected:
		SSystem::SObjectArray<MeshInfo>	m_aMeshInfo ;

	public:
		// 参照元メッシュ情報クリア
		void ClearRefMesh( void ) ;
		// 参照元メッシュ設定
		bool SetupRefMeshAt
			( size_t nIndex, const S3DModelBuffer& model, size_t iMesh ) ;
		void ClearRefMeshAt( size_t nIndex ) ;
		// メッシュ生成
		void BuildMesh
			( S3DVertexBufferInterface & vbDst, const SegmentCurve& curve ) ;

	} ;



	//////////////////////////////////////////////////////////////////////////
	// パス適用モデル生成コントローラー
	//////////////////////////////////////////////////////////////////////////

	class	S3DMeshBezierPathController
				: public S3DMeshBufferItemSerializer::MeshController
	{
	public:
		enum	ParameterIndex
		{
			paramRefModel,
			paramPathBezier,
			paramUnitLength,
			paramMaterialCount,
			paramMeshCount,
			paramRefMesh0,
			paramCount,
		} ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DMeshBezierPathController, MeshController )
		S3D_DECLARE_COMPOSER_ITEM( S3DMeshBezierPathController, path_mesh )
		// 構築関数
		S3DMeshBezierPathController( void ) ;
		// 消滅関数
		virtual ~S3DMeshBezierPathController( void ) ;

	protected:
		double							m_fpUnitLength ;
		SSystem::SString				m_strRefModel ;
		S3DModelBuffer *				m_pRefModel ;
		SSystem::SObjectArray
				<SSystem::SString>		m_aRefMeshID ;
		S3DItemBezierCurveSerializer	m_bezierPath ;

		size_t							m_nRefMeshCount ;	// = m_nMaterialCount * m_nSubMeshCount
		size_t							m_nMaterialCount ;
		size_t							m_nSubMeshCount ;
		SSystem::SObjectArray
			<S3DMeshBezierPathBuilder>	m_aBuilders ;

		SSystem::SObjectArray
				<SSystem::SString>		m_aRefMeshPropID ;
		SSystem::SObjectArray
				<SSystem::SString>		m_aRefMeshPropName ;

	public:
		// 参照モデル更新
		void UpdateReferenceModel( void ) ;
		// 参照メッシュ数変更
		void UpdateReferenceMeshCount( size_t nCount ) ;
		// 参照メッシュ指標更新
		void UpdateReferenceMeshIndexAt( size_t iMaterial, size_t iSubMesh ) ;
		// 参照メッシュインターリーブ
		void InterleaveMeshRef( size_t nNewSubMesh, size_t nOldSubMesh ) ;

	public:	// ParameterProperty
		// パラメータ値取得
		virtual double GetScalarParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		virtual size_t GetBinaryParameter
			( void * pDst, size_t nBufBytes, size_t i ) const ;
		// パラメータ値設定
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		virtual size_t SetBinaryParameter
			( size_t i, const void * pSrc, size_t nBufBytes ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// S3DSceneComposer::Controller
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags ) ;

	public:	// MeshController
		// メッシュ追加処理（全視点・ビュー共通処理）
		virtual void AddMesh
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem,
				S3DVertexBufferInterface ** ppVBs, size_t nVBCount ) ;
		// フレーム描画前処理（全視点・ビュー共通処理）
		virtual void UpdateMesh
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem,
				S3DVertexBufferInterface ** ppVBs, size_t nVBCount ) ;

	} ;



	//////////////////////////////////////////////////////////////////////////
	// 稲妻効果メッシュ生成
	//////////////////////////////////////////////////////////////////////////

	class	S3DThunderMeshController
				: public S3DMeshBufferItemSerializer::MeshController
	{
	public:
		enum	ParameterIndex
		{
			paramGenShape,
			paramGenCount,
			paramGenMinCount,
			paramGenMaxCount,
			paramGenCurve,
			paramGenRadius,
			paramGenRotaion,
			paramGenTipShrink,
			paramThunderLife,
			paramThunderFadeout,
			paramThunderLength,
			paramJointAmplitude,
			paramJointLength,
			paramWaveAmplitude,
			paramWaveLength,
			paramUScale,
			paramVScale,
			paramZBais,
			paramBodyThickness,
			paramThicknessRandom,
			paramTipLength,
			paramTipThickness,
			paramTipAlpha,
			paramColorAlpha0,
			paramColorMul0,
			paramColorAdd0,
			paramColorAlpha1,
			paramColorMul1,
			paramColorAdd1,
			paramColorDivision1,
			paramColorAlpha2,
			paramColorMul2,
			paramColorAdd2,
			paramColorDivision2,
			paramColorAlpha3,
			paramColorMul3,
			paramColorAdd3,
			paramCount,
		} ;
		enum	GenerationShape
		{
			shapeSphere,
			shapeSphereVert,
			shapeSphereHorz,
			shapeRing,
			shapeRadial,
			shapeCurve,
			shapeCount,
		} ;
		static const wchar_t *	s_pwszGenerateShapes[shapeCount] ;
		static const SSystem::SXMLDocument::AttrInteger	s_aiGenerateShapes[shapeCount+1] ;

	protected:
		class	ThunderInstance
		{
		public:
			SSystem::SArray<S3DDVector>	m_aPoints ;
			bool						m_flagFirst ;
			size_t						m_msecLeft ;
			size_t						m_msecPast ;
		public:
			ThunderInstance( void )
				: m_flagFirst( true ),
					m_msecLeft( 0 ), m_msecPast( 0 ) { }
		} ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DThunderMeshController, MeshController )
		S3D_DECLARE_COMPOSER_ITEM( S3DThunderMeshController, thunder_mesh )
		// 構築関数
		S3DThunderMeshController( void ) ;
		// 消滅関数
		virtual ~S3DThunderMeshController( void ) ;

	protected:
		SSystem::SCriticalSection		m_csThunder ;
		SakuraCL::SCLRandomizer			m_random ;
		SSystem::SObjectArray<ThunderInstance>
										m_aInstances ;
		GenerationShape					m_shape ;
		S3DItemBezierCurveSerializer	m_bezierPath ;
		double							m_fpGenCount ;
		int32_t							m_nGenMinCount ;
		int32_t							m_nGenMaxCount ;
		double							m_fpGenRadius ;
		S3DDMatrix						m_matGenRotation ;
		double							m_fpGenTipShrink ;
		double							m_secThunderLife ;
		double							m_secThunderFadeout ;
		double							m_fpThunderLength ;
		double							m_fpThickness ;
		double							m_fpThicknessRnd ;
		double							m_fpTipLength ;
		double							m_fpTipThickness ;
		double							m_fpTipAlpha ;
		S3DMeshShaper::ThunderParam		m_tpParam ;
		S3DMeshShaper::ThickLinesParam	m_tlpThunder ;
		S3DColor						m_clrThunder[4] ;
		float32_t						m_fpColorDiv[4] ;

		S3DMeshShaper::ThunderContext	m_tcContext ;
		S3DMatrix						m_matICamera ;
		S3DVector						m_vCameraRay ;
		SSystem::SArray<double>			m_aLengthBuf ;
		SSystem::SArray<double>			m_aLength0Buf ;
		SSystem::SArray<double>			m_aLength1Buf ;
		SSystem::SArray<float32_t>		m_aThick0Buf ;
		SSystem::SArray<float32_t>		m_aThickBuf ;
		SSystem::SArray<uint32_t>		m_aAlphaBuf ;

	public:
		// 稲妻発生
		void GenerateThunder( size_t nGenCount ) ;
		// 稲妻追加
		void AddThunder
			( const S3DDVector * pvPoints, size_t nCount, size_t msecLeft = 0 ) ;

	protected:
		void GenerateThunderOnSphere( size_t nGenCount ) ;
		void GenerateThunderOnSphereVert( size_t nGenCount ) ;
		void GenerateThunderOnSphereHorz( size_t nGenCount ) ;
		void GenerateThunderOnRing( size_t nGenCount ) ;
		void GenerateThunderRadially( size_t nGenCount ) ;
		void GenerateThunderOnCurve( size_t nGenCount ) ;

	public:	// S3DSceneComposer::Parameter
		// パラメータ値取得
		virtual S3DDMatrix GetMatrixParameter( size_t i ) const ;
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		virtual size_t GetBinaryParameter
			( void * pDst, size_t nBufBytes, size_t i ) const ;
		// パラメータ値設定
		virtual void SetMatrixParameter( size_t i, const S3DDMatrix& mat ) ;
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		virtual size_t SetBinaryParameter
			( size_t i, const void * pSrc, size_t nBufBytes ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// Controller
		// タイマー処理
		virtual void OnTimer
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast ) ;
		// フレーム（パラメータ）更新後処理
		virtual void OnUpdateFrame
			( S3DSceneComposer::ItemSerializer * pItem,
				double fpFrame, S3DSceneComposer::SeekMethod seek ) ;

	public:	// S3DMeshBufferItemSerializer::MeshController
		// メッシュ追加処理（全視点・ビュー共通処理）
		virtual void AddMesh
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem,
				S3DVertexBufferInterface ** ppVBs, size_t nVBCount ) ;
		// フレーム描画前処理（全視点・ビュー共通処理）
		virtual void UpdateMesh
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem,
				S3DVertexBufferInterface ** ppVBs, size_t nVBCount ) ;

	protected:
		// 稲妻生成
		void MakeThunder( const ThunderInstance& thunder ) ;
	} ;


}

#endif
