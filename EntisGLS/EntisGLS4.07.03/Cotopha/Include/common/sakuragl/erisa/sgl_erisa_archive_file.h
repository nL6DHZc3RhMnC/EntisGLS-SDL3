
#if	!defined(__SAKURAGL_ERISA_ARCHIVE_FILE_H__)
#define	__SAKURAGL_ERISA_ARCHIVE_FILE_H__

#include <sakura/ssys_stack_buffer.h>
#include <sakura/ssys_smart_buffer.h>

namespace	ERISA
{
	//////////////////////////////////////////////////////////////////////////
	// NOA 書庫ファイル
	//////////////////////////////////////////////////////////////////////////

	class	SGLArchiveFile	: public SSystem::SChunkFile
	{
	public:
		// ファイル・エントリ構造体
		struct	FILE_TIME
		{
			uint8_t		nSecond ;
			uint8_t		nMinute ;
			uint8_t		nHour ;
			uint8_t		nWeek ;
			uint8_t		nDay ;
			uint8_t		nMonth ;
			uint16_t	nYear ;
		} ;
		struct	FILE_ENTRY
		{
			uint64_t	nBytes ;
			uint32_t	nAttribute ;
			uint32_t	nEncodeType ;
			uint64_t	nOffsetPos ;
			FILE_TIME	ftFileTime ;
		} ;
		struct	FILE_ENTRY_EX	: public FILE_ENTRY
		{
			uint32_t	nExtraInfoBytes ;	// ※アライメント互換性のため
											// sizeof(FILE_ENTRY_EX) は推奨されません
		} ;
		struct	FILE_EXTRA_INFO
		{
			uint32_t	nCRC32 ;			// CRC32
			uint32_t	nDecrypeKey[1] ;	// 暗号鍵
		} ;
		enum	FileAttribute
		{
			attrNormal			= 0x00000000,
			attrReadOnly		= 0x00000001,
			attrHidden			= 0x00000002,
			attrSystem			= 0x00000004,
			attrDirectory		= 0x00000010,
			attrEndOfDirectory	= 0x00000020,
			attrNextDirectory	= 0x00000040,
			attrFileNameUTF8	= 0x01000000,
		} ;
		enum	EncodeType
		{
			encodeRaw			= 0x00000000,
			encodeERISA			= 0x80000010,
			encodeCrypt32		= 0x20000000,
			encodeERISACrypt32	= 0xA0000010,
		} ;
		// ファイル情報
		struct	FileReferenceInfo
		{
			FILE_ENTRY_EX *		pfeEntry ;
			FILE_EXTRA_INFO *	pfxiExtra ;
			uint32_t			lenFilename ;
			uint8_t *			pszFilename ;		// UTF-8
		} ;
		// ファイルのアクセス方法
		enum	FileOpenMethod
		{
			openAsNormal,
			openAsStream,
		} ;
		// ディレクトリ・エントリ
		class	SDirectory : public SSystem::SObjectArray<FileReferenceInfo>
		{
		protected:
			uint64_t					m_fposDirectoryBase ;
			SSystem::SStackBuffer		m_sbufDescriptor ;
			SSystem::SObjectArray
				<SSystem::SByteBuffer>	m_bufDescriptor ;
		public:
			// 構築関数
			SDirectory( void ) ;
			SDirectory( const SDirectory & dirSrc ) ;
			// 代入演算子
			const SDirectory & operator = ( const SDirectory & dirSrc ) ;
			// 複製
			void CopyDirectoryFrom( const SDirectory & dirSrc ) ;
			// 削除
			void RemoveAll( void ) ;
			// ディレクトリ・ディスクリプタ追加読み込み
			SSystem::SError ReadDescriptor
				( SSystem::SInputStream& stream, size_t nBytes ) ;
			// ディレクトリ・ディスクリプタ書き出し
			SSystem::SError WriteDescriptor
				( SSystem::SOutputStream& stream ) const ;
			// ディレクトリ・ディスクリプタサイズ算出
			size_t GetDescriptorSize( void ) const ;
			// ディレクトリの基準ファイル位置を取得
			uint64_t GetBaseFilePosition( void ) const
			{
				return	m_fposDirectoryBase ;
			}
			// ディレクトリの基準ファイル位置を設定
			void SetBaseFilePosition( uint64_t fposBase )
			{
				m_fposDirectoryBase = fposBase ;
			}
		public:
			// ファイル・エントリを検索
			FileReferenceInfo *
				GetFileInfoAs( const uint8_t * pszFilenameUTF8 ) ;
			FileReferenceInfo *
				GetFileInfoAs( const wchar_t * pwszFilename ) ;
			// ファイル・エントリを追加
			size_t AddFileEntry
				( const uint8_t * pszFilenameUTF8,
					const FILE_ENTRY_EX& feEntry,
					const FILE_EXTRA_INFO * pfxiExtra = NULL ) ;
			size_t AddFileEntry
				( const wchar_t * pwszFilename,
					const FILE_ENTRY_EX& feEntry,
					const FILE_EXTRA_INFO * pfxiExtra = NULL ) ;
		public:
			// 指標検索
			size_t OrderIndex( const uint8_t * pszFilenameUTF8 ) const ;
			// ファイル名比較
			static int CompareFilename
				( const uint8_t * pszFile1, const uint8_t * pszFile2 ) ;
		} ;
		// ファイル参照
		class	RefFile	: public SSystem::SSmartFile
		{
		protected:
			SSystem::SError	m_errResult ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( RefFile, SSmartFile )
			// 構築関数
			RefFile( SGLArchiveFile * pArcFile ) ;
			// 消滅関数
			virtual ~RefFile( void ) ;
			// ファイルの参照を解除する
			virtual void Close( void ) ;
			// エラーコード取得
			SSystem::SError GetError( void ) const ;
		} ;

