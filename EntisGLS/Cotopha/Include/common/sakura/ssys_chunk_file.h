
#if	!defined(__SAKURA2_CHUNK_FILE_H__)
#define	__SAKURA2_CHUNK_FILE_H__


namespace	SSystem
{
	//////////////////////////////////////////////////////////////////////////
	// 部分領域ファイルインターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SFileDomainInterface	: public SFileInterface
	{
	protected:
		SFileInterface *	m_pFile ;
		bool				m_flagFileOwner ;	// m_pFile の自動破棄
		long int			m_nFlags ;			// enum OpenFlag の組み合わせ
		uint64_t			m_baseDomain ;		// 領域情報
		uint64_t			m_lengthDomain ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SFileDomainInterface, SFileInterface )
		// 構築関数
		SFileDomainInterface( void ) ;
		SFileDomainInterface
			( SFileInterface * pFile,
				bool flagOwner, long int nFlags,
				uint64_t baseDomain, uint64_t lengthDomain ) ;
		// 消滅関数
		virtual ~SFileDomainInterface( void ) ;
		// ファイル関連付け
		void AttachFile
			( SFileInterface * pFile,
				bool flagOwner, long int nFlags,
				uint64_t baseDomain, uint64_t lengthDomain ) ;
		// ファイルが関連付けられているか？
		bool IsAttachedFile( void ) const
			{
				return	(m_pFile != NULL) ;
			}
		// 関連付けられてるファイル取得
		SFileInterface * GetAttachedFile( void ) const
			{
				return	m_pFile ;
			}
		// ファイルフラグ（enum OpenFlag の組み合わせ）の取得
		long int GetFileOpenFlags( void ) const
			{
				return	m_nFlags ;
			}
		// 書庫ファイルは新規書き出し用に開かれているか？
		bool IsFileCreatingMode( void ) const
			{
				return	(m_nFlags & SFileOpener::modeCreateFlag) != 0 ;
			}
		// 書庫ファイルは書き出し用に開かれているか？
		bool IsFileWritingMode( void ) const
			{
				return	(m_nFlags & SFileOpener::modeWrite) != 0 ;
			}

