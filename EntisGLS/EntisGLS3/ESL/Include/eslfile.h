
/*****************************************************************************
                   Entis Standard Library declarations
 ----------------------------------------------------------------------------

	In this file, the file object classes definitions.

	Copyright (C) 1998-2010 Leshade Entis.  All rights reserved.

 ****************************************************************************/


#if	!defined(__ESLFILE_H__)
#define	__ESLFILE_H__	1

/*****************************************************************************
                           ファイル抽象クラス
 ****************************************************************************/

class	ESLFileObject : public	ESLObject
{
public:
	// 構築関数
	ESLFileObject( void ) ;
	// 消滅関数
	virtual ~ESLFileObject( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ESLFileObject, ESLObject )

protected:
	CRITICAL_SECTION	m_cs ;
	int					m_nAttribute ;

public:
	// スレッド排他アクセス用関数
	void Lock( void ) const ;
	void Unlock( void ) const ;

public:
	// 属性
	enum	OpenFlag
	{
		modeCreateFlag	= 0x0001,
		modeCreate		= 0x0005,
		modeRead		= 0x0002,
		modeWrite		= 0x0004,
		modeReadWrite	= 0x0006,
		shareRead		= 0x0010,
		shareWrite		= 0x0020,
	} ;
protected:
	void SetAttribute( int nAttribute )
		{
			m_nAttribute = nAttribute ;
		}
public:
	int GetAttribute( void ) const
		{
			return	m_nAttribute ;
		}

public:
	// ファイルオブジェクトを複製する
	virtual ESLFileObject * Duplicate( void ) const = 0 ;

public:
	// ファイルから読み込む
	virtual unsigned long int Read
		( void * ptrBuffer, unsigned long int nBytes ) = 0 ;
	// ファイルへ書き出す
	virtual unsigned long int Write
		( const void * ptrBuffer, unsigned long int nBytes ) = 0 ;

public:
	// シーク方法
	enum	SeekOrigin
	{
		FromBegin	= FILE_BEGIN,
		FromCurrent	= FILE_CURRENT,
		FromEnd		= FILE_END
	} ;
	// ファイルの長さを取得
	virtual unsigned long int GetLength( void ) const = 0 ;
	// ファイルポインタを移動
	virtual unsigned long int Seek
		( long int nOffsetPos, SeekOrigin fSeekFrom ) = 0 ;
	// ファイルポインタを取得
	virtual unsigned long int GetPosition( void ) const = 0 ;
	// ファイルの終端を現在の位置に設定する
	virtual ESLError SetEndOfFile( void ) ;

public:	// 64 ビットファイル長
	// ファイルの長さを取得
	virtual UINT64 GetLargeLength( void ) const ;
	// ファイルポインタを移動
	virtual UINT64 SeekLarge
		( INT64 nOffsetPos, SeekOrigin fSeekFrom ) ;
	// ファイルポインタを取得
	virtual UINT64 GetLargePosition( void ) const ;

protected:
	ESLFileObject *	m_pOpener ;

public:
	// 新規にファイルを開く
	virtual ESLFileObject *
		OpenFileObject( const wchar_t * pwszFileName, int nOpenFlags ) ;
	// ファイルオープナーを設定する
	void AttachFileOpener( ESLFileObject * pOpener )
		{
			m_pOpener = pOpener ;
		}
	// ファイルオープナー取得
	ESLFileObject * GetFileOpener( void ) const
		{
			return	m_pOpener ;
		}

} ;


/*****************************************************************************
                           生ファイルクラス
 ****************************************************************************/

