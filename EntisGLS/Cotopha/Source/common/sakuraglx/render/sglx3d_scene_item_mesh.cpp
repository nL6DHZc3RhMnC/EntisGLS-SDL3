
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/render/sglx3d_scene_item.h>

using namespace SSystem ;
using namespace SakuraGL ;
using namespace Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// メッシュバッファ・シリアライザ（要素）
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DMeshBufferPropertySerializer::S3DMeshBufferPropertySerializer( void )
{
	m_binHeader.nType = S3DSceneComposer::binaryMesh ;
	m_binHeader.nSubType = 0 ;
	m_binHeader.nBodyBytes = 0 ;
	m_binHeader.nReserved = 0 ;
	m_flagUpdateMesh = true ;
}

// メッシュバッファ編集
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshBufferPropertySerializer::GetMeshCount( void ) const
{
	return	m_aMeshs.GetLength() ;
}

S3DMeshBufferPropertySerializer::MeshBuffer *
	S3DMeshBufferPropertySerializer::GetMeshAt( size_t i ) const
{
	return	m_aMeshs.GetAt( i ) ;
}

size_t S3DMeshBufferPropertySerializer::AddMesh
		( S3DMeshBufferPropertySerializer::MeshBuffer * pMesh )
{
	m_flagUpdateMesh = true ;
	return	m_aMeshs.Add( pMesh ) ;
}

ssize_t S3DMeshBufferPropertySerializer::FindMesh( MeshBuffer * pMesh ) const
{
	return	m_aMeshs.FindPtr( pMesh ) ;
}

void S3DMeshBufferPropertySerializer::SwapMesh( size_t i0, size_t i1 )
{
	m_aMeshs.Swap( i0, i1 ) ;
}

void S3DMeshBufferPropertySerializer::RemoveMeshAt( size_t i )
{
	m_aMeshs.RemoveAt( i ) ;
	m_flagUpdateMesh = true ;
}

void S3DMeshBufferPropertySerializer::UpdateMesh
	( S3DMeshBufferPropertySerializer::MeshBuffer * pMesh )
{
	if ( pMesh != NULL )
	{
		pMesh->flagUpdate = true ;
	}
	m_flagUpdateMesh = true ;
}

// アイテムから取得
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshBufferPropertySerializer::GetMeshBufferParameterFrom
	( const S3DSceneComposer::Parameter& item, size_t iParam )
{
	size_t	nBytes = item.GetBinaryParameter( NULL, 0, iParam ) ;
	void *	pData = esl_malloc( nBytes ) ;
	item.GetBinaryParameter( pData, nBytes, iParam ) ;
	//
	bool	fSuccess = DeserializeBinary( pData, nBytes ) ;
	//
	esl_free( pData ) ;
	return	fSuccess ;
}

// アイテムへ設定
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferPropertySerializer::PutMeshBufferParameterTo
	( S3DSceneComposer::Parameter& item, size_t iParam )
{
	size_t	nHeaderBytes ;
	size_t	nTotalMeshBytes = SerializeBuffer( nHeaderBytes ) ;
	//
	SArray<uint8_t>	bufTemp ;
	SerializeBinary
		( bufTemp.GetArray( nTotalMeshBytes ),
						nTotalMeshBytes, nHeaderBytes ) ;
	bufTemp.FinishArray() ;
	//
	item.SetBinaryParameter
		( iParam, bufTemp.GetConstArray(), nTotalMeshBytes ) ;
}

// ヘッダ取得
//////////////////////////////////////////////////////////////////////////////
const S3DSceneComposer::BinaryHeader&
	S3DMeshBufferPropertySerializer::GetBinaryHeader( void ) const
{
	return	m_binHeader ;
}

// ヘッダ設定
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferPropertySerializer::SetBinaryType
				( const S3DSceneComposer::BinaryHeader& hdr )
{
	m_binHeader.nType = hdr.nType ;
	m_binHeader.nSubType = hdr.nSubType ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
const SSystem::SString& S3DMeshBufferPropertySerializer::Serialize( void )
{
	if ( m_flagUpdateMesh )
	{
		//
		// 各メッシュのヘッダをシリアライズ
		//
		size_t	nHeaderBytes ;
		size_t	nTotalMeshBytes = SerializeBuffer( nHeaderBytes ) ;
		//
		// バッファを準備
		//
		SArray<uint8_t>	bufTemp ;
		SerializeBinary
			( bufTemp.GetArray( nTotalMeshBytes ),
							nTotalMeshBytes, nHeaderBytes ) ;
		bufTemp.FinishArray() ;
		//
		Charset::EncodeBase64
			( m_strBase64, bufTemp.GetConstArray(), bufTemp.GetLength() ) ;
		m_flagUpdateMesh = false ;
	}
	return	m_strBase64 ;
}

void S3DMeshBufferPropertySerializer::SerializeBinary
		( uint8_t * pbytBinary, size_t nBufBytes, size_t nHeaderBytes )
{
	//
	// バイナリ共通ヘッダ
	//
	S3DSceneComposer::BinaryHeader *	pbh =
		(S3DSceneComposer::BinaryHeader*) pbytBinary ;
	pbh->nType = m_binHeader.nType ;
	pbh->nSubType = m_binHeader.nSubType ;
	pbh->nBodyBytes =
		(uint32_t) (nBufBytes - sizeof(S3DSceneComposer::BinaryHeader)) ;
	pbh->nReserved = 0 ;
	//
	// メッシュデータヘッダ
	//
	S3DSceneComposer::BinaryMeshData *	pbmd =
		(S3DSceneComposer::BinaryMeshData*) pbh->GetBodyPtr() ;
	pbmd->count = (uint32_t) m_aMeshs.GetLength() ;
	//
	uint32_t	addrNextMesh = (uint32_t) nHeaderBytes ;
	for ( size_t i = 0; i < m_aMeshs.GetLength(); i ++ )
	{
		//
		// メッシュエントリ設定
		//
		MeshBuffer *	pMesh = m_aMeshs.GetAt( i ) ;
		ESLAssert( pMesh != NULL ) ;
		if ( pMesh == NULL )
		{
			continue ;
		}
		pbmd->entries[i].addrData = addrNextMesh ;
		pbmd->entries[i].nBytes = (uint32_t) pMesh->nTotalBytes ;
		//
		// メッシュエントリ・ヘッダ複製
		//
		uint8_t *	pbytNext =
				(uint8_t*) pbh->GetBodyPtr() + addrNextMesh ;
		eslCopyMemory
			( pbytNext,
				pMesh->bufHeadData.GetConstArray(),
				pMesh->bufHeadData.GetLength() ) ;
		pbytNext += pMesh->bufHeadData.GetLength() ;
		//
		// 頂点バッファ複製
		//
		ESLAssert( pMesh->bufVertex.GetLength() == pMesh->countVertex ) ;
		eslCopyMemory
			( pbytNext,
				pMesh->bufVertex.GetConstArray(),
				pMesh->bufVertex.GetLength() * sizeof(S3DVector4) ) ;
		pbytNext += pMesh->bufVertex.GetLength() * sizeof(S3DVector4) ;
		ESLAssert( pbytNext <= pbytBinary + nBufBytes ) ;
		//
		// 法線バッファ複製
		//
		if ( pMesh->bufNormal.GetLength() == pMesh->countVertex )
		{
			eslCopyMemory
				( pbytNext,
					pMesh->bufNormal.GetConstArray(),
					pMesh->bufNormal.GetLength() * sizeof(S3DVector4) ) ;
			pbytNext += pMesh->bufNormal.GetLength() * sizeof(S3DVector4) ;
			ESLAssert( pbytNext <= pbytBinary + nBufBytes ) ;
		}
		//
		// UV マップ複製
		//
		if ( pMesh->bufUVMap.GetLength() == pMesh->countVertex )
		{
			eslCopyMemory
				( pbytNext,
					pMesh->bufUVMap.GetConstArray(),
					pMesh->bufUVMap.GetLength() * sizeof(S2DVector) ) ;
			pbytNext += pMesh->bufUVMap.GetLength() * sizeof(S2DVector) ;
			ESLAssert( pbytNext <= pbytBinary + nBufBytes ) ;
		}
		//
		// カラーマップ複製
		//
		if ( pMesh->bufColor.GetLength() == pMesh->countVertex )
		{
			eslCopyMemory
				( pbytNext,
					pMesh->bufColor.GetConstArray(),
					pMesh->bufColor.GetLength() * sizeof(S3DColor) ) ;
			pbytNext += pMesh->bufColor.GetLength() * sizeof(S3DColor) ;
			ESLAssert( pbytNext <= pbytBinary + nBufBytes ) ;
		}
		//
		// 拡張バッファ複製
		//
		for ( size_t j = 0; j < pMesh->m_ssaExBuf.GetLength(); j ++ )
		{
			SArray<uint8_t> *	pData = pMesh->m_ssaExBuf.GetAt( j ) ;
			if ( pData == NULL )
			{
				continue ;
			}
			eslCopyMemory
				( pbytNext, pData->GetConstArray(), pData->GetLength() ) ;
			pbytNext += ((pData->GetLength() + 3) & ~3) ;
			ESLAssert( pbytNext <= pbytBinary + nBufBytes ) ;
		}
		//
		// インデックスバッファ複製
		//
		if ( pMesh->bufIndex.GetLength() == pMesh->countIndex )
		{
			eslCopyMemory
				( pbytNext,
					pMesh->bufIndex.GetConstArray(),
					pMesh->bufIndex.GetLength() * sizeof(uint32_t) ) ;
			pbytNext += pMesh->bufIndex.GetLength() * sizeof(uint32_t) ;
			ESLAssert( pbytNext <= pbytBinary + nBufBytes ) ;
		}
		//
		addrNextMesh += (uint32_t) pMesh->nTotalBytes ;
		ESLAssert( addrNextMesh + sizeof(S3DSceneComposer::BinaryHeader) <= nBufBytes ) ;
	}
}

size_t S3DMeshBufferPropertySerializer::SerializeBuffer( size_t& nHeaderBytes )
{
	size_t	nTotalMeshBytes = 0 ;
	for ( size_t i = 0; i < m_aMeshs.GetLength(); i ++ )
	{
		MeshBuffer *	pMesh = m_aMeshs.GetAt( i ) ;
		ESLAssert( pMesh != NULL ) ;
		if ( (pMesh == NULL) || !(pMesh->flagUpdate) )
		{
			nTotalMeshBytes += pMesh->nTotalBytes ;
			continue ;
		}
		//
		// ヘッダ情報仮設定
		//
		S3DSceneComposer::BinaryPrimitiveData	bpdTemp ;
		bpdTemp.nPrimitiveType = (uint32_t) pMesh->typeMesh ;
		bpdTemp.addrPrimitiveName = 0 ;
		bpdTemp.nPrimitiveCount = pMesh->countPrimitive ;
		bpdTemp.nVertexCount = pMesh->countVertex ;
		bpdTemp.nIndexCount = pMesh->countIndex ;
		bpdTemp.addrIndexBuffer = 0 ;
		bpdTemp.nBufferCount = 1 ;
		bpdTemp.bufEntry[0].nBufferType = S3DSceneComposer::primitiveBufferVertex ;
		bpdTemp.bufEntry[0].addrTypeID = 0 ;
		bpdTemp.bufEntry[0].addrBuffer = 0 ;
		bpdTemp.bufEntry[0].nBufBytes = bpdTemp.nVertexCount * sizeof(S3DVector4) ;
		//
		// 各バッファ・エントリ追加とサイズを計算
		//
		pMesh->bufHeadData.RemoveAll() ;
		pMesh->bufHeadData.AddArray
			( (const uint8_t*) &bpdTemp, sizeof(bpdTemp) ) ;
		//
		uint32_t	addrNext = bpdTemp.nVertexCount * sizeof(S3DVector4) ;
		uint32_t	addrStrNext = 0 ;
		S3DSceneComposer::PrimitiveBufferEntry	pbe ;
		if ( pMesh->bufNormal.GetLength() == bpdTemp.nVertexCount )
		{
			pbe.nBufferType = S3DSceneComposer::primitiveBufferNormal ;
			pbe.addrTypeID = 0 ;
			pbe.addrBuffer = addrNext ;
			pbe.nBufBytes = bpdTemp.nVertexCount * sizeof(S3DVector4) ;
			pMesh->bufHeadData.AddArray
				( (const uint8_t*) &pbe, sizeof(pbe) ) ;
			bpdTemp.nBufferCount ++ ;
			addrNext += bpdTemp.nVertexCount * sizeof(S3DVector4) ;
		}
		if ( pMesh->bufUVMap.GetLength() == bpdTemp.nVertexCount )
		{
			pbe.nBufferType = S3DSceneComposer::primitiveBufferUV ;
			pbe.addrTypeID = 0 ;
			pbe.addrBuffer = addrNext ;
			pbe.nBufBytes = bpdTemp.nVertexCount * sizeof(S2DVector) ;
			pMesh->bufHeadData.AddArray
				( (const uint8_t*) &pbe, sizeof(pbe) ) ;
			bpdTemp.nBufferCount ++ ;
			addrNext += bpdTemp.nVertexCount * sizeof(S2DVector) ;
		}
		if ( pMesh->bufColor.GetLength() == bpdTemp.nVertexCount )
		{
			pbe.nBufferType = S3DSceneComposer::primitiveBufferColor ;
			pbe.addrTypeID = 0 ;
			pbe.addrBuffer = addrNext ;
			pbe.nBufBytes = bpdTemp.nVertexCount * sizeof(S3DColor) ;
			pMesh->bufHeadData.AddArray
				( (const uint8_t*) &pbe, sizeof(pbe) ) ;
			bpdTemp.nBufferCount ++ ;
			addrNext += bpdTemp.nVertexCount * sizeof(S3DColor) ;
		}
		//
		// 拡張バッファ・エントリ追加とを計算
		//
		for ( size_t j = 0; j < pMesh->m_ssaExBuf.GetLength(); j ++ )
		{
			SArray<uint8_t> *	pData = pMesh->m_ssaExBuf.GetAt( j ) ;
			const SString *	pstrTag = pMesh->m_ssaExBuf.GetTagAt( j ) ;
			if ( (pData == NULL) || (pstrTag == NULL) )
			{
				continue ;
			}
			pbe.nBufferType = S3DSceneComposer::primitiveBufferExtension ;
			pbe.addrTypeID = (uint32_t) j ;
			pbe.addrBuffer = addrNext ;
			pbe.nBufBytes = (uint32_t) pData->GetLength() ;
			pMesh->bufHeadData.AddArray
				( (const uint8_t*) &pbe, sizeof(pbe) ) ;
			bpdTemp.nBufferCount ++ ;
			addrNext += (uint32_t) ((pData->GetLength() + 3) & ~3) ;
		}
		//
		// プリミティブ名（文字列）追加
		//
		bpdTemp.addrPrimitiveName = (uint32_t) pMesh->bufHeadData.GetLength() ;
		pMesh->bufHeadData.AddArray
			( (const uint8_t*) pMesh->strName.GetConstArray(),
				pMesh->strName.GetLength() * sizeof(uint16_t) ) ;
		pMesh->bufHeadData.SetLength
			( pMesh->bufHeadData.GetLength() + 2 ) ;
		//
		// 拡張バッファの TypeID （文字列）追加
		//
		S3DSceneComposer::BinaryPrimitiveData *	pbpd ;
		for ( size_t j = 0; j < bpdTemp.nBufferCount; j ++ )
		{
			pbpd = (S3DSceneComposer::BinaryPrimitiveData*)
									pMesh->bufHeadData.GetArray() ;
			if ( pbpd->bufEntry[j].nBufferType
					== S3DSceneComposer::primitiveBufferExtension )
			{
				uint32_t	iExBuf = pbpd->bufEntry[j].addrTypeID ;
				pbpd->bufEntry[j].addrTypeID =
						(uint32_t) pMesh->bufHeadData.GetLength() ;
				pMesh->bufHeadData.FinishArray() ;
				//
				const SString *	pstrTag =
						pMesh->m_ssaExBuf.GetTagAt( (size_t) iExBuf ) ;
				ESLAssert( pstrTag != NULL ) ;
				pMesh->bufHeadData.AddArray
					( (const uint8_t*) pstrTag->GetConstArray(),
						pstrTag->GetLength() * sizeof(uint16_t) ) ;
				pMesh->bufHeadData.SetLength
					( pMesh->bufHeadData.GetLength() + 2 ) ;
			}
			else
			{
				pMesh->bufHeadData.FinishArray() ;
			}
		}
		//
		// 16バイトアライメント
		//
		pMesh->bufHeadData.SetLength
			( (pMesh->bufHeadData.GetLength() + 0x0F) & ~0x0F ) ;
		//
		// インデックスバッファのアドレス確定とサイズ追加
		//
		bpdTemp.addrIndexBuffer =
			(uint32_t) pMesh->bufHeadData.GetLength() + addrNext ;
		addrNext += bpdTemp.nIndexCount * sizeof(uint32_t) ;
		//
		// 各バッファのアドレスを確定
		//
		pbpd = (S3DSceneComposer::BinaryPrimitiveData*)
								pMesh->bufHeadData.GetArray() ;
		*pbpd = bpdTemp ;
		for ( uint32_t j = 0; j < bpdTemp.nBufferCount; j ++ )
		{
			pbpd->bufEntry[j].addrBuffer +=
					(uint32_t) pMesh->bufHeadData.GetLength() ;
		}
		pMesh->bufHeadData.FinishArray() ;
		//
		// 全体サイズ確定
		//
		pMesh->nTotalBytes = pMesh->bufHeadData.GetLength() + addrNext ;
		pMesh->nTotalBytes = (pMesh->nTotalBytes + 0x0F) & ~0x0F ;
		pMesh->flagUpdate = false ;
		nTotalMeshBytes += pMesh->nTotalBytes ;
	}
	//
	// 全体バッファサイズを計算
	//
	nHeaderBytes =
		sizeof(S3DSceneComposer::BinaryMeshData)
			- sizeof(S3DSceneComposer::BinaryPrimitiveEntry)
			+ m_aMeshs.GetLength()
				* sizeof(S3DSceneComposer::BinaryPrimitiveEntry) ;
	nHeaderBytes = (nHeaderBytes + 0x0F) & ~0x0F ;
	//
	return	sizeof(S3DSceneComposer::BinaryHeader)
								+ nTotalMeshBytes + nHeaderBytes ;
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferPropertySerializer::Deserialize( const wchar_t * pwszBase64 )
{
	SArray<uint8_t>	aBinary ;
	Charset::DecodeBase64( aBinary, pwszBase64 ) ;
	//
	DeserializeBinary( aBinary.GetConstArray(), aBinary.GetLength() ) ;
	//
	m_strBase64 = pwszBase64 ;
	m_flagUpdateMesh = true ;
}

bool S3DMeshBufferPropertySerializer::DeserializeBinary
	( const void * pbytBinary, size_t nBytes )
{
	m_aMeshs.RemoveAll() ;
	m_flagUpdateMesh = true ;
	//
	if ( nBytes < sizeof(S3DSceneComposer::BinaryHeader)
						+ sizeof(S3DSceneComposer::BinaryMeshData) )
	{
		return	false ;
	}
	S3DSceneComposer::BinaryHeader *	pbh =
			(S3DSceneComposer::BinaryHeader*) pbytBinary ;
	if ( ((pbh->nType != S3DSceneComposer::binaryMesh)
			&& (pbh->nType != S3DSceneComposer::binaryMeshEditor))
		|| (pbh->nBodyBytes
				+ sizeof(S3DSceneComposer::BinaryHeader) > nBytes) )
	{
		return	false ;
	}
	m_binHeader = *pbh ;
	//
	S3DSceneComposer::BinaryMeshData *	pbmd =
			(S3DSceneComposer::BinaryMeshData*) pbh->GetBodyPtr() ;
	for ( size_t i = 0; i < pbmd->count; i ++ )
	{
		if ( pbmd->entries[i].addrData
				+ pbmd->entries[i].nBytes > pbh->nBodyBytes )
		{
			continue ;
		}
		uint8_t *	pbytMeshData =
			((uint8_t*) pbh->GetBodyPtr()) + pbmd->entries[i].addrData ;
		S3DSceneComposer::BinaryPrimitiveData *
			pbpd = (S3DSceneComposer::BinaryPrimitiveData*) pbytMeshData ;
		//
		MeshBuffer *	pMesh = new MeshBuffer ;
		pMesh->strName =
			(const uint16_t*) (pbytMeshData + pbpd->addrPrimitiveName) ;
		pMesh->typeMesh = (S3DPrimitiveType) pbpd->nPrimitiveType ;
		pMesh->countPrimitive = pbpd->nPrimitiveCount ;
		pMesh->countVertex = pbpd->nVertexCount ;
		pMesh->countIndex = pbpd->nIndexCount ;
		pMesh->bufIndex.AddArray
			( (const uint32_t*)
				(pbytMeshData + pbpd->addrIndexBuffer),
								(size_t) pbpd->nIndexCount ) ;
		//
		for ( uint32_t j = 0; j < pbpd->nBufferCount; j ++ )
		{
			switch ( pbpd->bufEntry[j].nBufferType )
			{
			case	S3DSceneComposer::primitiveBufferVertex:
				pMesh->bufVertex.AddArray
					( (const S3DVector4*)
						(pbytMeshData + pbpd->bufEntry[j].addrBuffer),
						(size_t) pbpd->nVertexCount ) ;
				break ;
			case	S3DSceneComposer::primitiveBufferNormal:
				pMesh->bufNormal.AddArray
					( (const S3DVector4*)
						(pbytMeshData + pbpd->bufEntry[j].addrBuffer),
						(size_t) pbpd->nVertexCount ) ;
				break ;
			case	S3DSceneComposer::primitiveBufferUV:
				pMesh->bufUVMap.AddArray
					( (const S2DVector*)
						(pbytMeshData + pbpd->bufEntry[j].addrBuffer),
						(size_t) pbpd->nVertexCount ) ;
				break ;
			case	S3DSceneComposer::primitiveBufferColor:
				pMesh->bufColor.AddArray
					( (const S3DColor*)
						(pbytMeshData + pbpd->bufEntry[j].addrBuffer),
						(size_t) pbpd->nVertexCount ) ;
				break ;
			case	S3DSceneComposer::primitiveBufferExtension:
				{
					SString	strTypeID = 
						(const uint16_t*)
							(pbytMeshData + pbpd->bufEntry[j].addrTypeID) ;
					//
					SArray<uint8_t> *	pData = new SArray<uint8_t> ;
					pData->AddArray
						( pbytMeshData + pbpd->bufEntry[j].addrBuffer,
											pbpd->bufEntry[j].nBufBytes ) ;
					pMesh->m_ssaExBuf.SetAs( strTypeID, pData ) ;
				}
				break ;
			}
		}
		pMesh->flagUpdate = true ;
		//
		m_aMeshs.Add( pMesh ) ;
	}
	return	true ;
}

// レンダリング
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferPropertySerializer::RenderBuffer( S3DVertexBufferInterface& vb )
{
	for ( size_t i = 0; i < m_aMeshs.GetLength(); i ++ )
	{
		MeshBuffer *	pMesh = m_aMeshs.GetAt( i ) ;
		ESLAssert( pMesh != NULL ) ;
		if ( (pMesh == NULL)
			|| (pMesh->bufVertex.GetLength() != pMesh->countVertex) )
		{
			continue ;
		}
		S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
		vb.AllocatePrimitiveBuffer
			( prmbuf, pMesh->typeMesh, pMesh->countIndex, pMesh->countVertex ) ;
		//
		if ( pMesh->bufVertex.GetLength() == pMesh->countVertex )
		{
			eslCopyMemory
				( prmbuf.pvVertex,
					pMesh->bufVertex.GetConstArray(),
					pMesh->countVertex * sizeof(S3DVector4) ) ;
		}
		if ( pMesh->bufNormal.GetLength() == pMesh->countVertex )
		{
			eslCopyMemory
				( prmbuf.pvNormal,
					pMesh->bufNormal.GetConstArray(),
					pMesh->countVertex * sizeof(S3DVector4) ) ;
		}
		if ( pMesh->bufUVMap.GetLength() == pMesh->countVertex )
		{
			eslCopyMemory
				( prmbuf.pvUVMap,
					pMesh->bufUVMap.GetConstArray(),
					pMesh->countVertex * sizeof(S2DVector) ) ;
		}
		if ( pMesh->bufColor.GetLength() == pMesh->countVertex )
		{
			eslCopyMemory
				( prmbuf.pColor,
					pMesh->bufColor.GetConstArray(),
					pMesh->countVertex * sizeof(S3DColor) ) ;
		}
		if ( pMesh->bufIndex.GetLength() == pMesh->countIndex )
		{
			#if	defined(__DEBUG__)
			const uint32_t *	pSrcIndex = pMesh->bufIndex.GetConstArray() ;
			for ( size_t j = 0; j < pMesh->countIndex; j ++ )
			{
				ESLAssert( pSrcIndex[j] < pMesh->countVertex ) ;
			}
			#endif
			eslCopyMemory
				( prmbuf.pIndexedList,
					pMesh->bufIndex.GetConstArray(),
					pMesh->countIndex * sizeof(uint32_t) ) ;
		}
		//
		vb.AddPrimitiveBuffer
			( NULL, 0, pMesh->typeMesh, prmbuf,
					pMesh->countIndex, pMesh->countVertex ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// メッシュバッファ
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DMeshBufferItemSerializer::m_paramEntries
		[S3DMeshBufferItemSerializer::paramMeshCount] =
{
	{ L"dynamic_mesh",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration,
		L"動的メッシュ", L"メッシュバッファの更新モードを指定します" },
	{ L"material_count",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrDynamicValidation,
		L"マテリアル数", NULL },
	{ L"material",
		S3DSceneComposer::typeCommand,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration,
		L"マテリアル[0]", L"描画に使用するマテリアルを指定します" },
	{ L"mesh_data",
		S3DSceneComposer::typeBinary,
		S3DSceneComposer::attrConstant1,
		L"メッシュ[0]", L"静的なメッシュデータ" },
	{ L"material1",
		S3DSceneComposer::typeCommand,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration,
		L"マテリアル[1]", L"描画に使用するマテリアルを指定します" },
	{ L"mesh_data1",
		S3DSceneComposer::typeBinary,
		S3DSceneComposer::attrConstant1,
		L"メッシュ[1]", L"静的なメッシュデータ" },
	{ L"material2",
		S3DSceneComposer::typeCommand,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration,
		L"マテリアル[2]", L"描画に使用するマテリアルを指定します" },
	{ L"mesh_data2",
		S3DSceneComposer::typeBinary,
		S3DSceneComposer::attrConstant1,
		L"メッシュ[2]", L"静的なメッシュデータ" },
	{ L"material3",
		S3DSceneComposer::typeCommand,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration,
		L"マテリアル[3]", L"描画に使用するマテリアルを指定します" },
	{ L"mesh_data3",
		S3DSceneComposer::typeBinary,
		S3DSceneComposer::attrConstant1,
		L"メッシュ[3]", L"静的なメッシュデータ" },
	{ L"enable_collider",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"当たり判定", L"メッシュを当たり判定として設定します" },
	{ L"collider_classes",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrFlagSetInteger,
		L"衝突フラグ",
		L"当たり判定の対象を判別するためのビット集合を指定する。\n"
		L"システム既定値として 0x01 が形状、0x02 が移動障壁として定義済み。\n"
		L"0x04 は当たり判定（敵）、0x08 は（敵）攻撃当たり判定、0x10 はイベント発生判定として推奨。\n"
		L"0x10～0x80 は未定義の予約領域で、0x0100 以上がユーザー領域である。" },
	{ L"collision_alpha",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1,
		L"当たり判定α閾値",
		L"インスタンスのα値がこの値以上の時に当たり判定を有効にする" },
	{ L"visible_mask",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrFlagSetInteger,
		L"表示マスク",
		L"表示するマテリアルのビットマスクを指定します。" },
	{ L"collider_mask",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrFlagSetInteger,
		L"当たり判定マスク",
		L"当たり判定対象とするマテリアルのビットマスクを指定します。" },
	{ L"enable_instancing",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant2,
		L"複数描画有効", L"インスタンシング描画を行います" },
	{ L"instancing",
		S3DSceneComposer::typeBinary,
		S3DSceneComposer::attrConstant2, L"インスタンス", NULL },
	{ L"instance_rotate",
		S3DSceneComposer::typeRotation,
		S3DSceneComposer::attrConstant2, L"回転", NULL },
	{ L"instance_zoom",
		S3DSceneComposer::typeZoom,
		S3DSceneComposer::attrConstant2, L"拡大", NULL },
	{ L"sorting_method",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant2
		| S3DSceneComposer::attrStringEnumeration, L"ソート", NULL },
	{ L"culling_method",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant2
		| S3DSceneComposer::attrStringEnumeration
		| S3DSceneComposer::attrDynamicValidation, L"カリング", NULL },
	{ L"culling_near_z",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant2, L"カリング最近ｚ", NULL },
	{ L"culling_far_z",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant2, L"カリング最遠ｚ", NULL },
	{ L"culling_offset",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant2, L"カリングオフセット", NULL },
	{ L"culling_angle_gap",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant2,
		L"カリング遊び角", L"カリングを行わない追加の遊び視野角[deg]" },
} ;

const S3DSceneComposer::ParamSetClass
	S3DMeshBufferItemSerializer::m_pscClass =
{
	&ItemBasicSerializer::m_pscClass,
	S3DMeshBufferItemSerializer::paramMeshCount,
	&S3DMeshBufferItemSerializer::m_paramEntries[0]
} ;

const wchar_t *	S3DMeshBufferItemSerializer::m_pwszMeshDynamicsMode
						[S3DMeshBufferItemSerializer::meshModeCount] =
{
	L"static",
	L"updatable",
	L"dynamic",
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DMeshBufferItemSerializer::MeshInterface, ESLObject )
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DMeshBufferItemSerializer::MeshController, Controller, MeshInterface )
SGL_IMPLEMENT_CLASS_INFO3
	( SakuraGL::S3DMeshBufferItemSerializer,
		ItemBasicSerializer, RenderTarget, S3DInstancingItemInterface )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DMeshBufferItemSerializer, mesh_buffer )

// MeshController 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DMeshBufferItemSerializer::MeshController::MeshController( const wchar_t * pwszClassID )
	: Controller( pwszClassID )
{
}

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DMeshBufferItemSerializer::S3DMeshBufferItemSerializer( void )
	: ItemBasicSerializer
		( m_ItemClassDescriptor.pwszClassID,
				&S3DMeshBufferItemSerializer::m_pscClass )
{
	m_flagsBehavior |= S3DScene::itemOwnerBehavior ;
	m_maskClasses |= (1 << S3DScene::classPreRender)
					| (1 << S3DScene::classPreRender2) ;
	//
	m_dynamicsMesh = meshDynamic ;
	m_flagUpdateMesh = true ;
	m_flagCollision = false ;
	m_flagCollisionUpdate = true ;
	m_flagInstancedDraw = false ;
	m_nMaterialCount = 0 ;
	m_maskCollisionFlags = S3DCollision::colliderShape ;
	m_maskVisible = (1 << paramMaterialMaxCount) - 1 ;
	m_maskCollision = (1 << paramMaterialMaxCount) - 1 ;
	m_nCollisionAlpha = 0xFF ;
	m_fpCullingOffset = 0.0 ;
	m_fpCullingAngle = 0.0 ;
	//
	for ( int i = 0; i < paramMaterialMaxCount; i ++ )
	{
		m_pMaterials[i] = NULL ;
	}
	SetMaxMaterialCount( 1 ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DMeshBufferItemSerializer::~S3DMeshBufferItemSerializer( void )
{
}

// 出力先メッシュバッファ取得
//////////////////////////////////////////////////////////////////////////////
S3DVertexBufferInterface **
	S3DMeshBufferItemSerializer::GetTargetVertexBuffers( size_t& nTargetCount )
{
	S3DVertexBufferInterface **
			ppVBs = m_aVBArray.GetArray( (size_t) m_nMaterialCount ) ;
	for ( size_t i = 0; i < m_nMaterialCount; i ++ )
	{
		ppVBs[i] = &( m_aVBMesh.At( i ) ) ;
	}
	m_aVBArray.FinishArray() ;
	nTargetCount = m_nMaterialCount ;
	return	ppVBs ;
}

// 出力先メッシュバッファ取得（動的メッシュバッファの場合のみ）
// （動的メッシュの場合 S3DScene::classPreRender で追加可能）
//////////////////////////////////////////////////////////////////////////////
S3DVertexBufferInterface * S3DMeshBufferItemSerializer::GetVertexBuffer( void )
{
	if ( m_dynamicsMesh == meshDynamic )
	{
		return	&( m_aVBMesh.At( 0 ) ) ;
	}
	return	NULL ;
}

// 静的メッシュの更新フラグ設定
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferItemSerializer::PostUpdateMesh( void )
{
	m_flagUpdateMesh = true ;
}

// マテリアル最大数設定
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferItemSerializer::SetMaxMaterialCount( size_t nCount )
{
	size_t	nLastCount = (size_t) m_nMaterialCount ;
	nCount = (size_t) esl_clampi( (int) nCount, 1, paramMaterialMaxCount ) ;
	m_aMeshData.SetLength( nCount ) ;
	m_aVBMesh.SetLength( nCount ) ;
	m_aVBArray.SetLength( nCount ) ;
	m_nMaterialCount = (uint32_t) nCount ;
	//
	for ( size_t i = nLastCount; i < nCount; i ++ )
	{
		S3DVertexBuffer *	pvb = new S3DVertexBuffer ;
		pvb->SetBufferControlFlags
			( S3DVertexBufferInterface::bufferAutoMerge
				| S3DVertexBufferInterface::bufferKeepDeviceBuffer ) ;
		m_aMeshData.SetAt( i, new S3DMeshBufferPropertySerializer ) ;
		m_aVBMesh.SetAt( i, pvb ) ;
	}
}

size_t S3DMeshBufferItemSerializer::GetMaxMaterialCount( void ) const
{
	return	m_nMaterialCount ;
}

// マテリアル関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferItemSerializer::AttachMaterial
	( S3DMaterial * pMaterial, const wchar_t * pwszMaterialID, size_t iNum )
{
	if ( iNum < m_nMaterialCount )
	{
		ESLAssert( iNum < paramMaterialMaxCount ) ;
		m_pMaterials[iNum] = pMaterial ;
		m_strMaterialIDs[iNum] = pwszMaterialID ;
	}
}

S3DMaterial * S3DMeshBufferItemSerializer::GetMaterialAt( size_t iMaterial ) const
{
	if ( iMaterial < paramMaterialMaxCount )
	{
		return	m_pMaterials[iMaterial] ;
	}
	return	NULL ;
}

const wchar_t * S3DMeshBufferItemSerializer::GetMaterialIDAt( size_t iMaterial ) const
{
	if ( iMaterial < paramMaterialMaxCount )
	{
		return	m_strMaterialIDs[iMaterial] ;
	}
	return	NULL ;
}

// メッシュデータ
//////////////////////////////////////////////////////////////////////////////
S3DMeshBufferPropertySerializer *
	S3DMeshBufferItemSerializer::GetStaticMeshDataAt( size_t iMaterial ) const
{
	return	m_aMeshData.GetAt( iMaterial ) ;
}

// メッシュデータ・レンダリング
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferItemSerializer::RenderToVertexBuffers
	( S3DScene& scene, S3DVertexBufferInterface** ppVBs, size_t nVBCount )
{
	for ( size_t i = 0; (i < m_nMaterialCount) && (i < nVBCount); i ++ )
	{
		m_aMeshData.At(i).RenderBuffer( *(ppVBs[i]) ) ;
	}
	RenderControllersToVertexBuffers( scene, *this, ppVBs, nVBCount ) ;
}

void S3DMeshBufferItemSerializer::RenderControllersToVertexBuffers
	( S3DScene& scene, S3DSceneComposer::ItemSerializer& itemOwner,
					S3DVertexBufferInterface** ppVBs, size_t nVBCount )
{
	size_t	nCtrls = itemOwner.GetControllerCount() ;
	for ( size_t i = 0; i < nCtrls; i ++ )
	{
		S3DSceneComposer::Controller *	pCtrl = itemOwner.GetControllerAt( i ) ;
		MeshInterface *	pSubMesh = ESLTypeCast<MeshInterface>( pCtrl ) ;
		if ( (pSubMesh != nullptr) && !pCtrl->IsControllerDisabled() )
		{
			pSubMesh->AddMesh( scene, &itemOwner, ppVBs, nVBCount ) ;
		}
	}
}

// インスタンシングモード
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshBufferItemSerializer::IsInstancingDraw( void ) const
{
	return	m_flagInstancedDraw ;
}

// マテリアル参照更新
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferItemSerializer::UpdateMaterialRef( void )
{
	S3DSceneComposer *	pComposer = NULL ;
	//
	for ( size_t i = 0; i < m_nMaterialCount; i ++ )
	{
		m_pMaterials[i] = NULL ;
		m_aVBMesh.At(i).AttachDefaultMaterial( NULL ) ;
		//
		if ( m_strMaterialIDs[i].IsEmpty() )
		{
			continue ;
		}
		if ( pComposer == NULL )
		{
			pComposer = GetComposer() ;
			if ( pComposer == NULL )
			{
				continue ;
			}
		}
		m_pMaterials[i] =
			pComposer->GetAssets().
				GetMaterialLibrary().GetMaterialAs( m_strMaterialIDs[i] ) ;
	}
}

// モデルバッファ更新（静的メッシュ）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferItemSerializer::UpdateStaticMesh( void )
{
	if ( (m_dynamicsMesh == meshDynamic) || m_flagUpdateMesh )
	{
		SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
		for ( size_t i = 0; i < m_nMaterialCount; i ++ )
		{
			S3DVertexBuffer&	vb = m_aVBMesh.At(i) ;
			vb.AttachDefaultMaterial( m_pMaterials[i] ) ;
			vb.ClearBuffer() ;
			vb.ResetTransformation() ;
			m_aMeshData.At(i).RenderBuffer( vb ) ;
		}
		m_flagUpdateMesh = false ;
		m_flagCollisionUpdate = true ;
	}
}

// モデルバッファに MeshController メッシュ追加処理
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferItemSerializer::AddControllerMeshs( S3DScene& scene )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	size_t	nVBCount ;
	S3DVertexBufferInterface **
			ppVBs = GetTargetVertexBuffers( nVBCount ) ;
	size_t	nCtrls = GetControllerCount() ;
	for ( size_t i = 0; i < nCtrls; i ++ )
	{
		S3DSceneComposer::Controller *	pCtrl = GetControllerAt( i ) ;
		MeshInterface *	pSubMesh = ESLTypeCast<MeshInterface>( pCtrl ) ;
		if ( (pSubMesh != nullptr) && !pCtrl->IsControllerDisabled() )
		{
			pSubMesh->AddMesh( scene, this, ppVBs, nVBCount ) ;
		}
	}
}

// マテリアルごとのメッシュを統合する
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferItemSerializer::MergeMeshEachMaterials( void )
{
	for ( size_t i = 0; i < m_nMaterialCount; i ++ )
	{
		S3DRenderBuffer::RebuildAsSinglePrimitiveForVB( m_aVBMesh.At(i) ) ;
	}
}

// モデルバッファに MeshController による更新処理
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferItemSerializer::UpdateControllerMeshs( S3DScene& scene )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	size_t	nVBCount ;
	S3DVertexBufferInterface **
			ppVBs = GetTargetVertexBuffers( nVBCount ) ;
	size_t	nCtrls = GetControllerCount() ;
	for ( size_t i = 0; i < nCtrls; i ++ )
	{
		S3DSceneComposer::Controller *	pCtrl = GetControllerAt( i ) ;
		MeshInterface *	pSubMesh = ESLTypeCast<MeshInterface>( pCtrl ) ;
		if ( (pSubMesh != nullptr) && !pCtrl->IsControllerDisabled() )
		{
			pSubMesh->UpdateMesh( scene, this, ppVBs, nVBCount ) ;
		}
	}
}

// 当たり判定の構築
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferItemSerializer::UpdateCollisionBuffer( void )
{
	if ( m_flagCollisionUpdate )
	{
		m_collision.ClearBuffer() ;
		m_collision.AttachMeshUserData( (S3DScene::Item*) this ) ;
		m_collision.SetSceneClassesMask( (1 << m_classItem) | m_maskClasses ) ;
		m_collision.SetUserClassesMask( m_maskCollisionFlags ) ;
		for ( size_t i = 0; i < m_nMaterialCount; i ++ )
		{
			if ( m_maskCollision & (1 << i) )
			{
				m_aVBMesh.At(i).RenderBufferTo( &m_collision ) ;
			}
		}
		m_flagCollisionUpdate = false ;
	}
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DMeshBufferItemSerializer::GetMatrixParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramInstanceRotate:
		return	S3DDMatrix( m_instancing.GetBaseRotation() ) ;
	}
	return	ItemBasicSerializer::GetMatrixParameter( i ) ;
}

S3DDVector S3DMeshBufferItemSerializer::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramInstanceZoom:
		return	m_instancing.GetBaseZoom() ;
	}
	return	ItemBasicSerializer::GetVectorParameter( i ) ;
}

