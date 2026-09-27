
#if	!defined(__SAKURAGL_ERISA_MEDIA_FILE_H__)
#define	__SAKURAGL_ERISA_MEDIA_FILE_H__

#include <sakura/ssys_queue_buffer.h>

namespace	ERISA
{
	//////////////////////////////////////////////////////////////////////////
	// ERI メディアファイル
	//////////////////////////////////////////////////////////////////////////

	class	SGLMediaFile	: public SSystem::SChunkFile
	{
	public:
		// タグ情報インデックス
		enum	TagIndex
		{
			tagTitle,				// 曲名
			tagVocalPlayer,			// 歌手・演奏者
			tagComposer,			// 作曲者
			tagArranger,			// 編曲者
			tagSource,				// 出展・アルバム
			tagTrack,				// トラック
			tagReleaseDate,			// リリース年月日
			tagGenre,				// ジャンル
			tagRewindPoint,			// ループポイント[,ループ終端]
			tagLoopEndPoint,		// ループ終端
			tagHotSpot,				// ホットスポット
			tagResolution,			// 解像度
			tagComment,				// コメント
			tagWords,				// 歌詞
			tagReferenceFile,		// 画像参照ファイル
			tagMax
		} ;
		// タグ情報文字列
		static const wchar_t *	m_pwszTagName[tagMax] ;

		// タグ・エントリ
		class	STagEntry
		{
		public:
			SSystem::SString	m_tag ;
			SSystem::SString	m_contents ;
		public:
			// 構築関数
			STagEntry( void ) {}
			STagEntry( const STagEntry& tag )
				: m_tag( tag.m_tag ), m_contents( tag.m_contents ) {}
			// 代入
			const STagEntry& operator = ( const STagEntry& tag )
			{
				m_tag = tag.m_tag ;
				m_contents = tag.m_contents ;
				return	*this ;
			}
		} ;

		// タグ情報
		class	STagInfo
		{
		public:
			SSystem::SObjectArray<STagEntry>	m_tags ;
		public:
			// 構築関数
			STagInfo( void ) {}
			// タグ情報を解釈
			void ParseTagInfo( const wchar_t * pwszDesc ) ;
			// タグ情報をフォーマット
			void FormatTagInfo( SSystem::SString& strDesc ) const ;
			// タグを追加する
			void AddTag( TagIndex iTag, const wchar_t * pwszContents ) ;
			// タグ情報のクリア
			void DeleteContents( void ) ;
			// タグ情報取得
			const wchar_t * GetTagContents( const wchar_t * pwszTag ) const ;
			const wchar_t * GetTagContents( TagIndex iTag ) const ;
			STagEntry * GetTagAs( const wchar_t * pwszTag ) const ;
			// トラック番号を取得
			int GetTrackNumber( void ) const ;
			// リリース年月日を取得
			SSystem::SError GetReleaseDate( SSystem::DATE_TIME& date ) const ;
			// ループポイントを取得
			int64_t GetRewindPoint( size_t iEntry = 0 ) const ;
			int64_t GetLoopEndPoint( void ) const ;
			// ホットスポットを取得
			SSystem::SError GetHotSpot( SakuraGL::SGLPoint& ptHotSpot ) const ;
			// 解像度を取得
			long int GetResolution( void ) const ;
		} ;

		// シーケンス・エントリ
		struct	SEQUENCE_DELTA
		{
			uint32_t	nFrame ;
			uint32_t	nDuration ;
		} ;

