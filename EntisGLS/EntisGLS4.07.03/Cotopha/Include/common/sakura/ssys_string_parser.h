
#if	!defined(__SAKURA2_STRING_PARSER_H__)
#define	__SAKURA2_STRING_PARSER_H__

namespace	SSystem
{
	class	SStringFormatSupplier ;

	//////////////////////////////////////////////////////////////////////////
	// 文字列パーサー
	//////////////////////////////////////////////////////////////////////////

	class	SStringParser	: public ESLObject, public SString
	{
	protected:
		const uint16_t *	m_pszText ;
		size_t				m_lenText ;
		size_t				m_index ;
		size_t				m_mark ;
		wchar_t *			m_pwszFilePath ;

		static uint32_t		m_maskPunctuation[4] ;
		static uint32_t		m_maskSpecialMark[4] ;

		static const wchar_t *const	s_pwszHiragana ;
		static const wchar_t *const	s_pwszKatakana ;
		static const wchar_t *const	s_pwszKomojiKana ;
		static const wchar_t *const	s_pwszVoicelessKana ;
		static const wchar_t *const	s_pwszVoicedKana ;
		static const wchar_t *const	s_pwszSemivoicedKana ;
		static const wchar_t *const	s_pwszAuxiliaryKana ;
		static const wchar_t *const	s_pwszPunctuation ;
		static const wchar_t *const	s_pwszParenthesis ;
		static const wchar_t *const	s_pwszJISAlphabet ;
		static const wchar_t *const	s_pwszJISNumber ;
		static const wchar_t		s_wchWideKana[0x40] ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( SSystem::SStringParser, ESLObject, SString )
		// 構築関数
		SStringParser( void ) ;
		SStringParser( const SStringParser& ss ) ;
		SStringParser( const SString& src ) ;
		#if	!defined(__COTOPHA__)
		SStringParser( const wchar_t * pszSrc, ssize_t nLength = -1 ) ;
		#endif
		#if	!defined(__WCHAR_EQU_UINT16__)
		SStringParser( const uint16_t * pszSrc, ssize_t nLength = -1 ) ;
		#endif
		SStringParser( const char * pszSrc, ssize_t nLength = -1 ) ;
		// 消滅関数
		~SStringParser( void ) ;

	public:
		// 文字列関連付け
		void AttachString( void ) ;
		void AttachString( const SStringParser& ss ) ;
		void AttachString( const SString& strSrc ) ;
		void AttachSubString
			( const SStringParser& ssSrc, size_t iFirst = 0, ssize_t iEnd = -1 ) ;
		void AttachSubString
			( const SString& strSrc, size_t iFirst = 0, ssize_t iEnd = -1 ) ;
		void AttachString
			( const uint16_t * pszSrc, ssize_t nLength = -1 ) ;
		// 文字列解放
		void ReleaseString( void ) ;
		// 文字列設定
		void SetString( const SString& src ) ;
		#if	!defined(__COTOPHA__)
		void SetString( const wchar_t * pszSrc, ssize_t nLength = -1 ) ;
		#endif
		#if	!defined(__WCHAR_EQU_UINT16__)
		void SetString( const uint16_t * pszSrc, ssize_t nLength = -1 ) ;
		#endif
		void SetString( const char * pszSrc, ssize_t nLength = -1 ) ;
		// 代入操作
		const SStringParser& operator = ( const SStringParser& ss ) ;
		const SStringParser& operator = ( const SString& src )
		{
			SetString( src ) ;
			return	*this ;
		}
		const SStringParser& operator = ( const wchar_t * pszSrc )
		{
			SetString( pszSrc ) ;
			return	*this ;
		}

	public:
		// ファイルから読み込み
		virtual SError LoadTextFile
			( const wchar_t * pwszFilePath,
				Charset::EncodingType encoding = Charset::encodingUnknown ) ;
		virtual SError ReadTextFile
			( SFileInterface& file,
				Charset::EncodingType encoding = Charset::encodingUnknown ) ;
		// ファイル名関連付け
		void SetFilePath( const wchar_t * pwszFilePath ) ;
		const wchar_t * GetFilePath( void ) const
		{
			return	m_pwszFilePath ;
		}

