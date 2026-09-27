
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_d3d12_object.h>
#include <sakuragl/sgl2d/sgl_image_conversion.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// ベクトル表現型変換
//////////////////////////////////////////////////////////////////////////////

S4DMatrix SakuraGL::FromXMMATRIX( const DirectX::XMMATRIX& mat4 )
{
	S4DMatrix	mat4Dst ;
	for ( int i = 0; i < 4; i ++ )
	{
		mat4Dst.m[i][0] = mat4.r[0].m128_f32[i] ;
		mat4Dst.m[i][1] = mat4.r[1].m128_f32[i] ;
		mat4Dst.m[i][2] = mat4.r[2].m128_f32[i] ;
		mat4Dst.m[i][3] = mat4.r[3].m128_f32[i] ;
	}
	return	mat4Dst ;
}

S3DMatrix SakuraGL::FromXMFLOAT3X3( const DirectX::XMFLOAT3X3& mat3 )
{
	S3DMatrix	mat3Dst ;
	for ( int i = 0; i < 3; i ++ )
	{
		mat3Dst.m[i][0] = mat3.m[0][i] ;
		mat3Dst.m[i][1] = mat3.m[1][i] ;
		mat3Dst.m[i][2] = mat3.m[2][i] ;
	}
	return	mat3Dst ;
}

S4DVector SakuraGL::FromXMFLOAT4( const DirectX::XMFLOAT4& vec4 )
{
	return	S4DVector( vec4.x, vec4.y, vec4.z, vec4.w ) ;
}

S3DVector SakuraGL::FromXMFLOAT3( const DirectX::XMFLOAT3& vec3 )
{
	return	S3DVector( vec3.x, vec3.y, vec3.z ) ;
}

S2DVector SakuraGL::FromXMFLOAT2( const DirectX::XMFLOAT2& vec2 )
{
	return	S2DVector( vec2.x, vec2.y ) ;
}

DirectX::XMMATRIX SakuraGL::ToXMMATRIX( const S4DMatrix& mat4 )
{
	DirectX::XMMATRIX	mat4Dst ;
	for ( int i = 0; i < 4; i ++ )
	{
		mat4Dst.r[0].m128_f32[i] = mat4.m[i][0] ;
		mat4Dst.r[1].m128_f32[i] = mat4.m[i][1] ;
		mat4Dst.r[2].m128_f32[i] = mat4.m[i][2] ;
		mat4Dst.r[3].m128_f32[i] = mat4.m[i][3] ;
	}
	return	mat4Dst ;
}

DirectX::XMFLOAT3X3 SakuraGL::ToXMFLOAT3X3( const S3DMatrix& mat3 )
{
	DirectX::XMFLOAT3X3	mat3Dst ;
	for ( int i = 0; i < 3; i ++ )
	{
		mat3Dst.m[i][0] = mat3.m[0][i] ;
		mat3Dst.m[i][1] = mat3.m[1][i] ;
		mat3Dst.m[i][2] = mat3.m[2][i] ;
	}
	return	mat3Dst ;
}

DirectX::XMFLOAT4 SakuraGL::ToXMFLOAT4( const S4DVector& vec4 )
{
	return	DirectX::XMFLOAT4( vec4.x, vec4.y, vec4.z, vec4.w ) ;
}

DirectX::XMFLOAT3 SakuraGL::ToXMFLOAT3( const S3DVector& vec3 )
{
	return	DirectX::XMFLOAT3( vec3.x, vec3.y, vec3.z ) ;
}

DirectX::XMFLOAT2 SakuraGL::ToXMFLOAT2( const S2DVector& vec2 )
{
	return	DirectX::XMFLOAT2( vec2.x, vec2.y ) ;
}



//////////////////////////////////////////////////////////////////////////////
// Direct3D12 リソース
//////////////////////////////////////////////////////////////////////////////

// CPU ディスクリプタ・ハンドル取得
//////////////////////////////////////////////////////////////////////////////
D3D12_CPU_DESCRIPTOR_HANDLE
	SGLDirect3D12Device::DescriptorHeap::GetCPUDescriptorHandle( size_t i ) const
{
	ESLAssert( i < m_countDesc ) ;
	ESLAssert( m_d3dHeap != nullptr ) ;
	D3D12_CPU_DESCRIPTOR_HANDLE
		hDesc = m_d3dHeap->GetCPUDescriptorHandleForHeapStart() ;
	hDesc.ptr += m_sizeOfType* i ;
	return	hDesc ;
}

// GPU ディスクリプタ・ハンドル取得
//////////////////////////////////////////////////////////////////////////////
D3D12_GPU_DESCRIPTOR_HANDLE
	SGLDirect3D12Device::DescriptorHeap::GetGPUDescriptorHandle( size_t i ) const
{
	ESLAssert( i < m_countDesc ) ;
	ESLAssert( m_d3dHeap != nullptr ) ;
	D3D12_GPU_DESCRIPTOR_HANDLE
		hDesc = m_d3dHeap->GetGPUDescriptorHandleForHeapStart() ;
	hDesc.ptr += m_sizeOfType* i ;
	return	hDesc ;
}


//////////////////////////////////////////////////////////////////////////////
// Direct3D12 ディスクリプタ・ヒープ
//////////////////////////////////////////////////////////////////////////////

// バッファへ書き込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::Resource::WriteBuffer
	( const void * ptrSrc, size_t nBytes, size_t nDstOffset, UINT nDstSubrsrc )
{
	ESLAssert( m_d3dRsrc != nullptr ) ;
	if ( m_d3dRsrc == nullptr )
	{
		return	sglErrFailed ;
	}
	D3D12_RANGE	range ;
	range.Begin = nDstOffset ;
	range.End = nDstOffset + nBytes ;
	//
	void *	ptrData = nullptr ;
	HRESULT	hr = m_d3dRsrc->Map( nDstSubrsrc, &range, &ptrData ) ;
	if ( hr != S_OK )
	{
		ESLTrace( "Failed to ID3D12Resource::Map.(#%08X)\n", hr ) ;
		return	sglErrFailed ;
	}
	//
	eslCopyMemory( ptrData, ptrSrc, nBytes ) ;
	//
	m_d3dRsrc->Unmap( nDstSubrsrc, &range ) ;
	//
	return	sglErrSuccess ;
}

SGLError SGLDirect3D12Device::Resource::WriteImage
	( const SGLImageInfo& imginf, const void * ptrSrc,
		int xDst, int yDst, int zDst,
		const SGLImageRect * pSrcRect,
		UINT nDstSubrsrc, const SGLDirect3D12Device::Resource * pCopyDst )
{
	ESLAssert( m_d3dRsrc != nullptr ) ;
	if ( m_d3dRsrc == nullptr )
	{
		return	sglErrFailed ;
	}
	if ( pCopyDst == nullptr )
	{
		pCopyDst = this ;
	}
	FormatInfo	fmtinf = FromDXGIFormat( pCopyDst->m_dscRsrc.Format ) ;
	if ( fmtinf.format == 0 )
	{
		return	sglErrFailed ;
	}
	size_t	nArrayPitch = m_nBytes / pCopyDst->m_dscRsrc.DepthOrArraySize ;
	size_t	nRowPitch = nArrayPitch / pCopyDst->m_dscRsrc.Height ;
	//
	D3D12_RANGE	range ;
	range.Begin = (size_t) zDst * nArrayPitch ;
	range.End = range.Begin + nArrayPitch ;
	//
	void *	ptrData = nullptr ;
	HRESULT	hr = m_d3dRsrc->Map( nDstSubrsrc, &range, &ptrData ) ;
	if ( hr != S_OK )
	{
		ESLTrace( "Failed to ID3D12Resource::Map.(#%08X)\n", hr ) ;
		return	sglErrFailed ;
	}
	//
	SGLImageBuffer	imgDst ;
	imgDst.format = fmtinf.format ;
	imgDst.depth = fmtinf.bitsPerBlock ;
	imgDst.width = (uint32_t) pCopyDst->m_dscRsrc.Width ;
	imgDst.height = (uint32_t) pCopyDst->m_dscRsrc.Height ;
	imgDst.pitchPixel = (int32_t) imgDst.depth / 8 ;
	imgDst.pitchLine = (int32_t) nRowPitch ;
	imgDst.ptrBuffer = (uint8_t*) ptrData ;
	//
	SGLImageBuffer	imgSrc = imginf ;
	imgSrc.ptrBuffer = (uint8_t*) ptrSrc ;
	//
	SGLError	err =
		sglConvertImageBuffer( imgDst, imgSrc, xDst, yDst, pSrcRect ) ;
	//
	m_d3dRsrc->Unmap( nDstSubrsrc, &range ) ;
	//
	return	err ;
}

// バッファから読み込み (Map 使用)
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::Resource::ReadImage
	( const SGLImageInfo& imginf, void * ptrDst,
		int xDst, int yDst, int zSrc,
		const SGLImageRect * pSrcRect,
		UINT nSrcSubrsrc, const SGLDirect3D12Device::Resource * pCopySrc )
{
	ESLAssert( m_d3dRsrc != nullptr ) ;
	if ( m_d3dRsrc == nullptr )
	{
		return	sglErrFailed ;
	}
	if ( pCopySrc == nullptr )
	{
		pCopySrc = this ;
	}
	FormatInfo	fmtinf = FromDXGIFormat( pCopySrc->m_dscRsrc.Format ) ;
	if ( fmtinf.format == 0 )
	{
		return	sglErrFailed ;
	}
	size_t	nArrayPitch = m_nBytes / pCopySrc->m_dscRsrc.DepthOrArraySize ;
	size_t	nRowPitch = nArrayPitch / pCopySrc->m_dscRsrc.Height ;
	//
	D3D12_RANGE	range ;
	range.Begin = (size_t) zSrc * nArrayPitch ;
	range.End = range.Begin + nArrayPitch ;
	//
	void *	ptrData = nullptr ;
	HRESULT	hr = m_d3dRsrc->Map( nSrcSubrsrc, &range, &ptrData ) ;
	if ( hr != S_OK )
	{
		ESLTrace( "Failed to ID3D12Resource::Map.(#%08X)\n", hr ) ;
		return	sglErrFailed ;
	}
	//
	SGLImageBuffer	imgSrc ;
	imgSrc.format = fmtinf.format ;
	imgSrc.depth = fmtinf.bitsPerBlock ;
	imgSrc.width = (uint32_t) pCopySrc->m_dscRsrc.Width ;
	imgSrc.height = (uint32_t) pCopySrc->m_dscRsrc.Height ;
	imgSrc.pitchPixel = (int32_t) imgSrc.depth / 8 ;
	imgSrc.pitchLine = (int32_t) nRowPitch ;
	imgSrc.ptrBuffer = (uint8_t*) ptrData ;
	//
	SGLImageBuffer	imgDst = imginf ;
	imgDst.ptrBuffer = (uint8_t*) ptrDst ;
	//
	SGLError	err =
		sglConvertImageBuffer( imgDst, imgSrc, xDst, yDst, pSrcRect ) ;
	//
	m_d3dRsrc->Unmap( nSrcSubrsrc, &range ) ;
	//
	return	err ;
}