		// 読み込まれた情報フラグ
		enum	ContentsFlag
		{
			readFileHeader		= 0x00000001,
			readPreviewInfo		= 0x00000002,
			readImageInfo		= 0x00000004,
			readSoundInfo		= 0x00000008,
			readCopyright		= 0x00000010,
			readDescription		= 0x00000020,
			readPaletteTable	= 0x00000040,
			readSequenceTable	= 0x00000080,
		} ;
		uint32_t				m_flagsRead ;		// complex enum ContentsFlag
		// ファイルヘッダ
		ERISA::ERI_FILE_HEADER	m_eriFileHeader ;
		// プレビュー画像情報ヘッダ
		ERISA::ERI_INFO_HEADER	m_eriPreviewInfo ;
		// 画像情報ヘッダ
		ERISA::ERI_INFO_HEADER	m_eriInfoHeader ;
		// 音声情報ヘッダ
		ERISA::MIO_INFO_HEADER	m_mioInfoHeader ;
		// パレットテーブル
		SSystem::SArray<SakuraGL::SGLPalette>
								m_tablePalette ;
		// 著作権情報
		SSystem::SString		m_strCopyright ;
		// コメント
		SSystem::SString		m_strDescription ;
		// シーケンステーブル
		SSystem::SArray<SEQUENCE_DELTA>
								m_tableSequence ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLMediaFile, SChunkFile )
		// 構築関数
		SGLMediaFile( void ) ;
		// 消滅関数
		virtual ~SGLMediaFile( void ) ;

