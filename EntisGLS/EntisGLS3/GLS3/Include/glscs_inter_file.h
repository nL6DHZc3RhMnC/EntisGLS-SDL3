
#if	!defined(__GLSCS_INTER_FILE_H__)
#define	__GLSCS_INTER_FILE_H__

//////////////////////////////////////////////////////////////////////////////
// SSystem::SFileInterface -> ESLFileObject の変換
//////////////////////////////////////////////////////////////////////////////

class	ESLSFileInterface	: public ESLFileObject
{
public:
	SSystem::SFileInterface *	m_pfile ;
	bool						m_fOwner ;

public:
	// クラス情報
	ESL_DECLARE_CLASS_INFO( ESLSFileInterface, ESLFileObject )
	// 構築関数
	ESLSFileInterface( SSystem::SFileInterface * pfile, bool fOwner = false )
		: m_pfile( pfile ), m_fOwner( fOwner ) {}
	virtual ~ESLSFileInterface( void ) ;

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

	// ファイルの長さを取得
	virtual unsigned long int GetLength( void ) const ;
	// ファイルポインタを移動
	virtual unsigned long int Seek
		( long int nOffsetPos, SeekOrigin fSeekFrom ) ;
	// ファイルポインタを取得
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

} ;


//////////////////////////////////////////////////////////////////////////////
// ESLFileObject -> SSystem::SFileInterface の変換
//////////////////////////////////////////////////////////////////////////////

class	SESLFileInterface	: public SSystem::SFileInterface
{
public:
	ESLFileObject *	m_pfile ;
	bool			m_fOwner ;

public:
	// クラス情報
	ESL_DECLARE_CLASS_INFO( SESLFileInterface, SSystem::SFileInterface )
	// 構築関数
	SESLFileInterface( ESLFileObject * pfile, bool fOwner = false )
		: m_pfile( pfile ), m_fOwner( fOwner ) {}
	virtual ~SESLFileInterface( void ) ;

public:
	// ファイルを開く
	virtual SSystem::SFileInterface * OpenFile
		( const wchar_t * pwszFilePath, long int nOpenFlags ) ;
	// ファイルの存在
	virtual bool IsExisting( const wchar_t * pwszFilePath ) ;

public:
	// ファイルインターフェースの複製
	virtual SSystem::SFileInterface * Duplicate( void ) const ;

public:
	// ファイルから読み込み
	virtual size_t Read( void * ptrBuf, size_t nBytes ) ;
	// ファイルへ書き込み
	virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;

public:
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
	virtual SSystem::SError SetEndOfFile( void ) ;

} ;

#endif

