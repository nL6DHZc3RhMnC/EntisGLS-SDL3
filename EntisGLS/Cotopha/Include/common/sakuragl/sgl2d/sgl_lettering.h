
#if	!defined(__SAKURAGL_LETTERING_H__)
#define	__SAKURAGL_LETTERING_H__

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// レタリング
	//////////////////////////////////////////////////////////////////////////

	struct	SGLLetteringContext
	{
		enum	AlignmentType
		{
			alignLeft	= 0,		// 左揃え／上揃え
			alignRight,				// 右揃え／下揃え
			alignCenter,			// 中央揃え
			alignLong,				// 両端揃え
		} ;
		enum	VerticalWriting
		{
			writingHorizontal,
			writingVertical,
		} ;
		SGLPoint		ptStartWriting ;	// 描画開始位置
		SGLRect			rectWritable ;		// 描画可能域
		uint16_t		typeAlignment ;		// 整列方式 enum Alignment
		uint16_t		flagVertical ;		// 垂直整列 enum VerticalWriting
		int32_t			pitchChar ;			// 固定文字ピッチ (0 以外のときに使用)
		int32_t			offsetChar ;		// 文字ピッチ加算値
		int32_t			scalePitch ;		// 文字ピッチ比率（x10000H）
		int32_t			offsetInHalf ;		// 全角文字から半角文字の文字ピッチ加算
		int32_t			offsetOutHalf ;		// 半角文字から全角文字の文字ピッチ加算
		int32_t			pitchTab ;			// タブピッチ
		int32_t			pitchLine ;			// 行間 (0 のときはフォントサイズに依存)
		int32_t			widthIndent ;		// ２行目以降インデント
		uint32_t		minHyphening ;		// 最小ハイフニング長
		uint32_t		maxProhibition ;	// 最大禁則ぶら下げ長
		const wchar_t *	pwszProhibition ;	// 禁則文字

		static const wchar_t *	pwszDefProhibition ;

		// 構築関数
		SGLLetteringContext( void )
			: typeAlignment(alignLeft), flagVertical(writingHorizontal),
				pitchChar(0), offsetChar(0), scalePitch(0x10000),
				offsetInHalf(0), offsetOutHalf(0),
				pitchTab(0), pitchLine(0), widthIndent(0), minHyphening(7),
				maxProhibition(3), pwszProhibition(pwszDefProhibition) {}
		// シリアライズ（ポインタを除く）
		SSystem::SError SaveWithoutPointer( SSystem::SFileInterface& file ) const ;
		SSystem::SError LoadWithoutPointer( SSystem::SFileInterface& file ) ;
	} ;

	class	SGLLetterer	: public ESLObject
	{
	public:
		struct	Character
		{
			SGLImageBuffer *	pImage ;		// null for space
			SGLPoint			ptWriting ;		// 描画座標
			SGLPoint			ptOffset ;		// 画像描画座標
			SGLSize				sizeChar ;		// 文字ピッチ
			uint32_t			wchCode ;		// 文字コード
		} ;
		enum	DecorationFlag
		{
			flagShadow		= 0x0001,
			flagBorder		= 0x0002,
			flagBorder2		= 0x0004,
			flagGradation	= 0x0008,
		} ;
		struct	Decoration
		{
			uint32_t	nFlags ;		// enum DecorationFlag complex
			SGLPalette	rgbaBody ;		// 文字の色
			uint32_t	widthBorder ;	// 縁取りの幅
			SGLPalette	rgbaBorder ;	// 縁取りの色
			SGLPalette	rgbaShadow ;	// 影の色
			SGLPoint	ptShadow ;		// 影のオフセット
			uint32_t	widthBorder2 ;	// 縁取り２の幅
			SGLPalette	rgbaBorder2 ;	// 縁取り２の色
			const SGLPalette *
						pGradation ;
			uint32_t	nGradationCount ;
			uint32_t	nGradationHeight ;

			#if	!defined(__COTOPHA__)
			Decoration( void )
				: nFlags(0), widthBorder(0), widthBorder2(0),
					pGradation(NULL), nGradationCount(0), nGradationHeight(0) {}
			#endif
		} ;

	protected:
		SSystem::SArray<Character>				m_characters ;
		SSystem::SObjectArray<SGLImageObject>	m_aTempImageToDraw ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLLetterer, ESLObject )
		// 構築関数
		SGLLetterer( void ) ;
		// 消滅関数
		virtual ~SGLLetterer( void ) ;

	public:
		// レタリング
		virtual size_t WriteLetter
			( SGLFont& font,
				SGLLetteringContext& context,
						const wchar_t * pwszLetter ) ;
		// 文字装飾
		virtual SGLError DecorateLetter
			( const Decoration& deco,
				size_t iFirst = 0, ssize_t nCount = -1 ) ;
		// 画像結合
		virtual SGLError CombineLetter( void ) ;
		// 指定指標以降の全ての文字データを削除
		virtual void ClearLetter( size_t iFirst = 0 ) ;

	public:
		// 1文字ラスタライズ
		virtual SGLError RasterizeCharacter
			( SGLImageObject& imgChar,
				SGLPoint& ptOffset, int32_t& nCharWidth,
				SGLFont& font, const Decoration& deco, wchar_t wch ) ;
		// 文字装飾画像生成
		SGLError CreateDecoratedCharacterAt
			( Character& chChar, const Decoration& deco, size_t iChar ) ;

	protected:
		// 禁則文字判定
		static bool IsProhibitionChar
			( const wchar_t * pwszProhibition, wchar_t wchCode ) ;
		// アルファベット判定
		static bool IsAlphabetChar( wchar_t wchCode ) ;
		// 文字ピッチ取得
		static int32_t GetCharacterPitch
			( const SGLLetteringContext& context, const Character& chChar ) ;
		// 文字ラスタライズ
		virtual void RasterizeCharacter
				( SGLFont& font, Character& chChar,
					SSystem::SArray<uint8_t>& bufGray, wchar_t wchCode ) ;
		// 行アライメント
		virtual void DoLineAlignment
			( SGLLetteringContext& context,
				int typeAlign, size_t iFirst, size_t nCount ) ;
		// 文字装飾
		virtual void DoDecoration
				( const Decoration& deco, Character& chLetter ) ;

	protected:
		// Gray 画像を白色 ARGB 形式に変換
		virtual void NormalizeCharacterGrayToARGB( Character& chLetter ) ;
		// 文字色塗りつぶし
		virtual void FillLetterColor
				( const Decoration& deco, SGLImageBuffer& imgBuf,
					const SGLPoint& ptOffset, const SGLSize& sizeChar ) ;
		// 縁取り色塗りつぶし
		virtual void FillBorderColor
				( const Decoration& deco, SGLImageBuffer& imgBuf,
					const SGLPoint& ptOffset, const SGLSize& sizeChar ) ;
		virtual void FillBorder2Color
				( const Decoration& deco, SGLImageBuffer& imgBuf,
					const SGLPoint& ptOffset, const SGLSize& sizeChar ) ;
		// 影色塗りつぶし
		virtual void FillShadowColor
				( const Decoration& deco, SGLImageBuffer& imgBuf,
					const SGLPoint& ptOffset, const SGLSize& sizeChar ) ;

	public:
		// 文字列描画
		SGLError DrawLetterTo
			( SGLPaintContextInterface & paint,
							int xPos = 0, int yPos = 0 ) ;
		// 文字列取得
		const SSystem::SArray<Character>& GetLetter( void ) const
		{
			return	m_characters ;
		}
		// 文字数取得
		size_t GetLetterLength( void ) const
		{
			return	m_characters.GetLength() ;
		}
		// 文字取得
		Character * GetCharacterAt( size_t i ) const
		{
			return	m_characters.GetAt( i ) ;
		}

		// 外接矩形取得
		const SGLImageRect& GetLetterRect( SGLImageRect& rect ) const ;

	} ;

}

#endif
