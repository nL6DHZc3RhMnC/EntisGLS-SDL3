
#if	!defined(__ROSETTA_FILE_H__)
#define	__ROSETTA_FILE_H__

#include <sakura/ssys_smart_buffer.h>
#include <sakura/ssys_http_file.h>
#include <sakuragl/sgl_erisa_lib.h>

namespace	Rosetta
{
	//////////////////////////////////////////////////////////////////////////
	// File
	//////////////////////////////////////////////////////////////////////////

	class	RSFile	: public RSObject
	{
	public:
		SSystem::SString			m_strPath ;
		SSystem::SString			m_strDirectPath ;
		bool						m_existing ;
		SSystem::SFileOpener::State	m_state ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSFile, RSObject )
		// 構築関数
		RSFile( RSClass * pClass, const wchar_t * pwszPath )
			: RSObject(pClass,typeOther), m_strPath(pwszPath)
		{
			UpdateFileState() ;
		}
		// パス設定
		void SetFilePath( const wchar_t * pwszPath ) ;
		void UpdateFileState( void ) ;
		// 権限判定
		bool CanExecute( void ) const ;
		bool CanRead( void ) const ;
		bool CanWrite( void ) const ;
		// ファイル／ディレクトリ削除
		bool Delete( void ) ;
		// ファイル存在判定
		bool Exists( void ) const ;
		// 絶対パス取得
		SSystem::SString GetAbsolutePath( void ) const ;
		// ファイル名取得
		SSystem::SString GetName( void ) const ;
		// 親パス（ディレクトリ）取得
		SSystem::SString GetParent( void ) const ;
		// ディレクトリ判定
		bool IsDirectory( void ) const ;
		// ファイル判定
		bool IsFile( void ) const ;
		// 隠し属性判定
		bool IsHidden( void ) const ;
		// 最終更新時間取得
		int64_t GetLastModified( void ) const ;
		// ファイルサイズ取得
		uint64_t GetLength( void ) const ;
		// ディレクトリ作成
		bool MakeDirectory( void ) ;
		bool MakeDirectories( void ) ;
		// ファイル名変更／移動
		bool RenameTo( const wchar_t * pwszPath ) ;
		// ファイル列挙
		void ListFiles
			( SSystem::SObjectArray<SSystem::SString>& listFiles ) const ;
		void ListDirectories
			( SSystem::SObjectArray<SSystem::SString>& listDirs ) const ;

