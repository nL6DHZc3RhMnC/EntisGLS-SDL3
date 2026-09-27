
#if	!defined(__GLS4_LOQUATY_SCENE_H__)
#define	__GLS4_LOQUATY_SCENE_H__

#include <loquaty_date_time.h>
#include <loquaty_file.h>
#include <loquaty_parser.h>
#include <loquaty_source_file.h>
#include <loquaty_xml_document.h>
#include <loquaty_array_buffer.h>
#include <loquaty_arrangement.h>


namespace	Loquaty
{
	//////////////////////////////////////////////////////////////////////////
	// クラス情報を ParameterProperty へ受け渡す実装
	//////////////////////////////////////////////////////////////////////////

	class	S3DSceneCustomProperty	: public ESLObject
	{
	protected:
		LPtr<LTaskObj>	m_pTask ;
		LObject *		m_pObj ;
		bool			m_ownObj ;
		LString			m_strObjClassPath ;
		LClass *		m_pImageClass ;
		LClass *		m_pAudioClass ;
		LClass *		m_pModelClass ;
		LClass *		m_pPoseClass ;
		LClass *		m_pSceneItemClass ;

		SSystem::SArray<SakuraGL::S3DSceneComposer::ParamEntry>	m_params ;

		enum	RawType
		{
			typeNull	= -1,
			typeMatrix3d,
			typeMatrix3f,
			typeQuaterniond,
			typeQuaternionf,
			typeVector3d,
			typeVector3f,
			typeMatrix4d,
			typeMatrix4f,
			typeVector4d,
			typeVector4f,
			typeVector2d,
			typeVector2f,
			typeARGB8,
			typeBoolean,
			typeInteger,	// any int type
			typeNumber,		// any float type
			typeString,
			typeImage,
			typeAudioPlayer,
			typeModelBuffer,
			typeModelPose,
			typeSceneItem,

			typeDataCount	= typeBoolean,
			typeFirstObject	= typeString,
		} ;
		static const size_t	s_bytesRawType[typeDataCount] ;

		class	EnumEntry
		{
		public:
			LString	m_strName ;
			LString	m_strValue ;
			LLong	m_numLong ;
		} ;

		class	Property
		{
		public:
			LType::LComment *		m_pComment ;
			LXMLDocPtr				m_pxmlSelector ;
			std::vector<EnumEntry>	m_vSelEntries ;
			LType					m_type ;
			RawType					m_typeRaw ;
			ssize_t					m_iElement ;
			LArrangement::Desc		m_descBuf ;
			LString					m_strTempValue ;
			SakuraGL::S3DSceneComposer::ParameterType
									m_paramType ;
			uint32_t				m_attrFlags ;
			double					m_minRange ;
			double					m_maxRange ;
			LString					m_strPropID ;
			LString					m_strName ;
			LString					m_strDescription ;
		public:
			Property( void )
				: m_pComment( nullptr ),
					m_typeRaw( typeNull ), m_iElement( -1 ) { }
		} ;
		SSystem::SObjectArray<Property>	m_props ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DSceneCustomProperty, ESLObject )
		// 構築関数
		S3DSceneCustomProperty( void ) ;
		S3DSceneCustomProperty( LVirtualMachine& vm, const LObjPtr& pObj ) ;
		// 消滅関数
		virtual ~S3DSceneCustomProperty( void ) ;
		// デフォルト構築関数の場合、後でオブジェクトを設定する
		void AttachObject( LVirtualMachine& vm, const LObjPtr& pObj ) ;
		// オブジェクトの参照を保持する
		void OwnObject( void ) ;

	protected:
		// クラス情報のコメントからパラメータ情報を構築する
		void BuildPropertyList( void ) ;
		Property * CreateProperty( const LType& type, const wchar_t * pwszVarName ) ;

		// 変数ポインタ取得
		void * GetPropPointer( Property& prop ) const ;