// バッファへ書き込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::Resource::WriteToSubresource
	( const void * ptrSrc,
		size_t nRowBytes, size_t nDepthBytes,
		const D3D12_BOX *pDstBox, UINT nDstSubrsrc )
{
	ESLAssert( m_d3dRsrc != nullptr ) ;
	if ( m_d3dRsrc == nullptr )
	{
		return	sglErrFailed ;
	}
	HRESULT	hr =
		m_d3dRsrc->WriteToSubresource
			( nDstSubrsrc, pDstBox,
				ptrSrc, (UINT) nRowBytes, (UINT) nDepthBytes ) ;
	if ( hr != S_OK )
	{
		ESLTrace( "Failed to ID3D12Resource::WriteToSubresource.(#%08X)\n", hr ) ;
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// バッファから読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::Resource::ReadFromSubresource
	( void * ptrDst,
		size_t nRowBytes, size_t nDepthBytes,
		const D3D12_BOX *pSrcBox, UINT nSrcSubrsrc )
{
	ESLAssert( m_d3dRsrc != nullptr ) ;
	if ( m_d3dRsrc == nullptr )
	{
		return	sglErrFailed ;
	}
	HRESULT	hr =
		m_d3dRsrc->ReadFromSubresource
			( ptrDst, (UINT) nRowBytes,
				(UINT) nDepthBytes, nSrcSubrsrc, pSrcBox ) ;
	if ( hr != S_OK )
	{
		ESLTrace( "Failed to ID3D12Resource::ReadFromSubresource.(#%08X)\n", hr ) ;
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// Direct3D12 シェーダーコード
//////////////////////////////////////////////////////////////////////////////

// バイナリ読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::ShaderByteCode::LoadBinary( const wchar_t * pwszPath )
{
	SSmartPointer<SFileInterface>	pFile =
		SFileOpener::DefaultNewOpenFile( pwszPath, SFileOpener::shareRead ) ;
	if ( pFile == nullptr )
	{
		return	sglErrFailed ;
	}
	size_t	nBytes = (size_t) pFile->GetLength() ;
	pFile->Read( GetArray( nBytes ), nBytes ) ;
	FinishArray() ;
	return	sglErrSuccess ;
}

// コンパイル
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::ShaderByteCode::Compile
	( SSystem::SString& strErr,
		const char * pszSrc,
		LPCSTR pEntrypoint, LPCSTR pTarget,
		UINT Flags1, UINT Flags2,
		CONST D3D_SHADER_MACRO* pDefines, ID3DInclude* pInclude )
{
	ESLAssert( pszSrc != nullptr ) ;
	size_t	nSrcLen = 0 ;
	while ( pszSrc[nSrcLen] != 0 )
	{
		nSrcLen ++ ;
	}
	SComPtr<ID3DBlob>	blobCode ;
	SComPtr<ID3DBlob>	blobErrMsgs ;
	HRESULT	hr = D3DCompile( pszSrc, nSrcLen * sizeof(char), nullptr,
							pDefines, pInclude,
							pEntrypoint, pTarget, Flags1, Flags2,
							&blobCode.Ref(), &blobErrMsgs.Ref() ) ;
	strErr.FreeArray() ;
	if ( FAILED(hr) )
	{
		if ( blobErrMsgs != nullptr )
		{
			Charset::Decode
				( strErr, Charset::encodingUTF8,
					(const uint8_t*) blobErrMsgs->GetBufferPointer(),
					(ssize_t) blobErrMsgs->GetBufferSize() ) ;
		}
		return	sglErrFailed ;
	}
	if ( blobCode == nullptr )
	{
		return	sglErrFailed ;
	}
	size_t	nCodeBytes = blobCode->GetBufferSize() ;
	SetLength( nCodeBytes ) ;
	eslCopyMemory( GetArray(), blobCode->GetBufferPointer(), nCodeBytes ) ;
	FinishArray() ;
	return	sglErrSuccess ;
}

SGLError SGLDirect3D12Device::ShaderByteCode::CompileFromFile
	( SSystem::SString& strErr,
		const wchar_t * pwszPath,
		LPCSTR pEntrypoint, LPCSTR pTarget,
		UINT Flags1, UINT Flags2,
		CONST D3D_SHADER_MACRO* pDefines, ID3DInclude* pInclude )
{
	SStringParser	sparsSrc ;
	if ( sparsSrc.LoadTextFile( pwszPath ) )
	{
		return	sglErrFailed ;
	}
	return	Compile( strErr, sparsSrc.ToCharArray().GetConstArray(),
					pEntrypoint, pTarget, Flags1, Flags2, pDefines, pInclude ) ;
}


//////////////////////////////////////////////////////////////////////////////
// ルート・シグネチャ
//////////////////////////////////////////////////////////////////////////////

// 構築
//////////////////////////////////////////////////////////////////////////////
SGLDirect3D12Device::RootSignatureDesc::RootSignatureDesc( void )
{
	eslFillMemory( &m_dscRoot, 0, sizeof(m_dscRoot) ) ;
	m_dscRoot.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT ;
	m_iSRV = 0 ;
	m_iCBV = 0 ;
}

SGLDirect3D12Device::RootSignatureDesc::RootSignatureDesc
			( const SGLDirect3D12Device::RootSignatureDesc& rs )
	: m_dscRoot( rs.m_dscRoot ),
		m_aParams( rs.m_aParams ),
		m_aDscRanges( rs.m_aDscRanges ),
		m_aSamplers( rs.m_aSamplers )
{
	UpdateDesc() ;
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const SGLDirect3D12Device::RootSignatureDesc&
	SGLDirect3D12Device::RootSignatureDesc::operator = ( const RootSignatureDesc& rs )
{
	m_dscRoot = rs.m_dscRoot ;
	m_aParams = rs.m_aParams ;
	m_aDscRanges = rs.m_aDscRanges ;
	m_aSamplers = rs.m_aSamplers ;
	m_iSRV = rs.m_iSRV ;
	m_iCBV = rs.m_iCBV ;

	UpdateDesc() ;

	return	*this ;
}

void SGLDirect3D12Device::RootSignatureDesc::UpdateDesc( void )
{
	m_dscRoot.NumParameters = (UINT) m_aParams.GetLength() ;
	m_dscRoot.pParameters = m_aParams.GetConstArray() ;
	m_dscRoot.NumStaticSamplers = (UINT) m_aSamplers.GetLength() ;
	m_dscRoot.pStaticSamplers = m_aSamplers.GetConstArray() ;
}

void SGLDirect3D12Device::RootSignatureDesc::AppendDescRange
	( const D3D12_DESCRIPTOR_RANGE& dr, D3D12_SHADER_VISIBILITY shader )
{
	D3D12_ROOT_PARAMETER *				prp = m_aParams.GetLastAt( 0 ) ;
	SArray<D3D12_DESCRIPTOR_RANGE> *	pRanges = m_aDscRanges.GetLastAt( 0 ) ;
	if ( prp != NULL )
	{
		ESLAssert( pRanges != NULL ) ;
		ESLAssert( prp->ShaderVisibility == shader ) ;
		ESLAssert( prp->DescriptorTable.pDescriptorRanges == pRanges->GetConstArray() ) ;
		ESLAssert( prp->DescriptorTable.NumDescriptorRanges == pRanges->GetLength() ) ;
	}
	else
	{
		D3D12_ROOT_PARAMETER	rp ;
		eslFillMemory( &rp, 0, sizeof(rp) ) ;
		rp.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE ;
		rp.ShaderVisibility = shader ;
		m_aParams.Add( rp ) ;
		prp = m_aParams.GetLastAt( 0 ) ;
		ESLAssert( eslCompareMemory( prp, &rp, sizeof(rp) ) == 0 ) ;
		//
		pRanges = new SArray<D3D12_DESCRIPTOR_RANGE> ;
		m_aDscRanges.Add( pRanges ) ;
	}
	pRanges->Add( dr ) ;

	prp->DescriptorTable.pDescriptorRanges = pRanges->GetConstArray() ;
	prp->DescriptorTable.NumDescriptorRanges = (UINT) pRanges->GetLength() ;

	UpdateDesc() ;
}

void SGLDirect3D12Device::RootSignatureDesc::AddDescRange
	( const D3D12_DESCRIPTOR_RANGE& dr, D3D12_SHADER_VISIBILITY shader )
{
	SArray<D3D12_DESCRIPTOR_RANGE> *	pRanges = new SArray<D3D12_DESCRIPTOR_RANGE> ;
	pRanges->Add( dr ) ;
	m_aDscRanges.Add( pRanges ) ;
	//
	D3D12_ROOT_PARAMETER	rp ;
	eslFillMemory( &rp, 0, sizeof(rp) ) ;
	rp.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE ;
	rp.ShaderVisibility = shader ;
	rp.DescriptorTable.pDescriptorRanges = pRanges->GetConstArray() ;
	rp.DescriptorTable.NumDescriptorRanges = (UINT) pRanges->GetLength() ;
	m_aParams.Add( rp ) ;
	//
	UpdateDesc() ;
}

// Shader Resource View (Texture 等) 追加（ディスクリプタ連続）
//////////////////////////////////////////////////////////////////////////////
SGLDirect3D12Device::RootSignatureDesc&
	SGLDirect3D12Device::RootSignatureDesc::AppendSRV
		( size_t nCount, D3D12_SHADER_VISIBILITY shader )
{
	D3D12_DESCRIPTOR_RANGE	dr ;
	eslFillMemory( &dr, 0, sizeof(dr) ) ;
	dr.NumDescriptors = (UINT) nCount ;
	dr.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV ;
	dr.BaseShaderRegister = (UINT) m_iSRV ;
	dr.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND ;
	m_iSRV += nCount ;
	//
	AppendDescRange( dr, shader ) ;
	//
	return	*this ;
}

// Shader Resource View (Texture 等) 追加（ディスクリプタ分割）
//////////////////////////////////////////////////////////////////////////////
SGLDirect3D12Device::RootSignatureDesc&
	SGLDirect3D12Device::RootSignatureDesc::AddSRV
		( size_t nCount, D3D12_SHADER_VISIBILITY shader )
{
	D3D12_DESCRIPTOR_RANGE	dr ;
	eslFillMemory( &dr, 0, sizeof(dr) ) ;
	dr.NumDescriptors = (UINT) nCount ;
	dr.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV ;
	dr.BaseShaderRegister = (UINT) m_iSRV ;
	dr.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND ;
	m_iSRV += nCount ;
	//
	AddDescRange( dr, shader ) ;
	//
	return	*this ;
}

// Constant Buffer View 追加（ディスクリプタ連続）
//////////////////////////////////////////////////////////////////////////////
SGLDirect3D12Device::RootSignatureDesc&
	SGLDirect3D12Device::RootSignatureDesc::AppendCBV
		( size_t nCount, D3D12_SHADER_VISIBILITY shader )
{
	D3D12_DESCRIPTOR_RANGE	dr ;
	eslFillMemory( &dr, 0, sizeof(dr) ) ;
	dr.NumDescriptors = (UINT) nCount ;
	dr.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV ;
	dr.BaseShaderRegister = (UINT) m_iCBV ;
	dr.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND ;
	m_iCBV += nCount ;
	//
	AppendDescRange( dr, shader ) ;
	//
	return	*this ;
}

// Constant Buffer View 追加（ディスクリプタ分割）
//////////////////////////////////////////////////////////////////////////////
SGLDirect3D12Device::RootSignatureDesc&
	SGLDirect3D12Device::RootSignatureDesc::AddCBV
		( size_t nCount, D3D12_SHADER_VISIBILITY shader )
{
	D3D12_DESCRIPTOR_RANGE	dr ;
	eslFillMemory( &dr, 0, sizeof(dr) ) ;
	dr.NumDescriptors = (UINT) nCount ;
	dr.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV ;
	dr.BaseShaderRegister = (UINT) m_iCBV ;
	dr.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND ;
	m_iCBV += nCount ;
	//
	AddDescRange( dr, shader ) ;
	//
	return	*this ;
}

// テクスチャ・サンプラー追加
//////////////////////////////////////////////////////////////////////////////
SGLDirect3D12Device::RootSignatureDesc&
	SGLDirect3D12Device::RootSignatureDesc::AddSamplerTexture2D
		( bool flagWrap, D3D12_FILTER filter, D3D12_SHADER_VISIBILITY shader )
{
	D3D12_STATIC_SAMPLER_DESC	ssd ;
	eslFillMemory( &ssd, 0, sizeof(ssd) ) ;
	const D3D12_TEXTURE_ADDRESS_MODE
		addrMode = flagWrap ? D3D12_TEXTURE_ADDRESS_MODE_WRAP
							: D3D12_TEXTURE_ADDRESS_MODE_CLAMP ;
	ssd.AddressU = addrMode ;
	ssd.AddressV = addrMode ;
	ssd.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP ;
	ssd.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK ;
	ssd.Filter = filter ;
	ssd.MaxLOD = D3D12_FLOAT32_MAX ;
	ssd.MinLOD = 0.0f ;
	ssd.ShaderVisibility = shader ;
	ssd.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER ;
	//
	m_aSamplers.Add( ssd ) ;
	UpdateDesc() ;
	//
	return	*this ;
}



//////////////////////////////////////////////////////////////////////////////
// パイプライン・ステート
//////////////////////////////////////////////////////////////////////////////

// 構築
//////////////////////////////////////////////////////////////////////////////
SGLDirect3D12Device::PipelineState::PipelineState( void )
{
	eslFillMemory( &m_gpsd, 0, sizeof(m_gpsd) ) ;

	m_gpsd.BlendState.AlphaToCoverageEnable = false ;
	m_gpsd.BlendState.IndependentBlendEnable = false ;
	m_gpsd.BlendState.RenderTarget[0].BlendEnable = true ;
	m_gpsd.BlendState.RenderTarget[0].LogicOpEnable = false ;
	m_gpsd.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD ;
	m_gpsd.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE ;
	m_gpsd.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA ;
	m_gpsd.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD ;
	m_gpsd.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE ;
	m_gpsd.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA ;
	m_gpsd.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL ;

	m_gpsd.SampleMask = D3D12_DEFAULT_SAMPLE_MASK ;

	m_gpsd.RasterizerState.MultisampleEnable = false ;
	m_gpsd.RasterizerState.CullMode = D3D12_CULL_MODE_NONE ;
	m_gpsd.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID ;
	m_gpsd.RasterizerState.DepthClipEnable = true ;

	m_gpsd.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED ;

	m_gpsd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE ;

	m_gpsd.NumRenderTargets = 1 ;
	m_gpsd.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM ;

	m_gpsd.SampleDesc.Count = 1 ;
	m_gpsd.SampleDesc.Quality = 0 ;
}

SGLDirect3D12Device::PipelineState::PipelineState
			( const SGLDirect3D12Device::PipelineState& state )
	: m_shaders( state.m_shaders ), m_gpsd( state.m_gpsd )
{
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const SGLDirect3D12Device::PipelineState&
	SGLDirect3D12Device::PipelineState::operator =
				( const SGLDirect3D12Device::PipelineState& state )
{
	m_shaders = state.m_shaders ;
	m_gpsd = state.m_gpsd ;
	return	*this ;
}

// シェーダー設定
//////////////////////////////////////////////////////////////////////////////
void SGLDirect3D12Device::PipelineState::SetShaderSet
				( const SGLDirect3D12Device::ShaderSet& shaders )
{
	m_shaders = shaders ;
	//
	if ( shaders.psbcVertex != nullptr )
	{
		m_gpsd.VS.pShaderBytecode = shaders.psbcVertex->GetConstArray() ;
		m_gpsd.VS.BytecodeLength = shaders.psbcVertex->GetLength() ;
	}
	else
	{
		m_gpsd.VS.pShaderBytecode = nullptr ;
		m_gpsd.VS.BytecodeLength = 0 ;
	}
	if ( shaders.psbcPixel != nullptr )
	{
		m_gpsd.PS.pShaderBytecode = shaders.psbcPixel->GetConstArray() ;
		m_gpsd.PS.BytecodeLength = shaders.psbcPixel->GetLength() ;
	}
	else
	{
		m_gpsd.PS.pShaderBytecode = nullptr ;
		m_gpsd.PS.BytecodeLength = 0 ;
	}
	if ( shaders.psbcDomain != nullptr )
	{
		m_gpsd.DS.pShaderBytecode = shaders.psbcDomain->GetConstArray() ;
		m_gpsd.DS.BytecodeLength = shaders.psbcDomain->GetLength() ;
	}
	else
	{
		m_gpsd.DS.pShaderBytecode = nullptr ;
		m_gpsd.DS.BytecodeLength = 0 ;
	}
	if ( shaders.psbcHull != nullptr )
	{
		m_gpsd.HS.pShaderBytecode = shaders.psbcHull->GetConstArray() ;
		m_gpsd.HS.BytecodeLength = shaders.psbcHull->GetLength() ;
	}
	else
	{
		m_gpsd.HS.pShaderBytecode = nullptr ;
		m_gpsd.HS.BytecodeLength = 0 ;
	}
	if ( shaders.psbcGeometry != nullptr )
	{
		m_gpsd.GS.pShaderBytecode = shaders.psbcGeometry->GetConstArray() ;
		m_gpsd.GS.BytecodeLength = shaders.psbcGeometry->GetLength() ;
	}
	else
	{
		m_gpsd.GS.pShaderBytecode = nullptr ;
		m_gpsd.GS.BytecodeLength = 0 ;
	}
}

// ルートシグネチャ設定
//////////////////////////////////////////////////////////////////////////////
void SGLDirect3D12Device::PipelineState::SetRootSignature( ID3D12RootSignature * pRootSig )
{
	m_gpsd.pRootSignature = pRootSig ;
}

// シェーダー頂点要素追加
//////////////////////////////////////////////////////////////////////////////
void SGLDirect3D12Device::PipelineState::AddInputLayoutElement
	( LPCSTR pszSemanticName, DXGI_FORMAT format, UINT nAlignedByteOffse )
{
	D3D12_INPUT_ELEMENT_DESC	desc ;
	eslFillMemory( &desc, 0, sizeof(desc) ) ;
	desc.SemanticName = pszSemanticName ;
	desc.Format = format ;
	desc.AlignedByteOffset = nAlignedByteOffse ;
	desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA ;
	//
	m_aLayout.Add( desc ) ;
	//
	m_gpsd.InputLayout.pInputElementDescs = m_aLayout.GetConstArray() ;
	m_gpsd.InputLayout.NumElements = (UINT) m_aLayout.GetLength() ;
}

void SGLDirect3D12Device::PipelineState::AddInputLayoutPosition
	( LPCSTR pszSemanticName, DXGI_FORMAT format, UINT nAlignedByteOffset )
{
	AddInputLayoutElement( pszSemanticName, format, nAlignedByteOffset ) ;
}

void SGLDirect3D12Device::PipelineState::AddInputLayoutNormal
	( LPCSTR pszSemanticName, DXGI_FORMAT format, UINT nAlignedByteOffset )
{
	AddInputLayoutElement( pszSemanticName, format, nAlignedByteOffset ) ;
}

void SGLDirect3D12Device::PipelineState::AddInputLayoutUV
	( LPCSTR pszSemanticName, DXGI_FORMAT format, UINT nAlignedByteOffset )
{
	AddInputLayoutElement( pszSemanticName, format, nAlignedByteOffset ) ;
}

// デプス機能設定
//////////////////////////////////////////////////////////////////////////////
void SGLDirect3D12Device::PipelineState::SetDepthFunc
				( RenderContext::DepthMaskOperation depthMaskOp )
{
	switch( depthMaskOp )
	{
	case	RenderContext::depthMaskDefault:
	case	RenderContext::depthMaskEnable:
		m_gpsd.DepthStencilState.DepthEnable = true ;
		m_gpsd.DepthStencilState.StencilEnable = false ;
		m_gpsd.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL ;
		m_gpsd.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS ;
		m_gpsd.DSVFormat = DXGI_FORMAT_D32_FLOAT ;
		break ;

	case	RenderContext::depthMaskNoWrite:
		m_gpsd.DepthStencilState.DepthEnable = true ;
		m_gpsd.DepthStencilState.StencilEnable = false ;
		m_gpsd.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO ;
		m_gpsd.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS ;
		m_gpsd.DSVFormat = DXGI_FORMAT_D32_FLOAT ;
		break ;

	case	RenderContext::depthMaskNoWriteGT:
		m_gpsd.DepthStencilState.DepthEnable = true ;
		m_gpsd.DepthStencilState.StencilEnable = false ;
		m_gpsd.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO ;
		m_gpsd.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER ;
		m_gpsd.DSVFormat = DXGI_FORMAT_D32_FLOAT ;
		break ;

	case	RenderContext::depthMaskNoTest:
		m_gpsd.DepthStencilState.DepthEnable = false ;
		m_gpsd.DepthStencilState.StencilEnable = false ;
		m_gpsd.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO ;
		m_gpsd.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_NEVER ;
		break ;

	}
}




//////////////////////////////////////////////////////////////////////////////
// Direct3D12 オブジェクト
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLDirect3D12Device::SGLDirect3D12Device( void )
{
	m_flagDebug = false ;
	m_d3dFeatureLevel = D3D_FEATURE_LEVEL_12_0 ;
	m_hrPresentResult = S_OK ;
	m_nFenceValue = 0 ;
	m_iBackBufRTV = -1 ;
	m_nSwapBufCount = 0 ;
	m_flagDepthBuf = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLDirect3D12Device::~SGLDirect3D12Device( void )
{
	Release() ;
}

// デバッグモード有効化
//////////////////////////////////////////////////////////////////////////////
void SGLDirect3D12Device::EnableDebugMode( void )
{
	SComPtr<ID3D12Debug>	d3dDebug ;
	if ( D3D12GetDebugInterface( IID_PPV_ARGS( &d3dDebug.Ref() ) ) == S_OK )
	{
		d3dDebug->EnableDebugLayer() ;
	}
	m_flagDebug = true ;
}

// DXGIFactory を準備する
//////////////////////////////////////////////////////////////////////////////
bool SGLDirect3D12Device::PrepareDXGIFactory( void )
{
	if ( m_dxgiFactory4 != nullptr )
	{
		return	true ;
	}
	HRESULT	hr ;
	hr = CreateDXGIFactory( IID_PPV_ARGS( &m_dxgiFactory.Ref() ) ) ;
	if ( (hr == S_OK) && (m_dxgiFactory != nullptr) )
	{
		m_dxgiFactory4 = m_dxgiFactory.AddRef() ;
	}
	else
	{
		hr = CreateDXGIFactory( IID_PPV_ARGS( &m_dxgiFactory4.Ref() ) ) ;
		if ( (hr != S_OK) || (m_dxgiFactory4 == nullptr) )
		{
			ESLTrace( "failed to CreateDXGIFactory.\n" ) ;
			return	false ;
		}
	}
	return	true ;
}

// DXGIAdapter 配列を列挙する
//////////////////////////////////////////////////////////////////////////////
bool SGLDirect3D12Device::PrepareEnumAdapters( void )
{
	if ( m_aAdapters.GetLength() == 0 )
	{
		if ( !PrepareDXGIFactory()
			|| (m_dxgiFactory4 == nullptr) )
		{
			return	false ;
		}
		for ( UINT i = 0; true; i ++ )
		{
			IDXGIAdapter *	dxgiAdapter =nullptr ;
			if ( m_dxgiFactory4->EnumAdapters( i, &dxgiAdapter ) != S_OK )
			{
				break ;
			}
			if ( dxgiAdapter == nullptr )
			{
				break ;
			}
			m_aAdapters.SetAt
				( i, new SComPtr<IDXGIAdapter>( dxgiAdapter ) ) ;
		}
	}
	return	true ;
}

// アダプター名取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::EnumerateAdapterNames
	( SSystem::SObjectArray<SSystem::SString>& aAdapterNames )
{
	if ( !PrepareEnumAdapters() )
	{
		return	sglErrFailed ;
	}
	for ( size_t i = 0; i < m_aAdapters.GetLength(); i ++ )
	{
		ESLAssert( m_aAdapters.GetAt(i) != nullptr ) ;
		DXGI_ADAPTER_DESC	desc ;
		eslFillMemory( &desc, 0, sizeof(desc) ) ;
		m_aAdapters.At(i)->GetDesc( &desc ) ;
		aAdapterNames.SetAt( i, new SString( desc.Description ) ) ;
		//
		ESLTrace( "Direct3D12 Adapter[%d]:%s\n",
				i, SString(desc.Description).ToCharArray().GetConstArray() ) ;
	}
	return	sglErrSuccess ;
}

// Direct3D12 初期化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::CreateDevice
	( HWND hWnd, UINT nWidth, UINT nHeight,
		bool fFullscreen, bool flagDepthBuffer,
		UINT nSwapBufCount, ssize_t iAdapter )
{
	ESLAssert( m_d3dDevice == nullptr ) ;
	if ( m_d3dDevice != nullptr )
	{
		return	sglErrFailed ;
	}
	//
	// DXGIFactory 作成
	//
#if	defined(__DEBUG__)
	EnableDebugMode() ;
#endif
	if ( !PrepareDXGIFactory() )
	{
		return	sglErrFailed ;
	}
	//
	// D3D12Device 作成
	//
	static const D3D_FEATURE_LEVEL	s_d3dFeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	} ;
	IDXGIAdapter *	dxgiAdapter = nullptr ;
	if ( iAdapter >= 0 )
	{
		if ( PrepareEnumAdapters()
			&& ((size_t) iAdapter < m_aAdapters.GetLength()) )
		{
			dxgiAdapter = m_aAdapters.At( (size_t) iAdapter ) ;
		}
	}
	const size_t	nLevels = sizeof(s_d3dFeatureLevels)
									/ sizeof(s_d3dFeatureLevels[0]) ;
	HRESULT	hr ;
	for ( size_t i = 0; i < nLevels; i ++ )
	{
		hr = D3D12CreateDevice
				( dxgiAdapter,
					s_d3dFeatureLevels[i],
					IID_PPV_ARGS(&m_d3dDevice.Ref()) ) ;
		if ( hr == S_OK )
		{
			m_d3dFeatureLevel = s_d3dFeatureLevels[i] ;
			break ;
		}
	}
	if ( m_d3dDevice == nullptr )
	{
		return	sglErrFailed ;
	}
	//
	// コマンドアロケーター、コマンドリスト作成
	//
	hr = m_d3dDevice->CreateCommandAllocator
				( D3D12_COMMAND_LIST_TYPE_DIRECT,
					IID_PPV_ARGS(&m_d3dCmdAlloc.Ref()) ) ;
	if ( hr != S_OK )
	{
		ESLTrace( "Failed to CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT).(#%08X)\n", hr ) ;
		return	sglErrFailed ;
	}
	hr = m_d3dDevice->CreateCommandList
			( 0, D3D12_COMMAND_LIST_TYPE_DIRECT,
				m_d3dCmdAlloc, nullptr, IID_PPV_ARGS(&m_d3dCmdList.Ref()) ) ;
	if ( hr != S_OK )
	{
		ESLTrace( "Failed to CreateCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT).(#%08X)\n", hr ) ;
		return	sglErrFailed ;
	}
	//
	// コマンドキュー作成
	//
	D3D12_COMMAND_QUEUE_DESC	dscCmdQue ;
	eslFillMemory( &dscCmdQue, 0, sizeof(dscCmdQue) ) ;
	dscCmdQue.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE ;
	dscCmdQue.NodeMask = 0 ;
	dscCmdQue.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL ;
	dscCmdQue.Type = D3D12_COMMAND_LIST_TYPE_DIRECT ;
	//
	hr = m_d3dDevice->CreateCommandQueue
			( &dscCmdQue, IID_PPV_ARGS(&m_d3dCmdQueue.Ref()) ) ;
	if ( hr != S_OK )
	{
		ESLTrace( "Failed to CreateCommandQueue.(#%08X)\n", hr ) ;
		return	sglErrFailed ;
	}
	//
	// フェンスオブジェクト作成
	//
	m_nFenceValue = 0 ;
	m_sevFenceEvent.Initialize( false ) ;
	//
	hr = m_d3dDevice->CreateFence
			( m_nFenceValue,
				D3D12_FENCE_FLAG_NONE,
				IID_PPV_ARGS( &m_d3dFence.Ref() ) ) ;
	if ( hr != S_OK )
	{
		ESLTrace( "Failed to CreateFence.(#%08X)\n", hr ) ;
		return	sglErrFailed ;
	}
	//
	// スワップチェイン作成
	//
	DXGI_SWAP_CHAIN_DESC1	dscSwapChain ;
	eslFillMemory( &dscSwapChain, 0, sizeof(dscSwapChain) ) ;
	dscSwapChain.Width = nWidth ;
	dscSwapChain.Height = nHeight ;
	dscSwapChain.Format = DXGI_FORMAT_R8G8B8A8_UNORM ;
	dscSwapChain.Stereo = false ;
	dscSwapChain.SampleDesc.Count = 1 ;
	dscSwapChain.SampleDesc.Quality = 0 ;
	dscSwapChain.BufferUsage = DXGI_USAGE_BACK_BUFFER ;
	dscSwapChain.BufferCount = nSwapBufCount ;
	dscSwapChain.Scaling = DXGI_SCALING_STRETCH ;
	dscSwapChain.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD ;
	dscSwapChain.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED ;
	dscSwapChain.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH ;
	//
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC	dscFullscreen ;
	eslFillMemory( &dscFullscreen, 0, sizeof(dscFullscreen) ) ;
	dscFullscreen.RefreshRate.Numerator = 60 ;
	dscFullscreen.RefreshRate.Denominator = 0 ;
	dscFullscreen.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED ;
	dscFullscreen.Scaling = DXGI_MODE_SCALING_STRETCHED ;
	dscFullscreen.Windowed = !fFullscreen ;
	//
	hr = m_dxgiFactory4->CreateSwapChainForHwnd
		( m_d3dCmdQueue, hWnd, &dscSwapChain,
			fFullscreen ? &dscFullscreen : nullptr,
			nullptr, (IDXGISwapChain1**) &m_dxgiSwapChain.Ref() ) ;
	if ( hr != S_OK )
	{
		ESLTrace( "Failed to CreateSwapChainForHwnd.(#%08X)\n", hr ) ;
		return	sglErrFailed ;
	}
	m_nSwapBufCount = (size_t) nSwapBufCount ;
	//
	// スワップチェイン・レンダーターゲットビューを準備
	//
	SGLError	err =
		CreateDescriptorHeapRTV( m_dhSwapView, (UINT) m_nSwapBufCount ) ;
	if ( err )
	{
		return	err ;
	}
	for ( size_t i = 0; i < m_nSwapBufCount; i ++ )
	{
		SSmartPointer<Resource>	pRsrcBuf = new Resource ;
		hr = m_dxgiSwapChain->GetBuffer
				( (UINT) i, IID_PPV_ARGS( &(pRsrcBuf->m_d3dRsrc.Ref()) ) ) ;
		if ( hr != S_OK )
		{
			ESLTrace( "Failed to IDXGISwapChain4::GetBuffer(%d).(#%08X)\n", i, hr ) ;
			continue ;
		}
		pRsrcBuf->m_rsStates = D3D12_RESOURCE_STATE_PRESENT ;
		pRsrcBuf->Ptr()->AddRef() ;
		m_d3dDevice->CreateRenderTargetView
			( pRsrcBuf->Ptr(), nullptr,
				GetSwapBufferRTV().GetCPUDescriptorHandle(i) ) ;
		//
		m_aSwapBufs.SetAt( i, pRsrcBuf.Detach() ) ;
	}
	//
	// デプス・バッファ作成
	//
	if ( flagDepthBuffer )
	{
		if ( !CreateTexture2D
			( m_rsDepthBuf, nWidth, nHeight,
				DXGI_FORMAT_D32_FLOAT, 32, 1, 1, false, true ) )
		{
			if ( !CreateDescriptorHeapDSV( m_dhDepthBuf, 1 ) )
			{
				D3D12_DEPTH_STENCIL_VIEW_DESC	dsvd ;
				eslFillMemory( &dsvd, 0, sizeof(dsvd) ) ;
				dsvd.Format = DXGI_FORMAT_D32_FLOAT ;
				dsvd.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D ;
				dsvd.Flags = D3D12_DSV_FLAG_NONE ;
				//
				m_d3dDevice->CreateDepthStencilView
					( m_rsDepthBuf.Ptr(), nullptr,
						m_dhDepthBuf.GetCPUDescriptorHandle() ) ;
				m_flagDepthBuf = true ;
			}
			else
			{
				ESLTrace( "Failed to for depth stencil view descriptor.\n" ) ;
			}
		}
		else
		{
			ESLTrace( "Failed to create depth buffer.\n" ) ;
		}
	}

	return	sglErrSuccess ;
}

// 解放
//////////////////////////////////////////////////////////////////////////////
void SGLDirect3D12Device::Release( void )
{
	m_aAdapters.RemoveAll() ;
	//
	m_d3dFence.Release() ;
	m_d3dCmdQueue.Release() ;
	m_d3dCmdList.Release() ;
	m_d3dCmdAlloc.Release() ;
	//
	m_dxgiSwapChain.Release() ;
	m_dhSwapView.Release() ;
	m_aSwapBufs.RemoveAll() ;
	m_rsDepthBuf.Release() ;
	m_dhDepthBuf.Release() ;
	//
	m_d3dDevice.Release() ;
	//
	m_dxgiFactory4.Release() ;
	m_dxgiFactory.Release() ;
	//
	m_sevFenceEvent.Delete() ;
}

// バックバッファをレンダーターゲットに設定する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::SetBackBufferRenderTarget( void )
{
	ESLAssert( m_dxgiSwapChain != nullptr ) ;
	ESLAssert( m_d3dCmdList != nullptr ) ;
	ESLAssert( m_iBackBufRTV < 0 ) ;
	if ( (m_dxgiSwapChain == nullptr)
		|| (m_d3dCmdList == nullptr)
		|| (m_iBackBufRTV >= 0) )
	{
		return	sglErrFailed ;
	}
	size_t	iBackBuf = (size_t) m_dxgiSwapChain->GetCurrentBackBufferIndex() ;

	CmdResourceTransitionBarrier
		( *(GetSwapBufferAt( iBackBuf )), D3D12_RESOURCE_STATE_RENDER_TARGET ) ;

	if ( m_flagDepthBuf )
	{
		CmdResourceTransitionBarrier
			( *(GetDepthBuffer()), D3D12_RESOURCE_STATE_DEPTH_WRITE ) ;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE	hRTV =
			GetSwapBufferRTV().GetCPUDescriptorHandle( iBackBuf ) ;
	if ( m_flagDepthBuf )
	{
		D3D12_CPU_DESCRIPTOR_HANDLE	hDSV =
				GetDepthView().GetCPUDescriptorHandle( 0 ) ;
		m_d3dCmdList->OMSetRenderTargets( 1, &hRTV, false, &hDSV ) ;
	}
	else
	{
		m_d3dCmdList->OMSetRenderTargets( 1, &hRTV, false, nullptr ) ;
	}

	m_iBackBufRTV = (ssize_t) iBackBuf ;
	return	sglErrSuccess ;
}

// 現在のバックバッファとデプスバッファをクリアする
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::CmdClearBackBuffer( SGLPalette rgba, bool flagDepthBuf )
{
	ESLAssert( m_d3dCmdList != nullptr ) ;
	ESLAssert( m_iBackBufRTV >= 0 ) ;
	if ( (m_d3dCmdList == nullptr)
		|| (m_iBackBufRTV < 0) )
	{
		return	sglErrFailed ;
	}
	float	rgbaClear[] =
	{
		(float) rgba.argb.Red / 255.0f,
		(float) rgba.argb.Green / 255.0f,
		(float) rgba.argb.Blue / 255.0f,
		(float) rgba.argb.Alpha / 255.0f
	} ;
	m_d3dCmdList->ClearRenderTargetView
		( GetSwapBufferRTV().
			GetCPUDescriptorHandle
				( (size_t) m_iBackBufRTV ), rgbaClear, 0, nullptr ) ;
	//
	if ( m_flagDepthBuf && flagDepthBuf )
	{
		m_d3dCmdList->ClearDepthStencilView
			( GetDepthView().GetCPUDescriptorHandle(),
					D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr ) ;
	}
	return	sglErrSuccess ;
}

// 現在レンダーターゲットに設定しているバックバッファ番号を取得
//////////////////////////////////////////////////////////////////////////////
ssize_t SGLDirect3D12Device::GetCurrentBackBufferRTVIndex( void )
{
	return	m_iBackBufRTV ;
}

// バックバッファのレンダーターゲットを終了する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::FinishBackBufferRenderTarget( void )
{
	ESLAssert( m_dxgiSwapChain != nullptr ) ;
	if ( m_iBackBufRTV < 0 )
	{
		return	sglErrFailed ;
	}
	ESLAssert( m_d3dCmdList != nullptr ) ;

	CmdResourceTransitionBarrier
		( *(GetSwapBufferAt( (size_t) m_iBackBufRTV )),
						D3D12_RESOURCE_STATE_PRESENT ) ;

	if ( m_flagDepthBuf )
	{
		CmdResourceTransitionBarrier
			( *(GetDepthBuffer()), D3D12_RESOURCE_STATE_GENERIC_READ ) ;
	}

	m_iBackBufRTV = -1 ;
	return	sglErrSuccess ;
}

// パイプラインとルートシグネチャを設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::SetGraphicsPipeline
	( ID3D12PipelineState * pPipeline, ID3D12RootSignature * pRootSig )
{
	ESLAssert( m_d3dCmdList != nullptr ) ;
	if ( m_d3dCmdList == nullptr )
	{
		return	sglErrFailed ;
	}
	m_d3dCmdList->SetPipelineState( pPipeline ) ;
	m_d3dCmdList->SetGraphicsRootSignature( pRootSig ) ;
	return	sglErrSuccess ;
}

// ディスクリプタ・テーブル設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::SetGraphicsRootDescriptorTable
	( size_t iRootParam,
		const SGLDirect3D12Device::DescriptorHeap& heap, size_t iDesc )
{
	ESLAssert( m_d3dCmdList != nullptr ) ;
	if ( m_d3dCmdList == nullptr )
	{
		return	sglErrFailed ;
	}
	m_d3dCmdList->SetGraphicsRootDescriptorTable
		( (UINT) iRootParam, heap.GetGPUDescriptorHandle( iDesc ) ) ;
	return	sglErrSuccess ;
}

// ビューポート・描画領域設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::SetViewport
	( float xTopLeft, float yTopLeft,
		float width, float height, float zMinDepth, float zMaxDepth )
{
	ESLAssert( m_d3dCmdList != nullptr ) ;
	if ( m_d3dCmdList == nullptr )
	{
		return	sglErrFailed ;
	}

	D3D12_VIEWPORT	d3dViewport ;
	eslFillMemory( &d3dViewport, 0, sizeof(d3dViewport) ) ;

	d3dViewport.TopLeftX = xTopLeft ;
	d3dViewport.TopLeftY = yTopLeft ;
	d3dViewport.Width = width ;
	d3dViewport.Height = height ;
	d3dViewport.MinDepth = zMinDepth ;
	d3dViewport.MaxDepth = zMaxDepth ;

	m_d3dCmdList->RSSetViewports( 1, &d3dViewport ) ;


	D3D12_RECT	rctView ;
	eslFillMemory( &rctView, 0, sizeof(rctView) ) ;

	rctView.left = esl_roundfi( xTopLeft ) ;
	rctView.top = esl_roundfi( yTopLeft ) ;
	rctView.right = esl_roundfi( xTopLeft + width ) ;
	rctView.bottom = esl_roundfi( yTopLeft + height ) ;

	m_d3dCmdList->RSSetScissorRects( 1, &rctView ) ;

	return	sglErrSuccess ;
}

// ディスクリプタ・ヒープ設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::SetDescriptorHeaps
	( size_t nCount, const SGLDirect3D12Device::DescriptorHeap *const* ppHeaps )
{
	ESLAssert( m_d3dCmdList != nullptr ) ;
	if ( m_d3dCmdList == nullptr )
	{
		return	sglErrFailed ;
	}

	SPointerArray<ID3D12DescriptorHeap>	aPtrHeaps ;
	ID3D12DescriptorHeap**	ppDescHeaps = aPtrHeaps.GetArray( nCount ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ppDescHeaps[i] = ppHeaps[i]->Ptr() ;
	}
	m_d3dCmdList->SetDescriptorHeaps( (UINT) nCount, ppDescHeaps ) ;

	return	sglErrSuccess ;
}

// 描画領域設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::SetScissorRects( size_t nCount, const D3D12_RECT * pRects )
{
	ESLAssert( m_d3dCmdList != nullptr ) ;
	if ( m_d3dCmdList == nullptr )
	{
		return	sglErrFailed ;
	}
	m_d3dCmdList->RSSetScissorRects( (UINT) nCount, pRects ) ;
	return	sglErrSuccess ;
}

// CloseCommandList / ExecuteCommandList / WaitForCompleted / ResetCommandList 実行
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::SyncExecuteCommandList( void )
{
	SGLError	err = CloseCommandList() ;
	if ( !err )
	{
		err = ExecuteCommandList() ;
		if ( !err )
		{
			err = WaitForCompleted() ;
			if ( !err )
			{
				err = ResetCommandList() ;
			}
		}
	}
	return	err ;
}

// コマンドリストを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::CloseCommandList( void )
{
	ESLAssert( m_d3dCmdList != nullptr ) ;
	if ( m_d3dCmdList == nullptr )
	{
		return	sglErrFailed ;
	}
	HRESULT	hr = m_d3dCmdList->Close() ;
	if ( hr != S_OK )
	{
		ESLTrace( "Failed to ID3D12GraphicsCommandList::Close.(#%08X)\n", hr ) ;
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// コマンドリスト実行開始
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::ExecuteCommandList( void )
{
	ESLAssert( m_d3dCmdQueue != nullptr ) ;
	ESLAssert( m_d3dFence != nullptr ) ;
	if ( (m_d3dCmdQueue == nullptr) || (m_d3dFence == nullptr) )
	{
		return	sglErrFailed ;
	}
	ID3D12CommandList*	pCmdLists[] =
	{
		m_d3dCmdList.Ptr()
	} ;
	m_d3dCmdQueue->ExecuteCommandLists( 1, pCmdLists ) ;
	m_d3dCmdQueue->Signal( m_d3dFence, ++ m_nFenceValue ) ;
	//
	m_sevFenceEvent.ResetSignal() ;
	m_d3dFence->SetEventOnCompletion
			( m_nFenceValue, m_sevFenceEvent.GetHandle() ) ;
	return	sglErrSuccess ;
}

// コマンド実行完了待機
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::WaitForCompleted( int64_t msecTimeout )
{
	ESLAssert( m_d3dFence != nullptr ) ;
	if ( m_d3dFence->GetCompletedValue() == m_nFenceValue )
	{
		return	sglErrSuccess ;
	}
	return	(SGLError) m_sevFenceEvent.Wait( msecTimeout ) ;
}

// コマンドリスト／アロケータ・リセット
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::ResetCommandList( void )
{
	ESLAssert( m_d3dCmdAlloc != nullptr ) ;
	ESLAssert( m_d3dCmdList != nullptr ) ;
	if ( (m_d3dCmdAlloc == nullptr) || (m_d3dCmdList == nullptr) )
	{
		return	sglErrFailed ;
	}
	SGLError	err = sglErrSuccess ;
	HRESULT		hr = m_d3dCmdAlloc->Reset() ;
	if ( hr != S_OK )
	{
		ESLTrace( "Failed to ID3D12CommandAllocator::Reset.(#%08X)\n", hr ) ;
		err = sglErrFailed ;
	}
	hr = m_d3dCmdList->Reset( m_d3dCmdAlloc, nullptr ) ;
	if ( hr != S_OK )
	{
		ESLTrace( "Failed to ID3D12GraphicsCommandList::Reset.(#%08X)\n", hr ) ;
		err = sglErrFailed ;
	}
	return	err ;
}

// スワップ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::Present( UINT SyncInterval, UINT Flags )
{
	ESLAssert( m_dxgiSwapChain != nullptr ) ;
	if ( m_dxgiSwapChain == nullptr )
	{
		return	sglErrFailed ;
	}
	m_hrPresentResult = m_dxgiSwapChain->Present( SyncInterval, Flags ) ;
	if ( m_hrPresentResult != S_OK )
	{
		if ( m_hrPresentResult == DXGI_ERROR_DEVICE_RESET )
		{
			ESLTrace( "DXGI_ERROR_DEVICE_RESET: IDXGISwapChain::Present\n" ) ;
			return	sglErrContinue ;
		}
		else if ( m_hrPresentResult == DXGI_ERROR_DEVICE_REMOVED )
		{
			ESLTrace( "DXGI_ERROR_DEVICE_REMOVED: IDXGISwapChain::Present\n" ) ;
			return	sglErrContinue ;
		}
		else if ( m_hrPresentResult == DXGI_STATUS_OCCLUDED )
		{
			ESLTrace( "DXGI_STATUS_OCCLUDED: IDXGISwapChain::Present\n" ) ;
			return	sglErrPending ;
		}
		else
		{
			ESLTrace( "Failed to IDXGISwapChain::Present.(#%08X)\n", m_hrPresentResult ) ;
			return	sglErrFailed ;
		}
	}
	return	sglErrSuccess ;
}

// Present エラーコード
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDirect3D12Device::GetPresentResult( void ) const
{
	return	m_hrPresentResult ;
}

// トランジッション・リソースバリア
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::CmdResourceTransitionBarrier
	( SGLDirect3D12Device::Resource& rsrc,
			D3D12_RESOURCE_STATES states, UINT nSubrsrc )
{
	if ( rsrc.GetStates() == states )
	{
		return	sglErrSuccess ;
	}
	ESLAssert( m_d3dCmdList != nullptr ) ;
	if ( m_d3dCmdList == nullptr )
	{
		return	sglErrFailed ;
	}
	D3D12_RESOURCE_BARRIER	rb ;
	eslFillMemory( &rb, 0, sizeof(rb) ) ;
	rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION ;
	rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE ;
	rb.Transition.pResource = rsrc.Ptr() ;
	rb.Transition.Subresource = nSubrsrc ;
	rb.Transition.StateBefore = rsrc.GetStates() ;
	rb.Transition.StateAfter = states ;
	//
	m_d3dCmdList->ResourceBarrier( 1, &rb ) ;
	//
	rsrc.m_rsStates = states ;
	return	sglErrSuccess ;
}

// バーテックスバッファ設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::CmdSetVertexBuffer
	( size_t iSlot, const SGLDirect3D12Device::Resource& rsrc, size_t nStride )
{
	ESLAssert( m_d3dCmdList != nullptr ) ;
	if ( m_d3dCmdList == nullptr )
	{
		return	sglErrFailed ;
	}

	D3D12_VERTEX_BUFFER_VIEW	vbv ;
	vbv.BufferLocation = rsrc.Ptr()->GetGPUVirtualAddress() ;
	vbv.SizeInBytes = (UINT) rsrc.GetSizeInBytes() ;
	vbv.StrideInBytes = (UINT) nStride ;
	//
	m_d3dCmdList->IASetVertexBuffers( (UINT) iSlot, 1, &vbv ) ;

	return	sglErrSuccess ;
}

// インデックスバッファ設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::CmdSetIndexBuffer
	( const SGLDirect3D12Device::Resource& rsrc, DXGI_FORMAT format )
{
	ESLAssert( m_d3dCmdList != nullptr ) ;
	if ( m_d3dCmdList == nullptr )
	{
		return	sglErrFailed ;
	}

	D3D12_INDEX_BUFFER_VIEW	ibv ;
	ibv.BufferLocation = rsrc.Ptr()->GetGPUVirtualAddress() ;
	ibv.SizeInBytes = (UINT) rsrc.GetSizeInBytes() ;
	ibv.Format = format ;
	//
	m_d3dCmdList->IASetIndexBuffer( &ibv ) ;

	return	sglErrSuccess ;
}

// プリミティブタイプ設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::CmdSetPrimitiveTopology
	( D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology )
{
	ESLAssert( m_d3dCmdList != nullptr ) ;
	if ( m_d3dCmdList == nullptr )
	{
		return	sglErrFailed ;
	}
	m_d3dCmdList->IASetPrimitiveTopology( PrimitiveTopology ) ;
	return	sglErrSuccess ;
}

// 描画実行
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::CmdDrawInstanced
	( UINT VertexCountPerInstance, UINT InstanceCount,
		UINT StartVertexLocation, UINT StartInstanceLocation )
{
	ESLAssert( m_d3dCmdList != nullptr ) ;
	if ( m_d3dCmdList == nullptr )
	{
		return	sglErrFailed ;
	}
	m_d3dCmdList->DrawInstanced
		( VertexCountPerInstance, InstanceCount,
			StartVertexLocation , StartInstanceLocation ) ;
	return	sglErrSuccess ;
}

SGLError SGLDirect3D12Device::CmdDrawIndexedInstanced
	( UINT IndexCountPerInstance, UINT InstanceCount,
		UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation )
{
	ESLAssert( m_d3dCmdList != nullptr ) ;
	if ( m_d3dCmdList == nullptr )
	{
		return	sglErrFailed ;
	}
	m_d3dCmdList->DrawIndexedInstanced
		( IndexCountPerInstance, InstanceCount,
			StartIndexLocation , BaseVertexLocation, StartInstanceLocation ) ;
	return	sglErrSuccess ;
}

// ディスクリプタ・ヒープ作成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::CreateDescriptorHeap
		( SGLDirect3D12Device::DescriptorHeap& heap,
			const D3D12_DESCRIPTOR_HEAP_DESC& dscHeap )
{
	ESLAssert( m_d3dDevice != nullptr ) ;
	if ( m_d3dDevice == nullptr )
	{
		return	sglErrFailed ;
	}
	HRESULT	hr = m_d3dDevice->CreateDescriptorHeap
					( &dscHeap, IID_PPV_ARGS( &heap.m_d3dHeap.Ref() ) ) ;
	if ( hr != S_OK )
	{
		ESLTrace( "Failed to CreateDescriptorHeap.(#%08X)\n", hr ) ;
		return	sglErrFailed ;
	}
	heap.m_typeHeap = dscHeap.Type ;
	heap.m_countDesc = (size_t) dscHeap.NumDescriptors ;
	heap.m_sizeOfType =
		m_d3dDevice->GetDescriptorHandleIncrementSize( dscHeap.Type ) ;
	return	sglErrSuccess ;
}

SGLError SGLDirect3D12Device::CreateDescriptorHeap
	( SGLDirect3D12Device::DescriptorHeap& heap,
		D3D12_DESCRIPTOR_HEAP_TYPE type,
		UINT nDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags )
{
	D3D12_DESCRIPTOR_HEAP_DESC	dscHeap ;
	eslFillMemory( &dscHeap, 0, sizeof(dscHeap) ) ;
	dscHeap.Type = type ;
	dscHeap.NumDescriptors = nDescriptors ;
	dscHeap.Flags = flags ;
	dscHeap.NodeMask = 0 ;
	//
	return	CreateDescriptorHeap( heap, dscHeap ) ;
}

SGLError SGLDirect3D12Device::CreateDescriptorHeapRTV
	( DescriptorHeap& heap, UINT nDescriptors )
{
	return	CreateDescriptorHeap
			( heap, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, nDescriptors ) ;
}

SGLError SGLDirect3D12Device::CreateDescriptorHeapDSV
	( SGLDirect3D12Device::DescriptorHeap& heap, UINT nDescriptors )
{
	return	CreateDescriptorHeap
			( heap, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, nDescriptors ) ;
}

SGLError SGLDirect3D12Device::CreateDescriptorHeapCBV_SRV_UAV
	( SGLDirect3D12Device::DescriptorHeap& heap, UINT nDescriptors )
{
	return	CreateDescriptorHeap
			( heap, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
				nDescriptors, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE ) ;
}

// CBV ディスクリプタ作成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::CreateConstantBufferView
	( SGLDirect3D12Device::DescriptorHeap& heap, size_t index,
		const SGLDirect3D12Device::Resource& rsrc, size_t nOffset, ssize_t nBytes )
{
	ESLAssert( m_d3dDevice != nullptr ) ;
	if ( m_d3dDevice == nullptr )
	{
		return	sglErrFailed ;
	}
	ESLAssert( nOffset <= rsrc.GetSizeInBytes() ) ;
	ESLAssert( (nBytes < 0) || (nOffset + nBytes <= rsrc.GetSizeInBytes()) ) ;

	D3D12_CONSTANT_BUFFER_VIEW_DESC	cbvd ;
	eslFillMemory( &cbvd, 0, sizeof(cbvd) ) ;
	cbvd.BufferLocation = rsrc.Ptr()->GetGPUVirtualAddress() + nOffset ;
	cbvd.SizeInBytes = (nBytes >= 0) ? (UINT) nBytes
								: (UINT) (rsrc.GetSizeInBytes() - nOffset) ;

	m_d3dDevice->CreateConstantBufferView
		( &cbvd, heap.GetCPUDescriptorHandle(index) ) ;

	return	sglErrSuccess ;
}

// SRV ディスクリプタ作成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::CreateSRVTexture2D
	( SGLDirect3D12Device::DescriptorHeap& heap, size_t index,
		const SGLDirect3D12Device::Resource& rsrc )
{
	ESLAssert( m_d3dDevice != nullptr ) ;
	if ( m_d3dDevice == nullptr )
	{
		return	sglErrFailed ;
	}
	D3D12_SHADER_RESOURCE_VIEW_DESC	srvd ;
	eslFillMemory( &srvd, 0, sizeof(srvd) ) ;
	srvd.Format = rsrc.GetDesc().Format ;
	srvd.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING ;
	srvd.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D ;
	srvd.Texture2D.MipLevels = 1 ;

	m_d3dDevice->CreateShaderResourceView
		( rsrc.Ptr(), &srvd, heap.GetCPUDescriptorHandle(index) ) ;

	return	sglErrSuccess ;
}

// ルートシグネチャ作成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::CreateRootSignature
	( SComPtr<ID3D12RootSignature>& rootSig,
		const D3D12_ROOT_SIGNATURE_DESC& desc,
		D3D_ROOT_SIGNATURE_VERSION version )
{
	ESLAssert( m_d3dDevice != nullptr ) ;
	if ( m_d3dDevice == nullptr )
	{
		return	sglErrFailed ;
	}
	SComPtr<ID3DBlob>	blobSig ;
	SComPtr<ID3DBlob>	blobErr ;
	HRESULT	hr = D3D12SerializeRootSignature
					( &desc, version, &blobSig.Ref(), &blobErr.Ref() ) ;
	if ( hr != S_OK )
	{
		ESLTrace( "Failed to D3D12SerializeRootSignature.(#%08X)\n", hr ) ;
		return	sglErrFailed ;
	}
	if ( blobSig == nullptr )
	{
		return	sglErrFailed ;
	}
	hr = m_d3dDevice->CreateRootSignature
		( 0, blobSig->GetBufferPointer(),
			blobSig->GetBufferSize(), IID_PPV_ARGS( &rootSig.Ref() ) ) ;
	if ( hr != S_OK )
	{
		ESLTrace( "Failed to ID3D12Device::CreateRootSignature.(#%08X)\n", hr ) ;
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// パイプライン・ステート作成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::CreateGraphicsPipelineState
	( SComPtr<ID3D12PipelineState>& pipeline,
			const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc )
{
	ESLAssert( m_d3dDevice != nullptr ) ;
	if ( m_d3dDevice == nullptr )
	{
		return	sglErrFailed ;
	}
	HRESULT	hr = m_d3dDevice->CreateGraphicsPipelineState
							( &desc, IID_PPV_ARGS( &pipeline.Ref() ) ) ;
	if ( hr != S_OK )
	{
		ESLTrace( "Failed to ID3D12Device::CreateGraphicsPipelineState.(#%08X)\n", hr ) ;
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// リソース作成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::CreateCommittedResource
	( SGLDirect3D12Device::Resource& rsrc, size_t nBytes,
		const D3D12_HEAP_PROPERTIES& heapProp,
		D3D12_HEAP_FLAGS flags, const D3D12_RESOURCE_DESC& desc,
		D3D12_RESOURCE_STATES stateInit,
		const D3D12_CLEAR_VALUE * pOptClearValue )
{
	ESLAssert( m_d3dDevice != nullptr ) ;
	if ( m_d3dDevice == nullptr )
	{
		return	sglErrFailed ;
	}
	HRESULT	hr =
		m_d3dDevice->CreateCommittedResource
			( &heapProp, flags, &desc, stateInit,
				pOptClearValue, IID_PPV_ARGS( &rsrc.m_d3dRsrc.Ref() ) ) ;
	if ( hr != S_OK )
	{
		ESLTrace( "Failed to CreateCommittedResource.(#%08X)\n", hr ) ;
		return	sglErrFailed ;
	}
	rsrc.m_dscRsrc = desc ;
	rsrc.m_rsStates = stateInit ;
	rsrc.m_nBytes = nBytes ;
	return	sglErrSuccess ;
}

SGLError SGLDirect3D12Device::CreateVertexBuffer
	( SGLDirect3D12Device::Resource& rsrc, size_t nBytes )
{
	D3D12_HEAP_PROPERTIES	heapProp ;
	eslFillMemory( &heapProp, 0, sizeof(heapProp) ) ;
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD ;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN ;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN ;
	//
	D3D12_RESOURCE_DESC	desc ;
	eslFillMemory( &desc, 0, sizeof(desc) ) ;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER ;
	desc.Width = (UINT64) nBytes ;
	desc.Height = 1 ;
	desc.DepthOrArraySize = 1 ;
	desc.MipLevels = 1 ;
	desc.Format = DXGI_FORMAT_UNKNOWN ;
	desc.SampleDesc.Count = 1 ;
	desc.SampleDesc.Quality = 0 ;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR ;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE ;
	//
	return	CreateCommittedResource
				( rsrc, nBytes, heapProp, D3D12_HEAP_FLAG_NONE,
					desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr ) ;
}

SGLError SGLDirect3D12Device::CreateConstantBuffer
	( SGLDirect3D12Device::Resource& rsrc, size_t nBytes )
{
	return	CreateVertexBuffer( rsrc, AlignTexturePitch( nBytes ) ) ;
}

SGLError SGLDirect3D12Device::CreateTexture2D
	( SGLDirect3D12Device::Resource& rsrc,
		size_t nWidth, size_t nHeight,
		DXGI_FORMAT format, size_t bitsPerPixel,
		size_t nArraySize, size_t nMipLevels,
		bool flagMemoryL0, bool flagDepth )
{
	D3D12_HEAP_PROPERTIES	heapProp ;
	eslFillMemory( &heapProp, 0, sizeof(heapProp) ) ;
	if ( flagMemoryL0 )
	{
		heapProp.Type = D3D12_HEAP_TYPE_CUSTOM ;
		heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK ;
		heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0 ;
	}
	else
	{
		heapProp.Type = D3D12_HEAP_TYPE_DEFAULT ;
		heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN ;
		heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN ;
	}
	//
	D3D12_RESOURCE_DESC	desc ;
	eslFillMemory( &desc, 0, sizeof(desc) ) ;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D ;
	desc.Width = (UINT64) nWidth ;
	desc.Height = (UINT) nHeight ;
	desc.DepthOrArraySize = (UINT16) nArraySize ;
	desc.MipLevels = (UINT16) nMipLevels ;
	desc.Format = format ;
	desc.SampleDesc.Count = 1 ;
	desc.SampleDesc.Quality = 0 ;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN ;
	desc.Flags = flagDepth ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
							: D3D12_RESOURCE_FLAG_NONE ;
	//
	D3D12_RESOURCE_STATES	rsState = D3D12_RESOURCE_STATE_GENERIC_READ ;
	D3D12_CLEAR_VALUE *	pClearValue = nullptr ;
	D3D12_CLEAR_VALUE	dcv ;
	if ( flagDepth )
	{
		ESLAssert( format == DXGI_FORMAT_D32_FLOAT ) ;
		ESLAssert( bitsPerPixel == 32 ) ;
		//
		eslFillMemory( &dcv, 0, sizeof(dcv) ) ;
		dcv.DepthStencil.Depth = 1.0f ;
		dcv.Format = DXGI_FORMAT_D32_FLOAT ;
		pClearValue = &dcv ;
		//
		desc.Format = DXGI_FORMAT_D32_FLOAT ;
		bitsPerPixel = 32 ;
	}
	//
	size_t	nRowBytes = (nWidth * bitsPerPixel+ 7) / 8 ;
	return	CreateCommittedResource
				( rsrc, nRowBytes * nHeight * nArraySize,
					heapProp, D3D12_HEAP_FLAG_NONE,
					desc, rsState, pClearValue ) ;
}

SGLError SGLDirect3D12Device::CreateTexture2DUploadBuffer
	( SGLDirect3D12Device::Resource& rsrc,
		size_t nWidth, size_t nHeight,
		DXGI_FORMAT format, size_t bitsPerPixel, size_t nArraySize )
{
	size_t	nRowBytes = (nWidth * bitsPerPixel+ 7) / 8 ;
	size_t	nAlignedRowBytes = AlignTexturePitch( nRowBytes ) ;
	return	CreateVertexBuffer( rsrc, nAlignedRowBytes * nHeight * nArraySize ) ;
}

// テクスチャ・コピー
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D12Device::CmdCopyTextureRegion
	( SGLDirect3D12Device::Resource& rsDst, UINT xDst, UINT yDst, UINT zDst,
		SGLDirect3D12Device::Resource& rsSrc, const D3D12_BOX * pSrcBox )
{
	ESLAssert( m_d3dCmdList != nullptr ) ;
	if ( m_d3dCmdList == nullptr )
	{
		return	sglErrFailed ;
	}

	D3D12_TEXTURE_COPY_LOCATION	tclSrc ;
	D3D12_TEXTURE_COPY_LOCATION	tclDst ;
	bool	flagSrcBarrier = false ;
	bool	flagDstBarrier = false ;
	eslFillMemory( &tclSrc, 0, sizeof(tclSrc) ) ;
	eslFillMemory( &tclDst, 0, sizeof(tclDst) ) ;

	tclSrc.pResource = rsSrc.Ptr() ;
	if ( rsSrc.GetDesc().Format == DXGI_FORMAT_UNKNOWN )
	{
		ESLAssert( rsDst.GetDesc().Format != DXGI_FORMAT_UNKNOWN ) ;
		if ( rsDst.GetDesc().Format == DXGI_FORMAT_UNKNOWN )
		{
			return	sglErrFailed ;
		}
		tclSrc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT ;
		tclSrc.PlacedFootprint.Offset = 0 ;
		tclSrc.PlacedFootprint.Footprint.Width = (UINT) rsDst.GetDesc().Width ;
		tclSrc.PlacedFootprint.Footprint.Height = rsDst.GetDesc().Height ;
		tclSrc.PlacedFootprint.Footprint.Depth = rsDst.GetDesc().DepthOrArraySize ;
		tclSrc.PlacedFootprint.Footprint.RowPitch =
			(UINT) (rsSrc.GetSizeInBytes()
					/ (tclSrc.PlacedFootprint.Footprint.Height
						* tclSrc.PlacedFootprint.Footprint.Depth)) ;
		tclSrc.PlacedFootprint.Footprint.Format = rsDst.GetDesc().Format ;
	}
	else
	{
		tclSrc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX ;
		tclSrc.SubresourceIndex = 0 ;
		//
		CmdResourceTransitionBarrier( rsSrc, D3D12_RESOURCE_STATE_COPY_SOURCE ) ;
		flagSrcBarrier = true ;
	}

	tclDst.pResource = rsDst.Ptr() ;
	if ( rsDst.GetDesc().Format == DXGI_FORMAT_UNKNOWN )
	{
		ESLAssert( rsSrc.GetDesc().Format != DXGI_FORMAT_UNKNOWN ) ;
		if ( rsSrc.GetDesc().Format == DXGI_FORMAT_UNKNOWN )
		{
			return	sglErrFailed ;
		}
		tclDst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT ;
		tclDst.PlacedFootprint.Offset = 0 ;
		tclDst.PlacedFootprint.Footprint.Width = (UINT) rsSrc.GetDesc().Width ;
		tclDst.PlacedFootprint.Footprint.Height = rsSrc.GetDesc().Height ;
		tclDst.PlacedFootprint.Footprint.Depth = rsSrc.GetDesc().DepthOrArraySize ;
		tclDst.PlacedFootprint.Footprint.RowPitch =
			(UINT) (rsDst.GetSizeInBytes()
					/ (tclDst.PlacedFootprint.Footprint.Height
						* tclDst.PlacedFootprint.Footprint.Depth)) ;
		tclDst.PlacedFootprint.Footprint.Format = rsSrc.GetDesc().Format ;
	}
	else
	{
		tclDst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX ;
		tclDst.SubresourceIndex = 0 ;
		//
		CmdResourceTransitionBarrier( rsDst, D3D12_RESOURCE_STATE_COPY_DEST ) ;
		flagDstBarrier = true ;
	}

	m_d3dCmdList->CopyTextureRegion
			( &tclDst, xDst, yDst, zDst, &tclSrc, pSrcBox ) ;

	if ( flagSrcBarrier )
	{
		CmdResourceTransitionBarrier( rsSrc, D3D12_RESOURCE_STATE_GENERIC_READ ) ;
	}
	if ( flagDstBarrier )
	{
		CmdResourceTransitionBarrier( rsDst, D3D12_RESOURCE_STATE_GENERIC_READ ) ;
	}
	return	sglErrSuccess ;
}

// テクスチャ・ピッチ・アライメント
//////////////////////////////////////////////////////////////////////////////
size_t SGLDirect3D12Device::AlignTexturePitch( size_t nBytes )
{
	size_t	nAlignedCount = (nBytes + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1)
										/ D3D12_TEXTURE_DATA_PITCH_ALIGNMENT ;
	return	nAlignedCount * D3D12_TEXTURE_DATA_PITCH_ALIGNMENT ;
}

// DXGI 画像フォーマット
//////////////////////////////////////////////////////////////////////////////
SGLDirect3D12Device::FormatInfo
	SGLDirect3D12Device::FromDXGIFormat
		( DXGI_FORMAT format, uint32_t formatErr, uint32_t bitsErr )
{
	for ( size_t i = 0; s_dxgiFormatPairs[i].dxgif != DXGI_FORMAT_UNKNOWN; i ++ )
	{
		if ( s_dxgiFormatPairs[i].dxgif == format )
		{
			return	s_dxgiFormatPairs[i].fmtinf ;
		}
	}
	FormatInfo	fmtinf ;
	fmtinf.format = formatErr ;
	fmtinf.bitsPerBlock = bitsErr ;
	fmtinf.blockPixels = 1 ;
	return	fmtinf ;
}

DXGI_FORMAT SGLDirect3D12Device::ToDXGIFormat
			( uint32_t format, uint32_t bitsPerBlock )
{
	for ( size_t i = 0; s_dxgiFormatPairs[i].dxgif != DXGI_FORMAT_UNKNOWN; i ++ )
	{
		if ( (s_dxgiFormatPairs[i].fmtinf.format == format)
			&& (s_dxgiFormatPairs[i].fmtinf.bitsPerBlock == bitsPerBlock) )
		{
			return	s_dxgiFormatPairs[i].dxgif ;
		}
	}
	return	DXGI_FORMAT_UNKNOWN ;
}

const SGLDirect3D12Device::DXGIFormatPair
	SGLDirect3D12Device::s_dxgiFormatPairs[15] =
{
	{ DXGI_FORMAT_R8G8B8A8_UNORM, { formatImageABGR, 32, 1 } },
	{ DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, { formatImageABGR, 32, 1 } },
	{ DXGI_FORMAT_R8G8B8A8_UNORM, { formatImageBGR, 32, 1 } },
	{ DXGI_FORMAT_B8G8R8A8_UNORM, { formatImageARGB, 32, 1 } },
	{ DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, { formatImageARGB, 32, 1 } },
	{ DXGI_FORMAT_B8G8R8A8_UNORM, { formatImageRGB, 32, 1 } },
	{ DXGI_FORMAT_D32_FLOAT, { formatImageDepth, 32, 1 } },
	{ DXGI_FORMAT_R32_FLOAT, { formatImageZ, 32, 1 } },
	{ DXGI_FORMAT_R32G32B32A32_FLOAT, { formatImageFloatABGR, 32*4, 1 } },
	{ DXGI_FORMAT_R32G32B32_FLOAT, { formatImageFloatBGR, 32*3, 1 } },
	{ DXGI_FORMAT_A8_UNORM, { formatImageGray, 8, 1 } },
	{ DXGI_FORMAT_B5G6R5_UNORM, { formatImageRGB, 16, 1 } },
	{ DXGI_FORMAT_AYUV, { formatImageYUV | formatImageFlagAlpha, 32, 1 } },
	{ DXGI_FORMAT_YUY2, { formatImageYUV2, 32, 2 } },
	{ DXGI_FORMAT_UNKNOWN, { 0, 32, 1 } },
} ;

