
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_HEAP_BUFFER_H__)
#define	__GLSCS_SAKURA2_HEAP_BUFFER_H__

#include <sakura/ssys_heap_memory.h>

namespace	ECSSakura2
{
	using	ECSSakura2Processor::LinearAddressCache ;
	using	ECSSakura2Processor::Context ;
	using	SSystem::SError ;
	using	SSystem::SFileInterface ;

	//////////////////////////////////////////////////////////////////////////
	// ヒープ用バッファ
	//////////////////////////////////////////////////////////////////////////

	class	HeapBuffer	: public Buffer
	{
	public:
	protected:
		// ヒープ管理
		SSystem::SHeapManager		m_heapManager ;

		// 排他同期ミューテックス
		SSystem::SCriticalSection	m_csMutex ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( HeapBuffer, Buffer )
		// 構築関数
		HeapBuffer( void ) ;
		// 消滅関数
		virtual ~HeapBuffer( void ) ;

	public:
		// バッファ生成
		virtual SError CreateBuffer( DWORD dwBytes, DWORD dwBase = 0 ) ;
		// メモリブロック確保
		virtual bool AllocateHeapBlock( DWORD& dwAddr, DWORD dwBytes ) ;
		// メモリブロック再確保
		virtual bool ReallocateHeapBlock( DWORD& dwAddr, DWORD dwBytes ) ;
		// メモリブロック解放
		virtual void FreeHeapBlock( DWORD dwAddr ) ;
		// メモリブロックのサイズ取得
		virtual DWORD GetHeapBlockLength( DWORD dwAddr ) const ;
		// 空のヒープブロックか判定
		virtual bool IsEmptyHeap( void ) const ;

	public:
		// 同期
		void Lock( void )
		{
			m_csMutex.Lock() ;
		}
		void Unlock( void )
		{
			m_csMutex.Unlock() ;
		}

	public:
		// 保存処理
		virtual SError SaveBuffer( SFileInterface * file ) ;
		// 復元処理
		virtual SError LoadBuffer( SFileInterface * file ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ヒープ用バッファオブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	HeapBufferObject	: public ECSSakura2::Object, public HeapBuffer
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( HeapBufferObject, ECSSakura2::Object, HeapBuffer )
		// メモリマッピング
		virtual LinearAddressCache *
				GetSegmentBuffer( LinearAddressCache & seg ) ;

	protected:
		// 保存用ヘッダ
		struct	BUFFER_HEADER
		{
			DWORD	nBufSize ;
			DWORD	nBufBase ;
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

// new SSystem::HeapBuffer
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SSystem_HeapBuffer) ;


#endif