class	ERawFile : public	ESLFileObject
{
protected:
	HANDLE			m_hFile ;
	EString			m_strFilePath ;
	const char *	m_pszFileTitle ;

public:
	// 構築関数
	ERawFile( void ) ;
	// 消滅関数
	virtual ~ERawFile( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ERawFile, ESLFileObject )

public:
	// ファイルを開く
	ESLError Open( const char * pszFileName, int nOpenFlags ) ;
	// ファイルハンドルを複製してファイルオブジェクトに関連付ける
	ESLError Create( HANDLE hFile, int nOpenFlags ) ;
	// ファイルを閉じる
	void Close( void ) ;
	// ファイルオブジェクトを複製する
	virtual ESLFileObject * Duplicate( void ) const ;

public:
	// ファイルからデータを読み込む
	virtual unsigned long int Read
		( void * ptrBuffer, unsigned long int nBytes ) ;
	// ファイルへデータを書き出す
	virtual unsigned long int Write
		( const void * ptrBuffer, unsigned long int nBytes ) ;

public:
	// ファイルの長さを取得する
	virtual unsigned long int GetLength( void ) const ;
	// ファイルポインタを移動する
	virtual unsigned long int Seek
		( long int nOffsetPos, SeekOrigin fSeekFrom ) ;
	// ファイルポインタを取得する
	virtual unsigned long int GetPosition( void ) const ;
	// ファイルの終端を現在の位置に設定する
	virtual ESLError SetEndOfFile( void ) ;

public:	// 64 ビットファイル長
	// ファイルの長さを取得
	virtual UINT64 GetLargeLength( void ) const ;
	// ファイルポインタを移動
	virtual UINT64 SeekLarge
		( INT64 nOffsetPos, SeekOrigin fSeekFrom ) ;
	// ファイルポインタを取得
	virtual UINT64 GetLargePosition( void ) const ;

public:
	// 新規にファイルを開く
	virtual ESLFileObject *
		OpenFileObject( const wchar_t * pwszFileName, int nOpenFlags ) ;

public:
	// ファイルハンドルを取得する
	HANDLE GetFileHandle( void ) const
		{
			return	m_hFile ;
		}
	operator HANDLE ( void ) const
		{
			return	m_hFile ;
		}
	// ファイルが開かれているか？
	bool IsFileOpened( void ) const
		{
			return	(m_hFile != INVALID_HANDLE_VALUE) ;
		}
	// ファイルのタイムスタンプを取得する
	ESLError GetFileTime
		( LPSYSTEMTIME lpCreationTime,
			LPSYSTEMTIME lpLastAccessTime, LPSYSTEMTIME lpLastWriteTime ) ;
	// ファイルのタイムスタンプを設定する
	ESLError SetFileTime
		( const SYSTEMTIME * lpCreationTime,
			const SYSTEMTIME * lpLastAccessTime,
				const SYSTEMTIME * lpLastWriteTime ) ;

public:
	// ファイルフルパスを取得する
	const char * GetFilePath( void ) const
		{
			return	(const char *) m_strFilePath ;
		}
	// ファイル名を取得する
	const char * GetFileTitle( void ) const
		{
			return	 m_pszFileTitle ;
		}

} ;


/*****************************************************************************
                         メモリファイルクラス
 ****************************************************************************/

class	EMemoryFile : public	ESLFileObject
{
protected:
	void *				m_ptrMemory ;
	unsigned long int	m_nLength ;
	unsigned long int	m_nPosition ;
	unsigned long int	m_nBufferSize ;

public:
	// 構築関数
	EMemoryFile( void ) ;
	// 消滅関数
	virtual ~EMemoryFile( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EMemoryFile, ESLFileObject )

public:
	// 読み書き可能なメモリファイルを作成する
	ESLError Create( unsigned long int nLength ) ;
	// 読み込み専用のメモリファイルを作成する
	ESLError Open( const void * ptrMemory, unsigned long int nLength ) ;
	// 内部リソースを解放する
	void Delete( void ) ;
	// 内部メモリへのポインタを取得する
	void * GetBuffer( void ) const
		{
			return	m_ptrMemory ;
		}

	// メモリファイルを複製する
	virtual ESLFileObject * Duplicate( void ) const ;

public:
	// メモリファイルからデータを転送する
	virtual unsigned long int Read
		( void * ptrBuffer, unsigned long int nBytes ) ;
	// メモリファイルへデータを転送する
	virtual unsigned long int Write
		( const void * ptrBuffer, unsigned long int nBytes ) ;

public:
	// メモリファイルの長さを取得する
	virtual unsigned long int GetLength( void ) const ;
	// ファイルポインタを移動する
	virtual unsigned long int Seek
		( long int nOffsetPos, SeekOrigin fSeekFrom ) ;
	// ファイルポインタを取得する
	virtual unsigned long int GetPosition( void ) const ;
	// ファイルの終端を現在の位置に設定する
	virtual ESLError SetEndOfFile( void ) ;

} ;


#if	!defined(COMPACT_NOA_DECODER)

/*****************************************************************************
                 動的メモリマップファイルバッファクラス
 ****************************************************************************/

class	EFileMappedBuffer	: public	ESLFileObject
{
public:
	// 構築関数
	EFileMappedBuffer( void ) ;
	// 消滅関数
	virtual ~EFileMappedBuffer( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EFileMappedBuffer, ESLFileObject )

public:
	enum
	{
		BUFFER_SCALE	= 18,					// 256KB
		BUFFER_SIZE		= (1 << BUFFER_SCALE),
		BUFFER_MASK		= (BUFFER_SIZE - 1),
	} ;
	struct	BUFFER
	{
		BYTE *	pbytBuf ;
		int		nLastAccess ;
		bool	fLoaded ;
		bool	fModified ;