double S3DMeshBufferItemSerializer::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramCullingNearZ:
		return	m_instancing.GetCullingNearZ() ;
	case	paramCullingFarZ:
		return	m_instancing.GetCullingFarZ() ;
	case	paramCullingOffset:
		return	m_fpCullingOffset ;
	case	paramCullingAngleGap:
		return	m_fpCullingAngle ;
	}
	return	ItemBasicSerializer::GetScalarParameter( i ) ;
}

int32_t S3DMeshBufferItemSerializer::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramMaterialCount:
		return	(int32_t) m_nMaterialCount ;
	case	paramColliderFlags:
		return	(int32_t) m_maskCollisionFlags ;
	case	paramCollisionAlpha:
		return	m_nCollisionAlpha ;
	case	paramVisibleMask:
		return	m_maskVisible ;
	case	paramCollisionMask:
		return	m_maskCollision ;
	}
	return	ItemBasicSerializer::GetIntegerParameter( i ) ;
}

bool S3DMeshBufferItemSerializer::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramCollision:
		return	m_flagCollision ;
	case	paramInstancedDraw:
		return	m_flagInstancedDraw ;
	}
	return	ItemBasicSerializer::GetBooleanParameter( i ) ;
}

const wchar_t * S3DMeshBufferItemSerializer::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramDynamicMesh:
		return	m_pwszMeshDynamicsMode[m_dynamicsMesh] ;
	case	paramMaterial0:
		return	m_strMaterialIDs[0] ;
	case	paramMeshData0:
		return	m_aMeshData.At(0).Serialize() ;
	case	paramMaterial1:
		return	m_strMaterialIDs[1] ;
	case	paramMeshData1:
		return	m_aMeshData.At(1).Serialize() ;
	case	paramMaterial2:
		return	m_strMaterialIDs[2] ;
	case	paramMeshData2:
		return	m_aMeshData.At(2).Serialize() ;
	case	paramMaterial3:
		return	m_strMaterialIDs[3] ;
	case	paramMeshData3:
		return	m_aMeshData.At(3).Serialize() ;
	case	paramInstancing:
		return	m_instancing.GetInstancingEntriesBase64() ;
	case	paramSortingMethod:
		return	m_instancing.GetSortingName() ;
	case	paramCullingMethod:
		return	m_instancing.GetCullingName() ;
	}
	return	ItemBasicSerializer::GetCommandParameter( i ) ;
}

size_t S3DMeshBufferItemSerializer::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t i ) const
{
	switch ( i )
	{
	case	paramMeshData0:
	case	paramMeshData1:
	case	paramMeshData2:
	case	paramMeshData3:
		{
			size_t	iMesh = (i - paramMeshData0) / (paramMeshData1 - paramMeshData0) ;
			size_t	nHeaderBytes ;
			size_t	nTotalBytes =
						m_aMeshData.At(iMesh).SerializeBuffer( nHeaderBytes ) ;
			if ( pDst == NULL )
			{
				return	nTotalBytes ;
			}
			if ( nBufBytes == sizeof(S3DSceneComposer::BinaryHeader) )
			{
				S3DSceneComposer::BinaryHeader *
					pbh = (S3DSceneComposer::BinaryHeader*) pDst ;
				pbh->nType = S3DSceneComposer::binaryMesh ;
				pbh->nSubType = 0 ;
				pbh->nBodyBytes =
					(uint32_t) (nTotalBytes - sizeof(S3DSceneComposer::BinaryHeader)) ;
				pbh->nReserved = 0 ;
				return	sizeof(S3DSceneComposer::BinaryHeader) ;
			}
			if ( nBufBytes == nTotalBytes )
			{
				m_aMeshData.At(iMesh).SerializeBinary
						( (uint8_t*) pDst, nTotalBytes, nHeaderBytes ) ;
				return	nTotalBytes ;
			}
		}
		return	0 ;

	case	paramInstancing:
		if ( pDst == NULL )
		{
			return	sizeof(S3DSceneComposer::BinaryHeader)
					+ m_instancing.GetInstancingDataLengthInBytes() ;
		}
		if ( nBufBytes == sizeof(S3DSceneComposer::BinaryHeader) )
		{
			S3DSceneComposer::BinaryHeader *
				pbh = (S3DSceneComposer::BinaryHeader*) pDst ;
			pbh->nType = S3DSceneComposer::binaryInstancing ;
			pbh->nSubType = 0 ;
			pbh->nBodyBytes = (uint32_t) m_instancing.GetInstancingDataLengthInBytes() ;
			pbh->nReserved = 0 ;
			return	sizeof(S3DSceneComposer::BinaryHeader) ;
		}
		if ( nBufBytes == sizeof(S3DSceneComposer::BinaryHeader)
						+ m_instancing.GetInstancingDataLengthInBytes() )
		{
			S3DSceneComposer::BinaryHeader *
				pbh = (S3DSceneComposer::BinaryHeader*) pDst ;
			pbh->nType = S3DSceneComposer::binaryInstancing ;
			pbh->nSubType = 0 ;
			pbh->nBodyBytes = (uint32_t) m_instancing.GetInstancingDataLengthInBytes() ;
			//
			S3DSceneComposer::BinaryInstancingData *	pid =
				(S3DSceneComposer::BinaryInstancingData*) pbh->GetBodyPtr() ;
			return	sizeof(S3DSceneComposer::BinaryHeader)
						+ m_instancing.GetInstancingData( *pid ) ;
		}
		return	0 ;
	}
	return	ItemBasicSerializer::GetBinaryParameter( pDst, nBufBytes, i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferItemSerializer::SetMatrixParameter( size_t i, const S3DDMatrix& mat )
{
	switch ( i )
	{
	case	paramInstanceRotate:
		m_instancing.SetBaseRotation( S3DDQuaternion( mat ) ) ;
		return ;
	}
	ItemBasicSerializer::SetMatrixParameter( i, mat ) ;
}

void S3DMeshBufferItemSerializer::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramInstanceZoom:
		m_instancing.SetBaseZoom( vec ) ;
		return ;
	}
	ItemBasicSerializer::SetVectorParameter( i, vec ) ;
}

void S3DMeshBufferItemSerializer::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramCullingNearZ:
		m_instancing.SetCullingNearZ( (float32_t) s ) ;
		return ;
	case	paramCullingFarZ:
		m_instancing.SetCullingFarZ( (float32_t) s ) ;
		return ;
	case	paramCullingOffset:
		m_fpCullingOffset = s ;
		return ;
	case	paramCullingAngleGap:
		m_fpCullingAngle = s ;
		return ;
	}
	ItemBasicSerializer::SetScalarParameter( i, s ) ;
}

void S3DMeshBufferItemSerializer::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramMaterialCount:
		SetMaxMaterialCount( (size_t) n ) ;
		return ;
	case	paramColliderFlags:
		m_maskCollisionFlags = (uint32_t) n ;
		return ;
	case	paramCollisionAlpha:
		m_nCollisionAlpha = n ;
		return ;
	case	paramVisibleMask:
		m_maskVisible = n ;
		return ;
	case	paramCollisionMask:
		m_maskCollision = n ;
		return ;
	}
	ItemBasicSerializer::SetIntegerParameter( i, n ) ;
}

void S3DMeshBufferItemSerializer::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramCollision:
		m_flagCollision = b ;
//		m_flagsBehavior =
//			(m_flagsBehavior & ~S3DScene::itemCollision)
//				| (b ? S3DScene::itemCollision : 0) ;
		return ;
	case	paramInstancedDraw:
		m_flagInstancedDraw = b ;
		m_flagCollisionUpdate = true ;
		return ;
	}
	ItemBasicSerializer::SetBooleanParameter( i, b ) ;
}

void S3DMeshBufferItemSerializer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	size_t	iMesh ;
	switch ( i )
	{
	case	paramDynamicMesh:
		{
			for ( int j = 0; j < meshModeCount; j ++ )
			{
				if ( SString::Compare( pwszCmd, m_pwszMeshDynamicsMode[j] ) == 0 )
				{
					if ( m_dynamicsMesh != j )
					{
						m_dynamicsMesh = (MeshDynamicsMode) j ;
						m_flagUpdateMesh = true ;
						break ;
					}
				}
			}
		}
		return ;
	case	paramMaterial0:
	case	paramMaterial1:
	case	paramMaterial2:
	case	paramMaterial3:
		iMesh = (i - paramMaterial0) / (paramMaterial1 - paramMaterial0) ;
		if ( m_strMaterialIDs[iMesh] != pwszCmd )
		{
			m_strMaterialIDs[iMesh] = pwszCmd ;
			UpdateMaterialRef() ;
		}
		return ;
	case	paramMeshData0:
	case	paramMeshData1:
	case	paramMeshData2:
	case	paramMeshData3:
		iMesh = (i - paramMeshData0) / (paramMeshData1 - paramMeshData0) ;
		m_aMeshData.At(iMesh).Deserialize( pwszCmd ) ;
		m_flagUpdateMesh = true ;
		m_flagCollisionUpdate = true ;
		return ;
	case	paramInstancing:
		m_instancing.SetInstancingEntriesBase64( pwszCmd ) ;
		m_flagCollisionUpdate = true ;
		return ;
	case	paramSortingMethod:
		m_instancing.SetSortingByName( pwszCmd ) ;
		return ;
	case	paramCullingMethod:
		m_instancing.SetCullingByName( pwszCmd ) ;
		return ;
	}
	ItemBasicSerializer::SetCommandParameter( i, pwszCmd ) ;
}

