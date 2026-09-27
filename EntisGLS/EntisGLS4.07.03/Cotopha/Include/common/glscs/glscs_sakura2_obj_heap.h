
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_OBJECT_HEAP_H__)
#define	__GLSCS_SAKURA2_OBJECT_HEAP_H__

namespace	ECSSakura2
{
	using	SSystem::SError ;
	using	SSystem::SFileInterface ;


	//////////////////////////////////////////////////////////////////////////
	// オブジェクト・ヒープ
	//////////////////////////////////////////////////////////////////////////

	class	ObjectHeap	: public Object, public SSystem::SObjectArray<Object>
	{
	protected:
		unsigned int	m_nHeapCount ;		// 使用されているオブジェクト数
		unsigned int	m_iHeapNext ;		// 次に割り当てを実施する指標
		unsigned int	m_nSelector ;		// 割り当てるアドレス上位8ビット

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( ObjectHeap, Object )
		ESL_DECLARE_CLASS_OPERATOR_NEW( Object )
		// 構築関数
		ObjectHeap( int nSelector )
			: m_nHeapCount(0), m_iHeapNext(0), m_nSelector(nSelector) {}

	public:
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;

	public:
		// オブジェクトを割り当てる
		int AllocateObject( Object * pObj ) ;
		// オブジェクトを割り当てる
		int AllocateObjectAt( int nIndex, Object * pObj ) ;
		// オブジェクトを解放する
		SError FreeObjectAt
			( int nIndex, VirtualMachine * vm, Context * context ) ;
		// オブジェクトを分離する
		Object * DetachObjectAt( int nIndex ) ;
		// 全て削除
		void RemoveAll( VirtualMachine * vm, Context * context ) ;

	public:
		// ヒープの保存の準備処理
		virtual SError PrepareSave
			( VirtualMachine * vm, Context * context ) ;
		// オブジェクト・ヒープを保存する
		virtual SError SaveHeapStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		virtual SError SaveHeapDynamic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// オブジェクト・ヒープを復元する
		virtual SError LoadHeapStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		virtual SError LoadHeapDynamic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// ヒープの復元後処理
		virtual SError CommitAfterLoad
			( VirtualMachine * vm, Context * context ) ;
		// ヒープの復元後の後のスクリプト処理
		virtual SError OnLoadedDynamic
			( VirtualMachine * vm, Context * context ) ;

	protected:
		struct	HEAP_HEADER
		{
			uint32_t	nHeapCount ;
			uint32_t	iHeapNext ;
			uint32_t	nHeapLimit ;
		} ;
		// ヒープヘッダの保存
		SError SaveHeapHeader
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// ヒープヘッダの復元
		SError LoadHeapHeader
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;

	} ;

}

#endif