	public:
		// 文字列変換
		virtual bool AsString( SSystem::SString& strValue ) const ;
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// File クラスオブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	RSFileClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSFileClass, RSClass )
		// 構築関数
		RSFileClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"File" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	protected:	// File method
		// void <init>( String path )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean canExecute()
		static RSObject * method_canExecute
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean canRead()
		static RSObject * method_canRead
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean canWrite()
		static RSObject * method_canWrite
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean delete()
		static RSObject * method_delete
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean exists()
		static RSObject * method_exists
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String getAbsolutePath()
		static RSObject * method_getAbsolutePath
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String getName()
		static RSObject * method_getName
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String getParent()
		static RSObject * method_getParent
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isDirectory()
		static RSObject * method_isDirectory
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isFile()
		static RSObject * method_isFile
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isHidden()
		static RSObject * method_isHidden
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// long lastModified()
		static RSObject * method_lastModified
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// long length()
		static RSObject * method_length
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean mkdir()
		static RSObject * method_mkdir
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean mkdirs()
		static RSObject * method_mkdirs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean renameTo( File dest )
		static RSObject * method_renameTo
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String[] list()
		static RSObject * method_list
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String[] list( String wildcard )
		static RSObject * method_list2
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// File[] listFiles()
		static RSObject * method_listFiles
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// File[] listFiles( String wildcard )
		static RSObject * method_listFiles2
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	public:
		// ワイルドカード判定
		static bool IsMatchWildCardTo
			( const wchar_t * pwszWildCard, const wchar_t * pwszFileName ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// InputStream クラスオブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	RSInputStreamClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSInputStreamClass, RSClass )
		// 構築関数
		RSInputStreamClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"InputStream" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;

	public:
		// this オブジェクトのファイルを取得
		static SSystem::SFileInterface *
			GetThisFile( RSContext& context, RSObject* pThis ) ;
		static SSystem::SFileInterface *
			GetFileOf( RSContext& context, RSObject* pObj ) ;

	public:	// InputStream method
		// void <init>( String path, String encoding = null )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int read()
		static RSObject * method_read1
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int read( byte[] b )
		static RSObject * method_read2
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int read( byte[] b, int off, int len )
		static RSObject * method_read3
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int read( ArrayBuffer b, int off, int len )
		// int read( Uint8Pointer b, int off, int len )
		static RSObject * method_read4
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// long skip( long n )
		static RSObject * method_skip
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int available()
		static RSObject * method_available
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void close()
		static RSObject * method_close
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void mark( int readlimit )
		static RSObject * method_mark
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void reset()
		static RSObject * method_reset
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean markSupported()
		static RSObject * method_markSupported
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean readBoolean()
		static RSObject * method_readBoolean
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// byte readByte()
		static RSObject * method_readByte
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// char readChar()
		static RSObject * method_readChar
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// short readShort()
		static RSObject * method_readShort
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int readInt()
		static RSObject * method_readInt
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// long readLong()
		static RSObject * method_readLong
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int readUnsignedByte()
		static RSObject * method_readUnsignedByte
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int readUnsignedShort()
		static RSObject * method_readUnsignedShort
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// float readFloat()
		static RSObject * method_readFloat
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// double readDouble()
		static RSObject * method_readDouble
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String readLine()
		static RSObject * method_readLine
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String readUTF()
		static RSObject * method_readUTF
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// OutputStream クラスオブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	RSOutputStreamClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSOutputStreamClass, RSClass )
		// 構築関数
		RSOutputStreamClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"OutputStream" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;

	public:
		// this オブジェクトのファイルを取得
		static SSystem::SFileInterface *
			GetThisFile( RSContext& context, RSObject* pThis ) ;
		static SSystem::SFileInterface *
			GetFileOf( RSContext& context, RSObject* pObj ) ;

	public:	// OutputStream method
		// void <init>( String path, boolean append = false )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void write( int b )
		static RSObject * method_write1
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void write( byte[] b )
		static RSObject * method_write2
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void write( byte[] b, int off, int len )
		static RSObject * method_write3
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void write( ArrayBuffer b, int off, int len )
		// void write( Uint8Pointer b, int off, int len )
		static RSObject * method_write4
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void flush()
		static RSObject * method_flush
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void close()
		static RSObject * method_close
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void writeBoolean( boolean v )
		static RSObject * method_writeBoolean
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void writeByte( int v )
		static RSObject * method_writeByte
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void writeShort( int v )
		static RSObject * method_writeShort
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void writeChar( int v )
		static RSObject * method_writeChar
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void writeInt( int v )
		static RSObject * method_writeInt
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void writeLong( long v )
		static RSObject * method_writeLong
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void writeFloat( float v )
		static RSObject * method_writeFloat
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void writeDouble( double v )
		static RSObject * method_writeDouble
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void writeChars( String s )
		static RSObject * method_writeChars
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void writeUTF( String s )
		static RSObject * method_writeUTF
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// OutputStream printf( String fmt, ... )
		static RSObject * method_printf
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// RandomAccessFile クラスオブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	RSRandomAccessFileClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSRandomAccessFileClass, RSClass )
		// 構築関数
		RSRandomAccessFileClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"RandomAccessFile" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;

	public:
		// this オブジェクトのファイルを取得
		static SSystem::SFileInterface *
			GetThisFile( RSContext& context, RSObject* pThis ) ;
		static SSystem::SFileInterface *
			GetFileOf( RSContext& context, RSObject* pObj ) ;

	public:	// RandomAccessFile method
		// void <init>( String path, String mode )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void close()
		static RSObject * method_close
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String readLine()
		static RSObject * method_readLine
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// long getFilePointer()
		static RSObject * method_getFilePointer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// long length()
		static RSObject * method_length
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void seek( long pos )
		static RSObject * method_seek
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setLength( long newLength )
		static RSObject * method_setLength
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// InputStream getInputStream()
		static RSObject * method_getInputStream
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// OutputStream getOutputStream()
		static RSObject * method_getOutputStream
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// SmartBufferFile クラスオブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	RSSmartBufferFileClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSSmartBufferFileClass, RSClass )
		// 構築関数
		RSSmartBufferFileClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"SmartBufferFile" ) ;
		// メンバ初期設定
		virtual void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;

	public:
		// this オブジェクトのファイルを取得
		static SSystem::SSmartBuffer *
			GetThisFile( RSContext& context, RSObject* pThis ) ;
		static SSystem::SSmartBuffer *
			GetFileOf( RSContext& context, RSObject* pObj ) ;