	public:
		// 文字列配列へのポインタを取得
		const uint16_t * GetArray( void ) const
		{
			return	m_pszText ;
		}
		const uint16_t * GetConstArray( void ) const
		{
			return	m_pszText ;
		}
		// 文字全長取得
		size_t GetLength( void ) const
		{
			return	m_lenText ;
		}
		// 指標操作
		size_t GetIndex( void ) const
		{
			return	m_index ;
		}
		size_t SeekIndex( size_t index )
		{
			if ( index > m_lenText )
				return	(m_index = m_lenText) ;
			else
				return	(m_index = index) ;
		}
		bool IsIndexOverflow( void ) const
		{
			return	(m_index >= m_lenText) ;
		}
		// 特定文字列を発見するまで指標を移動する（失敗時は指標は変化しない）
		bool SeekString( const wchar_t * pszString ) ;
		// 特定文字を発見するまで指標を移動する（失敗時は指標は変化しない）
		bool SeekAnyCharacters( const wchar_t * pszChars ) ;
		// 指標を記憶する
		virtual void MarkIndex( void ) ;
		// 指標を記憶位置に戻す
		virtual void SeekToMark( void ) ;
		// 指標の記憶を解除する
		virtual void ReleaseMark( void ) ;
		// 文字列を切り出す
		virtual SString SubString( size_t iStart, ssize_t nCount = -1 ) const ;
		// 記憶指標から現在の指標までの文字列を切り出す
		virtual SString SubStringFromMark( void ) ;
		// 指定指標から現在の指標までの文字列を切り出す
		virtual SString SubStringFrom( size_t iStart ) const ;
		// 行番号(1～)取得
		virtual size_t GetLineNumberOf
			( size_t index, size_t * pGetLineIndex = NULL ) const ;
		// 行情報取得
		//  iCharIndex: in 文字指標 (0～), out 行カラム (1～)
		//  strLine: out 行文字列（終端改行含む）
		//  返り値: 行番号 (1～)
		virtual size_t GetLineCharIndexOf
				( SString& strLine, size_t& iCharIndex ) const ;
		// 文字取得
		wchar_t CurrentCharacter( void ) const
		{
			if ( m_index >= m_lenText )
				return	0 ;
			return	m_pszText[m_index] ;
		}
		wchar_t OffsetAt( size_t iOffset ) const
		{
			if ( m_index + iOffset >= m_lenText )
				return	0 ;
			return	m_pszText[m_index + iOffset] ;
		}
		// 空白記号か？
		static bool IsCharacterSpace( wchar_t wch )
		{
			return	(wch <= L' ') || (wch == 0xA0) || (wch == 0xFEFF) ;
		}
		// 区切り記号か？
		static bool IsPunctuation( wchar_t wch )
		{
			return	!(wch & ~0x7F)
				&& (m_maskPunctuation[wch >> 5] & (1 << (wch & 0x1F))) ;
		}
		// 特殊区切り記号か？
		static bool IsSpecialMark( wchar_t wch )
		{
			return	!(wch & ~0x7F)
				&& (m_maskSpecialMark[wch >> 5] & (1 << (wch & 0x1F))) ;
		}

