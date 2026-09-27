
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_OBJECT_VOLATILE_H__)
#define	__GLSCS_SAKURA2_OBJECT_VOLATILE_H__

namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// 揮発性オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	ECSVolatileObject	: public Object
	{
	protected:
		enum	ProcedureVector
		{
			procVectorOnLoaded,
			procVectorCount,
		} ;
		INT64	m_addrProc ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( ECSSakura2::ECSVolatileObject, Object )
		// 構築関数
		ECSVolatileObject( void ) : m_addrProc(0) {}
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 保存処理
		virtual SSystem::SError SaveStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元処理
		virtual SSystem::SError LoadStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元後の後のスクリプト処理
		virtual SError OnLoadedDynamic
			( VirtualMachine * vm, Context * context ) ;

	public:
		// インターフェース設定
		void AttachVolatileInterface( INT64 addrProc ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ネイティブ・オブジェクト（ランタイム派生）
	//////////////////////////////////////////////////////////////////////////

	class	RuntimeObject	: public ECSVolatileObject
	{
	protected:
		ESLObject *		m_pObj ;
		const wchar_t *	m_pwszType ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RuntimeObject, ECSVolatileObject )
		// 構築関数
		RuntimeObject( void ) ;
		RuntimeObject( ESLObject * pObj, const wchar_t * pwszType ) ;
		// 消滅関数
		virtual ~RuntimeObject( void ) ;

	public:
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// オブジェクト取得
		ESLObject * GetObject( void ) const ;
	} ;

}

// new SSystem::VolatileObject
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SSystem_VolatileObject) ;

// void SSystem::VolatileObject::AttachVolatileInterface
//				( SVolatileInterface * pInterface ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_VolatileObject_AttachVolatileInterface) ;

#endif
