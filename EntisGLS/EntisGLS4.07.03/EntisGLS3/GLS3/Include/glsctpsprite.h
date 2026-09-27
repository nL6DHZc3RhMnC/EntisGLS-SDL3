
/*****************************************************************************
               Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
   Copyright (C) 2003-2013 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#if	!defined(__GLSCTPSPRITE_H__)
#define	__GLSCTPSPRITE_H__


//////////////////////////////////////////////////////////////////////////////
// リソーススクリプトインターフェース
//////////////////////////////////////////////////////////////////////////////

class	ECSResource	: public ECSObject, public EGLSThread
{
public:
	// 構築関数
	ECSResource( ESLObject * pRsrc = NULL ) ;
	// 消滅関数
	virtual ~ECSResource( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO2( ECSResource, ECSObject, EGLSThread )

public:
	enum	PlayTypeFlag
	{
		ptfDevice	= 0x80000000,
		ptfNothing	= -1,
		ptfMusic,	ptfSound,	ptfVoice,	ptfSystem,
		ptfMovie,	ptfUser0,	ptfUser1,	ptfUser2,	ptfUser3,
		ptfMax,
	} ;
	enum	ResourceOwnFlag
	{
		rofNothing	= 0,
		rofImage,	rofSound,	rofMidi
	} ;
	class	ESoundResource	: public	MIOSoundStream
	{
	public:
		ESLFileObject *	m_pOwnFile ;
	public:
		// 構築関数
		ESoundResource( void ) ;
		// 消滅関数
		virtual ~ESoundResource( void ) ;
		// クラス情報
		DECLARE_CLASS_INFO( ESoundResource, MIOSoundStream )
	} ;

protected:
	ResourceOwnFlag	m_fOwnRsrc ;		// リソース所有フラグ
	int				m_nPlayType ;		// 再生タイプ
	ESLObject *		m_pRsrc ;			// リソース

	REAL32			m_rVolume[2] ;		// 音量
	static REAL32	m_rTotalVol[ptfMax] ;

	bool			m_fEnvelope ;
	HANDLE			m_hThreadReady ;
	EBezierCurves<E3D_VECTOR_2D>
					m_bzVolume ;

	// セーブ用
	ECSWideString	m_wstrFileName ;	// ファイル名
	unsigned int	m_nThreshold ;		// 音声リソースの読み込み閾値
	unsigned int	m_nRewindPos ;		// 再生中の場合、ループポイント
	unsigned int	m_nStartPos ;		// 再生開始位置
	unsigned int	m_nEndPos ;			// 再生終了位置
	unsigned int	m_nRepeatPlaying ;	// ループ再生フラグ
	ECSReference	m_refAttachSound ;	// 関連付けている音声

	enum
	{
		MAX_INNER_MEMBER_COUNT = 1,
	} ;

public:
	struct	PLUGIN_OBJECT_HEADER
	{
		ECSResource *	pBackLink ;
	} ;
	struct	PLUGIN_RESOURCE
		: public PLUGIN_OBJECT_HEADER, public ECS_RESOURCE_INTERFACE { } ;
	PLUGIN_RESOURCE *	m_ppir;		// プラグイン用インターフェース

	static ECSResource * ResourceFromPlugin( ECS_RESOURCE_INTERFACE * instance )
	{
		PLUGIN_RESOURCE *	ppir = (PLUGIN_RESOURCE*) instance ;
		ESLAssert( ppir->pBackLink->m_ppir == ppir ) ;
		return	ppir->pBackLink ;
	}

public:
	// 大域情報
	static EWaveMixingServer *
					m_pWaveDev ;		// 音声出力デバイス
	static EPtrObjArray<ECSResource> *
					m_plstPlayRsrc ;	// 再生しているリソース

public:
	// 画像リソースを読み込む
	virtual ESLError LoadImageFile
		( const wchar_t * pwszFilePath, ECSContext * pContext = NULL ) ;
	virtual ESLError ReadImageFile( ESLFileObject & file ) ;
	// 音声リソースを読み込む
	virtual ESLError LoadSoundFile
		( const wchar_t * pwszFilePath,
			unsigned int nThreshold, ECSContext * pContext = NULL ) ;
	virtual ESLError ReadSoundFile
		( ESLFileObject & file, unsigned int nThreshold ) ;
	// MIDI リソースを読み込む
	virtual ESLError LoadMidiFile
		( const wchar_t * pwszFilePath, ECSContext * pContext = NULL ) ;
	virtual ESLError ReadMidiFile( ESLFileObject & file ) ;
	// 音声関連付け
	virtual ESLError AttachSound( ECSResource * pRsrc ) ;
	// リソースを設定する
	virtual ESLError SetResource
		( ESLObject * pRsrc,
			ResourceOwnFlag rofType, const wchar_t * pwszFileName ) ;
	// リソースを解放する
	virtual ESLError Release( void ) ;
	// 画像リソースを保存する
	ESLError SaveImageFile
		( const wchar_t * pwszFilePath,
			const wchar_t * pwszMimeType,
			int nQuality, ECSContext * pContext = NULL ) const ;
	ESLError WriteImageFile
		( ESLFileObject & file, const wchar_t * pwszMimeType, int nQuality ) const ;

public:
	// リソースを取得
	ESLObject * GetResource( void ) const
		{
			return	m_pRsrc ;
		}
	EGLAnimation * GetImage( void ) const
		{
			return	ESLTypeCast<EGLAnimation>( m_pRsrc ) ;
		}
	EWaveSound * GetSound( void ) const
		{
			return	ESLTypeCast<EWaveSound>( m_pRsrc ) ;
		}
	EMidiMusic * GetMidiMusic( void ) const
		{
			return	ESLTypeCast<EMidiMusic>( m_pRsrc ) ;
		}
	virtual PEGL_IMAGE_INFO GetImageInfo( void ) const ;
	// リソースの再生タイプを取得
	int GetPlayTypeFlag( void ) const
		{
			return	m_nPlayType ;
		}
	// 音声出力デバイスを取得
	static EWaveMixingServer * GetWaveOutDevice( void )
		{
			return	m_pWaveDev ;
		}
	// 音声出力デバイスを設定
	static void SetWaveOutDevice( EWaveMixingServer * pWaveDev )
		{
			m_pWaveDev = pWaveDev ;
		}

public:
	// ファイルを開く
	virtual ESLFileObject * OpenResourceFile
		( const wchar_t * pwszFilePath, ECSContext * pContext ) ;

public:
	// 音声リソースを再生する
	virtual ESLError Play( unsigned int nIntroSamples, int fPlayType ) ;
	virtual ESLError PlayFrom
		( unsigned int nStartPos = 0, unsigned int nPlayEnd = -1,
				bool fRepeat = false,
				unsigned int nRewindPos = -1, int fPlayType = ptfMusic ) ;
	// 再生ループポイント設定
	virtual ESLError SetRewindingPortion
		( unsigned int nRewindPos = -1,
			unsigned int nEndPos = -1, bool fRepeat = true ) ;
	// 音声リソースの再生を停止する
	virtual ESLError Stop( void ) ;
	// 音声リソースの再生を一時停止する
	virtual ESLError Pause( void ) ;
	// 音声リソースの再生を再開する
	virtual ESLError Restart( void ) ;
	// 音量を取得する
	virtual ESLError GetVolume( REAL32 & rLeftVol, REAL32 & rRightVol ) ;
	// 音量を設定する
	virtual ESLError SetVolume( REAL32 rLeftVol, REAL32 rRightVol ) ;
	// 再生中か調べる
	virtual bool IsPlaying( void ) ;
	// 再生中の位置を取得する
	UINT64 GetPlayingPosition( void ) ;
	// 音量エンベロープを設定する
	ESLError SetVolumeEnvelope
		( const EBezierCurves<E3D_VECTOR_2D> & bezier,
							unsigned int nDurationTime ) ;
	// 音量エンベロープをキャンセルする
	void CancelVolumeEnvelope( void ) ;
	// 音量エンベロープ実行中か調べる
	bool IsPendingEnvelope( void ) ;
	// 全体音量を取得する
	static REAL32 GetTotalVolume( int fPlayType ) ;
	// 全体音量を設定する
	static ESLError SetTotalVolume( int fPlayType, REAL32 rVolume ) ;

protected:
	// スレッド関数
	virtual DWORD ThreadProc( void ) ;
	// スレッドメッセージ処理
	virtual void DispatchMessage( const MSG & msg ) ;

public:		// 通常のオブジェクト処理
	// オブジェクトの型名を取得する
	virtual const wchar_t * GetTypeName( void ) const ;
	virtual ECSObject * GetTypeOf( const wchar_t * pwszTypeName ) ;
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
	// メンバ変数取得
	virtual ECSObject * GetVariableAt( int nIndex ) ;
	// メンバ変数設定
	virtual ECSObject * SetVariableAt( int nIndex, ECSObject * obj ) ;
	// メンバ関数インデックス取得
	virtual ESLError GetFunction
		( ECSContext & context, int & nIndex, const wchar_t * pwszName ) ;
	// メンバ関数呼び出し
	virtual ESLError CallFunction
		( ECSContext & context,
			int nIndex, ECSObjArray<ECSObject> & lstArg ) ;

public:		// シリアル化のための関数（システムによって必要）
	// 全てのメンバ変数にインデックスを振る
	virtual void IndexAllMember( void ) ;
	// 全てのメンバ変数の参照を解消する
	virtual void CleanupAllReference( ECSContext & context ) ;
	// 全てのメンバ変数の参照を解決する
	virtual ESLError CommitAllReference( ECSContext & context ) ;
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;

public:
	// メンバ関数プロトタイプ
	typedef	ESLError (ECSResource::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[29] ;
	static const PFUNC_CALL	m_pfnCallFunc[28] ;
	// メンバ関数
	ESLError Call_LoadImage
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_LoadSound
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_LoadMidi
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SaveImage
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Release
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_AttachSound
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Play
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_PlayFrom
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetRewindingPortion
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Stop
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Pause
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Restart
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetVolume
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetVolume
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetTotalVolume
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetTotalVolume
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsPlaying
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetPlayingPosition
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetInfo
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetImageInfo
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetSoundInfo
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetVolumeEnvelope
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CancelVolumeEnvelope
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsPendingEnvelope
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetPixel
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetPixelRect
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetPixelRect
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetWaveData
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

public:
	// プラグインインターフェースを取得する
	virtual void * GetObjectInterface( const wchar_t * pwszType ) ;

protected:
	static ESLError __stdcall PIC_LoadImageFile
		( ECS_RESOURCE_INTERFACE * instance,
			const wchar_t * pwszFilePath, ECS_CONTEXT * pContext = NULL ) ;
	static ESLError __stdcall PIC_LoadSoundFile
		( ECS_RESOURCE_INTERFACE * instance,
			const wchar_t * pwszFilePath,
			unsigned int nThreshold, ECS_CONTEXT * pContext = NULL ) ;
	static ESLError __stdcall PIC_LoadMidiFile
		( ECS_RESOURCE_INTERFACE * instance,
			const wchar_t * pwszFilePath, ECS_CONTEXT * pContext = NULL ) ;
	static ESLError __stdcall PIC_AttachSound
		( ECS_RESOURCE_INTERFACE * instance, ECS_RESOURCE_INTERFACE * pRsrc ) ;
	static void __stdcall PIC_Release
		( ECS_RESOURCE_INTERFACE * instance ) ;
	static ESLError __stdcall PIC_PlayFrom
		( ECS_RESOURCE_INTERFACE * instance,
			unsigned int nStartPos, unsigned int nEndPos,
			int fRepeat, unsigned int nRewindPos, int fPlayType ) ;
	static ESLError __stdcall PIC_Stop
		( ECS_RESOURCE_INTERFACE * instance ) ;
	static ESLError __stdcall PIC_Pause
		( ECS_RESOURCE_INTERFACE * instance ) ;
	static ESLError __stdcall PIC_Restart
		( ECS_RESOURCE_INTERFACE * instance ) ;
	static ESLError __stdcall PIC_GetVolume
		( ECS_RESOURCE_INTERFACE * instance,
			REAL32 & rLeftVol, REAL32 & rRightVol ) ;
	static ESLError __stdcall PIC_SetVolume
		( ECS_RESOURCE_INTERFACE * instance,
			REAL32 rLeftVol, REAL32 rRightVol ) ;
	static int __stdcall PIC_IsPlaying
		( ECS_RESOURCE_INTERFACE * instance ) ;
	static ESLError __stdcall PIC_SetVolumeEnvelope
		( ECS_RESOURCE_INTERFACE * instance,
			const E3D_VECTOR_2D * bezier, unsigned int nDurationTime ) ;
	static void __stdcall PIC_CancelVolumeEnvelope
		( ECS_RESOURCE_INTERFACE * instance ) ;
	static int __stdcall PIC_IsPendingEnvelope
		( ECS_RESOURCE_INTERFACE * instance ) ;
	static REAL32 __stdcall PIC_GetTotalVolume
		( ECS_RESOURCE_INTERFACE * instance, int fPlayType ) ;
	static ESLError __stdcall PIC_SetTotalVolume
		( ECS_RESOURCE_INTERFACE * instance, int fPlayType, REAL32 rVolume ) ;
	static ESLError __stdcall PIC_SetResource
		( ECS_RESOURCE_INTERFACE * instance, ESLObject * pRsrc,
		ECS_RESOURCE_INTERFACE::ResourceOwnFlag rofType,
							const wchar_t * pwszFileName ) ;
	static ESLObject * __stdcall PIC_GetResource
		( ECS_RESOURCE_INTERFACE * instance ) ;

	friend	ECSResourceManager ;
} ;


//////////////////////////////////////////////////////////////////////////////
// リソース管理スクリプトインターフェース
//////////////////////////////////////////////////////////////////////////////

class	ECSResourceManager	: public ECSGlobal, public EFormResourceManager
{
public:
	// 構築関数
	ECSResourceManager( void ) ;
	// 消滅関数
	virtual ~ECSResourceManager( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO2( ECSResourceManager, ECSGlobal, EFormResourceManager )

protected:
	ECSWideString	m_wstrFileName ;		// スキンファイル名

public:
	// スキンファイルを読み込む
	ESLError LoadSkinFile
		( const wchar_t * pwszFileName, ECSContext * pContext = NULL ) ;
	// ページフォームを読み込んでページを作成する
	ESLError CreateFormPage
		( ECSSprite & siPage, const wchar_t * pwszPageID ) ;
	// リソース追加
	virtual ESLError AddResource( const wchar_t * pwszID, ESLObject * pRes ) ;
	// 内容削除
	virtual void Release( void ) ;
	// 内容削除
	virtual void DeleteContents( void ) ;

public:
	// ファイルを開く
	virtual ESLFileObject * OpenResourceFile
		( const wchar_t * pwszFilePath, ECSContext * pContext ) ;

public:		// 通常のオブジェクト処理
	// オブジェクトの型名を取得する
	virtual const wchar_t * GetTypeName( void ) const ;
	// オブジェクトを複製
	virtual ECSObject * Duplicate( void ) ;
	// メンバ関数インデックス取得
	virtual ESLError GetFunction
		( ECSContext & context, int & nIndex, const wchar_t * pwszName ) ;
	// メンバ関数呼び出し
	virtual ESLError CallFunction
		( ECSContext & context,
			int nIndex, ECSObjArray<ECSObject> & lstArg ) ;

public:		// シリアル化のための関数（システムによって必要）
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;

public:
	// メンバ関数プロトタイプ
	typedef	ESLError (ECSResourceManager::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[4] ;
	static const PFUNC_CALL	m_pfnCallFunc[3] ;
	// メンバ関数
	ESLError Call_LoadResource
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_DeleteContents
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CreateFormPage
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// スプライトフィルター
//////////////////////////////////////////////////////////////////////////////

class	ECSToneFilter	: public	ECSObject
{
public:
	// 構築関数
	ECSToneFilter( void ) ;
	// 消滅関数
	virtual ~ECSToneFilter( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSToneFilter, ECSObject )

public:
	enum	ToneFilterFlag
	{
		tffFixZero			= 0x0001,
		tffMaskWithAlpha	= 0x0002,
		tffYUVFilter		= 0x0004,
		tffGrayFilter		= 0x0008,
	} ;
	DWORD	m_dwFlags ;
	BYTE	m_bytBlue[0x100] ;
	BYTE	m_bytGreen[0x100] ;
	BYTE	m_bytRed[0x100] ;
	BYTE	m_bytAlpha[0x100] ;

protected:
	struct	ParallelFilterData
	{
		EGL_IMAGE_INFO	eiiBlock ;
	} ;
	class	ParallelFilterProc	: public SSystem::SParallelProcedure
	{
	protected:
		DWORD			m_dwFlags ;
		BYTE *			m_bytBlue ;
		BYTE *			m_bytGreen ;
		BYTE *			m_bytRed ;
		BYTE *			m_bytAlpha ;
		PEGL_IMAGE_INFO	m_pImage ;
		DWORD			m_dwStepLines ;
		DWORD			m_dwNextLine ;
	public:
		// 構築関数
		ParallelFilterProc
			( ECSToneFilter& filter, PEGL_IMAGE_INFO pImage ) ;
		// ループ処理／終了判定関数
		virtual bool Continue( void * pInstance ) ;
		// 並列処理関数
		virtual void RunParallel( void * pInstance ) ;
	} ;
	friend class ParallelFilterProc ;

public:
	// フィルタファイルを読み込む
	virtual ESLError LoadFilterFile
		( const wchar_t * pwszFileName, ECSContext * pContext = NULL ) ;
	// トーンテーブルを取得
	virtual void GetToneTables
		( void * pRed, void * pGreen,
			void * pBlue, void * pAlpha ) ;
	// 複雑なフィルタを設定
	virtual void SetGeneralTone
		( int nRedTone, int nRedFlag,
			int nGreenTone, int nGreenFlag,
			int nBlueTone, int nBlueFlag,
			int nAlphaTone, int nAlphaFlag, DWORD dwFlags = 0 ) ;
	// トーンテーブルを設定
	virtual void SetToneTables
		( const void * pRed, const void * pGreen,
			const void * pBlue, const void * pAlpha, DWORD dwFlags = 0 ) ;
	// フィルタ補完
	virtual void MorphingFilter
		( const ECSToneFilter & filter1,
			const ECSToneFilter & filter2, unsigned int nDegree ) ;
	// 代入演算子
	const ECSToneFilter & operator = ( const ECSToneFilter & filter ) ;
	// トーンフィルタ処理
	virtual void ApplyToneFilter( PEGL_IMAGE_INFO pImage ) ;

public:
	// フィルタファイルを開く
	virtual ESLFileObject * OpenFilterFile
		( const wchar_t * pwszFileName, ECSContext * pContext ) ;

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

public:		// シリアル化のための関数（システムによって必要）
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;

public:
	// メンバ関数プロトタイプ
	typedef	ESLError (ECSToneFilter::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[4] ;
	static const PFUNC_CALL	m_pfnCallFunc[3] ;
	// メンバ関数
	ESLError Call_LoadFilterFile
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetGeneralTone
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_MorphingFilter
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// スクリプトインターフェース＋スプライトインターフェース
//////////////////////////////////////////////////////////////////////////////

class	ECSSprite	: public ECSResource, public EAnimationSprite
{
public:
	// 構築関数
	ECSSprite( void ) ;
	// 消滅関数
	virtual ~ECSSprite( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO2( ECSSprite, ECSResource, EAnimationSprite )

public:
	struct	DRAW_TEXT_PARAM
	{
		EGL_RECT		rcArea ;			// 描画有効域
		EGL_POINT		ptCurPos ;			// 描画座標
		unsigned int	nFlags ;			// 機能フラグ
		EGL_PALETTE		rgbColor ;			// 文字色
		unsigned int	nTransparency ;		// 透明度
		unsigned int	nLineHeight ;		// 行間
		unsigned int	nIndentWidth ;		// インデント幅
		unsigned int	nFontSize ;			// フォントサイズ
		const wchar_t *	pwszFontFace ;		// フォントフェース名
	} ;
	enum	DrawTextFlag
	{
		DTPF_VERTICAL		= 0x0001,	// 縦書き
		DTPF_NOSMOOTHING	= 0x0002,	// アンチエイリアス無効化
		DTPF_LEFT			= 0x0000,	// 左寄せ（デフォルト）
		DTPF_CENTER			= 0x0010,	// 中央寄せ（単一行）
		DTPF_RIGHT			= 0x0020,	// 右寄せ（単一行）
		DTPF_ACCORDING		= 0x0030,	// 左右の幅調整（単一行）
		DTPF_ALIGN_MASK		= 0x0030
	} ;
	enum	ActionType
	{
		actNormal,	actLoop, actTurnLoop,
	} ;

protected:
	EGLImage			m_imgBlendAlpha ;	// 合成マスク
	unsigned int		m_nAlphaRange ;		// アルファチャネル幅
	SDWORD				m_nBlendDegree ;	// 合成度合い

	// フェード処理パラメータ
	bool				m_fEnableFading ;
	EBezierCurves<double>
						m_bzDegreeCurve ;
	EBezierCurves<EGL_PALETTE>
						m_bzColorFade ;

	// 移動パラメータ
	bool				m_fEnableMoving ;
	EBezierCurves<E3D_VECTOR>
						m_bzPositionCurves ;		// 移動ベジェ曲線
	EBezierCurves<double>
						m_bzRevolutionCurves ;		// 回転ベジェ曲線
	EBezierCurves<E3D_VECTOR_2D>
						m_bzMagnificationCurves ;	// 拡大ベジェ曲線

	// カメラアニメーション
	bool						m_fCameraMoving ;
	EBezierCurves<E3D_VECTOR>	m_bzCameraPos ;
	EBezierCurves<E3D_VECTOR>	m_bzCameraTarget ;
	EBezierCurves<double>		m_bzRevCameraZ ;

	// 活動パラメータ
	int					m_nActionType ;		// ActionType
	SDWORD				m_nDurationTime ;
	DWORD				m_dwCurrentMovingTime ;
	HANDLE				m_hDoneEvent ;
	ENumArray<DWORD>	m_lstDurationsTime ;

	// トーンフィルタ
	ECSToneFilter *		m_pToneFilter ;

	// スクリプト保存用
	DWORD				m_fdwFormatImage ;	// 画像バッファ作成パラメータ
	DWORD				m_dwWidthImage ;
	DWORD				m_dwHeightImage ;
	ECSReference		m_refImage ;		// 参照画像
	int					m_nFrameNum ;
	EGL_RECT			m_rectView ;
	ECSReference		m_refAlphaImage ;	// 合成マスク画像
	int					m_nParentFlag ;		// 親スプライトフラグ
											//	0 : メインスプライト
											//	1 : m_refParent 参照
	ECSReference		m_refParent ;		// 親スプライト
	ECSWideString		m_wstrPageID ;		// 非ヌル文字列のとき、ページ設定
	ECSReference		m_refRsrcManager ;	// リソースマネージャー
	ECSReference		m_refToneFilter ;	// トーンフィルタ

	ECSHash				m_hashItemStatus ;	// アイテムの各種設定履歴
		// m_hashItemStatus は ECSHash を要素に持つ
		// それぞれの要素には以下の要素が設定される場合がある
		//	"text"			: アイテムテイスト（ECSString）
		//	"font"			: アイテムフォント（ECSString）
		//	"image"			: アイテムイメージ（ECSString）
		//	"transparency"	: アイテム透明度（ECSInteger）
		//	"enabled"		: アイテム有効・無効状態（ECSInteger）
		//	"command"		: アイテムコマンド（ECSString）

	// スクリプトコールバック
	ECSReference		m_refHitTestProcedure ;	// SpriteHitTestProcedure
	ECSReference		m_refTimerProcedure ;	// SpriteTimerProcedure
	ECSReference		m_refMouseInterface ;	// SpriteMouseInterface
	ECSReference		m_refKeyInterface ;		// SpriteKeyInterface

	enum
	{
		MAX_INNER_MEMBER_COUNT = ECSResource::MAX_INNER_MEMBER_COUNT + 9,
	} ;

	static ECSWindow *	m_pMainWnd ;		// メインウィンドウ

public:
	struct	PLUGIN_OBJECT_HEADER
	{
		ECSSprite *	pBackLink ;
	} ;
	struct	PLUGIN_SPRITE
		: public PLUGIN_OBJECT_HEADER, public ECS_SPRITE_INTERFACE { } ;
	PLUGIN_SPRITE *	m_ppis ;		// プラグイン用インターフェース

	static ECSSprite * SpriteFromPlugin( ECS_SPRITE_INTERFACE * instance )
	{
		PLUGIN_SPRITE *	ppis = (PLUGIN_SPRITE*) instance ;
		ESLAssert( ppis->pBackLink->m_ppis == ppis ) ;
		return	ppis->pBackLink ;
	}

public:
	// 画像バッファ作成
	virtual PEGL_IMAGE_INFO CreateImage
			( DWORD fdwFormat, DWORD dwWidth, DWORD dwHeight,
					DWORD dwBitsPerPixel, DWORD dwFlags = 0 ) ;
	// スプライトを追加
	virtual void AddSprite( int nPriority, ESprite * pSprite ) ;
	// パラメータ複製
	virtual void CopyParameters( const EImageSprite * pSrc ) ;
	// リソース取得
	virtual PEGL_IMAGE_INFO GetImageInfo( void ) const ;

public:
	// 合成マスク設定
	ESLError SetAlphaImage
		( PEGL_IMAGE_INFO pBlendAlpha, unsigned int nAlphaRange ) ;
	// 現在の合成マスクの度合いを取得
	virtual unsigned int GetBlendDegree( void ) ;
	// 合成マスクの度合いを設定
	virtual void SetBlendDegree( unsigned int nDegree ) ;

protected:
	// スプライト表示アニメーションを指定の位置で切断し、後半だけ残す
	void DivideActivation( SDWORD dwOffsetTime ) ;

public:
	// フェード処理設定
	ESLError SetBlendingEnvelope( unsigned int nTargetDegree ) ;
	ESLError SetBlendingEnvelope( const EBezierCurves<double> & bzFading ) ;
	ESLError SetFadeEnvelope
		( const EBezierCurves<double> * pbzFade = NULL,
			const EBezierCurves<EGL_PALETTE> * pbzColor = NULL ) ;
	// 移動パラメータ設定
	ESLError SetBezierCurve
		( const EBezierCurves<E3D_VECTOR> * pbzCurve,
			const EBezierCurves<double> * pbzRev = NULL,
			const EBezierCurves<E3D_VECTOR_2D> * pbzMagnify = NULL ) ;
	ESLError SetBezierCurve
		( const EBezierCurves<E3D_VECTOR_2D> * pbzCurve = NULL,
			const EBezierCurves<double> * pbzRev = NULL,
			const EBezierCurves<E3D_VECTOR_2D> * pbzMagnify = NULL ) ;
	// カメラアニメーション設定
	ESLError SetCameraCurve
		( const EBezierCurves<E3D_VECTOR> & bzCamera,
			const EBezierCurves<E3D_VECTOR> & bzTarget,
			const EBezierCurves<double> & bzRevAngle ) ;
	// フェード処理・移動処理開始
	ESLError BeginActivation
		( unsigned int nDurationTime,
			HANDLE hDoneEvent = NULL, int nActionType = actNormal )
		{
			return	BeginActivation
				( &nDurationTime, 1, hDoneEvent, nActionType ) ;
		}
	ESLError BeginActivation
		( const unsigned int nDurationTime[], int nDurationCount,
			HANDLE hDoneEvent = NULL, int nActionType = actNormal ) ;
	// フェード処理を即時完了させる
	ESLError FlushActivation( void ) ;
	// フェード処理をキャンセルする
	ESLError CancelActivation( void ) ;
	// フェード処理中か？
	bool IsActivation( void ) const ;
	// トーンフィルタを関連付ける
	void AttachToneFilter( ECSToneFilter * pFilter ) ;

public:
	// 画像ファイルを開く
	virtual ESLError LoadImageFile
		( const wchar_t * pwszFileName, ECSContext * pContext = NULL ) ;
	// 画像バッファ消去
	virtual ESLError Release( void ) ;
	// スレッド同期
	ESLError Lock( DWORD dwTimeout = INFINITE ) ;
	ESLError Unlock( void ) ;
	// スキンアイテムを削除する
	void RemoveAllSkinItems( void ) ;

public:
	// 指定された識別子のアイテムを取得
	ESpriteInterface * GetSpriteItemAs
		( const wchar_t * pwszID, bool fChild = true ) ;

public:
	// 動的スプライトモード化取得
	virtual bool IsDynamicSpriteMode( void ) ;
	// 陰になる内接（最大）矩形取得
	virtual bool GetHiddenRectangle( EGL_RECT & rect ) ;
	// スプライト描画
	virtual void MTDraw( HEGL_RENDER_POLYGON hRenderPoly ) ;
protected:
	// 更新領域を再描画
	virtual void RefreshRect( const EGL_RECT & rectRefresh ) ;
	// 後処理フィルター
	virtual void RefreshRectPostFilter( const EGL_RECT & rectRefresh ) ;
public:
	// アニメーション進行
	virtual ESLError OnAdvanceAnimation( unsigned int nTime ) ;
	// 効果音を再生する
	virtual ESLError PlaySoundEffect
		( const wchar_t * pwszID, bool fRepeat = false ) ;

public:		// スクリプトコールバックインターフェース
	// SpriteHitTestProcedure
	enum	SpriteHitTestProcedure
	{
		virtualIsHitSprite,
	} ;
	// SpriteTimerProcedure
	enum	SpriteTimerProcedure
	{
		virtualOnTimer,
	} ;
	// SpriteMouseInterface
	enum	SpriteMouseInterface
	{
		virtualOnMouseMove,
		virtualOnMouseLeave,
		virtualOnMouseWheel,
		virtualOnLButtonDown,
		virtualOnLButtonUp,
		virtualOnLButtonDblClk,
		virtualOnRButtonDown,
		virtualOnRButtonUp,
		virtualOnRButtonDblClk,
	} ;
	// SpriteKeyInterface
	enum	SpriteKeyInterface
	{
		virtualOnKeyDown,
		virtualOnKeyUp,
	} ;
	// 当たり判定
	virtual bool IsHitSprite( int xPos, int yPos ) ;
	// メッセージ処理（マウスが上に乗っているかキャプチャーしているもののみ）
	virtual void OnMouseMove( UINT nFlags, int xPos, int yPos ) ;
	virtual void OnMouseLeave( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnMouseWheel
		( UINT nFlags, short int zDelta, int xPos, int yPos ) ;
	virtual bool OnLButtonDown( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnLButtonUp( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnLButtonDblClk( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnRButtonDown( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnRButtonUp( UINT nFlags, int xPos, int yPos ) ;
	virtual bool OnRButtonDblClk( UINT nFlags, int xPos, int yPos ) ;
	// メッセージ処理（フォーカスを持っているアイテムのみ）
	virtual bool MessageProc
		( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) ;
public:
	// コールバック関数実行用コンテキスト取得
	ECSContext * GetCallbackContext( void ) ;
	// スクリプト呼び出し
	void CallVoidScriptCallback
		( ECSReference& refInterface,
			int iVirtual, const int * pArg, int nArgCount ) ;
	bool CallBooleanScriptCallback
		( ECSReference& refInterface,
			int iVirtual, const int * pArg, int nArgCount ) ;
protected:
	// スクリプト呼び出し低水準関数
	ESLError CallScriptCallbackIntArgs
		( ECSContext & context,
			ECSReference& refInterface,
			int iVirtual, const int * pArg, int nArgCount ) ;

public:		// 通常のオブジェクト処理
	// オブジェクトの型名を取得する
	virtual const wchar_t * GetTypeName( void ) const ;
	virtual ECSObject * GetTypeOf( const wchar_t * pwszTypeName ) ;
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
	// メンバ変数取得
	virtual ECSObject * GetVariableAt( int nIndex ) ;
	// メンバ変数設定
	virtual ECSObject * SetVariableAt( int nIndex, ECSObject * obj ) ;
	// メンバ関数インデックス取得
	virtual ESLError GetFunction
		( ECSContext & context, int & nIndex, const wchar_t * pwszName ) ;
	// メンバ関数呼び出し
	virtual ESLError CallFunction
		( ECSContext & context,
			int nIndex, ECSObjArray<ECSObject> & lstArg ) ;

public:		// シリアル化のための関数（システムによって必要）
	// 全てのメンバ変数にインデックスを振る
	virtual void IndexAllMember( void ) ;
	// 全てのメンバ変数の参照を解消する
	virtual void CleanupAllReference( ECSContext & context ) ;
	// 全てのメンバ変数の参照を解決する
	virtual ESLError CommitAllReference( ECSContext & context ) ;
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;

public:
	// メンバ関数プロトタイプ
	typedef	ESLError (ECSSprite::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[92] ;
	static const PFUNC_CALL	m_pfnCallFunc[91] ;
	// メンバ関数（ImageSprite系）
	ESLError Call_GetInfo
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetImageInfo
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Release
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_AttachImage
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CreateSprite
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetBackColor
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_EnableDynamicMode
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CreateZBuffer
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_DeleteZBuffer
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CreateStereoBuffer
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetStereoViewInfo
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_DeleteStereoBuffer
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Set3DViewCamera
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Enable3DViewCamera
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Get3DViewCamera
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetScreenPosition
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetScreenPosition
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetDrawFunctionFlags
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetDrawFunctionFlags
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetRenderFunctionFlags
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetRenderFunctionFlags
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetParent
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsVisible
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetVisible
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetRectangle
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_MovePosition
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetPosition
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetTransparency
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetTransparency
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetZPosition
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetZPosition
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetParameter
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetParameter
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CopyParameters
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_UpdateRect
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Refresh
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetPriority
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ChangePriority
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_AddSprite
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_DetachSprite
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_DetachAllSprite
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_DrawImage
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_FillRect
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_DrawText
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数（SpriteInterface系）
	ESLError Call_GetSpriteID
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetSpriteID
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Enable
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsEnabled
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetSpriteText
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetSpriteText
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetSpriteFontFace
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetSpriteImage
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsHitSprite
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetSpriteAtPoint
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetFocus
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetFocus
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_KillFocus
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_MoveFocus
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetCapture
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ReleaseCapture
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetVertScrollPos
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetVertScrollPos
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetVertScrollRange
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetVertScrollRange
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetHorzScrollPos
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetHorzScrollPos
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetHorzScrollRange
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetHorzScrollRange
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsButtonChecked
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CheckButton
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetButtonViewStyle
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SendCommand
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetHitTestProcedure
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetTimerProcedure
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetMouseInterface
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetKeyInterface
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数（ScriptSpriteInterface系）
	ESLError Call_ModifyAnimationFlags
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_BeginAnimation
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_EndAnimation
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsDuringAnimation
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetAlphaImage
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetBlendDegree
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetBlendDegree
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetBlendingEnvelope
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetBezierCurve
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetCameraCurve
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_BeginActivation
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_FlushActivation
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CancelActivation
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsActivation
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_AttachToneFilter
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// スレッド同期（スクリプト用）
	inline ESLError QuickLock( void ) ;
	inline void QuickUnlock( void ) ;
	// スクリプトによってロックされているか
	static bool IsLocked( void ) ;

public:
	// プラグインインターフェースを取得する
	virtual void * GetObjectInterface( const wchar_t * pwszType ) ;

protected:
	// 基本情報
	static PCEGL_IMAGE_INFO __stdcall PIC_GetImageBuffer
		( ECS_SPRITE_INTERFACE * instance ) ;
	static HWND __stdcall PIC_GetWindow
		( ECS_SPRITE_INTERFACE * instance ) ;
	static ESLError __stdcall PIC_AttachImage
		( ECS_SPRITE_INTERFACE * instance, ECS_OBJECT * pImage ) ;
	static ESLError __stdcall PIC_SetImageView
		( ECS_SPRITE_INTERFACE * instance,
			PEGL_IMAGE_INFO pImage, PCEGL_RECT pViewRect = NULL ) ;
	static ESLError __stdcall PIC_CreateSprite
		( ECS_SPRITE_INTERFACE * instance,
			DWORD fdwFormat, int nWidth, int nHeight ) ;
	static void __stdcall PIC_Release
		( ECS_SPRITE_INTERFACE * instance ) ;
	static void __stdcall PIC_SetBackColor
		( ECS_SPRITE_INTERFACE * instance,
			EGL_PALETTE rgbBack, int fEnableBack ) ;
	static int __stdcall PIC_EnableDynamicMode
		( ECS_SPRITE_INTERFACE * instance, int fDynamicMode ) ;
	static PCEGL_IMAGE_INFO __stdcall PIC_GetZBuffer
		( ECS_SPRITE_INTERFACE * instance ) ;
	static void __stdcall PIC_CreateZBuffer
		( ECS_SPRITE_INTERFACE * instance ) ;
	static void __stdcall PIC_DeleteZBuffer
		( ECS_SPRITE_INTERFACE * instance ) ;
	static const E3D_VECTOR * __stdcall PIC_GetScreenPosition
		( ECS_SPRITE_INTERFACE * instance ) ;
	static void __stdcall PIC_SetScreenPosition
		( ECS_SPRITE_INTERFACE * instance, const E3D_VECTOR * vScreen ) ;
	static DWORD __stdcall PIC_GetDrawFunctionFlags
		( ECS_SPRITE_INTERFACE * instance ) ;
	static DWORD __stdcall PIC_GetRenderFunctionFlags
		( ECS_SPRITE_INTERFACE * instance ) ;
	static void __stdcall PIC_SetDrawFunctionFlags
		( ECS_SPRITE_INTERFACE * instance, DWORD dwFlags ) ;
	static void __stdcall PIC_SetRenderFunctionFlags
		( ECS_SPRITE_INTERFACE * instance, DWORD dwFlags ) ;
	static ECS_OBJECT * __stdcall PIC_GetParent
		( ECS_SPRITE_INTERFACE * instance ) ;
	static int __stdcall PIC_IsVisible
		( ECS_SPRITE_INTERFACE * instance ) ;
	static void __stdcall PIC_SetVisible
		( ECS_SPRITE_INTERFACE * instance, int fVisible ) ;
	static void __stdcall PIC_GetRectangle
		( ECS_SPRITE_INTERFACE * instance, EGL_RECT * rect ) ;
	static void __stdcall PIC_GetPosition
		( ECS_SPRITE_INTERFACE * instance, EGL_POINT * pos ) ;
	static void __stdcall PIC_MovePosition
		( ECS_SPRITE_INTERFACE * instance, long int xPos, long int yPos ) ;
	static unsigned int __stdcall PIC_GetTransparency
		( ECS_SPRITE_INTERFACE * instance ) ;
	static void __stdcall PIC_SetTransparency
		( ECS_SPRITE_INTERFACE * instance, unsigned int nTransparency ) ;
	static void __stdcall PIC_GetParameter
		( ECS_SPRITE_INTERFACE * instance,
			EImageSprite::PARAMETER * param ) ;
	static void __stdcall PIC_SetParameter
		( ECS_SPRITE_INTERFACE * instance,
			const EImageSprite::PARAMETER * param ) ;
	// 表示制御
	static void __stdcall PIC_UpdateRect
		( ECS_SPRITE_INTERFACE * instance,
			const EGL_RECT * pUpdateRect = NULL ) ;
	static void __stdcall PIC_Refresh
		( ECS_SPRITE_INTERFACE * instance ) ;
	static int __stdcall PIC_GetPriority
		( ECS_SPRITE_INTERFACE * instance ) ;
	static void __stdcall PIC_ChangePriority
		( ECS_SPRITE_INTERFACE * instance, int nPriority ) ;
	static void __stdcall PIC_AddSprite
		( ECS_SPRITE_INTERFACE * instance,
			int nPriority, ECS_SPRITE_INTERFACE * pChild ) ;
	static ESLError __stdcall PIC_DetachSprite
		( ECS_SPRITE_INTERFACE * instance, ECS_SPRITE_INTERFACE * pChild ) ;
	static ESLError __stdcall PIC_DetachAllSprite
		( ECS_SPRITE_INTERFACE * instance ) ;
	// アニメーション
	static DWORD __stdcall PIC_ModifyAnimationFlags
		( ECS_SPRITE_INTERFACE * instance,
			DWORD dwAddFlags = 0, DWORD dwRemoveFlags = 0 ) ;
	static ESLError __stdcall PIC_BeginAnimation
		( ECS_SPRITE_INTERFACE * instance,
			unsigned long int nLoopCount = 1,
				unsigned long int nBeginFrame = 0,
				unsigned long int nAnimationTime = -1,
				unsigned long int nRewindSequence = 0,
				unsigned long int nTurnSequence = -1 ) ;
	static ESLError __stdcall PIC_EndAnimation
		( ECS_SPRITE_INTERFACE * instance ) ;
	static int __stdcall PIC_IsDuringAnimation
		( ECS_SPRITE_INTERFACE * instance ) ;
	static ESLError __stdcall PIC_SetAlphaImage
		( ECS_SPRITE_INTERFACE * instance,
			PEGL_IMAGE_INFO pBlendAlpha, unsigned int nAlphaRange ) ;
	static unsigned int __stdcall PIC_GetBlendDegree
		( ECS_SPRITE_INTERFACE * instance ) ;
	static void __stdcall PIC_SetBlendDegree
		( ECS_SPRITE_INTERFACE * instance, unsigned int nDegree ) ;
	static ESLError __stdcall PIC_SetBlendingEnvelope
		( ECS_SPRITE_INTERFACE * instance,
			const double * pbzEnvelope, int nCount ) ;
	static ESLError __stdcall PIC_SetBezierCurve
		( ECS_SPRITE_INTERFACE * instance,
			const E3D_VECTOR_2D * pCurve, int nCurveCount,
			const double * pRev, int nRevCount,
			const E3D_VECTOR_2D * pZoom, int nZoomCount ) ;
	static ESLError __stdcall PIC_BeginActivation
		( ECS_SPRITE_INTERFACE * instance,
			const unsigned int nDurationTime[],
				int nDurationCount, int nActionType = actNormal ) ;
	static ESLError __stdcall PIC_FlushActivation
		( ECS_SPRITE_INTERFACE * instance ) ;
	static ESLError __stdcall PIC_CancelActivation
		( ECS_SPRITE_INTERFACE * instance ) ;
	static int __stdcall PIC_IsActivation
		( ECS_SPRITE_INTERFACE * instance ) ;

	friend	ECSResourceManager ;
} ;

//#define	ECSSprite	EScriptSpriteInterface

#include <glscsobj_window.h>


//////////////////////////////////////////////////////////////////////////////
// メッセージ出力スプライト
//////////////////////////////////////////////////////////////////////////////

class	ECSMessageSprite	: public	ECSSprite
{
public:
	// 構築関数
	ECSMessageSprite( void ) ;
	// 消滅関数
	virtual ~ECSMessageSprite( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSMessageSprite, ECSSprite )

protected:
	// 文字画像格納スプライト
	class	EMsgImageSprite	: public	EImageSprite
	{
	public:
		EGL_POINT		m_ptTarget ;	// 表示座標
		long int		m_nTime ;		// 表示時間 [ms]
		long int		m_nFadeSpeed ;	// フェード速度
	public:
		// 構築関数
		EMsgImageSprite( void ) ;
		// 消滅関数
		virtual ~EMsgImageSprite( void ) ;
		// クラス情報
		DECLARE_CLASS_INFO( EMsgImageSprite, EImageSprite )
	} ;

protected:
	EFormResourceManager *	m_pfrmStyle ;		// スタイル
	EWideString				m_wstrDefStyle ;	// デフォルトスタイル識別子
	EDescription *			m_pdscDefStyle ;	// デフォルトのスタイル
	EDescription			m_dscMsgStyle ;		// 現在のスタイル
	EStaticTextSprite::TEXT_STYLE
							m_tsMsgStyle ;		// 現在のスタイル
	EFontObject				m_fontMsg ;
	EFontObject				m_fontReading ;		// ルビフォント
	bool					m_fFontBold ;		// フォントに太字が使えるか？
												// m_fFontBold == true の時、
												// 描画の際に太字処理する
	bool					m_fFontBorder ;		// フォントに縁取り処理するか？
	bool					m_fMessageInSize ;	// 出力サイズ分しか出力しない

	long int				m_nDefCharSpeed ;	// メッセージ速度 [ms/char]
	long int				m_nCharSpeed ;
	long int				m_nCharSpeedRatio ;
	long int				m_nDefFadeSpeed ;	// メッセージフェード速度
	long int				m_nFadeSpeed ;

	long int				m_nMsgTimeCount ;	// 文字出力時間カウンタ
	ERealFontImage			m_rfiText ;			// 文字列
	HEGL_RENDER_POLYGON		m_hRenderPoly ;		// 描画オブジェクト
	EPtrObjArray<EMsgImageSprite>
							m_lstMsgChar ;		// 文字スプライト

	DWORD					m_dwCurMessageTime ;// 出力経過時間
	HANDLE					m_hOutputEvent ;	// 出力完了イベント
	unsigned int			m_nOutputCount ;	// 出力カウンタ
	long int				m_yScrollOffset ;	// スクロール座標

	double					m_rMsgEffectX ;		// 文字エフェクト
	double					m_rMsgEffectY ;
	double					m_rMsgEffectHorz ;
	double					m_rMsgEffectVert ;
	double					m_rMsgEffectRev ;

	// スクリプト保存用
	ECSReference			m_refRsrcManager ;	// リソースマネージャへの参照
	ECSWideString			m_wstrTextLog ;		// 出力されている文字列のログ

public:
	// アニメーション進行
	virtual ESLError OnAdvanceAnimation( unsigned int nTime ) ;

public:
	// メッセージスプライト削除
	virtual void DeleteImage( void ) ;
	// リソース開放
	virtual ESLError Release( void ) ;
	// 全てのスプライトを分離
	virtual void DetachAllSprite( void ) ;
	// 全てのスプライトを削除
	virtual void RemoveAllSprite( void ) ;
	// 文字フォント設定
	virtual void SetSpriteFontFace( const wchar_t * pwszFont ) ;

public:
	// メッセージスプライト作成
	virtual ESLError CreateMessage
		( DWORD dwWidth, DWORD dwHeight,
			const EGL_RECT * pMsgRect = NULL, bool fOutputInSize = false ) ;
	// メッセージ出力
	virtual int OutputMessage( const wchar_t * pwszMsg ) ;
protected:
	// 文字列を出力
	int DrawMessage( const wchar_t * pwszMsg ) ;
	// 外字を出力
	virtual ESLError DrawExtensionChar
		( const wchar_t * pwszID, bool fEffectColor, bool fEffectDeco ) ;
	// 文字を修飾して新たにスプライトを作成
	EMsgImageSprite * CreateMsgCharFromSprite( EImageSprite * pisChar ) ;
	// 文字列を行末を折り返さないように出力
	int DrawWordWithoutWrapping( const wchar_t * pwszWord ) ;
	// ルビ文字列を画像化してスプライトを作成
	ESLError DrawWordWithReading
		( const wchar_t * pwszWord, const wchar_t * pwszReading ) ;
	// 文字画像スプライトを出力
	ESLError DrawMsgImage
		( EMsgImageSprite * pmis, int nWordCount = 1,
			bool fCharOffset = false,
			int xOffset = 0, int yOffset = 0, int nTextPitch = 0 ) ;
public:
	// メッセージ出力を即時完了させる
	virtual ESLError FlushMessage( void ) ;
	// メッセージをクリアする
	virtual ESLError ClearMessage( void ) ;
	// 現在メッセージが出力中か？
	bool IsMessagePending( void ) ;
	// メッセージの出力が完了するまで待つ
	ESLError WaitUntilOutputMessage( DWORD dwTimeout ) ;

public:
	// 現在のカーソル位置を取得する
	EGL_POINT GetCursorPos( void )
		{
			return	m_rfiText.GetCursorPos( ) ;
		}
	// 現在のカーソル位置を設定する
	void MoveCursorPos( const EGL_POINT & ptCursor )
		{
			m_rfiText.MoveCursorPos( ptCursor ) ;
		}
	// スタイルを設定する
	ESLError AttachMessageStyle
		( EFormResourceManager * pfrmStyle,
			const wchar_t * pwszDefaultStyle ) ;
	// デフォルトメッセージ速度を設定する
	ESLError SetDefaultMsgSpeed
		( long int nCharSpeed,
			long int nFadeSpeed, long int nSpeedRatio = 0x100 ) ;
	// 文字出力エフェクトを設定する
	ESLError SetMessageEffect
		( double x, double y, double rHorz, double rVert, double rRev ) ;
	// フォントフェース設定
	ESLError SetFontFaceName( const wchar_t * pwszFaceList ) ;
	// 太字設定
	ESLError SetFontBold( bool fBold ) ;
	// 斜体設定
	ESLError SetFontItalic( bool fItalic ) ;
	// フォントサイズ設定
	ESLError SetFontSize( int nFontSize, bool fOffset ) ;
	// 文字色設定
	ESLError SetFontColor
		( EGL_PALETTE rgbText, bool fText,
			EGL_PALETTE rgbShadow, bool fShadow ) ;
	// 文字の影の透明度を設定
	ESLError SetShadowTransparency( unsigned int nTransparency ) ;
	// 文字の縁取りを設定する
	ESLError SetFontBordering( bool fBorder ) ;
	// 行間を設定する
	ESLError SetLineHeight( int nLineHeight, bool fOffset ) ;
	// スタイルを設定する
	ESLError SetFontStyle( const wchar_t * pwszStyleID = NULL ) ;
	// 表示文字の外接矩形を取得する
	bool GetMessageRect( EGL_RECT& rect ) const ;

protected:
	// スタイルを適用する
	ESLError UpdateCurrentStyle( void ) ;

public:		// 通常のオブジェクト処理
	// オブジェクトの型名を取得する
	virtual const wchar_t * GetTypeName( void ) const ;
	virtual ECSObject * GetTypeOf( const wchar_t * pwszTypeName ) ;
	// オブジェクトを複製
	virtual ECSObject * Duplicate( void ) ;
	// メンバ関数インデックス取得
	virtual ESLError GetFunction
		( ECSContext & context, int & nIndex, const wchar_t * pwszName ) ;
	// メンバ関数呼び出し
	virtual ESLError CallFunction
		( ECSContext & context,
			int nIndex, ECSObjArray<ECSObject> & lstArg ) ;

public:		// シリアル化のための関数（システムによって必要）
	// 全てのメンバ変数にインデックスを振る
	virtual void IndexAllMember( void ) ;
	// 全てのメンバ変数の参照を解決する
	virtual ESLError CommitAllReference( ECSContext & context ) ;
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;

public:
	// メンバ関数プロトタイプ
	typedef	ESLError (ECSMessageSprite::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[18] ;
	static const PFUNC_CALL	m_pfnCallFunc[17] ;
	// メンバ関数
	ESLError Call_CreateMessage
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_OutputMessage
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_FlushMessage
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ClearMessage
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsMessagePending
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_AttachMessageStyle
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetDefaultMsgSpeed
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetMessageEffect
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetShadowTransparency
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetFontBordering
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetFontStyle
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetFontFace
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetFontColor
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetCursorPos
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_MoveCursorPos
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetCharacterCount
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetMessageRect
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// 入力フィルタ
//////////////////////////////////////////////////////////////////////////////

class	ECSInputFilter	: public ECSObject, public EInputFilter
{
public:
	// 構築関数
	ECSInputFilter( void ) ;
	// 消滅関数
	virtual ~ECSInputFilter( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO2( ECSInputFilter, ECSObject, EnputFilter )

protected:
	ECSReference	m_refWindow ;
	DWORD			m_dwFilterMode ;

public:
	struct	PLUGIN_OBJECT_HEADER
	{
		ECSInputFilter *	pBackLink ;
	} ;
	struct	PLUGIN_INPUT_FILTER
		: public PLUGIN_OBJECT_HEADER, public ECS_INPUT_FILTER_INTERFACE { } ;
	PLUGIN_INPUT_FILTER *	m_ppiif ;		// プラグイン用インターフェース

	static ECSInputFilter *
		InputFilterFromPlugin( ECS_INPUT_FILTER_INTERFACE * instance )
	{
		PLUGIN_INPUT_FILTER *	ppiif = (PLUGIN_INPUT_FILTER*) instance ;
		ESLAssert( ppiif->pBackLink->m_ppiif == ppiif ) ;
		return	ppiif->pBackLink ;
	}

public:
	// フィルタファイルを読み込む
	virtual ESLError LoadInputFilter
		( const wchar_t * pwszFileName, ECSContext * pContext = NULL ) ;
	ESLError LoadInputFilter( EDescription & dscFilter )
		{
			return	EInputFilter::LoadInputFilter( dscFilter ) ;
		}

public:
	// ファイルを開く
	virtual ESLFileObject * OpenFilterFile
		( const wchar_t * pwszFilePath, ECSContext * pContext ) ;

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
	// メンバ変数取得
	virtual ECSObject * GetVariableAt( int nIndex ) ;
	// メンバ変数設定
	virtual ECSObject * SetVariableAt( int nIndex, ECSObject * obj ) ;
	// メンバ関数インデックス取得
	virtual ESLError GetFunction
		( ECSContext & context, int & nIndex, const wchar_t * pwszName ) ;
	// メンバ関数呼び出し
	virtual ESLError CallFunction
		( ECSContext & context,
			int nIndex, ECSObjArray<ECSObject> & lstArg ) ;

public:		// シリアル化のための関数（システムによって必要）
	// 全てのメンバ変数にインデックスを振る
	virtual void IndexAllMember( void ) ;
	// 全てのメンバ変数の参照を解消する
	virtual void CleanupAllReference( ECSContext & context ) ;
	// 全てのメンバ変数の参照を解決する
	virtual ESLError CommitAllReference( ECSContext & context ) ;
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;

public:
	// メンバ関数プロトタイプ
	typedef	ESLError (ECSInputFilter::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[19] ;
	static const PFUNC_CALL	m_pfnCallFunc[18] ;
	// メンバ関数
	ESLError Call_LoadInputFilter
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_DeleteInputFilter
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_OpenFilter
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CloseFilter
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetInputEvent
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_FlushInputQueue
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetCapturedJoyStick
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetStickPosition
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsJoyButtonPushing
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetJoyButtonPushed
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_FlushJoyButtonPushed
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ResetJoyButtonPushing
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetCursorPos
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_MoveCursorPos
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_AddFilter
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_RemoveFilter
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetFilter
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_DispatchEvent
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

public:
	// プラグインインターフェースを取得する
	virtual void * GetObjectInterface( const wchar_t * pwszType ) ;

protected:
	static ESLError __stdcall PIC_LoadInputFilter
		( ECS_INPUT_FILTER_INTERFACE * instance,
			const wchar_t * pwszFileName, ECS_CONTEXT * pContext = NULL ) ;
	static void __stdcall PIC_DeleteInputFilter
		( ECS_INPUT_FILTER_INTERFACE * instance ) ;
	static ESLError __stdcall PIC_OpenFilter
		( ECS_INPUT_FILTER_INTERFACE * instance,
						int nType, ECS_OBJECT * pWnd ) ;
	static ESLError __stdcall PIC_CloseFilter
		( ECS_INPUT_FILTER_INTERFACE * instance ) ;
	static ESLError __stdcall PIC_GetInputEvent
		( ECS_INPUT_FILTER_INTERFACE * instance,
			ECS_INPUT_FILTER_INTERFACE::INPUT_EVENT & ieEvent,
								DWORD dwTimeout = INFINITE ) ;
	static ESLError __stdcall PIC_FlushInputQueue
		( ECS_INPUT_FILTER_INTERFACE * instance, int nLimit ) ;
	static DWORD __stdcall PIC_GetCapturedJoyStick
		( ECS_INPUT_FILTER_INTERFACE * instance ) ;
	static ESLError __stdcall PIC_GetStickPosition
		( ECS_INPUT_FILTER_INTERFACE * instance,
					E3D_VECTOR & vPos, int iDevNum = 0 ) ;
	static int __stdcall PIC_IsJoyButtonPushing
		( ECS_INPUT_FILTER_INTERFACE * instance,
						int iKeyNum, int iDevNum = 0 ) ;
	static int __stdcall PIC_GetJoyButtonPushed
		( ECS_INPUT_FILTER_INTERFACE * instance,
						int iKeyNum, int iDevNum = 0 ) ;
	static ESLError __stdcall PIC_FlushJoyButtonPushed
		( ECS_INPUT_FILTER_INTERFACE * instance,
					int iDevNum = 0, int iKeyNum = -1 ) ;
	static void __stdcall PIC_GetCursorPos
		( ECS_INPUT_FILTER_INTERFACE * instance, EGL_POINT & ptCursor ) ;
	static void __stdcall PIC_MoveCursorPos
		( ECS_INPUT_FILTER_INTERFACE * instance,
						long int xPos, long int yPos ) ;

	friend	ECSWindow ;
} ;


//////////////////////////////////////////////////////////////////////////////
// ムービー再生用スプライト
//////////////////////////////////////////////////////////////////////////////

#include <sakuragl/media/sgl_audio_player.h>
#include <sakuragl/media/sgl_media_player.h>

#if	!defined(__SAKURAGL_MEI_MEDIA_MEDIA_PLAYER_H__)
namespace SakuraGL
{
	class	SGLMEIMediaPlayer ;
}
#endif

class	ECSMovieSprite	: public ECSSprite, public ERIAnimationPlayer,
							public SakuraGL::SGLMediaPlayerFrameNotification
{
public:
	// 構築関数
	ECSMovieSprite( void ) ;
	// 消滅関数
	virtual ~ECSMovieSprite( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO2( ECSMovieSprite, ECSSprite, ERIAnimationPlayer )

protected:
	// ムービー再生用スレッド
	class	ECSPlayerThread	: public	EGLSThread
	{
	public:
		// 構築関数
		ECSPlayerThread( void ) ;
		// 消滅関数
		virtual ~ECSPlayerThread( void ) ;
		// クラス情報
		DECLARE_CLASS_INFO( ECSPlayerThread, EGLSThread )
	public:
		ECSMovieSprite *	m_pSprite ;
	protected:
		// スレッド関数
		virtual DWORD ThreadProc( void ) ;
	} ;

public:
	enum	MoviePlayerFlag
	{
		mpfDirectDraw		= 0x0001,		// 直接画面に描画
		mpfLoopPlay			= 0x0002,		// 繰り返し再生
		mpfNoSkipFrame		= 0x0004,		// フレームをスキップしない
		mpfNoLoopFilter		= 0x0400,		// ループフィルタ無効化
		mpfUseLoopFilter	= 0x0800,		// ループフィルタ使用
	} ;

protected:
	// MEI 再生用
	enum	MovieFileStatus
	{
		mfsNotOpened,
		mfsOpened,
		mfsPlaying
	} ;
	MovieFileStatus	m_mfsStatus ;		// 再生ステータス
	ECSPlayerThread	m_taskPlayer ;		// 動画再生専用スレッド
	DWORD			m_dwMovieFlags ;
	HANDLE			m_hCancelPlaying ;	// 再生停止イベント

	// ループパラメータ
	DWORD			m_dwLoopStartFrame ;
	SDWORD			m_dwLoopEndFrame ;

	// EntisGLS4 MEI 再生
	SakuraGL::SGLMEIMediaPlayer *	m_player ;
	SSystem::SArray<EGL_PALETTE>	m_aMeiFramePalette ;
	EGL_IMAGE_INFO					m_eiiSGLMeiFrame ;
	bool							m_flagInStopPlayer ;

	// DirectShow 再生用
	struct IGraphBuilder *	m_pGraphBuilder ;
	struct IMediaControl *	m_pMediaControl ;
	struct IVideoWindow *	m_pVideoWindow ;
	struct IBasicAudio *	m_pBasicAudio ;
	struct IMediaPosition *	m_pMediaPosition ;
	bool					m_fWindowNullDraw ;

	// スクリプト用パラメータ
	ECSString		m_wstrFileName ;	// 開いているファイル名
	ESLFileObject *	m_pOwnFileObj ;		// ファイルオブジェクト
	DWORD			m_dwRestoredStatus ;// セーブされた時点での再生ステータス
	SDWORD			m_dwRestoredFrame ;	// セーブされた時点でのフレーム数
	DWORD			m_dwRestoredPlayFlags ;

	static EGLDrawImage *	m_pDrawImage ;

public:
	// 動画ファイルを開く
	ESLError OpenMovie( ESLFileObject & file ) ;
	ESLError OpenMovieFile
		( const wchar_t * pwszFileName, ECSContext * pContext = NULL ) ;
	ESLError OpenMovieWithDirectShow( const wchar_t * pwszFilePath ) ;
	// 動画ファイルを閉じる
	ESLError CloseMovie( void ) ;
	// 再生開始
	ESLError PlayMovie( DWORD dwFlags, int fPlayType = -1 ) ;
	ESLError PlayMovieWithDirectShow( void ) ;
	// 再生停止
	ESLError StopMovie( void ) ;
	// ループ位置設定
	void SetLoopPosition( DWORD dwStartFrame, SDWORD dwEndFrame ) ;

public:	// 再生制御オーバーロード
	// 指定のフレームに移動
	ESLError SeekToFrame( unsigned int iFrameIndex ) ;
	// 現在の再生位置を取得する
	unsigned int CurrentIndex( void ) const ;
	// 動画全長を取得
	unsigned int GetAllFrameCount( void ) const ;
	unsigned int GetTotalTime( void ) const ;

public:
	// アニメーション進行（DirectShow 再生時に表示位置調整）
	virtual ESLError OnAdvanceAnimation( unsigned int nTime ) ;

protected:
	// ムービーファイルを開く
	static LRESULT __stdcall OpenDShowMovieFileProc( void * pInstance ) ;
	// ムービーの再生を開始する
	static LRESULT __stdcall PlayDShowMovieProc( void * pInstance ) ;

public:
	// アニメーション再生を中断する
	virtual void CancelPlaying( void ) ;
	// アニメーションが再生中か判定する
	bool IsMoviePlaying( void ) ;

public:
	// 再生ループポイント設定
	virtual ESLError SetRewindingPortion
		( unsigned int nRewindPos = -1,
			unsigned int nEndPos = -1, bool fRepeat = true ) ;
	// 音量を設定する
	virtual ESLError SetVolume( REAL32 rLeftVol, REAL32 rRightVol ) ;
	// 再生中か調べる
	virtual bool IsPlaying( void ) ;

public:
	// 画像描画オブジェクトを取得する
	static EGLDrawImage * GetDrawImageObject( void )
		{
			return	m_pDrawImage ;
		}
	// 画像描画オブジェクトを設定する
	static void SetDrawImageObject( EGLDrawImage * pDrawImage )
		{
			m_pDrawImage = pDrawImage ;
		}

protected:	// ERIAnimationPlayer オーバーライド
	// 描画処理
	virtual ESLError OnDrawMovieFrame
		( HWND hwndTarget, int xPos, int yPos,
			const EGL_SIZE * pViewSize,
			EGLDrawImage * pDrawImage,
			PCEGL_IMAGE_INFO pImage, DWORD dwDuringTime ) ;
	// 再生中のメッセージ処理
	virtual ESLError OnDispatchMessage( void ) ;
	// 時間待ち
	virtual ESLError OnWaitingTime( DWORD dwTime ) ;

public:	// SGLMediaPlayerFrameNotification オーバーライド
	// SGLMEIMediaPlayer フレーム更新通知
	virtual void OnFrameUpdate( SakuraGL::SGLMediaPlayerInterface * player ) ;
	// 再生区間終端到達
	virtual void OnEndOfDuration( SakuraGL::SGLMediaPlayerInterface * player ) ;

protected:
	// SGLMEIMediaPlayer の現在のフレームを m_eiiSGLMeiFrame に取得
	void GetCurrentFrameOfSGLMeiMediaPlayer( void ) ;

public:		// 通常のオブジェクト処理
	// オブジェクトの型名を取得する
	virtual const wchar_t * GetTypeName( void ) const ;
	virtual ECSObject * GetTypeOf( const wchar_t * pwszTypeName ) ;
	// オブジェクトを複製
	virtual ECSObject * Duplicate( void ) ;
	// メンバ関数インデックス取得
	virtual ESLError GetFunction
		( ECSContext & context, int & nIndex, const wchar_t * pwszName ) ;
	// メンバ関数呼び出し
	virtual ESLError CallFunction
		( ECSContext & context,
			int nIndex, ECSObjArray<ECSObject> & lstArg ) ;

public:		// シリアル化のための関数（システムによって必要）
	// 全てのメンバ変数にインデックスを振る
	virtual void IndexAllMember( void ) ;
	// 全てのメンバ変数の参照を解決する
	virtual ESLError CommitAllReference( ECSContext & context ) ;
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;

public:
	// メンバ関数プロトタイプ
	typedef	ESLError (ECSMovieSprite::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[10] ;
	static const PFUNC_CALL	m_pfnCallFunc[9] ;
	// メンバ関数
	ESLError Call_OpenMovie
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CloseMovie
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_PlayMovie
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_StopMovie
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsMoviePlaying
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SeekFrame
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetCurrentFrame
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetTotalFrame
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetTotalTime
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

	friend	ECSPlayerThread ;
} ;


//////////////////////////////////////////////////////////////////////////////
// 特殊効果用スプライト
//////////////////////////////////////////////////////////////////////////////

class	ECSSuperSprite	: public	ECSSprite
{
public:
	// 構築関数
	ECSSuperSprite( void ) ;
	// 消滅関数
	virtual ~ECSSuperSprite( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSSuperSprite, ECSSprite )

public:
	// エフェクトタイプ
	enum	EffectType
	{
		etNothing		= 0,
		etTileImage,					// 画像をタイル状に並べる
		etFilterWhite,					// 白色フィルタ
		etFilterLight,
		etFilterBlack,					// 黒色フィルタ
		etFilterDark,
		etRasterScroll,					// ラスタスクロール
		etWaveCircle,					// 波紋変形スクロール
		etShimmer,						// 陽炎
		etMeshWarp,						// メッシュワープ
		etShadingOff,					// ぼかし
		etShadingLight,					// ぼかしと光
		efSmashParticle2D,				// 粉砕パーティクル（二次元）
		efSmashParticle3D,				// 粉砕パーティクル（三次元）
		etMax
	} ;
	// フラグ
	enum	EffectFlag
	{
		effTransition		= 0x0001,
		effLoopPlaying		= 0x0002,
		effTransformation	= 0x0004,
	} ;
	// エフェクトパラメータ構造体
	struct	EFFECT_PARAM
	{
		EffectType		eftType ;
		DWORD			fdwFlags ;
		int				nInterval ;
		int				nDegreeStep ;
		int				nShakingWidth ;
		int				nMeshSize ;
		int				nMeshDivision ;
		int				nFrequency ;
		EGL_SIZE		sizeView ;
		EGL_POINT		ptSpeed ;
		PEGL_IMAGE_INFO	pImageInf ;
		int				nAlphaRange ;
		int				nMilliSecPerDegree ;// 時間進行比率
		EGL_POINT		ptSmashPoint ;		// 粉砕中心点（画像中心からの相対座標）
		REAL32			rSmashDelay ;		// 粉砕遅延時間／距離比率
		REAL32			rSmashPower ;		// 粉砕力 [pixel/sec]
		REAL32			rRandomPower ;		// ランダム成分 [pixel/sec]
		REAL32			rDeceleration ;		// 速度減衰率 [/sec]
		E3D_VECTOR		vVelocity ;			// 初速度 [pixel/sec]
		E3D_VECTOR		vGravity ;			// 加速度 [pixel/sec]
		E3D_VECTOR		vRevSpeed ;			// 回転速度 [deg/sec]
		E3D_VECTOR		vRevRandom ;		// ランダム成分[deg/sec]
	} ;

protected:
	EffectType		m_eftType ;			// エフェクトタイプ
	DWORD			m_fdwFlags ;		// エフェクトフラグ
	EFFECT_PARAM	m_efprm ;			// エフェクトパラメータ（保存用）

	// タイル表示用
	EGLSize			m_sizeView ;		// タイル表示するサイズ
	EGLPoint		m_ptScroll ;		// 現在の表示位置
	EGLPoint		m_ptSpeed ;			// スクロール速度 [pixel]

	// 共通パラメータ
	int				m_nInterval ;		// アニメーションインターバル
	int				m_nIntervalCounter ;
	int				m_nDegreeStep ;		// 変化の度合い

	// メッシュ変形パラメータ
	int				m_nShakingWidth ;	// 揺れ幅（振幅）
	int				m_nMeshSize ;		// 揺れ幅（波長）
	int				m_nFrequency ;		// 変化度合い(100H)に対する周期

	// メッシュ変形用
	EGLSize			m_sizeInMesh ;		// メッシュ
	E3D_VECTOR_2D	m_vMeshSize ;
	E3D_VECTOR_2D *	m_pBaseMesh ;
	EStreamBuffer	m_bufBaseMesh ;
	E3D_VECTOR_2D *	m_pMesh ;
	EStreamBuffer	m_bufMesh ;
	int				m_nMeshListSize ;
	E3D_VECTOR_2D *	m_pMeshList ;
	EGL_RECT		m_rctMeshExt ;

	// 色フィルタ用
	ECSToneFilter *	m_ptfToneBuffer ;	// トーンフィルタ

	// ぼかしエフェクト
	EGLImage		m_imgViewBuf ;		// 中間描画用バッファ
	EObjArray<EGLImage>
					m_lstShadeOff ;		// ぼかしバッファ
	DWORD			m_fdwShadingUpdate ;// ぼかしバッファの更新フラグ

	// パーティクルオブジェクト
	class	EParticleObject
	{
	public:
		E3DVector		m_vCurrentPos ;		// 現在の座標
		E3DVector		m_vCurrentRev ;		// 現在の回転角度 [deg]
		EGLImage		m_imgEffect ;		// マスク処理用バッファ
		EGLImage		m_imgMask ;			// マスク用画像
		EGLImage		m_imgPortion ;		// 部分画像
		EGLPoint		m_ptRevCenter ;		// 画像の中心座標
		long int		m_nDelayMilliSec ;	// 遅延時間
		E3DVector		m_vPosition ;		// 表示中心座標
		E3DVector		m_vVelocity ;		// 初速度 [pixel/sec]
		E3DVector		m_vRevSpeed ;		// 回転速度 [deg/sec]
	public:
		void CalculateParticle
			( const EFFECT_PARAM & efprm, long int nMilliSec ) ;
		PEGL_IMAGE_INFO GetPortionImage( void ) ;
		void UpdatePortionImage
			( const EFFECT_PARAM & efprm, unsigned int nDegree ) ;
	} ;

	// パーティクル用
	ECSReference	m_refParticleImage ;// パーティクル用画像（保存用）
	EObjArray<EParticleObject>
					m_lstParticle ;		// パーティクルオブジェクト

public:
	// 画像バッファ消去
	virtual ESLError Release( void ) ;
	// 外接矩形を取得
	virtual EGL_RECT GetRectangle( void ) ;
	// 陰になる内接（最大）矩形取得
	virtual bool GetHiddenRectangle( EGL_RECT & rect ) ;
	// マルチスレッド描画前準備処理
	virtual void BeforeMTDraw( void ) ;
	// スプライト描画
	virtual void MTDraw( HEGL_RENDER_POLYGON hRenderPoly ) ;
	// スプライト上の指定領域の更新通知
	virtual bool UpdateRect( EGL_RECT * pUpdateRect = NULL ) ;
	// アニメーション進行
	virtual ESLError OnAdvanceAnimation( unsigned int nTime ) ;

protected:
	// スプライト描画関数
	typedef void (ECSSuperSprite::*PFUNC_BeforeDraw)( void ) ;
	typedef void (ECSSuperSprite::*PFUNC_Draw)
				( HEGL_RENDER_POLYGON hRenderPoly ) ;
	static const PFUNC_BeforeDraw	m_pfnBeforeDraw[etMax] ;
	static const PFUNC_Draw			m_pfnDraw[etMax] ;

	// 描画前準備処理（何もしない）
	void before_draw_Normal( void ) ;
	// 通常スプライト描画
	void draw_Normal( HEGL_RENDER_POLYGON hRenderPoly ) ;
	// タイル状表示
	void draw_TiledImage( HEGL_RENDER_POLYGON hRenderPoly ) ;
	// 色フィルタ描画
	void draw_FilteredImage( HEGL_RENDER_POLYGON hRenderPoly ) ;
	// ラスタスクロール
	void draw_RasterScroll( HEGL_RENDER_POLYGON hRenderPoly ) ;
	// 変形描画
	void draw_MeshTransformation( HEGL_RENDER_POLYGON hRenderPoly ) ;
	// ぼかし描画前準備処理
	void before_draw_ShadingOff( void ) ;
	// ぼかし描画
	void draw_ShadingOff( HEGL_RENDER_POLYGON hRenderPoly ) ;
	// パーティクル 2D 描画
	void draw_Particle2D( HEGL_RENDER_POLYGON hRenderPoly ) ;
	// パーティクル 3D 描画
	void draw_Particle3D( HEGL_RENDER_POLYGON hRenderPoly ) ;

public:
	// 現在の合成マスクの度合いを取得
	virtual unsigned int GetBlendDegree( void ) ;
	// 変形の度合いを設定
	virtual void SetBlendDegree( unsigned int nDegree ) ;

protected:
	// 色フィルタ設定
	void SetDegreeOnColorFilter
		( EffectType eftType, unsigned int nDegree ) ;
	// 波紋変形メッシュ生成
	void SetDegreeOnWaveCircle( unsigned int nDegree ) ;
	// メッシュワープ変形
	void SetDegreeOnMeshWarp( unsigned int nDegree ) ;
	// 粉砕パーティクル設定
	void SetDegreeOnSmashParticle( unsigned int nDegree ) ;

public:
	// エフェクトを設定する
	void SetEffectParameter( const EFFECT_PARAM & param ) ;
	// メッシュワープエフェクトを設定する
	void SetMeshWarpEffect
		( const E3D_VECTOR_2D * pvMesh, int nMeshListCount,
			const EGL_SIZE & sizeInMesh,
			const E3D_VECTOR_2D * pvBaseMesh = NULL ) ;

protected:
	// 色フィルタの初期設定
	void InitializeColorFilter( void ) ;
	// メッシュの初期設定
	void InitializeMeshTransformation( void ) ;
	// ぼかしバッファの初期設定
	void InitializeShadingBuffer( int iBuffering = 0 ) ;
	// 粉砕パーティクル初期化
	void InitializeSmashParticle( void ) ;

public:		// スクリプトインターフェース
	// オブジェクトの型名を取得する
	virtual const wchar_t * GetTypeName( void ) const ;
	virtual ECSObject * GetTypeOf( const wchar_t * pwszTypeName ) ;
	// オブジェクトを複製
	virtual ECSObject * Duplicate( void ) ;
	// メンバ関数インデックス取得
	virtual ESLError GetFunction
		( ECSContext & context, int & nIndex, const wchar_t * pwszName ) ;
	// メンバ関数呼び出し
	virtual ESLError CallFunction
		( ECSContext & context,
			int nIndex, ECSObjArray<ECSObject> & lstArg ) ;

public:		// シリアル化のための関数（システムによって必要）
	// 全てのメンバ変数にインデックスを振る
	virtual void IndexAllMember( void ) ;
	// 全てのメンバ変数の参照を解決する
	virtual ESLError CommitAllReference( ECSContext & context ) ;
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;

public:
	// メンバ関数プロトタイプ
	typedef	ESLError (ECSSuperSprite::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[3] ;
	static const PFUNC_CALL	m_pfnCallFunc[2] ;
	// メンバ関数
	ESLError Call_SetEffectParameter
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetMeshWarpEffect
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// パーティクル用スプライト
//////////////////////////////////////////////////////////////////////////////

class	ECSParticleSprite	: public	ECSSprite
{
public:
	// 構築関数
	ECSParticleSprite( void ) ;
	// 消滅関数
	virtual ~ECSParticleSprite( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSParticleSprite, ECSSprite )

public:
	enum	ParticleFlag
	{
		pfAnimationLoop	= 0x0001,	// 画像のアニメーションをループさせる
	} ;
	struct	PARTICLE_FLICK
	{
		double			rAmplitude ;		// 揺らぎ幅 [pixel]
		double			rAmplitudeRange ;	// 揺らぎ幅（乱数） [pixel]
		double			rFrequency ;		// 揺らぎ周期 [sec]
		double			rFrequencyRange ;	// 揺らぎ周期（乱数） [sec]
	} ;
	struct	PARTICLE_PARAM
	{
		unsigned int	nFlags ;			// フラグ
		unsigned int	nDuration ;			// 寿命 [ms]
		unsigned int	nAnimationSpeed ;	// アニメーション速度比 x100H
											//（アニメ画像パーティクル用）
		unsigned int	nFadein ;			// フェードイン時間 [ms]
		unsigned int	nFadeout ;			// フェードアウト時間 [ms]
		unsigned int	nFadeTransparency ;	// フェードアウト透明度
		double			rFadeZoom ;			// フェードアウト時の拡大比率
		double			rGenWidth ;			// 発生幅 [pixel]
		double			rGenHeight ;
		double			rGenAngle ;			// 発生角 [deg]
		double			rGenAngleRange ;	// 発生角の幅（乱数） [deg]
		double			rGenVelocity ;		// （中心から遠ざかる）初速 [pixel/sec]
		double			rGenVelocityRange ;	// 初速の幅（乱数） [pixel/sec]
		double			rShrink ;			// 初速減速率 [/sec]
		double			rRevSpeed ;			// 回転速度 [deg/sec]
		double			rRevSpeedRange ;	// 回転速度の幅（乱数）[deg/sec]
		double			rZoom ;				// 拡大率
		double			rZoomRange ;		// 拡大率の幅（乱数）
		PARTICLE_FLICK	pfFlickness[2] ;	// 揺らぎ
		E3D_VECTOR_2D	vGenSpeed ;			// 初速ベクトル [pixel/sec]
		double			rGenSpeedRange ;	// 初速の幅（乱数）[pixel/sec]
		E3D_VECTOR_2D	vStream ;			// 流速 [pixel/sec]
		E3D_VECTOR_2D	vGravity ;			// 重力加速度 [pixel/sec/sec]
	} ;
	struct	PARTICLE
	{
		unsigned int	iParticleImage ;	// 画像番号
		unsigned int	nPastTime ;			// 経過時間 [ms]
		unsigned int	nAnimeTime ;		// アニメーション用時間 [ms]
		E3D_VECTOR_2D	vShow ;				// 表示座標
		E3D_VECTOR_2D	vPos ;				// 座標
		E3D_VECTOR_2D	vVelocity ;			// （初）速度 [pixel/sec]
		E3D_VECTOR_2D	vAcceleration ;		// 現在加速度 [pixel/sec/sec]
											// 実速度 = vVelocity + vAcceleration
		double			rRevAngle ;			// 回転角度 [deg]
		double			rRevSpeed ;			// 回転速度 [deg/sec]
		double			rZoom ;				// 拡大率
		PARTICLE_FLICK	pfFlickness[2] ;	// 揺らぎ
		E3D_VECTOR_2D	vFlickUnit ;		// 揺らぎ基底ベクトル
		double			rFlicknessPhase ;	// 揺らぎ位相
	} ;
	struct	GENERATOR_PARAM
	{
		EGL_POINT		ptMaskCenter ;		// マスクの基準座標 [/10000H]
		EGL_SIZE		szMaskZoom ;		// マスク画像の拡大率 [/10000H]
		EGL_SIZE		szPointStep ;		// 発生座標ステップ [/10000H]
		int				nFlags ;			// フラグ
		int				nGenerationPoints ;	// 発生点数
		E3D_VECTOR_2D	vRay ;				// 光線ベクトル
	} ;
	enum	GeneratorFlags
	{
		gfNegativeMask		= 0x01,		// マスクは負論理
		gfGenerationPoints	= 0x02,		// 発生点を予め生成する
		gfRaySide			= 0x04,		// マスクの光線側に発生させる
	} ;

protected:
	unsigned int		m_nGenCount ;		// パーティクル生成数 [/100sec]
	DWORD				m_dwRandom ;		// 乱数の種

	class	EParticleImage
	{
	public:
		ECSReference		m_refImage ;		// 粒子画像
		ECSResource *		m_prsImage ;
		EGLAnimation *		m_pParticleImage ;
		EGL_POINT			m_ptHotspot ;		// 粒子ホットスポット
	public:
		EParticleImage( void )
			: m_prsImage( NULL ), m_pParticleImage( NULL ) { }
	} ;
	EObjArray<EParticleImage>
						m_lstImages ;		// パーティクル画像リスト

	EObjArray<E3D_VECTOR_2D>
						m_lstGeneratorPos ;		// 発生ポイント配列

	ECSReference		m_refGeneratorMask ;	// 発生領域マスク
	PEGL_IMAGE_INFO		m_pGeneratorMask ;
	GENERATOR_PARAM		m_gpParam ;

	PARTICLE_PARAM		m_ppParam ;			// パラメータ
	EGL_RECT			m_rctValidated ;	// 粒子有効領域
	EGL_RECT			m_rctParticle ;		// 粒子表示領域
	EObjArray<PARTICLE>	m_lstParticles ;	// 粒子配列

public:
	// 外接矩形を取得
	virtual EGL_RECT GetRectangle( void ) ;
	// 陰になる内接（最大）矩形取得
	bool GetHiddenRectangle( EGL_RECT & rect ) ;
	// スプライト描画
	virtual void MTDraw( HEGL_RENDER_POLYGON hRenderPoly ) ;
	// アニメーション進行
	virtual ESLError OnAdvanceAnimation( unsigned int nTime ) ;

public:
	// 乱数生成
	long int Random( long int nLimit ) ;
	// パーティクルの座標更新
	void AdvanceParticlePosition
		( PARTICLE * pp, int nPastTime ) const ;

public:
	// パーティクル画像を設定
	ESLError SetParticleImageResource
		( ECSResource * pImage,
			const EGL_POINT * pHotspot = NULL, int nIndex = 0 ) ;
	ESLError SetParticleImage
		( EGLAnimation * pImage,
			const EGL_POINT * pHotspot = NULL, int nIndex = 0 ) ;
	// パーティクル画像取得
	EGLAnimation * GetParticleImage( int nIndex = 0 ) ;
	// パーティクル画像の最大数を設定する
	void SetParticleImageLimit( int nLimit ) ;
	// パーティクルパラメータ設定
	void SetParticleParameter( const PARTICLE_PARAM & param ) ;
	// パーティクルパラメータ取得
	const PARTICLE_PARAM & GetParticleParameter( void ) const
		{
			return	m_ppParam ;
		}
	// パーティクル発生領域マスク設定
	ESLError SetGeneratorAreaMaskResource
		( ECSResource * pMask, const GENERATOR_PARAM * pgp = NULL ) ;
	ESLError SetGeneratorAreaMask
		( PEGL_IMAGE_INFO pMask, const GENERATOR_PARAM * pgp = NULL ) ;
	// パーティクル発生領域マスク取得
	PEGL_IMAGE_INFO GetGeneratorMask( GENERATOR_PARAM * pgp = NULL ) ;
	// 画面有効域設定
	void SetParticleRectangle( const EGL_RECT & rctValidated ) ;
	// 画面有効域取得
	const EGL_RECT & GetParticleRectangle( void )
		{
			return	m_rctValidated ;
		}
	// パーティクルを生成する
	void CreateParticle( int nCount ) ;
	// パーティクル発生座標を生成する
	E3D_VECTOR_2D * GenerateParticlePosition( E3D_VECTOR_2D & vPos ) ;
	// パーティクル生成数を設定する（/100sec）
	void SetParticleGenerator( int nCount ) ;

public:		// 通常のオブジェクト処理
	// オブジェクトの型名を取得する
	virtual const wchar_t * GetTypeName( void ) const ;
	virtual ECSObject * GetTypeOf( const wchar_t * pwszTypeName ) ;
	// オブジェクトを複製
	virtual ECSObject * Duplicate( void ) ;
	// メンバ関数インデックス取得
	virtual ESLError GetFunction
		( ECSContext & context, int & nIndex, const wchar_t * pwszName ) ;
	// メンバ関数呼び出し
	virtual ESLError CallFunction
		( ECSContext & context,
			int nIndex, ECSObjArray<ECSObject> & lstArg ) ;

public:		// シリアル化のための関数（システムによって必要）
	// 全てのメンバ変数にインデックスを振る
	virtual void IndexAllMember( void ) ;
	// 全てのメンバ変数の参照を解消する
	virtual void CleanupAllReference( ECSContext & context ) ;
	// 全てのメンバ変数の参照を解決する
	virtual ESLError CommitAllReference( ECSContext & context ) ;
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;

public:
	// メンバ関数プロトタイプ
	typedef	ESLError (ECSParticleSprite::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[8] ;
	static const PFUNC_CALL	m_pfnCallFunc[7] ;

	ESLError Call_SetParticleImageLimit
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetParticleImage
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetParticleParameter
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetParticleGeneratorMask
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetParticleRectangle
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CreateParticle
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetParticleGenerator
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// ファイル入出力用オブジェクト
//////////////////////////////////////////////////////////////////////////////

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
//////////////////////////////////////////////////////////////////////////////

class	ECSThread	: public ECSObject, public EGLSThread
{
public:
	// 構築関数
	ECSThread( void ) ;
	ECSThread( ECSContext & context ) ;
	// 消滅関数
	virtual ~ECSThread( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO2( ECSThread, ECSObject, EGLSThread )

protected:
	ECSContext *	m_pPrimaryContext ;
	ECSContext		m_context ;
	DWORD			m_dwSuspendCount ;
	EWideString		m_wstrExceptionFunc ;

	ECSReference	m_refThreadProc ;

	ECSThread *		m_pPrevThread ;
	ECSThread *		m_pNextThread ;

	struct	SAVE_DATA
	{
		ECSContext::ExecutionStatus	nStatus ;
//		DWORD						dwIP ;
		DWORD						dwSuspendCount ;
		DWORD						dwExceptionFuncLen ;
	} ;
	bool		m_fLoadedContext ;
	SAVE_DATA	m_sdRestore ;

public:
	// 実行イメージ関連付け
	virtual ESLError Initialize( ECSContext & context ) ;
	// スレッド関数呼び出し
	virtual ESLError BeginThread
		( DWORD dwFuncAddr, const ECSObjArray<ECSObject> & lstArg ) ;
	ESLError BeginThread
		( const wchar_t * pwszFuncName, const ECSObjArray<ECSObject> & lstArg ) ;
	ESLError BeginThread( ECSObject * pThreadProc ) ;
	// 実行を強制終了させる
	virtual ESLError AbortThread( DWORD dwTimeout = INFINITE ) ;
	// スレッドは実行中か？
	bool IsThreadRunning( void ) const ;
	// スクリプトの実行を一時停止する
	virtual ESLError SuspendThread( DWORD dwTimeout = INFINITE ) ;
	// 一時停止中のスクリプトを再開する
	virtual ESLError ResumeThread( void ) ;
	// コンテキスト取得
	ECSContext & GetContext( void )
		{
			return	m_context ;
		}
	// 例外エラー発生時の呼び出し関数を設定する
	void SetExceptionFunction( const wchar_t * pwszFuncName )
		{
			m_wstrExceptionFunc = pwszFuncName ;
		}
	// 例外エラー発生時に呼び出す関数名を取得する
	const wchar_t * GetExceptionFunction( void ) const
		{
			return	m_wstrExceptionFunc ;
		}

protected:
	// スレッド関数
	virtual DWORD ThreadProc( void ) ;

public:
	// オブジェクトの型名を取得する
	virtual const wchar_t * GetTypeName( void ) const ;
	virtual ECSObject * GetTypeOf( const wchar_t * pwszTypeName ) ;
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
	// メンバ変数インデックス取得
//	virtual ESLError GetVariableIndex( int & nIndex, ECSObject & obj ) ;
	virtual ESLError GetVariableIndex( int & nIndex, int iMember ) ;
	virtual ESLError GetVariableIndex
					( int & nIndex, const wchar_t * pwszMember ) ;
	// メンバ変数取得
	virtual ECSObject * GetVariableAt( int nIndex ) ;
	// メンバ変数設定
	virtual ECSObject * SetVariableAt( int nIndex, ECSObject * obj ) ;
	// メンバ関数インデックス取得
	virtual ESLError GetFunction
		( ECSContext & context, int & nIndex, const wchar_t * pwszName ) ;
	// メンバ関数呼び出し
	virtual ESLError CallFunction
		( ECSContext & context,
			int nIndex, ECSObjArray<ECSObject> & lstArg ) ;

public:		// シリアル化のための関数（システムによって必要）
	// 全てのメンバ変数にインデックスを振る
	virtual void IndexAllMember( void ) ;
	// 全てのメンバ変数の参照を解消する
	virtual void CleanupAllReference( ECSContext & context ) ;
	// 全てのメンバ変数の参照を解決する
	virtual ESLError CommitAllReference( ECSContext & context ) ;
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// セーブ処理を開始する
	virtual void OnBeginningSave( ECSContext & context ) ;
	// セーブ処理が全て完了した
	virtual void OnFinishedSave( ECSContext & context ) ;
	// ロード処理を開始する
	virtual void OnBeginningLoad( ECSContext & context ) ;
	// ロード処理が全て完了した
	virtual void OnFinishedLoad( ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;

public:
	// メンバ関数プロトタイプ
	typedef	ESLError (ECSThread::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[5] ;
	static const PFUNC_CALL	m_pfnCallFunc[4] ;
	// メンバ関数
	ESLError Call_BeginThread
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsThreadRunning
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetThreadResult
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetExceptionHandler
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

	friend	ECSContext ;

} ;


//////////////////////////////////////////////////////////////////////////////
// スレッド待機イベント
//////////////////////////////////////////////////////////////////////////////

class	ECSThreadEvent	: public	ECSObject
{
public:
	// 構築関数
	ECSThreadEvent( void ) ;
	// 消滅関数
	virtual ~ECSThreadEvent( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSThreadEvent, ECSObject )

protected:
	HANDLE		m_hEvent ;
	long int	m_nEvent ;

public:
	// イベントオブジェクト作成
	virtual ESLError CreateEvent( bool fInitState ) ;
	// イベントオブジェクト削除
	virtual ESLError DeleteEvent( void ) ;
	// イベントセット
	virtual void SetEvent( void ) ;
	// イベントリセット
	virtual void ResetEvent( void ) ;
	// イベント待機
	virtual ESLError WaitEvent( DWORD dwTimeout, ECSContext & context ) ;
	// イベント取得
	HANDLE GetEventHandle( void ) const
		{
			return	m_hEvent ;
		}
	long int GetEventState( void ) const
		{
			return	m_nEvent ;
		}

public:
	// オブジェクトの型名を取得する
	virtual const wchar_t * GetTypeName( void ) const ;
	virtual ECSObject * GetTypeOf( const wchar_t * pwszTypeName ) ;
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

public:		// シリアル化のための関数（システムによって必要）
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;

public:
	// メンバ関数プロトタイプ
	typedef	ESLError (ECSThreadEvent::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[7] ;
	static const PFUNC_CALL	m_pfnCallFunc[6] ;
	// メンバ関数
	ESLError Call_Create
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Delete
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Wait
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Set
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Reset
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Value
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// スレッド待機（ミューテックス）イベント
//////////////////////////////////////////////////////////////////////////////

class	ECSThreadMutex	: public	ECSThreadEvent
{
public:
	// 構築関数
	ECSThreadMutex( void ) ;
	// 消滅関数
	virtual ~ECSThreadMutex( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSThreadMutex, ECSThreadEvent )

protected:
	ECSReference	m_refOwnerThread ;

public:
	// イベントオブジェクト作成
	virtual ESLError CreateEvent( bool fInitState ) ;
	// イベントオブジェクト削除
	virtual ESLError DeleteEvent( void ) ;
	// イベントセット
	virtual void SetEvent( void ) ;
	// イベントリセット
	virtual void ResetEvent( void ) ;
	// イベント待機
	virtual ESLError WaitEvent( DWORD dwTimeout, ECSContext & context ) ;

public:
	// オブジェクトの型名を取得する
	virtual const wchar_t * GetTypeName( void ) const ;
	virtual ECSObject * GetTypeOf( const wchar_t * pwszTypeName ) ;
	// オブジェクトを複製
	virtual ECSObject * Duplicate( void ) ;
	// オブジェクトを代入
	virtual ESLError Move( ECSContext & context, ECSObject * obj ) ;

public:		// シリアル化のための関数（システムによって必要）
	// 全てのメンバ変数にインデックスを振る
	virtual void IndexAllMember( void ) ;
	// 全てのメンバ変数の参照を解消する
	virtual void CleanupAllReference( ECSContext & context ) ;
	// 全てのメンバ変数の参照を解決する
	virtual ESLError CommitAllReference( ECSContext & context ) ;
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;

} ;


#endif
