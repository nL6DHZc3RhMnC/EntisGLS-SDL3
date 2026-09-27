
#if	!defined(__SAKURA2_STACK_BUFFER_H__)
#define	__SAKURA2_STACK_BUFFER_H__

namespace	SSystem
{
	//////////////////////////////////////////////////////////////////////////
	// 積層バッファ
	//////////////////////////////////////////////////////////////////////////

	class	SStackBuffer	: public SObject
	{
	protected:
		class	Buffer	: public SArray<uint8_t>
		{
		public:
			size_t	m_nUsed ;
		public:
			Buffer( void ) : m_nUsed(0) { }
			void Initialize( void )
			{
				// ※16バイトアライメント
				m_nUsed = 0x10 - (((size_t) ((long_ptr_t) GetConstArray())) & 0x0F) ;

				#if	defined(__DEBUG__)
				// メモリ境界テスト用
				uint8_t *	pMem = GetArray() + m_nUsed ;
				pMem[0] = (uint8_t) 'E' ;
				pMem[1] = (uint8_t) 'N' ;
				pMem[2] = (uint8_t) 'T' ;
				pMem[3] = (uint8_t) 'S' ;
				FinishArray() ;
				#endif
			}
		} ;
		SObjectArray<Buffer>	m_stackBlock ;
		SObjectArray<Buffer>	m_stackLargeBlock ;
		size_t					m_nBlockSize ;
		size_t					m_iUsingBlock ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SSystem::SStackBuffer, SObject )
		// 構築関数
		SStackBuffer( void ) ;
		// 消滅関数
		virtual ~SStackBuffer( void ) ;

	public:
		// メモリ確保
		uint8_t * Allocate( size_t nBytes ) ;
		// メモリ解放
		SError Free( uint8_t * ptrBuf ) ;
		// 全メモリ解放
		SError FreeAll( void ) ;
		// 使用中のメモリサイズ取得
		size_t GetUsedSize( void ) const ;
		// メモリブロックサイズ設定
		void SetBlockSize( size_t nBytes ) ;

	} ;

}

#endif

