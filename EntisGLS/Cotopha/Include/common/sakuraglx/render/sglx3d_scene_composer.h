
#if	!defined(__SAKURAGLX3D_SCENE_COMPOSER_H__)
#define	__SAKURAGLX3D_SCENE_COMPOSER_H__	1

#include <rosetta/rosetta.h>
#include <antirrhinum/antirrhinum_kernel.h>
#include <sakuragl/sgl_media.h>
#include <sakuraglx/render/sglx3d_scene.h>
#include <sakuraglx/render/sglx_model_buffer.h>
#include <sakuragl/sgl_erisa_lib.h>


#define	S3D_DECLARE_COMPOSER_ITEM(class_name,class_id)	\
	static const S3DSceneComposer::ItemClassDescriptor	m_ItemClassDescriptor ;

#define	S3D_IMPLEMENT_COMPOSER_ITEM(class_name,class_id)	\
	const S3DSceneComposer::ItemClassDescriptor	\
		class_name::m_ItemClassDescriptor = {	\
			L###class_id, &ESL_RUNTIME_CLASS(class_name)	\
		} ;

#define	S3D_COMPOSER_ITEM_DESCRIPTOR(class_name)	&(class_name::m_ItemClassDescriptor)

namespace	SakuraGL
{
	class	S3DCompositionManager ;
	class	S3DCompositionEditorInterface ;
	class	S3DSceneComposerPluginInstance ;
	struct	S3DSceneComposerPluginSceneInfo ;

	//////////////////////////////////////////////////////////////////////////
	// シーン・コンポーザー
	//////////////////////////////////////////////////////////////////////////

	class	S3DSceneComposer	: public SSystem::SObject
	{
	public:
		// 標準的なリソース種別
		enum	ResourceType
		{
			resourceTypeInvalid	= -1,
			resourceTypeFolder,			// "folder", NULL
			resourceTypeImage,			// "image", SGLImageObject (*.eri;*.png;*.bmp;...)
			resourceTypeAudio,			// "audio", SGLAudioPlayer (*.mio;*.wav;...)
			resourceTypeMovie,			// "movie", SGLMediaPlayer (*.mei;*.avi;...)
			resourceTypeModel,			// "model", S3DModelBuffer (*.mdfx)
			resourceTypePose,			// "pose_lib", S3DModelPoseLibrary (*.psfx;*.xmlpsf)
			resourceTypeMaterial,		// "material_lib", S3DMaterialLibrary (*.xmlsmt)
			resourceTypeShaderDef,		// "shader_def", UserShader (*.shddsc)
			resourceTypeBasicForm,		// "basic_form", SGLBasicFormParser (*.xmlfrm)
			resourceTypeScript,			// "script_rosetta", SStringParser (*.rs)
			resourceTypeAntirrhinum,	// "script_antirrhinum", AGLModulePtr (*.xmlagl)
			resourceTypeBinary,			// "binary", SByteBuffer
			resourceTypeLoquaty,		// "script_loquaty", LSourceFilePtrObj (*.lqs)
			resourceTypeCount,
		} ;
		static const wchar_t * const	m_pwszResourceType[resourceTypeCount] ;

		static ResourceType GetResourceType( const wchar_t * pwszType ) ;

		// プロシージャル・リソース・インターフェース
		class	ResourceAssets ;
		class	ResourceProcedure	: public ESLObject
		{
		public:
			ESL_DECLARE_CLASS_INFO( ResourceProcedure, ESLObject )
			// S3DSceneComposer 関連付け
			virtual void AttachSceneComposer( S3DSceneComposer * pComposer ) = 0 ;
			// デシリアライズ
			virtual SGLError ParseParameter( const SSystem::SXMLDocument& xmlProc ) = 0 ;
			// シリアライズ
			virtual SGLError FormatParameter( SSystem::SXMLDocument& xmlProc ) = 0 ;
			// リソース生成
			virtual SGLError CreateResource
				( ESLObject*& pRsrc, ResourceAssets& assets ) = 0 ;
		} ;
		class	ImageRsrcProcedure	: public ResourceProcedure
		{
		public:
			ESL_DECLARE_CLASS_INFO( ImageRsrcProcedure, ResourceProcedure )
			// 参照元画像一覧
			virtual SGLError CollectReferenceSubImages
				( ResourceAssets& assets,
					SSystem::SPointerArray<SGLImageBuffer>& aSubImages ) = 0 ;
		} ;

		// ユーザー・シェーダ―
		class	UserShader	: public S3DRenderDevice::ShaderDescriptor
		{
		public:
			class	NotifyContainer
					: public ESLObject, public S3DRenderDevice::Notify
			{
			protected:
				UserShader *		m_pUserShader ;
				S3DRenderDevice *	m_pDevice ;
				S3DCustomShader *	m_pShader ;
			public:
				ESL_DECLARE_CLASS_INFO( NotifyContainer, ESLObject )
				NotifyContainer
					( UserShader * pUserShader,
						S3DRenderDevice * pDevice, S3DCustomShader * pShader = NULL )
					: m_pUserShader( pUserShader ),
						m_pDevice( pDevice ), m_pShader( pShader ) { }
				virtual ~NotifyContainer( void )
				{
					m_pDevice->DetachNotifyObject( this ) ;
				}
				virtual void OnReleaseDevice( S3DRenderDevice * pDev )
				{
					m_pUserShader->OnReleaseDevice( pDev ) ;
				}
				S3DCustomShader * GetShader( void ) const
				{
					return	m_pShader ;
				}
			} ;

		protected:
			SSystem::SCriticalSection	m_csSync ;
			SSystem::SString			m_idShader ;
			SSystem::SPtrSortObjectArray
				<S3DRenderDevice,NotifyContainer>
										m_psoaShaders ;

		public:
			ESL_DECLARE_CLASS_INFO( UserShader, ShaderDescriptor )
			// 構築関数
			UserShader( const wchar_t * pwszShaderID ) ;
			// 消滅関数
			virtual ~UserShader( void ) ;
			// シェーダ―コンパイル／コンパイル済み取得
			S3DCustomShader * LoadShaderFor
				( S3DRenderDevice * pDev,
					SSystem::SString * pstrErrMsg = NULL,
					SSystem::SString * pstrErrSource = NULL ) ;
			// シェーダ―参照削除
			void ReleaseAllShaderRef( void ) ;

		public:
			// デバイスの削除前に呼び出される
			virtual void OnReleaseDevice( S3DRenderDevice * pDev ) ;
		} ;

		// Antirrhinum モジュール・マネージャ
		class	AGLSubManager	: public AntirrhinumGL::AGLModuleManager
		{
		protected:
			S3DSceneComposer *						m_pComposer ;
			SSystem::SReferenceArray
				<AntirrhinumGL::AGLModuleManager>	m_refManager ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( AGLSubManager, AGLModuleManager )
			// 構築関数
			AGLSubManager( void ) ;
			// S3DSceneComposer 関連付け
			void AttachSceneComposer( S3DSceneComposer * pComposer ) ;
			S3DSceneComposer * GetSceneComposer( void ) const ;
			// 参照マネージャー追加
			void AddReferenceManager( AntirrhinumGL::AGLModuleManager * pManager ) ;
			// 参照マネージャー削除
			void DetachReferenceManager( AntirrhinumGL::AGLModuleManager * pManager ) ;
		public:
			// 読み込み済みスクリプト取得
			//（取得に成功した場合には UnloadScript 呼び出しで参照解放）
			virtual AntirrhinumGL::AGLModule * GetLoadedScript( const wchar_t * pwszFileName ) ;
			// スクリプト解放
			virtual SSystem::SError UnloadScript( AntirrhinumGL::AGLModule * pModule ) ;
			// ファイルを開く
			virtual SSystem::SFileInterface *
						OpenScriptFile( const wchar_t * pwszFileName ) ;
		} ;

		// リソースコンテナ
		class	ResourceContainer	: public SSystem::SSyncReference
		{
		protected:
			SSystem::SString		m_strType ;
			SSystem::SString		m_strSrcFile ;
			SSystem::SString		m_strTreePath ;	// リソースをエディタ上で表示する親フォルダのパス
													// m_strType=="folder" の時には、そのフォルダ自体のパス
			SSystem::SXMLDocument	m_xmlOptions ;	// <options> タグ
			ResourceProcedure *		m_pRsrcProc ;
			bool					m_flagPending ;
			ESLObject *				m_pAppData ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( ResourceContainer, SSyncReference )
			// 構築関数
			ResourceContainer( void ) ;
			ResourceContainer( const ResourceContainer& rsrc ) ;
			ResourceContainer
				( SObject * pRef, const wchar_t * pwszType,
					const wchar_t * pwszSrcFile = NULL,
					const wchar_t * pwszTreePath = NULL,
					const SSystem::SXMLDocument * pxmlOptions = NULL ) ;
			// 消滅関数
			~ResourceContainer( void ) ;
			// 種別
			const SSystem::SString& GetType( void ) const
			{
				return	m_strType ;
			}
			bool IsFolder( void ) const
			{
				return	m_strType == m_pwszResourceType[resourceTypeFolder] ;
			}
			bool IsResourceType( ResourceType type ) const ;
			void SetType( const wchar_t * pwszType ) ;
			// ソースファイル名
			const SSystem::SString& GetSourceFile( void ) const
			{
				return	m_strSrcFile ;
			}
			void SetSourceFile( const wchar_t * pwszSrcFile ) ;
			// ツリーパス（編集用）
			const SSystem::SString& GetTreePath( void ) const
			{
				return	m_strTreePath ;
			}
			void SetTreePath( const wchar_t * pwszPath ) ;
			// リソース読み込みオプション
			SSystem::SXMLDocument& Options( void )
			{
				return	m_xmlOptions ;
			}
			const SSystem::SXMLDocument& GetOptions( void ) const
			{
				return	m_xmlOptions ;
			}
			// リソースデータ取得
			template <class T> T * GetResource( void ) const
			{
				return	ESLTypeCast<T>( GetReference() ) ;
			}
			// プロシージャル
			void SetProcedure( ResourceProcedure * pProc ) ;
			ResourceProcedure * GetProcedure( void ) const ;
			bool CreateProceduralRsrc( ResourceAssets& assets ) ;
			bool RecreateProceduralRsrc( ResourceAssets& assets ) ;
			bool IsPendingProceduralRsrc( void ) const ;
			void SaveProceduralOption( void ) ;
			// アプリケーションデータ
			ESLObject * GetApplicationData( void ) const
			{
				return	m_pAppData ;
			}
			void SetApplicationData( ESLObject * pAppData ) ;
		} ;

		// リソース資産
		class	ResourceAssets	: public ESLObject
		{
		protected:
			SSystem::SStrSortObjectArray<ResourceContainer>	m_ssoaResources ;
			SSystem::SPointerArray<ResourceAssets>	m_arrRefAssets ;
			S3DModelPoseLibrary						m_libPose ;
			S3DTextureLibrary						m_libTexture ;
			S3DMaterialLibrary						m_libMaterial ;
			AGLSubManager							m_aglManager ;
			ESLObject *								m_pAppData ;

		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( ResourceAssets, ESLObject )
			// 構築関数
			ResourceAssets( void ) ;
			// 消滅関数
			virtual ~ResourceAssets( void ) ;
			// 全リソース削除
			void Release( void ) ;
			// S3DSceneComposer 関連付け
			void AttachSceneComposer( S3DSceneComposer * pComposer ) ;
			S3DSceneComposer * GetSceneComposer( void ) const ;
			// 参照キャッシュ追加
			void AddReferenceAssets( ResourceAssets * pRef ) ;
			// 参照キャッシュ削除
			void DetachReferenceAssets( ResourceAssets * pRef ) ;
			// 参照キャッシュ全削除
			void ClearAllReferenceAssets( void ) ;
			// 特定クラスリソースID列挙
			void EnumerateResourceIDsAs
				( SSystem::SObjectArray<SSystem::SString>& aStrSet,
					const ESLRuntimeClass& rtClass, bool flagRefAsserts = true ) const ;
			// 特定タイプリソースID列挙
			void EnumerateResourceIDsAs
				( SSystem::SObjectArray<SSystem::SString>& aStrSet,
					ResourceType rsrcType, bool flagRefAsserts = true ) const ;
			// テクスチャ列挙
			void EnumerateTextureStringSet( SSystem::SStringArray& aStrSet ) const ;
			static void EnumerateTextureStringSetOf
				( SSystem::SStringArray& aStrSet,
							const S3DTextureLibrary * pTextureLib ) ;
			// マテリアル列挙
			void EnumerateMaterialStringSet
				( SSystem::SStringArray& aStrSet ) const ;
			static void EnumerateMaterialStringSetOf
				( SSystem::SStringArray& aStrSet,
							const S3DMaterialLibrary * pMaterialLib ) ;
			// ポーズ列挙
			void EnumeratePoseStringSet
				( SSystem::SStringArray& aStrSet ) const ;
			static void EnumeratePoseStringSetOf
				( SSystem::SStringArray& aStrSet,
								const S3DModelPoseLibrary *	pPoseLib ) ;
			// スクリプトラベル列挙
			void EnumerateScriptLabels
				( SSystem::SObjectArray<SSystem::SString>& aStrSet,
					bool flagRefAsserts = true ) const ;
			// 文字列配列のソートと重複要素の削除
			static void SortStringSet( SSystem::SStringArray& aStrSet ) ;

		public:
			// モデルリソース
			virtual S3DModelBuffer *
						GetModelAs( const wchar_t * pwszID ) const ;
			// 画像リソース
			virtual SGLImageObject *
						GetImageAs( const wchar_t * pwszID ) const ;
			// オーディオリソース
			virtual SGLAudioPlayer *
						GetAudioAs( const wchar_t * pwszID ) const ;
			// 動画リソース
			virtual SGLMediaPlayer *
						GetMovieAs( const wchar_t * pwszID ) const ;
			// ポーズライブラリ
			virtual S3DModelPoseLibrary *
				GetPoseLibraryAs( const wchar_t * pwszID ) const ;
			// マテリアルライブラリ
			virtual S3DMaterialLibrary *
				GetMaterialLibraryAs( const wchar_t * pwszID ) const ;
			// シェーダーリソース
			virtual UserShader *
				GetUserShaderAs( const wchar_t * pwszID ) const ;
			// フォームリソース
			virtual SGLBasicFormParser *
				GetBasicFormAs( const wchar_t * pwszID ) const ;
			// バイナリリソース
			virtual SSystem::SByteBuffer *
				GetBinaryResourceAs( const wchar_t * pwszID ) const ;
			// リソースコンテナ
			virtual ResourceContainer *
					GetResourceContainerAs( const wchar_t * pwszID ) const ;
			// リソース逆引き
			virtual const wchar_t *
				GetContainerIdentityOf( ResourceContainer * prcTarget ) const ;
			virtual const wchar_t *
				GetResourceIdentityOf( ESLObject * pObject ) const ;
			// リソース検索
			ssize_t FindResourceFile( const wchar_t * pwszFilePath, size_t iFirst = 0 ) const ;
			// リソースIDを使用可能な文字でかつ重複しないように正規化
			void NormalizeResourceID( SSystem::SString& strID ) const ;
			// リソース列挙
			size_t GetResourceCount( void ) const ;
			ResourceContainer * GetResourceAt( size_t i ) const ;
			const wchar_t * GetResourceIdentityAt( size_t i ) const ;
			// リソース参照更新
			void UpdateAllResourceReference( void ) ;

		public:
			// 画像バッファと参照元画像バッファをリストする
			SGLError CollectReferenceSubImage
				( SSystem::SPointerArray<SGLImageBuffer>& aSubImages,
										const wchar_t * pwszImageID ) ;
			// 参照元の画像バッファの参照を更新する
			static void UpdateImageReference
				( SGLImageBuffer * pAtlasImageBuf,
					SSystem::SPointerArray<SGLImageBuffer>& aSubImages ) ;

		public:
			// ポーズライブラリ（参照ハブ）
			S3DModelPoseLibrary& PoseLibrary( void ) ;
			const S3DModelPoseLibrary& GetPoseLibrary( void ) const ;
			// テクスチャライブラリ
			S3DTextureLibrary& TextureLibrary( void ) ;
			const S3DTextureLibrary& GetTextureLibrary( void ) const ;
			// マテリアルライブラリ（参照ハブ）
			S3DMaterialLibrary& MaterialLibrary( void ) ;
			const S3DMaterialLibrary& GetMaterialLibrary( void ) const ;
			// Antirrhinum モジュール・マネージャー
			AGLSubManager& AntirrhinumManager( void ) ;
			const AGLSubManager& GetAntirrhinumManager( void ) const ;

		public:
			// アプリケーションデータ
			ESLObject * GetApplicationData( void ) const
			{
				return	m_pAppData ;
			}
			void SetApplicationData( ESLObject * pAppData ) ;

		public:
			// リソース登録
			SGLError RegisterResource
				( const wchar_t * pwszID, SObject * pRsrc,
					const wchar_t * pwszType,
					const wchar_t * pwszSrcFile = NULL,
					const wchar_t * pwszPath = NULL,
					const SSystem::SXMLDocument * pxmlOptions = NULL ) ;
			// リソース追加
			SGLError AddResourceAs
				( const wchar_t * pwszID, ESLObject * pRsrc,
					const wchar_t * pwszType,
					const wchar_t * pwszSrcFile = NULL,
					const wchar_t * pwszPath = NULL,
					const SSystem::SXMLDocument * pxmlOptions = NULL ) ;
			// リソース削除
			SGLError RemoveResourceAs( const wchar_t * pwszID ) ;
			// リソースID変更
			SGLError ChangeResourceID
				( const wchar_t * pwszID, ResourceContainer * pRsrc ) ;
			// リソース実体の差し替え
			SGLError ReloadResource
				( ResourceContainer * pRsrc, ESLObject * pData ) ;

		public:
			// リソースの追加時の処理
			virtual void OnAddResource
				( const wchar_t * pwszID, ESLObject * pRsrc ) ;
			// リソース削除時の処理
			virtual void OnRemoveResource
				( const wchar_t * pwszID, ESLObject * pRsrc ) ;

			friend class S3DSceneComposer ;
		} ;

		// コンポジション情報
		class	CompositionInfo	: public SObject
		{
		protected:
			// 表示設定
			SGLSize						m_sizeScreen ;
			S3DScene::ProjectionParam	m_projParam ;
			SSystem::SString			m_strDefCamera ;
			// 背景色
			SGLPalette					m_rgbaBack ;
			bool						m_fFillBack ;
			// 有効表示距離
			double						m_fpVisibleNearDistance ;
			double						m_fpVisibleFarDistance ;
			// 大域フォッグ
			bool						m_fEnableFog ;
			S3DScene::FogParam			m_fogParam ;
			// 輪郭線
			S3DScene::OffsetBorderParam	m_borderParam ;
			// アニメーション
			uint64_t					m_framesTotal ;
			uint32_t					m_framesPerSec ;
			uint32_t					m_framesScale ;
			// アイテム
			SSystem::SXMLDocument		m_xmlComposition ;	// <space>
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( CompositionInfo, SObject )
			// 構築関数
			CompositionInfo( void ) ;
			CompositionInfo( const CompositionInfo& ci ) ;
			// 消滅関数
			virtual ~CompositionInfo( void ) ;
		public:
			// 読み込み
			SGLError ParseComposition( const SSystem::SXMLDocument& xmlTag ) ;
			// 保存
			SGLError FormatComposition( SSystem::SXMLDocument& xmlTag ) ;
		public:
			// 画面サイズ設定
			const SGLSize& GetScreenSize( void ) const ;
			void SetScreenSize( const SGLSize& sizeScreen ) ;
			// 透視変換設定
			const S3DScene::ProjectionParam& GetProjectionParam( void ) const ;
			void SetProjectionParam( const S3DScene::ProjectionParam& projParam ) ;
			// 透視変換設定を適用
			void ApplyProjection
				( S3DScene& scene, const SGLSize& sizeView ) const ;
			// デフォルトカメラ
			const SSystem::SString& GetDefaultCameraID( void ) const ;
			void SetDefaultCameraID( const wchar_t * pwszCamera ) ;
			// 背景色
			bool IsEnabledFillBack( void ) const ;
			const SGLPalette& GetFillBack( void ) const ;
			void SetFillBack( const SGLPalette& rgbaBack, bool fFillBack ) ;
			// 有効表示距離
			double GetVisibleNearDistance( void ) const ;
			double GetVisibleFarDistance( void ) const ;
			void SetVisibleDistance( double fpNear, double fpFar ) ;
			// 大域フォッグ
			bool IsEnabledFog( void ) const ;
			const S3DScene::FogParam& GetFog( void ) const ;
			void SetFog( const S3DScene::FogParam& fogParam, bool fFog ) ;
			// 輪郭線
			const S3DScene::OffsetBorderParam& GetOffsetBorder( void ) const ;
			void SetOffsetBorder( const S3DScene::OffsetBorderParam& param ) ;
			// アニメーション
			uint64_t GetTotalFrameCount( void ) const ;
			uint32_t GetFramesPerSecond( void ) const ;
			uint32_t GetFrameRateScale( void ) const ;
			void SetTotalFrameCount( uint64_t nFrames ) ;
			void SetFrameRate( uint32_t framesPerSec, uint32_t framesScale ) ;
			double FrameIndexToSecond( double frame ) const ;
			double FrameIndexFromSecond( double sec ) const ;
			// 表示設定を適用
			void ApplyAllParameters
				( S3DScene& scene, const SGLSize& sizeView ) const ;
			// コンポジション
			const SSystem::SXMLDocument& GetComposition( void ) const ;
			SSystem::SXMLDocument& EditComposition( void ) ;
		} ;