size_t S3DMeshBufferItemSerializer::SetBinaryParameter
	( size_t i, const void * pSrc, size_t nBufBytes )
{
	size_t	iMesh ;
	switch ( i )
	{
	case	paramMeshData0:
	case	paramMeshData1:
	case	paramMeshData2:
	case	paramMeshData3:
		iMesh = (i - paramMeshData0) / (paramMeshData1 - paramMeshData0) ;
		m_aMeshData.At(iMesh).DeserializeBinary( pSrc, nBufBytes ) ;
		m_flagUpdateMesh = true ;
		m_flagCollisionUpdate = true ;
		return	nBufBytes ;

	case	paramInstancing:
		{
			const S3DSceneComposer::BinaryHeader *
				pbh = (const S3DSceneComposer::BinaryHeader*) pSrc ;
			if ( pbh->nType == S3DSceneComposer::binaryInstancing )
			{
				const S3DSceneComposer::BinaryInstancingData *	pid =
					(const S3DSceneComposer::BinaryInstancingData*) pbh->GetBodyPtr() ;
				m_instancing.SetInstancingEntries( *pid ) ;
				m_flagCollisionUpdate = true ;
				//
				return	sizeof(S3DSceneComposer::BinaryHeader) + pbh->nBodyBytes ;
			}
		}
		return	0 ;
	}
	return	ItemBasicSerializer::SetBinaryParameter( i, pSrc, nBufBytes ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshBufferItemSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	int	j ;
	S3DSceneComposer::Composition *	pComp ;
	switch ( i )
	{
	case	paramDynamicMesh:
		for ( j = 0; j < meshModeCount; j ++ )
		{
			aStrSet.Add( new SString( m_pwszMeshDynamicsMode[j] ) ) ;
		}
		return	true ;
	case	paramMaterial0:
	case	paramMaterial1:
	case	paramMaterial2:
	case	paramMaterial3:
		pComp = GetComposition() ;
		if ( pComp != NULL )
		{
			S3DSceneComposer *	pSceneComp = pComp->GetSceneComposer() ;
			if ( pSceneComp != NULL )
			{
				pSceneComp->Assets().EnumerateMaterialStringSet( aStrSet ) ;
			}
		}
		return	true ;
	case	paramSortingMethod:
		for ( j = 0; j < S3DItemInstancingSerializer::sortMethodCount; j ++ )
		{
			aStrSet.Add( new SString( S3DItemInstancingSerializer::m_pwszSortingMethodName[j] ) ) ;
		}
		return	true ;
	case	paramCullingMethod:
		for ( j = 0; j < S3DItemInstancingSerializer::CullingMethodCount; j ++ )
		{
			aStrSet.Add( new SString( S3DItemInstancingSerializer::m_pwszCullingMethodName[j] ) ) ;
		}
		return	true ;
	}
	return	ItemBasicSerializer::EnumerateStringSet( i, aStrSet ) ;
}

// Loquaty クラス名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DMeshBufferItemSerializer::GetLQClassName( void ) const
{
	return	L"EntisGLS4.SceneMeshBuffer" ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshBufferItemSerializer::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
//	case	paramUseCollision:
//		return	false ;
	case	paramMaterial1:
	case	paramMeshData1:
		return	(m_nMaterialCount >= 2) ;
	case	paramMaterial2:
	case	paramMeshData2:
		return	(m_nMaterialCount >= 3) ;
	case	paramMaterial3:
	case	paramMeshData3:
		return	(m_nMaterialCount >= 4) ;
	}
	return	ItemBasicSerializer::IsParameterValidation( i ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DMeshBufferItemSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	1:
		return	L"メッシュ" ;
	case	2:
		return	L"複数描画" ;
	}
	return	ItemBasicSerializer::GetParameterCategoryName( iCategory ) ;
}

// アイテム作用の追加処理
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferItemSerializer::OnUpdateBehavior( S3DScene& scene )
{
	ItemBasicSerializer::OnUpdateBehavior( scene ) ;
	//
	m_instancing.ResetDynamicInstancingEntries() ;
	m_instancing.UpdateDynamicInstancingEntries( this ) ;
	//
	if ( m_dynamicsMesh == meshDynamic )
	{
		UpdateStaticMesh() ;
	}
	else if ( m_flagUpdateMesh )
	{
		UpdateStaticMesh() ;
		AddControllerMeshs( scene ) ;
		//
		if ( m_dynamicsMesh == meshStatic )
		{
			MergeMeshEachMaterials() ;
		}
	}
}

// コントローラー通知
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferItemSerializer::OnAddController
		( S3DSceneComposer::Controller * pController )
{
	if ( ESLTypeCast<MeshInterface>( pController ) != nullptr )
	{
		m_flagUpdateMesh = true ;
	}
	ItemBasicSerializer::OnAddController( pController ) ;
}

void S3DMeshBufferItemSerializer::BeforeDetachController
		( S3DSceneComposer::Controller * pController )
{
	if ( ESLTypeCast<MeshInterface>( pController ) != nullptr )
	{
		m_flagUpdateMesh = true ;
	}
	ItemBasicSerializer::BeforeDetachController( pController ) ;
}

// アイテムのプライマリモデル取得
//////////////////////////////////////////////////////////////////////////////
S3DVertexBufferInterface * S3DMeshBufferItemSerializer::GetItemPrimaryModel( void )
{
	return	m_aVBMesh.GetAt( 0 ) ;
}

// アイテムのコリジョンバッファ取得
//////////////////////////////////////////////////////////////////////////////
S3DCollider * S3DMeshBufferItemSerializer::GetItemPrimaryCollider( void )
{
	return	&m_collision ;
}

// レンダリングの為のデバイスリソース準備
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferItemSerializer::OnPrepareToRender
				( S3DRenderDevice * pDevice, uint32_t nFlags )
{
	ItemBasicSerializer::OnPrepareToRender( pDevice, nFlags ) ;
	//
	m_instancing.ResetDynamicInstancingEntries() ;
	m_instancing.UpdateDynamicInstancingEntries( this ) ;
	//
	S3DScene *	pScene = GetScene() ;
	ESLAssert( pScene != NULL ) ;
	UpdateStaticMesh() ;
	AddControllerMeshs( *pScene ) ;
	//
	if ( m_dynamicsMesh == meshStatic )
	{
		MergeMeshEachMaterials() ;
	}
	if ( m_flagCollision )
	{
		UpdateCollisionBuffer() ;
	}
	//
	for ( size_t i = 0; i < m_nMaterialCount; i ++ )
	{
		S3DVertexBuffer&	vb = m_aVBMesh.At(i) ;
		pDevice->CommitDeviceVertexBuffer( &vb, 10 ) ;
	}
}

// フレームを適用
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferItemSerializer::SetFrameParameters
		( double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	ItemBasicSerializer::SetFrameParameters( fpFrame, seek ) ;
	//
	if ( m_dynamicsMesh == meshDynamic )
	{
		m_flagCollisionUpdate = true ;
	}
	if ( seek == S3DSceneComposer::seekJumpReset )
	{
		m_flagUpdateMesh = true ;
		//
		m_instancing.ResetDynamicInstancingEntries() ;
		m_instancing.UpdateDynamicInstancingEntries( this ) ;
		m_instancing.NotifyResetPotentialInstance( this ) ;
	}
}

// レンダリング前後処理（全視点共通）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferItemSerializer::OnItemRenderEvent
	( S3DScene& scene, S3DScene::ItemClass clsItem )
{
	ItemBasicSerializer::OnItemRenderEvent( scene, clsItem ) ;
	//
	if ( clsItem == S3DScene::classPreRender )
	{
		if ( m_dynamicsMesh == meshUpdatable )
		{
			UpdateControllerMeshs( scene ) ;
		}
		else if ( m_dynamicsMesh == meshDynamic )
		{
			AddControllerMeshs( scene ) ;
		}
	}
	else if ( clsItem == S3DScene::classPreRender2 )
	{
		m_instancing.ForceFaceDirection( false ) ;
		m_instancing.ResetInstanceIndex() ;
		//
		if ( m_instancing.IsNeededDynamicProcess() )
		{
			S3DDMatrix	matdModel ;
			S3DDVector	vdModel ;
			GetGlobalTransformation( matdModel, vdModel );
			//
			S3DVector	vZoom = m_instancing.GetBaseZoom() ;
			float32_t	zoom = esl_fmaxf( vZoom.x, esl_fmaxf( vZoom.y, vZoom.z ) ) ;
			//
			m_instancing.DoDynamicProcess
				( scene, matdModel, vdModel,
					(float32_t) m_fpCullingOffset,
					(float32_t) (m_fpCullingAngle * PI / 180.0) ) ;
		}
	}
}

// 当たり判定追加
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferItemSerializer::OnItemRenderCollision
	( const S3DScene& scene, S3DCollision& render )
{
	ItemBasicSerializer::OnItemRenderCollision( scene, render ) ;
	//
	if ( m_flagCollision )
	{
		UpdateCollisionBuffer() ;
		//
		render.SetUserClassesMask( m_maskCollisionFlags ) ;
		if ( m_flagInstancedDraw )
		{
			render.BeginBatchBuild() ;

			const S4DMatrix *	pmatInstancing ;
			const S3DColor *	pclrInstancing ;
			size_t	i ;
			size_t	nStaticInstanceCount =
				m_instancing.GetStaticInstancingArray
						( pmatInstancing, pclrInstancing ) ;
			for ( i = 0; i < nStaticInstanceCount; i ++ )
			{
				if ( (int32_t) pclrInstancing[i].rgbMul.argb.Alpha >= m_nCollisionAlpha )
				{
					render.AddColliderObject
						( &m_collision, pmatInstancing + i, i ) ;
				}
			}
			size_t	nDynamicInstanceCount =
				m_instancing.GetDynamicInstancingArray
						( pmatInstancing, pclrInstancing ) ;
			for ( i = 0; i < nDynamicInstanceCount; i ++ )
			{
				if ( (int32_t) pclrInstancing[i].rgbMul.argb.Alpha >= m_nCollisionAlpha )
				{
					render.AddColliderObject
						( &m_collision,
							pmatInstancing + i, nStaticInstanceCount + i ) ;
				}
			}
			render.EndBatchBuild() ;
		}
		else
		{
			render.AddColliderObject( &m_collision, NULL, 0 ) ;
		}
	}
}

// 表示モデル追加
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferItemSerializer::OnItemRenderModel
	( const S3DScene& scene,
		S3DRenderContextInterface& render,
		uint64_t flagsExclusion )
{
	ItemBasicSerializer::OnItemRenderModel( scene, render, flagsExclusion ) ;
	//
	for ( size_t i = 0; i < m_nMaterialCount; i ++ )
	{
		if ( !(m_maskVisible & (1 << i)) )
		{
			continue ;
		}
		S3DMaterial *	pMaterial = m_pMaterials[i] ;
		if ( pMaterial == NULL )
		{
			continue ;
		}
		if ( pMaterial->m_attrSurface.flagsShading & flagsExclusion )
		{
			continue ;
		}
		if ( m_flagInstancedDraw )
		{
			const S4DMatrix *	pmatInstancing ;
			const S3DColor *	pclrInstancing ;
			size_t				nInstanceCount ;
			if ( m_instancing.IsNeededDynamicProcess() )
			{
				nInstanceCount = m_instancing.GetProcessedInstancingArray
											( pmatInstancing, pclrInstancing ) ;
			}
			else
			{
				nInstanceCount =
					m_instancing.GetStaticInstancingArray
							( pmatInstancing, pclrInstancing ) ;
				if ( nInstanceCount > 0 )
				{
					render.AddVertexBuffer
						( pMaterial, 0, m_aVBMesh.GetAt(i), 0, -1,
							nInstanceCount, pmatInstancing, pclrInstancing ) ;
				}
				nInstanceCount =
					m_instancing.GetDynamicInstancingArray
							( pmatInstancing, pclrInstancing ) ;
			}
			if ( nInstanceCount > 0 )
			{
				render.AddVertexBuffer
					( pMaterial, 0, m_aVBMesh.GetAt(i), 0, -1,
						nInstanceCount, pmatInstancing, pclrInstancing ) ;
			}
		}
		else
		{
			render.AddVertexBuffer( pMaterial, 0, m_aVBMesh.GetAt(i) ) ;
		}
	}
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DMeshBufferItemSerializer::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp, uint32_t nFlags )
{
	uint32_t	nResFlags =
		ItemBasicSerializer::UpdatePropertyReference( comp, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefResource )
	{
		UpdateMaterialRef() ;
	}
	return	nResFlags ;
}

// アニメーション長取得
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshBufferItemSerializer::GetTargetAnimationLength( double& secLength ) const
{
	secLength = 0.0 ;
	return	false ;
}

// 全フレーム数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshBufferItemSerializer::GetTargetAnimationFrames( void ) const
{
	return	1 ;
}

// ターゲット空間（逆変換用）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferItemSerializer::GetTargetSpaceTransformation
		( S3DDMatrix& matITarget, S3DDVector& vITarget )
{
	S3DDMatrix	matItem ;
	S3DDVector	vItem ;
	CalcGlobalTransformation( matItem, vItem ) ;
	//
	matITarget.InverseOf( matItem ) ;
	vITarget = matITarget * - vItem ;
}

// パーティクル追加
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferItemSerializer::AddParticles
	( size_t nCount,
		const S3DVector4 * pvPoints,
		const size_t * pFrames,
		const S3DColor * pSrcColors,
		const float32_t * pZooms,
		const S4DVector * pFaceDirs,
		const float32_t * pxAspect )
{
	if ( nCount == 0 )
	{
		return ;
	}
	m_csLock.Lock() ;
	//
	S4DMatrix *	pMatrix = m_aTempMatrixs.GetArray( nCount ) ;
	S3DColor *	pColor = m_aTempColors.GetArray( nCount ) ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		float32_t	zx = 1.0f, zy = 1.0f ;
		if ( pZooms != NULL )
		{
			zx = pZooms[i] ;
			zy = zx ;
		}
		if ( pxAspect != NULL )
		{
			zx *= pxAspect[i] ;
		}
		S3DMatrix	mat3( zx, zy, zy ) ;
		if ( pFaceDirs != NULL )
		{
			mat3.InitializeMatrix( S3DVector( 1, 1, 1 ) ) ;
			mat3.RevolveForAngle( pFaceDirs[i] ) ;
			mat3.RevolveOnZ( sin(pFaceDirs[i].w), cos(pFaceDirs[i].w) ) ;
			mat3.MagnifyByVector( S3DVector( zx, zy, zy ) ) ;
		}
		S4DMatrix	mat4( 1, 1, 1, 1 ) ;
		mat4.SetMatrix3( mat3 ) ;
		mat4.SetTranslation( pvPoints[i] ) ;
		pMatrix[i] = mat4 ;
		//
		S3DColor	color( 0xFFFFFFFF, 0 ) ;
		if ( pSrcColors != NULL )
		{
			color = pSrcColors[i] ;
		}
		pColor[i] = color ;
	}
	m_instancing.AddDynamicInstancingEntries
		( pMatrix, pColor, nCount, (ParameterProperty*) this ) ;
	//
	m_aTempMatrixs.FinishArray() ;
	m_aTempColors.FinishArray() ;
	m_csLock.Unlock() ;
}

// AddIndexedParticles を使うか？
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshBufferItemSerializer::IsUsingIndexedParticles( void )
{
	return	true ;
}

// パーティクル追加（高機能）（classPreRender で呼び出す）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferItemSerializer::AddIndexedParticles
	( size_t nCount,
		const S3DVector4 * pvPoints,
		const S3DParticleSerializer::ParticleIndex * pIndexes,
		const size_t * pFrames,
		const S3DColor * pSrcColors,
		const S3DMatrix * pFaceDirs )
{
	if ( nCount == 0 )
	{
		return ;
	}
	m_csLock.Lock() ;
	//
	S4DMatrix *	pMatrix = m_aTempMatrixs.GetArray( nCount ) ;
	S3DColor *	pColor = m_aTempColors.GetArray( nCount ) ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		S4DMatrix	mat4( 1, 1, 1, 1 ) ;
		if ( pFaceDirs != NULL )
		{
			mat4.SetMatrix3( pFaceDirs[i] ) ;
		}
		mat4.SetTranslation( pvPoints[pIndexes[i].nIndex] ) ;
		pMatrix[i] = mat4 ;
		//
		S3DColor	color( 0xFFFFFFFF, 0 ) ;
		if ( pSrcColors != NULL )
		{
			color = pSrcColors[i] ;
		}
		pColor[i] = color ;
	}
	m_instancing.AddDynamicInstancingEntries
		( pMatrix, pColor, nCount, (ParameterProperty*) this ) ;
	//
	m_aTempMatrixs.FinishArray() ;
	m_aTempColors.FinishArray() ;
	m_csLock.Unlock() ;
}

// インスタンス処理（classPreRender で呼び出す）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshBufferItemSerializer::AddDynamicInstancingEntries
	( const S4DMatrix * pMatrixs,
		const S3DColor * pColors, size_t nCount, ESLObject * pSrcItem )
{
	m_csLock.Lock() ;
	m_instancing.AddDynamicInstancingEntries
				( pMatrixs, pColors, nCount, pSrcItem ) ;
	m_csLock.Unlock() ;
}

// S3DItemInstancingSerializer 取得
//////////////////////////////////////////////////////////////////////////////
S3DItemInstancingSerializer * S3DMeshBufferItemSerializer::GetInstancing( void )
{
	return	&m_instancing ;
}



//////////////////////////////////////////////////////////////////////////////
// メッシュ出力中継アイテム
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DIndirectMeshBuilderSerializer::m_paramEntries
		[S3DIndirectMeshBuilderSerializer::paramBuilderCount] =
{
	{ L"mesh_target",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration,
		L"メッシュ出力先", NULL },
} ;

const S3DSceneComposer::ParamSetClass
	S3DIndirectMeshBuilderSerializer::m_pscClass =
{
	&ItemBasicSerializer::m_pscClass,
	S3DIndirectMeshBuilderSerializer::paramBuilderCount,
	&S3DIndirectMeshBuilderSerializer::m_paramEntries[0]
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DIndirectMeshBuilderSerializer, ItemBasicSerializer )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DIndirectMeshBuilderSerializer, mesh_indirect )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DIndirectMeshBuilderSerializer::S3DIndirectMeshBuilderSerializer( void )
	: ItemBasicSerializer( m_ItemClassDescriptor.pwszClassID, &m_pscClass )
{
	m_classItem = S3DScene::classPreRender ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DIndirectMeshBuilderSerializer::~S3DIndirectMeshBuilderSerializer( void )
{
}

// 出力先設定
//////////////////////////////////////////////////////////////////////////////
void S3DIndirectMeshBuilderSerializer::AttachMeshTarget
	( S3DMeshBufferItemSerializer * pMesh, const wchar_t * pwszMeshID )
{
	m_refMeshTarget.SetReference( (S3DScene::Item*) pMesh ) ;
	m_strMeshTarget = pwszMeshID ;
}

bool S3DIndirectMeshBuilderSerializer::UpdateMeshTarget( void )
{
	if ( m_strMeshTarget.IsEmpty() )
	{
		m_refMeshTarget.SetReference( NULL ) ;
		return	true ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != NULL )
	{
		S3DSceneComposer::ItemSerializer *
			pItem = pComp->GetSceneItemAs( m_strMeshTarget ) ;
		if ( pItem != NULL )
		{
			m_refMeshTarget.SetReference( pItem ) ;
			return	true ;
		}
	}
	return	false ;
}

// 出力先取得
//////////////////////////////////////////////////////////////////////////////
S3DMeshBufferItemSerializer *
	S3DIndirectMeshBuilderSerializer::GetMeshTarget( void ) const
{
	return	ESLTypeCast<S3DMeshBufferItemSerializer>
						( m_refMeshTarget.GetReference() ) ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DIndirectMeshBuilderSerializer::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramMeshTarget:
		return	m_strMeshTarget ;
	}
	return	ItemBasicSerializer::GetCommandParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DIndirectMeshBuilderSerializer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramMeshTarget:
		if ( m_strMeshTarget != pwszCmd )
		{
			m_strMeshTarget = pwszCmd ;
			UpdateMeshTarget() ;
		}
		return ;
	}
	ItemBasicSerializer::SetCommandParameter( i, pwszCmd ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DIndirectMeshBuilderSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	switch ( i )
	{
	case	paramMeshTarget:
		{
			S3DSceneComposer::Composition *	pComp = GetComposition() ;
			if ( pComp != NULL )
			{
				pComp->EnumerateItemIDsAs
					( aStrSet, ESL_RUNTIME_CLASS(S3DMeshBufferItemSerializer) ) ;
			}
		}
		return	true ;
	}
	return	ItemBasicSerializer::EnumerateStringSet( i, aStrSet ) ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DIndirectMeshBuilderSerializer::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramTransparency:
	case	paramColorMul:
	case	paramColorAdd:
	case	paramForceToon:
	case	paramForceBorder:
	case	paramFreeToon:
	case	paramFreeBorder:
	case	paramUseCollision:
	case	paramHideNear:
	case	paramHideFar:
	case	paramItemClass:
		return	false ;
	}
	return	ItemBasicSerializer::IsParameterValidation( i ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DIndirectMeshBuilderSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"メッシュ出力" ;
	}
	return	NULL ;
}

// レンダリング前後処理（全視点共通）
//////////////////////////////////////////////////////////////////////////////
void S3DIndirectMeshBuilderSerializer::OnItemRenderEvent
	( S3DScene& scene, S3DScene::ItemClass clsItem )
{
	if ( (clsItem == S3DScene::classPreRender)
		&& (m_flagsBehavior & S3DScene::itemVisible))
	{
		S3DMeshBufferItemSerializer *	pMesh =
			ESLTypeCast<S3DMeshBufferItemSerializer>
						( m_refMeshTarget.GetReference() ) ;
		if ( pMesh != NULL )
		{
			SSmartLock<const SCriticalSection>	lock( pMesh->GetInstanceLocker() ) ;
			size_t						nVBCount ;
			S3DVertexBufferInterface **	ppVBs = pMesh->GetTargetVertexBuffers( nVBCount ) ;
			if ( ppVBs != NULL )
			{
				S3DDMatrix	matOffset ;
				S3DDVector	vOffset ;
				pMesh->GetTransformationFrom( matOffset, vOffset, this ) ;
				//
				S3DColor		clrEffect ;
				unsigned int	nTransparency ;
				GetGlobalColorEffect( clrEffect ) ;
				nTransparency =
					0x100 - (clrEffect.rgbMul.argb.Alpha
								+ (clrEffect.rgbMul.argb.Alpha >> 7)) ;
				//
				for ( size_t i = 0; i < nVBCount; i ++ )
				{
					ppVBs[i]->PushTransformation() ;
					ppVBs[i]->SetMatrixTransformation
						( matOffset, vOffset, &clrEffect, nTransparency ) ;
				}
				//
				size_t	nCount = m_arrControllers.GetLength() ;
				S3DSceneComposer::Controller *const*
						ppControllers = m_arrControllers.GetConstArray() ;
				for ( size_t i = 0; i < nCount; i ++ )
				{
					S3DMeshBufferItemSerializer::MeshInterface *	pMeshCtrl =
						ESLTypeCast<S3DMeshBufferItemSerializer::MeshInterface>( ppControllers[i] ) ;
					if ( (pMeshCtrl != nullptr)
						&& !ppControllers[i]->IsControllerDisabled() )
					{
						pMeshCtrl->AddMesh( scene, pMesh, ppVBs, nVBCount ) ;
					}
				}
				//
				for ( size_t i = 0; i < nVBCount; i ++ )
				{
					ppVBs[i]->PopTransformation() ;
				}
			}
		}
	}
	ItemBasicSerializer::OnItemRenderEvent( scene, clsItem ) ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DIndirectMeshBuilderSerializer::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp, uint32_t nFlags )
{
	uint32_t	nResFlags =
		ItemBasicSerializer::UpdatePropertyReference( comp, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefItem )
	{
		if ( !UpdateMeshTarget() )
		{
			nResFlags |= S3DSceneComposer::updateRefItem ;
		}
	}
	return	nResFlags ;
}



//////////////////////////////////////////////////////////////////////////////
// 多角形・多面体メッシュ生成
//////////////////////////////////////////////////////////////////////////////

const wchar_t *	S3DPolyhedronMeshController::m_pwszShapeType
					[S3DPolyhedronMeshController::shapeTypeCount] =
{
	L"triangle",
	L"squeare",
	L"pentagon",
	L"hexagon",
	L"star",
	L"tetrahedron",
	L"hexahedron",
	L"octahedron",
	L"dodecahedron",
	L"icosahedron",
	L"truncated_icosahedron",
	L"icosahedron_x4",
	L"truncated_icosahedron_x4",
} ;

const wchar_t *	S3DPolyhedronMeshController::m_pwszUVProjection
					[S3DPolyhedronMeshController::uvTypeCount] =
{
	L"orthogonal",
	L"cube",
} ;

const wchar_t *	S3DPolyhedronMeshController::m_pwszBevelType
					[S3DPolyhedronMeshController::bevelTypeCount] =
{
	L"no", L"flat", L"curved", L"cut_off",
} ;

