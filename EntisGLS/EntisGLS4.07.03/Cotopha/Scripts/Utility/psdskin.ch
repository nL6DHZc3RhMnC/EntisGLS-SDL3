
; -----------------------------------------------------------------------------
;	ERISA 画像コンバーター用 PSD->スキン変換スクリプトライブラリ
; -----------------------------------------------------------------------------

Include	"imagecontext.ch"



;	画像リソース情報構造体
; -----------------------------------------------------------------------------

Structure	ResourceInfo
	String	sRsrcType		; リソースタイプ（image, sound）
	String	sFileName		; リソースファイル名
	Point	ptOffset		; 表示基準座標
	Size	sizeImage		; 画像サイズ
EndStruct


;	ページアイテム構造体
; -----------------------------------------------------------------------------

Structure	PageItem
	String	sID				; アイテム識別子
	Point	ptItem			; アイテム座標
	Size	sizeItem		; アイテムサイズ
	Integer	nFlags

	Prototype	void FormatSkin() abstract
EndStruct


;	ページフォーム構造体
; -----------------------------------------------------------------------------

Structure	PageForm
	String		sID				; ページ識別子
	Point		ptPage			; ページデフォルト座標
	Size		sizePage		; ページサイズ
	String		sBG				; 背景画像
	PageItem&[]	aItems			; アイテム（PageItem 配列）
EndStruct


;	基底スタイル構造体
; -----------------------------------------------------------------------------

Structure	ItemStyle
	String	sID				; スタイル識別子

	Prototype	void FormatSkin() abstract
EndStruct


;	静テキストスタイル構造体
; -----------------------------------------------------------------------------

Structure	TextStyle	: ItemStyle
	String	sBoxAlign		; ボックスアライン
	String	sAlign			; テキストアライン
	Integer	nLineHeight		; 行間
	Rect	rctExt			; 外接矩形
	Integer	nFontSize		; フォントサイズ
	String	sFontFace		; フォント名
	Boolean	fBold			; 太字
	Boolean	fItalic			; 斜体
	Integer	rgbColor		; テキストカラー
	Integer	nTransparency	; 透明度
	Point	ptShadow		; 影オフセット
	Integer	rgbShadow		; 影色
	Integer	nShadowTrans	; 影透明度
	Boolean	fBordering		; 縁取り
	Integer	nFontPitch		; 固定ピッチ幅

	Prototype	void FormatSkin()
	Prototype	void FormatSkin( Boolean fNoStyleTag )
EndStruct

Constant	txtAlignLeft		:= 0,
			txtAlignTop			:= 1,
			txtAlignRight		:= 2,
			txtAlignCenter		:= 3,
			txtAlignAccordance	:= 4


;	ボタンスタイル構造体
; -----------------------------------------------------------------------------

Structure	ButtonStyle	: ItemStyle
	String			sType		; ボタンタイプ { "button" | "check" | "radio" }
	Point			ptOffset	; 表示基準座標
	Size			sizeButton	; ボタンのサイズ
	Boolean			fHitRect	; 矩形による当たり判定を行うか？
	Hash<String>	aImages		; 状態ごとの画像識別子
	Hash<TextStyle>	aTextStyles	; 状態ごとのテキストスタイル

	Prototype	void FormatSkin()
EndStruct

Data	ButtonStatusTable
	:= "normal", := "mask", := "focus", := "pushed"
	:= "pushed_focus", := "active_pushed"
	:= "disabled", := "push_disabled"
EndData

Constant	btButton			:= 0,
			btCheck				:= 1,
			btRadio				:= 2
Constant	btnHideNormal		:= 01H,
			btnHitRect			:= 02H,
			btnHitMask			:= 04H,
			btnVisFocus			:= 08H,
			btnVisPushed		:= 10H,
			btnVisPushedFocus	:= 20H,
			btnVisActivePushed	:= 40H,
			btnHiddenRect		:= \
				(btnHideNormal or btnHitRect or \
					btnVisFocus or btnVisPushed or btnVisPushedFocus),
			btnHiddenMask		:= \
				(btnHideNormal or btnHitMask or \
					btnVisFocus or btnVisPushed or btnVisPushedFocus)


;	進捗バースタイル構造体
; -----------------------------------------------------------------------------

Structure	ProgressBarStyle	: ItemStyle
	Point	ptOffset		; 表示基準座標
	Size	sizeFrame		; フレーム画像サイズ
	String	sType			; バータイプ { "horz" | "vert" }
	Point	ptBarOffset		; バー表示相対座標
	String	sFrameImage		; フレーム画像
	String	sBarImage		; バー画像

	Prototype	void FormatSkin()