		// パラメータ型
		enum	ParameterType
		{
			typeInvalid	= -1,
			typeMatrix,		// 3x3 行列     : S3DDMatrix
			typePosition,	// 座標         : S3DDVector
			typeDirection,	// 方向ベクトル : S3DDVector
			typeZoom,		// 拡大率       : S3DDVector
			typeRotation,	// 回転行列     : S3DDMatrix
			typeColor,		// 色           : S3DDVector
			typeScalar,		// スカラ値     : double
			typeInteger,	// 整数値		: int32_t
			typeBoolean,	// 設定フラグ   : bool
			typeSelector,	// セレクタ     : wchar_t[]
			typeCommand,	// コマンド     : wchar_t[]
			typePose,		// ポーズ       : wchar_t[] | PoseInstance
			typeBinary,		// バイナリ     : wchar_t[] (base64)
			typeMatrix4,	// 4x4 行列     : S4DDMatrix
			typeVector4,	// 4Dベクトル   : S4DDVector
			typeVector2,	// 2Dベクトル   : S2DDVector
		} ;
		enum	ParameterAttribute
		{
			attrConstant			= 0x0001,	// 定数的なパラメータ
			attrNoLocalTransform	= 0x0002,	// 座標／ベクトルは自身のローカル空間ではなく親空間
			attrStringEnumeration	= 0x0004,	// 取りうる値（文字列）の列挙可能 EnumerateStringSet
			attrUIOnlyEnumeration	= 0x0008,	// UI では列挙値のみ選択可能
			attrDynamicValidation	= 0x0010,	// 値の変化で有効なパラメータ集合が変化する
			attrFlagSetInteger		= 0x0020,	// フラグ集合の整数型である
			attrUIScalarSlider		= 0x0040,	// 指定範囲の値のスライダ操作
			attrEditUpdateFrame		= 0x0080,	// 編集を反映するにはフレーム全体の更新が必要
			attrEditorCommand		= 0x0100,	// 編集ツール用コマンド
			attrGlobalTransform		= 0x0200,	// 座標は常にグローバル空間
			attrReadOnlyParam		= 0x0400,	// 読み取り専用・記録対象外
			attrNoSerializeFlags	= attrEditorCommand | attrReadOnlyParam,
			attrCategory1			= 0x1000,
			attrCategory2			= 0x2000,
			attrCategory3			= 0x3000,
			attrCategory4			= 0x4000,
			attrCategory5			= 0x5000,
			attrCategory6			= 0x6000,
			attrCategory7			= 0x7000,
			attrCategoryMask		= 0xF000,
			attrCategoryShift		= 12,
			attrConstant1			= attrConstant | attrCategory1,
			attrConstant2			= attrConstant | attrCategory2,
			attrConstant3			= attrConstant | attrCategory3,
			attrConstant4			= attrConstant | attrCategory4,
			attrConstant5			= attrConstant | attrCategory5,
			attrConstant6			= attrConstant | attrCategory6,
			attrConstant7			= attrConstant | attrCategory7,
			attrAppExtension1		= 0x01000000,
			attrAppExtension2		= 0x02000000,
			attrAppExtension3		= 0x04000000,
			attrAppExtension4		= 0x08000000,
			attrAppExtension5		= 0x10000000,
			attrAppExtension6		= 0x20000000,
			attrAppExtension7		= 0x40000000,
			attrAppExtension8		= 0x80000000,
		} ;

		// ポーズインスタンス
		struct	PoseInstance
		{
			S3DModelPose *	pPoseBase ;
			S3DModelPose *	pPoseTarget ;
			double			secBasePose ;
			double			secTargetPose ;
			double			fpTransition ;

			PoseInstance( void )
				: pPoseBase(NULL), pPoseTarget(NULL),
					secBasePose(0.0), secTargetPose(0.0), fpTransition(0.0) { }
			PoseInstance( const PoseInstance& pi )
				: pPoseBase(pi.pPoseBase),
					pPoseTarget(pi.pPoseTarget),
					secBasePose(pi.secBasePose),
					secTargetPose(pi.secTargetPose),
					fpTransition(pi.fpTransition) { }
			const PoseInstance& operator = ( const PoseInstance& pi )
			{
				pPoseBase = pi.pPoseBase ;
				pPoseTarget = pi.pPoseTarget ;
				secBasePose = pi.secBasePose ;
				secTargetPose = pi.secTargetPose ;
				fpTransition = pi.fpTransition ;
				return	*this ;
			}
		} ;

		// バイナリ共通ヘッダ
		struct	BinaryHeader
		{
			uint32_t	nType ;			// enum BinaryType
			uint32_t	nSubType ;
			uint32_t	nBodyBytes ;
			uint32_t	nReserved ;

			const void * GetBodyPtr( void ) const
			{
				return	((const uint8_t*) this) + sizeof(BinaryHeader) ;
			}
			void * GetBodyPtr( void )
			{
				return	((uint8_t*) this) + sizeof(BinaryHeader) ;
			}
		} ;
		enum	BinaryType
		{
			binaryInstancing	= 0x00000100,
			binaryMesh			= 0x00000200,
			binaryMeshEditor	= 0x00000210,
			binaryBezierCurve	= 0x00000300,
		} ;
		struct	BinaryInstancingEntry
		{
			S4DMatrix	matrix ;
			S3DColor	color ;
		} ;
		struct	BinaryInstancingData
		{
			uint32_t				count ;
			BinaryInstancingEntry	entries[1] ;
		} ;
		struct	BinaryPrimitiveEntry
		{
			uint32_t	addrData ;		// pointer to BinaryPrimitiveData
			uint32_t	nBytes ;
		} ;
		struct	BinaryMeshData
		{
			uint32_t				count ;
			BinaryPrimitiveEntry	entries[1] ;
		} ;
		enum	PrimitiveBufferType
		{
			primitiveBufferVertex,
			primitiveBufferNormal,
			primitiveBufferUV,
			primitiveBufferColor,
			primitiveBufferExtension,
		} ;
		struct	PrimitiveBufferEntry
		{
			uint32_t	nBufferType ;	// enum PrimitiveBufferType
			uint32_t	addrTypeID ;	// null-terminated utf-16
			uint32_t	addrBuffer ;
			uint32_t	nBufBytes ;
		} ;
		struct	BinaryPrimitiveData
		{
			uint32_t	nPrimitiveType ;	// enum S3DPrimitiveType
			uint32_t	addrPrimitiveName ;	// null-terminated utf-16
			uint32_t	nPrimitiveCount ;
			uint32_t	nVertexCount ;
			uint32_t	nIndexCount ;
			uint32_t	addrIndexBuffer ;	// uint32_t array
			uint32_t	nBufferCount ;
			PrimitiveBufferEntry	bufEntry[1] ;
		} ;
		struct	BinaryBezierCurvePoint
		{
			S3DVector	vPoint ;
			S3DVector	vHandle[2] ;
			S3DVector	vZoom ;
			float32_t	degGimbal ;
			SGLPalette	argbColor ;
			uint32_t	nExData ;
		} ;
		struct	BinaryBezierCurveData
		{
			uint32_t				count ;
			uint32_t				type ;
			uint32_t				reserved[2] ;
			BinaryBezierCurvePoint	points[1] ;
		} ;

		// バリアント
		class	Variant
		{
		protected:
			ParameterType	m_type ;
			uint8_t *		m_pData ;
			size_t			m_nBytes ;
			uint8_t			m_buf[16*8] ;
		public:
			// バリアント構築
			Variant( void ) ;
			Variant( const Variant& val ) ;
			Variant( const S3DDMatrix& mat3, ParameterType type = typeMatrix ) ;
			Variant( const S3DDVector& vec3, ParameterType type = typePosition ) ;
			Variant( const S3DDQuaternion& q, ParameterType type = typeRotation ) ;
			Variant( const SGLPalette& rgb, ParameterType type = typeColor ) ;
			Variant( bool b, ParameterType type = typeBoolean ) ;
			Variant( int32_t n, ParameterType type = typeInteger ) ;
			Variant( double s, ParameterType type = typeScalar ) ;
			Variant( const wchar_t * pwszCmd, ParameterType type = typeCommand ) ;
			Variant( const PoseInstance& pose ) ;
			Variant( const S4DDMatrix& mat4, ParameterType type = typeMatrix4 ) ;
			Variant( const S4DDVector& vec4, ParameterType type = typeVector4 ) ;
			Variant( const S2DDVector& vec2, ParameterType type = typeVector2 ) ;
			Variant( const void * pData, size_t nBytes, ParameterType type ) ;
			Variant( Rosetta::RSObject * pObj, ParameterType type = typeInvalid ) ;
			// 消滅
			~Variant( void ) ;
			// データ設定
			void SetData( const void * pData, size_t nBytes, ParameterType type ) ;
			bool SetFromRosetta( Rosetta::RSObject * pObj, ParameterType type = typeInvalid ) ;
			// バッファ確保
			uint8_t * PutBuffer( size_t nBytes, ParameterType type ) ;
			// 代入
			const Variant& operator = ( const Variant& val ) ;
			// クリア
			void Clear( void ) ;
			// 空判定
			bool IsEmpty( void ) const ;
			// 型取得
			ParameterType GetType( void ) const ;
			// データ取得
			const uint8_t * GetData( void ) const ;
			size_t GetDataBytes( void ) const ;
			const S3DDMatrix& GetMatrix( void ) const ;
			const S3DDVector& GetVector( void ) const ;
			double GetScalar( void ) const ;
			int32_t GetInteger( void ) const ;
			bool GetBoolean( void ) const ;
			const wchar_t * GetString( void ) const ;
			const PoseInstance& GetPose( void ) const ;
			const S4DDMatrix& GetMatrix4( void ) const ;
			const S4DDVector& GetVector4( void ) const ;
			const S2DDVector& GetVector2( void ) const ;
			bool GetToRosetta( Rosetta::RSObject * pObj ) const ;
			// シリアライズ
			SSystem::SString Serialize( void ) const ;
			// デシリアライズ
			bool Deserialize( const SSystem::SString& strValue ) ;
			// 解釈
			static bool ParseMatrix( S3DDMatrix& mat, const SSystem::SString& strValue ) ;
			static bool ParseVector( S3DDVector& vec, const SSystem::SString& strValue ) ;
			static bool ParseScalar( double& s, const SSystem::SString& strValue ) ;
			static bool ParseInteger( int32_t& n, const SSystem::SString& strValue ) ;
			static bool ParseBoolean( bool& b, const SSystem::SString& strValue ) ;
			static bool ParseVectorX( double * pVec, size_t nCount, const SSystem::SString& strValue ) ;
			// フォーマット
			static void FormatMatrix( SSystem::SString& strValue, const S3DDMatrix& mat ) ;
			static void FormatVector( SSystem::SString& strValue, const S3DDVector& vec ) ;
			static void FormatScalar( SSystem::SString& strValue, double s ) ;
			static void FormatInteger( SSystem::SString& strValue, int32_t n ) ;
			static void FormatBoolean( SSystem::SString& strValue, bool b ) ;
			static void FormatVectorX( SSystem::SString& strValue, const double * pVec, size_t nCount ) ;
			// S3DDVector -> SGLPalette 変換
			static SGLPalette ColorFromVector( const S3DDVector& vec ) ;
			// SGLPalette -> S3DDVector 変換
			static S3DDVector VectorFromColor( const SGLPalette& rgb ) ;
		} ;

		// OnExtendNotify コマンド
		static const wchar_t *	CmdInitializeItem ;	// "InitializeItem"
		static const wchar_t *	CmdFinishItem ;		// "FinishItem"
		static const wchar_t *	CmdStartItem ;		// "StartItem"
		static const wchar_t *	CmdStopItem ;		// "StopItem"

		// キーフレーム・パラメータ
		enum	KeyFrameFlag
		{
			keyframeCorner	= 0x0001,
		} ;
		struct	KeyFrameParam
		{
			int32_t		iFrame ;
			uint32_t	nFlags ;		// enum KeyFrameFlag 集合
			float32_t	speedIn ;
			float32_t	speedOut ;
		} ;

		static const SSystem::SXMLDocument::AttrInteger	m_aiKeyFrameFlags[2] ;

		// パラメータエントリ配列
		struct	ParamEntry
		{
			const wchar_t *	id ;		// 識別子
			ParameterType	type ;
			uint32_t		attr ;		// enum ParameterAttribute 集合
			const wchar_t *	name ;		// 項目名（UI表示用）
			const wchar_t *	desc ;		// 説明文（UI表示用）
			double			minRange ;	// 値の有効範囲（UI用）
			double			maxRange ;
		} ;
		struct	ParamSetClass
		{
			const ParamSetClass *	pParent ;
			size_t					nCount ;
			const ParamEntry *		pEntries ;
		} ;

		// UpdatePropertyReference フラグ
		enum	UpdatePropReferenceFlag
		{
			updateRefResource		= 0x0001,
			updateRefItem			= 0x0002,
			updateRefSubComposition	= 0x0004,
			updateRefScriptObject	= 0x0008,
			updateRefRsrcAndItem	= 0x0003,
			updateRefAll			= 0x000F,
		} ;

		// FormatSequencer, FormatController, FormatItem フラグ
		enum	FormatPropertyFlag
		{
			formatOptWithFrameSequence	= 0x0001,	// フレーム値配列を保存する
		} ;

		// パラメータ・インターフェース
		class	Parameter	: public SObject
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( Parameter, SObject )
			// パラメータ値取得
			virtual S3DDMatrix GetMatrixParameter( size_t iParam ) const ;
			virtual S3DDVector GetVectorParameter( size_t iParam ) const ;
			virtual double GetScalarParameter( size_t iParam ) const ;
			virtual int32_t GetIntegerParameter( size_t iParam ) const ;
			virtual bool GetBooleanParameter( size_t iParam ) const ;
			virtual const wchar_t * GetCommandParameter( size_t iParam ) const ;
			virtual PoseInstance GetPoseParameter( size_t iParam ) const ;
			virtual size_t GetBinaryParameter
				( void * pDst, size_t nBufBytes, size_t iParam ) const ;
			template <class T> static size_t GetBinaryTypeParameter
				( void * pDst, size_t nBufBytes, const T& tSrc )
			{
				if ( pDst == NULL )	return	sizeof(T) ;
				if ( nBufBytes == sizeof(T) )
				{
					*((T*)pDst) = tSrc ;
					return	sizeof(T) ;
				}
				return	0 ;
			}
			void GetParameter
				( SSystem::SString& strValue, ParameterType type, size_t iParam ) ;
			// パラメータ値設定
			virtual void SetMatrixParameter( size_t iParam, const S3DDMatrix& mat ) ;
			virtual void SetVectorParameter( size_t iParam, const S3DDVector& vec ) ;
			virtual void SetScalarParameter( size_t iParam, double s ) ;
			virtual void SetIntegerParameter( size_t iParam, int32_t n ) ;
			virtual void SetBooleanParameter( size_t iParam, bool b ) ;
			virtual void SetCommandParameter( size_t iParam, const wchar_t * pwszCmd ) ;
			virtual void SetPoseParameter( size_t iParam, const PoseInstance& pose ) ;
			virtual size_t SetBinaryParameter
				( size_t iParam, const void * pSrc, size_t nBufBytes ) ;
			template <class T> static size_t SetBinaryTypeParameter
				( T& tDst, const void * pSrc, size_t nBufBytes )
			{
				if ( nBufBytes == sizeof(T) )
				{
					tDst = *((const T*)pSrc) ;
					return	sizeof(T) ;
				}
				return	0 ;
			}
			bool SetParameter
				( ParameterType type, size_t i,
					const SSystem::SString& strValue ) ;
			bool SetVariant( size_t iParam, const Variant& val ) ;
			// パラメータ値域列挙
			virtual bool EnumerateStringSet
				( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		public:
			// バイナリヘッダ取得
			bool GetBinaryParameterHeader( BinaryHeader& hdr, size_t iParam ) const ;
			// インスタンス・パラメータ
			bool GetInstancingParameter
				( SSystem::SArray<BinaryInstancingEntry>& aInstancing, size_t iParam ) const ;
			void SetInstancingParameter
				( size_t iParam, const BinaryInstancingEntry * pInstancing, size_t nCount ) ;
		public:
			// パラメータ解釈
			static bool ParseMatrix( S3DDMatrix& mat, const SSystem::SString& strValue ) ;
			static bool ParseVector( S3DDVector& vec, const SSystem::SString& strValue ) ;
			static bool ParseScalar( double& s, const SSystem::SString& strValue ) ;
			static bool ParseInteger( int32_t& n, const SSystem::SString& strValue ) ;
			static bool ParseBoolean( bool& b, const SSystem::SString& strValue ) ;
			static bool ParseVectorX( double * pVec, size_t nCount, const SSystem::SString& strValue ) ;
			// パラメータフォーマット
			static void FormatMatrix( SSystem::SString& strValue, const S3DDMatrix& mat ) ;
			static void FormatVector( SSystem::SString& strValue, const S3DDVector& vec ) ;
			static void FormatScalar( SSystem::SString& strValue, double s ) ;
			static void FormatInteger( SSystem::SString& strValue, int32_t n ) ;
			static void FormatBoolean( SSystem::SString& strValue, bool b ) ;
			static void FormatVectorX( SSystem::SString& strValue, const double * pVec, size_t nCount ) ;
			// S3DDVector -> SGLPalette 変換
			static SGLPalette ColorFromVector( const S3DDVector& vec ) ;
			// SGLPalette -> S3DDVector 変換
			static S3DDVector VectorFromColor( const SGLPalette& rgb ) ;

		public:
			// Loquaty クラス名
			virtual const wchar_t * GetLQClassName( void ) const ;
		} ;

		// アイテム生成エントリ
		struct	ItemClassDescriptor
		{
			const wchar_t *			pwszClassID ;
			const ESLRuntimeClass *	pRuntimeClass ;
		} ;
		class	ItemCreator : public SObject
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( ItemCreator, SObject )
			// アイテム生成
			virtual Parameter * NewItem( void ) = 0 ;
		} ;
		class	ESLItemCreator : public ItemCreator
		{
		private:
			const ESLRuntimeClass *	m_pRuntimeClass ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( ESLItemCreator, ItemCreator )
			// 構築関数
			ESLItemCreator( const ItemClassDescriptor* pDesc ) ;
			// アイテム生成
			virtual Parameter * NewItem( void ) ;
		} ;

