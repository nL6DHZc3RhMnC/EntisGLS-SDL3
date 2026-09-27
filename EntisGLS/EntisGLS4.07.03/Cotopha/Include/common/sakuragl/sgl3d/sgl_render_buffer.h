
#if	!defined(__SAKURAGL_RENDER_BUFFER_H__)
#define	__SAKURAGL_RENDER_BUFFER_H__	1

#include <sakura/ssys_stack_buffer.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// トライアングル・ストリップ→インデックスリスト用一時バッファ
	//////////////////////////////////////////////////////////////////////////

	class	S3DTemporaryIndexTriangleStrip	: public SSystem::SArray<uint32_t>
	{
	public:
		// 構築関数
		S3DTemporaryIndexTriangleStrip( void ) ;
		// 消滅関数
		~S3DTemporaryIndexTriangleStrip( void ) ;
		// トライアングルストリップのインデックスリストを取得
		const uint32_t * MakeIndexList( size_t countTriangleStrip ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 法線計算用一時バッファ
	//////////////////////////////////////////////////////////////////////////

	class	S3DTemporaryNormalBuffer
	{
	protected:
		SSystem::SArray<S3DVector4>	m_bufNormals ;

	public:
		// 構築関数
		S3DTemporaryNormalBuffer( void ) ;
		// 消滅関数
		~S3DTemporaryNormalBuffer( void ) ;
		// 法線取得
		const S3DVector4 * GetNormalBuffer( void ) const ;
		// プリミティブリストの法線を計算
		bool SetForIndexedPrimitiveList
			( S3DPrimitiveType typePrimitive,
				size_t countPrimitive, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S2DVector * pvUVMap,
				const uint32_t * pIndexedList ) ;
		// 三角ポリゴンリストの法線を計算
		void SetForIndexedTriangleList
			( size_t countPolygon, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S2DVector * pvUVMap,
				const uint32_t * pIndexedList ) ;
		void SetForTriangleList
			( size_t countPolygon, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S2DVector * pvUVMap ) ;
		// トライアングルストリップの法線を計算
		void SetForTriangleStrip
			( size_t countTriangleStrip,
				const S3DVector4 * pvVertex,
				const S2DVector * pvUVMap ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// テクスチャ基底ベクトル計算用一時バッファ
	//////////////////////////////////////////////////////////////////////////

	class	S3DTemporaryTextureAxisBuffer
	{
	protected:
		SSystem::SArray<S3DVector4>	m_bufAxisX ;
		SSystem::SArray<S3DVector4>	m_bufAxisY ;
		SSystem::SArray<int32_t>	m_bufCount ;

	public:
		// 構築関数
		S3DTemporaryTextureAxisBuffer( void ) ;
		// 消滅関数
		~S3DTemporaryTextureAxisBuffer( void ) ;
		// バッファ取得
		const S3DVector4 * GetBufferAxisX( void ) const ;
		const S3DVector4 * GetBufferAxisY( void ) const ;
		// プリミティブリストの基底ベクトルを計算
		bool SetForIndexedPrimitiveList
			( S3DPrimitiveType typePrimitive,
				size_t countPrimitive, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S2DVector * pvUVMap,
				const uint32_t * pIndexedList ) ;
		// 三角ポリゴンリストの基底ベクトルを計算
		void SetForIndexedTriangleList
			( size_t countPolygon, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S2DVector * pvUVMap,
				const uint32_t * pIndexedList ) ;
		// トライアングルストリップの基底ベクトルを計算
		void SetForTriangleStrip
			( size_t countTriangleStrip,
				const S3DVector4 * pvVertex,
				const S2DVector * pvUVMap ) ;
		// テクスチャｘｙ基底ベクトル計算
		static void TextureBaseAxis
			( S3DVector& vTexAxisX, S3DVector& vTexAxisY,
				const S3DVector& vDelta1, const S3DVector& vDelta2,
				const S2DVector& vTexDelta1, const S2DVector& vTexDelta2 ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// レンダリング・バッファ
	//////////////////////////////////////////////////////////////////////////

	class	S3DRenderBuffer	: public S3DVertexBufferInterface
	{
	public:
		// カスタムシェーダーパラメータ
		struct	CustomUniformEntry
		{
			const wchar_t *					m_pwszID ;
			S3DCustomShader::UniformType	m_type ;
			void *							m_pData ;
			size_t							m_nLength ;
		} ;
		enum	ShaderType
		{
			shaderDefaultNoShade,
			shaderDefaultGouraud,
			shaderDefaultPhong,
			shaderDefaultPhongToon,
			shaderUserCustom,
		} ;
		struct	ShaderContextEntry
		{
			OptionalContextSet		optContext ;
			CustomUniformEntry *	pUniformBuf ;
			size_t					nUnitofms ;
		} ;

		typedef	S3DCustomShader::UniformData	CustomUniform ;
		typedef	S3DCustomShader::UniformSet		CustomUniformSet ;

		// 座標変換
		struct	Transformation
		{
			S3DDMatrix			matTransform ;
			S3DDVector			vTransform ;
			S3DColor			colorEffect ;
			unsigned int		nTransparency ;
			uint32_t			nPriority ;
			OptionalContextSet	optContext ;	// ※現在の値ではなく、Pop 時に Push 字の値を復元する為

			// 初期化
			void InitTransformation( void ) ;
		} ;

		struct	TransformationList	: public Transformation
		{
			TransformationList *	pPrev ;
			Transformation *		pTransBuf ;
			CustomUniformSet *		pcus ;
			ShaderContextEntry *	pContextBuf ;
			CustomUniformEntry *	pUniformBuf ;
			size_t					nUnitofms ;

			// 構築関数
			TransformationList( void )
					: pPrev( NULL ), pTransBuf( NULL ),
						pcus( NULL ), pContextBuf( NULL ),
						pUniformBuf( NULL ), nUnitofms( 0 )
			{
				InitTransformation() ;
			}
			// 消滅関数
			~TransformationList( void )
			{
				delete	pcus ;
				pcus = NULL ;
			}
			// 初期化
			void InitTransformation( void )
			{
				Transformation::InitTransformation() ;
				//
				pTransBuf = NULL ;
				delete	pcus ;
				pcus = NULL ;
				pContextBuf = NULL ;
				pUniformBuf = NULL ;
				nUnitofms = 0 ;
			}
		} ;

		// メッシュバッファ
		class	MeshBuffer
		{
		public:
			S3DMaterial *				m_pMaterial ;
			S3DPrimitiveType			m_type ;
			size_t						m_nVertexCount ;
			size_t						m_nExAttrCount ;
			size_t						m_nIndexCount ;
			SSystem::SArray<S3DVector4>	m_bufVertex ;
			SSystem::SArray<S3DVector4>	m_bufNormal ;
			SSystem::SArray<S2DVector>	m_bufUVMap ;
			SSystem::SArray<S3DColor>	m_bufColor ;
			SSystem::SArray<float32_t>	m_bufExAttr ;
			SSystem::SArray<uint32_t>	m_bufIndex ;

		public:
			// 構築関数
			MeshBuffer( void )
				: m_pMaterial( NULL ), m_type( primitiveTriangle ),
					m_nVertexCount( 0 ), m_nExAttrCount( 0 ), m_nIndexCount( 0 ) { }
			MeshBuffer( const MeshBuffer& buf ) ;
			// 代入
			const MeshBuffer& operator = ( const MeshBuffer& buf ) ;
			// 結合
			const MeshBuffer& operator += ( const MeshBuffer& buf ) ;
			SGLError AddIndexedPrimitiveList
				( S3DMaterial * pMaterial,
					S3DPrimitiveType type,
					size_t countIndex,
					size_t countVertex, size_t countExAttr,
					const S3DVector4 * pvVertex,
					const S3DVector4 * pvNormal,
					const S2DVector * pvUVMap,
					const S3DColor * pColor,
					const float32_t * pfpExAttrs,
					const uint32_t * pIndexedList,
					const Transformation * pTransform = nullptr ) ;
			// プリミティブが空か？
			bool IsEmpty( void ) const ;
			// 描画頂点数（インデックス数）取得
			size_t GetIndexCount( void ) const ;
			// クリア
			void ClearBuffer( void ) ;
			// バッファ解放
			void FreeBuffer( void ) ;
			// 出力
			SGLError RenderToVertexBuffer( S3DVertexBufferInterface& vbo ) const ;
			SGLError Render( S3DRenderBufferInterface& render ) const ;
			// 頂点変換
			void Transform( const S3DMatrix& matTransform, const S3DVector& vMove ) ;
			// 外接直方体
			bool GetCircumscribedBox( S3DVector& vMin, S3DVector& vMax ) const ;
		} ;

		// メッシュ・バリアント・パラメータ
		struct	MESH_VARIANT
		{
			bool			flagRenderable ;
			bool			flagUpdateBone ;
			bool			flagUpdateMorph ;
			bool			flagUpdateMaterial ;
			S3DMaterial *	pMaterial ;
			size_t			countBone ;
			S3DMatrix *		pBoneMatrix ;
			S3DVector *		pBoneTrans ;
			size_t			countMorph ;
			ssize_t *		pMorphTargetMesh ;
			float32_t *		pMorphApplication ;
			size_t			nTargetMeshCount ;
		} ;

		// レンダリングエントリ
		struct	RENDER_ENTRY
		{
			uint64_t					nSortHash ;
			uint32_t					nType ;			// enum S3DPrimitiveType
			uint32_t					nFlags ;
			//
			Transformation *			pTransform ;
			ShaderContextEntry *		pShaderContext ;
			//
			S3DVertexBufferInterface *	pVertexBuffer ;
			size_t						iFirstBuf ;
			ssize_t						iEndBuf ;
			//
			bool						flagRenderable ;
			size_t						countPrimitive ;
			size_t						countIndex ;
			size_t						countIndexLimit ;
			size_t						countVertex ;
			S3DVector					vVertexMax ;
			S3DVector					vVertexMin ;
			S3DVector					vCenter ;
			float32_t					fpRadius ;
			//
			S3DMaterial *				pMaterial ;
			S3DVector4 *				pvVertex ;
			S3DVector4 *				pvNormal ;
			S2DVector *					pvUVMap ;
			S3DColor *					pColor ;
			S3DVector4 *				pvTexAxisX ;
			S3DVector4 *				pvTexAxisY ;
			uint32_t *					pIndexedList ;
			uint32_t *					pSubIndexedList[countSubMesh] ;
			uint32_t					nSubIndexCount[countSubMesh] ;
			ssize_t						iSubMeshSelector ;
			float32_t					fpSubMeshDensity ;
			//
			size_t						nExAttrElements ;
			float32_t *					pfpExAttrElements ;
			//
			size_t						nInstancingCount ;
			S4DMatrix *					pInstancingMatrix ;
			S3DColor *					pInstancingColor ;
			//
			size_t						countBone ;
			size_t						countWeightMap ;
			bool						flagUpdateBone ;
			size_t						countFullBone ;
			float32_t **				ppWeightMap ;
			uint32_t **					ppJointMap ;
			S3DMatrix *					pBoneMatrix ;
			S3DVector *					pBoneTrans ;
			S3DVector4 *				pvTempVertex ;
			S3DVector4 *				pvTempNormal ;
			S2DVector *					pvTempUVMap ;
			S3DColor *					pvTempColor ;
			S3DVector4 *				pvTempMorphVertex ;
			S3DVector4 *				pvTempMorphNormal ;
			float32_t *					pfpTempMorphWeight ;
			S3DVector4 *				pvTempBoneVertex ;
			S3DVector4 *				pvTempBoneNormal ;
			//
			size_t						countMorph ;
			bool						flagUpdateMorph ;
			bool						flagMorphWithWeight ;
			ssize_t *					pMorphTargetMesh ;
			float32_t *					pMorphApplication ;
			size_t						nTargetMeshCount ;
			S3DVector4 *				pvMorphVertex ;
			S3DVector4 *				pvMorphNormal ;
			S2DVector *					pvMorphUVMap ;
			S3DColor *					pMorphColor ;
			S3DVector4 *				pvMorphTexAxisX ;
			S3DVector4 *				pvMorphTexAxisY ;
			bool *						pbMorphWeight ;
			float32_t *					pfpMorphWeight ;

			// モーフィングの有無
			bool IsMorphing( void ) const
			{
				return	(countMorph != 0) && (nTargetMeshCount != 0) ;
			}
		} ;
		enum	AddRenderFlagEx
		{
			renderAutoNormal	= 0x0100,
			renderAutoColor		= 0x0200,
			renderAutoTexAxis	= 0x0400,
			renderNormalizeFace	= 0x0800,		// ※ S3DModelBuffer で実装
		} ;
		enum	RenderType
		{
			typeVertexBuffer		= -1,
			typeIndexedTriangleList	= primitiveTriangle,
			typeTriangleStrip		= primitiveTriangleStrip,
		} ;

		// バリアント・バッファ
		class	VariantBuffer : public S3DVertexVariantBuffer
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( VariantBuffer, S3DVertexVariantBuffer )
			// 構築関数
			VariantBuffer( void ) ;
			// メッシュ表示状態を反映する
			void ReflectMeshVisibleTo( S3DRenderBuffer * pBuffer ) ;

		protected:
			SSystem::SArray<MESH_VARIANT>	m_arrMeshVars ;
			SSystem::SArray<S3DMatrix>		m_bufBoneMatrix ;
			SSystem::SArray<S3DVector>		m_bufBoneTrans ;
			SSystem::SArray<ssize_t>		m_bufMorphTarget ;
			SSystem::SArray<float32_t>		m_bufMorphApplication ;

		public:
			// メッシュにボーン行列設定
			virtual SGLError SetBoneMatrix
				( size_t iMesh, size_t nCount,
					const S3DMatrix * pMatrix, const S3DVector * pTrans ) ;
			// メッシュのボーン行列取得
			virtual size_t GetBoneMatrix
				( size_t iMesh, size_t nCount,
					S3DMatrix * pMatrix, S3DVector * pTrans ) ;
			// モーフィング設定
			virtual SGLError SetMorphingApplication
				( size_t iMesh, const ssize_t * pTargetMesh,
					const float32_t * pApplication, size_t nTargetMeshCount ) ;
			// モーフィング設定取得
			virtual SGLError GetMorphingApplication
				( size_t iMesh, ssize_t& iTargetMesh,
						float32_t& fpApplication, size_t iTargetMeshIndex ) ;
			// メッシュ表示設定
			virtual SGLError EnableToRenderMesh
				( size_t iFirst = 0, ssize_t iEnd = -1, bool fEnable = true ) ;
			// メッシュ表示フラグ取得
			virtual bool IsEnabledToRenderMesh( size_t iMesh ) const ;
			// メッシュマテリアル設定
			virtual SGLError SetMaterialToRenderMesh
				( size_t iMesh, S3DMaterial * pMaterial ) ;
			// メッシュマテリアル取得
			virtual S3DMaterial * GetMaterialToRenderMesh( size_t iMesh ) const ;

			friend class S3DRenderBuffer ;
		} ;

	protected:
		S3DMaterial *			m_pDefaultMaterial ;
		uint32_t				m_nBufCtrlFlags ;	// complex of enum BufferControlFlag

		SSystem::SCriticalSection	m_csBufSync ;
		SSystem::SSignalEvent		m_signalFreeRefMesh ;	// m_nRefShiftMesh==0 時シグナル

		SSystem::SPointerArray<RENDER_ENTRY>
								m_arrRender ;
		size_t					m_iShiftOffset ;
		atomic_int_t			m_nRefShiftMesh ;
		size_t					m_iFenceOrder ;
		SSystem::SStackBuffer	m_bufRender ;
		bool					m_flagSorting ;
		ShaderContextEntry *	m_pLastShaderContext ;

		TransformationList *	m_pTransformation ;
		TransformationList *	m_pGarbage ;

		S3DDMatrix				m_matCamera ;
		S3DDVector				m_vCameraPos ;

		bool					m_flagCircumscribed ;
		S3DVector				m_vMinParallelepiped ;
		S3DVector				m_vMaxParallelepiped ;
		S3DVector				m_vCircumscribedCenter ;
		double					m_fpCircumscribedRadius ;

		OptionalContextSet		m_optContext ;
		uint32_t				m_nCurShaderHash ;
		SSystem::SPtrSortArray<S3DCustomShader,uint32_t>
								m_psaShaderMap ;
		uint32_t				m_nRenderPriority ;

		SSystem::SArray<S4DMatrix>		m_bufTempInstancingMatrix ;
		SSystem::SArray<S3DColor>		m_bufTempInstancingColor ;

		S3DTemporaryNormalBuffer		m_bufTempNormal ;
		S3DTemporaryTextureAxisBuffer	m_bufTempTexAxis ;

		S3DVertexVariantBuffer *	m_pvvbLast ;

		S3DVertexDeviceBufferInterface *	m_pFirstDevBuf ;

		SSystem::SObjectArray<ESLObject>	m_arrayTemporary ;

		class	MergedPrimitiveBuffer
		{
		public:
			S3DMaterial *				m_pMaterial ;
			RENDER_ENTRY *				m_preRel ;
			bool						m_flagAllocPrmBuf ;
			PrimitiveBuffer				m_prmbuf ;
			size_t						m_nVertexCount ;
			size_t						m_nIndexCount ;
			size_t						m_nMaxVertexCount ;
			size_t						m_nMaxIndexCount ;
			//
			SSystem::SArray<S3DVector4>	m_bufVertex ;
			SSystem::SArray<S3DVector4>	m_bufNormal ;
			SSystem::SArray<S2DVector>	m_bufUVMap ;
			SSystem::SArray<S3DColor>	m_bufColor ;
			SSystem::SArray<uint32_t>	m_bufIndex ;

		public:
			MergedPrimitiveBuffer( void )
				: m_pMaterial(NULL), m_preRel(NULL),
					m_flagAllocPrmBuf(false),
					m_nVertexCount(0), m_nIndexCount(0),
					m_nMaxVertexCount(0), m_nMaxIndexCount(0) { }
			~MergedPrimitiveBuffer( void ) { }
		} ;
		enum	PrimitiveTypeIndex
		{
			indexPrimitivePoints,
			indexPrimitiveLines,
			indexPrimitiveTriangles,
			indexPrimitiveCount,
		} ;
		MergedPrimitiveBuffer	m_mpbuf[indexPrimitiveCount] ;

		static const PrimitiveTypeIndex	m_iPrimitiveIndex[primitiveCount] ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( SakuraGL::S3DRenderBuffer, S3DRenderBufferInterface )
		// 構築関数
		S3DRenderBuffer( void ) ;
		S3DRenderBuffer( const S3DRenderBuffer & buf ) ;
		// 消滅関数
		virtual ~S3DRenderBuffer( void ) ;
		// バッファにデータがあるか？
		virtual bool IsEmptyBuffer( void ) const ;
		// 複製
		const S3DRenderBuffer & operator = ( const S3DRenderBuffer & buf ) ;
		// 3D 変換行列を取得
		bool GetTransformMatrix( S3DMatrix& mat, S3DVector& pos ) const ;
		// 透明度を取得
		unsigned int GetTransparency( void ) const ;
		// 色効果を取得
		bool GetColorEffect( S3DColor& colorEffect ) const ;
		// 以前に Push された Transformation 取得
		const Transformation * GetPrevTransformation( void ) const ;
		// ソートを有効／無効化
		void EnableSorting( bool flagSorting ) ;
		// デフォルトシェーダータイプ
		static ShaderType GetDefaultShaderType( uint64_t nShadingMethod ) ;
		// カメラ変換行列設定
		void SetCamera
			( const S3DDMatrix& matCamera, const S3DDVector& posCamera ) ;
		// 直接 S3DRenderBufferInterface へ出力
		SGLError RenderTemporaryBufferTo
			( S3DRenderBufferInterface * render,
					uint64_t flagsExclusion = 0,
					size_t iFirst = 0, ssize_t iEnd = -1,
					size_t nInstancing = 0,
					const S4DMatrix * pmatInstancing = NULL,
					const S3DColor * pColorInstancing = NULL ) const ;
		// Flush でフェンスされた範囲を順次 S3DRenderBufferInterface へ出力し、
		// スレッドセーフに順次バッファリストから除外する
		SGLError FlushRenderTemporaryBufferTo
			( S3DRenderBufferInterface * render,
					uint64_t flagsExclusion = 0,
					size_t iFirst = 0, ssize_t iEnd = -1 ) ;
	protected:
		SGLError RenderEntryTemporaryBufferTo
			( S3DRenderBufferInterface * render,
				RENDER_ENTRY * pre,
				ShaderContextEntry *& pShaderContext, OptionalContextSet& optContext ) ;

	public:
		// 指定の VertexBuffer 又は バリアントを
		// VertexBuffer として S3DRenderBufferInterface へ出力
		SGLError RenderVertexBufferTo
			( S3DRenderBufferInterface * render,
				S3DVertexBufferInterface * pvb,
				uint64_t flagsExclusion = 0,
				size_t iFirst = 0, ssize_t iEnd = -1,
				size_t nInstancing = 0,
				const S4DMatrix * pmatInstancing = NULL,
				const S3DColor * pColorInstancing = NULL ) const ;
		// インスタンス結合
		size_t MergeEntryInstancing
			( size_t iFirst, size_t nCount, const S3DDVector& vInstancingBase ) ;
		// メッシュエントリ取得
		RENDER_ENTRY * GetMeshEntryAt( size_t iMesh ) const ;
		// 先頭のメッシュエントリを取得し、バッファのリストから除外する
		RENDER_ENTRY * ShiftMeshEntry( void ) ;
		// ShiftMeshEntry で取得した RENDER_ENTRY の参照を完了した
		void ReleaseShiftMeshEntry( RENDER_ENTRY * pre ) ;
		// 法線・頂点色・接線要素の生成
		void NormalizeVertexElements( RENDER_ENTRY& entry, uint32_t nFlags ) ;
		// モーフィングによる頂点変換処理
		void MorphMeshVertics( RENDER_ENTRY& entry ) ;
		void MorphMeshInfoVertics
			( MeshInfo& info, RENDER_ENTRY& entry, size_t iFirst, size_t nCount ) ;
		// ボーンによる頂点変換処理
		void TransformMeshVerticsByBone( RENDER_ENTRY& entry ) ;
		void TransformMeshVerticsByBone
			( MeshInfo& info, RENDER_ENTRY& entry, size_t iFirst, size_t nCount ) ;
		// メッシュ外接球取得（モーフ・ボーン変形有り）
		double GetCircumscribedSphereTransformed( S3DVector& vCenter ) ;
		// スレッド排他処理
		void LockSyncBuffer( void ) const ;
		void UnlockSyncBuffer( void ) const ;
		bool TestLockedSyncBuffer( void ) const ;
		// 全てのプリミティブが同一タイプ・同一マテリアル・同一拡張属性数で、
		// ボーン・モーフィングがない場合、結合して単一のプリミティブとして再構築する
		SGLError RebuildAsSinglePrimitive( void ) ;
		SGLError GetMeshBufferAsSinglePrimitive( MeshBuffer& mbuf ) const ;
		static SGLError RebuildAsSinglePrimitiveForVB( S3DVertexBufferInterface& vb ) ;
		static SGLError GetMeshBufferAsSinglePrimitiveOfVB
						( MeshBuffer& mbuf, S3DVertexBufferInterface& vb ) ;
		// 現在の変換情報取得
		const Transformation * GetTransformation( void ) const
		{
			return	m_pTransformation ;
		}

	public:	// S3DRenderBufferInterface オーバーライド
		// シェーディング設定
		virtual void SetShadingFlag( uint64_t nShadingMethod ) ;
		// シェーディング取得
		virtual uint64_t GetShadingFlag( void ) ;
		// カスタムシェーダー設定
		virtual SGLError AttachCustomShader( S3DCustomShader * pShader ) ;
		// カスタムシェーダー取得
		virtual S3DCustomShader * GetCustomShader( void ) const ;
		// 輪郭描画色設定
		virtual void SetOffsetBorderColor( uint32_t rgbBorder ) ;
		// 輪郭描画オフセット係数設定 (ax+b)
		virtual void SetOffsetBorderCoefficient( float32_t a, float32_t b ) ;
		// オプショナル機能設定
		virtual SGLError SetOptionalFeature
			( FeatureType feature, int32_t nParam1,
						const void * pParam2, size_t sizeOfParam2 ) ;
		// オプショナル機能取得
		virtual SGLError GetOptionalFeature
			( FeatureType feature, int32_t nParam1,
						void * pParam2, size_t sizeOfParam2 ) const ;
	protected:
		void FlushOptionalContextBuffer( void ) ;
	public:
		// 描画座標空間設定
		virtual SGLError AppendMatrixTransformation
			( const S3DDMatrix& mat, const S3DDVector& pos,
				const S3DColor * color = NULL,
				unsigned int nTransparency = 0 ) ;
		virtual SGLError SetMatrixTransformation
			( const S3DDMatrix& mat, const S3DDVector& pos,
				const S3DColor * color = NULL,
				unsigned int nTransparency = 0 ) ;
		virtual SGLError GetMatrixTransformation
			( S3DDMatrix& mat, S3DDVector& pos,
				S3DColor * color = NULL,
				unsigned int * pTransparency = NULL ) const ;
		virtual SGLError PushTransformation( void ) ;
		virtual SGLError PopTransformation( void ) ;
		virtual SGLError ResetTransformation( void ) ;
		// カスタムシェーダーパラメータ設定
		virtual SGLError SetCustomShaderUniform
			( const wchar_t * pwszUniformId,
				S3DCustomShader::UniformType type,
				const void * pData, size_t nCount ) ;
		virtual SGLError ResetCustomShaderUniform( void ) ;
		// ポリゴンリストをレンダリングバッファに追加
		virtual SGLError AddIndexedTriangleList
			( S3DMaterial * pMaterial, uint32_t nFlags,
				size_t countPolygon, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) ;
		// トライアングルストリップをレンダリングバッファに追加
		virtual SGLError AddTriangleStrip
			( S3DMaterial * pMaterial, uint32_t nFlags,
				size_t countTriangleStrip,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap, const S3DColor * pColor ) ;
		// プリミティブリストをレンダリングバッファに追加
		virtual SGLError AddIndexedPrimitiveList
			( S3DMaterial * pMaterial, uint32_t nFlags,
				S3DPrimitiveType typePrimitive,
				size_t countIndex, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) ;
		// プリミティブを追加するためのバッファを確保する
		virtual SGLError AllocatePrimitiveBuffer
			( PrimitiveBuffer& prmbuf,
				S3DPrimitiveType typePrimitive,
				size_t countIndex, size_t countVertex ) ;
		// プリミティブを追加する（バッファの管理は S3DVertexBufferInterface に移る）
		virtual SGLError AddPrimitiveBuffer
			( S3DMaterial * pMaterial, uint32_t nFlags,
				S3DPrimitiveType typePrimitive,
				const PrimitiveBuffer& prmbuf,
				size_t countIndex, size_t countVertex ) ;
		// プリミティブを追加せずにバッファを開放する
		virtual SGLError FreePrimitiveBuffer
			( const PrimitiveBuffer& prmbuf ) ;
		// 頂点バッファの内容を描画
		virtual SGLError AddVertexBuffer
			( S3DMaterial * pMaterial, uint32_t nFlags,
				S3DVertexBufferInterface * pBuffer,
				size_t iFirst = 0, ssize_t iEnd = -1,
				size_t nInstancing = 0,
				const S4DMatrix * pmatInstancing = NULL,
				const S3DColor * pColorInstancing = NULL ) ;
		// 描画の確定
		virtual SGLError Flush( void ) ;
		// 遅延削除オブジェクト追加（Flush 時に削除）
		virtual void AddTemporaryObject( ESLObject * pObj ) ;
		// バッファ制御フラグ
		virtual uint32_t GetBufferControlFlags( void ) const ;
		virtual void SetBufferControlFlags( uint32_t nFlags ) ;
		// デフォルトマテリアル
		virtual S3DMaterial * GetDefaultMaterial( void ) const ;
		virtual void AttachDefaultMaterial( S3DMaterial * pMaterial ) ;
		// メッシュ数を取得する
		virtual size_t GetMeshCount( void ) const ;
		// メッシュ情報取得
		virtual SGLError GetMeshInfoAt
			( MeshInfo& info, size_t iMesh, size_t nCopyVertices,
				size_t iFirstVertex = 0, uint32_t nFlags = 0 ) const ;
		// ポリゴンリストを更新
		virtual SGLError UpdateIndexedTriangleList
			( size_t iMesh, uint32_t nFlags,
				size_t countPolygon, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) ;
		// トライアングルストリップを更新
		virtual SGLError UpdateTriangleStrip
			( size_t iMesh, uint32_t nFlags,
				size_t countTriangleStrip,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap, const S3DColor * pColor ) ;
		// プリミティブリストを更新
		virtual SGLError UpdateIndexedPrimitiveList
			( size_t iMesh, uint32_t nFlags,
				size_t countIndex, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) ;
		// サブメッシュ（ポリゴンリスト）を更新
		virtual SGLError UpdateSubIndexedTriangleList
			( size_t iMesh, size_t iSubMesh, uint32_t nFlags,
				size_t countPolygon, const uint32_t * pIndexedList ) ;
		// サブメッシュ切り替えｚ座標比を設定する
		virtual SGLError SetSubMeshDensity
			( size_t iMesh, float32_t fpDensity, ssize_t iSelector = -1 ) ;
		// 追加的な頂点属性を設定
		virtual SGLError SetExtendVertexAttribute
			( size_t iMesh, size_t countElements,
				size_t countVertex, const float32_t * pfpAttrElements ) ;
		// メッシュにウェイトマップを設定
		virtual SGLError SetBoneWeightMap
			( size_t iMesh, size_t nCount, const float32_t ** ppWeightMaps ) ;
		// メッシュにジョイントマップを設定
		virtual SGLError SetBoneJointMap
			( size_t iMesh, size_t nBoneCount,
				size_t nJointCount, const uint32_t ** ppJointMaps ) ;
		// メッシュにボーン行列設定
		virtual SGLError SetBoneMatrix
			( size_t iMesh, size_t nCount,
				const S3DMatrix * pMatrix, const S3DVector * pTrans ) ;
		// メッシュのボーン行列取得
		virtual size_t GetBoneMatrix
			( size_t iMesh, size_t nCount,
				S3DMatrix * pMatrix, S3DVector * pTrans ) ;
		// メッシュにモーフターゲット枠を確保
		virtual SGLError AllocateMorphing( size_t iMesh, size_t nCount ) ;
		// メッシュにモーフターゲットを設定
		virtual SGLError SetMorphingTargetMesh
			( size_t iMesh, size_t iMorph, size_t countVertex,
				const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap, const S3DColor * pColor ) ;
		// メッシュのモーフターゲットにウェイトを設定
		virtual SGLError SetMorphingTargetWeight
			( size_t iMesh, size_t iMorph,
				size_t countVertex, const float32_t * pfpWeight ) ;
		// モーフィング設定
		virtual SGLError SetMorphingApplication
			( size_t iMesh, const ssize_t * pTargetMesh,
					const float32_t * pApplication, size_t nTargetMeshCount ) ;
		// モーフィング設定取得
		virtual SGLError GetMorphingApplication
			( size_t iMesh, ssize_t& iTargetMesh,
					float32_t& fpApplication, size_t iTargetMeshIndex ) ;
		// メッシュ表示設定
		virtual SGLError EnableToRenderMesh
			( size_t iFirst = 0, ssize_t iEnd = -1, bool fEnable = true ) ;
		// メッシュ表示フラグ取得
		virtual bool IsEnabledToRenderMesh( size_t iMesh ) const ;
		// メッシュマテリアル設定
		virtual SGLError SetMaterialToRenderMesh
			( size_t iMesh, S3DMaterial * pMaterial ) ;
		// メッシュマテリアル取得
		virtual S3DMaterial * GetMaterialToRenderMesh( size_t iMesh ) const ;
		// バッファを S3DRenderBufferInterface へ出力
		virtual SGLError RenderBufferTo
			( S3DRenderBufferInterface * render,
					uint64_t flagsExclusion = 0,
					size_t iFirst = 0, ssize_t iEnd = -1,
					size_t nInstancing = 0,
					const S4DMatrix * pmatInstancing = NULL,
					const S3DColor * pColorInstancing = NULL ) const ;
		// バッファを消去
		virtual void ClearBuffer( void ) ;
		// デバイスリソースを解放
		virtual void ReleaseAllDeviceResources( void ) ;
		// バッファのメモリブロックサイズ設定
		virtual void SetBufferUnitSize( size_t nBytes ) ;
		// メッシュ外接球取得
		virtual double GetCircumscribedSphere( S3DVector& vCenter ) ;
		// メッシュ外接直方体取得
		virtual bool GetCircumscribedParallelepiped
							( S3DVector& vMin, S3DVector& vMax ) ;
		// 現在の設定に適合する S3DVertexVariantBuffer を生成
		virtual S3DVertexVariantBuffer * CreateVariantBuffer( void ) ;
		// S3DVertexVariantBuffer のパラメータを VertexBuffer へ反映
		virtual SGLError UpdateVertexVariant
			( S3DVertexVariantBuffer * pVVB, size_t iFirst = 0, ssize_t iEnd = -1 ) ;
		// 参照バリアントを生成
		virtual S3DVertexBufferInterface * NewReferenceVariantBuffer( void ) ;
		// S3DVertexDeviceBufferInterface 取得
		virtual S3DVertexDeviceBufferInterface *
			GetDeviceBufferTypeOf( SGLImageBufferObjectType type ) ;
		virtual S3DVertexDeviceBufferInterface *
			GetDeviceBufferAs( const ESLRuntimeClass& rtClass ) ;
		// S3DVertexDeviceBufferInterface 追加
		virtual void AttachDeviceBuffer( S3DVertexDeviceBufferInterface * pDevBuf ) ;

	protected:
		// 自動結合プリミティブ追加処理
		SGLError MergePrimitiveBuffer
			( MergedPrimitiveBuffer& mpbuf, uint32_t nFlags,
				S3DPrimitiveType typePrimitive,
				size_t countIndex, size_t countVertex ) ;
		// ソート処理
		virtual void OnSortRenderBuffer
			( RENDER_ENTRY** ppRender, size_t nCount ) ;
		// 追加時処理 (デフォルトは頂点バッファ複製)
		virtual bool OnAddRenderBuffer
			( RENDER_ENTRY& entry,
				const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap, const S3DColor * pColor ) ;
		// バッファ更新処理
		virtual void UpdateRenderBuffer
			( RENDER_ENTRY& entry,
				const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap, const S3DColor * pColor ) ;
		// 頂点を複製するとともに最大値と最小値を取得
		static void MoveVertexAndCubeRange
			( S3DVector& vMax, S3DVector& vMin,
				S3DVector4 * pvDstVertex,
				const S3DVector4 * pvSrcVertex, size_t nCount ) ;
		// 現在の変換をエントリに設定
		void SetEntryTransformation( RENDER_ENTRY& entry ) ;
		// カスタムシェーダーパラメータリストを生成
		void AllocateCustumShaderUniformList( TransformationList * pTrans ) ;
		ShaderContextEntry * AllocateShaderContextEntry( void ) ;
		// 法線を自動生成
		void GenerateDefaultNormal( RENDER_ENTRY& entry ) ;
		// ソート用ハッシュ値計算
		virtual void MakeSoftHash( RENDER_ENTRY& entry, float32_t zFloatSort ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// バーテックス・バッファとして使う S3DRenderBuffer
	//////////////////////////////////////////////////////////////////////////

	class	S3DRenderVertexBuffer	: public S3DRenderBuffer
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DRenderVertexBuffer, S3DRenderBuffer )
		// 構築関数
		S3DRenderVertexBuffer( void ) ;
		S3DRenderVertexBuffer( const S3DRenderBuffer & buf ) ;

	public:	// S3DVertexBufferInterface
		// バッファを S3DRenderBufferInterface へ出力
		virtual SGLError RenderBufferTo
			( S3DRenderBufferInterface * render,
					uint64_t flagsExclusion = 0,
					size_t iFirst = 0, ssize_t iEnd = -1,
					size_t nInstancing = 0,
					const S4DMatrix * pmatInstancing = NULL,
					const S3DColor * pColorInstancing = NULL ) const ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// レンダリング・バリアント・バッファ
	//////////////////////////////////////////////////////////////////////////

	class	S3DRenderVariantBuffer	: public S3DVertexBuffer
	{
	public:
		S3DVertexVariantBuffer *	m_pvvbVar ;

	protected:
		bool						m_flagInstancing ;
		SSystem::SArray<S4DMatrix>	m_aInstanceMatrixs ;
		SSystem::SArray<S3DColor>	m_aInstanceColors ;
		SSystem::SPointerArray<S3DVertexVariantBuffer>
									m_aInstanceVVB ;
		SSystem::SCriticalSection	m_csInstanceSync ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DRenderVariantBuffer, S3DVertexBuffer )
		// 構築関数
		S3DRenderVariantBuffer( S3DRenderBuffer * prb ) ;
		// 消滅関数
		virtual ~S3DRenderVariantBuffer( void ) ;
		// バリアント取得
		S3DVertexVariantBuffer * GetVariantBuffer( void ) const
		{
			return	m_pvvbVar;
		}

	public:	// S3DVertexBufferInterface
		// バッファを S3DRenderBufferInterface へ出力
		virtual SGLError RenderBufferTo
			( S3DRenderBufferInterface * render,
					uint64_t flagsExclusion = 0,
					size_t iFirst = 0, ssize_t iEnd = -1,
					size_t nInstancing = 0,
					const S4DMatrix * pmatInstancing = NULL,
					const S3DColor * pColorInstancing = NULL ) const ;
		// VertexBuffer 取得
		// ※S3DRenderVariantBuffer自身を返す
		// 　参照先を取得するには GetVertexBuffer を呼び出す
		virtual VertexBuffer * GetVertexBufferObject( void ) const ;

	public:
		// マルチインスタンス描画モード設定
		virtual SGLError EnableMultiInstancingMode( bool flagEnable ) ;
		// マルチインスタンス描画モード設定
		virtual bool IsMultiInstancingMode( void ) const ;
		// インスタンス数取得
		virtual size_t GetInstancingCount( void ) const ;
		// インスタンス取得
		virtual size_t GetInstancingEntries
			( S3DVertexVariantBuffer** ppVVB,
				S4DMatrix * pmatInstance,
				S3DColor * pcolorInstance,
				size_t iFirst, size_t nCount ) const ;
		// インスタンス全消去
		virtual SGLError ClearAllInstance( void ) ;
		// インスタンス追加設定
		virtual SGLError AddInstanceVariant
			( S3DVertexVariantBuffer * pVVB,
				const S4DMatrix & matInstance,
				const S3DColor & colorInstance ) ;

	public:	// S3DVertexVariantBuffer
		// メッシュにボーン行列設定
		virtual SGLError SetBoneMatrix
			( size_t iMesh, size_t nCount,
				const S3DMatrix * pMatrix, const S3DVector * pTrans ) ;
		// メッシュのボーン行列取得
		virtual size_t GetBoneMatrix
			( size_t iMesh, size_t nCount,
				S3DMatrix * pMatrix, S3DVector * pTrans ) ;
		// モーフィング設定
		virtual SGLError SetMorphingApplication
			( size_t iMesh, const ssize_t * pTargetMesh,
					const float32_t * pApplication, size_t nTargetMeshCount ) ;
		// モーフィング設定取得
		virtual SGLError GetMorphingApplication
			( size_t iMesh, ssize_t& iTargetMesh,
					float32_t& fpApplication, size_t iTargetMeshIndex ) ;
		// メッシュ表示設定
		virtual SGLError EnableToRenderMesh
			( size_t iFirst = 0, ssize_t iEnd = -1, bool fEnable = true ) ;
		// メッシュ表示フラグ取得
		virtual bool IsEnabledToRenderMesh( size_t iMesh ) const ;
		// メッシュマテリアル設定
		virtual SGLError SetMaterialToRenderMesh
			( size_t iMesh, S3DMaterial * pMaterial ) ;
		// メッシュマテリアル取得
		virtual S3DMaterial * GetMaterialToRenderMesh( size_t iMesh ) const ;
	} ;


}


#endif