	protected:
		// 同期オブジェクト
		SSystem::SCriticalSection	m_csSync ;

		// [<dir1-name>[\<dir2-name>...]] 形式のパスをキーに使う
		// キーのアルファベットは小文字に正規化
		SSystem::SStrSortObjectArray<SDirectory>	m_cacheDir ;

		// 現在のディレクトリ
		SSystem::SString	m_strCurDirectory ;
		SDirectory *		m_pdirCurrent ;

		// 現在開いているファイルの情報
		FileReferenceInfo *	m_pfriFile ;

		// 現在開いているファイルの展開されたバッファ
		SSystem::SSmartPointer<SSystem::SSmartBuffer>
								m_pFileBuffer ;

		// ファイル書き出し用ストリーム
		SSystem::SSmartPointer<ERISA::SGLEncrypt32OutputStream>
								m_pEncrypt32 ;		// 簡易 32 ビット暗号化
		SSystem::SSmartPointer<ERISA::SGLEncodeBitStream>
								m_pEncBitStream ;	// ERISAN 符号化出力用
		SSystem::SSmartPointer<ERISA::SGLERISANEncodeContext>
								m_pEncERISAN ;		// ERISAN 符号化
		SSystem::SSmartPointer<SakuraCL::CRC32OutputStream>
								m_pOutCRC32 ;		// CRC32 出力ストリーム

		// ファイル読み込み用ストリーム
		SSystem::SSmartPointer<ERISA::SGLDecrypt32InputStream>
								m_pDecrypt32 ;		// 簡易 32 ビット暗号化
		SSystem::SSmartPointer<ERISA::SGLDecodeBitStream>
								m_pDecBitStream ;	// ERISAN 符号化入力用
		SSystem::SSmartPointer<ERISA::SGLERISANDecodeContext>
								m_pDecERISAN ;		// ERISAN 復号
		SSystem::SSmartPointer<SakuraCL::CRC32InputStream>
								m_pInCRC32 ;		// CRC32 入力ストリーム

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLArchiveFile, SChunkFile )
		// 構築関数
		SGLArchiveFile( void ) ;
		// 消滅関数
		virtual ~SGLArchiveFile( void ) ;

