
#include <imagecontext.h>


// ImageContext 内のレイヤーを
// filename + conjunction + layer_name の名前のファイル名で書き出す
void SaveImageEachLayer
	( ImageContext& imgctx, String dstdir,
		String mime = "image/x-erina", int quality = -1,
		String conjunction = "_", bool fKeepPosition = true,
		int alignCut = 1, int marginCut = 0, int thresholdCut = 0 ) ;

// ImageContext 内のレイヤーをアニメーション画像として出力
void SaveAnimationRichLayer
	( ImageContext& imgctx, String dstdir,
		String mime = "image/x-erina", int quality = -1,
		bool fKeepPosition = true,
		int alignCut = 1, int marginCut = 0, int thresholdCut = 0 ) ;

// ImageContext 内のレイヤーをスキンデータとして出力
int MakeSkinRichLayer
	( ImageContext& imgctx, String dstdir,
		String mime = "image/x-erina",
		bool fKeepPosition = false,
		int alignCut = 1, int marginCut = 0, int thresholdCut = 0 ) ;


// スキン・スタイル
class	SkinItemStyle
{
public:
	XMLDocument&	m_xmlStyle ;
	Point			m_ptStyle ;
	Size			m_sizeStyle ;
} ;

// スキン・画像
class	SkinResource	: public SkinItemStyle
{
public:
	String	m_strFile ;
	Point	m_ptHotSpot ;
} ;

// ページ・データ
class	SkinPageData
{
public:
	XMLDocument&		m_xmlPage ;
	SkinItemStyle&[]	m_items ;
	int					m_iAlign ;

public:
	// 初期設定
	void InitializePage( String sID, int nWidth, int nHeight ) ;
	// アイテム追加
	SkinItemStyle& AddItem( void ) ;
	// ページの位置とサイズを最適化
	void SmartPage( void ) ;
	// アイテム座標変更
	void OffsetItems( int xOffset, int yOffset ) ;
} ;

// スキンアイテム・レイヤー・プロパティ・エントリ
class	SkinItemLayerProperty
{
public:
	String	sName ;		// プロパティ名
	int		iLayer ;	// レイヤー番号
	String	sProperty ;	// プロパティ
} ;

// スキンデータ
class	SkinContext
{
public:
	Hash<SkinResource>	m_rsrc ;
	Hash<SkinItemStyle>	m_styles ;
	Hash<SkinPageData>	m_pages ;
	SkinPageData&		m_curPage ;

	String				m_sDefItemType = "image" ;
	String				m_sDefFocusSE = "" ;
	String				m_sDefPushedSE = "" ;

	String				m_dstdir ;
	String				m_mimeImage ;
	int					m_alignCut ;
	int					m_marginCut ;
	int					m_thresholdCut ;
	int					m_flagsCut ;

	int					m_countError ;

public:
	// レイヤー解釈
	int MakeSkin
		( ImageContext& imgctx, String dstdir,
			String mime, bool fKeepPosition,
			int alignCut, int marginCut, int thresholdCut ) ;
	// system.inf 書き出し
	Error SaveSkinDescription( String filename ) ;
	// エラー出力
	void OutputError( String sErrMsg ) ;

public:
	// 画像リソースを保存して登録
	String SaveImageResource
		( ImageContext& imgctx, String sRsrcID,
			const Point& ptImage, const Size& sizeImage ) ;
	// 画像リソースを登録
	String AddImageResource
		( ImageContext& imgctx, String sType, String sRsrcID,
			Hash<SkinItemLayerProperty>& mapProperty ) ;
	// アニメーション画像リソースを登録
	String AddAnimationResource
		( ImageContext& imgctx, String sType, String sRsrcID,
			Hash<SkinItemLayerProperty>& mapProperty ) ;
	// テキストスタイルを登録
	String AddTextStyle
		( ImageContext& imgctx, String sType, String sStyleID,
			Hash<SkinItemLayerProperty>& mapProperty ) ;
	// ボタンスタイルを登録
	String AddButtonStyle
		( ImageContext& imgctx, String sType, String sStyleID,
			Hash<SkinItemLayerProperty>& mapProperty ) ;
	// フレームスタイルを登録
	String AddFrameStyle
		( ImageContext& imgctx, String sType, String sStyleID,
			Hash<SkinItemLayerProperty>& mapProperty ) ;
	// 進捗バースタイルを登録
	String AddProgressBarStyle
		( ImageContext& imgctx, String sType, String sStyleID,
			Hash<SkinItemLayerProperty>& mapProperty ) ;
	// スクロールバースタイルを登録
	String AddScrollBarStyle
		( ImageContext& imgctx, String sType, String sStyleID,
			Hash<SkinItemLayerProperty>& mapProperty ) ;

public:
	// 画像アイテムを追加
	SkinItemStyle& AddImageItem
		( String sType, String sItemID, String sImageID,
			Hash<SkinItemLayerProperty>& mapProperty,
			const Point& ptItem, const Size& sizeItem ) ;
	// 矩形アイテムを追加
	SkinItemStyle& AddRectangleItem
		( String sType, String sItemID,
			Hash<SkinItemLayerProperty>& mapProperty,
			const Point& ptItem, const Size& sizeItem ) ;
	// テキストアイテムを追加
	SkinItemStyle& AddTextItem
		( String sType, String sItemID, String sStyleID,
			Hash<SkinItemLayerProperty>& mapProperty,
			const Point& ptItem, const Size& sizeItem ) ;
	// ボタンアイテムを追加
	SkinItemStyle& AddButtonItem
		( String sType, String sItemID, String sStyleID,
			Hash<SkinItemLayerProperty>& mapProperty,
			const Point& ptItem, const Size& sizeItem ) ;
	// フレームアイテムを追加
	SkinItemStyle& AddFrameItem
		( String sType, String sItemID, String sStyleID,
			Hash<SkinItemLayerProperty>& mapProperty,
			const Point& ptItem, const Size& sizeItem ) ;
	// 進捗バーアイテムを追加
	SkinItemStyle& AddProgressBarItem
		( String sType, String sItemID, String sStyleID,
			Hash<SkinItemLayerProperty>& mapProperty,
			const Point& ptItem, const Size& sizeItem ) ;
	// スクロールバーアイテムを追加
	SkinItemStyle& AddScrollBarItem
		( String sType, String sItemID, String sStyleID,
			Hash<SkinItemLayerProperty>& mapProperty,
			const Point& ptItem, const Size& sizeItem ) ;

} ;