	public:
		// ファイルのオープン方法
		enum	OpenType
		{
			openRoot,			// ルートレコードを開くだけ
			readHeader,			// 情報ヘッダレコードを読み込んで値を検証
			openStream,			// ヘッダを読み込みストリームレコードを開く
			openImageData		// 画像データレコードを開く
		} ;
		// メディアファイルを開く
		SSystem::SError OpenMediaFile
			( SSystem::SFileInterface * pFile,
				OpenType type = openImageData,
				bool flagOwner = false, long int nFlags = 0 ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ERI メディアファイル出力
	//////////////////////////////////////////////////////////////////////////

	class	SGLMediaFileWriter	: public SGLMediaFile
	{
	protected:
		// 状態
		enum	WriterStatus
		{
			wsNotOpened,
			wsOpened,
			wsWritingHeader,
			wsWritingStream
		} ;
		WriterStatus	m_wsStatus ;		// ステータス

		// ヘッダ付加情報
		int64_t			m_fposEndOfHeader ;

		// フレーム番号
		bool			m_fWithSeqTable ;	// シーケンステーブル使用
		size_t			m_nKeyFrame ;		// キーフレーム
		size_t			m_nBidirectKey ;	// B ピクチャ間隔
		size_t			m_nKeyWave ;		// キーウェーブ
		size_t			m_nFrameCount ;		// 出力済みフレーム総数
		size_t			m_nWaveCount ;		// ウェーブ出力回数（ブロック数）
		size_t			m_nDiffFrames ;		// 差分フレーム蓄積数（Bピクチャ）

		// 音声出力情報
		int64_t			m_fposMioHeader ;		// 音声情報ヘッダのファイル位置
		size_t			m_nOutputWaveSamples ;	// 出力済みサンプル数

		// 音声出力バッファ
		bool					m_fKeyWaveBlock ;
		SSystem::SQueueBuffer	m_bufWaveBuffer ;
		size_t					m_nWaveBufSamples ;

		// 圧縮オブジェクト
		SSystem::SSmartPointer<SGLImageEncoder>
									m_pencImage1 ;
		SSystem::SSmartPointer<SGLImageEncoder>
									m_pencImage2 ;
		SSystem::SSmartPointer<SGLSoundEncoder>
									m_pencSound ;

		// 差分処理用フレームバッファ
		SSystem::SSmartPointer
			<SakuraGL::SGLImageObject>	m_pLastImage ;	// 直前フレーム
		SSystem::SSmartPointer
			<SakuraGL::SGLImageObject>	m_pNextImage ;	// 次のI/Pフレーム
		SSystem::SSmartPointer
			<SakuraGL::SGLImageObject>	m_pCurImage ;	// 現在フレームの複製
		SSystem::SObjectArray
			<SakuraGL::SGLImageObject>	m_arrFrameBuf ;	// B ピクチャ用
		SSystem::SArray<uint32_t>		m_arrEncFlags ;	// m_arrFrameBuf 要素の各エンコードフラグ

		// 画像の圧縮パラメータ
		SGLImageEncoder::Parameter	m_iencp_i ;
		SGLImageEncoder::Parameter	m_iencp_p ;
		SGLImageEncoder::Parameter	m_iencp_b ;

		// 音声の圧縮パラメータ
		SGLSoundEncoder::Parameter	m_sencp ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLMediaFileWriter, SGLMediaFile )
		// 構築関数
		SGLMediaFileWriter( void ) ;
		// 消滅関数
		virtual ~SGLMediaFileWriter( void ) ;

	public:
		// ファイルタイプ
		enum	MediaFileIdentity
		{
			fidImage,
			fidSound,
			fidMovie
		} ;
		// ファイルを開く
		SSystem::SError OpenMediaFile
			( SSystem::SFileInterface * pFile,
				MediaFileIdentity fidType,
				bool flagOwner = false, long int nFlags = 0 ) ;
		// リソースを解放する
		virtual void Close( void ) ;

	public:
		// ファイルヘッダを開く
		SSystem::SError BeginFileHeader
			( size_t nKeyFrame, size_t nKeyWave, size_t nBidirectKey = 3 ) ;
		// プレビュー画像情報ヘッダを書き出す
		SSystem::SError WritePreviewInfo( const ERISA::ERI_INFO_HEADER & eih ) ;
		// 画像情報ヘッダを書き出す
		SSystem::SError WriteEriInfoHeader( const ERISA::ERI_INFO_HEADER & eih ) ;
		// 音声情報ヘッダを書き出す
		SSystem::SError WriteMioInfoHeader( const ERISA::MIO_INFO_HEADER & mih ) ;
		// 著作権情報を書き出す
		SSystem::SError WriteCopyright( const wchar_t * pwszCopyright, ssize_t nLength = -1 ) ;
		// コメントを書き出す
		SSystem::SError WriteDescription( const wchar_t * pwszDescription, ssize_t nLength = -1 ) ;
		// シーケンステーブルを書き出す
		SSystem::SError WriteSequenceTable
			( SGLMediaFile::SEQUENCE_DELTA * pSequence, size_t nLength ) ;
		// ファイルヘッダを閉じる
		void EndFileHeader( void ) ;

	public:
		// 画像の圧縮パラメータを設定する
		void SetImageCompressionParameter
				( const SGLImageEncoder::Parameter & iencp ) ;
		// 音声の圧縮パラメータを設定する
		void SetSoundCompressionParameter
				( const SGLSoundEncoder::Parameter & sencp ) ;

	public:
		// ストリームを開始する
		virtual SSystem::SError BeginStream( void ) ;
		// パレットテーブルを書き出す
		virtual SSystem::SError WritePaletteTable
			( const SakuraGL::SGLPalette * paltbl, size_t nLength ) ;
		// プレビュー画像を出力する
		virtual SSystem::SError WritePreviewData
				( SakuraGL::SGLImageObject & image, uint32_t flagsEncode ) ;
		// 音声データを出力する
		virtual SSystem::SError WriteWaveData
				( const void * ptrWaveBuf, size_t nSampleCount ) ;
		// 画像データを出力する
		SSystem::SError WriteImageData
				( SakuraGL::SGLImageObject & image, uint32_t flagsEncode ) ;
		virtual SSystem::SError WriteImageData
				( SakuraGL::SGLImageObject & image ) ;
		// ストリームを閉じる
		virtual SSystem::SError EndStream( uint32_t msecTotalTime ) ;

	protected:
		// B ピクチャを圧縮して書き出す
		SSystem::SError WriteBirectionalFrames( void ) ;
		// 音声データを圧縮して書き出す
		SSystem::SError WriteWaveBuffer( void ) ;
		// 画像バッファを生成
		SakuraGL::SGLImageObject *
			CreateImageBuffer( const ERISA::ERI_INFO_HEADER & eih ) ;
	} ;

}

#endif