	public:
		// パラメータ値取得
		SakuraGL::S3DDMatrix GetMatrixParameter( ::size_t iParam ) const ;
		SakuraGL::S3DDVector GetVectorParameter( ::size_t iParam ) const ;
		double GetScalarParameter( ::size_t iParam ) const ;
		LLong GetIntegerParameter( ::size_t iParam ) const ;
		bool GetBooleanParameter( ::size_t iParam ) const ;
		const wchar_t * GetCommandParameter( ::size_t iParam ) const ;
		::size_t GetBinaryParameter
			( void * pDst, ::size_t nBufBytes, ::size_t iParam ) const ;
		// パラメータ値設定
		void SetMatrixParameter( ::size_t iParam, const SakuraGL::S3DDMatrix& mat ) ;
		void SetVectorParameter( ::size_t iParam, const SakuraGL::S3DDVector& vec ) ;
		void SetScalarParameter( ::size_t iParam, double s ) ;
		void SetIntegerParameter( ::size_t iParam, LLong n ) ;
		void SetBooleanParameter( ::size_t iParam, bool b ) ;
		void SetCommandParameter( ::size_t iParam, const wchar_t * pwszCmd ) ;
		::size_t SetBinaryParameter
			( ::size_t iParam, const void * pSrc, ::size_t nBufBytes ) ;
		// パラメータ値域列挙
		bool EnumerateStringSet
			( ::size_t iParam, SSystem::SStringArray& aStrSet ) ;