		// シーク方法
		enum	SeekMethod
		{
			seekStream,			// ストリーミング（通常）
			seekStreamPaused,	// Pause 時の OnUpdateFrame 呼び出し
			seekJump,			// フレームジャンプ時（ストリーミング中）
			seekJumpReset,		// フレームジャンプ時（リセット動作）
		} ;

		class	ParameterProperty ;
		class	ItemSerializer ;
		class	Composition ;

		// シーケンサ
		class	Sequencer	: public Parameter
		{
		public:
			enum	InterpolateMethod
			{
				methodSignal,		// シグナル（キーフレームで実行）
				methodSelector,		// 補完無し（直線のキーフレーム値）
				methodLinear,		// 線形補完
				methodSpline,		// 三次スプライン補完
				methodBezier,		// 三次ベジェ曲線
			} ;
			enum	KeyFrameIndex
			{
				indexDefault	 = -1,	// XetXxxParameter 指標・デフォルト値
			} ;
			static const SSystem::SXMLDocument::AttrInteger
										m_aiInterpolateMethods[6] ;
		protected:
			InterpolateMethod				m_methodInterpolate ;
			SSystem::SArray<KeyFrameParam>	m_arrKeyFrames ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( Sequencer, Parameter )
			// 構築関数
			Sequencer( void ) ;
			Sequencer( const Sequencer& seq ) ;
			// 消滅関数
			virtual ~Sequencer( void ) ;
		public:
			// 補完方法取得
			InterpolateMethod GetInterpolateMethod( void ) const ;
			// 補完方法設定
			void SetInterpolateMethod( InterpolateMethod method ) ;
			// キーフレーム範囲取得
			bool GetKeyFrameRange( int32_t& iFirst, int32_t& iEnd ) const ;
			// キーフレームパラメータ取得
			virtual bool GetKeyFrameParameter
					( size_t i, KeyFrameParam& kfp ) const ;
			// キーフレームパラメータ設定
			virtual bool SetKeyFrameParameter
					( size_t i, const KeyFrameParam& kfp ) ;
			// キーフレーム個数取得
			virtual size_t GetKeyFrameCount( void ) const ;
			// キーフレーム挿入
			virtual void InsertKeyFrame
					( size_t i, const KeyFrameParam& kfp ) ;
			// キーフレーム削除
			virtual void RemoveKeyFrame( size_t i ) ;
			// キーフレームから各フレームの値を計算する
			virtual void UpdateAllFrameValues( void ) ;
			// キーフレーム順を正規化する（後ろのキーフレームが前に来ないように）
			virtual bool NormalizeKeyFrameOrder( void ) ;
			// キーフレームの順番を入れ替える
			virtual void SwapKeyFrame( size_t i1, size_t i2 ) ;
			// キーフレーム挿入指標検索
			size_t OrderKeyFrameIndex( int32_t iFrame ) const ;
			// キーフレーム検索
			ssize_t FindKeyFrame( int32_t iFrame ) const ;
			// 区間取得
			size_t GetKeyFrameProgress( double& t, int32_t iFrame ) const ;
		public:
			// シーケンサー解釈
			virtual SGLError ParseSequencer
				( const SSystem::SXMLDocument& xmlTag, ParameterType type ) ;
			// アニメーションフレーム解釈
			virtual SGLError ParseFrameSequence
				( const SSystem::SXMLDocument& xmlSeq, ParameterType type ) ;
			// シーケンサー保存
			virtual SGLError FormatSequencer
					( SSystem::SXMLDocument& xmlTag,
							ParameterType type, uint32_t nFlags = 0 ) ;
			// アニメーションフレーム保存
			virtual SGLError FormatFrameSequence
					( SSystem::SXMLDocument& xmlSeq, ParameterType type ) ;
		public:
			// 補完パラメータ値取得
			virtual S3DDMatrix GetFrameMatrix( double fpFrame ) ;
			virtual S3DDVector GetFrameVector( double fpFrame ) ;
			virtual double GetFrameScalar( double fpFrame ) ;
			virtual int32_t GetFrameInteger( double fpFrame ) ;
			virtual bool GetFrameBoolean( double fpFrame ) ;
			virtual const wchar_t * GetFrameCommand( double fpFrame, SeekMethod seek ) ;
			virtual PoseInstance GetFramePoseInstance( double fpFrame ) ;
			virtual size_t GetFrameBinary
				( void * pDst, size_t nBufBytes, double fpFrame ) const ;
		public:
			// フレームシーケンス作成
			virtual SGLError CreateFrameSequence
						( size_t iFirstFrame, size_t nDuration ) ;
			// フレーム値設定
			virtual SGLError SetFrameMatrix( size_t iFrame, const S3DDMatrix& mat ) ;
			virtual SGLError SetFrameVector( size_t iFrame, const S3DDVector& vec ) ;
			virtual SGLError SetFrameScalar( size_t iFrame, double s ) ;
			virtual SGLError SetFrameInteger( size_t iFrame, int32_t n ) ;
			virtual SGLError SetFrameBoolean( size_t iFrame, bool b ) ;
			virtual size_t SetFrameBinary
				( size_t iFrame, const void * pSrc, size_t nBufBytes ) ;
		public:
			// シーケンサー生成
			static Sequencer * NewParameterSequencer( ParameterType type ) ;
			// シーケンサー型照合
			static bool VerifyParameterSequencer( ParameterType type, Sequencer * pSeq ) ;
			// 複製
			virtual Sequencer * DuplicateSequencer( void ) = 0 ;
			// 作成時の処理
			virtual void OnSequencerAttached( ParameterProperty * pItem ) ;
			// リソース等の参照を更新する
			virtual uint32_t UpdatePropertyReference
				( S3DSceneComposer::Composition& comp,
							ItemSerializer * pItem, uint32_t nFlags ) ;
		public:
			// Loquaty クラス名
			virtual const wchar_t * GetLQClassName( void ) const ;
		} ;

		// パラメータ・エントリ保存情報
		class	ParameterEntryStorage
		{
		public:
			SSystem::SString					m_strValue ;
			SSystem::SSmartPointer<Sequencer>	m_pSequencer ;
		} ;

