
#if	!defined(__SAKURAGLX3D_SCENE_RSRC_PROC_H__)
#define	__SAKURAGLX3D_SCENE_RSRC_PROC_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// テクスチャ・アトラス化
	//////////////////////////////////////////////////////////////////////////

	class	S3DResourceAtlasTextureProc
				: public S3DSceneComposer::ImageRsrcProcedure,
					public S3DSceneComposer::Controller
	{
	public:
		enum	ParameterIndex
		{
			paramWidth,
			paramHeight,
			paramSizePOT,
			paramClampBorder,
			paramRefCount,
			paramRefTexture1,
		} ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2
			( S3DResourceAtlasTextureProc, ImageRsrcProcedure, Controller )
		S3D_DECLARE_COMPOSER_ITEM( S3DResourceAtlasTextureProc, atlas_texture )
		// 構築関数
		S3DResourceAtlasTextureProc( void ) ;
		// 消滅関数
		virtual ~S3DResourceAtlasTextureProc( void ) ;

	protected:
		S3DSceneComposer *		m_pComposer ;
		SGLSize					m_sizeInit ;
		bool					m_flagSizePOT ;
		bool					m_flagClampBorder ;
		size_t					m_nRefCount ;
		SSystem::SObjectArray
			<SSystem::SString>	m_aPropImageID ;
		SSystem::SObjectArray
			<SSystem::SString>	m_aPropImageName ;
		SSystem::SObjectArray
			<SSystem::SString>	m_aRefImages ;

	public:
		// 参照画像プロパティエントリ設定
		void UpdateParameterEntry( void ) ;
		// 参照画像収集
		SGLError CollectReferenceImages
			( S3DSceneComposer::ResourceAssets& assets,
				SSystem::SPointerArray<SGLImageBuffer>& aImageBufs,
				SSystem::SArray<SGLSize>& aImageSizes,
				SSystem::SPointerArray<SGLImageBuffer>& aSubImages ) ;

	public:	// Controller
		// パラメータ値取得
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// ResourceProcedure
		// S3DSceneComposer 関連付け
		virtual void AttachSceneComposer( S3DSceneComposer * pComposer ) ;
		// デシリアライズ
		virtual SGLError ParseParameter( const SSystem::SXMLDocument& xmlProc ) ;
		// シリアライズ
		virtual SGLError FormatParameter( SSystem::SXMLDocument& xmlProc ) ;
		// リソース生成
		virtual SGLError CreateResource
			( ESLObject*& pRsrc, S3DSceneComposer::ResourceAssets& assets ) ;
		// 参照元画像一覧
		virtual SGLError CollectReferenceSubImages
			( S3DSceneComposer::ResourceAssets& assets,
				SSystem::SPointerArray<SGLImageBuffer>& aSubImages ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ３次元テクスチャ化
	//////////////////////////////////////////////////////////////////////////

	class	S3DRsrc3DTextureBuilderProc
				: public S3DSceneComposer::ImageRsrcProcedure,
					public S3DSceneComposer::Controller
	{
	public:
		enum	ParameterIndex
		{
			paramTextureArray,
			paramMakeMipmap,
			paramCompressed,
			paramRefCount,
			paramRefTexture1,
		} ;

	protected:
		S3DSceneComposer *		m_pComposer ;
		bool					m_flagTextureArray ;
		bool					m_flagMipmap ;
		bool					m_flagCompressed ;
		size_t					m_nRefCount ;
		SSystem::SObjectArray
			<SSystem::SString>	m_aPropImageID ;
		SSystem::SObjectArray
			<SSystem::SString>	m_aPropImageName ;
		SSystem::SObjectArray
			<SSystem::SString>	m_aRefImages ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2
			( S3DRsrc3DTextureBuilderProc, ImageRsrcProcedure, Controller )
		S3D_DECLARE_COMPOSER_ITEM( S3DRsrc3DTextureBuilderProc, texture3d_builder )
		// 構築関数
		S3DRsrc3DTextureBuilderProc( void ) ;
		// 消滅関数
		virtual ~S3DRsrc3DTextureBuilderProc( void ) ;

	public:
		// 参照画像プロパティエントリ設定
		void UpdateParameterEntry( void ) ;
		// 参照画像収集
		SGLError CollectReferenceImages
			( S3DSceneComposer::ResourceAssets& assets,
				SGLSize& sizeMaxImage,
				SSystem::SPointerArray<SGLImageBuffer>& aImageBufs,
				SSystem::SPointerArray<SGLImageBuffer>& aSubImages ) ;

	public:	// Controller
		// パラメータ値取得
		virtual int32_t GetIntegerParameter( size_t iParam ) const ;
		virtual bool GetBooleanParameter( size_t iParam ) const ;
		virtual const wchar_t * GetCommandParameter( size_t iParam ) const ;
		// パラメータ値設定
		virtual void SetIntegerParameter( size_t iParam, int32_t n ) ;
		virtual void SetBooleanParameter( size_t iParam, bool b ) ;
		virtual void SetCommandParameter( size_t iParam, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// ResourceProcedure
		// S3DSceneComposer 関連付け
		virtual void AttachSceneComposer( S3DSceneComposer * pComposer ) ;
		// デシリアライズ
		virtual SGLError ParseParameter( const SSystem::SXMLDocument& xmlProc ) ;
		// シリアライズ
		virtual SGLError FormatParameter( SSystem::SXMLDocument& xmlProc ) ;
		// リソース生成
		virtual SGLError CreateResource
			( ESLObject*& pRsrc, S3DSceneComposer::ResourceAssets& assets ) ;
		// 参照元画像一覧
		virtual SGLError CollectReferenceSubImages
			( S3DSceneComposer::ResourceAssets& assets,
				SSystem::SPointerArray<SGLImageBuffer>& aSubImages ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 反復画像
	//////////////////////////////////////////////////////////////////////////

	class	S3DRsrcImageRepeaterProc
				: public S3DSceneComposer::ImageRsrcProcedure,
					public S3DSceneComposer::Controller
	{
	public:
		enum	ParameterIndex
		{
			paramImageID,
			paramXRepeat,
			paramYRepeat,
		} ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2
			( S3DRsrcImageRepeaterProc, ImageRsrcProcedure, Controller )
		S3D_DECLARE_COMPOSER_ITEM( S3DRsrcImageRepeaterProc, image_rep )
		// 構築関数
		S3DRsrcImageRepeaterProc( void ) ;
		// 消滅関数
		virtual ~S3DRsrcImageRepeaterProc( void ) ;

	protected:
		S3DSceneComposer *		m_pComposer ;
		SSystem::SString		m_strImageID ;
		SGLSize					m_sizeRepeat ;

	public:	// Controller
		// パラメータ値取得
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// ResourceProcedure
		// S3DSceneComposer 関連付け
		virtual void AttachSceneComposer( S3DSceneComposer * pComposer ) ;
		// デシリアライズ
		virtual SGLError ParseParameter( const SSystem::SXMLDocument& xmlProc ) ;
		// シリアライズ
		virtual SGLError FormatParameter( SSystem::SXMLDocument& xmlProc ) ;
		// リソース生成
		virtual SGLError CreateResource
			( ESLObject*& pRsrc, S3DSceneComposer::ResourceAssets& assets ) ;
		// 参照元画像一覧
		virtual SGLError CollectReferenceSubImages
			( S3DSceneComposer::ResourceAssets& assets,
				SSystem::SPointerArray<SGLImageBuffer>& aSubImages ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// パーリンノイズ画像
	//////////////////////////////////////////////////////////////////////////

	class	S3DRsrcImagePerlinProc
				: public S3DSceneComposer::ImageRsrcProcedure,
					public S3DSceneComposer::Controller
	{
	public:
		enum	ParameterIndex
		{
			paramWidth,
			paramHeight,
			paramDepth,
			paramRandomSeed,
			paramLowLevel,
			paramHighLevel,
			paramColorMap0,
			paramAlphaFill0,
			paramColorMap1,
			paramColorPos1,
			paramColorMap2,
			paramColorPos2,
			paramColorMap3,
			paramAlphaFill3,
		} ;
		struct	GenerateParameter
		{
			uint32_t	nRandomSeed ;
			double		fpLowCutOff ;
			double		fpHighSaturation ;
			SGLPalette	argbGradation[4] ;
			float32_t	fpGradationPos[2] ;
		} ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2
			( S3DRsrcImagePerlinProc, ImageRsrcProcedure, Controller )
		S3D_DECLARE_COMPOSER_ITEM( S3DRsrcImageRepeaterProc, perlin_image )
		// 構築関数
		S3DRsrcImagePerlinProc( void ) ;
		// 消滅関数
		virtual ~S3DRsrcImagePerlinProc( void ) ;

	protected:
		S3DSceneComposer *	m_pComposer ;
		SGLSize				m_sizeImage ;
		int32_t				m_nDepth ;
		double				m_fpAlphaFill[2] ;
		GenerateParameter	m_genParam ;

	public:	// Controller
		// パラメータ値取得
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;

	public:	// ResourceProcedure
		// S3DSceneComposer 関連付け
		virtual void AttachSceneComposer( S3DSceneComposer * pComposer ) ;
		// デシリアライズ
		virtual SGLError ParseParameter( const SSystem::SXMLDocument& xmlProc ) ;
		// シリアライズ
		virtual SGLError FormatParameter( SSystem::SXMLDocument& xmlProc ) ;
		// リソース生成
		virtual SGLError CreateResource
			( ESLObject*& pRsrc, S3DSceneComposer::ResourceAssets& assets ) ;
		// 参照元画像一覧
		virtual SGLError CollectReferenceSubImages
			( S3DSceneComposer::ResourceAssets& assets,
				SSystem::SPointerArray<SGLImageBuffer>& aSubImages ) ;

	public:
		// 画像生成
		static void GeneratePerlin
			( SGLImageObject& image, const GenerateParameter& gp ) ;
		// パーリン関数
		struct	PerlinParam
		{
			int			xRepeat ;
			int			yRepeat ;
			int			zRepeat ;
			uint32_t *	pRandomTable ;
		} ;
		static float32_t OctavePerlin
			( const PerlinParam& pp, float32_t x, float32_t y, float32_t z ) ;
		static float32_t Perlin
			( const PerlinParam& pp, float32_t x, float32_t y, float32_t z ) ;
		static float32_t Grad
			( uint32_t hash, float32_t x, float32_t y, float32_t z ) ;
		static float32_t Fade( float32_t x ) ;
		static float32_t Lerp
			( float32_t x, float32_t y, float32_t t ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 法線画像の符号反転（チャネル毎の輝度反転）
	//////////////////////////////////////////////////////////////////////////

	class	S3DRsrcImageNormalInverseProc
				: public S3DSceneComposer::ImageRsrcProcedure,
					public S3DSceneComposer::Controller
	{
	public:
		enum	ParameterIndex
		{
			paramImageID,
			paramMakeMipmap,
			paramCompressed,
			paramChannel0,
			paramChannel1,
			paramChannel2,
			paramChannel3,
			countChannel	= 4,
		} ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2
			( S3DRsrcImageNormalInverseProc, ImageRsrcProcedure, Controller )
		S3D_DECLARE_COMPOSER_ITEM( S3DRsrcImageNormalInverseProc, normal_inverser )
		// 構築関数
		S3DRsrcImageNormalInverseProc( void ) ;
		// 消滅関数
		virtual ~S3DRsrcImageNormalInverseProc( void ) ;

	protected:
		S3DSceneComposer *		m_pComposer ;
		SSystem::SString		m_strImageID ;
		bool					m_flagMipmap ;
		bool					m_flagCompressed ;
		bool					m_flagInverse[countChannel] ;

	public:	// Controller
		// パラメータ値取得
		virtual bool GetBooleanParameter( size_t iParam ) const ;
		virtual const wchar_t * GetCommandParameter( size_t iParam ) const ;
		// パラメータ値設定
		virtual void SetBooleanParameter( size_t iParam, bool b ) ;
		virtual void SetCommandParameter( size_t iParam, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// ResourceProcedure
		// S3DSceneComposer 関連付け
		virtual void AttachSceneComposer( S3DSceneComposer * pComposer ) ;
		// デシリアライズ
		virtual SGLError ParseParameter( const SSystem::SXMLDocument& xmlProc ) ;
		// シリアライズ
		virtual SGLError FormatParameter( SSystem::SXMLDocument& xmlProc ) ;
		// リソース生成
		virtual SGLError CreateResource
			( ESLObject*& pRsrc, S3DSceneComposer::ResourceAssets& assets ) ;
		// 参照元画像一覧
		virtual SGLError CollectReferenceSubImages
			( S3DSceneComposer::ResourceAssets& assets,
				SSystem::SPointerArray<SGLImageBuffer>& aSubImages ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 高度画像（輝度）→ 法線画像
	//////////////////////////////////////////////////////////////////////////

	class	S3DRsrcImageBumpNormalProc
				: public S3DSceneComposer::ImageRsrcProcedure,
					public S3DSceneComposer::Controller
	{
	public:
		enum	ParameterIndex
		{
			paramImageID,
			paramMakeMipmap,
			paramCompressed,
			paramHeight,
		} ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2
			( S3DRsrcImageBumpNormalProc, ImageRsrcProcedure, Controller )
		S3D_DECLARE_COMPOSER_ITEM( S3DRsrcImageBumpNormalProc, bump_normal )
		// 構築関数
		S3DRsrcImageBumpNormalProc( void ) ;
		// 消滅関数
		virtual ~S3DRsrcImageBumpNormalProc( void ) ;

	protected:
		S3DSceneComposer *		m_pComposer ;
		SSystem::SString		m_strImageID ;
		double					m_fpHeight ;
		bool					m_flagMipmap ;
		bool					m_flagCompressed ;

	public:	// Controller
		// パラメータ値取得
		virtual double GetScalarParameter( size_t iParam ) const ;
		virtual bool GetBooleanParameter( size_t iParam ) const ;
		virtual const wchar_t * GetCommandParameter( size_t iParam ) const ;
		// パラメータ値設定
		virtual void SetScalarParameter( size_t iParam, double s ) ;
		virtual void SetBooleanParameter( size_t iParam, bool b ) ;
		virtual void SetCommandParameter( size_t iParam, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// ResourceProcedure
		// S3DSceneComposer 関連付け
		virtual void AttachSceneComposer( S3DSceneComposer * pComposer ) ;
		// デシリアライズ
		virtual SGLError ParseParameter( const SSystem::SXMLDocument& xmlProc ) ;
		// シリアライズ
		virtual SGLError FormatParameter( SSystem::SXMLDocument& xmlProc ) ;
		// リソース生成
		virtual SGLError CreateResource
			( ESLObject*& pRsrc, S3DSceneComposer::ResourceAssets& assets ) ;
		// 参照元画像一覧
		virtual SGLError CollectReferenceSubImages
			( S3DSceneComposer::ResourceAssets& assets,
				SSystem::SPointerArray<SGLImageBuffer>& aSubImages ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// コンポジション・モデル・インスタンス化
	//////////////////////////////////////////////////////////////////////////

	class	S3DResourceCompositionBakerProc
				: public S3DSceneComposer::ResourceProcedure,
					public S3DSceneComposer::Controller
	{
	public:
		enum	ParameterIndex
		{
			paramRefComposition,
			paramBaseRotation,
			paramBaseZoom,
			paramBaseCenter,
			paramAllItems,
			paramAllSpaces,
			paramOptimizeBone,
			paramWithoutBone,
			paramMergeByMaterialID,
			paramMergeMeshs,
			paramMergeByMeshID,
			paramMergeMaterials,
			paramAnimationTrack,
		} ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2
			( S3DResourceCompositionBakerProc, ResourceProcedure, Controller )
		S3D_DECLARE_COMPOSER_ITEM( S3DResourceCompositionBakerProc, comp_baker )
		// 構築関数
		S3DResourceCompositionBakerProc( void ) ;
		// 消滅関数
		virtual ~S3DResourceCompositionBakerProc( void ) ;

	protected:
		S3DSceneComposer *	m_pComposer ;
		SSystem::SString	m_strCompositionID ;
		S3DDMatrix			m_matBaseRotation ;
		S3DDVector			m_vBaseZoom ;
		S3DDVector			m_vBaseCenter ;
		size_t				m_nCreateCount ;
		bool				m_flagAllItems ;
		bool				m_flagAllSpaces ;
		bool				m_flagOptimizeBone ;
		bool				m_flagWithoutBone ;
		bool				m_flagMergeMaterialByName ;
		bool				m_flagMergeMeshs ;
		bool				m_flagMergeByMeshID ;
		bool				m_flagMergeMaterials ;
		bool				m_flagAnimaionTrack ;

	public:	// Controller
		// パラメータ値取得
		virtual S3DDMatrix GetMatrixParameter( size_t iParam ) const ;
		virtual S3DDVector GetVectorParameter( size_t iParam ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetMatrixParameter( size_t iParam, const S3DDMatrix& mat ) ;
		virtual void SetVectorParameter( size_t iParam, const S3DDVector& vec ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// ResourceProcedure
		// S3DSceneComposer 関連付け
		virtual void AttachSceneComposer( S3DSceneComposer * pComposer ) ;
		// デシリアライズ
		virtual SGLError ParseParameter( const SSystem::SXMLDocument& xmlProc ) ;
		// シリアライズ
		virtual SGLError FormatParameter( SSystem::SXMLDocument& xmlProc ) ;
		// リソース生成
		virtual SGLError CreateResource
			( ESLObject*& pRsrc, S3DSceneComposer::ResourceAssets& assets ) ;
	} ;


}

#endif