	public:	// SmartBufferFile method
		// void <init>( void )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// NoaFileArchiver クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSNoaFileArchiverClass	: public RSClass
	{
	public:
		class	RSFileInfoClass	: public RSClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( RSFileInfoClass, RSClass )
			// 構築関数
			RSFileInfoClass
				( RSClass * pClass,
					const wchar_t * pwszClassName = L"FileInfo" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
		public:
			// 変換
			static void ObjectToFileEntry
				( RSContext& context, RSObject * pObj,
					ERISA::SGLArchiveFile::FILE_ENTRY_EX& fe,
							SSystem::SString& strFileName ) ;
			static void ObjectFromFileEntry
				( RSContext& context, RSObject * pObj,
					const ERISA::SGLArchiveFile::FILE_ENTRY_EX& fe,
									const wchar_t * pwszFileName ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSNoaFileArchiverClass, RSClass )
		// 構築関数
		RSNoaFileArchiverClass
			( RSClass * pClass,
				const wchar_t * pwszClassName = L"NoaFileArchiver" ) ;
		// メンバ初期設定
		virtual void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;

	protected:
		// this オブジェクトのファイルを取得
		static ERISA::SGLArchiveFile *
			GetThisFile( RSContext& context, RSObject* pThis ) ;
		static ERISA::SGLArchiveFile *
			GetFileOf( RSContext& context, RSObject* pObj ) ;
		// NoaFileArchiver.FileInfo[] から ERISA::SGLArchiveFile::SDirectory
		static void DirectoryFromObject
			( RSContext& context,
				ERISA::SGLArchiveFile::SDirectory& dirFiles, RSObject * pObj ) ;

	public:	// NoaFileArchiver method
		// void <init>( void )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean openArchive( RandomAccessFile file )
		static RSObject * method_openArchive
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean createArchive
		//	( RandomAccessFile file, NoaFileArchiver.FileInfo[] dirRoot )
		static RSObject * method_createArchive
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void close()
		static RSObject * method_close
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean descendDirectory( String sDirName )
		static RSObject * method_descendDirectory
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean createDirectory
		//	( String sDirName, NoaFileArchiver.FileInfo[] dirFiles )
		static RSObject * method_createDirectory
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean ascendDirectory()
		static RSObject * method_ascendDirectory
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean descendFile
		//	( String sFileName,
		//		String sPassword = null, boolean flagStream = false )
		static RSObject * method_descendFile
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean ascendFile()
		static RSObject * method_ascendFile
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// NoaFileArchiver.FileInfo getCurrentFileInfo()
		static RSObject * method_getCurrentFileInfo
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// RandomAccessFile openFile( String sFilePath )
		static RSObject * method_openFile
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isExisting( String sFilePath )
		static RSObject * method_isExisting
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String[] listFiles( String sDirPath )
		static RSObject * method_listFiles
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String[] listDirectories( String sDirPath )
		static RSObject * method_listDirectories
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// HttpInputStream クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSHttpInputStreamClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSHttpInputStreamClass, RSClass )
		// 構築関数
		RSHttpInputStreamClass
			( RSClass * pClass,
				const wchar_t * pwszClassName = L"HttpInputStream" ) ;
		// メンバ初期設定
		virtual void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;

	public:
		// this オブジェクトのファイルを取得
		static SSystem::SHttpFileInterface *
			GetThisHttpFile( RSContext& context, RSObject* pThis ) ;
		static SSystem::SHttpFileInterface *
			GetHttpFileOf( RSContext& context, RSObject* pObj ) ;

	public:	// HttpInputStream method
		// void <init>( void )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean openURL
		//	( String url, String agent,
		//		Uint8Pointer data = null, int len = -1,
		//		String strContentType = null )
		static RSObject * method_openURL
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setRequest( String url, String cmd = "GET" )
		static RSObject * method_setRequest
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setSendURLFormData
		//	( String param, int encoding = String.encodingUTF8 )
		static RSObject * method_setSendURLFormData
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setSendData( Uint8Pointer b, int len )
		static RSObject * method_setSendData
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean addHeader( String strHeader )
		static RSObject * method_addHeader
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean connect()
		static RSObject * method_connect
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean sendRequest()
		static RSObject * method_sendRequest
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int queryStatusCode()
		static RSObject * method_queryStatusCode
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// long queryContentLength()
		static RSObject * method_queryContentLength
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String queryContentType()
		static RSObject * method_queryContentType
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String queryContentTypeCharset()
		static RSObject * method_queryContentTypeCharset
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String queryContentTransferEncoding()
		static RSObject * method_queryContentTransferEncoding
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Date queryContentDate()
		static RSObject * method_queryContentDate
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Date queryContentLastModified()
		static RSObject * method_queryContentLastModified
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;

}

#endif

