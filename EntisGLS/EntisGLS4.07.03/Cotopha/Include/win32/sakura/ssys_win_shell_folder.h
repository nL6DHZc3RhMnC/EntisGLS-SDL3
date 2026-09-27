
#if	!defined(__SAKURA2_WIN32_SHELL_FOLDER_H__)
#define	__SAKURA2_WIN32_SHELL_FOLDER_H__

#include <sakura/ssys_smart_buffer.h>

#include <shlobj.h>
#include <PortableDeviceApi.h>
#include <PortableDevice.h>

namespace	SSystem
{
	//////////////////////////////////////////////////////////////////////////
	// ストリームファイル
	//////////////////////////////////////////////////////////////////////////

	class	SWin32StreamFile	: public SFileInterface
	{
	protected:
		IStream *	m_stream ;
		bool		m_fSeekable ;
		int64_t		m_nSpecLength ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SWin32StreamFile, SFileInterface )
			// 構築関数
			SWin32StreamFile
			( IStream * stream,
			bool fSeekable = false, int64_t nSpecLength = -1 ) ;
		// 消滅関数
		virtual ~SWin32StreamFile( void ) ;

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
			( int64_t posFile, SeekOrigin seekFrom ) ;
		// ファイルポインタを取得
		virtual int64_t GetPosition( void ) const ;
		// ファイルの終端を現在の位置に設定する
		virtual SError SetEndOfFile( void ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// シェルフォルダー
	//////////////////////////////////////////////////////////////////////////

	class	SWin32ShellFolder	: public ESLObject //public SFileOpener
	{
	protected:
		// Windows XP 以降 API
		typedef HRESULT (STDAPICALLTYPE *API_SHGetFolderLocation)
			( __reserved HWND hwnd, __in int csidl,
				__in_opt HANDLE hToken, __in DWORD dwFlags,
				__deref_out PIDLIST_ABSOLUTE *ppidl ) ;
		typedef HRESULT (STDAPICALLTYPE *API_SHCreateShellItem)
			( __in_opt PCIDLIST_ABSOLUTE pidlParent,
				__in_opt IShellFolder *psfParent,
				__in PCUITEMID_CHILD pidl, __out IShellItem **ppsi ) ;
		API_SHGetFolderLocation	m_apiSHGetFolderLocation ;
		API_SHCreateShellItem	m_apiSHCreateShellItem ;

		static const GUID	CLSID_FileOperation ;

		IFileOperation *	m_pfOperation ;

		class	FileName	: public SString
		{
		public:
			FileName( void ) { }
			FileName( const wchar_t * pwszName )
				: SString( pwszName ) { }
			const FileName& operator = ( const wchar_t * pwszName )
			{
				SetString( pwszName ) ;
				return	*this ;
			}
			bool operator == ( const wchar_t * pszSrc ) const
			{
				return	CompareNoCase( pszSrc ) == 0 ;
			}
			bool operator != ( const wchar_t * pszSrc ) const
			{
				return	CompareNoCase( pszSrc ) != 0 ;
			}
			bool operator < ( const wchar_t * pszSrc ) const
			{
				return	CompareNoCase( pszSrc ) < 0 ;
			}
			bool operator <= ( const wchar_t * pszSrc ) const
			{
				return	CompareNoCase( pszSrc ) <= 0 ;
			}
			bool operator > ( const wchar_t * pszSrc ) const
			{
				return	CompareNoCase( pszSrc ) > 0 ;
			}
			bool operator >= ( const wchar_t * pszSrc ) const
			{
				return	CompareNoCase( pszSrc ) >= 0 ;
			}
		} ;

		class	ShellItemIdList
		{
		public:
			FileName		m_path ;
			LPITEMIDLIST	m_pidl ;
		public:
			ShellItemIdList( void ) : m_pidl(NULL) { }
			~ShellItemIdList( void )
			{
				if ( m_pidl != NULL )
				{
					::CoTaskMemFree( m_pidl ) ;
					m_pidl = NULL ;
				}
			}
		} ;
		SObjectArray<ShellItemIdList>	m_aFolderNest ;

		SString				m_strFullPath ;
		IShellItem *		m_psiFolder ;
		IShellFolder *		m_pshFolder ;
		LPITEMIDLIST		m_pidlFolder ;

		bool					m_fEnumFolders ;
		bool					m_fEnumFiles ;
		SArray<LPITEMIDLIST>	m_aFolders ;
		SArray<LPITEMIDLIST>	m_aFiles ;

		SIndexedArray<FileName, const wchar_t *>	m_aFileNames ;

	public:
		// 構築関数
		SWin32ShellFolder( void ) ;
		// 消滅関数
		virtual ~SWin32ShellFolder( void ) ;
		// 解放
		void Release( void ) ;
		void ReleaseChildrenItems( void ) ;
		// 現在のパスを取得
		const SString & GetCurrentPath( void ) const
		{
			return	m_strFullPath ;
		}

	public:
		// シェル上のフォルダパスを指定して移動
		SError SearchShellFolder( const wchar_t * pwszPath ) ;
		// 指定パスへ移動
		SError OpenFolderPath( const wchar_t * pwszPath ) ;
		// ドライブルート（コンピューター）へ移動
		SError OpenDrives( void ) ;
		// 下層フォルダへ移動
		SError DescendFolder( const wchar_t * pwszName ) ;
		// 上層フォルダへ移動
		SError AscendFolder( void ) ;

	public:
		// ファイルを開く
		SFileInterface *
			OpenFile( const wchar_t * pwszName, long int nOpenFlags ) ;
		// ファイル作成
		SFileInterface * NewFile( const wchar_t * pwszName ) ;
		// フォルダ作成
		SError NewFolder( const wchar_t * pwszName ) ;
		// ファイル削除
		SError DeleteFile( const wchar_t * pwszName ) ;
		// フォルダ削除
		SError DeleteFolder( const wchar_t * pwszName ) ;
		// ファイル名変更
		SError RenameFile
			( const wchar_t * pwszOldName, const wchar_t * pwszNewName ) ;
		// フォルダ名変更
		SError RenameFolder
			( const wchar_t * pwszOldName, const wchar_t * pwszNewName ) ;
		// ファイル複製
		SError CopyFile
			( IShellItem * psiSrcFile, const wchar_t * pwszDstName = NULL ) ;
		// ファイル移動
		SError MoveFile
			( IShellItem * psiSrcFile, const wchar_t * pwszDstName = NULL ) ;

	public:
		// 現在のフォルダ IShellItem を取得
		IShellItem * GetFolderShellItem( void ) const ;
		// 現在のフォルダ IShellFolder を取得
		IShellFolder * GetFolderShellFolder( void ) const ;
		// フォルダの IShellItem を取得
		IShellItem * GetFolderShellItem( const wchar_t * pwszName ) ;
		// ファイルの IShellItem を取得
		IShellItem * GetFileShellItem( const wchar_t * pwszName ) ;

	protected:
		// SFileOpener::OpenFlag から IBindCtx を生成
		static IBindCtx * CreateBindCtxWithOpenFlags( long int nOpenFlags ) ;
		static DWORD AccessModeFromOpenFlags( long int nOpenFlags ) ;
		// ファイルオペレーション取得
		IFileOperation * GetFileOperation( void ) ;
		// フォルダアイテム列挙
		void EnumFolderItems( void ) ;
		// ファイルアイテム列挙
		void EnumFileItems( void ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ポータブルデバイス (MSC/MTP)
	//////////////////////////////////////////////////////////////////////////

	class	SWin32PortableDevice	: public SFileOpener
	{
	protected:
		IPortableDeviceManager *	m_pDevManager ;
		IPortableDevice *			m_pDevice ;
		IPortableDeviceContent *	m_pContent ;
		IPortableDeviceProperties *	m_pProp ;

	public:
		class	DeviceInfo
		{
		public:
			SString	m_DeviceId ;
			SString	m_FriendlyName ;
			SString	m_Manufacturer ;
			SString	m_Description ;
		} ;
		enum	ContentType
		{
			typeFolder			= 0x00000001,
			typeStorage			= 0x00000002,
			flagCreatedDate		= 0x00010000,
			flagModifiedDate	= 0x00020000,
			flagAuthoredDate	= 0x00040000,
		} ;
		class	ContentInfo
		{
		public:
			uint32_t	m_flags ;
			SString		m_id ;
			SString		m_name ;
			SString		m_filename ;
			uint64_t	m_size ;
			DATE_TIME	m_dtCreated ;
			DATE_TIME	m_dtModified ;
			DATE_TIME	m_dtAuthored ;
		} ;
		class	FolderContents	: public SObjectArray<ContentInfo>
		{
		public:
			// 構築関数
			FolderContents( void ) { }
			FolderContents( const FolderContents& fc )
				: SObjectArray<ContentInfo>( fc ) { }
		} ;

	protected:
		SObjectArray<DeviceInfo>	m_aDevInfos ;

		SStrSortObjectArray<SString>		m_ssoaFolderIDs ;
		SStrSortObjectArray<FolderContents>	m_ssoaFolders ;

		bool						m_fReadOnly ;
		SString						m_idDevice ;
		SString						m_idParent ;
		SString						m_strCurrentPath ;
		FolderContents *			m_pContents ;
		SCriticalSection			m_csSync ;

		static const GUID	CLSID_PortableDeviceManager ;
		static const GUID	CLSID_PortableDeviceFTM ;
		static const GUID	CLSID_PortableDeviceValues ;
		static const GUID	CLSID_PortableDeviceKeyCollection ;
		static const GUID	CLSID_PortableDevicePropVariantCollection ;

		static const PROPERTYKEY	WPD_CLIENT_DESIRED_ACCESS ;
		static const PROPERTYKEY	WPD_OBJECT_CONTENT_TYPE ;
		static const PROPERTYKEY	WPD_OBJECT_PARENT_ID ;
		static const PROPERTYKEY	WPD_OBJECT_NAME ;
		static const PROPERTYKEY	WPD_OBJECT_ORIGINAL_FILE_NAME ;
		static const PROPERTYKEY	WPD_OBJECT_SIZE ;
		static const PROPERTYKEY	WPD_OBJECT_DATE_CREATED ;
		static const PROPERTYKEY	WPD_OBJECT_DATE_MODIFIED ;
		static const PROPERTYKEY	WPD_OBJECT_DATE_AUTHORED ;
		static const PROPERTYKEY	WPD_RESOURCE_DEFAULT ;
		static const GUID			WPD_FUNCTIONAL_CATEGORY_STORAGE ;
		static const GUID			WPD_CONTENT_TYPE_FUNCTIONAL_OBJECT ;
		static const GUID			WPD_CONTENT_TYPE_FOLDER ;
		static const GUID			WPD_CONTENT_TYPE_UNSPECIFIED ;

	public:
		// デバイスへ転送するストリーム
		class	TransferToDevice : public SWin32StreamFile
		{
		protected:
			SSmartReference<SWin32PortableDevice>	m_refDevice ;
			SString									m_idParent ;
			IPortableDeviceDataStream *				m_ppddsStream ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( TransferToDevice, SWin32StreamFile )
				// 構築関数
				TransferToDevice( SWin32PortableDevice * pDev, IStream * pStream ) ;
			// 消滅関数
			virtual ~TransferToDevice( void ) ;
			// コミット
			void Commit( void ) ;
		} ;
		friend class	TransferToDevice ;

		// バッファ遅延転送ストリーム
		class	BufferedTransferToDevice	: public SSmartBuffer
		{
		protected:
			SSmartReference<SWin32PortableDevice>	m_refDevice ;
			SString									m_strFolderPath ;
			SString									m_strFileName ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( BufferedTransferToDevice, SSmartBuffer )
				// 構築関数
				BufferedTransferToDevice
				( SWin32PortableDevice * pDev, const wchar_t * pwszName ) ;
			// 消滅関数
			virtual ~BufferedTransferToDevice( void ) ;
			// コミット
			SError Commit( void ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SWin32PortableDevice, SFileOpener )
			// 構築関数
			SWin32PortableDevice( void ) ;
		// 消滅関数
		virtual ~SWin32PortableDevice( void ) ;
		// 保有リソース解放
		void Release( void ) ;

	public:	// SFileOpener
		// ファイルを開く
		virtual SFileInterface * NewOpenFile
			( const wchar_t * pszFilePath, long int nOpenFlags ) ;
		// ファイルの存在
		virtual bool IsExisting( const wchar_t * pszFilePath ) ;
		// ファイル状態
		virtual SError QueryState
			( const wchar_t * pszFilePath, State& state ) ;
		// ファイルの一覧取得
		virtual void ListSubFiles
			( SObjectArray<SString>& listFiles,
			const wchar_t * pszDirPath = NULL ) ;
		// ディレクトリの一覧取得
		virtual void ListSubDirectories
			( SObjectArray<SString>& listDirs,
			const wchar_t * pszDirPath = NULL ) ;
		// ファイルを削除する
		virtual SError RemoveSubFile( const wchar_t * pszFilePath ) ;
		// ディレクトリを作成する
		virtual SError CreateSubDirectory
			( const wchar_t * pszPath, long int nFlags = 0 ) ;
		// ディレクトリを削除する
		virtual SError RemoveSubDirectory( const wchar_t * pszPath ) ;
		// ファイル名を変更する
		virtual SError RenameSubFile
			( const wchar_t * pszOldPath, const wchar_t * pszNewPath ) ;

	public:
		// デバイス列挙
		SError EnumerateDevices( void ) ;
		// デバイス数取得
		size_t GetDeviceCount( void ) const ;
		// デバイス情報取得
		const DeviceInfo * GetDeviceInfoAt( size_t i ) const ;
		// デバイス検索
		ssize_t FindDeviceIndex( const wchar_t * pwszName ) const ;
		const DeviceInfo * GetDeviceInfoAs( const wchar_t * pwszName ) const ;

	public:
		// デバイス選択
		SError SelectDevice( const DeviceInfo * pDevInfo ) ;
		// ルートストレージ列挙
		SError EnumerateRootStorages( void ) ;
		// コンテンツ列挙
		SError EnumerateContents( const wchar_t * pwszParentID ) ;
		// 現在のコンテンツリストの親ID
		SString GetCurrentContentsParent( void ) const ;
		// コンテンツリストの排他処理用
		void LockContentsList( void ) ;
		void UnlockContentsList( void ) ;

	public:
		// 現在のフォルダを移動
		SError SetCurrentFolder
			( const wchar_t * pwszPath, bool fCreateDirectories = false ) ;
		// 現在のフォルダパス取得
		SString GetCurrentFolderPath( void ) const ;
	protected:
		static SString ParseNextFolderName
			( const wchar_t * pwszPath, size_t& iNext ) ;
		// 現在のフォルダコンテンツリストを設定
		void SetCurrentFolderContents
			( const wchar_t * pwszParentID,
				const wchar_t * pwszFolderPath,
				FolderContents * pContents ) ;
	public:
		// 下層フォルダへ移動
		SError DescendFolder( const wchar_t * pwszName ) ;
		// 上層フォルダへ移動
		SError AscendFolder( void ) ;
		// フォルダ作成
		SError CreateFolder( const wchar_t * pwszName ) ;
		// フォルダ作成
		SError DeleteContent( const wchar_t * pwszName ) ;
		// ファイルを開く
		SFileInterface * OpenFileContent
			( const wchar_t * pwszName,
			long int nOpenFlags = SFileOpener::modeRead ) ;
		// ファイル新規作成
		SFileInterface * NewFileContent
			( const wchar_t * pwszName, uint64_t nFileBytes ) ;
		// ファイル名変更
		SError RenameFile
			( const wchar_t * pwszOldName, const wchar_t * pwszNewName ) ;
		// ファイル時刻変更
		SError SetFileTime
			( const wchar_t * pwszName,
				const DATE_TIME * pdtCreated = NULL,
				const DATE_TIME * pdtModified = NULL,
				const DATE_TIME * pdtAuthored = NULL ) ;

	public:
		// アイテム数取得
		size_t GetContentCount( void ) const ;
		// アイテム取得
		const ContentInfo * GetContentInfo( size_t i ) const ;
		// アイテム検索
		ssize_t FindContentIndex( const wchar_t * pwszName ) const ;
		const ContentInfo * GetContentAs( const wchar_t * pwszName ) const ;

	protected:
		// アイテム情報取得
		ContentInfo * NewContentInfo( const wchar_t * pwszID ) const ;
		// 文字列プロパティ取得
		bool GetContentStringProperty
			( SString& strValue,
			const wchar_t * pwszID, const PROPERTYKEY& propKey ) const ;
		// 数値プロパティ取得
		bool GetContentULargeIntegerProperty
			( uint64_t& nValue,
			const wchar_t * pwszID, const PROPERTYKEY& propKey ) const ;
		// 日付プロパティ取得
		bool GetContentDateProperty
			( DATE_TIME& dtValue,
			const wchar_t * pwszID, const PROPERTYKEY& propKey ) const ;
		// PROPVARIANT から時刻へ変換
		static bool DateFromPropVariant( DATE_TIME& dt, const PROPVARIANT& pv ) ;
		// 時刻から PROPVARIANT へ変換
		static void PropVariantFromDate( PROPVARIANT& pv, const DATE_TIME& dt ) ;
	} ;

}

#endif