const S3DPolyhedronMeshController::PFUNC_ADD_MESH
	S3DPolyhedronMeshController::m_pfnAddMesh
		[S3DPolyhedronMeshController::shapeTypeCount] =
{
	&S3DPolyhedronMeshController::AddMeshTriangle,
	&S3DPolyhedronMeshController::AddMeshSqueare,
	&S3DPolyhedronMeshController::AddMeshPentagon,
	&S3DPolyhedronMeshController::AddMeshHexagon,
	&S3DPolyhedronMeshController::AddMeshStar,
	&S3DPolyhedronMeshController::AddMeshTetrahedron,
	&S3DPolyhedronMeshController::AddMeshHexahedron,
	&S3DPolyhedronMeshController::AddMeshOctahedron,
	&S3DPolyhedronMeshController::AddMeshDodecahedron,
	&S3DPolyhedronMeshController::AddMeshIcosahedron,
	&S3DPolyhedronMeshController::AddMeshTruncatedIcosahedron,
	&S3DPolyhedronMeshController::AddMeshIcosahedronX4,
	&S3DPolyhedronMeshController::AddMeshTruncatedIcosahedronX4,
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DPolyhedronMeshController, MeshController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DPolyhedronMeshController, polyhedron )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DPolyhedronMeshController::S3DPolyhedronMeshController( void )
	: MeshController( m_ItemClassDescriptor.pwszClassID ),
		m_shape( shapeHexahedron ),
		m_uvPorj( uvOrthogonal ),
		m_bevel( bevelNo ),
		m_flagBackFace( false ),
		m_vPosition( 0, 0, 0 ),
		m_vSize( 1, 1, 1 ),
		m_rectAtlasRef( 0, 0, 1, 1 ),
		m_vUVPosition( 0, 0 ),
		m_vUVScale( 1, 1 ),
		m_matRotation( 1, 1, 1 ),
		m_fpBevel( 0.1 ), m_fpBevelDepth( 1.0 ),
		m_fpSwelling( 0.0 ), m_nSwellDiv( 1 ),
		m_nIcosahedronDiv( 0 ),
		m_flagTrianglePolyhedron( false ), m_flagPolyhedronTruncated( false )
{
	PrepareParameterEntryCount( paramCount ) ;
	AddParameterEntry
		( L"shape_type", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration,
			L"形状", NULL ) ;
	AddParameterEntry
		( L"uv_projection", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration,
			L"UV投影", NULL ) ;
	AddParameterEntry
		( L"back_face", S3DSceneComposer::typeBoolean, 0,
			L"面反転", NULL ) ;
	AddParameterEntry
		( L"position", S3DSceneComposer::typePosition,
			S3DSceneComposer::attrNoLocalTransform,
			L"位置", NULL ) ;
	AddParameterEntry
		( L"rotation", S3DSceneComposer::typeRotation, 0,
			L"回転", NULL ) ;
	AddParameterEntry
		( L"zoom", S3DSceneComposer::typeZoom, 0,
			L"サイズ", NULL ) ;
	AddParameterEntry
		( L"uv_atlas", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration,
			L"アトラス画像参照", NULL ) ;
	AddParameterEntry
		( L"uv_position", S3DSceneComposer::typeVector2, 0,
			L"UV位置", NULL ) ;
	AddParameterEntry
		( L"uv_scale", S3DSceneComposer::typeVector2, 0,
			L"UVスケール", NULL ) ;
	AddParameterEntry
		( L"bevel_type", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration,
			L"ベベル", NULL ) ;
	AddParameterEntry
		( L"bevel_size", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"ベベルサイズ", NULL, 0.0, 1.0 ) ;
	AddParameterEntry
		( L"bevel_depth", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"ベベル切込係数", NULL, 0.0, 2.0 ) ;
	AddParameterEntry
		( L"swelling", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"面膨らみ", NULL, -1.0, 1.0 ) ;
	AddParameterEntry
		( L"swell_div", S3DSceneComposer::typeInteger, 0,
			L"面分割", NULL ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DPolyhedronMeshController::~S3DPolyhedronMeshController( void )
{
}

// アトラス画像参照領域更新
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMeshController::UpdateRefAtlasRect( void )
{
	if ( m_strUVAtlas.IsEmpty() )
	{
		m_rectAtlasRef.x = 0 ;
		m_rectAtlasRef.y = 0 ;
		m_rectAtlasRef.w = 1 ;
		m_rectAtlasRef.h = 1 ;
		return ;
	}
	S3DSceneComposer *	pComposer = GetComposer() ;
	if ( pComposer != NULL )
	{
		SGLImageObject *	pImage =
			pComposer->Assets().GetImageAs( m_strUVAtlas ) ;
		if ( pImage != NULL )
		{
			if ( pImage->GetReferenceRectOfAtlas( m_rectAtlasRef ) )
			{
				m_rectAtlasRef.x = 0 ;
				m_rectAtlasRef.y = 0 ;
				m_rectAtlasRef.w = 1 ;
				m_rectAtlasRef.h = 1 ;
			}
		}
	}
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DPolyhedronMeshController::GetMatrixParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRotation:
		return	m_matRotation ;
	}
	return	S3DDMatrix( 1, 1, 1 ) ;
}

S3DDVector S3DPolyhedronMeshController::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramPosition:
		return	m_vPosition ;
	case	paramSize:
		return	m_vSize ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DPolyhedronMeshController::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramBevelSize:
		return	m_fpBevel ;
	case	paramBevelDepth:
		return	m_fpBevelDepth ;
	case	paramSwelling:
		return	m_fpSwelling ;
	}
	return	0 ;
}

int32_t S3DPolyhedronMeshController::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramSwellDiv:
		return	m_nSwellDiv ;
	}
	return	0 ;
}

bool S3DPolyhedronMeshController::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramBackFace:
		return	m_flagBackFace ;
	}
	return	false ;
}

const wchar_t * S3DPolyhedronMeshController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramShapeType:
		return	m_pwszShapeType[m_shape] ;
	case	paramUVAtlas:
		return	m_strUVAtlas ;
	case	paramUVProjection:
		return	m_pwszUVProjection[m_uvPorj] ;
	case	paramBevelType:
		return	m_pwszBevelType[m_bevel] ;
	}
	return	NULL ;
}

size_t S3DPolyhedronMeshController::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t i ) const
{
	switch ( i )
	{
	case	paramUVPosition:
		if ( nBufBytes == 0 )
		{
			return	sizeof(S2DDVector) ;
		}
		if ( nBufBytes == sizeof(S2DDVector) )
		{
			*((S2DDVector*)pDst) = m_vUVPosition ;
			return	sizeof(S2DDVector) ;
		}
		break ;
	case	paramUVScale:
		if ( nBufBytes == 0 )
		{
			return	sizeof(S2DDVector) ;
		}
		if ( nBufBytes == sizeof(S2DDVector) )
		{
			*((S2DDVector*)pDst) = m_vUVScale ;
			return	sizeof(S2DDVector) ;
		}
		break ;
	}
	return	0 ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMeshController::SetMatrixParameter( size_t i, const S3DDMatrix& mat )
{
	switch ( i )
	{
	case	paramRotation:
		m_matRotation = mat ;
		return ;
	}
}

void S3DPolyhedronMeshController::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramPosition:
		m_vPosition = vec ;
		return ;
	case	paramSize:
		m_vSize = vec ;
		return ;
	}
}

void S3DPolyhedronMeshController::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramBevelSize:
		m_fpBevel = s ;
		return ;
	case	paramBevelDepth:
		m_fpBevelDepth = s ;
		return ;
	case	paramSwelling:
		m_fpSwelling = s ;
		return ;
	}
}

void S3DPolyhedronMeshController::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramSwellDiv:
		m_nSwellDiv = n ;
		return ;
	}
}

void S3DPolyhedronMeshController::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramBackFace:
		m_flagBackFace= b ;
		return ;
	}
}

void S3DPolyhedronMeshController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	size_t	j ;
	switch ( i )
	{
	case	paramShapeType:
		for ( j = 0; j < shapeTypeCount; j ++ )
		{
			if ( SString::Compare( pwszCmd, m_pwszShapeType[j] ) == 0 )
			{
				m_shape = (ShapeType) j ;
				break ;
			}
		}
		return ;

	case	paramUVProjection:
		for ( j = 0; j < uvTypeCount; j ++ )
		{
			if ( SString::Compare( pwszCmd, m_pwszUVProjection[j] ) == 0 )
			{
				m_uvPorj = (UVProjectionType) j ;
				break ;
			}
		}
		return ;

	case	paramUVAtlas:
		if ( m_strUVAtlas != pwszCmd )
		{
			m_strUVAtlas = pwszCmd ;
			UpdateRefAtlasRect() ;
		}
		return ;

	case	paramBevelType:
		for ( j = 0; j < bevelTypeCount; j ++ )
		{
			if ( SString::Compare( pwszCmd, m_pwszBevelType[j] ) == 0 )
			{
				m_bevel = (BevelType) j ;
				break ;
			}
		}
		return ;
	}
}

size_t S3DPolyhedronMeshController::SetBinaryParameter
	( size_t i, const void * pSrc, size_t nBufBytes )
{
	switch ( i )
	{
	case	paramUVPosition:
		if ( nBufBytes == sizeof(S2DDVector) )
		{
			m_vUVPosition = *((S2DDVector*)pSrc) ;
			return	sizeof(S2DDVector) ;
		}
		break ;
	case	paramUVScale:
		if ( nBufBytes == sizeof(S2DDVector) )
		{
			m_vUVScale = *((S2DDVector*)pSrc) ;
			return	sizeof(S2DDVector) ;
		}
		break ;
	}
	return	0 ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DPolyhedronMeshController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	size_t	j ;
	switch ( i )
	{
	case	paramShapeType:
		for ( j = 0; j < shapeTypeCount; j ++ )
		{
			aStrSet.Add( new SString( m_pwszShapeType[j] ) ) ;
		}
		return	true ;

	case	paramUVAtlas:
		{
			S3DSceneComposer *	pComp = GetComposer() ;
			if ( pComp != NULL )
			{
				pComp->Assets().EnumerateResourceIDsAs
					( aStrSet, ESL_RUNTIME_CLASS(SGLImageObject) ) ;
			}
		}
		return	true ;

	case	paramUVProjection:
		for ( j = 0; j < uvTypeCount; j ++ )
		{
			aStrSet.Add( new SString( m_pwszUVProjection[j] ) ) ;
		}
		return	true ;

	case	paramBevelType:
		for ( j = 0; j < bevelTypeCount; j ++ )
		{
			aStrSet.Add( new SString( m_pwszBevelType[j] ) ) ;
		}
		return	true ;
	}
	return	false ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DPolyhedronMeshController::GetParameterCategoryName( size_t iCategory ) const
{
	return	L"多角形・多面体" ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DPolyhedronMeshController::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags )
{
	uint32_t	nResFlags =
		MeshController::UpdatePropertyReference( comp, pItem, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefResource )
	{
		UpdateRefAtlasRect() ;
	}
	//
	return	nResFlags ;
}

// メッシュ追加処理（全視点・ビュー共通処理）
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMeshController::AddMesh
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem,
		S3DVertexBufferInterface ** ppVBs, size_t nVBCount )
{
	if ( nVBCount == 0 )
	{
		return ;
	}
	m_matMeshSpace = m_matRotation
						* S3DDMatrix( m_vSize.x, m_vSize.y, m_vSize.z ) ;
	(this->*m_pfnAddMesh[m_shape])( *(ppVBs[0]) ) ;
}

// フレーム描画前処理（全視点・ビュー共通処理）
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMeshController::UpdateMesh
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem,
		S3DVertexBufferInterface ** ppVBs, size_t nVBCount )
{
}

// 正三角形
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMeshController::AddMeshTriangle( S3DVertexBufferInterface& vb )
{
	S3DVector	vVertex[3] ;
	S3DVector	vNormal[3] ;
	for ( int i = 0; i < 3; i ++ )
	{
		double	rad = PI * 2.0 * i / 3.0 ;
		vVertex[i].x = (float32_t) sin( rad ) * 0.5f ;
		vVertex[i].y = 0 ;
		vVertex[i].z = (float32_t) cos( rad ) * 0.5f ;
		//
		vNormal[i].x = 0 ;
		vNormal[i].y = -1 ;
		vNormal[i].z = 0 ;
	}
	AddMeshPlane( vb, 3, vVertex, vNormal, vNormal[0], (float32_t) cos(PI/6.0) ) ;
}

// 正四角形
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMeshController::AddMeshSqueare( S3DVertexBufferInterface& vb )
{
	S3DVector	vVertex[4] ;
	S3DVector	vNormal[4] ;
	float32_t	x = 1.0f ;
	float32_t	z = 1.0f ;
	for ( int i = 0; i < 4; i ++ )
	{
		vVertex[i].x = x ;
		vVertex[i].y = 0 ;
		vVertex[i].z = z ;
		//
		vNormal[i].x = 0 ;
		vNormal[i].y = -1 ;
		vNormal[i].z = 0 ;
		//
		float32_t	x0 = x ;
		float32_t	z0 = z ;
		z = - x0 ;
		x = z0 ;
	}
	AddMeshPlane( vb, 4, vVertex, vNormal, vNormal[0], (float32_t) cos(PI/4.0) ) ;
}

// 正五角形
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMeshController::AddMeshPentagon( S3DVertexBufferInterface& vb )
{
	S3DVector	vVertex[5] ;
	S3DVector	vNormal[5] ;
	for ( int i = 0; i < 5; i ++ )
	{
		double	rad = PI * 2.0 * i / 5.0 ;
		vVertex[i].x = (float32_t) sin( rad ) * 0.5f ;
		vVertex[i].y = 0 ;
		vVertex[i].z = (float32_t) cos( rad ) * 0.5f ;
		//
		vNormal[i].x = 0 ;
		vNormal[i].y = -1 ;
		vNormal[i].z = 0 ;
	}
	AddMeshPlane( vb, 5, vVertex, vNormal, vNormal[0], (float32_t) cos(PI*3.0/10.0) ) ;
}

// 正六角形
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMeshController::AddMeshHexagon( S3DVertexBufferInterface& vb )
{
	S3DVector	vVertex[6] ;
	S3DVector	vNormal[6] ;
	for ( int i = 0; i < 6; i ++ )
	{
		double	rad = PI * 2.0 * i / 6.0 ;
		vVertex[i].x = (float32_t) sin( rad ) * 0.5f ;
		vVertex[i].y = 0 ;
		vVertex[i].z = (float32_t) cos( rad ) * 0.5f ;
		//
		vNormal[i].x = 0 ;
		vNormal[i].y = -1 ;
		vNormal[i].z = 0 ;
	}
	AddMeshPlane( vb, 6, vVertex, vNormal, vNormal[0], (float32_t) cos(PI/3.0) ) ;
}

// 星型
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMeshController::AddMeshStar( S3DVertexBufferInterface& vb )
{
	S3DVector	vVertex[10] ;
	S3DVector	vNormal[10] ;
	for ( int i = 0; i < 10; i ++ )
	{
		double		rad = PI * 2.0 * i / 10.0 ;
		float32_t	r = (i & 1) ? 0.25f : 0.5f ;
		vVertex[i].x = (float32_t) sin( rad ) * r ;
		vVertex[i].y = 0 ;
		vVertex[i].z = (float32_t) cos( rad ) * r ;
		//
		vNormal[i].x = 0 ;
		vNormal[i].y = -1 ;
		vNormal[i].z = 0 ;
	}
	AddMeshPlane( vb, 10, vVertex, vNormal, vNormal[0], 0.0f ) ;
}

// 正四面体
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMeshController::AddMeshTetrahedron( S3DVertexBufferInterface& vb )
{
	S3DVector	vVertex[4] =
	{
		S3DVector( 1, 1, 1 ),
		S3DVector( 1, -1, -1 ),
		S3DVector( -1, 1, -1 ),
		S3DVector( -1, -1, 1 ),
	} ;
	size_t	nIndex[4][3] =
	{
		{ 0, 2, 1 },
		{ 0, 1, 3 },
		{ 1, 2, 3 },
		{ 2, 0, 3 },
	} ;
	for ( int i = 0; i < 4; i ++ )
	{
		S3DVector	vTriangle[3] ;
		S3DVector	vNormal[3] ;
		for ( int j = 0; j < 3; j ++ )
		{
			size_t	k = nIndex[i][j] ;
			vTriangle[j] = vVertex[k] ;
			vNormal[j] = vTriangle[j] ;
			vNormal[j].Normalize() ;
		}
		S3DVector	vPlane = (vTriangle[2] - vTriangle[0])
								* (vTriangle[1] - vTriangle[0]) ;
		vPlane.Normalize() ;
		//
		AddMeshPlane( vb, 3, vTriangle, vNormal, vPlane, (float32_t) cos(PI/6.0) ) ;
	}
}

// 立方体
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMeshController::AddMeshHexahedron( S3DVertexBufferInterface& vb )
{
	S3DVector	vVertex[8] =
	{
		S3DVector( 1, -1, 1 ),
		S3DVector( 1, -1, -1 ),
		S3DVector( -1, -1, -1 ),
		S3DVector( -1, -1, 1 ),
		S3DVector( 1, 1, 1 ),
		S3DVector( 1, 1, -1 ),
		S3DVector( -1, 1, -1 ),
		S3DVector( -1, 1, 1 ),
	} ;
	size_t	nIndex[6][4] =
	{
		{ 0, 1, 2, 3 },
		{ 0, 3, 7, 4 },
		{ 1, 0, 4, 5 },
		{ 2, 1, 5, 6 },
		{ 3, 2, 6, 7 },
		{ 7, 6, 5, 4 },
	} ;
	for ( int i = 0; i < 6; i ++ )
	{
		S3DVector	vSqueare[4] ;
		S3DVector	vNormal[4] ;
		for ( int j = 0; j < 4; j ++ )
		{
			size_t	k = nIndex[i][j] ;
			vSqueare[j] = vVertex[k] ;
			vNormal[j] = vSqueare[j] ;
			vNormal[j].Normalize() ;
		}
		S3DVector	vPlane = (vSqueare[2] - vSqueare[0])
								* (vSqueare[1] - vSqueare[0]) ;
		vPlane.Normalize() ;
		//
		AddMeshPlane( vb, 4, vSqueare, vNormal, vPlane, (float32_t) cos(PI/4.0) ) ;
	}
}

// 正八面体
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMeshController::AddMeshOctahedron( S3DVertexBufferInterface& vb )
{
	S3DVector	vVertex[6] =
	{
		S3DVector( 0, -1, 0 ),
		S3DVector( 1, 0, 0 ),
		S3DVector( 0, 0, -1 ),
		S3DVector( -1, 0, 0 ),
		S3DVector( 0, 0, 1 ),
		S3DVector( 0, 1, 0 ),
	} ;
	size_t	nIndex[8][3] =
	{
		{ 0, 1, 2 },
		{ 0, 2, 3 },
		{ 0, 3, 4 },
		{ 0, 4, 1 },
		{ 2, 1, 5 },
		{ 3, 2, 5 },
		{ 4, 3, 5 },
		{ 1, 4, 5 },
	} ;
	for ( int i = 0; i < 8; i ++ )
	{
		S3DVector	vTriangle[3] ;
		S3DVector	vNormal[3] ;
		for ( int j = 0; j < 3; j ++ )
		{
			size_t	k = nIndex[i][j] ;
			vTriangle[j] = vVertex[k] ;
			vNormal[j] = vTriangle[j] ;
			vNormal[j].Normalize() ;
		}
		S3DVector	vPlane = (vTriangle[2] - vTriangle[0])
								* (vTriangle[1] - vTriangle[0]) ;
		vPlane.Normalize() ;
		//
		AddMeshPlane( vb, 3, vTriangle, vNormal, vPlane, (float32_t) cos(PI/4.0) ) ;
	}
}

// 正12面体
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMeshController::AddMeshDodecahedron( S3DVertexBufferInterface& vb )
{
	float32_t	phi = (float32_t) ((1.0 + sqrt(5.0)) * 0.5) ;
	float32_t	rphi = 1.0f / phi ;
	S3DVector	vVertex[20] =
	{
		S3DVector( 1, -1, 1 ),
		S3DVector( 1, -1, -1 ),
		S3DVector( -1, -1, -1 ),
		S3DVector( -1, -1, 1 ),
		S3DVector( 1, 1, 1 ),
		S3DVector( 1, 1, -1 ),
		S3DVector( -1, 1, -1 ),
		S3DVector( -1, 1, 1 ),
		S3DVector( rphi, -phi, 0 ),
		S3DVector( -rphi, -phi, 0 ),
		S3DVector( phi, 0, rphi ),
		S3DVector( phi, 0, -rphi ),
		S3DVector( -phi, 0, rphi ),
		S3DVector( -phi, 0, -rphi ),
		S3DVector( 0, -rphi, -phi ),
		S3DVector( 0, rphi, -phi ),
		S3DVector( 0, -rphi, phi ),
		S3DVector( 0, rphi, phi ),
		S3DVector( rphi, phi, 0 ),
		S3DVector( -rphi, phi, 0 ),
	} ;
	size_t	nIndex[12][5] =
	{
		{ 2, 15, 3, 10, 9 },
		{ 1, 9, 10, 4, 17 },
		{ 1, 11, 12, 2, 9 },
		{ 10, 3, 14, 13, 4 },
		{ 2, 12, 6, 16, 15 },
		{ 15, 16, 7, 14, 3 },
		{ 4, 13, 8, 18, 17 },
		{ 17, 18, 5, 11, 1 },
		{ 11, 5, 19, 6, 12 },
		{ 14, 7, 20, 8, 13 },
		{ 16, 6, 19, 20, 7 },
		{ 18, 8, 20, 19, 5 },
	} ;
	for ( int i = 0; i < 12; i ++ )
	{
		S3DVector	vPentagon[5] ;
		S3DVector	vNormal[5] ;
		for ( int j = 0; j < 5; j ++ )
		{
			size_t	k = nIndex[i][j] - 1 ;
			vPentagon[j] = vVertex[k] * rphi ;
			vNormal[j] = vPentagon[j] ;
			vNormal[j].Normalize() ;
		}
		S3DVector	vPlane = (vPentagon[2] - vPentagon[0])
								* (vPentagon[1] - vPentagon[0]) ;
		vPlane.Normalize() ;
		//
		AddMeshPlane( vb, 5, vPentagon, vNormal, vPlane, (float32_t) cos(PI/3.0) ) ;
	}
}

// 正20面体
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMeshController::AddMeshIcosahedron( S3DVertexBufferInterface& vb )
{
	float32_t	y1 = (float32_t) cos( PI * 2.0 / 3.0 ) ;
	float32_t	y2 = (float32_t) cos( PI / 3.0 ) ;
	float32_t	x1 = (float32_t) sin( PI / 3.0 ) ;
	//
	S3DVector	vTemp[6] =
	{
		S3DVector( 0, -1, 0 ),
		S3DVector( 0, y1, 0 ),
		S3DVector( 0, y1, 0 ),
		S3DVector( 0, y2, 0 ),
		S3DVector( 0, y2, 0 ),
		S3DVector( 0, 1, 0 ),
	} ;
	size_t	nIndex[4][3] =
	{
		{ 0, 2, 1 },
		{ 1, 2, 3 },
		{ 2, 4, 3 },
		{ 3, 4, 5 },
	} ;
	//
	for ( int i = 0; i < 5; i ++ )
	{
		int		j = (i + 1) % 5 ;
		double	r0 = PI * 2.0 * i / 5.0 ;
		double	r2 = PI * 2.0 * j / 5.0 ;
		double	r1 = PI * 2.0 * (i + 0.5) / 5.0 ;
		double	r3 = PI * 2.0 * (j + 0.5) / 5.0 ;
		//
		vTemp[1].x = x1 * (float32_t) cos( r0 ) ;
		vTemp[1].z = x1 * (float32_t) sin( r0 ) ;
		vTemp[2].x = x1 * (float32_t) cos( r2 ) ;
		vTemp[2].z = x1 * (float32_t) sin( r2 ) ;
		vTemp[3].x = x1 * (float32_t) cos( r1 ) ;
		vTemp[3].z = x1 * (float32_t) sin( r1 ) ;
		vTemp[4].x = x1 * (float32_t) cos( r3 ) ;
		vTemp[4].z = x1 * (float32_t) sin( r3 ) ;
		//
		for ( int j = 0; j < 4; j ++ )
		{
			S3DVector	vTriangle[3] ;
			S3DVector	vNormal[3] ;
			for ( int k = 0; k < 3; k ++ )
			{
				vTriangle[k] = vTemp[nIndex[j][k]] ;
				vNormal[k] = vTriangle[k] ;
				vNormal[k].Normalize() ;
			}
			//
			S3DVector	vPlane = (vTriangle[2] - vTriangle[0])
									* (vTriangle[1] - vTriangle[0]) ;
			vPlane.Normalize() ;
			//
			AddMeshPlane( vb, 3, vTriangle, vNormal, vPlane, (float32_t) cos(PI/3.0) ) ;
		}
	}
}

// 切頂20面体（36面体）
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMeshController::AddMeshTruncatedIcosahedron( S3DVertexBufferInterface& vb )
{
	AddTruncatedPolyhedron( vb, 0 ) ;
}

// 80面体
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMeshController::AddMeshIcosahedronX4( S3DVertexBufferInterface& vb )
{
	AddTrianglePolyhedron( vb, 1 ) ;
}

// 切頂80面体
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMeshController::AddMeshTruncatedIcosahedronX4( S3DVertexBufferInterface& vb )
{
	AddTruncatedPolyhedron( vb, 1 ) ;
}

// 正20面体分割体・切頂多面体更新
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMeshController::UpdateIcosahedronMesh( size_t nDiv, bool flagTruncate )
{
	if ( !m_flagTrianglePolyhedron || (m_nIcosahedronDiv != nDiv) )
	{
		m_triangles.CreateIcosahedron() ;
		for ( size_t i = 0; i < nDiv; i ++ )
		{
			m_triangles.DividedTriangle() ;
		}
		m_flagTrianglePolyhedron = true ;
		m_flagPolyhedronTruncated = false ;
		m_nIcosahedronDiv = nDiv ;
	}
	if ( flagTruncate && !m_flagPolyhedronTruncated )
	{
		m_truncated.CreateTruncatedFace( m_triangles ) ;
		m_flagPolyhedronTruncated = true ;
	}
}

// 正20面体分割体
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMeshController::AddTrianglePolyhedron
		( S3DVertexBufferInterface& vb, size_t nDiv )
{
	UpdateIcosahedronMesh( nDiv, false ) ;

	const S3DVector *	pVertex = m_triangles.m_aVertex.GetConstArray() ;
	const S3DPolyhedronMesh::Triangle *
						pFace = m_triangles.m_aFace.GetConstArray() ;
	const size_t		nFaceCount = m_triangles.m_aFace.GetLength() ;

	double		n = 6.0 * pow( 2.0, (double) nDiv ) ;
	float32_t	cosBevelAngle = (float32_t) cos( PI * (n - 2.0) * 0.5 / n ) ;
	for ( size_t i = 0; i < nFaceCount; i ++ )
	{
		const S3DPolyhedronMesh::Triangle&	face = pFace[i] ;

		S3DVector	vTriangle[3] ;
		S3DVector	vNormal[3] ;
		for ( int k = 0; k < 3; k ++ )
		{
			vTriangle[k] = pVertex[face.iVertex[2 - k]] ;
			vNormal[k] = vTriangle[k] ;
			vNormal[k].Normalize() ;
		}
		//
		S3DVector	vPlane = (vTriangle[2] - vTriangle[0])
								* (vTriangle[1] - vTriangle[0]) ;
		vPlane.Normalize() ;
		//
		AddMeshPlane( vb, 3, vTriangle, vNormal, vPlane, cosBevelAngle ) ;
	}
}

