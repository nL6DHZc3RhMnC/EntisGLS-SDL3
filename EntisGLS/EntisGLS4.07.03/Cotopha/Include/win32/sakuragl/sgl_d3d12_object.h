
#if	!defined(__SAKURAGL_DIRECT3D12_OBJECT_H__)
#define	__SAKURAGL_DIRECT3D12_OBJECT_H__	1

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#pragma comment( lib, "d3d12.lib" )
#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "d3dcompiler.lib" )


namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// COM ポインタ
	//////////////////////////////////////////////////////////////////////////

	template <class T>	class	SComPtr
	{
	protected:
		T *	m_pCom ;

	public:
		SComPtr( void ) : m_pCom( nullptr ) { }
		SComPtr( const SComPtr<T>& ptr ) : m_pCom( ptr.AddRef() ) { }
		SComPtr( T * pCom ) : m_pCom( pCom ) { }
		~SComPtr( void )
		{
			Release() ;
		}
		void Release( void )
		{
			if ( m_pCom != nullptr )
			{
				m_pCom->Release() ;
				m_pCom = nullptr ;
			}
		}
		T * AddRef( void ) const
		{
			if ( m_pCom != nullptr )
			{
				m_pCom->AddRef() ;
			}
			return	m_pCom ;
		}
		T * Detach( void )
		{
			IUnknown *	pCom = m_pCom ;
			m_pCom = nullptr ;
			return	pCom ;
		}
		T * Ptr( void ) const
		{
			return	m_pCom ;
		}
		T *& Ref( void )
		{
			return	m_pCom ;
		}
		bool operator == ( const T * pCom ) const
		{
			return	(m_pCom == pCom) ;
		}
		bool operator != ( const T * pCom ) const
		{
			return	(m_pCom != pCom) ;
		}
		operator T * ( void ) const
		{
			return	m_pCom ;
		}
		T * operator -> ( void ) const
		{
			return	m_pCom ;
		}
		const SComPtr<T>& operator = ( const SComPtr<T>& ptr )
		{
			Release() ;
			m_pCom = ptr.AddRef() ;
			return	*this ;
		}
		T * operator = ( T * pCom )
		{
			Release() ;
			m_pCom = pCom ;
			return	pCom ;
		}

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ベクトル表現型変換
	//////////////////////////////////////////////////////////////////////////

	S4DMatrix FromXMMATRIX( const DirectX::XMMATRIX& mat4 ) ;
	S3DMatrix FromXMFLOAT3X3( const DirectX::XMFLOAT3X3& mat3 ) ;
	S4DVector FromXMFLOAT4( const DirectX::XMFLOAT4& vec4 ) ;
	S3DVector FromXMFLOAT3( const DirectX::XMFLOAT3& vec3 ) ;
	S2DVector FromXMFLOAT2( const DirectX::XMFLOAT2& vec2 ) ;

	DirectX::XMMATRIX ToXMMATRIX( const S4DMatrix& mat4 ) ;
	DirectX::XMFLOAT3X3 ToXMFLOAT3X3( const S3DMatrix& mat3 ) ;
	DirectX::XMFLOAT4 ToXMFLOAT4( const S4DVector& vec4 ) ;
	DirectX::XMFLOAT3 ToXMFLOAT3( const S3DVector& vec3 ) ;
	DirectX::XMFLOAT2 ToXMFLOAT2( const S2DVector& vec2 ) ;


	//////////////////////////////////////////////////////////////////////////
	// Direct3D12 オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	SGLDirect3D12Device
	{
	public:
		// ディスクリプタ・ヒープ
		class	DescriptorHeap
		{
		protected:
			SComPtr<ID3D12DescriptorHeap>	m_d3dHeap ;
			D3D12_DESCRIPTOR_HEAP_TYPE		m_typeHeap ;
			size_t							m_countDesc ;
			size_t							m_sizeOfType ;

		public:
			// 構築
			DescriptorHeap( void ) { }
			DescriptorHeap( const DescriptorHeap& dh )
				: m_d3dHeap( dh.m_d3dHeap ),
					m_typeHeap( dh.m_typeHeap ),
					m_countDesc( dh.m_countDesc ),
					m_sizeOfType( dh.m_sizeOfType ) { }
			// 解放
			void Release( void )
			{
				m_d3dHeap.Release() ;
			}
			// CPU ディスクリプタ・ハンドル取得
			D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle( size_t i = 0 ) const ;
			// GPU ディスクリプタ・ハンドル取得
			D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle( size_t i = 0 ) const ;
			// 参照
			const SComPtr<ID3D12DescriptorHeap>& Ptr( void ) const
			{
				return	m_d3dHeap ;
			}
			// タイプ
			D3D12_DESCRIPTOR_HEAP_TYPE GetType( void ) const
			{
				return	m_typeHeap ;
			}

			friend class SGLDirect3D12Device ;
		} ;

		// リソース
		class	Resource
		{
		protected:
			SComPtr<ID3D12Resource>	m_d3dRsrc ;
			D3D12_RESOURCE_DESC		m_dscRsrc ;
			D3D12_RESOURCE_STATES	m_rsStates ;
			size_t					m_nBytes ;

		public:
			// 構築
			Resource( void )
				: m_nBytes( 0 ),
					m_rsStates( D3D12_RESOURCE_STATE_COMMON ) { }
			Resource( const Resource& rs )
				: m_d3dRsrc( rs.m_d3dRsrc ),
					m_dscRsrc( rs.m_dscRsrc ),
					m_rsStates( rs.m_rsStates ),
					m_nBytes( rs.m_nBytes ) { }
			// 解放
			void Release( void )
			{
				m_d3dRsrc.Release() ;
				m_nBytes = 0 ;
			}
			// 参照
			const SComPtr<ID3D12Resource>& Ptr( void ) const
			{
				return	m_d3dRsrc ;
			}
			// D3D12_RESOURCE_DESC 取得
			const D3D12_RESOURCE_DESC& GetDesc( void ) const
			{
				return	m_dscRsrc ;
			}
			// 現在のステート
			D3D12_RESOURCE_STATES GetStates( void ) const
			{
				return	m_rsStates ;
			}
			// バイト数取得
			size_t GetSizeInBytes( void ) const
			{
				return	m_nBytes ;
			}
			// バッファへ書き込み (Map 使用)
			SGLError WriteBuffer
				( const void * ptrSrc, size_t nBytes,
					size_t nDstOffset = 0, UINT nDstSubrsrc = 0 ) ;
			SGLError WriteImage
				( const SGLImageInfo& imginf, const void * ptrSrc,
					int xDst, int yDst, int zDst = 0,
					const SGLImageRect * pSrcRect = nullptr,
					UINT nDstSubrsrc = 0, const Resource * pCopyDst = nullptr ) ;
			// バッファから読み込み (Map 使用)
			SGLError ReadImage
				( const SGLImageInfo& imginf, void * ptrDst,
					int xDst, int yDst, int zSrc = 0,
					const SGLImageRect * pSrcRect = nullptr,
					UINT nSrcSubrsrc = 0, const Resource * pCopySrc = nullptr ) ;
			// バッファへ書き込み
			SGLError WriteToSubresource
				( const void * ptrSrc, size_t nRowBytes, size_t nDepthBytes,
					const D3D12_BOX *pDstBox = nullptr, UINT nDstSubrsrc = 0 ) ;
			// バッファから読み込み
			SGLError ReadFromSubresource
				( void * ptrDst, size_t nRowBytes, size_t nDepthBytes,
					const D3D12_BOX *pSrcBox = nullptr, UINT nSrcSubrsrc = 0 ) ;

			friend class SGLDirect3D12Device ;
		} ;

		// シェーダーコード
		class	ShaderByteCode	 : public SSystem::SArray<uint8_t>
		{
		public:
			// 構築
			ShaderByteCode( void ) { }
			ShaderByteCode( const ShaderByteCode& sbc )
				: SSystem::SArray<uint8_t>( sbc ) { }
			// 代入
			const ShaderByteCode& operator = ( const ShaderByteCode& sbc )
			{
				SSystem::SArray<uint8_t>::operator = ( sbc ) ;
				return	*this ;
			}
			// バイナリ読み込み
			SGLError LoadBinary( const wchar_t * pwszPath ) ;
			// コンパイル
			SGLError Compile
				( SSystem::SString& strErr,
					const char * pszSrc,
					LPCSTR pEntrypoint, LPCSTR pTarget,
					UINT Flags1 = 0, UINT Flags2 = 0,
					CONST D3D_SHADER_MACRO* pDefines = nullptr,
					ID3DInclude* pInclude = D3D_COMPILE_STANDARD_FILE_INCLUDE ) ;
			SGLError CompileFromFile
				( SSystem::SString& strErr,
					const wchar_t * pwszPath,
					LPCSTR pEntrypoint, LPCSTR pTarget,
					UINT Flags1 = 0, UINT Flags2 = 0,
					CONST D3D_SHADER_MACRO* pDefines = nullptr,
					ID3DInclude* pInclude = D3D_COMPILE_STANDARD_FILE_INCLUDE ) ;
		} ;

		struct	ShaderSet
		{
			ShaderByteCode *	psbcVertex ;
			ShaderByteCode *	psbcPixel ;
			ShaderByteCode *	psbcDomain ;
			ShaderByteCode *	psbcHull ;
			ShaderByteCode *	psbcGeometry ;

			ShaderSet( void )
				: psbcVertex(nullptr), psbcPixel(nullptr),
					psbcDomain(nullptr), psbcHull(nullptr), psbcGeometry(nullptr) { }
		} ;

		// ルート・シグネチャ
		class	RootSignatureDesc
		{
		protected:
			D3D12_ROOT_SIGNATURE_DESC					m_dscRoot ;
			SSystem::SArray<D3D12_ROOT_PARAMETER>		m_aParams ;
			SSystem::SObjectArray
				< SSystem::SArray
					<D3D12_DESCRIPTOR_RANGE> >			m_aDscRanges ;
			SSystem::SArray<D3D12_STATIC_SAMPLER_DESC>	m_aSamplers ;
			size_t										m_iSRV ;
			size_t										m_iCBV ;

		public:
			// 構築
			RootSignatureDesc( void ) ;
			RootSignatureDesc( const RootSignatureDesc& rs ) ;
			// 代入
			const RootSignatureDesc& operator = ( const RootSignatureDesc& rs ) ;
			// D3D12_ROOT_SIGNATURE_DESC 参照
			const D3D12_ROOT_SIGNATURE_DESC& Desc( void ) const
			{
				return	m_dscRoot ;
			}
			operator const D3D12_ROOT_SIGNATURE_DESC * ( void ) const
			{
				return	&m_dscRoot ;
			}
		protected:
			void UpdateDesc( void ) ;
			void AppendDescRange
				( const D3D12_DESCRIPTOR_RANGE& dr, D3D12_SHADER_VISIBILITY shader ) ;
			void AddDescRange
				( const D3D12_DESCRIPTOR_RANGE& dr, D3D12_SHADER_VISIBILITY shader ) ;
		public:
			// Shader Resource View (Texture 等) 追加（ディスクリプタ連続）
			RootSignatureDesc& AppendSRV
				( size_t nCount,
					D3D12_SHADER_VISIBILITY shader = D3D12_SHADER_VISIBILITY_ALL ) ;
			// Shader Resource View (Texture 等) 追加（ディスクリプタ分割）
			RootSignatureDesc& AddSRV
				( size_t nCount,
					D3D12_SHADER_VISIBILITY shader = D3D12_SHADER_VISIBILITY_ALL ) ;
			// Constant Buffer View 追加（ディスクリプタ連続）
			RootSignatureDesc& AppendCBV
				( size_t nCount,
					D3D12_SHADER_VISIBILITY shader = D3D12_SHADER_VISIBILITY_ALL ) ;
			// Constant Buffer View 追加（ディスクリプタ分割）
			RootSignatureDesc& AddCBV
				( size_t nCount,
					D3D12_SHADER_VISIBILITY shader = D3D12_SHADER_VISIBILITY_ALL ) ;
			// テクスチャ・サンプラー追加
			RootSignatureDesc& AddSamplerTexture2D
				( bool flagWrap,
					D3D12_FILTER filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
					D3D12_SHADER_VISIBILITY shader = D3D12_SHADER_VISIBILITY_ALL ) ;

		} ;

		// パイプライン・ステート
		class	PipelineState
		{
		protected:
			ShaderSet									m_shaders ;
			SSystem::SArray<D3D12_INPUT_ELEMENT_DESC>	m_aLayout ;
			D3D12_GRAPHICS_PIPELINE_STATE_DESC			m_gpsd ;

		public:
			// 構築
			PipelineState( void ) ;
			PipelineState( const PipelineState& state ) ;
			// 代入
			const PipelineState& operator = ( const PipelineState& state ) ;

			// シェーダー設定
			void SetShaderSet( const ShaderSet& shaders ) ;
			// シェーダー取得
			const ShaderSet& GetShaders( void ) const
			{
				return	m_shaders ;
			}

			// ルートシグネチャ設定
			void SetRootSignature( ID3D12RootSignature * pRootSig ) ;

			// シェーダー頂点要素追加
			void AddInputLayoutElement
				( LPCSTR pszSemanticName, DXGI_FORMAT format,
					UINT nAlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT ) ;
			void AddInputLayoutPosition
				( LPCSTR pszSemanticName = "POSITION",
					DXGI_FORMAT format = DXGI_FORMAT_R32G32B32_FLOAT,
					UINT nAlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT ) ;
			void AddInputLayoutNormal
				( LPCSTR pszSemanticName = "NORMAL",
					DXGI_FORMAT format = DXGI_FORMAT_R32G32B32_FLOAT,
					UINT nAlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT ) ;
			void AddInputLayoutUV
				( LPCSTR pszSemanticName = "TEXCOORD",
					DXGI_FORMAT format = DXGI_FORMAT_R32G32_FLOAT,
					UINT nAlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT ) ;

			// デプス機能設定
			void SetDepthFunc( RenderContext::DepthMaskOperation depthMaskOp ) ;

			// D3D12_GRAPHICS_PIPELINE_STATE_DESC 参照
			D3D12_GRAPHICS_PIPELINE_STATE_DESC& Desc( void )
			{
				return	m_gpsd ;
			}
			const D3D12_GRAPHICS_PIPELINE_STATE_DESC& Desc( void ) const
			{
				return	m_gpsd ;
			}
			operator const D3D12_GRAPHICS_PIPELINE_STATE_DESC * ( void ) const
			{
				return	&m_gpsd ;
			}
		} ;

	protected:
		bool								m_flagDebug ;
		SComPtr<IDXGIFactory6>				m_dxgiFactory ;
		SComPtr<IDXGIFactory4>				m_dxgiFactory4 ;

		D3D_FEATURE_LEVEL					m_d3dFeatureLevel ;
		SComPtr<ID3D12Device>				m_d3dDevice ;
		HRESULT								m_hrPresentResult ;

		SComPtr<ID3D12CommandAllocator>		m_d3dCmdAlloc ;
		SComPtr<ID3D12GraphicsCommandList>	m_d3dCmdList ;
		SComPtr<ID3D12CommandQueue>			m_d3dCmdQueue ;

		SComPtr<ID3D12Fence>				m_d3dFence ;
		UINT64								m_nFenceValue ;
		SSystem::SSignalEvent				m_sevFenceEvent ;

		SComPtr<IDXGISwapChain4>			m_dxgiSwapChain ;

		ssize_t								m_iBackBufRTV ;
		size_t								m_nSwapBufCount ;
		DescriptorHeap						m_dhSwapView ;
		SSystem::SObjectArray<Resource>		m_aSwapBufs ;

		bool								m_flagDepthBuf ;
		DescriptorHeap						m_dhDepthBuf ;
		Resource							m_rsDepthBuf ;

		SSystem::SObjectArray
				< SComPtr<IDXGIAdapter> >	m_aAdapters ;

	public:
		// 構築関数
		SGLDirect3D12Device( void ) ;
		// 消滅関数
		~SGLDirect3D12Device( void ) ;
		// デバッグモード有効化
		void EnableDebugMode( void ) ;

	protected:
		// DXGIFactory を準備する
		bool PrepareDXGIFactory( void ) ;
		// DXGIAdapter 配列を列挙する
		bool PrepareEnumAdapters( void ) ;

	public:
		// アダプター名取得
		SGLError EnumerateAdapterNames
			( SSystem::SObjectArray<SSystem::SString>& aAdapterNames ) ;
		// Direct3D12 初期化
		SGLError CreateDevice
			( HWND hWnd, UINT nWidth, UINT nHeight,
				bool fFullscreen, bool flagDepthBuffer = true,
				UINT nSwapBufCount = 2, ssize_t iAdapter = -1 ) ;
		// 解放
		void Release( void ) ;

	public:
		// バックバッファをレンダーターゲットに設定する
		SGLError SetBackBufferRenderTarget( void ) ;
		// 現在のバックバッファとデプスバッファをクリアする
		SGLError CmdClearBackBuffer( SGLPalette rgba, bool flagDepthBuf = true ) ;
		// 現在レンダーターゲットに設定しているバックバッファ番号を取得
		ssize_t GetCurrentBackBufferRTVIndex( void ) ;
		// バックバッファのレンダーターゲットを終了する
		SGLError FinishBackBufferRenderTarget( void ) ;

	public:
		// パイプラインとルートシグネチャを設定
		SGLError SetGraphicsPipeline
			( ID3D12PipelineState * pPipeline, ID3D12RootSignature * pRootSig ) ;
		// ディスクリプタ・ヒープ設定
		SGLError SetDescriptorHeaps
			( size_t nCount, const DescriptorHeap *const* ppHeaps ) ;
		// ディスクリプタ・テーブル設定
		SGLError SetGraphicsRootDescriptorTable
			( size_t iRootParam, const DescriptorHeap& heap, size_t iDesc = 0 ) ;
		// ビューポート・描画領域設定
		SGLError SetViewport
			( float xTopLeft, float yTopLeft,
				float width, float height,
				float zMinDepth = 0.0f, float zMaxDepth = 1.0f ) ;
		// 描画領域設定
		SGLError SetScissorRects( size_t nCount, const D3D12_RECT * pRects ) ;

	public:
		// CloseCommandList / ExecuteCommandList / WaitForCompleted / ResetCommandList 実行
		SGLError SyncExecuteCommandList( void ) ;
		// コマンドリストを閉じる
		SGLError CloseCommandList( void ) ;
		// コマンドリスト実行開始
		SGLError ExecuteCommandList( void ) ;
		// コマンド実行完了待機
		SGLError WaitForCompleted( int64_t msecTimeout = SSystem::Synchronism::Infinite ) ;
		// コマンドリスト／アロケータ・リセット
		SGLError ResetCommandList( void ) ;
		// スワップ
		SGLError Present( UINT SyncInterval, UINT Flags ) ;
		// Present エラーコード
		HRESULT GetPresentResult( void ) const ;

	public:
		// トランジッション・リソースバリア
		SGLError CmdResourceTransitionBarrier
			( Resource& rsrc, D3D12_RESOURCE_STATES states, UINT nSubrsrc = 0 ) ;
		// バーテックスバッファ設定
		SGLError CmdSetVertexBuffer
			( size_t iSlot, const Resource& rsrc, size_t nStride ) ;
		// インデックスバッファ設定
		SGLError CmdSetIndexBuffer
			( const Resource& rsrc, DXGI_FORMAT format = DXGI_FORMAT_R32_UINT ) ;
		// プリミティブタイプ設定
		SGLError CmdSetPrimitiveTopology( D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology ) ;
		// 描画実行
		SGLError CmdDrawInstanced
			( UINT VertexCountPerInstance,
				UINT InstanceCount = 1,
				UINT StartVertexLocation = 0,
				UINT StartInstanceLocation = 0 ) ;
		SGLError CmdDrawIndexedInstanced
			( UINT IndexCountPerInstance,
				UINT InstanceCount = 1,
				UINT StartIndexLocation = 0,
				INT BaseVertexLocation = 0,
				UINT StartInstanceLocation = 0 ) ;

	public:
		// ディスクリプタ・ヒープ作成
		SGLError CreateDescriptorHeap
			( DescriptorHeap& heap, const D3D12_DESCRIPTOR_HEAP_DESC& dscHeap ) ;
		SGLError CreateDescriptorHeap
			( DescriptorHeap& heap,
				D3D12_DESCRIPTOR_HEAP_TYPE type,
				UINT nDescriptors,
				D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE ) ;
		SGLError CreateDescriptorHeapRTV
			( DescriptorHeap& heap, UINT nDescriptors ) ;
		SGLError CreateDescriptorHeapDSV
			( DescriptorHeap& heap, UINT nDescriptors ) ;
		SGLError CreateDescriptorHeapCBV_SRV_UAV
			( DescriptorHeap& heap, UINT nDescriptors ) ;
		// CBV ディスクリプタ作成
		SGLError CreateConstantBufferView
			( DescriptorHeap& heap, size_t index,
				const Resource& rsrc, size_t nOffset = 0, ssize_t nBytes = -1 ) ;
		// SRV ディスクリプタ作成
		SGLError CreateSRVTexture2D
			( DescriptorHeap& heap, size_t index, const Resource& rsrc ) ;

	public:
		// ルート・シグネチャ作成
		SGLError CreateRootSignature
			( SComPtr<ID3D12RootSignature>& rootSig,
				const D3D12_ROOT_SIGNATURE_DESC& desc,
				D3D_ROOT_SIGNATURE_VERSION version = D3D_ROOT_SIGNATURE_VERSION_1_0 ) ;
		// パイプライン・ステート作成
		SGLError CreateGraphicsPipelineState
			( SComPtr<ID3D12PipelineState>& pipeline,
					const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc ) ;

	public:
		// リソース作成
		SGLError CreateCommittedResource
			( Resource& rsrc, size_t nBytes, const D3D12_HEAP_PROPERTIES& heapProp,
				D3D12_HEAP_FLAGS flags, const D3D12_RESOURCE_DESC& desc,
				D3D12_RESOURCE_STATES stateInit,
				const D3D12_CLEAR_VALUE * pOptClearValue = nullptr ) ;
		SGLError CreateVertexBuffer( Resource& rsrc, size_t nBytes ) ;
		SGLError CreateConstantBuffer( Resource& rsrc, size_t nBytes ) ;
		SGLError CreateTexture2D
			( Resource& rsrc,
				size_t nWidth, size_t nHeight,
				DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM,
				size_t bitsPerPixel = 32, size_t nArraySize = 1,
				size_t nMipLevels = 1,
				bool flagMemoryL0 = false, bool flagDepth = false ) ;
		SGLError CreateTexture2DUploadBuffer
			( Resource& rsrc,
				size_t nWidth, size_t nHeight,
				DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM,
				size_t bitsPerPixel = 32, size_t nArraySize = 1 ) ;
		// テクスチャ・コピー
		SGLError CmdCopyTextureRegion
			( Resource& rsDst, UINT xDst, UINT yDst, UINT zDst,
				Resource& rsSrc, const D3D12_BOX * pSrcBox = nullptr ) ;

	public:
		// テクスチャ・ピッチ・アライメント
		static size_t AlignTexturePitch( size_t nBytes ) ;
		// DXGI 画像フォーマット
		struct	FormatInfo
		{
			uint32_t	format ;
			uint32_t	bitsPerBlock ;
			uint32_t	blockPixels ;
		} ;
		static FormatInfo FromDXGIFormat
			( DXGI_FORMAT format, uint32_t formatErr = 0, uint32_t bitsErr = 32 ) ;
		static DXGI_FORMAT ToDXGIFormat( uint32_t format, uint32_t bitsPerBlock ) ;

		struct	DXGIFormatPair
		{
			DXGI_FORMAT	dxgif ;
			FormatInfo	fmtinf ;
		} ;
		static const DXGIFormatPair	s_dxgiFormatPairs[15] ;

	public:
		// Feature Level 取得
		D3D_FEATURE_LEVEL GetFeatureLevel( void ) const
		{
			return	m_d3dFeatureLevel ;
		}
		// D3D12Device 取得
		const SComPtr<ID3D12Device>& Device( void ) const
		{
			return	m_d3dDevice ;
		}
		// D3D12CommandAllocator 取得
		const SComPtr<ID3D12CommandAllocator>& CommandAllocator( void ) const
		{
			return	m_d3dCmdAlloc ;
		}
		// D3D12GraphicsCommandList 取得
		const SComPtr<ID3D12GraphicsCommandList>& CommandList( void ) const
		{
			return	m_d3dCmdList ;
		}
		// D3D12CommandQueue 取得
		const SComPtr<ID3D12CommandQueue>& CommandQueue( void ) const
		{
			return	m_d3dCmdQueue ;
		}
		// DXGISwapChain4 取得
		const SComPtr<IDXGISwapChain4>& SwapChain( void ) const
		{
			return	m_dxgiSwapChain ;
		}
		// スワップバッファ数
		size_t GetSwapBufferCount( void ) const
		{
			return	m_nSwapBufCount ;
		}
		// スワップバッファ取得
		Resource * GetSwapBufferAt( size_t i ) const
		{
			return	m_aSwapBufs.GetAt( i ) ;
		}
		// スワップバッファ・レンダーターゲット
		const DescriptorHeap& GetSwapBufferRTV( void ) const
		{
			return	m_dhSwapView ;
		}
		// デプス・ステンシル・バッファ取得
		Resource * GetDepthBuffer( void )
		{
			ESLAssert( m_flagDepthBuf ) ;
			return	m_flagDepthBuf ? &m_rsDepthBuf : nullptr ;
		}
		// デプス・ステンシル・ビュー
		const DescriptorHeap& GetDepthView( void ) const
		{
			ESLAssert( m_flagDepthBuf ) ;
			return	m_dhDepthBuf ;
		}

	} ;

}


#endif