EndStruct

Constant	pbtVert				:= 0,
			pbtHorz				:= 1


;	スクロールバースタイル構造体
; -----------------------------------------------------------------------------

Structure	ScrollBarStyle	: ItemStyle
	Point		ptOffset		; 表示基準座標
	Size		sizeColum		; つまみ軌道サイズ
	String		sType			; アラインタイプ
	Integer		nBarOffset		; バーオフセット
	String		sUpButton		; 上ボタンスタイル
	String		sDownButton		; 下ボタンスタイル
	String[]	aBarImages		; つまみ画像
	String[]	aTrackImages	; つまみ軌道画像
	Rect		rctTrackSpace	; つまみ軌道余白
	String		sProgressBar	; 進捗バー画像
	Size		sizeProgBar		; 進捗バーサイズ
	Boolean		fProgBarStretch	; 進捗バー左右指定
	Integer		nProgBarLeft	; 進捗バー：伸縮しない画像端の幅
	Integer		nProgBarRight

	Prototype	void FormatSkin()
EndStruct

Constant	sbtVert				:= 0,
			sbtHorz				:= 1


;	編集テキストスタイル構造体
; -----------------------------------------------------------------------------

Structure	EditTextStyle	: ItemStyle
	String	sType			; 編集タイプ
	Integer	nFontSize		; フォントサイズ
	String	sFontFace		; フォントフェイス
	Boolean	fBold			; 太字指定
	Boolean	fItalic			; 斜体指定
	Integer	nLineTop		; 行のトップ下げ幅
	Integer	nLineBottom		; 行の下ライン
	Integer	rgbTextColor	; 文字色
	Integer	rgbSelColor		; 選択文字色
	Size	sizeCaret		; カレットサイズ
	Integer	nCaretInterval	; カレットインターバル
	Integer	rgbaCaretColor	; カレット色
	Integer	nIMEFontSize	; IME フォントサイズ
	String	sIMEFontFace	; IME フォントフェイス

	Prototype	void FormatSkin()
EndStruct

Constant	edtSingleLine	:= 0,
			edtMultiLine	:= 1


;	フレームスタイル構造体
; -----------------------------------------------------------------------------

Structure	FrameStyle	: ItemStyle
	Hash<String>	aFrameImages

	Prototype	void FormatSkin()
EndStruct


;	画像アイテム構造体
; -----------------------------------------------------------------------------

Structure	ImageItem	: PageItem
	String	sRsrcID			; リソース識別子
	Boolean	fHitTransparency

	Prototype	void FormatSkin()
EndStruct


;	静テキストアイテム構造体
; -----------------------------------------------------------------------------

Structure	TextItem	: PageItem
	String	sStyleID
	String	sText

	Prototype	void FormatSkin()
EndStruct


;	矩形アイテム構造体
; -----------------------------------------------------------------------------

Structure	RectangleItem	: PageItem
	Integer	rgbaColor

	Prototype	void FormatSkin()
EndStruct


;	ボタンアイテム構造体
; -----------------------------------------------------------------------------

Structure	ButtonItem	: PageItem
	String			sStyleID
	String			sPushedSE			; SE
	String			sFocusSE
	String			sReflectTarget		; ステータスリフレクションターゲット
	Hash<String>	aReflectionImage

	Prototype	void FormatSkin()
EndStruct

Constant	itemNoTabStop	:= 01H,
			itemNoGroup		:= 02H,
			buttonStatusNotification	:= 10H,
			buttonAcceptRightClick		:= 20H


;	進捗バーアイテム構造体
; -----------------------------------------------------------------------------

Structure	ProgressBarItem	: PageItem
	String	sStyleID
	Integer	nBarWidth
	Integer	nRange

	Prototype	void FormatSkin()
EndStruct


;	スクロールバーアイテム構造体
; -----------------------------------------------------------------------------

Structure	ScrollBarItem	: PageItem
	String	sStyleID
	Integer	nBarWidth
	Integer	nRange

	Prototype	void FormatSkin()
EndStruct


;	編集テキストアイテム構造体
; -----------------------------------------------------------------------------

Structure	EditTextItem	: PageItem
	String	sStyleID
	String	sText
	Integer	nLimit			; 入力制限文字数
	Integer	nAccept			; 入力許可

	Prototype	void FormatSkin()
EndStruct

Constant	acceptEditReturn	:= 01H,
			acceptEditTab		:= 02H


;	オブジェクトアイテム構造体
; -----------------------------------------------------------------------------

Structure	ObjectItem	: PageItem
	String	sStyleID

	Prototype	void FormatSkin()