	public:
		// ファイルインターフェースの複製
		virtual SFileInterface * Duplicate( void ) const ;
		// ファイルから読み込み
		virtual size_t Read( void * ptrBuf, size_t nBytes ) ;
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;
		// シーク可能か否か？
		virtual bool IsSeekable( void ) const ;
		// ファイル長の取得
		virtual int64_t GetLength( void ) const ;
		// ファイルポインタを移動
		virtual int64_t Seek
			( int64_t posFile, SeekOrigin seekFrom = FromBegin ) ;
		// ファイルポインタを取得
		virtual int64_t GetPosition( void ) const ;
		// ファイルの終端を現在の位置に設定する
		virtual SError SetEndOfFile( void ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// チャンクファイル（Entis メディア複合ファイル形式）
	//////////////////////////////////////////////////////////////////////////

	class	SChunkFile	: public SFileDomainInterface
	{
	public:
		// ファイルヘッダ
		struct	FILE_HEADER
		{
			BYTE	bytSignature[8] ;		// ファイルシグネチャ
			DWORD	dwFileID ;				// ファイル識別子
			DWORD	dwReserved ;			// 予約＝０
			BYTE	bytFormatDesc[0x30] ;	// フォーマット名

			void SetHeaderInfo( DWORD idFile, const char * pszDesc ) ;
		} ;
		enum	FileIdentity
		{
			fidArchive			= 0x02000400,
			fidRasterizedImage	= 0x03000100,
			fidEGL3DModel		= 0x03001200,
			fidEGL3DModel2		= 0x03001201,
			fidEGL3DModelPose	= 0x03001300,
			fidEGL3DPoseLibrary	= 0x03001400,
			fidBitmapFont		= 0x04002000,
			fidUndefined		= -1
		} ;
		// チャンクヘッダ
		struct	CHUNK_HEADER
		{
			UINT64	idChunk ;		// 識別子
			UINT64	nLength ;		// 長さ
		} ;
		// チャンク
		struct	CHUNK_INFO	: public CHUNK_HEADER
		{
			UINT64	nPos ;			// 位置
		} ;
		// チャンク識別子一致判定
		static bool IsEqualChunkID
				( UINT64 idChunk, const char * pszChunk ) ;

	protected:
		SSystem::SArray<CHUNK_INFO>	m_nestChunk ;	// チャンクネスト

		FILE_HEADER		m_fhHeader ;	// ファイルヘッダ
		CHUNK_INFO *	m_pChunk ;		// 現在のチャンク

		static const BYTE	m_bytDefaultSignature[8] ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SChunkFile, SFileDomainInterface )
		// 構築関数
		SChunkFile( void ) ;
		// 消滅関数
		virtual ~SChunkFile( void ) ;

	public:
		// ファイルを開く
		SError OpenChunkFile
			( SFileInterface * pFile, bool flagOwner = false,
				long int nFlags = 0, const FILE_HEADER * pfhHeader = NULL ) ;
		// リソースを解放する
		virtual void Close( void ) ;
		// ファイルヘッダ取得
		const FILE_HEADER & GetFileHeader( void ) const
			{
				return	m_fhHeader ;
			}

	public:
		// チャンクを開く
		virtual SError DescendChunk( const char * pszChunkID = NULL ) ;
		// チャンクを閉じる
		virtual SError AscendChunk( void ) ;
		// 現在のチャンク識別子を取得
		UINT64 GetCurrentChunkID( void ) const
			{
				ESLAssert( m_pChunk != NULL ) ;
				return	m_pChunk->idChunk ;
			}
		// 現在のチャンクの一致判定
		bool IsEqualCurrentChunkID( const char * pszChunk ) const
			{
				ESLAssert( m_pChunk != NULL ) ;
				return	IsEqualChunkID( m_pChunk->idChunk, pszChunk ) ;
			}
		// 現在のチャンク長を取得
		UINT64 GetCurrentChunkLength( void ) const
			{
				ESLAssert( m_pChunk != NULL ) ;
				return	m_pChunk->nLength ;
			}
		// ファイルポインタの更新通知（書き込みモード時のチャンクサイズ反映）
		void UpdateFilePointer( void ) ;

	public:
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;
		// ファイルポインタを移動
		virtual int64_t Seek
			( int64_t posFile, SeekOrigin seekFrom = FromBegin ) ;
		// ファイルの終端を現在の位置に設定する
		virtual SError SetEndOfFile( void ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// チャンクファイル（Entis メディア複合ファイル形式）エディタ
	//////////////////////////////////////////////////////////////////////////

	class	SChunkFileEditor	: public SChunkFile
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SChunkFileEditor, SChunkFile )
		// 構築関数
		SChunkFileEditor( void ) ;
		// 消滅関数
		virtual ~SChunkFileEditor( void ) ;

	protected:
		SChunkFile	m_cfSrc ;
		bool		m_flagWritten ;

	public:
		// ファイルを開く
		SError OpenChunkFile
			( SFileInterface * pDstFile, bool flagDstOwner,
				SFileInterface * pSrcFile, bool flagSrcOwner ) ;
		// リソースを解放する
		virtual void Close( void ) ;

	public:
		// ファイルから読み込み
		virtual size_t Read( void * ptrBuf, size_t nBytes ) ;
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;
		// ファイル長の取得
		virtual int64_t GetLength( void ) const ;
		// ファイルポインタを移動
		virtual int64_t Seek
			( int64_t posFile, SeekOrigin seekFrom = FromBegin ) ;
		// ファイルポインタを取得
		virtual int64_t GetPosition( void ) const ;

	public:
		// チャンクを開く
		virtual SError DescendChunk( const char * pszChunkID = NULL ) ;
		// チャンクを閉じる
		virtual SError AscendChunk( void ) ;

	public:
		// 現在のチャンクの残りチャンクを複製
		virtual SError CopyAllSubChunks( void ) ;

	} ;

}


#endif
