
#if	!defined(__SAKURAGL_DIRECTX_OBJECT_H__)
#define	__SAKURAGL_DIRECTX_OBJECT_H__	1

struct IDirect3D9 ;
struct IDirect3DDevice9 ;
struct _D3DPRESENT_PARAMETERS_ ;

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// Direct3D9 オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	SGLDirect3D9Device	: public SSystem::SDependentNotificationServer
	{
	public:
		// 通知インターフェース
		class	INotify	: public SSystem::SDependentNotification
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( INotify, SDependentNotification )
			// 構築関数
			INotify( void ) ;
			// 消滅関数
			virtual ~INotify( void ) ;
		public:	// SDependentNotification オーバーライド
			// 依存先オブジェクト生成／再生成後に呼び出される
			virtual void OnCreateObject
				( SDependentNotificationServer * pObject, bool fInitialize ) ;
			// 依存先のリソースが解放される／リセットされる前等に呼び出される
			virtual void OnReleaseObject( SDependentNotificationServer * pObject ) ;
			// 依存先オブジェクトが削除される前に呼び出される
			virtual void OnFinalizeObject( SDependentNotificationServer * pObject ) ;
		public:
			// Direct3D オブジェクトが作成された後に呼び出される
			virtual void OnCreateDevice
					( SGLDirect3D9Device * pd3dDev, bool fInitialize ) ;
			// Direct3D オブジェクトがリセットされる前に呼び出される
			virtual void OnReleaseDevice( SGLDirect3D9Device * pd3dDev ) ;
			// Direct3D オブジェクトが削除される前に呼び出される
			virtual void OnFinalizeDevice( SGLDirect3D9Device * pd3dDev ) ;
		} ;

	protected:
		static HMODULE				m_hModuleD3D9 ;
		IDirect3D9 *				m_id3d9 ;
		IDirect3DDevice9 *			m_id3d9Dev ;
		_D3DPRESENT_PARAMETERS_ *	m_pd3dpp ;

		bool		m_fFullscreen ;
		INotify *	m_pFirstNotify ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLDirect3D9Device, SObject )
		// 構築関数
		SGLDirect3D9Device( void ) ;
		// 消滅関数
		virtual ~SGLDirect3D9Device( void ) ;

	public:
		// Direct3D9 初期化
		SGLError CreateDevice
			( HWND hWnd,
				unsigned int nWidth,
				unsigned int nHeight, bool fFullscreen ) ;
		SGLError CreateDevice
			( HWND hWnd, UINT nAdapter,
				const struct _D3DPRESENT_PARAMETERS_ * pd3dpp ) ;
		// Direct3D9 リセット
		SGLError ResetDevice
			( const struct _D3DPRESENT_PARAMETERS_ * pd3dpp = NULL ) ;
		// 解放
		void Release( void ) ;

	public:
		// 通知オブジェクトを追加する
		void AddNotify( INotify * pNotify ) ;
		// 通知を解除する
		void DetachNotify( INotify * pNotify ) ;

	protected:
		// OnCreateDevice を通知する
		void NotifyAllOnCreateDevice( bool fInitialize ) ;
		// OnReleaseDevice を通知する
		void NotifyAllOnReleaseDevice( void ) ;
		// OnFinalizeDevice を通知する
		void NotifyAllOnFinalizeDevice( void ) ;

	public:
		// デバイス取得
		struct IDirect3DDevice9 * GetDirect3DDevice9( void ) const
			{
				return	m_id3d9Dev ;
			}
		// DirectX 9 がインストールされているか？
		static bool IsInstalledDirectX9( void ) ;
		// フルスクリーンモードか？
		bool IsFullscreenMode( void ) const
			{
				return	m_fFullscreen ;
			}
	} ;

}


#endif