// 正20面体分割切頂多面体
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMeshController::AddTruncatedPolyhedron( S3DVertexBufferInterface& vb, size_t nDiv )
{
	UpdateIcosahedronMesh( nDiv, true ) ;

	const S3DPolyhedronMesh::TruncatedFace *
					pFaces = m_truncated.m_aFace.GetConstArray() ;
	const size_t	nFaceCount = m_truncated.m_aFace.GetLength() ;

	double		n = 6.0 * pow( 2.0, (double) nDiv ) ;
	float32_t	cosBevelAngle = (float32_t) cos( PI * (n - 2.0) * 0.5 / n ) ;
	for ( size_t i = 0; i < nFaceCount; i ++ )
	{
		const S3DPolyhedronMesh::TruncatedFace &	face = pFaces[i] ;

		S3DVector	vVertex[S3DPolyhedronMesh::vertexMaxEdge] ;
		S3DVector	vNormal[S3DPolyhedronMesh::vertexMaxEdge] ;
		for ( size_t k = 0; k < face.nVertex; k ++ )
		{
			vVertex[k] = face.vVertex[face.nVertex - k - 1] ;
			vNormal[k] = vVertex[k] ;
			vNormal[k].Normalize() ;
		}
		//
		S3DVector	vPlane = face.vCenter ;
		vPlane.Normalize() ;
		//
		AddMeshPlane( vb, face.nVertex, vVertex, vNormal, vPlane, cosBevelAngle ) ;
	}
}

// 一つの面を追加
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMeshController::AddMeshPlane
	( S3DVertexBufferInterface& vb,
		size_t nCount,
		S3DVector * pvVertex,
		const S3DVector * pvNormal,
		const S3DVector & vPlaneNormal,
		float32_t cosBevelAngle )
{
	if ( nCount <= 2 )
	{
		return ;
	}
	//
	// 中心点計算
	//
	S3DVector	vCenter( 0, 0, 0 ) ;
	uint32_t	i, j, k ;
	for ( i = 0; i < nCount; i ++ )
	{
		vCenter += pvVertex[i] ;
	}
	vCenter /= (float32_t) nCount ;
	//
	// UV マップ
	//
	if ( m_uvPorj == uvOrthogonal )
	{
		S3DMatrix	matUVDir( 1, 1, 1 ) ;
		matUVDir.RevolveByAngleOn( - vPlaneNormal ) ;
		//
		float32_t	xMin = -0.5f, yMin = -0.5f ;
		float32_t	xMax = 0.5f, yMax = 0.5f ;
		for ( i = 0; i < nCount; i ++ )
		{
			S3DVector	v = matUVDir * pvVertex[i] ;
			xMin = esl_fminf( v.x, xMin ) ;
			yMin = esl_fminf( v.y, yMin ) ;
			xMax = esl_fmaxf( v.x, xMax ) ;
			yMax = esl_fmaxf( v.y, yMax ) ;
		}
		float32_t	sz = 1.0f / esl_fmaxf( xMax - xMin, yMax - yMin ) ;
		matUVDir *= sz ;
		//
		m_matUVSpace = matUVDir ;
		m_vUVSpace.x = 0.5f - (xMax + xMin) * sz ;
		m_vUVSpace.y = 0.5f - (yMax + yMin) * sz ;
		m_vUVSpace.z = -sz ;
	}
	else
	{
		float32_t	sx = 0.25f * 0.5f ;
		float32_t	sy = 0.5f / 3.0f ;
		float32_t	rsq2 = (float32_t) sqrt( 0.5 ) ;
		if ( vPlaneNormal.y < -rsq2 )
		{
			m_matUVSpace =
				S3DMatrix( sx, 0, 0,
							0, 0, -sy,
							0, 1.0f, 0 ) ;
			m_vUVSpace = S3DVector( sx * 3.0f, sy, 0.0f ) ;
		}
		else if ( vPlaneNormal.y > rsq2 )
		{
			m_matUVSpace =
				S3DMatrix( sx, 0, 0,
							0, 0, -sy,
							0, 1.0f, 0 ) ;
			m_vUVSpace = S3DVector( sx * 3.0f, sy * 5.0f, 0.0f ) ;
		}
		else if ( vPlaneNormal.z < -rsq2 )
		{
			m_matUVSpace =
				S3DMatrix( sx, 0, 0,
							0, sy, 0,
							0, 0, 1.0f ) ;
			m_vUVSpace = S3DVector( sx * 3.0f, sy * 3.0f, 0.0f ) ;
		}
		else if ( vPlaneNormal.z > rsq2 )
		{
			m_matUVSpace =
				S3DMatrix( -sx, 0, 0,
							0, sy, 0,
							0, 0, 1.0f ) ;
			m_vUVSpace = S3DVector( sx * 7.0f, sy * 3.0f, 0.0f ) ;
		}
		else if ( vPlaneNormal.x < -rsq2 )
		{
			m_matUVSpace =
				S3DMatrix( 0, 0, -sx,
							0, sy, 0,
							1.0f, 0, 0 ) ;
			m_vUVSpace = S3DVector( sx, sy * 3.0f, 0.0f ) ;
		}
		else // if ( vPlaneNormal.x > rsq2 )
		{
			m_matUVSpace =
				S3DMatrix( 0, 0, sx,
							0, sy, 0,
							1.0f, 0, 0 ) ;
			m_vUVSpace = S3DVector( sx * 5.0f, sy * 3.0f, 0.0f ) ;
		}
	}
	m_matUVSpace =
		S3DMatrix( m_vUVScale.x * m_rectAtlasRef.w,
					m_vUVScale.y * m_rectAtlasRef.h, 1 ) * m_matUVSpace ;
	m_vUVSpace.x =
		(float32_t) (m_vUVSpace.x * m_vUVScale.x
							* m_rectAtlasRef.w + m_rectAtlasRef.x) ;
	m_vUVSpace.y =
		(float32_t) (m_vUVSpace.y * m_vUVScale.y
							* m_rectAtlasRef.h + m_rectAtlasRef.y) ;
	//
	// ベベル処理
	//
	if ( m_bevel != bevelNo )
	{
		AddMeshBevel
			( vb, nCount, pvVertex, pvNormal,
				cosBevelAngle, vCenter, vPlaneNormal ) ;
	}
	//
	// メモリ確保
	//
	size_t	nSwellDiv = (size_t) esl_max( m_nSwellDiv, 1 ) ;
	//
	S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
	const size_t	nPolygonCount = ((nSwellDiv - 1) * 2 + 1) * nCount ;
	const size_t	nVertexCount = nSwellDiv * nCount + 1 ;
	vb.AllocatePrimitiveBuffer
		( prmbuf, primitiveTriangle,
			nPolygonCount * 3, nVertexCount ) ;
	//
	// 座標計算
	//
	S3DVector	vBase = m_vPosition ;
	S3DColor	clrTemp( 0xFFFFFFFF, 0 ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		S3DVector	vDelta = pvVertex[i] - vCenter ;
		S3DVector	vDir = vDelta ;
		vDir.Normalize() ;
		//
		for ( j = 0; j < nSwellDiv; j ++ )
		{
			double		r = (double) j / nSwellDiv ;
			double		h = (0.5 - 0.5 * cos( PI * r )) * m_fpSwelling ;
			double		dh = sin( PI * r ) * 0.5 * m_fpSwelling ;
			S3DVector	vPos = vCenter ;
			S3DVector	vUV ;
			vPos += vDelta * (1.0 - r) ;
			vUV = m_matUVSpace * vPos + m_vUVSpace ;
			vPos += vPlaneNormal * h ;
			//
			S3DVector	vNormal = vPlaneNormal ;
			vNormal += vDir * dh ;
			vNormal.Normalize() ;
			//
			if ( m_flagBackFace )
			{
				vNormal = -vNormal ;
			}
			//
			k = j * (uint32_t) nCount + i ;
			prmbuf.pvVertex[k] = m_matMeshSpace * vPos + vBase ;
			prmbuf.pvNormal[k] = m_matMeshSpace * vNormal ;
			prmbuf.pvUVMap[k] = S2DVector( vUV.x, vUV.y ) ;
			prmbuf.pColor[k] = clrTemp ;
		}
	}
	S3DVector	vUV = m_matUVSpace * vCenter + m_vUVSpace ;
	k = (uint32_t) nVertexCount - 1 ;
	prmbuf.pvVertex[k] =
		m_matMeshSpace * (vCenter + vPlaneNormal * m_fpSwelling) + vBase ;
	prmbuf.pvNormal[k] = m_matMeshSpace * vPlaneNormal ;
	prmbuf.pvUVMap[k] = S2DVector( vUV.x, vUV.y ) ;
	prmbuf.pColor[k] = clrTemp ;
	//
	// 三角形リスト
	//
	k = 0 ;
	for ( i = 0; i < nSwellDiv - 1; i ++ )
	{
		uint32_t	i0 = i * (uint32_t) nCount ;
		uint32_t	i1 = i0 + (uint32_t) nCount ;
		for ( j = 0; j < nCount; j ++ )
		{
			uint32_t	j1 = (j + 1) % nCount ;
			if ( m_flagBackFace )
			{
				prmbuf.pIndexedList[k ++] = i0 + j ;
				prmbuf.pIndexedList[k ++] = i1 + j1 ;
				prmbuf.pIndexedList[k ++] = i1 + j ;
				prmbuf.pIndexedList[k ++] = i0 + j ;
				prmbuf.pIndexedList[k ++] = i0 + j1 ;
				prmbuf.pIndexedList[k ++] = i1 + j1 ;
			}
			else
			{
				prmbuf.pIndexedList[k ++] = i0 + j ;
				prmbuf.pIndexedList[k ++] = i1 + j ;
				prmbuf.pIndexedList[k ++] = i1 + j1 ;
				prmbuf.pIndexedList[k ++] = i0 + j ;
				prmbuf.pIndexedList[k ++] = i1 + j1 ;
				prmbuf.pIndexedList[k ++] = i0 + j1 ;
			}
		}
	}
	uint32_t	i0 = (uint32_t) ((nSwellDiv - 1) * nCount) ;
	uint32_t	i1 = (uint32_t) nVertexCount - 1 ;
	for ( j = 0; j < nCount; j ++ )
	{
		uint32_t	j1 = (j + 1) % nCount ;
		prmbuf.pIndexedList[k ++] = i0 + j ;
		if ( m_flagBackFace )
		{
			prmbuf.pIndexedList[k ++] = i0 + j1 ;
			prmbuf.pIndexedList[k ++] = i1 ;
		}
		else
		{
			prmbuf.pIndexedList[k ++] = i1 ;
			prmbuf.pIndexedList[k ++] = i0 + j1 ;
		}
	}
	ESLAssert( k == nPolygonCount * 3 ) ;
	//
	// 追加
	//
	vb.AddPrimitiveBuffer
		( NULL, 0, primitiveTriangle, prmbuf,
			nPolygonCount * 3, nVertexCount ) ;
}

// 面のベベル処理
//////////////////////////////////////////////////////////////////////////////
void S3DPolyhedronMeshController::AddMeshBevel
	( S3DVertexBufferInterface& vb,
		size_t nCount,
		S3DVector * pvVertex,
		const S3DVector * pvNormal,
		float32_t cosBevelAngle,
		const S3DVector & vPlaneCenter,
		const S3DVector & vPlaneNormal )
{
	ESLAssert( m_bevel != bevelNo ) ;
	//
	// メモリ確保
	//
	S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
	const size_t	nPolygonCount = nCount * 2 ;
	const size_t	nVertexCount = nCount * 2 ;
	if ( m_bevel != bevelCutOff )
	{
		vb.AllocatePrimitiveBuffer
			( prmbuf, primitiveTriangle,
				nPolygonCount * 3, nVertexCount ) ;
	}
	//
	// 座標計算
	//
	uint32_t	i, j, k ;
	S3DVector	vBase = m_vPosition ;
	S3DColor	clrTemp( 0xFFFFFFFF, 0 ) ;
	float32_t	fpBevel = (float32_t) m_fpBevel ;
	float32_t	fpBevelCorner = fpBevel ;
	if ( m_bevel == bevelCurved )
	{
		fpBevelCorner *= 0.66667f ;
	}
	cosBevelAngle *= (float32_t) m_fpBevelDepth ;
	//
	for ( i = 0; i < nCount; i ++ )
	{
		S3DVector	vDelta = pvVertex[i] - vPlaneCenter ;
		S3DVector	vDir = vDelta ;
		vDir.Normalize() ;
		//
		float32_t	cosAngle = cosBevelAngle ;//vDir.InnerProduct( pvNormal[i] ) ;
		float32_t	r = cosAngle * fpBevelCorner ;
		//
		S3DVector	vPos = pvVertex[i] - pvNormal[i] * r ;
		pvVertex[i] -= vDir * fpBevel ;
		//
		if ( m_bevel == bevelCutOff )
		{
			continue ;
		}
		//
		j = i + (uint32_t) nCount ;
		prmbuf.pvVertex[i] = m_matMeshSpace * vPos + vBase ;
		prmbuf.pvVertex[j] = m_matMeshSpace * pvVertex[i] + vBase ;
		//
		prmbuf.pvNormal[i] = m_matMeshSpace * pvNormal[i] ;
		if ( m_bevel == bevelCurved )
		{
			prmbuf.pvNormal[j] = m_matMeshSpace * vPlaneNormal ;
		}
		else
		{
			prmbuf.pvNormal[j] = prmbuf.pvNormal[i] ;
		}
		if ( m_flagBackFace )
		{
			prmbuf.pvNormal[i] = - prmbuf.pvNormal[i] ;
			prmbuf.pvNormal[j] = - prmbuf.pvNormal[j] ;
		}
		//
		S3DVector	vUV = m_matUVSpace * vPos + m_vUVSpace ;
		prmbuf.pvUVMap[i] = S2DVector( vUV.x, vUV.y ) ;
		vUV = m_matUVSpace * pvVertex[i] + m_vUVSpace ;
		prmbuf.pvUVMap[j] = S2DVector( vUV.x, vUV.y ) ;
		//
		prmbuf.pColor[i] = clrTemp ;
		prmbuf.pColor[j] = clrTemp ;
	}
	if ( m_bevel == bevelCutOff )
	{
		return ;
	}
	//
	// 三角形リスト
	//
	k = 0 ;
	for ( i = 0; i < nCount; i ++ )
	{
		j = (i + 1) % nCount ;
		if ( m_flagBackFace )
		{
			prmbuf.pIndexedList[k ++] = i ;
			prmbuf.pIndexedList[k ++] = j + (uint32_t) nCount ;
			prmbuf.pIndexedList[k ++] = i + (uint32_t) nCount ;
			prmbuf.pIndexedList[k ++] = i ;
			prmbuf.pIndexedList[k ++] = j ;
			prmbuf.pIndexedList[k ++] = j + (uint32_t) nCount ;
		}
		else
		{
			prmbuf.pIndexedList[k ++] = i ;
			prmbuf.pIndexedList[k ++] = i + (uint32_t) nCount ;
			prmbuf.pIndexedList[k ++] = j + (uint32_t) nCount ;
			prmbuf.pIndexedList[k ++] = i ;
			prmbuf.pIndexedList[k ++] = j + (uint32_t) nCount ;
			prmbuf.pIndexedList[k ++] = j ;
		}
	}
	ESLAssert( k == nPolygonCount * 3 ) ;
	//
	// 追加
	//
	vb.AddPrimitiveBuffer
		( NULL, 0, primitiveTriangle, prmbuf,
			nPolygonCount * 3, nVertexCount ) ;
}



//////////////////////////////////////////////////////////////////////////////
// 平面／円柱／球／トーラス／放物錐メッシュ生成
//////////////////////////////////////////////////////////////////////////////

const wchar_t *	S3DStandardMeshController::m_pwszShapeType
						[S3DStandardMeshController::shapeCount] =
{
	L"plane", L"cylinder", L"sphere", L"torus", L"parabola",
} ;

const S3DStandardMeshController::PFUNC_ADD_MESH
	S3DStandardMeshController::m_pfnAddMesh
		[S3DStandardMeshController::shapeCount] =
{
	&S3DStandardMeshController::AddMeshPlane,
	&S3DStandardMeshController::AddMeshCylinder,
	&S3DStandardMeshController::AddMeshSphere,
	&S3DStandardMeshController::AddMeshTorus,
	&S3DStandardMeshController::AddMeshParabola,
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DStandardMeshController, MeshController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DStandardMeshController, simple_mesh_builder )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DStandardMeshController::S3DStandardMeshController( void )
	: MeshController( m_ItemClassDescriptor.pwszClassID ),
		m_shape( shapeSphere ),
		m_flagBackFace( false ),
		m_vPosition( 0, 0, 0 ),
		m_matRotation( 1, 1, 1 ),
		m_vSize( 1, 1, 1 ), m_fpRadius( 1 ),
		m_nHorzDiv( 16 ), m_nVertDiv( 16 ),
		m_degStartLat( 0 ), m_degRangeLat( 360 ),
		m_degStartLong( 0 ), m_degRangeLong( 180 ),
		m_rectAtlasRef( 0, 0, 1, 1 ),
		m_vUVPosition( 0, 0 ), m_vUVScale( 1, 1 ),
		m_colorBase( 0xFFFFFF, 0 ),
		m_alphaMaster( 1 ),
		m_alphaCenter( 1 ), m_alphaTop( 1 ),
		m_alphaBottom( 1 ), m_alphaLeft( 1 ), m_alphaRight( 1 )
{
	PrepareParameterEntryCount( paramCount ) ;
	AddParameterEntry
		( L"shape_type", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration,
			L"形状", NULL ) ;
	AddParameterEntry
		( L"back_face", S3DSceneComposer::typeBoolean, 0,
			L"面反転", NULL ) ;
	AddParameterEntry
		( L"position", S3DSceneComposer::typePosition,
			S3DSceneComposer::attrNoLocalTransform,
			L"位置", NULL ) ;
	AddParameterEntry
		( L"rotation", S3DSceneComposer::typeRotation, 0,
			L"回転", NULL ) ;
	AddParameterEntry
		( L"zoom", S3DSceneComposer::typeZoom, 0,
			L"サイズ", NULL ) ;
	AddParameterEntry
		( L"radius", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"スケール", NULL, 0.0, 2.0 ) ;
	AddParameterEntry
		( L"horz_div", S3DSceneComposer::typeInteger, 0,
			L"経度分割数", L"回転／ｘ方向分割数" ) ;
	AddParameterEntry
		( L"vert_div", S3DSceneComposer::typeInteger, 0,
			L"緯度分割数", L"垂直／ｙ方向分割数" ) ;
	AddParameterEntry
		( L"start_latitude", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"開始経度", L"開始経度 [deg]", -180, 180 ) ;
	AddParameterEntry
		( L"range_latitude", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"経度幅", L"経度幅 [deg]", 0, 360 ) ;
	AddParameterEntry
		( L"start_longitude", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"開始緯度", L"開始緯度 [deg]", -180, 180 ) ;
	AddParameterEntry
		( L"range_longitude", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"緯度幅", L"緯度幅 [deg]", 0, 180 ) ;
	AddParameterEntry
		( L"uv_atlas", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration,
			L"アトラス画像参照", NULL ) ;
	AddParameterEntry
		( L"uv_scale", S3DSceneComposer::typeVector2, 0,
			L"UVスケール", NULL ) ;
	AddParameterEntry
		( L"uv_offset", S3DSceneComposer::typeVector2, 0,
			L"UVオフセット", NULL ) ;
	AddParameterEntry
		( L"color_mul", S3DSceneComposer::typeColor, 0,
			L"乗算色", NULL ) ;
	AddParameterEntry
		( L"color_add", S3DSceneComposer::typeColor, 0,
			L"加算色", NULL ) ;
	AddParameterEntry
		( L"alpha_master", S3DSceneComposer::typeScalar, 0,
			L"全体α", NULL ) ;
	AddParameterEntry
		( L"alpha_center", S3DSceneComposer::typeScalar, 0,
			L"中心α", NULL ) ;
	AddParameterEntry
		( L"alpha_top", S3DSceneComposer::typeScalar, 0,
			L"上辺α", NULL ) ;
	AddParameterEntry
		( L"alpha_bottom", S3DSceneComposer::typeScalar, 0,
			L"底辺α", NULL ) ;
	AddParameterEntry
		( L"alpha_left", S3DSceneComposer::typeScalar, 0,
			L"左辺α", NULL ) ;
	AddParameterEntry
		( L"alpha_right", S3DSceneComposer::typeScalar, 0,
			L"右辺α", NULL ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DStandardMeshController::~S3DStandardMeshController( void )
{
}

// アトラス画像参照領域更新
//////////////////////////////////////////////////////////////////////////////
void S3DStandardMeshController::UpdateRefAtlasRect( void )
{
	if ( m_strUVAtlas.IsEmpty() )
	{
		m_rectAtlasRef.x = 0 ;
		m_rectAtlasRef.y = 0 ;
		m_rectAtlasRef.w = 1 ;
		m_rectAtlasRef.h = 1 ;
		return ;
	}
	S3DSceneComposer *	pComposer = GetComposer() ;
	if ( pComposer != NULL )
	{
		SGLImageObject *	pImage =
			pComposer->Assets().GetImageAs( m_strUVAtlas ) ;
		if ( pImage != NULL )
		{
			if ( pImage->GetReferenceRectOfAtlas( m_rectAtlasRef ) )
			{
				m_rectAtlasRef.x = 0 ;
				m_rectAtlasRef.y = 0 ;
				m_rectAtlasRef.w = 1 ;
				m_rectAtlasRef.h = 1 ;
			}
		}
	}
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DStandardMeshController::GetMatrixParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRotation:
		return	m_matRotation ;
	}
	return	S3DDMatrix( 1, 1, 1 ) ;
}

S3DDVector S3DStandardMeshController::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramPosition:
		return	m_vPosition ;
	case	paramSize:
		return	m_vSize ;
	case	paramColorMul:
		return	VectorFromColor( m_colorBase.rgbMul ) ;
	case	paramColorAdd:
		return	VectorFromColor( m_colorBase.rgbAdd ) ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DStandardMeshController::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRadius:
		return	m_fpRadius ;
	case	paramStartLatitude:
		return	m_degStartLat ;
	case	paramRangeLatitude:
		return	m_degRangeLat ;
	case	paramStartLongitude:
		return	m_degStartLong ;
	case	paramRangeLongitude:
		return	m_degRangeLong ;
	case	paramColorAlphaMaster:
		return	m_alphaMaster ;
	case	paramColorAlphaCenter:
		return	m_alphaCenter ;
	case	paramColorAlphaTop:
		return	m_alphaTop ;
	case	paramColorAlphaBottom:
		return	m_alphaBottom ;
	case	paramColorAlphaLeft:
		return	m_alphaLeft ;
	case	paramColorAlphaRight:
		return	m_alphaRight ;
	}
	return	0.0 ;
}

int32_t S3DStandardMeshController::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramHorzDiv:
		return	m_nHorzDiv ;
	case	paramVertDiv:
		return	m_nVertDiv ;
	}
	return	0 ;
}

bool S3DStandardMeshController::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramBackFace:
		return	m_flagBackFace ;
	}
	return	false ;
}

const wchar_t * S3DStandardMeshController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramShapeType:
		return	m_pwszShapeType[m_shape] ;
	case	paramUVAtlas:
		return	m_strUVAtlas ;
	}
	return	NULL ;
}

size_t S3DStandardMeshController::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t i ) const
{
	switch ( i )
	{
	case	paramUVScale:
		if ( nBufBytes == 0 )
		{
			return	sizeof(S2DDVector) ;
		}
		else if ( nBufBytes == sizeof(S2DDVector) )
		{
			*((S2DDVector*)pDst) = m_vUVScale ;
			return	sizeof(S2DDVector) ;
		}
		break ;
	case	paramUVOffset:
		if ( nBufBytes == 0 )
		{
			return	sizeof(S2DDVector) ;
		}
		else if ( nBufBytes == sizeof(S2DDVector) )
		{
			*((S2DDVector*)pDst) = m_vUVPosition ;
			return	sizeof(S2DDVector) ;
		}
		break ;
	}
	return	0 ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DStandardMeshController::SetMatrixParameter( size_t i, const S3DDMatrix& mat )
{
	switch ( i )
	{
	case	paramRotation:
		m_matRotation = mat ;
		return ;
	}
}

void S3DStandardMeshController::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramPosition:
		m_vPosition = vec ;
		return ;
	case	paramSize:
		m_vSize = vec ;
		return ;
	case	paramColorMul:
		m_colorBase.rgbMul = ColorFromVector( vec ) ;
		return ;
	case	paramColorAdd:
		m_colorBase.rgbAdd = ColorFromVector( vec ) ;
		return ;
	}
}

void S3DStandardMeshController::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramRadius:
		m_fpRadius = s ;
		return ;
	case	paramStartLatitude:
		m_degStartLat = s ;
		return ;
	case	paramRangeLatitude:
		m_degRangeLat = s ;
		return ;
	case	paramStartLongitude:
		m_degStartLong = s ;
		return ;
	case	paramRangeLongitude:
		m_degRangeLong = s ;
		return ;
	case	paramColorAlphaMaster:
		m_alphaMaster = s ;
		return ;
	case	paramColorAlphaCenter:
		m_alphaCenter = s ;
		return ;
	case	paramColorAlphaTop:
		m_alphaTop = s ;
		return ;
	case	paramColorAlphaBottom:
		m_alphaBottom = s ;
		return ;
	case	paramColorAlphaLeft:
		m_alphaLeft = s ;
		return ;
	case	paramColorAlphaRight:
		m_alphaRight = s ;
		return ;
	}
}

void S3DStandardMeshController::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramHorzDiv:
		m_nHorzDiv = esl_max( n, 1 ) ;
		return ;
	case	paramVertDiv:
		m_nVertDiv = esl_max( n, 1 ) ;
		return ;
	}
}

void S3DStandardMeshController::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramBackFace:
		m_flagBackFace = b ;
		return ;
	}
}

void S3DStandardMeshController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	size_t	j ;
	switch ( i )
	{
	case	paramShapeType:
		for ( j = 0; j < shapeCount; j ++ )
		{
			if ( SString::Compare( m_pwszShapeType[j], pwszCmd ) == 0 )
			{
				m_shape = (ShapeType) j ;
				break ;
			}
		}
		return ;

	case	paramUVAtlas:
		if ( m_strUVAtlas != pwszCmd )
		{
			m_strUVAtlas = pwszCmd ;
			UpdateRefAtlasRect() ;
		}
		return ;
	}
}

