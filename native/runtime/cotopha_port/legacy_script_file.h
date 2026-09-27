#pragma once
// Ported from EntisGLS3, copyright (c) 2003-2013 Leshade Entis, Entis-soft.
class	ECSFile	: public	ECSObject
{
public:
	// 構築関数
	ECSFile( void ) ;
	// 消滅関数
	virtual ~ECSFile( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSFile, ECSObject )

protected:
	ESLFileObject *	m_pFile ;			// ファイルオブジェクト
	ECSString		m_strFileName ;		// ファイル名
	DWORD			m_dwOpenFlags ;		// ファイルモード
	EObjArray<ESLFileObject>
					m_stackFile ;		// ファイルインターフェース

public:
	ESLFileObject * GetFileInterface( void ) const
		{
			return	m_pFile ;
		}

protected:
	void BeginSaveAttempt();
	ESLError SaveStaged(ECSObject *object, ECSObject *title, bool saveContext,
	                    bool noCompress, ECSContext &context);
	ESLError CommitStaged(EMemoryFile &complete, UINT64 prefix);
	bool m_downloadFailed = false;
	int				m_nCharaEncoding ;	// 文字エンコーディング
										// EDescription::CharacterEncoding

public:
	struct	PLUGIN_OBJECT_HEADER
	{
		ECSFile *	pBackLink ;
	} ;
	struct	PLUGIN_FILE
		: public PLUGIN_OBJECT_HEADER, public ECS_FILE_INTERFACE { } ;
	PLUGIN_FILE *	m_ppif ;		// プラグイン用インターフェース

