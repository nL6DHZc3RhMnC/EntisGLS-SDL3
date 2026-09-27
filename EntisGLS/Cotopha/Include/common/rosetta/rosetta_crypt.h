
#if	!defined(__ROSETTA_CRYPT_H__)
#define	__ROSETTA_CRYPT_H__

#include <sakuracl/erisa/sgl_erisa_md5_context.h>

namespace	Rosetta
{
	//////////////////////////////////////////////////////////////////////////
	// CRC32Context 型オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	RSCRC32Context	: public RSObject
	{
	public:
		SakuraCL::CRC32Context	m_context ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSCRC32Context, RSObject )
		// 構築関数
		RSCRC32Context( RSClass * pClass )
				: RSObject( pClass, typeOther ) { }
		RSCRC32Context
			( RSClass * pClass, const SakuraCL::CRC32Context& crc32 )
				: RSObject( pClass, typeOther ), m_context( crc32 ) { }

	public:	// オブジェクト
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// CRC32Context クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSCRC32ContextClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSCRC32ContextClass, RSClass )
		// 構築関数
		RSCRC32ContextClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"CRC32Context" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	protected:
		// this オブジェクトのファイルを取得
		static RSCRC32Context *
				GetThisCRC32( RSContext& context, RSObject* pThis ) ;

	public:	// CRC32Context method
		// void <init>( void )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void stream( Uint8Pointer ptrData, int nBytes )
		static RSObject * method_stream
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getCRC32()
		static RSObject * method_getCRC32
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// MD5DigestContext 型オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	RSMD5DigestContext	: public RSObject
	{
	public:
		SakuraCL::MD5Context	m_context ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSMD5DigestContext, RSObject )
		// 構築関数
		RSMD5DigestContext( RSClass * pClass )
				: RSObject( pClass, typeOther ) { }

	public:	// オブジェクト
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// MD5DigestContext クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSMD5DigestContextClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSMD5DigestContextClass, RSClass )
		// 構築関数
		RSMD5DigestContextClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"MD5DigestContext" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	protected:
		// this オブジェクトのファイルを取得
		static RSMD5DigestContext *
				GetThisMD5( RSContext& context, RSObject* pThis ) ;

	public:	// MD5DigestContext method
		// void <init>( void )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void stream( Uint8Pointer ptrData, int nBytes )
		static RSObject * method_stream
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void flush()
		static RSObject * method_flush
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void getDigest( Uint8Pointer digest )
		static RSObject * method_getDigest
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String getDigestHex()
		static RSObject * method_getDigestHex
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// Randomizer 型オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	RSRondomizer	: public RSObject
	{
	public:
		SakuraCL::SCLRandomizer	m_random ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSRondomizer, RSObject )
		// 構築関数
		RSRondomizer( RSClass * pClass )
				: RSObject( pClass, typeOther )
		{
			m_random.InitializeSeed() ;
		}

	public:	// オブジェクト
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// Randomizer クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSRandomizerClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSRandomizerClass, RSClass )
		// 構築関数
		RSRandomizerClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Randomizer" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	protected:
		// this オブジェクトのファイルを取得
		static RSRondomizer *
				GetThisRandomizer( RSContext& context, RSObject* pThis ) ;

	public:	// Randomizer method
		// void <init>( void )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void initSeed( int seed )
		static RSObject * method_initSeed
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int randomize( int nRange )
		static RSObject * method_randomize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int quickInt( int nRange )
		static RSObject * method_quickInt
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// float quickFloat( float nRange )
		static RSObject * method_quickFloat
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// Encrypt32OutputStream クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSEncrypt32OutputStreamClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSEncrypt32OutputStreamClass, RSClass )
		// 構築関数
		RSEncrypt32OutputStreamClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Encrypt32OutputStream" ) ;
		// メンバ初期設定
		void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	protected:
		// this オブジェクトのファイルを取得
		static ERISA::SGLEncrypt32OutputStream *
				GetThisOutputStream( RSContext& context, RSObject* pThis ) ;

	public:	// Encrypt32OutputStream method
		// void <init>( OutputStream stream )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void initialize( String password )
		static RSObject * method_initialize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int generateKey()
		static RSObject * method_generateKey
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void flushData()
		static RSObject * method_flushData
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Decrypt32InputStream クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSDecrypt32InputStreamClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSDecrypt32InputStreamClass, RSClass )
		// 構築関数
		RSDecrypt32InputStreamClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Decrypt32InputStream" ) ;
		// メンバ初期設定
		void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	protected:
		// this オブジェクトのファイルを取得
		static ERISA::SGLDecrypt32InputStream *
				GetThisInputStream( RSContext& context, RSObject* pThis ) ;

	public:	// Decrypt32InputStream method
		// void <init>( InputStream stream )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void initialize( String password )
		static RSObject * method_initialize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setDecryptKey( int key )
		static RSObject * method_setDecryptKey
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// EncryptRSA96OutputStream クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSEncryptRSA96OutputStreamClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSEncryptRSA96OutputStreamClass, RSClass )
		// 構築関数
		RSEncryptRSA96OutputStreamClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"EncryptRSA96OutputStream" ) ;
		// メンバ初期設定
		void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	protected:
		// this オブジェクトのファイルを取得
		static ERISA::SGLEncryptRSA96OutputStream *
				GetThisOutputStream( RSContext& context, RSObject* pThis ) ;

	public:	// EncryptRSA96OutputStream method
		// void <init>( OutputStream stream )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void initialize( String password )
		static RSObject * method_initialize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setPublicKey( Uint8Pointer key )
		static RSObject * method_setPublicKey
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void flushData()
		static RSObject * method_flushData
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// DecryptRSA96InputStream クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSDecryptRSA96InputStreamClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSDecryptRSA96InputStreamClass, RSClass )
		// 構築関数
		RSDecryptRSA96InputStreamClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"DecryptRSA96InputStream" ) ;
		// メンバ初期設定
		void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	protected:
		// this オブジェクトのファイルを取得
		static ERISA::SGLDecryptRSA96InputStream *
				GetThisInputStream( RSContext& context, RSObject* pThis ) ;

	public:	// DecryptRSA96InputStream method
		// void <init>( InputStream stream )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void initialize( String password )
		static RSObject * method_initialize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Uint8Pointer generateKey()
		static RSObject * method_generateKey
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;

}

#endif