size_t S3DStandardMeshController::SetBinaryParameter
	( size_t i, const void * pSrc, size_t nBufBytes )
{
	switch ( i )
	{
	case	paramUVScale:
		if ( nBufBytes == sizeof(S2DDVector) )
		{
			m_vUVScale = *((S2DDVector*)pSrc) ;
			return	sizeof(S2DDVector) ;
		}
		break ;
	case	paramUVOffset:
		if ( nBufBytes == sizeof(S2DDVector) )
		{
			m_vUVPosition = *((S2DDVector*)pSrc) ;
			return	sizeof(S2DDVector) ;
		}
		break ;
	}
	return	0 ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DStandardMeshController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	size_t	j ;
	switch ( i )
	{
	case	paramShapeType:
		for ( j = 0; j < shapeCount; j ++ )
		{
			aStrSet.Add( new SString( m_pwszShapeType[j] ) ) ;
		}
		return	true ;

	case	paramUVAtlas:
		{
			S3DSceneComposer *	pComposer = GetComposer() ;
			if ( pComposer != NULL )
			{
				pComposer->Assets().EnumerateResourceIDsAs
					( aStrSet, ESL_RUNTIME_CLASS(SGLImageObject) ) ;
			}
		}
		return	true ;
	}
	return	false ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DStandardMeshController::GetParameterCategoryName( size_t iCategory ) const
{
	return	L"形状" ;
}

// メッシュ追加処理（全視点・ビュー共通処理）
//////////////////////////////////////////////////////////////////////////////
void S3DStandardMeshController::AddMesh
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem,
		S3DVertexBufferInterface ** ppVBs, size_t nVBCount )
{
	if ( (m_nHorzDiv < 1) || (m_nVertDiv < 1) || (nVBCount == 0) )
	{
		return ;
	}
	S3DVertexBufferInterface&	vb = *(ppVBs[0]) ;
	//
	// バッファ確保
	//
	const size_t	nHorzCount = (size_t) m_nHorzDiv + 1 ;
	const size_t	nVertCount = (size_t) m_nVertDiv + 1 ;
	const size_t	nPolyCount = (size_t) (m_nHorzDiv * m_nVertDiv) * 2 ;
	const size_t	nIndexCount = nPolyCount * 3 ;
	const size_t	nVertexCount = nHorzCount * nVertCount ;
	//
	S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
	vb.AllocatePrimitiveBuffer
		( prmbuf, primitiveTriangle, nIndexCount, nVertexCount ) ;
	//
	// 座標計算
	//
	(this->*m_pfnAddMesh[m_shape])( prmbuf.pvVertex, prmbuf.pvNormal ) ;
	//
	S3DMatrix	matRot = m_matRotation ;
	S3DVector	vBasePos = m_vPosition ;
	S3DVector	vZero( 0, 0, 0 ) ;
	matRot.RevolveVectors
		( prmbuf.pvVertex, prmbuf.pvVertex, nVertexCount, vBasePos ) ;
	matRot.RevolveVectors
		( prmbuf.pvNormal, prmbuf.pvNormal, nVertexCount, vZero ) ;
	//
	// UV 計算
	//
	S2DDVector	vUVScale = m_vUVScale ;
	vUVScale.x /= m_nHorzDiv ;
	vUVScale.y /= m_nVertDiv ;
	vUVScale.x *= m_rectAtlasRef.w ;
	vUVScale.y *= m_rectAtlasRef.h ;
	//
	S2DDVector	vUVOffset = m_vUVPosition ;
	vUVOffset.x += m_rectAtlasRef.x ;
	vUVOffset.y += m_rectAtlasRef.y ;
	//
	size_t	i, j, k ;
	for ( i = 0; i < nVertCount; i ++ )
	{
		S2DVector *	pvUVLine = prmbuf.pvUVMap + (i * nHorzCount) ;
		float32_t	v = (float32_t) (i * vUVScale.y + vUVOffset.y) ;
		for ( j = 0; j < nHorzCount; j ++ )
		{
			pvUVLine->x = (float32_t) (j * vUVScale.x + vUVOffset.x) ;
			pvUVLine->y = v ;
			pvUVLine ++ ;
		}
	}
	//
	// 色マップ
	//
	S2DDVector	vGradPos( 2.0 / m_nHorzDiv, 2.0 / m_nVertDiv ) ;
	S3DColor	clrVertex = m_colorBase ;
	float32_t	alphaMaster = (float32_t) (m_alphaMaster * 0xFF) ;
	for ( i = 0; i < nVertCount; i ++ )
	{
		double	y = vGradPos.y * i ;
		double	ay0, ay1 ;
		if ( y < 1.0 )
		{
			ay0 = m_alphaTop ;
			ay1 = m_alphaCenter ;
		}
		else
		{
			ay0 = m_alphaCenter ;
			ay1 = m_alphaBottom ;
			y -= 1.0 ;
		}
		double		ay = ay0 + (ay1 - ay0) * y - m_alphaCenter ;
		S3DColor *	pColorLine = prmbuf.pColor + (i * nHorzCount) ;
		for ( j = 0; j < nHorzCount; j ++ )
		{
			double	x = vGradPos.x * j ;
			double	ax0, ax1 ;
			if ( x < 1.0 )
			{
				ax0 = m_alphaLeft ;
				ax1 = m_alphaCenter ;
			}
			else
			{
				ax0 = m_alphaCenter ;
				ax1 = m_alphaRight ;
				x -= 1.0 ;
			}
			float32_t	ax = (float32_t) (ay + ax0 + (ax1 - ax0) * x) ;
			//
			clrVertex.rgbMul.argb.Alpha =
				(uint8_t) esl_clampi( eslRoundR32ToInt( ax * alphaMaster ), 0, 0xFF ) ;
			*pColorLine = clrVertex ;
			pColorLine ++ ;
		}
	}
	//
	// インデックス
	//
	const size_t	nVertDiv = (size_t) m_nVertDiv ;
	const size_t	nHorzDiv = (size_t) m_nHorzDiv ;
	//
	uint32_t *	pIndex = prmbuf.pIndexedList ;
	for ( i = 0, k = 0; i < nVertDiv; i ++ )
	{
		size_t	iv = i * nHorzCount ;
		for ( j = 0; j < nHorzDiv; j ++ )
		{
			if ( m_flagBackFace )
			{
				pIndex[k ++] = (uint32_t) (iv + j) ;
				pIndex[k ++] = (uint32_t) (iv + j + 1) ;
				pIndex[k ++] = (uint32_t) (iv + j + nHorzCount) ;
				pIndex[k ++] = (uint32_t) (iv + j + 1) ;
				pIndex[k ++] = (uint32_t) (iv + j + nHorzCount + 1) ;
				pIndex[k ++] = (uint32_t) (iv + j + nHorzCount) ;
			}
			else
			{
				pIndex[k ++] = (uint32_t) (iv + j) ;
				pIndex[k ++] = (uint32_t) (iv + j + nHorzCount) ;
				pIndex[k ++] = (uint32_t) (iv + j + 1) ;
				pIndex[k ++] = (uint32_t) (iv + j + 1) ;
				pIndex[k ++] = (uint32_t) (iv + j + nHorzCount) ;
				pIndex[k ++] = (uint32_t) (iv + j + nHorzCount + 1) ;
			}
		}
	}
	ESLAssert( k == nIndexCount ) ;
	//
	// 確定
	//
	vb.AddPrimitiveBuffer
		( NULL, 0, primitiveTriangle, prmbuf, nIndexCount, nVertexCount ) ;
}

// フレーム描画前処理（全視点・ビュー共通処理）
//////////////////////////////////////////////////////////////////////////////
void S3DStandardMeshController::UpdateMesh
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem,
		S3DVertexBufferInterface ** ppVBs, size_t nVBCount )
{
}

// 平面
//////////////////////////////////////////////////////////////////////////////
void S3DStandardMeshController::AddMeshPlane( S3DVector4 * pvVertex, S3DVector4 * pvNormal )
{
	const size_t	nHorzCount = (size_t) m_nHorzDiv + 1 ;
	const size_t	nVertCount = (size_t) m_nVertDiv + 1 ;
	//
	S3DVector4	vNormal( 0, -1, 0, 0 ) ;
	if ( m_flagBackFace )
	{
		vNormal.y = 1 ;
	}
	double	xScale = m_vSize.x * m_fpRadius * m_degRangeLat
										/ (360.0 * m_nHorzDiv) ;
	double	zScale = m_vSize.z * m_fpRadius * m_degRangeLong
										/ (180.0 * m_nVertDiv) ;
	double	xOffset = m_vSize.x * m_fpRadius
								* (m_degStartLat - 180.0) / 360.0 ;
	double	zOffset = m_vSize.z * m_fpRadius
								* (m_degStartLong - 90.0) / 180.0 ;
	//
	for ( size_t i = 0; i < nVertCount; i ++ )
	{
		S3DVector4 *	pvNextVert = pvVertex + (i * nHorzCount) ;
		S3DVector4 *	pvNextNorm = pvNormal + (i * nHorzCount) ;
		float32_t		z = (float32_t) (zScale * (m_nVertDiv - i) + zOffset) ;
		for ( size_t j = 0; j < nHorzCount; j ++ )
		{
			pvNextVert->x = (float32_t) (xScale * j + xOffset) ;
			pvNextVert->y = 0.0f ;
			pvNextVert->z = z ;
			pvNextVert->d = 0.0f ;
			*pvNextNorm = vNormal ;
			//
			pvNextVert ++ ;
			pvNextNorm ++ ;
		}
	}
}

// 円柱
//////////////////////////////////////////////////////////////////////////////
void S3DStandardMeshController::AddMeshCylinder( S3DVector4 * pvVertex, S3DVector4 * pvNormal )
{
	const size_t	nHorzCount = (size_t) m_nHorzDiv + 1 ;
	const size_t	nVertCount = (size_t) m_nVertDiv + 1 ;
	//
	double	xScale = m_vSize.x * m_fpRadius ;
	double	zScale = m_vSize.z * m_fpRadius ;
	double	radScale = m_degRangeLat * PI / (180.0 * m_nHorzDiv) ;
	double	radOffset = m_degStartLat * PI / 180.0 ;
	double	yScale = m_vSize.y * m_fpRadius * m_degRangeLong
										/ (180.0 * m_nVertDiv) ;
	double	yOffset = m_vSize.y * m_fpRadius
								* (m_degStartLong - 90.0) / 180.0 ;
	//
	for ( size_t i = 0; i < nVertCount; i ++ )
	{
		S3DVector4 *	pvNextVert = pvVertex + (i * nHorzCount) ;
		S3DVector4 *	pvNextNorm = pvNormal + (i * nHorzCount) ;
		float32_t		y = (float32_t) (yScale * i + yOffset) ;
		for ( size_t j = 0; j < nHorzCount; j ++ )
		{
			S3DVector	vPos, vNormal ;
			double	rad = radScale * j + radOffset ;
			vNormal.x = (float32_t) (xScale * cos( rad )) ;
			vNormal.y = 0 ;
			vNormal.z = (float32_t) (zScale * sin( rad )) ;
			vPos = vNormal ;
			vPos.y = y ;
			vNormal.Normalize() ;
			//
			if ( m_flagBackFace )
			{
				vNormal = - vNormal ;
			}
			*pvNextVert = vPos ;
			*pvNextNorm = vNormal ;
			//
			pvNextVert ++ ;
			pvNextNorm ++ ;
		}
	}
}

// 球
//////////////////////////////////////////////////////////////////////////////
void S3DStandardMeshController::AddMeshSphere( S3DVector4 * pvVertex, S3DVector4 * pvNormal )
{
	const size_t	nHorzCount = (size_t) m_nHorzDiv + 1 ;
	const size_t	nVertCount = (size_t) m_nVertDiv + 1 ;
	//
	double		radScaleX = m_degRangeLat * PI / (180.0 * m_nHorzDiv) ;
	double		radOffsetX = m_degStartLat * PI / 180.0 ;
	double		radScaleY = m_degRangeLong * PI / (180.0 * m_nVertDiv) ;
	double		radOffsetY = m_degStartLong * PI / 180.0 ;
	S3DVector	vScale = m_vSize * m_fpRadius ;
	//
	for ( size_t i = 0; i < nVertCount; i ++ )
	{
		S3DVector4 *	pvNextVert = pvVertex + (i * nHorzCount) ;
		S3DVector4 *	pvNextNorm = pvNormal + (i * nHorzCount) ;
		double	radY = radScaleY * i + radOffsetY ;
		double	y = - vScale.y * cos( radY ) ;
		double	w = sin( radY ) ;
		for ( size_t j = 0; j < nHorzCount; j ++ )
		{
			S3DVector	vPos, vNormal ;
			double	radX = radScaleX * j + radOffsetX ;
			vNormal.x = (float32_t) (w * vScale.x * cos( radX )) ;
			vNormal.y = (float32_t) y ;
			vNormal.z = (float32_t) (w * vScale.z * sin( radX )) ;
			vPos = vNormal ;
			vNormal.Normalize() ;
			//
			if ( m_flagBackFace )
			{
				vNormal = - vNormal ;
			}
			*pvNextVert = vPos ;
			*pvNextNorm = vNormal ;
			//
			pvNextVert ++ ;
			pvNextNorm ++ ;
		}
	}
}

// トーラス
//////////////////////////////////////////////////////////////////////////////
void S3DStandardMeshController::AddMeshTorus( S3DVector4 * pvVertex, S3DVector4 * pvNormal )
{
	const size_t	nHorzCount = (size_t) m_nHorzDiv + 1 ;
	const size_t	nVertCount = (size_t) m_nVertDiv + 1 ;
	//
	double		radScaleX = m_degRangeLat * PI / (180.0 * m_nHorzDiv) ;
	double		radOffsetX = m_degStartLat * PI / 180.0 ;
	double		radScaleY = m_degRangeLong * PI / (180.0 * m_nVertDiv) ;
	double		radOffsetY = m_degStartLong * PI / 180.0 ;
	S3DVector	vScale = m_vSize * m_fpRadius ;
	//
	for ( size_t i = 0; i < nVertCount; i ++ )
	{
		S3DVector4 *	pvNextVert = pvVertex + (i * nHorzCount) ;
		S3DVector4 *	pvNextNorm = pvNormal + (i * nHorzCount) ;
		double	radY = radScaleY * i + radOffsetY ;
		double	y = - vScale.y * cos( radY ) ;
		double	w = vScale.x * sin( radY ) ;
		for ( size_t j = 0; j < nHorzCount; j ++ )
		{
			S3DVector	vDir, vPos, vNormal ;
			double	radX = radScaleX * j + radOffsetX ;
			vDir.x = (float32_t) cos( radX ) ;
			vDir.y = 0.0f ;
			vDir.z = (float32_t) sin( radX ) ;
			//
			vNormal = vDir * w ;
			vNormal.y = (float32_t) y ;
			vPos = vNormal ;
			vPos += vDir * vScale.z ;
			vNormal.Normalize() ;
			//
			if ( m_flagBackFace )
			{
				vNormal = - vNormal ;
			}
			*pvNextVert = vPos ;
			*pvNextNorm = vNormal ;
			//
			pvNextVert ++ ;
			pvNextNorm ++ ;
		}
	}
}

// 放物線回転体
//////////////////////////////////////////////////////////////////////////////
void S3DStandardMeshController::AddMeshParabola( S3DVector4 * pvVertex, S3DVector4 * pvNormal )
{
	const size_t	nHorzCount = (size_t) m_nHorzDiv + 1 ;
	const size_t	nVertCount = (size_t) m_nVertDiv + 1 ;
	//
	double		radScale = m_degRangeLat * PI / (180.0 * m_nHorzDiv) ;
	double		radOffset = m_degStartLat * PI / 180.0 ;
	double		xScale = m_degRangeLong / (180.0 * m_nVertDiv) ;
	double		xOffset = m_degStartLong / 180.0 ;
	S3DVector	vScale = m_vSize * m_fpRadius ;
	//
	for ( size_t i = 0; i < nVertCount; i ++ )
	{
		S3DVector4 *	pvNextVert = pvVertex + (i * nHorzCount) ;
		S3DVector4 *	pvNextNorm = pvNormal + (i * nHorzCount) ;
		double	x = xScale * (m_nVertDiv - i) + xOffset ;
		double	y = x * x ;
		for ( size_t j = 0; j < nHorzCount; j ++ )
		{
			S3DVector	vDir, vPos, vNormal ;
			double	rad = radScale * j + radOffset ;
			vDir.x = (float32_t) cos( rad ) ;
			vDir.y = 0.0f ;
			vDir.z = (float32_t) sin( rad ) ;
			vPos.x = (float32_t) (vScale.x * x * vDir.x) ;
			vPos.y = (float32_t) (vScale.y * -y) ;
			vPos.z = (float32_t) (vScale.z * x * vDir.z) ;
			vNormal.x = (float32_t) (vScale.y * x * vDir.x) ;
			vNormal.y = 1.0f ;
			vNormal.z = (float32_t) (vScale.y * x * vDir.z) ;
			vNormal.Normalize() ;
			//
			if ( m_flagBackFace )
			{
				vNormal = - vNormal ;
			}
			*pvNextVert = vPos ;
			*pvNextNorm = vNormal ;
			//
			pvNextVert ++ ;
			pvNextNorm ++ ;
		}
	}
}



//////////////////////////////////////////////////////////////////////////////
// モデル参照メッシュ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DModelRefMeshController, MeshController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DModelRefMeshController, ref_model_mesh )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DModelRefMeshController::S3DModelRefMeshController( void )
	: MeshController( m_ItemClassDescriptor.pwszClassID ),
		m_pRefModel( NULL ), m_pmgRefMesh( NULL ),
		m_matTransform( 1, 1, 1 ), m_matRotate( 1, 1, 1 ),
		m_vZoom( 1, 1, 1 ), m_vOffset( 0, 0, 0 ), m_iOutBuffer( 0 )
{
	PrepareParameterEntryCount( paramCount ) ;
	AddParameterEntry
		( L"ref_model", S3DSceneComposer::typeCommand,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrDynamicValidation,
			L"参照モデル", NULL ) ;
	AddParameterEntry
		( L"ref_mesh", S3DSceneComposer::typeCommand,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration,
			L"参照メッシュ", NULL ) ;
	AddParameterEntry
		( L"out_buffer", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"出力バッファ番号", NULL ) ;
	AddParameterEntry
		( L"rotate", S3DSceneComposer::typeRotation,
			S3DSceneComposer::attrConstant,
			L"回転", NULL ) ;
	AddParameterEntry
		( L"zoom", S3DSceneComposer::typeZoom,
			S3DSceneComposer::attrConstant,
			L"拡大率", NULL ) ;
	AddParameterEntry
		( L"position", S3DSceneComposer::typePosition,
			S3DSceneComposer::attrConstant,
			L"頂点オフセット", NULL ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DModelRefMeshController::~S3DModelRefMeshController( void )
{
}

// 参照モデル更新
//////////////////////////////////////////////////////////////////////////////
void S3DModelRefMeshController::UpdateRefModel( void )
{
	m_pRefModel = NULL ;
	m_pmgRefMesh = NULL ;
	//
	if ( m_strRefModel.IsEmpty() )
	{
		return ;
	}
	S3DSceneComposer *	pComposer = GetComposer() ;
	if ( pComposer == NULL )
	{
		return ;
	}
	m_pRefModel = pComposer->GetAssets().GetModelAs( m_strRefModel ) ;
	if ( m_pRefModel == NULL )
	{
		return ;
	}
	m_pmgRefMesh = m_pRefModel->GetMeshGroupAs( m_strRefMesh ) ;
}

// 行列更新
//////////////////////////////////////////////////////////////////////////////
void S3DModelRefMeshController::UpdateTransformation( void )
{
	m_matTransform = m_matRotate * S3DMatrix( m_vZoom ) ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DModelRefMeshController::GetMatrixParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRotate:
		return	m_matRotate ;
	}
	return	S3DDMatrix( 1, 1, 1 ) ;
}

S3DDVector S3DModelRefMeshController::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramZoom:
		return	m_vZoom ;
	case	paramOffset:
		return	m_vOffset ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

int32_t S3DModelRefMeshController::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramOutBuffer:
		return	m_iOutBuffer ;
	}
	return	0 ;
}

const wchar_t * S3DModelRefMeshController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramReferenceModel:
		return	m_strRefModel ;
	case	paramReferenceMesh:
		return	m_strRefMesh ;
	}
	return	NULL ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DModelRefMeshController::SetMatrixParameter( size_t i, const S3DDMatrix& mat )
{
	switch ( i )
	{
	case	paramRotate:
		m_matRotate = mat ;
		UpdateTransformation() ;
		return ;
	}
}

void S3DModelRefMeshController::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramZoom:
		m_vZoom = vec ;
		UpdateTransformation() ;
		return ;
	case	paramOffset:
		m_vOffset = vec ;
		return ;
	}
}

void S3DModelRefMeshController::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramOutBuffer:
		m_iOutBuffer = n ;
		return ;
	}
}

void S3DModelRefMeshController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramReferenceModel:
		if ( m_strRefModel != pwszCmd )
		{
			m_strRefModel = pwszCmd ;
			UpdateRefModel() ;
		}
		return ;

	case	paramReferenceMesh:
		if ( m_strRefMesh != pwszCmd )
		{
			m_strRefMesh = pwszCmd ;
			UpdateRefModel() ;
		}
		return ;
	}
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DModelRefMeshController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	switch ( i )
	{
	case	paramReferenceModel:
		{
			S3DSceneComposer *	pComposer = GetComposer() ;
			if ( pComposer != NULL )
			{
				pComposer->GetAssets().EnumerateResourceIDsAs
						( aStrSet, S3DSceneComposer::resourceTypeModel ) ;
			}
		}
		return	true ;

	case	paramReferenceMesh:
		if ( m_pRefModel != NULL )
		{
			for ( size_t i = 0; i < m_pRefModel->GetMeshGroupList().GetLength(); i ++ )
			{
				const SString *
					pstrID = m_pRefModel->GetMeshGroupList().GetTagAt( i ) ;
				if ( pstrID != NULL )
				{
					aStrSet.Add( new SString( *pstrID ) ) ;
				}
			}
		}
		return	true ;
	}
	return	false ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DModelRefMeshController::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags )
{
	uint32_t	nResFlags =
			MeshController::UpdatePropertyReference( comp, pItem, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefResource )
	{
		UpdateRefModel() ;
		//
		nResFlags |= S3DSceneComposer::updateRefResource ;
	}
	return	nResFlags ;
}

// メッシュ追加処理（全視点・ビュー共通処理）
//////////////////////////////////////////////////////////////////////////////
void S3DModelRefMeshController::AddMesh
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem,
		S3DVertexBufferInterface ** ppVBs, size_t nVBCount )
{
	if ( (size_t) m_iOutBuffer >= nVBCount )
	{
		return ;
	}
	S3DVertexBufferInterface *	pVBO = ppVBs[m_iOutBuffer] ;
	if ( pVBO == NULL )
	{
		return ;
	}
	if ( (m_pRefModel == NULL) || (m_pmgRefMesh == NULL) )
	{
		return ;
	}
	for ( size_t iMesh = 0; iMesh < m_pmgRefMesh->m_nMeshCount; iMesh ++ )
	{
		S3DModelData::MeshObject *	pmoMesh =
			m_pRefModel->GetMeshObjectAt( m_pmgRefMesh->m_iFirstMesh + iMesh ) ;
		if ( pmoMesh == NULL )
		{
			continue ;
		}
		const S3DVector4 *	pvVertex =
				m_pRefModel->GetVertexBufferAt( pmoMesh->m_iVertex ) ;
		const S3DVector4 *	pvNormal =
				m_pRefModel->GetNormalBufferAt( pmoMesh->m_iNormal ) ;
		//
		S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
		pVBO->AllocatePrimitiveBuffer
			( prmbuf, pmoMesh->m_typeMesh,
				pmoMesh->m_bufIndex.GetLength(),
				pmoMesh->m_countVertex ) ;
		//
		m_matTransform.RevolveVectors
			( prmbuf.pvVertex, pvVertex,
				pmoMesh->m_countVertex, m_vOffset ) ;
		//
		S3DVector	vZero( 0, 0, 0 ) ;
		m_matRotate.RevolveVectors
			( prmbuf.pvNormal, pvNormal,
				pmoMesh->m_countVertex, vZero ) ;
		//
		if ( pmoMesh->m_bufUVMap.GetLength() >= pmoMesh->m_countVertex )
		{
			eslCopyMemory
				( prmbuf.pvUVMap,
					pmoMesh->m_bufUVMap.GetConstArray(),
					pmoMesh->m_countVertex * sizeof(S2DVector) ) ;
		}
		else
		{
			eslFillMemory
				( prmbuf.pvUVMap, 0,
					pmoMesh->m_countVertex * sizeof(S2DVector) ) ;
		}
		if ( pmoMesh->m_bufColor.GetLength() >= pmoMesh->m_countVertex )
		{
			eslCopyMemory
				( prmbuf.pColor,
					pmoMesh->m_bufColor.GetConstArray(),
					pmoMesh->m_countVertex * sizeof(S3DColor) ) ;
		}
		else
		{
			S3DColor	clrDummy( 0xFFFFFFFF, 0 ) ;
			for ( size_t i = 0; i < pmoMesh->m_countVertex; i ++ )
			{
				prmbuf.pColor[i] = clrDummy ;
			}
		}
		//
		eslCopyMemory
			( prmbuf.pIndexedList,
				pmoMesh->m_bufIndex.GetConstArray(),
				pmoMesh->m_bufIndex.GetLength() * sizeof(uint32_t) ) ;
		//
		pVBO->AddPrimitiveBuffer
			( NULL, 0, pmoMesh->m_typeMesh, prmbuf,
				pmoMesh->m_bufIndex.GetLength(), pmoMesh->m_countVertex ) ;
	}
}

// フレーム描画前処理（全視点・ビュー共通処理）
//////////////////////////////////////////////////////////////////////////////
void S3DModelRefMeshController::UpdateMesh
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem,
		S3DVertexBufferInterface ** ppVBs, size_t nVBCount )
{
}