	public:
		// 書庫ファイルを開く
		SSystem::SError OpenArchive
			( SSystem::SFileInterface * pFile, bool flagOwner = false,
				long int nFlags = 0, SDirectory * pRootDir = NULL ) ;
		// 書庫ファイルを閉じる
		SSystem::SError CloseArchive( void ) ;
		// 書庫ファイルを閉じる
		virtual void Close( void ) ;

	protected:
		// ディレクトリを書き出す
		SSystem::SError WriteDirectoryDescription( SDirectory& dir ) ;
		// ディレクトリを読み込む
		SSystem::SError ReadDirectoryDescription( SDirectory& dir ) ;
		// エンコーダーを設定する
		SSystem::SError PrepareEncoder
			( FileReferenceInfo * pfriInfo,
				SSystem::SOutputStream * pStream,
					const wchar_t * pwszPassword ) ;
		// デコーダーを設定する
		SSystem::SError PrepareDecoder
			( FileReferenceInfo * pfriInfo,
				SSystem::SInputStream * pStream,
					const wchar_t * pwszPassword ) ;
		// デコード済みのバッファを生成する
		SSystem::SSmartBuffer *
				CreateDecodedFile( FileReferenceInfo * pfriInfo ) ;

	public:
		// サブディレクトリを開く
		SSystem::SError DescendDirectory
			( const wchar_t * pwszDirName, SDirectory * pWriteDir = NULL ) ;
		// ディレクトリを一つ上に移動
		SSystem::SError AscendDirectory( void ) ;
		// 書庫内のファイルを開く
		SSystem::SError DescendFile
			( const wchar_t * pwszFilename,
				const wchar_t * pwszPassword = NULL,
				FileOpenMethod openMethod = openAsNormal ) ;
		// 書庫内のファイルを閉じる
		SSystem::SError AscendFile( void ) ;

	public:
		// ディレクトリ情報をロードし取得
		SDirectory * LoadDirectoryDescriptorAs( const wchar_t * pwszDirPath ) ;
		// ディレクトリパスを正規化
		static void NormalizeDirectoryPath( SSystem::SString& strDirPath ) ;
		// 現在のディレクトリを取得
		const SSystem::SString & GetCurrentDirectoryPath( void ) const
		{
			return	m_strCurDirectory ;
		}
		SDirectory * GetCurrentDirectory( void ) const
		{
			return	m_pdirCurrent ;
		}
		// 現在開いているファイル情報を取得
		const FileReferenceInfo * GetCurrentFileInfo( void ) const
		{
			return	m_pfriFile ;
		}

	public:	// SFileOpener オーバーライド
		// ファイルを開く
		virtual SSystem::SFileInterface * NewOpenFile
			( const wchar_t * pszFilePath, long int nOpenFlags ) ;
		// ファイルの存在
		virtual bool IsExisting( const wchar_t * pszFilePath ) ;
		// ファイル状態
		virtual SSystem::SError QueryState
			( const wchar_t * pszFilePath,
				SSystem::SFileOpener::State& state ) ;
		// ファイルの一覧取得
		virtual void ListSubFiles
			( SSystem::SObjectArray<SSystem::SString>& listFiles,
								const wchar_t * pszDirPath = NULL ) ;
		// ディレクトリの一覧取得
		virtual void ListSubDirectories
			( SSystem::SObjectArray<SSystem::SString>& listDirs,
								const wchar_t * pszDirPath = NULL ) ;

	protected:
		SSystem::SString	m_strDefPassword ;

	public:
		// デフォルトのパスワード設定
		void SetDefaultPassword( const wchar_t * pwszPassword ) ;

	public:	// SFileInterface オーバーライド
		// ファイルインターフェースの複製
		virtual SSystem::SFileInterface * Duplicate( void ) const ;
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
		virtual SSystem::SError SetEndOfFile( void ) ;

	} ;

}

#endif

