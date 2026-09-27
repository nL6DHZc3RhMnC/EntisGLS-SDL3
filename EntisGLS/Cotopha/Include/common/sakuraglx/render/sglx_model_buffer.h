
#if	!defined(__SAKURAGLX_MODEL_BUFFER_H__)
#define	__SAKURAGLX_MODEL_BUFFER_H__	1

#include <sakura/ssys_bit_array.h>
#include <sakuragl/sgl3d/sgl_render_buffer.h>
#include <sakuraglx/render/sglx3d_scene.h>

namespace	SakuraGL
{
	class	S3DModelBuffer ;

	//////////////////////////////////////////////////////////////////////////
	// テクスチャ画像ライブラリ
	//////////////////////////////////////////////////////////////////////////

	class	S3DTextureLibrary	: public SGLResourceProducer,
									public S3DTextureLibraryReferencer
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DTextureLibrary, SGLResourceProducer )
		// 構築関数
		S3DTextureLibrary( void ) ;
		S3DTextureLibrary( const S3DTextureLibrary& lib ) ;
		// 消滅関数
		virtual ~S3DTextureLibrary( void ) ;
		// 代入（複製）
		const S3DTextureLibrary& operator = ( const S3DTextureLibrary& lib ) ;

	protected:
		SSystem::SStrSortArray<SSystem::SSyncReference>		m_ssaTextures ;
		SSystem::SSmartReference<S3DTextureLibrary>			m_refParent ;
		SSystem::SReferenceArray<S3DTextureLibrary>			m_aRefLib ;
		SSystem::SString									m_strName ;

	public:
		// 画像取得
		virtual SGLImageObject * GetTextureAs
			( const wchar_t * pwszID, bool flagNoRefOther = false ) const ;
		virtual SSystem::SObject * GetResourceAs( const wchar_t * pwszID ) ;
		// 画像登録
		SGLError AddTextureAs
				( const wchar_t * pwszID, SGLImageObject * pTexture ) ;
		SGLError AddSmartTextureAs
				( const wchar_t * pwszID, SGLImageObject * pTexture ) ;
		SGLError AddLibraryFrom( const S3DTextureLibrary & lib ) ;
		// 登録削除
		SGLError RemoveTextureAs( const wchar_t * pwszID ) ;
		// 全登録削除
		SGLError RemoveAllTexture( void ) ;
		// 名前変更
		SGLError RenameTextureAs
			( SGLImageObject * pTexture, const wchar_t * pwszID ) ;
		// 画像検索
		virtual ssize_t FindTexturePtr( SGLImageObject * pImage ) const ;
		// 登録数取得
		virtual size_t GetTextureCount( void ) const ;
		// 登録名取得
		virtual const wchar_t * GetTextureIdentityAt( size_t i ) const ;
		virtual const wchar_t * GetTextureIdentityOf( SGLImageObject * pImage ) const ;
		// 画像取得
		virtual SGLImageObject * GetTextureAt( size_t i ) const ;
		// ライブラリ名
		const SSystem::SString& GetLibraryName( void ) const ;
		void SetLibraryName( const wchar_t * pwszName ) ;
		// 参照ライブラリを設定する
		void SetParentLibrary( S3DTextureLibrary * pLib ) ;
		size_t AddReferenceLibrary( S3DTextureLibrary * pLib ) ;
		// 参照ライブラリを取得する
		S3DTextureLibrary * GetParentLibrary( void ) const ;
		size_t GetReferenceLibraryCount( void ) const ;
		S3DTextureLibrary * GetReferenceLibraryAt( size_t i ) const ;
		void DetachReferenceLibraryAt( size_t i ) ;
		void DetachReferenceLibraryOf( S3DTextureLibrary * pLib ) ;
		void DetachAllReferenceLibrarys( void ) ;
		// 画像名正規化（未使用であることを保証）
		const SSystem::SString& NormalizeIdentity( SSystem::SString& strID ) const ;
		// 解放
		void Release( void ) ;

	public:
		// 圧縮フラグ付きのテクスチャを事前に圧縮する
		SGLError MakeCompressedTexture
			( uint32_t format = formatImageRGB_S3TC_DXT1, uint64_t nFlags = 0 ) ;
		// デバイスメモリ上に準備する
		SGLError CommitToDevice
			( S3DRenderDevice * pDev, int64_t msecTimeout = 0 ) ;
		// デバイス上リソースを開放する
		SGLError ReleaseAllDeviceResources( void ) ;
		// バンプマッピング用画像から法線画像生成
		static SGLImageObject * MakeNormalMapFromBumpMap
			( SGLImageObject * pNormal,
				SGLImageObject * pTexture, float32_t fpHeight ) ;
		// 法線画像をグレイスケールへ戻す（※不可逆）
		static SGLImageObject * RestoreBumpTexture
			( SGLImageObject * pGrayscale, SGLImageObject * pNormal ) ;
		// キューブマップ画像からパノラマ画像を生成
		static SGLError MakePanoramaImageFromCubeMap
			( SGLImageObject& imgPanorama,
				int nPanoramaWidth, int nPanoramaHeight, SGLImageObject& imgCubeMap ) ;
		// パノラマ画像から環境マッピング（球マップ）画像を生成
		static SGLError MakeSphereMapFromPanoramaImage
			( SGLImageObject& imgSphereMap,
				int nTextureSize, double degHRotAngle,
				SGLImageObject& imgPanorama ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 表面属性ライブラリ
	//////////////////////////////////////////////////////////////////////////

	class	S3DMaterialLibrary	: public SSystem::SObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DMaterialLibrary, SObject )
		// 構築関数
		S3DMaterialLibrary( void ) ;
		S3DMaterialLibrary( const S3DMaterialLibrary& lib ) ;
		// 消滅関数
		virtual ~S3DMaterialLibrary( void ) ;
		// 代入（複製）
		const S3DMaterialLibrary& operator = ( const S3DMaterialLibrary& lib ) ;

	protected:
		SSystem::SStrSortArray<SSystem::SSyncReference>	m_ssaMaterials ;
		SSystem::SSmartReference<S3DTextureLibrary>		m_refLocalTexture ;
		SSystem::SSmartReference<S3DMaterialLibrary>	m_refParent ;
		SSystem::SReferenceArray<S3DMaterialLibrary>	m_aRefLib ;
		SSystem::SString								m_strName ;

	public:
		// ファイル読み込み
		SGLError LoadLibraryXML
			( const wchar_t * pwszFilePath,
				const S3DTextureLibraryReferencer& libTexture ) ;
		// XML デシリアライズ
		SGLError ParseXML
			( const SSystem::SXMLDocument & xmlMaterials,
					const S3DTextureLibraryReferencer& libTexture ) ;
		// ファイル書き出し
		SGLError SaveLibraryXML
			( const wchar_t * pwszFilePath,
				const S3DTextureLibraryReferencer& libTexture ) ;
		// XML シリアライズ
		SGLError FormatXML
			( SSystem::SXMLDocument & xmlMaterials,
					const S3DTextureLibraryReferencer& libTexture ) ;
		// ローカルテクスチャライブラリ設定
		void AttachLocalTextureLibrary( S3DTextureLibrary * pTxtLib ) ;
		// 全テクスチャ参照更新
		void UpdateAllTextureReference
			( const S3DTextureLibraryReferencer& libTexture ) ;
		// 解放
		void Release( void ) ;

	public:
		// 表面属性取得
		S3DMaterial * GetMaterialAs
			( const wchar_t * pwszID, bool flagNoRefOther = false ) const ;
		// 表面属性登録
		SGLError AddMaterialAs
				( const wchar_t * pwszID, S3DMaterial * pMaterial ) ;
		SGLError AddSmartMaterialAs
				( const wchar_t * pwszID, S3DMaterial * pMaterial ) ;
		SGLError AddLibraryFrom( const S3DMaterialLibrary & lib ) ;
		// 登録削除
		SGLError RemoveMaterialAs( const wchar_t * pwszID ) ;
		// 全登録削除
		SGLError RemoveAllMaterial( void ) ;
		// 名前変更
		SGLError RenameMaterialAs
			( S3DMaterial * pMaterial, const wchar_t * pwszID ) ;
		// 表面属性検索
		ssize_t FindMaterialPtr( S3DMaterial * pMaterial ) const ;
		// 登録数取得
		size_t GetMaterialCount( void ) const ;
		// 登録名取得
		const wchar_t * GetMaterialIdentityAt( size_t i ) const ;
		const wchar_t * GetMaterialIdentityOf( S3DMaterial * pMaterial ) const ;
		// 表面属性取得
		S3DMaterial * GetMaterialAt( size_t i ) const ;
		// 参照ライブラリを設定する
		void SetParentLibrary( S3DMaterialLibrary * pLib ) ;
		size_t AddReferenceLibrary( S3DMaterialLibrary * pLib ) ;
		// ライブラリ名
		const SSystem::SString& GetLibraryName( void ) const ;
		void SetLibraryName( const wchar_t * pwszName ) ;
		// 参照ライブラリを取得する
		S3DMaterialLibrary * GetParentLibrary( void ) const ;
		size_t GetReferenceLibraryCount( void ) const ;
		S3DMaterialLibrary * GetReferenceLibraryAt( size_t i ) const ;
		void DetachReferenceLibraryAt( size_t i ) ;
		void DetachReferenceLibraryOf( S3DMaterialLibrary * pLib ) ;
		void DetachAllReferenceLibrarys( void ) ;
		// 属性名正規化（未使用であることを保証）
		const SSystem::SString& NormalizeIdentity( SSystem::SString& strID ) const ;

	public:
		// テクスチャの用途を調査する（シェーディングフラグで取得する）
		uint64_t GetTextureUsedFlags( SGLImageObject * pTexture ) const ;
		// 指定画像をテクスチャとして参照しているか？
		bool IsTextureUsed( SGLImageObject * pTexture ) const ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ウェイトマップ（疎な配列用）
	//////////////////////////////////////////////////////////////////////////

	class	S3DWeightMapBuffer
	{
	public:
		enum	PageConstant
		{
			WPAGE_SIZE_SHIFTER	= 7,
			WPAGE_SIZE			= 1 << WPAGE_SIZE_SHIFTER,
			WPAGE_ODD_MASK		= WPAGE_SIZE - 1,
		} ;
		static inline size_t PageLength( size_t nLength )
		{
			return	(nLength + WPAGE_ODD_MASK) >> WPAGE_SIZE_SHIFTER ;
		}
		struct	Page
		{
			float32_t	buf[WPAGE_SIZE] ;

			Page( void ) ;
			Page( const Page& src ) ;
			void Clear( void ) ;
			void CopyFrom
				( size_t iDst, const Page& pageSrc,
							size_t iSrc, size_t nCount ) ;
			void Write
				( size_t iDst, const float32_t * pfpSrc, size_t nCount ) ;
			bool IsEmpty( void ) const ;
		} ;

	protected:
		SSystem::SObjectArray<Page>	m_pages ;
		size_t						m_iFirst ;
		size_t						m_nLength ;
		SSystem::SArray<float32_t>	m_bufLocked ;
		size_t						m_iLockFirst ;
		size_t						m_nLockLength ;

	public:
		// 構築関数
		S3DWeightMapBuffer( void ) ;
		S3DWeightMapBuffer( const S3DWeightMapBuffer& wmb ) ;
		// 消滅関数
		~S3DWeightMapBuffer( void ) ;
		// 範囲取得
		size_t GetFirst( void ) const
		{
			return	m_iFirst ;
		}
		size_t GetLength( void ) const
		{
			return	m_nLength ;
		}
		// クリア
		void ClearBuffer( void ) ;
		// 範囲拡張
		void ExpandBounds( size_t iFirst, size_t nCount ) ;
		// 一部書き換え
		size_t WriteWeight
			( size_t iDstFirst, const float32_t * pfpWeight, size_t nCount ) ;
		// ボーン影響範囲切り捨て
		void TrimBounds( void ) ;
		void ChopBounds( size_t nLeftCount, size_t nRightCount ) ;
	protected:
		// 末端ページの終端を０パディング
		void ZeroPadForEndOfPage( void ) ;
	public:
		// 有効領域取得
		bool NormalizeBounds( size_t& iFirst, size_t& nCount ) const ;
		// バッファ参照
		float32_t GetAt( size_t i ) const ;
		void SetAt( size_t i, float32_t w ) ;
		void ReadWeight
			( float32_t * pfpWeight, size_t iFirst, size_t nCount ) const ;
		float32_t * LockBuffer( size_t& iFirst, size_t& nCount ) ;
		void UnlockBuffer( bool flagWrite ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ボーン
	//////////////////////////////////////////////////////////////////////////

	class	S3DModelBoneSpace	: public S3DScene::Space
	{
	public:
		// ボーンフラグ
		enum	BoneFlag
		{
			flagBonePhysics		= 0x00000001,		// 物理演算
			flagNoCollision		= 0x00000010,		// 当たり判定無し（物理演算時）
			flagTrackingPos		= 0x00000100,		// IK 操作時のトラッキング位置マーク
			flagHaveOrgMatrix	= 0x00000200,		// 元モデルの行列を保持
			flagFreezePhysics	= 0x00010000,		// 物理演算一時停止
		} ;
		// IK 用パラメータ
		enum	IKParameterFlag
		{
			flagIKMinBent			= 0x00000001,
			flagIKMaxBent			= 0x00000002,
			flagIKBendDirection		= 0x00000004,	// 曲げ方向を vBendDirection と同じ方向に制限
			flagIKParentAxis		= 0x00000008,	// 親を初めに vBendDirection を回転軸にして回転
			flagIKTerminate			= 0x00008000,	// IK 処理をこのボーンより親のボーンには影響しない
		} ;
		struct	IKParameter
		{
			uint32_t	nIKFlags ;
			float32_t	fpWeight ;
			float32_t	degMinBent  ;		// 最小曲がり角 [deg]
			float32_t	degMaxBent  ;		// 最大曲がり角 [deg]
			S3DVector	vBendDirection ;	// 曲げ可能方向 or 回転軸
		} ;
		// 疑似物理演算パラメータ（物性）
		enum	PhysExtensionFlags1
		{
			flagPhysExCollider0			= 0x00000001,	// 当たり判定除外クラス
			flagPhysExColliderMaxCount	= 16,
			flagPhysExColliderAll		= 0x0000FFFF,
			flagPhysExColliderUseMask	= 0x00010000,	// 当たり判定除外フラグ
		} ;
		struct	PhysMaterial
		{
			double		fpAttenuation ;		// 速度減衰率 [0,1] [/sec]
			double		fpShrinkable ;		// 縮み弾性 [/frame]
			double		fpElasticity ;		// 伸び弾性 [/frame]
			double		fpMinStretch ;		// 縮み率限界 [/frame]
			double		fpMaxStretch ;		// 伸び率限界 [/frame]
			double		fpHardness ;		// 形状弾性 [/frame]
			double		fpEffect ;			// 加速度加算比率 [0,1] [/frame]
			double		fpLimitedAngle ;	// 制限角 [deg]（180-θを曲がり限界とする）
			double		fpCollisionRadius ;	// 当たり判定半径
			double		fpFrictionalResistance ;	// 摩擦係数（加速率） [0,1] [/frame]
			uint32_t	nPhysExFlags1 ;		// 拡張フラグ１
			uint32_t	nPhysExFlags2 ;		// 拡張フラグ２（未使用）

			// 構築関数
			PhysMaterial
				( double a = 0.995, double s = 0.9, double e = 0.9,
					double mnst = 0.95, double mxst = 1.05, double h = 0.1,
					double ef = 0.5, double la = 120.0,
					double cr = 0.0, double fr = 0.001,
					uint32_t xf1 = 0, uint32_t xf2 = 0 )
				: fpAttenuation(a), fpShrinkable(s),
					fpElasticity(e),
					fpMinStretch(mnst), fpMaxStretch(mxst),
					fpHardness(h), fpEffect(ef), fpLimitedAngle(la),
					fpCollisionRadius(cr),
					fpFrictionalResistance(fr),
					nPhysExFlags1(xf1), nPhysExFlags2(xf2) { }
			PhysMaterial( const PhysMaterial& phmt )
				: fpAttenuation(phmt.fpAttenuation),
					fpShrinkable(phmt.fpShrinkable),
					fpElasticity(phmt.fpElasticity),
					fpMinStretch(phmt.fpMinStretch),
					fpMaxStretch(phmt.fpMaxStretch),
					fpHardness(phmt.fpHardness),
					fpEffect(phmt.fpEffect),
					fpLimitedAngle(phmt.fpLimitedAngle),
					fpCollisionRadius(phmt.fpCollisionRadius),
					fpFrictionalResistance(phmt.fpFrictionalResistance),
					nPhysExFlags1(phmt.nPhysExFlags1),
					nPhysExFlags2(phmt.nPhysExFlags2) { }
			// 代入
			const PhysMaterial& operator = ( const PhysMaterial& phmt )
			{
				fpAttenuation = phmt.fpAttenuation ;
				fpShrinkable = phmt.fpShrinkable ;
				fpElasticity = phmt.fpElasticity ;
				fpMinStretch = phmt.fpMinStretch ;
				fpMaxStretch = phmt.fpMaxStretch ;
				fpHardness = phmt.fpHardness ;
				fpEffect = phmt.fpEffect ;
				fpLimitedAngle = phmt.fpLimitedAngle ;
				fpCollisionRadius = phmt.fpCollisionRadius ;
				fpFrictionalResistance = phmt.fpFrictionalResistance ;
				nPhysExFlags1 = phmt.nPhysExFlags1 ;
				nPhysExFlags2 = phmt.nPhysExFlags2 ;
				return	*this ;
			}
			// シリアライズ
			void ParseXML( const SSystem::SXMLDocument& xmlMaterial ) ;
			void FormatXML( SSystem::SXMLDocument& xmlMaterial ) const ;

			// プリセット
			enum	PresetIndex
			{
				presetPlate,
				presetResin,
				presetHardRubber,
				presetSoftRubber,
				presetHeavtCloth,
				presetLightCloth,
				presetFilm,
				presetCount,
			} ;
			static const PhysMaterial	m_preset[presetCount] ;
		} ;
		// （親以外の）影響ボーン情報
		class	EffectiveBoneEntry
		{
		public:
			SSystem::SString	m_strBoneID ;
			double				m_fpWeight ;
		} ;
		// 疑似物理演算パラメータ（ボーン状態）
		struct	PhysVertex
		{
			S3DDVector	vPos ;			// 位置（ローカル）（※ハンドル）
			S3DDVector	vSpeed ;		// 速度（グローバル） [/sec]
			S3DDVector	vLastExSpeed ;	// 直前の外影響速度 [/sec]（グローバル）
			S3DDVector	vExSpeedLPF ;	// 外影響速度のローパスフィルタ
			S3DDMatrix	matLastSpace ;	// 直前の回転行列（グローバル）
			S3DDVector	vLastSpace ;	// 直前のベース座標（グローバル）
			size_t		nHitCollider ;	// 衝突カウンタ（チャタリング用バッファ）
			S3DDVector	vHitNormal ;	// 直前の衝突法線（グローバル）
			S3DDVector	vHitDeltaSpeed ;// 衝突での速度変化（グローバル） [/sec]
		} ;
		// 疑似物理演算パラメータ（外因）
		struct	PhysExogenous
		{
			S3DDMatrix	matSpace ;			// 現在の回転行列
			S3DDVector	vSpace ;			// 現在のベース座標
			S3DDMatrix	matLastSpace ;		// 直前の回転行列
			S3DDVector	vLastSpace ;		// 直前のベース座標
			S3DDVector	vAcceleration ;		// 加速度 [/sec^2]
			S3DDVector	vStream ;			// 空間流速 [/sec]
		} ;
		// 当たり判定用パラメータ
		struct	ColliderParam
		{
			S3DCollider *	pCollider ;
			uint32_t		maskColUser ;
			S3DDMatrix		matCollider ;
			S3DDMatrix		matICollider ;
			S3DDVector		vCollider ;
		} ;
		// メッシュ関連情報
		struct	REF_MESH_INFO
		{
			size_t		iMesh ;		// M = matRelMesh * Bone * matIMesh
			S4DMatrix	matIMesh ;
			S4DMatrix	matRelMesh ;
		} ;

	protected:
		uint32_t					m_flagsBone ;		// complex of enum BoneFlag
		S4DMatrix					m_mat4Original ;	// 元のモデルデータのグローバル変換行列（座標空間も元のまま）
		S3DDVector					m_vBoneOffset ;		// ボーン平行移動（ボーン内空間）
		S3DDVector					m_vBoneHandle ;		// ボーンハンドル（相対座標）
		IKParameter					m_paramIK ;			// IK 用パラメータ
		PhysMaterial				m_physMaterial ;	// 物理属性
		double						m_fpPhysBelnd ;		// 物理演算結果ブレンド [0,1]
		double						m_fpLastPhysBelnd ;
		SSystem::SString			m_idPhysMaterial ;	// 物理属性名
		SSystem::SObjectArray<EffectiveBoneEntry>
									m_aEffectiveBones ;	// 影響ボーン
		PhysVertex					m_physCurrent ;		// 物理状態
		#if	defined(__DEBUG__)
		bool						m_flagDebug ;
		#endif

		S3DModelBuffer *			m_pModel ;
		S3DWeightMapBuffer			m_wmbWeight ;

		SSystem::SArray<REF_MESH_INFO>	m_arrRefMesh ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DModelBoneSpace, Space )
		// 構築関数
		S3DModelBoneSpace( void ) ;
		S3DModelBoneSpace( const S3DModelBoneSpace& bone ) ;
		// 消滅関数
		virtual ~S3DModelBoneSpace( void ) ;
		// デバッグフラグ
		void SetDebugFlag( bool flagDebug ) ;

	public:	// 変換行列
		// ボーンの変換行列計算
		void CalcBoneTransformation
			( S3DDMatrix& matrix, S3DDVector& translate ) const ;
		const S3DDVector&
			CalcBoneBasePosition( S3DDVector& pos ) const ;
		void CalcSubBoneTransformation
			( S3DDMatrix& matSub, S3DDVector& vSub,
				const S3DDMatrix& matCur, S3DDVector& vCur,
				S3DModelBoneSpace * pSubBone ) const ;
		// ボーンの基準座標（回転とオフセットを含まない初期状態のグローバル座標）
		S3DDVector CalcBoneNormalizedPosition( void ) const ;
		// グローバル空間変換行列を計算
		virtual void CalcGlobalTransformation
			( S3DDMatrix& matrix, S3DDVector& pos ) const ;
		virtual const S3DDVector&
				CalcGlobalPosition( S3DDVector& pos ) const ;
		// 親ボーン
		S3DModelBoneSpace * GetParentBone( void ) const ;
		// 親ボーン判定
		bool IsParentBoneOf( S3DModelBoneSpace * pChild ) const ;
		// このボーンと全ての子ボーンの変更フラグをクリアする
		void ClearAllModifiedFlags( void ) ;

	public:	// ボーン属性
		// ボーンフラグ
		uint32_t GetBoneFlags( void ) const ;
		void SetBoneFlags( uint32_t nFlags ) ;
		// 元モデルの行列（座標空間変換なしのグローバル変換行列）
		const S4DMatrix& GetOriginalBoneMatrix( void ) const ;
		void SetOriginalBoneMatrix( const S4DMatrix& matOrg ) ;
		// 元モデルと同じ座標空間のグローバル変換行列を反映させる
		// （親ボーンから順に設定する＆flagHaveOrgMatrix 必須）
		void ApplyModifiedOriginalBoneMatrix
			( const S4DMatrix& matGlobal, const S4DMatrix& matCvtSpace ) ;
		// グローバル変換行列をローカルに変換して設定する
		void ApplyGlobalTransformation( const S4DMatrix& matGlobal ) ;
		void ApplyGlobalTransformation
			( const S3DDMatrix& matGlobal, const S3DDVector& vGlobalPos ) ;
		// ボーン内平行移動（ボーン内空間）
		const S3DDVector& GetBoneOffset( void ) const ;
		void SetBoneOffset( const S3DDVector& vOffset ) ;
		// ボーンハンドル（相対座標）
		const S3DDVector& GetBoneHandle( void ) const ;
		void SetBoneHandle( const S3DDVector& vHandle ) ;
		// IK 用パラメータ
		const IKParameter& GetIKParameter( void ) const ;
		void SetIKParameter( const IKParameter& param ) ;
		// 物性（物理演算）
		const PhysMaterial& GetBonePhysicalMaterial( void ) const ;
		void SetBonePhysicalMaterial( const PhysMaterial& phyMaterial ) ;
		// 物理属性パレット名
		const SSystem::SString& GetBonePhysicalMaterialID( void ) const ;
		void SetBonePhysicalMaterialID( const wchar_t * pwszID ) ;
		// このボーンの物性（物理演算）を子ボーンにも設定する
		void CopyPhysicalMaterialToAllChildren( bool fWithMaterialID = true ) ;
		// 物理演算・被影響ボーン
		size_t GetEffectivePhysBoneCount( void ) const ;
		EffectiveBoneEntry * GettEffectivePhysBoneAt( size_t i ) const ;
		size_t AddEffectivePhysBone( EffectiveBoneEntry * pebe ) ;
		void InsertEffectivePhysBoneAt( size_t i, EffectiveBoneEntry * pebe ) ;
		ssize_t FindEffectivePhysBone( const wchar_t * pwszBoneID ) const ;
		ssize_t FindEffectivePhysBoneOf( EffectiveBoneEntry * pebe ) const ;
		void RemoveEffectivePhysBoneAt( size_t i ) ;
		void RemoveAllEffectivePhysBones( void ) ;

	public:	// 物理演算
		// 物理演算の内部パラメータをリセット
		void ResetPhysicsParameter( void ) ;
		void ResetPhysicsParameter( const S3DDMatrix& matBone, const S3DDVector& vBone ) ;
		// 物理演算の内部パラメータ
		const PhysVertex& GetPhysicsParamater( void ) const ;
		void SetPhysicsParamater( const PhysVertex& physVertex ) ;
		// 物理演算の適用度
		double GetPhysicsBlendWeight( void ) const ;
		double GetLastPhysicsBlendWeight( void ) const ;
		void SetPhysicsBlendWeight( double fpBlend ) ;
		// 物理演算のハンドル座標
		const S3DDVector& GetPhysicsHandlePosition( void ) const ;
		void SetPhysicsHandlePosition( const S3DDVector& vPos ) ;
		// 物理演算適用度をリセット（フレームポーズ適用前に一度実行）
		void ResetPhysicsBlendWeight( void ) ;
		// 物理演算
		void CalculatePhysics
			( const PhysExogenous& exog,
				double secPast, const ColliderParam * pColParam = nullptr ) ;
	protected:
		void CalculateSubPhysics
			( const PhysExogenous& exog,
				const PhysMaterial& mtrl,
				const S3DDMatrix& matBone,
				const S3DDVector& vBone,
				double secPast, const ColliderParam * pColParam ) ;
		void ResetLastSpaceOfChildrenPhysics
			( const S3DDMatrix& matBone, const S3DDVector& vBone ) ;
	public:
		// 物理演算（PhysVertex インスタンス）
		static bool CalculatePhysicsVertex
			( PhysVertex& physVertex,			// このボーンのインスタンス
				PhysVertex * pParentPhys,		// 親ボーンのインスタンス
				const PhysExogenous& exog,
				const PhysMaterial& mtrl,
				const S3DDMatrix& matBone,		// 親ボーンまでのグローバル変換
				const S3DDVector& vBone,
				const S3DDMatrix& matLocal,		// このボーンのローカル変換
				const S3DDVector& vLocalMove, 
				uint32_t nBoneFlags,			// このボーンのフラグ
				const S3DDVector& vBoneHandle,	// このボーンのハンドル
				double secPast,					// 経過時間
				const ColliderParam * pColParam,// 当たり判定
				const S3DModelBoneSpace * pOtherParams = nullptr ) ;
	protected:
		struct	HitColiderParam
		{
			size_t		nHitCount ;
			S3DVector	vSumHitPos ;
			S3DVector	vSumHitNormal ;
			float		fpColRadius ;
		} ;
		static S3DCollision::HitColliderCallback
			Callback_OnHitCollider
				( const S3DCollisionResult& rsHit,
					const S3DVector& vHitPos, const S3DVector& vHitNormal,
					const S3DCollision::MeshCollision * pMesh, size_t iPolygon ) ;

	public:
		// 物理演算の状態をボーンに反映
		void ReflectPhysics( const S3DDMatrix& matParentBone ) ;
		void ReflectBonePhysics( const S3DDMatrix& matParentBone ) ;
		static void BoneTransformationFromPhysics
			( S3DDMatrix& matBoneLocal,
				const S3DDVector& vBoneHandle,
				PhysVertex& physVertex,
				double fpPhysBlend = 1.0 ) ;

	public:
		// 特定のボーン配列のインデックスを自分自身と子ボーンを親から順番列挙する
		void AddBoneOrderIndex
			( SSystem::SArray<size_t>& aBoneIndex,
				const S3DModelBoneSpace *const * ppBoneSet, size_t nBoneSetCount ) const ;

	public:
		// ボーン操作（IK）
		void OperateInverseKinematics
			( const S3DDVector& vPos,
				const S3DDMatrix& matRotate,
				const S3DDVector& vLocalTip,
				double fpBendingWeight,
				double zGimbalWeight,
				double fpTipBoneWeight,
				size_t nEffectParents = 0xFFFFFFFF ) ;
		bool OperateInverseKinematicsAtSubJoint
			( const S3DDVector& vPos,
				const S3DDMatrix& matRotate,
				const S3DDVector& vJointTip,
				S3DModelBoneSpace * pJoint,
				double fpBendingWeight, double zGimbalWeight ) ;
		bool OperateBoneRotation
			( S3DModelBoneSpace * pHandleBone,
				const S3DDVector& vGlobalPos,
				const S3DDMatrix& matRotate,
				double cosLimitBending, double zGimbalWeight ) ;
		bool OperateBoneRotation
			( const S3DDVector& vGlobalHandlePos,
				const S3DDVector& vGlobalPos,
				const S3DDMatrix& matRotate,
				double cosLimitBending, double zGimbalWeight ) ;
		static S3DDMatrix CalcGimbalRotationElement
			( const S3DDMatrix& matRotate,
				const S3DDMatrix& matBone,
				const S3DDVector& vAxis, double zGimbalWeight ) ;
		void SetGlobalMatrix( const S3DDMatrix& matRotate ) ;
		void SlerpGlobalMatrix( const S3DDMatrix& matRotate, double w = 1.0 ) ;
		bool SetGlobalMatrixLimitedAngle
				( const S3DDMatrix& matRotate, double cosLimit ) ;
		// ボーンを回転し、子ボーンの姿勢は維持する
		void RotateBoneAndKeepChildrenPosture
			( const S3DDMatrix& matRevDelta, S3DDVector * pvLocalPos ) ;

	public:	// 範囲
		// 座標とハンドルの範囲取得
		void GetExternalRectangular
			( S3DDVector& vMin, S3DDVector& vMax,
				const S3DDMatrix& matParent, const S3DDVector& vParent ) const ;

	public:	// ボーン設定・構築
		// モデル関連付け
		void AttachModel( S3DModelBuffer * pModel ) ;
		// ボーン設定（メッシュ境界補正あり）
		void SetBoneWeight
			( S3DModelBuffer * pModel,
				size_t iFirstVertex, size_t iFirstNormal,
				size_t nCount, const float32_t * pfpWeight = NULL ) ;
		// ボーン影響範囲変更
		void ExpandBoneWeightBounds
			( size_t iFirstVertex, size_t nCount ) ;
		// ボーンウェイトマップ部分書き換え
		size_t ModifyBoneWeightMapBounds
			( size_t iFirstVertex, const float32_t * pfpWeight, size_t nCount ) ;
		// ボーン影響範囲減少（前後０要素削除）
		void TrimBoneWeightBounds( void ) ;
		// ボーン影響範囲減少
		void ChopBoneWeightBounds( size_t nLeftCount, size_t nRightCount ) ;
		// ボーン影響範囲を有意な範囲になるように正規化
		void NormalizeBoneWeightBounds( void ) ;
		// ボーン重みマップを更新（指標はグローバル）
		void UpdateBoneWeight
			( size_t iFirstVertex, size_t nCount, const float32_t * pfpWeight ) ;
		// ボーン影響頂点取得
		size_t VertexIndexOfBoneWeight( void ) const ;
		size_t NormalIndexOfBoneWeight( void ) const ;
		size_t VertexCountOfBoneWeight( void ) const ;
		// ボーンウェイトマップ取得
//		const float32_t * GetBoneWeightMap( void ) const ;
//		const float32_t * GetBoneWeightMapBoundsAt( size_t& iVertex, size_t& nCount ) const ;
		float32_t GetBoneWeightAt( size_t iVertex ) const ;
		void SetBoneWeightAt( size_t iVertex, float32_t fpWeight ) ;
		void ReadBoneWeightMap
			( float32_t * pfpWeight, size_t iVertex, size_t nCount ) const ;
		float32_t * LockBoneWeightMap( size_t& iVertex, size_t& nCount ) ;
		void UnlockBoneWeightMap( bool flagWrite ) ;
		// ボーンが影響するメッシュ番号を追加
		void AddEffectiveMeshIndex
			( size_t iMesh, const S4DMatrix& matIMesh, const S4DMatrix& matRelMesh ) ;
		// ボーン影響範囲テスト
		bool IsBoneEffectVertexMap
			( size_t iVertex, const SSystem::SBitArray& maskVertex ) const ;
		// 影響ボーン数カウント
		size_t GetBoneCountEffectedVertexMap
			( size_t iVertex, const SSystem::SBitArray& maskVertex ) const ;
		// ボーン影響範囲取得
		bool GetBoneEffectVertexMap
			( size_t iVertex, SSystem::SBitArray& maskVertex ) const ;
		// メッシュ統合に付随して必要なら影響メッシュの追加
		void MergeEffectiveMeshIndex( size_t iMeshDst, size_t iMeshSrc ) ;
		// ボーンが影響するメッシュ番号配列取得
		const REF_MESH_INFO * GetEffectiveMeshIndexArray( size_t& nCount ) const ;

		friend class S3DModelBuffer ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// モデルデータ要素
	//////////////////////////////////////////////////////////////////////////

	namespace	S3DModelData
	{
		struct	BONE_LINK_INFO
		{
			S3DModelBoneSpace *	pRelBone ;
			S4DMatrix			matIMesh ;
			S4DMatrix			matRelMesh ;
		} ;

		// モデルデータ・メッシュオブジェクト
		class	MeshObject
		{
		public:
			S3DMaterial *				m_pMaterial ;
			S3DPrimitiveType			m_typeMesh ;
			size_t						m_countPolygon ;
			size_t						m_countSubPoly[VertexBuffer::countSubMesh] ;
			ssize_t						m_iSubMeshSelector ;
			float32_t					m_fpSubMeshDensity ;
			size_t						m_countVertex ;
			size_t						m_nExAttrElements ;
			ssize_t						m_iVertex ;
			ssize_t						m_iNormal ;
			SSystem::SArray<S2DVector>	m_bufUVMap ;
			SSystem::SArray<S3DColor>	m_bufColor ;
			SSystem::SArray<uint32_t>	m_bufIndex ;
			SSystem::SArray<uint32_t>	m_bufSubIndex[VertexBuffer::countSubMesh] ;
			SSystem::SArray<float32_t>	m_bufExAttrElements ;
			SSystem::SArray<BONE_LINK_INFO>
										m_arrRelBone ;		// ※S3DModelBuffer::BuildupBoneRelation 関数でセットアップされる
			SSystem::SObjectArray<SSystem::SString>
										m_arrMorphTarget ;
			SSystem::SArray<ssize_t>	m_arrMorphTargetMesh ;
			SSystem::SArray<float32_t>	m_arrMorphApplication ;
			size_t						m_nTargetMeshCount ;

		public:
			ESL_DECLARE_CLASS_OPERATOR_NEW_NV( ESLObject )
			// 構築関数
			MeshObject( void ) ;
			MeshObject( const MeshObject& mesh ) ;
			// 消滅関数
			~MeshObject( void ) ;
			// 影響ボーン検索
			ssize_t FindRelativeBone( S3DModelBoneSpace * pBone ) const ;
			// モーフィングターゲット指標検索
			ssize_t FindMorphTarget( const wchar_t * pwszTargetID ) const ;
		} ;

		// メッシュ・グループ
		struct	MeshGroup
		{
			uint32_t	m_iFirstMesh ;
			uint32_t	m_nMeshCount ;
			S3DDVector	m_vCenter ;
		} ;

		// 分割メッシュ情報
		class	MeshDivision
		{
		public:
			typedef	SSystem::SIndexedArray<SSystem::SString,const wchar_t*>	MorphEntryList ;
			class	SplittedEntry
			{
			public:
				SSystem::SString						m_strSplittedMesh ;
				SSystem::SObjectArray<SSystem::SString>	m_aSplittedMorph ;
			} ;
			MorphEntryList							m_aMorphEntries ;
			SSystem::SObjectArray<SplittedEntry>	m_aSplittedEntries ;

		public:
			// 分割先のモーフィングターゲットを取得
			const wchar_t * MapMorphTargetAs
				( const SplittedEntry * pSplitted,
					const wchar_t * pwszMorphTarget ) const ;
		} ;

		// マーカー情報
		class	MarkerInfo	: public SSystem::SObject
		{
		public:
			enum	Type
			{
				typeInvalid	= -1,
				typePosition,		// 位置と方向
				typeBoneColider,	// ボーン物理演算排他オブジェクト
				typeCollision,		// 被当たり判定ポイント（形状指定）
				typeCollider,		// 当たり判定ポイント（形状指定）
				typeEffector,		// 当たり判定作用ポイント
				typeCount,
			} ;
			enum	Shape
			{
				shapeSphere,
				shapeCube,
				shapeTube,
				shapeCount,
			} ;
			Type					m_type ;
			Shape					m_shape ;
			int						m_iCollider ;
			S3DQuaternion			m_qRotation ;
			S3DVector				m_vPosition ;
			S3DVector				m_vDirection ;
			S3DVector				m_vSize ;
			float32_t				m_fpRadius ;
			float32_t				m_fpLength ;
			SSystem::SString		m_strRefBone ;
			SSystem::SXMLDocument	m_xmlMarker ;

			static const wchar_t *	m_pwszTypeTags[typeCount] ;
			static const wchar_t *	m_pwszShapeIDs[shapeCount] ;

		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( MarkerInfo, SObject )
			// 構築関数
			MarkerInfo( void ) ;
			MarkerInfo( const MarkerInfo& mi ) ;
			// 消滅関数
			~MarkerInfo( void ) ;
			// 解釈
			SGLError ParseMarker( const SSystem::SXMLDocument& xmlMarker ) ;
			// 書式確定
			void CommitInfo( void ) ;
		} ;
	}


	//////////////////////////////////////////////////////////////////////////
	// ポーズ
	//////////////////////////////////////////////////////////////////////////

	class	S3DModelPose	: public SSystem::SObject
	{
	public:
		// メタ情報
		struct	MetaInfo
		{
			uint32_t	nFlags ;		// must be zero
			uint32_t	msecDuration ;	// animation total duration
			uint32_t	fxFrameRatio ;	// frame per second, fixed 16th-bit
		} ;

		// アニメーションデータヘッダ
		struct	AnimationHeader
		{
			uint32_t	nFlags ;		// must be zero
			uint32_t	nFrameCount ;
		} ;

		// フラグ
		enum	EntryFlag
		{
			flagDisabled		= 0x0001,
			flagDataSize		= 0x8000,
		} ;
		enum	JointExFlag
		{
			flagHaveOrgMatrix	= 0x0001,
		} ;

		// 文字列ID配列
		class	StringArray
					: public SSystem::SObjectArray<SSystem::SString>
		{
		public:
			ESL_DECLARE_CLASS_OPERATOR_NEW_NV( ESLObject )
			// 構築関数
			StringArray( void ) { }
			StringArray( const StringArray & src )
			{
				DuplicateArray( src ) ;
			}
			// 文字列ID取得
			int32_t GetStringIndex( const wchar_t * pwszStr ) const ;
			// 文字列ID割り当て
			uint32_t AllocateString( const wchar_t * pwszStr ) ;
		} ;

		// 間接情報
		struct	JointInfo
		{
			S3DDQuaternion	m_qRotation ;	// 回転情報
			S3DDVector		m_vZoom ;		// 拡大
			S3DDVector		m_vOffset ;		// 平行移動
			S3DDVector		m_vHandle ;		// 編集 UI 情報（ボーンハンドル）
			double			m_zRotation ;	// 編集 UI 情報（ハンドル軸回転角[deg]）
			double			m_wPhysBlend ;	// 物理演算合成用
			uint32_t		m_nExFlags ;	// 拡張データフラグ enum JointExFlag
			S4DMatrix		m_mat4OrgLocal ;// ポーズを生成した元ボーンのローカル行列

			ESL_DECLARE_CLASS_OPERATOR_NEW_NV( ESLObject )
			// 構築関数
			JointInfo( void )
				: m_qRotation( 1, 0, 0, 0 ),
					m_vZoom( 1, 1, 1 ),
					m_vOffset( 0, 0, 0 ),
					m_vHandle( 0, 0, 0 ),
					m_zRotation( 0 ), m_wPhysBlend( 1.0 ),
					m_nExFlags( 0 ), m_mat4OrgLocal( 1, 1, 1, 1 ) { }
			JointInfo( const JointInfo& ji )
				: m_qRotation( ji.m_qRotation ),
					m_vZoom( ji.m_vZoom ),
					m_vOffset( ji.m_vOffset ),
					m_vHandle( ji.m_vHandle ),
					m_zRotation( ji.m_zRotation ),
					m_wPhysBlend( ji.m_wPhysBlend ),
					m_nExFlags( ji.m_nExFlags ),
					m_mat4OrgLocal( ji.m_mat4OrgLocal ) { }
			// 代入
			const JointInfo& operator = ( const JointInfo& ji )
			{
				m_qRotation = ji.m_qRotation ;
				m_vZoom = ji.m_vZoom ;
				m_vOffset = ji.m_vOffset ;
				m_vHandle = ji.m_vHandle ;
				m_zRotation = ji.m_zRotation ;
				m_wPhysBlend = ji.m_wPhysBlend ;
				m_nExFlags = ji.m_nExFlags ;
				m_mat4OrgLocal = ji.m_mat4OrgLocal ;
				return	*this ;
			}
			// m_vHandle, m_zRotation から m_qRotation 計算
			void CalculateRotation( const S3DDVector& vOrgBoneHandle ) ;
		} ;
		struct	JointDataHeader
		{
			uint32_t	iStringID ;		// 対象ボーン名
			uint32_t	nFlags ;		// complex enum EntryFlag
			S3DDMatrix	matRotation ;
		} ;
		struct	JointData1		// 互換用データ構造
		{
			S3DDQuaternion	m_qRotation ;
			S3DDVector		m_vZoom ;
			S3DDVector		m_vOffset ;
			S3DDVector		m_vHandle ;
			double			m_zRotation ;
		} ;
		struct	MatrixElement
		{
			bool			fOrthogonal ;	// 直行行列か？
			S3DDMatrix		matTransform ;	// 直行行列でない場合こちらを使う
			S3DDQuaternion	qRotation ;		// 直行行列の場合には回転を補完できる
			S3DDVector		vZoom ;

			// 構築関数
			MatrixElement( void ) ;
			// 補完処理のために要素分解
			void FromMatrix( const S3DDMatrix& mat ) ;
			// 行列へ変換
			void ToMatrix( S3DDMatrix& mat ) const ;
			// 補完
			void Slerp( const MatrixElement& meSrc,
						const MatrixElement& meDst, double t ) ;
			// 回転行列取得（拡大成分以外）
			void GetRotation( S3DDMatrix& matRot ) const ;
		} ;
		class	JointAnimation	: public JointInfo
		{
		public:
			bool							m_fDisabled ;
			SSystem::SArray<S4DMatrix>		m_aMatrixs ;
			SSystem::SArray<S3DDQuaternion>	m_aRotations ;
			SSystem::SArray<S3DDVector>		m_aOffsets ;
			SSystem::SArray<S3DDVector>		m_aZooms ;
			SSystem::SArray<float32_t>		m_aPhysBlends ;
		public:
			// 構築関数
			JointAnimation( void ) : m_fDisabled( false ) { }
			JointAnimation( const JointAnimation& ja )
				: JointInfo( ja ),
					m_fDisabled( ja.m_fDisabled ),
					m_aMatrixs( ja.m_aMatrixs ),
					m_aRotations( ja.m_aRotations ),
					m_aOffsets( ja.m_aOffsets ),
					m_aZooms( ja.m_aZooms ),
					m_aPhysBlends( ja.m_aPhysBlends ) { }
			// モーションデータが静止データの場合削除する
			void SmartMotion( void ) ;
		} ;

		// モーフィング情報
		struct	MorphData
		{
			uint32_t	iStringID ;		// 対象ボーン名
			uint32_t	iTargetID ;		// 対象メッシュ名
			uint32_t	nFlags ;		// complex enum EntryFlag
			uint32_t	nReserved ;
		} ;
		class	MorphInfo
		{
		public:
			bool				m_fDisabled ;
			SSystem::SString	m_strMorphTarget ;
			StringArray			m_aTargetIDs ;
			SSystem::SArray<float32_t>
								m_aAnimation ;	// 各フレーム毎に m_aTargetIDs 順の適用度配列
		public:
			ESL_DECLARE_CLASS_OPERATOR_NEW_NV( ESLObject )
			MorphInfo( void ) : m_fDisabled(false) {}
			MorphInfo( const MorphInfo& mi )
				: m_fDisabled( mi.m_fDisabled ),
					m_strMorphTarget( mi.m_strMorphTarget ),
					m_aTargetIDs( mi.m_aTargetIDs ),
					m_aAnimation( mi.m_aAnimation ) { }
		} ;
		class	MorphContext
		{
		public:
			size_t						nTargetCount ;
			ssize_t						iTarget ;
			SSystem::SArray<ssize_t>	aTarget ;
			SSystem::SArray<float32_t>	aWeight ;

			ESL_DECLARE_CLASS_OPERATOR_NEW_NV( ESLObject )
		} ;

		// メッシュセレクタ
		struct	MeshSelectorData
		{
			uint32_t	iStringID ;
			uint32_t	nVisible ;
			uint32_t	nSeqFrames ;
			uint32_t	nReserved ;
		} ;
		class	MeshSelector
		{
		public:
			bool						m_fDisabled ;
			bool						m_flagVisible ;
			SSystem::SArray<uint8_t>	m_aVisibles ;
		public:
			ESL_DECLARE_CLASS_OPERATOR_NEW_NV( ESLObject )
			MeshSelector( void )
				: m_fDisabled(false), m_flagVisible(false) {}
			MeshSelector( const MeshSelector& ms )
				: m_fDisabled(ms.m_fDisabled),
					m_flagVisible(ms.m_flagVisible),
					m_aVisibles(ms.m_aVisibles) { }
			const MeshSelector& operator = ( const MeshSelector& ms )
			{
				m_fDisabled = ms.m_fDisabled ;
				m_flagVisible = ms.m_flagVisible ;
				m_aVisibles = ms.m_aVisibles ;
				return	*this ;
			}
		} ;

		// マテリアルセレクタ
		struct	MaterialSelectorData
		{
			uint32_t	nFlags ;
			uint32_t	iStringID ;
			uint32_t	nMaterialCount ;
			uint32_t	nSeqFrames ;
			uint32_t	nReserved ;
		} ;
		class	MaterialSelector
		{
		public:
			bool									m_fDisabled ;
			SSystem::SObjectArray<SSystem::SString>	m_aMaterialIDs ;
			SSystem::SPointerArray<S3DMaterial>		m_aMaterials ;
			SSystem::SArray<uint32_t>				m_aAnimation ;
		public:
			ESL_DECLARE_CLASS_OPERATOR_NEW_NV( ESLObject )
			MaterialSelector( void ) : m_fDisabled(false){ }
			MaterialSelector( const MaterialSelector& ms )
				: m_fDisabled(ms.m_fDisabled),
					m_aMaterialIDs(ms.m_aMaterialIDs),
					m_aMaterials(ms.m_aMaterials),
					m_aAnimation(ms.m_aAnimation) { }
		} ;

		// 分割メッシュ
		struct	MeshDivMap
		{
			size_t												iElement ;
			size_t												iMeshDiv ;
			const S3DModelData::MeshDivision *					pMeshDiv ;
			const S3DModelData::MeshDivision::SplittedEntry *	pSplitted ;
		} ;

	protected:
		MetaInfo										m_metaInfo ;
		SSystem::SStrSortObjectArray<JointAnimation>	m_ssoaJoint ;
		SSystem::SStrSortObjectArray<MorphInfo>			m_ssoaMorph ;
		SSystem::SStrSortObjectArray<MeshSelector>		m_ssoaMeshSel ;
		SSystem::SStrSortObjectArray<MaterialSelector>	m_ssoaMaterialSel ;

		class	MorphTarget	: public MorphContext
		{
		public:
			S3DModelData::MeshGroup *	pmg ;
			S3DModelData::MeshObject *	pMesh ;
		} ;
		S3DModelBuffer *							m_pLastPoseTarget ;
		SSystem::SPointerArray<S3DModelBoneSpace>	m_aBoneBuf ;
		SSystem::SObjectArray<MorphTarget>			m_aMorphBuf ;
		SSystem::SArray<MeshDivMap>					m_aMorphMeshDiv ;
		SSystem::SPointerArray
					<S3DModelData::MeshGroup>		m_aMeshSelBuf ;
		SSystem::SArray<MeshDivMap>					m_aMeshSelMeshDiv ;
		SSystem::SPointerArray
					<S3DModelData::MeshGroup>		m_aMaterialSelBuf ;
		SSystem::SArray<MeshDivMap>					m_aMaterialSelMeshDiv ;
		SSystem::SCriticalSection					m_csLock ;

		atomic_int_t	m_countRef ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DModelPose, SObject )
		// 構築関数
		S3DModelPose( void ) ;
		S3DModelPose( const S3DModelPose& pose ) ;
		// 消滅関数
		virtual ~S3DModelPose( void ) ;
		// 代入
		const S3DModelPose& operator = ( const S3DModelPose& pose ) ;
		// リソース削除
		void ClearAll( void ) ;
		// 禁止状態の要素を削除する
		void ClearDisabledElement( void ) ;
		// 参照カウンタ加算
		atomic_int_t AddRef( void ) ;
		// 参照カウンタ減少／解放
		atomic_int_t ReleaseRef( void ) ;
		// 参照カウンタ加算
		atomic_int_t LockRef( void ) ;
		// 参照カウンタ減少
		atomic_int_t UnlockRef( void ) ;
		// 参照カウンタ取得
		atomic_int_t GetReferenceCount( void ) const
		{
			return	m_countRef ;
		}
		// スレッド排他アクセス用
		void Lock( void ) ;
		void Unlock( void ) ;

	public:
		// ポーズファイル読み込み (バイナリ)
		virtual SGLError LoadPose( const wchar_t * pwszFilePath ) ;
		virtual SGLError ReadPoseFile( SSystem::SFileInterface & file ) ;
		virtual SGLError ReadPose( SSystem::SChunkFile & cf ) ;
		virtual SGLError ReadMetaInfoChunk( SSystem::SChunkFile & cf ) ;
		virtual SGLError ReadPoseChunk( SSystem::SChunkFile & cf ) ;
		virtual SGLError ReadMorphingChunk( SSystem::SChunkFile & cf ) ;
		virtual SGLError ReadMeshSelectorChunk( SSystem::SChunkFile & cf ) ;
		virtual SGLError ReadMaterialSelectorChunk( SSystem::SChunkFile & cf ) ;
		virtual SGLError ReadExtentionChunk( SSystem::SChunkFile & cf ) ;
		// ポーズファイル読み込み (XML)
		SGLError LoadPoseXML( const wchar_t * pwszFilePath ) ;
		SGLError ParsePoseXML( const SSystem::SXMLDocument & xmlPoseTag ) ;
		SGLError ParseMetaTag( const SSystem::SXMLDocument & xmlTag ) ;
		SGLError ParseJointTag( const SSystem::SXMLDocument & xmlTag ) ;
		SGLError ParseMorphingTag( const SSystem::SXMLDocument & xmlTag ) ;
		SGLError ParseMeshSelectorTag( const SSystem::SXMLDocument & xmlTag ) ;
		SGLError ParseMaterialSelectorTag( const SSystem::SXMLDocument & xmlTag ) ;
		// ポーズファイル書き出し (バイナリ)
		virtual SGLError SavePose( const wchar_t * pwszFilePath ) ;
		virtual SGLError WritePoseFile( SSystem::SFileInterface & file ) ;
		virtual SGLError WritePose( SSystem::SChunkFile & cf ) ;
		virtual SGLError WriteMetaInfoChunk( SSystem::SChunkFile & cf ) ;
		virtual SGLError WritePoseChunk( SSystem::SChunkFile & cf ) ;
		virtual SGLError WriteMorphingChunk( SSystem::SChunkFile & cf ) ;
		virtual SGLError WriteMeshSelectorChunk( SSystem::SChunkFile & cf ) ;
		virtual SGLError WriteMaterialSelectorChunk( SSystem::SChunkFile & cf ) ;
		// ポーズファイル書き出し (XML)
		SGLError SavePoseXML( const wchar_t * pwszFilePath ) ;
		SGLError FormatPoseXML( SSystem::SXMLDocument & xmlPoseTag ) ;

	public:
		// メタ情報
		const MetaInfo& GetMetaInfo( void ) const
		{
			return	m_metaInfo ;
		}
		void SetMetaInfo( const MetaInfo& inf )
		{
			m_metaInfo = inf ;
		}

	public:
		// ポーズに含まれる関節数取得
		size_t GetJointCount( void ) const ;
		// 関節のボーン識別子名取得
		const wchar_t * GetJointNameAt( size_t i ) const ;
		// 回転情報取得
		JointAnimation * GetJointAt( size_t i ) const ;
		JointAnimation * GetJointAs( const wchar_t * pwszBoneID ) const ;
		// 関節追加
		JointAnimation * AddJointAs
			( const wchar_t * pwszBoneID, JointAnimation * pjaJoint ) ;
		// 関節削除
		void RemoveJointAt( size_t i ) ;
		void RemoveJointAs( const wchar_t * pwszBoneID ) ;
		// 全関節削除
		void RemoveAllJoints( void ) ;
		// モーションデータが静止データの場合削除する
		void SmartAllJointMotion( void ) ;
		// 禁止状態のジョイントを削除する
		void ClearDisabledJoint( void ) ;

	public:
		// ポーズに含まれるモーフィングメッシュ数取得
		size_t GetMorphingCount( void ) const ;
		// 対象となるメッシュ（グループ）名取得
		const wchar_t * GetMorphingMeshNameAt( size_t i ) const ;
		// モーフィングターゲット取得
		MorphInfo * GetMorphingAt( size_t i ) const ;
		MorphInfo * GetMorphingAs( const wchar_t * pwszMeshID ) const ;
		// モーフィングターゲット追加
		MorphInfo * AddMorphingAs
			( const wchar_t * pwszMeshID, MorphInfo * pmi ) ;
		// モーフィング削除
		void RemoveMorphingAt( size_t i ) ;
		void RemoveMorphingAs( const wchar_t * pwszMeshID ) ;
		// 全モーフィング削除
		void RemoveAllMorphings( void ) ;
		// 禁止状態のモーフィングを削除する
		void ClearDisabledMorphing( void ) ;

	public:
		// ポーズに含まれるメッシュセレクタ数取得
		size_t GetMeshSelectorCount( void ) const ;
		// 対象となるメッシュ（グループ）名取得
		const wchar_t * GetSelectedMeshNameAt( size_t i ) const ;
		// メッシュセレクタ取得
		MeshSelector * GetMeshSelectorAt( size_t i ) const ;
		MeshSelector * GetMeshSelectorAs( const wchar_t * pwszMeshID ) const ;
		// メッシュセレクタ追加
		MeshSelector * AddMeshSelectorAs
			( const wchar_t * pwszMeshID, MeshSelector * pmsel ) ;
		// メッシュセレクタ削除
		void RemoveMeshSelectorAt( size_t i ) ;
		void RemoveMeshSelectorAs( const wchar_t * pwszMeshID ) ;
		// 全メッシュセレクタ削除
		void RemoveAllMeshSelectors( void ) ;
		// 禁止状態のメッシュセレクタを削除する
		void ClearDisabledMeshSelector( void ) ;

	public:
		// ポーズに含まれるマテリアルセレクタ数取得
		size_t GetMaterialSelectorCount( void ) const ;
		// 対象となるメッシュ（グループ）名取得
		const wchar_t * GetSelMaterialMeshNameAt( size_t i ) const ;
		// マテリアルセレクタ取得
		MaterialSelector * GetMaterialSelectorAt( size_t i ) const ;
		MaterialSelector * GetMaterialSelectorAs( const wchar_t * pwszMeshID ) const ;
		// マテリアルセレクタ追加
		MaterialSelector * AddMaterialSelectorAs
			( const wchar_t * pwszMeshID, MaterialSelector * pmsel ) ;
		// マテリアルセレクタ削除
		void RemoveMaterialSelectorAt( size_t i ) ;
		void RemoveMaterialSelectorAs( const wchar_t * pwszMeshID ) ;
		// 全マテリアルセレクタ削除
		void RemoveAllMaterialSelectors( void ) ;
		// 禁止状態のマテリアルセレクタを削除する
		void ClearDisabledMaterialSelector( void ) ;

	public:
		// ポーズターゲット取得
		void UpdatePoseTarget( S3DModelBuffer * pModel ) ;

	protected:
		// アニメーションフレーム補完情報
		struct	AnimationFrame
		{
			int		iFrame0 ;
			int		iFrame1 ;
			double	fpDelta ;
		} ;
		void CalculateAnimationFrame
				( AnimationFrame& af, double t, size_t nFrames ) const ;

	public:
		// ポーズターゲットリセット
		void ResetPoseTarget( void ) ;
		// モデルへポーズ設定
		void ApplyPoseTo
			( S3DModelBuffer& model,
				double w = 1.0 /*0.0～1.0*/, double t = 0.0 /*sec*/ ) ;
		void ProductPoseTo
			( S3DModelBuffer& model,
				double w = 1.0 /*0.0～1.0*/, double t = 0.0 /*sec*/ ) ;

	public:
		// ボーン合成
		void BlendBonePoseTo
			( S3DModelBuffer& model,
				double w = 1.0 /*0.0～1.0*/, double t = 0.0 /*sec*/ ) ;
		void ProductBonePoseTo
			( S3DModelBuffer& model,
				double w = 1.0 /*0.0～1.0*/, double t = 0.0 /*sec*/ ) ;
		// モーフィング合成
		void BlendMorphPoseTo
			( S3DModelBuffer& model,
				double w = 1.0 /*0.0～1.0*/,
				double t = 0.0 /*sec*/, bool flagBlendAdd = false ) ;
		// メッシュ表示状態合成
		void BlendMeshSelectorPoseTo
			( S3DModelBuffer& model, double t = 0.0 /*sec*/ ) ;
		// マテリアル選択合成
		void BlendMaterialSelectorPoseTo
			( S3DModelBuffer& model, double t = 0.0 /*sec*/ ) ;

	public:
		// Joint フレーム取得
		void CalcJointFrameMatrix
			( MatrixElement& meBone,
				S3DDVector& vBoneMove, double& wPhysBlend,
				const JointAnimation& ja, double t /* sec */ ) const ;
		// モーフィングターゲット指標取得
		void PrepareMorphTargetIndex
			( MorphContext& mc,
				const MorphInfo& mi,
				const S3DModelData::MeshObject * pMesh,
				const S3DModelData::MeshDivision * pMeshDiv = nullptr,
				const S3DModelData::MeshDivision::SplittedEntry * pSplitted = nullptr ) ;
		// モーフィングフレーム取得
		void CalcMorphFrameContext
			( MorphContext& mc, const MorphInfo& mi, double t /* sec */ ) const ;
		// モーフィング合成のために全ターゲット順配列に正規化
		static void NormalizeMorphContext
			( MorphContext& mcDst,
				const MorphContext& mcSrc, size_t nTotalTargetCount ) ;
		// モーフィングターゲットから使用されていないものを削除
		static void TrimMorphContext( MorphContext& mc ) ;
		// モーフィングコンテキスト合成
		static void BlendMorphContext
			( MorphContext& mcDst,
				const MorphContext& mcSrc, double w ) ;
		static void BlendAddMorphContext
			( MorphContext& mcDst,
				const MorphContext& mcSrc, double w ) ;
		// メッシュ表示状態
		bool GetMeshSelectorFrameVisible
			( const MeshSelector& ms, double t /* sec */ ) const ;

	public:
		// ポーズを合成のために存在しないボーンやメッシュターゲットを追加
		SGLError PrepareBlendPoseTarget( const S3DModelPose& pose ) ;
		// ポーズを合成のために存在しないボーンやメッシュターゲットを追加
		enum	PoseTargetFlag
		{
			poseTargetBone		= 0x0001,		// ボーン（物理演算ボーン除く）
			poseTargetMorph		= 0x0002,		// モーフィングメッシュ
			poseTargetMeshSel	= 0x0004,		// メッシュ表示状態
			poseTargetMaterial	= 0x0008,		// マテリアル
		} ;
		SGLError PrepareBlendPoseAllTarget
				( const S3DModelBuffer& model, uint32_t nFlags ) ;
		// アニメーションの全長フレーム数を設定
		void AllocateAnimationFrameCount( size_t nFrameCount ) ;
		// 静止ポーズ用にバッファを確保
		void AllocateStillPoseBuffer( void ) ;
		// アニメーションフレームを合成
		SGLError BlendAnimationFrame
			( double secDst,
				const S3DModelPose& pose,
				double secFrame, double wBlend = 1.0 ) ;
		// モーション区間を切り出し複製
		SGLError DuplicatePoseDuration
			( const S3DModelPose& pose,
				int iFirstFrame = 0, int iEndFrame = -1 ) ;

	public:
		// ポーズに含まれる参照先メッシュIDの変更
		void ChangeTargetMeshID
			( const wchar_t * pwszOldID, const wchar_t * pwszNewID ) ;
		// 参照先メッシュの統合
		void MergeTargetMeshID
			( const wchar_t * pwszMergeTarget,
				const wchar_t *const* ppwszMergeSources, size_t nSourceCount ) ; 

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ポーズライブラリ
	//////////////////////////////////////////////////////////////////////////

	class	S3DModelPoseLibrary	: public SSystem::SObject
	{
	protected:
		SSystem::SStrSortObjectArray<S3DModelPose>		m_ssoaPoses ;
		SSystem::SSmartReference<S3DModelPoseLibrary>	m_refParent ;
		SSystem::SReferenceArray<S3DModelPoseLibrary>	m_aRefLib ;
		SSystem::SString								m_strName ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DModelPoseLibrary, SObject )
		// 構築関数
		S3DModelPoseLibrary( void ) ;
		S3DModelPoseLibrary( const S3DModelPoseLibrary& lib ) ;
		// 消滅関数
		virtual ~S3DModelPoseLibrary( void ) ;
		// 代入
		const S3DModelPoseLibrary& operator = ( const S3DModelPoseLibrary& lib ) ;

	public:
		// ポーズファイル読み込み
		SGLError LoadLibrary( const wchar_t * pwszFilePath ) ;
		SGLError ReadLibrary( SSystem::SFileInterface & file ) ;
		SGLError ReadLibraryChunk( SSystem::SChunkFile & cf ) ;
		SGLError LoadLibraryXML( const wchar_t * pwszFilePath ) ;
		SGLError ParseLibraryXML( const SSystem::SXMLDocument & xmlLibTag ) ;
		// ポーズファイル書き出し
		SGLError SaveLibrary( const wchar_t * pwszFilePath ) ;
		SGLError WriteLibrary( SSystem::SFileInterface & file ) ;
		SGLError WriteLibraryChunk( SSystem::SChunkFile & cf ) ;
		SGLError SaveLibraryXML( const wchar_t * pwszFilePath ) ;
		SGLError FormatLibraryXML( SSystem::SXMLDocument & xmlLibTag ) ;

	public:
		// 解放
		void Release( void ) ;
		void ReleaseAllPoses( void ) ;
		// ポーズ取得
		S3DModelPose * GetPoseAs
			( const wchar_t * pwszID, bool fOnlyLocal = false ) const ;
		S3DModelPose * GetPoseAt( size_t i ) const ;
		// ポーズ検索
		ssize_t FindPosePtr( S3DModelPose * pPose ) const ;
		// ポーズ数取得
		size_t GetPoseCount( void ) const ;
		// 登録名取得
		const wchar_t * GetPoseIdentityAt( size_t i ) const ;
		const wchar_t * GetPoseIdentityOf
				( S3DModelPose * pPose, bool fOnlyLocal = false ) const ;
		// ポーズ配列取得
		SSystem::SStrSortObjectArray<S3DModelPose>& GetPoseList( void )
		{
			return	m_ssoaPoses ;
		}
		// ポーズ追加
		void AddPoseAs( const wchar_t * pwszID, S3DModelPose * pPose ) ;
		// ポーズ名変更
		SGLError RenamePoseAs
			( S3DModelPose * pPose, const wchar_t * pwszID ) ;
		// ポーズ削除
		void RemovePoseAs( const wchar_t * pwszID ) ;
		void RemovePoseAt( size_t i ) ;
		// 参照されていないポーズを削除
		void CleanupPose( void ) ;
		// 全ポーズ削除
		void RemoveAllPoses( void ) ;
		// ライブラリ名
		const SSystem::SString& GetLibraryName( void ) const ;
		void SetLibraryName( const wchar_t * pwszName ) ;
		// 参照ライブラリを設定する
		void SetParentLibrary( S3DModelPoseLibrary * pLib ) ;
		size_t AddReferenceLibrary( S3DModelPoseLibrary * pLib ) ;
		// 参照ライブラリを取得する
		S3DModelPoseLibrary * GetParentLibrary( void ) const ;
		size_t GetReferenceLibraryCount( void ) const ;
		S3DModelPoseLibrary * GetReferenceLibraryAt( size_t i ) const ;
		void DetachReferenceLibraryAt( size_t i ) ;
		void DetachReferenceLibraryOf( S3DModelPoseLibrary * pLib ) ;
		void DetachAllReferenceLibrarys( void ) ;

	public:
		// ポーズターゲットリセット
		void ResetAllPoseTarget( bool flagResetRef = true ) ;
		// 全てのポーズに含まれる参照先メッシュIDの変更
		void ChangeTargetMeshID
			( const wchar_t * pwszOldID, const wchar_t * pwszNewID ) ;
		// 参照先メッシュの統合
		void MergeTargetMeshID
			( const wchar_t * pwszMergeTarget,
				const wchar_t *const* ppwszMergeSources, size_t nSourceCount ) ; 
	} ;


	//////////////////////////////////////////////////////////////////////////
	// モデルファイル・抽象ローダー
	//////////////////////////////////////////////////////////////////////////

	class	S3DModelLoaderInterface	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DModelLoaderInterface, ESLObject )
		// ファイル拡張子判定
		virtual bool IsMatchableFileExtension( const wchar_t * pszExt ) ;
		// MIME 判定
		virtual bool IsMatchableMIMEType( const wchar_t * pszMIME ) ;
		// モデルデータ読み込み
		virtual SGLError ReadModel
			( S3DModelBuffer & model, SSystem::SFileInterface & file ) = 0 ;

	public:
		enum	SupportModelFileType
		{
			standardModelBinary,		// .mdfx (EntisGLS4 標準)
			standardModelXml,			// .xmlmdf (EntisGLS4 標準)
			legacyModelBinary,			// .mdf (EntisGLS3 互換)
			glTransmissionFormat,		// .glb | .vrm (glTF バイナリ)
			supportModelFileTypeCount,
		} ;
		struct	FileExtensionMIMEPair
		{
			SupportModelFileType	type ;
			const wchar_t *			pszExts[8] ;
			const wchar_t *			pszMIME ;
		} ;
		static const FileExtensionMIMEPair	s_fileExeMimePairs[supportModelFileTypeCount] ;

		// モデルローダー生成
		static S3DModelLoaderInterface *
			NewModelLoaderTypeAs( SupportModelFileType type ) ;
		static S3DModelLoaderInterface *
			NewModelLoaderFileExtensionAs( const wchar_t * pszExt ) ;
		static S3DModelLoaderInterface *
			NewModelLoaderMIMETypeAs( const wchar_t * pszMIME ) ;

	} ;



	//////////////////////////////////////////////////////////////////////////
	// モデルファイル・抽象セーバー
	//////////////////////////////////////////////////////////////////////////

	class	S3DModelSaverInterface	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DModelSaverInterface, ESLObject )
		// MIME 判定
		virtual bool IsMatchableMIMEType( const wchar_t * pszMIME ) ;
		// テクスチャ画像書き出しフォーマット指定
		virtual SGLError SetImageFormat
			( const wchar_t * pwszMIME,
				const wchar_t * pwszExt = NULL,
				const SGLImageEncoderInterface::Options * pOpt = NULL ) ;
		// モデルデータ書き出し
		virtual SGLError WriteModel
			( SSystem::SFileInterface & file, S3DModelBuffer & model ) = 0 ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// モデルデータ・バッファ
	//////////////////////////////////////////////////////////////////////////

	class	S3DModelBuffer	: public S3DVertexBuffer
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DModelBuffer, S3DVertexBuffer )
		// 構築関数
		S3DModelBuffer( void ) ;
		S3DModelBuffer( const S3DModelBuffer & model ) ;
		// 消滅関数
		virtual ~S3DModelBuffer( void ) ;

	public:
		// メッシュ
		typedef S3DModelData::MeshObject	MeshObject ;
		typedef	S3DModelData::MeshGroup		MeshGroup ;
		typedef	S3DModelData::MeshDivision	MeshDivision ;

		// モーフィング・ターゲット・メッシュ
		class	MorphTargetMesh
		{
		public:
			size_t						m_countVertex ;
			SSystem::SArray<S3DVector4>	m_bufVertex ;
			SSystem::SArray<S3DVector4>	m_bufNormal ;
			SSystem::SArray<S2DVector>	m_bufUVMap ;
			SSystem::SArray<S3DColor>	m_bufColor ;
			SSystem::SArray<float32_t>	m_bufWeight ;
		public:
			MorphTargetMesh( void ) : m_countVertex(0) {}
			MorphTargetMesh( const MorphTargetMesh& mtm )
				: m_countVertex(mtm.m_countVertex),
					m_bufVertex(mtm.m_bufVertex),
					m_bufNormal(mtm.m_bufNormal),
					m_bufUVMap(mtm.m_bufUVMap),
					m_bufColor(mtm.m_bufColor),
					m_bufWeight(mtm.m_bufWeight) {}
		} ;

	protected:
		// オブジェクトリスト
		SSystem::SObjectArray<MeshObject>	m_arrMeshObj ;

		// バッファ
		SSystem::SArray<S3DVector4>			m_bufVertex ;
		SSystem::SArray<S3DVector4>			m_bufNormal ;

		size_t								m_iUpdateVertex ;
		size_t								m_iUpdateNormal ;

		// ライブラリ
		S3DTextureLibrary					m_textures ;
		S3DMaterialLibrary					m_materials ;
		S3DModelPoseLibrary					m_poses ;

		// メッシュ・グループ
		SSystem::SStrSortArray<MeshGroup>	m_ssaMeshGroup ;

		// モーフターゲット
		SSystem::SStrSortObjectArray<MorphTargetMesh>
											m_ssoaMorphTarget ;

		// 分割メッシュ情報
		SSystem::SStrSortObjectArray<MeshDivision>
											m_ssoaMeshDivInfo ;

		// マーカー情報
		SSystem::SStrSortObjectArray<S3DModelData::MarkerInfo>
											m_ssoaMarker ;

		// 参照モデル
		S3DModelBuffer *					m_pRefModel ;

		// ボーン更新フラグ
		bool								m_flagUpdateBones ;

		// 物理演算ボーンの有無
		bool								m_flagPhysicsBones ;

		// 物理演算パラメータパレット
		SSystem::SStrSortArray<S3DModelBoneSpace::PhysMaterial>
											m_ssaPhysMaterial ;

		// ボーン
		S3DModelBoneSpace					m_boneSpace ;
		S3DModelBoneSpace					m_boneRoot ;
		SSystem::SStrSortObjectArray<S3DModelBoneSpace>
											m_ssoaBones ;

		// 関連シーンアイテム
		SSystem::SSyncReference				m_refRelItem ;

		// メタ情報
		SSystem::SXMLDocument				m_xmlMetaInfo ;

		// シーン情報
		SSystem::SXMLDocument				m_xmlSceneComposer ;

		// スレッド排他処理用
		SSystem::SCriticalSection			m_csSync ;

	public:
		// モデル読み込み
		virtual SGLError LoadModel
			( const wchar_t * pszFilePath, const wchar_t * pszMIME = NULL ) ;
		virtual SGLError ReadModel
			( SSystem::SFileInterface * file, const wchar_t * pszMIME = NULL ) ;
		// モデル書き出し
		virtual SGLError SaveModel
			( const wchar_t * pszFilePath,
				const wchar_t * pszMIME = NULL,
				const wchar_t * pszImageMIME = NULL ) ;
		virtual SGLError WriteModel
			( SSystem::SFileInterface * file,
				const wchar_t * pszMIME = NULL,
				const wchar_t * pszImageMIME = NULL ) ;
		// ボーン情報のみ読み込み
		SGLError ReadBoneFile( SSystem::SChunkFile * file ) ;
		SGLError ParseBoneXML( SSystem::SXMLDocument & xmlBone ) ;
		SGLError ParseBonePhysicsXML( SSystem::SXMLDocument & xmlBone ) ;
		// ボーン情報のみ書き出し
		SGLError WriteBoneFile( SSystem::SChunkFile * file ) ;
		SGLError FormatBoneXML( SSystem::SXMLDocument & xmlBone ) ;
		SGLError FormatBonePhysicsXML( SSystem::SXMLDocument & xmlBone ) ;
		// マーカー情報読み込み
		SGLError ImportMarkerXML
			( SSystem::SXMLDocument & xmlMarker, bool fOverwrite ) ;
		// マーカー情報書き出し
		SGLError FormatMarkerXML( SSystem::SXMLDocument & xmlMarker ) ;

	public:
		// 参照先モデル取得
		S3DModelBuffer * GetReferenceModel( void ) const
		{
			return	m_pRefModel ;
		}
		// ボーンアニメーション用のモデル参照設定
		SGLError AttachModelReference( S3DModelBuffer & model ) ;		
	protected:
		struct	BoneMapper
		{
			S3DModelBoneSpace *	pDupBone ;
		} ;
		typedef	SSystem::SPtrSortObjectArray
					<S3DModelBoneSpace,BoneMapper>	BonePtrSortObjectArray ;
		void DuplicateReferenceBones
			( BonePtrSortObjectArray& mapBone,
				S3DModelBoneSpace& boneDst,
				S3DModelBuffer & modelRef, S3DModelBoneSpace& boneRef ) ;
		void ReflectAllBonesIdentity( void ) ;

	public:
		// テクスチャライブラリ
		const S3DTextureLibrary& GetTextureLibrary( void ) const
		{
			return	(m_pRefModel == NULL)
						? m_textures : m_pRefModel->m_textures ;
		}
		S3DTextureLibrary& GetTextureLibrary( void )
		{
			return	(m_pRefModel == NULL)
						? m_textures : m_pRefModel->m_textures ;
		}
		// 表面属性ライブラリ
		const S3DMaterialLibrary& GetMaterialLibrary( void ) const
		{
			return	(m_pRefModel == NULL)
						? m_materials : m_pRefModel->m_materials ;
		}
		S3DMaterialLibrary& GetMaterialLibrary( void )
		{
			return	(m_pRefModel == NULL)
						? m_materials : m_pRefModel->m_materials ;
		}
		// ポーズライブラリ
		const S3DModelPoseLibrary& GetPoseLibrary( void ) const
		{
			return	(m_pRefModel == NULL)
						? m_poses : m_pRefModel->m_poses ;
		}
		S3DModelPoseLibrary& GetPoseLibrary( void )
		{
			return	(m_pRefModel == NULL)
						? m_poses : m_pRefModel->m_poses ;
		}
		// モデル全体用特殊ボーン用
		S3DModelBoneSpace& GetLocalSpaceBone( void )
		{
			return	m_boneSpace ;
		}
		// ルートボーン
		S3DModelBoneSpace& GetBoneRoot( void )
		{
			return	m_boneRoot ;
		}
		// メッシュ・グループ配列
		SSystem::SStrSortArray<MeshGroup>& GetMeshGroupList( void )
		{
			return	(m_pRefModel == NULL)
						? m_ssaMeshGroup : m_pRefModel->m_ssaMeshGroup ;
		}
		const SSystem::SStrSortArray<MeshGroup>& GetMeshGroupList( void ) const
		{
			return	(m_pRefModel == NULL)
						? m_ssaMeshGroup : m_pRefModel->m_ssaMeshGroup ;
		}
		// メッシュグループ取得
		MeshGroup * GetMeshGroupAs( const wchar_t * pwszID ) const ;
		// メッシュ名取得
		const SSystem::SString * GetMeshIdentityAt( size_t iMesh ) const ;
		// モーフターゲット追加
		MorphTargetMesh * AddMorhTargetAs
			( const wchar_t * pwszID, MorphTargetMesh * pMorphTarget ) ;
		// モーフターゲット取得
		MorphTargetMesh * GetMorhTargetAs( const wchar_t * pwszID ) const ;
		// モーフターゲット配列
		SSystem::SStrSortObjectArray<MorphTargetMesh>& GetMorphTargetList( void )
		{
			return	(m_pRefModel == NULL)
						? m_ssoaMorphTarget
							: m_pRefModel->m_ssoaMorphTarget ;
		}
		// 分割メッシュ情報取得
		const MeshDivision * GetMeshDivisionAs( const wchar_t * pwszID ) const ;
		// 分割メッシュ情報配列
		SSystem::SStrSortObjectArray<MeshDivision>& GetMeshDivisionList( void )
		{
			return	(m_pRefModel == NULL)
						? m_ssoaMeshDivInfo
						: m_pRefModel->m_ssoaMeshDivInfo ;
		}
		// マーカー情報数
		size_t GetMarkerInfoCount( void ) const ;
		// マーカー情報ID取得
		const SSystem::SString * GetMarkerInfoIdentityAt( size_t i ) const ;
		// マーカー情報取得
		S3DModelData::MarkerInfo * GetMarkerInfoAs( const wchar_t * pwszID ) const ;
		S3DModelData::MarkerInfo * GetMarkerInfoAt( size_t i ) const ;
		// マーカー情報配列
		SSystem::SStrSortObjectArray<S3DModelData::MarkerInfo>& GetMarkerInfoList( void )
		{
			return	(m_pRefModel == NULL)
						? m_ssoaMarker : m_pRefModel->m_ssoaMarker ;
		}
		// 所有ボーン登録
		void AddBonePropertyAs
			( const wchar_t * pwszName, S3DModelBoneSpace * pBone ) ;
		// 所有ボーン取得
		S3DModelBoneSpace *
			GetBonePropertyAs( const wchar_t * pwszName ) const ;
		// 所有ボーンID取得
		const SSystem::SString *
			GetBoneIdentityOf( S3DModelBoneSpace * pBone ) const ;
		// 所有ボーン配列
		SSystem::SStrSortObjectArray<S3DModelBoneSpace>&
									GetBonePropertyList( void )
		{
			return	m_ssoaBones ;
		}
		const SSystem::SStrSortObjectArray<S3DModelBoneSpace>&
									ConstBonePropertyList( void ) const
		{
			return	m_ssoaBones ;
		}
		// ボーン物理演算パラメータパレット配列
		SSystem::SStrSortArray<S3DModelBoneSpace::PhysMaterial>&
											GetPhysMaterialList( void )
		{
			return	m_ssaPhysMaterial ;
		}
		// ボーン物理演算パラメータパレット削除
		void RemovePhysMaterialPaletteAs( const wchar_t * pwszID ) ;
		// ボーン物理演算パラメータパレット更新反映
		void UpdatePhysMaterialPaletteAs
			( const wchar_t * pwszID,
				const S3DModelBoneSpace::PhysMaterial& physMaterial ) ;
		// 関連アイテムを設定
		void AttachRelationItem( S3DScene::Item * pItem ) ;
		// 関連アイテムを取得
		S3DScene::Item * GetRelationItem( void ) const ;
		// デバイスメモリ上に準備する
		SGLError CommitToDevice
			( S3DRenderDevice * pDev, int64_t msecTimeout = 0 ) ;
		// デバイス上のメモリを開放する
		SGLError ReleaseForDevice
			( S3DRenderDevice * pDev, int64_t msecTimeout = 0 ) ;

	public:
		// 頂点バッファのサイズを設定する
		virtual SGLError SetVertexBufferLength( size_t nLength ) ;
		// 頂点バッファのサイズを取得する
		size_t GetVertexBufferLength( void ) const ;
		// 頂点バッファへの変更を確定する
		virtual void CommitVertexBuffer( size_t iFirst, size_t nLength ) ;
		// 頂点バッファへのポインタを取得
		S3DVector4 * GetVertexBufferAt( size_t index = 0 ) const ;
		// 頂点ポインタをインデックスへ変換
		ssize_t VertexPointerToIndex( const S3DVector4 * pvVertex ) const ;
		// 法線バッファのサイズを設定する
		virtual SGLError SetNormalBufferLength( size_t nLength ) ;
		// 法線バッファのサイズを取得する
		size_t GetNormalBufferLength( void ) const ;
		// 法線バッファへの変更を確定する
		virtual void CommitNormalBuffer( size_t iFirst, size_t nLength ) ;
		// 法線バッファへのポインタを取得
		S3DVector4 * GetNormalBufferAt( size_t index = 0 ) const ;
		// 法線ポインタをインデックスへ変換
		ssize_t NormalPointerToIndex( const S3DVector4 * pvNormal ) const ;
		// メッシュエントリ取得
		MeshObject * GetMeshObjectAt( size_t iMesh ) const ;
		// 指定メッシュのメッシュ名取得
		const SSystem::SString * GetMeshGroupNameIndexOf( size_t iMesh ) const ;
		// メッシュの外接直方体取得
		bool GetCircumscribedBoxOfMesh
			( S3DVector& vMin, S3DVector& vMax, const MeshObject& meshObj ) const ;
		bool GetCircumscribedBoxOfMeshAt
			( S3DVector& vMin, S3DVector& vMax, size_t iMesh ) const ;
		// 物理演算の内部パラメータを複製
		void CopyBonePhysicsParamaters( const S3DModelBuffer& model ) ;
		void CopyBonePhysicsParamaters
			( const SSystem::SStrSortObjectArray<S3DModelBoneSpace>& ssoaBones ) ;
		void GetBonePhysicsParamaters
			( SSystem::SStrSortArray
					<S3DModelBoneSpace::PhysVertex>& ssaPhys ) ;
		void SetBonePhysicsParamaters
			( const SSystem::SStrSortArray
					<S3DModelBoneSpace::PhysVertex>& ssaPhys ) ;

	public:
		// 関連性のあるモーフターゲットをメッシュに設定する
		void BuildupMeshMorphingTarget( void ) ;

	public:
		// ボーン関連性を解決して構造を完成する
		void BuildupBoneRelation( void ) ;
	protected:
		void BuildupSubBoneRelation( S3DModelBoneSpace * pBone ) ;
		void BuildupBoneWeightMap
			( size_t iMesh, const float32_t ** ppWeightMaps,
				size_t nBoneCount, size_t nVertexCount ) ;

	public:
		// ボーンの有無
		bool AreAnyBones( void ) const ;
		// 物理演算ボーンの有無
		bool AreAnyPhysicsBones( void ) const ;
		// 物理演算ボーンの更新処理
		void NotifyPhysicsBonesUpdate( void ) ;
	protected:
		void NotifyPhysicsSubBonesUpdate( S3DModelBoneSpace * pBone ) ;
	public:
		// すべてのボーンの変更フラグをクリア
		void ClearAllBonesModifiedFlags( void ) ;
		// ボーン更新フラグ取得
		bool IsUpdateBone( void ) const ;
		// ボーン更新フラグ設定
		void PostUpdateBone( void ) ;
		// ボーン回転行列を VBO に反映する
		void UpdateBoneMatrix( void ) ;
		// バッファバリアントの場合、ボーンやマテリアルの設定を参照先のバッファへ即時に反映する
		void ReflectBufferVariantImmediately( void ) ;
		// ボーンの外接直方体を取得する
		void GetBoneCircumscribedParallelepiped
					( S3DDVector& vMin, S3DDVector& vMax ) const ;

	public:
		// VBO を再構築する（MeshObject 等を編集した場合）
		void RebuildVertexBuffer( void ) ;

	public:	// S3DVertexVariantBuffer オーバーライド
		// メッシュにボーン行列設定
		virtual SGLError SetBoneMatrix
			( size_t iMesh, size_t nCount,
				const S3DMatrix * pMatrix, const S3DVector * pTrans ) ;
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

	public:	// S3DRenderBufferInterface オーバーライド
		enum	AddRenderFlagEx
		{
			renderNormalizeFace	= S3DRenderBuffer::renderNormalizeFace,
		} ;
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
		// 描画の確定
		virtual SGLError Flush( void ) ;
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
		// バッファを S3DRenderBufferInterface へ出力
		virtual SGLError RenderBufferTo
			( S3DRenderBufferInterface * render,
					uint64_t flagsExclusion = 0,
					size_t iFrist = 0, ssize_t iEnd = -1,
					size_t nInstancing = 0,
					const S4DMatrix * pmatInstancing = NULL,
					const S3DColor * pColorInstancing = NULL ) const ;
		// モデル描画が表示範囲にあるか見積もる
		virtual bool IsModelIntoView
			( S3DRenderContextInterface * render,
						float32_t fpScaleMargin = 0.5f,
						float32_t fpModelMargin = 1.0f ) ;

	protected:
		// 三角ポリゴンリストの頂点順（表裏）正規化
		void NormalizeTriangleSurface( MeshObject * pMesh ) ;

	public:
		// メタ情報
		enum	MetaInfoFlag
		{
			metaInfoFlagImport	= 0x0001,
			metaInfoFlagEditLog	= 0x0002,
		} ;
		struct	MetaInfo
		{
			uint32_t				nFlags ;
			S3DVector				vImportScale ;
			const wchar_t *			pwszSrcFile ;
			SSystem::SXMLDocument *	pxmlEditLog ;
		} ;
		SGLError ParseMetaInfo( MetaInfo& infMeta ) const ;
		void SetMetaInfo( const MetaInfo& infMeta ) ;
		const SSystem::SXMLDocument & GetMetaInfo( void ) const
		{
			return	m_xmlMetaInfo ;
		}
		SSystem::SXMLDocument & EditMetaInfo( void )
		{
			return	m_xmlMetaInfo ;
		}
		// シーン情報
		const SSystem::SXMLDocument & GetSceneComposition( void ) const
		{
			return	m_xmlSceneComposer ;
		}
		SSystem::SXMLDocument & EditSceneComposition( void )
		{
			return	m_xmlSceneComposer ;
		}

	public:	// S3DVertexBufferInterface オーバーライド
		// バッファを消去
		virtual void ClearBuffer( void ) ;

	public:	// マーカー・当たり判定
		// マーカーを当たり判定として追加
		enum	MarkerCollisionOffset
		{
			markerInvalidOffset	= -32,		// マーカーの値で UserClassesMask を変更しない
		} ;
		uint32_t AddMarkerForCollision
			( S3DCollision& render,
				const S3DModelData::MarkerInfo& mi, ssize_t iColOffset ) const ;
		size_t AddAllMarkerForCollision
			( S3DCollision& render,
				S3DModelData::MarkerInfo::Type
					type = S3DModelData::MarkerInfo::typeCollider,
				const wchar_t * pwszLeadID = nullptr,
				ssize_t iColOffset = 0, uint32_t * pGetMarkerColMask = nullptr ) ;
		// マーカーの当たり判定
		bool IsHitAgainstMarker
			( S3DCollider& collider,
				S3DCollisionResult& rsHit,
				const S3DDMatrix& matSpace,
				const S3DDVector& vSpace,
				const S3DModelData::MarkerInfo& mi ) const ;
		// マーカー球座標計算
		void CalcMarkerPosition
			( S3DDVector& vPos, float& fpRadius,
				const S3DDMatrix& matSpace,
				const S3DDVector& vSpace,
				const S3DModelData::MarkerInfo& mi ) const ;
		void CalcMarkerTransformation
			( S3DDVector& vPos, float& fpRadius,
				S3DDMatrix& matSpace,
				S3DDVector& vSpace,
				const S3DModelData::MarkerInfo& mi ) const ;
		void CalcMarkerReferenceBoneSpace
			( S3DDMatrix& matBone,
				S3DDVector& vBone,
				const S3DModelData::MarkerInfo& mi ) const ;

	public:
		// スレッド排他処理
		void LockModelData( void ) ;
		void UnlockModelData( void ) ;

	public:
		// 別モデルのポーズをこのモデルへ複製する
		void DuplicatePoseOf( const S3DModelBuffer& model ) ;

	public:
		// 現在のポーズを取得する
		void TranscribeCurrentPose( S3DModelPose * pPose ) const ;
		// 現在のボーンの状態のポーズを取得する
		void TranscribeBoneToPose( S3DModelPose * pPose ) const ;
		// 現在のモーフィングの状態のポーズを取得する
		void TranscribeMorphToPose( S3DModelPose * pPose ) const ;
		// 現在のメッシュ表示状態のポーズを取得する
		void TranscribeMeshVisibleToPose( S3DModelPose * pPose ) const ;

	public:
		// 現在のポーズをモーションフレームとしてサンプリングする
		SGLError SampleCurrentPose
			( S3DModelPose * pPose, size_t iFrame,
				const S3DDMatrix * pBaseMatrix = NULL,
				const S3DDVector * pBasePos = NULL ) const ;
		// モーションデータの中で
		// 物理演算ボーン要素（物理演算適用度1.0）ジョイントを削除るする
		void CleanupPhysJointOfMotion( S3DModelPose * pPose ) const ;

	public:
		// 指定画像をテクスチャとして参照しているか？
		bool IsTextureUsed( SGLImageObject * pImage ) const ;
		// 指定マテリアルは使用中か？
		bool IsMaterialUsed( S3DMaterial * pMaterial ) const ;
		// 指定モーフターゲットは使用中か？
		bool IsMorphTargetUsed( const wchar_t * pwszMorphID ) const ;

	public:	// モデル編集
		// メッシュの裏表を法線に向きに一致させる
		size_t NormalizeAllMeshsFace( void ) ;
		// メッシュ名変更
		SGLError ModifyMeshIdentity
			( const wchar_t * pwszOldID, const wchar_t * pwszNewID ) ;
		// 表裏反転
		void InverseMeshFace( size_t iMesh, bool fInverseNormal ) ;
		// 重複頂点の法線を揃える
		size_t MergeNormalOfRedundantVertex
			( size_t iMesh, size_t nMeshCount,
				double cosThreshold,
				double fpError = 0.001,
				bool flagRefLocalMesh = false, bool flagUVMatch = false ) ;
		// 重複頂点のボーンウェイトを揃える
		size_t MergeBoneWeightOfRedundantVertex
			( size_t iMesh, size_t nMeshCount, double fpError = 0.001 ) ;
		// 未使用頂点を削除する
		void TrimUnusedVertex( void ) ;
		void TrimRemapVertex
			( const ssize_t * pIndexMap, size_t nIndexLength, size_t nTotalUsed ) ;
		// ボーンのウェイトマップサイズをメッシュ境界に合うように正規化する
		void NormalizeBoneWeightMapRange( void ) ;
		// マテリアル統合
		SGLError MergeMaterials
			( S3DMaterial*const* ppMaterials, size_t nCount,
				const wchar_t * pwszTextureBaseID = nullptr,
				const wchar_t * pwszBackTextureBaseID = nullptr ) ;
	protected:
		// テクスチャ統合
		SGLImageObject * MergeMaterialTextures
			( const SGLSize& sizeMergeTexture,
				S3DMaterial*const* ppMaterials,
				SGLImageRect** ppDstRects,
				const SGLImageRect* pSrcRects,
				int nTextureType, bool fBackSurface, size_t nCount ) ;
	public:
		// メッシュ統合
		SGLError MergeMeshMaterialOf( S3DMaterial * pMaterial ) ;
		SGLError MergeMeshs
			( const size_t * pTargetMeshs, size_t nTargetMeshCount ) ;
	public:
		// メッシュオブジェクト削除（頂点は無修正）
		void RemoveMeshObjectAt( size_t iMesh, bool fRebuildVBO = false ) ;
		// モーフィングターゲット範囲補正
		void BoundMorphTargetMesh
			( MorphTargetMesh * pMorph, ssize_t nLeftPad, size_t nVertexCount ) ;
	protected:
		void RemoveBoneMeshRef( S3DModelBoneSpace * pBone, size_t iMesh ) ;

	public:
		// メッシュ削除（頂点も削除）
		SGLError RemoveVertexMeshAt( size_t iMesh, bool fRebuildVBO = false ) ;

	public:
		// メッシュをボーン影響ごとに分割
		struct	DivieMeshByBoneParam
		{
			size_t	nMaxBoneCount ;		// １メッシュあたりの最大影響ボーン数
			size_t	nLowVertexCount ;	// 以下ならボーン数を無視する頂点数

			DivieMeshByBoneParam( void )
				: nMaxBoneCount(10), nLowVertexCount(0x100) { }
		} ;
		SGLError DivideMeshByBoneBounds
			( SSystem::SObjectArray<SSystem::SString>& aDivMeshIDs,
					size_t iMesh, const DivieMeshByBoneParam& dmbbp ) ;
	protected:
		SGLError DivideMeshByBoneBoundsInBone
			( SSystem::SObjectArray<SSystem::SString>& aDivMeshIDs,
				MeshObject * pmoSrc, size_t iSrcMesh,
				const SSystem::SString& strMeshBaseID,
				SSystem::SBitArray& maskException,
				SSystem::SBitArray& maskPending,
				const DivieMeshByBoneParam& dmbbp,
				S3DModelBoneSpace * pBone ) ;
		void DivideMeshByPendingBoneBounds
			( SSystem::SObjectArray<SSystem::SString>& aDivMeshIDs,
				size_t iSrcMesh,
				const SSystem::SString& strMeshBaseID,
				SSystem::SBitArray& maskPending, bool fMoveMorphMesh ) ;

	public:
		// １メッシュが指定ポリゴン数以下になるように分割
		SGLError DivideMeshByPolygonCount
			( SSystem::SObjectArray<SSystem::SString>& aDivMeshIDs,
								size_t iMesh, size_t nLimitPolygon ) ;
	protected:
		// ポリゴン分割用情報
		struct	DivPolyEntryInfo
		{
			double	fpDistance ;		// 基準点からの距離
			size_t	iPolygon ;			// ポリゴン番号
			size_t	iCriterion ;		// 基準点
		} ;
		// ポリゴンを基準点からの距離でソート
		void QuickSortDivPolyEntries
			( DivPolyEntryInfo * pdpis, size_t nCount ) ;

	public:
		// 部分メッシュ複製追加
		SGLError DuplicatePortionOfMesh
			( const wchar_t * pwszNewMeshID,
				size_t iSrcMesh,
				const SSystem::SBitArray& maskVertexPortion,
				bool fMoveMorphMesh = false, bool fRebuildVBO = false ) ;
		// メッシュ名を重複しない名前に正規化
		void NormalizeMeshID( SSystem::SString& strMeshID ) const ;

	public:
		// メッシュ頂点ポリゴン隣接マスク生成
		static void ExpandMeshEffectMaskNextPolygon
			( SSystem::SBitArray& maskEffect,
				const uint32_t * pPolyIndex, size_t nPolyCount ) ;
		// モーフィングメッシュ影響範囲取得
		static void GetMorphMeshEffectMask
			( SSystem::SBitArray& maskEffect, const MorphTargetMesh& mtmMesh ) ;

	public:
		// パノラマ画像から skybox モデルを生成
		SGLError BuildSkyboxFromPanoramaImage
			( double fpBoxScale, int nBoxSize,
				double degHRotAngle, SGLImageObject& imgPanorama ) ;
		static SGLError MakeSkyboxFromPanoramaImage
			( SGLImageObject& imgSkybox,
				SGLImageRect* pCubeFaces,
				int nBoxSize, double degHRotAngle,
				SGLImageObject& imgPanorama ) ;
		static void SampleSkyboxFaceForPanoramaImage
			( SGLImageObject& imgSkybox,
				SGLImageRect& rectFace,
				const S3DMatrix& matFace,
				const S3DVector& vScreen,
				SGLImageObject& imgPanorama ) ;
		static SGLPalette SampleByRayFromPanoramaImage
			( SGLImageObject& imgPanorama, const S3DVector& vRay ) ;
		// パノラマ画像から八面体モデルを生成
		SGLError BuildOctahedronFromPanoramaImage
			( double fpOctahedronSize, int nOctMapSize,
				double degHRotAngle,
				SGLImageObject& imgPanorama, bool flagDivFace = true ) ;
		static SGLError MakeOctahedronMapFromPanoramaImage
			( SGLImageObject& imgOctMap, int nOctMapSize,
				double degHRotAngle, SGLImageObject& imgPanorama ) ;

	public:
		// 指定メッシュの指定UVに関する情報を取得する
		struct	MeshPointInfo
		{
			S3DVector	vPoint ;
			S3DVector	vNormal ;
			S2DVector	vUV ;
			S3DVector	vAxisX ;		// U 基底
			S3DVector	vAxisY ;		// V 基底
		} ;
		size_t GetMeshPointInfoAtTexturePos
			( SSystem::SArray<MeshPointInfo>& aResult,
				size_t nResultLimit, size_t iMesh, const S2DVector& vUV ) const ;
		SGLError GetMeshPointInfoAtPolygon
			( MeshPointInfo& mpiResult, size_t iMesh,
				size_t iPolygon, float32_t uDelta, float32_t vDelta ) const ;

	public:
		// 処理進捗
		class	ProgressNotification
		{
		public:
			// 進捗通知 (false 返却で処理中断)
			// ※複数のスレッドから呼び出される
			virtual bool OnProgress
					( unsigned long int nCurrent,
							unsigned long int nTotal ) = 0 ;
		} ;

	public:
		// 形状をメッシュテクスチャに投影
		enum	ProjectMeshTextureFlag
		{
			flagProjTexWithLight	= 0x0001,
		} ;
		struct	ProjectMeshTextureParam
		{
			SGLImageObject *	pDiffusionTexture ;
			SGLImageObject *	pLuminousTexture ;
			SGLImageObject *	pNormalTexture ;
			uint32_t			nFlags ;		// complex of enum ProjectMeshTextureFlag
			size_t				iMesh ;
			float32_t			fpBackReach ;
			float32_t			fpFrontReach ;
			float32_t			fpHitErrorGap ;
			S3DVector			vVecLight ;
			float32_t			fpBrightness ;
			float32_t			fpAmbientLight ;

			ProjectMeshTextureParam( void )
				: pDiffusionTexture( NULL ),
					pLuminousTexture( NULL ),
					pNormalTexture( NULL ),
					nFlags( 0 ), iMesh( 0 ),
					fpBackReach( 1.0 ),
					fpFrontReach( 1.0 ),
					fpHitErrorGap( 0.00001f ),
					vVecLight( 0, 1, 0 ),
					fpBrightness( 1.0f ),
					fpAmbientLight( 0.5f ) { }
		} ;
		SGLError ProjectMeshTextureForShape
			( const ProjectMeshTextureParam& pmtp,
				S3DCollider& collider,
				ProgressNotification * pNotification ) const ;
	protected:
		struct	ProjectMeshTextureProcInstance
		{
			SSystem::SArray<MeshPointInfo>	aMeshPoints ;
			size_t							yLine ;

			ProjectMeshTextureProcInstance( void )
			{
				yLine = 0 ;
			}
		} ;
		class	ProjectMeshTextureProc : public SSystem::SParallelProcedure
		{
		protected:
			SSystem::STimeCounter	m_timer ;
			const S3DModelBuffer &	m_model ;
			ProjectMeshTextureParam	m_pmtp ;
			SGLImageBuffer *		m_pDiffusion ;
			SGLImageBuffer *		m_pLuminous ;
			SGLImageBuffer *		m_pNormal ;
			size_t					m_yLineCount ;
			size_t					m_yNextLine ;
			S3DCollider &			m_collider ;
			ProgressNotification *	m_pNotification ;
			bool					m_flagCanceled ;
		public:
			// 構築関数
			ProjectMeshTextureProc
				( const S3DModelBuffer& model,
					const ProjectMeshTextureParam& pmtp,
					SGLImageBuffer * pDiffusion,
					SGLImageBuffer * pLuminous,
					SGLImageBuffer * pNormal,
					S3DCollider & collider,
					ProgressNotification * pNotification ) ;
			// ループ処理／終了判定関数
			virtual bool Continue( void * pInstance ) ;
			// 並列処理関数
			virtual void RunParallel( void * pInstance ) ;
			// キャンセルしたか？
			bool IsCanceled( void ) const
			{
				return	m_flagCanceled ;
			}
		} ;

	public:
		// AO／GIを頂点色へ反映する
		struct	GlobalIlluminationParam
		{
			S3DMatrix					matItem ;
			S3DVector					vItem ;
			S3DVertexBufferInterface *	pVBO ;
			size_t						iMesh ;
			size_t						nSamplingCount ;
			SGLPalette					rgbAmbient ;
			float32_t					fpReachLength ;
			float32_t					fpApplyAO ;
			float32_t					fpApplyGI ;
			float32_t					fpErrorGap ;

			GlobalIlluminationParam( void )
				: matItem( 1, 1, 1 ), vItem( 0, 0, 0 ),
					pVBO( NULL ), iMesh( 0 ),
					nSamplingCount( 100 ), rgbAmbient( 0 ),
					fpReachLength( 100.0f ),
					fpApplyAO( 1.0f ),
					fpApplyGI( 0.1f ), fpErrorGap( 0.00001f ) { }
		} ;
		static SGLError GlobalIlluminationForVertexColor
			( const GlobalIlluminationParam& gip,
				S3DCollider& collider,
				ProgressNotification * pNotification ) ;
	protected:
		struct	GlobalIlluminationProcInstance
		{
			SakuraCL::SCLRandomizer	randomizer ;
			size_t					iVertex ;

			GlobalIlluminationProcInstance( void )
			{
				randomizer.InitializeSeed() ;
				iVertex = 0 ;
			}
		} ;
		class	GlobalIlluminationProc : public SSystem::SParallelProcedure
		{
		protected:
			SSystem::STimeCounter	m_timer ;
			GlobalIlluminationParam	m_gip ;
			const S3DVector4 *		m_pvVertex ;
			const S3DVector4 *		m_pvNormal ;
			S3DColor *				m_pColor ;
			size_t					m_nVertexCount ;
			size_t					m_iNextVertex ;
			S3DCollider &			m_collider ;
			ProgressNotification *	m_pNotification ;
			bool					m_flagCanceled ;
		public:
			// 構築関数
			GlobalIlluminationProc
				( const GlobalIlluminationParam& gip,
					const S3DVector4 * pvVertex,
					const S3DVector4 * pvNormal,
					S3DColor * pColor, size_t nVertexCount,
					S3DCollider & collider,
					ProgressNotification * pNotification ) ;
			// ループ処理／終了判定関数
			virtual bool Continue( void * pInstance ) ;
			// 並列処理関数
			virtual void RunParallel( void * pInstance ) ;
			// キャンセルしたか？
			bool IsCanceled( void ) const
			{
				return	m_flagCanceled ;
			}
		} ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// S3DScene 動的モデルデータアイテム
	//////////////////////////////////////////////////////////////////////////

	class	S3DDynamicModelItem	: public S3DScene::ModelItem
	{
	public:
		// 動的モデル・抽象ポーズアニメーション
		class	PoseAnimator	: public SSystem::SObject
		{
		public:
			enum	Flag
			{
				flagVolatile	= 0x0001,		// 単発アクション等
				flagTransition	= 0x0002,		// トランジッション
			} ;

		protected:
			int32_t		m_priority ;	// 優先度（小さいほうが先に処理）
			uint32_t	m_flags ;		// フラグ集合
			double		m_blend ;		// ブレンド率 (0.0～1.0)

		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( PoseAnimator, SObject )
			// 構築関数
			PoseAnimator( void ) ;
			PoseAnimator( const PoseAnimator& pose ) ;
			// 消滅関数
			virtual ~PoseAnimator( void ) ;
			// 時間経過 (終了時に true 返却)
			virtual bool OnTimer
				( S3DDynamicModelItem& item, uint32_t msecPast ) ;
			// ポーズ設定
			virtual void ApplyPose( S3DModelBuffer& model ) ;
			// 即時完了
			virtual void FlushAnimation( S3DDynamicModelItem& item ) ;
			// キャンセル（アニメーションの停止）
			virtual void CancelAnimation( S3DDynamicModelItem& item ) ;
			// 揮発ポーズか？
			virtual bool IsVolatilePose( void ) const ;
			// 複製
			virtual PoseAnimator * Duplicate( void ) ;

		public:
			// 優先度
			int32_t GetPriority( void ) const
			{
				return	m_priority ;
			}
			void SetPriority( int32_t priority ) ;
			// フラグ
			uint32_t GetFlags( void ) const
			{
				return	m_flags ;
			}
			void SetFlags( uint32_t flags ) ;
			// ブレンド率
			double GetBlendWeight( void ) const
			{
				return	m_blend ;
			}
			void SetBlendWeight( double blend ) ;
		} ;

		// アニメーション完了原因
		enum	FinishAnimationReason
		{
			reasonCompleted,		// アニメーション完了
			reasonCleanup,			// 揮発アニメーション削除
			reasonDetach,			// 分離
			reasonRemove,			// 削除
		} ;

		// モデルアニメーションリスナ
		class	AnimationListener
		{
		public:
			// アニメーション通知
			virtual void OnAnimation( S3DDynamicModelItem * pItem ) = 0 ;
			// アニメーションが追加された
			virtual void OnAddAnimation
				( S3DDynamicModelItem * pItem, PoseAnimator * pAnim ) = 0 ;
			// アニメーション完了通知
			virtual void OnFinishAnimation
				( S3DDynamicModelItem * pItem,
					PoseAnimator * pAnim, FinishAnimationReason reason ) = 0 ;
		} ;

	protected:
		SSystem::SObjectArray<PoseAnimator>
						m_arrPoseTracks ;	// ポーズ制御レイヤー
		S3DModelBoneSpace::PhysExogenous
						m_physExogenous ;	// 物理演算パラメータ
		uint32_t		m_msecPastPhys ;	// 物理演算デルタ経過時間
		bool			m_flagTimerAnime ;		// OnTimer でアニメーション進行
		bool			m_flagTimerPhysics ;	// OnTimer で物理演算時間進行
		bool			m_flagDelayResetPhys ;	// 次の物理演算更新時にパラメータをリセットする
		size_t			m_nDelayDrivePhysFrame ;
		double			m_secDelayDriveFrame ;

	public:
		enum	PhysEffectFlag
		{
			physNoEffectRotation	= 0x0001,	// 回転効果無効
			physSwayStream			= 0x0002,	// 流速揺らぎ有効
			physGlobalCollision		= 0x0100,	// 当たり判定を大域で（デフォルトはローカルのみ）
		} ;
		struct	PhysEffectParam
		{
			uint32_t	flagsEffects ;			// complex of enum PhysEffectFlag
			double		fpMoveAccel ;			// 平行移動加速度影響比率
			double		fpMoveStream ;			// 平行移動大気流速影響比率
			double		fpSwayAmplitude[2] ;	// 流速揺らぎ率
			double		fpSwayWaveLength[2] ;	// 流速揺らぎ波長
		} ;
	protected:
		S3DCollision	m_collisionBone ;		// ボーン当たり判定オブジェクト
		PhysEffectParam	m_physEffect ;			// 物理演算大域効果
//		uint32_t		m_flagsPhysEffects ;	// 物理演算影響フラグ
//		double			m_fpMoveEffect ;		// 平行移動影響比率
		double			m_fpMoveLength ;		// 平行移動累積距離

		AnimationListener *	m_pAnimListener ;

		uint32_t		m_maskMarkerCollider ;	// enum S3DModelData::MarkerInfo::Type のビットマスク

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DDynamicModelItem, ModelItem )
		// 構築関数
		S3DDynamicModelItem( void ) ;
		// 消滅関数
		virtual ~S3DDynamicModelItem( void ) ;
		// モデルデータ関連付け
		void AttachModel( S3DVertexBufferInterface * pModel ) ;
		void AttachCollisionModel( S3DVertexBufferInterface * pColModel ) ;
		// 当たり判定モデル構築
		virtual void BuildCollisionMesh( void ) ;
		// モデルデータのマーカーをコライダに設定する
		void SetModelMarkerForCollider( uint32_t maskTypes ) ;
		uint32_t GetModelMarkerForCollider( void ) const ;
		// モデルとポーズの関連付けを解除する
		virtual void DetachAll( void ) ;
		// ポーズトラック追加
		void AddPoseAnimator( PoseAnimator * pPose ) ;
		// ポーズトラック削除
		bool RemovePoseAnimator( PoseAnimator * pPose ) ;
		void RemoveAllPoseAnimators( void ) ;
		// ポーズトラック分離
		PoseAnimator * DetachPoseAnimator( PoseAnimator * pPose ) ;
		// ポーズトラック取得
		PoseAnimator * GetPoseAnimatorPriorityOf( int32_t nPriority ) const ;
		// ポーズ優先度挿入指標
		size_t OrderPoseAnimatorPriorityOf( int32_t nPriority ) const ;
		// ポーズアニメーションの即時強制完了と物理演算初期化
		void FlushPoseAnimation( void ) ;
		// ポーズアニメーションのキャンセル処理（アニメーションの停止）
		void CancelPoseAnimation( void ) ;
		// すべての揮発性アニメーションを削除
		void CleanupVolatilePoses( void ) ;
		// 不揮発アニメーションの有無
		bool AreAnyVolatilePoses( void ) const ;
		// 物理演算内部運動量リセット
		virtual void ResetPhysicsParameter( void ) ;
		// 物理演算を指定フレーム数行う
		void DrivePhysicsFrames( size_t nFrames, double secFrame = 0.016667 ) ;
		// 遅延物理演算リセット処理
		void DelayResetPhysicsAndDriveFrames( size_t nFrames = 0, double secFrame = 0.016667 ) ;
		// OnTimer でポーズアニメーション処理（デフォルト＝有効）
		void EnablePoseAnimationOnTimer( bool fTimerPoseAni ) ;
		bool IsEnabledPoseAnimationOnTimer( void ) const ;
		// ポーズアニメーション進行
		void AdvanceAnimationTime
			( S3DScene& scene, uint32_t msecPoseTime, uint32_t msecPhysTime ) ;
		// OnTimer で物理演算処理（デフォルト＝有効）
		void EnablePhysicsTimeOnTimer( bool fTimerPhysics ) ;
		bool IsEnabledPhysicsTimeOnTimer( void ) const ;
		// 物理演算進行
		void AddPhysicsTime( uint32_t msecPhysTime ) ;

	public:
		// アニメーションリスナ設定
		bool AttachAnimationListener( AnimationListener * pListener ) ;
		// アニメーションリスナ解除
		bool DetachAnimationListener( AnimationListener * pListener ) ;
	protected:
		// アニメーション通知
		virtual void NotifyOnAnimation( void ) ;
		// アニメーション追加通知
		virtual void NotifyOnAddAnimation( PoseAnimator * pAnim ) ;
		// アニメーション完了通知
		virtual void NotifyOnFinishAnimation
			( PoseAnimator * pAnim, FinishAnimationReason reason ) ;

	public:
		// 物理演算影響フラグ (enum PhysEffectFlag)
		uint32_t GetPhysEffectFlags( void ) const ;
		void SetPhysEffectFlags( uint32_t nFlags ) ;
		// 平行移動加速度影響度
		double GetMoveAccelEffect( void ) const ;
		void SetMoveAccelEffect( double fpEffect ) ;
		// 平行移動流速影響度
		double GetMoveStreamEffect( void ) const ;
		void SetMoveStreamEffect( double fpEffect ) ;
		// 物理演算影響パラメータ
		const PhysEffectParam& GetPhysEffectParam( void ) const ;
		void SetPhysEffectParam( const PhysEffectParam& param ) ;
		// 重力加速度
		const S3DDVector& GetAcceleration( void ) const ;
		void SetAcceleration( const S3DDVector& vAccel ) ;
		// 空間流速
		const S3DDVector& GetStream( void ) const ;
		void SetStream( const S3DDVector& vStream ) ;

	public:
		// タイマ処理
		virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;
		// アイテム作用の追加処理
		virtual void OnUpdateBehavior( S3DScene& scene ) ;
		// 当たり判定追加
		virtual void RenderLocalCollision
			( const S3DScene& scene, S3DCollision& render ) ;
		// 表示モデル追加
		virtual void RenderLocalModel
			( const S3DScene& scene,
				S3DRenderContextInterface& render,
				uint64_t flagsExclusion = 0 ) ;

	public:
		// ポーズ設定
		virtual void UpdateModelPose( S3DModelBuffer& model ) ;
	protected:
		// ボーンアニメーションするか？
		virtual bool DoesNeedPhysicsBoneAnimation( S3DModelBuffer& model ) ;
		// アニメーション・トラック処理
		virtual void OnPoseAnimationTrack( S3DModelBuffer& model ) ;
		// ボーンアニメーション実行
		virtual void OnPhysicsBoneAnimation( S3DModelBuffer& model ) ;
	public:
		// アイテム複製
		void DuplicateOf( const S3DDynamicModelItem& item ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ポーズアニメーション
	//////////////////////////////////////////////////////////////////////////

	class	S3DModelPoseAnimator
				: public S3DDynamicModelItem::PoseAnimator
	{
	protected:
		SSystem::SString	m_strPoseID ;
		S3DModelPose *		m_pPose ;
		bool				m_flagPlaying ;
		bool				m_flagLoop ;
		double				m_fpAnimationSpeed ;
		double				m_secAnimation ;
		double				m_secEndOfAnimation ;
		double				m_secLoopStart ;
		double				m_secLoopEnd ;

		PoseAnimator *		m_pTransitionPrev ;
		double				m_secTransTimer ;
		double				m_secTransDuration ;
		SGLBezierCurves<double>
							m_bzTransTime ;

		PoseAnimator *		m_pPostAnimator ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DModelPoseAnimator, PoseAnimator )
		// 構築関数
		S3DModelPoseAnimator( void ) ;
		S3DModelPoseAnimator( const S3DModelPoseAnimator& pose ) ;
		// 消滅関数
		virtual ~S3DModelPoseAnimator( void ) ;

	public:
		// ポーズ設定（デフォルト区間設定）
		void SetPose
			( S3DModelPose * pPose,
				bool fVolatile = false,
				const wchar_t * pwszID = NULL ) ;
		// ポーズ区間設定
		void SetDuration( double secStart, double secEnd ) ;
		// ループ設定
		void SetLoop
			( bool fLoop, double secStart = 0.0, double secEnd = -1.0 ) ;
		// アニメーション速度設定
		void SetAnimationSpeed( double fpSpeed ) ;
		// アニメーション中か？
		bool IsAnimationPlaying( void ) const ;
		// トランジッションを設定する
		void SetTransition
			( S3DModelPose * pBasePose,
				double secDuration, double v0 = 0.0, double v1 = 0.0 ) ;
		void SetTransition
			( PoseAnimator * pBasePose,
				double secDuration, double v0 = 0.0, double v1 = 0.0 ) ;
		// 次のアニメーションを設定する
		void SetPostAnimator( PoseAnimator * pAnimator ) ;
		// ポーズ取得
		S3DModelPose * GetPose( void ) const ;
		// ポーズ識別子取得
		const SSystem::SString& GetPoseID( void ) const ;
		// 次のアニメーションを取得する
		PoseAnimator * GetPostAnimation( void ) const ;

	public:
		// 時間経過 (終了時に true 返却)
		virtual bool OnTimer
			( S3DDynamicModelItem& item, uint32_t msecPast ) ;
		// ポーズ設定
		virtual void ApplyPose( S3DModelBuffer& model ) ;
		// 即時完了
		virtual void FlushAnimation( S3DDynamicModelItem& item ) ;
		// キャンセル（アニメーションの停止）
		virtual void CancelAnimation( S3DDynamicModelItem& item ) ;
		// 複製
		virtual PoseAnimator * Duplicate( void ) ;
		// 保存
		virtual SGLError SavePoseAnimator( SSystem::SFileInterface& file ) ;
		// 復元
		virtual SGLError LoadPoseAnimator
			( const S3DModelPoseLibrary& libPose, SSystem::SFileInterface& file ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 簡単な三角ポリゴンメッシュ・リダクション
	//////////////////////////////////////////////////////////////////////////

	class	S3DSimpleMeshReduction	: public ESLObject
	{
	protected:
		struct	EdgeInfo
		{
			size_t		iVertex0 ;		// 頂点１指標
			size_t		iVertex1 ;		// 頂点２指標
			size_t		iTriangle0 ;	// 辺を含む代表三角指標
			size_t		nShared ;		// 辺を共有している三角数
			size_t		nTrianglesBuf ;	// pTriangles のバッファサイズ
			size_t *	pTriangles ;	// 辺を含む三角指標配列
			float32_t	bend ;			// 辺の曲がり具合 cosθ
		} ;
		struct	VertexInfo
		{
			bool		fixed ;		// 固定頂点
			bool		collapsed ;
			bool		removed ;
			float32_t	bend ;		// 頂点を含む辺の曲がり具合 Σcosθ
			size_t		nEdges ;	// 頂点を含む辺の数
			size_t		nEdgesBuf ;
			EdgeInfo **	ppEdges ;	// 頂点を含む辺配列
		} ;
		struct	TriangleInfo
		{
			bool		collapsed ;		// 無効三角か？
			S3DVector	vNormal ;		// 面の法線
			uint32_t	iVertex[3] ;	// 頂点指標
			EdgeInfo *	pEdges[3] ;		// 辺情報 { v0-v1, v1-v2, v0-v2 }
		} ;

		S3DRenderBuffer::MeshBuffer *		m_pMesh ;		// 元メッシュ
		SSystem::SArray<size_t>				m_sorted ;		// ソート頂点指標
		double								m_meanBend ;	// bend の平均
		double								m_varBend ;		// bend の分散^2
		SSystem::SArray<VertexInfo>			m_vertices ;	// 頂点情報
		SSystem::SArray<TriangleInfo>		m_triangles ;	// 三角情報
		SSystem::SArray<uint32_t>			m_reduced ;		// 削減済み三角リスト
		SSystem::SStackBuffer				m_stackBuf ;	// バッファ

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DSimpleMeshReduction, ESLObject )
		// 構築関数
		S3DSimpleMeshReduction( void ) ;
		// 消滅関数
		virtual ~S3DSimpleMeshReduction( void ) ;

	public:
		// 変換元メッシュ・セットアップ
		SGLError SetupTargetMesh( S3DRenderBuffer::MeshBuffer * pMesh ) ;
		// バッファ開放
		void Release( void ) ;

	public:
		// 削減処理
		size_t ReduceTriangles( double threshold ) ;
		// ReduceTriangles 後に再度 ReduceTriangles するための処理
		void RecycleMesh( void ) ;
		// 削減後インデックス数取得
		size_t GetReducedIndexCount( void ) const ;
		// 削減後インデックス配列取得
		const uint32_t * GetReducedIndexList( void ) const ;

	protected:
		// ポリゴンの辺を検索
		EdgeInfo * FindEdge( size_t iVertex0, size_t iVertex1 ) const ;
		// ポリゴンの辺を追加
		EdgeInfo * AddEdgeOfTriangle( size_t iVertex0, size_t iVertex1, size_t iTriangle ) ;
		// 頂点に繋がる辺情報追加
		void AddEdgeOfVertex( VertexInfo& vi, EdgeInfo * pEdge ) ;
		// 頂点に繋がる辺情報削除
		void RemoveEdgeOfVertex( VertexInfo& vi, EdgeInfo * pEdge ) ;
		// 辺を含む三角をリストに追加
		void AddTriangleOfEdge( EdgeInfo& edge, size_t iTriangle ) ;
		// 三角の特定の辺を縮退して残りの辺を結合処理
		void CollapseAndMergeEdge
			( TriangleInfo& ti, size_t iTriangle,
				EdgeInfo * pColEdge, size_t iVertex0, size_t iMoveTo ) ;
		// 頂点を移動したときの辺周りの面の角度変化が最大のものを調べる
		float32_t MaximumAngleOfEdgeDelta( size_t iVertex0, size_t iMoveTo ) ;
		// 面の法線を計算する
		S3DVector CalcNormalOfTriangle( size_t vi0, size_t vi1, size_t vi2 ) const ;
		// 頂点を移動して頂点を含むすべての三角を再構築する
		void MoveVertex( size_t iVertex0, size_t iMoveTo ) ;
		// 頂点をソートする
		void SortVertices( float32_t bendThreshold ) ;
		// 三角リストを構築
		void BuildTriangleList( void ) ;

	} ;

}

#endif