	static ECSFile *
		FileFromPlugin( ECS_FILE_INTERFACE * instance )
	{
		PLUGIN_FILE *	ppif = (PLUGIN_FILE*) instance ;
		ESLAssert( ppif->pBackLink->m_ppif == ppif ) ;
		return	ppif->pBackLink ;
	}
	enum OpenURLFlag
	{
		flagNoCacheURLAccess	= 0x0100,
	} ;

public:
	// ファイルを開く
	ESLError Open
		( const wchar_t * pwszFileName,
			DWORD dwOpenFlags, ECSContext * pContext = NULL ) ;
	// インターネット上のファイルを開く
	ESLError OpenURL
		( const wchar_t * pwszURL,
			const wchar_t * pwszDownloadFile,
			unsigned int nFlags, ECSEnvironment * pEnv ) ;
	// メモリファイルを生成する
	ESLError CreateMemoryFile( DWORD dwInitBufSize ) ;
	// ファイルを閉じる
	ESLError Close( void ) ;
	// アーカイブを開く
	ESLError OpenArchive( void ) ;
	// アーカイブを閉じる
	ESLError CloseArchive( void ) ;
	// アーカイブファイルを開く
	ESLError OpenArchiveFile
		( const char * pszFilePath,
			const char * pszPassword, bool fStream = false ) ;
	// アーカイブファイルを閉じる
	ESLError CloseArchiveFile( void ) ;
	// 文字エンコーディングを設定する
	ESLError SetCharacterEncoding( const char * pszType ) ;
	// 文字エンコーディングを取得する
	const char * GetCharacterEncoding( void ) const ;
	// ファイルのダウンロード状況を取得
	INT64 GetCurrentDownloaded( void ) const ;
	// ダウンロードファイルのファイル長を取得
	INT64 GetDownloadingFileLength( void ) const ;
	// ファイルのダウンロード完了を取得
	bool IsFileDownloaded( void ) const ;
	// ファイルのダウンロードが失敗しているか？
	bool IsFileDownloadFailed( void ) const ;
	// ファイルのダウンロードを中断
	void CancelFileDownloading( void ) const ;
	// ファイルから読み込む
	unsigned long int Read
		( void * ptrBuffer, unsigned long int nBytes ) ;
	// ファイルへ書き出す
	unsigned long int Write
		( const void * ptrBuffer, unsigned long int nBytes ) ;
	// ファイル長取得
	UINT64 GetFileLength( void ) const ;
	// ファイルポインタ取得
	UINT64 GetFilePosition( void ) const ;
	// ファイルポインタ移動
	UINT64 Seek( INT64 nPos, int nSeekType ) ;
	// EOF 判定
	bool IsEndOfFile( void ) const ;
	// EOF 設定
	void SetEndOfFile( void ) ;
	// 文字列の読み込み
	ESLError ReadText( ECSObject & obj ) ;
	// 文字列の書き出し
	unsigned long int WriteText( ECSObject & obj ) ;
	// バイナリの読み込み
	unsigned long int ReadBinary( ECSObject & obj, long int nBytes ) ;
	// バイナリの書き出し
	unsigned long int WriteBinary( ECSObject & obj, long int nBytes ) ;

protected:
	// セーブファイルの中のチャンクを開く
	static ESLError OpenSaveFile( EMCFile & emcfile, ESLFileObject & file ) ;
public:
	// セーブファイル見出しの読み込み
	ESLError LoadContextTitle( ECSObject *& pObj, ECSContext & context ) ;
	// オブジェクトの読み込み
	ESLError LoadObject( ECSObject *& pObj, ECSContext & context ) ;
	// コンテキストの読み込み
	ESLError LoadContext( ECSContext & context, bool fNoCompressed ) ;
	static ESLError LoadContext
		( ESLFileObject & file, bool fNoCompressed, ECSContext & context ) ;
	// セーブファイルサムネイル画像の書き出し
	ESLError SaveThumbnailImage
		( ECSSprite * pPreview, int nWidth, int nHeight ) ;
	// オブジェクトの書き出し
	ESLError SaveObject
		( ECSObject & obj, ECSObject * pTitle, ECSContext & context ) ;
	// コンテキストの読み込み
	ESLError SaveContext
		( ECSObject * pTitle, bool fNoCompress, ECSContext & context ) ;
	// オブジェクトをダンプする
	ESLError DumpObject( ECSObject & obj, ECSContext & context ) ;
	// コンテキストをダンプする
	ESLError DumpContext( ECSContext & context ) ;

public:		// 通常のオブジェクト処理
	// オブジェクトの型名を取得する
	virtual const wchar_t * GetTypeName( void ) const ;
	// オブジェクトを複製
	virtual ECSObject * Duplicate( void ) ;
	// オブジェクトを代入
	virtual ESLError Move( ECSContext & context, ECSObject * obj ) ;
	// 単項演算子
	virtual ESLError UnaryOperate
		( ECSContext & context, CSUnaryOperatorType csuopType ) ;
	// 二項演算子
	virtual ESLError Operate
		( ECSContext & context, CSOperatorType csopType, ECSObject * obj ) ;
	// 比較演算子
	virtual ESLError Compare
		( ECSContext & context, int & nResult,
			CSCompareType cscpType, ECSObject & obj ) ;
	// メンバ関数インデックス取得
	virtual ESLError GetFunction
		( ECSContext & context, int & nIndex, const wchar_t * pwszName ) ;
	// メンバ関数呼び出し
	virtual ESLError CallFunction
		( ECSContext & context,
			int nIndex, ECSObjArray<ECSObject> & lstArg ) ;
	// 特殊演算子 : boolean 判定
	virtual ESLError OperateBoolean( int & nBoolean ) ;
	// 特殊演算子 : sizeof
	virtual ESLError OperateSizeOf( INT64 & nSize ) ;
	// 整数値取得
	virtual ESLError OperateInteger( INT64 & nValue ) ;
	// 文字列取得
	virtual ESLError OperateString( EWideString & wstrValue ) ;
	// 内部バッファインターフェース
	virtual void * GetBuffer( int iOffset, int nSize, bool fWritable ) ;

public:		// シリアル化のための関数（システムによって必要）
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;

public:
	// 関数プロトタイプ
	typedef	ESLError (ECSFile::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// 関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[40] ;
	static const PFUNC_CALL	m_pfnCallFunc[39] ;
	// スクリプト関数
	ESLError Call_Open
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_OpenURL
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CreateMemoryFile
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Close
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_OpenArchive
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CloseArchive
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_OpenArchiveFile
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CloseArchiveFile
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetCharacterEncoding
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetCharacterEncoding
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsEndOfFile
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetEndOfFile
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetCurrentDownloaded
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetDownloadingFileLength
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsFileDownloaded
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsFileDownloadFailed
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CancelFileDownloading
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetLength
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetPosition
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Seek
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ReadText
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_WriteText
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Read
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Write
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetFileTime
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_LoadContextTitle
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_LoadObject
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_LoadContext
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SaveThumbnailImage
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SaveObject
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SaveContext
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_DumpObject
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_DumpContext
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsExisting
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Rename
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_FindFile
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_FindDirectory
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_FindFileDirectory
		( ECSContext & context,
			ECSObjArray<ECSObject> & lstArg, bool fDirectory ) ;
	ESLError Call_FilterFilePath
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

public:
	// プラグインインターフェースを取得する
	virtual void * GetObjectInterface( const wchar_t * pwszType ) ;

protected:
	static ESLError __stdcall PIC_Open
		( ECS_FILE_INTERFACE * instance,
			const wchar_t * pwszFileName,
			DWORD dwOpenFlags, ECS_CONTEXT * pContext ) ;
	static ESLError __stdcall PIC_Close( ECS_FILE_INTERFACE * instance ) ;
	static ESLError __stdcall PIC_SetCharacterEncoding
		( ECS_FILE_INTERFACE * instance, const char * pszType ) ;
	static const char * __stdcall PIC_GetCharacterEncoding
		( ECS_FILE_INTERFACE * instance ) ;
	static unsigned long int __stdcall PIC_GetFileLength
		( ECS_FILE_INTERFACE * instance ) ;
	static unsigned long int __stdcall PIC_GetFilePosition
		( ECS_FILE_INTERFACE * instance ) ;
	static unsigned long int __stdcall PIC_Seek
		( ECS_FILE_INTERFACE * instance, long int nPos, int nSeekType ) ;
	static int __stdcall PIC_IsEndOfFile( ECS_FILE_INTERFACE * instance ) ;
	static void __stdcall PIC_SetEndOfFile( ECS_FILE_INTERFACE * instance ) ;
	static ESLError __stdcall PIC_ReadText
		( ECS_FILE_INTERFACE * instance, ECS_OBJECT * pObj ) ;
	static unsigned long int __stdcall PIC_WriteText
		( ECS_FILE_INTERFACE * instance, ECS_OBJECT * pObj ) ;
	static unsigned long int __stdcall PIC_ReadBinary
		( ECS_FILE_INTERFACE * instance, ECS_OBJECT * pObj, long int nBytes ) ;
	static unsigned long int __stdcall PIC_WriteBinary
		( ECS_FILE_INTERFACE * instance, ECS_OBJECT * pObj, long int nBytes ) ;
	static ESLError __stdcall PIC_LoadContextTitle
		( ECS_FILE_INTERFACE * instance,
			ECS_OBJECT **pObj, ECS_CONTEXT * pContext ) ;
	static ESLError __stdcall PIC_LoadObject
		( ECS_FILE_INTERFACE * instance,
			ECS_OBJECT **pObj, ECS_CONTEXT * pContext ) ;
	static ESLError __stdcall PIC_LoadContext
		( ECS_FILE_INTERFACE * instance, ECS_CONTEXT * pContext ) ;
	static ESLError __stdcall PIC_SaveThumbnailImage
		( ECS_FILE_INTERFACE * instance,
			ECS_OBJECT * pPreview, int nWidth, int nHeight ) ;
	static ESLError __stdcall PIC_SaveObject
		( ECS_FILE_INTERFACE * instance,
			ECS_OBJECT * pObj, ECS_OBJECT * pTitle, ECS_CONTEXT * pContext ) ;
	static ESLError __stdcall PIC_SaveContext
		( ECS_FILE_INTERFACE * instance,
			ECS_OBJECT * pTitle, ECS_CONTEXT * pContext ) ;
	static ESLError __stdcall PIC_DumpObject
		( ECS_FILE_INTERFACE * instance,
			ECS_OBJECT * pObj, ECS_CONTEXT * pContext ) ;
	static ESLError __stdcall PIC_DumpContext
		( ECS_FILE_INTERFACE * instance, ECS_CONTEXT * pContext ) ;
	static ECS_FILE * __stdcall PIC_GetFile( ECS_FILE_INTERFACE * instance ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// スレッドオブジェクト