EndStruct



;	スキンデータ構造体
; -----------------------------------------------------------------------------

Structure	SkinData
	Hash<ResourceInfo>	\
					aImages			; 画像リソース（Key=ID, ResourceInfo 配列）
	Hash<ItemStyle&>	\
					aStyles			; スタイル（ButtonStyle, etc 配列）
	PageForm[]		aPages			; ページフォーム（PageForm 配列）

	Integer			nError			; エラー発生数

	ImageContext&	rContext		; 入力画像コンテキスト
	Integer			nCutAlign		; 切り出しグリッド幅
	Integer			nCutMargin		; 切り出しマージン幅
	Integer			nCutThreshold	; 切り出し透明度閾値
	Boolean			fDirection		; アイテム挿入方向

	String			sDefPushedSE	; デフォルトのSE
	String			sDefFocusSE

	String			sDefDstDir		; デフォルトの出力ディレクトリ

	Prototype	void SetCutParam(
					Integer nAlign, Integer nMargin, Integer nThreshold )
	Prototype	void SetDestinationDirectory( String sDstDir )
	Prototype	void SetToAddItem( Boolean fAddFlag )
	Prototype	void SetDefaultSE( String sPushedSE, String sFocusSE )
	Prototype	void SetContext( ImageContext& imgctx )
	Prototype	Integer FindLayerIndex(
						Integer iLayer, Boolean fNoErrorMsg := false )
	Prototype	Integer FindLayerIndex(
						String sLayer, Boolean fNoErrorMsg := false )
	Prototype	void BeginPage(
						String sPageID, ImageContext& imgctx,
						String sBGID := "",
						String sBGFile := "", Integer iLayer := 0 )
	Prototype	Error SaveImage(
					String sImageID, String sImageFile,
					Integer iLayer, Boolean fKeepHotspot := false,
					Rect& rCurRect := null )
	Prototype	Error SaveImage(
					String sImageID, String sImageFile,
					String sLayer, Boolean fKeepHotspot := false,
					Rect& rCurRect := null )
	Prototype	Error SaveAnimation(
					String sImageID, String sImageFile,
					Integer iFirst, Integer nCount, String sName,
					Integer nAnimeDuration, Boolean fKeepHotspot,
					String sSequence := "", Rect& rCurRect := null,
					Boolean fGrayscale := false )
	Prototype	Error SaveImages(
					String[]& aImageID, String[]& aImageFile,
					String[]& aLayer,
					Integer[]& aDurations := null,
					Boolean fKeepHotspot := false,
					Rect& rCurRect := null )
	Prototype	ResourceInfo& RegImage(
					String sImageID, String sImageFile,
					Point& rPosition := null,
					Size& rSize := null, String sType := "" )
	Prototype	ResourceInfo& GetImage( String sImageID )
	Prototype	TextStyle& AddTextStyle(
					String sStyleID, Integer nAlign,
					String sFontFace, Integer nSize, Integer nLineHeight,
					Boolean fBold, Boolean fItalic,
					Integer rgbColor, Integer nTransparency,
					Point& rShadowOffset, Integer rgbShadow,
					Integer nShadowTrans, Boolean fBordering := false,
					Integer nFontPitch := 0 )
	Prototype	ButtonStyle& AddButtonStyle(
					String sStyleID, String sImageID,
					Integer nType, Integer nFlags,
					String sLayer, Integer iHitMask, Integer iFocus,
					Integer iPushed := 0, Integer iPushedFocus := 0, 
					Integer iActivePushed := 0, Integer iDisabled := 0,
					Integer iDisablePushed := 0, Rect& rCutRect := null )
	Prototype	ButtonStyle& AddButtonStyle(
					String sStyleID, String sImageID,
					Integer nType, Integer nFlags,
					Integer iLayer, Integer iHitMask, Integer iFocus,
					Integer iPushed := 0, Integer iPushedFocus := 0, 
					Integer iActivePushed := 0, Integer iDisabled := 0,
					Integer iDisablePushed := 0, Rect& rCutRect := null )
	Prototype	ButtonStyle& RegButtonStyle( 
					String sStyleID, String sImageID,
					ImageContext& imgctx, Integer nType, Integer nFlags,
					Integer iHitMask, Integer iFocus,
					Integer iPushed, Integer iPushedFocus, 
					Integer iActivePushed, Integer iDisabled,
					Integer iDisablePushed )
	Prototype	void SetButtonTextStyle(
					String sStyleID, Integer nLeft, Integer nTop,
					Integer nRight, Integer nBottom,
					Integer nDeltaX, Integer nDeltaY,
					String sNormal, String sFocus,
					String sPushed, String sPushedFocus,
					String sActivePushed, String sDisabled,
					String sDisablePushed )
	Prototype	Error GetButtonStyleRect(
					String sStyleID, Point& rPosition, Size& rSize )
	Prototype	ProgressBarStyle& AddProgressBarStyle(
					String sStyleID, Integer nBarType,
					String sBarLayer, String sFrameLayer,
					Rect& rFrameCutRect := null )
	Prototype	ScrollBarStyle& AddScrollBarStyle(
					String sStyleID, Integer nBarType,
					String sBarLayer, Integer nBarNormal,
					Integer nBarFocus, Integer nBarTracking,
					Integer nBarDisabled,
					String sColLayer, Integer nColNormal,
					Integer nColFocus, Integer nColTracking,
					Integer nColDisabled,
					Rect& rColCutRect, Integer nBarOffset := 0,
					String sUpButton := "", String sDownButton := "",
					Rect& rTrackSpaceRect := null,
					String sProgressLayer := "" )
	Prototype	ScrollBarStyle& AddScrollBarStyle(
					String sStyleID, Integer nBarType,
					Integer iBarLayer, Integer nBarNormal,
					Integer nBarFocus, Integer nBarTracking,
					Integer nBarDisabled,
					Integer iColLayer, Integer nColNormal,
					Integer nColFocus, Integer nColTracking,
					Integer nColDisabled,
					Rect& rColCutRect, Integer nBarOffset := 0,
					String sUpButton := "", String sDownButton := "",
					Rect& rTrackSpaceRect := null, Integer iProgressBar := -1 )
	Prototype	EditTextStyle& AddEditTextStyle(
					String sStyleID, Integer nType,
					Integer nFontSize, String sFontFace,
					Boolean fBold, Boolean fItalic,
					Integer nLineTop, Integer nLineBottom,
					Integer rgbTextColor, Integer rgbSelColor,
					Integer nCaretWidth, Integer nCaretHeight,
					Integer nCaretInterval, Integer rgbaCaretColor,
					Integer nIMEFontSize := 0, String sFontFace := "Default" )
	Prototype	FrameStyle& AddFrameStyle(
					String sStyleID, String sImageBaseID,
					Integer nFlags, String sBaseLayer,
					Integer iUpperLeft, Integer iUpper, Integer iUpperRight,
					Integer iLeft, Integer iCenter, Integer iRight,
					Integer iUnderLeft, Integer iUnder, Integer iUnderRight )
	Prototype	TextItem& AddTextItem(
					String sItemID, String sStyleID,
					Integer x, Integer y,
					Integer w, Integer h, String sText := "" )
	Prototype	RectangleItem& AddRectangleItem(
					String sItemID,
					Integer x, Integer y,
					Integer w, Integer h, Integer argbColor := 0 )
	Prototype	RectangleItem& AddRectangleItem(
					String sItemID, String sLayer, Integer argbColor := 0 )
	Prototype	ImageItem& AddImageItem(
					String sItemID, String sImageID,
					Boolean fHitTransparency := false,
					Point& rPosition := null )
	Prototype	ButtonItem& AddButtonItem(
					String sItemID, String sStyleID, Integer nFlags := 0,
					Point& rPosition := null,
					String sPushedSE := "", String sFocusSE := "",
					String sReflectionTarget := "",
					String sReflectionStyle := "" )
	Prototype	ProgressBarItem& AddProgressBarItem(
					String sItemID, String sStyleID,
					Integer nRange, Integer nFlags := 0,
					Point& rDeltaPos := null, Point& rPosition := null )
	Prototype	ScrollBarItem& AddScrollBarItem(
					String sItemID, String sStyleID,
					Integer nRange, Integer nFlags := 0,
					Point& rDeltaPos := null,
					Point& rPosition := null, Size& rSize := null )
	Prototype	EditTextItem& AddEditTextItem(
					String sItemID, String sStyleID,
					Integer x, Integer y, Integer w, Integer h,
					String sText, Integer nLimit )
	Prototype	ObjectItem& AddObjectItem(
					String	sItemID, String sPageID,
					Integer& x := null, Integer& y := null )
	Prototype	PageForm& GetCurrentPage()
	Prototype	void SmartPage( Integer nAlign,
					Integer xMarginLeft := 0, Integer yMarginTop := 0,
					Integer xMarginRight := 0, Integer yMarginBottom := 0 )
	Prototype	void ClipPage( Integer nLeft, Integer nTop,
								 Integer nWidth, Integer nHeight )
	Prototype	void FormatSkin()

EndStruct

