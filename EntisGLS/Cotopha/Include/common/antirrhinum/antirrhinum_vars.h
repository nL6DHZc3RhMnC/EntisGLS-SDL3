
#if	!defined(__ANTIRRHINUM_VARIABLES_H__)
#define	__ANTIRRHINUM_VARIABLES_H__

#include <sakura/ssys_bit_array.h>


namespace	AntirrhinumGL
{
	//////////////////////////////////////////////////////////////////////////
	// フラグ管理
	//////////////////////////////////////////////////////////////////////////

	class	AGLVariablesProcessor	: public AGLEpicFuncProcessor
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( AGLVariablesProcessor, AGLEpicFuncProcessor )
		AGL_DECLARE_EPIC_PROCESSOR( AGLVariablesProcessor )
		// 構築関数
		AGLVariablesProcessor( void ) ;
		// 消滅関数
		virtual ~AGLVariablesProcessor( void ) ;

	protected:
		Rosetta::RSVirtualMachine *	m_pVM ;
		AGLScriptContext	m_context ;

		AGLScriptObject		m_pGameFlags ;		// 通常のフラグ : HashMap
		AGLScriptObject		m_pSharedFlags ;	// 共有フラグ : HashMap
		AGLScriptObject		m_pReadFlags ;		// 既読フラグ : HashMap<String> : キーはスクリプト名、String は base64
		AGLScriptObject		m_pDeedFlags ;		// 功績フラグ : HashMap<Integer>

		SSystem::SString	m_strReadElement ;
		SSystem::SString	m_strDeedElement ;


		class	ReadHistory	: public SSystem::SBitArray
		{
		public:
			AGLScriptObject	m_pBase64 ;
			bool			m_flagModified ;
		public:
			ReadHistory( void ) : m_flagModified( false ) { }
		} ;
		SSystem::SStrSortObjectArray<ReadHistory>	m_ssoaReadHistory ;

	public:
		// Rosetta 仮想マシン関連付け
		void AttachRosettaVM( Rosetta::RSVirtualMachine * pVM ) ;
		// フラグルート設定
		void SetGameFlagsRoot( const AGLScriptObject& pFlags ) ;
		void SetSharedFlagsRoot( const AGLScriptObject& pFlags ) ;
		void SetSharedReadFlags( const AGLScriptObject& pFlags ) ;
		void SetSharedDeedFlags( const AGLScriptObject& pFlags ) ;
		void RefSharedReadFlags( void ) ;
		void RefSharedDeedFlags( void ) ;

	public:
		// 共有フラグ・シリアライズ
		void SerializeSharedFlags( SSystem::SXMLDocument& xmlTag ) ;
		// 共有フラグ・デシリアライズ
		void DeserializeSharedFlags( const SSystem::SXMLDocument& xmlTag ) ;

	public:
		// 既読フラグ
		void SetReadFlag( const wchar_t * pwszScript, size_t iMessage ) ;
		bool GetReadFlag( const wchar_t * pwszScript, size_t iMessage ) const ;
		// 既読フラグをシリアライズ
		void SerializeReadFlags( void ) ;
		// 既読フラグをデシリアライズ
		void DeerializeReadFlags( void ) ;
		// 功績フラグ
		int64_t AddDeedFlag( const wchar_t * pwszDeed ) ;
		int64_t GetDeedFlag( const wchar_t * pwszDeed ) const ;

	public:	// AGLObject
		// シリアライズ
		virtual SSystem::SError Serialize
				( SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel ) ;
		// デシリアライズ
		virtual SSystem::SError Deserialize
				( const SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel ) ;
		// デシリアライズ後の参照解決処理
		virtual SSystem::SError AfterDeserialize( AGLKernel * pKernel ) ;

	public:	// AGLEpicProcessor
		// 設定
		virtual void LoadConfiguration
			( const SSystem::SXMLDocument& xmlConfig ) ;
		// リリース時処理
		virtual void OnReleaseKernel( void ) ;
		// ゲーム開始時処理
		virtual void InitializeGame( void ) ;

	public:
		// コマンド実装
		DECL_ANTIRRHINUM_PROC(AGLVariablesProcessor,label)
		DECL_ANTIRRHINUM_PROC(AGLVariablesProcessor,add_deed)
		DECL_ANTIRRHINUM_PROC(AGLVariablesProcessor,init_sflag)
		DECL_ANTIRRHINUM_PROC(AGLVariablesProcessor,set_sflag)
		DECL_ANTIRRHINUM_PROC(AGLVariablesProcessor,add_sflag)

	} ;

}

#endif