//////////////////////////////////////////////////////////////////////////////
// 段階状密度平面メッシュ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DCascadePlaneMeshController, MeshController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DCascadePlaneMeshController, cascade_plane_mesh )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DCascadePlaneMeshController::S3DCascadePlaneMeshController( void )
	: MeshController( m_ItemClassDescriptor.pwszClassID ),
		m_vCenter( 0, 0, 0 ),
		m_fpUnit( 1.0 ), m_nDivision( 4 ), m_nCascade( 4 )
{
	PrepareParameterEntryCount( paramCount ) ;
	ESLVerify( paramCenter == AddParameterEntry
		( L"position", S3DSceneComposer::typePosition,
			S3DSceneComposer::attrConstant,
			L"中心座標", NULL ) ) ;
	ESLVerify( paramUnit == AddParameterEntry
		( L"unit_scale", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant,
			L"単位スケール", NULL ) ) ;
	ESLVerify( paramDivision == AddParameterEntry
		( L"division", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"分割数", NULL ) ) ;
	ESLVerify( paramCascade == AddParameterEntry
		( L"cascade", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"拡張反復数", NULL ) ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DCascadePlaneMeshController::~S3DCascadePlaneMeshController( void )
{
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DCascadePlaneMeshController::GetVectorParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramCenter:
		return	m_vCenter ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DCascadePlaneMeshController::GetScalarParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramUnit:
		return	m_fpUnit ;
	}
	return	0.0 ;
}

int32_t S3DCascadePlaneMeshController::GetIntegerParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramDivision:
		return	(int32_t) m_nDivision ;
	case	paramCascade:
		return	(int32_t) m_nCascade ;
	}
	return	0 ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DCascadePlaneMeshController::SetVectorParameter( size_t iParam, const S3DDVector& vec )
{
	switch ( iParam )
	{
	case	paramCenter:
		m_vCenter = vec ;
		return ;
	}
}

void S3DCascadePlaneMeshController::SetScalarParameter( size_t iParam, double s )
{
	switch ( iParam )
	{
	case	paramUnit:
		m_fpUnit = s ;
		return ;
	}
}

void S3DCascadePlaneMeshController::SetIntegerParameter( size_t iParam, int32_t n )
{
	switch ( iParam )
	{
	case	paramDivision:
		m_nDivision = (size_t) n ;
		return ;
	case	paramCascade:
		m_nCascade = (size_t) n ;
		return ;
	}
}

// メッシュ追加処理（全視点・ビュー共通処理）
//////////////////////////////////////////////////////////////////////////////
void S3DCascadePlaneMeshController::AddMesh
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem,
		S3DVertexBufferInterface ** ppVBs, size_t nVBCount )
{
	if ( (nVBCount == 0) || (ppVBs[0] == nullptr) )
	{
		return ;
	}
	if ( m_nDivision == 0 )
	{
		return ;
	}
	const size_t	nDivision = (size_t) esl_min( (int)m_nDivision, 0x1000 ) ;
	const size_t	nInnerVertex = (nDivision + 1) * (nDivision + 1) ;
	const size_t	nInnerTriangle = nDivision * nDivision * 2 ;
	const size_t	nTotalVertex = nInnerVertex + m_nCascade * 16 ;
	const size_t	nTotalTriangle = nInnerTriangle
						+ ((m_nCascade >= 1) ? (nDivision + 4) * 4 : 0)
						+ ((m_nCascade >= 2) ? (m_nCascade - 1) * 32 : 0) ;
	const size_t	nTotalSize = (size_t) 1 << m_nCascade ;
	const double	fpTotalScale = (double) nTotalSize ;
	//
	// バッファ確保
	//
	S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
	ppVBs[0]->AllocatePrimitiveBuffer
		( prmbuf, primitiveTriangle, nTotalTriangle * 3, nTotalVertex ) ;
	//
	// 中心部分メッシュ
	//
	double	fpStride = m_fpUnit * 2.0 / (double) nDivision ;
	for ( size_t y = 0; y <= nDivision; y ++ )
	{
		for ( size_t x = 0; x <= nDivision; x ++ )
		{
			const size_t	i = y * (nDivision + 1) + x ;
			S3DVector4&		v = prmbuf.pvVertex[i] ;
			v.x = (float32_t) ((double) x * fpStride - m_fpUnit) ;
			v.z = (float32_t) (m_fpUnit - (double) y * fpStride) ;
			v.y = 0.0f ;
			v.d = 0.0f ;
		}
	}
	for ( size_t y = 0; y < nDivision; y ++ )
	{
		for ( size_t x = 0; x < nDivision; x ++ )
		{
			const size_t	i = y * (nDivision + 1) + x ;
			const size_t	j = (y * nDivision + x) * 2 ;
			uint32_t *		pIndex = prmbuf.pIndexedList + j * 3 ;
			pIndex[0] = (uint32_t) i ;
			pIndex[1] = (uint32_t) (i + (nDivision + 1)) ;
			pIndex[2] = (uint32_t) (i + (nDivision + 1) + 1) ;
			pIndex[3] = (uint32_t) i ;
			pIndex[4] = (uint32_t) (i + (nDivision + 1) + 1) ;
			pIndex[5] = (uint32_t) i + 1 ;
		}
	}
	//
	// 四辺情報
	//
	SArray<size_t>	bufLastSideIndex ;
	const size_t	nMaxLastSideCount = (size_t) esl_max((int)nDivision,4) + 1 ;
	size_t *		pLastSideIndex[4] ;
	size_t			nLastSideCount = nDivision + 1 ;
	pLastSideIndex[0] = bufLastSideIndex.GetArray( nMaxLastSideCount * 4 ) ;
	pLastSideIndex[1] = pLastSideIndex[0] + nMaxLastSideCount ;
	pLastSideIndex[2] = pLastSideIndex[1] + nMaxLastSideCount ;
	pLastSideIndex[3] = pLastSideIndex[2] + nMaxLastSideCount ;
	//
	const SGLPoint	ptCorner[4] =
	{
		SGLPoint( 0, 0 ), SGLPoint( (int) nDivision, 0 ),
		SGLPoint( (int) nDivision, (int) nDivision ), SGLPoint( 0, (int) nDivision )
	} ;
	static const SGLPoint	s_ptMoveDir[4] =
	{
		SGLPoint( 1, 0 ), SGLPoint( 0, 1 ),
		SGLPoint( -1, 0 ), SGLPoint( 0, -1 )
	} ;
	for ( int i = 0; i < 4; i ++ )
	{
		SGLPoint	ptSide = ptCorner[i] ;
		for ( size_t j = 0; j < nLastSideCount; j ++ )
		{
			pLastSideIndex[i][j] = (size_t) (ptSide.y * (nDivision + 1) + ptSide.x) ;
			ptSide += s_ptMoveDir[i] ;
		}
	}
	//
	// 周辺延長メッシュ
	//
	static const S3DVector	s_vCorner[4] =
	{
		S3DVector( -1, 0, 1 ), S3DVector( 1, 0, 1 ),
		S3DVector( 1, 0, -1 ), S3DVector( -1, 0, -1 ),
	} ;
	static const S3DVector	s_vMoveDir[4] =
	{
		S3DVector( 1, 0, 0 ), S3DVector( 0, 0, -1 ),
		S3DVector( -1, 0, 0 ), S3DVector( 0, 0, 1 ),
	} ;
	size_t	iNextVertex = (nDivision + 1) * (nDivision + 1) ;
	size_t	iNextIndex = nDivision * nDivision * 2 * 3 ;
	double	fpUnit = m_fpUnit * 2.0 ;
	size_t	iNextSideIndex[4][5] ;
	for ( size_t iCascade = 0; iCascade < m_nCascade; iCascade ++ )
	{
		//
		// 外周辺頂点追加
		//
		for ( int i = 0; i < 4; i ++ )
		{
			S3DVector4	vSide = s_vCorner[i] * fpUnit ;
			for ( int j = 0; j < 4; j ++ )
			{
				const size_t	vi = iNextVertex + i * 4 + j ;
				prmbuf.pvVertex[vi] = vSide ;
				iNextSideIndex[i][j] = vi ;
				vSide += s_vMoveDir[i] * (fpUnit * 0.5) ;
			}
			iNextSideIndex[i][4] = iNextVertex + ((i * 4 + 4) % 16) ;
		}
		iNextVertex += 16 ;
		//
		// 外側面追加
		//
		for ( int i = 0; i < 4; i ++ )
		{
			prmbuf.pIndexedList[iNextIndex++] = (uint32_t) iNextSideIndex[i][0] ;
			prmbuf.pIndexedList[iNextIndex++] = (uint32_t) pLastSideIndex[i][0] ;
			prmbuf.pIndexedList[iNextIndex++] = (uint32_t) iNextSideIndex[i][1] ;
			//
			double	fpExHalf = -0.5 ;
			size_t	iExLast = 1 ;
			size_t	iExNext = 2 ;
			size_t	iInLast = 0 ;
			size_t	iInNext = 1 ;
			while ( iInNext < nLastSideCount )
			{
				double	x = (double) iInNext * 2.0 / (nLastSideCount - 1) - 1.0 ;
				if ( x > fpExHalf )
				{
					prmbuf.pIndexedList[iNextIndex++] = (uint32_t) iNextSideIndex[i][iExLast] ;
					prmbuf.pIndexedList[iNextIndex++] = (uint32_t) pLastSideIndex[i][iInLast] ;
					prmbuf.pIndexedList[iNextIndex++] = (uint32_t) iNextSideIndex[i][iExNext] ;
					iExLast = iExNext ;
					iExNext ++ ;
					ESLAssert( iExNext <= 4 ) ;
					fpExHalf = ((double) (iExLast + iExNext) - 4.0) * 0.5 ;
				}
				else
				{
					prmbuf.pIndexedList[iNextIndex++] = (uint32_t) iNextSideIndex[i][iExLast] ;
					prmbuf.pIndexedList[iNextIndex++] = (uint32_t) pLastSideIndex[i][iInLast] ;
					prmbuf.pIndexedList[iNextIndex++] = (uint32_t) pLastSideIndex[i][iInNext] ;
					iInLast = iInNext ;
					iInNext ++ ;
				}
			}
			//
			prmbuf.pIndexedList[iNextIndex++] = (uint32_t) iNextSideIndex[i][iExLast] ;
			prmbuf.pIndexedList[iNextIndex++] = (uint32_t) pLastSideIndex[i][iInLast] ;
			prmbuf.pIndexedList[iNextIndex++] = (uint32_t) iNextSideIndex[i][iExNext] ;
		}
		//
		// 四辺情報を内側へコピー
		//
		for ( int i = 0; i < 4; i ++ )
		{
			for ( size_t j = 0; j < 5; j ++ )
			{
				pLastSideIndex[i][j] = iNextSideIndex[i][j] ;
			}
		}
		nLastSideCount = 5 ;
		//
		fpUnit *= 2.0 ;
	}
	ESLAssert( iNextVertex == nTotalVertex ) ;
	ESLAssert( iNextIndex == nTotalTriangle * 3 ) ;

	//
	// 法線・UV・頂点色設定
	//
	S3DVector4	vNormal( 0.0f, -1.0f, 0.0f, 0.0f ) ;
	S3DColor	color( 0xFFFFFFFF, 0 ) ;
	float32_t	fpUVScale = 1.0f / (float32_t) fpUnit ;
	for ( size_t i = 0; i < nTotalVertex; i ++ )
	{
		const S3DVector4&	v = prmbuf.pvVertex[i] ;
		S2DVector&			uv = prmbuf.pvUVMap[i] ;
		prmbuf.pvNormal[i] = vNormal ;
		prmbuf.pColor[i] = color ;
		uv.x = v.x * fpUVScale + 0.5f ;
		uv.y = v.z * fpUVScale + 0.5f ;
	}
	//
	// メッシュ出力
	//
	ppVBs[0]->AddPrimitiveBuffer
		( nullptr, 0, primitiveTriangle,  prmbuf, iNextIndex, iNextVertex ) ;
}

// フレーム描画前処理（全視点・ビュー共通処理）
//////////////////////////////////////////////////////////////////////////////
void S3DCascadePlaneMeshController::UpdateMesh
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem,
		S3DVertexBufferInterface ** ppVBs, size_t nVBCount )
{
}



//////////////////////////////////////////////////////////////////////////////
// 木メッシュ生成
//////////////////////////////////////////////////////////////////////////////

const wchar_t *	S3DTreeMeshBuilderController::s_pwszLeafShape[S3DTreeMeshBuilderController::leafShapeCount] =
{
	L"plane", L"pyramid", L"both_pyramid"
} ;

const SSystem::SXMLDocument::AttrInteger
	S3DTreeMeshBuilderController::s_aiLeafShape
			[S3DTreeMeshBuilderController::leafShapeCount+1] =
{
	{ L"plane", S3DTreeMeshBuilderController::leafPlane },
	{ L"pyramid", S3DTreeMeshBuilderController::leafPyramid },
	{ L"both_pyramid", S3DTreeMeshBuilderController::leafBothPyramid },
	{ nullptr, 0 },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DTreeMeshBuilderController, MeshController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DTreeMeshBuilderController, treee_mesh_builder )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DTreeMeshBuilderController::S3DTreeMeshBuilderController( void )
	: MeshController( m_ItemClassDescriptor.pwszClassID ),
		m_nRandomSeed( 1 ),
		m_flagNoBranchMesh( false ),
		m_fpRootThickness( 0.2 ),
		m_fpLeafThickness( 0.02 ),
		m_fpSegmentLength( 1.5 ),
		m_fpLeafWidth( 1.0 ),
		m_fpLeafHeight( 1.0 ),
		m_fpLeafSizeRandom( 0.2 ),
		m_rgbLeafBaseColor( 0x808080 ),
		m_fpTrunkVScale( 3.0 ),
		m_fpTrunkShrink( 0.7 ),
		m_fpBranchShrink( 0.4 ),
		m_nRootTrunkLength( 2 ),
		m_nTrunkDivCount( 12 ),
		m_nBranchCount( 3 ),
		m_nBranchRandom( 2 ),
		m_degTrunkAngle( 10 ),
		m_degBranchAngle( 60 ),
		m_degBranchRndAngle( 20 ),
		m_shapeLeaf( leafPlane ),
		m_shapeTopLeaf( leafPlane ),
		m_fpLeafSwell( 1.0 ),
		m_degTopLeafAngle( 20.0 ),
		m_nTipLeafCount( 4 ),
		m_nTopLeafCount( 2 ),
		m_degLeafHangAngle( 0 ),
		m_degLeafRndAngle( 30 ),
		m_degLeafFaceAngle( 30 )
{
	PrepareParameterEntryCount( paramCount ) ;
	AddParameterEntry
		( L"random_seed", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"乱数種", NULL ) ;
	AddParameterEntry
		( L"no_branch_mesh", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"幹・枝を生成しない", L"幹・枝メッシュを生成しない" ) ;
	AddParameterEntry
		( L"root_thickness", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"幹の太さ", L"根元の幹の太さ", 0.01, 2.0 ) ;
	AddParameterEntry
		( L"leaf_thickness", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"葉の茎太さ", L"分割した枝が葉になる枝の太さ", 0.001, 0.5 ) ;
	AddParameterEntry
		( L"seg_length", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"枝の分割区間長", L"幹・枝が分割する区間の長さ", 0.1, 5.0 ) ;
	AddParameterEntry
		( L"leaf_width", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"葉の幅", L"葉の板ポリゴンの幅", 0.01, 1.0 ) ;
	AddParameterEntry
		( L"leaf_height", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"葉の長さ", L"葉の板ポリゴンの長さ（テクスチャのｙ方向に対応）", 0.01, 1.0 ) ;
	AddParameterEntry
		( L"leaf_size_random", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"葉のサイズ揺らぎ比", L"葉のサイズの乗じる乱数比率（1.0±ｘ）", 0.0, 1.0 ) ;
	AddParameterEntry
		( L"leaf_base_color", S3DSceneComposer::typeColor,
			S3DSceneComposer::attrConstant,
			L"葉の付け根色", L"葉の付け根側の乗算色" ) ;
	AddParameterEntry
		( L"trunk_v_scale", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"幹テクスチャＶスケール",
			L"幹テクスチャの高さに対応するモデル空間長", 0.1, 10.0 ) ;
	AddParameterEntry
		( L"trunk_shrink", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"幹太さ減衰率",
			L"幹から枝が分割する場合の減衰率（拡大率）", 0.01, 1.0 ) ;
	AddParameterEntry
		( L"branch_shrink", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"枝太さ減衰率",
			L"幹から枝が分割する場合の枝の太さ率", 0.001, 1.0 ) ;
	AddParameterEntry
		( L"root_trunk_length", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"根本幹の長さ", L"枝を分岐させない根元の幹の節の数" ) ;
	AddParameterEntry
		( L"trunk_div_count", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"幹周り分割", L"幹・枝周りの頂点分割数" ) ;
	AddParameterEntry
		( L"branch_count", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"枝分岐数", L"分岐する枝の数（最低数）" ) ;
	AddParameterEntry
		( L"branch_random", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"枝分岐乱数", L"分岐する枝の追加的な乱数" ) ;
	AddParameterEntry
		( L"trunk_angle", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"幹の曲がり角",
			L"幹から枝が分岐する点での幹の曲がり角 [deg]", 0.0, 90.0 ) ;
	AddParameterEntry
		( L"branch_angle", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"枝の曲がり角",
			L"幹から枝が分岐する点での枝の曲がり角 [deg]", 0.0, 90.0 ) ;
	AddParameterEntry
		( L"branch_angle_random", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"枝の曲がり角乱数",
			L"幹から枝が分岐する点での枝の曲がり角の乱数幅 [deg]", 0.0, 90.0 ) ;
	AddParameterEntry
		( L"leaf_shape", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration,
			L"葉形状" ) ;
	AddParameterEntry
		( L"top_leaf_shape", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration,
			L"天頂葉形状" ) ;
	AddParameterEntry
		( L"leaf_swell", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"葉の膨らみ率", nullptr, 0.0, 2.0 ) ;
	AddParameterEntry
		( L"top_leaf_angle", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"天頂葉角",
			L"天頂方向に対してこの角度以内の葉は、天頂葉として処理する", 0.0, 90.0 ) ;
	AddParameterEntry
		( L"tip_leaf_count", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"先端の葉枚数", L"枝の先端の葉の枚数" ) ;
	AddParameterEntry
		( L"top_leaf_count", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"天頂の葉枚数", L"天頂方向の先端の葉の枚数" ) ;
	AddParameterEntry
		( L"leaf_hang_angle", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"葉の垂れ角",
			L"葉の垂れさがり角[deg]", 0.0, 90.0 ) ;
	AddParameterEntry
		( L"leaf_angle_random", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"葉の曲がり角乱数",
			L"枝から葉が分岐する点での葉の曲がり角の乱数幅[deg]", 0.0, 90.0 ) ;
	AddParameterEntry
		( L"leaf_face_angle", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"葉の天頂方向遊び角",
			L"葉の表面の天頂方向からの乱数角 [deg]", 0.0, 90.0 ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DTreeMeshBuilderController::~S3DTreeMeshBuilderController( void )
{
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DTreeMeshBuilderController::GetVectorParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramLeafBaseColor:
		return	S3DSceneComposer::Parameter::VectorFromColor( m_rgbLeafBaseColor ) ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DTreeMeshBuilderController::GetScalarParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramRootThickness:
		return	m_fpRootThickness ;
	case	paramLeafThickness:
		return	m_fpLeafThickness ;
	case	paramSegmentLength:
		return	m_fpSegmentLength ;
	case	paramLeafWidth:
		return	m_fpLeafWidth ;
	case	paramLeafHeight:
		return	m_fpLeafHeight ;
	case	paramLeafSizeRandom:
		return	m_fpLeafSizeRandom ;
	case	paramTrunkVScale:
		return	m_fpTrunkVScale ;
	case	paramTrunkShrink:
		return	m_fpTrunkShrink ;
	case	paramBranchShrink:
		return	m_fpBranchShrink ;
	case	paramTrunkAngle:
		return	m_degTrunkAngle ;
	case	paramBranchAngle:
		return	m_degBranchAngle ;
	case	paramBranchRndAngle:
		return	m_degBranchRndAngle ;
	case	paramLeafSwell:
		return	m_fpLeafSwell ;
	case	paramTopLeafAnglel:
		return	m_degTopLeafAngle ;
	case	paramLeafHangAngle:
		return	m_degLeafHangAngle ;
	case	paramLeafRndAngle:
		return	m_degLeafRndAngle ;
	case	paramLeafFaceAngle:
		return	m_degLeafFaceAngle ;
	}
	return	0.0 ;
}

int32_t S3DTreeMeshBuilderController::GetIntegerParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramRandomSeed:
		return	m_nRandomSeed ;
	case	paramRootTrunkLength:
		return	m_nRootTrunkLength ;
	case	paramTrunkDivCount:
		return	m_nTrunkDivCount ;
	case	paramBranchCount:
		return	m_nBranchCount ;
	case	paramBranchRandom:
		return	m_nBranchRandom ;
	case	paramTipLeafCount:
		return	m_nTipLeafCount ;
	case	paramTopLeafCount:
		return	m_nTopLeafCount ;
	}
	return	0 ;
}

bool S3DTreeMeshBuilderController::GetBooleanParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramNoBranchMesh:
		return	m_flagNoBranchMesh ;
	}
	return	false ;
}

const wchar_t * S3DTreeMeshBuilderController::GetCommandParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramLeafShape:
		return	s_pwszLeafShape[m_shapeLeaf] ;

	case	paramTopLeafShape:
		return	s_pwszLeafShape[m_shapeTopLeaf] ;
	}
	return	nullptr ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DTreeMeshBuilderController::SetVectorParameter( size_t iParam, const S3DDVector& vec )
{
	switch ( iParam )
	{
	case	paramLeafBaseColor:
		m_rgbLeafBaseColor = S3DSceneComposer::Parameter::ColorFromVector( vec ) ;
		break ;
	}
}

void S3DTreeMeshBuilderController::SetScalarParameter( size_t iParam, double s )
{
	switch ( iParam )
	{
	case	paramRootThickness:
		m_fpRootThickness  = s ;
		return ;
	case	paramLeafThickness:
		m_fpLeafThickness = s ;
		return ;
	case	paramSegmentLength:
		m_fpSegmentLength = s ;
		return ;
	case	paramLeafWidth:
		m_fpLeafWidth = s ;
		return ;
	case	paramLeafHeight:
		m_fpLeafHeight = s ;
		return ;
	case	paramLeafSizeRandom:
		m_fpLeafSizeRandom = s ;
		return ;
	case	paramTrunkVScale:
		m_fpTrunkVScale = s ;
		return ;
	case	paramTrunkShrink:
		m_fpTrunkShrink = s ;
		return ;
	case	paramBranchShrink:
		m_fpBranchShrink = s ;
		return ;
	case	paramTrunkAngle:
		m_degTrunkAngle = s ;
		return ;
	case	paramBranchAngle:
		m_degBranchAngle = s ;
		return ;
	case	paramBranchRndAngle:
		m_degBranchRndAngle = s ;
		return ;
	case	paramLeafSwell:
		m_fpLeafSwell = s ;
		return ;
	case	paramTopLeafAnglel:
		m_degTopLeafAngle = s ;
		return ;
	case	paramLeafHangAngle:
		m_degLeafHangAngle = s ;
		return ;
	case	paramLeafRndAngle:
		m_degLeafRndAngle = s;
		return ;
	case	paramLeafFaceAngle:
		m_degLeafFaceAngle = s ;
		return ;
	}
}

void S3DTreeMeshBuilderController::SetIntegerParameter( size_t iParam, int32_t n )
{
	switch ( iParam )
	{
	case	paramRandomSeed:
		m_nRandomSeed  = n ;
		return ;
	case	paramRootTrunkLength:
		m_nRootTrunkLength = n ;
		return ;
	case	paramTrunkDivCount:
		m_nTrunkDivCount = n ;
		return ;
	case	paramBranchCount:
		m_nBranchCount = n ;
		return ;
	case	paramBranchRandom:
		m_nBranchRandom = n ;
		return ;
	case	paramTipLeafCount:
		m_nTipLeafCount = n ;
		return ;
	case	paramTopLeafCount:
		m_nTopLeafCount = n ;
		return ;
	}
}

void S3DTreeMeshBuilderController::SetBooleanParameter( size_t iParam, bool b )
{
	switch ( iParam )
	{
	case	paramNoBranchMesh:
		m_flagNoBranchMesh = b ;
		return ;
	}
}

