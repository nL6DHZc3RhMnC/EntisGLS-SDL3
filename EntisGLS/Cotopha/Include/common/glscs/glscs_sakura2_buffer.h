
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_OBJECT_BUFFER_H__)
#define	__GLSCS_SAKURA2_OBJECT_BUFFER_H__

namespace	ECSSakura2
{
	using	ECSSakura2Processor::LinearAddressCache ;
	using	ECSSakura2Processor::Context ;
	using	SSystem::SError ;
	using	SSystem::SFileInterface ;

	//////////////////////////////////////////////////////////////////////////
	// 単純なバッファ
	//////////////////////////////////////////////////////////////////////////

	class	Buffer	: public ESLObject
	{
	protected:
		BYTE *	m_pbytBuf ;
		DWORD	m_nBufSize ;
		DWORD	m_nBufBase ;
		DWORD	m_nBufLimit ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( Buffer, ESLObject )
		// 構築関数
		Buffer( void )
			: m_pbytBuf(NULL), m_nBufSize(0),
					m_nBufBase(0), m_nBufLimit(0) {}
		Buffer( const Buffer & buf ) ;
		// 消滅関数
		virtual ~Buffer( void ) ;

	public:	// バッファ管理
		// バッファ生成
		virtual SError CreateBuffer( DWORD nBytes, DWORD nBase = 0 ) ;
		// バッファリサイズ
		virtual SError ResizeBuffer( DWORD nBytes, DWORD nBase = 0 ) ;
		// バッファリミット設定
		virtual SError ResizeBufferLimit( DWORD nLimit ) ;
		// バッファ解放
		virtual void FreeBuffer( void ) ;
		// バッファ複製
		SError CopyBufferFrom( const Buffer & buf ) ;

	public:	// バッファアクセス
		// バッファ取得
		BYTE * GetBuffer( void ) const
			{
				return	m_pbytBuf ;
			}
		// バッファ長取得
		DWORD GetLength( void ) const
			{
				return	m_nBufSize ;
			}
		// バッファベースアドレス
		DWORD GetBufferBase( void ) const
			{
				return	m_nBufBase ;
			}

	protected:
		// 保存用ヘッダ
		struct	BUFFER_HEADER
		{
			DWORD	nBufSize ;
			DWORD	nBufBase ;
		} ;

	public:
		// 保存処理
		virtual SError SaveBuffer( SFileInterface * file ) ;
		// 復元処理
		virtual SError LoadBuffer( SFileInterface * file ) ;

	protected:
		// メモリアロケーション
		virtual BYTE * AllocateMemory( DWORD nBytes ) ;
		virtual BYTE * ReallocateMemory( BYTE * pbytBuf, DWORD nBytes ) ;
		virtual void FreeMemory( BYTE * pbytBuf ) ;
	} ;



	//////////////////////////////////////////////////////////////////////////
	// 二重バッファ（コード用バッファ）
	//////////////////////////////////////////////////////////////////////////

	class	DualBuffer	: public Buffer
	{
	protected:
		BYTE *	m_pbytShadow ;		// シャドウバッファ
		BYTE *	m_pbytCode ;		// コードバッファ（オリジナル・コピー）

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( DualBuffer, Buffer )
		// 構築関数
		DualBuffer( void ) : m_pbytShadow(NULL), m_pbytCode(NULL) {}
		DualBuffer( const DualBuffer & buf ) ;
		// 消滅関数
		virtual ~DualBuffer( void ) ;

	public:	// バッファ管理
		// バッファリサイズ
		virtual SError ResizeBuffer( DWORD nBytes, DWORD nBase = 0 ) ;
		// バッファリミット設定
		virtual SError ResizeBufferLimit( DWORD nLimit ) ;
		// バッファ解放
		virtual void FreeBuffer( void ) ;
		// シャドウバッファ生成
		virtual SError CreateShadowBuffer( void ) ;
		// バッファ複製
		SError CopyBufferFrom( const DualBuffer & buf ) ;

	public:
		// バッファ取得
		BYTE * GetShadowBuffer( void ) const
			{
				return	m_pbytShadow ;
			}
		// バッファ取得
		BYTE * GetCodeShadowBuffer( void ) const
			{
				return	m_pbytCode ;
			}
	public:
		// 保存処理
		virtual SError SaveBuffer( SFileInterface * file ) ;
		// 復元処理
		virtual SError LoadBuffer( SFileInterface * file ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 単純なバッファ・オブジェクト (class native SSystem::Buffer)
	//////////////////////////////////////////////////////////////////////////

	class	BufferObject	: public ECSSakura2::Object, public Buffer
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( BufferObject, ECSSakura2::Object, Buffer )
		// メモリマッピング
		virtual LinearAddressCache *
				GetSegmentBuffer( LinearAddressCache & seg ) ;

	public:
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 保存処理
		virtual SError SaveStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元処理
		virtual SError LoadStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 二重バッファ・オブジェクト (class native SSystem::DualBuffer)
	//////////////////////////////////////////////////////////////////////////

	class	DualBufferObject	: public ECSSakura2::Object, public DualBuffer
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( DualBufferObject, ECSSakura2::Object, DualBuffer )
		// メモリマッピング
		virtual LinearAddressCache *
				GetSegmentBuffer( LinearAddressCache & seg ) ;
		virtual BYTE * GetSegmentShadowBuffer( int iShadow = 0 ) ;

	protected:
		// 保存用ヘッダ
		struct	BUFFER_HEADER
		{
			DWORD	nBufSize ;
			DWORD	nBufBase ;
			DWORD	dwFlags ;
		} ;

	public:
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 保存処理
		virtual SError SaveStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元処理
		virtual SError LoadStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;

	} ;

}

// new SSystem::Buffer
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SSystem_Buffer) ;

// new SSystem::DualBuffer
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SSystem_DualBuffer) ;


#endif