		BUFFER( void )
			: pbytBuf(NULL), fLoaded(false), fModified(false) {}
		~BUFFER( void )
			{	Release( ) ;	}
		BYTE * Allocate( void )
			{
				if ( pbytBuf == NULL )
				{
					pbytBuf =
						(BYTE*) ::VirtualAlloc
							( NULL, BUFFER_SIZE, MEM_COMMIT, PAGE_READWRITE ) ;
				}
				return	pbytBuf ;
			}
		void Release( void )
			{
				if ( pbytBuf != NULL )
				{
					::VirtualFree( pbytBuf, 0, MEM_RELEASE ) ;
					pbytBuf = NULL ;
				}
			}
	} ;

protected:
	EObjArray<BUFFER>	m_lstBuffer ;		// バッファ配列
	ULONG				m_nLoadedPages ;	// ロード済みページ数
	ULONG				m_nCacheSize ;		// キャッシュページ数
	int					m_nAccessCounter ;

	ESLFileObject *		m_pfile ;			// マッピングファイル
	UINT64				m_nPointer ;		// ファイルポインタ
	UINT64				m_nLength ;			// ファイル長

	enum	BufferReferenceMode
	{
		modeNothing			= -1,
		modeReadDirect,
		modeWriteDirect,
		modeRead,
		modeWrite,
	} ;
	BufferReferenceMode	m_mode ;
	UINT64				m_nRefAddr ;		// バッファ参照アドレス
	ULONG				m_nRefLength ;		// バッファ参照長
	EStreamBuffer		m_bufTemp ;			// テンポラリバッファ
	BYTE *				m_pbytTempBuf ;

public:
	// ファイルオブジェクトを複製する
	virtual ESLFileObject * Duplicate( void ) const ;
	// ファイルから読み込む
	virtual unsigned long int Read
		( void * ptrBuffer, unsigned long int nBytes ) ;
	// ファイルへ書き出す
	virtual unsigned long int Write
		( const void * ptrBuffer, unsigned long int nBytes ) ;
	// ファイルの長さを取得
	virtual unsigned long int GetLength( void ) const ;
	// ファイルポインタを移動
	virtual unsigned long int Seek
		( long int nOffsetPos, SeekOrigin fSeekFrom ) ;
	// ファイルポインタを取得
	virtual unsigned long int GetPosition( void ) const ;
	// ファイルの終端を現在の位置に設定する
	virtual ESLError SetEndOfFile( void ) ;
	// ファイルの長さを取得
	virtual UINT64 GetLargeLength( void ) const ;
	// ファイルポインタを移動
	virtual UINT64 SeekLarge
		( INT64 nOffsetPos, SeekOrigin fSeekFrom ) ;
	// ファイルポインタを取得
	virtual UINT64 GetLargePosition( void ) const ;

public:
	// バッファ新規作成
	ESLError OpenBuffer( ESLFileObject * pfile, ULONG nCacheSize = 0 ) ;
	// バッファを開放する
	void CloseBuffer( void ) ;
	// キャッシュされている内容をファイルへ書き出す
	ESLError WriteBackFile( void ) ;
	// 読み出し用バッファを参照する
	const BYTE * GetBuffer( UINT64 nAddr, ULONG & nBytes ) ;
	// 読み出し用バッファを開放する
	void ReleaseBuffer( void ) ;
	// 書き出し用バッファを参照する
	BYTE * PutBuffer( UINT64 nAddr, ULONG nBytes ) ;
	// 書き出し用バッファを開放する
	void FlushBuffer( ULONG nBytes ) ;

protected:
	// ブロックをロードする
	BUFFER * LoadPageBuffer( unsigned int nPageIndex ) ;
	// ロードしているページキャッシュを解放する
	void UnloadOldPageCache( void ) ;

} ;



/*****************************************************************************
                 ストリーミングバッファファイルクラス
 ****************************************************************************/