	public:
		// 平仮名判定
		static bool IsHiragana( wchar_t wch )
		{
			return	(wch >= 0x3041) && (wch <= 0x3096) ;
		}
		static const wchar_t * GetHiraganaChars( void )
		{
			return	s_pwszHiragana ;
		}
		// 片仮名判定
		static bool IsKatakana( wchar_t wch )
		{
			return	(wch >= 0x30A1) && (wch <= 0x30FA) ;
		}
		static const wchar_t * GetKatakanaChars( void )
		{
			return	s_pwszKatakana ;
		}
		// 全角 ASCII 文字判定
		static bool IsWideASCIIChar( wchar_t wch )
		{
			return	(wch >= 0xFF01) && (wch <= 0xFF5E) ;
		}
		// 半角仮名判定
		static bool IsHalfWidthKana( wchar_t wch )
		{
			return	(wch >= 0xFF61) && (wch <= 0xFF9F) ;
		}
		// 平仮名→片仮名（該当しない場合には元の文字を返す）
		static wchar_t HiraganaToKatakana( wchar_t wch ) ;
		// 片仮名→平仮名（該当しない場合には元の文字を返す）
		static wchar_t KatakanaToHiragana( wchar_t wch ) ;
		// 濁音判定（濁音の場合清音仮名、それ以外の場合 0）
		static wchar_t ClearVoicedKana( wchar_t wch ) ;
		// 半濁音判定（半濁音の場合清音仮名、それ以外の場合 0）
		static wchar_t ClearSemivoicedKana( wchar_t wch ) ;
		// 濁音仮名生成（合成不可能の場合には 0）
		static wchar_t MakeVoicedKana( wchar_t wch ) ;
		// 半濁音仮名生成（合成不可能の場合には 0）
		static wchar_t MakeSemivoicedKana( wchar_t wch ) ;
		// 全角文字（仮名は清音のみ）→半角文字（対応文字がないときは 0）
		static wchar_t ToHalfWidthChar( wchar_t wch ) ;
		// 半角文字→全角文字（対応文字がないときは 0）
		static wchar_t WideCharFromHalfWidth( wchar_t wch ) ;
		// 小文字仮名判定（ぁぃぅぇぉゃゅょ...）
		static bool IsSmallKana( wchar_t wch ) ;
		static const wchar_t * GetSmallKanaChars( void )
		{
			return	s_pwszKomojiKana ;
		}
		// 読み補助文字判定（長音・濁音・反復等）
		static bool IsAuxiliaryKana( wchar_t wch ) ;
		static const wchar_t * GetAuxiliaryChars( void )
		{
			return	s_pwszAuxiliaryKana ;
		}
		// 句読点判定（ASCII ,.:; 及び日本語句読点等）
		static bool IsJISXPunctuation( wchar_t wch ) ;
		static const wchar_t * GetJISXPunctuationChars( void )
		{
			return	s_pwszPunctuation ;
		}
		// 括弧判定（ASCII "'()<>[]{} 及び日本語各種括弧）
		static bool IsJISXParenthesis( wchar_t wch ) ;
		static const wchar_t * GetISXParenthesisChars( void )
		{
			return	s_pwszParenthesis ;
		}

	public:	// 文字列ストリーミング
		// トークン種別
		enum	TokenType
		{
			tokenInvalid	= -1,
			tokenNormal,
			tokenPunctuation,
			tokenSpecialMark,
		} ;
		// 制御フラグ
		enum	ControlFlag
		{
			ctrlNoEscInQuote	= 0x0001,	// '～' 内で \ 記号処理をしない
			ctrlNoEscInDQuote	= 0x0002,	// "～" 内で \ 記号処理をしない
			ctrlNoRadixPostfix	= 0x0004,	// 数値の末尾の進数指定は無視
			ctrlCStyleNumber	= 0x0008,	// 0から始まる数字を8進数とする
			ctrlEscCSVInDQuote	= 0x0010,	// "～" 内で "" を処理する
		} ;
		// 空白をスキップ（空白以外を発見したら true）
		virtual bool PassSpace( void ) ;
		// 次の行へ移動
		virtual uint32_t SeekToNextLine( void ) ;
		// 現在の文字列（空白で区切られた）を通過する
		virtual void PassString( void ) ;
		// 特定の文字まで通過する
		virtual void PassEnclosedString
			( wchar_t wchCloser, int nCtrlFlags = 0 ) ;
		// 現在のトークンを通過する
		virtual TokenType PassToken( void ) ;
		// 式の１項を通過する
		virtual void PassExpressionTerm( int nCtrlFlags = 0 ) ;
		// 式を通過する
		virtual void PassExpression
			( const wchar_t * pszCloses, int nCtrlFlags = 0 ) ;