		// パラメータ・プロパティ情報
		class	ParameterProperty	: public Parameter
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( ParameterProperty, Parameter )
			// アイテムID取得
			virtual const wchar_t * GetItemIdentity( void ) const = 0 ;
			// アイテムID設定
			virtual void SetItemIdentity( const wchar_t * pwszID ) = 0 ;
			// パラメータ総数
			virtual size_t GetParameterCount( void ) const = 0 ;
			// パラメータ識別名
			virtual const wchar_t * GetParameterID( size_t iParam ) const = 0 ;
			// パラメータ表示名
			virtual const wchar_t * GetParameterFriendlyName( size_t iParam ) const = 0 ;
			// パラメータ説明
			virtual const wchar_t * GetParameterDescription( size_t iParam ) const = 0 ;
			// パラメータ指標検索
			virtual ssize_t FindParameterID( const wchar_t * pwszID ) const = 0 ;
			// パラメータ型
			virtual ParameterType GetParameterType( size_t iParam ) const = 0 ;
			// パラメータ属性
			virtual uint32_t GetParameterAttributes( size_t iParam ) const = 0 ;
			// パラメータ有効範囲
			virtual bool GetParameterScalarRange
				( size_t i, double& fpMin, double& fpMax ) const = 0 ;
			// パラメーター有効性
			virtual bool IsParameterValidation( size_t iParam ) const = 0 ;
			// パラメータカテゴリ名取得
			virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;
			// パラメータ・シーケンサ取得
			virtual Sequencer * GetParameterSequencer( size_t iParam ) const = 0 ;
			virtual Sequencer * CreateParameterSequencer( size_t iParam ) = 0 ;
			// パラメータ・シーケンサ削除
			virtual void RemoveParameterSequencer( size_t iParam ) = 0 ;
		protected:
			// パラメータ・シーケンサ設定
			virtual void SetParameterSequencer( size_t iParam, Sequencer * pSeq ) = 0 ;
		public:
			// ポーズライブラリ取得
			virtual S3DModelPoseLibrary * GetPoseLibraryChain( void ) ;
		public:	// Parameter
			// パラメータ値取得
			virtual size_t GetBinaryParameter
				( void * pDst, size_t nBufBytes, size_t i ) const ;
			// パラメータ値設定
			virtual size_t SetBinaryParameter
				( size_t i, const void * pSrc, size_t nBufBytes ) ;
			// バリアントとして取得
			Variant GetVariant( size_t iParam ) const ;
		public:
			// シーケンスパラメータをプロパティに設定
			void SetFrameSequenceParameter
				( ParameterType type, size_t i,
					Sequencer * pSeq, double fpFrame, SeekMethod seek ) ;
		public:
			// ポーズ文字列解釈
			virtual void ParsePoseString
				( PoseInstance& pose, const wchar_t * pwszPose ) const ;
			// ポーズ文字列書式化
			virtual SSystem::SString
					FormatPoseString( const PoseInstance& pose ) const ;
			// ポーズ取得
			virtual S3DModelPose *
				GetPoseIdentityAs( const wchar_t * pwszPoseID ) const ;
			// ポーズID取得
			virtual bool GetPoseIdentityOf
				( SSystem::SString& strPoseID, S3DModelPose * pPose ) const ;
		public:
			// デシリアライズ
			SGLError ParseParameterProperties
				( const SSystem::SXMLDocument& xmlParam ) ;
			// シリアライズ
			SGLError FormatParameterProperties
				( SSystem::SXMLDocument& xmlParam ) ;
		public:
			// パラメータの一時保存
			SGLError SaveParameterEntryAt
				( ParameterEntryStorage& storage, size_t i ) ;
			// 一時保存パラメータの復元
			SGLError RestoreParameterEntryAt
				( size_t i, ParameterEntryStorage& storage ) ;
		public:
			// Loquaty クラス名
			virtual const wchar_t * GetLQClassName( void ) const ;
		} ;

		// アイテム・コントローラー
		enum	ControllerBehaviorFlag
		{
			behaviorRenderEvent		= 0x00000001,	// OnRenderEvent
			behaviorCollision		= 0x00000002,	// RenderCollision
			behaviorRender			= 0x00000004,	// RenderModel
			behaviorRenderContext	= 0x00000008,	// BeforeRenderModel, AfterRenderModel
			behaviorAlwaysActive	= 0x00000010,
			behaviorOnTimer			= 0x00000020,	// OnTimer
			behaviorDisabled		= 0x00000100,
		} ;
		class	SpaceSerializer ;
		class	Controller	: public ParameterProperty
		{
		protected:
			const wchar_t *						m_pwszClassID ;
			ItemSerializer *					m_pOwnerItem ;
			SSystem::SString					m_strControlID ;
			uint32_t							m_flagsBehavior ;		// enum ControllerBehaviorFlag
			uint32_t							m_flagsEventClasses ;	// enum ItemClassFlag
			SSystem::SArray<ParamEntry>			m_arrParamClass ;
			SSystem::SObjectArray<Sequencer>	m_arrSequencers ;
			SSystem::SObjectArray<SSystem::SXMLDocument>
												m_arrUnknownProp ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( Controller, ParameterProperty )
			// 構築関数
			Controller( const wchar_t * pwszClassID ) ;
			// 消滅関数
			virtual ~Controller( void ) ;
		public:
			// パラメータ数宣言
			void PrepareParameterEntryCount( size_t nCount ) ;
			// パラメータ追加
			size_t AddParameterEntry( const ParamEntry& paramEntry ) ;
			size_t AddParameterEntry
				( const wchar_t * id,
					ParameterType type, uint32_t attr,
					const wchar_t * name, const wchar_t * desc = NULL,
					double minRange = 0.0, double maxRange = 1.0 ) ;
			// パラメータエントリ編集
			ParamEntry& ParameterEntryAt( size_t iParam ) ;
			// パラメータ削除
			void RemoveAllParameterEntries( void ) ;
			void ChopParameterEntryLastAt( size_t iParam ) ;
			// 所有アイテム取得
			ItemSerializer * GetOwnerItem( void ) const ;
			// コンポジション取得
			Composition * GetComposition( void ) const ;
			// コンポーザー取得
			S3DSceneComposer * GetComposer( void ) const ;
			// コンポーザー・マネージャー取得
			S3DCompositionManager * GetManager( void ) const ;
			// エディター取得
			S3DCompositionEditorInterface * GetEditor( void ) const ;
			// アイテム取得
			ItemSerializer * GetSceneItemAs( const wchar_t * pwszID ) const ;
			SpaceSerializer * GetSceneSpaceAs( const wchar_t * pwszID ) const ;
			S3DScene::Space * GetReferenceSpaceAs( const wchar_t * pwszPath ) const ;
			// オーナーアイテムのプライマリモデル取得
			S3DVertexBufferInterface * GetOwnerItemPrimaryModel( void ) ;
		public:
			// 読み込み処理
			virtual SGLError ParseController
				( S3DSceneComposer& composer,
					Composition& composition,
					const SSystem::SXMLDocument& xmlTag ) ;
			// 保存処理
			virtual SGLError FormatController
				( S3DSceneComposer& composer,
					SSystem::SXMLDocument& xmlTag, uint32_t nFlags = 0 ) ;
		public:
			// フレームを適用
			virtual void SetFrameParameters( double fpFrame, SeekMethod seek ) ;
			// パラメータ値取得
			virtual S3DDMatrix GetMatrixParameter( size_t iParam ) const ;
			virtual S3DDVector GetVectorParameter( size_t iParam ) const ;
			virtual double GetScalarParameter( size_t iParam ) const ;
			virtual int32_t GetIntegerParameter( size_t iParam ) const ;
			virtual bool GetBooleanParameter( size_t iParam ) const ;
			virtual const wchar_t * GetCommandParameter( size_t iParam ) const ;
			virtual PoseInstance GetPoseParameter( size_t iParam ) const ;
			virtual size_t GetBinaryParameter
				( void * pDst, size_t nBufBytes, size_t iParam ) const ;
			// パラメータ値設定
			virtual void SetMatrixParameter( size_t iParam, const S3DDMatrix& mat ) ;
			virtual void SetVectorParameter( size_t iParam, const S3DDVector& vec ) ;
			virtual void SetScalarParameter( size_t iParam, double s ) ;
			virtual void SetIntegerParameter( size_t iParam, int32_t n ) ;
			virtual void SetBooleanParameter( size_t iParam, bool b ) ;
			virtual void SetCommandParameter( size_t iParam, const wchar_t * pwszCmd ) ;
			virtual void SetPoseParameter( size_t iParam, const PoseInstance& pose ) ;
			virtual size_t SetBinaryParameter
				( size_t iParam, const void * pSrc, size_t nBufBytes ) ;
		public:
			// コントローラークラスID取得
			virtual const wchar_t * GetControllerClass( void ) const ;
			// アイテムID取得
			virtual const wchar_t * GetItemIdentity( void ) const ;
			// アイテムID設定
			virtual void SetItemIdentity( const wchar_t * pwszID ) ;
			// パラメータ総数
			virtual size_t GetParameterCount( void ) const ;
			// パラメータ識別名
			virtual const wchar_t * GetParameterID( size_t iParam ) const ;
			// パラメータ表示名
			virtual const wchar_t * GetParameterFriendlyName( size_t iParam ) const ;
			// パラメータ説明
			virtual const wchar_t * GetParameterDescription( size_t iParam ) const ;
			// パラメータ指標検索
			virtual ssize_t FindParameterID( const wchar_t * pwszID ) const ;
			// パラメータ型
			virtual ParameterType GetParameterType( size_t iParam ) const ;
			// パラメータ属性
			virtual uint32_t GetParameterAttributes( size_t iParam ) const ;
			// パラメータ有効範囲
			virtual bool GetParameterScalarRange
				( size_t iParam, double& fpMin, double& fpMax ) const ;
			// パラメーター有効性
			virtual bool IsParameterValidation( size_t iParam ) const ;
			// パラメータ・シーケンサ取得
			virtual Sequencer * GetParameterSequencer( size_t iParam ) const ;
			virtual Sequencer * CreateParameterSequencer( size_t iParam ) ;
			virtual Sequencer * NewParameterSequencer( ParameterType type ) const ;
			// パラメータ・シーケンサ削除
			virtual void RemoveParameterSequencer( size_t iParam ) ;
		protected:
			// パラメータ・シーケンサ設定
			virtual void SetParameterSequencer( size_t iParam, Sequencer * pSeq ) ;
		public:
			// ポーズライブラリ取得
			virtual S3DModelPoseLibrary * GetPoseLibraryChain( void ) ;
			// ポーズ取得
			virtual S3DModelPose *
				GetPoseIdentityAs( const wchar_t * pwszPoseID ) const ;
			// ポーズID取得
			virtual bool GetPoseIdentityOf
				( SSystem::SString& strPoseID, S3DModelPose * pPose ) const ;
		public:
			// アイテムプロパティのリソース等の参照を更新する
			virtual uint32_t UpdatePropertyReference
				( S3DSceneComposer::Composition& comp,
					ItemSerializer * pItem, uint32_t nFlags ) ;
			// レンダリング設定
			virtual void OnSetupSceneSettings
				( S3DScene& scene, ItemSerializer * pItem,
					const SGLSize& sizeFrame,
					SGLSecondaryViewProducer * psvp = NULL ) ;
			// レンダリング後始末
			virtual void OnShoutdownSceneSettings
				( S3DScene& scene, ItemSerializer * pItem,
					SGLSecondaryViewProducer * psvp = NULL ) ;
			// 拡張的な処理の通知
			virtual void OnExtendNotify
				( const wchar_t * pwszCmd, const wchar_t * pwszParam,
					const void * pExParam, size_t nExParamBytes ) ;
			// タイマー処理
			virtual void OnTimer
				( S3DScene& scene,
					ItemSerializer * pItem, uint32_t msecPast ) ;
			// フレーム（パラメータ）更新後処理
			virtual void OnUpdateFrame
				( ItemSerializer * pItem, double fpFrame, SeekMethod seek ) ;
		public:
			// 動作フラグ
			uint32_t GetControllerBehaviorFlags( void ) const
			{
				return	m_flagsBehavior ;
			}
			uint32_t GetBehaviorRenderEventClasses( void ) const
			{
				return	m_flagsEventClasses ;
			}
			bool IsControllerDisabled( void ) const
			{
				return	(m_flagsBehavior & behaviorDisabled) != 0 ;
			}
			void EnableController( bool flagEnable ) ;
			// 規定のレンダリングデバイス設定
			virtual void OnSetRenderDevice( S3DRenderDevice * pDevice ) ;
			// レンダリングの為のデバイスリソース準備
			virtual void OnPrepareToRender
				( S3DRenderDevice * pDevice, uint32_t nFlags = 0 ) ;
			// レンダリングイベント
			virtual void OnRenderEvent
				( S3DScene& scene,
					S3DScene::ItemClass clsItem,
					ItemSerializer * pItem ) ;
			// 当たり判定追加
			//（必要であれば scene.PhysicsScene() へ Actor の追加も行う）
			virtual void RenderCollision
				( const S3DScene& scene,
					ItemSerializer * pItem, S3DCollision& render ) ;
			// 表示モデル追加
			virtual void RenderModel
				( const S3DScene& scene,
					S3DScene::ItemClass clsItem,
					ItemSerializer * pItem,
					S3DRenderContextInterface& render,
					uint64_t flagsExclusion = 0 ) ;
			// 描画前処理
			virtual void BeforeRenderModel
				( const S3DScene& scene,
					S3DScene::ItemClass clsItem,
					ItemSerializer * pItem,
					S3DRenderContextInterface& render,
					uint64_t flagsExclusion = 0 ) ;
			// 描画後処理
			virtual void AfterRenderModel
				( const S3DScene& scene,
					S3DScene::ItemClass clsItem,
					ItemSerializer * pItem,
					S3DRenderContextInterface& render,
					uint64_t flagsExclusion = 0 ) ;
		protected:
			// アイテムに追加された
			virtual void OnAddController( ItemSerializer * pItem ) ;
			// アイテムから分離される
			virtual void BeforeDetachController( ItemSerializer * pItem ) ;
		public:
			// Loquaty クラス名
			virtual const wchar_t * GetLQClassName( void ) const ;

			friend class ItemSerializer ;
		} ;

		// アイテム・シリアライザ
		class	ItemSerializer	: public ParameterProperty,
									public S3DScene::ItemEventListener
		{
		protected:
			const wchar_t *						m_pwszClassID ;
			const ParamSetClass *				m_ppsClass ;
			SSystem::SString					m_strItemID ;
			SSystem::SString					m_strRefStyle ;
			SSystem::SObjectArray<Sequencer>	m_arrSequencers ;
			SSystem::SObjectArray<Controller>	m_arrControllers ;
			SSystem::SObjectArray<SSystem::SXMLDocument>
												m_arrUnknownProp ;
			uint32_t							m_flagsCtrlBehavior ;
			uint32_t							m_flagsCtrlEventClasses ;
			SSystem::SCriticalSection			m_csCtrlSync ;
			SSystem::SObjectArray<Controller>	m_arrDelayAddCtrls ;
			SSystem::SArray<size_t>				m_aDelayRemoveCtrls ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( ItemSerializer, ParameterProperty )
			// 構築関数
			ItemSerializer
				( const wchar_t * pwszClassID,
					const ParamSetClass * pClass ) ;
		public:
			// 読み込み処理
			virtual SGLError ParseItem
				( S3DSceneComposer& composer,
					Composition& composition,
					S3DScene::Space& space,
					const SSystem::SXMLDocument& xmlTag ) ;
			virtual SGLError ParseStyle
				( S3DSceneComposer& composer,
					Composition& composition,
					S3DScene::Space& space,
					const SSystem::SXMLDocument& xmlTag ) ;
			virtual SGLError ParseNonParameterTag
				( S3DSceneComposer& composer,
					Composition& composition,
					S3DScene::Space& space,
					const SSystem::SXMLDocument& xmlTag ) ;
			virtual SGLError ParseControllersTag
				( S3DSceneComposer& composer,
					Composition& composition,
					S3DScene::Space& space,
					const SSystem::SXMLDocument& xmlTag ) ;
			// 保存処理
			virtual SGLError FormatItem
				( S3DSceneComposer& composer,
					SSystem::SXMLDocument& xmlTag, uint32_t nFlags = 0 ) ;
		public:
			// フレームの初期化処理（デフォルトは0フレームへのシーク）
			virtual void InitializeFrameParameters( void ) ;
			// フレームを適用
			virtual void SetFrameParameters( double fpFrame, SeekMethod seek ) ;
		public:
			// スタイル参照アイテムか？
			virtual const wchar_t * GetStyleReference( void ) const ;
			// アイテムクラス取得
			virtual const wchar_t * GetItemClass( void ) const ;
			// アイテムID取得
			virtual const wchar_t * GetItemIdentity( void ) const ;
			// アイテムID設定
			virtual void SetItemIdentity( const wchar_t * pwszID ) ;
			// パラメータエントリ取得
			virtual const ParamEntry * GetParameterEntryAt( size_t i ) const ;
			static const ParamEntry *
				GetParameterEntryAt( const ParamSetClass * ppsClass, size_t i ) ;
			// パラメータ総数
			virtual size_t GetParameterCount( void ) const ;
			static size_t GetParameterCount( const ParamSetClass * ppsClass ) ;
			// パラメータ識別名
			virtual const wchar_t * GetParameterID( size_t i ) const ;
			// パラメータ表示名
			virtual const wchar_t * GetParameterFriendlyName( size_t i ) const ;
			// パラメータ説明
			virtual const wchar_t * GetParameterDescription( size_t i ) const ;
			// パラメータ指標検索
			virtual ssize_t FindParameterID( const wchar_t * pwszID ) const ;
			static ssize_t FindParameterID
				( const ParamSetClass * ppsClass, const wchar_t * pwszID ) ;
			// パラメータ型
			virtual ParameterType GetParameterType( size_t i ) const ;
			// パラメータ属性
			virtual uint32_t GetParameterAttributes( size_t i ) const ;
			// パラメータ有効範囲
			virtual bool GetParameterScalarRange
				( size_t i, double& fpMin, double& fpMax ) const ;
			// パラメーター有効性
			virtual bool IsParameterValidation( size_t i ) const ;
			// パラメータ・シーケンサ取得
			virtual Sequencer * GetParameterSequencer( size_t i ) const ;
			virtual Sequencer * CreateParameterSequencer( size_t i ) ;
			virtual Sequencer * NewParameterSequencer( ParameterType type ) const ;
			// パラメータ・シーケンサ削除
			virtual void RemoveParameterSequencer( size_t i ) ;
		protected:
			// パラメータ・シーケンサ設定
			virtual void SetParameterSequencer( size_t i, Sequencer * pSeq ) ;
		public:
			// ポーズライブラリ取得
			virtual S3DModelPoseLibrary * GetPoseLibraryChain( void ) ;
		public:
			// コントローラー数
			virtual size_t GetControllerCount( void ) const ;
			// コントローラー指標検索
			virtual ssize_t FindControllerID( const wchar_t * pwszID ) const ;
			virtual ssize_t FindController( Controller * pContoroller ) const ;
			virtual ssize_t FindControllerClassOf
				( const ESLRuntimeClass& rtClass, size_t iFirst = 0 ) const ;
			// コントローラー取得
			virtual Controller * GetControllerAt( size_t i ) const ;
			virtual Controller * GetControllerAs( const wchar_t * pwszID ) const ;
			template <class T> T * GetController( size_t iFirst = 0 ) const
				{
					return	ESLTypeCast<T>( GetControllerAt
						( (size_t) FindControllerClassOf
									( ESL_RUNTIME_CLASS(T), iFirst ) ) ) ;
				}
			// コントローラー追加
			virtual size_t AddController( Controller * pContoroller ) ;
			virtual size_t InsertController( size_t i, Controller * pContoroller ) ;
			// コントローラー削除
			virtual SGLError RemoveControllerAt( size_t i ) ;
			virtual SGLError RemoveControllerAs( const wchar_t * pwszID ) ;
			virtual Controller * DetachControllerAt( size_t i ) ;
			virtual Controller * DetachControllerAs( const wchar_t * pwszID ) ;
			// コントローラー操作排他操作
			//（OnTimer は複数スレッドで実行される可能性があるため Lock 状態で実行される）
			void LockController( void ) const ;
			void UnlockController( void ) const ;
			atomic_int_t UnlockControllerAll( void ) const ;
			void RelockController( atomic_int_t nLock ) const ;
			const SSystem::SCriticalSection * GetControllerLocker( void ) const ;
			// コントローラー遅延追加
			void DelayAddController( Controller * pController ) ;
			// コントローラー遅延削除
			void DelayRemoveControllerAt( size_t iCtrl ) ;
			// コントローラー遅延処理確定
			void CommitDelayControllers( void ) ;
		protected:
			// コントローラー通知
			virtual void OnAddController( Controller * pController ) ;
			virtual void BeforeDetachController( Controller * pController ) ;
		public:
			// コントローラーID正規化
			void NormalizeControllerID( SSystem::SString& strID ) const ;
			// スクリプト・インスタンス取得
			virtual class S3DSceneScriptInstance * GetScriptInstanceOf( const wchar_t * pwszClass ) const ;
			virtual Rosetta::RSObject * GetRosettaInstanceOf( const wchar_t * pwszClass ) const ;
			virtual Loquaty::LObjPtr GetLoquatyInstanceOf( const wchar_t * pwszClass ) const ;
			uint8_t * GetScriptStructInstanceOf
					( const wchar_t * pwszStruct, size_t nStructBytes ) const ;
		public:
			// 親空間取得
			virtual ItemSerializer * GetParentSpaceItem( void ) const ;
			// コンポジション取得
			virtual Composition * GetComposition( void ) const ;
			// コンポーザー取得
			virtual S3DSceneComposer * GetComposer( void ) const ;
			// コンポーザー・マネージャー取得
			virtual S3DCompositionManager * GetManager( void ) const ;
			// シーン取得
			virtual S3DScene * GetScene( void ) const ;
			// エディター取得
			virtual S3DCompositionEditorInterface * GetEditor( void ) const ;
			// デバッグ用（エディタ出力）
			void OutputError
				( const wchar_t * pwszErrMsg,
					const wchar_t * pwszSrcFile,
					int nLineNum, const wchar_t * pwszSrcLine ) ;
			void OutputTraceLog( const wchar_t * pwszLogMsg ) ;
			// アイテム取得
			ItemSerializer * GetSceneItemAs( const wchar_t * pwszID ) const ;
			SpaceSerializer * GetSceneSpaceAs( const wchar_t * pwszID ) const ;
			S3DScene::Space * GetReferenceSpaceAs( const wchar_t * pwszPath ) const ;
			// アイテム空間変換行列（アイテム自身を含まないグローバル変換）を計算
			virtual void GetItemLinkTransformation
					( S3DDMatrix& matLink, S3DDVector& vLinkPos ) const ;
			// 空間行列取得
			virtual void GetGlobalTransformation
				( S3DDMatrix& matGlobal, S3DDVector& vGlobalPos ) const ;
			// 空間行列取得
			virtual void GetTransformationFrom
				( S3DDMatrix& matOffset, S3DDVector& vOffset,
								const ItemSerializer * pItem ) const ;
			virtual void GetTransformationFrom
				( S3DDMatrix& matOffset, S3DDVector& vOffset,
					const S3DDMatrix& matAnother, const S3DDVector& vAnother ) const ;
			// 空間色効果取得（乗算色α要素に不透明度取得）
			virtual void GetGlobalColorEffect( S3DColor& clrEffect ) const ;
			// 子アイテム数取得
			virtual size_t GetChildItemCount( void ) const ;
			// 子アイテム取得
			virtual ItemSerializer * GetChildItemAt( size_t i ) const ;
			// 子アイテム検索
			virtual ssize_t FindChildItem( ItemSerializer * pItem ) const ;
		public:
			// ポーズ取得
			virtual S3DModelPose *
				GetPoseIdentityAs( const wchar_t * pwszPoseID ) const ;
			// ポーズID取得
			virtual bool GetPoseIdentityOf
				( SSystem::SString& strPoseID, S3DModelPose * pPose ) const ;
			// アイテムのプライマリモデル取得
			virtual S3DVertexBufferInterface * GetItemPrimaryModel( void ) ;
			// アイテムのコリジョンバッファ取得
			virtual S3DCollider * GetItemPrimaryCollider( void ) ;
		public:
			// コンポジションツリーに追加された
			virtual void OnAttachedCompositionTree
				( S3DSceneComposer::Composition& comp ) ;
			// アイテムプロパティのリソース等の参照を更新する
			virtual uint32_t UpdatePropertyReference
				( S3DSceneComposer::Composition& comp, uint32_t nFlags ) ;
			// レンダリング設定
			virtual void OnSetupSceneSettings
				( S3DScene& scene,
					const SGLSize& sizeFrame,
					SGLSecondaryViewProducer * psvp = NULL ) ;
			// レンダリング後始末
			virtual void OnShoutdownSceneSettings
				( S3DScene& scene,
					SGLSecondaryViewProducer * psvp = NULL ) ;
			// 拡張的な処理の通知
			virtual void OnExtendNotify
				( const wchar_t * pwszCmd, const wchar_t * pwszParam,
					const void * pExParam, size_t nExParamBytes ) ;
			// タイマー処理
			virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;
			// フレーム更新後処理
			virtual void OnUpdateFrame( double fpFrame, SeekMethod seek ) ;
		public:	// S3DScene::ItemEventListener
			// 規定のレンダリングデバイス設定
			virtual void OnSetRenderDevice( S3DRenderDevice * pDevice ) ;
			// レンダリングの為のデバイスリソース準備
			virtual void OnPrepareToRender
				( S3DRenderDevice * pDevice, uint32_t nFlags = 0 ) ;
			// レンダリング前後処理（全視点共通）
			virtual void OnItemRenderEvent
				( S3DScene& scene, S3DScene::ItemClass clsItem ) ;
			// 当たり判定追加
			//（必要であれば scene.PhysicsScene() へ Actor の追加も行う）
			virtual void OnItemRenderCollision
				( const S3DScene& scene, S3DCollision& render ) ;
			// 表示モデル追加
			virtual void OnItemRenderModel
				( const S3DScene& scene,
					S3DRenderContextInterface& render,
					uint64_t flagsExclusion = 0 ) ;
			// 描画前処理
			virtual void BeforeItemRenderModel
				( const S3DScene& scene,
					S3DRenderContextInterface& render,
					uint64_t flagsExclusion = 0 ) ;
			// 描画後処理
			virtual void AfterItemRenderModel
				( const S3DScene& scene,
					S3DRenderContextInterface& render,
					uint64_t flagsExclusion = 0 ) ;
		protected:
			// Controller の SetFrameParameters 呼び出し
			void CallControllerSetFrameParameters
				( double fpFrame, S3DSceneComposer::SeekMethod seek ) ;
			// Controller の OnUpdateFrame 呼び出し処理と挙動フラグの更新
			void CallControllerOnUpdateFrame
				( double fpFrame, S3DSceneComposer::SeekMethod seek,
					uint32_t flagsNeesdBehavior = 0xFFFFFFFFUL,
					uint32_t flagsNegativeBehavior = 0xFFFFFFFFUL ) ;
			// Controller の OnSetRenderDevice 呼び出し
			void CallControllerOnSetRenderDevice( S3DRenderDevice * pDevice ) ;
			// Controller の OnPrepareToRender 呼び出し
			void CallControllerOnPrepareToRender
				( S3DRenderDevice * pDevice, uint32_t nFlags = 0 ) ;
			// Controller の OnRenderEvent 呼び出し
			void CallControllerOnRenderEvent
				( S3DScene& scene, S3DScene::ItemClass clsItem ) ;
			// Controller の RenderCollision 呼び出し
			void CallControllerRenderCollision
				( const S3DScene& scene, S3DCollision& render ) ;
			// Controller の RenderModel 呼び出し
			void CallControllerRenderModel
				( const S3DScene& scene, S3DScene::ItemClass clsItem,
					S3DRenderContextInterface& render, uint64_t flagsExclusion ) ;
			// Controller の BeforeRenderModel 呼び出し
			void CallControllerBeforeRenderModel
				( const S3DScene& scene, S3DScene::ItemClass clsItem,
					S3DRenderContextInterface& render, uint64_t flagsExclusion ) ;
			// Controller の AfterRenderModel 呼び出し
			void CallControllerAfterRenderModel
				( const S3DScene& scene, S3DScene::ItemClass clsItem,
					S3DRenderContextInterface& render, uint64_t flagsExclusion ) ;
		public:
			// Loquaty クラス名
			virtual const wchar_t * GetLQClassName( void ) const ;
		} ;

		// 空間・シリアライザ（共通基底）
		class	CommonSerializer	: public ItemSerializer
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
				paramVisible,
				paramForceToon,
				paramForceBorder,
				paramFreeToon,
				paramFreeBorder,
				paramUseCollision,
				paramGlobalSpace,
				paramCameraShift,
				paramCameraSpace,
				paramHideNear,
				paramHideFar,
				paramCount,
			} ;
		protected:
			static const ParamEntry		m_paramEntries[paramCount] ;
			static const ParamSetClass	m_pscClass ;

			S3DDMatrix	m_matSpaceRotation ;
			S3DDVector	m_vSpaceZoom ;

		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( CommonSerializer, ItemSerializer )
			// 構築関数
			CommonSerializer
				( const wchar_t * pwszClassID, const ParamSetClass * pClass ) ;

		public:
			// 回転
			const S3DDMatrix& GetItemRotation( void ) const ;
			void SetItemRotation( const S3DDMatrix& matRot ) ;
			// 拡大
			const S3DDVector& GetItemZoom( void ) const ;
			void SetItemZoom( const S3DDVector& vZoom ) ;
			// 位置
			const S3DDVector& GetItemPosition( void ) const ;
			// 透明度
			unsigned int GetItemTransparency( void ) const ;
			// 色効果
			const S3DColor& GetItemColorEffect( void ) const ;
			// 変更フラグ取得
			bool IsModifiedItemMatrix( void ) const ;
			bool IsModifiedItemPosition( void ) const ;
			bool IsModifiedItemTransparency( void ) const ;
			bool IsModifiedItemColorAdd( void ) const ;
			bool IsModifiedItemColorMul( void ) const ;

		public:	// Parameter
			// パラメータ値取得
			virtual S3DDMatrix GetMatrixParameter( size_t iParam ) const ;
			virtual S3DDVector GetVectorParameter( size_t iParam ) const ;
			virtual double GetScalarParameter( size_t iParam ) const ;
			virtual bool GetBooleanParameter( size_t iParam ) const ;
			// パラメータ値設定
			virtual void SetMatrixParameter( size_t iParam, const S3DDMatrix& mat ) ;
			virtual void SetVectorParameter( size_t iParam, const S3DDVector& vec ) ;
			virtual void SetScalarParameter( size_t iParam, double s ) ;
			virtual void SetBooleanParameter( size_t iParam, bool b ) ;
			// パラメータカテゴリ名取得
			virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;
		public:	// ItemSerializer
			// パラメーター有効性
			virtual bool IsParameterValidation( size_t i ) const ;
			// フレームを適用
			virtual void SetFrameParameters( double fpFrame, SeekMethod seek ) ;
		public:
			// S3DScene::SpaceInfo 取得
			virtual const S3DScene::SpaceInfo& GetSpaceInfo( void ) const = 0 ;
			virtual S3DScene::SpaceInfo& SpaceParameter( void ) = 0 ;
			// 表示フラグ取得
			virtual bool GetVisibleParameter( void ) const = 0 ;
			// 表示フラグ設定
			virtual void SetVisibleParameter( bool b ) = 0 ;
			// 動作フラグ取得
			virtual uint32_t GetBehaviorFlags( void ) const = 0 ;
			// 動作フラグ設定
			virtual void SetBehaviorFlags( uint32_t nFlags ) = 0 ;
			// 行列設定
			virtual void SetItemMatrix( const S3DDMatrix& matrix ) ;
			// 座標設定
			virtual void SetItemPositioin( const S3DDVector& vPos ) ;
			// 変換行列更新
			virtual void UpdateSpaceMatrix( void ) ;
		} ;

		// 空間・シリアライザ
		class	SpaceSerializer
					: public S3DScene::Space, public CommonSerializer
		{
		public:
			enum	ParameterIndex
			{
				paramIgnore			= CommonSerializer::paramCount,
				paramSetLayerSace,
				paramLayeredPriority,
				paramLayeredParam,
				paramLayeredReflMap,
				paramLayeredRefrMap,
				paramLayeredField,
				paramLayeredStaticItem1,
				paramLayeredStaticItem2,
				paramLayeredDynamicItem1,
				paramLayeredDynamicItem2,
				paramLayeredDynamicItem3,
				paramLayeredEffectItem,
				paramSpaceTotalCount,
				paramSpaceCount		= paramSpaceTotalCount - paramIgnore,
			} ;
		protected:
			static const ParamEntry		m_paramEntries[paramSpaceCount] ;
			static const ParamSetClass	m_pscClass ;

			static const SSystem::SXMLDocument::AttrInteger	m_aiEnvMapSource[3] ;

			S3DScene::Space *	m_pSpace ;
			SSystem::SObjectArray<ItemSerializer>
								m_arrItemAssets ;

		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO2( SpaceSerializer, Space, CommonSerializer )
			S3D_DECLARE_COMPOSER_ITEM( SpaceSerializer, space )
			// 構築関数
			SpaceSerializer( S3DScene::Space * pSpace = NULL ) ;
			SpaceSerializer
				( const wchar_t * pwszClassID,
					const ParamSetClass * pClass, S3DScene::Space * pSpace = NULL ) ;
			// S3DScene::Space 関連付け
			void AttachSpace( S3DScene::Space * pSpace ) ;
			// S3DScene::Space 取得
			S3DScene::Space * GetSceneSpace( void ) const
			{
				return	m_pSpace ;
			}
			// 所有リソース解放
			virtual void ReleaseAllAssets( void ) ;
			// 親空間取得
			virtual ItemSerializer * GetParentSpaceItem( void ) const ;
			// コンポジション取得
			virtual Composition * GetComposition( void ) const ;
			// シーン取得
			virtual S3DScene * GetScene( void ) const ;
			// アイテム空間変換行列（アイテム自身を含まないグローバル変換）を計算
			virtual void GetItemLinkTransformation
					( S3DDMatrix& matLink, S3DDVector& vLinkPos ) const ;
			// 空間行列取得
			virtual void GetGlobalTransformation
				( S3DDMatrix& matGlobal, S3DDVector& vGlobalPos ) const ;
			// 空間色効果取得（乗算色α要素に不透明度取得）
			virtual void GetGlobalColorEffect( S3DColor& clrEffect ) const ;
			// 子アイテム数取得
			virtual size_t GetChildItemCount( void ) const ;
			// 子アイテム取得
			virtual ItemSerializer * GetChildItemAt( size_t i ) const ;
			// アイテムID設定
			virtual void SetItemIdentity( const wchar_t * pwszID ) ;

		public:
			// 読み込み処理
			virtual SGLError ParseNonParameterTag
				( S3DSceneComposer& composer,
					Composition& composition,
					S3DScene::Space& space,
					const SSystem::SXMLDocument& xmlTag ) ;
			static SGLError ParseSpaceChildren
				( S3DSceneComposer& composer,
					Composition& composition,
					SpaceSerializer& space,
					bool flagSubComposition,
					const SSystem::SXMLDocument& xmlChildren ) ;
			virtual bool IsSubCompositionToParseChildren( void ) const ;
			// 保存処理
			virtual SGLError FormatItem
				( S3DSceneComposer& composer,
					SSystem::SXMLDocument& xmlTag, uint32_t nFlags ) ;
			static SGLError FormatSpaceChildren
				( S3DSceneComposer& composer,
					SpaceSerializer& space,
					SSystem::SXMLDocument& xmlChildren, uint32_t nFlags ) ;

		public:
			// フレーム反映
			virtual void SetAllItemsFrame( double fpFrame, SeekMethod seek ) ;

		public:	// Parameter
			// パラメータ値取得
			virtual S3DDVector GetVectorParameter( size_t iParam ) const ;
			virtual double GetScalarParameter( size_t iParam ) const ;
			virtual int32_t GetIntegerParameter( size_t iParam ) const ;
			virtual bool GetBooleanParameter( size_t iParam ) const ;
			virtual const wchar_t * GetCommandParameter( size_t iParam ) const ;
			// パラメータ値設定
			virtual void SetVectorParameter( size_t iParam, const S3DDVector& vec ) ;
			virtual void SetScalarParameter( size_t iParam, double s ) ;
			virtual void SetIntegerParameter( size_t iParam, int32_t n ) ;
			virtual void SetBooleanParameter( size_t iParam, bool b ) ;
			virtual void SetCommandParameter( size_t iParam, const wchar_t * pwszCmd ) ;
			// パラメータ値域列挙
			virtual bool EnumerateStringSet
				( size_t iParam, SSystem::SStringArray& aStrSet ) ;

		public:	// ParameterProperty
			// パラメータカテゴリ名取得
			virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

		public:
			// S3DScene::SpaceInfo 取得
			virtual const S3DScene::SpaceInfo& GetSpaceInfo( void ) const ;
			virtual S3DScene::SpaceInfo& SpaceParameter( void ) ;
			// 表示フラグ取得
			virtual bool GetVisibleParameter( void ) const ;
			// 表示フラグ設定
			virtual void SetVisibleParameter( bool b ) ;
			// 動作フラグ取得
			virtual uint32_t GetBehaviorFlags( void ) const ;
			// 動作フラグ設定
			virtual void SetBehaviorFlags( uint32_t nFlags ) ;
			// 変換行列更新
			virtual void UpdateSpaceMatrix( void ) ;

		public:
			// 無効化状態
			virtual bool IsIgnoreParameter( void ) const ;
			virtual void SetIgnoreParameter( bool flagIgnore ) ;

		public:
			// アイテムプロパティのリソース等の参照を更新する
			virtual uint32_t UpdatePropertyReference
				( S3DSceneComposer::Composition& comp, uint32_t nFlags ) ;
			// レンダリング設定
			virtual void OnSetupSceneSettings
				( S3DScene& scene,
					const SGLSize& sizeFrame,
					SGLSecondaryViewProducer * psvp = NULL ) ;
			// レンダリング後始末
			virtual void OnShoutdownSceneSettings
				( S3DScene& scene,
					SGLSecondaryViewProducer * psvp = NULL ) ;
			// 拡張的な処理の通知
			virtual void OnExtendNotify
				( const wchar_t * pwszCmd, const wchar_t * pwszParam,
					const void * pExParam, size_t nExParamBytes ) ;
			// タイマ処理 Space::OnTimer / ItemSerializer::OnTimer 実装
			virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;
			// フレーム更新後処理
			virtual void OnUpdateFrame( double fpFrame, SeekMethod seek ) ;

		public:	// S3DScene::Space
			// レンダリング前後処理（全視点共通）
			virtual void OnRenderEvent
				( S3DScene& scene, S3DScene::ItemClass clsItem ) ;

		public:
			// Loquaty クラス名
			virtual const wchar_t * GetLQClassName( void ) const ;

			friend class Composition ;
		} ;

		// スクリプト・ユーザー・データ・コンテナ
		typedef	AntirrhinumGL::AGLScriptObject	ScriptObject ;

		// コンポジション・インスタンス
		class	Composition	: public SpaceSerializer
		{
		protected:
			// 生成情報
			S3DSceneComposer *				m_pMasterComposer ;
			Composition *					m_pOwnerComposition ;
			SSystem::SSmartReference
						<CompositionInfo>	m_refCompositionInfo ;
			SSystem::SSmartReference<S3DScene::Item>
											m_refOwnerItem ;
			S3DScene *						m_pTargetScene ;
			S3DCompositionEditorInterface *	m_pEditorInteface ;
			bool							m_flagParseAsSubComp ;

			// 名前参照
			SSystem::SStrSortObjectArray<ItemSerializer>	m_ssoaItems ;

			// フレーム管理
			double			m_fpCurrentFrame ;
			double			m_fpNextJumpFrame ;
			double			m_fpFrameSpeed ;
			bool			m_flagFinished ;
			bool			m_flagPlaying ;
			bool			m_flagPaused ;
			bool			m_flagPauseTimer ;
			bool			m_flagTimerByFrame ;
			bool			m_flagFrameByTimer ;
			bool			m_flagFrameSpeedByTimer ;
			bool			m_flagEditMode ;
			bool			m_flagPostJump ;
			atomic_int_t	m_nPausedCounter ;

			bool			m_flagFixTimerInterval ;
			int32_t			m_msecTimerInterval ;
			int32_t			m_msecOddTimer ;

			// 遅延削除情報
			struct	DelayRemoveEntry
			{
				SpaceSerializer *	pParent ;
				ItemSerializer *	pItem ;
				S3DScene::Space *	pChild ;
			} ;
			SSystem::SArray<DelayRemoveEntry>	m_aDelayRemoveSpace ;
			SSystem::SArray<DelayRemoveEntry>	m_aDelayRemoveItem ;
			SSystem::SCriticalSection			m_csDelayRemove ;

			// ユーザー・データ・コンテナ
			ScriptObject	m_instance ;

		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( Composition, SpaceSerializer )
			// 構築関数
			Composition
				( S3DSceneComposer * pComposer = NULL,
					const CompositionInfo * pciCreateRef = NULL,
					Composition * pOwnerComposition = NULL,
					bool flagParseAsSubComp = false ) ;
			Composition
				( const wchar_t * pwszClassID,
					const ParamSetClass * pClass,
					S3DSceneComposer * pComposer = NULL,
					const CompositionInfo * pciCreateRef = NULL,
					Composition * pOwnerComposition = NULL,
					bool flagParseAsSubComp = false ) ;
			// 消滅関数
			virtual ~Composition( void ) ;
			// 生成情報取得
			S3DSceneComposer * GetSceneComposer( void ) const
			{
				return	m_pMasterComposer ;
			}
			const CompositionInfo * GetCompositionInfo( void ) const
			{
				return	m_refCompositionInfo.GetReference() ;
			}
			Composition * GetOwnerComposition( void ) const
			{
				return	m_pOwnerComposition ;
			}
			// 表示設定を適用
			void ApplySceneParameters
				( S3DScene& scene, const SGLSize& sizeView ) const ;
			// ターゲット S3DScene を設定する
			void AttachTargetScene( S3DScene * pScene ) ;
			// レンダリングの為のデバイスリソース準備（Scene に追加する前用）
			void PrepareToRender
				( S3DRenderDevice * pDevice, S3DScene * pScene, uint32_t nFlags = 0 ) ;

		public:
			// コンポジション・パースモード
			virtual bool IsSubCompositionToParseChildren( void ) const ;
			// サブコンポジションの親アイテム
			void AttachOwnerItem( S3DScene::Item * pItem ) ;
			S3DScene::Item * GetOwnerItem( void ) const ;
			// グローバル空間変換行列を計算
			virtual void CalcGlobalTransformation
				( S3DDMatrix& matrix, S3DDVector& pos ) const ;
			virtual const S3DDVector&
					CalcGlobalPosition( S3DDVector& pos ) const ;
			// 相対空間変換行列を計算
			virtual void CalcOffsetTransformation
				( S3DDMatrix& matrix, S3DDVector& pos,
								const Space * pStandardSpace ) const ;

		public:	// ItemSerializer
			// コンポジション取得
			virtual Composition * GetComposition( void ) const ;
			// シーン取得
			virtual S3DScene * GetScene( void ) const ;
			// アイテム空間変換行列（アイテム自身を含まないグローバル変換）を計算
			virtual void GetItemLinkTransformation
					( S3DDMatrix& matLink, S3DDVector& vLinkPos ) const ;
			// 空間行列取得
			virtual void GetGlobalTransformation
				( S3DDMatrix& matGlobal, S3DDVector& vGlobalPos ) const ;
			// 空間色効果取得（乗算色α要素に不透明度取得）
			virtual void GetGlobalColorEffect( S3DColor& clrEffect ) const ;
			// エディター取得
			virtual S3DCompositionEditorInterface * GetEditor( void ) const ;
			// フレームの初期化処理（デフォルトは0フレームへのシーク）
			virtual void InitializeFrameParameters( void ) ;
			// レンダリング設定
			virtual void OnSetupSceneSettings
				( S3DScene& scene,
					const SGLSize& sizeFrame,
					SGLSecondaryViewProducer * psvp = NULL ) ;
			// レンダリング後始末
			virtual void OnShoutdownSceneSettings
				( S3DScene& scene,
					SGLSecondaryViewProducer * psvp = NULL ) ;

		public:	// SpaceSerializer
			// フレーム反映
			virtual void SetAllItemsFrame( double fpFrame, SeekMethod seek ) ;
			// フレーム更新後処理
			virtual void OnUpdateFrame( double fpFrame, SeekMethod seek ) ;
			// タイマ処理
			virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;

		public:	// S3DScene::Space
			// アイテム作用の追加処理
			virtual void OnUpdateBehavior( S3DScene& scene ) ;

		public:
			// 再生中フレーム番号取得
			double GetCurrentPlayingFrame( void ) const ;
			// 初期化処理通知（CmdInitializeItem 通知）
			void InitializeItems( S3DScene * pScene = NULL ) ;
			// コンポジションの再生開始（Timer駆動）（CmdStartItem 通知あり）
			void PlayComposition( void ) ;
			// コンポジションの停止（CmdStopItem 通知あり）
			void StopCompositoin( void ) ;
			// コンポジションの再生開始（Timer駆動）（CmdStartItem 通知なし）
			void RestartComposition( void ) ;
			// コンポジションの停止（CmdStopItem 通知なし）
			void PauseCompositoin( void ) ;
			// コンポジションの終了（CmdFinishItem 通知あり）
			void FinishComposition( void ) ;
			// コンポジションの再生中か？
			bool IsPlayingComposition( void ) const ;
			// コンポジションの一時停止中か？
			bool IsPausedComposition( void ) const ;
			// コンポジション終了か？
			bool IsCompositionFinished( void ) const ;
			// OnTimer 再開／停止
			void EnableTimerEvent( bool flagEnable, bool flagTimerByFrame = false ) ;
			bool IsEnabledTimerEvent( void ) const ;
			bool IsTimerByFrame( void ) const ;
			// OnTimer 駆動での OnUpdateFrame 呼び出しの有効化
			void EnableFrameOnTimer( bool flagEnable ) ;
			// フレームを 1.0 単位で SetAllItemsFrame, OnTimer を呼び出すか？
			void SetFixTimerInterval( bool flagFixTimer ) ;
			bool IsFixedTimerInterval( void ) const ;
			// フレーム進行速度（フレーム比）の設定（FixTimer 時に有効）
			void SetFrameSpeed( double fpFrameSpeed, bool flagEnable = true ) ;
			bool GetFrameSpeedFlag( void ) const ;
			double GetFrameSpeed( void ) const ;
			// ジャンプ先フレーム設定
			void PostTimelineFrame( double fpFrame ) ;
			// PostTimelineFrame で設定したジャンプ先へ未移行か？
			bool IsPendingPostTimeline( void ) const ;

		public:
			// コンポジション終了フラグのリセット
			void ResetCompositionFinished( void ) ;
			// ジャンプ先フレームが設定されたかの判定とジャンプ先の取得と削除
			bool GetPostTimelineFrame( double& fpFrame ) ;
			// フレーム進行（マニュアル実行）
			void AdvanceCompositionTime( S3DScene& scene, uint32_t msecPast ) ;
			void AdvanceCompositionFrame
				( S3DScene& scene,
					const CompositionInfo * pci, double nNextFrame, bool flagJump ) ;

		public:
			// エディットモードか？
			bool IsEditMode( void ) const ;
			// エディットモード設定
			void SetEditMode( bool flagEdit ) ;
			// エディター・インターフェース関連付け
			void AttachEditor( S3DCompositionEditorInterface * pEditor ) ;

		public:
			// 子アイテムを登録と追加
			void AddSpaceChild
				( SpaceSerializer& space,
					ItemSerializer * pItem, const wchar_t * pwszID ) ;
			// 子アイテムを削除
			SGLError RemoveSpaceChild
				( SpaceSerializer& space, ItemSerializer * pItem ) ;
			// 子アイテムを削除
			// （アイテム自体は遅延削除に追加し、親子関係は即時解消）
			SGLError DelayRemoveSpaceChild
				( SpaceSerializer& space, ItemSerializer * pItem ) ;
			SGLError DetachSpaceChild
				( SpaceSerializer& space, ItemSerializer * pItem ) ;
			// 遅延削除リストに追加
			void PostDelayRemoveItem
				( SpaceSerializer& space, ItemSerializer * pItem ) ;
			// 遅延削除実行
			void FlushDelayRemoveItems( void ) ;

		protected:
			void PostDelayRemoveItemEntry
				( SSystem::SArray<DelayRemoveEntry>& list,
								const DelayRemoveEntry& dre ) ;
			void RemoveDelayItemEntries
				( SSystem::SArray<DelayRemoveEntry>& list ) ;

		public:
			// アイテムIDが重複する場合、固有IDを生成する
			void NormalizeSceneItemID( SSystem::SString& strID ) const ;
			// アイテム登録
			void RegisterSceneItem
				( const wchar_t * pwszID, ItemSerializer * pItem ) ;
			// アイテム登録解除
			void UnregisterSceneItem( ItemSerializer * pItem ) ;
			// アイテムID変更
			SGLError ChangeSceneItemID
				( ItemSerializer * pItem, const wchar_t * pwszID ) ;
			// アイテム取得（コンポジションの階層は \ で区切る）
			ItemSerializer * GetSceneItemAs( const wchar_t * pwszID ) const ;
			// 空間取得（モデルアイテムのボーンは @ で区切る）
			S3DScene::Space * GetSceneSpaceAs( const wchar_t * pwszID ) const ;
			// アイテムID取得
			const wchar_t * GetSceneItemIDOf( ItemSerializer * pItem ) const ;
			// 全アイテムID列挙
			void EnumerateAllItemIDs
				( SSystem::SObjectArray<SSystem::SString>& aIDs,
								const wchar_t * pwszBasePath = NULL ) const ;
			// 特定クラスアイテムID列挙
			void EnumerateItemIDsAs
				( SSystem::SObjectArray<SSystem::SString>& aIDs,
					const ESLRuntimeClass& rtClass,
					const wchar_t * pwszBasePath = NULL ) const ;
			// アイテム列挙
			void EnumerateItemsAs
				( SSystem::SPointerArray<ItemSerializer>& aItems,
									const ESLRuntimeClass& rtClass ) const ;

		public:
			// ユーザーデータ
			const ScriptObject& GetScriptInstance( void ) const ;
			void SetScriptInstance( const ScriptObject& instance ) ;
			const Rosetta::RSSmartPtr& GetUserInstance( void ) const ;
			void SetUserInstance( const Rosetta::RSSmartPtr& ptrInstance ) ;
			const Loquaty::LObjPtr& GetLoquatyInstance( void ) const ;
			void SetLoquatyInstance( const Loquaty::LObjPtr& pObj ) ;

		public:
			// Loquaty クラス名
			virtual const wchar_t * GetLQClassName( void ) const ;

			friend class S3DSceneComposer ;
		} ;

		// リソース・プラグイン
		class	ResourcePlugin	: public ESLObject
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( ResourcePlugin, ESLObject )
			// 構築関数
			ResourcePlugin( void ) { }
			ResourcePlugin( const ResourcePlugin& src ) { }
			// 消滅関数
			virtual ~ResourcePlugin( void ) { }
			// 種別
			virtual const wchar_t * ResourceType( void ) = 0 ;
			// 読み込み処理
			virtual SObject * ReadResource( SSystem::SFileInterface & file ) = 0 ;
		} ;

		// インポート情報
		class	ImportEntry	: public ESLObject
		{
		protected:
			SSystem::SString		m_strSrcFile ;
			S3DCompositionManager *	m_pManager ;
			S3DSceneComposer *		m_pComposer ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( ImportEntry, ESLObject )
			// 構築関数
			ImportEntry( S3DCompositionManager * pManager,
								S3DSceneComposer * pComposer ) ;
			// 消滅関数
			virtual ~ImportEntry( void ) ;
			// アンロード
			void UnloadComposer( void ) ;
			// リロード
			SGLError ReloadComposer( S3DSceneComposer * pMaster ) ;
			// S3DSceneComposer 取得
			operator S3DSceneComposer * ( void ) const
			{
				return	m_pComposer ;
			}
			S3DSceneComposer * GetComposer( void ) const
			{
				return	m_pComposer ;
			}
			// S3DCompositionManager 取得
			S3DCompositionManager * GetManager( void ) const
			{
				return	m_pManager ;
			}
			// ソースファイル名
			const SSystem::SString& GetSourceFile( void ) const
			{
				return	m_strSrcFile ;
			}
			void SetSourceFile( const wchar_t * pwszSrcFile )
			{
				m_strSrcFile = pwszSrcFile ;
			}
		} ;

	protected:
		S3DCompositionManager *				m_pManager ;
		atomic_int_t						m_nRefCount ;
		SSystem::SObjectArray<ImportEntry>	m_imports ;

		// リソース資産
		ResourceAssets			m_assets ;
		SSystem::SFileOpener *	m_pRsrcFileOpener ;
		SSystem::SString		m_strRsrcBaseDir ;

		// パーツ・ライブラリ
		SSystem::SStrSortObjectArray<SSystem::SXMLDocument>	m_ssoaParts ;

		// コンポジション
		SSystem::SStrSortObjectArray<CompositionInfo>	m_ssoaCompositions ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DSceneComposer, SObject )
		// 構築関数
		S3DSceneComposer( S3DCompositionManager * pManager ) ;
		// 消滅関数
		virtual ~S3DSceneComposer( void ) ;
		// マネージャ取得
		S3DCompositionManager * GetManager( void ) const ;
		// 参照カウンタ
		void AddReference( void ) ;
		bool ReleaseRef( void ) ;

	public:
		// ファイル読み込み
		virtual SGLError LoadComposeFile( const wchar_t * pwszFileName ) ;
		virtual SGLError ReadComposeFile
					( SSystem::SFileInterface& file,
								const wchar_t * pwszBaseDir = NULL ) ;
		virtual SGLError ParseComposeFile
					( const SSystem::SXMLDocument& xmlDoc,
								const wchar_t * pwszBaseDir = NULL ) ;
		// ファイル書き出し
		virtual SGLError SaveComposeFile( const wchar_t * pwszFileName ) ;
		virtual SGLError ExportComposeFile( const wchar_t * pwszFileName ) ;
	protected:
		void ExportSubDirectory
			( ERISA::SGLArchiveFile& arcfile,
				SSystem::SStrSortArray<const ResourceContainer*>& ssaFiles,
				const wchar_t * pwszDirPath ) ;
		void CollectDirectory
			( ERISA::SGLArchiveFile::SDirectory& dirEntries,
				SSystem::SStrSortArray<const ResourceContainer*>& ssaCollected,
				SSystem::SStrSortArray<const ResourceContainer*>& ssaFiles,
				const wchar_t * pwszFileDir ) ;
		void WriteFileIntoArchive
			( ERISA::SGLArchiveFile& arcfile,
				const SSystem::SStrSortArray<const ResourceContainer*>& ssaFiles ) ;
	public:
		virtual SGLError WriteComposeFile
					( SSystem::SFileInterface& file,
								const wchar_t * pwszBaseDir = NULL ) ;
		virtual SGLError FormatComposeFile
					( SSystem::SXMLDocument& xmlDoc,
						const wchar_t * pwszBaseDir = NULL,
						SSystem::SStrSortArray<const ResourceContainer*> * pssaExport = NULL ) ;
		// リソース解放
		virtual void Release( void ) ;
		// アセットファイル読み込みファイルオープナー
		SSystem::SFileOpener * GetResourceFileOpener( void ) const ;
		void AttachResourceFileOpener( SSystem::SFileOpener * pOpener ) ;
		// アセットファイル読み込みベースディレクトリ
		const wchar_t * GetResourceBaseDirectory( void ) const ;
		void SetResourceBaseDirectory( const wchar_t * pwszBaseDir ) ;

	public:
		// リソース
		const ResourceAssets & GetAssets( void ) const
		{
			return	m_assets ;
		}
		ResourceAssets & Assets( void )
		{
			return	m_assets ;
		}
		// パーツ・スタイル取得
		SSystem::SXMLDocument *
			GetPartsStyleAs( const wchar_t * pwszID ) const
		{
			return	m_ssoaParts.GetAs( pwszID ) ;
		}
		const SSystem::SStrSortObjectArray<SSystem::SXMLDocument>&
											GetPartsStyleList( void ) const
		{
			return	m_ssoaParts ;
		}
		SSystem::SStrSortObjectArray<SSystem::SXMLDocument>& PartsStyleList( void )
		{
			return	m_ssoaParts ;
		}

	public:
		// PrepareToRenderDevice, UnprepareToRenderDevice 用フラグ
		enum	PrepareDeviceFlag
		{
			prepareImportAssets			= 0x0001,
			unprepareImportAssets		= prepareImportAssets,
			prepareCompressedTexture	= 0x0010,
		} ;
		// リソースをデバイス用にロードする
		SGLError PrepareToRenderDevice
			( S3DRenderDevice * pDevice, uint32_t nFlags = prepareImportAssets ) ;
		// デバイスリソースを解放する
		SGLError UnprepareToRenderDevice
			( S3DRenderDevice * pDevice, uint32_t nFlags = unprepareImportAssets ) ;
		// コンポジション取得
		CompositionInfo * GetCompositionAs
			( const wchar_t * pwszID, bool flagWithImport = true ) const ;
		CompositionInfo * GetCompositionAt( size_t i ) const ;
		// コンポジション数取得
		size_t GetCompositionCount( void ) const ;
		// コンポジション名取得
		const SSystem::SString * GetCompositionID( size_t i ) const ;
		// コンポジション検索
		ssize_t FindCompositionOf( const CompositionInfo * pciComp ) const ;
		// コンポジション追加
		void AddCompositionAs
			( const wchar_t * pwszID, CompositionInfo * pciComp ) ;
		// コンポジション削除
		void RemoveCompositionAs( const wchar_t * pwszID ) ;
		// コンポジションID列挙
		void EnumerateCompositionIDs
			( SSystem::SObjectArray<SSystem::SString>& aIDs,
							bool flagWithImport = true ) const ;

	public:
		// 表示用コンポジション生成
		virtual Composition * CreateComposition
			( const CompositionInfo& ci,
				Composition * pOwner = NULL, bool flagAsSubComp = false ) ;

	public:
		// コンポーザー・インポート
		SGLError AddImportComposer( const wchar_t * pwszFilePath ) ;
		// 全インポート・コンポーザーを読み込み直す
		SGLError ReloadAllImportComposers( void ) ;
		// インポート数取得
		size_t GetImportComposerCount( void ) const ;
		// インポート情報取得
		ImportEntry * GetImportComposerAt( size_t i ) const ;
		// インポートコンポーザー削除
		void RemoveImportComposerAt( size_t i ) ;

	public:
		// リソース追加
		virtual SGLError AddAssetResource
			( const wchar_t * pwszID, ESLObject * pRsrc,
				const wchar_t * pwszType,
				const wchar_t * pwszSrcFile = NULL,
				const wchar_t * pwszPath = NULL,
				const SSystem::SXMLDocument * pxmlOptions = NULL ) ;
		// プロシージャルリソース追加
		virtual SGLError AddProceduralResource
			( const wchar_t * pwszID,
				ResourceProcedure * pProc,
				const wchar_t * pwszType,
				const wchar_t * pwszPath = NULL,
				const SSystem::SXMLDocument * pxmlOptions = NULL ) ;
		// 全スクリプトを読み込み直す
		void ReloadAllScripts( bool flagReloadImportScripts = true ) ;
	public:
		// アセットフォーマット
		virtual SGLError FormatResource
			( SSystem::SXMLDocument& xmlRsrc,
				const wchar_t * pwszID,
				const ResourceContainer& rcRsrc,
				SSystem::SStrSortArray<const ResourceContainer*> * pssaExport = NULL ) ;
	protected:
		// アセット読み込み処理
		virtual SGLError ParseImportAssets
			( ResourceAssets& assets,
					const SSystem::SXMLDocument& xmlAssets,
					const wchar_t * pwszBasePath ) ;
		virtual SGLError ImportAssets
			( ResourceAssets& assets, const wchar_t * pwszAssetsFile ) ;
	public:
		virtual SGLError LoadAssetFile
			( ResourceAssets& assets,
				const SSystem::SXMLDocument& xmlTag,
				const wchar_t * pwszBaseTreePath ) ;
		virtual SGLError LoadAssetFile
			( ResourceAssets& assets,
				const wchar_t *	pwszTypeID,
				const wchar_t *	pwszRsrcID,
				const wchar_t *	pwszFilePath,
				const wchar_t * pwszTreePath,
				const SSystem::SXMLDocument * pxmlOptions ) ;
		virtual SGLError AddResourceAs
			( ResourceAssets& assets,
				const wchar_t * pwszID, ESLObject * pRsrc,
				const wchar_t * pwszType,
				const wchar_t * pwszSrcFile = NULL,
				const wchar_t * pwszPath = NULL,
				const SSystem::SXMLDocument * pxmlOptions = NULL ) ;
		virtual ESLObject * LoadAssetResourceObject
			( ResourceAssets& assets,
				const wchar_t * pwszTypeID,
				const wchar_t *	pwszRsrcID,
				const wchar_t * pwszFilePath,
				const SSystem::SXMLDocument * pxmlOptions ) ;
		virtual S3DModelBuffer * LoadModelAsset
			( ResourceAssets& assets,
				const wchar_t * pwszTypeID,
				const wchar_t * pwszFilePath,
				const SSystem::SXMLDocument * pxmlOptions ) ;
		virtual SGLImage * LoadImageAsset
			( ResourceAssets& assets,
				const wchar_t * pwszTypeID,
				const wchar_t *	pwszRsrcID,
				const wchar_t * pwszFilePath,
				const SSystem::SXMLDocument * pxmlOptions ) ;
		virtual SGLAudioPlayer * LoadAudioAsset
			( ResourceAssets& assets,
				const wchar_t * pwszTypeID,
				const wchar_t * pwszFilePath,
				const SSystem::SXMLDocument * pxmlOptions ) ;
		virtual SGLMediaPlayer * LoadMovieAsset
			( ResourceAssets& assets,
				const wchar_t * pwszTypeID,
				const wchar_t * pwszFilePath,
				const SSystem::SXMLDocument * pxmlOptions ) ;
		virtual S3DModelPoseLibrary * LoadPoseLibraryAsset
			( ResourceAssets& assets,
				const wchar_t * pwszTypeID,
				const wchar_t * pwszFilePath,
				const SSystem::SXMLDocument * pxmlOptions ) ;
		virtual S3DMaterialLibrary * LoadMaterialLibraryAsset
			( ResourceAssets& assets,
				const wchar_t * pwszTypeID,
				const wchar_t * pwszFilePath,
				const SSystem::SXMLDocument * pxmlOptions ) ;
		virtual UserShader * LoadUserShaderAsset
			( ResourceAssets& assets,
				const wchar_t * pwszTypeID,
				const wchar_t *	pwszRsrcID,
				const wchar_t * pwszFilePath,
				const SSystem::SXMLDocument * pxmlOptions ) ;
		virtual SGLBasicFormParser * LoadBasicFormAsset
			( ResourceAssets& assets,
				const wchar_t * pwszTypeID,
				const wchar_t * pwszFilePath,
				const SSystem::SXMLDocument * pxmlOptions ) ;
		virtual SSystem::SStringParser * LoadRosettaScriptAsset
			( ResourceAssets& assets,
				const wchar_t * pwszTypeID,
				const wchar_t * pwszFilePath,
				const SSystem::SXMLDocument * pxmlOptions ) ;
		virtual AntirrhinumGL::AGLModulePtr * LoadAntirrhinumScriptAsset
			( ResourceAssets& assets,
				const wchar_t * pwszTypeID,
				const wchar_t * pwszFilePath,
				const SSystem::SXMLDocument * pxmlOptions ) ;
		virtual SSystem::SByteBuffer * LoadBinaryAsset
			( ResourceAssets& assets,
				const wchar_t * pwszTypeID,
				const wchar_t * pwszFilePath,
				const SSystem::SXMLDocument * pxmlOptions ) ;
		virtual ESLObject * LoadLoquatyScriptAsset
			( ResourceAssets& assets,
				const wchar_t * pwszTypeID,
				const wchar_t * pwszFilePath,
				const SSystem::SXMLDocument * pxmlOptions ) ;
		virtual ESLObject * LoadExtendAssetFile
			( ResourceAssets& assets,
				const wchar_t * pwszTypeID,
				const wchar_t * pwszFilePath,
				const SSystem::SXMLDocument * pxmlOptions ) ;

	public:	// リソース読み込みオプション
		// アニメーション画像／テクスチャオプション
		struct	LoadImageOption
		{
			uint32_t	nTextureFlags ;		// complex of enum SGLImageObject::BufferTypeFlag
			uint32_t	nNormalizeFlags ;	// complex of enum SGLImageObject::NormalizeFormatFlag
			uint32_t	format ;
			uint32_t	depth ;
			uint32_t	width ;
			uint32_t	height ;
			SGLPalette	argbBackColor ;

			LoadImageOption( void ) ;
			void ParseOptions( const SSystem::SXMLDocument& xmlOptions ) ;
			void FormatOptions( SSystem::SXMLDocument& xmlOptions ) ;

			static const SSystem::SXMLDocument::AttrInteger	m_aiTextureFlags[10] ;
			static const SSystem::SXMLDocument::AttrInteger	m_aiNormalizeFlags[10] ;
			static const SSystem::SXMLDocument::AttrInteger	m_aiFormatFlags[10] ;
		} ;

		// オーディオオプション
		struct	LoadAudioOption
		{
			uint64_t	nOpenFlags ;
			size_t		iVolumeLine ;

			LoadAudioOption( void ) ;
			void ParseOptions( const SSystem::SXMLDocument& xmlOptions ) ;
			void FormatOptions( SSystem::SXMLDocument& xmlOptions ) ;

			static const SSystem::SXMLDocument::AttrInteger	m_aiOpenFlags[10] ;
		} ;

		// 動画オプション
		struct	LoadMovieOption
		{
			uint64_t	nOpenFlags ;
			size_t		iVolumeLine ;

			LoadMovieOption( void ) ;
			void ParseOptions( const SSystem::SXMLDocument& xmlOptions ) ;
			void FormatOptions( SSystem::SXMLDocument& xmlOptions ) ;

			static const SSystem::SXMLDocument::AttrInteger	m_aiOpenFlags[10] ;
		} ;

	public:
		// S3DSceneComposer 生成
		virtual S3DSceneComposer * NewSceneComposer( void ) const ;
		// リソースファイルを開く
		virtual SSystem::SFileInterface * OpenAssetFile( const wchar_t * pwszFile ) ;
		// アイテム生成
		virtual ItemSerializer * CreateSceneItem( const wchar_t * pwszType ) ;
		// コントローラー生成
		virtual Controller * CreateController( const wchar_t * pwszClass ) ;

	public:
		// エディタ・インターフェース取得
		virtual S3DCompositionEditorInterface * GetEditorInterface( void ) ;
		// エラー出力
		virtual void OutputError
			( const wchar_t * pwszErrMsg,
				const wchar_t * pwszSrcFile = NULL,
				int nLineNum = 0, const wchar_t * pwszSrcLine = NULL ) ;
		// デバッグ用ログ出力
		virtual void OutputTraceLog( const wchar_t * pwszLogMsg ) ;
		// Rosetta スクリプトの例外を出力
		virtual void OutputRosettaException( Rosetta::RSContext& context ) ;

	public:
		// 行列シーケンサ
		class	MatrixSequencer	: public Sequencer
		{
		protected:
			S3DDMatrix					m_matDefault ;
			SSystem::SArray<S3DDMatrix>	m_arrValues ;
			SSystem::SArray<S3DDMatrix>	m_arrFrames ;
			size_t						m_iFirstFrame ;
			bool						m_fEachFrames ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( MatrixSequencer, Sequencer )
			// 構築関数
			MatrixSequencer( void ) ;
			MatrixSequencer( const MatrixSequencer& seq ) ;
			// 消滅関数
			virtual ~MatrixSequencer( void ) ;
		public:	// ParameterInterface
			// パラメータ値取得
			virtual S3DDMatrix GetMatrixParameter( size_t i ) const ;
			// パラメータ値設定
			virtual void SetMatrixParameter( size_t i, const S3DDMatrix& mat ) ;
		public:	// Sequencer
			// キーフレーム挿入
			virtual void InsertKeyFrame
					( size_t i, const KeyFrameParam& kfp ) ;
			// キーフレーム削除
			virtual void RemoveKeyFrame( size_t i ) ;
			// キーフレームから各フレームの値を計算する
			virtual void UpdateAllFrameValues( void ) ;
			// キーフレームの順番を入れ替える
			virtual void SwapKeyFrame( size_t i1, size_t i2 ) ;
			// アニメーションフレーム解釈
			virtual SGLError ParseFrameSequence
				( const SSystem::SXMLDocument& xmlSeq, ParameterType type ) ;
			// アニメーションフレーム保存
			virtual SGLError FormatFrameSequence
					( SSystem::SXMLDocument& xmlSeq, ParameterType type ) ;
			// 補完パラメータ値取得
			virtual S3DDMatrix GetFrameMatrix( double fpFrame ) ;
		public:
			// フレームシーケンス作成
			virtual SGLError CreateFrameSequence
						( size_t iFirstFrame, size_t nDuration ) ;
			// フレーム値設定
			virtual SGLError SetFrameMatrix( size_t iFrame, const S3DDMatrix& mat ) ;
			// 複製
			virtual Sequencer * DuplicateSequencer( void ) ;
		} ;
		// 回転シーケンサ
		class	RotationSequencer	: public Sequencer
		{
		protected:
			S3DDQuaternion					m_qDefault ;
			SSystem::SArray<S3DDQuaternion>	m_arrValues ;
			SSystem::SArray<S3DDQuaternion>	m_arrFrames ;
			size_t							m_iFirstFrame ;
			bool							m_fEachFrames ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( RotationSequencer, Sequencer )
			// 構築関数
			RotationSequencer( void ) ;
			RotationSequencer( const RotationSequencer& seq ) ;
			// 消滅関数
			virtual ~RotationSequencer( void ) ;
		public:	// ParameterInterface
			// パラメータ値取得
			virtual S3DDMatrix GetMatrixParameter( size_t i ) const ;
			// パラメータ値設定
			virtual void SetMatrixParameter( size_t i, const S3DDMatrix& mat ) ;
		public:	// Sequencer
			// キーフレーム挿入
			virtual void InsertKeyFrame
					( size_t i, const KeyFrameParam& kfp ) ;
			// キーフレーム削除
			virtual void RemoveKeyFrame( size_t i ) ;
			// キーフレームから各フレームの値を計算する
			virtual void UpdateAllFrameValues( void ) ;
			// キーフレームの順番を入れ替える
			virtual void SwapKeyFrame( size_t i1, size_t i2 ) ;
			// アニメーションフレーム解釈
			virtual SGLError ParseFrameSequence
				( const SSystem::SXMLDocument& xmlSeq, ParameterType type ) ;
			// アニメーションフレーム保存
			virtual SGLError FormatFrameSequence
					( SSystem::SXMLDocument& xmlSeq, ParameterType type ) ;
			// 補完パラメータ値取得
			virtual S3DDMatrix GetFrameMatrix( double fpFrame ) ;
		public:
			// フレームシーケンス作成
			virtual SGLError CreateFrameSequence
						( size_t iFirstFrame, size_t nDuration ) ;
			// フレーム値設定
			virtual SGLError SetFrameMatrix( size_t iFrame, const S3DDMatrix& mat ) ;
			// 複製
			virtual Sequencer * DuplicateSequencer( void ) ;
		} ;
		// ベクタシーケンサ
		class	VectorSequencer	: public Sequencer
		{
		protected:
			S3DDVector					m_vDefault ;
			SSystem::SArray<S3DDVector>	m_arrValues ;
			SSystem::SArray<S3DDVector>	m_arrFrames ;
			size_t						m_iFirstFrame ;
			bool						m_fEachFrames ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( VectorSequencer, Sequencer )
			// 構築関数
			VectorSequencer( void ) ;
			VectorSequencer( const VectorSequencer& seq ) ;
			// 消滅関数
			virtual ~VectorSequencer( void ) ;
		public:	// ParameterInterface
			// パラメータ値取得
			virtual S3DDVector GetVectorParameter( size_t i ) const ;
			// パラメータ値設定
			virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		public:	// Sequencer
			// キーフレーム挿入
			virtual void InsertKeyFrame
					( size_t i, const KeyFrameParam& kfp ) ;
			// キーフレーム削除
			virtual void RemoveKeyFrame( size_t i ) ;
			// キーフレームから各フレームの値を計算する
			virtual void UpdateAllFrameValues( void ) ;
			virtual void UpdateFrameValues
						( size_t iFirstKey, size_t iEndKey ) ;
			// キーフレームの順番を入れ替える
			virtual void SwapKeyFrame( size_t i1, size_t i2 ) ;
			// アニメーションフレーム解釈
			virtual SGLError ParseFrameSequence
				( const SSystem::SXMLDocument& xmlSeq, ParameterType type ) ;
			// アニメーションフレーム保存
			virtual SGLError FormatFrameSequence
					( SSystem::SXMLDocument& xmlSeq, ParameterType type ) ;
			// 補完パラメータ値取得
			virtual S3DDVector GetFrameVector( double fpFrame ) ;
			// 補完方法を変更
			void ChangeInterpolateMethod( InterpolateMethod itpl ) ;
		public:
			// フレームシーケンス作成
			virtual SGLError CreateFrameSequence
						( size_t iFirstFrame, size_t nDuration ) ;
			// フレーム値設定
			virtual SGLError SetFrameVector( size_t iFrame, const S3DDVector& vec ) ;
			// 複製
			virtual Sequencer * DuplicateSequencer( void ) ;
		} ;
		// スカラシーケンサ
		class	ScalarSequencer	: public Sequencer
		{
		protected:
			double					m_fpDefault ;
			SSystem::SArray<double>	m_arrValues ;
			SSystem::SArray<double>	m_arrFrames ;
			size_t					m_iFirstFrame ;
			bool					m_fEachFrames ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( ScalarSequencer, Sequencer )
			// 構築関数
			ScalarSequencer( void ) ;
			ScalarSequencer( const ScalarSequencer& seq ) ;
			// 消滅関数
			virtual ~ScalarSequencer( void ) ;
		public:	// ParameterInterface
			// パラメータ値取得
			virtual double GetScalarParameter( size_t i ) const ;
			// パラメータ値設定
			virtual void SetScalarParameter( size_t i, double s ) ;
		public:	// Sequencer
			// キーフレーム挿入
			virtual void InsertKeyFrame
					( size_t i, const KeyFrameParam& kfp ) ;
			// キーフレーム削除
			virtual void RemoveKeyFrame( size_t i ) ;
			// キーフレームから各フレームの値を計算する
			virtual void UpdateAllFrameValues( void ) ;
			// キーフレームの順番を入れ替える
			virtual void SwapKeyFrame( size_t i1, size_t i2 ) ;
			// アニメーションフレーム解釈
			virtual SGLError ParseFrameSequence
				( const SSystem::SXMLDocument& xmlSeq, ParameterType type ) ;
			// アニメーションフレーム保存
			virtual SGLError FormatFrameSequence
					( SSystem::SXMLDocument& xmlSeq, ParameterType type ) ;
			// 補完パラメータ値取得
			virtual double GetFrameScalar( double fpFrame ) ;
		public:
			// フレームシーケンス作成
			virtual SGLError CreateFrameSequence
						( size_t iFirstFrame, size_t nDuration ) ;
			// フレーム値設定
			virtual SGLError SetFrameScalar( size_t iFrame, double s ) ;
			// 複製
			virtual Sequencer * DuplicateSequencer( void ) ;
		} ;
		// 整数シーケンサ
		class	IntegerSequencer	: public Sequencer
		{
		protected:
			int32_t						m_nDefault ;
			SSystem::SArray<int32_t>	m_arrValues ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( IntegerSequencer, Sequencer )
			// 構築関数
			IntegerSequencer( void ) ;
			IntegerSequencer( const IntegerSequencer& seq ) ;
			// 消滅関数
			virtual ~IntegerSequencer( void ) ;
		public:	// ParameterInterface
			// パラメータ値取得
			virtual int32_t GetIntegerParameter( size_t i ) const ;
			// パラメータ値設定
			virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		public:	// Sequencer
			// キーフレーム挿入
			virtual void InsertKeyFrame
					( size_t i, const KeyFrameParam& kfp ) ;
			// キーフレーム削除
			virtual void RemoveKeyFrame( size_t i ) ;
			// キーフレームの順番を入れ替える
			virtual void SwapKeyFrame( size_t i1, size_t i2 ) ;
			// 補完パラメータ値取得
			virtual int32_t GetFrameInteger( double fpFrame ) ;
			// 複製
			virtual Sequencer * DuplicateSequencer( void ) ;
		} ;
		// ブーリアンシーケンサ
		class	BooleanSequencer	: public Sequencer
		{
		protected:
			bool					m_fDefault ;
			SSystem::SArray<bool>	m_arrValues ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( BooleanSequencer, Sequencer )
			// 構築関数
			BooleanSequencer( void ) ;
			BooleanSequencer( const BooleanSequencer& seq ) ;
			// 消滅関数
			virtual ~BooleanSequencer( void ) ;
		public:	// ParameterInterface
			// パラメータ値取得
			virtual bool GetBooleanParameter( size_t i ) const ;
			// パラメータ値設定
			virtual void SetBooleanParameter( size_t i, bool b ) ;
		public:	// Sequencer
			// キーフレーム挿入
			virtual void InsertKeyFrame
					( size_t i, const KeyFrameParam& kfp ) ;
			// キーフレーム削除
			virtual void RemoveKeyFrame( size_t i ) ;
			// キーフレームの順番を入れ替える
			virtual void SwapKeyFrame( size_t i1, size_t i2 ) ;
			// 補完パラメータ値取得
			virtual bool GetFrameBoolean( double fpFrame ) ;
			// 複製
			virtual Sequencer * DuplicateSequencer( void ) ;
		} ;
		// N次元ベクタシーケンサ
		class	VectorXSequencer	: public Sequencer
		{
		protected:
			SSystem::SArray<double>	m_arrDefault ;
			SSystem::SArray<double>	m_arrValues ;
			size_t					m_nDimension ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( VectorXSequencer, Sequencer )
			// 構築関数
			VectorXSequencer( size_t nDim ) ;
			VectorXSequencer( const VectorXSequencer& seq ) ;
			// 消滅関数
			virtual ~VectorXSequencer( void ) ;
			// 次元数取得
			size_t GetDimension( void ) const ;
		public:	// ParameterInterface
			// パラメータ値取得
			virtual size_t GetBinaryParameter
				( void * pDst, size_t nBufBytes, size_t i ) const ;
			// パラメータ値設定
			virtual size_t SetBinaryParameter
				( size_t i, const void * pSrc, size_t nBufBytes ) ;
		public:	// Sequencer
			// キーフレーム挿入
			virtual void InsertKeyFrame
					( size_t i, const KeyFrameParam& kfp ) ;
			// キーフレーム削除
			virtual void RemoveKeyFrame( size_t i ) ;
			// キーフレームの順番を入れ替える
			virtual void SwapKeyFrame( size_t i1, size_t i2 ) ;
			// 補完パラメータ値取得
			virtual size_t GetFrameBinary
				( void * pDst, size_t nBufBytes, double fpFrame ) const ;
			// 複製
			virtual Sequencer * DuplicateSequencer( void ) ;
		} ;
		// コマンドシーケンサ
		class	CommandSequencer	: public Sequencer
		{
		protected:
			SSystem::SObjectArray<SSystem::SString>	m_arrValues ;
			SSystem::SString	m_strDefault ;
			ssize_t				m_iLastKeyFrame ;
			SSystem::SString	m_strLastFrameCmd ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( CommandSequencer, Sequencer )
			// 構築関数
			CommandSequencer( void ) ;
			CommandSequencer( const CommandSequencer& seq ) ;
			// 消滅関数
			virtual ~CommandSequencer( void ) ;
		public:	// ParameterInterface
			// パラメータ値取得
			virtual const wchar_t * GetCommandParameter( size_t i ) const ;
			// パラメータ値設定
			virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		public:	// Sequencer
			// キーフレーム挿入
			virtual void InsertKeyFrame
					( size_t i, const KeyFrameParam& kfp ) ;
			// キーフレーム削除
			virtual void RemoveKeyFrame( size_t i ) ;
			// キーフレームの順番を入れ替える
			virtual void SwapKeyFrame( size_t i1, size_t i2 ) ;
			// 補完パラメータ値取得
			virtual const wchar_t * GetFrameCommand( double fpFrame, SeekMethod seek ) ;
			// 複製
			virtual Sequencer * DuplicateSequencer( void ) ;
		} ;
		// ポーズシーケンサ
		class	PoseSequencer	: public Sequencer
		{
		protected:
			SSystem::SObjectArray<SSystem::SString>	m_arrValues ;
			SSystem::SArray<PoseInstance>			m_arrPoses ;
			PoseInstance							m_poseDefault ;
			S3DModelPoseLibrary						m_libPose ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( PoseSequencer, Sequencer )
			// 構築関数
			PoseSequencer( void ) ;
			PoseSequencer( const PoseSequencer& seq ) ;
			// 消滅関数
			virtual ~PoseSequencer( void ) ;
			// 参照ライブラリを設定する
			void SetReferenceLibrary( S3DModelPoseLibrary * pLib ) ;
			// テンポラリポーズライブラリを取得
			const S3DModelPoseLibrary& GetTemporaryPose( void ) const ;
		public:
			// シーケンサー解釈
			virtual SGLError ParseSequencer
				( const SSystem::SXMLDocument& xmlTag, ParameterType type ) ;
			// シーケンサー保存
			virtual SGLError FormatSequencer
					( SSystem::SXMLDocument& xmlTag,
							ParameterType type, uint32_t nFlags = 0 ) ;
			// 使用されていないテンポラリポーズを削除する
			void CleanupTemporaryPose( void ) ;
		public:
			// ポーズ文字列解釈 <pose-identity>[,<time-in-second>]
			void ParsePoseInstance( PoseInstance& pose, const wchar_t * pwszPose ) const ;
			static SSystem::SString ParsePoseUsage
					( double& secPose, const wchar_t * pwszPose ) ;
			// ポーズ解釈（完全版） <pose0-id>[,<time0>[,<pose1-id>,<time>,<trans>]]
			virtual void ParsePoseString
					( PoseInstance& pose, const wchar_t * pwszPose ) const ;
			// ポーズ文字列書式化
			SSystem::SString
					FormatPoseString( const PoseInstance& pose ) const ;
			// ユニークな新しいポーズIDを生成する
			SSystem::SString NewUniquePoseID( void ) const ;
			// テンポラリポーズ生成
			S3DModelPose * NewTemporaryPose( SSystem::SString& strID ) ;
			// テンポラリポーズ取得
			S3DModelPose *
				GetTemporaryPoseIdentityAs( const wchar_t * pwszPoseID ) const ;
			// ポーズID取得
			bool GetPoseIdentityOf
				( SSystem::SString& strPoseID, S3DModelPose * pPose ) const ;
			// テンポラリポーズID取得
			bool GetTemporaryPoseIdentityOf
				( SSystem::SString& strPoseID, S3DModelPose * pPose ) const ;
			// テンポラリポーズ判定
			bool IsTemporaryPose( S3DModelPose * pPose ) const ;
		public:	// ParameterInterface
			// パラメータ値取得
			virtual const wchar_t * GetCommandParameter( size_t i ) const ;
			virtual PoseInstance GetPoseParameter( size_t i ) const ;
			// パラメータ値設定
			virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
			virtual void SetPoseParameter( size_t i, const PoseInstance& pose ) ;
		public:	// Sequencer
			// キーフレーム挿入
			virtual void InsertKeyFrame
					( size_t i, const KeyFrameParam& kfp ) ;
			// キーフレーム削除
			virtual void RemoveKeyFrame( size_t i ) ;
			// キーフレームの順番を入れ替える
			virtual void SwapKeyFrame( size_t i1, size_t i2 ) ;
			// 補完パラメータ値取得
			virtual PoseInstance GetFramePoseInstance( double fpFrame ) ;
			// 複製
			virtual Sequencer * DuplicateSequencer( void ) ;
			// 作成時の処理
			virtual void OnSequencerAttached( ParameterProperty * pItem ) ;
			// リソース等の参照を更新する
			virtual uint32_t UpdatePropertyReference
				( S3DSceneComposer::Composition& comp,
						ItemSerializer * pItem, uint32_t nFlags ) ;
		} ;

	public:
		// アイテム・基底シリアライザ
		class	ItemCommonSerializer : public CommonSerializer
		{
		public:
			enum	ParameterIndex
			{
				paramItemClass			= CommonSerializer::paramCount,
				paramItemPriority,
				paramItemTotalCount,
				paramItemCount		= paramItemTotalCount - paramItemClass,
			} ;
			static const ParamEntry		m_paramEntries[paramItemCount] ;
			static const ParamSetClass	m_pscClass ;
			static const wchar_t *		m_pwszItemClassIDs[S3DScene::classCount] ;

			S3DScene::Item *	m_pItem ;

		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( ItemCommonSerializer, CommonSerializer )
			// 構築関数
			ItemCommonSerializer
				( const wchar_t * pwszClassID,
					const ParamSetClass * pClass, S3DScene::Item * pItem ) ;
			// アイテム関連付け
			void AttachSceneItem( S3DScene::Item * pItem ) ;
			// アイテム取得
			S3DScene::Item * GetSceneItem( void ) const
			{
				return	m_pItem ;
			}
			// 親空間取得
			virtual ItemSerializer * GetParentSpaceItem( void ) const ;
			// コンポジション検索
			virtual Composition * GetComposition( void ) const ;
			// アイテム空間変換行列（アイテム自身を含まないグローバル変換）を計算
			virtual void GetItemLinkTransformation
					( S3DDMatrix& matLink, S3DDVector& vLinkPos ) const ;
			// 空間行列取得
			virtual void GetGlobalTransformation
				( S3DDMatrix& matGlobal, S3DDVector& vGlobalPos ) const ;
			// 空間色効果取得（乗算色α要素に不透明度取得）
			virtual void GetGlobalColorEffect( S3DColor& clrEffect ) const ;
			// アイテムID設定
			virtual void SetItemIdentity( const wchar_t * pwszID ) ;

		public:
			// S3DScene::SpaceInfo 取得
			virtual const S3DScene::SpaceInfo& GetSpaceInfo( void ) const ;
			virtual S3DScene::SpaceInfo& SpaceParameter( void ) ;
			// 表示フラグ取得
			virtual bool GetVisibleParameter( void ) const ;
			// 表示フラグ設定
			virtual void SetVisibleParameter( bool b ) ;
			// 動作フラグ取得
			virtual uint32_t GetBehaviorFlags( void ) const ;
			// 動作フラグ設定
			virtual void SetBehaviorFlags( uint32_t nFlags ) ;

		public:	// Parameter
			// パラメータ値取得
			virtual int32_t GetIntegerParameter( size_t i ) const ;
			virtual const wchar_t * GetCommandParameter( size_t i ) const ;
			// パラメータ値設定
			virtual void SetIntegerParameter( size_t i, int32_t n ) ;
			virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
			// パラメータ値域列挙
			virtual bool EnumerateStringSet
				( size_t iParam, SSystem::SStringArray& aStrSet ) ;

		public:	// ItemSerializer
			// パラメーター有効性
			virtual bool IsParameterValidation( size_t i ) const ;
			// タイマー処理
			virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;
			// フレーム更新後処理
			virtual void OnUpdateFrame( double fpFrame, SeekMethod seek ) ;

		public:
			// Loquaty クラス名
			virtual const wchar_t * GetLQClassName( void ) const ;
		} ;

		// アイテム・シリアライザ
		class	ItemBasicSerializer
					: public S3DScene::Item, public ItemCommonSerializer
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO2( ItemBasicSerializer, Item, ItemCommonSerializer )
			// 構築関数
			ItemBasicSerializer
				( const wchar_t * pwszClassID,
					const ParamSetClass * pClass ) ;
		public:
			// タイマ処理 Item::OnTimer / ItemSerializer::OnTimer 実装
			virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;
			// アイテム空間変換行列（アイテム自身を含まないグローバル変換）を計算
			virtual void GetItemLinkTransformation
					( S3DDMatrix& matLink, S3DDVector& vLinkPos ) const ;
			// 空間行列取得
			virtual void GetGlobalTransformation
				( S3DDMatrix& matGlobal, S3DDVector& vGlobalPos ) const ;
			// シーン取得
			virtual S3DScene * GetScene( void ) const ;
		} ;

		// 光源アイテム・シリアライザ
		class	LightSerializer
					: public S3DScene::Light, public ItemCommonSerializer
		{
		public:
			enum	ParameterIndex
			{
				paramLightType		= ItemCommonSerializer::paramItemTotalCount,
				paramLightColor,
				paramLightBrightness,
				paramLightDirection,
				paramLightAttenuationPower,
				paramLightAngle,
				paramLightGradation,
				paramLightShadowMapping,
				paramShadowMapWidth,
				paramShadowMapHeight,
				paramShadowPixelDensity,
				paramShadowPersDistance,
				paramShadowAdjustByCamera,
				paramShadowCameraStdDistance,
				paramShadowPersNear,
				paramShadowPersFar,
				paramShadowErrorPrecision,
				paramShadowErrorSubPrecision,
				paramShadowFilterGauss,
				paramShadowCascadeCount,
				paramShadowCascadeDensity,
				paramShadowCascadeTargetLengthRatio,
				paramShadowCascadeTargetLengthOffset,
				paramShadowCascadeDepth,
				paramShadowCascadeDistanceOffsetStep,
				paramLightTotalCount,
				paramLightCount		= paramLightTotalCount - paramLightType,
			} ;
			enum	LightTypeIndex
			{
				typeLightAmbient,
				typeLightAmbientMul,
				typeLightVector,
				typeLightPoint,
				typeLightSpot,
				typeLightFog,
				typeLightCount,
			} ;
		protected:
			int			m_typeLight ;
			SGLPalette	m_rgbColor ;
			bool		m_flagShadowmapping ;
			double		m_degLightAngle ;
			double		m_degLightGradation ;

			static const ParamEntry		m_paramEntries[paramLightCount] ;
			static const ParamSetClass	m_pscClass ;
			static const wchar_t *		m_pwszLightTypeIDs[typeLightCount] ;
			static const uint32_t		m_nLightTypeFlag[typeLightCount] ;

		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO2
				( LightSerializer, Light, ItemCommonSerializer )
			S3D_DECLARE_COMPOSER_ITEM( LightSerializer, light )
			// 構築関数
			LightSerializer( void ) ;

		public:
			// 光源タイプ
			LightTypeIndex GetLightType( void ) const ;
			void SetLightType( LightTypeIndex type ) ;
			// 光源色
			SGLPalette GetLightColor( void ) const ;
			void SetLightColor( const SGLPalette& rgbColor ) ;
			// 輝度
			double GetLightBrightness( void ) const ;
			void SetLightBrightness( double fpBrightness ) ;
			// 点光源減衰力 x : (1/r^x)
			double GetAttenuationPower( void ) const ;
			void SetAttenuationPower( double fpAttenuation ) ;
			// 光源位置
			const S3DDVector& GetLightPosition( void ) const ;
			void SetLightPosition( const S3DDVector& vPos ) ;
			// 光源向き
			const S3DVector& GetLightDirection( void ) const ;
			void SetLightDirection( const S3DVector& vDir ) ;
			// スポットライト範囲角 [deg]
			double GetLightAngle( void ) const ;
			void SetLightAngle( double degAngle ) ;
			// スポットライトぼかし角 [deg]
			double GetLightGradation( void ) const ;
			void SetLightGradation( double degGradation ) ;
			// シャドウマッピング有効
			bool IsEnabledShadowMapping( void ) const ;
			void EnableShadowMapping( bool fShadowmap ) ;
			// シャドウマッピングパラメータ
			void GetShadowMappingParam
				( S3DScene::ShadowMapParam& param ) const ;
			void SetShadowMappingParam
				( const S3DScene::ShadowMapParam& param ) ;

		public:
			// パラメーター有効性
			virtual bool IsParameterValidation( size_t i ) const ;

		public:	// Parameter
			// パラメータ値取得
			virtual S3DDVector GetVectorParameter( size_t i ) const ;
			virtual double GetScalarParameter( size_t i ) const ;
			virtual int32_t GetIntegerParameter( size_t i ) const ;
			virtual bool GetBooleanParameter( size_t i ) const ;
			virtual const wchar_t * GetCommandParameter( size_t i ) const ;
			// パラメータ値設定
			virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
			virtual void SetScalarParameter( size_t i, double s ) ;
			virtual void SetIntegerParameter( size_t i, int32_t n ) ;
			virtual void SetBooleanParameter( size_t i, bool b ) ;
			virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
			// パラメータ値域列挙
			virtual bool EnumerateStringSet
				( size_t iParam, SSystem::SStringArray& aStrSet ) ;
			// パラメータカテゴリ名取得
			virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

		public:	// ItemSerializer
			// レンダリングの為のデバイスリソース準備
			virtual void OnPrepareToRender
				( S3DRenderDevice * pDevice, uint32_t nFlags = 0 ) ;

		public:
			// タイマ処理 Item::OnTimer / ItemSerializer::OnTimer 実装
			virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;

		public:
			// Loquaty クラス名
			virtual const wchar_t * GetLQClassName( void ) const ;
		} ;

		// カメラアイテム・シリアライザ
		class	CameraSerializer
					: public S3DScene::Camera, public ItemCommonSerializer
		{
		public:
			enum	ParameterIndex
			{
				paramCameraTarget		= ItemCommonSerializer::paramItemTotalCount,
				paramCameraTop,
				paramCameraHFOV,
				paramCameraForceHFOV,
				paramCameraLeftEar,
				paramCameraRightEar,
				paramCameraEarAngle,
				paramCameraBackVolume,
				paramCameraTotalCount,
				paramCameraCount		= paramCameraTotalCount - paramCameraTarget,
			} ;
		protected:
			static const ParamEntry		m_paramEntries[paramCameraCount] ;
			static const ParamSetClass	m_pscClass ;

		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO2
				( CameraSerializer, Camera, ItemCommonSerializer )
			S3D_DECLARE_COMPOSER_ITEM( CameraSerializer, camera )
			// 構築関数
			CameraSerializer( void ) ;

		public:
			// パラメーター有効性
			virtual bool IsParameterValidation( size_t i ) const ;

		public:	// Parameter
			// パラメータ値取得
			virtual S3DDVector GetVectorParameter( size_t i ) const ;
			virtual double GetScalarParameter( size_t i ) const ;
			virtual bool GetBooleanParameter( size_t i ) const ;
			// パラメータ値設定
			virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
			virtual void SetScalarParameter( size_t i, double s ) ;
			virtual void SetBooleanParameter( size_t i, bool b ) ;
			// パラメータカテゴリ名取得
			virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

		public:
			// タイマ処理 Item::OnTimer / ItemSerializer::OnTimer 実装
			virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;

		public:
			// Loquaty クラス名
			virtual const wchar_t * GetLQClassName( void ) const ;
		} ;

		// モデルアイテム・シリアライザ
		class	ModelSerializer
					: public S3DScene::ModelItem, public ItemCommonSerializer
		{
		public:
			enum	ParameterIndex
			{
				paramModel			= ItemCommonSerializer::paramItemTotalCount,
				paramCollision,
				paramColliderFlags,
				paramModelTotalCount,
				paramModelCount		= paramModelTotalCount - paramModel,
			} ;
		protected:
			static const ParamEntry		m_paramEntries[paramModelCount] ;
			static const ParamSetClass	m_pscClass ;

		protected:
			SSystem::SString	m_strModelID ;
			SSystem::SString	m_strCollisionID ;

		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO2
				( ModelSerializer, ModelItem, ItemCommonSerializer )
			S3D_DECLARE_COMPOSER_ITEM( ModelSerializer, model )
			// 構築関数
			ModelSerializer( void ) ;

		public:
			// モデル
			void SetModel( const wchar_t * pwszModelID ) ;
			void UpdateModel( void ) ;
			const wchar_t * GetModelID( void ) const ;
			// 衝突判定モデル
			void SetCollision( const wchar_t * pwszModelID ) ;
			void UpdateCollision( void ) ;
			const wchar_t * GetCollisionID( void ) const ;

		public:	// ItemSerializer
			// パラメーター有効性
			virtual bool IsParameterValidation( size_t i ) const ;
			// ポーズ取得
			virtual S3DModelPose *
				GetPoseIdentityAs( const wchar_t * pwszPoseID ) const ;
			// ポーズID取得
			virtual bool GetPoseIdentityOf
				( SSystem::SString& strPoseID, S3DModelPose * pPose ) const ;
			// アイテムのプライマリモデル取得
			virtual S3DVertexBufferInterface * GetItemPrimaryModel( void ) ;
			// アイテムのコリジョンバッファ取得
			virtual S3DCollider * GetItemPrimaryCollider( void ) ;

		public:	// Parameter
			// パラメータ値取得
			virtual int32_t GetIntegerParameter( size_t i ) const ;
			virtual const wchar_t * GetCommandParameter( size_t i ) const ;
			// パラメータ値設定
			virtual void SetIntegerParameter( size_t i, int32_t n ) ;
			virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
			// パラメータ値域列挙
			virtual bool EnumerateStringSet
				( size_t iParam, SSystem::SStringArray& aStrSet ) ;

		public:
			// タイマ処理 Item::OnTimer / ItemSerializer::OnTimer 実装
			virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;

		public:	// ItemSerializer
			// ポーズライブラリ取得
			virtual S3DModelPoseLibrary * GetPoseLibraryChain( void ) ;
			// アイテムプロパティのリソース等の参照を更新する
			virtual uint32_t UpdatePropertyReference
				( S3DSceneComposer::Composition& comp, uint32_t nFlags ) ;

		public:	// S3DScene::ModelItem
			// モデルデータ関連付け
			virtual void AttachModel( S3DVertexBufferInterface * pModel ) ;
			virtual void AttachCollisionModel
				( S3DVertexBufferInterface * pColModel, bool fBuildCollision = true ) ;
		} ;

		// コライダー・コントローラー
		class	ColliderController	: public Controller
		{
		public:
			enum	ParameterIndex
			{
				paramMarkerType,
				paramLeadMarkerID,
				paramColliderMask,
				paramCount,
			} ;

		protected:
			S3DModelData::MarkerInfo::Type	m_typeMarker ;
			SSystem::SString				m_strLeadMarkerID ;
			uint32_t						m_maskCollider ;

		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( ColliderController, Controller )
			S3D_DECLARE_COMPOSER_ITEM( ColliderController, collider_controller )
			// 構築関数
			ColliderController( void ) ;
			// 消滅関数
			virtual ~ColliderController( void ) ;

		public:	// Parameter
			// パラメータ値取得
			virtual int32_t GetIntegerParameter( size_t i ) const ;
			virtual const wchar_t * GetCommandParameter( size_t i ) const ;
			// パラメータ値設定
			virtual void SetIntegerParameter( size_t i, int32_t n ) ;
			virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
			// パラメータ値域列挙
			virtual bool EnumerateStringSet
				( size_t iParam, SSystem::SStringArray& aStrSet ) ;

		public:	// Controller
			// 当たり判定追加
			//（必要であれば scene.PhysicsScene() へ Actor の追加も行う）
			virtual void RenderCollision
				( const S3DScene& scene,
					ItemSerializer * pItem, S3DCollision& render ) ;
		} ;

	} ;

}

