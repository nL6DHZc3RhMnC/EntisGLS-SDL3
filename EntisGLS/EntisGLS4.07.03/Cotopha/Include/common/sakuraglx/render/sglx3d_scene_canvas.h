
#if	!defined(__SAKURAGLX3D_SCENE_CANVAS_H__)
#define	__SAKURAGLX3D_SCENE_CANVAS_H__	1

#include <sakuraglx/render/sglx3d_scene_sprite.h>

namespace	SakuraGL
{
	////////////////////////////////////////////////////////////////////////////////////
	// キャンバス・アイテム
	////////////////////////////////////////////////////////////////////////////////////

	class	S3DCanvasSerializer
				: public S3DSceneItemSprite,
					public S3DSceneComposer::ItemCommonSerializer
	{
	public:
		class	ImageDrawer	: public ESLObject
		{
		public:
			ESL_DECLARE_CLASS_INFO( ImageDrawer, ESLObject )
			virtual void OnDrawImage( SGLDrawImageParamList& dipl ) = 0 ;
			virtual bool IsHitCursor( const S2DDVector& vPos ) = 0 ;
			virtual bool OnMouseMove( const S2DVector& vPos ) = 0 ;
			virtual void OnMouseLeave( void ) = 0 ;
			virtual bool OnMouseWheel( const S2DVector& vGlobal, float32_t zDelta ) = 0 ;
			virtual bool OnClickDown( const S2DVector& vPos, SGLBasicForm::MouseButton button ) = 0 ;
			virtual bool OnClickUp( const S2DVector& vPos, SGLBasicForm::MouseButton button ) = 0 ;
		} ;
		class	ImageEffector	: public ESLObject
		{
			ESL_DECLARE_CLASS_INFO( ImageEffector, ESLObject )
			virtual bool IsEnabledFilter( void ) const = 0 ;
			virtual SGLImageObject * OnImageFilter
				( S3DRenderContextInterface& render,
					SGLImageObject * pDstImage, SGLImageObject * pSrcImage ) = 0 ;
		} ;

	public:
		enum	ParameterIndex
		{
			paramCanvasWidth		= ItemCommonSerializer::paramItemTotalCount,
			paramCanvasHeight,
			paramCanvasFrameBuffer,
			paramCanvasBackAlpha,
			paramCanvasCenterOffset,
			paramCoordnatesMode,
			paramDepthMaskOp,
			paramCanvas3DScale,
			param2DViewAngle,
			paramCmdFit2DViewAngle,
			paramCmdFit2DViewPos,
			paramCanvasTotalCount,
			paramCanvasCount		= paramCanvasTotalCount - paramCanvasWidth,
		} ;

	protected:
		SGLSize		m_sizeCanvas ;
		bool		m_flagFrameBuffer ;
		bool		m_flagStereoBuffer ;
		int			m_nFrameBackAlpha ;
		S2DDVector	m_vCenterOffset ;
		double		m_fp3DViewScale ;
		double		m_deg2DViewAngle ;

		SSystem::SCriticalSection	m_csDrawList ;
		SGLImage					m_imgFilterBuf ;

		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramCanvasCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;

		static const SSystem::SXMLDocument::AttrInteger	m_aiCoordsModes[3] ;
		static const SSystem::SXMLDocument::AttrInteger	m_aiDepthMaskOps[6] ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2
			( S3DCanvasSerializer, S3DSceneItemSprite, ItemCommonSerializer )
		S3D_DECLARE_COMPOSER_ITEM( S3DCanvasSerializer, canvas )
		// 構築関数
		S3DCanvasSerializer( void ) ;
		// 消滅関数
		virtual ~S3DCanvasSerializer( void ) ;

	public:
		// キャンバスサイズ取得
		const SGLSize& GetCanvasSize( void ) const ;
		// ステレオバッファ設定
		void SetFrameStereoBuffer( bool flagStereo ) ;
		// フレームバッファ設定反映
		void UpdateFrameBuffer( void ) ;
		// 中心座標反映
		void UpdateCenterPosition( void ) ;

	public:	// Parameter
		// パラメータ値取得
		virtual double GetScalarParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		virtual size_t GetBinaryParameter
			( void * pDst, size_t nBufBytes, size_t i ) const ;
		// パラメータ値設定
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		virtual size_t SetBinaryParameter
			( size_t i, const void * pSrc, size_t nBufBytes ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// ParameterProperty
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// CommonSerializer
		// 行列設定
		virtual void SetItemMatrix( const S3DDMatrix& matrix ) ;
		// 変換行列更新
		virtual void UpdateSpaceMatrix( void ) ;

	public:	// S3DScene::Item
		// アイテム作用の追加処理
		virtual void OnUpdateBehavior( S3DScene& scene ) ;
		// タイマー処理
		virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;

	public:	// ItemSerializer
		// レンダリング設定
		virtual void OnSetupSceneSettings
			( S3DScene& scene,
				const SGLSize& sizeFrame,
				SGLSecondaryViewProducer * psvp = NULL ) ;
		// 規定のレンダリングデバイス設定
		virtual void OnSetRenderDevice( S3DRenderDevice * pDevice ) ;

	public:	// SGLSprite
		// カスタムフィルタ
		virtual SGLImageObject * CustomFilter
			( S3DRenderContextInterface& render,
				SGLImageObject * pFrameBuf, SGLImageObject * pSrcImage ) ;
		// ヒットアイテム検索
		virtual SGLSprite* GetHitSpriteAt( S2DDVector& vPos ) const ;
		// ヒット判定
		virtual bool IsHitSprite( double x, double y ) const ;
		bool IsHitImageDrawer( double x, double y ) const ;
		// マウス移動
		virtual bool OnMouseMove
			( double xPos, double yPos, int64_t nFlags ) ;
		virtual void OnMouseLeave( int64_t nFlags ) ;
		// ホイール回転
		virtual bool OnMouseWheel
			( int32_t zDelta, double xPos, double yPos, int64_t nFlags ) ;
		// マウスボタン
		virtual bool OnButtonDown
			( double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnButtonUp
			( double xPos, double yPos, int64_t nFlags ) ;

	public:
		// 視野角を合わせる
		void CmdFit2DViewAngle( void ) ;
		// 現在のカメラに 2D 表示位置と 3D 座標空間が一致するように変更する
		void CmdFit2DViewPosition( void ) ;

	public:
		// 画像描画リスト
		virtual void OnDrawImage( SGLDrawImageParamList& dipl ) ;
		// 画像描画追加
		void AddDrawImageParam
			( const SGLPaintParam& param,
				SGLImageObject * pImage,
				const SGLImageRect * pSrcRect = NULL ) ;
		void AddDrawImageParams
			( size_t nCount,
				const SGLPaintParam * pParams,
				SGLImageObject *const* ppImages,
				const SGLImageRect * pSrcRects = NULL ) ;

	} ;


	////////////////////////////////////////////////////////////////////////////////////
	// 画像表示コントローラー基底
	////////////////////////////////////////////////////////////////////////////////////

	class	S3DCanvasBasicController
				: public S3DSceneComposer::Controller,
					public S3DCanvasSerializer::ImageDrawer
	{
	public:
		enum	ParameterIndex
		{
			paramPosition,
			paramCenter,
			paramZoom,
			paramRotation,
			paramTransparency,
			paramVisible,
			paramBasicCount,
		} ;

	protected:
		S3DDVector	m_vPosition ;
		S2DDVector	m_vCenter ;
		S3DDVector	m_vZoom ;
		double		m_degRotation ;
		double		m_fpTransparency ;
		bool		m_flagVisible ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( S3DCanvasBasicController, Controller, ImageDrawer )
		// 構築関数
		S3DCanvasBasicController( const wchar_t * pwszClassID ) ;
		// 消滅関数
		virtual ~S3DCanvasBasicController( void ) ;

	public:	// Parameter
		// パラメータ値取得
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual size_t GetBinaryParameter
			( void * pDst, size_t nBufBytes, size_t i ) const ;
		// パラメータ値設定
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual size_t SetBinaryParameter
			( size_t i, const void * pSrc, size_t nBufBytes ) ;

	public:	// ImageDrawer
		// 画像描画
		virtual void OnDrawImage( SGLDrawImageParamList& dipl ) ;
		virtual void OnLocalDrawImage( SGLDrawImageParamList& dipl ) = 0 ;
		// カーソル当たり判定
		virtual bool IsHitCursor( const S2DDVector& vPos ) ;
		// マウス入力
		virtual bool OnMouseMove( const S2DVector& vPos ) ;
		virtual void OnMouseLeave( void ) ;
		virtual bool OnMouseWheel( const S2DVector& vGlobal, float32_t zDelta ) ;
		virtual bool OnClickDown( const S2DVector& vPos, SGLBasicForm::MouseButton button ) ;
		virtual bool OnClickUp( const S2DVector& vPos, SGLBasicForm::MouseButton button ) ;

	} ;


	////////////////////////////////////////////////////////////////////////////////////
	// 画像表示コントローラー
	////////////////////////////////////////////////////////////////////////////////////

	class	S3DCanvasImageController	: public S3DCanvasBasicController
	{
	public:
		enum	ParameterIndex
		{
			paramImageID	= S3DCanvasBasicController::paramBasicCount,
			paramSmoothing,
			paramColorEffectType,
			paramEffectColor,
			paramCmdRefCenter,
		} ;
		enum	ColorEffectType
		{
			effectColorNo,
			effectColorMul,
			effectColorAdd,
			effectColorCount,
		} ;
		static const SSystem::SXMLDocument::AttrInteger
							s_aiColorEffectType[effectColorCount+1] ;

	protected:
		SSystem::SString	m_strImageID ;
		SGLImageObject *	m_pImage ;
		bool				m_flagSmoothing ;
		ColorEffectType		m_colorEffect ;
		SGLPalette			m_rgbEffectColor ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DCanvasImageController, S3DCanvasBasicController )
		S3D_DECLARE_COMPOSER_ITEM( S3DCanvasImageController, canvas_image )
		// 構築関数
		S3DCanvasImageController( void ) ;
		// 消滅関数
		virtual ~S3DCanvasImageController( void ) ;

	public:
		// 画像参照
		void UpdateImageRef( void ) ;
		// 画像基準座標を設定する
		void CmdRefImageCenter( void ) ;

	public:	// Parameter
		// パラメータ値取得
		virtual S3DDVector GetVectorParameter( size_t iParam ) const ;
		virtual bool GetBooleanParameter( size_t iParam ) const ;
		virtual const wchar_t * GetCommandParameter( size_t iParam ) const ;
		// パラメータ値設定
		virtual void SetVectorParameter( size_t iParam, const S3DDVector& vec ) ;
		virtual void SetBooleanParameter( size_t iParam, bool b ) ;
		virtual void SetCommandParameter( size_t iParam, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// ParameterProperty
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// Controller
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags ) ;

	public:	// ImageDrawer
		// 画像描画
		virtual void OnLocalDrawImage( SGLDrawImageParamList& dipl ) ;

	} ;



	////////////////////////////////////////////////////////////////////////////////////
	// 簡易フォーム
	////////////////////////////////////////////////////////////////////////////////////

	class	S3DCanvasBasicFormController	: public S3DCanvasBasicController
	{
	public:
		enum	ParameterIndex
		{
			paramFormResource	= S3DCanvasBasicController::paramBasicCount,
			paramFormID,
			paramFormX,
			paramFormY,
			paramFormWidth,
			paramFormHeight,
			paramCmdRefPos,
		} ;

	protected:
		SSystem::SString						m_strRsrcID ;
		SSystem::SString						m_strFormID ;
		SGLBasicFormParser *					m_pFormParser ;
		SSystem::SSmartPointer<SGLBasicForm>	m_pForm ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DCanvasBasicFormController, S3DCanvasBasicController )
		S3D_DECLARE_COMPOSER_ITEM( S3DCanvasBasicFormController, canvas_form )
		// 構築関数
		S3DCanvasBasicFormController( void ) ;
		// 消滅関数
		virtual ~S3DCanvasBasicFormController( void ) ;

	public:
		// フォーム取得
		SGLBasicForm * GetForm( void ) const ;

	public:
		// 画像参照
		void UpdateForm( void ) ;
		// 画像基準座標を設定する
		void CmdRefFormPos( void ) ;

	public:	// Parameter
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

	public:	// ParameterProperty
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// Controller
		// タイマー処理
		virtual void OnTimer
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast ) ;
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags ) ;

	public:	// ImageDrawer
		// 画像描画
		virtual void OnLocalDrawImage( SGLDrawImageParamList& dipl ) ;
		// カーソル当たり判定
		virtual bool IsHitCursor( const S2DDVector& vPos ) ;
		// マウス入力
		virtual bool OnMouseMove( const S2DVector& vPos ) ;
		virtual void OnMouseLeave( void ) ;
		virtual bool OnMouseWheel( const S2DVector& vPos, float32_t zDelta ) ;
		virtual bool OnClickDown( const S2DVector& vPos, SGLBasicForm::MouseButton button ) ;
		virtual bool OnClickUp( const S2DVector& vPos, SGLBasicForm::MouseButton button ) ;

	} ;


	////////////////////////////////////////////////////////////////////////////////////
	// メディア表示コントローラー基底
	////////////////////////////////////////////////////////////////////////////////////

	class	S3DCanvasBasicMediaController : public S3DCanvasBasicController
	{
	public:
		enum	ParameterIndex
		{
			paramMediaSource	= S3DCanvasBasicController::paramBasicCount,
			paramPlay,
			paramPause,
			paramSeek,
			paramLoopCount,
			paramCmdVisDuration,
			paramCmdPlayDuration,
			paramBasicMediaCount,
		} ;

	protected:
		SSystem::SString	m_strSource ;
		bool				m_flagPlay ;
		bool				m_flagPause ;
		bool				m_flagStarted ;
		bool				m_flagPaused ;
		int32_t				m_nLoopCount ;
		double				m_secSeek ;
		double				m_fpPitch ;
		double				m_fpLastFrame ;
		double				m_secPlayingPos ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DCanvasBasicMediaController, S3DCanvasBasicController )
		// 構築関数
		S3DCanvasBasicMediaController( const wchar_t * pwszClassID ) ;
		// 消滅関数
		virtual ~S3DCanvasBasicMediaController( void ) ;

	public:
		// ソース更新
		virtual void UpdateMediaSource( void ) ;
		// 再生開始
		virtual void OnPlayStart( double secTime ) ;
		// 再生一時停止
		virtual void OnPlayPause( void ) ;
		// 再生再開
		virtual void OnPlayRestart( void ) ;
		// 再生停止
		virtual void OnPlayEnd( void ) ;
		// フレーム更新
		virtual void OnUpdateMediaFrame( double secTime ) ;
		// メディア時間の正規化
		virtual double NormalizeMediaTime( double secTime ) ;
		// メディアの長さ[秒]を取得する
		virtual double GetMediaDuration( void ) ;

	public:	// Parameter
		// パラメータ値取得
		virtual double GetScalarParameter( size_t iParam ) const ;
		virtual int32_t GetIntegerParameter( size_t iParam ) const ;
		virtual bool GetBooleanParameter( size_t iParam ) const ;
		virtual const wchar_t * GetCommandParameter( size_t iParam ) const ;
		// パラメータ値設定
		virtual void SetScalarParameter( size_t iParam, double s ) ;
		virtual void SetIntegerParameter( size_t iParam, int32_t n ) ;
		virtual void SetBooleanParameter( size_t iParam, bool b ) ;
		virtual void SetCommandParameter( size_t iParam, const wchar_t * pwszCmd ) ;

	public:	// ParameterProperty
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// Controller
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags ) ;
		// フレーム（パラメータ）更新後処理
		virtual void OnUpdateFrame
			( S3DSceneComposer::ItemSerializer * pItem,
				double fpFrame, S3DSceneComposer::SeekMethod seek ) ;
		// 拡張的な処理の通知
		virtual void OnExtendNotify
			( const wchar_t * pwszCmd, const wchar_t * pwszParam,
				const void * pExParam, size_t nExParamBytes ) ;
	public:
		// 再生区間にタイムライン上の表示区間を設定する
		void CmdVisDuration( void ) ;
		// 再生区間をタイムライン上に生成する
		void CmdMediaDuration( void ) ;
	} ;


	////////////////////////////////////////////////////////////////////////////////////
	// 音声トラックコントローラー
	////////////////////////////////////////////////////////////////////////////////////

	class	S3DCanvasAudioTrackController : public S3DCanvasBasicMediaController
	{
	public:
		enum	ParameterIndex
		{
			paramVolume	= S3DCanvasBasicMediaController::paramBasicMediaCount,
			paramPan,
			paramPitch,
			paramAudioTrackCount,
		} ;

	protected:
		SSystem::SSmartPointer<SGLAudioPlayerInterface>
									m_audioPlayer ;
		SGLAudioInputStream *		m_audioStream ;
		SGLSoundFormat				m_soundFormat ;
		SGLSoundPlayer				m_soundPlayer ;
		SSystem::SQueueBuffer		m_queSoundBuf ;
		SSystem::SQueueBuffer		m_queSoundOutBuf ;
		SSystem::SArray<uint8_t>	m_bufSoundPitchBuf ;
		bool						m_flagSoundOpened ;
		uint64_t					m_nSoundPlayPos ;
		double						m_fpVolume ;
		double						m_fpPan ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DCanvasAudioTrackController, S3DCanvasBasicMediaController )
		S3D_DECLARE_COMPOSER_ITEM( S3DCanvasAudioTrackController, audio_track )
		// 構築関数
		S3DCanvasAudioTrackController( const wchar_t * pwszClassID = NULL ) ;
		// 消滅関数
		virtual ~S3DCanvasAudioTrackController( void ) ;

	public:
		// オーディオストリーム解放
		void ReleaseAudioStream( void ) ;
		// 音声出力停止
		void CloseSoundPlayer( void ) ;
		// ソース更新
		virtual void UpdateMediaSource( void ) ;
		// 再生開始
		virtual void OnPlayStart( double secTime ) ;
		// 再生一時停止
		virtual void OnPlayPause( void ) ;
		// 再生再開
		virtual void OnPlayRestart( void ) ;
		// 再生停止
		virtual void OnPlayEnd( void ) ;
		// フレーム更新
		virtual void OnUpdateMediaFrame( double secTime ) ;
		// メディアの長さ[秒]を取得する
		virtual double GetMediaDuration( void ) ;

	protected:
		// 指定時間まで音声データを再生出力
		void PlayAudioStream( double secTime ) ;
		// 音声のピッチ処理
		void StreamPitchSound( void ) ;
		// 音量反映
		void RelfectAudioVolume( void ) ;

	public:	// Parameter
		// パラメータ値取得
		virtual double GetScalarParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetScalarParameter( size_t i, double s ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;

	public:	// ImageDrawer
		// 画像描画
		virtual void OnLocalDrawImage( SGLDrawImageParamList& dipl ) ;

	} ;


	////////////////////////////////////////////////////////////////////////////////////
	// 動画トラックコントローラー
	////////////////////////////////////////////////////////////////////////////////////

	class	S3DCanvasMovieTrackController : public S3DCanvasAudioTrackController
	{
	public:
		enum	ParameterIndex
		{
			paramCutLeft	= S3DCanvasAudioTrackController::paramAudioTrackCount,
			paramCutTop,
			paramCutRight,
			paramCutBottom,
			paramKeepVisible,
			paramMovieTrackCount,
		} ;

	protected:
		SGLMediaPlayerInterface *				m_pRefMovie ;
		SGLMediaPlayerInterface *				m_mediaPlayer ;
		SGLVideoInputStream *					m_videoStream ;
		SGLImageInfo							m_imginf ;
		SSystem::SSmartPointer<SGLImageObject>	m_pFrame ;
		uint64_t								m_nFrameIndex ;
		SGLRect									m_rectCutOff ;
		bool									m_flagKeepVisible ;
		bool									m_flagOutOfDuration ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DCanvasMovieTrackController, S3DCanvasAudioTrackController )
		S3D_DECLARE_COMPOSER_ITEM( S3DCanvasMovieTrackController, movie_track )
		// 構築関数
		S3DCanvasMovieTrackController( void ) ;
		// 消滅関数
		virtual ~S3DCanvasMovieTrackController( void ) ;

	public:
		// ビデオストリーム解放
		void ReleaseVideoStream( void ) ;
		// ビデオフレームシーク
		void SeekVideoFrame( double secTime ) ;
		// ビデオフレーム読み込み
		void ReadVideoFrame( void ) ;
		// ソース更新
		virtual void UpdateMediaSource( void ) ;
		// 再生開始
		virtual void OnPlayStart( double secTime ) ;
		// 再生一時停止
		virtual void OnPlayPause( void ) ;
		// 再生再開
		virtual void OnPlayRestart( void ) ;
		// 再生停止
		virtual void OnPlayEnd( void ) ;
		// フレーム更新
		virtual void OnUpdateMediaFrame( double secTime ) ;
		// メディアの長さ[秒]を取得する
		virtual double GetMediaDuration( void ) ;

	public:	// Parameter
		// パラメータ値取得
		virtual int32_t GetIntegerParameter( size_t iParam ) const ;
		virtual bool GetBooleanParameter( size_t iParam ) const ;
		// パラメータ値設定
		virtual void SetIntegerParameter( size_t iParam, int32_t n ) ;
		virtual void SetBooleanParameter( size_t iParam, bool b ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParami, SSystem::SStringArray& aStrSet ) ;
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;

	public:	// ImageDrawer
		// 画像描画
		virtual void OnLocalDrawImage( SGLDrawImageParamList& dipl ) ;

	} ;


	////////////////////////////////////////////////////////////////////////////////////
	// コンポジション描画コントローラー
	////////////////////////////////////////////////////////////////////////////////////

	class	S3DCanvasCompositionTrackController : public S3DCanvasBasicMediaController
	{
	public:
		enum	ParameterIndex
		{
			paramSpeed		= S3DCanvasBasicMediaController::paramBasicMediaCount,
			paramKeepVisible,
			paramCompositionCount,
		} ;

	protected:
		enum	InternalBufferFlag
		{
			renderTargetTemporary0	= S3DScene::renderTargetCount,
			renderTargetTemporary1,
			renderLayeredBuffer,
			renderTargetAllCount,
		} ;
		S3DRenderContext	m_render ;
		S3DSceneSprite		m_scene ;
		S3DScene::Camera	m_cameraDummy ;
		SGLImage			m_imgRender[renderTargetAllCount] ;
		SGLImage			m_imgZBuffer ;
		SGLImage			m_imgLayeredZBuffer ;
		SGLImage			m_imgRendered ;
		SGLSize				m_sizeFrameBuf ;
		bool				m_flagUpdateFrame ;
		bool				m_flagPlaying ;
		bool				m_flagPaused ;
		bool				m_flagOutOfDuration ;
		bool				m_flagKeepVisible ;
		double				m_fpLastFrame ;

		const S3DSceneComposer::CompositionInfo *				m_pciCompInfo ;
		SSystem::SSmartPointer<S3DSceneComposer::Composition>	m_pComposition ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DCanvasCompositionTrackController, S3DCanvasBasicMediaController )
		S3D_DECLARE_COMPOSER_ITEM( S3DCanvasCompositionTrackController, comp_track )
		// 構築関数
		S3DCanvasCompositionTrackController( void ) ;
		// 消滅関数
		virtual ~S3DCanvasCompositionTrackController( void ) ;

	public:
		// ソース更新
		virtual void UpdateMediaSource( void ) ;
		// 再生開始
		virtual void OnPlayStart( double secTime ) ;
		// 再生一時停止
		virtual void OnPlayPause( void ) ;
		// 再生再開
		virtual void OnPlayRestart( void ) ;
		// 再生停止
		virtual void OnPlayEnd( void ) ;
		// フレーム更新
		virtual void OnUpdateMediaFrame( double secTime ) ;
		// メディアの長さ[秒]を取得する
		virtual double GetMediaDuration( void ) ;
		// コンポジション解放
		void ReleaseComposition( void ) ;
		// フレームサイズを更新する
		void UpdateFrameSize( void ) ;
		// フレームを更新する
		void UpdateFrameImage( void ) ;

	public:	// Parameter
		// パラメータ値取得
		virtual double GetScalarParameter( size_t iParam ) const ;
		virtual bool GetBooleanParameter( size_t iParam ) const ;
		// パラメータ値設定
		virtual void SetScalarParameter( size_t iParam, double s ) ;
		virtual void SetBooleanParameter( size_t iParam, bool b ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// ImageDrawer
		// 画像描画
		virtual void OnLocalDrawImage( SGLDrawImageParamList& dipl ) ;

	} ;


	////////////////////////////////////////////////////////////////////////////////////
	// テキスト表示コントローラー
	////////////////////////////////////////////////////////////////////////////////////

	class	S3DCanvasTextController	: public S3DCanvasBasicController
	{
	public:
		enum	ParameterIndex
		{
			paramText	= S3DCanvasBasicController::paramBasicCount,
			paramTextUsage,
			paramFontFace,
			paramFontSize,
			paramTextColor,
			paramGradation,
			paramGradationColor,
			paramBorderWidth1,
			paramBorderColor1,
			paramBorderWidth2,
			paramBorderColor2,
			paramShadowAlpha,
			paramShadowColor,
			paramShadowOffsetX,
			paramShadowOffsetY,
			paramLineWidth,
			paramAlign,
			paramVCenter,
			paramCharPitch,
			paramPitchOffset,
			paramPitchScale,
			paramInHalfOffset,
			paramOutHalfOffset,
			paramLinePitch,
			paramTextCount,
		} ;
		enum	TextUsageType
		{
			textPlain,
			textCString,
			textUsageTypeCount,
		} ;

		static const SSystem::SXMLDocument::AttrInteger	m_aiUsageTypes[textUsageTypeCount+1] ;
		static const SSystem::SXMLDocument::AttrInteger	m_aiAlignTypes[4] ;

	protected:
		SSystem::SString		m_strText ;
		TextUsageType			m_usageText ;
		SSystem::SString		m_strFont ;
		int32_t					m_nFontSize ;
		int32_t					m_nLineWidth ;
		bool					m_flagVCenter ;
		bool					m_flagGradation ;
		SGLPalette				m_argbGradation[2] ;
		SGLLetteringContext		m_lettering ;
		SGLLetterer::Decoration	m_decoration ;

		SSystem::SSmartPointer<SGLImageObject>
								m_pTextImage ;
		bool					m_flagTextModified ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DCanvasTextController, S3DCanvasBasicController )
		S3D_DECLARE_COMPOSER_ITEM( S3DCanvasTextController, canvas_text )
		// 構築関数
		S3DCanvasTextController( void ) ;
		// 消滅関数
		virtual ~S3DCanvasTextController( void ) ;

	public:
		// 文字外観画像更新
		void UpdateTextImage( void ) ;

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

	public:	// ParameterProperty
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// ImageDrawer
		// 画像描画
		virtual void OnLocalDrawImage( SGLDrawImageParamList& dipl ) ;

	} ;


	////////////////////////////////////////////////////////////////////////////////////
	// ビルボード（疑似3D）パーティクル・コントローラー
	////////////////////////////////////////////////////////////////////////////////////

	class	S3DCanvasBillboardController	: public S3DCanvasBasicController
	{
	public:
		// 描画画像情報
		struct	Description
		{
			size_t		iImage ;		// 画像インデックス
			S2DVector	vOffset ;		// オフセット座標
			S2DVector	vScale ;		// 表示スケール
			uint32_t	nTransparency ;	// 透明度

			Description( void )
				: iImage(0), vOffset(0,0), vScale(1,1), nTransparency(0) { }
			Description( const Description& desc )
				: iImage(desc.iImage),
					vOffset(desc.vOffset),
					vScale(desc.vScale),
					nTransparency(desc.nTransparency) { }
		} ;
		struct	ParticleDesc
		{
			S3DVector			vPosition ;		// 粒子座標
			size_t				nCount ;		// 画像数
			const Description *	pDesc ;			// Description 配列
		} ;

		// 粒子状態
		enum	ParticleState
		{
			stateContinue,
			stateDestroyed,
		} ;

		// 抽象粒子
		class	Particle	: public ESLObject
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( Particle, ESLObject )
			// 時間変化
			virtual ParticleState OnTimer
				( S3DCanvasBillboardController& billboard, uint32_t msecPast ) = 0 ;
			// 描画情報取得
			virtual void GetParticle
				( S3DCanvasBillboardController& billboard, ParticleDesc& desc ) = 0 ;
		} ;

		// 画像情報
		struct	ImageEntry
		{
			SGLImageObject *	pImage ;
			SGLImageRect		rectRef ;
			S2DVector			vScale ;
		} ;

	protected:
		SSystem::SArray<ImageEntry>				m_aImages ;
		S2DVector								m_vImageScale ;

		SSystem::SCriticalSection				m_csParticle ;
		SSystem::SSmartObjectArray<Particle>	m_aParticles ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DCanvasBillboardController, S3DCanvasBasicController )
		S3D_DECLARE_COMPOSER_ITEM( S3DCanvasBillboardController, canvas_billboard )
		// 構築関数
		S3DCanvasBillboardController( void ) ;
		// 消滅関数
		virtual ~S3DCanvasBillboardController( void ) ;

	public:
		// 画像登録
		size_t AddImageEntry
			( SGLImageObject * pImage,
				const SGLImageRect * pSrcRect = nullptr,
				const S2DVector * pvScale = nullptr ) ;
		size_t AddImageEntries
			( const ImageEntry * pEntries, size_t nCount ) ;
		// 画像登録解除
		void ClearAllImageEntries( void ) ;
		// 画像表示スケール（距離/ピクセル）
		void SetImageScale( const S2DVector& vScale ) ;
		const S2DVector& GetImageScale( void ) const ;

	public:
		// 粒子追加
		void AddParticle( Particle * pParticle ) ;

	public:	// Controller
		// タイマー処理
		virtual void OnTimer
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast ) ;

	public:	// ImageDrawer
		// 画像描画
		virtual void OnLocalDrawImage( SGLDrawImageParamList& dipl ) ;
	} ;


	////////////////////////////////////////////////////////////////////////////////////
	// ビルボード（疑似3D）パーティクル基底
	////////////////////////////////////////////////////////////////////////////////////

	class	S3DCanvasBillboardParticle	: public S3DCanvasBillboardController::Particle
	{
	public:
		// パーティクル基本情報
		struct	ParticleParam
		{
			S3DDVector	vPos ;			// 位置
			S3DDVector	vSpeed ;		// 速度 [/sec]
			double		fpAttenuation ;	// 減速率 [0,1] [/sec]
			uint32_t	msecLife ;		// 寿命 [msec]
			uint32_t	msecFadeout ;	// フェードアウト時間 [msec]

			ParticleParam( void )
				: vPos(0,0,0), vSpeed(0,0,0),
					fpAttenuation(0.9), msecLife(1000), msecFadeout(500) { }
		} ;

		// 文字画像割り当て
		class	FontMap
			: public SSystem::SSortArray
						< SSystem::SSortElement<wchar_t,size_t> >
		{
		public:
			SSystem::SArray
				<S3DCanvasBillboardController::ImageEntry>
									m_aImageEntries ;

		public:
			// クリア
			void RemoveAll( void ) ;
			// 文字画像追加
			void AddFont
				( wchar_t wchCode,
					SGLImageObject * pImage,
					const SGLImageRect * pRect = nullptr ) ;
			// 画像登録
			void AddImageEntriesTo( S3DCanvasBillboardController& billboard ) ;
		} ;

	protected:
		ParticleParam	m_param ;
		SSystem::SArray
			<S3DCanvasBillboardController::Description>
						m_aImageDesc ;		// 画像
		SSystem::SArray
			<S3DCanvasBillboardController::Description>
						m_aImageDescTemp ;
		uint32_t		m_msecPast ;		// 経過時間 [msec]

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DCanvasBillboardParticle, Particle )
		// 構築関数
		S3DCanvasBillboardParticle( void ) ;
		S3DCanvasBillboardParticle( const ParticleParam& param, size_t iImage ) ;
		// パラメータ
		void SetParticleParam( const ParticleParam& param ) ;
		const ParticleParam& GetParticleParam( void ) const ;
		// 画像
		void SetImageIndex( size_t iImage ) ;
		size_t GetImageIndex( void ) const ;
		void SetImageDescription
			( const S3DCanvasBillboardController::Description * pDesc, size_t nCount ) ;
		// 文字列画像
		void SetTextFont
			( const FontMap& fontMap, const wchar_t * pwszText,
				size_t pitchChar, float32_t fpScale = 1.0f ) ;

	public:
		// 時間変化
		virtual S3DCanvasBillboardController::ParticleState
			OnTimer( S3DCanvasBillboardController& billboard, uint32_t msecPast ) ;
		// 描画情報取得
		virtual void GetParticle
			( S3DCanvasBillboardController& billboard,
				S3DCanvasBillboardController::ParticleDesc& desc ) ;
	} ;


	////////////////////////////////////////////////////////////////////////////////////
	// ブラー効果コントローラー
	////////////////////////////////////////////////////////////////////////////////////

	class	S3DCanvasBlurEffectController
				: public S3DSceneComposer::Controller,
					public S3DCanvasSerializer::ImageEffector
	{
	public:
		enum	ParameterIndex
		{
			paramBlurType,
			paramBlurCenter,
			paramBlurDirection,
			paramBlurOffset,
			paramBlurGauss,
			paramBlurScaleUnit,
			paramBlurScalePower,
			paramBlurBrightness,
			paramColorMul,
			paramColorAdd,
			paramBlurCount,
		} ;
		enum	BlurType
		{
			blurNo,
			blurBiAxial,
			blurVector,
			blurRadial,
			blurCount,
		} ;

		static const SSystem::SXMLDocument::AttrInteger	m_aiBlurTypes[blurCount + 1] ;

	protected:
		BlurType	m_typeBlur ;
		S3DDVector	m_vCenter ;
		S2DDVector	m_vDirection ;
		S2DDVector	m_vOffset ;
		double		m_fpGauss ;
		double		m_fpScaleUnit ;
		double		m_fpScalePower ;
		double		m_fpBrightness ;
		SGLPalette	m_rgbMul ;
		SGLPalette	m_rgbAdd ;

		SGLImage	m_imgWorkBuf ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2( S3DCanvasBlurEffectController, Controller, ImageEffector )
		S3D_DECLARE_COMPOSER_ITEM( S3DCanvasBlurEffectController, canvas_blur_effector )
		// 構築関数
		S3DCanvasBlurEffectController( void ) ;
		// 消滅関数
		virtual ~S3DCanvasBlurEffectController( void ) ;

	public:	// Parameter
		// パラメータ値取得
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		virtual size_t GetBinaryParameter
			( void * pDst, size_t nBufBytes, size_t i ) const ;
		// パラメータ値設定
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		virtual size_t SetBinaryParameter
			( size_t i, const void * pSrc, size_t nBufBytes ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;

	public:
		// フィルタ有効か？
		virtual bool IsEnabledFilter( void ) const ;
		// フィルタ処理
		virtual SGLImageObject * OnImageFilter
			( S3DRenderContextInterface& render,
				SGLImageObject * pDstImage, SGLImageObject * pSrcImage ) ;
	protected:
		SGLImageObject * NextFilterBuffer
			( SGLImageObject * pDstImage, SGLImageObject * pSrcImage ) ;

	} ;


}

#endif