	public:	// 文字列ストリーミング（取得と指標の移動）
		// 次の文字取得
		wchar_t GetCharacter( void )
		{
			if ( m_index >= m_lenText )
				return	0 ;
			return	m_pszText[m_index ++] ;
		}
		// 現在の行（残り）取得
		virtual uint32_t NextLine( SString& strLine ) ;
		virtual SString GetLine( uint32_t* pRetCode = NULL ) ;
		// 次の文字列（空白で区切られた）を取得
		virtual bool NextString( SString& strString ) ;
		virtual SString GetString( void ) ;
		// 次の文字列（特定の文字で区切られた）を取得
		virtual wchar_t NextEnclosedString
			( SString& strString,
				wchar_t wchCloser, int nCtrlFlags = 0 ) ;
		virtual SString GetEnclosedString
			( wchar_t wchCloser,
				int nCtrlFlags = 0, wchar_t * pGetClosed = NULL ) ;
		// 次のトークンを取得
		virtual TokenType NextToken( SString& strToken ) ;
		virtual SString GetToken( TokenType* pGetType = NULL ) ;
		// 次の文字列項（空白で区切られた｜引用符で囲まれた）を取得
		virtual bool NextStringTerm( SString& strTerm, int nCtrlFlags = 0 ) ;
		virtual SString GetStringTerm( int nCtrlFlags = 0 ) ;
		// 式の１項を取得
		virtual bool NextExpressionTerm
				( SString& strTerm, int nCtrlFlags = 0 ) ;
		virtual SString GetExpressionTerm( int nCtrlFlags = 0 ) ;
		// 式を取得
		virtual wchar_t NextExpression
			( SString& strExpr,
				const wchar_t * pszCloses, int nCtrlFlags = 0 ) ;
		virtual SString GetExpression
			( const wchar_t * pszCloses,
				int nCtrlFlags = 0, wchar_t* pGetClosed = NULL ) ;

	public:
		// 次に一致する文字を通過する
		virtual wchar_t HasToComeChar( const wchar_t * pszNext ) ;
		// 次に一致する文字列を通過する
		virtual bool HasToComeString( const wchar_t * pszNext ) ;
		virtual bool HasToComeNoCaseString( const wchar_t * pszNext ) ;
		// 次に一致するトークンを通過する
		virtual bool HasToComeToken( const wchar_t * pszNext ) ;

	public:
		// 数値タイプ
		enum	NumberType
		{
			numberInvalid		= -1,
			numberDefault		= 0,
			numberInteger		= 0,
			numberRadix2		= 2,		// ～B
			numberRadix8		= 8,		// ～O
			numberRadix10		= 10,		// ～T
			numberRadix16		= 16,		// ～H
			numberStyleOct		= 0x0108,	// 0～
			numberStyleHex		= 0x0110,	// 0x～
			numberRadixMask		= 0x00FF,
			numberFlagReal		= 0x0200,	// ～.～
		} ;
		// 数値文字列判定
		virtual int IsNextNumber( int nCtrlFlags = 0 ) ;
		// 整数解釈
		virtual int64_t NextInteger( int type = numberInteger ) ;
		virtual int64_t GetNextInteger( int& type, int nCtrlFlags = 0 ) ;
		// 実数解釈
		virtual double NextRealNumber( int type = numberDefault ) ;
		virtual double GetNextRealNumber( int& type, int nCtrlFlags = 0 ) ;
		// 整数配列解釈
		size_t ParseIntegerArray
			( int64_t * pNumbers, size_t nCount,
				int nCtrlFlags = 0, const wchar_t * pwszSeparator = NULL ) ;
		size_t ParseHexIntegerArray
			( int64_t * pNumbers, size_t nCount,
					const wchar_t * pwszSeparator = NULL ) ;
		// 実数配列解釈
		size_t ParseNumberArray
			( double * pNumbers, size_t nCount,
				int nCtrlFlags = 0, const wchar_t * pwszSeparator = NULL ) ;

