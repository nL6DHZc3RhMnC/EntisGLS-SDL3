
#if	!defined(__SAKURAGL_RENDER_CONTEXT_H__)
#define	__SAKURAGL_RENDER_CONTEXT_H__

#include <sakuragl/sgl3d/sgl_render_device.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// テクスチャライブラリ・インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	S3DTextureLibraryReferencer
	{
	public:
		// 画像取得
		virtual SGLImageObject * GetTextureAs
			( const wchar_t * pwszID, bool flagNoRefOther = false ) const = 0 ;
		// 画像検索
		virtual ssize_t FindTexturePtr( SGLImageObject * pImage ) const = 0 ;
		// 登録数取得
		virtual size_t GetTextureCount( void ) const = 0 ;
		// 登録名取得
		virtual const wchar_t * GetTextureIdentityAt( size_t i ) const = 0 ;
		// 表面属性取得
		virtual SGLImageObject * GetTextureAt( size_t i ) const = 0 ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 表面属性
	//////////////////////////////////////////////////////////////////////////

	struct	S3DSurfaceAttribute
	{
		uint64_t	flagsShading ;			// シェーディング・フラグ enum S3DShadingFlags
		S3DColor	colorBase ;				// 基本色
		S3DColor	colorShade ;			// 影色
		int32_t		nAmbient ;				// 環境光 (x256)
		int32_t		nDiffusion ;			// 拡散反射強度 (x256)
		int32_t		nSpecular ;				// 鏡面反射強度 (x256)
		int32_t		nSpecularSize ;			// 鏡面反射の鋭さ (x256)
		int32_t		nTransparency ;			// 透明度 (x256)
		int32_t		nDeepness ;				// 透明深度係数 (x256)
		int32_t		nDeepnessPower ;		// 透明深度指数 (x256)
		uint32_t	nReflection ;			// 反射率 (x256)
		float32_t	fpRefraction ;			// 屈折率 (-1.0)（0.0 の時屈折率 1.0）
		uint32_t	nEmission ;				// 発光度 (x256)
		uint32_t	nExFlags ;				// 拡張フラグ enum S3DShadingExtensionFlag
		float32_t	cosShadeThreshold ;		// 陰角度余弦 (default = 0.0)
		float32_t	fpToonShadeThreshold ;	// トゥーンシェード中間色閾値 (default = 0.5)
		float32_t	fpToonShadeBrightness ;	// トゥーンシェード中間色輝度 (default = 0.5)
		SGLPalette	rgbSpecularColor ;		// 鏡面反射色
		SGLPalette	rgbBorderColor ;		// 輪郭線色
		float32_t	fpBorderThicknessA ;	// 輪郭線太さ係数
		float32_t	fpBorderThicknessB ;
		float32_t	fpBackLight ;			// バックライト輝度
		S3DColor	colorBackLight ;		// バックライト色
		float32_t	fpRimLight ;			// リムライト輝度
		float32_t	fpRimLightDeepness ;	// リムライトサイズ
		SGLPalette	rgbRimLightColor ;		// リムライト色
		int32_t		nBackDiffusion ;		// 裏拡散反射（疑似AO）

		// 構築関数
		#if	!defined(__COTOPHA__)
		S3DSurfaceAttribute( void )
			: flagsShading(0),
				colorBase( 0x00FFFFFF, 0 ), colorShade( 0, 0 ),
				nAmbient(0), nDiffusion(0), nSpecular(0), nSpecularSize(0),
				nTransparency(0), nDeepness(0), nDeepnessPower(0x100),
				nReflection(0), fpRefraction(0), nEmission(0),
				nExFlags(0), cosShadeThreshold(0.0f),
				fpToonShadeThreshold(0.5f), fpToonShadeBrightness(0.5f),
				nBackDiffusion(0), rgbSpecularColor(0x00FFFFFF),
				rgbBorderColor(0),
				fpBorderThicknessA(1.0f), fpBorderThicknessB(0.0f),
				fpBackLight(0.0f), colorBackLight( 0x00FFFFFF, 0 ),
				fpRimLight(0.0f), fpRimLightDeepness(0.1f),
				rgbRimLightColor(0x00FFFFFF) {}
		#endif
		S3DSurfaceAttribute
			( uint64_t flags,
				uint32_t rgbMulBase, uint32_t rgbAddBase,
				uint32_t rgbMulShade, uint32_t rgbAddShade,
				int32_t amb, int32_t diff, int32_t spec, int32_t specSize,
				int32_t trans, int32_t deep, int32_t refl, float32_t refr,
				int32_t emission = 0 )
			: flagsShading(flags),
				colorBase( rgbMulBase, rgbAddBase ),
				colorShade( rgbMulShade, rgbAddShade ),
				nAmbient(amb), nDiffusion(diff),
				nSpecular(spec), nSpecularSize(specSize),
				nTransparency(trans), nDeepness(deep), nDeepnessPower(0x100),
				nReflection(refl), fpRefraction(refr), nEmission(emission),
				nExFlags(0), cosShadeThreshold(0.0f),
				fpToonShadeThreshold(0.5f), fpToonShadeBrightness(0.5f),
				nBackDiffusion(0), rgbSpecularColor(0x00FFFFFF),
				rgbBorderColor(0),
				fpBorderThicknessA(1.0f), fpBorderThicknessB(0.0f),
				fpBackLight(0.0f), colorBackLight( 0x00FFFFFF, 0 ),
				fpRimLight(0.0f), fpRimLightDeepness(0.1f),
				rgbRimLightColor(0x00FFFFFF) {}
		// XML デシリアライズ
		SGLError ParseXML( SSystem::SXMLDocument& xmlAttr ) ;
		// XML シリアライズ
		SGLError FormatXML( SSystem::SXMLDocument& xmlAttr ) const ;
	} ;

	enum	S3DShadingFlags
	{
		// シェーディング方式
		shadingMethodNothing			= 0x00000000,		// シェーディング無し
		shadingMethodFlat				= 0x00000001,		// フラットシェーディング（未使用）
		shadingMethodGouraud			= 0x00000002,		// グーローシェーディング
		shadingMethodPhong				= 0x00000004,		// フォンシェーディング
		shadingMethodPhongBeforeTexture	= 0x00000006,
		shadingMethodToon				= 0x00000008,		// トゥーン・シェーディング
		shadingMethodRayShadowing		= 0x00000010,		// 影を落とす
		shadingMethodRayReflecting		= 0x00000020,		// 反射を有効にする
		shadingMethodRayRefracting		= 0x00000040,		// 屈折を有効にする
		shadingMethodRayTracing			= 0x00000074,		// レイトレーシング
		shadingMethodRayTracingBeforeTexture	= 0x00000076,
		shadingMethodOpenGL				= 0x00000080,		// シェーディングに OpenGL を使用する (EntisGLS4 以降不使用)
		shadingMethodDirectX			= 0x00000080,		// シェーディングに DirectX を使用する (EntisGLS4 以降不使用)
		shadingMethodHardware			= 0x00000080,		// シェーディングにハードウェアを使用する (EntisGLS4 以降不使用)
		shadingMethodMask				= 0x000000FF,
		#if	defined(__COTOPHA__)
		shadingMethodRayTracingByPixel	= 0x0000000100000000,	// シェーディングだけでなくピクセル単位のレイトレーシング
		#endif
		// その他のフラグ
		shadingTextureTiling			= 0x00000100,		// テクスチャをタイリング
		shadingTextureTriming			= 0x00000200,		// αトリミング
		shadingTextureSmoothing			= 0x00000400,		// テクスチャ補完拡大
		shadingTextureDithering			= 0x00000800,		// αディザリング
		shadingTextureMapping			= 0x00001000,		// テクスチャマッピング
		shadingEnvironmentMapping		= 0x00002000,		// 環境マッピング
		shadingGEnvironmentMapping		= 0x00004000,		// グローバル環境マッピング
		shadingNormalTexture			= 0x00008000,		// 法線テクスチャ
		shadingLuminousTexture			= 0x00400000,		// 発光テクスチャ
		shadingAlphaTexture				= 0x00800000,		// αテクスチャ
		shadingHeightTexture			= 0x40000000,		// 標高テクスチャ
		#if	defined(__COTOPHA__)
		shadingRefractEnvMapping		= 0x0000004000000000,// 屈折反映用大域環境マッピング有効
		shadingSpecularMapping			= 0x0000008000000000,// スペキュラー・粗さ・反射率テクスチャ
		shadingNormalizedUVScale		= 0x0000010000000000,// UV 座標は [0,1] （無指定はピクセル単位）
		shadingGlobalAOLightMap			= 0x0000100000000000,// 大域AO（ライトマップ）テクスチャ
		shadingAllLocalTextureMask		= shadingTextureMapping
											| shadingEnvironmentMapping
											| shadingNormalTexture
											| shadingLuminousTexture
											| shadingAlphaTexture
											| shadingHeightTexture
											| shadingSpecularMapping
											| shadingGlobalAOLightMap,
		#endif
		shadingSingleSidePlane			= 0x00010000,		// 片面ポリゴン
		shadingNoZBuffer				= 0x00020000,		// ｚ比較を行わないで描画
		shadingZBufferNoWrite			= 0x00040000,		// ｚ比較のみ（ｚバッファへ書き込まない）
		shadingDrawOffsetBorder			= 0x00080000,		// 輪郭描画
		#if	defined(__COTOPHA__)
		shadingMeshSurfaceOffset		= 0x0000000400000000,	// 法線に従って膨張処理を行う（膨張距離は輪郭パラメータに準拠）
		#endif
		shadingHintOfUnknown			= 0x00000000,		// 描画域の透明・不透明は不明
		shadingHintOfFullAlpha			= 0x00100000,		// 描画域のほとんどが不透明
		shadingHintOfAlpha				= 0x00200000,		// 描画域の多くが 100% 透明と不透明
		shadingHintOfHalfAlpha			= 0x00300000,		// 描画域に多くの半透明（※ｚソート用ヒント）
		shadingHintPriority0			= 0x00000000,
		shadingHintPriority1			= 0x00100000,
		shadingHintPriority2			= 0x00200000,
		shadingHintPriority3			= 0x00300000,
		shadingHintMask					= 0x00300000,
		shadingHintShifter				= 20,
		#if	defined(__COTOPHA__)
		shadingHintNoZSort				= 0x0000000200000000,	// ｚソートは必要としない（※ソート用ヒント：同マテリアルをひとまとめにしてよい）
		#endif
		shadingNoShadowObject			= 0x01000000,		// 他のオブジェクトに影を落とさない
		shadingNoDropShadow				= 0x02000000,		// このオブジェクトに影を落とさない
		shadingNoReflectObject			= 0x04000000,		// 他のオブジェクトに映りこまない
		shadingGlobalReflectObject		= 0x08000000,		// 距離に関係なく他のオブジェクトに映りこむ
		shadingNoFogEffect				= 0x10000000,		// フォグ光源の影響を受けない
		shadingVertexAlpha				= 0x20000000,		// 頂点色の乗算成分の Alpha を頂点αとして使用する
		#if	defined(__COTOPHA__)
		shadingMakeBlendAdd				= 0x0000000800000000,	// 強制加算描画
		shadingEmisiveTarget			= 0x0000001000000000,	// 発光効果用出力
		shadingNoDrawOffsetBorder		= 0x0000002000000000,	// shadingDrawOffsetBorder の無効化
		shadingDisableColorEffect		= 0x0000020000000000,	// 色効果無効化 （AppendMatrixTransformation の色要素）
		shadingAppExtension1			= 0x0001000000000000,	// アプリケーション拡張
		shadingAppExtension2			= 0x0002000000000000,	// アプリケーション拡張
		shadingAppExtension3			= 0x0004000000000000,	// アプリケーション拡張
		shadingAppExtension4			= 0x0008000000000000,	// アプリケーション拡張
		#endif
	} ;

	#if	!defined(__COTOPHA__)
	// enum 値が32ビットの範囲を超えるため
	static constexpr uint64_t	shadingMethodRayTracingByPixel	= 0x0000000100000000 ;
	static constexpr uint64_t	shadingHintNoZSort				= 0x0000000200000000 ;
	static constexpr uint64_t	shadingMeshSurfaceOffset		= 0x0000000400000000 ;
	static constexpr uint64_t	shadingMakeBlendAdd				= 0x0000000800000000 ;
	static constexpr uint64_t	shadingEmisiveTarget			= 0x0000001000000000 ;
	static constexpr uint64_t	shadingNoDrawOffsetBorder		= 0x0000002000000000 ;
	static constexpr uint64_t	shadingRefractEnvMapping		= 0x0000004000000000 ;
	static constexpr uint64_t	shadingSpecularMapping			= 0x0000008000000000 ;
	static constexpr uint64_t	shadingAllLocalTextureMask		= shadingTextureMapping
																	| shadingEnvironmentMapping
																	| shadingNormalTexture
																	| shadingLuminousTexture
																	| shadingAlphaTexture
																	| shadingHeightTexture
																	| 0x0000008000000000
																	| 0x0000100000000000;
	static constexpr uint64_t	shadingNormalizedUVScale		= 0x0000010000000000 ;
	static constexpr uint64_t	shadingDisableColorEffect		= 0x0000020000000000 ;
	static constexpr uint64_t	shadingGlobalAOLightMap			= 0x0000100000000000 ;
	static constexpr uint64_t	shadingAppExtension1			= 0x0001000000000000 ;
	static constexpr uint64_t	shadingAppExtension2			= 0x0002000000000000 ;
	static constexpr uint64_t	shadingAppExtension3			= 0x0004000000000000 ;
	static constexpr uint64_t	shadingAppExtension4			= 0x0008000000000000 ;
/*
	extern	const uint64_t	shadingMethodRayTracingByPixel ;
	extern	const uint64_t	shadingHintNoZSort ;
	extern	const uint64_t	shadingMeshSurfaceOffset ;
	extern	const uint64_t	shadingMakeBlendAdd ;
	extern	const uint64_t	shadingEmisiveTarget ;
	extern	const uint64_t	shadingNoDrawOffsetBorder ;
	extern	const uint64_t	shadingRefractEnvMapping ;
	extern	const uint64_t	shadingSpecularMapping ;
	extern	const uint64_t	shadingAllLocalTextureMask ;
	extern	const uint64_t	shadingNormalizedUVScale ;
	extern	const uint64_t	shadingDisableColorEffect ;
	extern	const uint64_t	shadingGlobalAOLightMap ;
	extern	const uint64_t	shadingAppExtension1 ;
	extern	const uint64_t	shadingAppExtension2 ;
	extern	const uint64_t	shadingAppExtension3 ;
	extern	const uint64_t	shadingAppExtension4 ;
*/
	#endif

	enum	S3DShadingExtensionFlag
	{
		shadingExVarietyShade	= 0x00000001,	// cosShadeThreshold, fpToonShadeThreshold, fpToonShadeBrightness 有効
		shadingExBackDiffusion	= 0x00000002,	// nBackDiffusion 有効
		shadingExSpecularColor	= 0x00000004,	// rgbSpecularColor 有効
		shadingExBorderParam	= 0x00000008,	// rgbBorderColor, fpBorderThicknessA, fpBorderThicknessB 有効
		shadingExBackLight		= 0x00000010,	// fpBackLight 有効
		shadingExRimLight		= 0x00000020,	// fpRimLight, fpRimLightPower, rgbRimLightColor 有効
	} ;


	#if	defined(__COTOPHA__)
	class	native Material
	{
	public:
		enum	TextureFlags
		{
			textureMain			= 0,
			textureSub			= 1,
			textureDiffusion	= 0,		// 拡散反射光
			textureNormal		= 2,		// 法線
			textureLuminous		= 3,		// 発光
			textureEnvironment	= 4,		// 環境マッピング
			textureAlpha		= 5,		// αチャネル
			textureHeight		= 6,		// 標高
			textureReflection	= 7,		// 反射（スペキュラ・粗さ・反射率）
			textureTypeMask		= 0xFF,
			textureMaxCount		= 4,
		} ;
		enum	CubeMapIndex
		{
			cubeNegativeX,	// 左手
			cubeNegativeY,	// 天井
			cubeNegativeZ,	// 手前裏
			cubePositiveX,	// 右手
			cubePositiveY,	// 床
			cubePositiveZ,	// 奥
		} ;
		// パラメータ取得
		native void GetSurfaceAttribute( S3DSurfaceAttribute& attr ) ;
		native void GetBackSurfaceAttribute( S3DSurfaceAttribute& attr ) ;
		native bool IsEnabledBackSurfaceAttribute( bool flagBack ) ;
		// パラメータ設定
		native void SetSurfaceAttribute( const S3DSurfaceAttribute& attr ) ;
		native void SetBackSurfaceAttribute( const S3DSurfaceAttribute& attr ) ;
		native void EnableBackSurfaceAttribute( bool flagBack ) ;
		// テクスチャ取得
		native Image * GetTexture( int iTexture = 0 ) ;
		native Image * GetBackTexture( int iTexture = 0 ) ;
		// テクスチャ設定
		native void SetTexture
			( Image * pImage, int iTexture = 0,
				uint32_t nFlags = 0,
				double nApply = 1.0, double nParam1 = 0.0 ) ;
		native void SetBackTexture
			( Image * pImage, int iTexture = 0,
				uint32_t nFlags = 0,
				double nApply = 1.0, double nParam1 = 0.0 ) ;
		// テクスチャ切替基準ｚ座標設定
		native void SetSubTextureZ( double zTexture ) ;
	} ;
	#endif

	class	S3DMaterial ;
	class	S3DMaterialBuffer	: public ESLObject
	{
	public:
		S3DMaterialBuffer *	m_ptrNext ;
		uint32_t			m_typeMaterial ;	// enum SGLImageBufferObjectType
		bool				m_flagUpdate ;
		uint64_t			m_nReserved ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DMaterialBuffer, ESLObject )
		// 構築関数
		S3DMaterialBuffer( void )
			: m_ptrNext(NULL), m_typeMaterial(0),
				m_flagUpdate(true), m_nReserved(0) {}
		// 属性更新処理
		virtual void UpdateMaterial( const S3DMaterial * pMaterial ) = 0 ;
	} ;

	#if	defined(__COTOPHA__)
	class	SGLMaterialBuffer	: public S3DMaterialBuffer
	{
	public:
		Material *	m_material ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLMaterialBuffer, S3DMaterialBuffer )
		// 構築関数
		SGLMaterialBuffer( void ) ;
		// 消滅関数
		virtual ~SGLMaterialBuffer( void ) ;
		// 属性更新処理
		virtual void UpdateMaterial( const S3DMaterial * pMaterial ) ;
	} ;
	#endif

	class	S3DMaterial	: public SSystem::SObject
	{
	public:
		enum	TextureFlags
		{
			textureMain			= 0,
			textureSub			= 1,
			textureDiffusion	= 0,		// 拡散反射光
			textureNormal		= 2,		// 法線
			textureLuminous		= 3,		// 発光
			textureEnvironment	= 4,		// 環境マッピング
			textureAlpha		= 5,		// αチャネル
			textureHeight		= 6,		// 標高
			textureSpecular		= 7,		// 反射（スペキュラ・粗さ・反射率）
			textureGlobalAO		= 8,		// 大域AO（ライトマップ）
			textureTypeMask		= 0xFF,
			textureMaxCount		= 5,
		} ;
		enum	CubeMapIndex
		{
			cubeNegativeX,	// 左手
			cubeNegativeY,	// 天井
			cubeNegativeZ,	// 手前裏
			cubePositiveX,	// 右手
			cubePositiveY,	// 床
			cubePositiveZ,	// 奥
		} ;
		S3DSurfaceAttribute	m_attrSurface ;
		SGLImageObject *	m_pTexture[textureMaxCount] ;
		SSystem::SString	m_idTexture[textureMaxCount] ;
		uint32_t			m_flagTexture[textureMaxCount] ;
		float32_t			m_fpTextureApply[textureMaxCount] ;
		float32_t			m_fpTextureParam1[textureMaxCount] ;
		float32_t			m_zTexture ;
		bool				m_flagBack ;
		S3DSurfaceAttribute	m_attrBack ;
		SGLImageObject *	m_pBackTexture[textureMaxCount] ;
		SSystem::SString	m_idBackTexture[textureMaxCount] ;
		uint32_t			m_flagBackTexture[textureMaxCount] ;
		float32_t			m_fpBackTextureApply[textureMaxCount] ;
		float32_t			m_fpBackTextureParam1[textureMaxCount] ;

		SSystem::SCriticalSection	m_csBufLock ;
		S3DMaterialBuffer *	m_pMaterialBuf ;

		// 各種テクスチャサンプリング
		struct	ColorAttribute
		{
			uint32_t		nTextureFlags ;
			SGLPalette		argbDiffusion ;
			SGLPalette		argbLuminous ;
			float32_t		fpLuminousApply ;
			SGLPalette		argbNormal ;
			SGLPalette		argbHeight ;
			float32_t		fpHeightParam ;
			SGLPalette		argbSpecular ;

			void Clear( void )
			{
				nTextureFlags = 0 ;
				argbDiffusion = 0 ;
				argbLuminous = 0 ;
				fpLuminousApply = 0 ;
				argbNormal = 0xFF8080FF ;
				argbHeight = 0 ;
				fpHeightParam = 0 ;
				argbSpecular = 0xFFFFFFFF ;
			}
		} ;

		// デフォルトマテリアル
		enum	DefaultMaterial
		{
			defaultWhite,
			defaultBlack,
			defaultTransparent,
			defaultWhiteDouble,
			defaultBlackDouble,
			defaultTransparenctDouble,
			defaultMaterialCount,
		} ;
		static S3DMaterial	m_materialDefault[defaultMaterialCount] ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DMaterial, SObject )
		// 構築関数
		S3DMaterial( void ) ;
		S3DMaterial( const S3DMaterial& material ) ;
		S3DMaterial( const S3DSurfaceAttribute& attr ) ;
		// 消滅関数
		virtual ~S3DMaterial( void ) ;
		// 複製
		void CopyFrom( const S3DMaterial& material ) ;
		// XML デシリアライズ
		SGLError ParseXML
			( const SSystem::SXMLDocument & xmlMaterial,
					const S3DTextureLibraryReferencer& libTexture ) ;
		// XML シリアライズ
		SGLError FormatXML
			( SSystem::SXMLDocument & xmlMaterial,
					const S3DTextureLibraryReferencer& libTexture ) ;
		// テクスチャタイプ
		static const wchar_t * GetTextureTypeString( uint32_t nType ) ;
		static uint32_t GetTextureTypeFlag( const wchar_t * pwszType ) ;
		// テクスチャ参照更新
		void UpdateTextureReference
			( const S3DTextureLibraryReferencer& libTexture ) ;
		// テクスチャ参照有効判定
		bool IsValidTextureReferenceFor
			( const S3DTextureLibraryReferencer& libTexture ) const ;
		// パラメータ取得
		void GetSurfaceAttribute( S3DSurfaceAttribute& attr ) const ;
		void GetBackSurfaceAttribute( S3DSurfaceAttribute& attr ) const ;
		bool IsEnabledBackSurfaceAttribute( void ) const ;
		// パラメータ設定
		void SetSurfaceAttribute( const S3DSurfaceAttribute& attr ) ;
		void SetBackSurfaceAttribute( const S3DSurfaceAttribute& attr ) ;
		void EnableBackSurfaceAttribute( bool flagBack ) ;
		// テクスチャ取得
		SGLImageObject * GetTexture( int iTexture = 0 ) const ;
		SGLImageObject * GetBackTexture( int iTexture = 0 ) const ;
		uint32_t GetTextureFlags( int iTexture = 0 ) const ;
		uint32_t GetTextureType( int iTexture = 0 ) const ;
		uint32_t GetBackTextureFlags( int iTexture = 0 ) const ;
		uint32_t GetBackTextureType( int iTexture = 0 ) const ;
		float32_t GetTextureApplication( int iTexture = 0 ) const ;
		float32_t GetBackTextureApplication( int iTexture = 0 ) const ;
		float32_t GetTextureParameter( int iTexture = 0 ) const ;
		float32_t GetBackTextureParameter( int iTexture = 0 ) const ;
		// 特定種類のテクスチャ検索
		int FindTextureTypeOf( uint32_t nType ) const ;
		int FindBackTextureTypeOf( uint32_t nType ) const ;
		// 未使用テクスチャ番号取得
		int FindEmptyTextureIndex( void ) const ;
		int FindEmptyBackTextureIndex( void ) const ;
		// 指定テクスチャ検索
		int FindTextureOf( SGLImageObject * pImage ) const ;
		int FindBackTextureOf( SGLImageObject * pImage ) const ;
		// テクスチャ設定
		void SetTexture
			( SGLImageObject * pImage, int iTexture = 0,
				uint32_t nFlags = textureDiffusion,
				float32_t nApply = 1.0f, float32_t nParam1 = 0.0f,
				const wchar_t * pwszTextureID = NULL ) ;
		void SetBackTexture
			( SGLImageObject * pImage, int iTexture = 0,
				uint32_t nFlags = textureDiffusion,
				float32_t nApply = 1.0f, float32_t nParam1 = 0.0f,
				const wchar_t * pwszTextureID = NULL ) ;
		void SetTextureApplication( int iTexture, float32_t nApply ) ;
		void SetBackTextureApplication( int iTexture, float32_t nApply ) ;
		void SetTextureParameter( int iTexture, float32_t nParam1 ) ;
		void SetBackTextureParameter( int iTexture, float32_t nParam1 ) ;
		// テクスチャ切替用の基準ｚ座標を取得（EntisGLS3互換用）
		double GetSubTextureZ( void ) const ;
		// テクスチャ切替用の基準ｚ座標を設定（EntisGLS3互換用）
		void SetSubTextureZ( double zTexture ) ;
		// マテリアル・バッファ更新設定
		void SetUpdateMaterialBuffer( void ) ;
		// マテリアル・バッファ取得 (SGLImageBufferObjectType)
		S3DMaterialBuffer * GetMaterialBuffer( uint32_t idType ) const ;
		// マテリアル・バッファ追加
		void AddMaterialBuffer( S3DMaterialBuffer * pBuf ) ;
		// 全てのマテリアル・バッファ削除
		void RemoveAllMaterialBuffer( void ) ;
		// テクスチャタイプフラグ（TextureFlags）
		// - シェーディングフラグ（S3DShadingFlags）変換
		static uint64_t ShadingFlagOfTextureType( uint32_t nType ) ;
		static TextureFlags TextureTypeOfShadingFlag( uint64_t nTextueFlag ) ;
		// アトラステクスチャへの画像参照領域の取得
		bool GetImageReferenceRect
			( SGLImageRect& rectRef, SGLSize& sizeAtlas,
				SGLImageObject * pImage, ssize_t iFrame = -1 ) const ;
		// テクスチャ・サンプリング
		bool SampleDiffusionTexture
			( SGLPalette& rgbaTexture, float32_t x, float32_t y ) const ;
		bool SampleLuminousTexture
			( SGLPalette& rgbaTexture, float32_t x, float32_t y ) const ;
		void SampleTextures
			( ColorAttribute& clrAttr, float32_t x, float32_t y ) const ;
		static bool SampleDiffusionTexture
			( SGLPalette& rgbaTexture, float32_t x, float32_t y,
				const S3DSurfaceAttribute& attr, SGLImageObject * pImage ) ;
		// Material 取得
		#if	defined(__COTOPHA__)
		Material * GetMaterialObject( void ) ;
		#else
		S3DMaterial * GetMaterialObject( void )
		{
			return	this ;
		}
		#endif
		// デフォルトマテリアル
		static S3DMaterial * GetDefaultMaterial( DefaultMaterial def )
		{
			return	&m_materialDefault[def] ;
		}
	} ;

	#if	!defined(__COTOPHA__)
	typedef	S3DMaterial	Material ;
	#endif

	enum	S3DMaterialBufferObjectType
	{
		materialObjectEntisGLS4Material		= 0x00000004,
	} ;


	//////////////////////////////////////////////////////////////////////////
	// SGLImageObject - S3DMaterial 変換 SGLImageBufferInterface
	// ※2D描画を3D描画へ変換するためのシェーディング無しの表面属性
	//////////////////////////////////////////////////////////////////////////

	class	SGLImageNoShadeMaterialInterface	: public SGLImageBufferInterface
	{
	public:
		enum	IndexShader
		{
			indexNormal,			// 通常
			indexNormalNoZ,			// ｚ比較無し
			indexNormalReadZ,		// ｚ書き込み無し
			indexNonSmooth,			// 補完無し
			indexNonSmooth_NoZ,
			indexNonSmooth_ReadZ,
			indexVertexAlpha,		// 頂点α有り
			indexVertexAlpha_NoZ,
			indexVertexAlpha_ReadZ,
			indexDrawAdd,			// 加算描画（＋頂点α）
			indexDrawAdd_NoZ,
			indexDrawAdd_ReadZ,
			indexDither,			// ディザリング描画（＋頂点α）
			indexDither_NoZ,
			indexDither_ReadZ,
			indexCount,
		} ;
		S3DMaterial *		m_pMaterial[indexCount] ;
		SGLImageObject *	m_pImage ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLImageNoShadeMaterialInterface, SGLImageBufferInterface )
		// 構築関数
		SGLImageNoShadeMaterialInterface( SGLImageObject * pImage ) ;
		// 消滅関数
		virtual ~SGLImageNoShadeMaterialInterface( void ) ;

	public:
		// 更新通知
		virtual SGLError UpdateBuffer
			( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect = NULL ) ;
		// 更新確定処理
		virtual SGLError CommitBuffer( SGLImageBuffer * pImageBuf ) ;
		// 反映処理
		virtual SGLError ReflectBuffer
			( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect = NULL ) ;
		// ミップマップ化通知
		virtual SGLError MakeMipmap( void ) ;
		// 画像バッファの再確保通知
		virtual bool OnImageReBuffered( SGLImageBuffer * pImageBuf ) ;
		// 関連オブジェクトの削除処理
		virtual bool OnDestroyObject( ESLObject * pObj ) ;

	public:
		// SGLImageObject からシェーディング無しの表面属性取得
		static S3DMaterial * GetMaterialBy
			( SGLImageObject * pImage,
				uint64_t nShadingFlags, SGLImageRect * pRefRect ) ;
		static S3DMaterial * GetMaterialOf
			( SGLImageObject * pImage, SGLImageRect * pRefRect ) ;
		static S3DMaterial * GetMaterialNoZOf
			( SGLImageObject * pImage, SGLImageRect * pRefRect ) ;
		static S3DMaterial * GetMaterialNoWriteZOf
			( SGLImageObject * pImage, SGLImageRect * pRefRect ) ;
		static SGLImageNoShadeMaterialInterface *
			GetNoShadeMaterialOf
				( SGLImageObject * pImage, SGLImageRect * pRefRect ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 光源
	//////////////////////////////////////////////////////////////////////////

	struct	S3DLightEntry
	{
		uint32_t		typeLight ;				// 光源タイプ
		SGLPalette		rgbColor ;				// 光源色
		float32_t		fpBrightness ;			// 光源輝度
		float32_t		fpAttenuationPower ;	// 点光源減衰力 (1/r^x)
		S3DVector4		vecPosition ;			// 光源位置
		S3DVector4		vecDirection ;			// 光源ベクトル
		float32_t		fpAngle ;				// 範囲角 cosθ
		float32_t		fpGradation ;			// ぼかし範囲 Δcosθ
												// ぼかし開始角＝acos(fpGradation+fpAngle)
		uint32_t		nReserved2[2] ;

		S3DLightEntry( void )
			: typeLight(0), fpBrightness(0),
				fpAttenuationPower(1.0), fpAngle(0), fpGradation(0) {}
	} ;

	enum	S3DLightType
	{
		lightTypeAmbient	= 0x00000001,		// 環境光
		lightTypeVector		= 0x00000002,		// 無限遠光源
		lightTypePoint		= 0x00000004,		// 点光源
		lightTypeSpot		= 0x00000006,		// 点光源（スポットライト）
		lightTypeFog		= 0x00000008,		// 面境界擬似フォッグ
		lightTypeAmbientMul	= 0x00000009,		// 環境光乗算
		lightTypeMask		= 0x000000FF,
		lightShadowMapping	= 0x80000000,		// シャドウマッピング利用
	} ;


	//////////////////////////////////////////////////////////////////////////
	// シャドウマップ情報
	//////////////////////////////////////////////////////////////////////////

	struct	S3DShadowMapInfo
	{
		S3DVector4	vLight ;		// 投影原点
		S3DVector4	vRay ;			// 光線ベクトル
		S3DVector4	vTarget ;		// シャドウマップ画像原点座標
		S3DVector4	vAxisX ;		// シャドウマップのｘ基底ベクトル
		S3DVector4	vAxisY ;		// シャドウマップのｙ基底ベクトル
		float32_t	fpFixErrorGap ;	// z 演算誤差固定比率（1/4096 等）
		float32_t	fpVarErrorGap ;	// 角度に応じた z 演算誤差比率（1/512 等）
		float32_t	zPersScreen ;	// 透視変換用パラメータ
		float32_t	zPersScale ;
		float32_t	zPersNear ;		// シャドウマッピングの手前ｚ
		float32_t	zPersFar ;		// シャドウマッピングの奥ｚ
		uint32_t	nReserved[4] ;

		// パラメータ設定
		SGLError SetShadowMappingInfo
			( S3DRenderContextInterface * renderShadowMap,
				const S3DLightEntry& lightShadowMap,
				const S3DDVector& vCameraTarget,
				const SGLSize& sizeDepthMap,
				double zScreen,
				double zDistance, double zScale = 1.0,
				double zNear = 100.0, double zFar = 10000.0,
				double zErrorPrec = -12.0, double zErroSubPrec = -10.0,
				double degAngleVarX = 0.0, double degAngleVarY = 0.0 ) ;
		// カメラ行列取得
		void GetCameraMatrix( S3DDMatrix& matCamera ) const ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 3D レンダリング・コンテキスト
	//////////////////////////////////////////////////////////////////////////

	// レンダリング機能
	struct	S3DRenderingCapacity
	{
		uint64_t	flagsRendering ;
		uint64_t	flagsShading ;
		uint32_t	typeHeadware ;
		uint32_t	maxTextureSize ;
		uint32_t	maxTextureUnit ;
		uint32_t	maxLightCount ;
		uint32_t	maxShadowmapCount ;
		uint32_t	flagsExtensions1 ;
		uint32_t	flagsExtensions2 ;
		uint64_t	flagsReserved[10] ;

		enum	RenderingFlag
		{
			renderingAutoStereo		= 0x00000001,
			renderingSortBuffered	= 0x00000002,
		} ;
		enum	ShadingFlag
		{
			shadingGouraud			= 0x00000001,
			shadingPhong			= 0x00000002,
			shadingRayTracing		= 0x00000010,
			shadingShadowMapping	= 0x00000100,
		} ;
		enum	HeadwareType
		{
			softwareEntisGLS3		= 0x0000,
			softwareEntisGLS4		= 0x0001,
			hardwareOpenGL			= 0x0100,
		} ;
		enum	ExtensionFlag1
		{
			extFramebuffer			= 0x00000001,
			extTextureNonPowerOf2	= 0x00000002,
			extMultiTexture			= 0x00000004,
			extVertexBuffer			= 0x00000008,
			extProgramableShader	= 0x00000100,
			extMultiRenderTarget	= 0x00000200,
		} ;
	} ;

	// レイトレーシング・パラメータ
	struct	S3DRenderRayTracingParam
	{
		#if	defined(__COTOPHA__)
		uint32_t	nFlags = 0 ;						// フラグ (S3DRenderRayTracingFlag)
		float32_t	fpShadowingDistance = 1000.0f ;		// 陰を落とす有効距離
		float32_t	fpRayTracingDistance = 10000.0f ;	// 光線追跡有効距離
		uint32_t	nRayReflectionCount = 3 ;			// 光線追跡回数
		uint32_t	nAppendShadowingCount = 1 ;			// 陰光線追跡回数（追加）

		#else
		uint32_t	nFlags ;					// フラグ (S3DRenderRayTracingFlag)
		float32_t	fpShadowingDistance ;		// 影を落とす有効距離
		float32_t	fpRayTracingDistance ;		// 光線追跡有効距離
		uint32_t	nRayReflectionCount ;		// 光線追跡回数
		uint32_t	nAppendShadowingCount ;		// 陰光線追跡回数（追加）

		S3DRenderRayTracingParam( void )
			: nFlags(0), fpShadowingDistance(1000.0f),
				fpRayTracingDistance(10000.0f),
				nRayReflectionCount(3), nAppendShadowingCount(1) {}
		#endif
	} ;

	enum	S3DRenderRayTracingFlag
	{
		raytraceShadowing		= 0x0001,		// 影を落とす
		raytraceShadowingAlpha	= 0x0002,		// 影に透明度を考慮する
		raytraceReflection		= 0x0010,		// 反射を有効にする
		raytraceRefraction		= 0x0020,		// 屈折を有効にする
		raytraceOnlyGlobalRef	= 0x0040,		// 大域反射・屈折のみ有効にする
	} ;

	enum	S3DPrimitiveType
	{
		primitivePoint			= 0,
		primitiveLine			= 2,
		primitiveLineStrip		= 3,
		primitiveTriangle		= 4,
		primitiveTriangleStrip	= 5,
		primitiveCount,
	} ;

	// プリミティブ形状頂点数
	size_t GetPrimitiveVertexCount( S3DPrimitiveType typePrimitive ) ;


	#if	defined(__COTOPHA__)
	class	native RenderContext ;
	class	native VertexBuffer
	{
	public:
		enum	AddRenderFlag
		{
			renderFenceOrder	= 0x0001,
			renderUncombinable	= 0x0002,
		} ;
		enum	MeshInfoConstant
		{
			countSubMesh	= 4,
		} ;
		enum	MeshInfoFlag
		{
			flagMeshMorphedVertex	= 0x0001,
			flagMeshBoneTransformed	= 0x0002,
			flagMeshTransformed		= 0x0003,
		} ;
		struct	MeshInfo
		{
			Material *		pMaterial ;
			uint32_t		typeMesh ;		// enum S3DPrimitiveType
			uint32_t		countPolygon ;
			uint32_t		countVertex ;
			uint32_t		nReserved ;
			S3DVector		vCenter ;
			float32_t		fpRadius ;
			S3DVector4 *	pvVertex ;
			S3DVector4 *	pvNormal ;
			S2DVector *		pvUVMap ;
			S3DColor *		pColor ;
			uint32_t *		pIndexedList ;
			uint32_t *		pSubIndexedList[countSubMesh] ;
			uint32_t		nSubPolyCount[countSubMesh] ;
			float32_t		fpSubMeshDensity ;
			uint32_t		nExAttrElements ;
			float32_t *		pfpExAttrElements ;
		} ;
		// 頂点オブジェクト生成
		native static VertexBuffer *
			NewBuffer( SGLPaintContextType type = typePaintDefault ) ;
		// 描画座標空間設定
		native SGLError AppendMatrixTransformation
			( const S3DDMatrix& mat, const S3DDVector& pos,
				const S3DColor * color = NULL,
				unsigned int nTransparency = 0 ) ;
		native SGLError SetMatrixTransformation
			( const S3DDMatrix& mat, const S3DDVector& pos,
				const S3DColor * color = NULL,
				unsigned int nTransparency = 0 ) ;
		native SGLError GetMatrixTransformation
			( S3DDMatrix& mat, S3DDVector& pos,
				S3DColor * color = NULL,
				unsigned int * pTransparency = NULL ) const ;
		native SGLError PushTransformation( void ) ;
		native SGLError PopTransformation( void ) ;
		native SGLError ResetTransformation( void ) ;
		// カスタムシェーダーパラメータ設定
		native SGLError SetCustomShaderUniform
			( const wchar_t * pwszUniformId,
					S3DCustomShader::UniformType type,
					const void * pData, size_t nCount ) ;
		native SGLError ResetCustomShaderUniform( void ) ;
		// ポリゴンリストをレンダリングバッファに追加
		native SGLError AddIndexedTriangleList
			( Material * pMaterial, uint32_t nFlags,
				size_t countPolygon, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) ;
		// トライアングルストリップをレンダリングバッファに追加
		native SGLError AddTriangleStrip
			( Material * pMaterial, uint32_t nFlags,
				size_t countTriangleStrip,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap, const S3DColor * pColor ) ;
		// プリミティブリストをレンダリングバッファに追加
		native SGLError AddIndexedPrimitiveList
			( S3DMaterial * pMaterial, uint32_t nFlags,
				S3DPrimitiveType typePrimitive,
				size_t countIndex, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) ;
		// 頂点バッファの内容を描画
		native SGLError AddVertexBuffer
			( Material * pMaterial, uint32_t nFlags,
					VertexBuffer * pBuffer,
					size_t iFirst = 0, ssize_t iEnd = -1,
					size_t nInstancing = 0,
					const S4DMatrix * pmatInstancing = NULL,
					const S3DColor * pColorInstancing = NULL ) ;
		// 描画の確定
		native SGLError Flush( void ) ;
		// メッシュ数を取得する
		native size_t GetMeshCount( void ) const ;
		// メッシュ情報取得
		native SGLError GetMeshInfoAt
			( MeshInfo& info, size_t iMesh, size_t nCopyVertices,
				size_t iFirstVertex = 0, uint32_t nFlags = 0 ) const ;
		// ポリゴンリストを更新
		native SGLError UpdateIndexedTriangleList
			( size_t iMesh, uint32_t nFlags,
				size_t countPolygon, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) ;
		// トライアングルストリップを更新
		native SGLError UpdateTriangleStrip
			( size_t iMesh, uint32_t nFlags,
				size_t countTriangleStrip,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap, const S3DColor * pColor ) ;
		// プリミティブリストを更新
		native SGLError UpdateIndexedPrimitiveList
			( size_t iMesh, uint32_t nFlags,
				size_t countIndex, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) = 0 ;
		// サブメッシュ（ポリゴンリスト）を更新
		native SGLError UpdateSubIndexedTriangleList
			( size_t iMesh, size_t iSubMesh, uint32_t nFlags,
				size_t countPolygon, const uint32_t * pIndexedList ) ;
		// サブメッシュ切り替えｚ座標比を設定する
		native SGLError SetSubMeshDensity
			( size_t iMesh, float32_t fpDensity, ssize_t iSelector = -1 ) ;
		// 追加的な頂点属性を設定
		native SGLError SetExtendVertexAttribute
			( size_t iMesh, size_t countElements,
				size_t countVertex, const float32_t * pfpAttrElements ) ;
		// メッシュにウェイトマップを設定（親→子順）
		native SGLError SetBoneWeightMap
			( size_t iMesh, size_t nCount, const float32_t ** ppWeightMaps ) ;
		// メッシュにボーン行列設定（親→子順）
		native SGLError SetBoneMatrix
			( size_t iMesh, size_t nCount,
				const S3DMatrix * pMatrix, const S3DVector * pTrans ) ;
		// メッシュにモーフターゲット枠を確保
		native SGLError AllocateMorphing( size_t iMesh, size_t nCount ) ;
		// メッシュにモーフターゲットを設定
		native SGLError SetMorphingTargetMesh
			( size_t iMesh, size_t iMorph, size_t countVertex,
				const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap, const S3DColor * pColor ) ;
		// メッシュのモーフターゲットにウェイトを設定
		native SGLError SetMorphingTargetWeight
			( size_t iMesh, size_t iMorph,
				size_t countVertex, const float32_t * pfpWeight ) ;
		// モーフィング設定
		native SGLError SetMorphingApplication
			( size_t iMesh, const ssize_t * pTargetMesh,
					const float32_t * pApplication, size_t nTargetMeshCount ) ;
		// モーフィング設定取得
		native SGLError GetMorphingApplication
			( size_t iMesh, ssize_t& iTargetMesh,
					float32_t& fpApplication, size_t iTargetMeshIndex ) ;
		// メッシュ表示設定
		native SGLError EnableToRenderMesh
			( size_t iFirst = 0, ssize_t iEnd = -1, bool fEnable = true ) ;
		// メッシュ表示フラグ取得
		native bool IsEnabledToRenderMesh( size_t iMesh ) const ;
		// バッファを S3DRenderBufferInterface へ出力
		native SGLError RenderBufferTo
			( RenderContext * render,
					uint64_t flagsExclusion = 0,
					size_t iFirst = 0, ssize_t iEnd = -1,
					size_t nInstancing = 0,
					const S4DMatrix * pmatInstancing = NULL,
					const S3DColor * pColorInstancing = NULL ) const ;
		// バッファを消去
		native void ClearBuffer( void ) ;
		// バッファのメモリブロックサイズ設定
		native void SetBufferUnitSize( size_t nBytes ) ;
		// メッシュ外接球取得
		native double GetCircumscribedSphere( S3DVector& vCenter ) ;
	} ;

	class	native RenderContext	: public PaintContext
	{
	public:
		// 描画オブジェクト生成
		native static RenderContext *
			NewContext( SGLPaintContextType type = typePaintDefault ) ;
		// 描画座標空間設定
		native SGLError AppendMatrixTransformation
			( const S3DDMatrix& mat, const S3DDVector& pos,
				const S3DColor * color = NULL,
				unsigned int nTransparency = 0 ) ;
		native SGLError SetMatrixTransformation
			( const S3DDMatrix& mat, const S3DDVector& pos,
				const S3DColor * color = NULL,
				unsigned int nTransparency = 0 ) ;
		native SGLError GetMatrixTransformation
			( S3DDMatrix& mat, S3DDVector& pos,
				S3DColor * color = NULL,
				unsigned int * pTransparency = NULL ) const ;
		// カスタムシェーダーパラメータ設定
		native SGLError SetCustomShaderUniform
			( S3DCustomShader::UniformType type,
					const void * pData, size_t nCount ) ;
		native SGLError ResetCustomShaderUniform( void ) ;
		// 投影スクリーン座標設定
		native SGLError SetProjectionScreen
			( const S3DVector& vScreen,
				double zScale = 1.0, double fpPixelAspect = 1.0 ) ;
		// 投影スクリーン座標取得
		native SGLError GetProjectionScreen
			( S3DVector& vScreen, double& zScale, double& fpPixelAspect ) const ;
		// 透視変換行列取得
		native bool GetPerspectiveMatrix( S4DMatrix& matPars ) const ;
		// 透視変換行列設定
		native void SetPerspectiveMatrix
			( StereoViewIndex sviView,
				const S4DMatrix& matPers, bool fPersMatrix = true ) ;
		native void EnablePerspectiveMatrix( bool fPersMatrix ) ;
		// カメラ設定
		native void SetCamera
			( const S3DDMatrix& matCamera,
					const S3DDVector& posCamera ) ;
		// カメラ取得
		native void GetCamera
			( S3DDMatrix& matCamera, S3DDVector& posCamera ) const ;
		// 立体視視差設定
		native void SetParallax
			( double xParallax, double zFocusRate, double xScreenDelta ) ;
		// 立体視視差取得
		native double GetParallax( void ) ;
		// ｚクリップ範囲を設定
		native void SetZClipRange( double zMin, double zMax ) ;
		// 光源を設定
		native void SetLightEntries
			( const S3DLightEntry* pLights, size_t countLight ) ;
		// シャドウマップを設定
		native void SetShadowMap
			( uint32_t idLight,
				Image* pShadowMap,
				const S3DShadowMapInfo& infShadowMap,
				Image* pShadowMapColor = NULL ) ;
		// 疑似フォッグを設定
		native void SetFog
			( uint32_t rgbFog, double zFogNear, double zFogFar ) ;
		native void EnableFog( bool fFog ) ;
		// シェーディング設定
		native void SetShadingFlag( uint64_t nShadingMethod ) ;
		// シェーディング取得
		native uint64_t GetShadingFlag( void ) ;
		// レイトレーシング設定
		native void SetRayTracingParameter
					( const S3DRenderRayTracingParam& rrtp ) ;
		// グローバル環境マッピング設定
		enum	EnvironmentMappingType
		{
			envMappingHemisphere,
			envMappingSphere,
			envMappingCube,
			envMappingViewport,
			envMappingViewportDepth,
			envMappingTypeMask	= 0xFF,
		} ;
		native void SetEnvironmentMappingImage
					( Image * pImage, uint32_t nFlags ) ;
		// グローバル環境マッピング変換行列設定
		native void SetEnvironmentMappingMatrix( const S3DMatrix& matMapping ) ;
		// 輪郭描画色設定
		native void SetOffsetBorderColor( uint32_t rgbBorder ) ;
		// 輪郭描画オフセット係数設定
		native void SetOffsetBorderCoefficient( float32_t a, float32_t b ) ;
		// オプショナル機能設定
		enum	FeatureType
		{
			featureSRGB,		// sRGB
		} ;
		native SGLError SetOptionalFeature
			( FeatureType feature, int32_t nParam1,
							void * pParam2, size_t sizeOfParam2 ) ;

	public:
		// 対応機能取得
		native void GetRenderingCapacity( S3DRenderingCapacity& caps ) ;
		// 立体視用バッファ選択
		enum	StereoViewIndex
		{
			stereoViewAuto	= -1,
			stereoViewRight,
			stereoViewLeft,
		} ;
		native StereoViewIndex CurrentParallaxView( void ) ;
		// 立体視用バッファ選択
		native SGLError SelectParallaxView( StereoViewIndex sviView ) ;
		// 内部バッファサイズ設定
		native SGLError SetRenderingBufferSize( uint32_t countVertex ) ;
		// 3D レンダリング用バッファ・インターフェース開始
		native SGLError Begin3DRenderer( uint64_t nFlags = 0 ) ;
		// 3D レンダリング用バッファ・インターフェース終了
		native SGLError End3DRenderer( uint64_t nFlags = 0 ) ;

	public:
		enum	AddRenderFlag
		{
			renderFenceOrder	= 0x0001,
			renderUncombinable	= 0x0002,
		} ;
		// ポリゴンリストをレンダリングバッファに追加
		native SGLError AddIndexedTriangleList
			( Material * pMaterial, uint32_t nFlags,
				size_t countPolygon, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) ;
		// トライアングルストリップをレンダリングバッファに追加
		native SGLError AddTriangleStrip
			( Material * pMaterial, uint32_t nFlags,
				size_t countTriangleStrip,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap, const S3DColor * pColor ) ;
		// プリミティブリストをレンダリングバッファに追加
		native SGLError AddIndexedPrimitiveList
			( S3DMaterial * pMaterial, uint32_t nFlags,
				S3DPrimitiveType typePrimitive,
				size_t countIndex, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) ;
		// 頂点バッファの内容を描画
		native SGLError AddVertexBuffer
			( Material * pMaterial, uint32_t nFlags,
					VertexBuffer * pBuffer,
					size_t iFirst = 0, ssize_t iEnd = -1,
					size_t nInstancing = 0,
					const S4DMatrix * pmatInstancing = NULL,
					const S3DColor * pColorInstancing = NULL ) ;
	} ;
	#endif

	class	S3DVertexBufferInterface ;
	#if	!defined(__COTOPHA__)
	typedef	S3DVertexBufferInterface	VertexBuffer ;
	#endif

	class	S3DRenderBufferInterface	: public SSystem::SObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SakuraGL::S3DRenderBufferInterface, SObject )

	public:
		// シェーディング設定
		virtual void SetShadingFlag( uint64_t nShadingMethod ) = 0 ;
		// シェーディング取得
		virtual uint64_t GetShadingFlag( void ) = 0 ;
		// カスタムシェーダー設定
		virtual SGLError AttachCustomShader( S3DCustomShader * pShader ) = 0 ;
		// カスタムシェーダー取得
		virtual S3DCustomShader * GetCustomShader( void ) const = 0 ;
		// 描画座標空間設定
		virtual SGLError AppendMatrixTransformation
			( const S3DDMatrix& mat, const S3DDVector& pos,
				const S3DColor * color = NULL,
				unsigned int nTransparency = 0 ) = 0 ;
		SGLError AddMatrixTransformation
			( const S3DDMatrix& mat, const S3DDVector& pos,
				const S3DColor * color = NULL,
				unsigned int nTransparency = 0 )
		{
			return	AppendMatrixTransformation( mat, pos, color, nTransparency ) ;
		}
		virtual SGLError SetMatrixTransformation
			( const S3DDMatrix& mat, const S3DDVector& pos,
				const S3DColor * color = NULL,
				unsigned int nTransparency = 0 ) = 0 ;
		virtual SGLError GetMatrixTransformation
			( S3DDMatrix& mat, S3DDVector& pos,
				S3DColor * color = NULL,
				unsigned int * pTransparency = NULL ) const = 0 ;
		virtual SGLError PushTransformation( void ) = 0 ;
		virtual SGLError PopTransformation( void ) = 0 ;
		virtual SGLError ResetTransformation( void ) = 0 ;
		// カスタムシェーダーパラメータ設定
		virtual SGLError SetCustomShaderUniform
			( const wchar_t * pwszUniformId,
					S3DCustomShader::UniformType type,
					const void * pData, size_t nCount ) = 0 ;
		virtual SGLError ResetCustomShaderUniform( void ) = 0 ;
		SGLError SetCustomShaderUniformInt
			( const wchar_t * pszUniform,
				const int32_t * pData, size_t nCount ) ;
		SGLError SetCustomShaderUniformFloat
			( const wchar_t * pszUniform,
				const float32_t * pData, size_t nCount ) ;
		SGLError SetCustomShaderUniformVector2D
			( const wchar_t * pszUniform,
				const S2DVector * pData, size_t nCount ) ;
		SGLError SetCustomShaderUniformVector3D
			( const wchar_t * pszUniform,
				const S3DVector * pData, size_t nCount ) ;
		SGLError SetCustomShaderUniformVector4D
			( const wchar_t * pszUniform,
				const S4DVector * pData, size_t nCount ) ;
		SGLError SetCustomShaderUniformMatrix3x3
			( const wchar_t * pszUniform,
				const S3DMatrix * pData, size_t nCount ) ;
		SGLError SetCustomShaderUniformMatrix4x4
			( const wchar_t * pszUniform,
				const S4DMatrix * pData, size_t nCount ) ;
		SGLError SetCustomShaderUniformTexture
			( const wchar_t * pszUniform, SGLImageObject * pTexture ) ;
		// 輪郭描画色設定
		struct	OffsetBorderParam
		{
			SGLPalette	rgbBorder ;
			float32_t	aThickness ;
			float32_t	bThickness ;
		} ;
		virtual void SetOffsetBorderColor( uint32_t rgbBorder ) = 0 ;
		// 輪郭描画オフセット係数設定 (ax+b)
		virtual void SetOffsetBorderCoefficient( float32_t a, float32_t b ) = 0 ;
		// オプショナル機能設定
		enum	FeatureType
		{
			featureSRGB,			// sRGB : nParam1 = {true|false}, pParam2 = NULL (&int32_t to get)
			featureEnvMap,			// nParam1 = {0|envMappingRefraction|envMappingViewportDepth}, pParam2 = &struct EnvMappingParam
			featureOffsetBorder,	// nParam1 = 0, pParam2 = &struct OffsetBorderParam
			featureFaceCulling,		// nParam1 = enum FaceCullingOperation, pParam2 = NULL (&int32_t to get)
			featureDepthMask,		// nParam1 = enum DepthMaskOperation, pParam2 = NULL (&int32_t to get)
			featureBlendOperation,	// nParam1 = enum BlendOperation, pParam2 = NULL (&int32_t to get)
			featureContextSet,		// nParam1 = 0, pParam2 = &struct OptionalContext
			featureOrderPriority,	// nParam1 = priority(0～15), pParam2 = NULL (&int32_t to get)
			featurePointSize,		// nParam1 = 0, pParam2 = &float32_t
			featureLineWidth,		// nParam1 = 0, pParam2 = &float32_t
			featureAnisotropy,		// nParam1 = 0, pParam2 = &float32_t
		} ;
		enum	FaceCullingOperation
		{
			faceCullingDefault,
			faceCullingBack,
			faceCullingFront,
			faceCullingNo,
		} ;
		enum	DepthMaskOperation
		{
			depthMaskDefault,
			depthMaskEnable,
			depthMaskNoWrite,
			depthMaskNoWriteGT,
			depthMaskNoTest,
		} ;
		enum	BlendOperation
		{
			blendDefault,
			blendProductedSrc,
			blendAdd,
			blendUnproductedSrc,
			blendCopy,
			blendDstMasked,
			blendMulColor,
		} ;
		enum	OptionalContextMask
		{
			optionShadingFlag		= 0x00000001,
			optionCustomShader		= 0x00000002,
			optionBorderParam		= 0x00000004,
			optionFaceCulling		= 0x00000008,
			optionDepthMask			= 0x00000010,
			optionBlendOperation	= 0x00000020,
			optionPointSize			= 0x00000040,
			optionLineWidth			= 0x00000080,
			optionAnisotropy		= 0x00000100,
			optionContextAll		= 0x000001FF,
		} ;
		struct	OptionalContextSet
		{
			uint64_t				nOptionMask ;		// complext of enum OptionalContextMask
			uint64_t				nShadingFlags ;		// complext of enum S3DShadingFlags
			S3DCustomShader *		pShader ;
			OffsetBorderParam		opbBorder ;
			FaceCullingOperation	faceCulling ;
			DepthMaskOperation		depthMask ;
			BlendOperation			blendOp ;
			float32_t				pointSize ;
			float32_t				lineWidth ;
			float32_t				anisotropy ;
		} ;
		virtual SGLError SetOptionalFeature
			( FeatureType feature, int32_t nParam1,
						const void * pParam2, size_t sizeOfParam2 ) = 0 ;
		// オプショナル機能取得
		virtual SGLError GetOptionalFeature
			( FeatureType feature, int32_t nParam1,
						void * pParam2, size_t sizeOfParam2 ) const = 0 ;

	public:
		enum	AddRenderFlag
		{
			renderFenceOrder	= 0x0001,	// 描画順序をフェンスする
			renderUncombinable	= 0x0002,	// AddIndexedPrimitiveList の自動結合を無効化
		} ;
		// ポリゴンリストをレンダリングバッファに追加
		virtual SGLError AddIndexedTriangleList
			( S3DMaterial * pMaterial, uint32_t nFlags,
				size_t countPolygon, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) = 0 ;
		// トライアングルストリップをレンダリングバッファに追加
		virtual SGLError AddTriangleStrip
			( S3DMaterial * pMaterial, uint32_t nFlags,
				size_t countTriangleStrip,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap, const S3DColor * pColor ) = 0 ;
		// プリミティブリストをレンダリングバッファに追加
		virtual SGLError AddIndexedPrimitiveList
			( S3DMaterial * pMaterial, uint32_t nFlags,
				S3DPrimitiveType typePrimitive,
				size_t countIndex, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) = 0 ;
		// 頂点バッファの内容を描画
		virtual SGLError AddVertexBuffer
			( S3DMaterial * pMaterial, uint32_t nFlags,
					S3DVertexBufferInterface * pBuffer,
					size_t iFirst = 0, ssize_t iEnd = -1,
					size_t nInstancing = 0,
					const S4DMatrix * pmatInstancing = NULL,
					const S3DColor * pColorInstancing = NULL ) = 0 ;
		// 描画の確定
		virtual SGLError Flush( void ) = 0 ;

	public:
		// 遅延削除オブジェクト追加（Flush 時に削除）
		virtual void AddTemporaryObject( ESLObject * pObj ) = 0 ;
	} ;

	class	S3DVertexVariantBuffer	: public SSystem::SObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SakuraGL::S3DVertexVariantBuffer, SObject )
	public:
		// メッシュにボーン行列設定
		virtual SGLError SetBoneMatrix
			( size_t iMesh, size_t nCount,
				const S3DMatrix * pMatrix, const S3DVector * pTrans ) = 0 ;
		// メッシュのボーン行列取得
		virtual size_t GetBoneMatrix
			( size_t iMesh, size_t nCount,
				S3DMatrix * pMatrix, S3DVector * pTrans ) = 0 ;
		// モーフィング設定
		virtual SGLError SetMorphingApplication
			( size_t iMesh, const ssize_t * pTargetMesh,
					const float32_t * pApplication, size_t nTargetMeshCount ) = 0 ;
		// モーフィング設定取得
		virtual SGLError GetMorphingApplication
			( size_t iMesh, ssize_t& iTargetMesh,
					float32_t& fpApplication, size_t iTargetMeshIndex ) = 0 ;
		// メッシュ表示設定
		virtual SGLError EnableToRenderMesh
			( size_t iFirst = 0, ssize_t iEnd = -1, bool fEnable = true ) = 0 ;
		// メッシュ表示フラグ取得
		virtual bool IsEnabledToRenderMesh( size_t iMesh ) const = 0 ;
		// メッシュマテリアル設定
		virtual SGLError SetMaterialToRenderMesh
			( size_t iMesh, S3DMaterial * pMaterial ) = 0 ;
		// メッシュマテリアル取得
		virtual S3DMaterial * GetMaterialToRenderMesh( size_t iMesh ) const = 0 ;
	} ;

	class	S3DVertexDeviceBufferInterface	: public ESLObject
	{
	protected:
		S3DVertexDeviceBufferInterface *	m_ptrNext ;
		uint32_t							m_typeObject ;	// enum SGLImageBufferObjectType

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SakuraGL::S3DVertexDeviceBufferInterface, ESLObject )
		// 構築関数
		S3DVertexDeviceBufferInterface( void )
			: m_ptrNext(NULL), m_typeObject(0) {}
		// 消滅関数
		virtual ~S3DVertexDeviceBufferInterface( void ) ;
		// 指定タイプの S3DVertexDeviceBufferInterface 取得
		S3DVertexDeviceBufferInterface *
			GetDeviceBufferTypeOf( SGLImageBufferObjectType type ) ;
		S3DVertexDeviceBufferInterface *
			GetDeviceBufferAs( const ESLRuntimeClass& rtClass ) ;
		template <class T> T * GetDeviceBuffer( void )
		{
			return	ESLTypeCast<T>( GetDeviceBufferAs( ESL_RUNTIME_CLASS(T) ) ) ;
		}
		// リストの次に追加
		void AddNextVertexDeviceBuffer( S3DVertexDeviceBufferInterface * pVDB ) ;
		// リストの次の S3DVertexDeviceBufferInterface を分離
		S3DVertexDeviceBufferInterface * DetachNextVertexDeviceBuffer( void ) ;

	public:
		// 描画の確定時処理
		virtual void OnFlush( S3DVertexBufferInterface * pVB ) ;
		// プリミティブリストを更新時処理
		virtual void OnUpdateIndexedPrimitiveList
			( S3DVertexBufferInterface * pVB,
				size_t iMesh, uint32_t nFlags,
				size_t countIndex, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) ;
		// サブメッシュ（ポリゴンリスト）を更新時処理
		virtual void OnUpdateSubIndexedTriangleList
			( S3DVertexBufferInterface * pVB,
				size_t iMesh, size_t iSubMesh, uint32_t nFlags,
				size_t countPolygon, const uint32_t * pIndexedList ) ;
		// 追加的な頂点属性を設定時処理
		virtual void OnSetExtendVertexAttribute
			( S3DVertexBufferInterface * pVB,
				size_t iMesh, size_t countElements,
				size_t countVertex, const float32_t * pfpAttrElements ) ;
		// メッシュにウェイトマップを設定時処理
		virtual void OnSetBoneWeightMap
			( S3DVertexBufferInterface * pVB,
				size_t iMesh, size_t nCount, const float32_t ** ppWeightMaps ) ;
		// メッシュにモーフターゲット枠を確保時処理
		virtual void OnAllocateMorphing
			( S3DVertexBufferInterface * pVB, size_t iMesh, size_t nCount ) ;
		// メッシュにモーフターゲットを設定時処理
		virtual void OnSetMorphingTargetMesh
			( S3DVertexBufferInterface * pVB,
				size_t iMesh, size_t iMorph, size_t countVertex,
				const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap, const S3DColor * pColor ) ;
		// バッファ消去時処理
		virtual void OnClearBuffer( S3DVertexBufferInterface * pVB ) ;
	} ;

	class	S3DVertexBufferInterface
					: public S3DRenderBufferInterface,
						public S3DVertexVariantBuffer
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SakuraGL::S3DVertexBufferInterface,
				S3DRenderBufferInterface, S3DVertexVariantBuffer )
		// 頂点オブジェクト生成
		static S3DVertexBufferInterface *
			NewBuffer( SGLPaintContextType type = typePaintDefault ) ;

	public:
		enum	MeshInfoConstant
		{
			countSubMesh	= 4,
		} ;
		enum	MeshInfoFlag
		{
			flagMeshMorphedVertex	= 0x0001,
			flagMeshBoneTransformed	= 0x0002,
			flagMeshTransformed		= 0x0003,
		} ;
		struct	MeshInfo
		{
			S3DMaterial *		pMaterial ;
			S3DPrimitiveType	typeMesh ;
			uint32_t			countPrimitive ;
			uint32_t			countVertex ;
			uint32_t			nReserved ;
			S3DVector			vCenter ;
			float32_t			fpRadius ;
			S3DVector4 *		pvVertex ;
			S3DVector4 *		pvNormal ;
			S2DVector *			pvUVMap ;
			S3DColor *			pColor ;
			uint32_t *			pIndexedList ;
			uint32_t *			pSubIndexedList[countSubMesh] ;
			uint32_t			nSubPolyCount[countSubMesh] ;
			ssize_t				iSubMeshSelector ;	// 0 以上でサブメッシュ明示指定
			float32_t			fpSubMeshDensity ;	// 表示切り替えｚ座標比
			size_t				nExAttrElements ;	// 拡張属性要素数
			float32_t *			pfpExAttrElements ;

			// モデルｚ座標をzm, 投影スクリーンｚ座標をzs とした時
			// i = floor( zm / (fpSubMeshDensity * zs) ) - 1
			// i を pSubIndexedList 指標とする。
			// 但し i < 0 は pIndexedList を使用し、
			// i >= countSubMesh は countSubMesh - 1 にクリップする
		} ;
		struct	PrimitiveBuffer
		{
			S3DVector4 *	pvVertex ;
			S3DVector4 *	pvNormal ;
			S2DVector *		pvUVMap ;
			S3DColor *		pColor ;
			uint32_t *		pIndexedList ;
		} ;
		enum	BufferControlFlag
		{
			bufferAutoMerge			= 0x0001,
			bufferKeepDeviceBuffer	= 0x0002,
			bufferDynamicVertex		= 0x0004,
			bufferDynamicIndex		= 0x0008,
		} ;

	public:
		// バッファ制御フラグ
		virtual uint32_t GetBufferControlFlags( void ) const ;
		virtual void SetBufferControlFlags( uint32_t nFlags ) ;
		// デフォルトマテリアル
		virtual S3DMaterial * GetDefaultMaterial( void ) const ;
		virtual void AttachDefaultMaterial( S3DMaterial * pMaterial ) ;
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
		// メッシュ数を取得する
		virtual size_t GetMeshCount( void ) const = 0 ;
		// メッシュ情報取得
		virtual SGLError GetMeshInfoAt
			( MeshInfo& info, size_t iMesh, size_t nCopyVertices,
					size_t iFirstVertex = 0, uint32_t nFlags = 0 ) const = 0 ;
		// ポリゴンリストを更新
		virtual SGLError UpdateIndexedTriangleList
			( size_t iMesh, uint32_t nFlags,
				size_t countPolygon, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) = 0 ;
		// トライアングルストリップを更新
		virtual SGLError UpdateTriangleStrip
			( size_t iMesh, uint32_t nFlags,
				size_t countTriangleStrip,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap, const S3DColor * pColor ) = 0 ;
		// プリミティブリストを更新
		virtual SGLError UpdateIndexedPrimitiveList
			( size_t iMesh, uint32_t nFlags,
				size_t countIndex, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) = 0 ;
		// サブメッシュ（ポリゴンリスト）を更新
		virtual SGLError UpdateSubIndexedTriangleList
			( size_t iMesh, size_t iSubMesh, uint32_t nFlags,
				size_t countPolygon, const uint32_t * pIndexedList ) = 0 ;
		// 追加的な頂点属性を設定
		virtual SGLError SetExtendVertexAttribute
			( size_t iMesh, size_t countElements,
				size_t countVertex, const float32_t * pfpAttrElements ) = 0 ;
		// サブメッシュ切り替えｚ座標比を設定する
		virtual SGLError SetSubMeshDensity
			( size_t iMesh, float32_t fpDensity, ssize_t iSelector = -1 ) = 0 ;
		// メッシュにウェイトマップを設定
		virtual SGLError SetBoneWeightMap
			( size_t iMesh, size_t nCount, const float32_t ** ppWeightMaps ) = 0 ;
		// メッシュにジョイントマップを設定
		virtual SGLError SetBoneJointMap
			( size_t iMesh, size_t nBoneCount,
				size_t nJointCount, const uint32_t ** ppJointMaps ) = 0 ;
		// メッシュにボーン行列設定
		virtual SGLError SetBoneMatrix
			( size_t iMesh, size_t nCount,
				const S3DMatrix * pMatrix, const S3DVector * pTrans ) = 0 ;
		// メッシュのボーン行列取得
		virtual size_t GetBoneMatrix
			( size_t iMesh, size_t nCount,
				S3DMatrix * pMatrix, S3DVector * pTrans ) = 0 ;
		// メッシュにモーフターゲット枠を確保
		virtual SGLError AllocateMorphing( size_t iMesh, size_t nCount ) = 0 ;
		// メッシュにモーフターゲットを設定
		virtual SGLError SetMorphingTargetMesh
			( size_t iMesh, size_t iMorph, size_t countVertex,
				const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap, const S3DColor * pColor ) = 0 ;
		// メッシュのモーフターゲットにウェイトを設定
		virtual SGLError SetMorphingTargetWeight
			( size_t iMesh, size_t iMorph,
				size_t countVertex, const float32_t * pfpWeight ) = 0 ;
		// モーフィング設定
		virtual SGLError SetMorphingApplication
			( size_t iMesh, const ssize_t * pTargetMesh,
					const float32_t * pApplication, size_t nTargetMeshCount ) = 0 ;
		// モーフィング設定取得
		virtual SGLError GetMorphingApplication
			( size_t iMesh, ssize_t& iTargetMesh,
					float32_t& fpApplication, size_t iTargetMeshIndex ) = 0 ;
		// メッシュ表示設定
		virtual SGLError EnableToRenderMesh
			( size_t iFirst = 0, ssize_t iEnd = -1, bool fEnable = true ) = 0 ;
		// メッシュ表示フラグ取得
		virtual bool IsEnabledToRenderMesh( size_t iMesh ) const = 0 ;
		// 現在の設定に適合する S3DVertexVariantBuffer を生成
		virtual S3DVertexVariantBuffer * CreateVariantBuffer( void ) = 0 ;
		// S3DVertexVariantBuffer のパラメータを VertexBuffer へ反映
		virtual SGLError UpdateVertexVariant
			( S3DVertexVariantBuffer * pVVB, size_t iFirst = 0, ssize_t iEnd = -1 ) = 0 ;
		// 参照バリアントを生成
		virtual S3DVertexBufferInterface * NewReferenceVariantBuffer( void ) = 0 ;

	public:
		// バッファを S3DRenderBufferInterface へ出力
		virtual SGLError RenderBufferTo
			( S3DRenderBufferInterface * render,
					uint64_t flagsExclusion = 0,
					size_t iFirst = 0, ssize_t iEnd = -1,
					size_t nInstancing = 0,
					const S4DMatrix * pmatInstancing = NULL,
					const S3DColor * pColorInstancing = NULL ) const ;
		// モデル描画が表示範囲にあるか見積もる
		virtual bool IsModelIntoView
			( S3DRenderContextInterface * render,
						float32_t fpScaleMargin = 0.5f,
						float32_t fpModelMargin = 1.0f ) ;
		// バッファを消去
		virtual void ClearBuffer( void ) ;
		// デバイスリソースを解放
		virtual void ReleaseAllDeviceResources( void ) ;
		// バッファのメモリブロックサイズ設定
		virtual void SetBufferUnitSize( size_t nBytes ) ;
		// メッシュ外接球取得
		virtual double GetCircumscribedSphere( S3DVector& vCenter ) = 0 ;
		// メッシュ外接直方体取得
		virtual bool GetCircumscribedParallelepiped
							( S3DVector& vMin, S3DVector& vMax ) ;
		// 総ポリゴン数集計
		virtual size_t CountOfTotalPolygons( void ) const ;
		// 総頂点数集計
		virtual size_t CountOfTotalVertices( void ) const ;

	public:
		// マルチインスタンス描画モード設定
		virtual SGLError EnableMultiInstancingMode( bool flagEnable ) ;
		// マルチインスタンス描画モード判定
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

	public:
		// VertexBuffer 取得
		virtual VertexBuffer * GetVertexBufferObject( void ) const ;
		// S3DVertexDeviceBufferInterface 取得
		virtual S3DVertexDeviceBufferInterface *
			GetDeviceBufferTypeOf( SGLImageBufferObjectType type ) = 0 ;
		virtual S3DVertexDeviceBufferInterface *
			GetDeviceBufferAs( const ESLRuntimeClass& rtClass ) = 0 ;
		template <class T> T * GetDeviceBuffer( void )
		{
			return	ESLTypeCast<T>( GetDeviceBufferAs( ESL_RUNTIME_CLASS(T) ) ) ;
		}
		// S3DVertexDeviceBufferInterface 追加
		virtual void AttachDeviceBuffer( S3DVertexDeviceBufferInterface * pDevBuf ) = 0 ;
	} ;

	class	S3DRenderContextInterface
					: public SGLPaintContextInterface,
						public S3DRenderBufferInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SakuraGL::S3DRenderContextInterface,
				SGLPaintContextInterface, S3DRenderBufferInterface )

	protected:
		static SGLPaintContextType	m_typeDefaultRender ;

	public:
		// 描画オブジェクト生成
		static S3DRenderContextInterface *
			NewContext( SGLPaintContextType type = typePaintDefault ) ;
		// デフォルトのレンダラタイプ
		static SGLPaintContextType GetDefaultRenderType( void )
		{
			return	m_typeDefaultRender ;
		}
		static void SetDefaultRenderType( SGLPaintContextType type )
		{
			m_typeDefaultRender = type ;
		}

	public:
		enum	StereoViewIndex
		{
			stereoViewAuto	= SGLDrawImageParamList::stereoViewAuto,
			stereoViewRight	= SGLDrawImageParamList::stereoViewRight,
			stereoViewLeft	= SGLDrawImageParamList::stereoViewLeft,
		} ;
		// バッファ複製
		enum	CopyBufferFlag
		{
			copyBufferColor	= 0x0001,
			copyBufferDepth	= 0x0002,
		} ;
		virtual SGLError CopyBufferFrom
			( S3DRenderContextInterface& renderSrc, uint32_t nFlags = 0,
				int xDst = 0, int yDst = 0, const SGLImageRect * pSrcRect = nullptr ) = 0 ;
		// マルチターゲット（2つ目以降）描画先設定
		virtual SGLError AttachMultiTargetImages
			( SGLImageObject*const* ppTargets, size_t nCount ) = 0 ;
		// マルチターゲット（2つ目以降）取得
		virtual SGLImageObject*const* GetMultiTargetImages( size_t& nCount ) const = 0 ;
		// カスタムシェーダー設定
		virtual SGLError AttachCustomShader( S3DCustomShader * pShader ) = 0 ;
		// カスタムシェーダー取得
		virtual S3DCustomShader * GetCustomShader( void ) const = 0 ;
		// 描画座標空間設定
		virtual SGLError PushTransformation( void ) = 0 ;
		virtual SGLError PopTransformation( void ) = 0 ;
		virtual SGLError ResetTransformation( void ) = 0 ;
		// 描画の確定
		virtual SGLError Flush( void ) = 0 ;
		// 投影スクリーン座標設定
		virtual SGLError SetProjectionScreen
			( const S3DVector& vScreen,
				double zScale = 1.0, double fpPixelAspect = 1.0 ) = 0 ;
		// 投影スクリーン座標取得
		virtual SGLError GetProjectionScreen
			( S3DVector& vScreen,
				double& zScale, double& fpPixelAspect ) const = 0 ;
		// 透視変換行列取得
		virtual bool GetPerspectiveMatrix( S4DMatrix& matPers ) const = 0 ;
		// 透視変換行列設定
		virtual void SetPerspectiveMatrix
			( StereoViewIndex sviView,
				const S4DMatrix& matPers, bool fPersMatrix = true ) = 0 ;
		virtual void EnablePerspectiveMatrix( bool fPersMatrix ) = 0 ;
		// カメラ設定
		virtual void SetCamera
			( const S3DDMatrix& matCamera,
					const S3DDVector& posCamera ) = 0 ;
		// 注視点と視点指定による標準的なカメラの設定
		void SetCameraAngle
			( const S3DDVector& posTarget,
					const S3DDVector& posView, double zAngle = 0.0 /*[deg]*/ ) ;
		// 注視点と視点指定、上ベクトルによるカメラの設定
		void SetCameraAngleVector
			( const S3DDVector& posTarget,
					const S3DDVector& posView,
					const S3DDVector& vAngleTop ) ;
		// カメラ取得
		virtual void GetCamera
			( S3DDMatrix& matCamera, S3DDVector& posCamera ) const = 0 ;
		// カメラの逆変換行列を現在の座標空間に設定する
		void SetInverseCameraTransformation
			( const S3DColor * color = NULL, unsigned int nTransparency = 0 ) ;
		// 球（ローカル座標）が視界に収まるか判定する
		virtual bool IsSphereIntoView( const S3DDVector& vPos, double radius ) const ;
		// 座標（ローカル）の透視変換
		virtual bool GetProjectedPosition
			( S2DDVector& vProjPos, const S3DDVector& vPos, double fpLimit = 100000.0 ) const ;
		// 立体視視差設定
		virtual void SetParallax
			( double xParallax, double zFocusRate, double xScreenDelta ) = 0 ;
		// 立体視視差取得
		virtual double GetParallax( void ) const = 0 ;
		// ｚクリップ範囲を設定
		virtual void SetZClipRange( double zMin, double zMax ) = 0 ;
		// 光源を設定
		virtual void SetLightEntries
			( const S3DLightEntry* pLights, size_t countLight ) = 0 ;
		// シャドウマップを設定
		virtual void SetShadowMap
			( uint32_t idLight,
				SGLImageObject* pShadowMapDepth,
				const S3DShadowMapInfo& infShadowMap,
				SGLImageObject* pShadowMapColor = NULL ) = 0 ;
		// 疑似フォッグを設定
		virtual void SetFog
			( uint32_t rgbFog, double zFogNear, double zFogFar ) = 0 ;
		virtual void EnableFog( bool fFog ) = 0 ;
		// シェーディング設定
		virtual void SetShadingFlag( uint64_t nShadingMethod ) = 0 ;
		// シェーディング取得
		virtual uint64_t GetShadingFlag( void ) = 0 ;
		// レイトレーシング設定
		virtual void SetRayTracingParameter
					( const S3DRenderRayTracingParam& rrtp ) = 0 ;
		// グローバル環境マッピング設定
		enum	EnvironmentMappingType
		{
			envMappingHemisphere,		// 半球（xz平面y軸投影）
			envMappingSphere,			// 球（xz平面y軸投影／上半分北半球／下半分南半球）
			envMappingCube,				// キューブマッピング
			envMappingViewport,			// ビューポート（前段レンダリングターゲット）
			envMappingTypeMask		= 0xFF,
			envMappingRefraction	= 0x0100,	// 屈折反映ターゲット
			envMappingViewportDepth	= 0x0200,	// ビューポート深度
		} ;
		struct	EnvMappingParam
		{
			SGLImageObject *	pImage ;
			uint32_t			typeMap ;
			S3DMatrix			matMap ;
		} ;
		virtual void SetEnvironmentMappingImage
					( SGLImageObject * pImage, uint32_t nFlags ) = 0 ;
		// グローバル環境マッピング変換行列設定
		virtual void SetEnvironmentMappingMatrix( const S3DMatrix& matMapping ) = 0 ;
		// 輪郭描画色設定
		typedef	S3DRenderBufferInterface::OffsetBorderParam	OffsetBorderParam ;
		virtual void SetOffsetBorderColor( uint32_t rgbBorder ) = 0 ;
		// 輪郭描画オフセット係数設定 (ax+b)
		virtual void SetOffsetBorderCoefficient( float32_t a, float32_t b ) = 0 ;
		// オプショナル機能設定
		typedef	S3DRenderBufferInterface::FeatureType	FeatureType ;
		virtual SGLError SetOptionalFeature
			( FeatureType feature, int32_t nParam1,
						const void * pParam2, size_t sizeOfParam2 ) = 0 ;
		// オプショナル機能取得
		virtual SGLError GetOptionalFeature
			( FeatureType feature, int32_t nParam1,
						void * pParam2, size_t sizeOfParam2 ) const = 0 ;

	public:
		// 対応機能取得
		virtual void GetRenderingCapacity( S3DRenderingCapacity& caps ) = 0 ;
		// 選択中の立体視用バッファ取得
		virtual StereoViewIndex CurrentParallaxView( void ) = 0 ;
		// 立体視用バッファ選択
		virtual SGLError SelectParallaxView( StereoViewIndex sviView ) = 0 ;
		// 内部バッファサイズ設定
		virtual SGLError SetRenderingBufferSize( uint32_t countVertex ) = 0 ;
		// 3D レンダリング用バッファ・インターフェース開始
		virtual SGLError Begin3DRenderer( uint64_t nFlags = 0 ) = 0 ;
		// 3D レンダリング用バッファ・インターフェース終了
		virtual SGLError End3DRenderer( uint64_t nFlags = 0 ) = 0 ;
		// 非同期レンダリング開始
		enum	AsyncFlushFlag
		{
			asyncFlushCommitTarget	= 0x0001,	// レンダーターゲットへの反映処理
												// （システムメモリへの転送が必要な場合）を行う
		} ;
		virtual SGLError AsyncFlush
			( uint32_t nFlags = 0, SSystem::SSignalEvent * pSignal = NULL ) = 0 ;
		// 非同期レンダリング完了待機
		virtual SGLError WaitFlush( int64_t msecTimeout = SSystem::SSynchronism::Infinite ) = 0 ;
		// 非同期レンダリングに適したスレッドで実行
		typedef void (*PROCEDURE_RENDERING)( void * pInstance ) ;
		virtual void SuitableProcedure
					( PROCEDURE_RENDERING pfnRendering, void * pInstance ) = 0 ;

	public:
		#if	defined(__COTOPHA__)
		// RenderContext 取得
		virtual RenderContext * GetRenderContextObject( void ) const ;
		#else
		// ハードウェア描画オブジェクト取得
		enum	RenderDeviceObjectFlag
		{
			deviceNeedsOnThread	= 0x0001,
		} ;
		virtual S3DRenderDevice * GetRenderDeviceObject( uint64_t nFlags = 0 ) ;
		// ハードウェア描画オブジェクト変更
		virtual SGLError SetRenderDeviceObject
				( S3DRenderDevice * pDevice, uint64_t nFlags = 0 ) ;
		#endif
	} ;

	#if	!defined(__COTOPHA__)
	typedef	S3DRenderContextInterface	RenderContext ;
	#endif


	//////////////////////////////////////////////////////////////////////////
	// VertexBuffer - S3DVertexBufferInterface インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	S3DVertexBuffer	: public S3DVertexBufferInterface
	{
	protected:
		// 接続オブジェクト
		VertexBuffer *		m_buffer ;
		bool				m_flagOwner ;

		SSystem::SObjectArray<ESLObject>	m_arrayTemporary ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DVertexBuffer, S3DVertexBufferInterface )
		// 構築関数
		S3DVertexBuffer( void ) ;
		S3DVertexBuffer( VertexBuffer * buffer, bool flagOwner ) ;
		// 消滅関数
		virtual ~S3DVertexBuffer( void ) ;
		// 関連付け
		void AttachVertexBuffer( VertexBuffer * buffer, bool flagOwner ) ;
		// VertexBuffer 取得
		VertexBuffer * GetVertexBuffer( void ) const
		{
			return	m_buffer ;
		}
		operator VertexBuffer * ( void ) const
		{
			return	m_buffer ;
		}
		VertexBuffer * operator -> ( void ) const
		{
			return	m_buffer ;
		}

	public:
		// シェーディング設定
		virtual void SetShadingFlag( uint64_t nShadingMethod ) ;
		// シェーディング取得
		virtual uint64_t GetShadingFlag( void ) ;
		// カスタムシェーダー設定
		virtual SGLError AttachCustomShader( S3DCustomShader * pShader ) ;
		// カスタムシェーダー取得
		virtual S3DCustomShader * GetCustomShader( void ) const ;
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

	public:
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

	public:
		// VertexBuffer 取得
		virtual VertexBuffer * GetVertexBufferObject( void ) const ;
		// S3DVertexDeviceBufferInterface 取得
		virtual S3DVertexDeviceBufferInterface *
			GetDeviceBufferTypeOf( SGLImageBufferObjectType type ) ;
		virtual S3DVertexDeviceBufferInterface *
			GetDeviceBufferAs( const ESLRuntimeClass& rtClass ) ;
		// S3DVertexDeviceBufferInterface 追加
		virtual void AttachDeviceBuffer( S3DVertexDeviceBufferInterface * pDevBuf ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// RenderContext - S3DRenderContextInterface インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	S3DRenderContext : public S3DRenderContextInterface
	{
	protected:
		// 接続オブジェクト
		RenderContext *		m_render ;
		bool				m_flagOwner ;

		// 描画先情報
		SGLImage			m_imgTarget ;
		SGLImage			m_imgZBuffer ;
		SGLImageObject *	m_pTarget ;
		SGLImageObject *	m_pZBuffer ;

		#if	defined(__COTOPHA__)
		SSystem::SObjectArray<ESLObject>	m_arrayTemporary ;
		#endif

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DRenderContext, S3DRenderContextInterface )
		// 構築関数
		S3DRenderContext( void ) ;
		S3DRenderContext( RenderContext * render, bool flagOwner ) ;
		// 消滅関数
		virtual ~S3DRenderContext( void ) ;
		// 関連付け
		void AttachRenderContext( RenderContext * render, bool flagOwner ) ;
		// RenderContext 取得
		RenderContext * GetRenderContext( void ) const
		{
			return	m_render ;
		}
		operator RenderContext * ( void ) const
		{
			return	m_render ;
		}
		RenderContext * operator -> ( void ) const
		{
			return	m_render ;
		}
		// PaintContext 取得
		PaintContext * GetPaintContext( void ) const
		{
			return	m_render ;
		}
		operator PaintContext * ( void ) const
		{
			return	m_render ;
		}

	public:	// SGLPaintContextInterface オーバーライド
		// 描画先取得
		virtual SGLImageObject * GetTargetImage( void ) ;
		virtual SGLImageObject * GetTargetZBuffer( void ) ;
		// ビューポート取得
		virtual SGLError GetViewPort( SGLImageRect & rctView ) const ;
		// 描画先設定
		virtual SGLError AttachTargetImage
			( SGLImageObject * pImage,
					SGLImageObject * pZBuffer,
					const SGLImageRect * pView = NULL ) ;
		// 描画先解除
		virtual SGLError DetachTargetImage( void ) ;
		// 描画座標空間設定
		virtual SGLError AppendTransformation
			( const SGLAffine & af, unsigned int nTransparency ) ;
		virtual SGLError SetTransformation
			( const SGLAffine & af, unsigned int nTransparency ) ;
		virtual SGLError CurrentAffine( SGLAffine & af ) ;
		virtual unsigned int CurrentTransparency( void ) ;
		virtual SGLError PushTransformation( void ) ;
		virtual SGLError PopTransformation( void ) ;
		virtual SGLError ResetTransformation( void ) ;
		// 描画デフォルトフラグ
		virtual void SetPaintFlags( int64_t nFlags ) ;
		virtual int64_t GetPaintFlags( void ) ;
		// 描画先クリア
		virtual SGLError FillClearTarget
				( uint32_t argb = 0xff000000, int64_t flags = 0 ) ;
		// 形状描画
		virtual SGLError FillRectangle
			( int x, int y, int width, int height,
				uint32_t argb, double z = 0.0, uint32_t flags = 0 ) ;
		virtual SGLError FillPolygon
			( const S2DVector * vertices, size_t count,
				uint32_t argb, double z = 0.0, uint32_t flags = 0 ) ;
		// 画像描画
		virtual SGLError DrawImage
			( const SGLPaintParam & ppPaint,
				SGLImageObject * pSrcImage,
				const SGLImageRect * pSrcClip = NULL ) ;
		// ２Ｄメッシュ描画
		virtual SGLError DrawMesh
			( const S2DVector * pDstMesh,
				const S2DVector * pSrcMesh,
				size_t widthMesh, size_t heightMesh,
				const SGLPaintParam & ppPaint,
				SGLImageObject * pSrcImage,
				const SGLImageRect * pSrcClip = NULL ) ;
		// 複数画像描画
		virtual SGLError DrawMultiImages
			( size_t nCount,
				const SGLPaintParam * pParams,
				SGLImageObject *const* ppSrcImages,
				const SGLImageRect * pSrcClips = NULL ) ;
		// 描画の確定
		virtual SGLError Flush( void ) ;
		virtual SGLError Finish( void ) ;

	public:	// S3DRenderBufferInterface オーバーライド
		// バッファ複製
		virtual SGLError CopyBufferFrom
			( S3DRenderContextInterface& renderSrc, uint32_t nFlags = 0,
				int xDst = 0, int yDst = 0, const SGLImageRect * pSrcRect = nullptr ) ;
		// マルチターゲット（2つ目以降）描画先設定
		virtual SGLError AttachMultiTargetImages
			( SGLImageObject*const* ppTargets, size_t nCount ) ;
		// マルチターゲット（2つ目以降）取得
		virtual SGLImageObject*const* GetMultiTargetImages( size_t& nCount ) const ;
		// カスタムシェーダー設定
		virtual SGLError AttachCustomShader( S3DCustomShader * pShader ) ;
		// カスタムシェーダー取得
		virtual S3DCustomShader * GetCustomShader( void ) const ;
		// カスタムシェーダーパラメータ設定
		virtual SGLError SetCustomShaderUniform
			( const wchar_t * pwszUniformId,
					S3DCustomShader::UniformType type,
					const void * pData, size_t nCount ) ;
		virtual SGLError ResetCustomShaderUniform( void ) ;
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
		// 頂点バッファの内容を描画
		virtual SGLError AddVertexBuffer
			( S3DMaterial * pMaterial, uint32_t nFlags,
				S3DVertexBufferInterface * pBuffer,
				size_t iFirst = 0, ssize_t iEnd = -1,
				size_t nInstancing = 0,
				const S4DMatrix * pmatInstancing = NULL,
				const S3DColor * pColorInstancing = NULL ) ;
		// 遅延削除オブジェクト追加（Flush 時に削除）
		virtual void AddTemporaryObject( ESLObject * pObj ) ;

	public:	// S3DRenderContextInterface オーバーライド
		// 投影スクリーン座標設定
		virtual SGLError SetProjectionScreen
			( const S3DVector& vScreen,
				double zScale = 1.0, double fpPixelAspect = 1.0 ) ;
		// 投影スクリーン座標取得
		virtual SGLError GetProjectionScreen
			( S3DVector& vScreen,
				double& zScale, double& fpPixelAspect ) const ;
		// 透視変換行列取得
		virtual bool GetPerspectiveMatrix( S4DMatrix& matPars ) const ;
		// 透視変換行列設定
		virtual void SetPerspectiveMatrix
			( StereoViewIndex sviView,
				const S4DMatrix& matPers, bool fPersMatrix = true ) ;
		virtual void EnablePerspectiveMatrix( bool fPersMatrix ) ;
		// カメラ設定
		virtual void SetCamera
			( const S3DDMatrix& matCamera,
					const S3DDVector& posCamera ) ;
		// カメラ取得
		virtual void GetCamera
			( S3DDMatrix& matCamera, S3DDVector& posCamera ) const ;
		// 球（ローカル座標）が視界に収まるか判定する
		virtual bool IsSphereIntoView( const S3DDVector& vPos, double radius ) const ;
		// 立体視視差設定
		virtual void SetParallax
			( double xParallax, double zFocusRate, double xScreenDelta ) ;
		// 立体視視差取得
		virtual double GetParallax( void ) const ;
		// ｚクリップ範囲を設定
		virtual void SetZClipRange( double zMin, double zMax ) ;
		// 光源を設定
		virtual void SetLightEntries
			( const S3DLightEntry* pLights, size_t countLight ) ;
		// シャドウマップを設定
		virtual void SetShadowMap
			( uint32_t idLight,
				SGLImageObject* pShadowMapDepth,
				const S3DShadowMapInfo& infShadowMap,
				SGLImageObject* pShadowMapColor = NULL ) ;
		// 疑似フォッグを設定
		virtual void SetFog
			( uint32_t rgbFog, double zFogNear, double zFogFar ) ;
		virtual void EnableFog( bool fFog ) ;
		// シェーディング設定
		virtual void SetShadingFlag( uint64_t nShadingMethod ) ;
		// シェーディング取得
		virtual uint64_t GetShadingFlag( void ) ;
		// レイトレーシング設定
		virtual void SetRayTracingParameter
					( const S3DRenderRayTracingParam& rrtp ) ;
		// グローバル環境マッピング設定
		virtual void SetEnvironmentMappingImage
					( SGLImageObject * pImage, uint32_t nFlags ) ;
		// グローバル環境マッピング変換行列設定
		virtual void SetEnvironmentMappingMatrix( const S3DMatrix& matMapping ) ;
		// 輪郭描画色設定
		virtual void SetOffsetBorderColor( uint32_t rgbBorder ) ;
		// 輪郭描画オフセット係数設定
		virtual void SetOffsetBorderCoefficient( float32_t a, float32_t b ) ;
		// オプショナル機能設定
		virtual SGLError SetOptionalFeature
			( FeatureType feature, int32_t nParam1,
						const void * pParam2, size_t sizeOfParam2 ) ;
		// オプショナル機能取得
		virtual SGLError GetOptionalFeature
			( FeatureType feature, int32_t nParam1,
						void * pParam2, size_t sizeOfParam2 ) const ;
		// 対応機能取得
		virtual void GetRenderingCapacity( S3DRenderingCapacity& caps ) ;
		// 選択中の立体視用バッファ取得
		virtual StereoViewIndex CurrentParallaxView( void ) ;
		// 立体視用バッファ選択
		virtual SGLError SelectParallaxView( StereoViewIndex sviView ) ;
		// 内部バッファサイズ設定
		virtual SGLError SetRenderingBufferSize( uint32_t countVertex ) ;
		// 3D レンダリング用バッファ・インターフェース開始
		virtual SGLError Begin3DRenderer( uint64_t nFlags = 0 ) ;
		// 3D レンダリング用バッファ・インターフェース終了
		virtual SGLError End3DRenderer( uint64_t nFlags = 0 ) ;
		// 非同期レンダリング開始
		virtual SGLError AsyncFlush
			( uint32_t nFlags = 0, SSystem::SSignalEvent * pSignal = NULL ) ;
		// 非同期レンダリング完了待機
		virtual SGLError WaitFlush( int64_t msecTimeout = SSystem::SSynchronism::Infinite ) ;
		// 非同期レンダリングに適したスレッドで実行
		typedef void (*PROCEDURE_RENDERING)( void * pInstance ) ;
		virtual void SuitableProcedure
					( PROCEDURE_RENDERING pfnRendering, void * pInstance ) ;

	public:
		#if	defined(__COTOPHA__)
		// PaintContext 取得
		virtual PaintContext * GetPaintContextObject( void ) const ;
		// RenderContext 取得
		virtual RenderContext * GetRenderContextObject( void ) const ;
		#else
		// ハードウェア描画オブジェクト取得
		virtual S3DRenderDevice * GetRenderDeviceObject( uint64_t nFlags = 0 ) ;
		// ハードウェア描画オブジェクト変更
		virtual SGLError SetRenderDeviceObject
				( S3DRenderDevice * pDevice, uint64_t nFlags = 0 ) ;
		#endif
	} ;


}

#endif

