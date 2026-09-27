
#if	!defined(__ROSETTA_SPRITE_H__)
#define	__ROSETTA_SPRITE_H__

#include <rosetta/rosetta_reference.h>
#include <sakuraglx/sprite/sglx_sprite_rectangle.h>
#include <sakuraglx/sprite/sglx_sprite_message.h>
#include <sakuraglx/sprite/sglx_sprite_movie.h>

namespace	Rosetta
{
	//////////////////////////////////////////////////////////////////////////
	// SkinManager クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSSkinManagerClass	: public RGenericNativeObjectClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSSkinManagerClass, RGenericNativeObjectClass )
		// 構築関数
		RSSkinManagerClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"SkinManager" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::SGLSkinManager *
			GetThisSkinManager( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean loadSkinFile( String file, boolean fStaticRsrc = false ) ;
		static RSObject * method_loadSkinFile
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean readSkinFile( InputStream is, boolean fStaticRsrc = false ) ;
		// boolean readSkinFile( RandomAccessFile raf, boolean fStaticRsrc = false ) ;
		static RSObject * method_readSkinFile
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void cleanupResource()
		static RSObject * method_cleanupResource
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void removeSkinResource()
		static RSObject * method_removeSkinResource
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Image getImageAs( String id )
		static RSObject * method_getImageAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// TextSprite.TextStyle getTextStyleAs( String id )
		static RSObject * method_getTextStyleAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Sprite createFormedPage( String id )
		static RSObject * method_createFormedPage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// BasicFormParser クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSBasicFormParserClass	: public RGenericNativeObjectClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSBasicFormParserClass, RGenericNativeObjectClass )
		// 構築関数
		RSBasicFormParserClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"BasicFormParser" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::SGLBasicFormParser *
			GetThisFormParser( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean openPackage( String file ) ;
		static RSObject * method_openPackage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void closePackage( void ) ;
		static RSObject * method_closePackage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean loadForm( String file )
		static RSObject * method_loadForm
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Image getImageAs( String id )
		static RSObject * method_getImageAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int buildImageAtlas( int nMaxAtlasSize = 2048, int nReqBatchCount = 3 )
		static RSObject * method_buildImageAtlas
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Sprite createFormedSprite
		//	( String id, boolean flagBuffered = false, long nBufFlags = 0 )
		static RSObject * method_createFormedSprite
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// SpriteTimer オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	RSSpriteTimer	: public RSGenericObject,
								public SakuraGL::SGLSpriteTimer
	{
	public:
		RSVirtualMachine *	m_vm ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( RSSpriteTimer, RSGenericObject, SGLSpriteTimer )
		// 構築関数
		RSSpriteTimer
			( RSVirtualMachine * vm,
				RSClass * pClass, BasicType type = typeGenericObject )
				: RSGenericObject(pClass, type), m_vm(vm) {}
		// 消滅関数
		virtual ~RSSpriteTimer( void ) ;

	public:	// オブジェクト
		// 複製（参照の複製を含む）
		virtual RSObject * DuplicateObject( RSContext& context ) const ;
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;

	public:
		// タイマー処理（true で終了）
		virtual bool OnTimer( SakuraGL::SGLSprite& sprite, uint32_t msecPast ) ;
	} ;

	class	RSSpriteTimerClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSSpriteTimerClass, RSClass )
		// 構築関数
		RSSpriteTimerClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"SpriteTimer" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// マウス入力インターフェースオブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	RSSpriteMouseListener	: public RSGenericObject,
										public SakuraGL::SGLSpriteMouseStateListener
	{
	public:
		RSVirtualMachine *	m_vm ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( RSSpriteMouseListener, RSGenericObject, SGLSpriteMouseListener )
		// 構築関数
		RSSpriteMouseListener
			( RSVirtualMachine * vm,
				RSClass * pClass, BasicType type = typeGenericObject )
				: RSGenericObject(pClass, type), m_vm(vm) {}
		// 消滅関数
		virtual ~RSSpriteMouseListener( void ) ;

	public:	// オブジェクト
		// 複製（参照の複製を含む）
		virtual RSObject * DuplicateObject( RSContext& context ) const ;
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;

	protected:
		// スクリプト呼び出し
		bool CallMouseListener
			( const wchar_t * pwszFunc,
				SakuraGL::SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;

	public:
		// マウス移動
		virtual bool OnMouseMove
			( SakuraGL::SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual void OnMouseLeave( SakuraGL::SGLSprite& sprite, int64_t nFlags ) ;
		// ホイール回転
		virtual bool OnMouseWheel
			( SakuraGL::SGLSprite& sprite, int32_t zDelta,
				double xPos, double yPos, int64_t nFlags ) ;
		// 左ボタン
		virtual bool OnLButtonDown
			( SakuraGL::SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnLButtonUp
			( SakuraGL::SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnLButtonDblClk
			( SakuraGL::SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		// 右ボタン
		virtual bool OnRButtonDown
			( SakuraGL::SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnRButtonUp
			( SakuraGL::SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnRButtonDblClk
			( SakuraGL::SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		// 中央ボタン
		virtual bool OnMButtonDown
			( SakuraGL::SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnMButtonUp
			( SakuraGL::SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnMButtonDblClk
			( SakuraGL::SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		// 左ボタン（後処理）
		virtual bool AfterLButtonDown
			( SakuraGL::SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool AfterLButtonUp
			( SakuraGL::SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool AfterLButtonDblClk
			( SakuraGL::SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		// 右ボタン（後処理）
		virtual bool AfterRButtonDown
			( SakuraGL::SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool AfterRButtonUp
			( SakuraGL::SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool AfterRButtonDblClk
			( SakuraGL::SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		// 中央ボタン（後処理）
		virtual bool AfterMButtonDown
			( SakuraGL::SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool AfterMButtonUp
			( SakuraGL::SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool AfterMButtonDblClk
			( SakuraGL::SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
	} ;

	class	RSSpriteMouseListenerClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSSpriteMouseListenerClass, RSClass )
		// 構築関数
		RSSpriteMouseListenerClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"SpriteMouseListener" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// this オブジェクトを取得
		static SakuraGL::SGLSpriteMouseStateListener *
			GetThisMouseListener( RSContext& context, RSObject* pThis ) ;

	public:
		// int getPointerCount()
		static RSObject * method_getPointerCount
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int findMouseIndexById( int idMouse )
		static RSObject * method_findMouseIndexById
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Vector2D getMousePointAt( int i )
		static RSObject * method_getMousePointAt
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isLButtonDownAt( int i )
		static RSObject * method_isLButtonDownAt
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isRButtonDownAt( int i )
		static RSObject * method_isRButtonDownAt
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Vector2D enumerateLDownPoints()
		static RSObject * method_enumerateLDownPoints
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean onMouseEvent
		//	( Sprite sprite, double xPos, double yPos, long nFlags )
		static RSObject * method_onMouseEvent
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void onMouseLeave( Sprite sprite, long nFlags )
		static RSObject * method_onMouseLeave
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean onMouseWheel
		//	( Sprite sprite, int zDelta, double xPos, double yPos, long nFlags )
		static RSObject * method_onMouseWheel
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getMouseID( long nFlags )
		static RSObject * method_getMouseID
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// キー入力インターフェースオブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	RSSpriteKeyListener	: public RSGenericObject,
										public SakuraGL::SGLSpriteKeyListener
	{
	public:
		RSVirtualMachine *	m_vm ;
		SSystem::SString	m_strFontFaceTemp ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( RSSpriteKeyListener, RSGenericObject, SGLSpriteKeyListener )
		// 構築関数
		RSSpriteKeyListener
			( RSVirtualMachine * vm,
				RSClass * pClass, BasicType type = typeGenericObject )
				: RSGenericObject(pClass, type), m_vm(vm) {}
		// 消滅関数
		virtual ~RSSpriteKeyListener( void ) ;

	public:	// オブジェクト
		// 複製（参照の複製を含む）
		virtual RSObject * DuplicateObject( RSContext& context ) const ;
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;

	public:
		// キー入力
		virtual bool OnKeyDown
			( SakuraGL::SGLSprite& sprite, int64_t nVirtKey, int64_t nFlags ) ;
		virtual bool OnKeyUp
			( SakuraGL::SGLSprite& sprite, int64_t nVirtKey, int64_t nFlags ) ;
		// 文字入力
		virtual bool OnChar( SakuraGL::SGLSprite& sprite, uint16_t codeChar ) ;
		// コンポジション開始
		virtual bool OnStartComposition
			( SakuraGL::SGLSprite& sprite,
					SakuraGL::SGLInputStartComposition& iscForm ) ;
		// コンポジション終了
		virtual bool OnEndComposition( SakuraGL::SGLSprite& sprite ) ;
		// コンポジション文字列
		virtual bool OnCompositionString
			( SakuraGL::SGLSprite& sprite,
				const SakuraGL::SGLInputCompositionString& icsComp ) ;
		// コマンド
		virtual bool OnCommand
			( SakuraGL::SGLSprite& sprite, const wchar_t * pszCmd,
				int64_t nParam, int64_t nCode, int nPriority, bool fOverwritable ) ;
	} ;

	class	RSSpriteKeyListenerClass	: public RSClass
	{
	public:
		// SpriteKeyListener.StartComposition
		class	StartCompositionClass	: public RSClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( StartCompositionClass, RSClass )
			// 構築関数
			StartCompositionClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"StartComposition" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
		} ;
		// SpriteKeyListener.CompositionString
		class	CompositionStringClass	: public RSClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( CompositionStringClass, RSClass )
			// 構築関数
			CompositionStringClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"CompositionString" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSSpriteKeyListenerClass, RSClass )
		// 構築関数
		RSSpriteKeyListenerClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"SpriteKeyListener" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	public:
		static RSObject * method_onDummyKeyEvent
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Sprite クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSSpriteClass	: public RGenericNativeObjectClass
	{
	public:
		// Sprite.Parameter クラス
		class	ParameterClass	: public RSClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( ParameterClass, RSClass )
			// 構築関数
			ParameterClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"Parameter" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
			// Object -> SGLSprite::Parameter 変換
			static void ParameterFromObject
				( RSContext& context,
					SakuraGL::SGLSprite::Parameter& param,
					SSystem::SArray<SakuraGL::S2DVector>& vertics, RSObject * obj ) ;
			// Object <- SGLSprite::Parameter 変換
			static void ParameterToObject
				( RSContext& context, RSObject * obj,
					const SakuraGL::SGLSprite::Parameter& param ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSSpriteClass, RGenericNativeObjectClass )
		// 構築関数
		RSSpriteClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Sprite" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトのスプライトを取得
		static SakuraGL::SGLSprite *
			GetThisSprite( RSContext& context, RSObject* pThis ) ;

	public:
		// SGLSprite 参照オブジェクト生成
		RSNativeObject * CreateRefObject( SakuraGL::SGLSprite * pSprite ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const long getUIFlag()
		static RSObject * method_getUIFlag
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// long modifyUIFlag( long nAddFlags, long nRemoveFlags = 0 )
		static RSObject * method_modifyUIFlag
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Rect getClickableRect()
		static RSObject * method_getClickableRect
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setClickableRect( Rect rect )
		static RSObject * method_setClickableRect
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Sprite.Parameter getParameter()
		static RSObject * method_getParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Vector2D getPosition()
		static RSObject * method_getPosition
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Vector2D getCenterPosition()
		static RSObject * method_getCenterPosition
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Vector2D getZoom()
		static RSObject * method_getZoom
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// double getRotation()
		static RSObject * method_getRotation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getTransparency()
		static RSObject * method_getTransparency
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getFilterParameter()
		static RSObject * method_getFilterParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getFilter2Parameter()
		static RSObject * method_getFilter2Parameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isVisible()
		static RSObject * method_isVisible
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getPriority()
		static RSObject * method_getPriority
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String getID()
		static RSObject * method_getID
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Rect getRectangle()
		static RSObject * method_getRectangle
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setParameter( Sprite.Parameter param )
		static RSObject * method_setParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setPosition( double x, double y )
		static RSObject * method_setPosition
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setCenterPosition( double x, double y )
		static RSObject * method_setCenterPosition
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setZoom( double x, double y )
		static RSObject * method_setZoom
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setRotation( double zAngle )
		static RSObject * method_setRotation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setTransparency( int nTransparency )
		static RSObject * method_setTransparency
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setFilterParameter( int paramFilter )
		static RSObject * method_setFilterParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setFilter2Parameter( int paramFilter )
		static RSObject * method_setFilter2Parameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean regulateCenter
		//	( int nFlags = 0, double xOffset = 0.0, double yOffset = 0.0 )
		static RSObject * method_regulateCenter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setVisible( boolean visible )
		static RSObject * method_setVisible
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void changePriority( int nPriority )
		static RSObject * method_changePriority
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setID( String id )
		static RSObject * method_setID
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void postUpdate()
		static RSObject * method_postUpdate
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void freezeFrameUpdate()
		static RSObject * method_freezeFrameUpdate
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void defrostFrameUpdate()
		static RSObject * method_defrostFrameUpdate
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void addChild( Sprite sprite )
		static RSObject * method_addChild
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void detachChild( Sprite sprite )
		static RSObject * method_detachChild
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void detachAllChildren()
		static RSObject * method_detachAllChildren
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Sprite getParent()
		static RSObject * method_getParent
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Sprite getItemAs( String id )
		static RSObject * method_getItemAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Sprite getHitSpriteAt( double x, double y )
		static RSObject * method_getHitSpriteAt
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isHitSprite( double x, double y )
		static RSObject * method_isHitSprite
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getChildCount()
		static RSObject * method_getChildCount
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Sprite getChildAt( int i )
		static RSObject * method_getChildAt
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean loadImage( String file )
		static RSObject * method_loadImage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void attachAnimation( Image image )
		static RSObject * method_attachAnimation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void beginAnimation
		//	( int loop = -1, int iLoopStart = 0, int iLoopEnd = 0,
		//		int iAnimeStart = 0, int msecDuration = 0 )
		static RSObject * method_beginAnimation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setLoopAnimation
		//	( int loop = -1, int iLoopStart = 0, int iLoopEnd = 0 )
		static RSObject * method_setLoopAnimation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Size getImageSize()
		static RSObject * method_getImageSize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean createBuffer
		//	( int width, int height, int format = Image.formatDefaultRGBA,
		//		int depth = 32, int nFlags = Image.bufferOnMemory,
		//		boolean flagZBuffer = false, boolean flagStereo3D = false )
		static RSObject * method_createBuffer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void releaseBuffer()
		static RSObject * method_releaseBuffer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isBuffered()
		static RSObject * method_isBuffered
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setFillBackColor( int argbFill, boolean flagFillBack = true )
		static RSObject * method_setFillBackColor
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String getText()
		static RSObject * method_getText
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setText( String text )
		static RSObject * method_setText
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setTextFont( String font, int nSize = 0 )
		static RSObject * method_setTextFont
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isEnabled()
		static RSObject * method_isEnabled
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setEnable( boolean fEnable )
		static RSObject * method_setEnable
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getScrollPos( int scrlDir = Sprite.scrollDefault )
		static RSObject * method_getScrollPos
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setScrollPos( int nPos, int scrlDir = Sprite.scrollDefault )
		static RSObject * method_setScrollPos
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getScrollRange( int scrlDir = Sprite.scrollDefault )
		static RSObject * method_getScrollRange
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setScrollRange( int nRange, int scrlDir = Sprite.scrollDefault )
		static RSObject * method_setScrollRange
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getScrollPageSize( int scrlDir = Sprite.scrollDefault )
		static RSObject * method_getScrollPageSize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setScrollPageSize( int nPageSize, int scrlDir = Sprite.scrollDefault )
		static RSObject * method_setScrollPageSize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isButtonChecked()
		static RSObject * method_isButtonChecked
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void checkButton( boolean check )
		static RSObject * method_checkButton
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void invokeCommands( String xmlCommands )
		static RSObject * method_invokeCommands
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isSpriteVisible( String id )
		static RSObject * method_isSpriteVisible
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setSpriteVisible( String id, boolean visible )
		static RSObject * method_setSpriteVisible
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getSpritePriority( String id )
		static RSObject * method_getSpritePriority
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void changeSpritePriority( String id, int priority )
		static RSObject * method_changeSpritePriority
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getSpriteTransparency( String id )
		static RSObject * method_getSpriteTransparency
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setSpriteTransparency( String id, int transparency )
		static RSObject * method_setSpriteTransparency
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Rect getSpriteRectangle( String id )
		static RSObject * method_getSpriteRectangle
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String getSpriteText( String id )
		static RSObject * method_getSpriteText
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setSpriteText( String id, String text )
		static RSObject * method_setSpriteText
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setSpriteTextFont( String id, String font, int nSize = 0 )
		static RSObject * method_setSpriteTextFont
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setSpriteImage( String id, String image )
		static RSObject * method_setSpriteImage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isSpriteEnabled( String id )
		static RSObject * method_isSpriteEnabled
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setSpriteEnable( String id, boolean fEnable )
		static RSObject * method_setSpriteEnable
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getSpriteScrollPos( String id, int scrlDir = Sprite.scrollDefault )
		static RSObject * method_getSpriteScrollPos
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setSpriteScrollPos( String id, int nPos, int scrlDir = Sprite.scrollDefault )
		static RSObject * method_setSpriteScrollPos
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getSpriteScrollRange( String id, int scrlDir = Sprite.scrollDefault )
		static RSObject * method_getSpriteScrollRange
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setSpriteScrollRange( String id, int nRange, int scrlDir = Sprite.scrollDefault )
		static RSObject * method_setSpriteScrollRange
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getSpriteScrollPageSize( String id, int scrlDir = Sprite.scrollDefault )
		static RSObject * method_getSpriteScrollPageSize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setSpriteScrollPageSize( String id, int nPageSize, int scrlDir = Sprite.scrollDefault )
		static RSObject * method_setSpriteScrollPageSize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isSpriteButtonChecked( String id )
		static RSObject * method_isSpriteButtonChecked
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void checkSpriteButton( String id, boolean check )
		static RSObject * method_checkSpriteButton
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// SpriteAction setActionLinearTo
		//		( int msecDuration, int nTransparency,
		//			Vector2D vPos = null, Vector2D vZoom = null,
		//			double a0 = 0.0, double a1 = 0.0 )
		static RSObject * method_setActionLinearTo
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void addAction( SpriteAction act )
		static RSObject * method_addAction
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void flushAction()
		static RSObject * method_flushAction
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isAction()
		static RSObject * method_isAction
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void pauseAllAction()
		static RSObject * method_pauseAllAction
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void restartAllAction()
		static RSObject * method_restartAllAction
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

		// void addTimer( SpriteTimer timer )
		static RSObject * method_addTimer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void removeTimer( SpriteTimer timer )
		static RSObject * method_removeTimer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void attachMouseListener( SpriteMouseListener listener )
		static RSObject * method_attachMouseListener
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void detachMouseListener( SpriteMouseListener listener )
		static RSObject * method_detachMouseListener
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void attachKeyListener( SpriteKeyListener listener )
		static RSObject * method_attachKeyListener
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void detachKeyListener( SpriteKeyListener listener )
		static RSObject * method_detachKeyListener
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

		// void setMouseCapture()
		static RSObject * method_setMouseCapture
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void releaseMouseCapture()
		static RSObject * method_releaseMouseCapture
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setKeyFocus()
		static RSObject * method_setKeyFocus
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void killKeyFocus()
		static RSObject * method_killKeyFocus
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean hasKeyFocus()
		static RSObject * method_hasKeyFocus
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// SpriteAction クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSSpriteActionClass	: public RGenericNativeObjectClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSSpriteActionClass, RGenericNativeObjectClass )
		// 構築関数
		RSSpriteActionClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"SpriteAction" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::SGLSpriteAction *
			GetThisSpriteAction( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setActionType( int type ) ;
		static RSObject * method_setActionType
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setDuration( int msecDuration, int msecDelay = 0 ) ;
		static RSObject * method_setDuration
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setMoveTo
		//	( Sprite sprite, double x, double y, double a0 = 0.0, double a1 = 0.0 ) ;
		static RSObject * method_setMoveTo
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setZoomTo
		//	( Sprite sprite, double x, double y, double a0 = 0.0, double a1 = 0.0 ) ;
		static RSObject * method_setZoomTo
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setRotationTo
		//	( Sprite sprite, double z, double a0 = 0.0, double a1 = 0.0 ) ;
		static RSObject * method_setRotationTo
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setTransparencyTo( Sprite sprite, int nTransparency ) ;
		static RSObject * method_setTransparencyTo
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setFilterTo( Sprite sprite, int paramFilter ) ;
		static RSObject * method_setFilterTo
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setFilter2To( Sprite sprite, int paramFilter ) ;
		static RSObject * method_setFilter2To
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setBezierCurve( Vector2D bzCurve, boolean fOffset = false ) ;
		static RSObject * method_setBezierCurve
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setCenterCurve( Vector2D bzCurve, boolean fOffset = false ) ;
		static RSObject * method_setCenterCurve
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setZoomCurve( Vector2D bzCurve, boolean fOffset = false ) ;
		static RSObject * method_setZoomCurve
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setAngleCurve( Float32Pointer bzCurve, boolean fOffset = false ) ;
		static RSObject * method_setAngleCurve
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setTransparencyCurve( Float32Pointer bzCurve, boolean fOffset = false ) ;
		static RSObject * method_setTransparencyCurve
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setFilterParamCurve( Float32Pointer bzCurve, boolean fOffset = false ) ;
		static RSObject * method_setFilterParamCurve
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setFilter2ParamCurve( Float32Pointer bzCurve, boolean fOffset = false ) ;
		static RSObject * method_setFilter2ParamCurve
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// WindowSprite クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSWindowSpriteClass	: public RSClass
	{
	public:
		// WindowSprite.UpdateParameter クラス
		class	UpdateParameterClass	: public RSStructuredPointerClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( UpdateParameterClass, RSStructuredPointerClass )
			// 構築関数
			UpdateParameterClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"UpdateParameter" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
		} ;
		// WindowSprite.Stereo3D クラス
		class	Stereo3DClass	: public RSClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( Stereo3DClass, RSClass )
			// 構築関数
			Stereo3DClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"Stereo3D" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSWindowSpriteClass, RSClass )
		// 構築関数
		RSWindowSpriteClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"WindowSprite" ) ;
		// メンバ初期設定
		virtual void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::SGLWindowSprite *
			GetThisWindowSprite( RSContext& context, RSObject* pThis ) ;

	public:
		// SGLWindowSprite 参照オブジェクト生成
		RSNativeObject * CreateRefObject( SakuraGL::SGLWindowSprite * pWindow ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean createDisplay
		//	( String name, int mode,
		//		int width, int height, int bpp = 0, int frequency = 0 )
		static RSObject * method_createDisplay
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean closeDisplay()
		static RSObject * method_closeDisplay
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const long getOptionalFlags()
		static RSObject * method_getOptionalFlags
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setOptionalFlags( long nFlags )
		static RSObject * method_setOptionalFlags
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean changeCooperationLevel( int mode )
		static RSObject * method_changeCooperationLevel
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean changeDisplaySize
		//	( int width, int height, int bpp = 0, int frequency = 0 )
		static RSObject * method_changeDisplaySize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean getDisplaySize( Size sizeDisplay )
		static RSObject * method_getDisplaySize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean enableChangePhysicalMode( boolean flagEnable )
		static RSObject * method_enableChangePhysicalMode
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean enableZBuffer( boolean flagZBuffer )
		static RSObject * method_enableZBuffer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setStereoDisplayMode( String sMethodID, long nParam = 0 )
		static RSObject * method_setStereoDisplayMode
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isSupportedStereoDisplayMode( String sMethodID )
		static RSObject * method_isSupportedStereoDisplayMode
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean initWindowPosition
		//		( int xPos, int yPos, Size pInitExSize = null )
		static RSObject * method_initWindowPosition
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean getNormalWindowPosition
		//		( Point ptWindow, Size sizeWindow = null )
		static RSObject * method_getNormalWindowPosition
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean getInternalDisplayPosition
		//		( Rect rectRender, Rect rectDisplay )
		static RSObject * method_getInternalDisplayPosition
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setExteriorBackgroundFrame
		//		( int nFlags, int rgbColor, Image pTile,
		//			Image pLeft = null, Image pRight = null,
		//			Image pUpper = null, Image pUnder = null )
		static RSObject * method_setExteriorBackgroundFrame
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean createWindow
		//		( String name, int width, int height,
		//				int flags, WindowSprite parent = null )
		static RSObject * method_createWindow
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean closeWindow()
		static RSObject * method_closeWindow
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setWindowLayout( int nFlags, int xPos = 0, int yPos = 0 )
		static RSObject * method_setWindowLayout
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean changeWindowSize( int width, int height )
		static RSObject * method_changeWindowSize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean getWindowClientRect( Rect rectClient )
		static RSObject * method_getWindowClientRect
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Vector2D screenPositionFromClient( Vector2D vClient )
		static RSObject * method_screenPositionFromClient
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Vector2D clientPositionFromScreen( Vector2D vScreen )
		static RSObject * method_clientPositionFromScreen
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void postUpdate( Rect pUpdate = null )
		static RSObject * method_postUpdate
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean updateWindow( WindowSprite.UpdateParameter pUpdate = null )
		static RSObject * method_updateWindow
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean processUserInput( long msecTimeout = 1 )
		static RSObject * method_processUserInput
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean postRenderingThread( Runnable proc )
		static RSObject * method_postRenderingThread
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean postUIThread( Runnable proc )
		static RSObject * method_postUIThread
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isWindowActive()
		static RSObject * method_isWindowActive
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setWindowCaption( String name )
		static RSObject * method_setWindowCaption
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean showCursor( boolean fShow )
		static RSObject * method_showCursor
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isShowCursor()
		static RSObject * method_isShowCursor
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setCursor( String sCursorID )
		static RSObject * method_setCursor
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean moveCursorPosition( int xPos, int yPos, int idMouse = 0 )
		static RSObject * method_moveCursorPosition
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean getCursorPosition( Point ptCursor, int idMouse = 0 )
		static RSObject * method_getCursorPosition
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getMonitorFrequency()
		static RSObject * method_getMonitorFrequency
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void lock()
		static RSObject * method_lock
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void unlock()
		static RSObject * method_unlock
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// long testLocked()
		static RSObject * method_testLocked
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// VirtualInput クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSVirtualInputClass	: public RGenericNativeObjectClass
	{
	public:
		// VirtualInput.InputEvent クラス
		class	InputEventClass	: public RSClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( InputEventClass, RSClass )
			// 構築関数
			InputEventClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"InputEvent" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
			// Object -> SGLVirtualInput::InputEvent 変換
			static void InputEventFromObject
				( RSContext& context, SakuraGL::SGLVirtualInput::InputEvent& ev, RSObject * obj ) ;
			// Object <- SGLVirtualInput::InputEvent 変換
			static void InputEventToObject
				( RSContext& context, RSObject * obj, const SakuraGL::SGLVirtualInput::InputEvent& ev ) ;
		} ;
		// VirtualInput.Command クラス
		class	CommandClass	: public RSClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( CommandClass, RSClass )
			// 構築関数
			CommandClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"Command" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
			// Object <- SGLVirtualInput::Command 変換
			static void CommandToObject
				( RSContext& context, RSObject * obj, const SakuraGL::SGLVirtualInput::Command& cmd ) ;
		} ;

	public:
		static const SSystem::SXMLDocument::AttrInteger	m_aiKeyCode[] ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSVirtualInputClass, RGenericNativeObjectClass )
		// 構築関数
		RSVirtualInputClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"VirtualInput" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::SGLVirtualInput *
			GetThisVirtualInput( RSContext& context, RSObject* pThis ) ;

	public:
		// SGLVirtualInput 参照オブジェクト生成
		RSNativeObject * CreateRefObject( SakuraGL::SGLVirtualInput * pInput ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void attachPostListenerToWindow( WindowSprite window )
		static RSObject * method_attachPostListenerToWindow
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void detachPostListenerToWindow( WindowSprite window )
		static RSObject * method_detachPostListenerToWindow
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean pollJoyStick( void ) ;
		static RSObject * method_pollJoyStick
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean getAnalogJoyPosition( Vector4D vPos, int joyStick = 0 ) ;
		static RSObject * method_getAnalogJoyPosition
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isJoyButtonPushing( int joyButton, int joyStick = 0 ) ;
		static RSObject * method_isJoyButtonPushing
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getJoyButtonPushed( int joyButton, int joyStick = 0 ) ;
		static RSObject * method_getJoyButtonPushed
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void resetJoyButtonPushed( int joyButton, int joyStick = 0 ) ;
		static RSObject * method_resetJoyButtonPushed
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void resetAllJoyButtonPushed( void ) ;
		static RSObject * method_resetAllJoyButtonPushed
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void pressInputEvent( InputEvent evIn ) ;
		static RSObject * method_pressInputEvent
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void releaseInputEvent( InputEvent evIn ) ;
		static RSObject * method_releaseInputEvent
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean getInputEvent( InputEvent ev ) ;
		static RSObject * method_getInputEvent
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setInputQueueLimit( int nLimit ) ;
		static RSObject * method_setInputQueueLimit
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void flushInputQueue( void ) ;
		static RSObject * method_flushInputQueue
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void addCommand
		//	( String strCmd, int nParam = 0, int nCode = 0,
		//		int nPriority = Sprite.commandNormal,
		//		boolean fOverwritable = false ) ;
		static RSObject * method_addCommand
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean getCommand( Command cmd ) ;
		static RSObject * method_getCommand
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void flushCommandQueue( void ) ;
		static RSObject * method_flushCommandQueue
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void addFilter( InputEvent evIn, InputEvent evOut ) ;
		static RSObject * method_addFilter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void removeFilter( InputEvent evIn ) ;
		static RSObject * method_removeFilter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void removeAllFilter( void ) ;
		static RSObject * method_removeAllFilter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// InputEvent getFilterAs( InputEvent evIn ) ;
		static RSObject * method_getFilterAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void addInputMap( InputEvent evIn, InputEvent evOut ) ;
		static RSObject * method_addInputMap
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void removeInputMap( InputEvent evIn ) ;
		static RSObject * method_removeInputMap
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void removeAllInputMap( void ) ;
		static RSObject * method_removeAllInputMap
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// InputEvent getInputMapAs( InputEvent evIn ) ;
		static RSObject * method_getInputMapAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean loadPrefilter( String sFilterFile ) ;
		static RSObject * method_loadPrefilter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean readPrefilter( InputStream isFilter ) ;
		static RSObject * method_readPrefilter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean loadPostfilter( String sFilterFile ) ;
		static RSObject * method_loadPostfilter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean readPostfilter( InputStream isFilter ) ;
		static RSObject * method_readPostfilter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// FontStyle クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSFontStyleClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSFontStyleClass, RSClass )
		// 構築関数
		RSFontStyleClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"FontStyle" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	public:
		// SGLFontStyle から変換
		static void ConvertToObject
			( RSContext& context, RSObject * pObj,
				const SakuraGL::SGLFontStyle& style ) ;
		// SGLFontStyle へ変換
		static void ConvertFromObject
			( RSContext& context,
				SakuraGL::SGLFontStyle& style,
				SSystem::SString& strFace, RSObject * pObj ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// LetteringContext クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSLetteringContextClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSLetteringContextClass, RSClass )
		// 構築関数
		RSLetteringContextClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"LetteringContext" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	public:
		// SGLLetteringContext から変換
		static void ConvertToObject
			( RSContext& context, RSObject * pObj,
				const SakuraGL::SGLLetteringContext& ltctx ) ;
		// SGLLetteringContext へ変換
		static void ConvertFromObject
			( RSContext& context,
				SakuraGL::SGLLetteringContext& ltctx,
				SSystem::SString& strProhibition, RSObject * pObj ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// LetteringDecoration クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSLetteringDecorationClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSLetteringDecorationClass, RSClass )
		// 構築関数
		RSLetteringDecorationClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"LetteringDecoration" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	public:
		// SGLLetterer::Decoration から変換
		static void ConvertToObject
			( RSContext& context, RSObject * pObj,
				const SakuraGL::SGLLetterer::Decoration& ltdec ) ;
		// SGLLetterer::Decoration へ変換
		static void ConvertFromObject
			( RSContext& context,
				SakuraGL::SGLLetterer::Decoration& ltdec, RSObject * pObj ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// RectangleSprite クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSRectangleSpriteClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSRectangleSpriteClass, RSClass )
		// 構築関数
		RSRectangleSpriteClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"RectangleSprite" ) ;
		// メンバ初期設定
		virtual void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::SGLSpriteRectangle *
			GetThisSpriteRectangle( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// RectangleSprite.RectStyle getRectStyle() ;
		static RSObject * method_getRectStyle
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setRectStyle( RectangleSprite.RectStyle style ) ;
		static RSObject * method_setRectStyle
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setRectangleSize( int w, int h )
		static RSObject * method_setRectangleSize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setRectangleColor( int color )
		static RSObject * method_setRectangleColor
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	public:
		class	RectStyleClass	: public RSStructuredPointerClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( RectStyleClass, RSStructuredPointerClass )
			// 構築関数
			RectStyleClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"RectStyle" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
		} ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// TextSprite クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSTextSpriteClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSTextSpriteClass, RSClass )
		// 構築関数
		RSTextSpriteClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"TextSprite" ) ;
		// メンバ初期設定
		virtual void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::SGLSpriteText *
			GetThisSpriteText( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// TextSprite.TextStyle getTextStyle() ;
		static RSObject * method_getTextStyle
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setTextStyle( TextSprite.TextStyle style ) ;
		static RSObject * method_setTextStyle
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	public:
		class	TextStyleClass	: public RSClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( TextStyleClass, RSClass )
			// 構築関数
			TextStyleClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"TextStyle" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
		public:
			// SGLSpriteText::TextStyle から変換
			static void ConvertToObject
				( RSContext& context, RSObject * pObj,
					const SakuraGL::SGLSpriteText::TextStyle& style ) ;
			// SGLSpriteText::TextStyle へ変換
			static void ConvertFromObject
				( RSContext& context,
					SakuraGL::SGLSpriteText::TextStyle& style,
					SSystem::SString& strFace,
					SSystem::SString& strProhibition, RSObject * pObj ) ;
		} ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ButtonSprite クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSButtonSpriteClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSButtonSpriteClass, RSClass )
		// 構築関数
		RSButtonSpriteClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"ButtonSprite" ) ;
		// メンバ初期設定
		virtual void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::SGLSpriteButton *
			GetThisSpriteButton( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void attachButtonListener( ButtonSprite.Listener listener )
		static RSObject * method_attachButtonListener
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void attachSoundEffect( AudioPlayer seFocus, AudioPlayer sePushed )
		static RSObject * method_attachSoundEffect
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean createSimpleButton
		//	( Image[] pImages, boolean flagHitRect, Image pHitMask,
		//		Size sizeButton, TextSprite.TextStyle textStyle,
		//		int[] pTextColors, int[] pBackColors,
		//		int maskStatus = ButtonSprite.flagNormal4Buttons,
		//		int typeButton = ButtonSprite.typeNormal )
		static RSObject * method_createSimpleButton
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean createSimpleImageButton
		//	( Image[] pImages, boolean flagHitRect = false,
		//		Image pHitMask = null,
		//		int maskStatus = ButtonSprite.flagNormal4Buttons,
		//		int typeButton = ButtonSprite.typeNormal )
		static RSObject * method_createSimpleImageButton
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean createSimpleTextButton
		//	( TextSprite.TextStyle textStyle,
		//		Size sizeTextExt,
		//		int[] pTextColors, int[] pBackColors,
		//		int maskStatus = ButtonSprite.flagNormal4Buttons,
		//		int typeButton = ButtonSprite.typeNormal )
		static RSObject * method_createSimpleTextButton
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean createSimpleRectButton
		//	( Size sizeRect, int[] pTextColors,
		//		int maskStatus = ButtonSprite.flagNormal4Buttons,
		//		int typeButton = ButtonSprite.typeNormal )
		static RSObject * method_createSimpleRectButton
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const ButtonSprite.ButtonStyle getButtonStyle()
		static RSObject * method_getButtonStyle
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setButtonStyle( ButtonSprite.ButtonStyle style )
		static RSObject * method_setButtonStyle
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setButtonSize( int w, int h )
		static RSObject * method_setButtonSize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const int getButtonStatus()
		static RSObject * method_getButtonStatus
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setButtonStatus( int status )
		static RSObject * method_setButtonStatus
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Image newButtonImageReference( int status )
		static RSObject * method_newButtonImageReference
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setButtonRepeat
		//	( boolean flagRepeat, int msecBefore, int msecInterval )
		static RSObject * method_setButtonRepeat
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setRightClickNotify( boolean flagRightClick, long nRightClickParam )
		static RSObject * method_setRightClickNotify
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setStatusNotification( boolean flagNotify, long nNotifyParam )
		static RSObject * method_setStatusNotification
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void enableDrag( boolean flagDraggable, int nThreshold = 8 )
		static RSObject * method_enableDrag
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	public:
		class	ButtonStyleClass	: public RSClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( ButtonStyleClass, RSClass )
			// 構築関数
			ButtonStyleClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"ButtonStyle" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
		public:
			// SGLSpriteText::TextStyle から変換
			static void ConvertToObject
				( RSContext& context, RSObject * pObj,
					const SakuraGL::SGLSpriteButton::ButtonStyle& style ) ;
			// SGLSpriteText::TextStyle へ変換
			static void ConvertFromObject
				( RSContext& context,
					SakuraGL::SGLSpriteButton::ButtonStyle& style,
					SSystem::SString * pstrFontFaces,
					SSystem::SString * pstrProhibitions, RSObject * pObj ) ;
		} ;

		class	Listener	: public RSGenericObject,
									public SakuraGL::SGLSpriteButtonListener
		{
		public:
			RSVirtualMachine *	m_vm ;

		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO2
				( Listener, RSGenericObject, SGLSpriteButtonListener )
			// 構築関数
			Listener
				( RSVirtualMachine * vm,
					RSClass * pClass, BasicType type = typeGenericObject )
					: RSGenericObject(pClass, type), m_vm(vm) {}
			// 消滅関数
			virtual ~Listener( void ) ;

		public:	// オブジェクト
			// 複製（参照の複製を含む）
			virtual RSObject * DuplicateObject( RSContext& context ) const ;
			// 複製（実体も可能な限り複製）
			virtual RSObject * CloneObject( RSContext& context ) const ;

		public:
			// ボタンが押された
			virtual bool OnButtonPushed( SakuraGL::SGLSpriteButton& button, bool fRepeat ) ;
			// ボタンのステータスが変化した
			virtual bool OnChangedButtonStatus( SakuraGL::SGLSpriteButton& button ) ;
			// ドラッグが開始した
			virtual void OnBeginDrag
				( SakuraGL::SGLSpriteButton& button, double xOffset, double yOffset ) ;
		} ;

		class	ListenerClass	: public RSClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( ListenerClass, RSClass )
			// 構築関数
			ListenerClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"Listener" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
		public:
			// boolean onButtonPushed
			//	( ButtonSprite button, boolean fRepeat )
			static RSObject * method_onButtonPushed
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
			// boolean onChangedButtonStatus( ButtonSprite button )
			static RSObject * method_onChangedButtonStatus
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
			// void onBeginDrag
			//	( ButtonSprite button, double xOffset, double yOffset )
			static RSObject * method_onBeginDrag
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
		} ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// MessageSprite クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSMessageSpriteClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSMessageSpriteClass, RSClass )
		// 構築関数
		RSMessageSpriteClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"MessageSprite" ) ;
		// メンバ初期設定
		virtual void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::SGLSpriteMessage *
			GetThisSpriteMessage( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// TextSprite.TextStyle getTextStyle() ;
		static RSObject * method_getTextStyle
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setTextStyle( TextSprite.TextStyle style ) ;
		static RSObject * method_setTextStyle
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// FontStyle getRubyFontStyle() ;
		static RSObject * method_getRubyFontStyle
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setRubyFontStyle( FontStyle style ) ;
		static RSObject * method_setRubyFontStyle
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// MessageSprite.ViewActionStyle getViewActionStyle() ;
		static RSObject * method_getViewActionStyle
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setViewActionStyle( MessageSprite.ViewActionStyle style ) ;
		static RSObject * method_setViewActionStyle
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// SkinManager getAttachedSkin()
		static RSObject * method_getAttachedSkin
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void attachSkin( SkinManager skin )
		static RSObject * method_attachSkin
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void clearMessage()
		static RSObject * method_clearMessage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void flushMessage()
		static RSObject * method_flushMessage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isMessagePending()
		static RSObject * method_isMessagePending
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setMessageSpeedRatio( int fxSpeedRatio )
		static RSObject * method_setMessageSpeedRatio
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void addMessageText( String strText )
		static RSObject * method_addMessageText
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void addMessageXML( String strXML )
		static RSObject * method_addMessageXML
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Point getNextMessagePoint()
		static RSObject * method_getNextMessagePoint
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean getCircumscribedRect( Rect rectMsg )
		static RSObject * method_getCircumscribedRect
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getMessageCharacterCount()
		static RSObject * method_getMessageCharacterCount
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	public:
		class	ViewActionStyleClass	: public RSClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( ViewActionStyleClass, RSClass )
			// 構築関数
			ViewActionStyleClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"ViewActionStyle" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
		public:
			// SGLSpriteMessage::ViewActionStyle から変換
			static void ConvertToObject
				( RSContext& context, RSObject * pObj,
					const SakuraGL::SGLSpriteMessage::ViewActionStyle& style ) ;
			// SGLSpriteMessage::ViewActionStyle へ変換
			static void ConvertFromObject
				( RSContext& context,
					SakuraGL::SGLSpriteMessage::ViewActionStyle& style,
					RSObject * pObj ) ;
		} ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// MovieSprite クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSMovieSpriteClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSMovieSpriteClass, RSClass )
		// 構築関数
		RSMovieSpriteClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"MovieSprite" ) ;
		// メンバ初期設定
		virtual void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::SGLSpriteMovie *
			GetThisSpriteMovie( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean openMovieFile( String path )
		static RSObject * method_openMovieFile
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean closeMovieFile()
		static RSObject * method_closeMovieFile
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean playMovie( long nFlags = 0 )
		static RSObject * method_playMovie
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean stopMovie()
		static RSObject * method_stopMovie
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setMovieLoop
		//	( boolean flagLoop = true, long nStart = -1, long nEnd = -1 )
		static RSObject * method_setMovieLoop
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean pauseMovie()
		static RSObject * method_pauseMovie
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean restartMovie()
		static RSObject * method_restartMovie
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean getVolume( float[] pVolumes, int nChannels )
		static RSObject * method_getVolume
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setVolume( float[] pVolumes, int nChannels )
		static RSObject * method_setVolume
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setVolumeLine( int iLine )
		static RSObject * method_setVolumeLine
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const boolean isMoviePlaying()
		static RSObject * method_isMoviePlaying
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const boolean isMoviePaused()
		static RSObject * method_isMoviePaused
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const int getMovieFrequency()
		static RSObject * method_getMovieFrequency
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const long getMovieLength()
		static RSObject * method_getMovieLength
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const long getMoviePosition()
		static RSObject * method_getMoviePosition
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void seekMovie( long nPos )
		static RSObject * method_seekMovie
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Size getMovieSize()
		static RSObject * method_getMovieSize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const boolean hasEndedOfDuration()
		static RSObject * method_hasEndedOfDuration
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const void resetEndOfDuration()
		static RSObject * method_resetEndOfDuration
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// RenderableSprite クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSRenderableSpriteClass	: public RSClass
	{
	public:
		enum	ErrorMode
		{
			errorIgnore,
			errorStdOut,
		} ;
		class	Sprite	: public SakuraGL::SGLSprite,
								public RSNativeObject::ObjectListener
		{
		public:
			SSystem::SSmartPointer<RSContext>	m_context ;
			SSystem::SSmartReference<RSObject>	m_refObject ;
			ErrorMode							m_errMode ;
			RSSmartPtr							m_pTempRect ;
			SakuraGL::SGLImageRect *			m_pRectBuf ;

		public:
			ESL_DECLARE_CLASS_INFO2( Sprite, SGLSprite, ObjectListener )
			Sprite( RSVirtualMachine * vm, RSSmartPtr pObject ) ;
			virtual ~Sprite( void ) ;
			// オブジェクト削除時
			virtual void OnRelease( RSNativeObject * pNObj ) ;
			virtual void OnDetach( RSNativeObject * pNObj ) ;
			// 時間経過処理
			virtual void AdvanceTime( uint32_t msecPast ) ;
			// 外接矩形取得
			virtual bool GetRectangle( SakuraGL::SGLRect& rectExt ) const ;
			// ヒット判定
			virtual bool IsHitSprite( double x, double y ) const ;
			// 子スプライトを描画
			virtual void DrawChildren
				( SakuraGL::S3DRenderContextInterface& render,
					SGLSprite::Stereo3DView s3dView = s3dMonoview ) const ;
		public:
			// 例外クリア
			void ClearException( void ) const ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSRenderableSpriteClass, RSClass )
		// 構築関数
		RSRenderableSpriteClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"RenderableSprite" ) ;
		// メンバ初期設定
		virtual void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static Sprite * GetThisSprite( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setErrorMode( int error )
		static RSObject * method_setErrorMode
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void onTimer( int msecPast )
		static RSObject * method_onTimer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean getRenderRect( Rect rect )
		static RSObject * method_getRenderRect
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isHitTest( double xLocal, double yLocal )
		static RSObject * method_isHitTest
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void onRender( RenderContext render, int s3dView )
		static RSObject * method_onRender
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;

}

#endif