	public:	// CSV (Comma-Separated Values) 書式
		// CSV ライン解釈（CSVダブルクオーテーションカラムのデコード処理）
		SError ParseCommaSeparatedValues
			( SObjectArray<SString>& aValues ) ;
		static SError ParseCommaSeparatedValues
			( SObjectArray<SString>& aValues, const SString& strLine ) ;
		static SError ParseCommaSeparatedValues
			( SObjectArray<SString>& aValues, const wchar_t * pwszStrLine ) ;
		// 文字列配列解釈
		SError SplitValuesOneLine
			( SObjectArray<SString>& aValues,
				int nCtrlFlags = ctrlEscCSVInDQuote,
				wchar_t wchSeparator = L',',
				const wchar_t * pwszQuotes = L"\"" ) ;

	public:
		// 16進数文字列のデコード
		static SError DecodeHexString
			( SArray<uint8_t>& buf,
				const wchar_t * pwszHexText, ssize_t nHexLength = -1 ) ;
		// 16進数文字列のエンコード
		static SError EncodeHexString
			( SString& strText, const uint8_t * pbytBin, size_t nBytes ) ;
		// base64 文字列のデコード
		static SError DecodeBase64String
			( SArray<uint8_t>& buf,
				const wchar_t * pwszBase64, ssize_t nLength = -1 ) ;
		// base64 文字列のエンコード
		static SError EncodeBase64String
			( SString& strText, const uint8_t * pbytBin, size_t nBytes ) ;
		// C 言語文字列エスケープシーケンス文字列のデコード
		static SError DecodeCLangString
			( SString& strText, const wchar_t * pwszText ) ;
		// C 言語文字列エスケープシーケンス文字列へエンコード
		static SError EncodeCLangString
			( SString& strText, const wchar_t * pwszText ) ;
		// CSV ダブルクオーテーション文字列のデコード
		static SError DecodeCSVString( SString& strText ) ;
		// CSV ダブルクオーテーション文字列へエンコード
		static SError EncodeCSVString
			( SString& strText,
				const wchar_t * pwszText, bool& flagNeedDQuote ) ;

