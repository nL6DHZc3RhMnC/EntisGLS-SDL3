
#if	!defined(__SAKURAGLX_SPRITE_FORMED_H__)
#define	__SAKURAGLX_SPRITE_FORMED_H__	1

#include <sakuraglx/sprite/sglx_sprite_message.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// スキンページ用スプライト
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteFormed	: public SGLSprite
	{
	protected:
		SSystem::SSmartReference<SGLSkinManager>		m_refSkin ;
		SSystem::SSmartReference<SGLBasicFormParser>	m_refFormParser ;
		SSystem::SSmartPointer<SGLBasicForm>			m_pForm ;

		SGLDrawImageParamList	m_dipList ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteFormed, SGLSprite )
		// 構築関数
		SGLSpriteFormed( void ) ;
		SGLSpriteFormed( const SGLSpriteFormed& src ) ;
		// 消滅関数
		virtual ~SGLSpriteFormed( void ) ;

	public:
		// スキンを関連付け
		void AttachSkin( SGLSkinManager * pSkin ) ;
		// 関連付けられたスキンを取得
		SGLSkinManager * GetAttachedSkin( void ) const ;
		// SGLBasicFormParser 関連付け
		void AttachFormParser( SGLBasicFormParser * pFormParser ) ;
		// 関連付けられた SGLBasicFormParser を取得
		SGLBasicFormParser * GetFormParser( void ) const ;
		// フォーム設定
		void SetBasicForm( SGLBasicForm * pForm ) ;
		// フォーム取得
		SGLBasicForm * GetBasicForm( void ) const ;
		// スキンリソース解放
		void ReleaseForm( void ) ;

	public:
		// フレーム描画（視点に関係しない）共通処理
		virtual void PrepareDrawFrame( void ) ;
		// 子スプライトを描画
		virtual void DrawChildren
			( S3DRenderContextInterface& render,
					Stereo3DView s3dView = s3dMonoview ) const ;
		virtual void DrawChildrenImageList
			( SGLDrawImageParamList& dipl,
					Stereo3DView s3dView = s3dMonoview ) const ;

	public:	// 子アイテムへの属性
		// 可視状態
		virtual bool IsSpriteVisible( const wchar_t * pwszID ) const ;
		virtual void SetSpriteVisible( const wchar_t * pwszID, bool fVisible ) ;
		// 透明度
		virtual uint32_t GetSpriteTransparency( const wchar_t * pwszID ) const ;
		virtual void SetSpriteTransparency
			( const wchar_t * pwszID, uint32_t nTransparency ) ;
		// 表示領域
		virtual bool GetSpriteRectangle
				( const wchar_t * pwszID, SGLRect& rectExt ) const ;
		// 文字列属性
		virtual SSystem::SString GetSpriteText( const wchar_t * pwszID ) const ;
		virtual void SetSpriteText
			( const wchar_t * pwszID, const wchar_t * pwszText ) ;
		// 文字フォント属性
		virtual void SetSpriteTextFont
			( const wchar_t * pwszID,
				const wchar_t * pwszFont, int nSize = 0 ) ;
		// 画像属性
		virtual void SetSpriteImage
			( const wchar_t * pwszID, const wchar_t * pwszImageID ) ;
		// 入力禁止状態
		virtual bool IsSpriteEnabled( const wchar_t * pwszID ) const ;
		virtual void SetSpriteEnable( const wchar_t * pwszID, bool fEnable ) ;
		// スクロール・トラック位置属性
		virtual int GetSpriteScrollPos
			( const wchar_t * pwszID,
				ScrollDirection scrlDir = scrollDefault ) const ;
		virtual void SetSpriteScrollPos
			( const wchar_t * pwszID,
				int nPos, ScrollDirection scrlDir = scrollDefault ) ;
		// スクロール・トラック位置範囲属性
		virtual int GetSpriteScrollRange
			( const wchar_t * pwszID,
				ScrollDirection scrlDir = scrollDefault ) const ;
		virtual void SetSpriteScrollRange
			( const wchar_t * pwszID,
				int nRange, ScrollDirection scrlDir = scrollDefault ) ;
		// ボタン属性
		virtual bool IsSpriteButtonChecked( const wchar_t * pwszID ) const ;
		virtual void CheckSpriteButton
			( const wchar_t * pwszID, bool fCheck ) ;

	public:
		// 時間経過処理
		virtual void AdvanceTime( uint32_t msecPast ) ;
		// 更新領域通知
		virtual void PostUpdate( SGLRect* pUpdate = NULL ) ;
		// 外接矩形取得
		virtual bool GetRectangle( SGLRect& rectExt ) const ;
		// ヒット判定
		virtual bool IsHitSprite( double x, double y ) const ;
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
		static SGLBasicForm::MouseButton MouseButtonFromSpriteMouseFlags( int64_t nFlags ) ;

	protected:
		// 画像描画リスト
		virtual void OnDrawImage( SGLDrawImageParamList& dipl ) ;
	public:
		// 描画リスト取得
		SGLDrawImageParamList& DrawImageParamList( void ) ;

	public:
		// 複製
		virtual SGLObject * DuplicateObject( void ) ;
		// シリアライズ
		virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
		// 復元
		virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;
	} ;



	//////////////////////////////////////////////////////////////////////////
	// ストレッチフレーム付きスプライト
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteFormFramed	: public SGLSpriteFormed
	{
	public:
		// スタイル
		struct	FrameStyle	: public SGLSpriteMessage::RichTextStyle
		{
			const SGLBasicForm::ImageSet *	pImageSet ;
			SGLRect							rectInnerMargin ;
			SGLRect							rectBackMargin ;
			SGLRect							rectCaptionMargin ;

			// 構築関数（デフォルト値）
			FrameStyle( void ) ;
			// 構築関数（複製）
			FrameStyle( const FrameStyle& style ) ;
			// 代入
			const FrameStyle& operator = ( const FrameStyle& style ) ;
		} ;

	protected:
		// スタイル
		SSystem::SString		m_strFontFace ;
		SSystem::SString		m_strRubyFont ;
		SSystem::SString		m_strProhibition ;
		FrameStyle				m_styleFrame ;

		SGLSize					m_sizeFrame ;
		SGLSize					m_sizeInner ;
		bool					m_flagInnerBuffer ;

		SSystem::SString		m_strCaption ;
		SSystem::SSmartPointer
			<SGLImageObject>	m_imgCaption ;
		SGLPoint				m_ptCaptionOffset ;

		SGLBasicForm::StretchFrame *	m_pFrame ;
		SGLBasicForm::SubForm *			m_pSubForm ;
		SGLBasicForm::Sprite *			m_pSubSprite ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteFormFramed, SGLSpriteFormed )
		// 構築関数
		SGLSpriteFormFramed( void ) ;
		SGLSpriteFormFramed( const SGLSpriteFormFramed& src ) ;
		// 消滅関数
		virtual ~SGLSpriteFormFramed( void ) ;

	public:
		// フレームスタイルIDを指定してスタイル設定
		void AttachFrameStyle
			( SGLBasicFormParser * pFormParser, const wchar_t * pwszStyleID ) ;
		// スタイル設定
		void SetFrameStyle( const FrameStyle& style ) ;
		// スタイル取得
		const FrameStyle& GetFrameStyle( void ) const ;
		// スタイル解釈
		static void ParseFrameStyle
			( FrameStyle& style,
				SGLBasicFormParser * pFormParser,
				SSystem::SString& strFontFace,
				SSystem::SString& strRubyFont,
				const SSystem::SXMLDocument& xmlStyle ) ;

	public:
		// フレーム作成
		SGLError CreateFrame
			( const wchar_t * pwszCaption,
				const SGLSize& sizeInner, bool flagInnerBuffer = true ) ;
		// フレーム削除
		void ReleaseFrame( void ) ;
		// フレーム取得
		SGLBasicForm::StretchFrame * GetFrame( void ) const ;
		// クライアント領域アイテム取得
		SGLBasicForm * GetClientForm( void ) ;
		SGLSpriteFormed * GetClientSprite( void ) ;
		// クライアントサイズ変更
		SGLError ResizeFrame( const SGLSize& sizeInner ) ;
		// キャプション変更
		void SetCaptionText( const wchar_t * pwszCaption ) ;
		void UpdateCaption( void ) ;

	public:
		// フレーム描画（視点に関係しない）共通処理
		virtual void PrepareDrawFrame( void ) ;
	} ;

}

#endif