	protected:
		// 参照リソース更新
		void UpdateReferenceParameter( ::size_t iParam ) ;
		void UpdateReferenceAllParameters( void ) ;
		// コンポジション取得
		virtual SakuraGL::S3DSceneComposer::Composition * GetComposition( void ) const = 0 ;
		// コンポーザー取得
		virtual SakuraGL::S3DSceneComposer * GetComposer( void ) const ;
		// 例外エラーをデバッグ用に出力
		void ExceptionDebugTrace( LObjPtr pException ) ;
	} ;



	//////////////////////////////////////////////////////////////////////////
	// SceneCustomItem オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	S3DSceneCustomItem
				: public SakuraGL::S3DSceneComposer::ItemBasicSerializer,
					public S3DSceneCustomProperty
	{
	protected:
		SSystem::SString							m_strClassID ;
		SakuraGL::S3DSceneComposer::ParamSetClass	m_pscClass ;

		LVirtualMachine&	m_vm ;
		LClass *			m_pDeviceClass ;
		LClass *			m_pRendererClass ;
		LClass *			m_pCollisionClass ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( S3DSceneCustomItem, ItemBasicSerializer, S3DSceneCustomProperty )
		// 構築関数
		S3DSceneCustomItem
			( LVirtualMachine& vm, LObjPtr pObj, const wchar_t * pwszClassID ) ;

	protected:	// S3DSceneCustomProperty
		// コンポジション取得
		virtual SakuraGL::S3DSceneComposer::Composition * GetComposition( void ) const ;

	public:	// S3DSceneComposer::Parameter
		// パラメータ値取得
		virtual SakuraGL::S3DDMatrix GetMatrixParameter( ::size_t iParam ) const ;
		virtual SakuraGL::S3DDVector GetVectorParameter( ::size_t iParam ) const ;
		virtual double GetScalarParameter( ::size_t iParam ) const ;
		virtual int32_t GetIntegerParameter( ::size_t iParam ) const ;
		virtual bool GetBooleanParameter( ::size_t iParam ) const ;
		virtual const wchar_t * GetCommandParameter( ::size_t iParam ) const ;
		virtual ::size_t GetBinaryParameter
			( void * pDst, ::size_t nBufBytes, ::size_t iParam ) const ;
		// パラメータ値設定
		virtual void SetMatrixParameter( ::size_t iParam, const SakuraGL::S3DDMatrix& mat ) ;
		virtual void SetVectorParameter( ::size_t iParam, const SakuraGL::S3DDVector& vec ) ;
		virtual void SetScalarParameter( ::size_t iParam, double s ) ;
		virtual void SetIntegerParameter( ::size_t iParam, int32_t n ) ;
		virtual void SetBooleanParameter( ::size_t iParam, bool b ) ;
		virtual void SetCommandParameter( ::size_t iParam, const wchar_t * pwszCmd ) ;
		virtual ::size_t SetBinaryParameter
			( ::size_t iParam, const void * pSrc, ::size_t nBufBytes ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( ::size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// S3DSceneComposer::ItemSerializer
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( ::size_t iCategory ) const ;
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( SakuraGL::S3DSceneComposer::Composition& comp, uint32_t nFlags ) ;
		// 拡張的な処理の通知
		virtual void OnExtendNotify
			( const wchar_t * pwszCmd, const wchar_t * pwszParam,
				const void * pExParam, ::size_t nExParamBytes ) ;
		// タイマー処理
		virtual void OnTimer( SakuraGL::S3DScene& scene, uint32_t msecPast ) ;
		// フレーム更新後処理
		virtual void OnUpdateFrame
			( double fpFrame, SakuraGL::S3DSceneComposer::SeekMethod seek ) ;

		// レンダリングの為のデバイスリソース準備
		virtual void OnPrepareToRender
			( SakuraGL::S3DRenderDevice * pDevice, uint32_t nFlags = 0 ) ;
		// レンダリング前後処理（全視点共通）
		virtual void OnItemRenderEvent
			( SakuraGL::S3DScene& scene, SakuraGL::S3DScene::ItemClass clsItem ) ;
		// 当たり判定追加
		//（必要であれば scene.PhysicsScene() へ Actor の追加も行う）
		virtual void OnItemRenderCollision
			( const SakuraGL::S3DScene& scene, SakuraGL::S3DCollision& render ) ;
		// 表示モデル追加
		virtual void OnItemRenderModel
			( const SakuraGL::S3DScene& scene,
				SakuraGL::S3DRenderContextInterface& render,
				uint64_t flagsExclusion = 0 ) ;
		// 描画前処理
		virtual void BeforeItemRenderModel
			( const SakuraGL::S3DScene& scene,
				SakuraGL::S3DRenderContextInterface& render,
				uint64_t flagsExclusion = 0 ) ;
		// 描画後処理
		virtual void AfterItemRenderModel
			( const SakuraGL::S3DScene& scene,
				SakuraGL::S3DRenderContextInterface& render,
				uint64_t flagsExclusion = 0 ) ;

	protected:
		void CallRenderModel
			( const wchar_t * pwszFuncName,
				const SakuraGL::S3DScene& scene,
				SakuraGL::S3DRenderContextInterface& render,
				uint64_t flagsExclusion ) ;

	public:
		// Loquaty クラス名
		virtual const wchar_t * GetLQClassName( void ) const ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// SceneCustomController コントローラー
	//////////////////////////////////////////////////////////////////////////

	class	SceneCustomController
				: public SakuraGL::S3DSceneComposer::Controller,
					public S3DSceneCustomProperty
	{
	protected:
		SSystem::SString	m_strClassID ;

		LVirtualMachine&	m_vm ;
		LClass *			m_pDeviceClass ;
		LClass *			m_pRendererClass ;
		LClass *			m_pCollisionClass ;
		LClass *			m_pSceneItemClass ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SceneCustomController, Controller, S3DSceneCustomProperty )
		// 構築関数
		SceneCustomController
			( LVirtualMachine& vm, LObjPtr pObj, const wchar_t * pwszClassID ) ;

		// 動作フラグ変更
		void SetControllerBehaviorFlags( uint32_t nFlags )
		{
			m_flagsBehavior = nFlags ;
		}
		void SetBehaviorRenderEventClasses( uint32_t nClasses )
		{
			m_flagsEventClasses = nClasses ;
		}

	protected:	// S3DSceneCustomProperty
		// コンポジション取得
		virtual SakuraGL::S3DSceneComposer::Composition * GetComposition( void ) const ;

	public:	// S3DSceneComposer::Parameter
		// パラメータ値取得
		virtual SakuraGL::S3DDMatrix GetMatrixParameter( ::size_t iParam ) const ;
		virtual SakuraGL::S3DDVector GetVectorParameter( ::size_t iParam ) const ;
		virtual double GetScalarParameter( ::size_t iParam ) const ;
		virtual int32_t GetIntegerParameter( ::size_t iParam ) const ;
		virtual bool GetBooleanParameter( ::size_t iParam ) const ;
		virtual const wchar_t * GetCommandParameter( ::size_t iParam ) const ;
		virtual ::size_t GetBinaryParameter
			( void * pDst, ::size_t nBufBytes, ::size_t iParam ) const ;
		// パラメータ値設定
		virtual void SetMatrixParameter( ::size_t iParam, const SakuraGL::S3DDMatrix& mat ) ;
		virtual void SetVectorParameter( ::size_t iParam, const SakuraGL::S3DDVector& vec ) ;
		virtual void SetScalarParameter( ::size_t iParam, double s ) ;
		virtual void SetIntegerParameter( ::size_t iParam, int32_t n ) ;
		virtual void SetBooleanParameter( ::size_t iParam, bool b ) ;
		virtual void SetCommandParameter( ::size_t iParam, const wchar_t * pwszCmd ) ;
		virtual ::size_t SetBinaryParameter
			( ::size_t iParam, const void * pSrc, ::size_t nBufBytes ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( ::size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// S3DSceneComposer::ItemSerializer
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( ::size_t iCategory ) const ;
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( SakuraGL::S3DSceneComposer::Composition& comp,
				SakuraGL::S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags ) ;
		// 拡張的な処理の通知
		virtual void OnExtendNotify
			( const wchar_t * pwszCmd, const wchar_t * pwszParam,
				const void * pExParam, ::size_t nExParamBytes ) ;
		// タイマー処理
		virtual void OnTimer
			( SakuraGL::S3DScene& scene,
				SakuraGL::S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast ) ;
		// フレーム更新後処理
		virtual void OnUpdateFrame
			( SakuraGL::S3DSceneComposer::ItemSerializer * pItem,
				double fpFrame, SakuraGL::S3DSceneComposer::SeekMethod seek ) ;

		// レンダリングの為のデバイスリソース準備
		virtual void OnPrepareToRender
			( SakuraGL::S3DRenderDevice * pDevice, uint32_t nFlags = 0 ) ;
		// レンダリングイベント
		virtual void OnRenderEvent
			( SakuraGL::S3DScene& scene,
				SakuraGL::S3DScene::ItemClass clsItem,
				SakuraGL::S3DSceneComposer::ItemSerializer * pItem ) ;
		// 当たり判定追加
		//（必要であれば scene.PhysicsScene() へ Actor の追加も行う）
		virtual void RenderCollision
			( const SakuraGL::S3DScene& scene,
				SakuraGL::S3DSceneComposer::ItemSerializer * pItem,
				SakuraGL::S3DCollision& render ) ;
		// 表示モデル追加
		virtual void RenderModel
			( const SakuraGL::S3DScene& scene,
				SakuraGL::S3DScene::ItemClass clsItem,
				SakuraGL::S3DSceneComposer::ItemSerializer * pItem,
				SakuraGL::S3DRenderContextInterface& render,
				uint64_t flagsExclusion = 0 ) ;
		// 描画前処理
		virtual void BeforeRenderModel
			( const SakuraGL::S3DScene& scene,
				SakuraGL::S3DScene::ItemClass clsItem,
				SakuraGL::S3DSceneComposer::ItemSerializer * pItem,
				SakuraGL::S3DRenderContextInterface& render,
				uint64_t flagsExclusion = 0 ) ;
		// 描画後処理
		virtual void AfterRenderModel
			( const SakuraGL::S3DScene& scene,
				SakuraGL::S3DScene::ItemClass clsItem,
				SakuraGL::S3DSceneComposer::ItemSerializer * pItem,
				SakuraGL::S3DRenderContextInterface& render,
				uint64_t flagsExclusion = 0 ) ;

	protected:
		void CallRenderModel
			( const wchar_t * pwszFuncName,
				const SakuraGL::S3DScene& scene,
				SakuraGL::S3DScene::ItemClass clsItem,
				SakuraGL::S3DSceneComposer::ItemSerializer * pItem,
				SakuraGL::S3DRenderContextInterface& render,
				uint64_t flagsExclusion ) ;

	public:
		// Loquaty クラス名
		virtual const wchar_t * GetLQClassName( void ) const ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ScenePoseController コントローラー
	//////////////////////////////////////////////////////////////////////////

	class	ScenePoseController
				: public SceneCustomController,
					public SakuraGL::S3DDynamicModelSerializer::PoseInterface
	{
	protected:
		LClass *	m_pModelClass ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( ScenePoseController, SceneCustomController, PoseInterface )
		// 構築関数
		ScenePoseController
			( LVirtualMachine& vm, LObjPtr pObj, const wchar_t * pwszClassID ) ;

	public:
		// ポーズ適用
		virtual void OnPoseTrack
			( SakuraGL::S3DDynamicModelSerializer& item, SakuraGL::S3DModelBuffer& model ) ;
	protected:
		// モデル変更時の処理
		virtual void OnChangedModel
			( SakuraGL::S3DDynamicModelSerializer& item, SakuraGL::S3DModelBuffer * pModel ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// SceneMeshController コントローラー
	//////////////////////////////////////////////////////////////////////////

	class	SceneMeshController
			: public SceneCustomController,
				public SakuraGL::S3DMeshBufferItemSerializer::MeshInterface
	{
	protected:
		LClass *	m_pVertexBufferClass ;
		LClass *	m_pVBArrayClass ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SceneMeshController, SceneCustomController, MeshInterface )
		// 構築関数
		SceneMeshController
			( LVirtualMachine& vm, LObjPtr pObj, const wchar_t * pwszClassID ) ;

	public:
		// メッシュ追加処理（全視点・ビュー共通処理）
		virtual void AddMesh
			( SakuraGL::S3DScene& scene,
				SakuraGL::S3DSceneComposer::ItemSerializer * pItem,
				SakuraGL::S3DVertexBufferInterface ** ppVBs, size_t nVBCount ) ;
		// フレーム描画前処理（全視点・ビュー共通処理）
		virtual void UpdateMesh
			( SakuraGL::S3DScene& scene,
				SakuraGL::S3DSceneComposer::ItemSerializer * pItem,
				SakuraGL::S3DVertexBufferInterface ** ppVBs, size_t nVBCount ) ;

	protected:
		void CallMakeMesh
			( const wchar_t * pwszFuncName,
				SakuraGL::S3DScene& scene,
				SakuraGL::S3DSceneComposer::ItemSerializer * pItem,
				SakuraGL::S3DVertexBufferInterface ** ppVBs, size_t nVBCount ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// S3DBulletItemInterface::Bullet コンテナ
	//////////////////////////////////////////////////////////////////////////

	class	SceneBulleteItemBulletInstance	: public SSystem::SObject
	{
	public:
		SakuraGL::S3DBulletItemInterface::Bullet *	m_pBullet ;

	public:
		ESL_DECLARE_CLASS_INFO( SceneBulleteItemBulletInstance, SObject ) ;
		SceneBulleteItemBulletInstance
				( SakuraGL::S3DBulletItemInterface::Bullet * pBullet )
			: m_pBullet( pBullet ) { }
	} ;


}

#endif