	public:
		// printf 書式化
		virtual const wchar_t * Format
			( SString& strResult, SStringFormatSupplier& suppl ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 書式化パラメータ供給インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SStringFormatSupplier
	{
	public:
		// クラス情報
		ESL_DECLARE_NV_CLASS_INFO( SStringFormatSupplier )
		// 次の整数取得
		virtual int64_t NextInteger( void ) = 0 ;
		// 次の浮動小数点取得
		virtual double NextDouble( void ) = 0 ;
		// 次の文字取得
		virtual wchar_t NextCharacter( void ) = 0 ;
		// 次の文字列取得
		virtual const wchar_t * NextString( SString& strNext ) = 0 ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 標準的な書式化パラメータ
	//////////////////////////////////////////////////////////////////////////

	class	SStringFormatArgList	: public SStringFormatSupplier
	{
	protected:
		enum	Type
		{
			typeInteger,
			typeRealNumber,
			typeString,
		} ;
		class	Argument
		{
		public:
			Type	type ;
			int64_t	numInt ;
			double	numReal ;
			SString	varStr ;
		public:
			Argument( void ) : type(typeInteger), numInt(0), numReal(0.0) {}
			Argument( const Argument& arg )
				: type(arg.type), numInt(arg.numInt),
					numReal(arg.numReal), varStr(arg.varStr) {}
		} ;
		SObjectArray<Argument>	m_arg ;
		size_t					m_next ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SStringFormatArgList, SStringFormatSupplier )
		// 構築関数
		SStringFormatArgList( void ) ;
		// 消滅関数
		~SStringFormatArgList( void ) ;

	public:
		// 次の整数取得
		virtual int64_t NextInteger( void ) ;
		// 次の浮動小数点取得
		virtual double NextDouble( void ) ;
		// 次の文字取得
		virtual wchar_t NextCharacter( void ) ;
		// 次の文字列取得
		virtual const wchar_t * NextString( SString& strNext ) ;

	public:
		// 整数追加
		void AddInteger( int64_t num ) ;
		// 浮動小数点追加
		void AddDouble( double num ) ;
		// 次の文字取得
		void AddCharacter( wchar_t ch ) ;
		// 次の文字列取得
		void AddString( const wchar_t * str ) ;

	public:
		// 可変長引数を追加
		void AddArgumentList( const char * pszFormat, va_list argptr ) ;
	#if	!defined(__COTOPHA__)
		void AddArgumentList( const wchar_t * pwszFormat, va_list argptr ) ;
	#endif

	} ;


	//////////////////////////////////////////////////////////////////////////
	// パーサーエラー出力（基底）
	//////////////////////////////////////////////////////////////////////////

	class	SParserErrorInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_NV_CLASS_INFO( SParserErrorInterface )
		// エラー出力
		virtual void OutputError
			( const SStringParser& ss, const wchar_t * pszError ) ;
		// 警告出力
		virtual void OutputWarning
			( const SStringParser& ss, const wchar_t * pszWarning ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// パーサーエラー出力（デバッグ出力）
	//////////////////////////////////////////////////////////////////////////

	class	SParserErrorTracer	: public SParserErrorInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SParserErrorTracer, SParserErrorInterface )
		// エラー出力
		virtual void OutputError
			( const SStringParser& ss, const wchar_t * pszError ) ;
		// 警告出力
		virtual void OutputWarning
			( const SStringParser& ss, const wchar_t * pszWarning ) ;
	public:
		// トレース文字出力時見出し
		virtual const char * GetParserNameForTrace( void ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// パーサーエラー出力（ログ記録）
	//////////////////////////////////////////////////////////////////////////

	class	SParserErrorLogger	: public SParserErrorTracer
	{
	public:
		class	ErrorLog
		{
		public:
			SString	m_strFilePath ;
			size_t	m_nLineNum ;
			size_t	m_nColNum ;
			SString	m_strLine ;
			SString	m_strError ;
			void *	m_pUser ;

		public:
			ErrorLog( void )
				: m_nLineNum(0), m_nColNum(0), m_pUser(NULL) {}
			ErrorLog( const ErrorLog& el )
				: m_strFilePath(el.m_strFilePath),
					m_nLineNum(el.m_nLineNum),
					m_nColNum(el.m_nColNum),
					m_strLine(el.m_strLine),
					m_strError(el.m_strError), m_pUser(el.m_pUser) {}
		} ;

	protected:
		SObjectArray<ErrorLog>	m_logErrors ;
		SObjectArray<ErrorLog>	m_logWarnings ;
		void *					m_pUserContext ;
		bool					m_flagTraceError ;
		bool					m_flagTraceWarning ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SParserErrorLogger, SParserErrorTracer )
		// 構築関数
		SParserErrorLogger( void ) ;
		// 消滅関数
		virtual ~SParserErrorLogger( void ) ;

	public:
		// デバッグ出力有効／無効化
		void EnableDebugTrace
			( bool fError = true, bool fWarning = true ) ;
		// ユーザーコンテキスト設定
		void AttachErrorLogContext( void * pUser ) ;
		// ユーザーコンテキスト取得
		void * GetErrorLogContext( void ) const ;
		// エラー数取得
		size_t GetErrorCount( void ) const ;
		// 警告数取得
		size_t GetWarningCount( void ) const ;
		// エラーログ取得
		ErrorLog * GetErrorLogAt( size_t iError ) const ;
		// 警告ログ取得
		ErrorLog * GetWarningLogAt( size_t iWarning ) const ;
		// エラーログ追加
		void AddErrorLog( ErrorLog * pErrLog ) ;
		// 警告ログ追加
		void AddWarningLog( ErrorLog * pErrLog ) ;
		// ログの全消去
		void ClearAll( void ) ;

	public:
		// エラー出力
		virtual void OutputError
			( const SStringParser& ss, const wchar_t * pszError ) ;
		// 警告出力
		virtual void OutputWarning
			( const SStringParser& ss, const wchar_t * pszWarning ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 構文表現
	//////////////////////////////////////////////////////////////////////////

	class	SUsageMatcher	: public ESLObject
	{
	public:
		// 書式構文の構造体
		enum	UsageType
		{
			utToken,				// トークン
			utString,				// 文字列
			utSpace,				// 任意の空白
			utEnclosed,				// 指定文字コードで閉じられた任意文字列
			utWildCardToken,		// 任意のトークン
			utWildCardUsage,		// 任意の文字列
			utWildCardExpression,	// 任意の式
			utNumberUsage,			// 10進数文字列
			utEndOfUsage,			// 文字列の終端
			utCharacters,			// 指定の文字
			utRepeat,				// 反復
			utBeginParam,			// パラメータの開始
			utEndParam,				// パラメータの終了
			utOmittable,			// 省略可能
			utSelectable,			// 書式の選択
			utList,
		} ;
		enum	UsageFlag
		{
			ufNoCase	= 0x0001,	// 大文字小文字を区別しない
			ufNegative	= 0x0002,	// 判定論理の否定
			ufRepeat	= 0x0004,	// 繰り返し
		} ;
		class	Usage
		{
		public:
			UsageType			m_type ;
			uint32_t			m_flags ;		// enum UsageFlag の組み合わせ
			Usage *				m_parent ;		// 親
			SString				m_string ;		// 書式文字列
			uint32_t			m_mask[4] ;		// 7bit 文字の判定マスク
			SObjectArray<Usage>	m_elements ;	// 子
		public:
			// 構築関数
			Usage( UsageType type = utList, Usage * pParent = NULL ) ;
			Usage( const Usage& src ) ;
			// 消滅関数
			~Usage( void ) ;
			// パラメータ数カウント
			size_t GetUsageParamCount( void ) const ;
			// 子指標取得
			ssize_t GetChildIndex( Usage * pUsage ) const ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SUsageMatcher, ESLObject )
		// 構築関数
		SUsageMatcher( void ) ;
		// 消滅関数
		virtual ~SUsageMatcher( void ) ;

	protected:
		SSmartPointer<Usage>	m_pUsage ;

	public:
		// 書式判定
		static SError Match
			( const wchar_t * pwszTarget,
				const wchar_t * pwszUsage,
				SObjectArray<SString> * parrParam = NULL,
				SParserErrorInterface * perr = NULL ) ;

	public:
		// 書式判定
		virtual SError IsMatched
			( SStringParser & sparsTarget,
				const wchar_t * pwszUsage,
				SObjectArray<SString> * parrParam = NULL,
				SParserErrorInterface * perr = NULL ) ;
		// 書式検索
		virtual ssize_t FindMatched
			( SStringParser & sparsTarget,
				const wchar_t * pwszUsage,
				UsageType utWildCardType = utWildCardToken,
				SParserErrorInterface * perr = NULL ) ;

	public:
		// 書式の解釈
		virtual SError ParseUsage
			( const wchar_t * pwszUsage,
				SParserErrorInterface& perr ) ;
		// 書式の解放
		virtual void ReleaseUsage( void ) ;
		// 構文の解釈
		virtual SError IsMatchedWith
			( SStringParser & sparsTarget,
				SObjectArray<SString> * parrParam,
				SParserErrorInterface & perr ) const ;
		// 書式検索
		virtual ssize_t FindMatchedWith
			( SStringParser & sparsTarget,
				UsageType utWildCardType = utWildCardToken ) const ;

	protected:
		// 書式の解釈
		static SError ParseUsageFor
			( Usage * pParent,
				SStringParser & sparsUsage,
				SParserErrorInterface& perr ) ;
		// 書式の一致判定
		virtual SError IsMatchedWithAUsage
			( SStringParser & sparsTarget,
				Usage * pUsage,
				SObjectArray<SString> * parrParam,
				SParserErrorInterface & perr ) const ;
		virtual SError IsMatchedWithUsageList
			( SStringParser & sparsTarget,
				Usage & usage, size_t iStart,
				SObjectArray<SString> * parrParam,
				SParserErrorInterface & perr ) const ;
		// 一致書式を見つける
		virtual ssize_t FindMatchedWithUsageList
			( SStringParser & sparsTarget,
				Usage & usage, size_t iStart,
				UsageType utWildCardType ) const ;
		virtual ssize_t FindMatchedWithAUsage
			( SStringParser & sparsTarget,
				Usage * pParent, size_t nIndex,
				Usage * pUsage, UsageType utWildCardType ) const ;

	} ;

}


#endif
