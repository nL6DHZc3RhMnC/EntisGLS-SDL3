
/*****************************************************************************
                         E R I S A - L i b r a r y
 -----------------------------------------------------------------------------
      Copyright (C) 2002-2013 Leshade Entis, Entis-soft. All rights reserved.
 *****************************************************************************/


#if	!defined(__SAKURA_ERISA_CRYPT_CONTEXT_H__)
#define	__SAKURA_ERISA_CRYPT_CONTEXT_H__

#include <sakura/ssys_queue_buffer.h>

namespace	ERISA
{
	//////////////////////////////////////////////////////////////////////////
	// 簡易暗号化コンテキスト
	//////////////////////////////////////////////////////////////////////////

	class	SGLSimpleCrypt32Context
	{
	public:
		enum	BufferSize
		{
			bufferSizeInDWords	= 8,
			bufferSizeInBytes	= bufferSizeInDWords * 4,
			bufferSizeModMask	= bufferSizeInBytes - 1,
		} ;

	protected:
		// 暗号化／復号バッファ
		uint32_t	m_bufCrypt[bufferSizeInDWords] ;

		// 難読化マスク
		// ・パスワード UTF-8 のビット反転
		// 　長い場合は先頭にラップアランドして XOR
		// ・パスワード長（ラップアラウンド含む）がバッファ長に対して余る場合
		// 　パスワードの先頭からバイト単位でビット反転して擬似乱数との積を XOR
		// ・擬似乱数の初期値は 7
		// ・擬似乱数はパスワードが一巡するたびに更新
		// 　x' = x * 7 + 5
		// ・パスワードがない場合にはゼロで初期化
		uint32_t	m_bufSalt[bufferSizeInDWords] ;

		uint32_t	m_crcPassword ;		// 鍵マスク（パスワード CRC32）
		uint32_t	m_keyCrypt ;		// 暗号化／復号鍵
		size_t		m_nBuffered ;		// バッファに溜まっているバイト数

	public:
		// クラス情報
		ESL_DECLARE_NV_CLASS_INFO( SGLSimpleCrypt32Context )
		// 構築関数
		SGLSimpleCrypt32Context( void ) ;
		// 消滅関数
		~SGLSimpleCrypt32Context( void ) ;

	public:
		// 初期化
		void Initialize( const wchar_t * pszPassword ) ;
		// 暗号化鍵生成・復号鍵取得
		uint32_t GenerateKey( void ) ;
		// 復号鍵設定
		void SetDecryptKey( uint32_t keyDecrypt ) ;
		// パスワード・鍵情報複製
		void DuplicateKey( const SGLSimpleCrypt32Context& ctx ) ;