#include <sakuraglx/extra/sglx3d_scene_plugin.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// コンポーザー・エディター・インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	S3DCompositionEditorInterface	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DCompositionEditorInterface, ESLObject )

	public:
		// エラー出力
		virtual void OutputError
			( const wchar_t * pwszErrMsg,
				const wchar_t * pwszSrcFile = NULL,
				int nLineNum = 0, const wchar_t * pwszSrcLine = NULL ) = 0 ;
		// デバッグ用ログ出力
		virtual void OutputTraceLog( const wchar_t * pwszLogMsg ) = 0 ;
		// メッセージボックス表示
		virtual int DoMessageBox
			( const wchar_t * pwszMsg,
				const wchar_t * pwszCaption, int nStyles = 0 ) = 0 ;
		// メインウィンドウ取得
		virtual SakuraGL::Window * GetFrameWindow( void ) = 0 ;

	public:
		// プロパティ編集（UI表示反映・オートキーフレーム有効）
		virtual void EditMatrixProperty
			( S3DSceneComposer::Composition * pComp,
				S3DSceneComposer::ParameterProperty * pProp,
				size_t iParam, const S3DDMatrix& mat,
				bool flagAddUndo = true, const wchar_t * pwszUndoText = NULL ) = 0 ;
		virtual void EditVectorProperty
			( S3DSceneComposer::Composition * pComp,
				S3DSceneComposer::ParameterProperty * pProp,
				size_t iParam, const S3DDVector& vec,
				bool flagAddUndo = true, const wchar_t * pwszUndoText = NULL ) = 0 ;
		virtual void EditScalarProperty
			( S3DSceneComposer::Composition * pComp,
				S3DSceneComposer::ParameterProperty * pProp,
				size_t iParam, double s,
				bool flagAddUndo = true, const wchar_t * pwszUndoText = NULL ) = 0 ;
		virtual void EditIntegerProperty
			( S3DSceneComposer::Composition * pComp,
				S3DSceneComposer::ParameterProperty * pProp,
				size_t iParam, int32_t n,
				bool flagAddUndo = true, const wchar_t * pwszUndoText = NULL ) = 0 ;
		virtual void EditBooleanProperty
			( S3DSceneComposer::Composition * pComp,
				S3DSceneComposer::ParameterProperty * pProp,
				size_t iParam, bool b,
				bool flagAddUndo = true, const wchar_t * pwszUndoText = NULL ) = 0 ;
		virtual void EditCommandProperty
			( S3DSceneComposer::Composition * pComp,
				S3DSceneComposer::ParameterProperty * pProp,
				size_t iParam, const wchar_t * pwszCmd,
				bool flagAddUndo = true, const wchar_t * pwszUndoText = NULL ) = 0 ;
		virtual void EditPoseProperty
			( S3DSceneComposer::Composition * pComp,
				S3DSceneComposer::ParameterProperty * pProp,
				size_t iParam, const S3DSceneComposer::PoseInstance& pose,
				bool flagAddUndo = true, const wchar_t * pwszUndoText = NULL ) = 0 ;
		virtual size_t EditBinaryProperty
			( S3DSceneComposer::Composition * pComp,
				S3DSceneComposer::ParameterProperty * pProp,
				size_t iParam, const void * pSrc, size_t nBufBytes,
				bool flagAddUndo = true, const wchar_t * pwszUndoText = NULL ) = 0 ;
		virtual bool EditProperty
			( S3DSceneComposer::Composition * pComp,
				S3DSceneComposer::ParameterProperty * pProp,
				S3DSceneComposer::ParameterType type,
				size_t iParam, const SSystem::SString& strValue,
				bool flagAddUndo = true, const wchar_t * pwszUndoText = NULL ) ;
		// UNDO の追加
		virtual void AddEditUndo
			( S3DSceneComposer::Composition * pComp,
				S3DSceneComposer::ParameterProperty * pProp,
				const wchar_t * pwszUndoText = NULL ) = 0 ;
		// 現在のフレーム取得
		virtual int GetCurrentTimelineFrame
			( S3DSceneComposer::Composition * pComp ) const = 0 ;
		// カーソル位置取得
		virtual S3DDVector GetCursorPosition
			( S3DSceneComposer::Composition * pComp ) const = 0 ;
		// リソースに編集フラグ設定
		virtual void SetResourceModifiedFlag
				( S3DSceneComposer::ResourceContainer * prc ) = 0 ;
		// リソース要素の変更通知
		virtual void NotifyEditResourceElements
				( S3DSceneComposer::ResourceContainer * prc ) = 0 ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// エラー出力
	//////////////////////////////////////////////////////////////////////////

	class	S3DSceneDebugTracer	: public SSystem::SParserErrorInterface
	{
	protected:
		S3DCompositionEditorInterface *	m_pEditor ;
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DSceneDebugTracer, SParserErrorInterface )
		// 構築関数
		S3DSceneDebugTracer( S3DCompositionEditorInterface * pEditor = nullptr ) ;
		// 関連付け
		void AttachCompositionEditor( S3DCompositionEditorInterface * pEditor ) ;
		// エラー出力
		virtual void OutputError
			( const SSystem::SStringParser& ss, const wchar_t * pszError ) ;
		// 警告出力
		virtual void OutputWarning
			( const SSystem::SStringParser& ss, const wchar_t * pszWarning ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// コンポーザー・マネージャー
	//////////////////////////////////////////////////////////////////////////

	class	S3DCompositionManager	: public SSystem::SObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DCompositionManager, SObject )
		// 構築関数
		S3DCompositionManager( void ) ;
		// 消滅関数
		virtual ~S3DCompositionManager( void ) ;

	protected:
		S3DCompositionManager *									m_pParent ;
		Loquaty::LVirtualMachine *								m_pLoquatyVM ;
		Rosetta::RSVirtualMachine								m_vmRosetta ;
		SSystem::SSmartPointer<Rosetta::RSContext>				m_pContext ;
		SSystem::SStrSortObjectArray<S3DSceneComposer>			m_ssoaComposer ;
		SSystem::SObjectArray<S3DSceneComposerPluginInstance>	m_aPlugins ;
		S3DSceneComposerDynamicPlugin *							m_pDynamicPlugin ;
		SakuraCL::SCLRandomizer									m_random ;
		bool													m_flagScene ;
		S3DSceneComposerPluginSceneInfo							m_scpsiScene ;

	public:
		// VM の初期化
		void InitializeVM
			( Rosetta::RSVirtualMachine * pRefVM = nullptr,
				ECSSakura2::StandardVM * pSakura2VM = nullptr,
				bool flagDebugMode = false,
				Loquaty::LVirtualMachine * pRefLoquatyVM = nullptr ) ;
		void InitializeRef
			( S3DCompositionManager * pParent,
				bool flagDebugMode = false ) ;
		// 解放
		void Release( void ) ;
		// VM 取得
		Rosetta::RSVirtualMachine& VM( void )
		{
			return	m_vmRosetta ;
		}
		const Rosetta::RSVirtualMachine& GetVM( void ) const
		{
			return	m_vmRosetta ;
		}
		Loquaty::LVirtualMachine* LoquatyVM( void )
		{
			return	m_pLoquatyVM ;
		}
		const Loquaty::LVirtualMachine* GetLoquatyVM( void ) const
		{
			return	m_pLoquatyVM ;
		}
		// VM のみ解放
		void ReleaseVM( void ) ;
		// コンテキスト取得
		Rosetta::RSContext & Context( void )
		{
			return	*(m_pContext.Ptr()) ;
		}
		// Rosetta スクリプトを Sakura2 コードへコンパイル
		SSystem::SError CompileRosettaToSakura2
			( bool flagFlatPointer,
				bool flagCompileToNative,
				SSystem::SParserErrorLogger& perr ) ;

	public:
		// スクリプト・中間表現
		class	ScriptExpression
		{
		public:
			AntirrhinumGL::AGLScriptContext		m_context ;
			Rosetta::RSExpression				m_exprRosetta ;
			Loquaty::LInstantEvaluator			m_evalLoquaty ;
		public:
			// 数式を Rosetta VM で評価
			Rosetta::RSSmartPtr EvalExpressionAsRosetta
						( S3DSceneDebugTracer * pTracer = nullptr ) ;
			// 数式を Loquaty VM で評価
			Loquaty::LValue EvalExpressionAsLoquaty
						( S3DSceneDebugTracer * pTracer = nullptr ) ;
		} ;

		// 数式を Rosetta として事前処理
		void MakeExpressionAsRosetta
			( ScriptExpression& expr,
				const wchar_t * pwszExpr, S3DSceneDebugTracer * pTracer = nullptr ) ;
		// 数式を Rosetta VM で評価
		Rosetta::RSSmartPtr EvalExpressionAsRosetta
			( const wchar_t * pwszExpr, S3DSceneDebugTracer * pTracer = nullptr ) ;
		// 数式を Loquaty として事前処理
		bool MakeExpressionAsLoquaty
			( ScriptExpression& expr,
				const wchar_t * pwszExpr, S3DSceneDebugTracer * pTracer = nullptr ) ;
		// 数式を Loquaty VM で評価
		Loquaty::LValue EvalExpressionAsLoquaty
			( const wchar_t * pwszExpr, S3DSceneDebugTracer * pTracer = nullptr ) ;
		// 数式を Loquaty VM で評価（参照）
		Loquaty::LValue EvalRefExpressionAsLoquaty( const wchar_t * pwszExpr ) ;

	public:
		// 共有擬似乱数生成器
		SakuraCL::SCLRandomizer& Randomizer( void )
		{
			return	m_random ;
		}

	public:
		// プラグイン追加
		void AddPlugin( S3DSceneComposerPluginInstance * pPlugin ) ;
		// プラグイン数取得
		size_t GetPluginCount( void ) const ;
		// プラグイン取得
		S3DSceneComposerPluginInstance * GetPluginAt( size_t iPlugin ) const ;

	public:
		// プラグイン OnStartScene 呼び出し
		void OnStartScene( const S3DSceneComposerPluginSceneInfo& scpsi ) ;
		// プラグイン OnEndScene 呼び出し
		void OnEndScene( void ) ;
		// プラグイン OnStartComposition 呼び出し
		void OnStartComposition( S3DSceneComposer::Composition * pComposition ) ;
		// シーン情報取得
		bool GetSceneInfo( S3DSceneComposerPluginSceneInfo& scpsi ) const ;

	public:
		// コンポーザー取得（参照カウンタ不変）
		S3DSceneComposer * GetComposer( const wchar_t * pwszFileName ) const ;
		// コンポーザー登録（参照カウンタ不変）
		SGLError AddComposer
			( const wchar_t * pwszFileName, S3DSceneComposer * pComposer ) ;
		// コンポーザー読み込み
		S3DSceneComposer * LoadComposer
			( const wchar_t * pwszFileName,
					SSystem::SFileInterface * pFile = NULL ) ;
		// コンポーザー解放（参照カウンタ減少）
		SGLError UnloadComposer( S3DSceneComposer * pComposer ) ;

	protected:
		// ファイル名判定用正規化（ディレクトリを含まない小文字ファイル名）
		static SSystem::SString NormalizeComposerFileName( const wchar_t * pwszFileName ) ;

	protected:
		SSystem::SStrSortObjectArray
			<S3DSceneComposer::ItemCreator>	m_ssaItemCreators ;
		SSystem::SStrSortObjectArray
			<S3DSceneComposer::ItemCreator>	m_ssaCtrlCreators ;
		SSystem::SStrSortObjectArray
			<S3DSceneComposer::ItemCreator>	m_ssaRsrcProcCreators ;

		// ESLItemCreator 以外の ItemCreator を削除
		static void ReleaseTempItemCreator
			( SSystem::SStrSortObjectArray
				<S3DSceneComposer::ItemCreator>& ssaCreators ) ;

	public:
		// アイテム・タイプ追加
		virtual void AddItemCreator
				( const wchar_t * pwszType,
						S3DSceneComposer::ItemCreator * pCreator ) ;
		// コントローラー・タイプ追加
		virtual void AddControllerCreator
				( const wchar_t * pwszType,
						S3DSceneComposer::ItemCreator * pCreator ) ;
		// リソースプロシージャ・タイプ追加
		virtual void AddResourceProcCreator
				( const wchar_t * pwszType,
						S3DSceneComposer::ItemCreator * pCreator ) ;
		// プラグイン・メニュ項目を追加
		virtual void AddDynamicPluginDescriptor
			( S3DSceneComposerDynamicPlugin::Descriptor* pDesc ) ;

	public:
		// アイテムディスクリプタ追加
		virtual void AddItemDescriptor
				( const S3DSceneComposer::ItemClassDescriptor* pDesc ) ;
		// コントローラーディスクリプタ追加
		virtual void AddControllerDescriptor
				( const S3DSceneComposer::ItemClassDescriptor* pDesc ) ;
		// リソースプロシージャディスクリプタ追加
		virtual void AddResourceProcDescriptor
				( const S3DSceneComposer::ItemClassDescriptor* pDesc ) ;

	public:
		// S3DSceneComposer 生成
		virtual S3DSceneComposer * NewSceneComposer( void ) ;
		// アイテム生成
		virtual S3DSceneComposer::ItemSerializer *
							CreateSceneItem( const wchar_t * pwszType ) ;
		// コントローラー生成
		virtual S3DSceneComposer::Controller *
							CreateController( const wchar_t * pwszClass ) ;
		// リソースプロシージャ生成
		virtual S3DSceneComposer::ResourceProcedure *
							CreateResourceProc( const wchar_t * pwszClass ) ;

	} ;


}

#endif