class	EStreamFileBuffer	: public ESLFileObject, public EStreamBuffer
{
public:
	// 構築関数
	EStreamFileBuffer( void ) ;
	// 消滅関数
	virtual ~EStreamFileBuffer( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO2( EStreamFileBuffer, ESLFileObject, EStreamBuffer )

public:
	// ファイルオブジェクトを複製する
	virtual ESLFileObject * Duplicate( void ) const ;

public:
	// ファイルから読み込む
	virtual unsigned long int Read
		( void * ptrBuffer, unsigned long int nBytes ) ;
	// ファイルへ書き出す
	virtual unsigned long int Write
		( const void * ptrBuffer, unsigned long int nBytes ) ;

public:
	// ファイルの長さを取得
	virtual unsigned long int GetLength( void ) const ;
	// ファイルポインタを移動
	virtual unsigned long int Seek
		( long int nOffsetPos, SeekOrigin fSeekFrom ) ;
	// ファイルポインタを取得
	virtual unsigned long int GetPosition( void ) const ;

} ;


/*****************************************************************************
                 同期ストリーミングバッファファイルクラス
 ****************************************************************************/

class	ESyncStreamFile	: public	ESLFileObject
{
public:
	// 構築関数
	ESyncStreamFile( void ) ;
	// 消滅関数
	virtual ~ESyncStreamFile( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ESyncStreamFile, ESLFileObject )

public:
	enum
	{
		BUFFER_SIZE = 0x4000,
		BUFFER_SCALE = 14,
		BUFFER_MASK = 0x3FFF
	} ;
protected:
	class	EBuffer
	{
	public:
		BYTE *			pbytBuf ;
		DWORD			dwBytes ;
		DWORD			dwAddress ;
		ESLFileObject *	pTempFile ;
		DWORD			dwLocked ;
	public:
		EBuffer( DWORD dwAddr, ESLFileObject * pfTemp = NULL )
			{
				pbytBuf = (BYTE*) ::eslHeapAllocate( NULL, BUFFER_SIZE, 0 ) ;
				dwBytes = 0 ;
				dwAddress = dwAddr ;
				pTempFile = pfTemp ;
				dwLocked = 1 ;
			}
		~EBuffer( void )
			{
				if ( pbytBuf != NULL )
				{
					::eslHeapFree( NULL, pbytBuf ) ;
				}
			}
		void Lock( void )
			{
				if ( pTempFile != NULL )
				{
					if ( dwLocked == 0 )
					{
						ESLAssert( pbytBuf == NULL ) ;
						pbytBuf =
							(BYTE*) ::eslHeapAllocate
										( NULL, BUFFER_SIZE, 0 ) ;
						if ( pTempFile != NULL )
						{
							pTempFile->Lock( ) ;
							pTempFile->Seek
								( dwAddress, ESLFileObject::FromBegin ) ;
							pTempFile->Read( pbytBuf, dwBytes ) ;
							pTempFile->Unlock( ) ;
						}
					}
					dwLocked ++ ;
				}
			}
		void Unlock( ESLFileObject * pfTemp = NULL )
			{
				if ( pfTemp != NULL )
				{
					pTempFile = pfTemp ;
				}
				else
				{
					pfTemp = pTempFile ;
				}
				if ( pfTemp != NULL )
				{
					if ( dwLocked == 1 )
					{
						ESLAssert( pbytBuf != NULL ) ;
						pTempFile->Lock( ) ;
						pTempFile->Seek
							( dwAddress, ESLFileObject::FromBegin ) ;
						pTempFile->Write( pbytBuf, dwBytes ) ;
						pTempFile->Unlock( ) ;
						dwLocked = 0 ;
						::eslHeapFree( NULL, pbytBuf ) ;
						pbytBuf = NULL ;
					}
					else if ( dwLocked > 1 )
					{
						dwLocked -- ;
					}
				}
			}
	} ;
	EObjArray<EBuffer>	m_tblBuffer ;
	EBuffer *			m_pbLastRead ;
	ESLFileObject *		m_pTempFile ;
	HANDLE				m_hFinished ;
	HANDLE				m_hWritten ;
	unsigned long int	m_nLength ;
	unsigned long int	m_nPosition ;
	unsigned long int	m_nBufLength ;

public:
	// 内容を初期化
	void Initialize
		( unsigned long int nLength = -1,
			ESLFileObject * pTempFile = NULL ) ;
	// バッファへの書き込みを完了
	void FinishStream( void ) ;

public:
	// ファイルオブジェクトを複製する
	virtual ESLFileObject * Duplicate( void ) const ;

public:
	// ファイルから読み込む
	virtual unsigned long int Read
		( void * ptrBuffer, unsigned long int nBytes ) ;
	unsigned long int ReadTimeout
		( void * ptrBuffer, unsigned long int nBytes,
			bool fAsync = false, DWORD dwTimeout = INFINITE ) ;
	// ファイルへ書き出す
	virtual unsigned long int Write
		( const void * ptrBuffer, unsigned long int nBytes ) ;

public:
	// ファイルの長さを取得
	virtual unsigned long int GetLength( void ) const ;
	unsigned long int GetBufferLength( void ) const
		{
			return	m_nBufLength ;
		}
	// ファイルポインタを移動
	virtual unsigned long int Seek
		( long int nOffsetPos, SeekOrigin fSeekFrom ) ;
	// ファイルポインタを取得
	virtual unsigned long int GetPosition( void ) const ;
	// ファイルの終端を現在の位置に設定する
	virtual ESLError SetEndOfFile( void ) ;

} ;


#endif	//	!defined(COMPACT_NOA_DECODER)

#endif