	public:
		// 暗号化／復号バッファにデータを追加
		size_t WriteData( const void * ptrSrc, size_t nBytes ) ;
		// データの終端処理
		void FlushData( void ) ;
		// バッファ状態判定
		bool IsBufferFull( void ) const
		{
			return	(m_nBuffered >= bufferSizeInBytes) ;
		}
		// 暗号化処理実行
		size_t EncryptBuffer( void ) ;
		// 復号化処理実行
		size_t DecryptBuffer( void ) ;
		// 暗号化／復号バッファ取得
		const void * GetCryptBuffer( void ) const
		{
			return	&m_bufCrypt[0] ;
		} ;
		// 暗号化／復号バッファからデータを取得
		size_t ReadData( void * ptrDst, size_t nBytes ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 簡易暗号化出力ストリーム
	//////////////////////////////////////////////////////////////////////////

	class	SGLEncrypt32OutputStream
				: public SSystem::SOutputStream, public SGLSimpleCrypt32Context
	{
	protected:
		SSystem::SOutputStream *	m_pStream ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLEncrypt32OutputStream, SOutputStream, SGLSimpleCrypt32Context )
		// 構築関数
		SGLEncrypt32OutputStream( SSystem::SOutputStream * pStream )
												: m_pStream( pStream ) {}

	public:
		// データの終端処理
		void FlushData( void ) ;

	public:	// SSystem::SOutputStream オーバーライド
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 簡易暗号復号入力ストリーム
	//////////////////////////////////////////////////////////////////////////

	class	SGLDecrypt32InputStream
				: public SSystem::SInputStream, public SGLSimpleCrypt32Context
	{
	protected:
		SSystem::SInputStream *	m_pStream ;

		uint8_t		m_bufDecrypt[bufferSizeInBytes * 0x11] ;
		size_t		m_nDecrypted ;
		size_t		m_iNextOutput ;
		bool		m_flagEOF ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLDecrypt32InputStream, SInputStream, SGLSimpleCrypt32Context )
		// 構築関数
		SGLDecrypt32InputStream( SSystem::SInputStream * pStream )
			: m_pStream( pStream ), m_nDecrypted( 0 ),
						m_iNextOutput( 0 ), m_flagEOF( false ) {}

	public:	// SSystem::SInputStream オーバーライド
		// ファイルから読み込み
		virtual size_t Read( void * ptrBuf, size_t nBytes ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 簡易暗号化ファイル読み込みフィルタ
	//////////////////////////////////////////////////////////////////////////

	class	SGLDecrypt32File	: public SSystem::SFileInterface
	{
	public:
		struct	FILE_HEADER
		{
			uint8_t		chSign[4] ;		// 'scpd'
			uint32_t	keyDecrypt ;	// key to decrypt
			uint64_t	nBytes ;		// original file length in bytes
			uint64_t	nReserved ;		// must be zero
		} ;

	protected:
		SSystem::SFileInterface *
					m_pFile ;				// 読み込み元ファイル
		bool		m_flagOwnFile ;
		FILE_HEADER	m_fhHeader ;			// ファイルヘッダ
		int64_t		m_fpBasePos ;			// 入力基準ファイルポインタ
		SGLSimpleCrypt32Context
					m_context ;				// 復号器
		int64_t		m_fpPos ;				// ファイルポインタ
		SSystem::SArray<uint8_t>
					m_bufDataCache ;		// 暗号データ読み込みキャッシュ
		int64_t		m_fpDataCache ;			// 読み込みキャッシュファイル位置
		size_t		m_nDataCacheBytes ;		// 読み込みキャッシュバイト数
		uint8_t		m_bufDecrypt
						[SGLSimpleCrypt32Context::bufferSizeInBytes] ;
											// 復号済みバッファ
		size_t		m_nDecryptBytes ;		// 復号済みバイト数
		int64_t		m_fpDecryptCache ;		// 復号済みキャッシュファイル位置

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLDecrypt32File, SFileInterface )
		// 構築関数
		SGLDecrypt32File( void ) ;
		// 消滅関数
		virtual ~SGLDecrypt32File( void ) ;

	public:
		// ファイルを開く
		SSystem::SError Open
			( SFileInterface * pFile,
				bool flagOwnFile, const wchar_t * pwszPassword ) ;
		// ファイルを閉じる
		void Close( void ) ;

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
		virtual SSystem::SError SetEndOfFile( void ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 簡易暗号化ファイル書き出し
	//////////////////////////////////////////////////////////////////////////

	class	SGLEncrypt32FileWriter	: public SSystem::SFileInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLEncrypt32FileWriter, SFileInterface )
		// 構築関数
		SGLEncrypt32FileWriter( void ) ;
		// 消滅関数
		virtual ~SGLEncrypt32FileWriter( void ) ;

	protected:
		SSystem::SFileInterface *		m_pFile ;		// 出力ファイル
		bool							m_flagOwnFile ;
		SSystem::SQueueBuffer			m_qbufTemp ;
		SGLSimpleCrypt32Context			m_context ;		// 暗号器
		int64_t							m_fpHeaderPos ;	// ヘッダ位置
		SGLDecrypt32File::FILE_HEADER	m_fhHeader ;	// ヘッダ

	public:
		// ファイルを開く
		SSystem::SError Open
			( SFileInterface * pFile,
				bool flagOwnFile, const wchar_t * pwszPassword ) ;
		// ファイルを閉じる
		void Close( void ) ;

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
		virtual SSystem::SError SetEndOfFile( void ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 簡易暗号化ファイル・オープナー
	//////////////////////////////////////////////////////////////////////////

	class	SGLDecrypt32FileOpener	: public SSystem::SOffsetFileOpener
	{
	protected:
		SSystem::SString	m_strPassword ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLDecrypt32FileOpener, SOffsetFileOpener )
		// 構築関数
		SGLDecrypt32FileOpener
			( const wchar_t * pszBasePath,
				wchar_t wchSeparator,
				SFileOpener * pOpener, bool flagOwner,
				const wchar_t * pszPassword ) ;
		// 消滅関数
		virtual ~SGLDecrypt32FileOpener( void ) ;

	public:
		// ファイルを開く
		virtual SSystem::SFileInterface * NewOpenFile
			( const wchar_t * pszFilePath, long int nOpenFlags ) ;
	} ;


	#if	!defined(__COTOPHA__)
	//////////////////////////////////////////////////////////////////////////
	// （符号なし）大きな数 (32bit単位)
	//////////////////////////////////////////////////////////////////////////

	template <int N> class	BigNumber
	{
	public:
		uint32_t	m_num[N] ;

	public:
		// 構築
		BigNumber( uint32_t n = 0 )
		{
			m_num[0] = n ;
			for ( int i = 1; i < N; i ++ )
			{
				m_num[i] = 0 ;
			}
		}
		BigNumber( const BigNumber<N>& bn )
		{
			for ( int i = 0; i < N; i ++ )
			{
				m_num[i] = bn.m_num[i] ;
			}
		}
		BigNumber( const uint32_t * pNum, size_t n )
		{
			if ( n > N )
			{
				n = N ;
			}
			for ( size_t i = 0; i < n; i ++ )
			{
				m_num[i] = pNum[i] ;
			}
			for ( size_t i = n; i < N; i ++ )
			{
				m_num[i] = 0 ;
			}
		}
		// 代入
		const BigNumber<N>& operator = ( const BigNumber<N>& bn )
		{
			for ( int i = 0; i < N; i ++ )
			{
				m_num[i] = bn.m_num[i] ;
			}
			return	*this ;
		}
		const BigNumber<N>& operator = ( uint32_t n )
		{
			m_num[0] = n ;
			for ( int i = 1; i < N; i ++ )
			{
				m_num[i] = 0 ;
			}
			return	*this ;
		}
		// 加算
		const BigNumber<N>& operator += ( const BigNumber<N>& bn )
		{
			uint64_t	c = 0 ;
			for ( int i = 0; i < N; i ++ )
			{
				c += (uint64_t) m_num[i] + bn.m_num[i] ;
				m_num[i] = (uint32_t) c ;
				c >>= 32 ;
			}
			return	*this ;
		}
		const BigNumber<N>& operator += ( uint32_t n )
		{
			BigNumber<N>	bn( n ) ;
			return	*this += bn ;
		}
		BigNumber<N> operator + ( const BigNumber<N>& bn ) const
		{
			BigNumber<N>	bt( *this ) ;
			bt += bn ;
			return	bt ;
		}
		BigNumber<N> operator + ( uint32_t n ) const
		{
			BigNumber<N>	bn( n ) ;
			bn += *this ;
			return	bn ;
		}
		// 減算
		uint32_t Subtract( const BigNumber<N>& bn )
		{
			uint64_t	c = 0 ;
			for ( int i = 0; i < N; i ++ )
			{
				c = (uint64_t) m_num[i] - bn.m_num[i] - (c & 0x01) ;
				m_num[i] = (uint32_t) c ;
				c >>= 32 ;
			}
			return	(uint32_t) (c & 0x01) ;
		}
		const BigNumber<N>& operator -= ( const BigNumber<N>& bn )
		{
			Subtract( bn ) ;
			return	*this ;
		}
		const BigNumber<N>& operator -= ( uint32_t n )
		{
			BigNumber<N>	bn( n ) ;
			Subtract( bn ) ;
			return	*this ;
		}
		BigNumber<N> operator - ( const BigNumber<N>& bn ) const
		{
			BigNumber<N>	bt( *this ) ;
			bt -= bn ;
			return	bt ;
		}
		BigNumber<N> operator - ( uint32_t n ) const
		{
			BigNumber<N>	bt( *this ) ;
			BigNumber<N>	bn( n ) ;
			bt -= bn ;
			return	bt ;
		}
		// 比較
		bool IsEqual( const BigNumber<N>& bn ) const
		{
			for ( int i = 0; i < N; i ++ )
			{
				if ( m_num[i] != bn.m_num[i] )
				{
					return	false ;
				}
			}
			return	true ;
		}
		bool IsZero( void ) const
		{
			for ( int i = 0; i < N; i ++ )
			{
				if ( m_num[i] != 0 )
				{
					return	false ;
				}
			}
			return	true ;
		}
		bool IsSign( void ) const
		{
			return	(m_num[N - 1] & 0x80000000) != 0 ;
		}
		bool operator == ( const BigNumber<N>& bn ) const
		{
			return	IsEqual( bn ) ;
		}
		bool operator != ( const BigNumber<N>& bn ) const
		{
			return	!IsEqual( bn ) ;
		}
		bool operator == ( uint32_t n ) const
		{
			BigNumber<N>	bn( n ) ;
			return	IsEqual( bn ) ;
		}
		bool operator != ( uint32_t n ) const
		{
			BigNumber<N>	bn( n ) ;
			return	!IsEqual( bn ) ;
		}
		bool operator > ( const BigNumber<N>& bn ) const
		{
			BigNumber<N>	t( *this ) ;
			return	(t.Subtract( bn ) == 0) && !t.IsZero() ;
		}
		bool operator >= ( const BigNumber<N>& bn ) const
		{
			BigNumber<N>	t( *this ) ;
			return	(t.Subtract( bn ) == 0) ;
		}
		bool operator < ( const BigNumber<N>& bn ) const
		{
			BigNumber<N>	t( *this ) ;
			return	(t.Subtract( bn ) != 0) ;
		}
		bool operator <= ( const BigNumber<N>& bn ) const
		{
			BigNumber<N>	t( *this ) ;
			return	(t.Subtract( bn ) != 0) || t.IsZero() ;
		}
		// シフト
		const BigNumber<N>& operator <<= ( uint32_t n )
		{
			ESLAssert( (n > 0) && (n <= 32) ) ;
			uint32_t	c = 0 ;
			uint32_t	m = 32 - n ;
			for ( int i = 0; i < N; i ++ )
			{
				uint32_t	t = m_num[i] >> m ;
				m_num[i] = (m_num[i] << n) | c ;
				c = t ;
			}
			return	*this ;
		}
		const BigNumber<N>& operator >>= ( uint32_t n )
		{
			ESLAssert( (n > 0) && (n <= 32) ) ;
			uint32_t	c = 0 ;
			uint32_t	m = 32 - n ;
			for ( int i = N - 1; i >= 0; i -- )
			{
				uint32_t	t = m_num[i] << m ;
				m_num[i] = (m_num[i] >> n) | c ;
				c = t ;
			}
			return	*this ;
		}
		// ビット演算
		const BigNumber<N>& operator &= ( const BigNumber<N>& bn )
		{
			for ( int i = 0; i < N; i ++ )
			{
				m_num[i] &= bn.m_num[i] ;
			}
			return	*this ;
		}
		uint32_t operator & ( uint32_t n ) const
		{
			return	(m_num[0] & n) ;
		}
		const BigNumber<N>& operator |= ( const BigNumber<N>& bn )
		{
			for ( int i = 0; i < N; i ++ )
			{
				m_num[i] |= bn.m_num[i] ;
			}
			return	*this ;
		}
		const BigNumber<N>& operator ^= ( const BigNumber<N>& bn )
		{
			for ( int i = 0; i < N; i ++ )
			{
				m_num[i] ^= bn.m_num[i] ;
			}
			return	*this ;
		}
		// 乗算
		BigNumber<N> operator * ( const BigNumber<N>& bn ) const
		{
			BigNumber<N>	bnDst( 0 ) ;
			for ( uint32_t i = 0; i < N; i ++ )
			{
				BigNumber<N>	bnTemp ;
				bnTemp.ShiftMul( *this, bn.m_num[i], i ) ;
				bnDst += bnTemp ;
			}
			return	bnDst ;
		}
		void ShiftMul( const BigNumber<N>& bn, uint32_t n, uint32_t m )
		{
			for ( size_t i = 0; i < m; i ++ )
			{
				m_num[i] = 0 ;
			}
			uint64_t	c = 0 ;
			for ( size_t i = m; i < N; i ++ )
			{
				uint64_t	t = (uint64_t) bn.m_num[i - m] * n ;
				uint32_t	tl = (uint32_t) t ;
				c += tl ;
				m_num[i] = (uint32_t) c ;
				c >>= 32 ;
				c += (t >> 32) ;
			}
		}
		BigNumber<N> operator * ( uint32_t n ) const
		{
			BigNumber<N>	bn( *this ) ;
			bn.ShiftMul( *this, n, 0 ) ;
			return	bn ;
		}
		const BigNumber<N>& operator *= ( uint32_t n )
		{
			BigNumber<N>	bn( *this ) ;
			ShiftMul( bn, n, 0 ) ;
			return	*this ;
		}
		const BigNumber<N>& operator *= ( const BigNumber<N>& bn )
		{
			return	operator = ( *this * bn ) ;
		}
		// 剰余・除算
		BigNumber<N> Division( const BigNumber<N>& bn )
		{
			BigNumber<N>	n = bn ;
			BigNumber<N>	m = 1 ;
			BigNumber<N>	s = 0 ;
			//
			while ( (n < *this) && !n.IsSign() )
			{
				n <<= 1 ;
				m <<= 1 ;
			}
			for ( ; ; )
			{
				if ( Subtract( n ) )
				{
					*this += n ;
				}
				else
				{
					s |= m ;
				}
				if ( m & 0x01 )
				{
					break ;
				}
				n >>= 1 ;
				m >>= 1 ;
			}
			return	s ;
		}
		BigNumber<N> operator / ( const BigNumber<N>& bn ) const
		{
			BigNumber<N>	t = *this ;
			return	t.Division( bn ) ;
		}
		BigNumber<N> operator % ( const BigNumber<N>& bn ) const
		{
			BigNumber<N>	t = *this ;
			t.Division( bn ) ;
			return	t ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// （符号あり）大きな数 (32bit単位)
	//////////////////////////////////////////////////////////////////////////

	template <int N> class	SignedBigNumber	: public BigNumber<N>
	{
	public:
		// 構築
		SignedBigNumber( int32_t n = 0 )
		{
			BigNumber<N>::m_num[0] = n ;
			n >>= 1 ;
			for ( int i = 1; i < N; i ++ )
			{
				BigNumber<N>::m_num[i] = n ;
			}
		}
		SignedBigNumber( const BigNumber<N>& bn )
		{
			for ( int i = 0; i < N; i ++ )
			{
				BigNumber<N>::m_num[i] = bn.m_num[i] ;
			}
		}
		// 代入
		const SignedBigNumber<N>& operator = ( const BigNumber<N>& bn )
		{
			for ( int i = 0; i < N; i ++ )
			{
				BigNumber<N>::m_num[i] = bn.m_num[i] ;
			}
			return	*this ;
		}
		const SignedBigNumber<N>& operator = ( int32_t n )
		{
			BigNumber<N>::m_num[0] = n ;
			n >>= 1 ;
			for ( int i = 1; i < N; i ++ )
			{
				BigNumber<N>::m_num[i] = n ;
			}
			return	*this ;
		}
		// 加算
		const SignedBigNumber<N>& operator += ( const BigNumber<N>& bn )
		{
			BigNumber<N>::operator += ( bn ) ;
			return	*this ;
		}
		const SignedBigNumber<N>& operator += ( int32_t n )
		{
			SignedBigNumber<N>	bn( n ) ;
			return	*this += bn ;
		}
		SignedBigNumber<N> operator + ( const BigNumber<N>& bn ) const
		{
			SignedBigNumber<N>	bt( *this ) ;
			bt += bn ;
			return	bt ;
		}
		SignedBigNumber<N> operator + ( int32_t n ) const
		{
			SignedBigNumber<N>	bn( n ) ;
			bn += *this ;
			return	bn ;
		}
		// 減算
		const SignedBigNumber<N>& operator -= ( const BigNumber<N>& bn )
		{
			BigNumber<N>::Subtract( bn ) ;
			return	*this ;
		}
		const SignedBigNumber<N>& operator -= ( int32_t n )
		{
			SignedBigNumber<N>	bn( n ) ;
			BigNumber<N>::Subtract( bn ) ;
			return	*this ;
		}
		SignedBigNumber<N> operator - ( const BigNumber<N>& bn ) const
		{
			SignedBigNumber<N>	bt( *this ) ;
			bt -= bn ;
			return	bt ;
		}
		SignedBigNumber<N> operator - ( int32_t n ) const
		{
			SignedBigNumber<N>	bt( *this ) ;
			SignedBigNumber<N>	bn( n ) ;
			bt -= bn ;
			return	bt ;
		}
		// 符号反転
		SignedBigNumber<N> operator - ( void ) const
		{
			SignedBigNumber<N>	bn( 0 ) ;
			SignedBigNumber<N>	bt( *this ) ;
			bn -= bt ;
			return	bn ;
		}
		// 比較
		bool operator > ( const SignedBigNumber<N>& bn ) const
		{
			SignedBigNumber<N>	t( *this ) ;
			return	((t.Subtract( bn ) == 0) && !t.IsZero())
						|| (!BigNumber<N>::IsSign() && bn.IsSign()) ;
		}
		bool operator >= ( const SignedBigNumber<N>& bn ) const
		{
			SignedBigNumber<N>	t( *this ) ;
			return	(t.Subtract( bn ) == 0)
						|| (!BigNumber<N>::IsSign() && bn.IsSign()) ;
		}
		bool operator < ( const SignedBigNumber<N>& bn ) const
		{
			SignedBigNumber<N>	t( *this ) ;
			return	(t.Subtract( bn ) != 0)
						|| (BigNumber<N>::IsSign() && !bn.IsSign()) ;
		}
		bool operator <= ( const SignedBigNumber<N>& bn ) const
		{
			SignedBigNumber<N>	t( *this ) ;
			return	((t.Subtract( bn ) != 0) || t.IsZero())
						|| (BigNumber<N>::IsSign() && !bn.IsSign()) ;
		}
		// シフト
		const SignedBigNumber<N>& operator >>= ( uint32_t n )
		{
			ESLAssert( (n > 0) && (n <= 32) ) ;
			uint32_t	m = 32 - n ;
			uint32_t	c = BigNumber<N>::m_num[N - 1] << m ;
			BigNumber<N>::m_num[N - 1] =
						((int32_t) BigNumber<N>::m_num[N - 1] >> n) ;
			for ( int i = N - 2; i >= 0; i -- )
			{
				uint32_t	t = BigNumber<N>::m_num[i] << m ;
				BigNumber<N>::m_num[i] = (BigNumber<N>::m_num[i] >> n) | c ;
				c = t ;
			}
			return	*this ;
		}
		// 剰余・除算
		SignedBigNumber<N> Division( const SignedBigNumber<N>& bn )
		{
			SignedBigNumber<N>	bt = bn ;
			bool				fRSign = false ;
			bool				fSSign = false ;
			if ( BigNumber<N>::IsSign() )
			{
				fRSign = true ;
				fSSign = true ;
				*this = - *this ;
			}
			if ( bt.IsSign() )
			{
				fRSign = !fRSign ;
				bt = - bt ;
			}
			SignedBigNumber<N>	br = BigNumber<N>::Division( bt ) ;
			if ( fRSign )
			{
				br = - br ;
			}
			if ( fSSign )
			{
				*this = - *this ;
			}
			return	br ;
		}
		SignedBigNumber<N> operator / ( const SignedBigNumber<N>& bn ) const
		{
			SignedBigNumber<N>	t = *this ;
			return	t.Division( bn ) ;
		}
		SignedBigNumber<N> operator % ( const SignedBigNumber<N>& bn ) const
		{
			SignedBigNumber<N>	t = *this ;
			t.Division( bn ) ;
			return	t ;
		}

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 簡易 RSA 暗号化コンテキスト (96bit)
	//////////////////////////////////////////////////////////////////////////

	class	RSA96CryptContext
	{
	public:
		enum	BufferSize
		{
			plainInBytes	= 11,		// 平文ブロックサイズ
			cryptInBytes	= 12,		// 暗号文ブロックサイズ
		} ;

	protected:
		BigNumber<6>	m_bnSaltForData ;
		BigNumber<6>	m_bnSaltForKey ;
		BigNumber<6>	m_keyEncrypt ;
		BigNumber<6>	m_keyDecrypt ;
		BigNumber<6>	m_keyMod ;

	public:
		// クラス情報
		ESL_DECLARE_NV_CLASS_INFO( RSA96CryptContext )
		// 構築関数
		RSA96CryptContext( void ) ;
		// 消滅関数
		~RSA96CryptContext( void ) ;

	public:
		// 初期化
		void Initialize( const wchar_t * pszPassword ) ;
		// 暗号化鍵生成・復号鍵取得
		BigNumber<3> GenerateKey( void ) ;
		// 公開鍵設定
		void SetPublicKey( const BigNumber<3>& keyPublic ) ;

	protected:
		// 暗号鍵用素数を生成
		static BigNumber<6> GeneratePrime( SakuraCL::SCLRandomizer& rand ) ;

	public:
		// 暗号化処理実行
		void Encrypt( BigNumber<3>& bnCrypt, const uint8_t * pbytSrc ) ;
		// 復号化処理実行
		void Decrypt( uint8_t * pbytDst, const BigNumber<3>& bnCrypt ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 簡易 RSA 暗号化出力ストリーム
	//////////////////////////////////////////////////////////////////////////

	class	SGLEncryptRSA96OutputStream
				: public SSystem::SOutputStream, public RSA96CryptContext
	{
	protected:
		SSystem::SOutputStream *	m_pStream ;
		uint8_t	m_bufSrc[plainInBytes * cryptInBytes] ;
		uint8_t	m_bufDst[cryptInBytes * cryptInBytes] ;
		size_t	m_nBuffered ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLEncryptRSA96OutputStream, SOutputStream, RSA96CryptContext )
		// 構築関数
		SGLEncryptRSA96OutputStream( SSystem::SOutputStream * pStream )
								: m_pStream( pStream ), m_nBuffered( 0 ) {}

	public:
		// データの終端処理
		void FlushData( void ) ;

	public:	// SSystem::SOutputStream オーバーライド
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 簡易 RSA 復号入力ストリーム
	//////////////////////////////////////////////////////////////////////////

	class	SGLDecryptRSA96InputStream
				: public SSystem::SInputStream, public RSA96CryptContext
	{
	protected:
		SSystem::SInputStream *	m_pStream ;

		uint8_t	m_bufSrc[cryptInBytes * cryptInBytes] ;
		uint8_t	m_bufDst[plainInBytes * cryptInBytes] ;
		size_t	m_nSrcBuffered ;
		size_t	m_nDstBuffered ;
		size_t	m_iDstOffset ;
		bool	m_flagEOF ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLDecryptRSA96InputStream, SInputStream, RSA96CryptContext )
		// 構築関数
		SGLDecryptRSA96InputStream( SSystem::SInputStream * pStream )
			: m_pStream( pStream ), m_nSrcBuffered( 0 ),
				m_nDstBuffered( 0 ), m_iDstOffset( 0 ), m_flagEOF( false ) {}

	public:	// SSystem::SInputStream オーバーライド
		// ファイルから読み込み
		virtual size_t Read( void * ptrBuf, size_t nBytes ) ;

	} ;
	#endif

}

#endif