void S3DTreeMeshBuilderController::SetCommandParameter( size_t iParam, const wchar_t * pwszCmd )
{
	switch ( iParam )
	{
	case	paramLeafShape:
		m_shapeLeaf = (LeafShape)
			SXMLDocument::GetIntegerAsSymbolOf( s_aiLeafShape, pwszCmd, m_shapeLeaf ) ;
		return ;

	case	paramTopLeafShape:
		m_shapeTopLeaf = (LeafShape)
			SXMLDocument::GetIntegerAsSymbolOf( s_aiLeafShape, pwszCmd, m_shapeTopLeaf ) ;
		return ;
	}
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DTreeMeshBuilderController::EnumerateStringSet
	( size_t iParam, SSystem::SStringArray& aStrSet )
{
	switch ( iParam )
	{
	case	paramLeafShape:
	case	paramTopLeafShape:
		{
			for ( int j = 0; j < leafShapeCount; j ++ )
			{
				aStrSet.Add( new SString(s_pwszLeafShape[j]) ) ;
			}
		}
		return	true ;
	}
	return	false ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DTreeMeshBuilderController::GetParameterCategoryName( size_t iCategory ) const
{
	return	L"木生成" ;
}

// メッシュ追加処理（全視点・ビュー共通処理）
//////////////////////////////////////////////////////////////////////////////
void S3DTreeMeshBuilderController::AddMesh
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem,
		S3DVertexBufferInterface ** ppVBs, size_t nVBCount )
{
	if ( (nVBCount < 2)
		|| (ppVBs[0] == NULL) || (ppVBs[1] == NULL) )
	{
		return ;
	}
	if ( (m_nTrunkDivCount < 2)
		|| (m_fpLeafThickness <= 0.0) || (m_fpSegmentLength <= 0.0)
		|| (fabs(m_fpTrunkShrink) >= 1.0) || (fabs(m_fpBranchShrink) >= 1.0)
		|| (m_nBranchCount < 0) || (m_nBranchRandom < 0) )
	{
		return ;
	}
	m_random.InitializeSeedBy( (uint32_t) m_nRandomSeed ) ;
	m_nSegCount = 0 ;
	//
	S3DMatrix	matTree( 1, 0, 0,
						 0, 0, -1,
						 0, 1, 0 ) ;
	S3DVector	vTreeOrg( 0, 0, 0 ) ;
	BuildBranch
		( ppVBs, nVBCount, matTree, matTree, vTreeOrg, 0.0f,
			(float32_t) m_fpRootThickness,
			(float32_t) m_fpSegmentLength, 0, 1 ) ;
}

// フレーム描画前処理（全視点・ビュー共通処理）
//////////////////////////////////////////////////////////////////////////////
void S3DTreeMeshBuilderController::UpdateMesh
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem,
		S3DVertexBufferInterface ** ppVBs, size_t nVBCount )
{
}

// 枝発生
//////////////////////////////////////////////////////////////////////////////
void S3DTreeMeshBuilderController::BuildBranch
	( S3DVertexBufferInterface ** ppVBs, size_t nVBCount,
		const S3DMatrix& matBuild0, const S3DMatrix& matBuild1,
		S3DVector& vPos0, float32_t vCoord0,
		float32_t fpThickness0, float32_t fpLength,
		int nBranch, int nNestCount )
{
	if ( (fpThickness0 <= m_fpLeafThickness) || (++ m_nSegCount >= 100000) )
	{
		BuildLeaf( ppVBs, nVBCount, matBuild1, vPos0,
						(size_t) esl_max( m_nTipLeafCount - 1, 0 ) ) ;
		return ;
	}
	ESLAssert( nVBCount >= 2 ) ;
	ESLAssert( ppVBs[0] != NULL ) ;
	//
	// 枝メッシュ
	//
	S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
	const int32_t	nDivCount = m_nTrunkDivCount ;
	ppVBs[0]->AllocatePrimitiveBuffer
		( prmbuf, primitiveTriangle,
			(size_t) nDivCount * 6, (size_t) (nDivCount + 1) * 2 ) ;
	//
	float32_t	fpThickness1 =
		fpThickness0 * esl_fclampf( (float32_t) m_fpTrunkShrink, 0.0f, 0.9f ) ;
	S3DVector	vPos1 = matBuild1 * S3DVector( 0, 0, fpLength ) + vPos0 ;
	float32_t	vCoord1 = vCoord0 + fpLength * (float32_t) m_fpTrunkVScale ;
	float32_t	fpRcpDivCount = 1.0f / (float32_t) nDivCount ;
	S3DColor	clrDummy( 0xFFFFFFFF, 0 ) ;
	//
	if ( !m_flagNoBranchMesh )
	{
		for ( int32_t i = 0, j = 0; i <= nDivCount; i ++, j += 2 )
		{
			double		u = (float32_t) i * fpRcpDivCount ;
			double		rad = (i == nDivCount) ? 0.0 : (2.0 * PI * u) ;
			double		s = sin( rad ) ;
			double		c = cos( rad ) ;
			S3DVector	v1( c, s, 0 ) ;
			S3DVector	v0 = v1 ;
			matBuild0.RevolveVector( v0 ) ;
			matBuild1.RevolveVector( v1 ) ;
			//
			prmbuf.pvVertex[j]      = vPos0 + v0 * fpThickness0 ;
			prmbuf.pvVertex[j + 1]  = vPos1 + v1 * fpThickness1 ;
			prmbuf.pvNormal[j]      = v0 ;
			prmbuf.pvNormal[j + 1]  = v1 ;
			prmbuf.pvUVMap[j].x     = (float32_t) u ;
			prmbuf.pvUVMap[j].y     = vCoord0 ;
			prmbuf.pvUVMap[j + 1].x = (float32_t) u ;
			prmbuf.pvUVMap[j + 1].y = vCoord1 ;
			prmbuf.pColor[j]        = clrDummy ;
			prmbuf.pColor[j + 1]    = clrDummy ;
		}
		for ( int32_t i = 0, j = 0; i < nDivCount; i ++, j += 6 )
		{
			uint32_t	k = (uint32_t) (i + i) ;
			prmbuf.pIndexedList[j]     = k ;
			prmbuf.pIndexedList[j + 1] = k + 3 ;
			prmbuf.pIndexedList[j + 2] = k + 1 ;
			prmbuf.pIndexedList[j + 3] = k ;
			prmbuf.pIndexedList[j + 4] = k + 2 ;
			prmbuf.pIndexedList[j + 5] = k + 3 ;
		}
		ppVBs[0]->AddPrimitiveBuffer
			( NULL, 0, primitiveTriangle,
					prmbuf, (size_t) nDivCount * 6, (size_t) (nDivCount + 1) * 2 ) ;
	}
	else
	{
		ppVBs[0]->FreePrimitiveBuffer( prmbuf ) ;
	}
	//
	// 次の幹生成
	//
	double	deg2rad = PI / 180.0 ;
	double	radBendDir = m_random.QuickRandomDouble( PI ) ;
	double	radBendTrunk = m_random.QuickRandomFloat
							( (float32_t) (m_degTrunkAngle * deg2rad) ) ;
	//
	S3DMatrix	matBendTrunk = matBuild1 ;
	S3DVector	vBendTrunk
					( -sin(radBendDir) * sin(radBendTrunk),
						cos(radBendDir) * sin(radBendTrunk),
						cos(radBendTrunk) ) ;
	matBendTrunk.RevolveForAngle( vBendTrunk ) ;
	//
	BuildBranch
		( ppVBs, nVBCount, matBuild1, matBendTrunk, vPos1, vCoord1,
			fpThickness1, fpLength * (float32_t) m_fpTrunkShrink,
			nBranch, nNestCount + 1 ) ;
	//
	if ( (nBranch == 0) && (nNestCount < m_nRootTrunkLength) )
	{
		return ;
	}
	//
	// 枝生成
	//
	uint32_t	nBranchCount =
		(uint32_t) esl_clampi( m_nBranchCount
							+ m_random.QuickRandomize
								( (uint32_t) m_nBranchRandom ), 0, 10 ) ;
	float32_t	fpBranchThickness =
		fpThickness0 * esl_fclampf( (float32_t) m_fpBranchShrink, 0.0f, 0.9f ) ;
	double		radBranchStep = (360.0 - m_degTrunkAngle)
									* PI / (180.0 * (nBranchCount + 1)) ;
	double		radBranchOffset = radBendDir + m_degTrunkAngle * PI / 360.0 ;
	//
	for ( uint32_t i = 0; i < nBranchCount; i ++ )
	{
		double	radBranchDir = i * radBranchStep + radBranchOffset
							+ m_random.QuickRandomDouble( radBranchStep * 0.25 ) ;
		double	radBendBranch =
				(m_degBranchAngle
					+ m_random.QuickRandomFloat
						( (float32_t) m_degBranchRndAngle )) * deg2rad ;
		//
		S3DMatrix	matBendBranch = matBuild1 ;
		matBendBranch.RevolveOnZ( sin(radBranchDir), cos(radBranchDir) ) ;
		matBendBranch.RevolveOnX( sin(radBendBranch), cos(radBendBranch) ) ;
		//
		S3DVector	vBendBranch0 = matBendBranch * S3DVector( 0, 0, 1 ) ;
		S3DVector	vBendBranch1 = vBendBranch0 ;
		vBendBranch1.y = vBendBranch1.y * 0.5f - 0.5f ;
		//
		S3DMatrix	matBendBranch2( 1, 1, 1 ) ;
		matBendBranch2.VectorRotationOf( vBendBranch0, vBendBranch1 ) ;
		//
		matBendBranch = matBendBranch2 * matBendBranch ;
		//
		float32_t	fpBranchThickness1 =
						fpBranchThickness
							* (1.0f - m_random.QuickRandomFloat(0.5f)) ;
		if ( fpBranchThickness1 <= m_fpLeafThickness )
		{
			BuildLeaf( ppVBs, nVBCount, matBendBranch, vPos1, 0 ) ;
		}
		else
		{
			BuildBranch
				( ppVBs, nVBCount, matBendBranch, matBendBranch,
					vPos1, vCoord1, fpBranchThickness1,
					fpLength * (float32_t) m_fpTrunkShrink,
					nBranch + 1, nNestCount + 1 ) ;
		}
	}
}

// 葉発生
//////////////////////////////////////////////////////////////////////////////
void S3DTreeMeshBuilderController::BuildLeaf
	( S3DVertexBufferInterface ** ppVBs, size_t nVBCount,
		const S3DMatrix& matBuild, S3DVector& vPos, size_t nAddCount )
{
	S3DVector	vLeafDir = (matBuild * S3DVector( 0, 0, 1 )).Normalized() ;
	double		cosTopLeaf = cos( m_degTopLeafAngle * PI / 180.0 ) ;
	S3DMatrix	matLeafBase = matBuild ;
	LeafShape	shape = m_shapeLeaf ;
	bool		flagTopLeaf = false ;
	if ( cosTopLeaf < - vLeafDir.y )
	{
		// 天頂葉
		double		radRot = m_random.QuickRandomDouble( PI ) ;
		S3DMatrix	matRot( 1, 1, 1 ) ;
		matRot.RevolveOnY( sin(radRot), cos(radRot) ) ;
		matLeafBase = matRot * matLeafBase ;
		shape = m_shapeTopLeaf ;
		nAddCount = (size_t) esl_max( m_nTopLeafCount - 1, 0 ) ;
		flagTopLeaf = true ;
	}
	else
	{
		// 天頂葉ではない
		matLeafBase = S3DMatrix( 1, 1, 1 ) ;
		matLeafBase.RevolveForAngle( vLeafDir ) ;
		//
		double		radHang = m_degLeafHangAngle * PI / 180.0 ;
		if ( vLeafDir.y < 0 )
		{
			radHang *= 1.0 - vLeafDir.y / -cosTopLeaf ;
		}
		matLeafBase.RevolveOnX( sin(radHang), cos(radHang) ) ;
	}
	//
	if ( shape == leafPyramid )
	{
		BuildThickLeaf
			( ppVBs, nVBCount, matLeafBase, vPos, nAddCount, flagTopLeaf ) ;
		return ;
	}
	if ( shape == leafBothPyramid )
	{
		BuildBothThickLeaf
			( ppVBs, nVBCount, matLeafBase, vPos, nAddCount, flagTopLeaf ) ;
		return ;
	}
	//
	ESLAssert( nVBCount >= 2 ) ;
	ESLAssert( ppVBs[1] != NULL ) ;
	//
	// 葉メッシュ準備
	//
	S3DVector	vVertex[4] ;
	S3DVector	vNormal( 0, -1, 0 ) ;
	S2DVector	vUV[4] =
	{
		S2DVector( 0, 1 ),
		S2DVector( 1, 1 ),
		S2DVector( 0, 0 ),
		S2DVector( 1, 0 ),
	} ;
	S3DColor	clrBase( m_rgbLeafBaseColor.ui32 | 0xFF000000, 0 ) ;
	S3DColor	clrTip( 0xFFFFFFFF, 0 ) ;
	uint32_t	nIndex[6] =
	{
		0, 1, 3,  0, 3, 2
	} ;
	//
	vVertex[0].x = (float32_t) m_fpLeafWidth * -0.5f ;
	vVertex[0].y = 0.0f ;
	vVertex[0].z = 0.0f ;
	vVertex[1].x = (float32_t) m_fpLeafWidth * 0.5f ;
	vVertex[1].y = 0.0f ;
	vVertex[1].z = 0.0f ;
	vVertex[2].x = vVertex[0].x ;
	vVertex[2].y = 0.0f ;
	vVertex[2].z = (float32_t) m_fpLeafHeight ;
	vVertex[3].x = vVertex[1].x ;
	vVertex[3].y = 0.0f ;
	vVertex[3].z = (float32_t) m_fpLeafHeight ;
	//
	for ( size_t i = 0; i <= nAddCount; i ++ )
	{
		//
		// 葉の向き補正
		//
		S3DMatrix	matLeaf = matLeafBase ;
		if ( flagTopLeaf )
		{
			double	radLeafZ = m_random.QuickRandomDouble
										( m_degLeafRndAngle * PI / 180.0 ) ;
			radLeafZ += i * 2.0 * PI / (nAddCount + 1) ;
			matLeaf.RevolveOnZ( sin(radLeafZ), cos(radLeafZ) ) ;
		}
		else
		{
			double	radLeafX = m_random.QuickRandomDouble
										( m_degLeafRndAngle * PI / 180.0 ) ;
			double	radLeafY = m_random.QuickRandomDouble
										( m_degLeafRndAngle * PI / 180.0 ) ;
			double	radLeafZ = m_random.QuickRandomDouble
										( m_degLeafFaceAngle * PI / 180.0 ) ;
			//
			matLeaf.RevolveOnX( sin(radLeafX), cos(radLeafX) ) ;
			matLeaf.RevolveOnY( sin(radLeafY), cos(radLeafY) ) ;
			matLeaf.RevolveOnZ( sin(radLeafZ), cos(radLeafZ) ) ;
		}
		//
		// メッシュ追加
		//
		S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
		ppVBs[1]->AllocatePrimitiveBuffer( prmbuf, primitiveTriangle, 6, 4 ) ;
		//
		float32_t	fpScale =
			(float32_t) esl_fmax( 1.0 + m_random.QuickRandomDouble( m_fpLeafSizeRandom ), 0.0 ) ;
		//
		for ( int i = 0; i < 4; i ++ )
		{
			prmbuf.pvVertex[i] = matLeaf * vVertex[i] * fpScale + vPos ;
			prmbuf.pvNormal[i] = matLeaf * vNormal ;
			prmbuf.pvUVMap[i] = vUV[i] ;
		}
		prmbuf.pColor[0] = clrBase ;
		prmbuf.pColor[1] = clrBase ;
		prmbuf.pColor[2] = clrTip ;
		prmbuf.pColor[3] = clrTip ;
		//
		for ( int i = 0; i < 6; i ++ )
		{
			prmbuf.pIndexedList[i] = nIndex[i] ;
		}
		//
		ppVBs[1]->AddPrimitiveBuffer( NULL, 0, primitiveTriangle, prmbuf, 6, 4 ) ;
	}
}

void S3DTreeMeshBuilderController::BuildThickLeaf
	( S3DVertexBufferInterface ** ppVBs, size_t nVBCount,
		const S3DMatrix& matLeafBase, S3DVector& vPos,
		size_t nAddCount, bool flagTopLeaf )
{
	ESLAssert( nVBCount >= 2 ) ;
	ESLAssert( ppVBs[1] != NULL ) ;
	//
	// 葉メッシュ準備
	//
	S3DVector	vVertex[5] ;
	S3DVector	vNormal[5] ;
	S2DVector	vUV[5] =
	{
		S2DVector( 0, 1 ),
		S2DVector( 1, 1 ),
		S2DVector( 0, 0 ),
		S2DVector( 1, 0 ),
		S2DVector( 0.5f, 0.5f ),
	} ;
	S3DColor	clrBase( m_rgbLeafBaseColor.ui32 | 0xFF000000, 0 ) ;
	S3DColor	clrTip( 0xFFFFFFFF, 0 ) ;
	S3DColor	clrCenter
					( sglPackedColorAdd
						( (m_rgbLeafBaseColor.ui32 >> 1)
									& 0x7F7F7F, 0x808080 ) | 0xFF000000, 0 ) ;
	S3DColor	clrVertex[5] =
	{
		clrBase, clrBase, clrTip, clrTip, clrCenter,
	} ;
	uint32_t	nIndex[12] =
	{
		0, 1, 4,  1, 3, 4,  3, 2, 4,  2, 0, 4,
	} ;
	//
	vVertex[0].x = (float32_t) m_fpLeafWidth * -0.5f ;
	vVertex[0].y = 0.0f ;
	vVertex[0].z = 0.0f ;
	vVertex[1].x = (float32_t) m_fpLeafWidth * 0.5f ;
	vVertex[1].y = 0.0f ;
	vVertex[1].z = 0.0f ;
	vVertex[2].x = vVertex[0].x ;
	vVertex[2].y = 0.0f ;
	vVertex[2].z = (float32_t) m_fpLeafHeight ;
	vVertex[3].x = vVertex[1].x ;
	vVertex[3].y = 0.0f ;
	vVertex[3].z = (float32_t) m_fpLeafHeight ;
	vVertex[4].x = 0.0f ;
	vVertex[4].y = (float32_t) (m_fpLeafWidth * m_fpLeafSwell * -0.25) ;
	vVertex[4].z = (float32_t) m_fpLeafHeight * 0.5f ;
	//
	for ( size_t i = 0; i < 5; i ++ )
	{
		vNormal[i] = S3DVector( 0, 0, 0 ) ;
	}
	for ( size_t i = 0, j = 0; i < 4; i ++, j += 3 )
	{
		size_t		k0 = nIndex[j] ;
		size_t		k1 = nIndex[j + 1] ;
		size_t		k2 = nIndex[j + 2] ;
		S3DVector	v = ((vVertex[k1] - vVertex[k0])
						* (vVertex[k2] - vVertex[k0])).Normalized() ;
		vNormal[k0] += v ;
		vNormal[k1] += v ;
		vNormal[k2] += v ;
	}
	for ( size_t i = 0; i < 5; i ++ )
	{
		vNormal[i].Normalize() ;
	}
	//
	for ( size_t i = 0; i <= nAddCount; i ++ )
	{
		//
		// 葉の向き補正
		//
		S3DMatrix	matLeaf = matLeafBase ;
		if ( flagTopLeaf )
		{
			double	radLeafZ = m_random.QuickRandomDouble
										( m_degLeafRndAngle * PI / 180.0 ) ;
			radLeafZ += i * 2.0 * PI / (nAddCount + 1) ;
			matLeaf.RevolveOnZ( sin(radLeafZ), cos(radLeafZ) ) ;
		}
		else
		{
			double	radLeafX = m_random.QuickRandomDouble
										( m_degLeafRndAngle * PI / 180.0 ) ;
			double	radLeafY = m_random.QuickRandomDouble
										( m_degLeafRndAngle * PI / 180.0 ) ;
			double	radLeafZ = m_random.QuickRandomDouble
										( m_degLeafFaceAngle * PI / 180.0 ) ;
			//
			matLeaf.RevolveOnX( sin(radLeafX), cos(radLeafX) ) ;
			matLeaf.RevolveOnY( sin(radLeafY), cos(radLeafY) ) ;
			matLeaf.RevolveOnZ( sin(radLeafZ), cos(radLeafZ) ) ;
		}
		//
		// メッシュ追加
		//
		S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
		ppVBs[1]->AllocatePrimitiveBuffer( prmbuf, primitiveTriangle, 24, 10 ) ;
		//
		float32_t	fpScale =
			(float32_t) esl_fmax( 1.0 + m_random.QuickRandomDouble( m_fpLeafSizeRandom ), 0.0 ) ;
		//
		for ( int i = 0; i < 5; i ++ )
		{
			prmbuf.pvVertex[i] = matLeaf * vVertex[i] * fpScale + vPos ;
			prmbuf.pvNormal[i] = matLeaf * vNormal[i] ;
			prmbuf.pvUVMap[i] = vUV[i] ;
			prmbuf.pColor[i] = clrVertex[i] ;
		}
		for ( int i = 0; i < 12; i ++ )
		{
			prmbuf.pIndexedList[i] = nIndex[i] ;
		}
		//
		ppVBs[1]->AddPrimitiveBuffer( NULL, 0, primitiveTriangle, prmbuf, 12, 5 ) ;
	}
}


void S3DTreeMeshBuilderController::BuildBothThickLeaf
	( S3DVertexBufferInterface ** ppVBs, size_t nVBCount,
		const S3DMatrix& matLeafBase, S3DVector& vPos,
		size_t nAddCount, bool flagTopLeaf )
{
	ESLAssert( nVBCount >= 2 ) ;
	ESLAssert( ppVBs[1] != NULL ) ;
	//
	// 葉メッシュ準備
	//
	S3DVector	vVertex[10] ;
	S3DVector	vNormal[10] ;
	S2DVector	vUV[10] =
	{
		S2DVector( 0, 1 ),
		S2DVector( 1, 1 ),
		S2DVector( 0, 0 ),
		S2DVector( 1, 0 ),
		S2DVector( 0.5f, 0.5f ),
		S2DVector( 0, 1 ),
		S2DVector( 1, 1 ),
		S2DVector( 0, 0 ),
		S2DVector( 1, 0 ),
		S2DVector( 0.5f, 0.5f ),
	} ;
	S3DColor	clrBase( m_rgbLeafBaseColor.ui32 | 0xFF000000, 0 ) ;
	S3DColor	clrTip( 0xFFFFFFFF, 0 ) ;
	S3DColor	clrCenter
					( sglPackedColorAdd
						( (m_rgbLeafBaseColor.ui32 >> 1)
									& 0x7F7F7F, 0x808080 ) | 0xFF000000, 0 ) ;
	S3DColor	clrVertex[10] =
	{
		clrBase, clrBase, clrTip, clrTip, clrCenter,
		clrBase, clrBase, clrTip, clrTip, clrCenter,
	} ;
	uint32_t	nIndex[24] =
	{
		0, 1, 4,  1, 3, 4,  3, 2, 4,  2, 0, 4,
		6, 5, 9,  8, 6, 9,  7, 8, 9,  5, 7, 9,
	} ;
	//
	vVertex[0].x = (float32_t) m_fpLeafWidth * -0.5f ;
	vVertex[0].y = 0.0f ;
	vVertex[0].z = 0.0f ;
	vVertex[1].x = (float32_t) m_fpLeafWidth * 0.5f ;
	vVertex[1].y = 0.0f ;
	vVertex[1].z = 0.0f ;
	vVertex[2].x = vVertex[0].x ;
	vVertex[2].y = 0.0f ;
	vVertex[2].z = (float32_t) m_fpLeafHeight ;
	vVertex[3].x = vVertex[1].x ;
	vVertex[3].y = 0.0f ;
	vVertex[3].z = (float32_t) m_fpLeafHeight ;
	vVertex[4].x = 0.0f ;
	vVertex[4].y = (float32_t) (m_fpLeafWidth * m_fpLeafSwell * -0.25) ;
	vVertex[4].z = (float32_t) m_fpLeafHeight * 0.5f ;
	vVertex[5] = vVertex[0] ;
	vVertex[6] = vVertex[1] ;
	vVertex[7] = vVertex[2] ;
	vVertex[8] = vVertex[3] ;
	vVertex[9] = vVertex[4] ;
	vVertex[9].y = - vVertex[9].y ;
	//
	for ( size_t i = 0; i < 10; i ++ )
	{
		vNormal[i] = S3DVector( 0, 0, 0 ) ;
	}
	for ( size_t i = 0, j = 0; i < 8; i ++, j += 3 )
	{
		size_t		k0 = nIndex[j] ;
		size_t		k1 = nIndex[j + 1] ;
		size_t		k2 = nIndex[j + 2] ;
		S3DVector	v = ((vVertex[k1] - vVertex[k0])
						* (vVertex[k2] - vVertex[k0])).Normalized() ;
		vNormal[k0] += v ;
		vNormal[k1] += v ;
		vNormal[k2] += v ;
	}
	for ( size_t i = 0; i < 10; i ++ )
	{
		vNormal[i].Normalize() ;
	}
	//
	for ( size_t i = 0; i <= nAddCount; i ++ )
	{
		//
		// 葉の向き補正
		//
		S3DMatrix	matLeaf = matLeafBase ;
		if ( flagTopLeaf )
		{
			double	radLeafZ = m_random.QuickRandomDouble
										( m_degLeafRndAngle * PI / 180.0 ) ;
			radLeafZ += i * 2.0 * PI / (nAddCount + 1) ;
			matLeaf.RevolveOnZ( sin(radLeafZ), cos(radLeafZ) ) ;
		}
		else
		{
			double	radLeafX = m_random.QuickRandomDouble
										( m_degLeafRndAngle * PI / 180.0 ) ;
			double	radLeafY = m_random.QuickRandomDouble
										( m_degLeafRndAngle * PI / 180.0 ) ;
			double	radLeafZ = m_random.QuickRandomDouble
										( m_degLeafFaceAngle * PI / 180.0 ) ;
			//
			matLeaf.RevolveOnX( sin(radLeafX), cos(radLeafX) ) ;
			matLeaf.RevolveOnY( sin(radLeafY), cos(radLeafY) ) ;
			matLeaf.RevolveOnZ( sin(radLeafZ), cos(radLeafZ) ) ;
		}
		//
		// メッシュ追加
		//
		S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
		ppVBs[1]->AllocatePrimitiveBuffer( prmbuf, primitiveTriangle, 24, 10 ) ;
		//
		float32_t	fpScale =
			(float32_t) esl_fmax( 1.0 + m_random.QuickRandomDouble( m_fpLeafSizeRandom ), 0.0 ) ;
		//
		for ( int i = 0; i < 10; i ++ )
		{
			prmbuf.pvVertex[i] = matLeaf * vVertex[i] * fpScale + vPos ;
			prmbuf.pvNormal[i] = matLeaf * vNormal[i] ;
			prmbuf.pvUVMap[i] = vUV[i] ;
			prmbuf.pColor[i] = clrVertex[i] ;
		}
		for ( int i = 0; i < 24; i ++ )
		{
			prmbuf.pIndexedList[i] = nIndex[i] ;
		}
		//
		ppVBs[1]->AddPrimitiveBuffer( NULL, 0, primitiveTriangle, prmbuf, 24, 10 ) ;
	}
}

