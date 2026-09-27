
//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 実行イメージ・リンカ
//////////////////////////////////////////////////////////////////////////////

class	ECSExecutionImageLinker	: public	ECSExecutionImage
{
public:
	// 構築関数
	ECSExecutionImageLinker( void ) ;
	// 消滅関数
	virtual ~ECSExecutionImageLinker( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSExecutionImageLinker, ECSExecutionImage )

public:
	// エラー出力
	virtual void OutputError( const char * pszErrMsg ) ;

public:
	// マージフラグ
	enum	MergeFlag
	{
		mfOverwriteVariable	= 0x00000001,
		mfOverwriteFunction	= 0x00000002,
		mfOverwriteImage	= 0x00000003,
		mfMirrorClassInf	= 0x00010000
	} ;
	// 実行イメージを結合
	ESLError MergeImage
		( const ECSExecutionImage & image, DWORD dwFlags = 0 ) ;

protected:
	// コードをアライメント
	DWORD AlignCodeBuffer( int nAlign = 8 ) ;
	// 大域変数を結合
	ESLError MergeGlobalVariable
		( ECSGlobal & csgDst, const ECSGlobal & csgSrc, DWORD dwFlags = 0 ) ;
	// 変数参照リストを結合
	ESLError MergeCodeRef
		( ENumArray<DWORD> & lstDst,
			const ENumArray<DWORD> & lstSrc, DWORD dwImageBias ) ;
	ESLError MergeGlobalRef
		( ENumArray<DWORD> & lstDst, const ECSGlobal & varDst,
			const ENumArray<DWORD> & lstSrc, const ECSGlobal & varSrc,
			DWORD dwImageBias, DWORD dwGlobalBias, DWORD dwFlags = 0 ) ;
	ESLError MergeGlobalRefList
		( TaggedRefAddresList & impDst, const ECSGlobal & varDst,
			const TaggedRefAddresList & impSrc, const ECSGlobal & varSrc,
			DWORD dwImageBias, DWORD dwGlobalBias, DWORD dwFlags = 0 ) ;
	// naked データ領域を結合
	ESLError MergeNakedGlobalVariable
		( ECSBuffer & bufDst, const ECSBuffer & bufSrc, DWORD dwFlags = 0 ) ;
	// naked データ参照リストを結合
	ESLError MergeNakedGlobalRef
		( ENumArray<DWORD> & lstDst, const ENumArray<DWORD> & lstSrc,
			DWORD dwImageBias, DWORD dwGlobalBias, DWORD dwFlags = 0 ) ;
	ESLError MergeNakedGlobalRefList
		( TaggedRefAddresList & impDst,
			const TaggedRefAddresList & impSrc,
			DWORD dwImageBias, DWORD dwGlobalBias, DWORD dwFlags = 0 ) ;
	// インポート名関連リストを結合
	ESLError MergeImportNameRef
		( ECSStrBufTagArray & staDstName,
			ENumArray<DWORD> & lstDst, 
			const ECSStrBufTagArray & staSrcName,
			const ENumArray<DWORD> & lstSrc, DWORD dwImageBias ) ;
	ESLError MergeImportNameRef
		( SSystem::SIndexedArray
				<SSystem::SString,const wchar_t*> & staDstName,
			ENumArray<DWORD> & lstDst, 
			const SSystem::SIndexedArray
				<SSystem::SString,const wchar_t*> & staSrcName,
			const ENumArray<DWORD> & lstSrc, DWORD dwImageBias ) ;

public:
	// 使用していないクラス情報を削除する（naked only mode 用）
	ESLError TrimUnsedClassInfo( void ) ;
	// 未解決リンクを解決する
	ESLError SolveLinkInfo( bool fNoImport = false ) ;
	// リンク用データを削除
	void DeleteLinkerInfo( void ) ;
	// 未解決のリンクの総数を取得
	unsigned int GetUnsolvedLinkCount( void ) const ;
	// 未解決のリンク名を取得
	ECSWideString GetUnsolveLinkName( unsigned int nIndex ) const ;

protected:
	// naked クラスの初期値イメージ・仮想関数ベクタがなければ追加する
	ESLError CommitAllClassInitImage( void ) ;
	// クラス情報のメンバ関数アドレスを確定する
	ESLError CommitAllClassMethod( void ) ;
	// object シンボル参照解決
	ESLError SolveImportObjectSymbol
		( ENumArray<DWORD> & expList,
			TaggedRefAddresList & impRef,
			const ECSGlobal & csgGlobalType ) ;
	// naked シンボル参照解決
	ESLError SolveImportNakedSymbol
		( ENumArray<DWORD> & expList,
			TaggedRefAddresList & impRef ) ;
	// 関数参照解決
	ESLError SolveImportFunctionPointer
		( ENumArray<DWORD> & expList,
			TaggedRefAddresList & impRef, bool fRef64 ) ;


public:
	// コードの参照情報削除とコードのシフト
	void RemoveCodeImage( DWORD dwAddress, int nRange ) ;
	// コードの参照情報削除
	void RemoveCodeReferenceInfo( DWORD dwAddress, int nRange ) ;
	// コードのシフト操作
	//（特定アドレス以降への参照リンク情報をオフセットとコード自体のムーブ）
	void ShiftCodeImage( DWORD dwAddress, int nShiftOffset ) ;
protected:
	static void RemoveCodeReferenceAddressList
		( TaggedRefAddresList & listCodeRef, DWORD dwAddress, int nRange ) ;
	static void RemoveCodeReferenceAddress
		( ENumArray<DWORD> & extCodeRef, DWORD dwAddress, int nRange ) ;
	static void ShiftCodeReferenceAddressList
		( TaggedRefAddresList & listCodeRef, DWORD dwAddress, int nShiftOffset ) ;
	static void ShiftCodeReferenceAddress
		( ENumArray<DWORD> & extCodeRef, DWORD dwAddress, int nShiftOffset ) ;


	friend	ECSCompiler ;
	friend	ECSContext ;
} ;

