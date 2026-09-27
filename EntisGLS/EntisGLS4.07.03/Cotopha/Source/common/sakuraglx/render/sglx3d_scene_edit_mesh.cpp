
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl2d/sgl_spline_curve.h>
#include <sakuraglx/render/sglx3d_scene_item.h>
#include <sakuraglx/render/sglx3d_scene_edit_mesh.h>

using namespace SSystem ;
using namespace SakuraGL ;
using namespace Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// メッシュ編集クラス・縮退頂点
//////////////////////////////////////////////////////////////////////////////

// 追加
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshEditor::
		DegenerateCollection::AddDegenerate
			( uint32_t nFlags, const uint32_t * pIndex, size_t nCount )
{
	size_t	iDeg = m_entries.GetLength() ;
	//
	DegenerateEntry	de ;
	de.nFlags = nFlags ;
	de.nCount = (uint32_t) nCount ;
	de.iRef = (uint32_t) m_indexes.GetLength() ;
	m_entries.Add( de ) ;
	m_indexes.AddArray( pIndex, nCount ) ;
	//
	return	iDeg ;
}

size_t S3DMeshEditor::DegenerateCollection::AddDegenerateNullEntry( uint32_t nFlags )
{
	size_t	iDeg = m_entries.GetLength() ;
	//
	DegenerateEntry	de ;
	de.nFlags = nFlags ;
	de.nCount = 0 ;
	de.iRef = (uint32_t) m_indexes.GetLength() ;
	m_entries.Add( de ) ;
	//
	return	iDeg ;
}

void S3DMeshEditor::DegenerateCollection::AddDegenerateEntryAt( size_t iEntry, uint32_t iVertex )
{
	DegenerateEntry *	pde = m_entries.GetArray() ;
	const size_t	nEntries = m_entries.GetLength() ;
	ESLAssert( iEntry < nEntries ) ;
	//
	DegenerateEntry&	de = pde[iEntry] ;
	m_indexes.InsertAt( (size_t) (de.iRef + de.nCount), iVertex ) ;
	de.nCount ++ ;
	//
	size_t	iRefBase = de.iRef ;
	for ( size_t i = 0; i < nEntries; i ++ )
	{
		if ( (pde[i].iRef >= iRefBase) && (i != iEntry) )
		{
			pde[i].iRef ++ ;
		}
	}
	m_entries.FinishArray() ;
}

// 削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::DegenerateCollection::RemoveAt( size_t nIndex )
{
	if ( nIndex < m_entries.GetLength() )
	{
		DegenerateEntry	de = m_entries.At( nIndex ) ;
		m_indexes.Remove( (size_t) de.iRef, (size_t) de.nCount ) ;
		m_entries.RemoveAt( nIndex ) ;
		//
		DegenerateEntry *	pde = m_entries.GetArray() ;
		size_t				nEntries = m_entries.GetLength() ;
		for ( size_t i = 0; i < nEntries; i ++ )
		{
			if ( pde[i].iRef > de.iRef )
			{
				pde[i].iRef -= de.nCount ;
			}
		}
		m_entries.FinishArray() ;
	}
}

// 頂点削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::DegenerateCollection::RemoveIndex
										( size_t iFirst, size_t nCount )
{
	DegenerateEntry *	pde = m_entries.GetArray() ;
	const size_t		nEntryCount = m_entries.GetLength() ;
	uint32_t *			pIndexes = m_indexes.GetArray() ;
	const size_t		nIndexCount = m_indexes.GetLength() ;
	//
	size_t	iNextDst = 0 ;
	for ( size_t i = 0; i < nEntryCount; i ++ )
	{
		DegenerateEntry	de = pde[i] ;
		pde[i].iRef = (uint32_t) iNextDst ;
		//
		for ( size_t j = 0; j < de.nCount; j ++ )
		{
			ESLAssert( de.iRef + j < nIndexCount ) ;
			uint32_t	nIndex = pIndexes[de.iRef + j] ;
			if ( nIndex < iFirst )
			{
				pIndexes[iNextDst ++] = nIndex ;
			}
			else if ( iFirst + nCount <= nIndex )
			{
				pIndexes[iNextDst ++] = nIndex - (uint32_t) nCount ;
			}
		}
		pde[i].nCount = (uint32_t) iNextDst - pde[i].iRef ;
	}
	//
	m_entries.FinishArray() ;
	m_indexes.FinishArray() ;
	ESLAssert( iNextDst <= nIndexCount ) ;
	m_indexes.SetLength( iNextDst ) ;
}

// すべて削除（逆引きインデックスのみ）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::DegenerateCollection::RemoveAllIndex( void )
{
	DegenerateEntry *	pde = m_entries.GetArray() ;
	const size_t		nEntryCount = m_entries.GetLength() ;
	//
	for ( size_t i = 0; i < nEntryCount; i ++ )
	{
		pde[i].iRef = 0 ;
		pde[i].nCount = 0 ;
	}
	//
	m_entries.FinishArray() ;
	m_indexes.RemoveAll() ;
}

// すべて削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::DegenerateCollection::RemoveAll( void )
{
	m_entries.RemoveAll() ;
	m_indexes.RemoveAll() ;
}

// 頂点指標オフセット
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::
		DegenerateCollection::ShiftIndexOffset
						( size_t iFirst, size_t iEnd, ssize_t nOffset )
{
	uint32_t *		pIndexes = m_indexes.GetArray() ;
	const size_t	nIndexCount = m_indexes.GetLength() ;
	//
	for ( size_t i = 0; i < nIndexCount; i ++ )
	{
		uint32_t	nIndex = pIndexes[i] ;
		if ( (iFirst <= nIndex) && (nIndex < iEnd) )
		{
			pIndexes[i] = (uint32_t) (nIndex + nOffset) ;
		}
	}
	m_indexes.FinishArray() ;
}

// 参照頂点指標入れ替え（iFirst1～iFirst1+nCount-1, iFirst2～iFirst2+nCount-1）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::DegenerateCollection::
		SwapIndexes( size_t iFirst1, size_t iFirst2, size_t nCount )
{
	uint32_t *		pIndexes = m_indexes.GetArray() ;
	const size_t	nIndexCount = m_indexes.GetLength() ;
	//
	for ( size_t i = 0; i < nIndexCount; i ++ )
	{
		uint32_t	nIndex = pIndexes[i] ;
		if ( (iFirst1 <= nIndex) && (nIndex < iFirst1 + nCount) )
		{
			pIndexes[i] = nIndex + (uint32_t) (iFirst2 - iFirst1) ;
		}
		else if ( (iFirst2 <= nIndex) && (nIndex < iFirst2 + nCount) )
		{
			pIndexes[i] = nIndex - (uint32_t) (iFirst2 - iFirst1) ;
		}
	}
	m_indexes.FinishArray() ;
}

// 取得（縮退頂点配列ポインタ返却）
//////////////////////////////////////////////////////////////////////////////
const uint32_t * S3DMeshEditor::
					DegenerateCollection::GetDegenerateAt
							( DegenerateEntry& de, size_t nIndex ) const
{
	if ( nIndex >= m_entries.GetLength() )
	{
		return	nullptr ;
	}
	de = m_entries.At( nIndex ) ;
	return	m_indexes.GetConstArray() + de.iRef ;
}

// 頂点検索
//////////////////////////////////////////////////////////////////////////////
const uint32_t * S3DMeshEditor::
					DegenerateCollection::FindVertex
							( DegenerateEntry& de, size_t iVertex ) const
{
	const uint32_t *	pIndexes = m_indexes.GetConstArray() ;
	const size_t		nIndexCount = m_indexes.GetLength() ;
	ssize_t				iIndex = -1 ;
	for ( size_t i = 0; i < nIndexCount; i ++ )
	{
		if ( pIndexes[i] == iVertex )
		{
			iIndex = (ssize_t) i ;
			break ;
		}
	}
	if ( iIndex < 0 )
	{
		return	nullptr ;
	}
	const DegenerateEntry *	pde = m_entries.GetConstArray() ;
	const size_t			nEntryCount = m_entries.GetLength() ;
	for ( size_t i = 0; i < nEntryCount; i ++ )
	{
		if ( (pde[i].iRef >= (size_t) iIndex)
			&& ((size_t) iIndex < pde[i].iRef + pde[i].nCount) )
		{
			de = pde[i] ;
			return	pIndexes + de.iRef ;
		}
	}
	return	nullptr ;
}

// 縮退頂点フラグ取得
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DMeshEditor::DegenerateCollection::GetDegenerateFlagAt( size_t nIndex ) const
{
	ESLAssert( nIndex < m_entries.GetLength() ) ;
	return	m_entries.At(nIndex).nFlags ;
}

// 縮退頂点フラグ変更
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditor::DegenerateCollection::
			ChangeDegenerateFlagAt( size_t nIndex, uint32_t nFlags )
{
	if ( nIndex >= m_entries.GetLength() )
	{
		return	false ;
	}
	m_entries.At(nIndex).nFlags = nFlags ;
	return	true ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
const S3DMeshEditor::DegenerateCollection&
	S3DMeshEditor::DegenerateCollection::operator =
			( const S3DMeshEditor::DegenerateCollection& dc )
{
	m_entries = dc.m_entries ;
	m_indexes = dc.m_indexes ;
	return	*this ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::DegenerateCollection::Serialize( SArray<uint8_t>& buf ) const
{
	uint8_t *	pBinBuf =
		buf.GetArray
			( sizeof(SerializedHeader)
				+ m_entries.GetLength() * sizeof(DegenerateEntry)
				+ m_indexes.GetLength() * sizeof(uint32_t) ) ;
	//
	SerializedHeader *	pHeader = (SerializedHeader*) pBinBuf ;
	pHeader->nEntryCount = (uint32_t) m_entries.GetLength() ;
	pHeader->nIndexCount = (uint32_t) m_indexes.GetLength() ;
	//
	DegenerateEntry *	pEntries =
			(DegenerateEntry*) (pBinBuf + sizeof(SerializedHeader)) ;
	eslCopyMemory
		( pEntries, m_entries.GetConstArray(),
			m_entries.GetLength() * sizeof(DegenerateEntry) ) ;
	//
	uint32_t *	pIndexes = (uint32_t*) (pEntries + m_entries.GetLength()) ;
	eslCopyMemory
		( pIndexes, m_indexes.GetConstArray(),
			m_indexes.GetLength() * sizeof(uint32_t) ) ;
	//
	buf.FinishArray() ;
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::DegenerateCollection::Deserialize( const SArray<uint8_t>& buf )
{
	const uint8_t *		pBinBuf = buf.GetConstArray() ;
	if ( buf.GetLength() < sizeof(SerializedHeader) )
	{
		return ;
	}
	const SerializedHeader *
				pHeader = (const SerializedHeader*) pBinBuf ;
	if ( buf.GetLength()
			< sizeof(SerializedHeader)
				+ pHeader->nEntryCount * sizeof(DegenerateEntry)
				+ pHeader->nIndexCount * sizeof(uint32_t) )
	{
		return ;
	}
	m_entries.SetLength( (size_t) pHeader->nEntryCount ) ;
	m_indexes.SetLength( (size_t) pHeader->nIndexCount ) ;
	//
	const DegenerateEntry *	pEntries =
		(const DegenerateEntry*) (pBinBuf + sizeof(SerializedHeader)) ;
	eslCopyMemory
		( m_entries.GetArray(), pEntries,
			pHeader->nEntryCount * sizeof(DegenerateEntry) ) ;
	m_entries.FinishArray() ;
	//
	const uint32_t *	pIndexes =
		(const uint32_t*) (pEntries + pHeader->nEntryCount) ;
	eslCopyMemory
		( m_indexes.GetArray(), pIndexes,
			pHeader->nIndexCount * sizeof(uint32_t) ) ;
	m_indexes.FinishArray() ;
}



//////////////////////////////////////////////////////////////////////////////
// パッチ稜線 S3DMeshEditor::EdgeSet
//////////////////////////////////////////////////////////////////////////////

// 頂点削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::EdgeSet::RemovePointIndex( size_t iFirst, size_t nCount )
{
	Edge *			pEdge = GetArray() ;
	const size_t	nEdgeCount = GetLength() ;
	size_t			iDst = 0 ;
	for ( size_t i = 0; i < nEdgeCount; i ++ )
	{
		Edge	edge = pEdge[i] ;
		if ( ((iFirst <= edge.iVertex0)
				&& (edge.iVertex0 < iFirst + nCount))
			|| ((iFirst <= edge.iVertex1)
				&& (edge.iVertex1 < iFirst + nCount)) )
		{
			continue ;
		}
		if ( iFirst + nCount <= edge.iVertex0 )
		{
			edge.iVertex0 -= nCount ;
		}
		if ( iFirst + nCount <= edge.iVertex1 )
		{
			edge.iVertex1 -= nCount ;
		}
		pEdge[iDst ++] = edge ;
	}
	FinishArray() ;
	ESLAssert( iDst <= GetLength() ) ;
	SetLength( iDst ) ;
}

// 頂点指標オフセット
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::EdgeSet::ShiftIndexOffset
				( size_t iFirst, size_t iEnd, ssize_t nOffset )
{
	Edge *			pEdge = GetArray() ;
	const size_t	nCount = GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Edge&	edge = pEdge[i] ;
		if ( (iFirst <= edge.iVertex0) && (edge.iVertex0 < iEnd) )
		{
			edge.iVertex0 += nOffset ;
		}
		if ( (iFirst <= edge.iVertex1) && (edge.iVertex1 < iEnd) )
		{
			edge.iVertex1 += nOffset ;
		}
	}
}

// 複製
//////////////////////////////////////////////////////////////////////////////
const S3DMeshEditor::EdgeSet&
	S3DMeshEditor::EdgeSet::operator = ( const S3DMeshEditor::EdgeSet& es )
{
	SArraySet<Edge>::operator = ( es ) ;
	return	*this ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::EdgeSet::Serialize( SArray<uint8_t>& buf ) const
{
	const size_t	nCount = GetLength() ;
	const Edge *	pEdgeSrc = GetConstArray() ;
	uint32_t *		pEdgeSet =
		(uint32_t*) buf.GetArray( nCount * 2 * sizeof(uint32_t) ) ;
	for ( size_t i = 0, j = 0; i < nCount; i ++, j += 2 )
	{
		pEdgeSet[j]     = (uint32_t) pEdgeSrc[i].iVertex0 ;
		pEdgeSet[j + 1] = (uint32_t) pEdgeSrc[i].iVertex1 ;
	}
	buf.FinishArray() ;
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::EdgeSet::Deserialize( const SArray<uint8_t>& buf )
{
	const uint32_t *	pEdgeSet = (const uint32_t*) buf.GetConstArray() ;
	const size_t		nCount = buf.GetLength() / (2 * sizeof(uint32_t)) ;
	Edge *				pEdgeDst = GetArray( nCount ) ;
	for ( size_t i = 0, j = 0; i < nCount; i ++, j += 2 )
	{
		pEdgeDst[i].iVertex0 = (size_t) pEdgeSet[j] ;
		pEdgeDst[i].iVertex1 = (size_t) pEdgeSet[j + 1] ;
	}
	FinishArray() ;
	SetLength( nCount ) ;
}



//////////////////////////////////////////////////////////////////////////////
// パッチ間の接続情報 S3DMeshEditor::SewPatchPoints
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditor::SewPatchPoints::SewPatchPoints( void ) : m_nFlags( 0 )
{
}

S3DMeshEditor::SewPatchPoints::SewPatchPoints( const SewPatchPoints& spp )
	: SArray<PatchPoint>( spp ), m_nFlags( spp.m_nFlags )
{
}

// 検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DMeshEditor::SewPatchPoints::FindPoint
				( const S3DMeshEditor::PatchPoint& pp ) const
{
	const PatchPoint *	ppp = GetConstArray() ;
	const size_t		nCount = GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( ppp[i].pPatch == pp.pPatch )
		{
			if ( ppp[i].iVertex == pp.iVertex )
			{
				return	(ssize_t) i ;
			}
			DegenerateEntry	de ;
			const uint32_t *
				pIndexes = ppp[i].pPatch->GetDegenerateAt( de, ppp[i].iVertex ) ;
			if ( pIndexes != nullptr )
			{
				for ( size_t j = 0; j < de.nCount; j ++ )
				{
					if ( pIndexes[j] == pp.iVertex )
					{
						return	(ssize_t) i ;
					}
				}
			}
		}
	}
	return	-1 ;
}

// 削除
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DMeshEditor::SewPatchPoints::RemoveAs
				( const S3DMeshEditor::PatchPoint& pp )
{
	ssize_t	i = FindPoint( pp ) ;
	if ( i >= 0 )
	{
		RemoveAt( (size_t) i ) ;
	}
	return	i ;
}

// 結合
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::SewPatchPoints::Merge( const SewPatchPoints& spp )
{
	for ( size_t i = 0; i < spp.GetLength(); i ++ )
	{
		if ( FindPoint( spp.At(i) ) < 0 )
		{
			Add( spp.At(i) ) ;
		}
	}
}

// 頂点削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::SewPatchPoints::RemovePointIndex
	( const S3DMeshEditor::Patch * pPatch, size_t iFirst, size_t nCount )
{
	PatchPoint *	ppp = GetArray() ;
	const size_t	nLen = GetLength() ;
	size_t			iDst = 0 ;
	for ( size_t i = 0; i < nLen; i ++ )
	{
		if ( (ppp[i].pPatch == pPatch)
			&& (ppp[i].iVertex >= iFirst + nCount) )
		{
			ppp[iDst].pPatch = ppp[i].pPatch ;
			ppp[iDst].iVertex = ppp[i].iVertex - (uint32_t) nCount ;
			iDst ++ ;
		}
		else if ( (ppp[i].pPatch != pPatch)
				|| (ppp[i].iVertex < iFirst)
				|| (iFirst + nCount <= ppp[i].iVertex) )
		{
			ppp[iDst ++] = ppp[i] ;
		}
	}
	FinishArray() ;
	ESLAssert( iDst <= GetLength() ) ;
	SetLength( iDst ) ;
}

// 頂点指標オフセット
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::SewPatchPoints::ShiftIndexOffset
	( const S3DMeshEditor::Patch * pPatch,
					size_t iFirst, size_t iEnd, ssize_t nOffset )
{
	PatchPoint *	ppp = GetArray() ;
	const size_t	nLen = GetLength() ;
	size_t			iDst = 0 ;
	for ( size_t i = 0; i < nLen; i ++ )
	{
		if ( (ppp[i].pPatch == pPatch)
			&& (iFirst <= ppp[i].iVertex) && (ppp[i].iVertex < iEnd) )
		{
			ppp[i].iVertex += nOffset ;
		}
	}
	FinishArray() ;
}

// 参照頂点指標入れ替え（iFirst1～iFirst1+nCount-1, iFirst2～iFirst2+nCount-1）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::SewPatchPoints::SwapPointIndexes
	( const S3DMeshEditor::Patch * pPatch,
			size_t iFirst1, size_t iFirst2, size_t nCount )
{
	PatchPoint *	ppp = GetArray() ;
	const size_t	nLen = GetLength() ;
	size_t			iDst = 0 ;
	for ( size_t i = 0; i < nLen; i ++ )
	{
		if ( ppp[i].pPatch != pPatch )
		{
			continue ;
		}
		if ( (iFirst1 <= ppp[i].iVertex) && (ppp[i].iVertex < iFirst1 + nCount) )
		{
			ppp[i].iVertex += iFirst2 - iFirst1 ;
		}
		else if ( (iFirst2 <= ppp[i].iVertex) && (ppp[i].iVertex < iFirst2 + nCount) )
		{
			ppp[i].iVertex -= iFirst2 - iFirst1 ;
		}
	}
	FinishArray() ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
const S3DMeshEditor::SewPatchPoints&
		S3DMeshEditor::SewPatchPoints::operator =
				( const S3DMeshEditor::SewPatchPoints& spp )
{
	SArray<PatchPoint>::operator = ( spp ) ;
	m_nFlags = spp.m_nFlags ;
	return	*this ;
}

// Patch 参照の置き換え
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::SewPatchPoints::RepointerPatch
	( const S3DMeshEditor::Patch * pPatchOld, S3DMeshEditor::Patch * pPatchNew )
{
	PatchPoint *	ppp = GetArray() ;
	const size_t	nLen = GetLength() ;
	size_t			iDst = 0 ;
	for ( size_t i = 0; i < nLen; i ++ )
	{
		if ( ppp[i].pPatch == pPatchOld )
		{
			ppp[i].pPatch = pPatchNew ;
		}
	}
	FinishArray() ;
}



//////////////////////////////////////////////////////////////////////////////
// パッチ間の接続情報 S3DMeshEditor::SeamPatchCollection
//////////////////////////////////////////////////////////////////////////////

// 検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DMeshEditor::SeamPatchCollection::FindPoint
				( const S3DMeshEditor::PatchPoint& pp ) const
{
	for ( size_t i = 0; i < GetLength(); i ++ )
	{
		SewPatchPoints *	pspp = GetAt( i ) ;
		if ( pspp->FindPoint( pp ) >= 0 )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

// 追加
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshEditor::SeamPatchCollection::AddSewPatchPoints
	( uint32_t nFlags, const S3DMeshEditor::PatchPoint * ppp, size_t nCount )
{
	SewPatchPoints *	pspp = new SewPatchPoints ;
	pspp->m_nFlags = nFlags ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RemoveAs( ppp[i] ) ;
		pspp->Add( ppp[i] ) ;
	}
	return	Add( pspp ) ;
}

// 削除
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DMeshEditor::SeamPatchCollection::RemoveAs( const PatchPoint& pp )
{
	ssize_t	iRemoved = -1 ;
	for ( size_t i = 0; i < GetLength(); i ++ )
	{
		SewPatchPoints *	pspp = GetAt( i ) ;
		ssize_t	j = pspp->FindPoint( pp ) ;
		if ( j >= 0 )
		{
			pspp->RemoveAt( (size_t) j ) ;
			iRemoved = (ssize_t) i ;
			//
			if ( pspp->GetLength() == 0 )
			{
				RemoveAt( i -- ) ;
			}
			break ;
		}
	}
	return	iRemoved ;
}

// 頂点削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::SeamPatchCollection::RemovePointIndex
	( const S3DMeshEditor::Patch * pPatch, size_t iFirst, size_t nCount )
{
	for ( size_t i = 0; i < GetLength(); i ++ )
	{
		SewPatchPoints *	pspp = GetAt( i ) ;
		pspp->RemovePointIndex( pPatch, iFirst, nCount ) ;
	}
}

// 頂点指標オフセット
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::SeamPatchCollection::ShiftIndexOffset
	( const Patch * pPatch, size_t iFirst, size_t iEnd, ssize_t nOffset )
{
	for ( size_t i = 0; i < GetLength(); i ++ )
	{
		SewPatchPoints *	pspp = GetAt( i ) ;
		pspp->ShiftIndexOffset( pPatch, iFirst, iEnd, nOffset ) ;
	}
}

// 参照頂点指標入れ替え（iFirst1～iFirst1+nCount-1, iFirst2～iFirst2+nCount-1）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::SeamPatchCollection::SwapPointIndexes
	( const Patch * pPatch, size_t iFirst1, size_t iFirst2, size_t nCount )
{
	for ( size_t i = 0; i < GetLength(); i ++ )
	{
		SewPatchPoints *	pspp = GetAt( i ) ;
		pspp->SwapPointIndexes( pPatch, iFirst1, iFirst2, nCount ) ;
	}
}

// 複製
//////////////////////////////////////////////////////////////////////////////
const S3DMeshEditor::SeamPatchCollection&
		S3DMeshEditor::SeamPatchCollection::operator =
				( const S3DMeshEditor::SeamPatchCollection& spc )
{
	SObjectArray<SewPatchPoints>::DuplicateArray( spc ) ;
	return	*this ;
}

// Patch 参照の置き換え
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::SeamPatchCollection::RepointerPatch
	( const S3DMeshEditor::Patch * pPatchOld, S3DMeshEditor::Patch * pPatchNew )
{
	for ( size_t i = 0; i < GetLength(); i ++ )
	{
		SewPatchPoints *	pspp = GetAt( i ) ;
		pspp->RepointerPatch( pPatchOld, pPatchNew ) ;
	}
}

// 結合
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::SeamPatchCollection::Merge( const SeamPatchCollection& spc )
{
	for ( size_t i = 0; i < spc.GetLength(); i ++ )
	{
		SewPatchPoints *	psppSrc = spc.GetAt(i) ;
		ESLAssert( psppSrc != nullptr ) ;
		if ( psppSrc == nullptr )
		{
			continue ;
		}
		SSmartPointer<SewPatchPoints>	psppMerge ;
		for ( size_t j = 0; j < psppSrc->GetLength(); j ++ )
		{
			ssize_t	iFind = FindPoint( psppSrc->At(j) ) ;
			if ( iFind >= 0 )
			{
				if ( psppMerge == nullptr )
				{
					psppMerge = new SewPatchPoints( At( (size_t) iFind ) ) ;
				}
				else
				{
					psppMerge->Merge( At( (size_t) iFind ) ) ;
				}
				RemoveAt( (size_t) iFind ) ;
			}
		}
		if ( psppMerge != nullptr )
		{
			Add( psppMerge.Detach() ) ;
		}
		else
		{
			Add( new SewPatchPoints( *psppSrc ) ) ;
		}
	}
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::SeamPatchCollection::Serialize
	( SSystem::SArray<uint8_t>& buf, const S3DMeshEditor& me ) const
{
	SerializedHeader	hdr ;
	hdr.nEntryCount = (uint32_t) GetLength() ;
	hdr.nIndexCount = 0 ;
	//
	SewPatchPoints*const*	ppSewPatchPoints = GetConstArray() ;
	for ( size_t i = 0; i < hdr.nEntryCount; i ++ )
	{
		SewPatchPoints *	pSewPoints = ppSewPatchPoints[i] ;
		ESLAssert( pSewPoints != nullptr ) ;
		hdr.nIndexCount += (uint32_t) pSewPoints->GetLength() ;
	}
	//
	const size_t	nTotalBytes =
		sizeof(SerializedHeader)
			+ hdr.nEntryCount * sizeof(SerializedEntry)
			+ hdr.nIndexCount * sizeof(PatchPointIndex) ;
	buf.SetLength( nTotalBytes ) ;
	//
	uint8_t *	pBinBuf = buf.GetArray() ;
	*((SerializedHeader*) pBinBuf) = hdr ;
	//
	SerializedEntry *	pEntries =
		(SerializedEntry*) (pBinBuf + sizeof(SerializedHeader)) ;
	PatchPointIndex *	pIndexes =
		(PatchPointIndex*) (pEntries + hdr.nEntryCount) ;
	//
	uint32_t	iNextIndex = 0 ;
	for ( size_t i = 0; i < hdr.nEntryCount; i ++ )
	{
		SewPatchPoints *	pspp = ppSewPatchPoints[i] ;
		ESLAssert( pspp != nullptr ) ;
		//
		SerializedEntry &	entry = pEntries[i] ;
		entry.nFlags = pspp->m_nFlags ;
		entry.nCount = (uint32_t) pspp->GetLength() ;
		entry.iRef = iNextIndex ;
		//
		const PatchPoint *	pppSrc = pspp->GetConstArray() ;
		for ( size_t j = 0; j < entry.nCount; j ++ )
		{
			pIndexes[iNextIndex].iPatch = (uint32_t) me.FindPatch( pppSrc[j].pPatch ) ;
			pIndexes[iNextIndex].iVertex = (uint32_t) pppSrc[j].iVertex ;
			ESLAssert( pIndexes[iNextIndex].iPatch != (uint32_t) -1 ) ;
			iNextIndex ++ ;
		}
	}
	ESLAssert( iNextIndex <= hdr.nIndexCount ) ;
	//
	buf.FinishArray() ;
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::SeamPatchCollection::Deserialize
	( const SSystem::SArray<uint8_t>& buf, const S3DMeshEditor& me )
{
	const uint8_t *	pBinBuf = buf.GetConstArray() ;
	if ( buf.GetLength() < sizeof(SerializedHeader) )
	{
		return ;
	}
	const SerializedHeader *	pHeader = (const SerializedHeader*) pBinBuf ;
	if ( buf.GetLength()
			< sizeof(SerializedHeader)
				+ pHeader->nEntryCount * sizeof(SerializedEntry)
				+ pHeader->nIndexCount * sizeof(PatchPointIndex) )
	{
		return ;
	}
	const SerializedEntry *	pEntries =
		(const SerializedEntry*) (pBinBuf + sizeof(SerializedHeader)) ;
	const PatchPointIndex *	pIndexes =
		(const PatchPointIndex*) (pEntries + pHeader->nEntryCount) ;
	//
	SetLength( (size_t) pHeader->nEntryCount ) ;
	for ( size_t i = 0; i < pHeader->nEntryCount; i ++ )
	{
		SewPatchPoints *	pspp = new SewPatchPoints ;
		SetAt( i, pspp ) ;
		//
		const SerializedEntry &	entry = pEntries[i] ;
		pspp->SetLength( (size_t) entry.nCount ) ;
		//
		PatchPoint *			pppDst = pspp->GetArray() ;
		const PatchPointIndex *	ppiSrc = pIndexes + entry.iRef ;
		for ( size_t j = 0; j < entry.nCount; j ++ )
		{
			pppDst[j].pPatch = me.GetPatchAt( (size_t) ppiSrc->iPatch ) ;
			pppDst[j].iVertex = (size_t) ppiSrc->iVertex ;
			ppiSrc ++ ;
		}
		pspp->FinishArray() ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// メッシュバッファ S3DMeshEditor::MeshBuffer
//////////////////////////////////////////////////////////////////////////////

// 構築
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditor::MeshBuffer::MeshBuffer( void )
	: m_countVertex( 0 ), m_countIndex( 0 ), m_nWeightLayers( 0 )
{
}

S3DMeshEditor::MeshBuffer::MeshBuffer( const MeshBuffer& mbuf )
	: m_countVertex( mbuf.m_countVertex ),
		m_countIndex( mbuf.m_countIndex ),
		m_nWeightLayers( mbuf.m_nWeightLayers ),
		m_bufExAttrIndex( mbuf.m_bufExAttrIndex ),
		m_bufVertex( mbuf.m_bufVertex ),
		m_bufNormal( mbuf.m_bufNormal ),
		m_bufUVMap( mbuf.m_bufUVMap ),
		m_bufColor( mbuf.m_bufColor ),
		m_bufWeight( mbuf.m_bufWeight ),
		m_bufSrcVertex( mbuf.m_bufSrcVertex ),
		m_bufIndex( mbuf.m_bufIndex ),
		m_edegs( mbuf.m_edegs )
{
}

// クリア
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::MeshBuffer::ClearBuffer( void )
{
	m_countVertex = 0 ;
	m_countIndex = 0 ;
	m_nWeightLayers = 0 ;
	m_bufExAttrIndex.RemoveAll() ;
	m_bufVertex.RemoveAll() ;
	m_bufNormal.RemoveAll() ;
	m_bufUVMap.RemoveAll() ;
	m_bufColor.RemoveAll() ;
	m_bufWeight.RemoveAll() ;
	m_bufSrcVertex.RemoveAll() ;
	m_bufIndex.RemoveAll() ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::MeshBuffer::CopyBufferFrom
				( const S3DMeshEditor::MeshBuffer& mbuf )
{
	m_countVertex = mbuf.m_countVertex ;
	m_countIndex = mbuf.m_countIndex ;
	m_nWeightLayers = mbuf.m_nWeightLayers ;
	m_bufExAttrIndex = mbuf.m_bufExAttrIndex ;
	m_bufVertex = mbuf.m_bufVertex ;
	m_bufNormal = mbuf.m_bufNormal ;
	m_bufUVMap = mbuf.m_bufUVMap ;
	m_bufColor = mbuf.m_bufColor ;
	m_bufWeight = mbuf.m_bufWeight ;
	m_bufSrcVertex = mbuf.m_bufSrcVertex ;
	m_bufIndex = mbuf.m_bufIndex ;
}

const S3DMeshEditor::MeshBuffer&
		S3DMeshEditor::MeshBuffer::operator =
					( const S3DMeshEditor::MeshBuffer& mbuf )
{
	CopyBufferFrom( mbuf ) ;
	return	*this ;
}

// 外接直方体
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditor::MeshBuffer::GetCircumscribedBox( S3DVector& vMin, S3DVector& vMax ) const
{
	if ( m_bufVertex.GetLength() == 0 )
	{
		vMin.x = 0 ;
		vMin.y = 0 ;
		vMin.z = 0 ;
		vMax.x = 0 ;
		vMax.y = 0 ;
		vMax.z = 0 ;
		return	false ;
	}
	S3DVector4	vMin4, vMax4 ;
	MinMaxVector4DArray
		( vMin4, vMax4, m_bufVertex.GetConstArray(), m_bufVertex.GetLength() ) ;
	vMin = vMin4 ;
	vMax = vMax4 ;
	return	true ;
}



//////////////////////////////////////////////////////////////////////////////
// メッシュ編集クラス・パッチメッシュ情報
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DMeshEditor::Patch, ESLObject )

// 構築
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditor::Patch::Patch
		( S3DMeshEditor::SeamPatchCollection * pspc )
	: m_pspc( pspc ), m_pSerializer( nullptr ),
		m_wPatch( 0 ), m_hPatch( 0 ),
		m_nWeightLayers( 0 ), m_iMaterial( 0 ),
		m_flagUpdateVertex( false ),
		m_flagUpdateFace( false ),
		m_flagUpdateSerialize( true ), m_nFlags( 0 )
{
}

// 名前
//////////////////////////////////////////////////////////////////////////////
const SSystem::SString& S3DMeshEditor::Patch::GetName( void ) const
{
	return	m_strName ;
}

void S3DMeshEditor::Patch::SetName( const wchar_t * pwszName )
{
	m_strName = pwszName ;
	m_flagUpdateSerialize = true ;
}

// シリアライザ
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::AttachSerializer
	( S3DMeshBufferPropertySerializer::MeshBuffer * pSerializer )
{
	m_pSerializer = pSerializer ;
}

S3DMeshBufferPropertySerializer::MeshBuffer *
		S3DMeshEditor::Patch::GetSerializer( void ) const
{
	return	m_pSerializer ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::CopyFrom( const S3DMeshEditor::Patch& patch )
{
	m_strName = patch.m_strName ;
	m_wPatch = patch.m_wPatch ;
	m_hPatch = patch.m_hPatch ;
	m_nWeightLayers = patch.m_nWeightLayers ;
	m_iMaterial = patch.m_iMaterial ;
	m_flagUpdateVertex = patch.m_flagUpdateVertex ;
	m_flagUpdateFace = patch.m_flagUpdateFace ;
	m_flagUpdateSerialize = true ;
	m_nFlags = patch.m_nFlags ;
	//
	m_bufVertex = patch.m_bufVertex ;
	m_bufNormal = patch.m_bufNormal ;
	m_bufFaceNormal = patch.m_bufFaceNormal ;
	m_bufUVMap = patch.m_bufUVMap ;
	m_bufColor = patch.m_bufColor ;
	m_bufWeight = patch.m_bufWeight ;
	m_edges = patch.m_edges ;
	m_bufFace = patch.m_bufFace ;
	m_bufShifter = patch.m_bufShifter ;
	m_bufUsedVertex = patch.m_bufUsedVertex ;
	m_bufIUsedVertex = patch.m_bufIUsedVertex ;
	m_bufDegenerate = patch.m_bufDegenerate ;
	m_degenerates = patch.m_degenerates ;
	m_bufTriangles = patch.m_bufTriangles ;
	//
	SetUpdateVertexFlag() ;
	SetUpdateFaceFlag() ;
}

void S3DMeshEditor::Patch::CopyRect
	( int xDst, int yDst,
		const S3DMeshEditor::Patch& patch, const SGLImageRect& rectSrc )
{
	ESLAssert( (size_t) xDst < GetWidth() ) ;
	ESLAssert( (size_t) yDst < GetHeight() ) ;
	ESLAssert( (size_t) (xDst + rectSrc.w) <= GetWidth() ) ;
	ESLAssert( (size_t) (yDst + rectSrc.h) <= GetHeight() ) ;
	ESLAssert( (size_t) rectSrc.x < patch.GetWidth() ) ;
	ESLAssert( (size_t) rectSrc.y < patch.GetHeight() ) ;
	ESLAssert( (size_t) (rectSrc.x + rectSrc.w) <= patch.GetWidth() ) ;
	ESLAssert( (size_t) (rectSrc.y + rectSrc.h) <= patch.GetHeight() ) ;
	//
	size_t	nLayers = m_nWeightLayers ;
	if ( nLayers > patch.m_nWeightLayers )
	{
		nLayers = patch.m_nWeightLayers ;
	}
	for ( size_t y = 0; y < (size_t) rectSrc.h; y ++ )
	{
		for ( size_t x = 0; x < (size_t) rectSrc.w; x ++ )
		{
			size_t	xd = x + (size_t) xDst ;
			size_t	yd = y + (size_t) yDst ;
			size_t	xs = x + (size_t) rectSrc.x ;
			size_t	ys = y + (size_t) rectSrc.y ;
			//
			SetPoint( xd, yd, patch.GetPoint( xs, ys ) ) ;
			SetNormal( xd, yd, patch.GetNormal( xs, ys ) ) ;
			SetFaceNormal( xd, yd, patch.GetFaceNormal( xs, ys ) ) ;
			SetUV( xd, yd, patch.GetUV( xs, ys ) ) ;
			SetColor( xd, yd, patch.GetColor( xs, ys ) ) ;
			SetFace( xd, yd, patch.GetFace( xs, ys ) ) ;
			//
			for ( size_t i = 0; i < nLayers; i ++ )
			{
				SetWeight( xd, yd, i, patch.GetWeight( xs, ys, i ) ) ;
			}
		}
	}
	//
	SArray<uint32_t>	aIndexes ;
	for ( size_t i = 0; i < patch.m_degenerates.m_entries.GetLength(); i ++ )
	{
		DegenerateEntry	de ;
		const uint32_t *
			pIndexes = patch.m_degenerates.GetDegenerateAt( de, i ) ;
		ESLAssert( pIndexes != nullptr ) ;
		for ( size_t j = 0; j < de.nCount; j ++ )
		{
			SGLPoint	pt ;
			patch.PointFromIndex( pt, pIndexes[j] ) ;
			if ( (rectSrc.x <= pt.x) && (pt.x < rectSrc.x + rectSrc.w)
				&& (rectSrc.y <= pt.y) && (pt.y < rectSrc.y + rectSrc.h) )
			{
				aIndexes.Add
					( IndexFromPoint
						( (size_t) (pt.x + xDst - rectSrc.x),
							(size_t) (pt.y + yDst - rectSrc.y) ) ) ;
			}
		}
		if ( aIndexes.GetLength() >= 2 )
		{
			SetDegenerate
				( de.nFlags, aIndexes.GetConstArray(), aIndexes.GetLength() ) ;
		}
		aIndexes.RemoveAll() ;
	}
}

// サイズ（頂点数＝面数（常にループ））
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshEditor::Patch::GetWidth( void ) const
{
	return	m_wPatch ;
}

size_t S3DMeshEditor::Patch::GetHeight( void ) const
{
	return	m_hPatch ;
}

size_t S3DMeshEditor::Patch::GetAreaSize( void ) const
{
	return	m_wPatch * m_hPatch ;
}

// ループの有無を考慮した面数
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshEditor::Patch::GetFaceWidth( void ) const
{
	return	((m_wPatch == 0) || (m_nFlags & flagHorzLoop))
									? m_wPatch : m_wPatch - 1 ;
}

size_t S3DMeshEditor::Patch::GetFaceHeight( void ) const
{
	return	((m_hPatch == 0) || (m_nFlags & flagVertLoop))
									? m_hPatch : m_hPatch - 1 ;
}

// フラグ
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DMeshEditor::Patch::GetFlags( void ) const
{
	return	m_nFlags ;
}

void S3DMeshEditor::Patch::SetFlags( uint32_t nFlags )
{
	m_nFlags = nFlags ;
}

void S3DMeshEditor::Patch::ModifyFlags( uint32_t nAdd, uint32_t nRemove )
{
	m_nFlags = (m_nFlags | nAdd) & ~nRemove ;
}

// マテリアル
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshEditor::Patch::GetMaterialIndex( void ) const
{
	return	m_iMaterial ;
}

void S3DMeshEditor::Patch::SetMaterialIndex( size_t iMaterial )
{
	m_iMaterial = iMaterial ;
}

// 頂点更新フラグ設定（法線更新判定）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::SetUpdateVertexFlag( void )
{
	m_flagUpdateVertex = true ;
	m_flagUpdateSerialize = true ;
}

// 頂点更新フラグ取得
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditor::Patch::GetUpdateVertexFlag( void ) const
{
	return	m_flagUpdateVertex ;
}

// 面更新フラグ設定（使用頂点参照配列更新判定）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::SetUpdateFaceFlag( void )
{
	m_flagUpdateFace = true ;
	m_flagUpdateSerialize = true ;
}

// 面更新フラグ取得
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditor::Patch::GetUpdateFaceFlag( void ) const
{
	return	m_flagUpdateFace ;
}

// シリアライズ用更新フラグ設定
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::SetUpdateSerializeFlag( void )
{
	m_flagUpdateSerialize = true ;
}

// シリアライズ用更新フラグクリア
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::ResetUpdateSerializeFlag( void )
{
	m_flagUpdateSerialize = false ;
}

// シリアライズ用更新フラグ取得
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditor::Patch::GetUpdateSerializeFlag( void ) const
{
	return	m_flagUpdateSerialize ;
}

// 初期サイズ設定
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::CreatePatch( size_t nWidth, size_t nHeight )
{
	size_t	nLastPatchSize = GetAreaSize() ;
	RemovePoints( 0, nLastPatchSize ) ;
	RemoveFaces( 0, nLastPatchSize ) ;
	//
	m_wPatch = nWidth ;
	m_hPatch = nHeight ;
	//
	size_t	nNewPatchSize = GetAreaSize() ;
	InsertPoints( 0, nNewPatchSize ) ;
	InsertFaces( 0, nNewPatchSize ) ;
}

// 水平ライン挿入
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::InsertLine( size_t iLine, float_t w )
{
	ESLAssert( iLine <= m_hPatch ) ;
	if ( (m_wPatch == 0) || (iLine > m_hPatch) )
	{
		return ;
	}
	//
	// 要素挿入
	//
	InsertPoints( iLine * m_wPatch, m_wPatch ) ;
	InsertFaces( iLine * m_wPatch, m_wPatch ) ;
	//
	m_hPatch ++ ;
	if ( m_hPatch == 1 )
	{
		return ;
	}
	//
	// 補完
	//
	size_t	nWeightLayers = GetWeightLayerCount() ;
	size_t	iLine0 = iLine - 1 ;
	size_t	iLine2 = iLine + 1 ;
	if ( iLine == 0 )
	{
		iLine0 = (m_hPatch > 0) ? m_hPatch - 1 : 0 ;
	}
	if ( iLine2 >= m_hPatch )
	{
		iLine2 = 0 ;
	}
	float32_t	nw = 1.0f - w ;
	for ( size_t i = 0; i < m_wPatch; i ++ )
	{
		SetPoint( i, iLine, GetPoint( i, iLine0 ) * nw
								+ GetPoint( i, iLine2 ) * w ) ;
		SetUV( i, iLine, GetUV( i, iLine0 ) * nw
								+ GetUV( i, iLine2 ) * w ) ;
		SetColor( i, iLine, GetColor( i, iLine0 ) * nw
								+ GetColor( i, iLine2 ) * w ) ;
		//
		for ( size_t j = 0; j < nWeightLayers; j ++ )
		{
			SetWeight( i, iLine, j, GetWeight( i, iLine0, j ) * nw
									+ GetWeight( i, iLine2, j ) * w ) ;
		}
	}
	//
	// 面初期値複製
	//
	size_t	iSrcLine = iLine + 1 ;
	if ( (iSrcLine + 1 >= m_hPatch) && (iLine > 0) )
	{
		iSrcLine = iLine - 1 ;
	}
	if ( iSrcLine < m_hPatch )
	{
		for ( size_t i = 0; i < m_wPatch; i ++ )
		{
			SetFace( i, iLine, GetFace( i, iSrcLine ) ) ;
		}
	}
	//
	// 縮退
	//
	SArray<uint32_t>	aIndexes ;
	const size_t		iyLine0 = iLine0 * m_wPatch ;
	const size_t		iyLine1 = iLine * m_wPatch ;
	const size_t		iyLine2 = iLine2 * m_wPatch ;
	if ( (iLine0 != iLine) && (iLine != iLine2) && (iLine0 != iLine2) )
	{
		for ( size_t i = 0; i < m_wPatch; i ++ )
		{
			uint32_t	iDeg0 = m_bufDegenerate.At( iyLine0 + i ) ;
			uint32_t	iDeg1 = m_bufDegenerate.At( iyLine1 + i ) ;
			uint32_t	iDeg2 = m_bufDegenerate.At( iyLine2 + i ) ;
			if ( (iDeg1 == 0) && (iDeg0 != 0) && (iDeg2 != 0) && (iDeg0 == iDeg2) )
			{
				DegenerateEntry		de ;
				const uint32_t *	pDegIndex = GetDegenerateAt( de, iyLine0 + i ) ;
				ESLVerify( pDegIndex != nullptr ) ;
				//
				aIndexes.RemoveAll() ;
				aIndexes.AddArray( pDegIndex, de.nCount ) ;
				aIndexes.Add( (uint32_t) (iyLine1 + i) ) ;
				//
				if ( aIndexes.GetLength() >= 2 )
				{
					SetDegenerate
						( de.nFlags,
							aIndexes.GetConstArray(),
							aIndexes.GetLength() ) ;
				}
			}
		}
	}
	uint32_t	iDeg00 = m_bufDegenerate.At( iyLine0 ) ;
	uint32_t	iDeg01 = m_bufDegenerate.At( iyLine0 + m_wPatch - 1 ) ;
	uint32_t	iDeg10 = m_bufDegenerate.At( iyLine1 ) ;
	uint32_t	iDeg11 = m_bufDegenerate.At( iyLine1 + m_wPatch - 1 ) ;
	uint32_t	iDeg20 = m_bufDegenerate.At( iyLine2 ) ;
	uint32_t	iDeg21 = m_bufDegenerate.At( iyLine2 + m_wPatch - 1 ) ;
	if ( (iDeg00 != 0) && (iDeg00 == iDeg01)
		&& (iDeg20 != 0) && (iDeg20 == iDeg21) )
	{
		uint32_t	nFlags = 0 ;
		aIndexes.RemoveAll() ;
		if ( iDeg10 == 0 )
		{
			aIndexes.Add( (uint32_t) iyLine1 ) ;
		}
		else
		{
			DegenerateEntry		de ;
			const uint32_t *	pDegIndex = GetDegenerateAt( de, iyLine1 ) ;
			ESLVerify( pDegIndex != nullptr ) ;
			nFlags |= de.nFlags ;
			aIndexes.AddArray( pDegIndex, de.nCount ) ;
		}
		if ( iDeg11 == 0 )
		{
			aIndexes.Add( (uint32_t) (iyLine1 + m_wPatch - 1) ) ;
		}
		else
		{
			DegenerateEntry		de ;
			const uint32_t *	pDegIndex = GetDegenerateAt( de, iyLine1 + m_wPatch - 1 ) ;
			ESLVerify( pDegIndex != nullptr ) ;
			nFlags |= de.nFlags ;
			aIndexes.AddArray( pDegIndex, de.nCount ) ;
		}
		SetDegenerate
			( nFlags,
				aIndexes.GetConstArray(),
				aIndexes.GetLength() ) ;
	}
	/*
	for ( size_t i = 0; i < m_wPatch; i ++ )
	{
		uint32_t	iDeg1 = m_bufDegenerate.At( iyLine1 + i ) ;
		if ( iDeg1 != 0 )
		{
			continue ;
		}
		uint32_t	iDeg2 = m_bufDegenerate.At( iyLine2 + i ) ;
		if ( iDeg2 == 0 )
		{
			continue ;
		}
		DegenerateEntry		de ;
		const uint32_t *	pDegIndex = GetDegenerateAt( de, iyLine2 + i ) ;
		ESLVerify( pDegIndex != nullptr ) ;
		//
		aIndexes.RemoveAll() ;
		aIndexes.Add( (uint32_t) (iyLine1 + i) ) ;
		for ( size_t j = i + 1; j < m_wPatch; j ++ )
		{
			if ( (m_bufDegenerate.At( iyLine1 + j ) == 0)
				&& (m_bufDegenerate.At( iyLine2 + j ) == iDeg2) )
			{
				aIndexes.Add( (uint32_t) (iyLine1 + j) ) ;
			}
		}
		if ( aIndexes.GetLength() >= 2 )
		{
			SetDegenerate
				( de.nFlags,
					aIndexes.GetConstArray(),
					aIndexes.GetLength() ) ;
		}
	}
	*/
}

// 垂直ライン挿入
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::InsertColumn( size_t iCol, float_t w )
{
	ESLAssert( iCol <= m_wPatch ) ;
	if ( (m_hPatch == 0) || (iCol > m_wPatch) )
	{
		return ;
	}
	//
	// 要素挿入
	//
	for ( size_t i = 0; i < m_hPatch; i ++ )
	{
		InsertPoints( (m_hPatch - i - 1) * m_wPatch + iCol, 1 ) ;
		InsertFaces( (m_hPatch - i - 1) * m_wPatch + iCol, 1 ) ;
	}
	m_wPatch ++ ;
	if ( m_wPatch == 1 )
	{
		return ;
	}
	//
	// 補完
	//
	size_t	nWeightLayers = GetWeightLayerCount() ;
	size_t	iCol0 = iCol - 1 ;
	size_t	iCol2 = iCol + 1 ;
	if ( iCol == 0 )
	{
		iCol0 = (m_wPatch > 0) ? m_wPatch - 1 : 0 ;
	}
	if ( iCol2 >= m_wPatch )
	{
		iCol2 = 0 ;
	}
	float32_t	nw = 1.0f - w ;
	for ( size_t i = 0; i < m_hPatch; i ++ )
	{
		SetPoint( iCol, i, GetPoint( iCol0, i ) * nw
								+ GetPoint( iCol2, i ) * w ) ;
		SetUV( iCol, i, GetUV( iCol0, i ) * nw
								+ GetUV( iCol2, i ) * w ) ;
		SetColor( iCol, i, GetColor( iCol0, i ) * nw
								+ GetColor( iCol2, i ) * w ) ;
		//
		for ( size_t j = 0; j < nWeightLayers; j ++ )
		{
			SetWeight( iCol, i, j, GetWeight( iCol0, i, j ) * nw
									+ GetWeight( iCol2, i, j ) * w ) ;
		}
	}
	//
	// 面初期値複製
	//
	size_t	iSrcCol = iCol + 1 ;
	if ( (iSrcCol + 1 >= m_wPatch) && (iCol > 0) )
	{
		iSrcCol = iCol - 1 ;
	}
	if ( iSrcCol < m_wPatch )
	{
		for ( size_t i = 0; i < m_hPatch; i ++ )
		{
			SetFace( iCol, i, GetFace( iSrcCol, i ) ) ;
		}
	}
	//
	// 縮退
	//
	SArray<uint32_t>	aIndexes ;
	if ( (iCol0 != iCol) && (iCol != iCol2) && (iCol0 != iCol2) )
	{
		for ( size_t i = 0; i < m_hPatch; i ++ )
		{
			const size_t	iyLine = i * m_wPatch ;
			uint32_t	iDeg0 = m_bufDegenerate.At( iCol0 + iyLine ) ;
			uint32_t	iDeg1 = m_bufDegenerate.At( iCol + iyLine ) ;
			uint32_t	iDeg2 = m_bufDegenerate.At( iCol2 + iyLine ) ;
			if ( (iDeg1 == 0) && (iDeg0 != 0) && (iDeg2 != 0) && (iDeg0 == iDeg2) )
			{
				DegenerateEntry		de ;
				const uint32_t *	pDegIndex = GetDegenerateAt( de, iCol0 + iyLine ) ;
				ESLVerify( pDegIndex != nullptr ) ;
				//
				aIndexes.RemoveAll() ;
				aIndexes.AddArray( pDegIndex, de.nCount ) ;
				aIndexes.Add( (uint32_t) (iCol + iyLine) ) ;
				//
				if ( aIndexes.GetLength() >= 2 )
				{
					SetDegenerate
						( de.nFlags,
							aIndexes.GetConstArray(),
							aIndexes.GetLength() ) ;
				}
			}
		}
	}
	const size_t	iLineEnd = (m_hPatch - 1) * m_wPatch ;
	uint32_t	iDeg00 = m_bufDegenerate.At( iCol0 ) ;
	uint32_t	iDeg01 = m_bufDegenerate.At( iCol0 + iLineEnd ) ;
	uint32_t	iDeg10 = m_bufDegenerate.At( iCol ) ;
	uint32_t	iDeg11 = m_bufDegenerate.At( iCol + iLineEnd ) ;
	uint32_t	iDeg20 = m_bufDegenerate.At( iCol2 ) ;
	uint32_t	iDeg21 = m_bufDegenerate.At( iCol2 + iLineEnd ) ;
	if ( (iDeg00 != 0) && (iDeg00 == iDeg01)
		&& (iDeg20 != 0) && (iDeg20 == iDeg21) )
	{
		uint32_t	nFlags = 0 ;
		aIndexes.RemoveAll() ;
		if ( iDeg10 == 0 )
		{
			aIndexes.Add( (uint32_t) iCol ) ;
		}
		else
		{
			DegenerateEntry		de ;
			const uint32_t *	pDegIndex = GetDegenerateAt( de, iCol ) ;
			ESLVerify( pDegIndex != nullptr ) ;
			nFlags |= de.nFlags ;
			aIndexes.AddArray( pDegIndex, de.nCount ) ;
		}
		if ( iDeg11 == 0 )
		{
			aIndexes.Add( (uint32_t) (iCol + iLineEnd) ) ;
		}
		else
		{
			DegenerateEntry		de ;
			const uint32_t *	pDegIndex = GetDegenerateAt( de, iCol + iLineEnd ) ;
			ESLVerify( pDegIndex != nullptr ) ;
			nFlags |= de.nFlags ;
			aIndexes.AddArray( pDegIndex, de.nCount ) ;
		}
		SetDegenerate
			( nFlags,
				aIndexes.GetConstArray(),
				aIndexes.GetLength() ) ;
	}
	/*
	for ( size_t i = 0; i < m_hPatch; i ++ )
	{
		const size_t	iyLine = i * m_wPatch ;
		uint32_t	iDeg1 = m_bufDegenerate.At( iCol + iyLine ) ;
		if ( iDeg1 != 0 )
		{
			continue ;
		}
		uint32_t	iDeg2 = m_bufDegenerate.At( iCol2 + iyLine ) ;
		if ( iDeg2 == 0 )
		{
			continue ;
		}
		DegenerateEntry		de ;
		const uint32_t *	pDegIndex = GetDegenerateAt( de, iCol2 + iyLine ) ;
		ESLVerify( pDegIndex != nullptr ) ;
		//
		aIndexes.RemoveAll() ;
		aIndexes.Add( (uint32_t) (iCol + iyLine) ) ;
		for ( size_t j = i + 1; j < m_hPatch; j ++ )
		{
			const size_t	jyLine = j * m_wPatch ;
			if ( (m_bufDegenerate.At( iCol + jyLine ) == 0)
				&& (m_bufDegenerate.At( iCol2 + jyLine ) == iDeg2) )
			{
				aIndexes.Add( (uint32_t) (iCol + jyLine) ) ;
			}
		}
		if ( aIndexes.GetLength() >= 2 )
		{
			SetDegenerate
				( de.nFlags,
					aIndexes.GetConstArray(),
					aIndexes.GetLength() ) ;
		}
	}
	*/
}

// 水平ライン削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::RemoveLine( size_t iLine )
{
	ESLAssert( iLine <= m_hPatch ) ;
	if ( (m_wPatch == 0) || (m_hPatch <= 1) || (iLine > m_hPatch) )
	{
		return ;
	}
	RemovePoints( iLine * m_wPatch, m_wPatch ) ;
	RemoveFaces( iLine * m_wPatch, m_wPatch ) ;
	m_hPatch -- ;
	//
	NormalizeDegenerateEntry() ;
}

// 垂直ライン削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::RemoveColumn( size_t iCol )
{
	ESLAssert( iCol <= m_wPatch ) ;
	if ( (m_hPatch == 0) || (m_wPatch <= 1) || (iCol > m_wPatch) )
	{
		return ;
	}
	for ( size_t i = 0; i < m_hPatch; i ++ )
	{
		RemovePoints( (m_hPatch - i - 1) * m_wPatch + iCol, 1 ) ;
		RemoveFaces( (m_hPatch - i - 1) * m_wPatch + iCol, 1 ) ;
	}
	m_wPatch -- ;
	//
	NormalizeDegenerateEntry() ;
}

// ライン入れ替え
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::SwapLine( size_t iLine1, size_t iLine2 )
{
	if ( iLine1 == iLine2 )
	{
		return ;
	}
	ESLAssert( iLine1 < m_hPatch ) ;
	ESLAssert( iLine2 < m_hPatch ) ;
	if ( (iLine1 >= m_hPatch) || (iLine2 >= m_hPatch) )
	{
		return ;
	}
	if ( iLine1 > iLine2 )
	{
		size_t	iTemp = iLine2 ;
		iLine2 = iLine1 ;
		iLine1 = iTemp ;
	}
	ESLAssert( iLine2 >= 1 ) ;

	SArray<float32_t>	bufWeights1 ;
	SArray<float32_t>	bufWeights2 ;
	Elements			elVertex1 ;
	Elements			elVertex2 ;
	elVertex1.nWeights = GetWeightLayerCount() ;
	elVertex2.nWeights = elVertex1.nWeights ;
	elVertex1.pWeights = bufWeights1.GetArray( elVertex1.nWeights ) ;
	elVertex2.pWeights = bufWeights2.GetArray( elVertex2.nWeights ) ;
	//
	for ( size_t x = 0; x < m_wPatch; x ++ )
	{
		const size_t	iVertex1 = IndexFromPoint( x, iLine1 ) ;
		const size_t	iVertex2 = IndexFromPoint( x, iLine2 ) ;
		//
		GetElementsAt( elVertex1, iVertex1 ) ;
		GetElementsAt( elVertex2, iVertex2 ) ;
		SetElementsAt( iVertex2, elVertex1 ) ;
		SetElementsAt( iVertex1, elVertex2 ) ;
		//
		const size_t	iFace1 = iVertex1 ;
		const size_t	iFace2 = IndexFromPoint( x, iLine2 - 1 ) ;
		//
		int8_t	nFace1 = GetFaceAt( iFace1 ) ;
		int8_t	nFace2 = GetFaceAt( iFace2 ) ;
		SetFaceAt( iFace2, nFace1 ) ;
		SetFaceAt( iFace1, nFace2 ) ;
		//
		uint8_t	nFaceShifter1 = GetFaceShifterAt( iFace1 ) ;
		uint8_t	nFaceShifter2 = GetFaceShifterAt( iFace2 ) ;
		SetFaceShifterAt( iFace2, nFaceShifter1 ) ;
		SetFaceShifterAt( iFace1, nFaceShifter2 ) ;
		//
		uint32_t	nDeg1 = GetDegenerateNumberAt( iVertex1 ) ;
		uint32_t	nDeg2 = GetDegenerateNumberAt( iVertex2 ) ;
		SetDegenerateNumberAt( iVertex2, nDeg1 ) ;
		SetDegenerateNumberAt( iVertex1, nDeg2 ) ;
	}
	m_degenerates.SwapIndexes
		( IndexFromPoint( 0, iLine1 ),
			IndexFromPoint( 0, iLine2 ), m_wPatch ) ;
	if ( m_pspc != nullptr )
	{
		m_pspc->SwapPointIndexes
			( this, IndexFromPoint( 0, iLine1 ),
				IndexFromPoint( 0, iLine2 ), m_wPatch ) ;
	}
	VerifyDegenerate() ;
	//
	// 更新フラグ
	SetUpdateVertexFlag() ;
	SetUpdateFaceFlag() ;
}

// 頂点削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::RemovePoints( size_t iFirst, size_t nCount )
{
	// 要素削除
	m_bufVertex.Remove( iFirst, nCount ) ;
	m_bufNormal.Remove( iFirst, nCount ) ;
	m_bufUVMap.Remove( iFirst, nCount ) ;
	m_bufColor.Remove( iFirst, nCount ) ;
	m_bufDegenerate.Remove( iFirst, nCount ) ;
	//
	size_t	nWeightLayers = GetWeightLayerCount() ;
	size_t	nAreaSize = GetAreaSize() ;
	for ( size_t i = 0; i < nWeightLayers; i ++ )
	{
		m_bufWeight.Remove
			( (nWeightLayers - i - 1) * nAreaSize + iFirst, nCount ) ;
	}
	//
	m_edges.RemovePointIndex( iFirst, nCount ) ;
	m_degenerates.RemoveIndex( iFirst, nCount ) ;
	if ( m_pspc != nullptr )
	{
		m_pspc->RemovePointIndex( this, iFirst, nCount ) ;
	}
	//
	// パッチ外変則ポリゴンへの削除反映
	uint32_t *		pTriangleIndex = m_bufTriangles.GetArray() ;
	const size_t	nTriIndexCount = m_bufTriangles.GetLength() ;
	size_t			iNextIndex = 0 ;
	for ( size_t i = 0; i + 2 < nTriIndexCount; i += 3 )
	{
		if ( ((iFirst <= pTriangleIndex[i])
				&& (pTriangleIndex[i] < iFirst + nCount))
			|| ((iFirst <= pTriangleIndex[i + 1])
				&& (pTriangleIndex[i + 1] < iFirst + nCount))
			|| ((iFirst <= pTriangleIndex[i + 2])
				&& (pTriangleIndex[i + 2] < iFirst + nCount)) )
		{
			continue ;
		}
		for ( size_t j = 0; j < 3; j ++ )
		{
			uint32_t	nIndex = pTriangleIndex[i + j] ;
			if ( nIndex >= iFirst + nCount )
			{
				pTriangleIndex[iNextIndex ++] = nIndex - (uint32_t) nCount ;
			}
			else
			{
				pTriangleIndex[iNextIndex ++] = nIndex ;
			}
		}
	}
	m_bufTriangles.FinishArray() ;
	ESLAssert( iNextIndex <= m_bufTriangles.GetLength() ) ;
	m_bufTriangles.SetLength( iNextIndex ) ;
	//
	// 更新フラグ
	SetUpdateVertexFlag() ;
	SetUpdateFaceFlag() ;
}

// 頂点挿入
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::InsertPoints( size_t iFirst, size_t nCount )
{
	// 要素挿入
	m_bufVertex.Insert( iFirst, nCount ) ;
	m_bufNormal.Insert( iFirst, nCount ) ;
	m_bufUVMap.Insert( iFirst, nCount ) ;
	m_bufColor.Insert( iFirst, nCount ) ;
	m_bufDegenerate.Insert( iFirst, nCount ) ;
	//
	// ウェイト挿入
	size_t	nWeightLayers = GetWeightLayerCount() ;
	size_t	nAreaSize = GetAreaSize() ;
	for ( size_t i = 0; i < nWeightLayers; i ++ )
	{
		m_bufWeight.Insert( i * nAreaSize + iFirst, nCount ) ;
	}
	//
	// 初期値
	S3DColor	clrInit( 0xFFFFFFFF, 0 ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		m_bufColor.SetAt( iFirst + i, clrInit ) ;
	}
	//
	// 参照指標修正
	ShiftIndexOffset( iFirst, GetTotalVertexCount(), (ssize_t) nCount ) ;
	//
	// 更新フラグ
	SetUpdateVertexFlag() ;
	SetUpdateFaceFlag() ;
}

// 頂点参照インデックス操作
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::ShiftIndexOffset
				( size_t iFirst, size_t iEnd, ssize_t nOffset )
{
	m_edges.ShiftIndexOffset( iFirst, iEnd, nOffset ) ;
	m_degenerates.ShiftIndexOffset( iFirst, iEnd, nOffset ) ;
	if ( m_pspc != nullptr )
	{
		m_pspc->ShiftIndexOffset( this, iFirst, iEnd, nOffset ) ;
	}
	//
	uint32_t *		pTriangleIndex = m_bufTriangles.GetArray() ;
	const size_t	nTriIndexCount = m_bufTriangles.GetLength() ;
	for ( size_t i = 0; i < nTriIndexCount; i ++ )
	{
		uint32_t	nIndex = pTriangleIndex[i] ;
		if ( (iFirst  <= nIndex) && (nIndex < iEnd) )
		{
			pTriangleIndex[i] = (uint32_t) (nIndex + nOffset) ;
		}
	}
	m_bufTriangles.FinishArray() ;
}

// 面削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::RemoveFaces( size_t iFirst, size_t nCount )
{
	// 要素削除
	m_bufFace.Remove( iFirst, nCount ) ;
	m_bufFaceNormal.Remove( iFirst, nCount ) ;
	m_bufShifter.Remove( iFirst, nCount ) ;
	//
	// 更新フラグ
	SetUpdateVertexFlag() ;
	SetUpdateFaceFlag() ;
}

// 面挿入
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::InsertFaces( size_t iFirst, size_t nCount )
{
	// 要素挿入
	m_bufFaceNormal.Insert( iFirst, nCount ) ;
	m_bufFace.Insert( iFirst, nCount ) ;
	m_bufShifter.Insert( iFirst, nCount ) ;
	//
	// 初期値
	for ( size_t i = 0; i < nCount; i ++ )
	{
		m_bufFace.SetAt( iFirst + i, faceFront ) ;
	}
	//
	// 更新フラグ
	SetUpdateVertexFlag() ;
	SetUpdateFaceFlag() ;
}

// 頂点指標変換
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DMeshEditor::Patch::IndexFromPoint( size_t x, size_t y ) const
{
	ESLAssert( x < m_wPatch ) ;
	ESLAssert( y < m_hPatch ) ;
	return	(uint32_t) (y * m_wPatch + x) ;
}

void S3DMeshEditor::Patch::PointFromIndex( SGLPoint& pt, uint32_t i ) const
{
	ESLAssert( m_wPatch != 0 ) ;
	ESLAssert( i < GetAreaSize() ) ;
	pt.y = (int32_t) (i / m_wPatch) ;
	pt.x = (int32_t) (i - pt.y * m_wPatch) ;
}

bool S3DMeshEditor::Patch::IsValidPoint( const SGLPoint& pt ) const
{
	return	(pt.x >= 0) && (pt.y >= 0)
			&& ((size_t) pt.x < m_wPatch) && ((size_t) pt.y < m_hPatch) ;
}

bool S3DMeshEditor::Patch::IsValidPoint( size_t x, size_t y ) const
{
	return	(x < m_wPatch) && (y < m_hPatch) ;
}

// パッチ面の頂点指標（4要素）取得（up-left, up-right, down-left, down-right）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::QuadIndexesFromFaceIndex
				( size_t * pIndexes, size_t iFace ) const
{
	ESLAssert( m_wPatch != 0 ) ;
	ESLAssert( iFace < GetAreaSize() ) ;
	size_t	y = (size_t) (iFace / m_wPatch) ;
	size_t	x = iFace - y * m_wPatch ;
	return	QuadIndexesFromFacePoint( pIndexes, x, y ) ;
}

void S3DMeshEditor::Patch::QuadIndexesFromFacePoint
				( size_t * pIndexes, size_t x, size_t y ) const
{
	ESLAssert( x < m_wPatch ) ;
	ESLAssert( y < m_hPatch ) ;
	size_t	x1 = (x + 1) % m_wPatch ;
	size_t	y1 = (y + 1) % m_hPatch ;
	pIndexes[0] = y * m_wPatch + x ;
	pIndexes[1] = y * m_wPatch + x1 ;
	pIndexes[2] = y1 * m_wPatch + x ;
	pIndexes[3] = y1 * m_wPatch + x1 ;
}

// 指定頂点を含む稜線取得
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditor::Patch::EnumeratePointToEdge
	( SSystem::SArraySet<size_t>& asEdgePoint, size_t iOrgPoint ) const
{
	bool	flagAdded = false ;
	if ( iOrgPoint < GetAreaSize() )
	{
		SGLPoint	ptOrg ;
		PointFromIndex( ptOrg, (uint32_t) iOrgPoint ) ;
		//
		if ( m_wPatch >= 2 )
		{
			if ( ptOrg.x > 0 )
			{
				asEdgePoint.AddSorted
					( (size_t) IndexFromPoint
						( (size_t) ptOrg.x - 1, (size_t) ptOrg.y ) ) ;
			}
			else if ( m_nFlags & flagHorzLoop )
			{
				asEdgePoint.AddSorted
					( (size_t) IndexFromPoint
						( m_wPatch - 1, (size_t) ptOrg.y ) ) ;
			}
			if ( (size_t) ptOrg.x + 1 < m_wPatch )
			{
				asEdgePoint.AddSorted
					( (size_t) IndexFromPoint
						( (size_t) ptOrg.x + 1, (size_t) ptOrg.y ) ) ;
			}
			else if ( m_nFlags & flagHorzLoop )
			{
				asEdgePoint.AddSorted
					( (size_t) IndexFromPoint( 0, (size_t) ptOrg.y ) ) ;
			}
			flagAdded = true ;
		}
		if ( m_hPatch >= 2 )
		{
			if ( ptOrg.y > 0 )
			{
				asEdgePoint.AddSorted
					( (size_t) IndexFromPoint
						( (size_t) ptOrg.x, (size_t) ptOrg.y - 1 ) ) ;
			}
			else if ( m_nFlags & flagVertLoop )
			{
				asEdgePoint.AddSorted
					( (size_t) IndexFromPoint
						( (size_t) ptOrg.x, m_hPatch - 1 ) ) ;
			}
			if ( (size_t) ptOrg.y + 1 < m_hPatch )
			{
				asEdgePoint.AddSorted
					( (size_t) IndexFromPoint
						( (size_t) ptOrg.x, (size_t) ptOrg.y + 1 ) ) ;
			}
			else if ( m_nFlags & flagVertLoop )
			{
				asEdgePoint.AddSorted
					( (size_t) IndexFromPoint( (size_t) ptOrg.x, 0 ) ) ;
			}
			flagAdded = true ;
		}
	}
	size_t	nExFaceCount = GetExFaceCount() ;
	const uint32_t *
			pIndexes = GetExFaceTriangleIndexes( 0, nExFaceCount ) ;
	for ( size_t i = 0, j = 0; i < nExFaceCount; i ++, j += 3 )
	{
		if ( pIndexes[j] == iOrgPoint )
		{
			asEdgePoint.AddSorted( (size_t) pIndexes[j + 1] ) ;
			asEdgePoint.AddSorted( (size_t) pIndexes[j + 2] ) ;
			flagAdded = true ;
		}
		else if ( pIndexes[j + 1] == iOrgPoint )
		{
			asEdgePoint.AddSorted( (size_t) pIndexes[j] ) ;
			asEdgePoint.AddSorted( (size_t) pIndexes[j + 2] ) ;
			flagAdded = true ;
		}
		else if ( pIndexes[j + 2] == iOrgPoint )
		{
			asEdgePoint.AddSorted( (size_t) pIndexes[j] ) ;
			asEdgePoint.AddSorted( (size_t) pIndexes[j + 1] ) ;
			flagAdded = true ;
		}
	}
	return	flagAdded ;
}

// 稜線判定（パッチ内のみ）
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditor::Patch::IsPatchEdge( size_t iVertex0, size_t iVertex1 ) const
{
	const size_t	nAreaSize = GetAreaSize() ;
	if ( (iVertex0 >= nAreaSize) || (iVertex1 >= nAreaSize) )
	{
		return	false ;
	}
	SGLPoint	pt0, pt1 ;
	PointFromIndex( pt0, (uint32_t) iVertex0 ) ;
	PointFromIndex( pt1, (uint32_t) iVertex1 ) ;
	//
	if ( pt0.x == pt1.x )
	{
		if ( (pt0.y + 1 == pt1.y) || (pt0.y == pt1.y + 1) )
		{
			return	true ;
		}
		if ( m_nFlags & flagVertLoop )
		{
			const int	hBottom = (int) m_hPatch - 1 ;
			return	((pt0.y == 0) || (pt1.y == 0))
					&& ((pt0.y == hBottom) || (pt1.y == hBottom)) ;
		}
	}
	else if ( pt0.y == pt1.y )
	{
		if ( (pt0.x + 1 == pt1.x) || (pt0.x == pt1.x + 1) )
		{
			return	true ;
		}
		if ( m_nFlags & flagHorzLoop )
		{
			const int	wRight = (int) m_wPatch - 1 ;
			return	((pt0.x == 0) || (pt1.x == 0))
					&& ((pt0.x == wRight) || (pt1.x == wRight)) ;
		}
	}
	return	false ;
}

// 稜線を挟む面を取得（パッチ内のみ）
// （iVertex0->iVertex1 に向かって pFaces[0]:右側, pFaces[1]:左側）
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditor::Patch::GetSideFaceByEdge
	( size_t * pFaces /*[2]*/, size_t iVertex0, size_t iVertex1 ) const
{
	const size_t	nAreaSize = GetAreaSize() ;
	if ( (iVertex0 >= nAreaSize)
		|| (iVertex1 >= nAreaSize)
		|| (iVertex0 == iVertex1) )
	{
		return	false ;
	}
	SGLPoint	pt0, pt1 ;
	size_t		iDst0, iDst1 ;
	if ( iVertex0 < iVertex1 )
	{
		PointFromIndex( pt0, (uint32_t) iVertex0 ) ;
		PointFromIndex( pt1, (uint32_t) iVertex1 ) ;
		iDst0 = 0 ;
		iDst1 = 1 ;
	}
	else
	{
		PointFromIndex( pt0, (uint32_t) iVertex1 ) ;
		PointFromIndex( pt1, (uint32_t) iVertex0 ) ;
		iDst0 = 1 ;
		iDst1 = 0 ;
	}
	if ( pt0.x == pt1.x )
	{
		if ( pt0.y + 1 != pt1.y )
		{
			const int	hBottom = (int) m_hPatch - 1 ;
			if ( (pt0.y != 0) || (pt1.y != hBottom) )
			{
				return	false ;
			}
			pt0 = pt1 ;
		}
		if ( pt0.x == 0 )
		{
			pFaces[iDst0] = (size_t) pt0.y * m_wPatch + (m_wPatch - 1) ;
		}
		else
		{
			pFaces[iDst0] = (size_t) pt0.y * m_wPatch + (size_t) (pt0.x - 1) ;
		}
		pFaces[iDst1] = (size_t) pt0.y * m_wPatch + (size_t) pt0.x ;
		return	true ;
	}
	else if ( pt0.y == pt1.y )
	{
		if ( pt0.x + 1 != pt1.x )
		{
			const int	wRight = (int) m_wPatch - 1 ;
			if ( (pt0.x != 0) || (pt1.x != wRight) )
			{
				return	false ;
			}
			pt0 = pt1 ;
		}
		if ( pt0.y == 0 )
		{
			pFaces[iDst1] = (m_hPatch - 1) * m_wPatch + (size_t) pt0.x ;
		}
		else
		{
			pFaces[iDst1] = (size_t) (pt0.y - 1) * m_wPatch + (size_t) pt0.x ;
		}
		pFaces[iDst0] = (size_t) pt0.y * m_wPatch + (size_t) pt0.x ;
		return	true ;
	}
	return	false ;
}

// 有効面が存在しないか？
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditor::Patch::IsFaceEmpty( void ) const
{
	const size_t	nFaceCount = GetTotalFaceCount() ;
	for ( size_t i = 0; i < nFaceCount; i ++ )
	{
		if ( IsValidFaceAreaAt( i )
			&& (GetFaceAt( i ) != faceNull) )
		{
			return	false ;
		}
	}
	return	true ;
}

// ラインはループか？
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditor::Patch::IsLineLoop
			( S3DMeshEditor::LineDirection lineDir ) const
{
	if ( lineDir == lineHorizontal )
	{
		return	(m_nFlags & flagHorzLoop) != 0 ;
	}
	else
	{
		return	(m_nFlags & flagVertLoop) != 0 ;
	}
}

// パッチ外頂点も含めた全頂点数・面数
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshEditor::Patch::GetTotalVertexCount( void ) const
{
	return	m_bufVertex.GetLength() ;
}

size_t S3DMeshEditor::Patch::GetTotalFaceCount( void ) const
{
	return	GetAreaSize() + m_bufTriangles.GetLength() / 3 ;
}

// 座標
//////////////////////////////////////////////////////////////////////////////
const S3DVector& S3DMeshEditor::Patch::GetPoint( size_t x, size_t y ) const
{
	ESLAssert( x < m_wPatch ) ;
	ESLAssert( y < m_hPatch ) ;
	return	m_bufVertex.At( y * m_wPatch + x ) ;
}

const S3DVector& S3DMeshEditor::Patch::GetPointAt( size_t i ) const
{
	ESLAssert( i < GetTotalVertexCount() ) ;
	return	m_bufVertex.At( i ) ;
}

const S3DVector * S3DMeshEditor::Patch::GetConstPointArray( size_t i, size_t n ) const
{
	ESLAssert( i + n <= GetTotalVertexCount() ) ;
	return	m_bufVertex.GetConstArray() + i ;
}

void S3DMeshEditor::Patch::SetPoint( size_t x, size_t y, const S3DVector& v )
{
	ESLAssert( x < m_wPatch ) ;
	ESLAssert( y < m_hPatch ) ;
	m_bufVertex.SetAt( y * m_wPatch + x, v ) ;
	m_flagUpdateVertex = true ;
	m_flagUpdateSerialize = true ;
}

void S3DMeshEditor::Patch::SetPointAt( size_t i, const S3DVector& v )
{
	ESLAssert( i < GetTotalVertexCount() ) ;
	m_bufVertex.SetAt( i, v ) ;
	m_flagUpdateVertex = true ;
	m_flagUpdateSerialize = true ;
}

// 法線
//////////////////////////////////////////////////////////////////////////////
const S3DVector& S3DMeshEditor::Patch::GetNormal( size_t x, size_t y ) const
{
	ESLAssert( x < m_wPatch ) ;
	ESLAssert( y < m_hPatch ) ;
	return	m_bufNormal.At( y * m_wPatch + x ) ;
}

const S3DVector& S3DMeshEditor::Patch::GetNormalAt( size_t i ) const
{
	ESLAssert( i < GetTotalVertexCount() ) ;
	return	m_bufNormal.At( i ) ;
}

bool S3DMeshEditor::Patch::ShouldNormalInverseAt( size_t i ) const
{
	if ( i < m_bufIUsedVertex.GetLength() )
	{
		size_t	j = m_bufIUsedVertex.At(i) ;
		if ( j < m_bufUsedNormalFace.GetLength() )
		{
			return	(m_bufUsedNormalFace.At(j) < 0) ;
		}
	}
	return	false ;
}

const S3DVector * S3DMeshEditor::Patch::GetConstNormalArray( size_t i, size_t n ) const
{
	ESLAssert( i + n <= GetTotalVertexCount() ) ;
	return	m_bufNormal.GetConstArray() + i ;
}

void S3DMeshEditor::Patch::SetNormal( size_t x, size_t y, const S3DVector& v )
{
	ESLAssert( x < m_wPatch ) ;
	ESLAssert( y < m_hPatch ) ;
	m_bufNormal.SetAt( y * m_wPatch + x, v ) ;
	m_flagUpdateSerialize = true ;
}

void S3DMeshEditor::Patch::SetNormalAt( size_t i, const S3DVector& v )
{
	ESLAssert( i < GetTotalVertexCount() ) ;
	m_bufNormal.SetAt( i, v ) ;
	m_flagUpdateSerialize = true ;
}

// 面法線
//////////////////////////////////////////////////////////////////////////////
const S3DVector& S3DMeshEditor::Patch::GetFaceNormal( size_t x, size_t y ) const
{
	ESLAssert( x < m_wPatch ) ;
	ESLAssert( y < m_hPatch ) ;
	return	m_bufFaceNormal.At( y * m_wPatch + x ) ;
}

const S3DVector& S3DMeshEditor::Patch::GetFaceNormalAt( size_t i ) const
{
	ESLAssert( i < GetTotalFaceCount() ) ;
	return	m_bufFaceNormal.At( i ) ;
}

void S3DMeshEditor::Patch::SetFaceNormal( size_t x, size_t y, const S3DVector& v )
{
	ESLAssert( x < m_wPatch ) ;
	ESLAssert( y < m_hPatch ) ;
	m_bufFaceNormal.SetAt( y * m_wPatch + x, v ) ;
}

void S3DMeshEditor::Patch::SetFaceNormalAt( size_t i, const S3DVector& v )
{
	ESLAssert( i < GetTotalFaceCount() ) ;
	m_bufFaceNormal.SetAt( i, v ) ;
}

// UV
//////////////////////////////////////////////////////////////////////////////
const S2DVector& S3DMeshEditor::Patch::GetUV( size_t x, size_t y ) const
{
	ESLAssert( x < m_wPatch ) ;
	ESLAssert( y < m_hPatch ) ;
	return	m_bufUVMap.At( y * m_wPatch + x ) ;
}

const S2DVector& S3DMeshEditor::Patch::GetUVAt( size_t i ) const
{
	ESLAssert( i < GetTotalVertexCount() ) ;
	return	m_bufUVMap.At( i ) ;
}

const S2DVector * S3DMeshEditor::Patch::GetConstUVArray( size_t i, size_t n ) const
{
	ESLAssert( i + n <= GetTotalVertexCount() ) ;
	return	m_bufUVMap.GetConstArray() + i ;
}

void S3DMeshEditor::Patch::SetUV( size_t x, size_t y, const S2DVector& uv )
{
	ESLAssert( x < m_wPatch ) ;
	ESLAssert( y < m_hPatch ) ;
	m_bufUVMap.SetAt( y * m_wPatch + x, uv ) ;
	m_flagUpdateSerialize = true ;
}

void S3DMeshEditor::Patch::SetUVAt( size_t i, const S2DVector& uv )
{
	ESLAssert( i < GetTotalVertexCount() ) ;
	m_bufUVMap.SetAt( i, uv ) ;
	m_flagUpdateSerialize = true ;
}

// 色
//////////////////////////////////////////////////////////////////////////////
const S3DColor& S3DMeshEditor::Patch::GetColor( size_t x, size_t y ) const
{
	ESLAssert( x < m_wPatch ) ;
	ESLAssert( y < m_hPatch ) ;
	return	m_bufColor.At( y * m_wPatch + x ) ;
}

const S3DColor& S3DMeshEditor::Patch::GetColorAt( size_t i ) const
{
	ESLAssert( i < GetTotalVertexCount() ) ;
	return	m_bufColor.At( i ) ;
}

const S3DColor * S3DMeshEditor::Patch::GetConstColorArray( size_t i, size_t n ) const
{
	ESLAssert( i + n <= GetTotalVertexCount() ) ;
	return	m_bufColor.GetConstArray() + i ;
}

void S3DMeshEditor::Patch::SetColor( size_t x, size_t y, const S3DColor& color )
{
	ESLAssert( x < m_wPatch ) ;
	ESLAssert( y < m_hPatch ) ;
	m_bufColor.SetAt( y * m_wPatch + x, color ) ;
	m_flagUpdateSerialize = true ;
}

void S3DMeshEditor::Patch::SetColorAt( size_t i, const S3DColor& color )
{
	ESLAssert( i < GetTotalVertexCount() ) ;
	m_bufColor.SetAt( i, color ) ;
	m_flagUpdateSerialize = true ;
}

// 面
//////////////////////////////////////////////////////////////////////////////
int8_t S3DMeshEditor::Patch::GetFace( size_t x, size_t y ) const
{
	ESLAssert( x < m_wPatch ) ;
	ESLAssert( y < m_hPatch ) ;
	return	m_bufFace.At( y * m_wPatch + x ) ;
}

int8_t S3DMeshEditor::Patch::GetFaceAt( size_t i ) const
{
	ESLAssert( i < GetTotalFaceCount() ) ;
	return	m_bufFace.At( i ) ;
}

bool S3DMeshEditor::Patch::IsValidFaceArea( size_t x, size_t y ) const
{
	return	(x < GetFaceWidth()) && (y < GetFaceHeight()) ;
}

bool S3DMeshEditor::Patch::IsValidFaceAreaAt( size_t i ) const
{
	if ( i < GetAreaSize() )
	{
		ESLAssert( m_wPatch != 0 ) ;
		size_t	y = i / m_wPatch ;
		size_t	x = i - y * m_wPatch ;
		return	(x < GetFaceWidth()) && (y < GetFaceHeight()) ;
	}
	else
	{
		return	(i < GetTotalFaceCount()) ;
	}
}

const int8_t * S3DMeshEditor::Patch::GetConstFaceArray( size_t i, size_t n ) const
{
	ESLAssert( i + n <= GetTotalFaceCount() ) ;
	return	m_bufFace.GetConstArray() + i ;
}

void S3DMeshEditor::Patch::SetFace( size_t x, size_t y, int8_t nFace )
{
	ESLAssert( x < m_wPatch ) ;
	ESLAssert( y < m_hPatch ) ;
	m_bufFace.SetAt( y * m_wPatch + x, nFace ) ;
	m_flagUpdateFace = true ;
	m_flagUpdateSerialize = true ;
}

void S3DMeshEditor::Patch::SetFaceAt( size_t i, int8_t nFace )
{
	ESLAssert( i < GetTotalFaceCount() ) ;
	m_bufFace.SetAt( i, nFace ) ;
	m_flagUpdateFace = true ;
	m_flagUpdateSerialize = true ;
}

// 面三角化シフタ
//////////////////////////////////////////////////////////////////////////////
uint8_t S3DMeshEditor::Patch::GetFaceShifterAt( size_t i ) const
{
	ESLAssert( i < GetTotalFaceCount() ) ;
	return	m_bufShifter.At( i ) ;
}

const uint8_t * S3DMeshEditor::Patch::GetConstFaceShifterArray( size_t i, size_t n ) const
{
	ESLAssert( i + n <= GetTotalFaceCount() ) ;
	return	m_bufShifter.GetConstArray() + i ;
}

void S3DMeshEditor::Patch::SetFaceShifterAt( size_t i, uint8_t nShifter )
{
	ESLAssert( i < GetTotalFaceCount() ) ;
	m_bufShifter.SetAt( i, nShifter ) ;
	m_flagUpdateVertex = true ;
	m_flagUpdateFace = true ;
	m_flagUpdateSerialize = true ;
}

// ウェイト
//////////////////////////////////////////////////////////////////////////////
float32_t S3DMeshEditor::Patch::GetWeight( size_t x, size_t y, size_t z ) const
{
	ESLAssert( x < m_wPatch ) ;
	ESLAssert( y < m_hPatch ) ;
	ESLAssert( z < GetWeightLayerCount() ) ;
	return	m_bufWeight.At( z * m_bufVertex.GetLength() + y * m_wPatch + x ) ;
}

float32_t S3DMeshEditor::Patch::GetWeightAt( size_t i, size_t z ) const
{
	ESLAssert( i < GetTotalVertexCount() ) ;
	ESLAssert( z < GetWeightLayerCount() ) ;
	return	m_bufWeight.At( z * m_bufVertex.GetLength() + i ) ;
}

const float32_t * S3DMeshEditor::Patch::GetConstWeightArrayAt( size_t z ) const
{
	ESLAssert( z < GetWeightLayerCount() ) ;
	return	m_bufWeight.GetConstArray() + z * m_bufVertex.GetLength() ;
}

void S3DMeshEditor::Patch::SetWeight( size_t x, size_t y, size_t z, float32_t w )
{
	ESLAssert( x < m_wPatch ) ;
	ESLAssert( y < m_hPatch ) ;
	m_bufWeight.SetAt( z * m_bufVertex.GetLength() + y * m_wPatch + x, w ) ;
	m_flagUpdateSerialize = true ;
}

void S3DMeshEditor::Patch::SetWeightAt( size_t i, size_t z, float32_t w )
{
	ESLAssert( i < GetTotalVertexCount() ) ;
	ESLAssert( z < GetWeightLayerCount() ) ;
	m_bufWeight.SetAt( z * m_bufVertex.GetLength() + i, w ) ;
	m_flagUpdateSerialize = true ;
}

// 全要素
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::GetElements
	( S3DMeshEditor::Patch::Elements& el, size_t x, size_t y ) const
{
	ESLAssert( x < m_wPatch ) ;
	ESLAssert( y < m_hPatch ) ;
	GetElementsAt( el, y * m_wPatch + x ) ;
}

void S3DMeshEditor::Patch::GetElementsAt
	( S3DMeshEditor::Patch::Elements& el, size_t i ) const
{
	ESLAssert( i < GetTotalVertexCount() ) ;
	el.pos = m_bufVertex.At( i ) ;
	el.normal = m_bufNormal.At( i ) ;
	el.uv = m_bufUVMap.At( i ) ;
	el.color = m_bufColor.At( i ) ;
	//
	ESLAssert( el.nWeights <= m_nWeightLayers ) ;
	const size_t	nWeightCount = el.nWeights ;
	const size_t	nVertexCount = m_bufVertex.GetLength() ;
	float32_t *		pWeights = el.pWeights ;
	for ( size_t j = 0; j < nWeightCount; j ++ )
	{
		pWeights[j] = m_bufWeight.At( j * nVertexCount + i ) ;
	}
}

void S3DMeshEditor::Patch::SetElements
	( size_t x, size_t y, const S3DMeshEditor::Patch::Elements& el )
{
	ESLAssert( x < m_wPatch ) ;
	ESLAssert( y < m_hPatch ) ;
	SetElementsAt( y * m_wPatch + x, el ) ;
}

void S3DMeshEditor::Patch::SetElementsAt( size_t i, const Elements& el )
{
	m_bufVertex.At( i ) = el.pos ;
	m_bufNormal.At( i ) = el.normal ;
	m_bufUVMap.At( i ) = el.uv ;
	m_bufColor.At( i ) = el.color ;
	//
	ESLAssert( el.nWeights <= m_nWeightLayers ) ;
	const size_t	nWeightCount = el.nWeights ;
	const size_t	nVertexCount = m_bufVertex.GetLength() ;
	float32_t *		pWeights = el.pWeights ;
	for ( size_t j = 0; j < nWeightCount; j ++ )
	{
		m_bufWeight.At( j * nVertexCount + i ) = pWeights[j] ;
	}
	//
	m_flagUpdateVertex = true ;
	m_flagUpdateFace = true ;
	m_flagUpdateSerialize = true ;
}

void S3DMeshEditor::Patch::LerpElements
	( S3DMeshEditor::Patch::Elements& el0,
		const S3DMeshEditor::Patch::Elements& el1, float32_t t )
{
	t = esl_fclampf( t, 0.0f, 1.0f ) ;
	float32_t	nt = 1.0f - t ;
	uint32_t	ut = esl_roundfi( t * 0x100 ) ;
	uint32_t	unt = 0x100 - ut ;
	el0.pos += (el1.pos - el0.pos) * t ;
	el0.normal = (el0.normal * nt + el1.normal * t).Normalized() ;
	el0.uv += (el1.uv - el0.uv) * t ;
	el0.color = el0.color.imul(unt) + el1.color.imul(ut) ;
	//
	ESLAssert( el0.nWeights == el1.nWeights ) ;
	const size_t		nWeightCount = el0.nWeights ;
	float32_t *			pWeights0 = el0.pWeights ;
	const float32_t *	pWeights1 = el1.pWeights ;
	for ( size_t i = 0; i < nWeightCount; i ++ )
	{
		pWeights0[i] += (pWeights1[i] - pWeights0[i]) * t ;
	}
}

// 縮退頂点（逆引き番号）
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DMeshEditor::Patch::GetDegenerateNumber( size_t x, size_t y ) const
{
	ESLAssert( x < m_wPatch ) ;
	ESLAssert( y < m_hPatch ) ;
	return	GetDegenerateNumberAt( y * m_wPatch + x ) ;
}

uint32_t S3DMeshEditor::Patch::GetDegenerateNumberAt( size_t iVertex ) const
{
	ESLAssert( iVertex < GetTotalVertexCount() ) ;
	return	m_bufDegenerate.At( iVertex ) ;
}

const uint32_t *
	S3DMeshEditor::Patch::GetDegenerateNumberArrayAt( size_t iVertex, size_t nCount ) const
{
	ESLAssert( iVertex + nCount <= GetTotalVertexCount() ) ;
	return	m_bufDegenerate.GetConstArray() + iVertex ;
}

void S3DMeshEditor::Patch::SetDegenerateNumber( size_t x, size_t y, uint32_t nDeg )
{
	ESLAssert( x < m_wPatch ) ;
	ESLAssert( y < m_hPatch ) ;
	SetDegenerateNumberAt( y * m_wPatch + x, nDeg ) ;
}

void S3DMeshEditor::Patch::SetDegenerateNumberAt( size_t iVertex, uint32_t nDeg )
{
	ESLAssert( iVertex < GetTotalVertexCount() ) ;
	m_bufDegenerate.SetAt( iVertex, nDeg ) ;
}

void S3DMeshEditor::Patch::RebuildDegenerateByNumber( void )
{
	m_degenerates.RemoveAllIndex() ;
	//
	const uint32_t *	pDegIndexes = m_bufDegenerate.GetConstArray() ;
	const size_t		nVertexCount = m_bufDegenerate.GetLength() ;
	for ( size_t i = 0; i < nVertexCount; i ++ )
	{
		size_t	iDeg = (size_t) pDegIndexes[i] ;
		if ( iDeg > 0 )
		{
			m_degenerates.AddDegenerateEntryAt( iDeg - 1, (uint32_t) i ) ;
		}
	}
}

// 縮退エントリ配列
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditor::DegenerateCollection&
	S3DMeshEditor::Patch::GetDegenerateCollection( void )
{
	return	m_degenerates ;
}

const S3DMeshEditor::DegenerateCollection&
	S3DMeshEditor::Patch::GetDegenerateCollection( void ) const
{
	return	m_degenerates ;
}

// 縮退頂点とエントリの整合性検証
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditor::Patch::VerifyDegenerate( void ) const
{
	const DegenerateEntry *	pde = m_degenerates.m_entries.GetConstArray() ;
	const size_t			nEtries = m_degenerates.m_entries.GetLength() ;
	const uint32_t *		pIndexes = m_degenerates.m_indexes.GetConstArray() ;
	const size_t			nIndexCount = m_degenerates.m_indexes.GetLength() ;
	//
	for ( size_t i = 0; i < nEtries; i ++ )
	{
		const DegenerateEntry	de = pde[i] ;
		for ( size_t j = 0; j < de.nCount; j ++ )
		{
			ESLAssert( pIndexes[de.iRef + j] < m_bufDegenerate.GetLength() ) ;
			if ( pIndexes[de.iRef + j] >= m_bufDegenerate.GetLength() )
			{
				return	false ;
			}
			ESLAssert( m_bufDegenerate.At( pIndexes[de.iRef + j] ) == i + 1 ) ;
			if ( m_bufDegenerate.At( pIndexes[de.iRef + j] ) != i + 1 )
			{
				return	false ;
			}
		}
	}
	return	true ;
}

// 縮退頂点
//////////////////////////////////////////////////////////////////////////////
const uint32_t * S3DMeshEditor::Patch::GetDegenerate
	( S3DMeshEditor::DegenerateEntry& de, size_t x, size_t y ) const
{
	ESLAssert( x < m_wPatch ) ;
	ESLAssert( y < m_hPatch ) ;
	return	GetDegenerateAt( de, y * m_wPatch + x ) ;
}

const uint32_t * S3DMeshEditor::Patch::GetDegenerateAt
		( S3DMeshEditor::DegenerateEntry& de, size_t iVertex ) const
{
	if ( m_bufDegenerate.GetLength() <= iVertex )
	{
		de.iRef = 0 ;
		de.nCount = 0 ;
		return	nullptr ;
	}
	uint32_t	iDeg = m_bufDegenerate.At( iVertex ) ;
	if ( iDeg == 0 )
	{
		de.iRef = 0 ;
		de.nCount = 0 ;
		return	nullptr ;
	}
	return	m_degenerates.GetDegenerateAt( de, (size_t) iDeg - 1 ) ;
}

bool S3DMeshEditor::Patch::GetDegeneratedPoints( PatchPointSet& pps, size_t iVertex ) const
{
	DegenerateEntry		de ;
	const uint32_t *	pIndexes = GetDegenerateAt( de, iVertex ) ;
	if ( (pIndexes == nullptr) || (de.nCount == 0) )
	{
		return	false ;
	}
	for ( size_t i = 0; i < de.nCount; i ++ )
	{
		pps.QuickAdd( PatchPoint( (Patch*) this, (size_t) pIndexes[i] ) ) ;
	}
	return	true ;
}

void S3DMeshEditor::Patch::ReleaseDegenerate( size_t x, size_t y )
{
	ESLAssert( x < m_wPatch ) ;
	ESLAssert( y < m_hPatch ) ;
	ReleaseDegenerateAt( y * m_wPatch + x ) ;
}

void S3DMeshEditor::Patch::ReleaseDegenerateAt( size_t i )
{
	ESLAssert( i < GetTotalVertexCount() ) ;
	uint32_t	iDeg = m_bufDegenerate.At( i ) ;
	if ( iDeg == 0 )
	{
		return ;
	}
	// 縮退情報削除
	DegenerateEntry		de ;
	const uint32_t *	pIndex =
				m_degenerates.GetDegenerateAt( de, (size_t) iDeg - 1 ) ;
	for ( size_t i = 0; i < de.nCount; i ++ )
	{
		ESLAssert( pIndex[i] < m_bufDegenerate.GetLength() ) ;
		ESLAssert( m_bufDegenerate.At(pIndex[i]) == iDeg ) ;
		m_bufDegenerate.SetAt( pIndex[i], 0 ) ;
	}
	m_degenerates.RemoveAt( (size_t) iDeg - 1 ) ;
	//
	// 縮退指標修正
	uint32_t *		pDegIndex = m_bufDegenerate.GetArray() ;
	const size_t	nDegCount = m_bufDegenerate.GetLength() ;
	for ( size_t i = 0; i < nDegCount; i ++ )
	{
		if ( pDegIndex[i] >= iDeg )
		{
			pDegIndex[i] -- ;
		}
	}
	m_bufDegenerate.FinishArray() ;
	SetUpdateVertexFlag() ;
}

void S3DMeshEditor::Patch::SetDegenerate
	( uint32_t nFlags, const SGLPoint * pDegenerate, size_t nCount )
{
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ReleaseDegenerate
			( (size_t) pDegenerate[i].x, (size_t) pDegenerate[i].y ) ;
	}
	SArray<uint32_t>	bufIndexes ;
	uint32_t *			pIndexes = bufIndexes.GetArray( nCount ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pIndexes[i] =
			IndexFromPoint
				( (size_t) pDegenerate[i].x, (size_t) pDegenerate[i].y ) ;
	}
	size_t	iNewDeg = m_degenerates.AddDegenerate( nFlags, pIndexes, nCount ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		m_bufDegenerate.SetAt( pIndexes[i], (uint32_t) iNewDeg + 1 ) ;
	}
	bufIndexes.FinishArray() ;
	SetUpdateVertexFlag() ;
}

void S3DMeshEditor::Patch::SetDegenerate
	( uint32_t nFlags, const uint32_t * pIndexes, size_t nCount )
{
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ReleaseDegenerateAt( (size_t) pIndexes[i] ) ;
	}
	size_t	iNewDeg = m_degenerates.AddDegenerate( nFlags, pIndexes, nCount ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		m_bufDegenerate.SetAt( pIndexes[i], (uint32_t) iNewDeg + 1 ) ;
	}
	SetUpdateVertexFlag() ;
}

bool S3DMeshEditor::Patch::ChangeDegenerateFlagAt( size_t iVertex, uint32_t nFlags )
{
	ESLAssert( iVertex < GetTotalVertexCount() ) ;
	uint32_t	iDeg = m_bufDegenerate.At( iVertex ) ;
	if ( iDeg == 0 )
	{
		return	false ;
	}
	m_degenerates.ChangeDegenerateFlagAt( (size_t) iDeg - 1, nFlags ) ;
	SetUpdateVertexFlag() ;
	return	true ;
}

// ライン縮退判定
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditor::Patch::IsDegeneratedLines
	( LineDirection lineDir, size_t iLine0, size_t iLine1 ) const
{
	if ( lineDir == lineHorizontal )
	{
		const size_t	nWidth = GetWidth() ;
		for ( size_t i = 0; i < nWidth; i ++ )
		{
			uint32_t	deg0 = GetDegenerateNumber( i, iLine0 ) ;
			uint32_t	deg1 = GetDegenerateNumber( i, iLine1 ) ;
			if ( (deg0 == 0) || (deg0 != deg1) )
			{
				return	false ;
			}
		}
	}
	else
	{
		const size_t	nHeight = GetHeight() ;
		for ( size_t i = 0; i < nHeight; i ++ )
		{
			uint32_t	deg0 = GetDegenerateNumber( iLine0, i ) ;
			uint32_t	deg1 = GetDegenerateNumber( iLine1, i ) ;
			if ( (deg0 == 0) || (deg0 != deg1) )
			{
				return	false ;
			}
		}
	}
	return	true ;
}

// 縮退頂点を収束させる
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::ShrinkDegeneratedPoints( void )
{
	const size_t			nEntries = m_degenerates.m_entries.GetLength() ;
	const DegenerateEntry *	pde = m_degenerates.m_entries.GetConstArray() ;
	const uint32_t *		pIndexes = m_degenerates.m_indexes.GetConstArray() ;
	//
	for ( size_t i = 0; i < nEntries; i ++ )
	{
		DegenerateEntry	de = pde[i] ;
		if ( de.nCount == 0 )
		{
			continue ;
		}
		S3DVector	vPos( 0, 0, 0 ) ;
		for ( size_t j = 0; j < de.nCount; j ++ )
		{
			vPos += GetPointAt( pIndexes[de.iRef + j] ) ;
		}
		vPos *= 1.0f / (float32_t) de.nCount ;
		//
		for ( size_t j = 0; j < de.nCount; j ++ )
		{
			SetPointAt( pIndexes[de.iRef + j], vPos ) ;
		}
	}
}

// 縮退頂点のない空のエントリを削除する
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::NormalizeDegenerateEntry( void )
{
	uint32_t *		pDegIndex = m_bufDegenerate.GetArray() ;
	const size_t	nDegCount = m_bufDegenerate.GetLength() ;
	//
	for ( size_t i = 0; i < m_degenerates.m_entries.GetLength(); i ++ )
	{
		const DegenerateEntry&	de = m_degenerates.m_entries.At(i) ;
		if ( de.nCount <= 1 )
		{
			uint32_t	iDeg = (uint32_t) i + 1 ;
			for ( size_t j = 0; j < nDegCount; j ++ )
			{
				if ( pDegIndex[j] == iDeg )
				{
					pDegIndex[j] = 0 ;
				}
				else if ( pDegIndex[j] > iDeg )
				{
					pDegIndex[j] -- ;
				}
			}
			//
			m_degenerates.RemoveAt( i -- ) ;
		}
	}
	m_bufDegenerate.FinishArray() ;
}

// 全ての縮退頂点をクリアする
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::ClearAllDegeneration( void )
{
	m_degenerates.RemoveAll() ;
	//
	uint32_t *		pDegIndex = m_bufDegenerate.GetArray() ;
	const size_t	nDegCount = m_bufDegenerate.GetLength() ;
	for ( size_t i = 0; i < nDegCount; i ++ )
	{
		pDegIndex[i] = 0 ;
	}
	m_bufDegenerate.FinishArray() ;
}

// ウェイトマップ数
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshEditor::Patch::GetWeightLayerCount( void ) const
{
	return	m_nWeightLayers ;
}

// ウェイトマップレイヤー追加
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::InsertWeightLayer( size_t iLayer, size_t nCount )
{
	ESLAssert( iLayer <= m_nWeightLayers ) ;
	if ( iLayer > m_nWeightLayers )
	{
		iLayer = m_nWeightLayers ;
	}
	size_t	nVertexCount = GetTotalVertexCount() ;
	m_bufWeight.Insert( iLayer * nVertexCount, nCount * nVertexCount ) ;
	m_nWeightLayers += nCount ;
	ESLAssert( m_bufWeight.GetLength() == m_nWeightLayers * nVertexCount ) ;
}

// ウェイトマップレイヤー削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::RemoveWeightLayer( size_t iLayer, size_t nCount )
{
	ESLAssert( iLayer + nCount <= m_nWeightLayers ) ;
	size_t	nVertexCount = GetTotalVertexCount() ;
	m_bufWeight.Remove( iLayer * nVertexCount, nCount * nVertexCount ) ;
	m_nWeightLayers -= nCount ;
	ESLAssert( m_bufWeight.GetLength() == m_nWeightLayers * nVertexCount ) ;
}

// ウェイトマップレイヤー入れ替え
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::SwapWeightLayer( size_t iLayer0, size_t iLayer1 )
{
	ESLAssert( iLayer0 < m_nWeightLayers ) ;
	ESLAssert( iLayer1 < m_nWeightLayers ) ;
	size_t	nVertexCount = GetTotalVertexCount() ;
	float32_t *	pfpWeight = m_bufWeight.GetArray() ;
	float32_t *	pfpWeight0 = pfpWeight + iLayer0 * nVertexCount ;
	float32_t *	pfpWeight1 = pfpWeight + iLayer1 * nVertexCount ;
	for ( size_t i = 0; i < nVertexCount; i ++ )
	{
		float32_t	w = pfpWeight0[i] ;
		pfpWeight0[i] = pfpWeight1[i] ;
		pfpWeight1[i] = w ;
	}
	m_bufWeight.FinishArray() ;
}

// パッチ外頂点数
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshEditor::Patch::GetExVertexCount( void ) const
{
	return	m_bufVertex.GetLength() - GetAreaSize() ;
}

// パッチ外面数
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshEditor::Patch::GetExFaceCount( void ) const
{
	return	m_bufFace.GetLength() - GetAreaSize() ;
}

// 面追加
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::InsertExFace
				( size_t iExFace, int8_t nFace, const uint32_t * pVertics )
{
	const size_t	nExFaceCount = GetExFaceCount() ;
	ESLAssert( iExFace <= nExFaceCount ) ;
	if ( iExFace > nExFaceCount )
	{
		iExFace = nExFaceCount ;
	}
	m_bufFaceNormal.Insert( GetAreaSize() + iExFace, 1 ) ;
	m_bufFace.InsertAt( GetAreaSize() + iExFace, nFace ) ;
	m_bufShifter.Insert( GetAreaSize() + iExFace, 0 ) ;
	m_bufTriangles.Insert( iExFace * 3, 3 ) ;
	//
	for ( size_t j = 0; j < 3; j ++ )
	{
		ESLAssert( pVertics[j] < m_bufVertex.GetLength() ) ;
		m_bufTriangles.SetAt( iExFace * 3 + j, pVertics[j] ) ;
	}
}

// 面削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::RemoveExFace( size_t iExFace )
{
	ESLAssert( iExFace <= GetExFaceCount() ) ;
	m_bufFaceNormal.RemoveAt( GetAreaSize() + iExFace ) ;
	m_bufFace.RemoveAt( GetAreaSize() + iExFace ) ;
	m_bufShifter.RemoveAt( GetAreaSize() + iExFace ) ;
	m_bufTriangles.Remove( iExFace * 3, 3 ) ;
}

// パッチ外頂点追加
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::InsertExVertics
		( size_t iExPoint, const S3DVector * pVertics, size_t nCount )
{
	const size_t	nAreaSize = GetAreaSize() ;
	InsertPoints( nAreaSize + iExPoint, nCount ) ;
	//
	for ( size_t j = 0; j < nCount; j ++ )
	{
		SetPointAt( nAreaSize + iExPoint + j, pVertics[j] ) ;
	}
}

// パッチ外頂点削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::RemoveExVertics( size_t iExPoint, size_t nCount )
{
	const size_t	nAreaSize = GetAreaSize() ;
	RemovePoints( nAreaSize + iExPoint, nCount ) ;
}

// パッチ外面検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DMeshEditor::Patch::FindExFaceVertexOf
				( size_t iVertex, size_t iFirstExFace ) const
{
	const uint32_t *	pIndexes = m_bufTriangles.GetConstArray() ;
	const size_t		nCount = m_bufTriangles.GetLength() ;
	for ( size_t i = iFirstExFace * 3; i < nCount; i ++ )
	{
		if ( pIndexes[i] == iVertex )
		{
			return	(ssize_t) (i / 3) ;
		}
	}
	return	-1 ;
}

// パッチ外面数の頂点指標（3要素）取得
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::TriangleIndexesFromExFaceIndex
						( size_t * pIndexes, size_t iExFace ) const
{
	ESLAssert( iExFace < GetExFaceCount() ) ;
	ESLAssert( iExFace * 3 + 2 < m_bufTriangles.GetLength() ) ;
	const uint32_t *	pExIndexes = m_bufTriangles.GetConstArray() ;
	pIndexes[0] = (size_t) pExIndexes[iExFace * 3] ;
	pIndexes[1] = (size_t) pExIndexes[iExFace * 3 + 1] ;
	pIndexes[2] = (size_t) pExIndexes[iExFace * 3 + 2] ;
}

// パッチ外三角頂点配列取得
//////////////////////////////////////////////////////////////////////////////
const uint32_t * S3DMeshEditor::Patch::GetExFaceTriangleIndexes( size_t iExFace, size_t nCount ) const
{
	ESLAssert( iExFace + nCount <= GetExFaceCount() ) ;
	return	m_bufTriangles.GetConstArray() + iExFace * 3 ;
}

// パッチ外三角頂点変更
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::ModifyExFaceTriangleIndexes
	( size_t iExFace, size_t nCount, const uint32_t * pIndexes )
{
	ESLAssert( iExFace + nCount <= GetExFaceCount() ) ;
	ESLAssert( (iExFace + nCount) * 3 <= m_bufTriangles.GetLength() ) ;
	uint32_t *	pExIndexes = m_bufTriangles.GetArray() + (iExFace * 3) ;
	for ( size_t i = 0, j = 0; i < nCount; i ++, j += 3 )
	{
		pExIndexes[j] = pIndexes[j] ;
		pExIndexes[j + 1] = pIndexes[j + 1] ;
		pExIndexes[j + 2] = pIndexes[j + 2] ;
	}
	m_bufTriangles.FinishArray() ;
	//
	m_flagUpdateVertex = true ;
	m_flagUpdateSerialize = true ;
}

// 三角ポリゴンメッシュを構築
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::MakeTriangleMeshFrom( const S3DRenderBuffer::MeshBuffer& mbuf )
{
	ESLAssert( m_bufVertex.GetLength() == 0 ) ;
	ESLAssert( m_bufFace.GetLength() == 0 ) ;
	//
	ESLAssert( mbuf.m_type == primitiveTriangle ) ;
	if ( mbuf.m_type != primitiveTriangle )
	{
		return ;
	}
	InsertPoints( 0, mbuf.m_nVertexCount ) ;
	//
	const size_t	nExAttrCount =
		(size_t) esl_min( (int) mbuf.m_nExAttrCount,
							(int) GetWeightLayerCount() ) ;
	//
	for ( size_t i = 0; i < mbuf.m_nVertexCount; i ++ )
	{
		SetPointAt( i, mbuf.m_bufVertex.At(i) ) ;
		SetNormalAt( i, mbuf.m_bufNormal.At(i) ) ;
		SetUVAt( i, mbuf.m_bufUVMap.At(i) ) ;
		SetColorAt( i, mbuf.m_bufColor.At(i) ) ;
		//
		size_t	iExAttr = i * mbuf.m_nExAttrCount ;
		for ( size_t j = 0; j < nExAttrCount; j ++ )
		{
			SetWeightAt( i, j, mbuf.m_bufExAttr.At(iExAttr + j) ) ;
		}
	}
	//
	InsertFaces( 0, mbuf.m_nIndexCount / 3 ) ;
	m_bufTriangles = mbuf.m_bufIndex ;
}

// BuildPatchMesh 前に実行すべき更新処理を実行する
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::UpdatePatchMesh( void )
{
	UpdateVertexNormal() ;
	MakeUsedVertexIndexArray() ;
}

// 法線計算
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::UpdateVertexNormal( void )
{
	if ( m_nFlags & flagFreezeNormal )
	{
		return ;
	}
	if ( m_flagUpdateVertex )
	{
		UpdateFaceNormal() ;
	}
	S3DVector *		pvNormal = m_bufNormal.GetArray() ;
	const size_t	nCount = m_bufNormal.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pvNormal[i].x = 0.0f ;
		pvNormal[i].y = 0.0f ;
		pvNormal[i].z = 0.0f ;
	}
	//
	// 格子共有頂点の法線を結合する
	//
	const size_t	nAreaSize = GetAreaSize() ;
	if ( nAreaSize > 0 )
	{
		ESLAssert( m_hPatch > 0 ) ;
		ESLAssert( m_wPatch > 0 ) ;
		size_t	hPatch = (m_nFlags & flagVertLoop) ? m_hPatch : m_hPatch - 1 ;
		size_t	wPatch = (m_nFlags & flagHorzLoop) ? m_wPatch : m_wPatch - 1 ;
		//
		for ( size_t y = 0; y < hPatch; y ++ )
		{
			size_t	iy0 = y * m_wPatch ;
			size_t	iy1 = iy0 + m_wPatch ;
			if ( y + 1 >= m_hPatch )
			{
				iy1 = 0 ;
			}
			for ( size_t x = 0; x < wPatch; x ++ )
			{
				size_t	x1 = x + 1 ;
				if ( x1 >= m_wPatch )
				{
					x1 = 0 ;
				}
				S3DVector	vNormal = GetFaceNormalAt( iy0 + x ) ;
				ESLAssert( iy0 + x < nAreaSize ) ;
				ESLAssert( iy0 + x1 < nAreaSize ) ;
				ESLAssert( iy1 + x < nAreaSize ) ;
				ESLAssert( iy1 + x1 < nAreaSize ) ;
				pvNormal[iy0 + x] += vNormal ;
				pvNormal[iy0 + x1] += vNormal ;
				pvNormal[iy1 + x] += vNormal ;
				pvNormal[iy1 + x1] += vNormal ;
			}
		}
	}
	//
	// パッチ外変則ポリゴンの共有頂点の法線を結合する
	//
	const uint32_t *	pIndexes = m_bufTriangles.GetConstArray() ;
	const size_t		nIndexCount = m_bufTriangles.GetLength() ;
	const size_t		nTriangleCount = nIndexCount / 3 ;
	for ( size_t i = 0, j = 0; i < nTriangleCount; i ++, j += 3 )
	{
		S3DVector	vNormal = GetFaceNormalAt( nAreaSize + i ) ;
		ESLAssert( pIndexes[j] < nCount ) ;
		ESLAssert( pIndexes[j + 1] < nCount ) ;
		ESLAssert( pIndexes[j + 2] < nCount ) ;
		if ( GetFaceAt( nAreaSize + i ) == faceBack )
		{
			vNormal = - vNormal ;
		}
		pvNormal[ pIndexes[j] ] += vNormal ;
		pvNormal[ pIndexes[j + 1] ] += vNormal ;
		pvNormal[ pIndexes[j + 2] ] += vNormal ;
	}
	//
	// 縮退頂点の法線を結合する
	//
	const DegenerateEntry *	pDegEntries = m_degenerates.m_entries.GetConstArray() ;
	const size_t			nDegCount = m_degenerates.m_entries.GetLength() ;
	const uint32_t *		pDegIndexes = m_degenerates.m_indexes.GetConstArray() ;
	for ( size_t i = 0; i < nDegCount; i ++ )
	{
		const DegenerateEntry&	de = pDegEntries[i] ;
		if ( de.nFlags & degenerateDivNormal )
		{
			continue ;
		}
		const uint32_t *	pDegRef = pDegIndexes + de.iRef ;
		S3DVector			vNormal( 0, 0, 0 ) ;
		for ( size_t j = 0; j < de.nCount; j ++ )
		{
			ESLAssert( pDegRef[j] < nCount ) ;
			S3DVector	vn = pvNormal[ pDegRef[j] ] ;
			vn.Normalize() ;
			vNormal += vn ;
		}
		for ( size_t j = 0; j < de.nCount; j ++ )
		{
			ESLAssert( pDegRef[j] < nCount ) ;
			pvNormal[ pDegRef[j] ] = vNormal ;
		}
	}
	//
	// 法線を正規化する
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pvNormal[i].Normalize() ;
	}
	m_bufNormal.FinishArray() ;
}

void S3DMeshEditor::Patch::UpdateFaceNormal( void )
{
	if ( !m_flagUpdateVertex )
	{
		return ;
	}
	S3DVector *	pvFace = m_bufFaceNormal.GetArray() ;
	ESLAssert( m_bufFaceNormal.GetLength() >= GetTotalFaceCount() ) ;
	//
	for ( size_t y = 0; y < m_hPatch; y ++ )
	{
		size_t	iy0 = y * m_wPatch ;
		size_t	iy1 = iy0 + m_wPatch ;
		if ( y + 1 >= m_hPatch )
		{
			iy1 = 0 ;
		}
		for ( size_t x = 0; x < m_wPatch; x ++ )
		{
			size_t	x1 = x + 1 ;
			if ( x1 >= m_wPatch )
			{
				x1 = 0 ;
			}
			S3DVector	v0 = GetPointAt( iy0 + x ) ;
			S3DVector	v1 = GetPointAt( iy0 + x1 ) ;
			S3DVector	v2 = GetPointAt( iy1 + x ) ;
			S3DVector	v3 = GetPointAt( iy1 + x1 ) ;
			S3DVector	vd1 = v1 - v0 ;
			S3DVector	vd2 = v2 - v0 ;
			S3DVector	vd3 = v3 - v0 ;
			vd1.Normalize() ;
			vd2.Normalize() ;
			vd3.Normalize() ;
			S3DVector	vn1 = vd3 * vd1 ;
			S3DVector	vn2 = vd2 * vd3 ;
			if ( (vn1.Absolute() > 0.0001)
				|| (vn2.Absolute() > 0.0001) )
			{
				vn1 += vn2 ;
				vn1.Normalize() ;
			}
			SetFaceNormalAt( iy0 + x, vn1 ) ;
		}
	}
	//
	const size_t		nAreaSize = GetAreaSize() ;
	const uint32_t *	pIndexes = m_bufTriangles.GetConstArray() ;
	const size_t		nIndexCount = m_bufTriangles.GetLength() ;
	const size_t		nTriangleCount = nIndexCount / 3 ;
	for ( size_t i = 0, j = 0; i < nTriangleCount; i ++, j += 3 )
	{
		S3DVector	v0 = GetPointAt( pIndexes[j] ) ;
		S3DVector	v1 = GetPointAt( pIndexes[j + 1] ) ;
		S3DVector	v2 = GetPointAt( pIndexes[j + 2] ) ;
		S3DVector	vd1 = (v1 - v0).Normalized() ;
		S3DVector	vd2 = (v2 - v0).Normalized() ;
		S3DVector	vn = vd1 * vd2 ;
		if ( vn.Absolute() > 0.0001 )
		{
			vn.Normalize() ;
		}
		SetFaceNormalAt( nAreaSize + i, vn ) ;
	}
	m_bufFaceNormal.FinishArray() ;
	//
	m_flagUpdateVertex = false ;
}

// 使用頂点のみの頂点配列へ変換する参照配列を構築
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::MakeUsedVertexIndexArray( void )
{
	if ( !m_flagUpdateFace )
	{
		return ;
	}
	const size_t	nAreaSize = GetAreaSize() ;
	if ( nAreaSize == 0 )
	{
		m_bufUsedVertex.FreeArray() ;
		m_bufIUsedVertex.FreeArray() ;
		return ;
	}
	ESLAssert( m_hPatch > 0 ) ;
	ESLAssert( m_wPatch > 0 ) ;
	uint32_t *	pUsedVertex = m_bufUsedVertex.GetArray( nAreaSize ) ;
	uint32_t *	pIUsedVertex = m_bufIUsedVertex.GetArray( nAreaSize ) ;
	int8_t *	pUsedNormalFace = m_bufUsedNormalFace.GetArray( nAreaSize ) ;
	size_t	iUsedDst = 0 ;
	for ( size_t y = 0; y < m_hPatch; y ++ )
	{
		size_t	y0 = y - 1 ;
		size_t	y1 = y ;
		if ( y == 0 )
		{
			if ( m_nFlags & flagVertLoop )
			{
				y0 = m_hPatch - 1 ;
			}
			else
			{
				y0 = 0 ;
			}
		}
		if ( (y + 1 == m_hPatch) && !(m_nFlags & flagVertLoop) )
		{
			y1 = y0 ;
		}
		for ( size_t x = 0; x < m_wPatch; x ++ )
		{
			size_t	x0 = x - 1 ;
			size_t	x1 = x ;
			if ( x == 0 )
			{
				if ( m_nFlags & flagHorzLoop )
				{
					x0 = m_wPatch - 1 ;
				}
				else
				{
					x0 = 0 ;
				}
			}
			if ( (x + 1 == m_wPatch) && !(m_nFlags & flagHorzLoop) )
			{
				x1 = x0 ;
			}
			pIUsedVertex[y * m_wPatch + x] = (uint32_t) iUsedDst ;
			//
			int8_t	nFace0 = GetFace( x1, y1 ) ;
			int8_t	nFace1 = GetFace( x0, y1 ) ;
			int8_t	nFace2 = GetFace( x1, y0 ) ;
			int8_t	nFace3 = GetFace( x0, y0 ) ;
			if ( (nFace0 != faceNull) || (nFace1 != faceNull)
				|| (nFace2 != faceNull) || (nFace3 != faceNull) )
			{
				pUsedNormalFace[iUsedDst] = nFace0 + nFace1 + nFace2 + nFace3 ;
				pUsedVertex[iUsedDst ++] = (uint32_t) (y * m_wPatch + x) ;
			}
		}
	}
	m_bufUsedVertex.FinishArray() ;
	m_bufIUsedVertex.FinishArray() ;
	m_bufUsedNormalFace.FinishArray() ;
	ESLAssert( iUsedDst <= nAreaSize ) ;
	//
	m_bufUsedVertex.SetLength( iUsedDst ) ;
	m_bufUsedNormalFace.SetLength( iUsedDst ) ;
	//
	m_flagUpdateFace = false ;
}

// パッチ(QUAD)メッシュの構築
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::BuildPatchMesh
	( S3DMeshEditor::MeshBuffer& mbuf,
			const S3DMeshEditor::MeshParam& mparam ) const
{
	mbuf.m_edegs.RemoveAll() ;
	//
	switch ( mparam.nFlags & flagMeshEdgeMethodMask )
	{
	case	flagMeshPartialEdge:
	default:
		BuildPatchMeshPartialEdge( mbuf, mparam ) ;
		break ;
	case	flagMeshAllSmooth:
		BuildPatchMeshAllSmooth( mbuf, mparam ) ;
		break ;
	case	flagMeshAllDivPoints:
	case	flagMeshAllFlat:
		BuildPatchMeshAllDivPoints( mbuf, mparam ) ;
		break ;
	}
}

// 部分的に稜線を分割するメッシュ構築
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::BuildPatchMeshPartialEdge
	( S3DMeshEditor::MeshBuffer& mbuf,
			const S3DMeshEditor::MeshParam& mparam ) const
{
	const size_t	nAreaSize = GetAreaSize() ;
	if ( nAreaSize == 0 )
	{
		mbuf.m_countVertex = 0 ;
		mbuf.m_countIndex = 0 ;
		mbuf.m_nWeightLayers = 0 ;
		return ;
	}
	//
	// 頂点情報の複製
	//
	BuildPatchUsedVertex( mbuf ) ;
	//
	// 稜線法線分割判定と四角ポリゴン指標の構築
	//
	ESLAssert( m_hPatch > 0 ) ;
	ESLAssert( m_wPatch > 0 ) ;
	const size_t	hPatch = (m_nFlags & flagVertLoop) ? m_hPatch : m_hPatch - 1 ;
	const size_t	wPatch = (m_nFlags & flagHorzLoop) ? m_wPatch : m_wPatch - 1 ;
	const float32_t	cosEdgeAngle = (float32_t) cos( mparam.fpEdgeAngle * PI / 180.0 ) ;
	//
	const uint32_t *	pIUsedVertex = m_bufIUsedVertex.GetConstArray() ;
	const int8_t *		pUsedNormalFace = m_bufUsedNormalFace.GetConstArray() ;
	uint32_t *			pDstIndex = mbuf.m_bufIndex.GetArray( hPatch * wPatch * 4 ) ;
	size_t				iDst = 0 ;
	//
	for ( size_t y = 0; y < hPatch; y ++ )
	{
		size_t	iy0 = y * m_wPatch ;		// 頂点指標
		size_t	iy1 = iy0 + m_wPatch ;
		if ( y + 1 >= m_hPatch )
		{
			iy1 = 0 ;
		}
		size_t	ify1 = y * m_wPatch ;		// 面指標
		size_t	ify0 = (y - 1) * m_wPatch ;
		size_t	ify2 = (y + 1) * m_wPatch ;
		if ( y == 0 )
		{
			ify0 = (hPatch - 1) * m_wPatch ;
		}
		if ( y == hPatch - 1 )
		{
			ify2 = 0 ;
		}
		//
		for ( size_t x = 0; x < wPatch; x ++ )
		{
			size_t	x1 = x + 1 ;	// 頂点指標
			if ( x1 >= m_wPatch )
			{
				x1 = 0 ;
			}
			size_t	ifx1 = x ;		// 面指標
			size_t	ifx0 = x - 1 ;
			size_t	ifx2 = x + 1 ;
			if ( x == 0 )
			{
				ifx0 = wPatch - 1 ;
			}
			if ( x == wPatch - 1 )
			{
				ifx2 = 0 ;
			}
			//
			int8_t	nFace = GetFaceAt( ify1 + x ) ;
			if ( nFace == faceNull )
			{
				continue ;
			}
			ESLAssert( iy0 + x < nAreaSize ) ;
			ESLAssert( iy0 + x1 < nAreaSize ) ;
			ESLAssert( iy1 + x < nAreaSize ) ;
			ESLAssert( iy1 + x1 < nAreaSize ) ;
			static const size_t	iEdge[4][2] =
			{
				// 稜線の頂点指標
				{ 0, 1 }, { 1, 3 }, { 0, 2 }, { 2, 3 }
			} ;
			static const size_t	iEdgeByVertex[4][2] =
			{
				// 各頂点を含む稜線の指標
				{ 0, 2 }, { 0, 1 }, { 2, 3 }, { 1, 3 }
			} ;
			size_t	iNextFace[4] =
			{
				// 隣接面の指標
				ify0 + ifx1, ify1 + ifx2, ify1 + ifx0, ify2 + ifx1
			} ;
			bool	fEdge[4] =
			{
				// 稜線を角にするか？
				false, false, false, false
			} ;
			bool	fAdjoiningFace[4] =
			{
				// 隣接面
				true, true, true, true,
			} ;
			bool	fSharedVertex[4] =
			{
				// 共有頂点
				false, false, false, false,
			} ;
			bool	fEdgeVertex[4] =
			{
				// 各頂点を角にして分離するか？
				false, false, false, false
			} ;
			uint32_t	iVertex[4] =
			{
				// 頂点指標
				pIUsedVertex[iy0 + x],
				pIUsedVertex[iy0 + x1],
				pIUsedVertex[iy1 + x],
				pIUsedVertex[iy1 + x1]
			} ;
			uint32_t	iAliasVertex[4] =
			{
				iVertex[0], iVertex[1], iVertex[2], iVertex[3],
			} ;
			const size_t	iSrcVertex[4] =
			{
				// 頂点指標
				iy0 + x, iy0 + x1, iy1 + x, iy1 + x1
			} ;
			//
			// 隣接面の正規化（縮退している面の場合更にその隣の面を選択）
			//
			static const size_t	iNextEdge[4][2] =
			{
				// 隣接面の隣接稜線頂点指標
				{ 2, 3 }, { 0, 2 }, { 1, 3 }, { 0, 1 }
			} ;
			static const bool	fVertEdge[4] =
			{
				false, true, true, false
			} ;
			static const int	nNextDelta[4] =
			{
				-1, 1, -1, 1
			} ;
			for ( int i = 0; i < 4; i ++ )
			{
				DegenerateEntry	de ;
				if ( GetDegenerateAt( de, iSrcVertex[i] ) != nullptr )
				{
					fSharedVertex[i] = (de.nCount >= 2) ;
				}
				if ( !fSharedVertex[i] )
				{
					PatchPoint	pp( const_cast<Patch*>(this), iSrcVertex[i] ) ;
					if ( m_pspc->FindPoint( pp ) >= 0 )
					{
						fSharedVertex[i] = true ;
					}
				}
			}
			for ( int i = 0; i < 4; i ++ )
			{
				size_t	iFaceVert[4] ;
				QuadIndexesFromFaceIndex( iFaceVert, iNextFace[i] ) ;
				//
				S3DVector	vNext[2] ;
				S3DVector	vThis[2] ;
				vNext[0] = GetPointAt(iFaceVert[iNextEdge[i][0]]) ;
				vNext[1] = GetPointAt(iFaceVert[iNextEdge[i][1]]) ;
				vThis[0] = GetPointAt(iSrcVertex[iEdge[i][0]]) ;
				vThis[1] = GetPointAt(iSrcVertex[iEdge[i][1]]) ;
				double		d0 = (vNext[0] - vThis[0]).Absolute() ;
				double		d1 = (vNext[1] - vThis[1]).Absolute() ;
				//
				if ( (d0 + d1) > 1.0e-5 )
				{
					// 隣接面ではないので稜線分割
					fAdjoiningFace[i] = false ;
//					fEdge[i] = true ;
					continue ;
				}
				//
				while ( iNextFace[i] != ify1 + x )
				{
					QuadIndexesFromFaceIndex( iFaceVert, iNextFace[i] ) ;
					//
					bool	fShrink = false ;
					if ( fVertEdge[i] )
					{
						fShrink = (GetPointAt(iFaceVert[0]) == GetPointAt(iFaceVert[1]))
								&& (GetPointAt(iFaceVert[2]) == GetPointAt(iFaceVert[3])) ;
					}
					else
					{
						fShrink = (GetPointAt(iFaceVert[0]) == GetPointAt(iFaceVert[2]))
								&& (GetPointAt(iFaceVert[1]) == GetPointAt(iFaceVert[3])) ;
					}
					if ( !fShrink )
					{
						break ;
					}
					SGLPoint	ptFace ;
					PointFromIndex( ptFace, (uint32_t) iNextFace[i] ) ;
					if ( fVertEdge[i] )
					{
						ptFace.x = (int32_t) ((ptFace.x + nNextDelta[i] + m_wPatch) % m_wPatch) ;
					}
					else
					{
						ptFace.y = (int32_t) ((ptFace.y + nNextDelta[i] + m_hPatch) % m_hPatch) ;
					}
					iNextFace[i] = IndexFromPoint( ptFace.x, ptFace.y ) ;
				}
			}
			//
			// 角度で判定
			//
			if ( mparam.nFlags & flagMeshEdgeByAngle )
			{
				const S3DVector	vFaceNormal = GetFaceNormalAt( ify1 + x ) ;
				for ( int i = 0; i < 4; i ++ )
				{
					float32_t	cosAngle =
						vFaceNormal.InnerProduct( GetFaceNormalAt( iNextFace[i] ) ) ;
					if ( (cosAngle < cosEdgeAngle) && fAdjoiningFace[i] )
					{
						// 角度が超えているので稜線分割
						fEdge[i] = true ;
					}
				}
			}
			//
			// 稜線判定
			//
			for ( int i = 0; i < 4; i ++ )
			{
				if ( !fEdge[i] && fAdjoiningFace[i]
					&& (m_edges.FindSorted( Edge( iEdge[i][0], iEdge[i][1] ) ) >= 0) )
				{
					// 稜線に設定されているので稜線分割
					fEdge[i] = true ;
				}
			}
			//
			// 分割稜線の為に頂点を分離する
			//
			for ( int i = 0; i < 4; i ++ )
			{
				if ( !fEdge[i] )
				{
					continue ;
				}
				for ( int j = 0; j < 2; j ++ )
				{
					size_t	iv = iEdge[i][j] ;
					if ( fEdgeVertex[iv] )
					{
						continue ;
					}
					size_t	iSrc = iSrcVertex[iv] ;
					iAliasVertex[iv] = (uint32_t) mbuf.m_bufVertex.GetLength() ;
					fEdgeVertex[iv] = true ;
					fSharedVertex[iv] = false ;
					//
					ESLAssert( mbuf.m_bufVertex.GetLength() == mbuf.m_countVertex ) ;
					const float32_t	sn = (nFace < 0) ? -1.0f : 1.0f ;
					S3DVector	vNormal = GetFaceNormalAt( ify1 + x ) * sn ;
					for ( int k = 0; k < 2; k ++ )
					{
						if ( fEdge[iEdgeByVertex[iv][k]]
							|| !fAdjoiningFace[iEdgeByVertex[iv][k]]
							|| (GetFaceAt( iNextFace[iEdgeByVertex[iv][k]] ) == faceNull) )
						{
							continue ;
						}
						vNormal +=
							GetFaceNormalAt( iNextFace[iEdgeByVertex[iv][k]] ) * sn ;
					}
					mbuf.m_countVertex ++ ;
					mbuf.m_bufVertex.Add( GetPointAt( iSrc ) ) ;
					mbuf.m_bufNormal.Add( vNormal.Normalized() ) ;
					mbuf.m_bufUVMap.Add( GetUVAt( iSrc ) ) ;
					mbuf.m_bufColor.Add( GetColorAt( iSrc ) ) ;
					mbuf.m_bufSrcVertex.Add( iSrc ) ;
					//
					for ( size_t k = 0; k < m_nWeightLayers; k ++ )
					{
						mbuf.m_bufWeight.Add( GetWeightAt( iSrc, k ) ) ;
					}
				}
			}
			//
			// 面出力
			//
			if ( nFace == faceFront )
			{
				pDstIndex[iDst]     = fSharedVertex[0] ? iVertex[0] : iAliasVertex[0] ;
				pDstIndex[iDst + 1] = fSharedVertex[1] ? iVertex[1] : iAliasVertex[1] ;
				pDstIndex[iDst + 2] = fSharedVertex[2] ? iVertex[2] : iAliasVertex[2] ;
				pDstIndex[iDst + 3] = fSharedVertex[3] ? iVertex[3] : iAliasVertex[3] ;
			}
			else
			{
				pDstIndex[iDst]     = fSharedVertex[1] ? iVertex[1] : iAliasVertex[1] ;
				pDstIndex[iDst + 1] = fSharedVertex[0] ? iVertex[0] : iAliasVertex[0] ;
				pDstIndex[iDst + 2] = fSharedVertex[3] ? iVertex[3] : iAliasVertex[3] ;
				pDstIndex[iDst + 3] = fSharedVertex[2] ? iVertex[2] : iAliasVertex[2] ;
			}
			uint8_t	nShifter = GetFaceShifterAt( ify1 + x ) & 0x03 ;
			if ( nShifter != 0 )
			{
				const uint32_t	iTemp[4] =
				{
					pDstIndex[iDst],
					pDstIndex[iDst + 1],
					pDstIndex[iDst + 3],
					pDstIndex[iDst + 2],
				} ;
				pDstIndex[iDst]     = iTemp[nShifter] ;
				pDstIndex[iDst + 1] = iTemp[(nShifter + 1) & 0x03] ;
				pDstIndex[iDst + 2] = iTemp[(nShifter + 3) & 0x03] ;
				pDstIndex[iDst + 3] = iTemp[(nShifter + 2) & 0x03] ;
			}
			iDst += 4 ;
		}
	}
	ESLAssert( iDst <= mbuf.m_bufIndex.GetLength() ) ;
	mbuf.m_bufIndex.FinishArray() ;
	//
	mbuf.m_countIndex = iDst ;
	mbuf.m_bufIndex.SetLength( iDst ) ;
}

// 全てスムージングするメッシュ構築
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::BuildPatchMeshAllSmooth
	( S3DMeshEditor::MeshBuffer& mbuf,
			const S3DMeshEditor::MeshParam& mparam ) const
{
	const size_t	nAreaSize = GetAreaSize() ;
	if ( nAreaSize == 0 )
	{
		mbuf.m_countVertex = 0 ;
		mbuf.m_countIndex = 0 ;
		mbuf.m_nWeightLayers = 0 ;
		return ;
	}
	//
	// 頂点情報の複製
	//
	BuildPatchUsedVertex( mbuf ) ;
	//
	// 四角ポリゴン指標の構築
	//
	ESLAssert( m_hPatch > 0 ) ;
	ESLAssert( m_wPatch > 0 ) ;
	const size_t	hPatch = (m_nFlags & flagVertLoop) ? m_hPatch : m_hPatch - 1 ;
	const size_t	wPatch = (m_nFlags & flagHorzLoop) ? m_wPatch : m_wPatch - 1 ;
	//
	const uint32_t *	pIUsedVertex = m_bufIUsedVertex.GetConstArray() ;
	uint32_t *			pDstIndex = mbuf.m_bufIndex.GetArray( hPatch * wPatch * 4 ) ;
	size_t				iDst = 0 ;
	//
	for ( size_t y = 0; y < hPatch; y ++ )
	{
		size_t	iy0 = y * m_wPatch ;
		size_t	iy1 = iy0 + m_wPatch ;
		if ( y + 1 >= m_hPatch )
		{
			iy1 = 0 ;
		}
		for ( size_t x = 0; x < wPatch; x ++ )
		{
			size_t	x1 = x + 1 ;
			if ( x1 >= m_wPatch )
			{
				x1 = 0 ;
			}
			int8_t	nFace = GetFaceAt( iy0 + x ) ;
			if ( nFace == faceNull )
			{
				continue ;
			}
			ESLAssert( iy0 + x < nAreaSize ) ;
			ESLAssert( iy0 + x1 < nAreaSize ) ;
			ESLAssert( iy1 + x < nAreaSize ) ;
			ESLAssert( iy1 + x1 < nAreaSize ) ;
			if ( nFace == faceFront )
			{
				pDstIndex[iDst]     = pIUsedVertex[iy0 + x] ;
				pDstIndex[iDst + 1] = pIUsedVertex[iy0 + x1] ;
				pDstIndex[iDst + 2] = pIUsedVertex[iy1 + x] ;
				pDstIndex[iDst + 3] = pIUsedVertex[iy1 + x1] ;
			}
			else
			{
				pDstIndex[iDst]     = pIUsedVertex[iy0 + x1] ;
				pDstIndex[iDst + 1] = pIUsedVertex[iy0 + x] ;
				pDstIndex[iDst + 2] = pIUsedVertex[iy1 + x1] ;
				pDstIndex[iDst + 3] = pIUsedVertex[iy1 + x] ;
			}
			uint8_t	nShifter = GetFaceShifterAt( iy0 + x ) & 0x03 ;
			if ( nShifter != 0 )
			{
				const uint32_t	iTemp[4] =
				{
					pDstIndex[iDst],
					pDstIndex[iDst + 1],
					pDstIndex[iDst + 3],
					pDstIndex[iDst + 2],
				} ;
				pDstIndex[iDst]     = iTemp[nShifter] ;
				pDstIndex[iDst + 1] = iTemp[(nShifter + 1) & 0x03] ;
				pDstIndex[iDst + 2] = iTemp[(nShifter + 3) & 0x03] ;
				pDstIndex[iDst + 3] = iTemp[(nShifter + 2) & 0x03] ;
			}
			iDst += 4 ;
		}
	}
	//
	ESLAssert( iDst <= mbuf.m_bufIndex.GetLength() ) ;
	mbuf.m_bufIndex.FinishArray() ;
	//
	mbuf.m_countIndex = iDst ;
	mbuf.m_bufIndex.SetLength( iDst ) ;
}

// 全頂点を分割するメッシュ構築
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::BuildPatchMeshAllDivPoints
	( S3DMeshEditor::MeshBuffer& mbuf,
			const S3DMeshEditor::MeshParam& mparam ) const
{
	mbuf.m_countVertex = 0 ;
	mbuf.m_countIndex = 0 ;
	mbuf.m_nWeightLayers = 0 ;
	//
	if ( GetAreaSize() == 0 )
	{
		return ;
	}
	ESLAssert( m_hPatch > 0 ) ;
	ESLAssert( m_wPatch > 0 ) ;
	const size_t	hPatch = (m_nFlags & flagVertLoop) ? m_hPatch : m_hPatch - 1 ;
	const size_t	wPatch = (m_nFlags & flagHorzLoop) ? m_wPatch : m_wPatch - 1 ;
	const size_t	nAreaSize = hPatch * wPatch ;
	//
	mbuf.m_countVertex = nAreaSize * 4 ;
	mbuf.m_countIndex = nAreaSize * 4 ;
	mbuf.m_nWeightLayers = mbuf.m_bufExAttrIndex.GetLength() ;
	//
	const size_t	nWeightLayers = mbuf.m_nWeightLayers ;
	const size_t *	pExAttrLayers = mbuf.m_bufExAttrIndex.GetConstArray() ;
	S3DVector4 *	pvVertex = mbuf.m_bufVertex.GetArray( mbuf.m_countVertex ) ;
	S3DVector4 *	pvNormal = mbuf.m_bufNormal.GetArray( mbuf.m_countVertex ) ;
	S2DVector *		pvUV = mbuf.m_bufUVMap.GetArray( mbuf.m_countVertex ) ;
	S3DColor *		pColor = mbuf.m_bufColor.GetArray( mbuf.m_countVertex ) ;
	float32_t *		pfpWeight = mbuf.m_bufWeight.GetArray
								( mbuf.m_countVertex * nWeightLayers ) ;
	size_t *		pSrcIndex = mbuf.m_bufSrcVertex.GetArray( mbuf.m_countVertex ) ;
	uint32_t *		pIndex = mbuf.m_bufIndex.GetArray( mbuf.m_countIndex ) ;
	//
	size_t	iDstVertex = 0 ;
	size_t	iDstIndex = 0 ;
	for ( size_t y = 0; y < hPatch; y ++ )
	{
		size_t	iy0 = y * m_wPatch ;
		size_t	iy1 = iy0 + m_wPatch ;
		if ( y + 1 >= m_hPatch )
		{
			iy1 = 0 ;
		}
		for ( size_t x = 0; x < wPatch; x ++ )
		{
			size_t	x1 = x + 1 ;
			if ( x1 >= m_wPatch )
			{
				x1 = 0 ;
			}
			int8_t	nFace = GetFaceAt( iy0 + x ) ;
			if ( nFace == faceNull )
			{
				continue ;
			}
			//
			pvVertex[iDstVertex]     = GetPointAt( iy0 + x ) ;
			pvVertex[iDstVertex + 1] = GetPointAt( iy0 + x1 ) ;
			pvVertex[iDstVertex + 2] = GetPointAt( iy1 + x ) ;
			pvVertex[iDstVertex + 3] = GetPointAt( iy1 + x1 ) ;
			//
			if ( (mparam.nFlags & flagMeshEdgeMethodMask) == flagMeshAllFlat )
			{
				S3DVector	vNormal = GetFaceNormalAt( iy0 + x ) ;
				pvNormal[iDstVertex]     = vNormal ;
				pvNormal[iDstVertex + 1] = vNormal ;
				pvNormal[iDstVertex + 2] = vNormal ;
				pvNormal[iDstVertex + 3] = vNormal ;
			}
			else
			{
				pvNormal[iDstVertex]     = GetNormalAt( iy0 + x ) ;
				pvNormal[iDstVertex + 1] = GetNormalAt( iy0 + x1 ) ;
				pvNormal[iDstVertex + 2] = GetNormalAt( iy1 + x ) ;
				pvNormal[iDstVertex + 3] = GetNormalAt( iy1 + x1 ) ;
			}
			//
			pvUV[iDstVertex]     = GetUVAt( iy0 + x ) ;
			pvUV[iDstVertex + 1] = GetUVAt( iy0 + x1 ) ;
			pvUV[iDstVertex + 2] = GetUVAt( iy1 + x ) ;
			pvUV[iDstVertex + 3] = GetUVAt( iy1 + x1 ) ;
			//
			pColor[iDstVertex]     = GetColorAt( iy0 + x ) ;
			pColor[iDstVertex + 1] = GetColorAt( iy0 + x1 ) ;
			pColor[iDstVertex + 2] = GetColorAt( iy1 + x ) ;
			pColor[iDstVertex + 3] = GetColorAt( iy1 + x1 ) ;
			//
			size_t	iDstWeight0 = iDstVertex * nWeightLayers ;
			size_t	iDstWeight1 = (iDstVertex + 1) * nWeightLayers ;
			size_t	iDstWeight2 = (iDstVertex + 2) * nWeightLayers ;
			size_t	iDstWeight3 = (iDstVertex + 3) * nWeightLayers ;
			for ( size_t j = 0; j < nWeightLayers; j ++ )
			{
				size_t	iLayer = pExAttrLayers[j] ;
				pfpWeight[iDstWeight0 + j] = GetWeightAt( iy0 + x, iLayer ) ;
				pfpWeight[iDstWeight1 + j] = GetWeightAt( iy0 + x1, iLayer ) ;
				pfpWeight[iDstWeight2 + j] = GetWeightAt( iy1 + x, iLayer ) ;
				pfpWeight[iDstWeight3 + j] = GetWeightAt( iy1 + x1, iLayer ) ;
			}
			//
			pSrcIndex[iDstVertex]     = iy0 + x ;
			pSrcIndex[iDstVertex + 1] = iy0 + x1 ;
			pSrcIndex[iDstVertex + 2] = iy1 + x ;
			pSrcIndex[iDstVertex + 3] = iy1 + x1 ;
			//
			if ( nFace == faceFront )
			{
				pIndex[iDstIndex]     = (uint32_t) iDstVertex ;
				pIndex[iDstIndex + 1] = (uint32_t) iDstVertex + 1 ;
				pIndex[iDstIndex + 2] = (uint32_t) iDstVertex + 2 ;
				pIndex[iDstIndex + 3] = (uint32_t) iDstVertex + 3 ;
			}
			else
			{
				pIndex[iDstIndex]     = (uint32_t) iDstVertex + 1 ;
				pIndex[iDstIndex + 1] = (uint32_t) iDstVertex ;
				pIndex[iDstIndex + 2] = (uint32_t) iDstVertex + 3 ;
				pIndex[iDstIndex + 3] = (uint32_t) iDstVertex + 2 ;
				//
				pvNormal[iDstVertex]     = - pvNormal[iDstVertex] ;
				pvNormal[iDstVertex + 1] = - pvNormal[iDstVertex + 1] ;
				pvNormal[iDstVertex + 2] = - pvNormal[iDstVertex + 2] ;
				pvNormal[iDstVertex + 3] = - pvNormal[iDstVertex + 3] ;
			}
			uint8_t	nShifter = GetFaceShifterAt( iy0 + x ) & 0x03 ;
			if ( nShifter != 0 )
			{
				const uint32_t	iTemp[4] =
				{
					pIndex[iDstIndex],
					pIndex[iDstIndex + 1],
					pIndex[iDstIndex + 3],
					pIndex[iDstIndex + 2],
				} ;
				pIndex[iDstIndex]     = iTemp[nShifter] ;
				pIndex[iDstIndex + 1] = iTemp[(nShifter + 1) & 0x03] ;
				pIndex[iDstIndex + 2] = iTemp[(nShifter + 3) & 0x03] ;
				pIndex[iDstIndex + 3] = iTemp[(nShifter + 2) & 0x03] ;
			}
			//
			iDstVertex += 4 ;
			iDstIndex += 4 ;
		}
	}
	ESLAssert( mbuf.m_countVertex >= iDstVertex ) ;
	ESLAssert( mbuf.m_countIndex >= iDstIndex ) ;
	//
	mbuf.m_bufVertex.FinishArray() ;
	mbuf.m_bufNormal.FinishArray() ;
	mbuf.m_bufUVMap.FinishArray() ;
	mbuf.m_bufColor.FinishArray() ;
	mbuf.m_bufWeight.FinishArray() ;
	mbuf.m_bufSrcVertex.FinishArray() ;
	mbuf.m_bufIndex.FinishArray() ;
	//
	mbuf.m_countVertex = iDstVertex ;
	mbuf.m_countIndex = iDstIndex ;
}

// 使用頂点のみを MeshBuffer へ複製する
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::BuildPatchUsedVertex
				( S3DMeshEditor::MeshBuffer& mbuf ) const
{
	mbuf.m_countVertex = m_bufUsedVertex.GetLength() ;
	mbuf.m_nWeightLayers = mbuf.m_bufExAttrIndex.GetLength() ;
	//
	mbuf.m_bufVertex.SetLength( mbuf.m_countVertex ) ;
	mbuf.m_bufNormal.SetLength( mbuf.m_countVertex ) ;
	mbuf.m_bufUVMap.SetLength( mbuf.m_countVertex ) ;
	mbuf.m_bufColor.SetLength( mbuf.m_countVertex ) ;
	mbuf.m_bufWeight.SetLength( mbuf.m_countVertex * mbuf.m_nWeightLayers ) ;
	mbuf.m_bufSrcVertex.SetLength( mbuf.m_countVertex ) ;
	//
	const size_t		nWeightLayers = mbuf.m_nWeightLayers ;
	const size_t *		pExAttrLayers = mbuf.m_bufExAttrIndex.GetConstArray() ;
	const uint32_t *	pUsedVertex = m_bufUsedVertex.GetConstArray() ;
	const int8_t *		pUsedNormalFace = m_bufUsedNormalFace.GetConstArray() ;
	const S3DVector *	pSrcVertex = m_bufVertex.GetConstArray() ;
	const S3DVector *	pSrcNormal = m_bufNormal.GetConstArray() ;
	const S2DVector *	pSrcUV = m_bufUVMap.GetConstArray() ;
	const S3DColor *	pSrcColor = m_bufColor.GetConstArray() ;
	S3DVector4 *		pvVertex = mbuf.m_bufVertex.GetArray( mbuf.m_countVertex ) ;
	S3DVector4 *		pvNormal = mbuf.m_bufNormal.GetArray( mbuf.m_countVertex ) ;
	S2DVector *			pvUV = mbuf.m_bufUVMap.GetArray( mbuf.m_countVertex ) ;
	S3DColor *			pColor = mbuf.m_bufColor.GetArray( mbuf.m_countVertex ) ;
	float32_t *			pfpWeight = mbuf.m_bufWeight.GetArray
										( mbuf.m_countVertex * nWeightLayers ) ;
	size_t *			pSrcIndex = mbuf.m_bufSrcVertex.GetArray( mbuf.m_countVertex ) ;
	//
	for ( size_t i = 0; i < mbuf.m_countVertex; i ++ )
	{
		uint32_t	iVertex = pUsedVertex[i] ;
		pvVertex[i] = pSrcVertex[iVertex] ;
		if ( pUsedNormalFace[i] < 0 )
		{
			pvNormal[i] = - pSrcNormal[iVertex] ;
		}
		else
		{
			pvNormal[i] = pSrcNormal[iVertex] ;
		}
		pvUV[i] = pSrcUV[iVertex] ;
		pColor[i] = pSrcColor[iVertex] ;
		//
		size_t	iWeight = i * nWeightLayers ;
		for ( size_t j = 0; j < nWeightLayers; j ++ )
		{
			pfpWeight[iWeight + j] = GetWeightAt( i, pExAttrLayers[j] ) ;
		}
		pSrcIndex[i] = (size_t) iVertex ;
	}
	//
	mbuf.m_bufVertex.FinishArray() ;
	mbuf.m_bufNormal.FinishArray() ;
	mbuf.m_bufUVMap.FinishArray() ;
	mbuf.m_bufColor.FinishArray() ;
	mbuf.m_bufWeight.FinishArray() ;
	mbuf.m_bufSrcVertex.FinishArray() ;
}

// 全ポリゴンの単純な構築（当たり判定用）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::BuildAllPolygonsSimply( MeshBuffer& mbuf ) const
{
	const size_t	nAreaSize = GetAreaSize() ;
	const size_t	wFace = GetFaceWidth() ;
	const size_t	hFace = GetFaceHeight() ;
	const size_t	nExFaceCount = GetExFaceCount() ;
	//
	mbuf.m_countVertex = GetTotalVertexCount() ;
	mbuf.m_countIndex = ((wFace * hFace * 2) + nExFaceCount) * 3 ;
	mbuf.m_nWeightLayers = 0 ;
	//
	const S3DVector *	pSrcVertex = m_bufVertex.GetConstArray() ;
	const S3DVector *	pSrcNormal = m_bufNormal.GetConstArray() ;
	const S2DVector *	pSrcUV = m_bufUVMap.GetConstArray() ;
	const S3DColor *	pSrcColor = m_bufColor.GetConstArray() ;
	S3DVector4 *		pvVertex = mbuf.m_bufVertex.GetArray( mbuf.m_countVertex ) ;
	S3DVector4 *		pvNormal = mbuf.m_bufNormal.GetArray( mbuf.m_countVertex ) ;
	S2DVector *			pvUV = mbuf.m_bufUVMap.GetArray( mbuf.m_countVertex ) ;
	S3DColor *			pColor = mbuf.m_bufColor.GetArray( mbuf.m_countVertex ) ;
	uint32_t *			pIndex = mbuf.m_bufIndex.GetArray( mbuf.m_countIndex ) ;
	//
	for ( size_t i = 0; i < mbuf.m_countVertex; i ++ )
	{
		pvVertex[i] = pSrcVertex[i] ;
		pvNormal[i] = pSrcNormal[i] ;
		pvUV[i] = pSrcUV[i] ;
		pColor[i] = pSrcColor[i] ;
	}
	//
	size_t	iDst = 0 ;
	for ( size_t y = 0; y < hFace; y ++ )
	{
		for ( size_t x = 0; x < wFace; x ++ )
		{
			size_t	iQuad[4] ;
			QuadIndexesFromFacePoint( iQuad, x, y ) ;
			//
			pIndex[iDst]     = (uint32_t) iQuad[0] ;
			pIndex[iDst + 1] = (uint32_t) iQuad[3] ;
			pIndex[iDst + 2] = (uint32_t) iQuad[1] ;
			pIndex[iDst + 3] = (uint32_t) iQuad[0] ;
			pIndex[iDst + 4] = (uint32_t) iQuad[2] ;
			pIndex[iDst + 5] = (uint32_t) iQuad[3] ;
			iDst += 6 ;
		}
	}
	const uint32_t *	pExFaces = GetExFaceTriangleIndexes( 0, nExFaceCount ) ;
	for ( size_t i = 0, j = 0; i < nExFaceCount; i ++, j += 3 )
	{
		pIndex[iDst]     = pExFaces[j] ;
		pIndex[iDst + 1] = pExFaces[j + 1] ;
		pIndex[iDst + 2] = pExFaces[j + 2] ;
		iDst += 3 ;
	}
	ESLAssert( iDst == mbuf.m_countIndex ) ;
	//
	mbuf.m_bufVertex.FinishArray() ;
	mbuf.m_bufNormal.FinishArray() ;
	mbuf.m_bufUVMap.FinishArray() ;
	mbuf.m_bufColor.FinishArray() ;
	mbuf.m_bufIndex.FinishArray() ;
}

// シリアライズ (UpdatePatchMesh を実行してから)
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::SerializeToMesh
	( S3DMeshBufferPropertySerializer::MeshBuffer& mesh ) const
{
	//
	// 基本情報設定
	//
	const size_t	nVertexCount = GetTotalVertexCount() ;
	const size_t	nAreaSize = GetAreaSize() ;
	const size_t	nExFaceCount = GetExFaceCount() ;
	//
	mesh.strName = GetName() ;
	mesh.typeMesh = primitiveTriangle ;
	mesh.countPrimitive = (uint32_t) (nAreaSize * 2 + nExFaceCount) ;
	mesh.countVertex = (uint32_t) nVertexCount ;
	mesh.countIndex = mesh.countPrimitive * 3 ;
	//
	// パッチ情報ヘッダ
	//
	SerializedPatchHeader	hdr ;
	eslFillMemory( &hdr, 0, sizeof(hdr) ) ;
	hdr.nFlags = m_nFlags ;
	hdr.wPatch = (uint32_t) m_wPatch ;
	hdr.hPatch = (uint32_t) m_hPatch ;
	hdr.nWeightLayers = (uint32_t) m_nWeightLayers ;
	hdr.nExFaces = (uint32_t) nExFaceCount ;
	hdr.iMaterial = (uint32_t) m_iMaterial ;
	//
	SArray<uint8_t> *	pBinPatchHeader = new SArray<uint8_t> ;
	pBinPatchHeader->AddArray( (const uint8_t*) &hdr, sizeof(hdr) ) ;
	mesh.m_ssaExBuf.SetAs( L"patch_header", pBinPatchHeader ) ;
	//
	// 頂点バッファ複製
	//
	mesh.bufVertex.SetLength( nVertexCount ) ;
	mesh.bufNormal.SetLength( nVertexCount ) ;
	mesh.bufUVMap.SetLength( nVertexCount ) ;
	mesh.bufColor.SetLength( nVertexCount ) ;
	//
	const S3DVector *	pvSrcVertex =
							GetConstPointArray( 0, nVertexCount ) ;
	S3DVector4 *		pvBufVertex = mesh.bufVertex.GetArray() ;
	for ( size_t i = 0; i < nVertexCount; i ++ )
	{
		pvBufVertex[i] = pvSrcVertex[i] ;
	}
	//
	const S3DVector *	pvSrcNormal =
							GetConstNormalArray( 0, nVertexCount ) ;
	S3DVector4 *		pvBufNormal = mesh.bufNormal.GetArray() ;
	for ( size_t i = 0; i < nVertexCount; i ++ )
	{
		pvBufNormal[i] = pvSrcNormal[i] ;
	}
	//
	const S2DVector *	pvSrcUV =
							GetConstUVArray( 0, nVertexCount ) ;
	S2DVector *			pvBufUV= mesh.bufUVMap.GetArray() ;
	for ( size_t i = 0; i < nVertexCount; i ++ )
	{
		pvBufUV[i] = pvSrcUV[i] ;
	}
	//
	const S3DColor *	pSrcColor =
							GetConstColorArray( 0, nVertexCount ) ;
	S3DColor *			pBufColor = mesh.bufColor.GetArray() ;
	for ( size_t i = 0; i < nVertexCount; i ++ )
	{
		pBufColor[i] = pSrcColor[i] ;
	}
	//
	mesh.bufVertex.FinishArray() ;
	mesh.bufNormal.FinishArray() ;
	mesh.bufUVMap.FinishArray() ;
	mesh.bufColor.FinishArray() ;
	//
	// 三角ポリゴン
	//
	uint32_t *	pBufIndex = mesh.bufIndex.GetArray( (size_t) mesh.countIndex ) ;
	size_t		iDstIndex = 0 ;
	//
	const uint32_t *
			pExFaces = GetExFaceTriangleIndexes( 0, nExFaceCount ) ;
	eslCopyMemory( pBufIndex, pExFaces, nExFaceCount * 3 * sizeof(uint32_t) ) ;
	iDstIndex = nExFaceCount * 3 ;
	//
	const int8_t *	pSrcFace = GetConstFaceArray( 0, nAreaSize ) ;
	const size_t	nFaceWidth = GetFaceWidth() ;
	const size_t	nFaceHeight = GetFaceHeight() ;
	for ( size_t y = 0; y < nFaceHeight; y ++ )
	{
		size_t	iy = y * GetWidth() ;
		for ( size_t x = 0; x < nFaceWidth; x ++ )
		{
			if ( pSrcFace[iy + x] != Patch::faceNull )
			{
				continue ;
			}
			size_t	iQuad[4] ;
			QuadIndexesFromFacePoint( iQuad, x, y ) ;
			if ( pSrcFace[iy + x] != Patch::faceFront )
			{
				pBufIndex[iDstIndex]     = (uint32_t) iQuad[0] ;
				pBufIndex[iDstIndex + 1] = (uint32_t) iQuad[2] ;
				pBufIndex[iDstIndex + 2] = (uint32_t) iQuad[3] ;
				pBufIndex[iDstIndex + 3] = (uint32_t) iQuad[0] ;
				pBufIndex[iDstIndex + 4] = (uint32_t) iQuad[3] ;
				pBufIndex[iDstIndex + 5] = (uint32_t) iQuad[1] ;
			}
			else
			{
				pBufIndex[iDstIndex]     = (uint32_t) iQuad[0] ;
				pBufIndex[iDstIndex + 1] = (uint32_t) iQuad[1] ;
				pBufIndex[iDstIndex + 2] = (uint32_t) iQuad[3] ;
				pBufIndex[iDstIndex + 3] = (uint32_t) iQuad[0] ;
				pBufIndex[iDstIndex + 4] = (uint32_t) iQuad[3] ;
				pBufIndex[iDstIndex + 5] = (uint32_t) iQuad[2] ;
			}
			iDstIndex += 6 ;
		}
	}
	mesh.bufIndex.FinishArray() ;
	ESLAssert( iDstIndex <= mesh.bufIndex.GetLength() ) ;
	mesh.bufIndex.SetLength( iDstIndex ) ;
	//
	mesh.countIndex = (uint32_t) iDstIndex ;
	mesh.countPrimitive = mesh.countIndex / 3 ;
	//
	// ウェイトマップ
	//
	const size_t		nWeightLayerCount = GetWeightLayerCount() ;
	const size_t		nWeightMapBytes =
							nVertexCount * nWeightLayerCount * sizeof(float32_t) ;
	SArray<uint8_t> *	pBinWeight = new SArray<uint8_t> ;
	eslCopyMemory
		( pBinWeight->GetArray( nWeightMapBytes ),
			m_bufWeight.GetConstArray(), nWeightMapBytes ) ;
	pBinWeight->FinishArray() ;
	//
	mesh.m_ssaExBuf.SetAs( L"weight_maps", pBinWeight ) ;
	//
	// 面
	//
	SArray<uint8_t> *	pBinFace = new SArray<uint8_t> ;
	eslCopyMemory
		( pBinFace->GetArray( nAreaSize + nExFaceCount ),
			m_bufFace.GetConstArray(), nAreaSize + nExFaceCount ) ;
	pBinFace->FinishArray() ;
	//
	mesh.m_ssaExBuf.SetAs( L"face_maps", pBinFace ) ;
	//
	// 三角化シフタ
	//
	SArray<uint8_t> *	pBinShifter = new SArray<uint8_t> ;
	eslCopyMemory
		( pBinShifter->GetArray( nAreaSize + nExFaceCount ),
			m_bufShifter.GetConstArray(), nAreaSize + nExFaceCount ) ;
	pBinShifter->FinishArray() ;
	//
	mesh.m_ssaExBuf.SetAs( L"face_shifter", pBinShifter ) ;
	//
	// 分割稜線
	//
	SArray<uint8_t> *	pBinEdges = new SArray<uint8_t> ;
	m_edges.Serialize( *pBinEdges ) ;
	//
	mesh.m_ssaExBuf.SetAs( L"edge_set", pBinEdges ) ;
	//
	// 縮退頂点（マップ）
	//
	SArray<uint8_t> *	pBinDegMap = new SArray<uint8_t> ;
	eslCopyMemory
		( pBinDegMap->GetArray
				( m_bufDegenerate.GetLength() * sizeof(uint32_t) ),
			m_bufDegenerate.GetConstArray(),
			m_bufDegenerate.GetLength() * sizeof(uint32_t) ) ;
	pBinDegMap->FinishArray() ;
	//
	mesh.m_ssaExBuf.SetAs( L"degenerate_map", pBinDegMap ) ;
	//
	// 縮退頂点情報
	//
	SArray<uint8_t> *	pBinDegenerates = new SArray<uint8_t> ;
	m_degenerates.Serialize( *pBinDegenerates ) ;
	//
	mesh.m_ssaExBuf.SetAs( L"degenerate_set", pBinDegenerates ) ;
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::DeserializeFromMesh
	( const S3DMeshBufferPropertySerializer::MeshBuffer& mesh )
{
	if ( mesh.typeMesh != primitiveTriangle )
	{
		return ;
	}
	SetName( mesh.strName ) ;
	//
	// パッチ情報ヘッダ
	//
	SArray<uint8_t> *	pBinPatchHeader = mesh.m_ssaExBuf.GetAs( L"patch_header" ) ;
	if ( (pBinPatchHeader == nullptr)
		|| (pBinPatchHeader->GetLength() < sizeof(SerializedPatchHeader)) )
	{
		return ;
	}
	const SerializedPatchHeader&
		hdr = *((const SerializedPatchHeader*) (pBinPatchHeader->GetConstArray())) ;
	//
	m_nFlags = hdr.nFlags ;
	m_iMaterial = hdr.iMaterial ;
	//
	CreatePatch( (size_t) hdr.wPatch, (size_t) hdr.hPatch ) ;
	//
	// 頂点バッファ
	//
	const size_t	nAreaSize = GetAreaSize() ;
	if ( mesh.countVertex > nAreaSize )
	{
		InsertPoints( nAreaSize, mesh.countVertex - nAreaSize ) ;
	}
	InsertWeightLayer( 0, (size_t) hdr.nWeightLayers ) ;
	//
	ESLAssert( mesh.bufVertex.GetLength() >= mesh.countVertex ) ;
	S3DVector *			pvDstVertex = m_bufVertex.GetArray() ;
	const S3DVector4 *	pvBufVertex = mesh.bufVertex.GetConstArray() ;
	for ( size_t i = 0; i < mesh.countVertex; i ++ )
	{
		pvDstVertex[i] = pvBufVertex[i] ;
	}
	//
	ESLAssert( mesh.bufNormal.GetLength() >= mesh.countVertex ) ;
	S3DVector *			pvDstNormal = m_bufNormal.GetArray() ;
	const S3DVector4 *	pvBufNormal = mesh.bufNormal.GetConstArray() ;
	for ( size_t i = 0; i < mesh.countVertex; i ++ )
	{
		pvDstNormal[i] = pvBufNormal[i] ;
	}
	//
	ESLAssert( mesh.bufUVMap.GetLength() >= mesh.countVertex ) ;
	S2DVector *			pvDstUV = m_bufUVMap.GetArray() ;
	const S2DVector *	pvBufUV = mesh.bufUVMap.GetConstArray() ;
	for ( size_t i = 0; i < mesh.countVertex; i ++ )
	{
		pvDstUV[i] = pvBufUV[i] ;
	}
	//
	ESLAssert( mesh.bufColor.GetLength() >= mesh.countVertex ) ;
	S3DColor *			pDstColor = m_bufColor.GetArray() ;
	const S3DColor *	pBufColor = mesh.bufColor.GetConstArray() ;
	for ( size_t i = 0; i < mesh.countVertex; i ++ )
	{
		pDstColor[i] = pBufColor[i] ;
	}
	//
	ESLAssert( mesh.bufIndex.GetLength() >= mesh.countIndex ) ;
	InsertFaces( nAreaSize, hdr.nExFaces ) ;
	m_bufTriangles.AddArray
		( mesh.bufIndex.GetConstArray(), GetExFaceCount() * 3 ) ;
	//
	// ウェイトマップ
	//
	SArray<uint8_t> *	pBinWeight = mesh.m_ssaExBuf.GetAs( L"weight_maps" ) ;
	if ( pBinWeight != nullptr )
	{
		const size_t	nBytes =
			(size_t) esl_min( (int) pBinWeight->GetLength(),
							(int) m_bufWeight.GetLength() * sizeof(float32_t) ) ;
		eslCopyMemory
			( m_bufWeight.GetArray(), pBinWeight->GetConstArray(), nBytes ) ;
		m_bufWeight.FinishArray() ;
	}
	//
	// 面
	//
	SArray<uint8_t> *	pBinFace = mesh.m_ssaExBuf.GetAs( L"face_maps" ) ;
	if ( pBinFace != nullptr )
	{
		const size_t	nBytes =
			(size_t) esl_min( (int) pBinFace->GetLength(),
							(int) m_bufFace.GetLength() * sizeof(int8_t) ) ;
		eslCopyMemory
			( m_bufFace.GetArray(), pBinFace->GetConstArray(), nBytes ) ;
		m_bufFace.FinishArray() ;
	}
	//
	// 三角化シフタ
	//
	SArray<uint8_t> *	pBinShifter = mesh.m_ssaExBuf.GetAs( L"face_shifter" ) ;
	if ( pBinShifter != nullptr )
	{
		const size_t	nBytes =
			(size_t) esl_min( (int) pBinShifter->GetLength(),
							(int) m_bufShifter.GetLength() * sizeof(int8_t) ) ;
		eslCopyMemory
			( m_bufShifter.GetArray(), pBinShifter->GetConstArray(), nBytes ) ;
		m_bufShifter.FinishArray() ;
	}
	//
	// 分割稜線
	//
	SArray<uint8_t> *	pBinEdges = mesh.m_ssaExBuf.GetAs( L"edge_set" ) ;
	if ( pBinEdges != nullptr )
	{
		m_edges.Deserialize( *pBinEdges ) ;
	}
	//
	// 縮退頂点（マップ）
	//
	SArray<uint8_t> *	pBinDegMap = mesh.m_ssaExBuf.GetAs( L"degenerate_map" ) ;
	if ( pBinDegMap != nullptr )
	{
		const size_t	nBytes =
			(size_t) esl_min( (int) pBinDegMap->GetLength(),
							(int) m_bufDegenerate.GetLength() * sizeof(uint32_t) ) ;
		eslCopyMemory
			( m_bufDegenerate.GetArray(),
					pBinDegMap->GetConstArray(), nBytes ) ;
		m_bufDegenerate.FinishArray() ;
	}
	//
	// 縮退頂点情報
	//
	SArray<uint8_t> *	pBinDegenerates =
							mesh.m_ssaExBuf.GetAs( L"degenerate_set" ) ;
	if ( pBinDegenerates != nullptr )
	{
		m_degenerates.Deserialize( *pBinDegenerates ) ;
	}
	//
	SetUpdateVertexFlag() ;
}

// 通常のメッシュから変換
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::Patch::ConvertFromMeshBuffer
	( const S3DMeshBufferPropertySerializer::MeshBuffer& mesh )
{
	if ( mesh.typeMesh != primitiveTriangle )
	{
		return ;
	}
	SetName( mesh.strName ) ;
	//
	// 頂点バッファ
	//
	InsertPoints( 0, mesh.countVertex ) ;
	InsertFaces( 0, mesh.countIndex / 3 ) ;
	//
	ESLAssert( mesh.bufVertex.GetLength() >= mesh.countVertex ) ;
	ESLAssert( m_bufVertex.GetLength() >= mesh.countVertex ) ;
	S3DVector *			pvDstVertex = m_bufVertex.GetArray() ;
	const S3DVector4 *	pvBufVertex = mesh.bufVertex.GetConstArray() ;
	for ( size_t i = 0; i < mesh.countVertex; i ++ )
	{
		pvDstVertex[i] = pvBufVertex[i] ;
	}
	//
	ESLAssert( mesh.bufNormal.GetLength() >= mesh.countVertex ) ;
	if ( mesh.bufNormal.GetLength() >= mesh.countVertex )
	{
		ESLAssert( m_bufNormal.GetLength() >= mesh.countVertex ) ;
		S3DVector *			pvDstNormal = m_bufNormal.GetArray() ;
		const S3DVector4 *	pvBufNormal = mesh.bufNormal.GetConstArray() ;
		for ( size_t i = 0; i < mesh.countVertex; i ++ )
		{
			pvDstNormal[i] = pvBufNormal[i].Normalized() ;
		}
	}
	//
	if ( mesh.bufUVMap.GetLength() >= mesh.countVertex )
	{
		ESLAssert( m_bufUVMap.GetLength() >= mesh.countVertex ) ;
		S2DVector *			pvDstUV = m_bufUVMap.GetArray() ;
		const S2DVector *	pvBufUV = mesh.bufUVMap.GetConstArray() ;
		for ( size_t i = 0; i < mesh.countVertex; i ++ )
		{
			pvDstUV[i] = pvBufUV[i] ;
		}
	}
	//
	if ( mesh.bufColor.GetLength() >= mesh.countVertex )
	{
		ESLAssert( m_bufColor.GetLength() >= mesh.countVertex ) ;
		S3DColor *			pDstColor = m_bufColor.GetArray() ;
		const S3DColor *	pBufColor = mesh.bufColor.GetConstArray() ;
		for ( size_t i = 0; i < mesh.countVertex; i ++ )
		{
			pDstColor[i] = pBufColor[i] ;
		}
	}
	else
	{
		ESLAssert( m_bufColor.GetLength() >= mesh.countVertex ) ;
		S3DColor	clrDummy( 0xFFFFFFFF, 0 ) ;
		S3DColor *	pDstColor = m_bufColor.GetArray() ;
		for ( size_t i = 0; i < mesh.countVertex; i ++ )
		{
			pDstColor[i] = clrDummy ;
		}
	}
	//
	ESLAssert( mesh.bufIndex.GetLength() >= mesh.countIndex ) ;
	m_bufTriangles.AddArray
		( mesh.bufIndex.GetConstArray(), GetExFaceCount() * 3 ) ;
	//
	// 頂点座標と法線が一致する頂点を縮退設定する
	//
	const S3DVector *	pvNormal = m_bufNormal.GetConstArray() ;
	SArray<uint32_t>	aDegenerate ;
	for ( size_t i = 0; i < mesh.countVertex; i ++ )
	{
		if ( GetDegenerateNumberAt( i ) != 0 )
		{
			continue ;
		}
		for ( size_t j = i + 1; j < mesh.countVertex; j ++ )
		{
			if ( ((pvDstVertex[i] - pvDstVertex[j]).Absolute() < 0.00001f)
				&& (pvNormal[i].InnerProduct(pvNormal[j]) > 0.999f) )
			{
				aDegenerate.Add( (uint32_t) j ) ;
			}
		}
		if ( aDegenerate.GetLength() >= 1 )
		{
			aDegenerate.InsertAt( 0, (uint32_t) i ) ;
			SetDegenerate
				( 0, aDegenerate.GetConstArray(), aDegenerate.GetLength() ) ;
			aDegenerate.RemoveAll() ;
		}
	}
	//
	SetUpdateVertexFlag() ;
}

void S3DMeshEditor::Patch::ConvertFromVertexBuffer
	( const S3DVertexBufferInterface& vbo )
{
	S3DVertexBufferInterface::MeshInfo	infMesh ;
	const size_t	nMeshCount = vbo.GetMeshCount() ;
	size_t			nTotalVertex = 0 ;
	size_t			nTotalIndex = 0 ;
	eslFillMemory( &infMesh, 0, sizeof(infMesh) ) ;
	for ( size_t iMesh = 0; iMesh < nMeshCount; iMesh ++ )
	{
		vbo.GetMeshInfoAt( infMesh, iMesh, 0 ) ;
		nTotalVertex += infMesh.countVertex ;
		if ( infMesh.typeMesh == primitiveTriangle )
		{
			nTotalIndex += infMesh.countPrimitive * 3 ;
		}
	}
	S3DMeshBufferPropertySerializer::MeshBuffer	meshBuf ;
	meshBuf.typeMesh = primitiveTriangle ;
	meshBuf.countPrimitive = (uint32_t) nTotalIndex / 3 ;
	meshBuf.countVertex = (uint32_t) nTotalVertex ;
	meshBuf.countIndex = (uint32_t) nTotalIndex ;
	//
	S3DVector4 *	pvVertex = meshBuf.bufVertex.GetArray( nTotalVertex ) ;
	S3DVector4 *	pvNormal = meshBuf.bufNormal.GetArray( nTotalVertex ) ;
	S2DVector *		pvUVMap = meshBuf.bufUVMap.GetArray( nTotalVertex ) ;
	S3DColor *		pColor = meshBuf.bufColor.GetArray( nTotalVertex ) ;
	uint32_t *		pIndex = meshBuf.bufIndex.GetArray( nTotalIndex ) ;
	size_t			iNextVertex = 0 ;
	size_t			iNextIndex = 0 ;
	for ( size_t iMesh = 0; iMesh < nMeshCount; iMesh ++ )
	{
		vbo.GetMeshInfoAt( infMesh, iMesh, 0 ) ;
		//
		infMesh.pvVertex = pvVertex + iNextVertex ;
		infMesh.pvNormal = pvNormal + iNextVertex ;
		infMesh.pvUVMap = pvUVMap + iNextVertex ;
		infMesh.pColor = pColor + iNextVertex ;
		infMesh.pIndexedList = pIndex + iNextIndex ;
		vbo.GetMeshInfoAt( infMesh, iMesh, infMesh.countVertex ) ;
		//
		if ( infMesh.typeMesh == primitiveTriangle )
		{
			const size_t	nIndexCount = infMesh.countPrimitive * 3 ;
			for ( size_t i = 0; i < nIndexCount; i ++ )
			{
				infMesh.pIndexedList[i] += (uint32_t) iNextVertex ;
			}
			iNextIndex += nIndexCount ;
		}
		iNextVertex += infMesh.countVertex ;
	}
	ConvertFromMeshBuffer( meshBuf ) ;
}



//////////////////////////////////////////////////////////////////////////////
// メッシュ編集クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DMeshEditor, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditor::S3DMeshEditor( void )
	: m_flagUpdateVertex( false ), m_flagUpdateSerialize( true )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditor::~S3DMeshEditor( void )
{
}

// すべて削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::ClearAllMeshs( void )
{
	m_patchs.RemoveAll() ;
	m_seams.RemoveAll() ;
	m_aWeightLayerIDs.RemoveAll() ;
	m_aWeightLayerInfos.RemoveAll() ;
	m_flagUpdateVertex = false ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::DuplicateMesh( const S3DMeshEditor& mesh )
{
	ClearAllMeshs() ;
	//
	m_patchs.SetLimit( mesh.m_patchs.GetLength() ) ;
	m_seams = mesh.m_seams ;
	m_aWeightLayerIDs = mesh.m_aWeightLayerIDs ;
	m_aWeightLayerInfos = mesh.m_aWeightLayerInfos ;
	//
	for ( size_t i = 0; i < mesh.m_patchs.GetLength(); i ++ )
	{
		Patch *	pSrcPatch = mesh.m_patchs.GetAt( i ) ;
		ESLAssert( pSrcPatch != nullptr ) ;
		Patch *	pPatch = new Patch( &m_seams ) ;
		pPatch->CopyFrom( *pSrcPatch ) ;
		m_patchs.Add( pPatch ) ;
		//
		m_seams.RepointerPatch( pSrcPatch, pPatch ) ;
	}
	//
	m_flagUpdateVertex = true ;
}

// 選択点の縮退フラグ取得
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DMeshEditor::GetDegeneratedPointFlag
			( const S3DMeshEditor::PatchPointSet& pps ) const
{
	uint32_t	nFlags = 0 ;
	for ( size_t i = 0; i < pps.GetLength(); i ++ )
	{
		const PatchPoint&	pp = pps.At(i) ;
		ssize_t	iSeam = m_seams.FindPoint( pp ) ;
		if ( iSeam >= 0 )
		{
			SewPatchPoints *	pspp = m_seams.GetAt( (size_t) iSeam ) ;
			ESLAssert( pspp != nullptr ) ;
			if ( pspp != nullptr )
			{
				nFlags |= pspp->m_nFlags ;
			}
		}
		else
		{
			DegenerateEntry	de ;
			if ( pp.pPatch->GetDegenerateAt( de, pp.iVertex ) != nullptr )
			{
				nFlags |= de.nFlags ;
			}
		}
	}
	return	nFlags ;
}

// 選択点の縮退フラグ変更
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::SetDegeneratedPointFlag
		( const S3DMeshEditor::PatchPointSet& pps, uint32_t nFlags )
{
	for ( size_t i = 0; i < pps.GetLength(); i ++ )
	{
		const PatchPoint&	pp = pps.At(i) ;
		ssize_t	iSeam = m_seams.FindPoint( pp ) ;
		if ( iSeam >= 0 )
		{
			SewPatchPoints *	pspp = m_seams.GetAt( (size_t) iSeam ) ;
			ESLAssert( pspp != nullptr ) ;
			if ( pspp != nullptr )
			{
				pspp->m_nFlags = nFlags ;
			}
		}
		pp.pPatch->ChangeDegenerateFlagAt( pp.iVertex, nFlags ) ;
	}
	SetUpdateVertexFlag() ;
}

// 縮退頂点取得
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditor::GetDegeneratedPoints
	( S3DMeshEditor::PatchPointSet& pps, const S3DMeshEditor::PatchPoint& pp ) const
{
	ssize_t	iSeam = m_seams.FindPoint( pp ) ;
	if ( iSeam < 0 )
	{
		return	pp.pPatch->GetDegeneratedPoints( pps, pp.iVertex ) ;
	}
	else
	{
		SewPatchPoints *	pspp = m_seams.GetAt( (size_t) iSeam ) ;
		ESLAssert( pspp != nullptr ) ;
		if ( pspp != nullptr )
		{
			for ( size_t i = 0; i < pspp->GetLength(); i ++ )
			{
				PatchPoint *	ppp = pspp->GetAt( i ) ;
				ESLAssert( ppp != nullptr ) ;
				if ( !ppp->pPatch->GetDegeneratedPoints( pps, ppp->iVertex ) )
				{
					pps.QuickAdd( *ppp ) ;
				}
			}
		}
		return	true ;
	}
}

// 縮退頂点とエントリの整合性検証
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditor::VerifyDegenerate( void ) const
{
	for ( size_t i = 0; i < m_patchs.GetLength(); i ++ )
	{
		Patch *	pPatch = m_patchs.GetAt( i ) ;
		ESLAssert( pPatch != nullptr ) ;
		//
		bool	flagValid = pPatch->VerifyDegenerate() ;
		if ( !flagValid )
		{
			return	false ;
		}
	}
	return	true ;
}

// 頂点縮退処理（頂点座標操作は無し）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::ShrinkPoints
	( const S3DMeshEditor::PatchPointSet& pps, uint32_t nFlags )
{
	PatchPointSet	ppsn ;
	for ( size_t i = 0; i < pps.GetLength(); i ++ )
	{
		if ( !GetDegeneratedPoints( ppsn, pps.At(i) ) )
		{
			ppsn.QuickAdd( pps.At(i) ) ;
		}
	}
	ppsn.SortArray() ;
	ppsn.NormalizeSorted() ;
	//
	for ( size_t i = 0; i < pps.GetLength(); i ++ )
	{
		UntiShrinkPoints( pps.At(i) ) ;
	}
	//
	SArray<PatchPoint>	aPoints ;
	Patch *				pPatch = nullptr ;
	SArray<uint32_t>	aIndexes ;
	for ( size_t i = 0; i < ppsn.GetLength(); i ++ )
	{
		PatchPoint	pp = ppsn.At(i) ;
		if ( pp.pPatch != pPatch )
		{
			if ( (pPatch != nullptr) && (aIndexes.GetLength() > 1) )
			{
				pPatch->SetDegenerate
					( nFlags, aIndexes.GetConstArray(), aIndexes.GetLength() ) ;
			}
			pPatch = pp.pPatch ;
			aIndexes.RemoveAll() ;
			//
			pPatch->SetUpdateVertexFlag() ;
			//
			aPoints.Add( pp ) ;
		}
		aIndexes.Add( (uint32_t) pp.iVertex ) ;
	}
	if ( (pPatch != nullptr) && (aIndexes.GetLength() > 1) )
	{
		pPatch->SetDegenerate
			( nFlags, aIndexes.GetConstArray(), aIndexes.GetLength() ) ;
	}
	if ( aPoints.GetLength() > 1 )
	{
		m_seams.AddSewPatchPoints
			( nFlags, aPoints.GetConstArray(), aPoints.GetLength() ) ;
	}
	SetUpdateVertexFlag() ;
}

// 頂点座標を一点に収束
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::BundlePoints( const S3DMeshEditor::PatchPointSet& pps )
{
	if ( pps.GetLength() > 0 )
	{
		const SelectPoint&	spLast = pps.LastAt() ;
		S3DVector	vLastPos = spLast.pPatch->GetPointAt( spLast.iVertex ) ;
		S3DVector	vCenter( 0, 0, 0 ) ;
		int			nCount = 0 ;
		for ( size_t i = 0; i < pps.GetLength(); i ++ )
		{
			const SelectPoint&	sp = pps.At(i) ;
			S3DVector	vPos = sp.pPatch->GetPointAt( sp.iVertex ) ;
			if ( vPos != vLastPos )
			{
				vCenter += vPos ;
				vLastPos = vPos ;
				nCount ++ ;
			}
		}
		if ( nCount >= 1 )
		{
			vCenter /= (float32_t) nCount ;
		}
		else
		{
			vCenter = vLastPos ;
		}
		for ( size_t i = 0; i < pps.GetLength(); i ++ )
		{
			const SelectPoint&	sp = pps.At(i) ;
			sp.pPatch->SetPointAt( sp.iVertex, vCenter ) ;
		}
	}
}

// 頂点縮退解除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::UntiShrinkPoints( const S3DMeshEditor::PatchPoint& pp )
{
	pp.pPatch->ReleaseDegenerateAt( pp.iVertex ) ;
	pp.pPatch->SetUpdateVertexFlag() ;
	//
	ssize_t	iSeam = m_seams.FindPoint( pp ) ;
	if ( iSeam >= 0 )
	{
		m_seams.RemoveAt( (size_t) iSeam ) ;
	}
	SetUpdateVertexFlag() ;
}

// 縮退頂点の座標を収束させる
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::ShrinkDegeneratedPoints( void )
{
	for ( size_t iPatch = 0; iPatch < GetPatchCount(); iPatch ++ )
	{
		Patch *	pPatch = GetPatchAt( iPatch ) ;
		ESLAssert( pPatch != nullptr ) ;
		pPatch->ShrinkDegeneratedPoints() ;
	}
	for ( size_t i = 0; i < m_seams.GetLength(); i ++ )
	{
		SewPatchPoints *	pspp = m_seams.GetAt( i ) ;
		ESLAssert( pspp != nullptr ) ;
		//
		S3DVector			vPos( 0, 0, 0 ) ;
		const size_t		nCount = pspp->GetLength() ;
		const PatchPoint *	ppp = pspp->GetConstArray() ;
		if ( nCount == 0 )
		{
			continue ;
		}
		for ( size_t j = 0; j < nCount; j ++ )
		{
			vPos += ppp[j].pPatch->GetPointAt( ppp[j].iVertex ) ;
		}
		vPos *= 1.0f / nCount ;
		//
		for ( size_t j = 0; j < nCount; j ++ )
		{
			ppp[j].pPatch->SetPointAt( ppp[j].iVertex, vPos ) ;
		}
	}
	SetUpdateVertexFlag() ;
}

// 頂点更新フラグ設定
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::SetUpdateVertexFlag( void )
{
	m_flagUpdateVertex = true ;
	m_flagUpdateSerialize = true ;
}

// 頂点更新フラグ取得
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditor::GetUpdateVertexFlag( void ) const
{
	return	m_flagUpdateVertex ;
}

// 頂点更新フラグを検証
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::VerifyUpdateFlagByPatchUpdate( void )
{
	if ( m_flagUpdateVertex )
	{
		return ;
	}
	for ( size_t i = 0; i < m_patchs.GetLength(); i ++ )
	{
		Patch *	pPatch = m_patchs.GetAt( i ) ;
		ESLAssert( pPatch != nullptr ) ;
		ESLAssert( !pPatch->GetUpdateVertexFlag() ) ;
		ESLAssert( !pPatch->GetUpdateFaceFlag() ) ;
		if ( pPatch->GetUpdateVertexFlag() || pPatch->GetUpdateFaceFlag() )
		{
			m_flagUpdateVertex = true ;
			break ;
		}
	}
}

// 更新パッチを再構築する
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::UpdateAllPatchs( const S3DMeshEditor::MeshParam& param )
{
	if ( m_flagUpdateVertex )
	{
		for ( size_t i = 0; i < m_patchs.GetLength(); i ++ )
		{
			Patch *	pPatch = m_patchs.GetAt( i ) ;
			ESLAssert( pPatch != nullptr ) ;
			pPatch->UpdatePatchMesh() ;
		}
		SeamPatchNormals( param ) ;
		//
		m_flagUpdateVertex = false ;
	}
}

// パッチ間の縮退頂点の法線を合成する
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::SeamPatchNormals( const S3DMeshEditor::MeshParam& param )
{
	double	cosLimitAngle = -1.0 ;
	if ( param.nFlags & flagMeshEdgeByAngle )
	{
		cosLimitAngle = cos( param.fpEdgeAngle * (PI / 180.0 * 0.5) ) ;
	}
	//
	for ( size_t i = 0; i < m_seams.GetLength(); i ++ )
	{
		SewPatchPoints *	pspp = m_seams.GetAt( i ) ;
		ESLAssert( pspp != nullptr ) ;
		if ( pspp->m_nFlags & degenerateDivNormal )
		{
			continue ;
		}
		const size_t	nCount = pspp->GetLength() ;
		if ( nCount == 0 )
		{
			continue ;
		}
		S3DVector			vPoint( 0, 0, 0 ) ;
		S3DVector			vNormal( 0, 0, 0 ) ;
		const PatchPoint *	ppp = pspp->GetConstArray() ;
		for ( size_t j = 0; j < nCount; j ++ )
		{
			Patch *		pPatch = ppp[j].pPatch ;
			size_t		iVertex = ppp[j].iVertex ;
			if ( pPatch->GetTotalVertexCount() <= iVertex )
			{
				continue ;
			}
			S3DVector	vn = pPatch->GetNormalAt( iVertex ) ;
			if ( pPatch->ShouldNormalInverseAt(iVertex) )
			{
				vn = - vn ;
			}
			vPoint += pPatch->GetPointAt( iVertex ) ;
			vNormal += vn ;
		}
		vPoint *= 1.0f / (float32_t) nCount ;
		vNormal.Normalize() ;
		//
		for ( size_t j = 0; j < nCount; j ++ )
		{
			Patch *			pPatch = ppp[j].pPatch ;
			const size_t	nAreaSize = pPatch->GetAreaSize() ;
			const size_t	iVertex = ppp[j].iVertex ;
			if ( pPatch->GetTotalVertexCount() <= iVertex )
			{
				continue ;
			}
			S3DVector		vn = pPatch->GetNormalAt( iVertex ) ;
			if ( pPatch->ShouldNormalInverseAt(iVertex) )
			{
				vn = - vn ;
			}
			DegenerateEntry		de ;
			const uint32_t *	pIndex =
				pPatch->m_degenerates.FindVertex( de, ppp[j].iVertex ) ;
			if ( pIndex != nullptr )
			{
				for ( size_t k = 0; k < de.nCount; k ++ )
				{
					const size_t	kVertex = (size_t) pIndex[k] ;
					pPatch->SetPointAt( kVertex, vPoint ) ;
				}
			}
			else
			{
				pPatch->SetPointAt( iVertex, vPoint ) ;
			}
			if ( vn.Normalized().InnerProduct( vNormal ) < cosLimitAngle )
			{
				continue ;
			}
			if ( pIndex != nullptr )
			{
				for ( size_t k = 0; k < de.nCount; k ++ )
				{
					const size_t	kVertex = (size_t) pIndex[k] ;
					if ( pPatch->ShouldNormalInverseAt(kVertex) )
					{
						pPatch->SetNormalAt( kVertex, - vNormal ) ;
					}
					else
					{
						pPatch->SetNormalAt( kVertex, vNormal ) ;
					}
				}
			}
			else
			{
				if ( pPatch->ShouldNormalInverseAt(iVertex) )
				{
					pPatch->SetNormalAt( iVertex, - vNormal ) ;
				}
				else
				{
					pPatch->SetNormalAt( iVertex, vNormal ) ;
				}
			}
		}
	}
}

// バーテックスバッファ構築
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::RenderVertexBuffers
	( S3DVertexBufferInterface *const* ppVBOs,
		S3DMaterial *const* ppMaterials,
		size_t * pExistingCount, size_t nDstBufferCount,
		const S3DMeshEditor::MeshParam& param, uint32_t nFlags )
{
	UpdateAllPatchs( param ) ;
	//
	for ( size_t i = 0; i < nDstBufferCount; i ++ )
	{
		pExistingCount[i] = 0 ;
	}
	MeshBuffer	mesh ;
	GetExAttrWeightLayers( mesh.m_bufExAttrIndex ) ;
	//
	S3DMatrix	matI( 1, 1, 1 ) ;
	S3DVector	vZero( 0, 0, 0 ) ;
	for ( size_t i = 0; i < m_patchs.GetLength(); i ++ )
	{
		Patch *	pPatch = m_patchs.GetAt( i ) ;
		ESLAssert( pPatch != nullptr ) ;
		//
		size_t	iMaterial = pPatch->GetMaterialIndex() ;
		if ( !(nFlags & renderAll) )
		{
			if ( nFlags & renderCollision )
			{
				if ( !(pPatch->GetFlags() & Patch::flagCollision) )
				{
					continue ;
				}
			}
			else if ( pPatch->GetFlags() & Patch::flagInvisible )
			{
				continue ;
			}
			if ( nFlags & renderModifiable )
			{
				if ( pPatch->GetFlags() & Patch::flagDisableModifier )
				{
					continue ;
				}
			}
		}
		pPatch->BuildPatchMesh( mesh, param ) ;
		//
		if ( iMaterial < nDstBufferCount )
		{
			ESLAssert( ppVBOs[iMaterial] != nullptr ) ;
			S3DVertexBufferInterface *	pVBO = ppVBOs[iMaterial] ;
			S3DMaterial *				pMaterial = ppMaterials[iMaterial] ;
			RenderQuadMeshBuffer
				( *pVBO, pMaterial, matI, vZero, mesh, param ) ;
			RenderExPatchTriangles
				( *pVBO, pMaterial, matI, vZero, *pPatch, mesh, param ) ;
			pExistingCount[iMaterial] ++ ;
		}
	}
}

void S3DMeshEditor::RenderQuadMeshBuffer
	( S3DVertexBufferInterface& vbo,
		S3DMaterial * pMaterial,
		const S3DMatrix& matMesh, const S3DVector& vMesh,
		S3DMeshEditor::MeshBuffer& mesh,
		const S3DMeshEditor::MeshParam& param )
{
	const size_t	nVertexCount = mesh.m_countVertex ;
	const size_t	nQuadCount = mesh.m_countIndex / 4 ;
	if ( nQuadCount == 0 )
	{
		return ;
	}
	const size_t	iMesh = vbo.GetMeshCount() ;
	const S3DVector	vZero( 0, 0, 0 ) ;
	//
	// 頂点属性設定
	//
	S3DPrimitiveType	typePrimitive = primitiveTriangle ;
	size_t	nIndexCount = nQuadCount * 6 ;
	if ( param.nFlags & flagMeshPrimitiveType )
	{
		if ( param.nPrimitiveType == primitiveLine )
		{
			typePrimitive = primitiveLine ;
			nIndexCount = nQuadCount * 8 ;
		}
		else if ( param.nPrimitiveType == primitivePoint )
		{
			typePrimitive = primitivePoint ;
			nIndexCount = nQuadCount * 4 ;
		}
	}
	S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
	vbo.AllocatePrimitiveBuffer
		( prmbuf, primitiveTriangle, nIndexCount, nVertexCount ) ;
	//
	ESLAssert( mesh.m_bufVertex.GetLength() >= nVertexCount ) ;
	const S3DVector4 *	pvSrcVertex = mesh.m_bufVertex.GetConstArray() ;
	matMesh.RevolveVectors( prmbuf.pvVertex, pvSrcVertex, nVertexCount, vMesh ) ;
	//
	ESLAssert( mesh.m_bufNormal.GetLength() >= nVertexCount ) ;
	const S3DVector4 *	pvSrcNormal = mesh.m_bufNormal.GetConstArray() ;
	matMesh.RevolveVectors( prmbuf.pvNormal, pvSrcNormal, nVertexCount, vZero ) ;
	//
	ESLAssert( mesh.m_bufUVMap.GetLength() >= nVertexCount ) ;
	const S2DVector *	pvSrcUV = mesh.m_bufUVMap.GetConstArray() ;
	for ( size_t i = 0; i < nVertexCount; i ++ )
	{
		prmbuf.pvUVMap[i] = pvSrcUV[i] ;
	}
	//
	ESLAssert( mesh.m_bufColor.GetLength() >= nVertexCount ) ;
	const S3DColor *	pvSrcColor = mesh.m_bufColor.GetConstArray() ;
	for ( size_t i = 0; i < nVertexCount; i ++ )
	{
		prmbuf.pColor[i] = pvSrcColor[i] ;
	}
	//
	const uint32_t *	pSrcIndex = mesh.m_bufIndex.GetConstArray() ;
	if ( typePrimitive == primitiveTriangle )
	{
		for ( size_t i = 0, j = 0; i < nQuadCount; i ++, j += 6 )
		{
			size_t	k = i * 4 ;
			prmbuf.pIndexedList[j]     = pSrcIndex[k] ;
			prmbuf.pIndexedList[j + 1] = pSrcIndex[k + 2] ;
			prmbuf.pIndexedList[j + 2] = pSrcIndex[k + 3] ;
			prmbuf.pIndexedList[j + 3] = pSrcIndex[k] ;
			prmbuf.pIndexedList[j + 4] = pSrcIndex[k + 3] ;
			prmbuf.pIndexedList[j + 5] = pSrcIndex[k + 1] ;
		}
	}
	else if ( typePrimitive == primitiveLine )
	{
		if ( param.nFlags & flagMeshEdgeSingleLine )
		{
			static const size_t	s_iEdgeIndex[] =
			{
				0, 1, 3, 2, 0
			} ;
			size_t	iDst = 0 ;
			mesh.m_edegs.SetLimit( nIndexCount / 2 ) ;
			for ( size_t i = 0; i < nQuadCount; i ++ )
			{
				size_t	k = i * 4 ;
				for ( size_t l = 0; l < 4; l ++ )
				{
					Edge	edge( (size_t) pSrcIndex[k + s_iEdgeIndex[l]],
									(size_t) pSrcIndex[k + s_iEdgeIndex[l + 1]] ) ;
					if ( mesh.m_edegs.FindSorted( edge ) < 0 )
					{
						prmbuf.pIndexedList[iDst ++] = (uint32_t) edge.iVertex0 ;
						prmbuf.pIndexedList[iDst ++] = (uint32_t) edge.iVertex1 ;
						mesh.m_edegs.AddSorted( edge ) ;
					}
				}
			}
			nIndexCount = iDst ;
		}
		else
		{
			for ( size_t i = 0, j = 0; i < nQuadCount; i ++, j += 8 )
			{
				size_t	k = i * 4 ;
				prmbuf.pIndexedList[j]     = pSrcIndex[k] ;
				prmbuf.pIndexedList[j + 1] = pSrcIndex[k + 1] ;
				prmbuf.pIndexedList[j + 2] = pSrcIndex[k + 1] ;
				prmbuf.pIndexedList[j + 3] = pSrcIndex[k + 3] ;
				prmbuf.pIndexedList[j + 4] = pSrcIndex[k + 3] ;
				prmbuf.pIndexedList[j + 5] = pSrcIndex[k + 2] ;
				prmbuf.pIndexedList[j + 6] = pSrcIndex[k + 2] ;
				prmbuf.pIndexedList[j + 7] = pSrcIndex[k] ;
			}
		}
	}
	else
	{
		for ( size_t i = 0, j = 0; i < nQuadCount; i ++, j += 4 )
		{
			size_t	k = i * 4 ;
			prmbuf.pIndexedList[j]     = pSrcIndex[k] ;
			prmbuf.pIndexedList[j + 1] = pSrcIndex[k + 1] ;
			prmbuf.pIndexedList[j + 2] = pSrcIndex[k + 2] ;
			prmbuf.pIndexedList[j + 3] = pSrcIndex[k + 3] ;
		}
	}
	//
	vbo.AddPrimitiveBuffer
		( pMaterial, 0, typePrimitive, prmbuf, nIndexCount, nVertexCount ) ;
	//
	// 拡張属性設定
	//
	if ( mesh.m_nWeightLayers > 0 )
	{
		vbo.SetExtendVertexAttribute
			( iMesh, mesh.m_nWeightLayers,
				nVertexCount, mesh.m_bufWeight.GetConstArray() ) ;
	}
}

void S3DMeshEditor::RenderExPatchTriangles
	( S3DVertexBufferInterface& vbo,
		S3DMaterial * pMaterial,
		const S3DMatrix& matMesh, const S3DVector& vMesh,
		const S3DMeshEditor::Patch& patch,
		S3DMeshEditor::MeshBuffer& bufTemp,
		const S3DMeshEditor::MeshParam& param )
{
	const size_t	nAreaSize = patch.GetAreaSize() ;
	const size_t	nVertexCount = patch.GetExVertexCount() ;
	const size_t	nFaceCount = patch.GetExFaceCount() ;
	if ( nFaceCount == 0 )
	{
		return ;
	}
	const size_t	iMesh = vbo.GetMeshCount() ;
	//
	// 頂点属性設定
	//
	S3DPrimitiveType	typePrimitive = primitiveTriangle ;
	size_t	nIndexCount = nFaceCount * 3 ;
	if ( param.nFlags & flagMeshPrimitiveType )
	{
		if ( param.nPrimitiveType == primitiveLine )
		{
			typePrimitive = primitiveLine ;
			nIndexCount = nFaceCount * 6 ;
		}
		else if ( param.nPrimitiveType == primitivePoint )
		{
			typePrimitive = primitivePoint ;
			nIndexCount = nFaceCount * 3 ;
		}
	}
	S3DVertexBufferInterface::PrimitiveBuffer	prmbuf ;
	vbo.AllocatePrimitiveBuffer
		( prmbuf, typePrimitive, nIndexCount, nVertexCount ) ;
	//
	const S3DVector *
		pvSrcVertex = patch.GetConstPointArray( nAreaSize, nVertexCount ) ;
	for ( size_t i = 0; i < nVertexCount; i ++ )
	{
		prmbuf.pvVertex[i] = matMesh * pvSrcVertex[i] + vMesh ;
	}
	const S3DVector *
		pvSrcNormal = patch.GetConstNormalArray( nAreaSize, nVertexCount ) ;
	for ( size_t i = 0; i < nVertexCount; i ++ )
	{
		prmbuf.pvNormal[i] = matMesh * pvSrcNormal[i] ;
	}
	const S2DVector *
		pvSrcUV = patch.GetConstUVArray( nAreaSize, nVertexCount ) ;
	for ( size_t i = 0; i < nVertexCount; i ++ )
	{
		prmbuf.pvUVMap[i] = pvSrcUV[i] ;
	}
	const S3DColor *
		pvSrcColor = patch.GetConstColorArray( nAreaSize, nVertexCount ) ;
	for ( size_t i = 0; i < nVertexCount; i ++ )
	{
		prmbuf.pColor[i] = pvSrcColor[i] ;
	}
	const uint32_t *
		pSrcIndex = patch.GetExFaceTriangleIndexes( 0, nFaceCount ) ;
	if ( typePrimitive == primitiveLine )
	{
		if ( param.nFlags & flagMeshEdgeSingleLine )
		{
			static const size_t	s_iEdgeIndex[] =
			{
				0, 1, 2, 0
			} ;
			size_t	iDst = 0 ;
			bufTemp.m_edegs.SetLimit( bufTemp.m_edegs.GetLength() + (nIndexCount / 2) ) ;
			for ( size_t i = 0; i < nFaceCount; i ++ )
			{
				size_t	k = i * 3 ;
				for ( size_t l = 0; l < 3; l ++ )
				{
					Edge	edge( (size_t) pSrcIndex[k + s_iEdgeIndex[l]],
									(size_t) pSrcIndex[k + s_iEdgeIndex[l + 1]] ) ;
					if ( bufTemp.m_edegs.FindSorted( edge ) < 0 )
					{
						prmbuf.pIndexedList[iDst ++] = (uint32_t) edge.iVertex0 ;
						prmbuf.pIndexedList[iDst ++] = (uint32_t) edge.iVertex1 ;
						bufTemp.m_edegs.AddSorted( edge ) ;
					}
				}
			}
			nIndexCount = iDst ;
		}
		else
		{
			for ( size_t i = 0, j = 0; i < nFaceCount; i ++, j += 6 )
			{
				size_t	k = i * 3 ;
				prmbuf.pIndexedList[j]     = pSrcIndex[k] ;
				prmbuf.pIndexedList[j + 1] = pSrcIndex[k + 1] ;
				prmbuf.pIndexedList[j + 2] = pSrcIndex[k + 1] ;
				prmbuf.pIndexedList[j + 3] = pSrcIndex[k + 2] ;
				prmbuf.pIndexedList[j + 4] = pSrcIndex[k + 2] ;
				prmbuf.pIndexedList[j + 5] = pSrcIndex[k] ;
			}
		}
	}
	else
	{
		const int8_t *	pFaces =
			patch.GetConstFaceArray( patch.GetAreaSize(), nFaceCount ) ;
		size_t	iDst = 0 ;
		for ( size_t i = 0, j = 0; i < nFaceCount; i ++, j += 3 )
		{
			int8_t	face = pFaces[i] ;
			if ( face == S3DMeshEditor::Patch::faceFront )
			{
				prmbuf.pIndexedList[iDst]     = pSrcIndex[j] ;
				prmbuf.pIndexedList[iDst + 1] = pSrcIndex[j + 1] ;
				prmbuf.pIndexedList[iDst + 2] = pSrcIndex[j + 2] ;
				iDst += 3 ;
			}
			else if ( face == S3DMeshEditor::Patch::faceBack )
			{
				prmbuf.pIndexedList[iDst]     = pSrcIndex[j] ;
				prmbuf.pIndexedList[iDst + 1] = pSrcIndex[j + 2] ;
				prmbuf.pIndexedList[iDst + 2] = pSrcIndex[j + 1] ;
				iDst += 3 ;
			}
		}
		nIndexCount = iDst ;
		if ( iDst == 0 )
		{
			vbo.FreePrimitiveBuffer( prmbuf ) ;
			return ;
		}
	}
	//
	vbo.AddPrimitiveBuffer
		( pMaterial, 0, typePrimitive, prmbuf, nIndexCount, nVertexCount ) ;
	//
	// 拡張属性設定
	//
	if ( bufTemp.m_bufExAttrIndex.GetLength() > 0 )
	{
		const size_t	nWeightLayers = bufTemp.m_bufExAttrIndex.GetLength() ;
		const size_t *	pExAttrLayers = bufTemp.m_bufExAttrIndex.GetConstArray() ;
		float32_t *		pExAttrWeight =
			bufTemp.m_bufWeight.GetArray( nVertexCount * nWeightLayers ) ;
		for ( size_t i = 0; i < nVertexCount; i ++ )
		{
			for ( size_t j = 0; j < nWeightLayers; j ++ )
			{
				pExAttrWeight[i * nWeightLayers + j] =
					patch.GetWeightAt( nAreaSize + i, pExAttrLayers[j] ) ;
			}
		}
		vbo.SetExtendVertexAttribute
			( iMesh, nWeightLayers, nVertexCount, pExAttrWeight ) ;
		bufTemp.m_bufWeight.FinishArray() ;
	}
}

// シリアライズ更新フラグ設定
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::SetUpdateSerializeFlag( void )
{
	m_flagUpdateSerialize = true ;
}

// シリアライズ更新フラグ取得
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditor::GetUpdateSerializeFlag( void ) const
{
	return	m_flagUpdateSerialize ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::SerializeMesh( S3DMeshBufferPropertySerializer& mesh )
{
	S3DSceneComposer::BinaryHeader	binhdr ;
	binhdr.nType = S3DSceneComposer::binaryMeshEditor ;
	binhdr.nSubType = 0 ;
	binhdr.nBodyBytes = 0 ;
	binhdr.nReserved = 0 ;
	mesh.SetBinaryType( binhdr ) ;
	//
	for ( size_t iPatch = 0; iPatch < m_patchs.GetLength(); iPatch ++ )
	{
		Patch *	pPatch = m_patchs.GetAt( iPatch ) ;
		ESLAssert( pPatch != nullptr ) ;
		//
		// シリアライザ取得・設定
		//
		S3DMeshBufferPropertySerializer::MeshBuffer *
							pMeshBuf = pPatch->GetSerializer() ;
		bool	flagNewMesh = false ;
		if ( (pMeshBuf == nullptr) || (mesh.FindMesh( pMeshBuf ) < 0) )
		{
			pMeshBuf = new S3DMeshBufferPropertySerializer::MeshBuffer ;
			//
			size_t	iBuf = mesh.AddMesh( pMeshBuf ) ;
			mesh.SwapMesh( iPatch, iBuf ) ;
			ESLAssert( mesh.GetMeshAt( iPatch ) == pMeshBuf ) ;
			//
			pPatch->AttachSerializer( pMeshBuf ) ;
			flagNewMesh = true ;
		}
		else
		{
			ssize_t	iMeshBuf = mesh.FindMesh( pMeshBuf ) ;
			ESLAssert( iMeshBuf >= 0 ) ;
			if ( (iMeshBuf >= 0) && (iMeshBuf != iPatch) )
			{
				mesh.SwapMesh( iPatch, (size_t) iMeshBuf ) ;
			}
		}
		if ( !flagNewMesh && !pPatch->GetUpdateSerializeFlag() )
		{
			continue ;
		}
		mesh.UpdateMesh( pMeshBuf ) ;
		//
		// 基本情報設定
		//
		pPatch->NormalizeDegenerateEntry() ;
		pPatch->UpdatePatchMesh() ;
		pPatch->SerializeToMesh( *pMeshBuf ) ;
		if ( iPatch == 0 )
		{
			SerializeMeshInfo( *pMeshBuf ) ;
		}
	}
	while ( mesh.GetMeshCount() > m_patchs.GetLength() )
	{
		mesh.RemoveMeshAt( m_patchs.GetLength() ) ;
	}
	//
	// 全体のパラメータと縫い目情報、レイヤー名
	//
	S3DMeshBufferPropertySerializer::MeshBuffer *	pMeshBuf0 = nullptr ;
	if ( mesh.GetMeshCount() > 0 )
	{
		pMeshBuf0 = mesh.GetMeshAt( 0 ) ;
	}
	if ( pMeshBuf0 == nullptr )
	{
		pMeshBuf0 = new S3DMeshBufferPropertySerializer::MeshBuffer ;
		mesh.AddMesh( pMeshBuf0 ) ;
	}
	mesh.UpdateMesh( pMeshBuf0 ) ;
	SerializeMeshInfo( *pMeshBuf0 ) ;
	//
	m_flagUpdateSerialize = false ;
}

void S3DMeshEditor::SerializeMeshInfo
		( S3DMeshBufferPropertySerializer::MeshBuffer& meshBuf )
{
	//
	// メッシュ情報
	//
	SerializedMeshInfo	meshInfo ;
	eslFillMemory( &meshInfo, 0, sizeof(SerializedMeshInfo) ) ;
	meshInfo.nHeaderBytes = (uint32_t) sizeof(SerializedMeshInfo) ;
	meshInfo.nHeaderFlags = 0 ;
	meshInfo.nWeightLayerCount = (uint32_t) m_aWeightLayerIDs.GetLength() ;
	meshInfo.addrWeightLayerID =
		(uint32_t) (sizeof(SerializedMeshInfo)
					+ sizeof(WeightMapInfo) * m_aWeightLayerInfos.GetLength()) ;
	//
	SArray<uint8_t> *	pBinMeshInfo = new SArray<uint8_t> ;
	pBinMeshInfo->AddArray
		( (const uint8_t*) &meshInfo, sizeof(SerializedMeshInfo) ) ;
	//
	ESLAssert( m_aWeightLayerInfos.GetLength() == meshInfo.nWeightLayerCount ) ;
	for ( size_t i = 0; i < m_aWeightLayerInfos.GetLength(); i ++ )
	{
		ESLAssert( m_aWeightLayerInfos.GetAt( i ) != nullptr ) ;
		pBinMeshInfo->AddArray
			( (const uint8_t*) m_aWeightLayerInfos.GetAt( i ), sizeof(WeightMapInfo) ) ;
	}
	for ( size_t i = 0; i < m_aWeightLayerIDs.GetLength(); i ++ )
	{
		SString *	pstrID = m_aWeightLayerIDs.GetAt( i ) ;
		ESLAssert( pstrID != nullptr ) ;
		pBinMeshInfo->AddArray
			( (const uint8_t*) pstrID->GetConstArray(),
				(pstrID->GetLength() + 1) * sizeof(uint16_t) ) ;
	}
	//
	meshBuf.m_ssaExBuf.SetAs( L"mesh_info", pBinMeshInfo ) ;
	//
	// パッチ間接続情報
	//
	SArray<uint8_t> *	pBinSeams = new SArray<uint8_t> ;
	m_seams.Serialize( *pBinSeams, *this ) ;
	//
	meshBuf.m_ssaExBuf.SetAs( L"seams_set", pBinSeams ) ;
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::DeserializeMesh( const S3DMeshBufferPropertySerializer& mesh )
{
	ClearAllMeshs() ;
	//
	if ( mesh.GetBinaryHeader().nType == S3DSceneComposer::binaryMesh )
	{
		ConvertFromMeshBuffer( mesh ) ;
		return ;
	}
	if ( mesh.GetBinaryHeader().nType != S3DSceneComposer::binaryMeshEditor )
	{
		return ;
	}
	for ( size_t iMesh = 0; iMesh < mesh.GetMeshCount(); iMesh ++ )
	{
		S3DMeshBufferPropertySerializer::MeshBuffer *
									pMeshBuf = mesh.GetMeshAt( iMesh ) ;
		ESLAssert( pMeshBuf != nullptr ) ;
		if ( pMeshBuf == nullptr )
		{
			continue ;
		}
		Patch *	pPatch = NewPatch() ;
		pPatch->DeserializeFromMesh( *pMeshBuf ) ;
		pPatch->AttachSerializer( pMeshBuf ) ;
		m_patchs.Add( pPatch ) ;
	}
	//
	S3DMeshBufferPropertySerializer::MeshBuffer *
								pMeshBuf = mesh.GetMeshAt( 0 ) ;
	if ( pMeshBuf != nullptr )
	{
		DeserializeMeshInfo( *pMeshBuf ) ;
	}
	//
	if ( m_patchs.GetLength() == 1 )
	{
		Patch *	pPatch = m_patchs.GetAt( 0 ) ;
		if ( (pPatch->GetTotalVertexCount() == 0)
			&& (pPatch->GetTotalFaceCount() == 0) )
		{
			m_patchs.RemoveAll() ;
		}
	}
	//
	m_flagUpdateVertex = true ;
	m_flagUpdateSerialize = false ;
}

void S3DMeshEditor::DeserializeMeshInfo
		( S3DMeshBufferPropertySerializer::MeshBuffer& meshBuf )
{
	//
	// メッシュ情報
	//
	SArray<uint8_t> *	pBinMeshInfo = meshBuf.m_ssaExBuf.GetAs( L"mesh_info" ) ;
	if ( pBinMeshInfo != nullptr )
	{
		const uint8_t *	pMeshInfo = pBinMeshInfo->GetConstArray() ;
		const SerializedMeshInfo&
					meshInfo = *((const SerializedMeshInfo*) pMeshInfo) ;
		//
		const WeightMapInfo *	pwmiInfos =
				(const WeightMapInfo*) (pMeshInfo + meshInfo.nHeaderBytes) ;
		for ( size_t i = 0; i < meshInfo.nWeightLayerCount; i ++ )
		{
			WeightMapInfo *	pwmi = new WeightMapInfo ;
			*pwmi = pwmiInfos[i] ;
			m_aWeightLayerInfos.Add( pwmi ) ;
		}
		const uint16_t *	pwLayerID =
			(const uint16_t*) (pMeshInfo + meshInfo.addrWeightLayerID) ;
		for ( size_t i = 0; i < meshInfo.nWeightLayerCount; i ++ )
		{
			SString *	pstrID = new SString( pwLayerID ) ;
			m_aWeightLayerIDs.Add( pstrID ) ;
			//
			pwLayerID += pstrID->GetLength() + 1 ;
		}
	}
	//
	// パッチ間接続情報
	//
	SArray<uint8_t> *	pBinSeams = meshBuf.m_ssaExBuf.GetAs( L"seams_set" ) ;
	if ( pBinSeams != nullptr )
	{
		m_seams.Deserialize( *pBinSeams, *this ) ;
	}
}

// 通常のメッシュバッファから変換
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::ConvertFromMeshBuffer( const S3DMeshBufferPropertySerializer& mesh )
{
	ClearAllMeshs() ;
	//
	if ( mesh.GetBinaryHeader().nType != S3DSceneComposer::binaryMesh )
	{
		return ;
	}
	for ( size_t iMesh = 0; iMesh < mesh.GetMeshCount(); iMesh ++ )
	{
		S3DMeshBufferPropertySerializer::MeshBuffer *
									pMeshBuf = mesh.GetMeshAt( iMesh ) ;
		ESLAssert( pMeshBuf != nullptr ) ;
		if ( pMeshBuf == nullptr )
		{
			continue ;
		}
		Patch *	pPatch = NewPatch() ;
		pPatch->ConvertFromMeshBuffer( *pMeshBuf ) ;
		m_patchs.Add( pPatch ) ;
	}
}

// パッチ総数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshEditor::GetPatchCount( void ) const
{
	return	m_patchs.GetLength() ;
}

// パッチ生成
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditor::Patch * S3DMeshEditor::NewPatch( void )
{
	Patch *	pPatch = new Patch( &m_seams ) ;
	pPatch->InsertWeightLayer( 0, m_aWeightLayerIDs.GetLength() ) ;
	return	pPatch ;
}

// パッチ取得
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditor::Patch * S3DMeshEditor::GetPatchAt( size_t i ) const
{
	return	m_patchs.GetAt( i ) ;
}

// パッチ追加
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshEditor::AddPatch( Patch * pPatch )
{
	return	InsertPatchAt( m_patchs.GetLength(), pPatch ) ;
}

size_t S3DMeshEditor::InsertPatchAt( size_t i, S3DMeshEditor::Patch * pPatch )
{
	m_flagUpdateVertex = true ;
	m_flagUpdateSerialize = true ;
	if ( pPatch->GetWeightLayerCount() < GetWeightLayerCount() )
	{
		pPatch->InsertWeightLayer
			( pPatch->GetWeightLayerCount(),
				GetWeightLayerCount() - pPatch->GetWeightLayerCount() ) ;
	}
	else if ( pPatch->GetWeightLayerCount() > GetWeightLayerCount() )
	{
		pPatch->RemoveWeightLayer
			( GetWeightLayerCount(),
				pPatch->GetWeightLayerCount() - GetWeightLayerCount() ) ;
	}
	return	m_patchs.InsertAt( i, pPatch ) ;
}

// パッチ削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::RemovePatchAt( size_t i )
{
	Patch *	pPatch = m_patchs.GetAt( i ) ;
	if ( pPatch != nullptr )
	{
		m_seams.RemovePointIndex( pPatch, 0, pPatch->GetTotalVertexCount() ) ;
	}
	m_patchs.RemoveAt( i ) ;
	//
	m_flagUpdateVertex = true ;
	m_flagUpdateSerialize = true ;
}

void S3DMeshEditor::RemoveAllPatchs( void )
{
	m_seams.RemoveAll() ;
	m_patchs.RemoveAll() ;
	//
	m_flagUpdateVertex = true ;
	m_flagUpdateSerialize = true ;
}

// パッチ検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DMeshEditor::FindPatch( S3DMeshEditor::Patch * pPatch ) const
{
	return	m_patchs.FindPtr( pPatch ) ;
}

// パッチ順序入れ替え
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::SwapPatchOrder( size_t iPatch0, size_t iPatch1 )
{
	if ( (iPatch0 < m_patchs.GetLength())
		&& (iPatch1 < m_patchs.GetLength()) )
	{
		m_patchs.Swap( iPatch0, iPatch1 ) ;
		m_flagUpdateSerialize = true ;
	}
}

// ウェイトマップ数
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshEditor::GetWeightLayerCount( void ) const
{
	return	m_aWeightLayerIDs.GetLength() ;
}

// ウェイトマップレイヤーID取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DMeshEditor::GetWeightLayerIDAt( size_t iLayer ) const
{
	SString *	pstrID = m_aWeightLayerIDs.GetAt( iLayer ) ;
	if ( pstrID == nullptr )
	{
		return	nullptr ;
	}
	return	*pstrID ;
}

// ウェイトマップ情報取得
//////////////////////////////////////////////////////////////////////////////
const S3DMeshEditor::WeightMapInfo *
	S3DMeshEditor::GetWeightLayerInfoAt( size_t iLayer ) const
{
	return	m_aWeightLayerInfos.GetAt( iLayer ) ;
}

// ウェイトマップレイヤーID設定
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::SetWeightLayerIDAt( size_t iLayer, const wchar_t * pwszID )
{
	SString *	pstrID = m_aWeightLayerIDs.GetAt( iLayer ) ;
	if ( pstrID != nullptr )
	{
		*pstrID = pwszID ;
	}
}

// ウェイトマップ情報設定
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::SetWeightLayerInfoAt( size_t iLayer, const WeightMapInfo& wminf )
{
	WeightMapInfo *	pwmi = m_aWeightLayerInfos.GetAt( iLayer ) ;
	if ( pwmi != nullptr )
	{
		*pwmi = wminf ;
	}
}

// ウェイトマップレイヤー検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DMeshEditor::FindWeightLayerAs( const wchar_t * pwszID ) const
{
	for ( size_t i = 0; i < m_aWeightLayerIDs.GetLength(); i ++ )
	{
		SString *	pstrID = m_aWeightLayerIDs.GetAt( i ) ;
		if ( (pstrID != nullptr) && (*pstrID == pwszID) )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

// ウェイトマップレイヤー追加
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::InsertWeightLayerAt( size_t iLayer, const wchar_t * pwszID )
{
	ESLAssert( iLayer <= m_aWeightLayerIDs.GetLength() ) ;
	if ( iLayer > m_aWeightLayerIDs.GetLength() )
	{
		iLayer = m_aWeightLayerIDs.GetLength() ;
	}
	m_aWeightLayerIDs.InsertAt( iLayer, new SString( pwszID ) ) ;
	m_aWeightLayerInfos.InsertAt( iLayer, new WeightMapInfo ) ;
	//
	for ( size_t i = 0; i < m_patchs.GetLength(); i ++ )
	{
		Patch *	pPatch = m_patchs.GetAt( i ) ;
		ESLAssert( pPatch != nullptr ) ;
		pPatch->InsertWeightLayer( iLayer, 1 ) ;
	}
	//
	m_flagUpdateSerialize = true ;
}

// ウェイトマップレイヤー削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::RemoveWeightLayer( size_t iLayer, size_t nCount )
{
	ESLAssert( iLayer <= m_aWeightLayerIDs.GetLength() ) ;
	if ( iLayer >= m_aWeightLayerIDs.GetLength() )
	{
		iLayer = m_aWeightLayerIDs.GetLength() ;
	}
	if ( iLayer + nCount > m_aWeightLayerIDs.GetLength() )
	{
		nCount = m_aWeightLayerIDs.GetLength() - iLayer ;
	}
	m_aWeightLayerIDs.Remove( iLayer, nCount ) ;
	m_aWeightLayerInfos.Remove( iLayer, nCount ) ;
	//
	for ( size_t i = 0; i < m_patchs.GetLength(); i ++ )
	{
		Patch *	pPatch = m_patchs.GetAt( i ) ;
		ESLAssert( pPatch != nullptr ) ;
		pPatch->RemoveWeightLayer( iLayer, nCount ) ;
	}
	//
	m_flagUpdateSerialize = true ;
}

// ウェイトマップレイヤー入れ替え
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::SwapWeightLayer( size_t iLayer0, size_t iLayer1 )
{
	ESLAssert( iLayer0 < m_aWeightLayerIDs.GetLength() ) ;
	ESLAssert( iLayer1 < m_aWeightLayerIDs.GetLength() ) ;
	if ( (iLayer0 >= m_aWeightLayerIDs.GetLength())
		|| (iLayer1 >= m_aWeightLayerIDs.GetLength()) )
	{
		return ;
	}
	m_aWeightLayerIDs.Swap( iLayer0, iLayer1 ) ;
	m_aWeightLayerInfos.Swap( iLayer0, iLayer1 ) ;
	//
	for ( size_t i = 0; i < m_patchs.GetLength(); i ++ )
	{
		Patch *	pPatch = m_patchs.GetAt( i ) ;
		ESLAssert( pPatch != nullptr ) ;
		pPatch->SwapWeightLayer( iLayer0, iLayer1 ) ;
	}
	//
	m_flagUpdateSerialize = true ;
}

// 拡張属性（ボーン用ウェイトマップではない）レイヤー取得
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::GetExAttrWeightLayers( SSystem::SArray<size_t>& aLayers ) const
{
	aLayers.RemoveAll() ;
	//
	for ( size_t iLayer = 0; iLayer < m_aWeightLayerInfos.GetLength(); iLayer ++ )
	{
		const WeightMapInfo *	pwmi = m_aWeightLayerInfos.GetAt( iLayer ) ;
		ESLAssert( pwmi != nullptr ) ;
		if ( (pwmi != nullptr) && !(pwmi->nFlags & weightBone) )
		{
			aLayers.Add( iLayer ) ;
		}
	}
}

// ボーン用ウェイトマップを正規化
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::NormalizeBoneWeightMap( void )
{
	const size_t	nWeightLayerCount = GetWeightLayerCount() ;
	SArray<size_t>	aWeightLayer ;
	for ( size_t i = 0; i < nWeightLayerCount; i ++ )
	{
		const S3DMeshEditor::WeightMapInfo *
						pwmi = GetWeightLayerInfoAt( i ) ;
		ESLAssert( pwmi != nullptr ) ;
		if ( pwmi->nFlags & S3DMeshEditor::weightBone )
		{
			aWeightLayer.Add( i ) ;
		}
	}
	const size_t *	pWeightBoneLayer = aWeightLayer.GetConstArray() ;
	const size_t	nWeightBoneLayers = aWeightLayer.GetLength() ;
	for ( size_t iPatch = 0; iPatch < GetPatchCount(); iPatch ++ )
	{
		S3DMeshEditor::Patch *	pPatch = GetPatchAt( iPatch ) ;
		ESLAssert( pPatch != nullptr ) ;
		ESLAssert( pPatch->GetWeightLayerCount() == nWeightLayerCount ) ;
		if ( pPatch->GetWeightLayerCount() < nWeightLayerCount )
		{
			continue ;
		}
		const size_t	nVertexCount = pPatch->GetTotalVertexCount() ;
		for ( size_t i = 0; i < nVertexCount; i ++ )
		{
			float32_t	wSum = 0.0f ;
			for ( size_t j = 0; j  < nWeightBoneLayers; j ++ )
			{
				wSum += pPatch->GetWeightAt( i, pWeightBoneLayer[j] ) ;
			}
			if ( wSum > 0.0f )
			{
				float32_t	wRcp = 1.0f / wSum ;
				for ( size_t j = 0; j  < nWeightBoneLayers; j ++ )
				{
					pPatch->SetWeightAt
						( i, pWeightBoneLayer[j],
							pPatch->GetWeightAt( i, pWeightBoneLayer[j] ) * wRcp ) ;
				}
			}
		}
	}
}

// ボーン用ウェイトマップをGPU用に最適化
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::OptimizeBoneWeightMap( void )
{
	const size_t	nWeightLayerCount = GetWeightLayerCount() ;
	SArray<size_t>	aWeightLayer ;
	for ( size_t i = 0; i < nWeightLayerCount; i ++ )
	{
		const S3DMeshEditor::WeightMapInfo *
						pwmi = GetWeightLayerInfoAt( i ) ;
		ESLAssert( pwmi != nullptr ) ;
		if ( pwmi->nFlags & S3DMeshEditor::weightBone )
		{
			aWeightLayer.Add( i ) ;
		}
	}
	const size_t *	pWeightBoneLayer = aWeightLayer.GetConstArray() ;
	const size_t	nWeightBoneLayers = aWeightLayer.GetLength() ;
	if ( nWeightBoneLayers == 0 )
	{
		return ;
	}
	SArray<size_t>	aSortBuf ;
	size_t *		pSortBuf = aSortBuf.GetArray( nWeightBoneLayers ) ;
	for ( size_t iPatch = 0; iPatch < GetPatchCount(); iPatch ++ )
	{
		S3DMeshEditor::Patch *	pPatch = GetPatchAt( iPatch ) ;
		ESLAssert( pPatch != nullptr ) ;
		ESLAssert( pPatch->GetWeightLayerCount() == nWeightLayerCount ) ;
		const size_t	nVertexCount = pPatch->GetTotalVertexCount() ;
		for ( size_t i = 0; i < nVertexCount; i ++ )
		{
			size_t	nElements = 0 ;
			for ( size_t j = 0; j  < nWeightBoneLayers; j ++ )
			{
				if ( pPatch->GetWeightAt( i, pWeightBoneLayer[j] ) > 0.0f )
				{
					pSortBuf[nElements ++] = pWeightBoneLayer[j] ;
				}
			}
			size_t	nOptElements = (nElements <= 4) ? nElements : 4 ;
			for ( size_t j = 0; j < nElements; j ++ )
			{
				float32_t	wMax = pPatch->GetWeightAt( i, pSortBuf[j] ) ;
				size_t		iMax = j ;
				for ( size_t k = j + 1; k < nElements; k ++ )
				{
					float32_t	w = pPatch->GetWeightAt( i, pSortBuf[k] ) ;
					if ( w > wMax )
					{
						wMax = w ;
						iMax = k ;
					}
				}
				if ( iMax != j )
				{
					size_t	t = pSortBuf[j] ;
					pSortBuf[j] = pSortBuf[iMax] ;
					pSortBuf[iMax] = t ;
				}
			}
			float32_t	wSum = 0.0f ;
			for ( size_t j = 0; j < nOptElements; j ++ )
			{
				wSum += pPatch->GetWeightAt( i, pSortBuf[j] ) ;
			}
			float32_t	wRcp = 1.0f / wSum ;
			for ( size_t j = 0; j < nOptElements; j ++ )
			{
				pPatch->SetWeightAt
					( i, pSortBuf[j],
						pPatch->GetWeightAt( i, pSortBuf[j] ) * wRcp ) ;
			}
			for ( size_t j = nOptElements; j < nElements; j ++ )
			{
				pPatch->SetWeightAt( i, pSortBuf[j], 0.0f ) ;
			}
		}
	}
}

// 選択頂点座標操作
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::TransformPoints
	( const S3DMeshEditor::SelectPointSet& sps,
		const S3DMatrix& matTrans, const S3DVector& vMove )
{
	const SelectPoint *	psp = sps.GetConstArray() ;
	const size_t		nPoints = sps.GetLength() ;
	for ( size_t i = 0; i < nPoints; i ++ )
	{
		SelectPoint	sp = psp[i] ;
		if ( sp.fpWeight <= 0.0f )
		{
			continue ;
		}
		ESLAssert( sp.pPatch != nullptr ) ;
		S3DVector	vOrg = sp.pPatch->GetPointAt( sp.iVertex ) ;
		S3DVector	vTrans = matTrans * vOrg + vMove ;
		if ( sp.fpWeight < 1.0f )
		{
			vTrans = (vTrans - vOrg) * sp.fpWeight + vOrg ;
		}
		sp.pPatch->SetPointAt( sp.iVertex, vTrans ) ;
		sp.pPatch->SetUpdateVertexFlag() ;
	}
	m_flagUpdateVertex = true ;
	m_flagUpdateSerialize = true ;
}

void S3DMeshEditor::TransformUVPoints
	( const S3DMeshEditor::SelectPointSet& sps,
		const S3DMatrix& matTrans, const S3DVector& vMove )
{
	const SelectPoint *	psp = sps.GetConstArray() ;
	const size_t		nPoints = sps.GetLength() ;
	for ( size_t i = 0; i < nPoints; i ++ )
	{
		SelectPoint	sp = psp[i] ;
		if ( sp.fpWeight <= 0.0f )
		{
			continue ;
		}
		ESLAssert( sp.pPatch != nullptr ) ;
		S2DVector	vOrgUV = sp.pPatch->GetUVAt( sp.iVertex ) ;
		S3DVector	vOrg( vOrgUV.x, vOrgUV.y, 0 ) ;
		S3DVector	vTrans = matTrans * vOrg + vMove ;
		if ( sp.fpWeight < 1.0f )
		{
			vTrans = (vTrans - vOrg) * sp.fpWeight + vOrg ;
		}
		sp.pPatch->SetUVAt( sp.iVertex, S2DVector(vTrans.x, vTrans.y) ) ;
		sp.pPatch->SetUpdateVertexFlag() ;
	}
	m_flagUpdateVertex = true ;
	m_flagUpdateSerialize = true ;
}

// 選択頂点から有意な頂点へ変換
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::PatchPointsFromSelectedPoints
	( S3DMeshEditor::PatchPointSet& pps,
		const S3DMeshEditor::SelectPointSet& sps, float32_t wThreshold ) const
{
	pps.RemoveAll() ;
	pps.SetLimit( sps.GetLength() ) ;
	//
	for ( size_t i = 0; i < sps.GetLength(); i ++ )
	{
		const SelectPoint&	sp = sps.At(i) ;
		if ( sp.fpWeight > wThreshold )
		{
			pps.QuickAdd( sp ) ;
		}
	}
	pps.SortArray() ;
}

// 選択頂点へ変換（ソート済み）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::SelectPointsFromPoints
	( S3DMeshEditor::SelectPointSet& sps,
		const S3DMeshEditor::PatchPointSet& pps, const float32_t * pWeights ) const
{
	const size_t		nCount = pps.GetLength() ;
	const PatchPoint *	pppSrc = pps.GetConstArray() ;
	SelectPoint *		pspDst = sps.GetArray( nCount ) ;
	if ( pWeights != nullptr )
	{
		for ( size_t i = 0; i < nCount; i ++ )
		{
			SelectPoint&	sp = pspDst[i] ;
			sp.pPatch = pppSrc[i].pPatch ;
			sp.iVertex = pppSrc[i].iVertex ;
			sp.fpWeight = pWeights[i] ;
		}
	}
	else
	{
		for ( size_t i = 0; i < nCount; i ++ )
		{
			SelectPoint&	sp = pspDst[i] ;
			sp.pPatch = pppSrc[i].pPatch ;
			sp.iVertex = pppSrc[i].iVertex ;
			sp.fpWeight = 1.0f ;
		}
	}
	sps.SetLength( nCount ) ;
	sps.SortArray() ;
	sps.NormalizeSorted() ;
}

void S3DMeshEditor::SelectPointsFromEdges
	( S3DMeshEditor::SelectPointSet& sps, const S3DMeshEditor::PatchEdgeSet& pes ) const
{
	const PatchEdge *	ppe = pes.GetConstArray() ;
	const size_t		nCount = pes.GetLength() ;
	//
	sps.RemoveAll() ;
	sps.SetLimit( nCount * 2 ) ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ESLAssert( ppe[i].pPatch != nullptr ) ;
		sps.QuickAdd( SelectPoint( ppe[i].pPatch, ppe[i].iVertex0, 1.0f ) ) ;
		sps.QuickAdd( SelectPoint( ppe[i].pPatch, ppe[i].iVertex1, 1.0f ) ) ;
	}
	sps.SortArray() ;
	sps.NormalizeSorted() ;
}

void S3DMeshEditor::SelectPointsFromLines
	( S3DMeshEditor::SelectPointSet& sps, const S3DMeshEditor::PatchLineSet& pls ) const
{
	const PatchLine *	ppl = pls.GetConstArray() ;
	const size_t		nCount = pls.GetLength() ;
	//
	sps.RemoveAll() ;
	sps.SetLimit( nCount * 8 ) ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ESLAssert( ppl[i].pPatch != nullptr ) ;
		Patch *	pPatch = ppl[i].pPatch ;
		if ( ppl[i].lineDir == lineHorizontal )
		{
			size_t	nWidth = pPatch->GetWidth() ;
			size_t	iLine = ppl[i].iLine * nWidth ;
			for ( size_t j = 0; j < nWidth; j ++ )
			{
				sps.QuickAdd( SelectPoint( pPatch, iLine + j, 1.0f ) ) ;
			}
		}
		else
		{
			size_t	nWidth = pPatch->GetWidth() ;
			size_t	nHeight = pPatch->GetHeight() ;
			size_t	xLine = ppl[i].iLine ;
			for ( size_t j = 0; j < nHeight; j ++ )
			{
				sps.QuickAdd( SelectPoint( pPatch, j * nWidth + xLine, 1.0f ) ) ;
			}
		}
	}
	sps.SortArray() ;
	sps.NormalizeSorted() ;
}

void S3DMeshEditor::SelectPointsFromFaces
	( S3DMeshEditor::SelectPointSet& sps, const S3DMeshEditor::PatchFaceSet& pfs ) const
{
	const PatchFace *	ppf = pfs.GetConstArray() ;
	const size_t		nCount = pfs.GetLength() ;
	//
	sps.RemoveAll() ;
	sps.SetLimit( nCount * 4 ) ;
	//
	size_t	iFaceQuad[4] ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ESLAssert( ppf[i].pPatch != nullptr ) ;
		Patch *			pPatch = ppf[i].pPatch ;
		const size_t	nAreaSize = pPatch->GetAreaSize() ;
		if ( ppf[i].iFace < nAreaSize )
		{
			pPatch->QuadIndexesFromFaceIndex( iFaceQuad, ppf[i].iFace ) ;
			sps.QuickAdd( SelectPoint( pPatch, iFaceQuad[0], 1.0f ) ) ;
			sps.QuickAdd( SelectPoint( pPatch, iFaceQuad[1], 1.0f ) ) ;
			sps.QuickAdd( SelectPoint( pPatch, iFaceQuad[2], 1.0f ) ) ;
			sps.QuickAdd( SelectPoint( pPatch, iFaceQuad[3], 1.0f ) ) ;
		}
		else
		{
			pPatch->TriangleIndexesFromExFaceIndex
							( iFaceQuad, ppf[i].iFace - nAreaSize ) ;
			sps.QuickAdd( SelectPoint( pPatch, iFaceQuad[0], 1.0f ) ) ;
			sps.QuickAdd( SelectPoint( pPatch, iFaceQuad[1], 1.0f ) ) ;
			sps.QuickAdd( SelectPoint( pPatch, iFaceQuad[2], 1.0f ) ) ;
		}
	}
	sps.SortArray() ;
	sps.NormalizeSorted() ;
}

void S3DMeshEditor::SelectPointsFromPatchs
	( S3DMeshEditor::SelectPointSet& sps,
			const SSystem::SArraySet<S3DMeshEditor::Patch*>& sp ) const
{
	Patch *const*	pp = sp.GetConstArray() ;
	const size_t	nCount = sp.GetLength() ;
	//
	sps.RemoveAll() ;
	sps.SetLimit( nCount * 64 ) ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ESLAssert( pp[i] != nullptr ) ;
		Patch *	pPatch = pp[i] ;
		size_t	nVertexCount = pPatch->GetTotalVertexCount() ;
		for ( size_t j = 0; j < nVertexCount; j ++ )
		{
			sps.QuickAdd( SelectPoint( pPatch, j, 1.0f ) ) ;
		}
	}
	sps.SortArray() ;
	sps.NormalizeSorted() ;
}

void S3DMeshEditor::AllPatchFacesOfPatchs
	( S3DMeshEditor::PatchFaceSet& pfs,
			const SSystem::SArraySet<S3DMeshEditor::Patch*>& sp ) const
{
	Patch *const*	pp = sp.GetConstArray() ;
	const size_t	nCount = sp.GetLength() ;
	//
	pfs.RemoveAll() ;
	pfs.SetLimit( nCount * 64 ) ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ESLAssert( pp[i] != nullptr ) ;
		Patch *	pPatch = pp[i] ;
		size_t	nFaceCount = pPatch->GetTotalFaceCount() ;
		for ( size_t j = 0; j < nFaceCount; j ++ )
		{
			if ( pPatch->IsValidFaceAreaAt( j ) )
			{
				pfs.QuickAdd( PatchFace( pPatch, j ) ) ;
			}
		}
	}
	pfs.SortArray() ;
	pfs.NormalizeSorted() ;
}

// 選択頂点の正規化（縮退頂点がある場合ペアを必ず含む＆ソート済み）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::NormalizeSelectPointSet( S3DMeshEditor::SelectPointSet& sps ) const
{
	//
	// ソートと正規化
	//
	sps.SortArray() ;
	sps.NormalizeSorted() ;
	//
	// パッチ間の縫い目結合
	//
	for ( size_t i = 0; i < m_seams.GetLength(); i ++ )
	{
		const SewPatchPoints *	pspp = m_seams.GetAt( i ) ;
		ESLAssert( pspp != nullptr ) ;
		bool		fIsSeem = false ;
		float32_t	fpSelWeight = 0.0f ;
		for ( size_t j = 0; j < pspp->GetLength(); j ++ )
		{
			const PatchPoint&	pp = pspp->At( j ) ;
			ssize_t	iSel = sps.Find( SelectPoint( pp ) ) ;
			if ( iSel >= 0 )
			{
				fIsSeem = true ;
				fpSelWeight = sps.At( (size_t) iSel ).fpWeight ;
				break ;
			}
		}
		if ( fIsSeem )
		{
			for ( size_t j = 0; j < pspp->GetLength(); j ++ )
			{
				const PatchPoint&	pp = pspp->At( j ) ;
				sps.AddSorted( SelectPoint( pp, fpSelWeight ) ) ;
			}
		}
	}
	//
	// パッチ内の縮退頂点
	//
	SArraySet<Patch*>	aSelPatchs ;
	EnumerateSelectedPatchSet( aSelPatchs, sps ) ;
	//
	for ( size_t i = 0; i < aSelPatchs.GetLength(); i ++ )
	{
		Patch *	pPatch = aSelPatchs.At( i ) ;
		ESLAssert( pPatch != nullptr ) ;
		//
		const DegenerateEntry *
				pde = pPatch->m_degenerates.m_entries.GetConstArray() ;
		const uint32_t *
				pIndexes = pPatch->m_degenerates.m_indexes.GetConstArray() ;
		const size_t
				nEntryCount = pPatch->m_degenerates.m_entries.GetLength() ;
		for ( size_t j = 0; j < nEntryCount; j ++ )
		{
			const DegenerateEntry&	de = pde[j] ;
			ESLAssert( de.iRef < pPatch->m_degenerates.m_indexes.GetLength() ) ;
			ESLAssert( de.iRef + de.nCount <= pPatch->m_degenerates.m_indexes.GetLength() ) ;
			//
			bool		fIsSeem = false ;
			float32_t	fpSelWeight = 0.0f ;
			for ( size_t k = 0; k < de.nCount; k ++ )
			{
				ssize_t	iSel =
					sps.Find( SelectPoint( pPatch, (size_t) pIndexes[de.iRef + k] ) ) ;
				if ( iSel >= 0 )
				{
					fIsSeem = true ;
					fpSelWeight = sps.At( (size_t) iSel ).fpWeight ;
					break ;
				}
			}
			if ( fIsSeem )
			{
				for ( size_t k = 0; k < de.nCount; k ++ )
				{
					sps.AddSorted
						( SelectPoint
							( pPatch, (size_t) pIndexes[de.iRef + k], fpSelWeight ) ) ;
				}
			}
		}
	}
}

// （ソート済み）選択頂点が対象にする Patch を列挙
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::EnumerateSelectedPatchSet
	( SSystem::SArraySet<S3DMeshEditor::Patch*>& aSelPatchs,
			const S3DMeshEditor::SelectPointSet& sps ) const
{
	const SelectPoint *	psp = sps.GetConstArray() ;
	const size_t		nCount = sps.GetLength() ;
	Patch *				pLastPatch = nullptr ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( pLastPatch != psp[i].pPatch )
		{
			pLastPatch = psp[i].pPatch ;
			aSelPatchs.Add( pLastPatch ) ;
		}
	}
}

// （ソート済み）選択頂点から面と稜線へ変換
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::PatchFacesAndEdgesFromPoints
	( S3DMeshEditor::PatchFaceSet& pfs,
		S3DMeshEditor::PatchFaceSet& pfsNull,
		S3DMeshEditor::PatchEdgeSet& pes,
		S3DMeshEditor::PatchLineSet& pls,
		const S3DMeshEditor::SelectPointSet& sps ) const
{
	SArraySet<Patch*>	aSelPatchs ;
	EnumerateSelectedPatchSet( aSelPatchs, sps ) ;
	//
	pfs.RemoveAll() ;
	pfsNull.RemoveAll() ;
	pes.RemoveAll() ;
	pls.RemoveAll() ;
	pfs.SetLimit( pfs.GetLength() + sps.GetLength() ) ;
	pes.SetLimit( pes.GetLength() + sps.GetLength() ) ;
	//
	for ( size_t iPatch = 0; iPatch < aSelPatchs.GetLength(); iPatch ++ )
	{
		Patch *	pPatch = aSelPatchs.At( iPatch ) ;
		ESLAssert( pPatch != nullptr ) ;
		//
		// パッチ面を検査
		//
		const size_t	nPatchWidth = pPatch->GetWidth() ;
		const size_t	nPatchHeight = pPatch->GetHeight() ;
		const size_t	nAreaSize = pPatch->GetAreaSize() ;
		const bool		flagLinePatch = (nPatchWidth == 1) || (nPatchHeight == 1) ;
		for ( size_t iFace = 0; iFace < nAreaSize; iFace ++ )
		{
			if ( !flagLinePatch
				&& !pPatch->IsValidFaceAreaAt( iFace ) )
			{
				continue ;
			}
			SGLPoint	ptLine0, ptLine1 ;
			pPatch->PointFromIndex( ptLine0, (uint32_t) iFace ) ;
			//
			ptLine1.x = ((ptLine0.x + 1) == (int32_t) nPatchWidth) ? 0 : ptLine0.x + 1 ;
			ptLine1.y = ((ptLine0.y + 1) == (int32_t) nPatchHeight) ? 0 : ptLine0.y + 1 ;
			//
			size_t	iQuadIndex[4] ;
			pPatch->QuadIndexesFromFaceIndex( iQuadIndex, iFace ) ;
			//
			size_t	iTemp = iQuadIndex[2] ;
			iQuadIndex[2] = iQuadIndex[3] ;
			iQuadIndex[3] = iTemp ;
			//
			PatchLine	plEdgeLine[4] =
			{
				PatchLine( pPatch, lineHorizontal, (size_t) ptLine0.y ),
				PatchLine( pPatch, lineVertical, (size_t) ptLine1.x ),
				PatchLine( pPatch, lineHorizontal, (size_t) ptLine1.y ),
				PatchLine( pPatch, lineVertical, (size_t) ptLine0.x ),
			} ;
			//
			ssize_t	iSelQuad[4] ;
			size_t	nSelQuads = 0 ;
			for ( int i = 0; i < 4; i ++ )
			{
				iSelQuad[i] =
					sps.FindSorted( SelectPoint( pPatch, iQuadIndex[i] ) ) ;
				if ( iSelQuad[i] >= 0 )
				{
					nSelQuads ++ ;
				}
			}
			for ( int i = 0; i < 4; i ++ )
			{
				if ( (iSelQuad[i] >= 0)
					&& (iSelQuad[(i + 1) % 4] >= 0)
					&& (iQuadIndex[i] != iQuadIndex[(i + 1) % 4]) )
				{
					pes.QuickAdd
						( PatchEdge( pPatch, iQuadIndex[i],
											iQuadIndex[(i + 1) % 4] ) ) ;
					pls.QuickAdd( plEdgeLine[i] ) ;
				}
			}
			if ( (nSelQuads >= 4) && pPatch->IsValidFaceAreaAt( iFace ) )
			{
				if ( pPatch->GetFaceAt( iFace ) == Patch::faceNull )
				{
					pfsNull.AddSorted( PatchFace( pPatch, iFace ) ) ;
				}
				else
				{
					pfs.QuickAdd( PatchFace( pPatch, iFace ) ) ;
				}
			}
		}
		//
		// パッチ外変則ポリゴンを検査
		//
		size_t	nExFaceCount = pPatch->GetExFaceCount() ;
		for ( size_t iPoly = 0; iPoly < nExFaceCount; iPoly ++ )
		{
			size_t	iTriangleIndex[3] ;
			pPatch->TriangleIndexesFromExFaceIndex( iTriangleIndex, iPoly ) ;
			//
			ssize_t	iSelTriangle[3] ;
			size_t	nSelTriangle = 0 ;
			for ( int i = 0; i < 3; i ++ )
			{
				iSelTriangle[i] =
					sps.FindSorted( SelectPoint( pPatch, iTriangleIndex[i] ) ) ;
				if ( iSelTriangle[i] >= 0 )
				{
					nSelTriangle ++ ;
				}
			}
			for ( int i = 0; i < 3; i ++ )
			{
				if ( (iSelTriangle[i] >= 0)
					&& (iSelTriangle[(i + 1) % 3] >= 0) )
				{
					pes.QuickAdd
						( PatchEdge( pPatch, iTriangleIndex[i],
											iTriangleIndex[(i + 1) % 3] ) ) ;
				}
			}
			if ( nSelTriangle >= 3 )
			{
				if ( pPatch->GetFaceAt( iPoly ) == Patch::faceNull )
				{
					pfsNull.AddSorted( PatchFace( pPatch, nAreaSize + iPoly ) ) ;
				}
				else
				{
					pfs.QuickAdd( PatchFace( pPatch, nAreaSize + iPoly ) ) ;
				}
			}
		}
	}
	//
	pfs.SortArray() ;
	pfs.NormalizeSorted() ;
	//
	pes.SortArray() ;
	pes.NormalizeSorted() ;
	//
	pls.SortArray() ;
	pls.NormalizeSorted() ;
	//
	for ( size_t i = 0; i < pls.GetLength(); i ++ )
	{
		PatchLine	pl = pls.At(i) ;
		if ( pl.lineDir == lineHorizontal )
		{
			const size_t	nWidth = pl.pPatch->GetWidth() ;
			for ( size_t x = 0; x < nWidth; x ++ )
			{
				if ( sps.FindSorted
					( SelectPoint( pl.pPatch,
						pl.pPatch->IndexFromPoint( x, pl.iLine ) ) ) < 0 )
				{
					pls.RemoveAt( i -- ) ;
					break ;
				}
			}
		}
		else
		{
			const size_t	nHeight = pl.pPatch->GetHeight() ;
			for ( size_t y = 0; y < nHeight; y ++ )
			{
				if ( sps.FindSorted
					( SelectPoint( pl.pPatch,
						pl.pPatch->IndexFromPoint( pl.iLine, y ) ) ) < 0 )
				{
					pls.RemoveAt( i -- ) ;
					break ;
				}
			}
		}
	}
}

// （ソート済み）縮退頂点を1つだけ選択して除去
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::DenormalizeSelectPoints( S3DMeshEditor::SelectPointSet& sps ) const
{
	PatchPointSet	ppsDeg ;
	for ( size_t i = 0; i < sps.GetLength(); i ++ )
	{
		if ( GetDegeneratedPoints( ppsDeg, sps.At(i) ) )
		{
			PatchPoint	ppLeft = sps.At(i) ;
			for ( size_t j = 0; j < ppsDeg.GetLength(); j ++ )
			{
				PatchPoint	ppDeg = ppsDeg.At(j) ;
				if ( ppLeft == ppDeg )
				{
					continue ;
				}
				ssize_t	k = sps.FindSorted( ppDeg ) ;
				if ( k >= 0 )
				{
					ESLAssert( (size_t) k > i ) ;
					sps.RemoveAt( (size_t) k ) ;
				}
			}
			ppsDeg.RemoveAll() ;
		}
	}
}

// （ソート済み）選択頂点情報のパッチをインデックスに変換してソート
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::IndexPatchForSelectPoints( S3DMeshEditor::SelectPointSet& sps ) const
{
	Patch *	pPatch = nullptr ;
	ssize_t	iPatch = -1 ;
	for ( size_t i = 0; i < sps.GetLength(); i ++ )
	{
		SelectPoint&	sp = sps.At(i) ;
		if ( sp.pPatch != pPatch )
		{
			pPatch = sp.pPatch ;
			iPatch = FindPatch( pPatch ) ;
		}
		sp.pPatch = (Patch*) ((ulong_ptr_t) iPatch) ;
	}
	sps.SortArray() ;
}

// （ソート済み）選択頂点情報のインデックス化されたパッチをポインタに変換してソート
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::PointerPatchForSelectPoints( S3DMeshEditor::SelectPointSet& sps ) const
{
	for ( size_t i = 0; i < sps.GetLength(); i ++ )
	{
		SelectPoint&	sp = sps.At(i) ;
		sp.pPatch = GetPatchAt( (size_t) ((ulong_ptr_t) sp.pPatch) ) ;
	}
	sps.SortArray() ;
}

// （ソート済み）面集合のパッチをインデックスに変換してソート
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::IndexPatchForPatchFaceSet( S3DMeshEditor::PatchFaceSet& pfs ) const
{
	Patch *	pPatch = nullptr ;
	ssize_t	iPatch = -1 ;
	for ( size_t i = 0; i < pfs.GetLength(); i ++ )
	{
		PatchFace&	pf = pfs.At(i) ;
		if ( pf.pPatch != pPatch )
		{
			pPatch = pf.pPatch ;
			iPatch = FindPatch( pPatch ) ;
		}
		pf.pPatch = (Patch*) ((ulong_ptr_t) iPatch) ;
	}
	pfs.SortArray() ;
}

// （ソート済み）面集合のインデックス化されたパッチをポインタに変換してソート
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::PointerPatchForPatchFaceSet( S3DMeshEditor::PatchFaceSet& pfs ) const
{
	for ( size_t i = 0; i < pfs.GetLength(); i ++ )
	{
		PatchFace&	pf = pfs.At(i) ;
		pf.pPatch = GetPatchAt( (size_t) ((ulong_ptr_t) pf.pPatch) ) ;
	}
	pfs.SortArray() ;
}

// 含まれないパッチを削除する
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::RemoveInvalidPatchOfSelectPoints( S3DMeshEditor::SelectPointSet& sps ) const
{
	Patch *	pPatch = nullptr ;
	ssize_t	iPatch = -1 ;
	for ( size_t i = 0; i < sps.GetLength(); i ++ )
	{
		SelectPoint&	sp = sps.At(i) ;
		if ( sp.pPatch != pPatch )
		{
			pPatch = sp.pPatch ;
			iPatch = FindPatch( pPatch ) ;
		}
		if ( iPatch < 0 )
		{
			sps.RemoveAt( i -- ) ;
		}
	}
}

void S3DMeshEditor::RemoveInvalidPatchOfFaceSet( S3DMeshEditor::PatchFaceSet& pfs ) const
{
	Patch *	pPatch = nullptr ;
	ssize_t	iPatch = -1 ;
	for ( size_t i = 0; i < pfs.GetLength(); i ++ )
	{
		PatchFace&	pf = pfs.At(i) ;
		if ( pf.pPatch != pPatch )
		{
			pPatch = pf.pPatch ;
			iPatch = FindPatch( pPatch ) ;
		}
		if ( iPatch < 0 )
		{
			pfs.RemoveAt( i -- ) ;
		}
	}
}

void S3DMeshEditor::RemoveInvalidPatchOfEdgeSet( S3DMeshEditor::PatchEdgeSet& pes ) const
{
	Patch *	pPatch = nullptr ;
	ssize_t	iPatch = -1 ;
	for ( size_t i = 0; i < pes.GetLength(); i ++ )
	{
		PatchEdge&	pe = pes.At(i) ;
		if ( pe.pPatch != pPatch )
		{
			pPatch = pe.pPatch ;
			iPatch = FindPatch( pPatch ) ;
		}
		if ( iPatch < 0 )
		{
			pes.RemoveAt( i -- ) ;
		}
	}
}

// 稜線からラインへ変換
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditor::PatchLineFromEdge
	( S3DMeshEditor::PatchLine& line, const PatchEdge& edge ) const
{
	Patch *	pPatch = edge.pPatch ;
	ESLAssert( pPatch != nullptr ) ;
	//
	const size_t	nAreaSize = pPatch->GetAreaSize() ;
	if ( (edge.iVertex0 >= nAreaSize)
		|| (edge.iVertex1 >= nAreaSize) )
	{
		return	false ;
	}
	SGLPoint	pt0, pt1 ;
	pPatch->PointFromIndex( pt0, (uint32_t) edge.iVertex0 ) ;
	pPatch->PointFromIndex( pt1, (uint32_t) edge.iVertex1 ) ;
	//
	if ( pt0.x == pt1.x )
	{
		line.pPatch = pPatch ;
		line.lineDir = lineVertical ;
		line.iLine = (size_t) pt0.x ;
		return	true ;
	}
	else if ( pt0.y == pt1.y )
	{
		line.pPatch = pPatch ;
		line.lineDir = lineHorizontal ;
		line.iLine = (size_t) pt0.y ;
		return	true ;
	}
	return	false ;
}

// パッチの全ての有効面を取得
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::EnumeratePatchValidFaces
	( S3DMeshEditor::PatchFaceSet& pfs, const S3DMeshEditor::Patch& patch ) const
{
	size_t	nFaceCount = patch.GetTotalFaceCount() ;
	for ( size_t i = 0; i < nFaceCount; i ++ )
	{
		if ( (patch.GetFaceAt( i ) != Patch::faceNull)
			&& patch.IsValidFaceAreaAt( i ) )
		{
			pfs.QuickAdd( PatchFace( (Patch*) &patch, i ) ) ;
		}
	}
	pfs.SortArray() ;
	pfs.NormalizeSorted() ;
}

// インデックス化された選択点の情報をラインストリップへ
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditor::Patch * S3DMeshEditor::NewPatchFromIndexedSelectPoints
					( const S3DMeshEditor::SelectPointSet& spsIndexed )
{
	Patch *	pNewPatch = NewPatch() ;
	pNewPatch->CreatePatch( spsIndexed.GetLength(), 1 ) ;
	//
	const size_t	nWeightLayers = pNewPatch->GetWeightLayerCount() ;
	for ( size_t i = 0; i < spsIndexed.GetLength(); i ++ )
	{
		SelectPoint	sp = spsIndexed.At(i) ;
		Patch *	pSelPatch = GetPatchAt( (size_t) ((ulong_ptr_t) sp.pPatch) ) ;
		ESLAssert( pSelPatch != nullptr ) ;
		if ( pSelPatch != nullptr )
		{
			pNewPatch->SetPointAt( i, pSelPatch->GetPointAt( sp.iVertex ) ) ;
			pNewPatch->SetNormalAt( i, pSelPatch->GetNormalAt( sp.iVertex ) ) ;
			pNewPatch->SetUVAt( i, pSelPatch->GetUVAt( sp.iVertex ) ) ;
			pNewPatch->SetColorAt( i, pSelPatch->GetColorAt( sp.iVertex ) ) ;
			//
			ESLAssert( pSelPatch->GetWeightLayerCount() == nWeightLayers ) ;
			for ( size_t j = 0; j < nWeightLayers; j ++ )
			{
				pNewPatch->SetWeightAt
					( i, j, pSelPatch->GetWeightAt( sp.iVertex, j ) ) ;
			}
		}
	}
	return	pNewPatch ;
}

// インデックス化された選択点へパッチの頂点情報をペースト
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::PasteVertexByIndexedSelectPoints
	( const S3DMeshEditor::SelectPointSet& spsIndexed,
		const S3DMeshEditor::Patch& patch,
		uint32_t nElementFlags, bool flagReverse )
{
	const size_t	nSrcVertexCount = patch.GetTotalVertexCount() ;
	const size_t	nWeightLayers = patch.GetWeightLayerCount() ;
	PatchPointSet	ppsDeg ;
	for ( size_t i = 0; i < spsIndexed.GetLength(); i ++ )
	{
		if ( i >= nSrcVertexCount )
		{
			break ;
		}
		size_t	iSrc = i ;
		if ( flagReverse )
		{
			iSrc = spsIndexed.GetLength() - i - 1 ;
		}
		SelectPoint	sp = spsIndexed.At(iSrc) ;
		sp.pPatch = GetPatchAt( (size_t) ((ulong_ptr_t) sp.pPatch) ) ;
		ESLAssert( sp.pPatch != nullptr ) ;
		if ( sp.pPatch == nullptr )
		{
			continue ;
		}
		PatchPoint	ppSrc( (Patch*) &patch, i ) ;
		if ( GetDegeneratedPoints( ppsDeg, sp ) )
		{
			for ( size_t j = 0; j < ppsDeg.GetLength(); j ++ )
			{
				PasteVertexElements( ppsDeg.At(j), ppSrc, nElementFlags ) ;
			}
			ppsDeg.RemoveAll() ;
		}
		else
		{
			PasteVertexElements( sp, ppSrc, nElementFlags ) ;
		}
		sp.pPatch->SetUpdateVertexFlag() ;
	}
	SetUpdateVertexFlag() ;
}

void S3DMeshEditor::PasteVertexElements
	( const S3DMeshEditor::PatchPoint& ppDst,
		const S3DMeshEditor::PatchPoint& ppSrc, uint32_t nElementFlags )
{
	if ( nElementFlags & vertexElementPoint )
	{
		ppDst.pPatch->SetPointAt
			( ppDst.iVertex, ppSrc.pPatch->GetPointAt( ppSrc.iVertex ) ) ;
	}
	if ( nElementFlags & vertexElementUV )
	{
		ppDst.pPatch->SetUVAt
			( ppDst.iVertex, ppSrc.pPatch->GetUVAt( ppSrc.iVertex ) ) ;
	}
	if ( nElementFlags & vertexElementColor )
	{
		ppDst.pPatch->SetColorAt
			( ppDst.iVertex, ppSrc.pPatch->GetColorAt( ppSrc.iVertex ) ) ;
	}
	if ( nElementFlags & vertexElementWeight )
	{
		const size_t	nWeightLayers =
			(size_t) esl_min( (int) ppDst.pPatch->GetWeightLayerCount(),
						(int) ppSrc.pPatch->GetWeightLayerCount() ) ;
		for ( size_t i = 0; i < nWeightLayers; i ++ )
		{
			ppDst.pPatch->SetWeightAt
				( ppDst.iVertex, i,
					ppSrc.pPatch->GetWeightAt( ppSrc.iVertex, i ) ) ;
		}
	}
}

// 頂点要素をテキスト（XML）形式にエクスポートする
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::ExportVertexElements
	( SSystem::SXMLDocument& xmlDoc, const S3DMeshEditor::Patch& patch ) const
{
	const size_t	nVertexCount = patch.GetTotalVertexCount() ;
	const size_t	nWeightLayerCount = patch.GetWeightLayerCount() ;
	xmlDoc.SetTag( L"vertex_elements" ) ;
	xmlDoc.SetAttrIntegerAs( L"count", nVertexCount ) ;
	xmlDoc.SetAttrIntegerAs( L"weight_layer", nWeightLayerCount ) ;
	//
	SString	strTemp ;
	for ( size_t i = 0; i < nVertexCount; i ++ )
	{
		SXMLDocument *	pElement = new SXMLDocument ;
		pElement->SetTag( L"vertex" ) ;
		pElement->SetAttrIntegerAs( L"index", i ) ;
		//
		S3DVector	vPoint = patch.GetPointAt( i ) ;
		strTemp.Format( L"%f,%f,%f", vPoint.x, vPoint.y, vPoint.z ) ;
		pElement->SetAttributeAs( L"point", strTemp ) ;
		//
		S2DVector	vUV = patch.GetUVAt( i ) ;
		strTemp.Format( L"%f,%f", vUV.x, vUV.y ) ;
		pElement->SetAttributeAs( L"uv", strTemp ) ;
		//
		S3DColor	vColor = patch.GetColorAt( i ) ;
		strTemp.Format
			( L"%06X,%06X,%02X",
					(vColor.rgbMul.ui32 & 0x00FFFFFF),
					vColor.rgbAdd.ui32, vColor.rgbMul.argb.Alpha ) ;
		pElement->SetAttributeAs( L"color", strTemp ) ;
		//
		if ( nWeightLayerCount > 0 )
		{
			strTemp = L"" ;
			for ( size_t j = 0; j < nWeightLayerCount; j ++ )
			{
				if ( j > 0 )
				{
					strTemp += L',' ;
				}
				strTemp += SString( (double) patch.GetWeightAt( i, j ), 5 ) ;
			}
			pElement->SetAttributeAs( L"weight", strTemp ) ;
		}
		//
		xmlDoc.AddElement( pElement ) ;
	}
}

// 頂点要素をテキスト（XML）形式にインポートする
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditor::Patch *
	S3DMeshEditor::NewImportVertexElements( const SSystem::SXMLDocument& xmlDoc )
{
	const size_t	nVertexCount =
						(size_t) xmlDoc.GetAttrIntegerAs( L"count", 0 ) ;
	const size_t	nWeightLayerCount =
						(size_t) xmlDoc.GetAttrIntegerAs( L"weight_layer", 0 ) ;
	//
	Patch *	pNewPatch = NewPatch() ;
	pNewPatch->CreatePatch( nVertexCount, 1 ) ;
	//
	const size_t	nDstWeightCount = pNewPatch->GetWeightLayerCount() ;
	SStringParser	sparsTemp ;
	double			fpValues[3] ;
	int64_t			nHexValues[3] ;
	for ( size_t iTag = 0; iTag < xmlDoc.GetElementsCount(); iTag ++ )
	{
		const SXMLDocument *	pElement = xmlDoc.GetElementAt( iTag ) ;
		if ( (pElement == nullptr)
			|| (pElement->GetTag() != L"vertex") )
		{
			continue ;
		}
		size_t	iVertex =
				(size_t) pElement->GetAttrIntegerAs( L"index", iTag ) ;
		if ( iVertex >= nVertexCount )
		{
			continue ;
		}
		const SString *	pstrPoint =pElement->GetAttributeAs( L"point" ) ;
		if ( pstrPoint != nullptr )
		{
			sparsTemp.AttachString( *pstrPoint ) ;
			if ( sparsTemp.ParseNumberArray( fpValues, 3, 0, L"," ) == 3 )
			{
				pNewPatch->SetPointAt
					( iVertex,
						S3DVector( fpValues[0], fpValues[1], fpValues[2] ) ) ;
			}
		}
		const SString *	pstrUV =pElement->GetAttributeAs( L"uv" ) ;
		if ( pstrUV != nullptr )
		{
			sparsTemp.AttachString( *pstrUV ) ;
			if ( sparsTemp.ParseNumberArray( fpValues, 2, 0, L"," ) == 2 )
			{
				pNewPatch->SetUVAt
					( iVertex, S2DVector( fpValues[0], fpValues[1] ) ) ;
			}
		}
		const SString *	pstrColor =pElement->GetAttributeAs( L"color" ) ;
		if ( pstrColor != nullptr )
		{
			sparsTemp.AttachString( *pstrColor ) ;
			if ( sparsTemp.ParseHexIntegerArray( nHexValues, 3, L"," ) == 3 )
			{
				S3DColor	clrTemp ;
				clrTemp.rgbMul.ui32 = (uint32_t) (nHexValues[0] & 0x00FFFFFF) ;
				clrTemp.rgbAdd.ui32 = (uint32_t) (nHexValues[1] & 0x00FFFFFF) ;
				clrTemp.rgbMul.argb.Alpha = (uint8_t) nHexValues[2] ;
				//
				pNewPatch->SetColorAt( iVertex, clrTemp ) ;
			}
		}
		const SString *	pstrWeight =pElement->GetAttributeAs( L"weight" ) ;
		if ( pstrWeight != nullptr )
		{
			sparsTemp.AttachString( *pstrWeight ) ;
			for ( size_t j = 0; j < nDstWeightCount; j ++ )
			{
				if ( !sparsTemp.PassSpace() || !sparsTemp.IsNextNumber() )
				{
					break;
				}
				pNewPatch->SetWeightAt
					( iVertex, j, (float32_t) sparsTemp.NextRealNumber() ) ;
				if ( sparsTemp.HasToComeChar( L"," ) != L',' )
				{
					break ;
				}
			}
		}
	}
	return	pNewPatch ;
}

// メッシュを細分化複製
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::RedivideMeshFrom
	( const S3DMeshEditor& mesh, const S3DMeshEditor::MeshParam& param )
{
	//
	// 初期化
	//
	ClearAllMeshs() ;
	//
	m_patchs.SetLimit( mesh.m_patchs.GetLength() ) ;
	m_aWeightLayerIDs = mesh.m_aWeightLayerIDs ;
	m_aWeightLayerInfos = mesh.m_aWeightLayerInfos ;
	//
	// 各パッチを細分化
	//
	SPtrSortArray<Patch,Patch*>	psaPatchMap ;
	SObjectArray<SeemEdges>		aSeemEdges ;
	EdgeDivRefMap				edrmRefMap ;
	for ( size_t i = 0; i < mesh.m_patchs.GetLength(); i ++ )
	{
		Patch *	pSrcPatch = mesh.m_patchs.GetAt( i ) ;
		ESLAssert( pSrcPatch != nullptr ) ;
		//
		size_t	nDivWidth, nDivHeight ;
		CalcRedividedPatchSize
			( nDivWidth, nDivHeight,
				*pSrcPatch, param.nDivHorz, param.nDivVert ) ;
		//
		SeemEdges *	pSeemEdges = new SeemEdges ;
		mesh.GetPatchSeemEdges( *pSeemEdges, *pSrcPatch ) ;
		aSeemEdges.Add( pSeemEdges ) ;
		//
		SArray<EdgeDivRef> *	pSeemDivEdge = new SArray<EdgeDivRef> ;
		//
		Patch *	pPatch = NewRedividedPatch
			( nDivWidth, nDivHeight,
				*pSeemDivEdge, *pSrcPatch, *pSeemEdges,
				(DivInterpolationMethod) param.nDivMethod ) ;
		m_patchs.Add( pPatch ) ;
		//
		edrmRefMap.SetAs( pPatch, pSeemDivEdge ) ;
		psaPatchMap.SetAs( pSrcPatch, pPatch ) ;
	}
	//
	// 縮退点を再構築
	//
	RemapAllPatchPointers( aSeemEdges, psaPatchMap ) ;
	for ( size_t i = 0; i < m_patchs.GetLength(); i ++ )
	{
		Patch *	pPatch = m_patchs.GetAt( i ) ;
		ESLAssert( pPatch != nullptr ) ;
		//
		SeemEdges *	pSeemEdges = aSeemEdges.GetAt( i ) ;
		ESLAssert( pSeemEdges != nullptr ) ;
		//
		DegenerateRedividedPatch( pPatch, *pSeemEdges, edrmRefMap ) ;
	}
	#if	defined(__DEBUG__)
		VerifyDegenerate() ;
	#endif
	m_flagUpdateVertex = true ;
	//
	UpdateAllPatchs( param ) ;
}

// メッシュ稜線のパッチ間の縮退情報取得
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::GetPatchSeemEdges
	( S3DMeshEditor::SeemEdges& sedges, const S3DMeshEditor::Patch& patch ) const
{
	const size_t	nAreaSize = patch.GetAreaSize() ;
	const size_t	nWidth = patch.GetWidth() ;
	const size_t	nHeight = patch.GetHeight() ;
	const uint32_t	nFlags = patch.GetFlags() ;
	//
	PatchPointSet	pps0 ;
	PatchPointSet	ppsTemp ;
	//
	for ( size_t i = 0; i < nAreaSize; i ++ )
	{
		PatchPoint	pp0( (Patch*) &patch, i ) ;
		if ( m_seams.FindPoint( pp0 ) < 0 )
		{
			continue ;
		}
		pps0.RemoveAll() ;
		if ( !GetDegeneratedPoints( pps0, pp0 ) )
		{
			continue ;
		}
		SGLPoint	pt0 ;
		patch.PointFromIndex( pt0, (uint32_t) i ) ;
		//
		size_t		vi[2] ;
		size_t		n = 0 ;
		SGLPoint	pt1 = pt0 + SGLPoint( 1, 0 ) ;
		SGLPoint	pt2 = pt0 + SGLPoint( 0, 1 ) ;
		if ( ((size_t) pt1.x < nWidth) || (nFlags & Patch::flagHorzLoop) )
		{
			vi[n ++] = (size_t) patch.IndexFromPoint
								( (size_t) pt1.x % nWidth, (size_t) pt1.y ) ;
		}
		if ( ((size_t) pt2.y < nHeight) || (nFlags & Patch::flagVertLoop) )
		{
			vi[n ++] = (size_t) patch.IndexFromPoint
								( (size_t) pt2.x, (size_t) pt2.y % nHeight ) ;
		}
		for ( size_t j = 0; j < n; j ++ )
		{
			PatchPoint	ppj( (Patch*) &patch, vi[j] ) ;
			ppsTemp.RemoveAll() ;
			if ( !GetDegeneratedPoints( ppsTemp, ppj ) )
			{
				continue ;
			}
			Edge	edge( i, vi[j] ) ;
			ESLAssert( patch.IsPatchEdge( i, vi[j] ) ) ;
			//
			PatchEdgeSetSeem *	ppes = sedges.GetAs( edge ) ;
			ESLAssert( ppes == nullptr ) ;
			for ( size_t k0 = 0; k0 < pps0.GetLength(); k0 ++ )
			{
				PatchPoint	pp0 = pps0.At( k0 ) ;
				for ( size_t k1 = 0; k1 < ppsTemp.GetLength(); k1 ++ )
				{
					PatchPoint	pp1 = ppsTemp.At( k1 ) ;
					if ( (pp0.pPatch == pp1.pPatch)
						&& pp0.pPatch->IsPatchEdge( pp0.iVertex, pp1.iVertex ) )
					{
						if ( ppes == nullptr )
						{
							ppes = new PatchEdgeSetSeem ;
							ppes->m_nFlags = GetDegeneratedPointFlag( pps0 ) ;
							sedges.Add( edge, ppes ) ;
						}
						ppes->AddSorted
							( PatchEdge( pp0.pPatch, pp0.iVertex, pp1.iVertex ) ) ;
					}
				}
			}
		}
	}
}

void S3DMeshEditor::GetAllPatchSeemEdges
	( SPtrSortObjectArray<S3DMeshEditor::Patch,S3DMeshEditor::SeemEdges>& psoaEdges ) const
{
	for ( size_t i = 0; i < GetPatchCount(); i ++ )
	{
		Patch *	pPatch = GetPatchAt( i ) ;
		ESLAssert( pPatch != nullptr ) ;
		//
		SeemEdges *	pEdges = new SeemEdges ;
		GetPatchSeemEdges( *pEdges, *pPatch ) ;
		//
		psoaEdges.SetAs( pPatch, pEdges ) ;
	}
}

// メッシュ分割サイズ計算
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::CalcRedividedPatchSize
	( size_t& nDivWidth, size_t& nDivHeight,
		const S3DMeshEditor::Patch& patch, size_t nDivHorz, size_t nDivVert ) const
{
	nDivWidth = patch.GetWidth() ;
	if ( (nDivWidth >= 2) && (nDivHorz >= 2) )
	{
		if ( patch.GetFlags() & Patch::flagHorzLoop )
		{
			nDivWidth *= nDivHorz ;
		}
		else
		{
			nDivWidth = (nDivWidth - 1) * nDivHorz + 1 ;
		}
	}
	nDivHeight = patch.GetHeight() ;
	if ( (nDivHeight >= 2) && (nDivVert >= 2) )
	{
		if ( patch.GetFlags() & Patch::flagVertLoop )
		{
			nDivHeight *= nDivVert ;
		}
		else
		{
			nDivHeight = (nDivHeight - 1) * nDivVert + 1 ;
		}
	}
}

// メッシュ稜線補完処理
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::EdgeBezierInterpolation
	( S3DMeshEditor::Patch::Elements& el0,
		const S3DMeshEditor::Patch::Elements& el1, double t )
{
	S3DVector	vDelta = el1.pos - el0.pos ;
	float32_t	fpLen = (float32_t) vDelta.Absolute() ;
	if ( fpLen < 1.0e-7 )
	{
		Patch::LerpElements( el0, el1, (float32_t) t ) ;
		return ;
	}
	S3DVector	vDir = vDelta * (1.0f / fpLen) ;
	S3DVector	vCross0 = vDir * el0.normal ;
	S3DVector	vCross1 = vDir * el1.normal ;
	//
	float32_t	fpHandleLen = fpLen * (1.0f / 3.0f) ;
	S3DVector	vHandle0 = el0.pos + (el0.normal * vCross0).Normalized() * fpHandleLen ;
	S3DVector	vHandle1 = el1.pos - (el1.normal * vCross1).Normalized() * fpHandleLen ;
	//
	double		nt = 1.0f - t ;
	S3DVector	vPos = el0.pos * (nt * nt * nt)
						+ vHandle0 * (3.0 * t * nt * nt)
						+ vHandle1 * (3.0 * t * t * nt)
						+ el1.pos * (t * t * t) ;
	//
	Patch::LerpElements( el0, el1, (float32_t) t ) ;
	el0.pos = vPos ;
}

// メッシュ細分化処理
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditor::Patch * S3DMeshEditor::NewRedividedPatch
	( size_t nDivWidth, size_t nDivHeight,
		SArray<S3DMeshEditor::EdgeDivRef>& aSeemDivEdge,
		const S3DMeshEditor::Patch& patch,
		const S3DMeshEditor::SeemEdges& sedges,
		S3DMeshEditor::DivInterpolationMethod interpolation )
{
	const size_t	nSrcWidth = patch.GetWidth() ;
	const size_t	nSrcHeight = patch.GetHeight() ;
	const uint32_t	nSrcFlags = patch.GetFlags() ;
	Patch *			pPatch = NewPatch() ;
	if ( (nSrcWidth == nDivWidth) && (nSrcHeight == nDivHeight) )
	{
		pPatch->CopyFrom( patch ) ;
		return	pPatch ;
	}
	ESLAssert( nSrcWidth >= 1 ) ;
	ESLAssert( nSrcHeight >= 1 ) ;
	ESLAssert( nDivWidth >= 1 ) ;
	ESLAssert( nDivHeight >= 1 ) ;
	pPatch->CreatePatch( nDivWidth, nDivHeight ) ;
	pPatch->SetFlags( nSrcFlags ) ;
	//
	// エレメント用バッファ準備
	//
	SArray<float32_t>	bufWeights ;
	S3DMeshEditor::Patch::Elements	el00, el01, el10, el11 ;
	el00.nWeights = patch.GetWeightLayerCount() ;
	el01.nWeights = el00.nWeights ;
	el10.nWeights = el00.nWeights ;
	el11.nWeights = el00.nWeights ;
	el00.pWeights = bufWeights.GetArray( el00.nWeights * 4 ) ;
	el01.pWeights = el00.pWeights + el00.nWeights ;
	el10.pWeights = el01.pWeights + el01.nWeights ;
	el11.pWeights = el10.pWeights + el10.nWeights ;
	//
	// パッチ以外の頂点要素複製
	//
	const size_t	nSrcAreaSize = patch.GetAreaSize() ;
	const size_t	nDstAreaSize = pPatch->GetAreaSize() ;
	const size_t	nExVertexCount = patch.GetExVertexCount() ;
	const size_t	nExFaceCount = patch.GetExFaceCount() ;
	//
	DegenerateCollection&	degenerate = pPatch->GetDegenerateCollection() ;
	degenerate = patch.GetDegenerateCollection() ;
	//
	if ( nExVertexCount > 0 )
	{
		pPatch->InsertExVertics
			( 0, patch.GetConstPointArray
					( nSrcAreaSize, nExVertexCount ), nExVertexCount ) ;
		//
		for ( size_t i = 0; i < nExVertexCount; i ++ )
		{
			patch.GetElementsAt( el00, nSrcAreaSize + i ) ;
			pPatch->SetElementsAt( nDstAreaSize + i, el00 ) ;
			//
			pPatch->SetDegenerateNumberAt
				( nDstAreaSize + i, patch.GetDegenerateNumberAt( i ) ) ;
		}
	}
	if ( nExFaceCount > 0 )
	{
		for ( size_t i = 0; i < nExFaceCount; i ++ )
		{
			pPatch->InsertExFace
				( i, patch.GetFaceAt( nSrcAreaSize + i ),
					patch.GetExFaceTriangleIndexes( i, 1 ) ) ;
		}
	}
	pPatch->RebuildDegenerateByNumber() ;
	//
	// 各頂点計算
	//
	const size_t	nVertNoLoop =
		(nSrcFlags & Patch::flagVertLoop) || (nDivHeight <= 1) ? 0 : 1 ;
	const size_t	nHorzNoLoop =
		(nSrcFlags & Patch::flagHorzLoop) || (nDivWidth <= 1) ? 0 : 1 ;
	for ( size_t y = 0; y < nDivHeight; y ++ )
	{
		size_t	sy0 = y * (nSrcHeight - nVertNoLoop) / (nDivHeight - nVertNoLoop) ;
		size_t	sy1, syl, syn ;
		syn = (sy0 + 1) % nSrcHeight ;
		if ( nSrcFlags & Patch::flagVertLoop )
		{
			sy1 = syn ;
			syl = (sy0 + nSrcHeight - 1) % nSrcHeight ;
		}
		else
		{
			sy1 = (size_t) esl_min( (int) sy0 + 1, (int) nSrcHeight - 1 ) ;
			syl = (size_t) esl_max( (int) sy0 - 1, 0 ) ;
		}
		double	dy0 = (double) y * (nSrcHeight - nVertNoLoop)
									/ (nDivHeight - nVertNoLoop) - sy0 ;
		bool	evenLine = (dy0 < 1.0e-7) ;
		ESLAssert( dy0 < 1.000001 ) ;
		//
		for ( size_t x = 0; x < nDivWidth; x ++ )
		{
			size_t	sx0 = x * (nSrcWidth - nHorzNoLoop) / (nDivWidth - nHorzNoLoop) ;
			size_t	sx1, sxl, sxn ;
			sxn = (sx0 + 1) % nSrcWidth ;
			if ( nSrcFlags & Patch::flagHorzLoop )
			{
				sx1 = sxn ;
				sxl = (sx0 + nSrcWidth - 1) % nSrcWidth ;
			}
			else
			{
				sx1 = (size_t) esl_min( (int) sx0 + 1, (int) nSrcWidth - 1 ) ;
				sxl = (size_t) esl_max( (int) sx0 - 1, 0 ) ;
			}
			double	dx0 = (double) x * (nSrcWidth - nHorzNoLoop)
										/ (nDivWidth - nHorzNoLoop) - sx0 ;
			bool	evenCol = (dx0 < 1.0e-7) ;
			ESLAssert( dx0 < 1.000001 ) ;
			//
			uint32_t	nDeg00 = patch.GetDegenerateNumber( sx0, sy0 ) ;
			uint32_t	nDeg01 = patch.GetDegenerateNumber( sxn, sy0 ) ;
			uint32_t	nDeg10 = patch.GetDegenerateNumber( sx0, syn ) ;
			uint32_t	nDeg11 = patch.GetDegenerateNumber( sxn, syn ) ;
			uint32_t	nDeg = 0 ;
			//
			int8_t		face = patch.GetFace( sx0, sy0 ) ;
			patch.GetElements( el00, sx0, sy0 ) ;
			patch.GetElements( el01, sx1, sy0 ) ;
			if ( interpolation == divMethodBezier )
			{
				EdgeBezierInterpolation( el00, el01, dx0 ) ;
			}
			else
			{
				Patch::LerpElements( el00, el01, (float32_t) dx0 ) ;
			}
			//
			if ( evenLine )
			{
				Edge	edHorz( patch.IndexFromPoint( sx0, sy0 ),
								patch.IndexFromPoint( sx1, sy0 ) ) ;
				if ( (sx0 != sx1)
					&& (sedges.GetAs( edHorz ) != nullptr) )
				{
					aSeemDivEdge.Add
						( EdgeDivRef( EdgeDiv
							( patch.IndexFromPoint( sx0, sy0 ),
								patch.IndexFromPoint( sx1, sy0 ), (float32_t) dx0 ),
							pPatch->IndexFromPoint( x, y ) ) ) ;
				}
				if ( (nDeg00 == nDeg01) || evenCol )
				{
					nDeg = nDeg00 ;
				}
			}
			else
			{
				patch.GetElements( el10, sx0, sy1 ) ;
				patch.GetElements( el11, sx1, sy1 ) ;
				if ( interpolation == divMethodBezier )
				{
					EdgeBezierInterpolation( el10, el11, dx0 ) ;
					EdgeBezierInterpolation( el00, el10, dy0 ) ;
				}
				else
				{
					Patch::LerpElements( el10, el11, (float32_t) dx0 ) ;
					Patch::LerpElements( el00, el10, (float32_t) dy0 ) ;
				}
			}
			if ( evenCol )
			{
				Edge	edVert( patch.IndexFromPoint( sx0, sy0 ),
								patch.IndexFromPoint( sx0, sy1 ) ) ;
				if ( (sy0 != sy1)
					&& (sedges.GetAs( edVert ) != nullptr) )
				{
					aSeemDivEdge.Add
						( EdgeDivRef( EdgeDiv
							( patch.IndexFromPoint( sx0, sy0 ),
								patch.IndexFromPoint( sx0, sy1 ), (float32_t) dy0 ),
							pPatch->IndexFromPoint( x, y ) ) ) ;
				}
				if ( nDeg00 == nDeg10 )
				{
					nDeg = nDeg00 ;
				}
			}
			if ( (nDeg == 0) && (nDeg00 != 0) )
			{
				if ( (nDeg00 == nDeg01)
					&& (nDeg00 == nDeg10)
					&& (nDeg00 == nDeg11) )
				{
					nDeg = nDeg00 ;
				}
				else if ( (nDeg00 == nDeg01)
					|| (evenCol
						&& (patch.GetDegenerateNumber( sxl, sy0 ) == nDeg00)) )
				{
					if ( x > 0 )
					{
						nDeg = pPatch->GetDegenerateNumber( x - 1, y ) ;
					}
					if ( (nDeg == 0) && (nDeg10 != 0) && (nDeg10 == nDeg11) )
					{
						bool	flagWrap = false ;
						if ( evenCol && (nDeg00 == nDeg01)
							&& (x == nDivWidth - 1) && (sxn == 0) )
						{
							nDeg = pPatch->GetDegenerateNumber( 0, y ) ;
							flagWrap = (nDeg == 0) ;
						}
						if ( nDeg == 0 )
						{
							nDeg = (uint32_t) degenerate.AddDegenerateNullEntry
												( degenerate.GetDegenerateFlagAt
													( (size_t) nDeg00 - 1 ) ) + 1 ;
						}
						if ( flagWrap )
						{
							ESLAssert( pPatch->GetDegenerateNumber( 0, y ) == 0 ) ;
							pPatch->SetDegenerateNumber( 0, y, nDeg ) ;
							degenerate.AddDegenerateEntryAt
								( (size_t) nDeg - 1, pPatch->IndexFromPoint( 0, y ) ) ;
						}
					}
				}
				else if ( (nDeg00 == nDeg10)
					|| (evenLine
						&& (patch.GetDegenerateNumber( sx0, syl ) == nDeg00)) )
				{
					if ( y > 0 )
					{
						nDeg = pPatch->GetDegenerateNumber( x, y - 1 ) ;
					}
					if ( (nDeg == 0) && (nDeg01 != 0) && (nDeg01 == nDeg11) )
					{
						bool	flagWrap = false ;
						if ( evenLine && (nDeg00 == nDeg10)
							&& (y == nDivHeight - 1) && (syn == 0) )
						{
							nDeg = pPatch->GetDegenerateNumber( x, 0 ) ;
							flagWrap = (nDeg == 0) ;
						}
						if ( nDeg == 0 )
						{
							nDeg = (uint32_t) degenerate.AddDegenerateNullEntry
												( degenerate.GetDegenerateFlagAt
													( (size_t) nDeg00 - 1 ) ) + 1 ;
						}
						if ( flagWrap )
						{
							ESLAssert( pPatch->GetDegenerateNumber( x, 0 ) == 0 ) ;
							pPatch->SetDegenerateNumber( x, 0, nDeg ) ;
							degenerate.AddDegenerateEntryAt
								( (size_t) nDeg - 1, pPatch->IndexFromPoint( x, 0 ) ) ;
						}
					}
				}
			}
			pPatch->SetElements( x, y, el00 ) ;
			pPatch->SetFace( x, y, face ) ;
			ESLAssert( pPatch->GetDegenerateNumber( x, y ) == 0 ) ;
			pPatch->SetDegenerateNumber( x, y, nDeg ) ;
			//
			if ( nDeg != 0 )
			{
				degenerate.AddDegenerateEntryAt
					( (size_t) nDeg - 1, pPatch->IndexFromPoint( x, y ) ) ;
			}
		}
	}
	bufWeights.FinishArray() ;
	//
	return	pPatch ;
}

// パッチポインタ置き換え
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::RemapPatchPointers
	( S3DMeshEditor::SeemEdges& sedges,
		const SPtrSortArray<S3DMeshEditor::Patch,S3DMeshEditor::Patch*>& psaMap ) const
{
	for ( size_t i = 0; i < sedges.GetLength(); i ++ )
	{
		PatchEdgeSet *	pes =sedges.GetAt( i ) ;
		ESLAssert( pes != nullptr ) ;
		PatchEdge *		pe = pes->GetArray() ;
		const size_t	nCount = pes->GetLength() ;
		Patch *			pLastSrcPatch = nullptr ;
		Patch *			pMappedPatch = nullptr ;
		for ( size_t j = 0; j < nCount; j ++ )
		{
			if ( pe[j].pPatch != pLastSrcPatch )
			{
				pLastSrcPatch = pe[j].pPatch ;
				pMappedPatch = pLastSrcPatch ;
				//
				S3DMeshEditor::Patch**	ppPatch = psaMap.GetAs( pLastSrcPatch ) ;
				if ( ppPatch != nullptr )
				{
					pMappedPatch = *ppPatch ;
				}
			}
			pe[j].pPatch = pMappedPatch ;
		}
		pes->SortArray() ;
	}
}

void S3DMeshEditor::RemapAllPatchPointers
	( SObjectArray<S3DMeshEditor::SeemEdges>& aSeemEdges,
		const SPtrSortArray<S3DMeshEditor::Patch,S3DMeshEditor::Patch*>& psaMap ) const
{
	for ( size_t i = 0; i < aSeemEdges.GetLength(); i ++ )
	{
		S3DMeshEditor::SeemEdges *	pEdges = aSeemEdges.GetAt( i ) ;
		ESLAssert( pEdges != nullptr ) ;
		RemapPatchPointers( *pEdges, psaMap ) ;
	}
}

// 分割パッチの縮退点を設定
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditor::DegenerateRedividedPatch
	( S3DMeshEditor::Patch * pPatch,
		const S3DMeshEditor::SeemEdges& sedges,
		const S3DMeshEditor::EdgeDivRefMap& edrm )
{
	SArray<EdgeDivRef> *	pEdgeDivRef = edrm.GetAs( pPatch ) ;
	ESLAssert( pEdgeDivRef != nullptr ) ;
	if ( pEdgeDivRef == nullptr )
	{
		return ;
	}
	PatchPointSet	ppsDeg ;
	for ( size_t i = 0; i < pEdgeDivRef->GetLength(); i ++ )
	{
		const EdgeDivRef&	eddrThis = pEdgeDivRef->At(i) ;
		PatchEdgeSetSeem *	pessRef = sedges.GetAs( eddrThis ) ;
		if ( pessRef == nullptr )
		{
			continue ;
		}
		PatchPoint	ppThis( pPatch, eddrThis.iDst ) ;
		if ( m_seams.FindPoint( ppThis ) >= 0 )
		{
			continue ;
		}
		ppsDeg.RemoveAll() ;
		ppsDeg.AddSorted( ppThis ) ;
		//
		for ( size_t j = 0; j < pessRef->GetLength(); j ++ )
		{
			PatchEdge	peRef = pessRef->At(j) ;
			if ( peRef.pPatch == pPatch )
			{
				continue ;
			}
			SArray<EdgeDivRef> *	peddr = edrm.GetAs( peRef.pPatch ) ;
			if ( peddr == nullptr )
			{
				continue ;
			}
			const Edge	edge = peRef ;
			for ( size_t k = 0; k < peddr->GetLength(); k ++ )
			{
				const EdgeDivRef&	eddrRef = peddr->At(k) ;
				if ( edge == eddrRef )
				{
					if ( fabs(eddrThis.tDiv - eddrRef.tDiv) < 1.0e-5 )
					{
						ESLAssert( eddrRef.iDst < peRef.pPatch->GetTotalVertexCount() ) ;
						ppsDeg.AddSorted( PatchPoint( peRef.pPatch, eddrRef.iDst ) ) ;
					}
				}
			}
		}
		ShrinkPoints( ppsDeg, pessRef->m_nFlags ) ;
	}
	SetUpdateVertexFlag() ;
}



//////////////////////////////////////////////////////////////////////////////
// メッシュ編集インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DMeshEditorInterface, ESLObject )

// 表示用マテリアルの取得
//////////////////////////////////////////////////////////////////////////////
S3DMaterial * S3DMeshEditorInterface::GetMeshMaterial( size_t iMaterial ) const
{
	return	nullptr ;
}

// 空間
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorInterface::GetMeshItemMatrix( S3DDMatrix& mat, S3DDVector& pos ) const
{
	mat = S3DDMatrix( 1, 1, 1 ) ;
	pos = S3DDVector( 0, 0, 0 ) ;
}

// パッチ総数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshEditorInterface::GetPatchCount( void ) const
{
	return	GetMeshEditor().GetPatchCount() ;
}

// パッチ取得
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditor::Patch * S3DMeshEditorInterface::GetPatchAt( size_t i ) const
{
	return	GetMeshEditor().GetPatchAt( i ) ;
}

// パッチ生成
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditor::Patch * S3DMeshEditorInterface::NewPatch( void )
{
	return	MeshEditor().NewPatch() ;
}

// パッチ追加
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshEditorInterface::AddPatch( S3DMeshEditor::Patch * pPatch )
{
	return	MeshEditor().AddPatch( pPatch ) ;
}

size_t S3DMeshEditorInterface::InsertPatchAt( size_t i, S3DMeshEditor::Patch * pPatch )
{
	return	MeshEditor().InsertPatchAt( i, pPatch ) ;
}

// パッチ削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorInterface::RemovePatchAt( size_t i )
{
	MeshEditor().RemovePatchAt( i ) ;
}

// パッチ検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DMeshEditorInterface::FindPatch( S3DMeshEditor::Patch * pPatch ) const
{
	return	GetMeshEditor().FindPatch( pPatch ) ;
}

// パッチ順序入れ替え
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorInterface::SwapPatchOrder( size_t iPatch0, size_t iPatch1 )
{
	MeshEditor().SwapPatchOrder( iPatch0, iPatch1 ) ;
}

// 水平ライン挿入
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorInterface::InsertLine
	( S3DMeshEditor::Patch * pPatch, size_t iLine, float_t w )
{
	MeshEditor().SetUpdateVertexFlag() ;
	pPatch->InsertLine( iLine, w ) ;
}

// 垂直ライン挿入
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorInterface::InsertColumn
	( S3DMeshEditor::Patch * pPatch, size_t iCol, float_t w )
{
	MeshEditor().SetUpdateVertexFlag() ;
	pPatch->InsertColumn( iCol, w ) ;
}

// 水平ライン削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorInterface::RemoveLine
	( S3DMeshEditor::Patch * pPatch, size_t iLine )
{
	MeshEditor().SetUpdateVertexFlag() ;
	pPatch->RemoveLine( iLine ) ;
}

// 垂直ライン削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorInterface::RemoveColumn
	( S3DMeshEditor::Patch * pPatch, size_t iCol )
{
	MeshEditor().SetUpdateVertexFlag() ;
	pPatch->RemoveColumn( iCol ) ;
}

// 水平ライン入れ替え
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorInterface::SwapLine
	( S3DMeshEditor::Patch * pPatch, size_t iLine1, size_t iLine2 )
{
	MeshEditor().SetUpdateVertexFlag() ;
	pPatch->SwapLine( iLine1, iLine2 ) ;
}

// 頂点縮退フラグ変更
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorInterface::ChangeDegeneratedPointFlag
	( const S3DMeshEditor::PatchPointSet& pps, uint32_t nFlags )
{
	MeshEditor().SetDegeneratedPointFlag( pps, nFlags ) ;
}

// 頂点縮退処理（頂点座標操作は無し）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorInterface::ShrinkPoints
	( const S3DMeshEditor::PatchPointSet& pps, uint32_t nFlags )
{
	MeshEditor().ShrinkPoints( pps, nFlags ) ;
}

// 頂点縮退解除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorInterface::UntiShrinkPoints( const S3DMeshEditor::PatchPoint& pp )
{
	MeshEditor().UntiShrinkPoints( pp ) ;
}

// 値更新通知
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorInterface::NotifyUpdateVertex
		( const S3DMeshEditor::SelectPointSet& selPoints )
{
	MeshEditor().SetUpdateVertexFlag() ;
}

void S3DMeshEditorInterface::NotifyUpdateFace
		( const S3DMeshEditor::PatchFaceSet& selFaces )
{
	MeshEditor().SetUpdateVertexFlag() ;
}




//////////////////////////////////////////////////////////////////////////////
// メッシュ編集ブリッジ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DMeshEditorBridge, S3DMeshEditorInterface )

// パッチ追加
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshEditorBridge::AddPatch( S3DMeshEditor::Patch * pPatch )
{
	size_t	iPatch = S3DMeshEditorInterface::AddPatch( pPatch ) ;
	//
	class	FunctionAddPatch	: public Function
	{
	protected:
		S3DMeshEditor::Patch *	m_pPatch ;
	public:
		FunctionAddPatch( S3DMeshEditor::Patch * pPatch ) : m_pPatch( pPatch ) {}
		virtual void Invoke( S3DMeshEditorBridge * pBridge )
				{	pBridge->OnAddPatch( m_pPatch ) ;	}
	} ;
	FunctionAddPatch	func( pPatch ) ;
	InvokeFunction( func ) ;
	//
	return	iPatch ;
}

size_t S3DMeshEditorBridge::InsertPatchAt( size_t i, S3DMeshEditor::Patch * pPatch )
{
	size_t	iPatch = S3DMeshEditorInterface::InsertPatchAt( i, pPatch ) ;
	//
	class	FunctionInsertPatch	: public Function
	{
	protected:
		size_t					m_iPatch ;
		S3DMeshEditor::Patch *	m_pPatch ;
	public:
		FunctionInsertPatch( size_t i, S3DMeshEditor::Patch * pPatch )
				: m_iPatch( i ), m_pPatch( pPatch ) {}
		virtual void Invoke( S3DMeshEditorBridge * pBridge )
			{	pBridge->OnInsertPatchAt( m_iPatch, m_pPatch ) ;	}
	} ;
	FunctionInsertPatch	func( i, pPatch ) ;
	InvokeFunction( func ) ;
	//
	return	iPatch ;
}

// パッチ削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorBridge::RemovePatchAt( size_t i )
{
	S3DMeshEditorInterface::RemovePatchAt( i ) ;
	//
	class	FunctionRemovePatch	: public Function
	{
	protected:
		size_t	m_iPatch ;
	public:
		FunctionRemovePatch( size_t i ) : m_iPatch( i ) {}
		virtual void Invoke( S3DMeshEditorBridge * pBridge )
			{	pBridge->OnRemovePatchAt( m_iPatch ) ;	}
	} ;
	FunctionRemovePatch	func( i ) ;
	InvokeFunction( func ) ;
}

// パッチ順序入れ替え
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorBridge::SwapPatchOrder( size_t iPatch0, size_t iPatch1 )
{
	S3DMeshEditorInterface::SwapPatchOrder( iPatch0, iPatch1 ) ;
	//
	class	FunctionRemovePatch	: public Function
	{
	protected:
		size_t	m_iPatch0 ;
		size_t	m_iPatch1 ;
	public:
		FunctionRemovePatch( size_t iPatch0, size_t iPatch1 )
			: m_iPatch0( iPatch0 ), m_iPatch1( iPatch1 ) {}
		virtual void Invoke( S3DMeshEditorBridge * pBridge )
			{	pBridge->OnSwapPatchOrder( m_iPatch0, m_iPatch1 ) ;	}
	} ;
	FunctionRemovePatch	func( iPatch0, iPatch1 ) ;
	InvokeFunction( func ) ;
}

// 水平ライン挿入
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorBridge::InsertLine
	( S3DMeshEditor::Patch * pPatch, size_t iLine, float_t w )
{
	S3DMeshEditorInterface::InsertLine( pPatch, iLine, w ) ;
	//
	class	FunctionInsertLine	: public Function
	{
	protected:
		size_t		m_iPatch ;
		size_t		m_iLine ;
		float32_t	m_fpWeight ;
	public:
		FunctionInsertLine( size_t iPatch, size_t iLine, float_t w )
			: m_iPatch( iPatch ), m_iLine( iLine ), m_fpWeight( w ) {}
		virtual void Invoke( S3DMeshEditorBridge * pBridge )
			{	pBridge->OnInsertLine( m_iPatch, m_iLine, m_fpWeight ) ;	}
	} ;
	ssize_t	iPatch = GetMeshEditor().FindPatch( pPatch ) ;
	ESLAssert( iPatch >= 0 ) ;
	FunctionInsertLine	func( (size_t) iPatch, iLine, w ) ;
	InvokeFunction( func ) ;
}

// 垂直ライン挿入
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorBridge::InsertColumn
	( S3DMeshEditor::Patch * pPatch, size_t iCol, float_t w )
{
	S3DMeshEditorInterface::InsertColumn( pPatch, iCol, w ) ;
	//
	class	FunctionInsertColumn	: public Function
	{
	protected:
		size_t		m_iPatch ;
		size_t		m_iCol ;
		float32_t	m_fpWeight ;
	public:
		FunctionInsertColumn( size_t iPatch, size_t iLine, float_t w )
			: m_iPatch( iPatch ), m_iCol( iLine ), m_fpWeight( w ) {}
		virtual void Invoke( S3DMeshEditorBridge * pBridge )
			{	pBridge->OnInsertColumn( m_iPatch, m_iCol, m_fpWeight ) ;	}
	} ;
	ssize_t	iPatch = GetMeshEditor().FindPatch( pPatch ) ;
	ESLAssert( iPatch >= 0 ) ;
	FunctionInsertColumn	func( (size_t) iPatch, iCol, w ) ;
	InvokeFunction( func ) ;
}

// 水平ライン削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorBridge::RemoveLine
	( S3DMeshEditor::Patch * pPatch, size_t iLine )
{
	S3DMeshEditorInterface::RemoveLine( pPatch, iLine ) ;
	//
	class	FunctionRemoveLine	: public Function
	{
	protected:
		size_t		m_iPatch ;
		size_t		m_iLine ;
	public:
		FunctionRemoveLine( size_t iPatch, size_t iLine )
			: m_iPatch( iPatch ), m_iLine( iLine ) {}
		virtual void Invoke( S3DMeshEditorBridge * pBridge )
			{	pBridge->OnRemoveLine( m_iPatch, m_iLine ) ;	}
	} ;
	ssize_t	iPatch = GetMeshEditor().FindPatch( pPatch ) ;
	ESLAssert( iPatch >= 0 ) ;
	FunctionRemoveLine	func( (size_t) iPatch, iLine ) ;
	InvokeFunction( func ) ;
}

// 垂直ライン削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorBridge::RemoveColumn
	( S3DMeshEditor::Patch * pPatch, size_t iCol )
{
	S3DMeshEditorInterface::RemoveColumn( pPatch, iCol ) ;
	//
	class	FunctionRemoveColumn	: public Function
	{
	protected:
		size_t		m_iPatch ;
		size_t		m_iCol ;
	public:
		FunctionRemoveColumn( size_t iPatch, size_t iCol )
			: m_iPatch( iPatch ), m_iCol( iCol ) {}
		virtual void Invoke( S3DMeshEditorBridge * pBridge )
			{	pBridge->OnRemoveLine( m_iPatch, m_iCol ) ;	}
	} ;
	ssize_t	iPatch = GetMeshEditor().FindPatch( pPatch ) ;
	ESLAssert( iPatch >= 0 ) ;
	FunctionRemoveColumn	func( (size_t) iPatch, iCol ) ;
	InvokeFunction( func ) ;
}

// 水平ライン入れ替え
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorBridge::SwapLine
	( S3DMeshEditor::Patch * pPatch, size_t iLine1, size_t iLine2 )
{
	S3DMeshEditorInterface::SwapLine( pPatch, iLine1, iLine2 ) ;
	//
	class	FunctionSwapLine	: public Function
	{
	protected:
		size_t		m_iPatch ;
		size_t		m_iLine1 ;
		size_t		m_iLine2 ;
	public:
		FunctionSwapLine( size_t iPatch, size_t iLine1, size_t iLine2 )
			: m_iPatch( iPatch ), m_iLine1( iLine1 ), m_iLine2( iLine2 ) {}
		virtual void Invoke( S3DMeshEditorBridge * pBridge )
			{	pBridge->OnSwapLine( m_iPatch, m_iLine1, m_iLine2 ) ;	}
	} ;
	ssize_t	iPatch = GetMeshEditor().FindPatch( pPatch ) ;
	ESLAssert( iPatch >= 0 ) ;
	FunctionSwapLine	func( (size_t) iPatch, iLine1, iLine2 ) ;
	InvokeFunction( func ) ;
}

// 頂点縮退フラグ変更
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorBridge::ChangeDegeneratedPointFlag
	( const S3DMeshEditor::PatchPointSet& pps, uint32_t nFlags )
{
	S3DMeshEditorInterface::ChangeDegeneratedPointFlag( pps, nFlags ) ;
	//
	class	FunctionChangeDegeneratedPointFlag	: public Function
	{
	protected:
		S3DMeshEditor::SelectPointSet	m_ppsIndexed ;
		uint32_t						m_nFlags ;
		S3DMeshEditor::SelectPointSet	m_ppsTemp ;
	public:
		FunctionChangeDegeneratedPointFlag
			( const S3DMeshEditor& editor,
				const S3DMeshEditor::PatchPointSet& pps, uint32_t nFlags )
			: m_nFlags( nFlags )
		{
			editor.SelectPointsFromPoints( m_ppsIndexed, pps ) ;
			editor.IndexPatchForSelectPoints( m_ppsIndexed ) ;
		}
		virtual void Invoke( S3DMeshEditorBridge * pBridge )
		{
			m_ppsTemp = m_ppsIndexed ;
			pBridge->GetMeshEditor().PointerPatchForSelectPoints( m_ppsTemp ) ;
			pBridge->OnChangeDegeneratedPointFlag( m_ppsTemp, m_nFlags ) ;
		}
	} ;
	if ( IsSynchronizedMeshEditor() )
	{
		FunctionChangeDegeneratedPointFlag	func( GetMeshEditor(), pps, nFlags ) ;
		InvokeFunction( func ) ;
	}
}

// 頂点縮退処理（頂点座標操作は無し）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorBridge::ShrinkPoints
	( const S3DMeshEditor::PatchPointSet& pps, uint32_t nFlags )
{
	S3DMeshEditorInterface::ShrinkPoints( pps, nFlags ) ;
	//
	class	FunctionShrinkPoints	: public Function
	{
	protected:
		S3DMeshEditor::SelectPointSet	m_ppsIndexed ;
		uint32_t						m_nFlags ;
		S3DMeshEditor::SelectPointSet	m_ppsTemp ;
	public:
		FunctionShrinkPoints
			( const S3DMeshEditor& editor,
				const S3DMeshEditor::PatchPointSet& pps, uint32_t nFlags )
			: m_nFlags( nFlags )
		{
			editor.SelectPointsFromPoints( m_ppsIndexed, pps ) ;
			editor.IndexPatchForSelectPoints( m_ppsIndexed ) ;
		}
		virtual void Invoke( S3DMeshEditorBridge * pBridge )
		{
			m_ppsTemp = m_ppsIndexed ;
			pBridge->GetMeshEditor().PointerPatchForSelectPoints( m_ppsTemp ) ;
			pBridge->OnShrinkPoints( m_ppsTemp, m_nFlags ) ;
		}
	} ;
	if ( IsSynchronizedMeshEditor() )
	{
		FunctionShrinkPoints	func( GetMeshEditor(), pps, nFlags ) ;
		InvokeFunction( func ) ;
	}
}

// 頂点縮退解除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorBridge::UntiShrinkPoints( const S3DMeshEditor::PatchPoint& pp )
{
	S3DMeshEditorInterface::UntiShrinkPoints( pp ) ;
	//
	class	FunctionUntiShrinkPoints	: public Function
	{
	protected:
		size_t	m_iPatch ;
		size_t	m_iVertex ;
	public:
		FunctionUntiShrinkPoints( size_t iPatch, size_t iVertex )
			: m_iPatch( iPatch ), m_iVertex( iVertex ) { }
		virtual void Invoke( S3DMeshEditorBridge * pBridge )
			{	pBridge->OnUntiShrinkPoints( m_iPatch, m_iVertex ) ;	}
	} ;
	if ( IsSynchronizedMeshEditor() )
	{
		ssize_t	iPatch = GetMeshEditor().FindPatch( pp.pPatch ) ;
		ESLAssert( iPatch >= 0 ) ;
		FunctionUntiShrinkPoints	func( iPatch, pp.iVertex ) ;
		InvokeFunction( func ) ;
	}
}

// 値更新通知
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorBridge::NotifyUpdateVertex
		( const S3DMeshEditor::SelectPointSet& selPoints )
{
	S3DMeshEditorInterface::NotifyUpdateVertex( selPoints ) ;
	//
	class	FunctionNotifyUpdateVertex	: public Function
	{
	protected:
		S3DMeshEditor::SelectPointSet	m_ppsIndexed ;
		const S3DMeshEditor&			m_meshSrc ;
	public:
		FunctionNotifyUpdateVertex
			( const S3DMeshEditor& editor,
				const S3DMeshEditor::SelectPointSet& selPoints )
			: m_ppsIndexed( selPoints ), m_meshSrc( editor )
		{
			editor.IndexPatchForSelectPoints( m_ppsIndexed ) ;
		}
		virtual void Invoke( S3DMeshEditorBridge * pBridge )
		{
			pBridge->OnNotifyUpdateVertex( m_ppsIndexed, m_meshSrc ) ;
		}
	} ;
	if ( IsSynchronizedMeshEditor() )
	{
		FunctionNotifyUpdateVertex	func( GetMeshEditor(), selPoints ) ;
		InvokeFunction( func ) ;
	}
}

void S3DMeshEditorBridge::NotifyUpdateFace
		( const S3DMeshEditor::PatchFaceSet& selFaces )
{
	S3DMeshEditorInterface::NotifyUpdateFace( selFaces ) ;
	//
	class	FunctionNotifyUpdateFace	: public Function
	{
	protected:
		S3DMeshEditor::PatchFaceSet	m_pfsIndexed ;
		const S3DMeshEditor&		m_meshSrc ;
	public:
		FunctionNotifyUpdateFace
			( const S3DMeshEditor& editor,
				const S3DMeshEditor::PatchFaceSet& selFaces )
			: m_pfsIndexed( selFaces ), m_meshSrc( editor )
		{
			editor.IndexPatchForPatchFaceSet( m_pfsIndexed ) ;
		}
		virtual void Invoke( S3DMeshEditorBridge * pBridge )
		{
			pBridge->OnNotifyUpdateFace( m_pfsIndexed, m_meshSrc ) ;
		}
	} ;
	if ( IsSynchronizedMeshEditor() )
	{
		FunctionNotifyUpdateFace	func( GetMeshEditor(), selFaces ) ;
		InvokeFunction( func ) ;
	}
}

// パッチ追加
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshEditorBridge::OnAddPatch( S3DMeshEditor::Patch * pPatch )
{
	ESLAssert( IsSynchronizedMeshEditor() ) ;
	S3DMeshEditor&			editor = MeshEditor() ;
	S3DMeshEditor::Patch *	pNewPatch = editor.NewPatch() ;
	pNewPatch->CopyFrom( *pPatch ) ;
	pNewPatch->RemoveWeightLayer( 0, pNewPatch->GetWeightLayerCount() ) ;
	return	editor.AddPatch( pNewPatch ) ;
}

size_t S3DMeshEditorBridge::OnInsertPatchAt( size_t i, S3DMeshEditor::Patch * pPatch )
{
	ESLAssert( IsSynchronizedMeshEditor() ) ;
	S3DMeshEditor&			editor = MeshEditor() ;
	S3DMeshEditor::Patch *	pNewPatch = editor.NewPatch() ;
	pNewPatch->CopyFrom( *pPatch ) ;
	return	editor.InsertPatchAt( i, pNewPatch ) ;
}

// パッチ削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorBridge::OnRemovePatchAt( size_t i )
{
	ESLAssert( IsSynchronizedMeshEditor() ) ;
	MeshEditor().RemovePatchAt( i ) ;
}

// パッチ順序入れ替え
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorBridge::OnSwapPatchOrder( size_t iPatch0, size_t iPatch1 )
{
	ESLAssert( IsSynchronizedMeshEditor() ) ;
	MeshEditor().SwapPatchOrder( iPatch0, iPatch1 ) ;
}

// 水平ライン挿入
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorBridge::OnInsertLine
	( size_t iPatch, size_t iLine, float_t w )
{
	ESLAssert( IsSynchronizedMeshEditor() ) ;
	S3DMeshEditor&			editor = MeshEditor() ;
	S3DMeshEditor::Patch *	pPatch = editor.GetPatchAt( iPatch ) ;
	if ( pPatch != nullptr )
	{
		if ( iLine <= pPatch->GetHeight() )
		{
			editor.SetUpdateVertexFlag() ;
			pPatch->InsertLine( iLine, w ) ;
		}
	}
}

// 垂直ライン挿入
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorBridge::OnInsertColumn
	( size_t iPatch, size_t iCol, float_t w )
{
	ESLAssert( IsSynchronizedMeshEditor() ) ;
	S3DMeshEditor&			editor = MeshEditor() ;
	S3DMeshEditor::Patch *	pPatch = editor.GetPatchAt( iPatch ) ;
	if ( pPatch != nullptr )
	{
		if ( iCol <= pPatch->GetWidth() )
		{
			editor.SetUpdateVertexFlag() ;
			pPatch->InsertColumn( iCol, w ) ;
		}
	}
}

// 水平ライン削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorBridge::OnRemoveLine( size_t iPatch, size_t iLine )
{
	ESLAssert( IsSynchronizedMeshEditor() ) ;
	S3DMeshEditor&			editor = MeshEditor() ;
	S3DMeshEditor::Patch *	pPatch = editor.GetPatchAt( iPatch ) ;
	if ( pPatch != nullptr )
	{
		if ( iLine <= pPatch->GetHeight() )
		{
			editor.SetUpdateVertexFlag() ;
			pPatch->RemoveLine( iLine ) ;
		}
	}
}

// 垂直ライン削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorBridge::OnRemoveColumn( size_t iPatch, size_t iCol )
{
	ESLAssert( IsSynchronizedMeshEditor() ) ;
	S3DMeshEditor&			editor = MeshEditor() ;
	S3DMeshEditor::Patch *	pPatch = editor.GetPatchAt( iPatch ) ;
	if ( pPatch != nullptr )
	{
		if ( iCol <= pPatch->GetWidth() )
		{
			editor.SetUpdateVertexFlag() ;
			pPatch->RemoveColumn( iCol ) ;
		}
	}
}

// 水平ライン入れ替え
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorBridge::OnSwapLine
	( size_t iPatch, size_t iLine1, size_t iLine2 )
{
	ESLAssert( IsSynchronizedMeshEditor() ) ;
	S3DMeshEditor&			editor = MeshEditor() ;
	S3DMeshEditor::Patch *	pPatch = editor.GetPatchAt( iPatch ) ;
	if ( pPatch != nullptr )
	{
		if ( (iLine1 <= pPatch->GetHeight())
			&& (iLine2 <= pPatch->GetHeight()) )
		{
			editor.SetUpdateVertexFlag() ;
			pPatch->SwapLine( iLine1, iLine2 ) ;
		}
	}
}

// 頂点縮退フラグ変更
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorBridge::OnChangeDegeneratedPointFlag
	( const S3DMeshEditor::SelectPointSet& sps, uint32_t nFlags )
{
	ESLAssert( IsSynchronizedMeshEditor() ) ;
	S3DMeshEditor::PatchPointSet	pps ;
	GetMeshEditor().PatchPointsFromSelectedPoints( pps, sps ) ;
	MeshEditor().SetDegeneratedPointFlag( pps, nFlags ) ;
}

// 頂点縮退処理（頂点座標操作は無し）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorBridge::OnShrinkPoints
	( const S3DMeshEditor::SelectPointSet& sps, uint32_t nFlags )
{
	ESLAssert( IsSynchronizedMeshEditor() ) ;
	S3DMeshEditor::PatchPointSet	pps ;
	GetMeshEditor().PatchPointsFromSelectedPoints( pps, sps ) ;
	MeshEditor().ShrinkPoints( pps, nFlags ) ;
}

// 頂点縮退解除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorBridge::OnUntiShrinkPoints( size_t iPatch, size_t iVertex )
{
	ESLAssert( IsSynchronizedMeshEditor() ) ;
	S3DMeshEditor&			editor = MeshEditor() ;
	S3DMeshEditor::Patch *	pPatch = editor.GetPatchAt( iPatch ) ;
	if ( pPatch != nullptr )
	{
		editor.UntiShrinkPoints( S3DMeshEditor::PatchPoint( pPatch, iVertex ) ) ;
	}
}

// 値更新通知
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorBridge::OnNotifyUpdateVertex
		( const S3DMeshEditor::SelectPointSet& selPointsIndexed,
			const S3DMeshEditor& meshSrc )
{
	ESLAssert( IsSynchronizedMeshEditor() ) ;
}

void S3DMeshEditorBridge::OnNotifyUpdateFace
		( const S3DMeshEditor::PatchFaceSet& selFacesIndexed,
			const S3DMeshEditor& meshSrc )
{
	ESLAssert( IsSynchronizedMeshEditor() ) ;
	S3DMeshEditor&	meshDst = MeshEditor() ;
	for ( size_t i = 0; i < selFacesIndexed.GetLength(); i ++ )
	{
		S3DMeshEditor::PatchFace	pf = selFacesIndexed.At(i) ;
		const size_t				iPatch = (size_t) ((ulong_ptr_t) pf.pPatch) ;
		S3DMeshEditor::Patch *		pSrcPatch = meshSrc.GetPatchAt( iPatch ) ;
		S3DMeshEditor::Patch *		pDstPatch = meshDst.GetPatchAt( iPatch ) ;
		if ( (pSrcPatch != nullptr) && (pDstPatch != nullptr) )
		{
			ESLAssert( pf.iFace < pSrcPatch->GetTotalFaceCount() ) ;
			if ( pf.iFace < pDstPatch->GetTotalFaceCount() )
			{
				pDstPatch->SetFaceAt
					( pf.iFace, pSrcPatch->GetFaceAt( pf.iFace ) ) ;
				pDstPatch->SetFaceShifterAt
					( pf.iFace, pSrcPatch->GetFaceShifterAt( pf.iFace ) ) ;
			}
		}
	}
	meshDst.SetUpdateVertexFlag() ;
}



//////////////////////////////////////////////////////////////////////////////
// メッシュ編集オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DMeshEditorObject, S3DMeshEditorInterface, S3DMeshEditor )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditorObject::S3DMeshEditorObject( void )
	: m_flagUpdateEditCollision( false ),
		m_vSelMin( 0, 0, 0 ), m_vSelMax( 0, 0, 0 ),
		m_vSelUVMin( 0, 0 ), m_vSelUVMax( 0, 0 ),
		m_iCurPatch( -1 ), m_iCurWeight( -1 ), m_iCurMaterial( 0 ),
		m_pEditContext( nullptr ), m_flagOwnEditContext( false ),
		m_pExtraInfo( nullptr )
{
	m_param.nFlags = S3DMeshEditor::flagMeshAllSmooth
						| S3DMeshEditor::flagMeshEdgeByAngle ;
	m_param.fpEdgeAngle = 30.0f ;
	m_param.nDivHorz = 1 ;
	m_param.nDivVert = 1 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditorObject::~S3DMeshEditorObject( void )
{
	if ( m_pEditContext && m_flagOwnEditContext )
	{
		delete	m_pEditContext ;
		m_pEditContext = nullptr ;
		m_flagOwnEditContext = false ;
	}
}

// パラメータ
//////////////////////////////////////////////////////////////////////////////
const S3DMeshEditor::MeshParam& S3DMeshEditorObject::GetMeshParameter( void ) const
{
	return	m_param ;
}

S3DMeshEditor::MeshParam& S3DMeshEditorObject::MeshParameter( void )
{
	return	m_param ;
}

void S3DMeshEditorObject::SetMeshParameter( const S3DMeshEditor::MeshParam& param )
{
	m_param = param ;
}

// GetMeshMaterial, GetMeshItemMatrix リダイレクト先
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::AttachExtraMeshInfo( S3DMeshEditorInterface * pMeshEditor )
{
	m_pExtraInfo = pMeshEditor ;
}

// 更新パッチを再構築する
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::UpdateAllPatchs( const S3DMeshEditor::MeshParam& param )
{
	m_flagUpdateEditCollision |= GetUpdateVertexFlag() ;
	S3DMeshEditor::UpdateAllPatchs( param ) ;
}

// 編集メッシュ更新
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::UpdateEditCollision( void )
{
	if ( m_flagUpdateEditCollision || GetUpdateVertexFlag() )
	{
		S3DMeshEditor::UpdateAllPatchs( m_param ) ;
		//
		m_colliEditor.ClearBuffer() ;
		//
		MeshBuffer	mesh ;
		for ( size_t i = 0; i < m_patchs.GetLength(); i ++ )
		{
			Patch *	pPatch = m_patchs.GetAt( i ) ;
			ESLAssert( pPatch != nullptr ) ;
			if ( pPatch->GetFlags() & S3DMeshEditor::Patch::flagInvisible )
			{
				continue ;
			}
			pPatch->BuildAllPolygonsSimply( mesh ) ;
			//
			if ( mesh.m_countIndex == 0 )
			{
				continue ;
			}
			m_colliEditor.AttachMeshUserData( pPatch ) ;
			m_colliEditor.AddIndexedPrimitiveList
				( S3DMaterial::GetDefaultMaterial(S3DMaterial::defaultWhite),
					0, primitiveTriangle, mesh.m_countIndex, mesh.m_countVertex,
					mesh.m_bufVertex.GetConstArray(),
					mesh.m_bufNormal.GetConstArray(),
					mesh.m_bufUVMap.GetConstArray(),
					mesh.m_bufColor.GetConstArray(),
					mesh.m_bufIndex.GetConstArray() ) ;
		}
		m_flagUpdateEditCollision = false ;
	}
}

// 編集メッシュコリジョン範囲取得
//////////////////////////////////////////////////////////////////////////////
double S3DMeshEditorObject::GetEditCollisionCircumscribedSpher( S3DVector& vCenter )
{
	UpdateEditCollision() ;
	//
	return	m_colliEditor.GetCircumscribedSphere( vCenter ) ;
}

// 当たり判定
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditorObject::HitRayToEditMesh
	( const S3DDVector& vPos0, const S3DDVector& vPos1,
		float fpErrorGap, uint32_t nFlags,
		S3DMeshEditorObject::HitRayResult& hrrResult,
		Patch *const* ppExclusionPatchs, size_t nExclusionPatchs )
{
	UpdateEditCollision() ;
	//
	SPointerArray<const S3DCollision::MeshCollision>	aExclusion ;
	S3DCollisionResult	rsCross ;
	rsCross.fpDistance = (float32_t) (vPos1 - vPos0).Absolute() ;
	rsCross.pMesh = nullptr ;
	rsCross.pqpcHit = nullptr ;
	//
	HitRayToEditMeshInstance	hrtemi ;
	hrtemi.vRay = (vPos1 - vPos0).Normalized() ;
	hrtemi.nFlags = nFlags ;
	hrtemi.ppExclusionPatchs = ppExclusionPatchs ;
	hrtemi.nExclusionPatchs = nExclusionPatchs ;
	//
	rsCross.pfnOnHitCollider = &S3DMeshEditorObject::Callback_OnHitRayToEditMesh ;
	rsCross.ptrOnHitInstance = &hrtemi ;
	//
	if ( m_colliEditor.IsSegmentCrossing( vPos0, vPos1, fpErrorGap, rsCross ) )
	{
		S3DMeshEditor::Patch *	pPatch =
			ESLTypeCast<S3DMeshEditor::Patch>( rsCross.pMesh->pUserData ) ;
		ESLAssert( pPatch != nullptr ) ;
		//
		const size_t	wFace = pPatch->GetFaceWidth() ;
		const size_t	hFace = pPatch->GetFaceHeight() ;
		const size_t	nFaceCount = wFace * hFace ;
		size_t	iFace = rsCross.iPolygon / 2 ;
		if ( iFace < nFaceCount )
		{
			size_t	yFace = iFace / wFace ;
			iFace = yFace * pPatch->GetWidth() + (iFace - yFace * wFace) ;
		}
		else
		{
			iFace = rsCross.iPolygon - nFaceCount * 2 ;
		}
		//
		rsCross.ComputeHitLocalCoord() ;
		rsCross.ComplementeLocalNormal() ;
		rsCross.ComplementeTextureCoord() ;
		rsCross.ComplementeVertexColor() ;
		//
		hrrResult.pPatch = pPatch ;
		hrrResult.iFace = iFace ;
		hrrResult.vHitPos = rsCross.vHitLocal ;
		hrrResult.vNormal = rsCross.vNormalLocal ;
		hrrResult.vUV = rsCross.vTexCoord ;
		hrrResult.clrVertex = rsCross.vtxColor ;
		//
		return	true ;
	}
	return	false ;
}

S3DCollision::HitColliderCallback
	S3DMeshEditorObject::Callback_OnHitRayToEditMesh
		( const S3DCollisionResult& rsHit,
			const S3DVector& vHitPos, const S3DVector& vHitNormal,
			const S3DCollision::MeshCollision * pMesh, size_t iPolygon )
{
	S3DMeshEditor::Patch *	pPatch =
		ESLTypeCast<S3DMeshEditor::Patch>( pMesh->pUserData ) ;
	if ( pPatch == nullptr )
	{
		return	S3DCollision::hitColliderNext ;
	}
	HitRayToEditMeshInstance *	phrtemi =
				(HitRayToEditMeshInstance*) rsHit.ptrOnHitInstance ;
	ESLAssert( phrtemi != nullptr ) ;
	Patch *const*	ppExclusionPatchs = phrtemi->ppExclusionPatchs ;
	if ( ppExclusionPatchs != nullptr )
	{
		for ( size_t i = 0; i < phrtemi->nExclusionPatchs; i ++ )
		{
			if ( ppExclusionPatchs[i] == pPatch )
			{
				return	S3DCollision::hitColliderNext ;
			}
		}
	}
	const size_t	wFace = pPatch->GetFaceWidth() ;
	const size_t	hFace = pPatch->GetFaceHeight() ;
	const size_t	nFaceCount = wFace * hFace ;
	size_t	iFace = iPolygon / 2 ;
	if ( iFace < nFaceCount )
	{
		size_t	yFace = iFace / wFace ;
		iFace = yFace * pPatch->GetWidth() + (iFace - yFace * wFace) ;
	}
	else
	{
		iFace = iPolygon - nFaceCount * 2 ;
	}
	ESLAssert( iFace < pPatch->GetTotalFaceCount() ) ;
	bool	flagThrough = false ;
	if ( (phrtemi->nFlags & hitRayNoNullFace)
		&& (pPatch->GetFaceAt(iFace) == Patch::faceNull) )
	{
		return	S3DCollision::hitColliderNext ;
	}
	if ( (phrtemi->nFlags & hitRayNoBackFace)
		&& (phrtemi->vRay.InnerProduct( S3DDVector(vHitNormal) ) >= 0.0) )
	{
		return	S3DCollision::hitColliderNext ;
	}
	return	S3DCollision::hitColliderReturn ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
const SSystem::SString& S3DMeshEditorObject::Serialize( void )
{
	if ( GetUpdateSerializeFlag() )
	{
		SerializeMesh( m_serializer ) ;
	}
	return	m_serializer.Serialize() ;
}

void S3DMeshEditorObject::SerializeBinary
	( uint8_t * pbytBinary, size_t nBufBytes, size_t nHeaderBytes )
{
	if ( GetUpdateSerializeFlag() )
	{
		SerializeMesh( m_serializer ) ;
	}
	m_serializer.SerializeBinary( pbytBinary, nBufBytes, nHeaderBytes ) ;
}

size_t S3DMeshEditorObject::SerializeBuffer( size_t& nHeaderBytes )
{
	if ( GetUpdateSerializeFlag() )
	{
		SerializeMesh( m_serializer ) ;
	}
	return	m_serializer.SerializeBuffer( nHeaderBytes ) ;
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::Deserialize( const wchar_t * pwszBase64 )
{
	m_serializer.Deserialize( pwszBase64 ) ;
	DeserializeMesh( m_serializer ) ;
}

bool S3DMeshEditorObject::DeserializeBinary( const void * pbytBinary, size_t nBytes )
{
	bool	fResult =
				m_serializer.DeserializeBinary( pbytBinary, nBytes ) ;
	DeserializeMesh( m_serializer ) ;
	return	fResult ;
}

// 現在のパッチ編集選択状態
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DMeshEditorObject::GetCurrentPatch( void ) const
{
	return	m_iCurPatch ;
}

void S3DMeshEditorObject::SetCurrentPatch( ssize_t iPatch )
{
	ESLAssert( (iPatch < 0) || ((size_t) iPatch < GetPatchCount()) ) ;
	m_iCurPatch = iPatch ;
}

// 現在のウェイトマップ編集選択状態
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DMeshEditorObject::GetCurrentWeightLayer( void ) const
{
	return	m_iCurWeight ;
}

void S3DMeshEditorObject::SetCurrentWeightLayer( ssize_t iLayer )
{
	ESLAssert( (iLayer < 0) || ((size_t) iLayer < GetWeightLayerCount()) ) ;
	m_iCurWeight = iLayer ;
}

// 現在選択中のマテリアル
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshEditorObject::GetCurrentMaterial( void ) const
{
	return	m_iCurMaterial ;
}

void S3DMeshEditorObject::SetCurrentMaterial( size_t iMaterial )
{
	m_iCurMaterial = iMaterial ;
}

// 全選択解除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::ClearAllSelection( void )
{
	m_spsUVPoints.RemoveAll() ;
	m_spsPoints.RemoveAll() ;
	m_pesEdges.RemoveAll() ;
	m_pfsFaces.RemoveAll() ;
	m_plsLines.RemoveAll() ;
	m_asSelPatchs.RemoveAll() ;
	m_aSelWeights.RemoveAll() ;
}

// 頂点選択マスク取得
//////////////////////////////////////////////////////////////////////////////
const SSystem::SArray<float32_t> *
		S3DMeshEditorObject::GetSelectionWeights( size_t iPatch ) const
{
	return	m_aSelWeights.GetAt( iPatch ) ;
}

// 選択領域取得
//////////////////////////////////////////////////////////////////////////////
const S3DVector& S3DMeshEditorObject::GetSelectedRectMin( void ) const
{
	return	m_vSelMin ;
}

const S3DVector& S3DMeshEditorObject::GetSelectedRectMax( void ) const
{
	return	m_vSelMax ;
}

const S2DVector& S3DMeshEditorObject::GetSelectedRectUVMin( void ) const
{
	return	m_vSelUVMin ;
}

const S2DVector& S3DMeshEditorObject::GetSelectedRectUVMax( void ) const
{
	return	m_vSelUVMax ;
}

// 頂点選択
//////////////////////////////////////////////////////////////////////////////
const S3DMeshEditor::SelectPointSet& S3DMeshEditorObject::GetSelectedPoints( void ) const
{
	return	m_spsPoints ;
}

const S3DMeshEditor::SelectPointSet& S3DMeshEditorObject::GetSelectedUVPoints( void ) const
{
	return	m_spsUVPoints ;
}

void S3DMeshEditorObject::SetSelectedPoints( const S3DMeshEditor::SelectPointSet& selPoints )
{
	m_spsUVPoints = selPoints ;
}

void S3DMeshEditorObject::AddSelectedPoints( const S3DMeshEditor::SelectPointSet& selPoints )
{
	m_spsUVPoints.AppendSetNormalize( selPoints ) ;
}

void S3DMeshEditorObject::RemoveSelectedPoints( const S3DMeshEditor::SelectPointSet& selPoints )
{
	m_spsUVPoints.RemoveSetBySorted( selPoints ) ;
}

void S3DMeshEditorObject::UpdateSelectedPointWeights( void )
{
	m_aSelWeights.SetLength( GetPatchCount() ) ;
	for ( size_t iPatch = 0; iPatch < GetPatchCount(); iPatch ++ )
	{
		Patch *	pPatch = GetPatchAt( iPatch ) ;
		ESLAssert( pPatch != nullptr ) ;
		//
		SelectionWeights *	pWeights = m_aSelWeights.GetAt( iPatch ) ;
		if ( pWeights == nullptr )
		{
			pWeights = new SelectionWeights ;
			m_aSelWeights.SetAt( iPatch, pWeights ) ;
		}
		const size_t	nVertexCount = pPatch->GetTotalVertexCount() ;
		pWeights->SetLength( nVertexCount ) ;
		//
		float32_t *	pfpWeights = pWeights->GetArray() ;
		for ( size_t i = 0; i < nVertexCount; i ++ )
		{
			pfpWeights[i] = 0.0f ;
		}
	}
	//
	m_spsUVPoints.SortArray() ;
	m_spsUVPoints.NormalizeSorted() ;
	//
	SelectionWeights *	pWeights = nullptr ;
	Patch *				pSelPatch = nullptr ;
	const SelectPoint *	pspPoints = m_spsUVPoints.GetConstArray() ;
	const size_t		nPointCount = m_spsUVPoints.GetLength() ;
	//
	for ( size_t i = 0; i < nPointCount; i ++ )
	{
		const SelectPoint&	sp = pspPoints[i] ;
		if ( sp.pPatch != pSelPatch )
		{
			pWeights = m_aSelWeights.GetAt( (size_t) FindPatch( sp.pPatch ) ) ;
			ESLAssert( pWeights != nullptr ) ;
			if ( pWeights == nullptr )
			{
				pSelPatch = nullptr ;
				continue ;
			}
			pSelPatch = sp.pPatch ;
		}
		ESLAssert( sp.iVertex < pWeights->GetLength() ) ;
		pWeights->SetAt( sp.iVertex, sp.fpWeight ) ;
	}
}

void S3DMeshEditorObject::UpdateSelectedPointMinMax( void )
{
	const SelectPoint *	pspPoints = m_spsUVPoints.GetConstArray() ;
	const size_t		nPointCount = m_spsUVPoints.GetLength() ;
	//
	if ( nPointCount == 0 )
	{
		m_vSelMin = S3DVector( 0, 0, 0 ) ;
		m_vSelMax = S3DVector( 0, 0, 0 ) ;
		m_vSelUVMin = S2DVector( 0, 0 ) ;
		m_vSelUVMax = S2DVector( 0, 0 ) ;
		return ;
	}
	m_vSelMin = pspPoints[0].pPatch->GetPointAt( pspPoints[0].iVertex ) ;
	m_vSelMax = m_vSelMin ;
	m_vSelUVMin = pspPoints[0].pPatch->GetUVAt( pspPoints[0].iVertex ) ;
	m_vSelUVMax = m_vSelUVMin ;
	//
	for ( size_t i = 1; i < nPointCount; i ++ )
	{
		const SelectPoint&	sp = pspPoints[i] ;
		const S3DVector	vPoint = sp.pPatch->GetPointAt( sp.iVertex ) ;
		const S2DVector	vUV = sp.pPatch->GetUVAt( sp.iVertex ) ;
		m_vSelMin.x = esl_fminf( m_vSelMin.x, vPoint.x ) ;
		m_vSelMin.y = esl_fminf( m_vSelMin.y, vPoint.y ) ;
		m_vSelMin.z = esl_fminf( m_vSelMin.z, vPoint.z ) ;
		m_vSelMax.x = esl_fmaxf( m_vSelMax.x, vPoint.x ) ;
		m_vSelMax.y = esl_fmaxf( m_vSelMax.y, vPoint.y ) ;
		m_vSelMax.z = esl_fmaxf( m_vSelMax.z, vPoint.z ) ;
		m_vSelUVMin.x = esl_fminf( m_vSelUVMin.x, vUV.x ) ;
		m_vSelUVMin.y = esl_fminf( m_vSelUVMin.y, vUV.y ) ;
		m_vSelUVMax.x = esl_fmaxf( m_vSelUVMax.x, vUV.x ) ;
		m_vSelUVMax.y = esl_fmaxf( m_vSelUVMax.y, vUV.y ) ;
	}
}

void S3DMeshEditorObject::NormalizeSelecedPoints( void )
{
	m_spsUVPoints.SortArray() ;
	m_spsUVPoints.NormalizeSorted() ;
	//
	m_spsPoints = m_spsUVPoints ;
	NormalizeSelectPointSet( m_spsPoints ) ;
	//
	PatchFacesAndEdgesFromPoints
		( m_pfsFaces, m_pfsNullFaces, m_pesEdges, m_plsLines, m_spsPoints ) ;
	EnumerateSelectedPatchSet( m_asSelPatchs, m_spsUVPoints ) ;
	//
	UpdateSelectedPointWeights() ;
	UpdateSelectedPointMinMax() ;
}

// 稜線選択
//////////////////////////////////////////////////////////////////////////////
const S3DMeshEditor::PatchEdgeSet& S3DMeshEditorObject::GetSelectedEdges( void ) const
{
	return	m_pesEdges ;
}

void S3DMeshEditorObject::SetSelectedEdges( const S3DMeshEditor::PatchEdgeSet& selEdges )
{
	m_pesEdges = selEdges ;
}

void S3DMeshEditorObject::AddSelectedEdges( const S3DMeshEditor::PatchEdgeSet& selEdges )
{
	m_pesEdges.AppendSetNormalize( selEdges ) ;
}

void S3DMeshEditorObject::RemoveSelectedEdges( const S3DMeshEditor::PatchEdgeSet& selEdges )
{
	m_pesEdges.RemoveSetBySorted( selEdges ) ;
}

void S3DMeshEditorObject::UpdateSelecedPointByEdges( void )
{
	m_pesEdges.SortArray() ;
	m_pesEdges.NormalizeSorted() ;
	//
	SelectPointsFromEdges( m_spsUVPoints, m_pesEdges ) ;
}

void S3DMeshEditorObject::NormalizeSelecedEdges( void )
{
	UpdateSelecedPointByEdges() ;
	//
	m_spsPoints = m_spsUVPoints ;
	NormalizeSelectPointSet( m_spsPoints ) ;
	//
	S3DMeshEditor::PatchEdgeSet	pesEdgesTemp ;
	PatchFacesAndEdgesFromPoints
		( m_pfsFaces, m_pfsNullFaces, pesEdgesTemp, m_plsLines, m_spsPoints ) ;
	EnumerateSelectedPatchSet( m_asSelPatchs, m_spsUVPoints ) ;
	//
	UpdateSelectedPointWeights() ;
	UpdateSelectedPointMinMax() ;
}

// 面選択
//////////////////////////////////////////////////////////////////////////////
const S3DMeshEditor::PatchFaceSet& S3DMeshEditorObject::GetSelectedFaces( void ) const
{
	return	m_pfsFaces ;
}

const S3DMeshEditor::PatchFaceSet& S3DMeshEditorObject::GetSelectedNullFaces( void ) const
{
	return	m_pfsNullFaces ;
}

void S3DMeshEditorObject::SetSelectedFaces( const S3DMeshEditor::PatchFaceSet& selFaces )
{
	m_pfsFaces = selFaces ;
}

void S3DMeshEditorObject::AddSelectedFaces( const S3DMeshEditor::PatchFaceSet& selFaces )
{
	m_pfsFaces.AppendSetNormalize( selFaces ) ;
}

void S3DMeshEditorObject::RemoveSelectedFaces( const S3DMeshEditor::PatchFaceSet& selFaces )
{
	m_pfsFaces.RemoveSetBySorted( selFaces ) ;
}

void S3DMeshEditorObject::UpdateSelecedPointByFaces( void )
{
	m_pfsFaces.SortArray() ;
	m_pfsFaces.NormalizeSorted() ;
	//
	SelectPointsFromFaces( m_spsUVPoints, m_pfsFaces ) ;
}

void S3DMeshEditorObject::NormalizeSelecedFaces( void )
{
	UpdateSelecedPointByFaces() ;
	//
	m_spsPoints = m_spsUVPoints ;
	NormalizeSelectPointSet( m_spsPoints ) ;
	//
	S3DMeshEditor::PatchFaceSet	pfsFacesTemp ;
	PatchFacesAndEdgesFromPoints
		( pfsFacesTemp, m_pfsNullFaces, m_pesEdges, m_plsLines, m_spsPoints ) ;
	EnumerateSelectedPatchSet( m_asSelPatchs, m_spsUVPoints ) ;
	//
	UpdateSelectedPointWeights() ;
	UpdateSelectedPointMinMax() ;
}

// ライン選択
//////////////////////////////////////////////////////////////////////////////
const S3DMeshEditor::PatchLineSet& S3DMeshEditorObject::GetSelectedLines( void ) const
{
	return	m_plsLines ;
}

void S3DMeshEditorObject::SetSelectedLines( const S3DMeshEditor::PatchLineSet& selLines )
{
	m_plsLines = selLines ;
}

void S3DMeshEditorObject::AddSelectedLines( const S3DMeshEditor::PatchLineSet& selLines )
{
	m_plsLines.AppendSetNormalize( selLines ) ;
}

void S3DMeshEditorObject::RemoveSelectedLines( const S3DMeshEditor::PatchLineSet& selLines )
{
	m_plsLines.RemoveSetBySorted( selLines ) ;
}

void S3DMeshEditorObject::UpdateSelecedPointByLines( void )
{
	m_plsLines.SortArray() ;
	m_plsLines.NormalizeSorted() ;
	//
	SelectPointsFromLines( m_spsUVPoints, m_plsLines ) ;
}

void S3DMeshEditorObject::NormalizeSelecedLines( void )
{
	UpdateSelecedPointByLines() ;
	//
	m_spsPoints = m_spsUVPoints ;
	NormalizeSelectPointSet( m_spsPoints ) ;
	//
	S3DMeshEditor::PatchLineSet	plsLinesTemp ;
	PatchFacesAndEdgesFromPoints
		( m_pfsFaces, m_pfsNullFaces, m_pesEdges, plsLinesTemp, m_spsPoints ) ;
	EnumerateSelectedPatchSet( m_asSelPatchs, m_spsUVPoints ) ;
	//
	UpdateSelectedPointWeights() ;
	UpdateSelectedPointMinMax() ;
}

// パッチ選択
//////////////////////////////////////////////////////////////////////////////
const SSystem::SArraySet<S3DMeshEditor::Patch*>& S3DMeshEditorObject::GetSelectedPatchs( void ) const
{
	return	m_asSelPatchs ;
}

void S3DMeshEditorObject::SetSelectedPatchs( const SSystem::SArraySet<S3DMeshEditor::Patch*>& selPatchs )
{
	m_asSelPatchs = selPatchs ;
}

void S3DMeshEditorObject::AddSelectedPatchs( const SSystem::SArraySet<S3DMeshEditor::Patch*>& selPatchs )
{
	m_asSelPatchs.AppendSetNormalize( selPatchs ) ;
}

void S3DMeshEditorObject::RemoveSelectedPatchs( const SSystem::SArraySet<S3DMeshEditor::Patch*>& selPatchs )
{
	m_asSelPatchs.RemoveSetBySorted( selPatchs ) ;
}

void S3DMeshEditorObject::UpdateSelecedPointByPatchs( void )
{
	m_asSelPatchs.SortArray() ;
	m_asSelPatchs.NormalizeSorted() ;
	//
	SelectPointsFromPatchs( m_spsUVPoints, m_asSelPatchs ) ;
}

void S3DMeshEditorObject::NormalizeSelecedPatchs( void )
{
	UpdateSelecedPointByPatchs() ;
	//
	m_spsPoints = m_spsUVPoints ;
	NormalizeSelectPointSet( m_spsPoints ) ;
	//
	PatchFacesAndEdgesFromPoints
		( m_pfsFaces, m_pfsNullFaces, m_pesEdges, m_plsLines, m_spsPoints ) ;
	//
	UpdateSelectedPointWeights() ;
	UpdateSelectedPointMinMax() ;
}

// 選択1要素取得
//////////////////////////////////////////////////////////////////////////////
const S3DMeshEditor::SelectPoint *
		S3DMeshEditorObject::GetSelectedPoint( void ) const
{
	return	m_spsUVPoints.GetAt( 0 ) ;
}

const S3DMeshEditor::PatchLine *
		S3DMeshEditorObject::GetSelectedLine( void ) const
{
	return	m_plsLines.GetAt( 0 ) ;
}

// 稜線集合の中から指定頂点を含む稜線検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DMeshEditorObject::FindEdgeIncludePointAs
	( const S3DMeshEditor::PatchEdgeSet& pes, const S3DMeshEditor::PatchPoint& pp )
{
	for ( size_t i = 0; i < pes.GetLength(); i ++ )
	{
		S3DMeshEditor::PatchEdge	edge = pes.At(i) ;
		if ( edge.pPatch == pp.pPatch )
		{
			if ( (edge.iVertex0 == pp.iVertex)
				|| (edge.iVertex1 == pp.iVertex) )
			{
				return	(ssize_t) i ;
			}
		}
	}
	return	-1 ;
}

// 選択面輪郭取得
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::GetEdgeChainOfFaceOutline
	( SObjectArray<S3DMeshEditorObject::EdgeChain>& aEdgeChains,
			const S3DMeshEditor::PatchFaceSet& selFaces ) const
{
	S3DMeshEditor::PatchEdgeSet		selEdges ;
	S3DMeshEditor::PatchEdgeSet		selEdges2 ;
	S3DMeshEditor::PatchPointSet	ppsTemp ;
	//
	// 全ての面の稜線を列挙
	//
	static const size_t	iEdgePoints[4][2] =
	{
		{ 0, 1 }, { 1, 3 }, { 0, 2 }, { 2, 3 }
	} ;
	for ( size_t i = 0; i < selFaces.GetLength(); i ++ )
	{
		S3DMeshEditor::PatchFace	pf = selFaces.At(i) ;
		if ( pf.iFace >= pf.pPatch->GetAreaSize() )
		{
			continue ;
		}
		size_t	iQuad[4] ;
		pf.pPatch->QuadIndexesFromFaceIndex( iQuad, pf.iFace ) ;
		//
		for ( size_t j = 0; j < 4; j ++ )
		{
			S3DMeshEditor::PatchEdge	edge
				( pf.pPatch, iQuad[iEdgePoints[j][0]],
									iQuad[iEdgePoints[j][1]] ) ;
			if ( selEdges.FindSorted( edge ) < 0 )
			{
				selEdges.AddSorted( edge ) ;
			}
			else
			{
				selEdges2.QuickAdd( edge ) ;
			}
		}
	}
	for ( size_t i = 0; i < selFaces.GetLength(); i ++ )
	{
		S3DMeshEditor::PatchFace	pf = selFaces.At(i) ;
		if ( pf.iFace >= pf.pPatch->GetAreaSize() )
		{
			continue ;
		}
		size_t	iQuad[4] ;
		pf.pPatch->QuadIndexesFromFaceIndex( iQuad, pf.iFace ) ;
		//
		for ( size_t j = 0; j < 4; j ++ )
		{
			S3DMeshEditor::PatchEdge	edge
				( pf.pPatch, iQuad[iEdgePoints[j][0]],
									iQuad[iEdgePoints[j][1]] ) ;
			for ( size_t k = 0; k < 2; k ++ )
			{
				ppsTemp.RemoveAll() ;
				if ( GetDegeneratedPoints
						( ppsTemp, S3DMeshEditor::PatchPoint
							( pf.pPatch, iQuad[iEdgePoints[j][k]] ) ) )
				{
					ppsTemp.SortArray() ;
					ppsTemp.NormalizeSorted() ;
					//
					for ( size_t k0 = 0; k0 < ppsTemp.GetLength(); k0 ++ )
					{
						S3DMeshEditor::PatchPoint	pp0 = ppsTemp.At(k0) ;
						for ( size_t k1 = k0 + 1; k1 < ppsTemp.GetLength(); k1 ++ )
						{
							S3DMeshEditor::PatchPoint	pp1 = ppsTemp.At(k1) ;
							if ( (pp0.pPatch == pp1.pPatch)
								&& pp0.pPatch->IsPatchEdge( pp0.iVertex, pp1.iVertex ) )
							{
								S3DMeshEditor::PatchEdge
									pe( pp0.pPatch, pp0.iVertex, pp1.iVertex ) ;
								if ( pe != edge )
								{
									if ( selEdges.FindSorted( pe ) < 0 )
									{
										selEdges.AddSorted( pe ) ;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	selEdges2.SortArray() ;
	selEdges2.NormalizeSorted() ;
	//
	// 二重になっていない稜線のみを抽出
	//
	selEdges.RemoveSetBySorted( selEdges2 ) ;
	//
	// 稜線連鎖
	//
	SArray<PatchEdge>					aEdgeLog ;
	S3DMeshEditorObject::EdgeChain *	pChain = nullptr ;
	S3DMeshEditor::PatchPoint			ppLast( nullptr, 0 ) ;
	while ( selEdges.GetLength() > 0 )
	{
		if ( pChain == nullptr )
		{
			pChain = new S3DMeshEditorObject::EdgeChain ;
			//
			S3DMeshEditor::PatchEdge	edge = selEdges.At(0) ;
			ppLast = S3DMeshEditor::PatchPoint( edge.pPatch, edge.iVertex1 ) ;
			pChain->Add( S3DMeshEditor::PatchPoint( edge.pPatch, edge.iVertex0 ) ) ;
			pChain->Add( ppLast ) ;
			//
			aEdgeLog.RemoveAll() ;
			aEdgeLog.Add( edge ) ;
			aEdgeLog.Add( edge ) ;
			//
			selEdges.RemoveAt(0) ;
			continue ;
		}
		S3DMeshEditor::PatchPoint	ppTemp = ppLast ;
		ssize_t	iEdge = FindEdgeIncludePointAs( selEdges, ppLast ) ;
		if ( iEdge < 0 )
		{
			if ( pChain->GetLength() <= 1 )
			{
				delete	pChain ;
				pChain = nullptr ;
				continue ;
			}
			pChain->Pop() ;
			ppLast = pChain->LastAt(0) ;
			continue ;
		}
		S3DMeshEditor::PatchEdge	edge = selEdges.At( (size_t) iEdge ) ;
		ESLAssert( edge.pPatch == ppTemp.pPatch ) ;
		if ( edge.iVertex0 == ppTemp.iVertex )
		{
			ppLast.pPatch = edge.pPatch ;
			ppLast.iVertex = edge.iVertex1 ;
		}
		else
		{
			ppLast.pPatch = edge.pPatch ;
			ppLast.iVertex = edge.iVertex0 ;
		}
		pChain->Add( ppLast ) ;
		aEdgeLog.Add( edge ) ;
		selEdges.RemoveAt( (size_t) iEdge ) ;
		//
		for ( size_t i = 0; i + 2 < pChain->GetLength(); i ++ )
		{
			if ( pChain->At(i) == ppLast )
			{
				if ( i > 0 )
				{
					for ( size_t j = 1; j <= i; j ++ )
					{
						selEdges.AddSorted( aEdgeLog.At(j) ) ;
					}
					pChain->Remove( 0, i ) ;
				}
				aEdgeChains.Add( pChain ) ;
				pChain = nullptr ;
				break ;
			}
		}
	}
	delete	pChain ;
	//
	NormalizeEdgeChainOrders( aEdgeChains, selFaces ) ;
}

// 輪郭順序正規化
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::NormalizeEdgeChainOrders
	( SObjectArray<S3DMeshEditorObject::EdgeChain>& aEdgeChains,
			const S3DMeshEditor::PatchFaceSet& selFaces )
{
	for ( size_t i = 0; i < aEdgeChains.GetLength(); i ++ )
	{
		S3DMeshEditorObject::EdgeChain *	pChain = aEdgeChains.GetAt( i ) ;
		ESLAssert( pChain != nullptr ) ;
		if ( pChain != nullptr )
		{
			NormalizeEdgeChainOrder( *pChain, selFaces ) ;
		}
	}
}

void S3DMeshEditorObject::NormalizeEdgeChainOrder
	( S3DMeshEditorObject::EdgeChain& edgeChain,
			const S3DMeshEditor::PatchFaceSet& selFaces )
{
	if ( edgeChain.GetLength() == 0 )
	{
		return ;
	}
	//
	// 輪郭線の右側[0]と左側[1]のどちらに面が存在しているか？
	//
	size_t	nSideFace[2] = { 0, 0 } ;
	for ( size_t i = 1; i < edgeChain.GetLength(); i ++ )
	{
		S3DMeshEditor::PatchPoint	pp0 = edgeChain.At( i - 1 ) ;
		S3DMeshEditor::PatchPoint	pp1 = edgeChain.At( i ) ;
		if ( pp0.pPatch != pp1.pPatch )
		{
			continue ;
		}
		size_t	iFaces[2] ;
		if ( !pp0.pPatch->GetSideFaceByEdge( iFaces, pp0.iVertex, pp1.iVertex ) )
		{
			continue ;
		}
		if ( selFaces.FindSorted
			( S3DMeshEditor::PatchFace( pp0.pPatch, iFaces[0] ) ) >= 0 )
		{
			if ( pp0.pPatch->GetFaceAt( iFaces[0] )
							!= S3DMeshEditor::Patch::faceNull )
			{
				nSideFace[0] ++ ;
			}
		}
		if ( selFaces.FindSorted
			( S3DMeshEditor::PatchFace( pp0.pPatch, iFaces[1] ) ) >= 0 )
		{
			if ( pp0.pPatch->GetFaceAt( iFaces[1] )
							!= S3DMeshEditor::Patch::faceNull )
			{
				nSideFace[1] ++ ;
			}
		}
	}
	//
	// 輪郭線の左手に面があるのが正しい向き
	//
	if ( nSideFace[0] > nSideFace[1] )
	{
		ESLAssert( edgeChain.GetLength() >= 1 ) ;
		size_t	i = 0 ;
		size_t	j = edgeChain.GetLength() - 1 ;
		while ( i < j )
		{
			edgeChain.Swap( i ++, j -- ) ;
		}
	}
}

// 輪郭線が単一パッチの単一ラインか？
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditorObject::IsEdgeChainSinglePatchLine
	( S3DMeshEditor::PatchLine& pl,
		const S3DMeshEditorObject::EdgeChain& edgeChain )
{
	if ( edgeChain.GetLength() == 0 )
	{
		return	false ;
	}
	S3DMeshEditor::PatchPoint	pp0 = edgeChain.At(0) ;
	bool		flagVert = (pp0.pPatch->GetHeight() == edgeChain.GetLength()) ;
	bool		flagHorz = (pp0.pPatch->GetWidth() == edgeChain.GetLength()) ;
	SGLPoint	pt0 ;
	pp0.pPatch->PointFromIndex( pt0, (uint32_t) pp0.iVertex ) ;
	//
	for ( size_t i = 1; i < edgeChain.GetLength(); i ++ )
	{
		if ( !flagVert && !flagHorz )
		{
			return	false ;
		}
		S3DMeshEditor::PatchPoint	pp = edgeChain.At(i) ;
		if ( pp.pPatch != pp0.pPatch )
		{
			return	false ;
		}
		SGLPoint	pt ;
		pp0.pPatch->PointFromIndex( pt, (uint32_t) pp.iVertex ) ;
		if ( pt.x != pt0.x )
		{
			flagVert = false ;
		}
		if ( pt.y != pt0.y )
		{
			flagHorz = false ;
		}
	}
	if ( flagHorz )
	{
		pl.pPatch = pp0.pPatch ;
		pl.lineDir = S3DMeshEditor::lineHorizontal ;
		pl.iLine = (size_t) pt0.y ;
		return	true ;
	}
	if ( flagVert )
	{
		pl.pPatch = pp0.pPatch ;
		pl.lineDir = S3DMeshEditor::lineVertical ;
		pl.iLine = (size_t) pt0.x ;
		return	true ;
	}
	return	false ;
}

// 面部分選択か全体選択か？
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditorObject::IsFullSelectedFace
	( const S3DMeshEditor::Patch& patch,
		const S3DMeshEditor::PatchFaceSet& selFaces )
{
	const size_t	nFaceCount = patch.GetTotalFaceCount() ;
	for ( size_t i = 0; i < nFaceCount; i ++ )
	{
		if ( patch.IsValidFaceAreaAt( i )
			&& (patch.GetFaceAt( i ) != S3DMeshEditor::Patch::faceNull) )
		{
			if ( selFaces.FindSorted
				( S3DMeshEditor::PatchFace
					( (S3DMeshEditor::Patch*) &patch, i ) ) < 0 )
			{
				return	false ;
			}
		}
	}
	return	true ;
}

// 面選択部分のみを複製して新規パッチ作成
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditor::Patch * S3DMeshEditorObject::DuplicatePartialPatch
	( SGLPoint& ptOffset,
		const S3DMeshEditor::Patch& patch,
		const S3DMeshEditor::PatchFaceSet& selFaces )
{
	if ( patch.GetAreaSize() == 0 )
	{
		return	nullptr ;
	}
	SGLRect	rectSel( (int) patch.GetWidth(), (int) patch.GetHeight(), 0, 0 ) ;
	for ( size_t i = 0; i < selFaces.GetLength(); i ++ )
	{
		S3DMeshEditor::PatchFace	pf = selFaces.At(i) ;
		if ( pf.pPatch != &patch )
		{
			continue ;
		}
		if ( !pf.pPatch->IsValidFaceAreaAt( pf.iFace )
			|| (pf.pPatch->GetFaceAt( pf.iFace ) == S3DMeshEditor::Patch::faceNull) )
		{
			continue ;
		}
		SGLPoint	pt ;
		patch.PointFromIndex( pt, (uint32_t) pf.iFace ) ;
		if ( pt.x < rectSel.left )
		{
			rectSel.left = pt.x ;
		}
		if ( pt.y < rectSel.top )
		{
			rectSel.top = pt.y ;
		}
		if ( pt.x > rectSel.right )
		{
			rectSel.right = pt.x ;
		}
		if ( pt.y > rectSel.bottom )
		{
			rectSel.bottom = pt.y ;
		}
	}
	if ( rectSel.IsEmpty() )
	{
		return	nullptr ;
	}
	SGLImageRect	irctSel = rectSel ;
	irctSel.w ++ ;
	irctSel.h ++ ;
	//
	S3DMeshEditor::Patch *	pPatch = NewPatch() ;
	pPatch->SetName( patch.GetName() ) ;
	pPatch->CreatePatch( (size_t) irctSel.w, (size_t) irctSel.h ) ;
	pPatch->InsertWeightLayer( 0, patch.GetWeightLayerCount() ) ;
	//
	SGLImageRect	irctNormal = irctSel ;
	if ( irctNormal.x + irctNormal.w > (int) patch.GetWidth() )
	{
		irctNormal.w = (int) patch.GetWidth() - irctNormal.x ;
	}
	if ( irctNormal.y + irctNormal.h > (int) patch.GetHeight() )
	{
		irctNormal.h = (int) patch.GetHeight() - irctNormal.y ;
	}
	pPatch->CopyRect( 0, 0, patch, irctNormal ) ;
	pPatch->ClearAllDegeneration() ;
	//
	for ( size_t y = 0; y < (size_t) irctSel.h; y ++ )
	{
		for ( size_t x = 0; x < (size_t) irctSel.w; x ++ )
		{
			S3DMeshEditor::PatchFace
				pf( (S3DMeshEditor::Patch*) &patch,
						patch.IndexFromPoint
							( (x + (size_t) irctSel.x) % patch.GetWidth(),
								(y + (size_t) irctSel.y) %patch.GetHeight() ) ) ;
			if ( !pPatch->IsValidFaceArea( x, y )
				|| (selFaces.FindSorted( pf ) < 0) )
			{
				pPatch->SetFace( x, y, S3DMeshEditor::Patch::faceNull ) ;
			}
		}
	}
	//
	ptOffset = irctSel.GetPosition() ;
	return	pPatch ;
}

// 面選択部分の面更新
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::ChangeSelectedFace
	( S3DMeshEditor::Patch& patch, int8_t face,
		const S3DMeshEditor::PatchFaceSet& selFaces )
{
	size_t	nWidth = patch.GetWidth() ;
	size_t	nHeight = patch.GetHeight() ;
	for ( size_t y = 0; y < nHeight; y ++ )
	{
		for ( size_t x = 0; x < nWidth; x ++ )
		{
			S3DMeshEditor::PatchFace
				pf( (S3DMeshEditor::Patch*) &patch,
						patch.IndexFromPoint( x, y ) ) ;
			if ( patch.IsValidFaceArea( x, y )
				&& (selFaces.FindSorted( pf ) >= 0) )
			{
				patch.SetFace( x, y, face ) ;
			}
		}
	}
}

// 面非選択部分の面更新
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::ChangeUnselectedFace
	( S3DMeshEditor::Patch& patch, int8_t face,
		const S3DMeshEditor::PatchFaceSet& selFaces )
{
	size_t	nWidth = patch.GetWidth() ;
	size_t	nHeight = patch.GetHeight() ;
	for ( size_t y = 0; y < nHeight; y ++ )
	{
		for ( size_t x = 0; x < nWidth; x ++ )
		{
			S3DMeshEditor::PatchFace
				pf( (S3DMeshEditor::Patch*) &patch,
						patch.IndexFromPoint( x, y ) ) ;
			if ( patch.IsValidFaceArea( x, y )
				&& (selFaces.FindSorted( pf ) < 0) )
			{
				patch.SetFace( x, y, face ) ;
			}
		}
	}
}

// 編集コンテキスト
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::AttachEditContext( ESLObject * pContext, bool flagAutoDelete )
{
	if ( m_pEditContext && m_flagOwnEditContext )
	{
		delete	m_pEditContext ;
	}
	m_pEditContext = pContext ;
	m_flagOwnEditContext = flagAutoDelete ;
}

ESLObject * S3DMeshEditorObject::GetEditContext( void ) const
{
	return	m_pEditContext ;
}

// 選択頂点座標操作
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::DoTransformPoints
	( const S3DMatrix& matTrans, const S3DVector& vMove,
		S3DMeshEditorInterface * pEdit )
{
	TransformPoints( m_spsPoints, matTrans, vMove ) ;
	ShrinkDegeneratedPoints() ;
	//
	if ( pEdit != nullptr )
	{
		pEdit->NotifyUpdateVertex( m_spsPoints ) ;
	}
}

void S3DMeshEditorObject::DoTransformUVPoints
	( const S3DMatrix& matTrans, const S3DVector& vMove,
		S3DMeshEditorInterface * pEdit )
{
	TransformUVPoints( m_spsUVPoints, matTrans, vMove ) ;
	//
	if ( pEdit != nullptr )
	{
		pEdit->NotifyUpdateVertex( m_spsUVPoints ) ;
	}
}

// 選択点の縮退フラグ取得
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DMeshEditorObject::GetSelectedDegenerateFlag( void ) const
{
	PatchPointSet	pps ;
	PatchPointsFromSelectedPoints( pps, m_spsPoints ) ;
	return	GetDegeneratedPointFlag( pps ) ;
}

// 選択点の縮退フラグ変更
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::DoChangeSelectedDegenerateFlag
		( uint32_t nFlags, S3DMeshEditorInterface * pEdit )
{
	PatchPointSet	pps ;
	PatchPointsFromSelectedPoints( pps, m_spsPoints ) ;
	//
	if ( pEdit != nullptr )
	{
		pEdit->ChangeDegeneratedPointFlag( pps, nFlags ) ;
	}
	else
	{
		SetDegeneratedPointFlag( pps, nFlags ) ;
	}
}

// 選択頂点縮退
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::DoShrink( uint32_t nFlags, S3DMeshEditorInterface * pEdit )
{
	PatchPointSet	pps ;
	PatchPointsFromSelectedPoints( pps, m_spsPoints ) ;
	//
	if ( pEdit != nullptr )
	{
		pEdit->ShrinkPoints( pps, nFlags ) ;
	}
	else
	{
		ShrinkPoints( pps, nFlags ) ;
	}
	BundlePoints( pps ) ;
	NormalizeSelectPointSet( m_spsPoints ) ;
	//
	if ( pEdit != nullptr )
	{
		pEdit->NotifyUpdateVertex( m_spsPoints ) ;
	}
}

// 近接頂点縮退
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::DoShrinkNearPoints
	( const S3DMeshEditorObject::ShrinkParam& param,
			uint32_t nFlags, S3DMeshEditorInterface * pEdit )
{
	SBitArray		baProcessed ;
	const size_t	nSelPointCount = m_spsUVPoints.GetLength() ;
	baProcessed.SetLength( nSelPointCount ) ;
	//
	SelectPointSet	spsNearPoints ;
	PatchPointSet	ppsNearPoints ;
	for ( size_t i = 0; i < nSelPointCount; i ++ )
	{
		//
		// 近接点収集
		//
		const SelectPoint&	sp = m_spsUVPoints.At(i) ;
		S3DVector	vPos = sp.pPatch->GetPointAt( sp.iVertex ) ;
		SGLPoint	ptVertex( 0, 0 ) ;
		if ( (param.target != shrinkAny)
			&& (sp.iVertex >= sp.pPatch->GetAreaSize()) )
		{
			continue ;
		}
		if ( sp.iVertex < sp.pPatch->GetAreaSize() )
		{
			sp.pPatch->PointFromIndex( ptVertex, (uint32_t) sp.iVertex ) ;
		}
		for ( size_t j = i + 1; j < nSelPointCount; j ++ )
		{
			if ( baProcessed.GetAt(j) )
			{
				continue ;
			}
			const SelectPoint&	sp2 = m_spsUVPoints.At(j) ;
			if ( param.target != shrinkAny )
			{
				if ( (sp.pPatch != sp2.pPatch)
					|| (sp2.iVertex >= sp2.pPatch->GetAreaSize()) )
				{
					continue ;
				}
				SGLPoint	ptVertex2 ;
				sp2.pPatch->PointFromIndex( ptVertex2, (uint32_t) sp2.iVertex ) ;
				if ( param.target == shrinkHorz )
				{
					if ( ptVertex.y != ptVertex2.y )
					{
						continue ;
					}
				}
				else if ( param.target == shrinkVert )
				{
					if ( ptVertex.x != ptVertex2.x )
					{
						continue ;
					}
				}
			}
			S3DVector	vPos2 = sp2.pPatch->GetPointAt( sp2.iVertex ) ;
			if ( (vPos2 - vPos).Absolute() <= param.gap )
			{
				baProcessed.SetAt( j, true ) ;
				spsNearPoints.QuickAdd( sp2 ) ;
			}
		}
		if ( spsNearPoints.GetLength() == 0 )
		{
			continue ;
		}
		spsNearPoints.QuickAdd( sp ) ;
		NormalizeSelectPointSet( spsNearPoints ) ;
		//
		// 近接点収束
		//
		PatchPointsFromSelectedPoints( ppsNearPoints, spsNearPoints ) ;
		if ( pEdit != nullptr )
		{
			pEdit->ShrinkPoints( ppsNearPoints, nFlags ) ;
		}
		else
		{
			ShrinkPoints( ppsNearPoints, nFlags ) ;
		}
		BundlePoints( ppsNearPoints ) ;
		//
		if ( pEdit != nullptr )
		{
			pEdit->NotifyUpdateVertex( spsNearPoints ) ;
		}
		spsNearPoints.RemoveAll() ;
		ppsNearPoints.RemoveAll() ;
	}
	NormalizeSelectPointSet( m_spsPoints ) ;
}

// 選択頂点縮退解除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::DoUntiShrink( S3DMeshEditorInterface * pEdit )
{
	for ( size_t i = 0; i < m_spsUVPoints.GetLength(); i ++ )
	{
		if ( pEdit != nullptr )
		{
			pEdit->UntiShrinkPoints( m_spsUVPoints.At(i) ) ;
		}
		else
		{
			UntiShrinkPoints( m_spsUVPoints.At(i) ) ;
		}
	}
	NormalizeSelecedPoints() ;
}

// 頂点整列
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::DoArrange
	( const S3DMeshEditorObject::ArrangeParam& param, S3DMeshEditorInterface * pEdit )
{
	if ( m_spsUVPoints.GetLength() == 0 )
	{
		return ;
	}
	PFUNC_GET_VECTOR_ELEMENT	pfnGetElement = m_pfnGetVectorElement[param.axis] ;
	float32_t	num =
		pfnGetElement( m_spsUVPoints.At(0).pPatch->GetPointAt
									( m_spsUVPoints.At(0).iVertex ),
						m_spsUVPoints.At(0).pPatch->GetUVAt
									( m_spsUVPoints.At(0).iVertex ) ) ;
	if ( param.op == arrangeMin )
	{
		for ( size_t i = 1; i < m_spsUVPoints.GetLength(); i ++ )
		{
			const SelectPoint&	sp = m_spsUVPoints.At(i) ;
			num = esl_fminf( num, pfnGetElement
								( sp.pPatch->GetPointAt( sp.iVertex ),
									sp.pPatch->GetUVAt( sp.iVertex ) ) ) ;
		}
	}
	else if ( param.op == arrangeMax )
	{
		for ( size_t i = 1; i < m_spsUVPoints.GetLength(); i ++ )
		{
			const SelectPoint&	sp = m_spsUVPoints.At(i) ;
			num = esl_fmaxf( num, pfnGetElement
								( sp.pPatch->GetPointAt( sp.iVertex ),
									sp.pPatch->GetUVAt( sp.iVertex ) ) ) ;
		}
	}
	else if ( param.op == arrangeCenter )
	{
		float32_t	nMin = num ;
		float32_t	nMax = num ;
		for ( size_t i = 1; i < m_spsUVPoints.GetLength(); i ++ )
		{
			const SelectPoint&	sp = m_spsUVPoints.At(i) ;
			float32_t	n = pfnGetElement
								( sp.pPatch->GetPointAt( sp.iVertex ),
									sp.pPatch->GetUVAt( sp.iVertex ) ) ;
			nMin = esl_fminf( nMin, n ) ;
			nMax = esl_fmaxf( nMax, n ) ;
		}
		num = (nMin + nMax) * 0.5f ;
	}
	else
	{
		num = param.num ;
	}
	//
	PFUNC_SET_VECTOR_ELEMENT	pfnSetElement = m_pfnSetVectorElement[param.axis] ;
	for ( size_t i = 0; i < m_spsPoints.GetLength(); i ++ )
	{
		const SelectPoint&	sp = m_spsPoints.At(i) ;
		S3DVector	v = sp.pPatch->GetPointAt( sp.iVertex ) ;
		S2DVector	uv = sp.pPatch->GetUVAt( sp.iVertex ) ;
		pfnSetElement( v, uv, num ) ;
		sp.pPatch->SetPointAt( sp.iVertex, v ) ;
		sp.pPatch->SetUVAt( sp.iVertex, uv ) ;
	}
	//
	SetUpdateVertexFlag() ;
	//
	if ( pEdit != nullptr )
	{
		pEdit->NotifyUpdateVertex( m_spsPoints ) ;
	}
}

const S3DMeshEditorObject::PFUNC_GET_VECTOR_ELEMENT
	S3DMeshEditorObject::m_pfnGetVectorElement[5] =
{
	&S3DMeshEditorObject::GetVectorElementOfX,
	&S3DMeshEditorObject::GetVectorElementOfY,
	&S3DMeshEditorObject::GetVectorElementOfZ,
	&S3DMeshEditorObject::GetVectorElementOfU,
	&S3DMeshEditorObject::GetVectorElementOfV,
} ;

const S3DMeshEditorObject::PFUNC_SET_VECTOR_ELEMENT
	S3DMeshEditorObject::m_pfnSetVectorElement[5] =
{
	&S3DMeshEditorObject::SetVectorElementOfX,
	&S3DMeshEditorObject::SetVectorElementOfY,
	&S3DMeshEditorObject::SetVectorElementOfZ,
	&S3DMeshEditorObject::SetVectorElementOfU,
	&S3DMeshEditorObject::SetVectorElementOfV,
} ;

float32_t S3DMeshEditorObject::GetVectorElementOfX( const S3DVector& v, const S2DVector& uv )
{
	return	v.x ;
}

float32_t S3DMeshEditorObject::GetVectorElementOfY( const S3DVector& v, const S2DVector& uv )
{
	return	v.y ;
}

float32_t S3DMeshEditorObject::GetVectorElementOfZ( const S3DVector& v, const S2DVector& uv )
{
	return	v.z ;
}

float32_t S3DMeshEditorObject::GetVectorElementOfU( const S3DVector& v, const S2DVector& uv )
{
	return	uv.x ;
}

float32_t S3DMeshEditorObject::GetVectorElementOfV( const S3DVector& v, const S2DVector& uv )
{
	return	uv.y ;
}

void S3DMeshEditorObject::SetVectorElementOfX( S3DVector& v, S2DVector& uv, float32_t n )
{
	v.x = n ;
}

void S3DMeshEditorObject::SetVectorElementOfY( S3DVector& v, S2DVector& uv, float32_t n )
{
	v.y = n ;
}

void S3DMeshEditorObject::SetVectorElementOfZ( S3DVector& v, S2DVector& uv, float32_t n )
{
	v.z = n ;
}

void S3DMeshEditorObject::SetVectorElementOfU( S3DVector& v, S2DVector& uv, float32_t n )
{
	uv.x = n ;
}

void S3DMeshEditorObject::SetVectorElementOfV( S3DVector& v, S2DVector& uv, float32_t n )
{
	uv.y = n ;
}

// 頂点要素フィル
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::DoFillElementOfColorMul
		( const SGLPalette& rgb, S3DMeshEditorInterface * pEdit )
{
	for ( size_t i = 0; i < m_spsPoints.GetLength(); i ++ )
	{
		const SelectPoint&	sp = m_spsPoints.At(i) ;
		S3DColor	color = sp.pPatch->GetColorAt( sp.iVertex ) ;
		color.rgbMul.ui32 =
			(rgb.ui32 & 0x00FFFFFF) | (color.rgbMul.ui32 & 0xFF000000) ;
		sp.pPatch->SetColorAt( sp.iVertex, color ) ;
	}
	//
	SetUpdateVertexFlag() ;
	//
	if ( pEdit != nullptr )
	{
		pEdit->NotifyUpdateVertex( m_spsPoints ) ;
	}
}

void S3DMeshEditorObject::DoFillElementOfColorAdd
		( const SGLPalette& rgb, S3DMeshEditorInterface * pEdit )
{
	for ( size_t i = 0; i < m_spsPoints.GetLength(); i ++ )
	{
		const SelectPoint&	sp = m_spsPoints.At(i) ;
		S3DColor	color = sp.pPatch->GetColorAt( sp.iVertex ) ;
		color.rgbAdd.ui32 =
			(rgb.ui32 & 0x00FFFFFF) | (color.rgbAdd.ui32 & 0xFF000000) ;
		sp.pPatch->SetColorAt( sp.iVertex, color ) ;
	}
	//
	SetUpdateVertexFlag() ;
	//
	if ( pEdit != nullptr )
	{
		pEdit->NotifyUpdateVertex( m_spsPoints ) ;
	}
}

void S3DMeshEditorObject::DoFillElementOfColorAlpha
		( uint8_t alpha, S3DMeshEditorInterface * pEdit )
{
	for ( size_t i = 0; i < m_spsPoints.GetLength(); i ++ )
	{
		const SelectPoint&	sp = m_spsPoints.At(i) ;
		S3DColor	color = sp.pPatch->GetColorAt( sp.iVertex ) ;
		color.rgbMul.argb.Alpha = alpha ;
		sp.pPatch->SetColorAt( sp.iVertex, color ) ;
	}
	//
	SetUpdateVertexFlag() ;
	//
	if ( pEdit != nullptr )
	{
		pEdit->NotifyUpdateVertex( m_spsPoints ) ;
	}
}

void S3DMeshEditorObject::DoFillElementOfColorAddAlpha
		( uint8_t alpha, S3DMeshEditorInterface * pEdit )
{
	for ( size_t i = 0; i < m_spsPoints.GetLength(); i ++ )
	{
		const SelectPoint&	sp = m_spsPoints.At(i) ;
		S3DColor	color = sp.pPatch->GetColorAt( sp.iVertex ) ;
		color.rgbAdd.argb.Alpha = alpha ;
		sp.pPatch->SetColorAt( sp.iVertex, color ) ;
	}
	//
	SetUpdateVertexFlag() ;
	//
	if ( pEdit != nullptr )
	{
		pEdit->NotifyUpdateVertex( m_spsPoints ) ;
	}
}

void S3DMeshEditorObject::DoFillElementOfWeightMap
		( size_t iWeight, float32_t w, S3DMeshEditorInterface * pEdit )
{
	if ( iWeight >= GetWeightLayerCount() )
	{
		return ;
	}
	for ( size_t i = 0; i < m_spsPoints.GetLength(); i ++ )
	{
		const SelectPoint&	sp = m_spsPoints.At(i) ;
		sp.pPatch->SetWeightAt( sp.iVertex, iWeight, w ) ;
	}
	//
	SetUpdateVertexFlag() ;
	//
	if ( pEdit != nullptr )
	{
//		pEdit->NotifyUpdateVertex( m_spsPoints ) ;
	}
}

// 頂点要素サンプリング
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditorObject::DoSampleElementOfColorMul( SGLPalette& rgb ) const
{
	if ( m_spsPoints.GetLength() == 0 )
	{
		return	false ;
	}
	int	b = 0, g = 0, r = 0 ;
	for ( size_t i = 0; i < m_spsPoints.GetLength(); i ++ )
	{
		const SelectPoint&	sp = m_spsPoints.At(i) ;
		S3DColor	color = sp.pPatch->GetColorAt( sp.iVertex ) ;
		b += color.rgbMul.argb.Blue ;
		g += color.rgbMul.argb.Green ;
		r += color.rgbMul.argb.Red ;
	}
	int	n = (int) m_spsPoints.GetLength() ;
	rgb.argb.Blue = (uint8_t) esl_clampi( b / n, 0, 0xFF ) ;
	rgb.argb.Green = (uint8_t) esl_clampi( g / n, 0, 0xFF ) ;
	rgb.argb.Red = (uint8_t) esl_clampi( r / n, 0, 0xFF ) ;
	return	true ;
}

bool S3DMeshEditorObject::DoSampleElementOfColorAdd( SGLPalette& rgb ) const
{
	if ( m_spsPoints.GetLength() == 0 )
	{
		return	false ;
	}
	int	b = 0, g = 0, r = 0 ;
	for ( size_t i = 0; i < m_spsPoints.GetLength(); i ++ )
	{
		const SelectPoint&	sp = m_spsPoints.At(i) ;
		S3DColor	color = sp.pPatch->GetColorAt( sp.iVertex ) ;
		b += color.rgbAdd.argb.Blue ;
		g += color.rgbAdd.argb.Green ;
		r += color.rgbAdd.argb.Red ;
	}
	int	n = (int) m_spsPoints.GetLength() ;
	rgb.argb.Blue = (uint8_t) esl_clampi( b / n, 0, 0xFF ) ;
	rgb.argb.Green = (uint8_t) esl_clampi( g / n, 0, 0xFF ) ;
	rgb.argb.Red = (uint8_t) esl_clampi( r / n, 0, 0xFF ) ;
	return	true ;
}

bool S3DMeshEditorObject::DoSampleElementOfColorAlpha( uint8_t& alpha ) const
{
	if ( m_spsPoints.GetLength() == 0 )
	{
		return	false ;
	}
	int	a = 0 ;
	for ( size_t i = 0; i < m_spsPoints.GetLength(); i ++ )
	{
		const SelectPoint&	sp = m_spsPoints.At(i) ;
		S3DColor	color = sp.pPatch->GetColorAt( sp.iVertex ) ;
		a += color.rgbMul.argb.Alpha ;
	}
	int	n = (int) m_spsPoints.GetLength() ;
	alpha = (uint8_t) esl_clampi( a / n, 0, 0xFF ) ;
	return	true ;
}

bool S3DMeshEditorObject::DoSampleElementOfColorAddAlpha( uint8_t& alpha ) const
{
	if ( m_spsPoints.GetLength() == 0 )
	{
		return	false ;
	}
	int	a = 0 ;
	for ( size_t i = 0; i < m_spsPoints.GetLength(); i ++ )
	{
		const SelectPoint&	sp = m_spsPoints.At(i) ;
		S3DColor	color = sp.pPatch->GetColorAt( sp.iVertex ) ;
		a += color.rgbAdd.argb.Alpha ;
	}
	int	n = (int) m_spsPoints.GetLength() ;
	alpha = (uint8_t) esl_clampi( a / n, 0, 0xFF ) ;
	return	true ;
}

bool S3DMeshEditorObject::DoSampleElementOfWeightMap( size_t iWeight, float32_t& w ) const
{
	if ( iWeight >= GetWeightLayerCount() )
	{
		return	false ;
	}
	if ( m_spsPoints.GetLength() == 0 )
	{
		return	false ;
	}
	w = 0.0f ;
	for ( size_t i = 0; i < m_spsPoints.GetLength(); i ++ )
	{
		const SelectPoint&	sp = m_spsPoints.At(i) ;
		w += sp.pPatch->GetWeightAt( sp.iVertex, iWeight ) ;
	}
	w /= (float32_t) m_spsPoints.GetLength() ;
	return	true ;
}

// 選択点を共有するパッチ外ポリゴンの頂点を分離する
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::DoDivideExFaceVertex( S3DMeshEditorInterface * pEdit )
{
	SArray<float32_t>				aWeights ;
	S3DMeshEditor::Patch::Elements	elTemp ;
	elTemp.nWeights = GetMeshEditor().GetWeightLayerCount() ;
	elTemp.pWeights = aWeights.GetArray( elTemp.nWeights ) ;
	//
	for ( size_t i = 0; i < m_spsPoints.GetLength(); i ++ )
	{
		S3DMeshEditor::SelectPoint	sp = m_spsPoints.At(i) ;
		if ( sp.iVertex < sp.pPatch->GetAreaSize() )
		{
			continue ;
		}
		ssize_t	iExFace = sp.pPatch->FindExFaceVertexOf( sp.iVertex ) ;
		if ( iExFace < 0 )
		{
			continue ;
		}
		size_t	iNextExFace = (size_t) iExFace + 1 ;
		for ( ; ; )
		{
			iExFace = sp.pPatch->FindExFaceVertexOf( sp.iVertex, iNextExFace ) ;
			if ( iExFace < 0 )
			{
				break ;
			}
			const uint32_t *	pSrcIndexes =
				sp.pPatch->GetExFaceTriangleIndexes( (size_t) iExFace, 1 ) ;
			uint32_t	iDivIndexes[3] =
			{
				pSrcIndexes[0],
				pSrcIndexes[1],
				pSrcIndexes[2],
			} ;
			for ( size_t j = 0; j < 3; j ++ )
			{
				if ( iDivIndexes[j] == sp.iVertex )
				{
					size_t	iNew = sp.pPatch->GetTotalVertexCount() ;
					sp.pPatch->InsertPoints( iNew, 1 ) ;
					sp.pPatch->GetElementsAt( elTemp, sp.iVertex ) ;
					sp.pPatch->SetElementsAt( iNew, elTemp ) ;
					iDivIndexes[j] = (uint32_t) iNew ;
				}
			}
			sp.pPatch->ModifyExFaceTriangleIndexes
						( (size_t) iExFace, 1, iDivIndexes ) ;
			//
			iNextExFace = (size_t) iExFace + 1 ;
		}
	}
	aWeights.FinishArray() ;
	//
	SetUpdateVertexFlag() ;
}

// 面掃引準備
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::PrepareToSweepFace
	( bool flagMakeBack, S3DMeshEditorInterface * pEdit )
{
	ESLAssert( pEdit != nullptr ) ;
	//
	// 輪郭抽出
	//
	SObjectArray<EdgeChain>	aEdgeChains ;
	GetEdgeChainOfFaceOutline( aEdgeChains, m_pfsFaces ) ;
	//
	if ( aEdgeChains.GetLength() == 0 )
	{
		ClearAllSelection() ;
		return ;
	}
	//
	// 押し出しの継続判定
	//
	SPtrSortArray<S3DMeshEditor::Patch,bool>	psaFullFace ;
	bool										flagSingleLines = false ;
	for ( size_t i = 0; i < aEdgeChains.GetLength(); i ++ )
	{
		S3DMeshEditor::PatchLine	pl ;
		if ( IsEdgeChainSinglePatchLine( pl, aEdgeChains.At(i) ) )
		{
			flagSingleLines = true ;
		}
		else
		{
			EdgeChain&	chain = aEdgeChains.At(i) ;
			bool	fNotFullFace = false ;
			for ( size_t j = 0; j < chain.GetLength(); j ++ )
			{
				S3DMeshEditor::PatchPoint	pp = chain.At(j) ;
				bool *	pFullFace = psaFullFace.GetAs( pp.pPatch ) ;
				if ( pFullFace == nullptr )
				{
					bool	fullFace =
						IsFullSelectedFace( *(pp.pPatch), m_pfsFaces ) ;
					psaFullFace.SetAs( pp.pPatch, fullFace ) ;
					if ( !fullFace )
					{
						flagSingleLines = false ;
						fNotFullFace = true ;
						break ;
					}
				}
				else if ( !*pFullFace )
				{
					flagSingleLines = false ;
					fNotFullFace = true ;
					break ;
				}
			}
			if ( fNotFullFace )
			{
				break ;
			}
		}
	}
	if ( !flagMakeBack && flagSingleLines )
	{
		//
		// 既に押し出された面の押し出し継続
		//
		for ( size_t i = 0; i < aEdgeChains.GetLength(); i ++ )
		{
			S3DMeshEditor::PatchLine	pl ;
			if ( IsEdgeChainSinglePatchLine( pl, aEdgeChains.At(i) ) )
			{
				if ( pl.lineDir == S3DMeshEditor::lineHorizontal )
				{
					if ( pl.iLine == 0 )
					{
						pEdit->InsertLine( pl.pPatch, 1, 0.0f ) ;
					}
					else
					{
						pEdit->InsertLine( pl.pPatch, pl.iLine, 1.0f ) ;
					}
				}
				else
				{
					if ( pl.iLine == 0 )
					{
						pEdit->InsertColumn( pl.pPatch, 1, 0.0f ) ;
					}
					else
					{
						pEdit->InsertColumn( pl.pPatch, pl.iLine, 1.0f ) ;
					}
				}
			}
		}
		SArraySet<Patch*>	asNewSel ;
		for ( size_t i = 0; i < m_pfsFaces.GetLength(); i ++ )
		{
			asNewSel.AddSorted( m_pfsFaces.At(i).pPatch ) ;
		}
		SetSelectedPatchs( asNewSel ) ;
		NormalizeSelecedPatchs() ;
		return ;
	}
	//
	// 選択面だけパッチ複製
	//
	SPtrSortArray<S3DMeshEditor::Patch,SweepedPatch>	psaSweep ;
	SPtrSortArray<S3DMeshEditor::Patch,SweepedPatch>	psaBack ;
	for ( size_t i = 0; i < m_pfsFaces.GetLength(); i ++ )
	{
		S3DMeshEditor::PatchFace	pf = m_pfsFaces.At(i) ;
		if ( psaSweep.GetAs( pf.pPatch ) != nullptr )
		{
			continue ;
		}
		SGLPoint				ptOffset ;
		S3DMeshEditor::Patch *	pPatch =
			DuplicatePartialPatch( ptOffset, *(pf.pPatch), m_pfsFaces ) ;
		if ( pPatch != nullptr )
		{
			pPatch->SetName( pPatch->GetName() + L"_sweep" ) ;
			pEdit->AddPatch( pPatch ) ;
			//
			if ( flagMakeBack )
			{
				ESLAssert( psaBack.GetAs( pf.pPatch ) == nullptr ) ;
				//
				S3DMeshEditor::Patch *	pBackPatch =
					DuplicatePartialPatch( ptOffset, *(pf.pPatch), m_pfsFaces ) ;
				ESLAssert( pBackPatch != nullptr );
				if ( pBackPatch != nullptr )
				{
					pBackPatch->SetName( pBackPatch->GetName() + L"_back" ) ;
					pEdit->AddPatch( pBackPatch ) ;
					//
					PatchFaceSet	pfsBack ;
					EnumeratePatchValidFaces( pfsBack, *pBackPatch ) ;
					DoInverseFace( pfsBack, pEdit ) ;
					//
					SweepedPatch	spBack ;
					spBack.pPatch = pBackPatch ;
					spBack.ptOffset = ptOffset ;
					psaBack.SetAs( pf.pPatch, spBack ) ;
				}
			}
		}
		SweepedPatch	sp ;
		sp.pPatch = pPatch ;
		sp.ptOffset = ptOffset ;
		psaSweep.SetAs( pf.pPatch, sp ) ;
		//
		ChangeSelectedFace
			( *(pf.pPatch), S3DMeshEditor::Patch::faceNull, m_pfsFaces ) ;
		pEdit->NotifyUpdateFace( m_pfsFaces ) ;
	}
	//
	// 側面の生成
	//
	for ( size_t i = 0; i < aEdgeChains.GetLength(); i ++ )
	{
		const EdgeChain&	chain = aEdgeChains.At(i) ;
		if ( chain.GetLength() == 0 )
		{
			continue ;
		}
		const size_t	nWeightLayer = GetWeightLayerCount() ;
		S3DMeshEditor::Patch *	pPatch = NewPatch() ;
		pPatch->SetName( chain.At(0).pPatch->GetName() + L"_side" ) ;
		pPatch->CreatePatch( chain.GetLength(), 2 ) ;
		pPatch->InsertWeightLayer( 0, nWeightLayer ) ;
		pEdit->AddPatch( pPatch ) ;
		//
		S3DMeshEditor::SelectPointSet	spsSidePatch ;
		S3DMeshEditor::PatchPointSet	ppsShrink ;
		for ( size_t j = 0; j < chain.GetLength(); j ++ )
		{
			//
			// 頂点情報
			//
			S3DMeshEditor::PatchPoint	pp = chain.At(j) ;
			S3DVector	vPos = pp.pPatch->GetPointAt( pp.iVertex ) ;
			S2DVector	vUV = pp.pPatch->GetUVAt( pp.iVertex ) ;
			S3DColor	color = pp.pPatch->GetColorAt( pp.iVertex ) ;
			//
			pPatch->SetPoint( j, 0, vPos ) ;
			pPatch->SetPoint( j, 1, vPos ) ;
			pPatch->SetUV( j, 0, vUV ) ;
			pPatch->SetUV( j, 1, vUV ) ;
			pPatch->SetColor( j, 0, color ) ;
			pPatch->SetColor( j, 1, color ) ;
			//
			for ( size_t k = 0; k < nWeightLayer; k ++ )
			{
				float32_t	w = pp.pPatch->GetWeightAt( pp.iVertex, k ) ;
				pPatch->SetWeight( j, 0, k, w ) ;
				pPatch->SetWeight( j, 1, k, w ) ;
			}
			spsSidePatch.AddSorted
				( S3DMeshEditor::SelectPoint
					( pPatch, (size_t) pPatch->IndexFromPoint( j, 0 ) ) ) ;
			spsSidePatch.AddSorted
				( S3DMeshEditor::SelectPoint
					( pPatch, (size_t) pPatch->IndexFromPoint( j, 1 ) ) ) ;
			//
			// 押し出され面に縮退
			//
			SweepedPatch *	pSweepPatch = psaSweep.GetAs( pp.pPatch ) ;
			SweepedPatch *	pBackPatch = psaBack.GetAs( pp.pPatch ) ;
			//
			SGLPoint	ptSrcPoint ;
			pp.pPatch->PointFromIndex( ptSrcPoint, (uint32_t) pp.iVertex ) ;
			//
			if ( pSweepPatch != nullptr )
			{
				SGLPoint	ptSweepPoint = ptSrcPoint - pSweepPatch->ptOffset ;
				if ( pSweepPatch->pPatch->IsValidPoint( ptSweepPoint ) )
				{
					S3DMeshEditor::PatchPoint
								ppSweep( pSweepPatch->pPatch,
											pSweepPatch->pPatch->IndexFromPoint
												( (size_t) ptSweepPoint.x,
													(size_t) ptSweepPoint.y ) ) ;
					uint32_t	nShrinkFlags = 0 ;
					ppsShrink.RemoveAll() ;
					ppsShrink.AddSorted( ppSweep ) ;
					ppsShrink.AddSorted
						( S3DMeshEditor::PatchPoint
							( pPatch, pPatch->IndexFromPoint( j, 0 ) ) ) ;
					//
					pEdit->ShrinkPoints( ppsShrink, nShrinkFlags ) ;
				}
			}
			//
			// 押し出し元／裏面に縮退
			//
			uint32_t	nShrinkFlags = 0 ;
			if ( (pBackPatch != nullptr) && flagMakeBack )
			{
				SGLPoint	ptBackPoint = ptSrcPoint - pBackPatch->ptOffset ;
				if ( pBackPatch->pPatch->IsValidPoint( ptBackPoint ) )
				{
					S3DMeshEditor::PatchPoint
								ppBack( pBackPatch->pPatch,
											pBackPatch->pPatch->IndexFromPoint
												( (size_t) ptBackPoint.x,
													(size_t) ptBackPoint.y ) ) ;
					ppsShrink.RemoveAll() ;
					ppsShrink.AddSorted( ppBack ) ;
				}
			}
			else
			{
				ppsShrink.RemoveAll() ;
				ppsShrink.AddSorted( pp ) ;
			}
			ppsShrink.AddSorted
				( S3DMeshEditor::PatchPoint
					( pPatch, pPatch->IndexFromPoint( j, 1 ) ) ) ;
			//
			pEdit->ShrinkPoints( ppsShrink, nShrinkFlags ) ;
		}
		pEdit->NotifyUpdateVertex( spsSidePatch ) ;
	}
	//
	// 選択点の更新と空パッチの削除
	//
	SArraySet<Patch*>	asNewSel ;
	for ( size_t i = 0; i < psaSweep.GetLength(); i ++ )
	{
		const SPointerComparator<S3DMeshEditor::Patch> *
							ppPatch = psaSweep.GetTagAt( i ) ;
		SweepedPatch *		pSweep = psaSweep.GetAt( i ) ;
		ESLAssert( ppPatch != nullptr ) ;
		ESLAssert( pSweep != nullptr ) ;
		//
		S3DMeshEditor::Patch *	pPatch = *ppPatch ;
		if ( pPatch->IsFaceEmpty() )
		{
			ssize_t	iPatch = FindPatch( pPatch ) ;
			if ( iPatch >= 0 )
			{
				pEdit->RemovePatchAt( (size_t) iPatch ) ;
			}
		}
		if ( pSweep->pPatch != nullptr )
		{
			asNewSel.AddSorted( pSweep->pPatch ) ;
		}
	}
	SetSelectedPatchs( asNewSel ) ;
	NormalizeSelecedPatchs() ;
}

// 面反転
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::DoInverseFace
	( const S3DMeshEditor::PatchFaceSet& pfs, S3DMeshEditorInterface * pEdit )
{
	for ( size_t i = 0; i < pfs.GetLength(); i ++ )
	{
		S3DMeshEditor::PatchFace	pf = pfs.At(i) ;
		int8_t	face = pf.pPatch->GetFaceAt( pf.iFace ) ;
		if ( face == S3DMeshEditor::Patch::faceFront )
		{
			pf.pPatch->SetFaceAt( pf.iFace, S3DMeshEditor::Patch::faceBack ) ;
		}
		else if ( face == S3DMeshEditor::Patch::faceBack )
		{
			pf.pPatch->SetFaceAt( pf.iFace, S3DMeshEditor::Patch::faceFront ) ;
		}
	}
	SetUpdateVertexFlag() ;
	//
	if ( pEdit != nullptr )
	{
		pEdit->NotifyUpdateFace( pfs ) ;
	}
}

// 面更新
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::DoChangeFace
	( const S3DMeshEditor::PatchFaceSet& pfs,
			int8_t face, S3DMeshEditorInterface * pEdit )
{
	for ( size_t i = 0; i < pfs.GetLength(); i ++ )
	{
		S3DMeshEditor::PatchFace	pf = pfs.At(i) ;
		pf.pPatch->SetFaceAt( pf.iFace, face ) ;
	}
	SetUpdateVertexFlag() ;
	//
	if ( pEdit != nullptr )
	{
		pEdit->NotifyUpdateFace( pfs ) ;
	}
}

// 面シフタ変更
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::DoShiftFace
	( const S3DMeshEditor::PatchFaceSet& pfs, S3DMeshEditorInterface * pEdit )
{
	for ( size_t i = 0; i < pfs.GetLength(); i ++ )
	{
		S3DMeshEditor::PatchFace	pf = pfs.At(i) ;
		uint8_t	nShifter = pf.pPatch->GetFaceShifterAt( pf.iFace ) ;
		pf.pPatch->SetFaceShifterAt( pf.iFace, (nShifter ^ 1) ) ;
	}
	SetUpdateVertexFlag() ;
	//
	if ( pEdit != nullptr )
	{
		pEdit->NotifyUpdateFace( pfs ) ;
	}
}

// 選択面を別パッチへ分離
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::DoSeparatePatchFace
	( const S3DMeshEditor::PatchFaceSet& pfs, S3DMeshEditorInterface * pEdit )
{
	ESLAssert( pEdit != nullptr ) ;
	//
	SArraySet<S3DMeshEditor::Patch*>	asProcessedPatch ;
	SArraySet<S3DMeshEditor::Patch*>	asSeparatedPatch ;
	S3DMeshEditor::Patch *				pLastPatch = nullptr ;
	//
	for ( size_t i = 0; i < pfs.GetLength(); i ++ )
	{
		S3DMeshEditor::PatchFace	pf = pfs.At(i) ;
		if ( pf.pPatch == pLastPatch )
		{
			continue ;
		}
		if ( asProcessedPatch.FindSorted( pf.pPatch ) >= 0 )
		{
			continue ;
		}
		SGLPoint				ptOffset ;
		S3DMeshEditor::Patch *	pPatch =
			DuplicatePartialPatch( ptOffset, *(pf.pPatch), pfs ) ;
		if ( pPatch != nullptr )
		{
			pPatch->SetName( pf.pPatch->GetName() + L"_sep" ) ;
			pEdit->AddPatch( pPatch ) ;
			//
			ChangeSelectedFace
				( *(pf.pPatch), S3DMeshEditor::Patch::faceNull, pfs ) ;
			pEdit->NotifyUpdateFace( pfs ) ;
			//
			asProcessedPatch.AddSorted( pf.pPatch ) ;
			asSeparatedPatch.AddSorted( pPatch ) ;
		}
	}
	//
	SetSelectedPatchs( asSeparatedPatch ) ;
	NormalizeSelecedPatchs() ;
}

// パッチの順序反転（表裏反転）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::DoInversePatch
	( const SSystem::SArraySet<Patch*>& ps, S3DMeshEditorInterface * pEdit )
{
	ESLAssert( pEdit != nullptr ) ;
	//
	for ( size_t i = 0; i < ps.GetLength(); i ++ )
	{
		S3DMeshEditor::Patch **	ppPatch = ps.GetAt( i ) ;
		if ( (ppPatch != nullptr) && (*ppPatch != nullptr) )
		{
			S3DMeshEditor::Patch *	pPatch = *ppPatch ;
			size_t	hPatch = pPatch->GetHeight() ;
			if ( hPatch >= 2 )
			{
				for ( size_t y = 0; (ssize_t) y < (ssize_t) (hPatch - y - 1); y ++ )
				{
					pEdit->SwapLine( pPatch, y, hPatch - y - 1 ) ;
				}
			}
		}
	}
}

// ライン縮退
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::DoShrinkLine
	( const S3DMeshEditor::PatchLine& line0,
		const S3DMeshEditor::PatchLine& line1,
		uint32_t nFlags, bool flagGap,
		const S3DMeshEditor::PatchPointSet& xptPoints,
		S3DMeshEditorInterface * pEdit )
{
	if ( (line0.pPatch != line1.pPatch)
		|| (line0.lineDir != line1.lineDir) )
	{
		return ;
	}
	S3DMeshEditor::PatchPointSet	pps ;
	S3DMeshEditor::PatchFaceSet		pfs ;
	const size_t	iLine0 = (line0.iLine < line1.iLine)
									? line0.iLine : line1.iLine ;
	if ( line0.lineDir == S3DMeshEditor::lineHorizontal )
	{
		const size_t	nWidth = line0.pPatch->GetWidth() ;
		for ( size_t i = 0; i < nWidth; i ++ )
		{
			S3DMeshEditor::PatchPoint
				pp0( line0.pPatch,
						line0.pPatch->IndexFromPoint( i, line0.iLine ) ) ;
			S3DMeshEditor::PatchPoint
				pp1( line1.pPatch,
						line0.pPatch->IndexFromPoint( i, line1.iLine ) ) ;
			if ( flagGap )
			{
				S3DMeshEditor::PatchFace
					pf( line0.pPatch, line0.pPatch->IndexFromPoint( i, iLine0 ) ) ;
				pfs.AddSorted( pf ) ;
			}
			if ( (xptPoints.FindSorted( pp0 ) >= 0)
				|| (xptPoints.FindSorted( pp1 ) >= 0) )
			{
				continue ;
			}
			pps.AddSorted( pp0 ) ;
			pps.AddSorted( pp1 ) ;
			if ( pEdit != nullptr )
			{
				pEdit->ShrinkPoints( pps, nFlags ) ;
			}
			else
			{
				ShrinkPoints( pps, nFlags ) ;
			}
			pps.RemoveAll() ;
		}
	}
	else
	{
		const size_t	nHeight = line0.pPatch->GetHeight() ;
		for ( size_t i = 0; i < nHeight; i ++ )
		{
			S3DMeshEditor::PatchPoint
				pp0( line0.pPatch,
						line0.pPatch->IndexFromPoint( line0.iLine, i ) ) ;
			S3DMeshEditor::PatchPoint
				pp1( line1.pPatch,
						line0.pPatch->IndexFromPoint( line1.iLine, i ) ) ;
			if ( flagGap )
			{
				S3DMeshEditor::PatchFace
					pf( line0.pPatch, line0.pPatch->IndexFromPoint( iLine0, i ) ) ;
				pfs.AddSorted( pf ) ;
			}
			if ( (xptPoints.FindSorted( pp0 ) >= 0)
				|| (xptPoints.FindSorted( pp1 ) >= 0) )
			{
				continue ;
			}
			pps.AddSorted( pp0 ) ;
			pps.AddSorted( pp1 ) ;
			if ( pEdit != nullptr )
			{
				pEdit->ShrinkPoints( pps, nFlags ) ;
			}
			else
			{
				ShrinkPoints( pps, nFlags ) ;
			}
			pps.RemoveAll() ;
		}
	}
	if ( flagGap )
	{
		DoChangeFace( pfs, S3DMeshEditor::Patch::faceNull, pEdit ) ;
	}
	SetUpdateVertexFlag() ;
	//
	S3DMeshEditor::PatchLineSet	pls ;
	pls.AddSorted( line0 ) ;
	pls.AddSorted( line1 ) ;
	SetSelectedLines( pls ) ;
	NormalizeSelecedLines() ;
}

// 縮退済みライン判定
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditorObject::IsShrinkedLine
	( const S3DMeshEditor::PatchLineSet& pls ) const
{
	if ( pls.GetLength() <= 1 )
	{
		return	false ;
	}
	S3DMeshEditor::PatchLine	pl0 = pls.At(0) ;
	for ( size_t i = 1; i < pls.GetLength(); i ++ )
	{
		S3DMeshEditor::PatchLine	pl = pls.At(i) ;
		if ( (pl.pPatch != pl0.pPatch)
			|| (pl.lineDir != pl0.lineDir) )
		{
			return	false ;
		}
	}
	PatchPointSet	ppsTemp ;
	if ( pl0.lineDir == S3DMeshEditor::lineHorizontal )
	{
		const size_t	nWidth = pl0.pPatch->GetWidth() ;
		for ( size_t i = 0; i < nWidth; i ++ )
		{
			if ( !pl0.pPatch->GetDegeneratedPoints
				( ppsTemp, pl0.pPatch->IndexFromPoint( i, pl0.iLine ) ) )
			{
				return	false ;
			}
			for ( size_t j = 1; j < pls.GetLength(); j ++ )
			{
				if ( ppsTemp.Find
					( S3DMeshEditor::PatchPoint
						( pl0.pPatch,
							pl0.pPatch->IndexFromPoint( i, pls.At(j).iLine ) ) ) < 0 )
				{
					return	false ;
				}
			}
			ppsTemp.RemoveAll() ;
		}
	}
	else
	{
		const size_t	nHeight = pl0.pPatch->GetHeight() ;
		for ( size_t i = 0; i < nHeight; i ++ )
		{
			if ( !pl0.pPatch->GetDegeneratedPoints
				( ppsTemp, pl0.pPatch->IndexFromPoint( pl0.iLine, i ) ) )
			{
				return	false ;
			}
			for ( size_t j = 1; j < pls.GetLength(); j ++ )
			{
				if ( ppsTemp.Find
					( S3DMeshEditor::PatchPoint
						( pl0.pPatch,
							pl0.pPatch->IndexFromPoint( pls.At(j).iLine, i ) ) ) < 0 )
				{
					return	false ;
				}
			}
			ppsTemp.RemoveAll() ;
		}
	}
	return	true ;
}

// ライン融合
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::DoMeltLines
	( const S3DMeshEditor::PatchLineSet& pls, S3DMeshEditorInterface * pEdit )
{
	ESLAssert( pEdit != nullptr ) ;
	if ( pls.GetLength() <= 1 )
	{
		return ;
	}
	S3DMeshEditor::PatchLineSet	plsMelt ;
	S3DMeshEditor::PatchLine	line0 = pls.At(0) ;
	plsMelt.AddSorted( line0 ) ;
	for ( size_t i = 1; i < pls.GetLength(); i ++ )
	{
		S3DMeshEditor::PatchLine	pl = pls.At(i) ;
		if ( (pl.pPatch == line0.pPatch)
			&& (pl.lineDir == line0.lineDir) )
		{
			plsMelt.AddSorted( pl ) ;
		}
	}
	if ( plsMelt.GetLength() <= 1 )
	{
		return ;
	}
	line0 = plsMelt.At(0) ;
	//
	SArray<float32_t>	aWeight ;
	const size_t		nWeightCount = line0.pPatch->GetWeightLayerCount() ;
	float32_t *			pWeight = aWeight.GetArray( nWeightCount ) ;
	if ( line0.lineDir == S3DMeshEditor::lineHorizontal )
	{
		const size_t	nWidth = line0.pPatch->GetWidth();
		const float32_t	r = 1.0f / (float32_t) plsMelt.GetLength() ;
		const uint32_t	rn = 0x100 / (unsigned int) plsMelt.GetLength() + 1 ;
		//
		for ( size_t x = 0; x < nWidth; x ++ )
		{
			S3DVector	vPos = line0.pPatch->GetPoint( x, line0.iLine ) * r ;
			S2DVector	vUV = line0.pPatch->GetUV( x, line0.iLine ) * r ;
			S3DColor	color = line0.pPatch->GetColor( x, line0.iLine ).imul(rn) ;
			for ( size_t j = 0; j < nWeightCount; j ++ )
			{
				pWeight[j] = line0.pPatch->GetWeight( x, line0.iLine, j ) * r ;
			}
			//
			for ( size_t i = 1; i < plsMelt.GetLength(); i ++ )
			{
				S3DMeshEditor::PatchLine	pl = plsMelt.At(i) ;
				vPos += pl.pPatch->GetPoint( x, pl.iLine ) * r ;
				vUV += pl.pPatch->GetUV( x, pl.iLine ) * r ;
				color += pl.pPatch->GetColor( x, pl.iLine ).imul(rn) ;
				for ( size_t j = 0; j < nWeightCount; j ++ )
				{
					pWeight[j] += pl.pPatch->GetWeight( x, pl.iLine, j ) * r ;
				}
			}
			line0.pPatch->SetPoint( x, line0.iLine, vPos ) ;
			line0.pPatch->SetUV( x, line0.iLine, vUV ) ;
			line0.pPatch->SetColor( x, line0.iLine, color ) ;
			for ( size_t j = 0; j < nWeightCount; j ++ )
			{
				line0.pPatch->SetWeight( x, line0.iLine, j, pWeight[j] ) ;
			}
		}
		for ( size_t i = plsMelt.GetLength() - 1; i > 1; i -- )
		{
			S3DMeshEditor::PatchLine	pl = plsMelt.At(i) ;
			pEdit->RemoveLine( pl.pPatch, pl.iLine ) ;
		}
	}
	else
	{
		const size_t	nHeight = line0.pPatch->GetHeight();
		const float32_t	r = 1.0f / (float32_t) plsMelt.GetLength() ;
		const uint32_t	rn = 0x100 / (unsigned int) plsMelt.GetLength() + 1 ;
		//
		for ( size_t y = 0; y < nHeight; y ++ )
		{
			S3DVector	vPos = line0.pPatch->GetPoint( line0.iLine, y ) * r ;
			S2DVector	vUV = line0.pPatch->GetUV( line0.iLine, y ) * r ;
			S3DColor	color = line0.pPatch->GetColor( line0.iLine, y ).imul(rn) ;
			for ( size_t j = 0; j < nWeightCount; j ++ )
			{
				pWeight[j] = line0.pPatch->GetWeight( line0.iLine, y, j ) * r ;
			}
			//
			for ( size_t i = 1; i < plsMelt.GetLength(); i ++ )
			{
				S3DMeshEditor::PatchLine	pl = plsMelt.At(i) ;
				vPos += pl.pPatch->GetPoint( pl.iLine, y ) * r ;
				vUV += pl.pPatch->GetUV( pl.iLine, y ) * r ;
				color += pl.pPatch->GetColor( pl.iLine, y ).imul(rn) ;
				for ( size_t j = 0; j < nWeightCount; j ++ )
				{
					pWeight[j] += pl.pPatch->GetWeight( pl.iLine, y, j ) * r ;
				}
			}
			line0.pPatch->SetPoint( line0.iLine, y, vPos ) ;
			line0.pPatch->SetUV( line0.iLine, y, vUV ) ;
			line0.pPatch->SetColor( line0.iLine, y, color ) ;
			for ( size_t j = 0; j < nWeightCount; j ++ )
			{
				line0.pPatch->SetWeight( line0.iLine, y, j, pWeight[j] ) ;
			}
		}
		for ( size_t i = plsMelt.GetLength() - 1; i > 1; i -- )
		{
			S3DMeshEditor::PatchLine	pl = plsMelt.At(i) ;
			pEdit->RemoveColumn( pl.pPatch, pl.iLine ) ;
		}
	}
	SetUpdateVertexFlag() ;
	//
	plsMelt.RemoveAll() ;
	plsMelt.AddSorted( line0 ) ;
	SetSelectedLines( plsMelt ) ;
	NormalizeSelecedLines() ;
}

// ライン削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::DoRemoveLines
	( const S3DMeshEditor::PatchLineSet& pls, S3DMeshEditorInterface * pEdit )
{
	ESLAssert( pEdit != nullptr ) ;
	for ( size_t i = 0; i < pls.GetLength(); i ++ )
	{
		S3DMeshEditor::PatchLine	pl = pls.At( pls.GetLength() - 1 - i ) ;
		if ( pl.lineDir == S3DMeshEditor::lineHorizontal )
		{
			pEdit->RemoveLine( pl.pPatch, pl.iLine ) ;
		}
		else
		{
			pEdit->RemoveColumn( pl.pPatch, pl.iLine ) ;
		}
	}
	SetUpdateVertexFlag() ;
	//
	ClearAllSelection() ;
}

// ラインに面を張る
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::DoPutUpFaceIntoLine
	( const S3DMeshEditor::PatchLine& line, S3DMeshEditorInterface * pEdit )
{
	//
	// 頂点のインデックスと座標を収集する
	//
	SArray<S3DVector4>	bufVertex ;
	SArray<uint32_t>	bufIndex ;
	if ( line.lineDir == S3DMeshEditor::lineHorizontal )
	{
		const size_t	nWidth = line.pPatch->GetWidth() ;
		S3DVector4 *	pvVertex = bufVertex.GetArray( nWidth ) ;
		uint32_t *		pIndex = bufIndex.GetArray( nWidth ) ;
		for ( size_t x = 0; x < nWidth; x ++ )
		{
			pvVertex[x] = line.pPatch->GetPoint( x, line.iLine ) ;
			pIndex[x] = line.pPatch->IndexFromPoint( x, line.iLine ) ;
		}
		bufVertex.FinishArray() ;
		bufIndex.FinishArray() ;
	}
	else
	{
		const size_t	nHeight = line.pPatch->GetHeight() ;
		S3DVector4 *	pvVertex = bufVertex.GetArray( nHeight ) ;
		uint32_t *		pIndex = bufIndex.GetArray( nHeight ) ;
		for ( size_t y = 0; y < nHeight; y ++ )
		{
			pvVertex[y] = line.pPatch->GetPoint( line.iLine, y ) ;
			pIndex[y] = line.pPatch->IndexFromPoint( line.iLine, y ) ;
		}
		bufVertex.FinishArray() ;
		bufIndex.FinishArray() ;
	}
	if ( bufVertex.GetLength() < 3 )
	{
		return ;
	}
	//
	// 三角化
	//
	SArray<uint32_t>	aTriangleList ;
	SArray<uint32_t>	aWorkIndex ;
	//
	S3DMeshShaper::MakeTriangleIndexedList
		( aTriangleList, aWorkIndex,
			bufVertex.GetConstArray(), bufVertex.GetLength() ) ;
	//
	// Patch 作成
	//
	S3DMeshEditor::Patch *	pPatch = NewPatch() ;
	pPatch->SetName( line.pPatch->GetName() + L"_hatched" ) ;
	//
	// 頂点セットアップ
	//
	SArray<float32_t>	bufWeights ;
	S3DMeshEditor::Patch::Elements	el ;
	el.nWeights = pPatch->GetWeightLayerCount() ;
	el.pWeights = bufWeights.GetArray( el.nWeights ) ;
	//
	pPatch->InsertPoints( 0, bufVertex.GetLength() ) ;
	//
	for ( size_t i = 0; i < bufVertex.GetLength(); i ++ )
	{
		line.pPatch->GetElementsAt( el, (size_t) bufIndex.At(i) ) ;
		pPatch->SetElementsAt( i, el ) ;
	}
	//
	// 面セットアップ
	//
	const size_t	nFaceCount = aTriangleList.GetLength() / 3 ;
	pPatch->InsertFaces( 0, nFaceCount ) ;
	pPatch->m_bufTriangles = aTriangleList ;
	//
	// Patch 追加
	//
	pEdit->AddPatch( pPatch ) ;
	//
	// 頂点縮退設定
	//
	S3DMeshEditor::PatchPointSet	pps ;
	for ( size_t i = 0; i < bufVertex.GetLength(); i ++ )
	{
		pps.RemoveAll() ;
		pps.AddSorted
			( S3DMeshEditor::PatchPoint
				( line.pPatch, (size_t) bufIndex.At(i) ) ) ;
		pps.AddSorted
			( S3DMeshEditor::PatchPoint( pPatch, i ) ) ;
		//
		pEdit->ShrinkPoints( pps, S3DMeshEditor::degenerateDivNormal ) ;
	}
}

// ベベル処理準備
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::PrepareBevel
	( SArray<S3DMeshEditorObject::BevelPoint>& aBevelPoints,
		S3DMeshEditor::SelectPointSet& spsPoints,
		bool flagBoth, bool flagGap, S3DMeshEditorInterface * pEdit )
{
	ESLAssert( pEdit != nullptr ) ;
	S3DMeshEditor::PatchLineSet		plsTemp ;
	const bool	flagSelLine = (m_plsLines.GetLength() >= 1) ;
	if ( flagSelLine )
	{
		plsTemp = m_plsLines ;
	}
	else
	{
		//
		// 選択稜線を含む全ラインを収集する
		//
		for ( size_t i = 0; i < m_pesEdges.GetLength(); i ++ )
		{
			S3DMeshEditor::PatchEdge	pe = m_pesEdges.At(i) ;
			const size_t	nAreaSize = pe.pPatch->GetAreaSize() ;
			if ( (pe.iVertex0 < nAreaSize)
				&& (pe.iVertex1 < nAreaSize) )
			{
				SGLPoint	pt0, pt1 ;
				pe.pPatch->PointFromIndex( pt0, (uint32_t) pe.iVertex0 ) ;
				pe.pPatch->PointFromIndex( pt1, (uint32_t) pe.iVertex1 ) ;
				if ( pt0.x == pt1.x )
				{
					plsTemp.AddSorted
						( S3DMeshEditor::PatchLine
							( pe.pPatch, S3DMeshEditor::lineVertical, (size_t) pt0.x ) ) ;
				}
				else if ( pt0.y == pt1.y )
				{
					plsTemp.AddSorted
						( S3DMeshEditor::PatchLine
							( pe.pPatch, S3DMeshEditor::lineHorizontal, (size_t) pt0.y ) ) ;
				}
			}
		}
	}
	//
	// 選択点を多く含むラインを対象のラインとする
	//
	S3DMeshEditor::PatchPointSet	ppsTemp ;
	S3DMeshEditor::PatchLine		plMaxLine ;
	size_t							nMaxSelPoints = 0 ;
	size_t							nMaxLineLength = 0 ;
	//
	for ( size_t i = 0; i < plsTemp.GetLength(); i ++ )
	{
		S3DMeshEditor::PatchLine	pl = plsTemp.At(i) ;
		ppsTemp.RemoveAll() ;
		size_t	nSelPoints = 0 ;
		size_t	nLineLength = 0 ;
		if ( pl.lineDir == S3DMeshEditor::lineHorizontal )
		{
			const size_t	nWidth = pl.pPatch->GetWidth() ;
			nLineLength = nWidth ;
			for ( size_t x = 0; x < nWidth; x ++ )
			{
				S3DMeshEditor::PatchPoint
					pp( pl.pPatch, pl.pPatch->IndexFromPoint( x, pl.iLine ) ) ;
				if ( ppsTemp.Find( pp ) >= 0 )
				{
					continue ;
				}
				if ( m_spsPoints.FindSorted( pp ) >= 0 )
				{
					nSelPoints ++ ;
					pl.pPatch->GetDegeneratedPoints( ppsTemp, pp.iVertex) ;
				}
			}
		}
		else
		{
			const size_t	nHeight = pl.pPatch->GetHeight() ;
			nLineLength = nHeight ;
			for ( size_t y = 0; y < nHeight; y ++ )
			{
				S3DMeshEditor::PatchPoint
					pp( pl.pPatch, pl.pPatch->IndexFromPoint( pl.iLine, y ) ) ;
				if ( ppsTemp.Find( pp ) >= 0 )
				{
					continue ;
				}
				if ( m_spsPoints.FindSorted( pp ) >= 0 )
				{
					nSelPoints ++ ;
					pl.pPatch->GetDegeneratedPoints( ppsTemp, pp.iVertex) ;
				}
			}
		}
		if ( (nSelPoints > nMaxSelPoints)
			|| ((nSelPoints == nMaxSelPoints) && (nLineLength > nMaxLineLength)) )
		{
			plMaxLine = pl ;
			nMaxSelPoints = nSelPoints ;
			nMaxLineLength = nLineLength ;
		}
	}
	if ( nMaxSelPoints == 0 )
	{
		return ;
	}
	//
	// 新規ラインの挿入位置を確定する
	//
	size_t	iMinLine = plMaxLine.iLine;
	size_t	iMaxLine = plMaxLine.iLine ;
	size_t	iMaxLoopFirst = 0 ;
	size_t	iMinLoopLast = plMaxLine.iLine ;
	bool	flagLoopFirst = false ;
	bool	flagLoopLast = false ;
	if ( plMaxLine.lineDir == S3DMeshEditor::lineHorizontal )
	{
		const size_t	nWidth = plMaxLine.pPatch->GetWidth() ;
		const size_t	nHeight = plMaxLine.pPatch->GetHeight() ;
		iMinLoopLast = plMaxLine.pPatch->GetHeight() - 1 ;
		//
		if ( flagSelLine )
		{
			for ( size_t i = 0; i < nHeight; i ++ )
			{
				size_t	iNextMinLine = (iMinLine > 0) ? iMinLine - 1 : nHeight - 1 ;
				if ( !plMaxLine.pPatch->IsDegeneratedLines
					( plMaxLine.lineDir, iMinLine, iNextMinLine ) )
				{
					break ;
				}
				iMinLine = iNextMinLine ;
			}
			for ( size_t i = 0; i < nHeight; i ++ )
			{
				size_t	iNextMaxLine = (iMaxLine < nHeight - 1) ? iMaxLine + 1 : 0 ;
				if ( !plMaxLine.pPatch->IsDegeneratedLines
					( plMaxLine.lineDir, iMaxLine, iNextMaxLine ) )
				{
					break ;
				}
				iMaxLine = iNextMaxLine ;
			}
			if ( iMinLine > iMaxLine )
			{
				flagLoopFirst = true ;
				flagLoopLast = true ;
				iMaxLoopFirst = iMaxLine ;
				iMinLoopLast = iMinLine ;
			}
		}
		else
		for ( size_t x = 0; x < nWidth; x ++ )
		{
			S3DMeshEditor::PatchPoint
				pp( plMaxLine.pPatch,
					plMaxLine.pPatch->IndexFromPoint( x, plMaxLine.iLine ) ) ;
			if ( m_spsPoints.FindSorted( pp ) < 0 )
			{
				continue ;
			}
			ppsTemp.RemoveAll() ;
			if ( plMaxLine.pPatch->
					GetDegeneratedPoints( ppsTemp, pp.iVertex ) )
			{
				for ( size_t i = 0; i < ppsTemp.GetLength(); i ++ )
				{
					SGLPoint	pt ;
					plMaxLine.pPatch->PointFromIndex
							( pt, (uint32_t) ppsTemp.At(i).iVertex ) ;
					if ( (size_t) pt.y < iMinLine )
					{
						iMinLine = (size_t) pt.y ;
					}
					if ( iMaxLine < (size_t) pt.y )
					{
						iMaxLine = (size_t) pt.y ;
					}
					if ( pt.y == 0 )
					{
						flagLoopFirst = true ;
					}
					else if ( flagLoopFirst
						&& ((size_t) pt.y == iMaxLoopFirst + 1) )
					{
						iMaxLoopFirst = (size_t) pt.y ;
					}
					if ( (size_t) pt.y == iMinLoopLast )
					{
						flagLoopLast = true ;
					}
					else if ( flagLoopLast
						&&  ((size_t) pt.y == iMinLoopLast - 1) )
					{
						iMinLoopLast = (size_t) pt.y ;
					}
				}
			}
		}
	}
	else
	{
		const size_t	nWidth = plMaxLine.pPatch->GetWidth() ;
		const size_t	nHeight = plMaxLine.pPatch->GetHeight() ;
		iMinLoopLast = plMaxLine.pPatch->GetWidth() - 1 ;
		//
		if ( flagSelLine )
		{
			for ( size_t i = 0; i < nWidth; i ++ )
			{
				size_t	iNextMinLine = (iMinLine > 0) ? iMinLine - 1 : nWidth - 1 ;
				if ( !plMaxLine.pPatch->IsDegeneratedLines
					( plMaxLine.lineDir, iMinLine, iNextMinLine ) )
				{
					break ;
				}
				iMinLine = iNextMinLine ;
			}
			for ( size_t i = 0; i < nWidth; i ++ )
			{
				size_t	iNextMaxLine = (iMaxLine < nWidth - 1) ? iMaxLine + 1 : 0 ;
				if ( !plMaxLine.pPatch->IsDegeneratedLines
					( plMaxLine.lineDir, iMaxLine, iNextMaxLine ) )
				{
					break ;
				}
				iMaxLine = iNextMaxLine ;
			}
			if ( iMinLine > iMaxLine )
			{
				flagLoopFirst = true ;
				flagLoopLast = true ;
				iMaxLoopFirst = iMaxLine ;
				iMinLoopLast = iMinLine ;
			}
		}
		else
		for ( size_t y = 0; y < nHeight; y ++ )
		{
			S3DMeshEditor::PatchPoint
				pp( plMaxLine.pPatch,
					plMaxLine.pPatch->IndexFromPoint( plMaxLine.iLine, y ) ) ;
			if ( m_spsPoints.FindSorted( pp ) < 0 )
			{
				continue ;
			}
			ppsTemp.RemoveAll() ;
			if ( plMaxLine.pPatch->
					GetDegeneratedPoints( ppsTemp, pp.iVertex ) )
			{
				for ( size_t i = 0; i < ppsTemp.GetLength(); i ++ )
				{
					SGLPoint	pt ;
					plMaxLine.pPatch->PointFromIndex
							( pt, (uint32_t) ppsTemp.At(i).iVertex ) ;
					if ( (size_t) pt.x < iMinLine )
					{
						iMinLine = (size_t) pt.x ;
					}
					if ( iMaxLine < (size_t) pt.x )
					{
						iMaxLine = (size_t) pt.x ;
					}
					if ( pt.x == 0 )
					{
						flagLoopFirst = true ;
					}
					else if ( flagLoopFirst
						&& ((size_t) pt.x == iMaxLoopFirst + 1) )
					{
						iMaxLoopFirst = (size_t) pt.x ;
					}
					if ( (size_t) pt.x == iMinLoopLast )
					{
						flagLoopLast = true ;
					}
					else if ( flagLoopLast
						&&  ((size_t) pt.x == iMinLoopLast - 1) )
					{
						iMinLoopLast = (size_t) pt.x ;
					}
				}
			}
		}
	}
	//
	// ライン挿入
	//
	const size_t	nOldWidth = plMaxLine.pPatch->GetWidth() ;
	const size_t	nOldHeight = plMaxLine.pPatch->GetHeight() ;
	if ( flagLoopFirst && flagLoopLast )
	{
		ESLAssert( iMaxLine <= iMinLine ) ;
		iMaxLine = iMaxLoopFirst ;
		iMinLine = iMinLoopLast ;
		if ( flagBoth )
		{
			if ( plMaxLine.lineDir == S3DMeshEditor::lineHorizontal )
			{
				pEdit->InsertLine( plMaxLine.pPatch, iMaxLine ) ;
			}
			else
			{
				pEdit->InsertColumn( plMaxLine.pPatch, iMaxLine ) ;
			}
			iMaxLine += 1 ;
			iMinLine += 1 ;
		}
		if ( plMaxLine.lineDir == S3DMeshEditor::lineHorizontal )
		{
			pEdit->InsertLine( plMaxLine.pPatch, iMinLine ) ;
		}
		else
		{
			pEdit->InsertColumn( plMaxLine.pPatch, iMinLine ) ;
		}
	}
	else
	{
		if ( flagBoth )
		{
			if ( plMaxLine.lineDir == S3DMeshEditor::lineHorizontal )
			{
				pEdit->InsertLine( plMaxLine.pPatch, iMaxLine ) ;
			}
			else
			{
				pEdit->InsertColumn( plMaxLine.pPatch, iMaxLine ) ;
			}
			iMaxLine += 2 ;
		}
		else
		{
			iMaxLine = plMaxLine.iLine + 1 ;
		}
		if ( plMaxLine.lineDir == S3DMeshEditor::lineHorizontal )
		{
			pEdit->InsertLine( plMaxLine.pPatch, iMinLine ) ;
		}
		else
		{
			pEdit->InsertColumn( plMaxLine.pPatch, iMinLine ) ;
		}
	}
	//
	// 各頂点を縮退／又はベベル処理情報準備
	//
	S3DMeshEditor::Patch *		pPatch = plMaxLine.pPatch ;
	S3DMeshEditor::PatchFaceSet	pfsNull ;
	if ( plMaxLine.lineDir == S3DMeshEditor::lineHorizontal )
	{
		const size_t	nWidth = pPatch->GetWidth() ;
		for ( size_t x = 0; x < nWidth; x ++ )
		{
			if ( flagGap )
			{
				S3DMeshEditor::PatchFace
					pf( pPatch, pPatch->IndexFromPoint( x, iMinLine ) ) ;
				pPatch->SetFaceAt( pf.iFace, S3DMeshEditor::Patch::faceNull ) ;
				pfsNull.AddSorted( pf ) ;
			}
			S3DMeshEditor::PatchPoint
				pp( pPatch, x + plMaxLine.iLine * nOldWidth ) ;
			if ( m_spsPoints.FindSorted( pp ) < 0 )
			{
				// 非選択点は縮退
				ppsTemp.RemoveAll() ;
				ppsTemp.AddSorted
					( S3DMeshEditor::PatchPoint
						( pPatch, pPatch->IndexFromPoint( x, iMinLine ) ) ) ;
				ppsTemp.AddSorted
					( S3DMeshEditor::PatchPoint
						( pPatch, pPatch->IndexFromPoint( x, plMaxLine.iLine + 1 ) ) ) ;
				if ( flagBoth )
				{
					ppsTemp.AddSorted
						( S3DMeshEditor::PatchPoint
							( pPatch, pPatch->IndexFromPoint( x, iMaxLine ) ) ) ;
				}
				pEdit->ShrinkPoints( ppsTemp, 0 ) ;
				continue ;
			}
			bool	flagMinLine0 = true ;
			bool	flagMaxLine2 = true ;
			size_t	yMinLine0 = iMinLine - 1 ;
			size_t	yMaxLine2 = iMaxLine + 1 ;
			if ( iMinLine == 0 )
			{
				if ( (flagLoopFirst && flagLoopLast)
					|| (pPatch->GetFlags() & S3DMeshEditor::Patch::flagVertLoop) )
				{
					yMinLine0 = pPatch->GetHeight() - 1 ;
				}
				else if ( (pPatch->GetDegenerateNumber(x,iMinLine) != 0)
						&& (pPatch->GetDegenerateNumber(x,iMinLine)
								== pPatch->GetDegenerateNumber(x,pPatch->GetHeight()-1)) )
				{
					uint32_t	nDeg = pPatch->GetDegenerateNumber(x,iMinLine) ;
					yMinLine0 = pPatch->GetHeight() - 1 ;
					flagMinLine0 = false ;
					while ( yMinLine0 > iMaxLine )
					{
						if ( nDeg != pPatch->GetDegenerateNumber(x,yMinLine0) )
						{
							flagMinLine0 = true ;
							break ;
						}
						yMinLine0 -- ;
					}
				}
				else
				{
					flagMinLine0 = false ;
				}
			}
			if ( yMaxLine2 >= pPatch->GetHeight() )
			{
				if ( (flagLoopFirst && flagLoopLast)
					|| (pPatch->GetFlags() & S3DMeshEditor::Patch::flagVertLoop) )
				{
					yMaxLine2 = 0 ;
				}
				else if ( (pPatch->GetDegenerateNumber(x,iMaxLine) != 0)
						&& (pPatch->GetDegenerateNumber(x,iMaxLine)
								== pPatch->GetDegenerateNumber(x,0)) )
				{
					uint32_t	nDeg = pPatch->GetDegenerateNumber(x,iMaxLine) ;
					yMaxLine2 = 0 ;
					flagMaxLine2 = false ;
					while ( yMaxLine2 < iMinLine )
					{
						if ( nDeg != pPatch->GetDegenerateNumber(x,yMaxLine2) )
						{
							flagMaxLine2 = true ;
							break ;
						}
						yMaxLine2 ++ ;
					}
				}
				else
				{
					flagMaxLine2 = false ;
				}
			}
			if ( flagMinLine0 )
			{
				BevelPoint	bp ;
				bp.pp = S3DMeshEditor::PatchPoint
						( pPatch, pPatch->IndexFromPoint( x, iMinLine ) ) ;
				bp.vPos0 = pPatch->GetPointAt( bp.pp.iVertex ) ;
				bp.vPos1 = pPatch->GetPoint( x, yMinLine0 ) ;
				aBevelPoints.Add( bp ) ;
			}
			if ( flagMaxLine2 )
			{
				BevelPoint	bp ;
				bp.pp = S3DMeshEditor::PatchPoint
						( pPatch, pPatch->IndexFromPoint( x, iMaxLine ) ) ;
				bp.vPos0 = pPatch->GetPointAt( bp.pp.iVertex ) ;
				bp.vPos1 = pPatch->GetPoint( x, yMaxLine2 ) ;
				aBevelPoints.Add( bp ) ;
			}
		}
	}
	else
	{
		const size_t	nHeight = plMaxLine.pPatch->GetHeight() ;
		for ( size_t y = 0; y < nHeight; y ++ )
		{
			if ( flagGap )
			{
				S3DMeshEditor::PatchFace
					pf( pPatch, pPatch->IndexFromPoint( iMinLine, y ) ) ;
				pPatch->SetFaceAt( pf.iFace, S3DMeshEditor::Patch::faceNull ) ;
				pfsNull.AddSorted( pf ) ;
			}
			S3DMeshEditor::PatchPoint
				pp( pPatch, plMaxLine.iLine + y * nOldWidth ) ;
			if ( m_spsPoints.FindSorted( pp ) < 0 )
			{
				// 非選択点は縮退
				ppsTemp.RemoveAll() ;
				ppsTemp.AddSorted
					( S3DMeshEditor::PatchPoint
						( pPatch, pPatch->IndexFromPoint( iMinLine, y ) ) ) ;
				ppsTemp.AddSorted
					( S3DMeshEditor::PatchPoint
						( pPatch, pPatch->IndexFromPoint( plMaxLine.iLine + 1, y ) ) ) ;
				if ( flagBoth )
				{
					ppsTemp.AddSorted
						( S3DMeshEditor::PatchPoint
							( pPatch, pPatch->IndexFromPoint( iMaxLine, y ) ) ) ;
				}
				pEdit->ShrinkPoints( ppsTemp, 0 ) ;
				continue ;
			}
			bool	flagMinLine0 = true ;
			bool	flagMaxLine2 = true ;
			size_t	yMinLine0 = iMinLine - 1 ;
			size_t	yMaxLine2 = iMaxLine + 1 ;
			if ( iMinLine == 0 )
			{
				if ( (flagLoopFirst && flagLoopLast)
					|| (pPatch->GetFlags() & S3DMeshEditor::Patch::flagHorzLoop) )
				{
					yMinLine0 = pPatch->GetWidth() - 1 ;
				}
				else if ( (pPatch->GetDegenerateNumber(iMinLine,y) != 0)
						&& (pPatch->GetDegenerateNumber(iMinLine,y)
								== pPatch->GetDegenerateNumber(pPatch->GetWidth()-1,y)) )
				{
					uint32_t	nDeg = pPatch->GetDegenerateNumber(iMinLine,y) ;
					yMinLine0 = pPatch->GetHeight() - 1 ;
					flagMinLine0 = false ;
					while ( yMinLine0 > iMaxLine )
					{
						if ( nDeg != pPatch->GetDegenerateNumber(yMinLine0,y) )
						{
							flagMinLine0 = true ;
							break ;
						}
						yMinLine0 -- ;
					}
				}
				else
				{
					flagMinLine0 = false ;
				}
			}
			if ( yMaxLine2 >= pPatch->GetWidth() )
			{
				if ( (flagLoopFirst && flagLoopLast)
					|| (pPatch->GetFlags() & S3DMeshEditor::Patch::flagHorzLoop) )
				{
					yMaxLine2 = 0 ;
				}
				else if ( (pPatch->GetDegenerateNumber(iMaxLine,y) != 0)
						&& (pPatch->GetDegenerateNumber(iMaxLine,y)
								== pPatch->GetDegenerateNumber(0,y)) )
				{
					uint32_t	nDeg = pPatch->GetDegenerateNumber(iMaxLine,y) ;
					yMaxLine2 = 0 ;
					flagMaxLine2 = false ;
					while ( yMaxLine2 < iMinLine )
					{
						if ( nDeg != pPatch->GetDegenerateNumber(yMaxLine2,y) )
						{
							flagMaxLine2 = true ;
							break ;
						}
						yMaxLine2 ++ ;
					}
				}
				else
				{
					flagMaxLine2 = false ;
				}
			}
			if ( flagMinLine0 )
			{
				BevelPoint	bp ;
				bp.pp = S3DMeshEditor::PatchPoint
						( pPatch, pPatch->IndexFromPoint( iMinLine, y ) ) ;
				bp.vPos0 = pPatch->GetPointAt( bp.pp.iVertex ) ;
				bp.vPos1 = pPatch->GetPoint( yMinLine0, y ) ;
				aBevelPoints.Add( bp ) ;
			}
			if ( flagMaxLine2 )
			{
				BevelPoint	bp ;
				bp.pp = S3DMeshEditor::PatchPoint
						( pPatch, pPatch->IndexFromPoint( iMaxLine, y ) ) ;
				bp.vPos0 = pPatch->GetPointAt( bp.pp.iVertex ) ;
				bp.vPos1 = pPatch->GetPoint( yMaxLine2, y ) ;
				aBevelPoints.Add( bp ) ;
			}
		}
	}
	for ( size_t i = 0; i < aBevelPoints.GetLength(); i ++ )
	{
		const BevelPoint&	bp = aBevelPoints.At(i) ;
		spsPoints.QuickAdd( S3DMeshEditor::SelectPoint( bp.pp ) ) ;
	}
	spsPoints.SortArray() ;
	//
	if ( flagGap )
	{
		pEdit->NotifyUpdateFace( pfsNull ) ;
	}
	SetSelectedPoints( spsPoints ) ;
	NormalizeSelecedPoints() ;
}

// ベベル処理
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::DoBevelPoints
	( const SSystem::SArray<BevelPoint>& aBevelPoints,
		const S3DMeshEditor::SelectPointSet& spsPoints,
		double fpBevel, S3DMeshEditorInterface * pEdit )
{
	S3DMeshEditor::SelectPointSet	spsTemp ;
	S3DMeshEditor::PatchPointSet	ppsTemp ;
	for ( size_t i = 0; i < aBevelPoints.GetLength(); i ++ )
	{
		const BevelPoint&	bp = aBevelPoints.At(i) ;
		S3DVector	vDelta = bp.vPos1 - bp.vPos0 ;
		double		r = vDelta.Absolute() ;
		if ( r > 0.0 )
		{
			double	t = esl_fmin( fpBevel, r ) ;
			//
			if ( bp.pp.pPatch->GetDegeneratedPoints( ppsTemp, bp.pp.iVertex ) )
			{
				S3DVector	vPos = bp.vPos0 + vDelta * (t / r) ;
				for ( size_t j = 0; j < ppsTemp.GetLength(); j ++ )
				{
					S3DMeshEditor::PatchPoint	pp = ppsTemp.At(j) ;
					pp.pPatch->SetPointAt( pp.iVertex, vPos ) ;
				}
				ppsTemp.RemoveAll() ;
			}
			else
			{
				bp.pp.pPatch->SetPointAt
					( bp.pp.iVertex, bp.vPos0 + vDelta * (t / r) ) ;
			}
		}
	}
	SetUpdateVertexFlag() ;
	//
	if ( pEdit != nullptr )
	{
		pEdit->NotifyUpdateVertex( spsPoints ) ;
	}
}

// ループスライス
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditorObject::SlicePointFromEdgePoint
	( SlicePoint& sp, const S3DMeshEditor::PatchEdge& pe, double w )
{
	const size_t	nAreaSize = pe.pPatch->GetAreaSize() ;
	if ( (pe.iVertex0 >= nAreaSize) || (pe.iVertex1 >= nAreaSize) )
	{
		return	false ;
	}
	SGLPoint	pt0, pt1 ;
	pe.pPatch->PointFromIndex( pt0, (uint32_t) pe.iVertex0 ) ;
	pe.pPatch->PointFromIndex( pt1, (uint32_t) pe.iVertex1 ) ;
	if ( pt0.x == pt1.x )
	{
		sp.pl.pPatch = pe.pPatch ;
		sp.pl.lineDir = S3DMeshEditor::lineHorizontal ;
		sp.pl.iLine = (size_t) pt1.y ;
		sp.w = w ;
		if ( (pt0.y == 0) && (pt0.y + 1 < pt1.y) )
		{
			sp.pl.iLine = 0 ;
			sp.w = 1.0 - w ;
		}
		return	true ;
	}
	else if ( pt0.y == pt1.y )
	{
		sp.pl.pPatch = pe.pPatch ;
		sp.pl.lineDir = S3DMeshEditor::lineVertical ;
		sp.pl.iLine = (size_t) pt1.x ;
		sp.w = w ;
		if ( (pt0.x == 0) && (pt0.x + 1 < pt1.x) )
		{
			sp.pl.iLine = 0 ;
			sp.w = 1.0 - w ;
		}
		return	true ;
	}
	else
	{
		return	false ;
	}
}

size_t S3DMeshEditorObject::CountPatchOfSlicePoints
	( const SArray<S3DMeshEditorObject::SlicePoint>& aChain, Patch * pPatch )
{
	size_t	nCount = 0 ;
	for ( size_t i = 0; i < aChain.GetLength(); i ++ )
	{
		if ( aChain.At(i).pl.pPatch == pPatch )
		{
			nCount ++ ;
		}
	}
	return	nCount ;
}

ssize_t S3DMeshEditorObject::FindSlicePoints
	( const SSystem::SArray<S3DMeshEditorObject::SlicePoint>& aChain,
							const S3DMeshEditorObject::SlicePoint& sp )
{
	for ( size_t i = 0; i < aChain.GetLength(); i ++ )
	{
		if ( aChain.At(i).pl == sp.pl )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

void S3DMeshEditorObject::OffsetIndexByInsertLine
	( S3DMeshEditor::PatchPointSet& ppsLine, Patch * pPatch, size_t iLine )
{
	for ( size_t i = 0; i < ppsLine.GetLength(); i ++ )
	{
		S3DMeshEditor::PatchPoint&	pp = ppsLine.At(i) ;
		if ( pp.pPatch == pPatch )
		{
			SGLPoint	pt ;
			pp.pPatch->PointFromIndex( pt, (uint32_t) pp.iVertex ) ;
			if ( (size_t) pt.y >= iLine )
			{
				pp.iVertex =
					pp.pPatch->IndexFromPoint
						( (size_t) pt.x, (size_t) pt.y + 1 ) ;
			}
		}
	}
}

void S3DMeshEditorObject::OffsetIndexByInsertColumn
	( S3DMeshEditor::PatchPointSet& ppsLine, Patch * pPatch, size_t iCol )
{
	for ( size_t i = 0; i < ppsLine.GetLength(); i ++ )
	{
		S3DMeshEditor::PatchPoint&	pp = ppsLine.At(i) ;
		if ( pp.pPatch == pPatch )
		{
			SGLPoint	pt ;
			pp.pPatch->PointFromIndex( pt, (uint32_t) pp.iVertex ) ;
			if ( (size_t) pt.x >= iCol )
			{
				pt.x ++ ;
			}
			pp.iVertex =
				(size_t) pt.x
					+ (size_t) pt.y * (pp.pPatch->GetWidth() + 1) ;
		}
	}
}

bool S3DMeshEditorObject::EnumerateChainedSlice
	( SSystem::SArray<SlicePoint>& aChain,
		const S3DMeshEditor::PatchEdge& pe, double w ) const
{
	//
	// 稜線内の位置をスライス位置へ変換
	//
	SlicePoint	sp ;
	if ( !SlicePointFromEdgePoint( sp, pe, w ) )
	{
		return	false ;
	}
	if ( FindSlicePoints( aChain, sp ) >= 0 )
	{
		return	false ;
	}
	aChain.Add( sp ) ;
	//
	// ループ準備
	//
	size_t	nLineLen, nCrossLen ;
	if ( sp.pl.lineDir == S3DMeshEditor::lineHorizontal )
	{
		nLineLen = sp.pl.pPatch->GetWidth() ;
		nCrossLen = sp.pl.pPatch->GetHeight() ;
	}
	else
	{
		nLineLen = sp.pl.pPatch->GetHeight() ;
		nCrossLen = sp.pl.pPatch->GetWidth() ;
	}
	size_t	iLine0 = sp.pl.iLine - 1 ;
	if ( sp.pl.iLine == 0 )
	{
		iLine0 = nCrossLen - 1 ;
	}
	PatchPointSet	pps0, pps1 ;
	for ( size_t i = 0; i < nLineLen; i ++ )
	{
		//
		// ループ両サイド頂点を列挙
		//
		S3DMeshEditor::PatchPoint	pp0, pp1 ;
		if ( sp.pl.lineDir == S3DMeshEditor::lineHorizontal )
		{
			pp0 = S3DMeshEditor::PatchPoint
				( sp.pl.pPatch, sp.pl.pPatch->IndexFromPoint( i, iLine0 ) ) ;
			pp1 = S3DMeshEditor::PatchPoint
				( sp.pl.pPatch, sp.pl.pPatch->IndexFromPoint( i, sp.pl.iLine ) ) ;
		}
		else
		{
			pp0 = S3DMeshEditor::PatchPoint
				( sp.pl.pPatch, sp.pl.pPatch->IndexFromPoint( iLine0, i ) ) ;
			pp1 = S3DMeshEditor::PatchPoint
				( sp.pl.pPatch, sp.pl.pPatch->IndexFromPoint( sp.pl.iLine, i ) ) ;
		}
		//
		// 縮退点を列挙
		//
		pps0.RemoveAll() ;
		pps1.RemoveAll() ;
		if ( !GetDegeneratedPoints( pps0, pp0 )
			|| !GetDegeneratedPoints( pps1, pp1 ) )
		{
			continue ;
		}
		for ( size_t j = 0; j < pps0.GetLength(); j ++ )
		{
			S3DMeshEditor::PatchPoint	ppt0 = pps0.At(j) ;
			if ( ppt0.pPatch == sp.pl.pPatch )
			{
				continue ;
			}
			for ( size_t k = 0; k < pps1.GetLength(); k ++ )
			{
				S3DMeshEditor::PatchPoint	ppt1 = pps1.At(k) ;
				if ( (ppt0.pPatch == ppt1.pPatch)
					&& ppt0.pPatch->IsPatchEdge( ppt0.iVertex, ppt1.iVertex )
					&& (CountPatchOfSlicePoints( aChain, ppt0.pPatch ) < 2) )
				{
					S3DMeshEditor::PatchEdge
						peChain( ppt0.pPatch, ppt0.iVertex, ppt1.iVertex ) ;
					if ( peChain.iVertex0 == ppt0.iVertex )
					{
						EnumerateChainedSlice( aChain, peChain, w ) ;
					}
					else
					{
						EnumerateChainedSlice( aChain, peChain, 1.0 - w ) ;
					}
				}
			}
		}
	}
	return	true ;
}

void S3DMeshEditorObject::CalcSlicePoints
	( SSystem::SArray<S3DVector4>& aPoints,
		const SlicePoint& sp, bool flagCloseEndPoint ) const
{
	size_t	nLineLen, nCrossLen ;
	if ( sp.pl.lineDir == S3DMeshEditor::lineHorizontal )
	{
		nLineLen = sp.pl.pPatch->GetWidth() ;
		nCrossLen = sp.pl.pPatch->GetHeight() ;
	}
	else
	{
		nLineLen = sp.pl.pPatch->GetHeight() ;
		nCrossLen = sp.pl.pPatch->GetWidth() ;
	}
	size_t	iLine0 = sp.pl.iLine - 1 ;
	if ( sp.pl.iLine == 0 )
	{
		iLine0 = nCrossLen - 1 ;
	}
	for ( size_t i = 0; i < nLineLen; i ++ )
	{
		S3DMeshEditor::PatchPoint	pp0, pp1 ;
		if ( sp.pl.lineDir == S3DMeshEditor::lineHorizontal )
		{
			pp0 = S3DMeshEditor::PatchPoint
				( sp.pl.pPatch, sp.pl.pPatch->IndexFromPoint( i, iLine0 ) ) ;
			pp1 = S3DMeshEditor::PatchPoint
				( sp.pl.pPatch, sp.pl.pPatch->IndexFromPoint( i, sp.pl.iLine ) ) ;
		}
		else
		{
			pp0 = S3DMeshEditor::PatchPoint
				( sp.pl.pPatch, sp.pl.pPatch->IndexFromPoint( iLine0, i ) ) ;
			pp1 = S3DMeshEditor::PatchPoint
				( sp.pl.pPatch, sp.pl.pPatch->IndexFromPoint( sp.pl.iLine, i ) ) ;
		}
		S3DVector	vPos0 = pp0.pPatch->GetPointAt( pp0.iVertex ) ;
		S3DVector	vPos1 = pp1.pPatch->GetPointAt( pp1.iVertex ) ;
		//
		if ( flagCloseEndPoint )
		{
			if ( sp.w < 0.5 )
			{
				if ( m_seams.FindPoint( pp0 ) >= 0 )
				{
					aPoints.Add( vPos0 ) ;
					continue ;
				}
			}
			else
			{
				if ( m_seams.FindPoint( pp1 ) >= 0 )
				{
					aPoints.Add( vPos1 ) ;
					continue ;
				}
			}
		}
		aPoints.Add( vPos0 + (vPos1 - vPos0) * sp.w ) ;
	}
}

void S3DMeshEditorObject::DoLoopSlice
	( S3DMeshEditor::PatchPointSet& ppsLine, const SlicePoint& sp,
		bool flagCloseEndPoint, S3DMeshEditorInterface * pEdit )
{
	size_t	nLineLen, nCrossLen ;
	if ( sp.pl.lineDir == S3DMeshEditor::lineHorizontal )
	{
		nLineLen = sp.pl.pPatch->GetWidth() ;
		nCrossLen = sp.pl.pPatch->GetHeight() ;
		OffsetIndexByInsertLine( ppsLine, sp.pl.pPatch, sp.pl.iLine ) ;
		pEdit->InsertLine( sp.pl.pPatch, sp.pl.iLine, (float32_t) sp.w ) ;
	}
	else
	{
		nLineLen = sp.pl.pPatch->GetHeight() ;
		nCrossLen = sp.pl.pPatch->GetWidth() ;
		OffsetIndexByInsertColumn( ppsLine, sp.pl.pPatch, sp.pl.iLine ) ;
		pEdit->InsertColumn( sp.pl.pPatch, sp.pl.iLine, (float32_t) sp.w ) ;
	}
	size_t	iLine0 = sp.pl.iLine - 1 ;
	if ( sp.pl.iLine == 0 )
	{
		iLine0 = nCrossLen - 1 ;
	}
	//
	S3DMeshEditor::PatchFaceSet	pfsLine ;
	for ( size_t i = 0; i < nLineLen; i ++ )
	{
		S3DMeshEditor::PatchPoint	pp0, pp1, pp2 ;
		if ( sp.pl.lineDir == S3DMeshEditor::lineHorizontal )
		{
			pp0 = S3DMeshEditor::PatchPoint
				( sp.pl.pPatch, sp.pl.pPatch->IndexFromPoint( i, iLine0 ) ) ;
			pp1 = S3DMeshEditor::PatchPoint
				( sp.pl.pPatch, sp.pl.pPatch->IndexFromPoint( i, sp.pl.iLine ) ) ;
			pp2 = S3DMeshEditor::PatchPoint
				( sp.pl.pPatch, sp.pl.pPatch->IndexFromPoint( i, sp.pl.iLine + 1 ) ) ;
		}
		else
		{
			pp0 = S3DMeshEditor::PatchPoint
				( sp.pl.pPatch, sp.pl.pPatch->IndexFromPoint( iLine0, i ) ) ;
			pp1 = S3DMeshEditor::PatchPoint
				( sp.pl.pPatch, sp.pl.pPatch->IndexFromPoint( sp.pl.iLine, i ) ) ;
			pp2 = S3DMeshEditor::PatchPoint
				( sp.pl.pPatch, sp.pl.pPatch->IndexFromPoint( sp.pl.iLine + 1, i ) ) ;
		}
		S3DVector	vPos0 = pp0.pPatch->GetPointAt( pp0.iVertex ) ;
		S3DVector	vPos2 = pp1.pPatch->GetPointAt( pp2.iVertex ) ;
		//
		if ( flagCloseEndPoint )
		{
			if ( sp.w < 0.5 )
			{
				if ( m_seams.FindPoint( pp0 ) >= 0 )
				{
					S3DMeshEditor::PatchPointSet	pps ;
					pps.AddSorted( pp0 ) ;
					pps.AddSorted( pp1 ) ;
					//
					sp.pl.pPatch->SetPointAt( pp1.iVertex, vPos0 ) ;
					pEdit->ShrinkPoints( pps, 0 ) ;
					continue ;
				}
			}
			else
			{
				if ( m_seams.FindPoint( pp1 ) >= 0 )
				{
					S3DMeshEditor::PatchPointSet	pps ;
					pps.AddSorted( pp1 ) ;
					pps.AddSorted( pp2 ) ;
					//
					sp.pl.pPatch->SetPointAt( pp1.iVertex, vPos2 ) ;
					pEdit->ShrinkPoints( pps, 0 ) ;
					continue ;
				}
			}
		}
		ppsLine.AddSorted( pp1 ) ;
		sp.pl.pPatch->SetPointAt
			( pp1.iVertex, vPos0 + (vPos2 - vPos0) * sp.w ) ;
		//
		S3DMeshEditor::PatchFace	pf( pp1.pPatch, pp1.iVertex ) ;
		if ( pp0.pPatch->IsValidFaceAreaAt( pp0.iVertex ) )
		{
			pf.pPatch->SetFaceAt
				( pf.iFace, pf.pPatch->GetFaceAt( pp0.iVertex ) ) ;
		}
		else
		{
			pf.pPatch->SetFaceAt( pf.iFace, S3DMeshEditor::Patch::faceNull ) ;
		}
		pfsLine.AddSorted( pf ) ;
	}
	SetUpdateVertexFlag() ;
	ClearAllSelection() ;
	//
	pEdit->NotifyUpdateFace( pfsLine ) ;
}

void S3DMeshEditorObject::DoLoopSlices
	( S3DMeshEditor::PatchPointSet& ppsLine,
		const SSystem::SArray<SlicePoint>& aChain, S3DMeshEditorInterface * pEdit )
{
	ESLAssert( pEdit != nullptr ) ;
	ppsLine.RemoveAll() ;
	for ( size_t i = 0; i < aChain.GetLength(); i ++ )
	{
		SlicePoint		sp = aChain.At(i) ;
		const size_t	iOrgLine = sp.pl.iLine ;
		for ( size_t j = 0; j < i; j ++ )
		{
			const SlicePoint&	spj = aChain.At(j) ;
			if ( (spj.pl.pPatch == sp.pl.pPatch)
				&& (spj.pl.lineDir == sp.pl.lineDir)
				&& (spj.pl.iLine < iOrgLine) )
			{
				sp.pl.iLine ++ ;
			}
		}
		DoLoopSlice( ppsLine, sp, false, pEdit ) ;
	}
	S3DMeshEditor::PatchPointSet	ppsTemp ;
	size_t	iNext = 0 ;
	while ( ppsLine.GetLength() > iNext )
	{
		S3DMeshEditor::PatchPoint	pp0 = ppsLine.At( iNext ++ ) ;
		S3DVector	vPos0 = pp0.pPatch->GetPointAt( pp0.iVertex ) ;
		//
		ppsTemp.RemoveAll() ;
		for ( size_t i = iNext; i < ppsLine.GetLength(); i ++ )
		{
			S3DMeshEditor::PatchPoint	ppi = ppsLine.At(i) ;
			S3DVector	vPosi = ppi.pPatch->GetPointAt( ppi.iVertex ) ;
			if ( (vPosi - vPos0).Absolute() < 1.0e-8 )
			{
				ppsTemp.AddSorted( ppi ) ;
				ppsLine.RemoveAt( i -- ) ;
			}
		}
		if ( ppsTemp.GetLength() > 0 )
		{
			ppsTemp.AddSorted( pp0 ) ;
			pEdit->ShrinkPoints( ppsTemp, 0 ) ;
		}
	}
}

// UV マップ関数
//////////////////////////////////////////////////////////////////////////////
S2DVector S3DMeshEditorObject::UVProjectionOrthogonalX
	( const S3DMeshEditorObject::UVMapDevParam& param,
		S3DMeshEditorObject::UVMapDevResult& uvmapdRes, const S3DVector& vPos )
{
	if ( param.globalSpace )
	{
		S3DVector	v = param.matSpace * vPos + param.vSpace ;
		return	S2DVector( v.z, v.y ) ;
	}
	return	S2DVector( vPos.z, vPos.y ) ;
}

S2DVector S3DMeshEditorObject::UVProjectionOrthogonalY
	( const S3DMeshEditorObject::UVMapDevParam& param,
		S3DMeshEditorObject::UVMapDevResult& uvmapdRes, const S3DVector& vPos )
{
	if ( param.globalSpace )
	{
		S3DVector	v = param.matSpace * vPos + param.vSpace ;
		return	S2DVector( v.x, - v.z ) ;
	}
	return	S2DVector( vPos.x, - vPos.z ) ;
}

S2DVector S3DMeshEditorObject::UVProjectionOrthogonalZ
	( const S3DMeshEditorObject::UVMapDevParam& param,
		S3DMeshEditorObject::UVMapDevResult& uvmapdRes, const S3DVector& vPos )
{
	if ( param.globalSpace )
	{
		S3DVector	v = param.matSpace * vPos + param.vSpace ;
		return	S2DVector( v.x, v.y ) ;
	}
	return	S2DVector( vPos.x, vPos.y ) ;
}

S2DVector S3DMeshEditorObject::UVProjectionPolarMapX
	( const S3DMeshEditorObject::UVMapDevParam& param,
		S3DMeshEditorObject::UVMapDevResult& uvmapdRes, const S3DVector& vPos )
{
	S3DVector	v  = vPos ;
	if ( param.globalSpace )
	{
		v = param.matSpace * vPos + param.vSpace ;
	}
	double	dx = v.z - param.vCenter.z ;
	double	dy = v.y - param.vCenter.y ;
	double	rad  = atan2( dy, dx ) ;
	uvmapdRes.fpPolarRadius += sqrt( dx * dx + dy * dy ) ;
	return	S2DVector( rad, v.x ) ;
}

S2DVector S3DMeshEditorObject::UVProjectionPolarMapY
	( const S3DMeshEditorObject::UVMapDevParam& param,
		S3DMeshEditorObject::UVMapDevResult& uvmapdRes, const S3DVector& vPos )
{
	S3DVector	v  = vPos ;
	if ( param.globalSpace )
	{
		v = param.matSpace * vPos + param.vSpace ;
	}
	double	dx = v.x - param.vCenter.x ;
	double	dy = v.z - param.vCenter.z ;
	double	rad  = atan2( dy, dx ) ;
	uvmapdRes.fpPolarRadius += sqrt( dx * dx + dy * dy ) ;
	return	S2DVector( rad, v.y ) ;
}

S2DVector S3DMeshEditorObject::UVProjectionPolarMapZ
	( const S3DMeshEditorObject::UVMapDevParam& param,
		S3DMeshEditorObject::UVMapDevResult& uvmapdRes, const S3DVector& vPos )
{
	S3DVector	v  = vPos ;
	if ( param.globalSpace )
	{
		v = param.matSpace * vPos + param.vSpace ;
	}
	double	dx = v.x - param.vCenter.x ;
	double	dy = - (v.y - param.vCenter.y) ;
	double	rad  = atan2( dy, dx ) ;
	uvmapdRes.fpPolarRadius += sqrt( dx * dx + dy * dy ) ;
	return	S2DVector( rad, v.z ) ;
}

S2DVector S3DMeshEditorObject::UVProjectionSphereMapX
	( const S3DMeshEditorObject::UVMapDevParam& param,
		S3DMeshEditorObject::UVMapDevResult& uvmapdRes, const S3DVector& vPos )
{
	S3DVector	v  = vPos ;
	if ( param.globalSpace )
	{
		v = param.matSpace * vPos + param.vSpace ;
	}
	double	dx = v.z - param.vCenter.z ;
	double	dy = v.y - param.vCenter.y ;
	double	dz = v.x - param.vCenter.x ;
	double	dxy = sqrt( dx * dx +dy * dy ) ;
	double	radX  = atan2( dy, dx ) ;
	double	radY  = atan2( dxy, - dz ) ;
	uvmapdRes.fpPolarRadius += sqrt( dxy * dxy + dz * dz ) ;
	return	S2DVector( radX, radY ) ;
}

S2DVector S3DMeshEditorObject::UVProjectionSphereMapY
	( const S3DMeshEditorObject::UVMapDevParam& param,
		S3DMeshEditorObject::UVMapDevResult& uvmapdRes, const S3DVector& vPos )
{
	S3DVector	v  = vPos ;
	if ( param.globalSpace )
	{
		v = param.matSpace * vPos + param.vSpace ;
	}
	double	dx = v.x - param.vCenter.x ;
	double	dy = v.z - param.vCenter.z ;
	double	dz = v.y - param.vCenter.y ;
	double	dxy = sqrt( dx * dx +dy * dy ) ;
	double	radX  = atan2( dy, dx ) ;
	double	radY  = atan2( dxy, - dz ) ;
	uvmapdRes.fpPolarRadius += sqrt( dxy * dxy + dz * dz ) ;
	return	S2DVector( radX, radY ) ;
}

S2DVector S3DMeshEditorObject::UVProjectionSphereMapZ
	( const S3DMeshEditorObject::UVMapDevParam& param,
		S3DMeshEditorObject::UVMapDevResult& uvmapdRes, const S3DVector& vPos )
{
	S3DVector	v  = vPos ;
	if ( param.globalSpace )
	{
		v = param.matSpace * vPos + param.vSpace ;
	}
	double	dx = v.x - param.vCenter.x ;
	double	dy = - (v.y - param.vCenter.y) ;
	double	dz = v.z - param.vCenter.z ;
	double	dxy = sqrt( dx * dx +dy * dy ) ;
	double	radX  = atan2( dy, dx ) ;
	double	radY  = atan2( dxy, - dz ) ;
	uvmapdRes.fpPolarRadius += sqrt( dxy * dxy + dz * dz ) ;
	return	S2DVector( radX, radY ) ;
}

const S3DMeshEditorObject::PFUNC_UV_PROJECTION
	S3DMeshEditorObject::m_pfnUVProjection[3][3] =
{
	{
		&S3DMeshEditorObject::UVProjectionOrthogonalX,
		&S3DMeshEditorObject::UVProjectionOrthogonalY,
		&S3DMeshEditorObject::UVProjectionOrthogonalZ,
	},
	{
		&S3DMeshEditorObject::UVProjectionPolarMapX,
		&S3DMeshEditorObject::UVProjectionPolarMapY,
		&S3DMeshEditorObject::UVProjectionPolarMapZ,
	},
	{
		&S3DMeshEditorObject::UVProjectionSphereMapX,
		&S3DMeshEditorObject::UVProjectionSphereMapY,
		&S3DMeshEditorObject::UVProjectionSphereMapZ,
	},
} ;


// UV展開
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::DoUVMapDevelopment
	( const S3DMeshEditorObject::UVMapDevParam& param, S3DMeshEditorInterface * pEdit )
{
	ESLAssert( pEdit != nullptr ) ;
	//
	// 各パッチ毎にUV展開する
	//
	SPtrSortArray<Patch,UVMapDevResult>	psaPatchs ;
	for ( size_t i = 0; i < GetPatchCount(); i ++ )
	{
		Patch *	pPatch = GetPatchAt( i ) ;
		if ( pPatch != nullptr )
		{
			if ( param.onlySelected && !IsPatchUVSelected( pPatch ) )
			{
				continue ;
			}
			UVMapDevResult	uvmapdRes ;
			if ( DoUVMapDevelopmentOfPatch( pPatch, uvmapdRes, param, pEdit ) )
			{
				psaPatchs.SetAs( pPatch, uvmapdRes ) ;
			}
		}
	}
	if ( psaPatchs.GetLength() == 0 )
	{
		return ;
	}
	//
	// ノーマライズサイズを取得する
	//
	SGLSize			sizeTexture( 1, 1 ) ;
	S3DMaterial *	pMaterial = pEdit->GetMeshMaterial() ;
	if ( (pMaterial != nullptr)
		&& !(pMaterial->m_attrSurface.flagsShading & shadingNormalizedUVScale) )
	{
		SGLImageObject *	pTexture = pMaterial->GetTexture( 0 ) ;
		if ( pTexture == nullptr )
		{
			pTexture = pMaterial->GetTexture
						( pMaterial->FindTextureTypeOf
							( S3DMaterial::textureDiffusion ) ) ;
		}
		if ( pTexture != nullptr )
		{
			sizeTexture = pTexture->GetImageSize() ;
		}
	}
	S2DVector	vMaxUV( 0, 0 ) ;
	for ( size_t i = 0; i < psaPatchs.GetLength(); i ++ )
	{
		UVMapDevResult *	puvmapdRes = psaPatchs.GetAt( i ) ;
		ESLAssert( puvmapdRes != nullptr ) ;
		S2DVector	vSize = puvmapdRes->vMaxUV - puvmapdRes->vMinUV ;
		vMaxUV.x = esl_fmaxf( vMaxUV.x, vSize.x ) ;
		vMaxUV.y = esl_fmaxf( vMaxUV.y, vSize.y ) ;
	}
	//
	if ( param.arrangePatch )
	{
		//
		// 各パッチを重ならないように並べる
		//
		const size_t	nPatchCount = psaPatchs.GetLength() ;
		SArray<SGLSize>	aPatchSizes ;
		SGLSize *		pPatchSizes = aPatchSizes.GetArray( nPatchCount ) ;
		float32_t		fpScaleSrc = esl_fminf( 1000.0f / vMaxUV.x,
											1000.f / vMaxUV.y ) ;
		for ( size_t i = 0; i < nPatchCount; i ++ )
		{
			UVMapDevResult *	puvmapdRes = psaPatchs.GetAt( i ) ;
			ESLAssert( puvmapdRes != nullptr ) ;
			S2DVector	vSize = puvmapdRes->vMaxUV - puvmapdRes->vMinUV ;
			pPatchSizes[i].w = (int32_t) esl_roundfi( vSize.x * fpScaleSrc + 0.5f ) ;
			pPatchSizes[i].h = (int32_t) esl_roundfi( vSize.y * fpScaleSrc + 0.5f ) ;
		}
		//
		SPointerArray<SGLImageRect>	aTxtMapRects ;
		SGLImageRect **	ppTxtMapRects = aTxtMapRects.GetArray( nPatchCount ) ;
		//
		SGLAreaAllocator	aalcTexture ;
		aalcTexture.SetInitialSize( 1024, 1024 ) ;
		aalcTexture.SetFlags( SGLAreaAllocator::flagSizePoweredBy2 ) ;
		aalcTexture.BatchAllocate( ppTxtMapRects, pPatchSizes, nPatchCount ) ;
		//
		SGLRect	rectAll( 0, 0, 0, 0 ) ;
		if ( ppTxtMapRects[0] != nullptr )
		{
			rectAll = *(ppTxtMapRects[0]) ;
		}
		for ( size_t i = 1; i < nPatchCount; i ++ )
		{
			if ( ppTxtMapRects[i] != nullptr )
			{
				rectAll |= *(ppTxtMapRects[i]) ;
			}
		}
		//
		float32_t	fpScaleMap =
			esl_fminf( (float) sizeTexture.w
								/ (float) (rectAll.right + 2),
						(float) sizeTexture.h
								/ (float) (rectAll.bottom + 2) ) * fpScaleSrc ;
		S2DVector	vScale( fpScaleMap, fpScaleMap ) ;
		for ( size_t i = 0; i < nPatchCount; i ++ )
		{
			ESLAssert( ppTxtMapRects[i] != nullptr ) ;
			if ( ppTxtMapRects[i] == nullptr )
			{
				continue ;
			}
			Patch *				pPatch = *(psaPatchs.GetTagAt( i )) ;
			UVMapDevResult *	puvmapdRes = psaPatchs.GetAt( i ) ;
			ESLAssert( pPatch != nullptr ) ;
			ESLAssert( puvmapdRes != nullptr ) ;
			SGLImageRect	irectMap = *(ppTxtMapRects[i]) ;
			S2DVector		vOffset
				( ((float) irectMap.x / fpScaleSrc - puvmapdRes->vMinUV.x),
					(float) irectMap.y / fpScaleSrc - puvmapdRes->vMinUV.y ) ;
			vOffset *= fpScaleMap ;
			//
			DoScaleUVMapOfPatch
				( pPatch, *puvmapdRes, vScale, vOffset, param, pEdit ) ;
		}
		//
		aPatchSizes.FinishArray() ;
		aTxtMapRects.FinishArray() ;
	}
	else
	{
		//
		// 各パッチのスケールを合わせてノーマライズする
		//
		float32_t	fpScale = esl_fminf( (float) sizeTexture.w / vMaxUV.x,
										(float) sizeTexture.h / vMaxUV.y ) ;
		S2DVector	vScale( fpScale, fpScale ) ;
		for ( size_t i = 0; i < psaPatchs.GetLength(); i ++ )
		{
			Patch *				pPatch = *(psaPatchs.GetTagAt( i )) ;
			UVMapDevResult *	puvmapdRes = psaPatchs.GetAt( i ) ;
			ESLAssert( pPatch != nullptr ) ;
			ESLAssert( puvmapdRes != nullptr ) ;
			S2DVector	vCenter =
				(puvmapdRes->vMaxUV + puvmapdRes->vMinUV) * (fpScale * 0.5f) ;
			S2DVector	vOffset =
				S2DVector( (float) sizeTexture.w * 0.5f - vCenter.x,
							(float) sizeTexture.h * 0.5f - vCenter.y ) ;
			DoScaleUVMapOfPatch
				( pPatch, *puvmapdRes, vScale, vOffset, param, pEdit ) ;
		}
	}
}

bool S3DMeshEditorObject::IsPatchUVSelected( S3DMeshEditor::Patch * pPatch ) const
{
	for ( size_t i = 0; i < m_spsUVPoints.GetLength(); i ++ )
	{
		S3DMeshEditor::SelectPoint	sp = m_spsUVPoints.At(i) ;
		if ( sp.pPatch == pPatch )
		{
			return	true ;
		}
	}
	return	false ;
}

bool S3DMeshEditorObject::DoUVMapDevelopmentOfPatch
	( Patch * pPatch, S3DMeshEditorObject::UVMapDevResult& uvmapdRes,
		const S3DMeshEditorObject::UVMapDevParam& param, S3DMeshEditorInterface * pEdit )
{
	if ( (param.method == uvDevPatchOrder)
		|| (param.method == uvDevPatchModified)
		|| (param.method == uvDevPatchOrthogonal) )
	{
		return	DoUVMapDevelopmentOfPatchOrder( pPatch, uvmapdRes, param, pEdit ) ;
	}
	//
	// 各種投影方法でUV展開
	//
	const PFUNC_UV_PROJECTION		pfnUVProj = m_pfnUVProjection[param.method][param.axis] ;
	S3DMeshEditor::SelectPointSet	spsPatchPoints ;
	if ( param.onlySelected )
	{
		for ( size_t i = 0; i < m_spsUVPoints.GetLength(); i ++ )
		{
			S3DMeshEditor::SelectPoint	sp = m_spsUVPoints.At(i) ;
			if ( sp.pPatch == pPatch )
			{
				spsPatchPoints.QuickAdd( sp ) ;
			}
		}
	}
	else
	{
		for ( size_t i = 0; i < pPatch->GetTotalVertexCount(); i ++ )
		{
			spsPatchPoints.QuickAdd( S3DMeshEditor::SelectPoint( pPatch, i ) ) ;
		}
	}
	spsPatchPoints.SortArray() ;
	if ( spsPatchPoints.GetLength() == 0 )
	{
		return	false ;
	}
	//
	uvmapdRes.fpPolarRadius = 0 ;
	//
	for ( size_t i = 0; i < spsPatchPoints.GetLength(); i ++ )
	{
		S3DMeshEditor::SelectPoint	sp = spsPatchPoints.At(i) ;
		S2DVector	uv = pfnUVProj( param, uvmapdRes,
									pPatch->GetPointAt( sp.iVertex ) ) ;
		pPatch->SetUVAt( sp.iVertex, uv ) ;
		//
		if ( i == 0 )
		{
			uvmapdRes.vMinUV = uv ;
			uvmapdRes.vMaxUV = uv ;
		}
		else
		{
			uvmapdRes.vMinUV.x = esl_fminf( uvmapdRes.vMinUV.x, uv.x ) ;
			uvmapdRes.vMinUV.y = esl_fminf( uvmapdRes.vMinUV.y, uv.y ) ;
			uvmapdRes.vMaxUV.x = esl_fmaxf( uvmapdRes.vMaxUV.x, uv.x ) ;
			uvmapdRes.vMaxUV.y = esl_fmaxf( uvmapdRes.vMaxUV.y, uv.y ) ;
		}
	}
	uvmapdRes.fpPolarRadius /= spsPatchPoints.GetLength() ;
	//
	if ( param.method == uvDevPolarMap )
	{
		//
		// 回転軸投影の場合は回転方向と軸方向でスケールが異なるので合わせる
		//
		S2DVector	vScale( uvmapdRes.fpPolarRadius, 1.0f ) ;
		S2DVector	vOffset( 0, 0 ) ;
		DoScaleUVMapOfPatch( pPatch, uvmapdRes, vScale, vOffset, param, pEdit ) ;
	}
	pEdit->NotifyUpdateVertex( spsPatchPoints ) ;
	return	true ;
}

bool S3DMeshEditorObject::DoUVMapDevelopmentOfPatchOrder
	( S3DMeshEditor::Patch * pPatch,
		S3DMeshEditorObject::UVMapDevResult& uvmapdRes,
		const S3DMeshEditorObject::UVMapDevParam& param, S3DMeshEditorInterface * pEdit )
{
	//
	// パッチ順序に従ってUV展開
	//
	SArray<LineWidth>	aLineWidth ;
	SArray<LineWidth>	aLineHeight ;
	SArray<LineRange>	aHorzRange ;
	SArray<LineRange>	aVertRange ;
	const size_t	nPatchWidth = pPatch->GetWidth() ;
	const size_t	nPatchHeight = pPatch->GetHeight() ;
	const size_t	nFaceWidth = pPatch->GetFaceWidth() ;
	const size_t	nFaceHeight = pPatch->GetFaceHeight() ;
	const size_t	nAreaSize = pPatch->GetAreaSize() ;
	LineWidth *		pLineWidth = aLineWidth.GetArray( nFaceWidth + 1 ) ;
	LineWidth *		pLineHeight = aLineHeight.GetArray( nFaceHeight + 1 ) ;
	//
	// 選択頂点の判別と以前の値の保存
	//
	SArray<S2DVector>	aSaveUVs ;
	SBitArray			aSelMask ;
	aSaveUVs.AddArray( pPatch->GetConstUVArray( 0, nAreaSize ), nAreaSize ) ;
	aSelMask.SetLength( nAreaSize ) ;
	if ( param.onlySelected )
	{
		for ( size_t i = 0; i < m_spsUVPoints.GetLength(); i ++ )
		{
			S3DMeshEditor::SelectPoint	sp = m_spsUVPoints.At(i) ;
			if ( (sp.pPatch == pPatch)
				&& (sp.iVertex < nAreaSize) )
			{
				aSelMask.SetAt( sp.iVertex, true ) ;
			}
		}
	}
	else
	{
		aSelMask.Fill( true ) ;
	}
	//
	// 垂直ラインの各幅計算
	//
	size_t		iLast = 0 ;
	float32_t	fpTotalWidth = 0.0f ;
	float32_t	fpSumWidth = 0.0f ;
	float32_t	fpSumMaxWidth = 0.0f ;
	for ( size_t x = 0; x < nFaceWidth; x ++ )
	{
		size_t		x1 = (x + 1) % nPatchWidth ;
		bool		flagNullCol = true ;
		float32_t	fpMaxWidth = 0.0f ;
		float32_t	fpAvgWidth = 0.0f ;
		for ( size_t y = 0; y < nPatchHeight; y ++ )
		{
			float32_t	r =
				(float32_t) (pPatch->GetPoint( x, y )
								- pPatch->GetPoint( x1, y )).Absolute() ;
			if ( flagNullCol
				&& (pPatch->GetFace( x, y ) != Patch::faceNull) )
			{
				flagNullCol = false ;
			}
			fpMaxWidth = esl_fmaxf( fpMaxWidth, r ) ;
			fpAvgWidth += r ;
		}
		fpAvgWidth /= nPatchHeight ;
		//
		pLineWidth[x].fpPos = fpTotalWidth ;
		pLineWidth[x].fpMaxWidth = fpMaxWidth ;
		pLineWidth[x].fpWidth = fpAvgWidth ;
		//
		if ( flagNullCol )
		{
			if ( iLast < x )
			{
				LineRange	lr ;
				lr.nIndex = iLast ;
				lr.nCount = x - iLast ;
				lr.fpWidth = fpSumWidth ;
				lr.fpMaxWidth = fpSumMaxWidth ;
				aHorzRange.Add( lr ) ;
			}
			iLast = x + 1 ;
			fpSumWidth = 0.0f ;
			fpSumMaxWidth = 0.0f ;
		}
		else
		{
			fpSumWidth += fpAvgWidth ;
			fpSumMaxWidth += fpMaxWidth ;
		}
		fpTotalWidth += fpAvgWidth ;
	}
	pLineWidth[nFaceWidth].fpPos = fpTotalWidth ;
	//
	if ( iLast < nFaceWidth )
	{
		LineRange	lr ;
		lr.nIndex = iLast ;
		lr.nCount = nFaceWidth - iLast ;
		lr.fpWidth = fpSumWidth ;
		lr.fpMaxWidth = fpSumMaxWidth ;
		aHorzRange.Add( lr ) ;
	}
	if ( aHorzRange.GetLength() == 0 )
	{
		return	false ;
	}
	//
	// 水平ラインの各高さ計算
	//
	float32_t	fpTotalHeight = 0.0f ;
	fpSumWidth = 0.0f ;
	fpSumMaxWidth = 0.0f ;
	iLast = 0 ;
	for ( size_t y = 0; y < nFaceHeight; y ++ )
	{
		size_t		y1 = (y + 1) % nPatchHeight ;
		bool		flagNullLine = true ;
		float32_t	fpMaxWidth = 0.0f ;
		float32_t	fpAvgWidth = 0.0f ;
		for ( size_t x = 0; x < nPatchWidth; x ++ )
		{
			float32_t	r =
				(float32_t) (pPatch->GetPoint( x, y )
								- pPatch->GetPoint( x, y1 )).Absolute() ;
			if ( flagNullLine
				&& (pPatch->GetFace( x, y ) != Patch::faceNull) )
			{
				flagNullLine = false ;
			}
			fpMaxWidth = esl_fmaxf( fpMaxWidth, r ) ;
			fpAvgWidth += r ;
		}
		fpAvgWidth /= nPatchHeight ;
		//
		pLineHeight[y].fpPos = fpTotalHeight ;
		pLineHeight[y].fpMaxWidth = fpMaxWidth ;
		pLineHeight[y].fpWidth = fpAvgWidth ;
		//
		if ( flagNullLine )
		{
			if ( iLast < y )
			{
				LineRange	lr ;
				lr.nIndex = iLast ;
				lr.nCount = y - iLast ;
				lr.fpWidth = fpSumWidth ;
				lr.fpMaxWidth = fpSumMaxWidth ;
				aVertRange.Add( lr ) ;
			}
			iLast = y + 1 ;
			fpSumWidth = 0.0f ;
			fpSumMaxWidth = 0.0f ;
		}
		else
		{
			fpSumWidth += fpAvgWidth ;
			fpSumMaxWidth += fpMaxWidth ;
		}
		fpTotalHeight += fpAvgWidth ;
	}
	pLineHeight[nFaceHeight].fpPos = fpTotalHeight ;
	//
	if ( iLast < nFaceHeight )
	{
		LineRange	lr ;
		lr.nIndex = iLast ;
		lr.nCount = nFaceHeight - iLast ;
		lr.fpWidth = fpSumWidth ;
		lr.fpMaxWidth = fpSumMaxWidth ;
		aVertRange.Add( lr ) ;
	}
	if ( aVertRange.GetLength() == 0 )
	{
		return	false ;
	}
	//
	// 仮の設定（空の頂点含めて）
	//
	for ( size_t y = 0; y < nPatchHeight; y ++ )
	{
		float32_t	yUV = pLineHeight[y].fpPos ;
		for ( size_t x = 0; x < nPatchWidth; x ++ )
		{
			float32_t	xUV = pLineWidth[x].fpPos ;
			pPatch->SetUV( x, y, S2DVector( xUV, yUV ) ) ;
		}
	}
	//
	// 各領域毎にマッピング
	//
	float32_t	xLastRange = 0.0f ;
	for ( size_t xBlock = 0; xBlock < aHorzRange.GetLength(); xBlock ++ )
	{
		const LineRange	lrXRange = aHorzRange.At( xBlock ) ;
		const float32_t	xRangeWidth = lrXRange.fpWidth ;
		float32_t		yLastRange = 0.0f ;
		//
		for ( size_t yBlock = 0; yBlock < aVertRange.GetLength(); yBlock ++ )
		{
			const LineRange	lrYRange = aVertRange.At( yBlock ) ;
			const float32_t	yRangeHeight = lrYRange.fpWidth ;
			//
			PolarUVMapInfo	puvInfo ;
			if ( param.autoPolarMap
				&& (param.method == uvDevPatchModified)
				&& IsPolarUVMapOfPatchOrderBlock
					( puvInfo, pPatch, lrXRange, lrYRange ) )
			{
				DoPolarUVMapDevelopmentOfPatchOrder
					( pPatch, puvInfo,
						xLastRange, yLastRange, xRangeWidth, yRangeHeight, param ) ;
			}
			else if ( param.method == uvDevPatchModified )
			{
				DoUVMapDevelopmentOfPatchOrderModified
					( pPatch, lrXRange, lrYRange,
						xLastRange, yLastRange, xRangeWidth, yRangeHeight,
						fpTotalWidth, fpTotalHeight, param ) ;
			}
			else if ( param.method == uvDevPatchOrthogonal )
			{
				DoUVMapDevelopmentOfPatchBlockOrthogonal
					( pPatch, lrXRange, lrYRange,
						xLastRange, yLastRange, xRangeWidth, yRangeHeight,
						fpTotalWidth, fpTotalHeight, param ) ;
			}
			else //  if ( param.method == uvDevPatchOrder )
			{
				DoUVMapDevelopmentOfPatchOrderBlock
					( pPatch, lrXRange, lrYRange,
						xLastRange, yLastRange, xRangeWidth, yRangeHeight,
						fpTotalWidth, fpTotalHeight, param ) ;
			}
			//
			yLastRange += yRangeHeight ;
		}
		//
		xLastRange += xRangeWidth ;
	}
	aLineWidth.FinishArray() ;
	aLineHeight.FinishArray() ;
	//
	// 非選択点を以前の値に戻す
	//
	for ( size_t i = 0; i < nAreaSize; i ++ )
	{
		if ( !aSelMask.GetAt( i ) )
		{
			pPatch->SetUVAt( i, aSaveUVs.At(i) ) ;
		}
	}
	//
	// UV更新通知
	//
	S3DMeshEditor::SelectPointSet	spsPatchPoints ;
	//
	uvmapdRes.vMinUV = S2DVector( 0, 0 ) ;
	uvmapdRes.vMaxUV = S2DVector( 0, 0 ) ;
	uvmapdRes.fpPolarRadius = 1.0f ;
	//
	if ( nAreaSize > 0 )
	{
		bool	flagFirstSel = true ;
		for ( size_t i = 0; i < nAreaSize; i ++ )
		{
			if ( !aSelMask.GetAt( i ) )
			{
				continue ;
			}
			spsPatchPoints.QuickAdd( S3DMeshEditor::SelectPoint( pPatch, i ) ) ;
			//
			S2DVector	uv = pPatch->GetUVAt( i ) ;
			if ( flagFirstSel )
			{
				flagFirstSel = false ;
				uvmapdRes.vMinUV = uv ;
				uvmapdRes.vMaxUV = uv ;
			}
			else
			{
				uvmapdRes.vMinUV.x = esl_fminf( uvmapdRes.vMinUV.x, uv.x ) ;
				uvmapdRes.vMinUV.y = esl_fminf( uvmapdRes.vMinUV.y, uv.y ) ;
				uvmapdRes.vMaxUV.x = esl_fmaxf( uvmapdRes.vMaxUV.x, uv.x ) ;
				uvmapdRes.vMaxUV.y = esl_fmaxf( uvmapdRes.vMaxUV.y, uv.y ) ;
			}
		}
	}
	spsPatchPoints.SortArray() ;
	//
	pEdit->NotifyUpdateVertex( spsPatchPoints ) ;
	return	true ;
}

bool S3DMeshEditorObject::IsPolarUVMapOfPatchOrderBlock
	( S3DMeshEditorObject::PolarUVMapInfo& puvInfo,
		S3DMeshEditor::Patch * pPatch,
		const S3DMeshEditorObject::LineRange& lrXRange,
		const S3DMeshEditorObject::LineRange& lrYRange )
{
	SGLPoint	ptOrg[4] =
	{
		SGLPoint( (int) lrXRange.nIndex, (int) lrYRange.nIndex ),
		SGLPoint( (int) lrXRange.nIndex, (int) lrYRange.nIndex ),
		SGLPoint( (int) (lrXRange.nIndex + lrXRange.nCount),
					(int) (lrYRange.nIndex + lrYRange.nCount) ),
		SGLPoint( (int) (lrXRange.nIndex + lrXRange.nCount),
					(int) (lrYRange.nIndex + lrYRange.nCount) ),
	} ;
	size_t		nLineWidth[4] =
	{
		lrXRange.nCount, lrYRange.nCount,
		lrXRange.nCount, lrYRange.nCount,
	} ;
	size_t		nColHeight[4] =
	{
		lrYRange.nCount, lrXRange.nCount,
		lrYRange.nCount, lrXRange.nCount,
	} ;
	SGLPoint	ptHorzDelta[4] =
	{
		SGLPoint( 1, 0 ), SGLPoint( 0, 1 ),
		SGLPoint( -1, 0 ), SGLPoint( 0, -1 ),
	} ;
	SGLPoint	ptVertDelta[4] =
	{
		SGLPoint( 0, 1 ), SGLPoint( 1, 0 ),
		SGLPoint( 0, -1 ), SGLPoint( -1, 0 ),
	} ;
	bool	flagPolar[4] = { false, false, false, false } ;
	//
	// 端辺の縮退判定
	//
	for ( int i = 0; i < 4; i ++ )
	{
		SGLPoint	pt = ptOrg[i] ;
		uint32_t	nDeg =
			pPatch->GetDegenerateNumber
				( pt.x % pPatch->GetWidth(), pt.y % pPatch->GetHeight() ) ;
		if ( nDeg == 0 )
		{
			continue ;
		}
		flagPolar[i] = true ;
		for ( size_t j = 1; j <= nLineWidth[i]; j ++ )
		{
			pt += ptHorzDelta[i] ;
			pt.x %= pPatch->GetWidth() ;
			pt.y %= pPatch->GetHeight() ;
			if ( pPatch->GetDegenerateNumber( pt.x, pt.y ) != nDeg )
			{
				flagPolar[i] = false ;
				break ;
			}
		}
	}
	size_t	iPolar = 0 ;
	if ( flagPolar[0] && !flagPolar[2] )
	{
		puvInfo.linePolar.lineDir = lineHorizontal ;
		puvInfo.linePolar.iLine = lrYRange.nIndex ;
		iPolar = 0 ;
	}
	else if ( !flagPolar[0] && flagPolar[2] )
	{
		puvInfo.linePolar.lineDir = lineHorizontal ;
		puvInfo.linePolar.iLine = lrYRange.nIndex + lrYRange.nCount ;
		iPolar = 2 ;
	}
	else if ( flagPolar[1] && !flagPolar[3] )
	{
		puvInfo.linePolar.lineDir = lineVertical ;
		puvInfo.linePolar.iLine = lrXRange.nIndex ;
		iPolar = 1 ;
	}
	else if ( !flagPolar[1] && flagPolar[3] )
	{
		puvInfo.linePolar.lineDir = lineVertical ;
		puvInfo.linePolar.iLine = lrXRange.nIndex + lrXRange.nCount ;
		iPolar = 3 ;
	}
	else
	{
		return	false ;
	}
	puvInfo.nLineWidth = nLineWidth[iPolar] ;
	puvInfo.nColHeight = nColHeight[iPolar] ;
	puvInfo.ptPolar = ptOrg[iPolar] ;
	puvInfo.ptHorzDelta = ptHorzDelta[iPolar] ;
	puvInfo.ptVertDelta = ptVertDelta[iPolar] ;
	//
	// ループ判定
	//
	SGLPoint	ptNext = ptOrg[iPolar] ;
	puvInfo.flagHorzLoop = true ;
	for ( size_t i = 0; i <= nColHeight[iPolar]; i ++ )
	{
		uint32_t	nDeg = pPatch->GetDegenerateNumber( ptNext.x, ptNext.y ) ;
		if ( nDeg == 0 )
		{
			puvInfo.flagHorzLoop = false ;
			break ;
		}
		SGLPoint	ptEnd =
			ptNext + ptHorzDelta[iPolar] * (int32_t) nLineWidth[iPolar] ;
		ptEnd.x %= pPatch->GetWidth() ;
		ptEnd.y %= pPatch->GetHeight() ;
		//
		if ( pPatch->GetDegenerateNumber( ptEnd.x, ptEnd.y ) != nDeg )
		{
			puvInfo.flagHorzLoop = false ;
			continue ;
		}
		ptNext += ptVertDelta[iPolar] ;
		ptNext.x %= pPatch->GetWidth() ;
		ptNext.y %= pPatch->GetHeight() ;
	}
	return	true ;
}

void S3DMeshEditorObject::DoUVMapDevelopmentOfPatchOrderBlock
	( S3DMeshEditorObject::Patch * pPatch,
		const S3DMeshEditorObject::LineRange& lrXRange,
		const S3DMeshEditorObject::LineRange& lrYRange,
		float32_t xRangeLeft, float32_t yRangeTop,
		float32_t xRangeWidth, float32_t yRangeHeight,
		float32_t fpTotalWidth, float32_t fpTotalHeight,
		const S3DMeshEditorObject::UVMapDevParam& param )
{
	if ( (lrXRange.nCount == 0)
		|| (lrYRange.nCount == 0) )
	{
		return ;
	}
	SArray<LineWidth>	aLineWidth ;
	SArray<LineWidth>	aLineHeight ;
	const size_t	nPatchWidth = pPatch->GetWidth() ;
	const size_t	nPatchHeight = pPatch->GetHeight() ;
	LineWidth *		pLineWidth = aLineWidth.GetArray( lrXRange.nCount + 1 ) ;
	LineWidth *		pLineHeight = aLineHeight.GetArray( lrYRange.nCount + 1 ) ;
	//
	// 垂直ラインの各幅計算
	//
	float32_t	fpSumWidth = 0.0f ;
	for ( size_t x = 0; x <= lrXRange.nCount; x ++ )
	{
		size_t		x0 = (x + lrXRange.nIndex) % nPatchWidth ;
		size_t		x1 = (x0 + 1) % nPatchWidth ;
		float32_t	fpMaxWidth = 0.0f ;
		float32_t	fpAvgWidth = 0.0f ;
		for ( size_t y = 0; y < nPatchHeight; y ++ )
		{
			float32_t	r =
				(float32_t) (pPatch->GetPoint( x0, y )
								- pPatch->GetPoint( x1, y )).Absolute() ;
			fpMaxWidth = esl_fmaxf( fpMaxWidth, r ) ;
			fpAvgWidth += r ;
		}
		fpAvgWidth /= nPatchHeight ;
		//
		pLineWidth[x].fpPos = fpSumWidth ;
		pLineWidth[x].fpWidth = fpAvgWidth ;
		pLineWidth[x].fpMaxWidth = fpMaxWidth ;
		//
		fpSumWidth += fpAvgWidth ;
	}
	fpSumWidth -= pLineWidth[lrXRange.nCount].fpWidth ;
	//
	// 水平ラインの各高さ計算
	//
	float32_t	fpSumHeight = 0.0f ;
	for ( size_t y = 0; y <= lrYRange.nCount; y ++ )
	{
		size_t		y0 = (y + lrYRange.nIndex) % nPatchHeight ;
		size_t		y1 = (y0 + 1) % nPatchHeight ;
		float32_t	fpMaxHeight = 0.0f ;
		float32_t	fpAvgHeight = 0.0f ;
		for ( size_t x = 0; x < nPatchWidth; x ++ )
		{
			float32_t	r =
				(float32_t) (pPatch->GetPoint( x, y0 )
								- pPatch->GetPoint( x, y1 )).Absolute() ;
			fpMaxHeight = esl_fmaxf( fpMaxHeight, r ) ;
			fpAvgHeight += r ;
		}
		fpAvgHeight /= nPatchWidth ;
		//
		pLineHeight[y].fpPos = fpSumHeight ;
		pLineHeight[y].fpWidth = fpAvgHeight ;
		pLineHeight[y].fpMaxWidth = fpMaxHeight ;
		//
		fpSumHeight += fpAvgHeight ;
	}
	fpSumHeight -= pLineHeight[lrYRange.nCount].fpWidth ;
	//
	// 配置中心とスケール
	//
	float32_t	xRangeCenter = xRangeLeft + xRangeWidth * 0.5f ;
	float32_t	yRangeCenter = yRangeTop + yRangeHeight * 0.5f ;
	float32_t	xScale = 0.98f, yScale = 0.98f ;
	if ( fpSumWidth > xRangeWidth )
	{
		xScale = xRangeWidth / fpSumWidth * 0.98f ;
	}
	if ( fpSumHeight > yRangeHeight )
	{
		yScale = yRangeHeight / fpSumHeight * 0.98f ;
	}
	xScale = esl_fminf( xScale, yScale ) ;
	yScale = xScale ;
	//
	// UV計算
	//
	float32_t	yNext = yRangeCenter - fpSumHeight * yScale * 0.5f ;
	for ( size_t y = 0; y <= lrYRange.nCount; y ++ )
	{
		size_t	yi = y + lrYRange.nIndex ;
		if ( yi >= nPatchHeight )
		{
			break ;
		}
		float32_t	xNext = xRangeCenter - fpSumWidth * xScale * 0.5f ;
		for ( size_t x = 0; x <= lrXRange.nCount; x ++ )
		{
			size_t	xi = x + lrXRange.nIndex ;
			if ( xi >= nPatchWidth )
			{
				break ;
			}
			pPatch->SetUV( xi, yi, S2DVector( xNext, yNext ) ) ;
			xNext += pLineWidth[x].fpWidth * xScale ;
		}
		yNext += pLineHeight[y].fpWidth * yScale ;
	}
	aLineWidth.FinishArray() ;
	aLineHeight.FinishArray() ;
}

void S3DMeshEditorObject::DoUVMapDevelopmentOfPatchOrderModified
	( S3DMeshEditor::Patch * pPatch,
		const S3DMeshEditorObject::LineRange& lrXRange,
		const S3DMeshEditorObject::LineRange& lrYRange,
		float32_t xRangeLeft, float32_t yRangeTop,
		float32_t xRangeWidth, float32_t yRangeHeight,
		float32_t fpTotalWidth, float32_t fpTotalHeight,
		const S3DMeshEditorObject::UVMapDevParam& param )
{
	if ( (lrXRange.nCount == 0)
		|| (lrYRange.nCount == 0) )
	{
		return ;
	}
	SArray<float32_t>	aLineWidth ;
	SArray<float32_t>	aLineHeight ;
	SArray<float32_t>	aHorzEdgeWidth ;
	SArray<float32_t>	aVertEdgeHeight ;
	const size_t	nPatchWidth = pPatch->GetWidth() ;
	const size_t	nPatchHeight = pPatch->GetHeight() ;
	float32_t *		pLineWidth = aLineWidth.GetArray( lrYRange.nCount + 1 ) ;
	float32_t *		pLineHeight = aLineHeight.GetArray( lrXRange.nCount + 1 ) ;
	float32_t *		pHorzEdgeWidth =
						aHorzEdgeWidth.GetArray( lrXRange.nCount * (lrYRange.nCount + 1) ) ;
	float32_t *		pVertEdgeHeight =
						aVertEdgeHeight.GetArray( (lrXRange.nCount + 1) * lrYRange.nCount ) ;
	//
	// 各水平ラインの長さ計算
	//
	float32_t	fpMaxWidth = 0.0f ;
	size_t		yMaxHorzLine = 0 ;
	for ( size_t y = 0; y <= lrYRange.nCount; y ++ )
	{
		size_t		yi = (y + lrYRange.nIndex) % nPatchHeight ;
		float32_t	fpLineWidth = 0.0f ;
		for ( size_t x = 0; x < lrXRange.nCount; x ++ )
		{
			size_t		x0 = x + lrXRange.nIndex ;
			size_t		x1 = (x0 + 1) % nPatchWidth ;
			float32_t	r =
				(float32_t) (pPatch->GetPoint( x0, yi )
								- pPatch->GetPoint( x1, yi )).Absolute() ;
			pHorzEdgeWidth[y * lrXRange.nCount + x] = r ;
			fpLineWidth += r ;
		}
		pLineWidth[y] = fpLineWidth ;
		//
		if ( y + lrYRange.nIndex < nPatchHeight )
		{
			if ( (fpMaxWidth < fpLineWidth)
				|| ((fabs(fpMaxWidth - fpLineWidth) < fpMaxWidth * 0.05f)
					&& (esl_abs( (int) y - (int) lrYRange.nCount / 2 )
						< esl_abs( (int) yMaxHorzLine - (int) lrYRange.nCount / 2 ) )) )
			{
				fpMaxWidth = fpLineWidth ;
				yMaxHorzLine = y ;
			}
		}
	}
	//
	// 各垂直ラインの長さ計算
	//
	float32_t	fpMaxHeight = 0.0f ;
	size_t		xMaxVertLine = 0 ;
	for ( size_t x = 0; x <= lrXRange.nCount; x ++ )
	{
		size_t		xi = (x + lrXRange.nIndex) % nPatchWidth ;
		float32_t	fpLineHeight = 0.0f ;
		for ( size_t y = 0; y < lrYRange.nCount; y ++ )
		{
			size_t	y0 = y + lrYRange.nIndex ;
			size_t	y1 = (y0 + 1) % nPatchHeight ;
			float32_t	r =
				(float32_t) (pPatch->GetPoint( xi, y0 )
								- pPatch->GetPoint( xi, y1 )).Absolute() ;
			pVertEdgeHeight[y * (lrXRange.nCount + 1) + x] = r ;
			fpLineHeight += r ;
		}
		pLineHeight[x] = fpLineHeight ;
		//
		if ( x + lrXRange.nIndex < nPatchWidth )
		{
			if ( (fpMaxHeight < fpLineHeight)
				|| ((fabs(fpMaxHeight - fpLineHeight) < fpMaxHeight * 0.05f)
					&& ( esl_abs( (int) x - (int) lrXRange.nCount / 2 )
							< esl_abs( (int) xMaxVertLine - (int) lrXRange.nCount / 2 ) )) )
			{
				fpMaxHeight = fpLineHeight ;
				xMaxVertLine = x ;
			}
		}
	}
	//
	// 基準座標とスケール
	//
	float32_t	xScale = esl_fminf( 1.0f, fpTotalWidth / fpMaxWidth ) ;
	float32_t	yScale = esl_fminf( 1.0f, fpTotalHeight / fpMaxHeight ) ;
	float32_t	xOffset = xRangeLeft + (fpTotalWidth - fpMaxWidth * xScale) * 0.5f ;
	float32_t	yOffset = yRangeTop + (fpTotalHeight - fpMaxHeight * yScale) * 0.5f ;
	float32_t	xCenter = 0.0f ;
	float32_t	yCenter = 0.0f ;
	for ( size_t x = 0; x < xMaxVertLine; x ++ )
	{
		xCenter += pHorzEdgeWidth[yMaxHorzLine * lrXRange.nCount + x] ;
	}
	for ( size_t y = 0; y < yMaxHorzLine; y ++ )
	{
		yCenter += pVertEdgeHeight[y * (lrXRange.nCount + 1) + xMaxVertLine] ;
	}
	xCenter = xOffset + xCenter * xScale ;
	yCenter = yOffset + yCenter * yScale ;
	//
	// 軸計算
	//
	float32_t	yNext = yOffset ;
	for ( size_t y = 0; y <= lrYRange.nCount; y ++ )
	{
		size_t	yi = y + lrYRange.nIndex ;
		if ( yi >= nPatchHeight )
		{
			break ;
		}
		pPatch->SetUV
			( xMaxVertLine + lrXRange.nIndex, yi, S2DVector( xCenter, yNext ) ) ;
		yNext += pVertEdgeHeight[y * (lrXRange.nCount + 1) + xMaxVertLine] * yScale ;
	}
	float32_t	xNext = xOffset ;
	for ( size_t x = 0; x <= lrXRange.nCount; x ++ )
	{
		size_t	xi = x + lrXRange.nIndex ;
		if ( xi >= nPatchWidth )
		{
			break ;
		}
		pPatch->SetUV
			( xi, yMaxHorzLine + lrYRange.nIndex, S2DVector( xNext, yCenter ) ) ;
		xNext += pHorzEdgeWidth[yMaxHorzLine * lrXRange.nCount + x] * xScale ;
	}
	//
	// 補完延長
	//
	SGLPoint	ptDir[4] =
	{
		SGLPoint( -1, -1 ), SGLPoint( 1, -1 ), 
		SGLPoint( -1, 1 ), SGLPoint( 1, 1 ), 
	} ;
	for ( int iDir = 0; iDir < 4; iDir ++ )
	{
		SGLPoint	ptCoord( (int) xMaxVertLine, (int) yMaxHorzLine ) ;
		for ( ; ; )
		{
			ptCoord += ptDir[iDir] ;
			if ( (ptCoord.x < 0) || (ptCoord.y < 0)
				|| ((size_t) ptCoord.x >= nPatchWidth)
				|| ((size_t) ptCoord.y >= nPatchHeight) )
			{
				break ;
			}
			// 水平・垂直方向へ
			SGLPoint	ptLoopDir[2] =
			{
				SGLPoint( ptDir[iDir].x, 0 ),
				SGLPoint( 0, ptDir[iDir].y ),
			} ;
			SGLPoint	ptBlockBase( (int) lrXRange.nIndex, (int) lrYRange.nIndex ) ;
			for ( int i = 0; i < 2; i ++ )
			{
				SGLPoint	ptNext = ptCoord ;
				while ( (ptNext.x >= 0)
						&& (ptNext.y >= 0)
						&& (ptNext.x <= (int) lrXRange.nCount)
						&& (ptNext.y <= (int) lrYRange.nCount)
						&& ((size_t) ptNext.x + lrXRange.nIndex < nPatchWidth)
						&& ((size_t) ptNext.y + lrYRange.nIndex < nPatchHeight) )
				{
					SGLPoint	ptLastX = ptNext ;
					SGLPoint	ptLastY = ptNext ;
					SGLPoint	ptLastXY = ptNext ;
					ptLastX.x -= ptDir[iDir].x ;
					ptLastY.y -= ptDir[iDir].y ;
					ptLastXY -= ptDir[iDir] ;
					//
					int	xEdge = esl_min( ptNext.x, ptLastX.x ) ;
					int	yEdge = esl_min( ptNext.y, ptLastY.y ) ;
					//
					ptLastX += ptBlockBase ;
					ptLastY += ptBlockBase ;
					ptLastXY += ptBlockBase ;
					ptLastX.x %= nPatchWidth ;
					ptLastX.y %= nPatchHeight ;
					ptLastY.x %= nPatchWidth ;
					ptLastY.y %= nPatchHeight ;
					ptLastXY.x %= nPatchWidth ;
					ptLastXY.y %= nPatchHeight ;
					//
					S2DVector	uvOrg = pPatch->GetUV( (size_t) ptLastXY.x, (size_t) ptLastXY.y ) ;
					S2DVector	uvLastX = pPatch->GetUV( (size_t) ptLastX.x, (size_t) ptLastX.y ) ;
					S2DVector	uvLastY = pPatch->GetUV( (size_t) ptLastY.x, (size_t) ptLastY.y ) ;
					S2DVector	uvDirX = (uvLastY - uvOrg).Normalized() ;
					S2DVector	uvDirY = (uvLastX - uvOrg).Normalized() ;
					float32_t	fpEdgeLenX = pHorzEdgeWidth[yEdge * lrXRange.nCount + xEdge] * xScale ;
					float32_t	fpEdgeLenY = pVertEdgeHeight[yEdge * (lrXRange.nCount + 1) + xEdge] * yScale ;
					S2DVector	uvTempX = uvLastX + uvDirX * fpEdgeLenX ;
					S2DVector	uvTempY = uvLastY + uvDirY * fpEdgeLenY ;
					S2DVector	uvTemp = (uvTempX + uvTempY) * 0.5f ;
					//
					pPatch->SetUV
						( (size_t) ptNext.x + lrXRange.nIndex,
							(size_t) ptNext.y + lrYRange.nIndex, uvTemp ) ;
					//
					ptNext += ptLoopDir[i] ;
				}
			}
		}
	}
	aLineWidth.FinishArray() ;
	aLineHeight.FinishArray() ;
	aHorzEdgeWidth.FinishArray() ;
	aVertEdgeHeight.FinishArray() ;
	//
	// UV が指定範囲内に治まるようにスケーリング
	//
	PolarUVMapInfo	puvInfo ;
	puvInfo.nLineWidth = lrXRange.nCount ;
	puvInfo.nColHeight = lrYRange.nCount ;
	puvInfo.ptPolar.x = (int32_t) lrXRange.nIndex ;
	puvInfo.ptPolar.y = (int32_t) lrYRange.nIndex ;
	puvInfo.ptHorzDelta = SGLPoint( 1, 0 ) ;
	puvInfo.ptVertDelta = SGLPoint( 0, 1 ) ;
	//
	S2DVector	vUVMin( 0, 0 ) ;
	S2DVector	vUVMax( 0, 0 ) ;
	GetUVMinMaxOfPatchOrderBlock( vUVMin, vUVMax, pPatch, puvInfo ) ;
	//
	float32_t	fpScaleUV = esl_fminf( xRangeWidth / (vUVMax.x - vUVMin.x),
										yRangeHeight / (vUVMax.y - vUVMin.y) ) ;
	fpScaleUV *= 0.95f ;
	//
	S2DVector	vOffsetUV ;
	vOffsetUV.x = xRangeLeft + xRangeWidth * 0.5f
						- (vUVMax.x + vUVMin.x) * 0.5f * fpScaleUV ;
	vOffsetUV.y = yRangeTop + yRangeHeight * 0.5f
						- (vUVMax.y + vUVMin.y) * 0.5f * fpScaleUV ;
	//
	ScaleUVMapOfPatchOrderBlock( pPatch, puvInfo, fpScaleUV, vOffsetUV ) ;
}

void S3DMeshEditorObject::DoPolarUVMapDevelopmentOfPatchOrder
	( S3DMeshEditor::Patch * pPatch,
		const S3DMeshEditorObject::PolarUVMapInfo& puvInfo,
		float32_t xRangeLeft, float32_t yRangeTop,
		float32_t xRangeWidth, float32_t yRangeHeight,
		const S3DMeshEditorObject::UVMapDevParam& param )
{
	//
	// 縮退した開始点を設定
	//
	const int	nPatchWidth = (int) pPatch->GetWidth() ;
	const int	nPatchHeight = (int) pPatch->GetHeight() ;
	SGLPoint	ptVertex = puvInfo.ptPolar ;
	for ( size_t i = 0; i <= puvInfo.nLineWidth; i ++ )
	{
		if ( ((size_t) ptVertex.x < (size_t) nPatchWidth)
			&& ((size_t) ptVertex.y < (size_t) nPatchHeight) )
		{
			pPatch->SetUV( ptVertex.x, ptVertex.y, S2DVector( 0, 0 ) ) ;
		}
		ptVertex += puvInfo.ptHorzDelta ;
	}
	//
	// 順次展開
	//
	SArray<float32_t>	aVertLen ;
	SArray<float32_t>	aHorzLen ;
	float32_t *	pfpVertLen = aVertLen.GetArray( puvInfo.nLineWidth + 1 ) ;
	float32_t *	pfpHorzLen = aHorzLen.GetArray( puvInfo.nLineWidth + 1 ) ;
	//
	float32_t	fpAccVertLen = 0.0f ;
	ptVertex = puvInfo.ptPolar ;
	for ( size_t i = 1; i <= puvInfo.nColHeight; i ++ )
	{
		SGLPoint	ptLastLine = ptVertex ;
		ptVertex += puvInfo.ptVertDelta ;
		ptVertex.x %= nPatchWidth ;
		ptVertex.y %= nPatchHeight ;
		//
		// 各稜線の距離を計算
		//
		float32_t	fpSumVertLen = 0.0f ;
		float32_t	fpSumHorzLen = 0.0f ;
		SGLPoint	ptNext = ptVertex ;
		S3DVector	vLast = pPatch->GetPoint( ptNext.x, ptNext.y ) ;
		for ( size_t j = 0; j <= puvInfo.nLineWidth; j ++ )
		{
			ptLastLine.x %= nPatchWidth ;
			ptLastLine.y %= nPatchHeight ;
			ptNext.x %= nPatchWidth ;
			ptNext.y %= nPatchHeight ;
			//
			S3DVector	vPos = pPatch->GetPoint( ptNext.x, ptNext.y ) ;
			float32_t	fpVertLen =
				(float32_t) (vPos - pPatch->GetPoint( ptLastLine.x, ptLastLine.y )).Absolute() ;
			fpSumVertLen += fpVertLen ;
			pfpVertLen[j] = fpVertLen ;
			//
			float32_t	fpHorzLen = (float32_t) (vPos - vLast).Absolute() ;
			fpSumHorzLen += fpHorzLen ;
			pfpHorzLen[j] = fpHorzLen ;
			vLast = vPos ;
			//
			ptLastLine += puvInfo.ptHorzDelta ;
			ptNext += puvInfo.ptHorzDelta ;
		}
		float32_t	fpAvgVertLen = fpSumVertLen / (float32_t) (puvInfo.nLineWidth + 1) ;
		float32_t	fpAvgHorzLen = fpSumHorzLen / (float32_t) puvInfo.nLineWidth ;
		//
		// 角度分配係数計算
		//
		float32_t	fpVertScale = 1.0f ;
		float32_t	fpHorzScale = 1.0f ;
		float32_t	radTotalRound = 2.0f * (float32_t) PI ;
		if ( !puvInfo.flagHorzLoop )
		{
			float32_t	fpRoundLen = 2.0f * (float32_t) PI
										* (fpAccVertLen + fpAvgVertLen) ;
			float32_t	radRoundScale = fpSumHorzLen / fpRoundLen ;
			if ( radRoundScale > 1.0f )
			{
				fpVertScale = 1.0f / radRoundScale ;
				radRoundScale = 1.0f ;
			}
			radTotalRound *= radRoundScale ;
		}
		fpHorzScale = radTotalRound / fpSumHorzLen ;
		fpAccVertLen += fpAvgVertLen ;
		//
		// 各頂点へUVマッピング
		//
		double	radNext = (2.0 * PI - radTotalRound) * 0.5 ;
		vLast = pPatch->GetPoint( ptVertex.x, ptVertex.y ) ;
		ptNext = ptVertex ;
		for ( size_t j = 0; j <= puvInfo.nLineWidth; j ++ )
		{
			radNext += pfpHorzLen[j] * fpHorzScale ;
			if ( ((size_t) ptNext.x >= (size_t) nPatchWidth)
				|| ((size_t) ptNext.y >= (size_t) nPatchHeight) )
			{
				ptNext += puvInfo.ptHorzDelta ;
				continue ;
			}
			ptLastLine = ptNext - puvInfo.ptVertDelta ;
			ptLastLine.x %= nPatchWidth ;
			ptLastLine.y %= nPatchHeight ;
			//
			S3DVector	vPos = pPatch->GetPoint( ptNext.x, ptNext.y ) ;
			S3DVector	vLast = pPatch->GetPoint( ptLastLine.x, ptLastLine.y ) ;
			S2DVector	vLastUV = pPatch->GetUV( ptLastLine.x, ptLastLine.y ) ;
			float32_t	fpLastUVLen = (float32_t) vLastUV.Absolute() ;
			float32_t	fpNextUVLen = fpLastUVLen + pfpVertLen[j] * fpVertScale ;
			//
			S2DVector	vNextUV( fpNextUVLen * -sin(radNext),
									fpNextUVLen * -cos(radNext) ) ;
			S2DVector	vDeltaUV = vNextUV - vLastUV ;
			float32_t	fpDeltaUVLen = (float32_t) vDeltaUV.Absolute() ;
			if ( fpDeltaUVLen > 0.0f )
			{
				vDeltaUV *= 1.0f / fpDeltaUVLen ;
				vDeltaUV *= sqrt( fpDeltaUVLen / (pfpVertLen[j] * fpVertScale) ) ;
				vNextUV = vLastUV + vDeltaUV ;
			}
			pPatch->SetUV( ptNext.x, ptNext.y, vNextUV ) ;
			//
			ptNext += puvInfo.ptHorzDelta ;
		}
	}
	aVertLen.FinishArray() ;
	aHorzLen.FinishArray() ;
	//
	// UV が指定範囲内に治まるようにスケーリング
	//
	S2DVector	vUVMin( 0, 0 ) ;
	S2DVector	vUVMax( 0, 0 ) ;
	GetUVMinMaxOfPatchOrderBlock( vUVMin, vUVMax, pPatch, puvInfo ) ;
	//
	float32_t	fpScaleUV = esl_fminf( xRangeWidth / (vUVMax.x - vUVMin.x),
										yRangeHeight / (vUVMax.y - vUVMin.y) ) ;
	fpScaleUV *= 0.95f ;
	//
	S2DVector	vOffsetUV ;
	vOffsetUV.x = xRangeLeft + xRangeWidth * 0.5f
						- (vUVMax.x + vUVMin.x) * 0.5f * fpScaleUV ;
	vOffsetUV.y = yRangeTop + yRangeHeight * 0.5f
						- (vUVMax.y + vUVMin.y) * 0.5f * fpScaleUV ;
	//
	ScaleUVMapOfPatchOrderBlock( pPatch, puvInfo, fpScaleUV, vOffsetUV ) ;
}

void S3DMeshEditorObject::DoUVMapDevelopmentOfPatchBlockOrthogonal
	( S3DMeshEditor::Patch * pPatch,
		const S3DMeshEditorObject::LineRange& lrXRange,
		const S3DMeshEditorObject::LineRange& lrYRange,
		float32_t xRangeLeft, float32_t yRangeTop,
		float32_t xRangeWidth, float32_t yRangeHeight,
		float32_t fpTotalWidth, float32_t fpTotalHeight,
		const S3DMeshEditorObject::UVMapDevParam& param )
{
	const int	nPatchWidth = (int) pPatch->GetWidth() ;
	const int	nPatchHeight = (int) pPatch->GetHeight() ;
	//
	// 面の向きの平均を求める
	//
	S3DVector	vAccNormal( 0, 0, 0 ) ;
	for ( size_t y = 0; y < lrYRange.nCount; y ++ )
	{
		for ( size_t x = 0; x < lrXRange.nCount; x ++ )
		{
			vAccNormal +=
				pPatch->GetFaceNormal
					( lrXRange.nIndex + x, lrYRange.nIndex + y ).Normalized() ;
		}
	}
	if ( vAccNormal.Absolute() < 1.0e-5 )
	{
		vAccNormal = S3DVector( 0, 0, -1 ) ;
	}
	S3DVector	vModAccNormal( 0, 0, 0 ) ;
	for ( size_t y = 0; y < lrYRange.nCount; y ++ )
	{
		for ( size_t x = 0; x < lrXRange.nCount; x ++ )
		{
			S3DVector	vFaceNormal =
				pPatch->GetFaceNormal( lrXRange.nIndex + x, lrYRange.nIndex + y ) ;
			if ( vFaceNormal.InnerProduct( vAccNormal ) >= 0.0f )
			{
				vModAccNormal += vFaceNormal.Normalized() ;
			}
			else
			{
				vModAccNormal -= vFaceNormal.Normalized() ;
			}
		}
	}
	//
	// 投影行列
	//
	S3DMatrix	matOrth( 1, 1, 1 ) ;
	matOrth.RevolveByAngleOn( - vModAccNormal ) ;
	//
	// UV計算
	//
	for ( size_t y = 0; y <= lrYRange.nCount; y ++ )
	{
		if ( lrYRange.nIndex + y >= (size_t) nPatchHeight )
		{
			break ;
		}
		for ( size_t x = 0; x <= lrXRange.nCount; x ++ )
		{
			if ( lrXRange.nIndex + x < (size_t) nPatchWidth )
			{
				S3DVector	uv =
					matOrth * pPatch->GetPoint
								( lrXRange.nIndex + x,
									lrYRange.nIndex + y ) ;
				pPatch->SetUV
					( lrXRange.nIndex + x,
						lrYRange.nIndex + y, S2DVector( uv.x, uv.y ) ) ;
			}
		}
	}
	//
	// UV が指定範囲内に治まるようにスケーリング
	//
	PolarUVMapInfo	puvInfo ;
	puvInfo.nLineWidth = lrXRange.nCount ;
	puvInfo.nColHeight = lrYRange.nCount ;
	puvInfo.ptPolar.x = (int32_t) lrXRange.nIndex ;
	puvInfo.ptPolar.y = (int32_t) lrYRange.nIndex ;
	puvInfo.ptHorzDelta = SGLPoint( 1, 0 ) ;
	puvInfo.ptVertDelta = SGLPoint( 0, 1 ) ;
	//
	S2DVector	vUVMin( 0, 0 ) ;
	S2DVector	vUVMax( 0, 0 ) ;
	GetUVMinMaxOfPatchOrderBlock( vUVMin, vUVMax, pPatch, puvInfo ) ;
	//
	S2DVector	vSrcCenter( (vUVMax.x + vUVMin.x) * 0.5f,
							(vUVMax.y + vUVMin.y) * 0.5f ) ;
	S2DVector	vDstCenter( xRangeLeft + xRangeWidth * 0.5f,
							yRangeTop + yRangeHeight * 0.5f ) ;
	//
	float32_t	fpScaleUV = esl_fminf( xRangeWidth / (vUVMax.x - vUVMin.x),
										yRangeHeight / (vUVMax.y - vUVMin.y) ) ;
	float32_t	fpScaleVU = esl_fminf( yRangeHeight / (vUVMax.x - vUVMin.x),
										xRangeWidth / (vUVMax.y - vUVMin.y) ) ;
	if ( fpScaleVU > fpScaleUV )
	{
		fpScaleVU *= 0.95f ;
		//
		SGLAffine	affine( 0.0f, - fpScaleVU, 0.0f,
							fpScaleVU, 0.0f, 0.0f ) ;
		S2DVector	vOffsetUV = vDstCenter - affine * vSrcCenter ;
		//
		AffineUVMapOfPatchOrderBlock( pPatch, puvInfo, affine ) ;
	}
	else
	{
		fpScaleUV *= 0.95f ;
		//
		S2DVector	vOffsetUV = vDstCenter - vSrcCenter * fpScaleUV ;
		//
		ScaleUVMapOfPatchOrderBlock( pPatch, puvInfo, fpScaleUV, vOffsetUV ) ;
	}
}

bool S3DMeshEditorObject::GetUVMinMaxOfPatchOrderBlock
	( S2DVector& vUVMin, S2DVector& vUVMax,
		S3DMeshEditor::Patch * pPatch,
		const S3DMeshEditorObject::PolarUVMapInfo& puvInfo ) const
{
	SGLPoint		ptVertex = puvInfo.ptPolar ;
	const size_t	nPatchWidth = pPatch->GetWidth() ;
	const size_t	nPatchHeight = pPatch->GetHeight() ;
	bool			flagInit = true ;
	for ( size_t i = 0; i <= puvInfo.nColHeight; i ++ )
	{
		SGLPoint	ptNext = ptVertex ;
		for ( size_t j = 0; j <= puvInfo.nLineWidth; j ++ )
		{
			if ( ((size_t) ptNext.x < (size_t) nPatchWidth)
				&& ((size_t) ptNext.y < (size_t) nPatchHeight) )
			{
				S2DVector	uv = pPatch->GetUV( ptNext.x, ptNext.y ) ;
				if ( flagInit )
				{
					vUVMin = uv ;
					vUVMax = uv ;
					flagInit = false ;
				}
				else
				{
					vUVMin.x = esl_fminf( vUVMin.x, uv.x ) ;
					vUVMin.y = esl_fminf( vUVMin.y, uv.y ) ;
					vUVMax.x = esl_fmaxf( vUVMax.x, uv.x ) ;
					vUVMax.y = esl_fmaxf( vUVMax.y, uv.y ) ;
				}
			}
			ptNext += puvInfo.ptHorzDelta ;
		}
		ptVertex += puvInfo.ptVertDelta ;
	}
	return	!flagInit ;
}

void S3DMeshEditorObject::ScaleUVMapOfPatchOrderBlock
	( S3DMeshEditor::Patch * pPatch,
		const S3DMeshEditorObject::PolarUVMapInfo& puvInfo,
		const float32_t fpScaleUV, const S2DVector& vOffsetUV )
{
	SGLAffine	affine( fpScaleUV, 0.0f, vOffsetUV.x,
						0.0f, fpScaleUV, vOffsetUV.y ) ;
	AffineUVMapOfPatchOrderBlock( pPatch, puvInfo, affine ) ;
}

void S3DMeshEditorObject::AffineUVMapOfPatchOrderBlock
	( S3DMeshEditor::Patch * pPatch,
		const S3DMeshEditorObject::PolarUVMapInfo& puvInfo,
		const SGLAffine& affine )
{
	SGLPoint		ptVertex = puvInfo.ptPolar ;
	const size_t	nPatchWidth = pPatch->GetWidth() ;
	const size_t	nPatchHeight = pPatch->GetHeight() ;
	for ( size_t i = 0; i <= puvInfo.nColHeight; i ++ )
	{
		SGLPoint	ptNext = ptVertex ;
		for ( size_t j = 0; j <= puvInfo.nLineWidth; j ++ )
		{
			if ( ((size_t) ptNext.x < (size_t) nPatchWidth)
				&& ((size_t) ptNext.y < (size_t) nPatchHeight) )
			{
				S2DVector	uv = pPatch->GetUV( ptNext.x, ptNext.y ) ;
				pPatch->SetUV( ptNext.x, ptNext.y, affine * uv ) ;
			}
			ptNext += puvInfo.ptHorzDelta ;
		}
		ptVertex += puvInfo.ptVertDelta ;
	}
}

void S3DMeshEditorObject::DoScaleUVMapOfPatch
	( S3DMeshEditor::Patch * pPatch, S3DMeshEditorObject::UVMapDevResult& uvmapdRes,
		const S2DVector& vScale, const S2DVector& vOffset,
		const S3DMeshEditorObject::UVMapDevParam& param, S3DMeshEditorInterface * pEdit )
{
	S3DMeshEditor::SelectPointSet	spsPatchPoints ;
	if ( param.onlySelected )
	{
		for ( size_t i = 0; i < m_spsUVPoints.GetLength(); i ++ )
		{
			S3DMeshEditor::SelectPoint	sp = m_spsUVPoints.At(i) ;
			if ( sp.pPatch == pPatch )
			{
				spsPatchPoints.QuickAdd( sp ) ;
			}
		}
	}
	else
	{
		for ( size_t i = 0; i < pPatch->GetTotalVertexCount(); i ++ )
		{
			spsPatchPoints.QuickAdd( S3DMeshEditor::SelectPoint( pPatch, i ) ) ;
		}
	}
	spsPatchPoints.SortArray() ;
	if ( spsPatchPoints.GetLength() == 0 )
	{
		return ;
	}
	for ( size_t i = 0; i < spsPatchPoints.GetLength(); i ++ )
	{
		S3DMeshEditor::SelectPoint	sp = spsPatchPoints.At(i) ;
		S2DVector	uv = pPatch->GetUVAt( sp.iVertex ) ;
		uv.x = uv.x * vScale.x + vOffset.x ;
		uv.y = uv.y * vScale.y + vOffset.y ;
		pPatch->SetUVAt( sp.iVertex, uv ) ;
	}
	uvmapdRes.vMinUV.x = uvmapdRes.vMinUV.x * vScale.x + vOffset.x ;
	uvmapdRes.vMinUV.y = uvmapdRes.vMinUV.y * vScale.y + vOffset.y ;
	uvmapdRes.vMaxUV.x = uvmapdRes.vMaxUV.x * vScale.x + vOffset.x ;
	uvmapdRes.vMaxUV.y = uvmapdRes.vMaxUV.y * vScale.y + vOffset.y ;
	//
	pEdit->NotifyUpdateVertex( spsPatchPoints ) ;
}

// S3DMeshEditor 取得
//////////////////////////////////////////////////////////////////////////////
const S3DMeshEditor& S3DMeshEditorObject::GetMeshEditor( void ) const
{
	return	*this ;
}

S3DMeshEditor& S3DMeshEditorObject::MeshEditor( void )
{
	return	*this ;
}

// 表示用メッシュの更新
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::UpdateViewMesh( void )
{
}

// 表示用マテリアルの取得
//////////////////////////////////////////////////////////////////////////////
S3DMaterial * S3DMeshEditorObject::GetMeshMaterial( size_t iMaterial ) const
{
	if ( m_pExtraInfo != nullptr )
	{
		return	m_pExtraInfo->GetMeshMaterial( iMaterial ) ;
	}
	return	S3DMeshEditorInterface::GetMeshMaterial( iMaterial ) ;
}

// 空間
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::GetMeshItemMatrix( S3DDMatrix& mat, S3DDVector& pos ) const
{
	if ( m_pExtraInfo != nullptr )
	{
		return	m_pExtraInfo->GetMeshItemMatrix( mat, pos ) ;
	}
	S3DMeshEditorInterface::GetMeshItemMatrix( mat, pos ) ;
}

// パッチ総数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshEditorObject::GetPatchCount( void ) const
{
	return	S3DMeshEditor::GetPatchCount() ;
}

// パッチ取得
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditor::Patch * S3DMeshEditorObject::GetPatchAt( size_t i ) const
{
	return	S3DMeshEditor::GetPatchAt( i ) ;
}

// パッチ生成
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditor::Patch * S3DMeshEditorObject::NewPatch( void )
{
	return	S3DMeshEditor::NewPatch() ;
}

// パッチ追加
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshEditorObject::AddPatch( S3DMeshEditor::Patch * pPatch )
{
	return	S3DMeshEditor::AddPatch( pPatch ) ;
}

size_t S3DMeshEditorObject::InsertPatchAt( size_t i, S3DMeshEditor::Patch * pPatch )
{
	return	S3DMeshEditor::InsertPatchAt( i, pPatch ) ;
}

// パッチ削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::RemovePatchAt( size_t i )
{
	if ( m_iCurPatch > (ssize_t) i )
	{
		m_iCurPatch -- ;
	}
	else if ( m_iCurPatch == (ssize_t) i )
	{
		m_iCurPatch = -1 ;
	}
	S3DMeshEditor::RemovePatchAt( i ) ;
}

// パッチ検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DMeshEditorObject::FindPatch( S3DMeshEditor::Patch * pPatch ) const
{
	return	S3DMeshEditor::FindPatch( pPatch ) ;
}

// パッチ順序入れ替え
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::SwapPatchOrder( size_t iPatch0, size_t iPatch1 )
{
	if ( m_iCurPatch == (ssize_t) iPatch0 )
	{
		m_iCurPatch = (ssize_t) iPatch1 ;
	}
	else if ( m_iCurPatch == (ssize_t) iPatch1 )
	{
		m_iCurPatch = (ssize_t) iPatch0 ;
	}
	S3DMeshEditor::SwapPatchOrder( iPatch0, iPatch1 ) ;
}

// ウェイトマップレイヤー追加
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::InsertWeightLayerAt( size_t iLayer, const wchar_t * pwszID )
{
	if ( m_iCurWeight >= (ssize_t) iLayer )
	{
		m_iCurWeight -- ;
	}
	S3DMeshEditor::InsertWeightLayerAt( iLayer, pwszID ) ;
}

// ウェイトマップレイヤー削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::RemoveWeightLayer( size_t iLayer, size_t nCount )
{
	if ( m_iCurWeight > (ssize_t) iLayer )
	{
		m_iCurWeight -- ;
	}
	else if ( m_iCurWeight == (ssize_t) iLayer )
	{
		m_iCurWeight = -1 ;
	}
	S3DMeshEditor::RemoveWeightLayer( iLayer, nCount ) ;
}

// ウェイトマップレイヤー入れ替え
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::SwapWeightLayer( size_t iLayer0, size_t iLayer1 )
{
	if ( m_iCurWeight == (ssize_t) iLayer0 )
	{
		m_iCurWeight = (ssize_t) iLayer1 ;
	}
	else if ( m_iCurWeight == (ssize_t) iLayer1 )
	{
		m_iCurWeight = (ssize_t) iLayer0 ;
	}
	S3DMeshEditor::SwapWeightLayer( iLayer0, iLayer1 ) ;
}

// 頂点縮退処理（頂点座標操作は無し）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::ShrinkPoints
	( const S3DMeshEditor::PatchPointSet& pps, uint32_t nFlags )
{
	S3DMeshEditor::ShrinkPoints( pps, nFlags ) ;
}

// 頂点縮退解除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorObject::UntiShrinkPoints( const S3DMeshEditor::PatchPoint& pp )
{
	S3DMeshEditor::UntiShrinkPoints( pp ) ;
}




//////////////////////////////////////////////////////////////////////////////
// メッシュ編集アイテム・メッシュ・コントローラー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DMeshEditorSerializer::MeshController, Controller )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditorSerializer::MeshController::MeshController( const wchar_t * pwszClassID )
	: Controller( pwszClassID )
{
}



//////////////////////////////////////////////////////////////////////////////
// ボーン物理演算パラメータ
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DBonePhysMaterialSerializer::m_paramEntries
		[S3DBonePhysMaterialSerializer::paramPhysMaterialCount] =
{
	{ L"attenuation",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrUIScalarSlider,
		L"速度減速率", L"1秒当たりの減速率", 0.0, 1.0 },
	{ L"shrinkable",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrUIScalarSlider,
		L"縮み弾性", L"フレーム当たり縮み反発力比", 0.0, 1.0 },
	{ L"elasticity",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrUIScalarSlider,
		L"伸び弾性", L"フレーム当たり伸び反発力比", 0.0, 1.0 },
	{ L"min_stretch",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrUIScalarSlider,
		L"限界縮み率", nullptr, 0.0, 1.0 },
	{ L"max_stretch",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrUIScalarSlider,
		L"限界伸び率", nullptr, 1.0, 2.0 },
	{ L"hardness",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrUIScalarSlider,
		L"曲げ弾性", L"フレーム当たりの曲げ方向反発力比", 0.0, 1.0 },
	{ L"effect",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrUIScalarSlider,
		L"加速度効果", L"外要因加速度の影響比率", 0.0, 1.0 },
	{ L"limited_angle",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrUIScalarSlider,
		L"制限角", L"曲げ制限角[deg]\n"
		L"実際に曲がることのできる角度範囲は θ-180 ～ 180-θ になる。\n"
		L"0 の時には制限なし、180 の時には一切曲がらない。", 0.0, 180.0 },
	{ L"collision_radius",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1,
		L"当たり判定半径", nullptr },
	{ L"frictional_resistance",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrUIScalarSlider,
		L"摩擦係数", L"当たり判定衝突時のフレーム当たりの速度係数（加速率）", 0.0, 1.0 },
	{ L"phys_ex_flags1",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrFlagSetInteger,
		L"当たり判定マスク", L"当たり判定から除外するクラスのビットマスク\n"
		L"０の時には全てに当たり判定有効" },
} ;

const S3DSceneComposer::ParamSetClass
	S3DBonePhysMaterialSerializer::m_pscClass =
{
	&ItemBasicSerializer::m_pscClass,
	S3DBonePhysMaterialSerializer::paramPhysMaterialCount,
	&S3DBonePhysMaterialSerializer::m_paramEntries[0]
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DBonePhysMaterialSerializer, ItemBasicSerializer )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DBonePhysMaterialSerializer, bone_phys_material )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DBonePhysMaterialSerializer::S3DBonePhysMaterialSerializer( void )
	: ItemBasicSerializer( m_ItemClassDescriptor.pwszClassID,
								&S3DBonePhysMaterialSerializer::m_pscClass )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DBonePhysMaterialSerializer::~S3DBonePhysMaterialSerializer( void )
{
}

// 物理演算パラメータ
//////////////////////////////////////////////////////////////////////////////
S3DModelBoneSpace::PhysMaterial& S3DBonePhysMaterialSerializer::PhysMaterial( void )
{
	return	m_physMaterial ;
}

const S3DModelBoneSpace::PhysMaterial& S3DBonePhysMaterialSerializer::GetPhysMaterial( void ) const
{
	return	m_physMaterial ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
double S3DBonePhysMaterialSerializer::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramAttenuation:
		return	m_physMaterial.fpAttenuation ;
	case	paramShrinkable:
		return	m_physMaterial.fpShrinkable ;
	case	paramElasticity:
		return	m_physMaterial.fpElasticity ;
	case	paramMinStretch:
		return	m_physMaterial.fpMinStretch ;
	case	paramMaxStretch:
		return	m_physMaterial.fpMaxStretch ;
	case	paramHardness:
		return	m_physMaterial.fpHardness ;
	case	paramEffect:
		return	m_physMaterial.fpEffect ;
	case	paramLimitedAngle:
		return	m_physMaterial.fpLimitedAngle ;
	case	paramCollisionRadius:
		return	m_physMaterial.fpCollisionRadius ;
	case	paramFrictionalResistance:
		return	m_physMaterial.fpFrictionalResistance ;
	}
	return	ItemBasicSerializer::GetScalarParameter( i ) ;
}

int32_t S3DBonePhysMaterialSerializer::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramPhysExFlags1:
		return	(int32_t) m_physMaterial.nPhysExFlags1
							& S3DModelBoneSpace::flagPhysExColliderAll ;
	}
	return	ItemBasicSerializer::GetIntegerParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DBonePhysMaterialSerializer::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramAttenuation:
		m_physMaterial.fpAttenuation = s ;
		return ;
	case	paramShrinkable:
		m_physMaterial.fpShrinkable = s ;
		return ;
	case	paramElasticity:
		m_physMaterial.fpElasticity = s ;
		return ;
	case	paramMinStretch:
		m_physMaterial.fpMinStretch = s ;
		return ;
	case	paramMaxStretch:
		m_physMaterial.fpMaxStretch = s ;
		return ;
	case	paramHardness:
		m_physMaterial.fpHardness = s ;
		return ;
	case	paramEffect:
		m_physMaterial.fpEffect = s ;
		return ;
	case	paramLimitedAngle:
		m_physMaterial.fpLimitedAngle = s ;
		return ;
	case	paramCollisionRadius:
		m_physMaterial.fpCollisionRadius = s ;
		return ;
	case	paramFrictionalResistance:
		m_physMaterial.fpFrictionalResistance = s ;
		return ;
	}
	ItemBasicSerializer::SetScalarParameter( i, s ) ;
}

void S3DBonePhysMaterialSerializer::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramPhysExFlags1:
		m_physMaterial.nPhysExFlags1 =
				(uint32_t) n & S3DModelBoneSpace::flagPhysExColliderAll ;
		if ( n != 0 )
		{
			m_physMaterial.nPhysExFlags1 |= S3DModelBoneSpace::flagPhysExColliderUseMask ;
		}
		return ;
	}
	ItemBasicSerializer::SetIntegerParameter( i, n ) ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DBonePhysMaterialSerializer::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramZoom:
	case	paramTransparency:
	case	paramColorMul:
	case	paramColorAdd:
	case	paramVisible:
	case	paramForceToon:
	case	paramForceBorder:
	case	paramFreeToon:
	case	paramFreeBorder:
	case	paramUseCollision:
	case	paramGlobalSpace:
	case	paramCameraShift:
	case	paramCameraSpace:
	case	paramHideNear:
	case	paramHideFar:
	case	paramItemClass:
	case	paramItemPriority:
		return	false ;
	}
	return	ItemBasicSerializer::IsParameterValidation( i ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DBonePhysMaterialSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	1:
		return	L"物理演算パラメータ" ;
	}
	return	ItemBasicSerializer::GetParameterCategoryName( iCategory ) ;
}



//////////////////////////////////////////////////////////////////////////////
// ボーンアイテム
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DBoneSerializer::m_paramEntries[S3DBoneSerializer::paramBoneCount] =
{
	{ L"bone_handle",
		S3DSceneComposer::typePosition,
		S3DSceneComposer::attrConstant2,
		L"ボーンハンドル", nullptr },
	{ L"enable_bend_dir",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant2,
		L"IK曲げ方向有効", nullptr },
	{ L"use_parent_axis",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant2,
		L"IK親ボーン回転軸", nullptr },
	{ L"ik_terminate_flag",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant2,
		L"IK終端フラグ", L"IKで曲げる際にこのボーンより親のボーンへは影響しない" },
	{ L"bend_dir",
		S3DSceneComposer::typeDirection,
		S3DSceneComposer::attrConstant2,
		L"IK曲げ方向",
		L"IKで曲げる際にボーンハンドルとの内積がプラス方向になるように曲げる。\n"
		L"「IK親ボーン回転軸」が有効の場合、親ボーンの回転軸としても使用する。"},
	{ L"bend_max_angle",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory2
		| S3DSceneComposer::attrUIScalarSlider,
		L"IK曲げ最大角 [deg]", nullptr, 1.0, 180.0 },
	{ L"bend_weight",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory2
		| S3DSceneComposer::attrUIScalarSlider,
		L"IK曲げ重み", L"IKで曲げる際の曲がりやすさ（0.0の時IKで曲げない）", 0.0, 1.0 },
	{ L"org_matrix4",
		S3DSceneComposer::typeMatrix4,
		S3DSceneComposer::attrConstant2,
		L"元ボーン行列", nullptr },
	{ L"enable_physics",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant2
		| S3DSceneComposer::attrDynamicValidation,
		L"物理演算有効", nullptr },
	{ L"disable_bone_col",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant2,
		L"当たり判定無効", nullptr },
	{ L"pose_weight",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory2
		| S3DSceneComposer::attrUIScalarSlider,
		L"ポーズ適用度",
		L"物理演算などの他処理よりタイムライン上のポーズを優先する適用度", 0.0, 1.0 },
	{ L"phys_material_ref",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant2
		| S3DSceneComposer::attrStringEnumeration
		| S3DSceneComposer::attrDynamicValidation,
		L"物理演算パラメータ", nullptr },
	{ L"attenuation",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant3
		| S3DSceneComposer::attrUIScalarSlider,
		L"速度減速率", L"1秒当たりの減速率", 0.0, 1.0 },
	{ L"shrinkable",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant3
		| S3DSceneComposer::attrUIScalarSlider,
		L"縮み弾性", L"フレーム当たり縮み反発力比", 0.0, 1.0 },
	{ L"elasticity",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant3
		| S3DSceneComposer::attrUIScalarSlider,
		L"伸び弾性", L"フレーム当たり伸び反発力比", 0.0, 1.0 },
	{ L"min_stretch",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant3
		| S3DSceneComposer::attrUIScalarSlider,
		L"限界縮み率", nullptr, 0.0, 1.0 },
	{ L"max_stretch",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant3
		| S3DSceneComposer::attrUIScalarSlider,
		L"限界伸び率", nullptr, 1.0, 2.0 },
	{ L"hardness",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant3
		| S3DSceneComposer::attrUIScalarSlider,
		L"曲げ弾性", L"フレーム当たりの曲げ方向反発力比", 0.0, 1.0 },
	{ L"effect",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant3
		| S3DSceneComposer::attrUIScalarSlider,
		L"加速度効果", L"外要因加速度の影響比率", 0.0, 1.0 },
	{ L"limited_angle",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant3
		| S3DSceneComposer::attrUIScalarSlider,
		L"制限角", L"曲げ制限角[deg]\n"
		L"実際に曲がることのできる角度範囲は θ-180 ～ 180-θ になる。\n"
		L"0 の時には制限なし、180 の時には一切曲がらない。", 0.0, 180.0 },
	{ L"collision_radius",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant3,
		L"当たり判定半径", nullptr },
	{ L"frictional_resistance",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant3
		| S3DSceneComposer::attrUIScalarSlider,
		L"摩擦係数", L"当たり判定衝突時のフレーム当たりの速度係数（加速率）", 0.0, 1.0 },
	{ L"phys_ex_flags1",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant3
		| S3DSceneComposer::attrFlagSetInteger,
		L"当たり判定マスク", L"当たり判定の対象から除外するクラスのビットマスク\n"
		L"０の時には全てに当たり判定有効" },
	{ L"param_ref_pose_lib",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant4
		| S3DSceneComposer::attrStringEnumeration,
		L"参照ポーズライブラリ", nullptr },
	{ L"param_cmd_pose_id",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant4
		| S3DSceneComposer::attrStringEnumeration,
		L"ポーズ名", nullptr },
	{ L"cmd_reset_matrix",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant4
		| S3DSceneComposer::attrEditorCommand,
		L"ボーン行列リセット", nullptr },
	{ L"cmd_reverse_bone",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant4
		| S3DSceneComposer::attrDynamicValidation
		| S3DSceneComposer::attrEditorCommand,
		L"ボーン左右反転", nullptr },
	{ L"cmd_apply_ref_phys_params",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant4
		| S3DSceneComposer::attrDynamicValidation
		| S3DSceneComposer::attrEditorCommand,
		L"参照物理演算マテリアルをボーンに反映", nullptr },
	{ L"cmd_copy_phys_params",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant4
		| S3DSceneComposer::attrDynamicValidation
		| S3DSceneComposer::attrEditorCommand,
		L"子ボーンへ物理演算パラメータを複製", nullptr },
	{ L"cmd_register_pose",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant4
		| S3DSceneComposer::attrEditorCommand,
		L"ポーズ登録", nullptr },
	{ L"cmd_restore_pose",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant4
		| S3DSceneComposer::attrEditorCommand,
		L"ポーズ展開", nullptr },
} ;

const S3DSceneComposer::ParamSetClass
	S3DBoneSerializer::m_pscClass =
{
	&SpaceSerializer::m_pscClass,
	S3DBoneSerializer::paramBoneCount,
	&S3DBoneSerializer::m_paramEntries[0]
} ;

const wchar_t *	S3DBoneSerializer::m_pwszPresetName
					[S3DModelBoneSpace::PhysMaterial::presetCount] =
{
	L"[@preset:板金]",
	L"[@preset:プラスチック板]",
	L"[@preset:ゴム膜（硬質）]",
	L"[@preset:ゴム膜（軟質）]",
	L"[@preset:布（厚手）]",
	L"[@preset:布（薄手）]",
	L"[@preset:フィルム]",
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DBoneSerializer, SpaceSerializer )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DBoneSerializer, bone_space )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DBoneSerializer::S3DBoneSerializer( void )
	: SpaceSerializer( m_ItemClassDescriptor.pwszClassID,
								&S3DBoneSerializer::m_pscClass ),
		m_vBoneHandle( 0, 0, 1 ), m_vBendDir( 1, 0, 0 ),
		m_mat4OrgMatrix( 1, 1, 1, 1 ),
		m_flagBendDir( false ), m_flagParentAxis( false ),
		m_flagIKTerminate( false ),
		m_fpBendMaxAngle( 180.0 ), m_fpBendWeight( 1.0 ),
		m_flagBonePhysics( false ), m_flagNoHitCollision( false ),
		m_fpPoseWeight( 1.0 ), m_qPhysRotation( 1, 0, 0, 0 ),
		m_pPhysMaterial( nullptr ), m_flagPhysCurrent( false )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DBoneSerializer::~S3DBoneSerializer( void )
{
}

// ボーンハンドル
//////////////////////////////////////////////////////////////////////////////
const S3DDVector& S3DBoneSerializer::GetBoneHandle( void ) const
{
	return	m_vBoneHandle ;
}

void S3DBoneSerializer::SetBoneHandle( const S3DDVector& vHandle )
{
	m_vBoneHandle = vHandle ;
}

// 曲げ方向ベクトル
//////////////////////////////////////////////////////////////////////////////
bool S3DBoneSerializer::IsEnabledBendDir( void ) const
{
	return	m_flagBendDir ;
}

void S3DBoneSerializer::EnableBendDir( bool flagBendDir )
{
	m_flagBendDir = flagBendDir ;
}

const S3DDVector& S3DBoneSerializer::GetBendDir( void ) const
{
	return	m_vBendDir ;
}

void S3DBoneSerializer::SetBendDir( const S3DDVector& vDir )
{
	m_vBendDir = vDir ;
}

// IK親ボーン回転軸ベクトル・フラグ
//////////////////////////////////////////////////////////////////////////////
bool S3DBoneSerializer::GetBendParentAxisFlag( void ) const
{
	return	m_flagParentAxis ;
}

void S3DBoneSerializer::SetBendParentAxisFlag( bool flagParentAxis )
{
	m_flagParentAxis = flagParentAxis ;
}

// IK終端フラグ
//////////////////////////////////////////////////////////////////////////////
bool S3DBoneSerializer::GetIKTerminateFlag( void ) const
{
	return	m_flagIKTerminate ;
}

void S3DBoneSerializer::SetIKTerminateFlag( bool flagIKTerminate )
{
	m_flagIKTerminate = flagIKTerminate ;
}

// IK曲げ最大角 [deg]
//////////////////////////////////////////////////////////////////////////////
double S3DBoneSerializer::GetBendMaxAngle( void ) const
{
	return	m_fpBendMaxAngle ;
}

void S3DBoneSerializer::SetBendMaxAngle( double degAngle )
{
	m_fpBendMaxAngle = degAngle ;
}

// IK曲げ重み
//////////////////////////////////////////////////////////////////////////////
double S3DBoneSerializer::GetBendWeight( void ) const
{
	return	m_fpBendWeight ;
}

void S3DBoneSerializer::SetBendWeight( double fpWeight )
{
	m_fpBendWeight = fpWeight ;
}

// オリジナル行列
//////////////////////////////////////////////////////////////////////////////
const S4DDMatrix& S3DBoneSerializer::GetOrgMatrix( void ) const
{
	return	m_mat4OrgMatrix ;
}

void S3DBoneSerializer::SetOrgMatrix( const S4DDMatrix& mat4Org )
{
	m_mat4OrgMatrix = mat4Org ;
}

// 物理演算有効
//////////////////////////////////////////////////////////////////////////////
bool S3DBoneSerializer::IsEnabledPhysics( void ) const
{
	return	m_flagBonePhysics ;
}

void S3DBoneSerializer::EnablePhysics( bool flagPhys )
{
	if ( !m_flagBonePhysics )
	{
		m_flagPhysCurrent = false ;
	}
	m_flagBonePhysics = flagPhys ;
}

// 物理演算当たり判定無効
//////////////////////////////////////////////////////////////////////////////
bool S3DBoneSerializer::IsDisabledCollision( void ) const
{
	return	m_flagNoHitCollision ;
}

void S3DBoneSerializer::DisableCollision( bool flagNoHit )
{
	m_flagNoHitCollision = flagNoHit ;
}

// ポーズ適用度
//////////////////////////////////////////////////////////////////////////////
double S3DBoneSerializer::GetPoseWeight( void ) const
{
	return	m_fpPoseWeight ;
}

void S3DBoneSerializer::SetPoseWeight( double fpWeight )
{
	m_fpPoseWeight = fpWeight ;
}

// 物理演算パラメータ
//////////////////////////////////////////////////////////////////////////////
S3DModelBoneSpace::PhysMaterial& S3DBoneSerializer::PhysMaterial( void )
{
	return	m_physMaterial ;
}

const S3DModelBoneSpace::PhysMaterial& S3DBoneSerializer::GetPhysMaterial( void ) const
{
	return	m_physMaterial ;
}

// 物理演算パラメータパレット
//////////////////////////////////////////////////////////////////////////////
const SSystem::SString& S3DBoneSerializer::GetPhysMaterialRef( void ) const
{
	return	m_strPhysMaterialRef ;
}

void S3DBoneSerializer::SetPhysMaterialRef( const wchar_t * pwszMaterial )
{
	m_strPhysMaterialRef = pwszMaterial ;
	UpdatePhysMaterialRef() ;
}

void S3DBoneSerializer::UpdatePhysMaterialRef( void )
{
	m_pPhysMaterial = nullptr ;
	if ( m_strPhysMaterialRef.IsEmpty() )
	{
		return ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		m_pPhysMaterial =
			ESLTypeCast<S3DBonePhysMaterialSerializer>
				( pComp->GetSceneItemAs( m_strPhysMaterialRef ) ) ;
	}
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DBoneSerializer::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramBoneHandle:
		return	m_vBoneHandle ;
	case	paramBendDir:
		return	m_vBendDir ;
	}
	return	SpaceSerializer::GetVectorParameter( i ) ;
}

double S3DBoneSerializer::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramBendMaxAngle:
		return	m_fpBendMaxAngle ;
	case	paramBendWeight:
		return	m_fpBendWeight ;
	case	paramPoseWeight:
		return	m_fpPoseWeight ;
	case	paramAttenuation:
		return	m_physMaterial.fpAttenuation ;
	case	paramShrinkable:
		return	m_physMaterial.fpShrinkable ;
	case	paramElasticity:
		return	m_physMaterial.fpElasticity ;
	case	paramMinStretch:
		return	m_physMaterial.fpMinStretch ;
	case	paramMaxStretch:
		return	m_physMaterial.fpMaxStretch ;
	case	paramHardness:
		return	m_physMaterial.fpHardness ;
	case	paramEffect:
		return	m_physMaterial.fpEffect ;
	case	paramLimitedAngle:
		return	m_physMaterial.fpLimitedAngle ;
	case	paramCollisionRadius:
		return	m_physMaterial.fpCollisionRadius ;
	case	paramFrictionalResistance:
		return	m_physMaterial.fpFrictionalResistance ;
	}
	return	SpaceSerializer::GetScalarParameter( i ) ;
}

int32_t S3DBoneSerializer::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramPhysExFlags1:
		return	(int32_t) m_physMaterial.nPhysExFlags1
							& S3DModelBoneSpace::flagPhysExColliderAll ;
	}
	return	SpaceSerializer::GetIntegerParameter( i ) ;
}

bool S3DBoneSerializer::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramEnableBendDir:
		return	m_flagBendDir ;
	case	paramUseParentAxis:
		return	m_flagParentAxis ;
	case	paramIKTerminate:
		return	m_flagIKTerminate ;
	case	paramEnablePhysics:
		return	m_flagBonePhysics ;
	case	paramDisableCollision:
		return	m_flagNoHitCollision ;
	}
	return	SpaceSerializer::GetBooleanParameter( i ) ;
}

const wchar_t * S3DBoneSerializer::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramPhysMaterialRef:
		return	m_strPhysMaterialRef ;
	case	paramCmdParamRefPoseLib:
		return	m_strRefPoseLib ;
	case	paramCmdParamPoseID:
		return	m_strRefPoseID ;
	}
	return	SpaceSerializer::GetCommandParameter( i ) ;
}

size_t S3DBoneSerializer::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t i ) const
{
	switch ( i )
	{
	case	paramOrgMatrix:
		if ( pDst == nullptr )
		{
			return	sizeof(S4DDMatrix) ;
		}
		if ( nBufBytes == sizeof(S4DDMatrix) )
		{
			*((S4DDMatrix*)pDst) = m_mat4OrgMatrix ;
			return	sizeof(S4DDMatrix) ;
		}
		return	0 ;
	}
	return	SpaceSerializer::GetBinaryParameter( pDst, nBufBytes, i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DBoneSerializer::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramBoneHandle:
		m_vBoneHandle = vec ;
		return ;
	case	paramBendDir:
		m_vBendDir = vec ;
		return ;
	}
	SpaceSerializer::SetVectorParameter( i, vec ) ;
}

void S3DBoneSerializer::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramBendMaxAngle:
		m_fpBendMaxAngle = s ;
		return ;
	case	paramBendWeight:
		m_fpBendWeight = s ;
		return ;
	case	paramPoseWeight:
		m_fpPoseWeight = s ;
		return ;
	case	paramAttenuation:
		m_physMaterial.fpAttenuation = s ;
		if ( m_pPhysMaterial != nullptr )
			m_pPhysMaterial->PhysMaterial().fpAttenuation = s ;
		return ;
	case	paramShrinkable:
		m_physMaterial.fpShrinkable = s ;
		if ( m_pPhysMaterial != nullptr )
			m_pPhysMaterial->PhysMaterial().fpShrinkable = s ;
		return ;
	case	paramElasticity:
		m_physMaterial.fpElasticity = s ;
		if ( m_pPhysMaterial != nullptr )
			m_pPhysMaterial->PhysMaterial().fpElasticity = s ;
		return ;
	case	paramMinStretch:
		m_physMaterial.fpMinStretch = s ;
		if ( m_pPhysMaterial != nullptr )
			m_pPhysMaterial->PhysMaterial().fpMinStretch = s ;
		return ;
	case	paramMaxStretch:
		m_physMaterial.fpMaxStretch = s ;
		if ( m_pPhysMaterial != nullptr )
			m_pPhysMaterial->PhysMaterial().fpMaxStretch = s ;
		return ;
	case	paramHardness:
		m_physMaterial.fpHardness = s ;
		if ( m_pPhysMaterial != nullptr )
			m_pPhysMaterial->PhysMaterial().fpHardness = s ;
		return ;
	case	paramEffect:
		m_physMaterial.fpEffect = s ;
		if ( m_pPhysMaterial != nullptr )
			m_pPhysMaterial->PhysMaterial().fpEffect = s ;
		return ;
	case	paramLimitedAngle:
		m_physMaterial.fpLimitedAngle = s ;
		if ( m_pPhysMaterial != nullptr )
			m_pPhysMaterial->PhysMaterial().fpLimitedAngle = s ;
		return ;
	case	paramCollisionRadius:
		m_physMaterial.fpCollisionRadius = s ;
		if ( m_pPhysMaterial != nullptr )
			m_pPhysMaterial->PhysMaterial().fpCollisionRadius = s ;
		return ;
	case	paramFrictionalResistance:
		m_physMaterial.fpFrictionalResistance = s ;
		if ( m_pPhysMaterial != nullptr )
			m_pPhysMaterial->PhysMaterial().fpFrictionalResistance = s ;
		return ;
	}
	SpaceSerializer::SetScalarParameter( i, s ) ;
}

void S3DBoneSerializer::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramPhysExFlags1:
		m_physMaterial.nPhysExFlags1 =
			(uint32_t) n & S3DModelBoneSpace::flagPhysExColliderAll ;
		if ( n != 0 )
		{
			m_physMaterial.nPhysExFlags1 |= S3DModelBoneSpace::flagPhysExColliderUseMask ;
		}
		if ( m_pPhysMaterial != nullptr )
		{
			m_pPhysMaterial->PhysMaterial().nPhysExFlags1 = m_physMaterial.nPhysExFlags1 ;
		}
		return ;
	}
	SpaceSerializer::SetIntegerParameter( i, n ) ;
}

void S3DBoneSerializer::SetBooleanParameter( size_t i, bool b )
{
	S3DCompositionEditorInterface *	pEditor = nullptr ;
	S3DSceneComposer::Composition *	pComp = nullptr ;
	S3DSceneComposer *				pComposer = nullptr ;
	switch ( i )
	{
	case	paramEnableBendDir:
		m_flagBendDir = b ;
		return ;

	case	paramUseParentAxis:
		m_flagParentAxis = b ;
		return ;

	case	paramIKTerminate:
		m_flagIKTerminate = b ;
		return ;

	case	paramEnablePhysics:
		EnablePhysics( b ) ;
		return ;

	case	paramDisableCollision:
		m_flagNoHitCollision = b ;
		return ;

	case	paramCmdResetMatrix:
		pEditor = GetEditor() ;
		if ( pEditor != nullptr )
		{
			pEditor->AddEditUndo( GetComposition(), this, L"ボーン行列リセット" ) ;
		}
		CmdResetBoneMatrix( pEditor ) ;
		return ;

	case	paramCmdReverseBone:
		pEditor = GetEditor() ;
		if ( pEditor != nullptr )
		{
			pEditor->AddEditUndo( GetComposition(), this, L"ボーン左右反転" ) ;
		}
		CmdReverseBone( pEditor ) ;
		return ;

	case	paramCmdApplyRefPhysParams:
		pEditor = GetEditor() ;
		if ( pEditor != nullptr )
		{
			pEditor->AddEditUndo( GetComposition(), this, L"ボーン物理演算パラメータ反映" ) ;
		}
		CmdApplyPhysParams( pEditor ) ;
		return ;

	case	paramCmdCopyPhysParams:
		pEditor = GetEditor() ;
		if ( pEditor != nullptr )
		{
			pEditor->AddEditUndo( GetComposition(), this, L"子ボーンへ物理マテリアル複製" ) ;
		}
		CmdCopyChildrenPhysPamras( pEditor ) ;
		return ;
	case	paramCmdRegisterPose:
		pComp = GetComposition() ;
		if ( pComp == nullptr )
		{
			return ;
		}
		pEditor = pComp->GetEditor() ;
		pComposer = pComp->GetComposer() ;
		if ( pEditor != nullptr )
		{
			S3DModelPoseLibrary *
				pPoseLib = pComposer->GetAssets().GetPoseLibraryAs( m_strRefPoseLib ) ;
			S3DSceneComposer::ResourceContainer *
				prc = pComposer->GetAssets().
						GetResourceContainerAs( m_strRefPoseLib ) ;
			if ( (pPoseLib == nullptr) || (prc == nullptr) )
			{
				SString	strMsg ;
				strMsg.Format
					( L"保存先のポーズライブラリ「%s」がありません。",
									(const wchar_t*) m_strRefPoseLib ) ;
				pEditor->DoMessageBox( strMsg, L"エラー", msgboxStyleOk ) ;
				return ;
			}
			if ( m_strRefPoseID.IsEmpty() )
			{
				pEditor->DoMessageBox
					( L"保存先のポーズ名が指定されていません", L"エラー", msgboxStyleOk ) ;
				return ;
			}
			S3DModelPose *	pPose = pPoseLib->GetPoseAs( m_strRefPoseID ) ;
			if ( pPose != nullptr )
			{
				SString	strMsg ;
				strMsg.Format
					( L"保存先のポーズ「%s」は既に存在しています。\n"
						L"上書きしますか？",
						(const wchar_t*) m_strRefPoseID ) ;
				if ( pEditor->DoMessageBox
					( strMsg, L"確認", msgboxStyleYesNo ) != msgboxResultYes )
				{
					return ;
				}
			}
			else
			{
				pPose = new S3DModelPose ;
				pPoseLib->AddPoseAs( m_strRefPoseID, pPose ) ;
			}
			//
			CmdRegisterPose( pComp, pPose ) ;
			//
			pEditor->SetResourceModifiedFlag( prc ) ;
			pEditor->NotifyEditResourceElements( prc ) ;
		}
		return ;

	case	paramCmdRestorePose:
		pComp = GetComposition() ;
		if ( pComp == nullptr )
		{
			return ;
		}
		pEditor = pComp->GetEditor() ;
		pComposer = pComp->GetComposer() ;
		if ( pEditor != nullptr )
		{
			S3DModelPose *	pPose =
				pComposer->GetAssets().GetPoseLibrary().GetPoseAs( m_strRefPoseID ) ;
			if ( pPose == nullptr )
			{
				SString	strMsg ;
				strMsg.Format
					( L"展開元のポーズ「%s」が見つかりません。",
						(const wchar_t*) m_strRefPoseID ) ;
				pEditor->DoMessageBox( strMsg, L"エラー", msgboxStyleOk ) ;
				return ;
			}
			pEditor->AddEditUndo( pComp, this, L"ポーズ展開" ) ;
			CmdRestorePose( pComp, pPose, pEditor ) ;
		}
		return ;
	}
	SpaceSerializer::GetBooleanParameter( i ) ;
}

void S3DBoneSerializer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramPhysMaterialRef:
		if ( m_strPhysMaterialRef != pwszCmd )
		{
			m_strPhysMaterialRef = pwszCmd ;
			UpdatePhysMaterialRef() ;
			//
			if ( m_pPhysMaterial != nullptr )
			{
				m_physMaterial = m_pPhysMaterial->GetPhysMaterial() ;
			}
			else
			{
				for ( int j = 0; j < S3DModelBoneSpace::PhysMaterial::presetCount; j ++ )
				{
					if ( m_strPhysMaterialRef == m_pwszPresetName[j] )
					{
						m_physMaterial = S3DModelBoneSpace::PhysMaterial::m_preset[j] ;
						break ;
					}
				}
			}
		}
		return ;

	case	paramCmdParamRefPoseLib:
		m_strRefPoseLib = pwszCmd ;
		return ;

	case	paramCmdParamPoseID:
		m_strRefPoseID = pwszCmd ;
		return ;
	}
	SpaceSerializer::SetCommandParameter( i, pwszCmd ) ;
}

size_t S3DBoneSerializer::SetBinaryParameter
	( size_t i, const void * pSrc, size_t nBufBytes )
{
	switch ( i )
	{
	case	paramOrgMatrix:
		if ( pSrc == nullptr )
		{
			return	sizeof(S4DDMatrix) ;
		}
		if ( nBufBytes == sizeof(S4DDMatrix) )
		{
			m_mat4OrgMatrix = *((const S4DDMatrix*)pSrc) ;
			return	sizeof(S4DDMatrix) ;
		}
		return	0 ;
	}
	return	SpaceSerializer::SetBinaryParameter( i, pSrc, nBufBytes ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DBoneSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	S3DSceneComposer::Composition *	pComp ;
	S3DSceneComposer *				pComposer ;
	int								j ;
	switch ( i )
	{
	case	paramPhysMaterialRef:
		aStrSet.Add( new SString( L"" ) ) ;
		for ( j = 0; j < S3DModelBoneSpace::PhysMaterial::presetCount; j ++ )
		{
			aStrSet.Add( new SString( m_pwszPresetName[j] ) ) ;
		}
		pComp = GetComposition() ;
		if ( pComp != nullptr )
		{
			pComp->EnumerateItemIDsAs
				( aStrSet, ESL_RUNTIME_CLASS(S3DBonePhysMaterialSerializer) ) ;
		}
		return	true ;

	case	paramCmdParamRefPoseLib:
		pComposer = GetComposer() ;
		if ( pComposer != nullptr )
		{
			pComposer->Assets().EnumerateResourceIDsAs
				( aStrSet, S3DSceneComposer::resourceTypePose ) ;
		}
		return	true ;

	case	paramCmdParamPoseID:
		pComposer = GetComposer() ;
		if ( pComposer != nullptr )
		{
			pComposer->Assets().EnumeratePoseStringSet( aStrSet ) ;
		}
		return	true ;

	}
	return	SpaceSerializer::EnumerateStringSet( i, aStrSet ) ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DBoneSerializer::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramZoom:
	case	paramTransparency:
	case	paramColorMul:
	case	paramColorAdd:
	case	paramVisible:
	case	paramForceToon:
	case	paramForceBorder:
	case	paramFreeToon:
	case	paramFreeBorder:
	case	paramUseCollision:
	case	paramGlobalSpace:
	case	paramCameraShift:
	case	paramCameraSpace:
	case	paramHideNear:
	case	paramHideFar:
	case	paramIgnore:
	case	paramSetLayerSace:
	case	paramLayeredPriority:
	case	paramLayeredParam:
	case	paramLayeredReflMap:
	case	paramLayeredRefrMap:
	case	paramLayeredField:
	case	paramLayeredStaticItem1:
	case	paramLayeredStaticItem2:
	case	paramLayeredDynamicItem1:
	case	paramLayeredDynamicItem2:
	case	paramLayeredDynamicItem3:
	case	paramLayeredEffectItem:
		return	false ;

	case	paramAttenuation:
	case	paramShrinkable:
	case	paramElasticity:
	case	paramMinStretch:
	case	paramMaxStretch:
	case	paramHardness:
	case	paramEffect:
	case	paramLimitedAngle:
	case	paramCollisionRadius:
	case	paramFrictionalResistance:
	case	paramPhysExFlags1:
		return	m_flagBonePhysics ;
	}
	return	SpaceSerializer::IsParameterValidation( i ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DBoneSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	2:
		return	L"ボーン" ;
	case	3:
		return	L"物理演算" ;
	case	4:
		return	L"編集コマンドパラメータ" ;
	}
	return	SpaceSerializer::GetParameterCategoryName( iCategory ) ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DBoneSerializer::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp, uint32_t nFlags )
{
	uint32_t	nRefFlags =
			SpaceSerializer::UpdatePropertyReference( comp, nFlags ) ;
	if ( nFlags & S3DSceneComposer::updateRefItem )
	{
		UpdatePhysMaterialRef() ;
	}
	return	nRefFlags ;
}

// 変換行列更新
//////////////////////////////////////////////////////////////////////////////
void S3DBoneSerializer::UpdateSpaceMatrix( void )
{
	if ( m_fpPoseWeight >= 0.99999 )
	{
		SpaceSerializer::UpdateSpaceMatrix() ;
		return ;
	}
	if ( m_fpPoseWeight < 1.0e-5 )
	{
		S3DDMatrix	matLocal = m_qPhysRotation ;
		matLocal.MagnifyByVector( m_vSpaceZoom ) ;
		SetLocalTransformation( matLocal ) ;
		return ;
	}
	S3DDQuaternion	qPose( 1, 0, 0, 0 ) ;
	qPose.Lerp( m_qPhysRotation,
				S3DDQuaternion(m_matSpaceRotation), m_fpPoseWeight ) ;
	S3DDMatrix	matLocal = m_qPhysRotation ;
	matLocal.MagnifyByVector( m_vSpaceZoom ) ;
	SetLocalTransformation( matLocal ) ;
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DBoneSerializer::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	if ( m_flagBonePhysics && (msecPast >= 1) && (m_fpPoseWeight < 0.9999) )
	{
		S3DDMatrix	matBone ;
		S3DDVector	vBone ;
		CalcSpaceLinkTransformation( matBone, vBone ) ;
		//
		CalculateSubPhysics
			( m_physMaterial, matBone, vBone, msecPast * 0.001, &scene ) ;
		//
		scene.PostSceneUpdate() ;
	}
	SpaceSerializer::OnTimer( scene, msecPast ) ;
}

void S3DBoneSerializer::OnUpdateFrame
	( double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	if ( seek == S3DSceneComposer::seekJumpReset )
	{
		m_flagPhysCurrent = false ;
	}
	SpaceSerializer::OnUpdateFrame( fpFrame, seek ) ;
}

// 物理演算
//////////////////////////////////////////////////////////////////////////////
void S3DBoneSerializer::CalculateSubPhysics
	( const S3DModelBoneSpace::PhysMaterial& mtrl,
		const S3DDMatrix& matBone,
		const S3DDVector& vBone,
		double secPast, S3DCollider * pCollider )
{
	double	framesPast = secPast * 60.0 ;
	if ( !m_flagPhysCurrent )
	{
		m_physCurrent.vPos = m_vBoneHandle ;
		m_physCurrent.vSpeed = S3DDVector( 0, 0, 0 ) ;
		m_physCurrent.vLastExSpeed = S3DDVector( 0, 0, 0 ) ;
		m_physCurrent.matLastSpace = matBone ;
		m_physCurrent.vLastSpace = vBone ;
		m_physCurrent.nHitCollider = 0 ;
		m_physCurrent.vHitNormal = S3DDVector( 0, 0, 1 ) ;
		m_flagPhysCurrent = true ;
	}
	//
	// 変換行列計算
	//
	S3DDMatrix	matSubBone = matBone * m_matTransformation ;
	S3DDVector	vSubBonePos = matBone * m_vCenter ;
	S3DDMatrix	matCurBone = matBone ;
	S3DDVector	vCurBonePos = vBone ;
	S3DDMatrix	matLastBone = m_physCurrent.matLastSpace ;
	S3DDVector	vLastBonePos = m_physCurrent.vLastSpace ;
	S3DDVector	vBoneHandle = m_vBoneHandle ;
	S3DDMatrix	matICurBone, matIBone ;
	matICurBone.InverseOf( matCurBone ) ;
	matIBone.InverseOf( matBone ) ;
	//
	m_physCurrent.matLastSpace = matBone ;
	m_physCurrent.vLastSpace = vBone ;
	//
	// 外因（回転変換）変位
	//
	S3DDVector	vDeltaBone0 = matLastBone * m_physCurrent.vPos + vLastBonePos ;
	S3DDVector	vDeltaBone1 = matCurBone * m_physCurrent.vPos + vCurBonePos ;
	vDeltaBone0 -= vDeltaBone1 ;
	//
	S3DDVector	vSpeed = vDeltaBone0 * (1.0 / secPast) ;
	S3DDVector	vAccel = vSpeed - m_physCurrent.vLastExSpeed ;
	m_physCurrent.vLastExSpeed = vSpeed ;
	//
//	vAccel += exog.vAcceleration * secPast ;
	//
	m_physCurrent.vSpeed += vAccel * mtrl.fpEffect ;
	//
	// 内因（伸縮）変位
	//
	S3DDVector	vDeltaBone( 0, 0, 0 ) ;
	double		fpBoneLength = vBoneHandle.Absolute() ;
	double		fpCurLength = m_physCurrent.vPos.Absolute() ;
	S3DDVector	vAbsCurBone ;
	double		fpOrthSpeed ;
	if ( fpCurLength > 0.0 )
	{
		vAbsCurBone = m_physCurrent.vPos * (1.0 / fpCurLength) ;
	}
	else
	{
		vAbsCurBone = vBoneHandle ;
		vAbsCurBone.Normalize() ;
	}
	S3DDVector	vLocalSpeed = matICurBone * m_physCurrent.vSpeed ;
	fpOrthSpeed = (vAbsCurBone | vLocalSpeed) ;
	//
	if ( fpCurLength < fpBoneLength )
	{
		double	fpShrinkable = mtrl.fpShrinkable * framesPast ;
		double	fpMinLength = fpBoneLength * mtrl.fpMinStretch ;
		if ( fpCurLength > fpMinLength )
		{
			vDeltaBone =
				vAbsCurBone
					* ((fpBoneLength - fpCurLength) * fpShrinkable) ;
		}
		else
		{
			vDeltaBone =
				vAbsCurBone
					* ((fpBoneLength - fpMinLength) * fpShrinkable) ;
			m_physCurrent.vPos = vAbsCurBone * fpMinLength ;
			ESLAssert( !m_physCurrent.vPos.IsNaN() ) ;
		}
	}
	else
	{
		double	fpElasticity = mtrl.fpElasticity * framesPast ;
		double	fpMaxLength = fpBoneLength * mtrl.fpMaxStretch ;
		if ( fpCurLength < fpMaxLength )
		{
			vDeltaBone =
				vAbsCurBone
					* ((fpBoneLength - fpCurLength) * fpElasticity) ;
		}
		else
		{
			vDeltaBone =
				vAbsCurBone
					* ((fpBoneLength - fpMaxLength) * fpElasticity) ;
			m_physCurrent.vPos = vAbsCurBone * fpMaxLength ;
			ESLAssert( !m_physCurrent.vPos.IsNaN() ) ;
		}
	}
	vLocalSpeed += vDeltaBone ;
	//
	S3DBoneSerializer *	pParent =
		ESLTypeCast<S3DBoneSerializer>( m_refParent.GetReference() ) ;
	if ( pParent != nullptr )
	{
		S3DDVector	vDeltaSpeed = matBone * vDeltaBone * mtrl.fpEffect * 0.5 ;
		if ( pParent->m_physCurrent.nHitCollider > 0 )
		{
			double	fpResistance =
						pow( mtrl.fpFrictionalResistance, framesPast ) ;
			vDeltaSpeed *= fpResistance ;
		}
		pParent->m_physCurrent.vSpeed -= vDeltaSpeed ;
	}
	//
	// 内因（曲がり）変位
	//
	S3DDVector	vCurDelta = (m_physCurrent.vPos - vBoneHandle) * (1.0 / secPast) ;
	double	fpHardness = esl_fclamp( mtrl.fpHardness, 0.0, 0.99 ) * framesPast ;
	fpHardness *= pow( 1.0 - mtrl.fpAttenuation, secPast ) ;
	vLocalSpeed -= vCurDelta * fpHardness ;
	if ( m_physCurrent.nHitCollider > 0 )
	{
		double	fpResistance =
					pow( mtrl.fpFrictionalResistance, framesPast ) ;
		m_physCurrent.vSpeed *= fpResistance ;
	}
	m_physCurrent.vSpeed = matCurBone * vLocalSpeed ;
	//
	// 速度をボーンハンドルへ反映／行列計算
	//
//	S3DDVector	vStream = exog.vStream ;
	S3DDVector	vStream( 0, 0, 0 ) ;
	double		fpAttenuationDelta =
					pow( 1.0 - mtrl.fpAttenuation, secPast ) ;
	m_physCurrent.vSpeed -= vStream ;
	m_physCurrent.vSpeed *= fpAttenuationDelta ;
	m_physCurrent.vSpeed += vStream ;
	m_physCurrent.vPos += matICurBone * (m_physCurrent.vSpeed * secPast) ;
	ESLAssert( !m_physCurrent.vPos.IsNaN() ) ;
	//
	fpCurLength = m_physCurrent.vPos.Absolute() ;
	if ( fpCurLength < fpBoneLength * mtrl.fpMinStretch )
	{
		m_physCurrent.vPos.Normalize() ;
		m_physCurrent.vPos *= fpBoneLength * mtrl.fpMinStretch ;
		ESLAssert( !m_physCurrent.vPos.IsNaN() ) ;
	}
	if ( fpCurLength > fpBoneLength * mtrl.fpMaxStretch )
	{
		m_physCurrent.vPos *=
				fpBoneLength * mtrl.fpMaxStretch / fpCurLength ;
		ESLAssert( !m_physCurrent.vPos.IsNaN() ) ;
	}
	//
	if ( m_physCurrent.nHitCollider > 0 )
	{
		m_physCurrent.nHitCollider -- ;
	}
	//
	if ( pCollider != nullptr )
	{
		//
		// ボーン当たり判定
		//
		S3DDVector	vHandlePos =
				matBone * m_physCurrent.vPos + vSubBonePos ;
		//
		S3DCollisionResult	rsHit ;
		if ( m_physMaterial.nPhysExFlags1 != 0 )
		{
			rsHit.SetExceptionUserFlags( m_physMaterial.nPhysExFlags1 ) ;
		}
		if ( pCollider->IsHitAgainstSphere
			( vHandlePos, (float) mtrl.fpCollisionRadius, rsHit ) )
		{
			// 当たり座標（表面）座標をボーンローカル空間に変換
			S3DDVector	vdHitLocal = rsHit.vHitGlobal ;
			S3DDVector	vHitNormal = rsHit.vNormal ;
			vHitNormal.Normalize() ;
			//
			vdHitLocal += vHitNormal * mtrl.fpCollisionRadius ;
			vdHitLocal -= vSubBonePos ;
			matIBone.RevolveVector( vdHitLocal ) ;
			//
			matIBone.RevolveVector( vHitNormal ) ;
			vdHitLocal.Normalize() ;
			vHitNormal.Normalize() ;
			//
			S3DDVector	vLastHandle = m_physCurrent.vPos ;
			S3DDVector	vModHanlde =
					vdHitLocal * m_physCurrent.vPos.Absolute() ;
			S3DDVector	vModDelta =
					vModHanlde - m_physCurrent.vPos ;
			m_physCurrent.vPos = vModHanlde ;
			ESLAssert( !m_physCurrent.vPos.IsNaN() ) ;
			//
			// 運動量から当たり表面の法線方向を削除
			//
			S3DDVector	vGlobalNormal = rsHit.vNormal ;
			vGlobalNormal.Normalize() ;
			S3DDVector	vHitSpeed = m_physCurrent.vSpeed ;
			double		fpHitSpeed =
							vGlobalNormal.InnerProduct( vHitSpeed ) ;
			double		fpResistance =
							pow( mtrl.fpFrictionalResistance, framesPast ) ;
			vHitSpeed += vGlobalNormal * esl_fmax( - fpHitSpeed, 0.0 ) ;
			vHitSpeed *= fpResistance ;
			m_physCurrent.vSpeed = vHitSpeed ;
			//
			// 当たり判定オブジェクトに押された運動量加算
			S3DDVector	vModSpeed =
				vModDelta * (mtrl.fpEffect / secPast * fpResistance) ;
			m_physCurrent.vSpeed += matBone * vModSpeed ;
			//
			m_physCurrent.nHitCollider = 4 ;
			m_physCurrent.vHitNormal = vGlobalNormal ;
		}
	}
	if ( mtrl.fpLimitedAngle > 0.0 )
	{
		//
		// 曲がり角制限処理
		//
		S3DDVector	vPos = m_physCurrent.vPos ;
		double		r = vPos.Absolute() ;
		if ( r > 0 )
		{
			S3DDVector	vHandle = vBoneHandle ;
			vHandle.Normalize() ;
			//
			vPos *= 1.0 / r ;
			//
			double	radLim = (180.0 - mtrl.fpLimitedAngle) * PI / 180.0 ;
			double	cosLim = cos( radLim ) ;
			double	cosCross = vHandle.InnerProduct( vPos ) ;
			if ( cosCross < cosLim )
			{
				S3DDVector	vOrth = vPos - vHandle * cosCross ;
				vOrth.Normalize() ;
				//
				m_physCurrent.vPos =
						vHandle * (cosLim * r)
							+ vOrth * (sin(radLim) * r) ;
				ESLAssert( !m_physCurrent.vPos.IsNaN() ) ;
			}
		}
	}
	//
	// パラメータをボーンに反映
	//
	S3DDMatrix	matRotate( 1, 1, 1 ) ;
	matRotate.VectorRotationOf( m_vBoneHandle, m_physCurrent.vPos ) ;
	m_qPhysRotation = S3DDQuaternion(matRotate) ;
	//
	UpdateSpaceMatrix() ;
}

// ボーン行列リセット
//////////////////////////////////////////////////////////////////////////////
void S3DBoneSerializer::CmdResetBoneMatrix( S3DCompositionEditorInterface * pEditor )
{
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pEditor != nullptr )
	{
		pEditor->EditMatrixProperty
			( pComp, this, paramRotation, S3DDMatrix( 1, 1, 1 ), false ) ;
		pEditor->EditVectorProperty
			( pComp, this, paramZoom, S3DDVector( 1, 1, 1 ), false ) ;
	}
	else
	{
		SetMatrixParameter( paramRotation, S3DDMatrix( 1, 1, 1 ) ) ;
		SetVectorParameter( paramZoom, S3DDVector( 1, 1, 1 ) ) ;
	}
	for ( size_t i = 0; i < GetChildrenCount(); i ++ )
	{
		S3DBoneSerializer *	pBone =
			ESLTypeCast<S3DBoneSerializer>( GetChildAt( i ) ) ;
		if ( pBone != nullptr )
		{
			pBone->CmdResetBoneMatrix( pEditor ) ;
		}
	}
}

// ボーン左右反転
//////////////////////////////////////////////////////////////////////////////
void S3DBoneSerializer::CmdReverseBone( S3DCompositionEditorInterface * pEditor )
{
	//
	// ボーンの反転
	//
	S3DDMatrix	matRot = GetMatrixParameter( paramRotation ) ;
	S3DDVector	vPos = GetVectorParameter( paramPosition ) ;
	S3DDVector	vHandle = GetVectorParameter( paramBoneHandle ) ;
	S3DDVector	vBendDir = GetVectorParameter( paramBendDir ) ;
	//
	S3DDMatrix	matMirror( -1, 1, 1 ) ;
	matRot = matMirror.Inverse() * matRot * matMirror ;
	vPos = matMirror * vPos ;
	vHandle = matMirror * vHandle ;
	vBendDir = matMirror * vBendDir ;
	//
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pEditor != nullptr )
	{
		pEditor->EditMatrixProperty
			( pComp, this, paramRotation, matRot, false ) ;
		pEditor->EditVectorProperty
			( pComp, this, paramPosition, vPos, false ) ;
		pEditor->EditVectorProperty
			( pComp, this, paramBoneHandle, vHandle, false ) ;
		pEditor->EditVectorProperty
			( pComp, this, paramBendDir, vBendDir, false ) ;
	}
	else
	{
		SetMatrixParameter( paramRotation, matRot ) ;
		SetVectorParameter( paramPosition, vPos ) ;
		SetVectorParameter( paramBoneHandle, vHandle ) ;
		SetVectorParameter( paramBendDir, vBendDir ) ;
	}
	//
	// サブボーンの反転
	//
	for ( size_t i = 0; i < GetChildrenCount(); i ++ )
	{
		S3DBoneSerializer *	pBone =
			ESLTypeCast<S3DBoneSerializer>( GetChildAt( i ) ) ;
		if ( pBone != nullptr )
		{
			pBone->CmdReverseBone( pEditor ) ;
		}
	}
	//
	// サブアイテムの反転
	//
	for ( size_t i = 0; i < GetItemCount(); i ++ )
	{
		S3DSceneComposer::CommonSerializer *	pItem =
			ESLTypeCast<S3DSceneComposer::CommonSerializer>( GetItemAt( i ) ) ;
		if ( pItem == nullptr )
		{
			continue ;
		}
		matRot = pItem->GetMatrixParameter( paramRotation ) ;
		vPos = pItem->GetVectorParameter( paramPosition ) ;
		matRot = matMirror.Inverse() * matRot * matMirror ;
		vPos = matMirror * vPos ;
		if ( pEditor != nullptr )
		{
			pEditor->EditMatrixProperty
				( pComp, pItem, paramRotation, matRot, false ) ;
			pEditor->EditVectorProperty
				( pComp, pItem, paramPosition, vPos, false ) ;
		}
		else
		{
			pItem->SetMatrixParameter( paramRotation, matRot ) ;
			pItem->SetVectorParameter( paramPosition, vPos ) ;
		}
		S3DMeshEditorSerializer *
			pMesh = ESLTypeCast<S3DMeshEditorSerializer>( pItem ) ;
		if ( pMesh != nullptr )
		{
			//
			// メッシュの反転
			//
			S3DMeshEditorObject *	pMeshEditor =
				ESLTypeCast<S3DMeshEditorObject>( &(pMesh->MeshEditor()) ) ;
			if ( pMeshEditor != nullptr )
			{
				SArraySet<S3DMeshEditor::Patch*>	ps ;
				for ( size_t j = 0; j < pMeshEditor->GetPatchCount(); j ++ )
				{
					ps.AddSorted( pMeshEditor->GetPatchAt(j) ) ;
				}
				S3DMeshEditor::SelectPointSet	sps ;
				pMeshEditor->SelectPointsFromPatchs( sps, ps ) ;
				pMeshEditor->SetSelectedPoints( sps ) ;
				pMeshEditor->NormalizeSelecedPoints() ;
				//
				S3DMatrix	matsMirror( -1, 1, 1 ) ;
				S3DVector	vsZero( 0, 0, 0 ) ;
				pMeshEditor->DoTransformPoints( matsMirror, vsZero, pMesh ) ;
				//
				pMeshEditor->DoInversePatch( ps, pMesh ) ;
			}
		}
	}
}

// 全子ボーンに物理マテリアルを複製
//////////////////////////////////////////////////////////////////////////////
void S3DBoneSerializer::CmdCopyChildrenPhysPamras( S3DCompositionEditorInterface * pEditor )
{
	for ( size_t i = 0; i < GetChildrenCount(); i ++ )
	{
		S3DBoneSerializer *	pBone =
			ESLTypeCast<S3DBoneSerializer>( GetChildAt( i ) ) ;
		if ( pBone != nullptr )
		{
			pBone->CmdCopyPhysPamrasFrom
				( GetPhysMaterial(), GetPhysMaterialRef(), pEditor ) ;
		}
	}
}

void S3DBoneSerializer::CmdCopyPhysPamrasFrom
	( const S3DModelBoneSpace::PhysMaterial& physMaterial,
		const wchar_t * pwszPhysMaterial, S3DCompositionEditorInterface * pEditor )
{
	PhysMaterial() = physMaterial ;
	SetPhysMaterialRef( pwszPhysMaterial ) ;
	//
	for ( size_t i = 0; i < GetChildrenCount(); i ++ )
	{
		S3DBoneSerializer *	pBone =
			ESLTypeCast<S3DBoneSerializer>( GetChildAt( i ) ) ;
		if ( pBone != nullptr )
		{
			pBone->CmdCopyPhysPamrasFrom
				( physMaterial, pwszPhysMaterial, pEditor ) ;
		}
	}
}

// 物理演算マテリアルをボーンに反映
//////////////////////////////////////////////////////////////////////////////
void S3DBoneSerializer::CmdApplyPhysParams( S3DCompositionEditorInterface * pEditor )
{
	if ( m_pPhysMaterial != nullptr )
	{
		if ( pEditor != nullptr )
		{
			S3DSceneComposer::Composition *	pComp = GetComposition() ;
			const S3DModelBoneSpace::PhysMaterial&
						physParam = m_pPhysMaterial->GetPhysMaterial() ;
			pEditor->EditScalarProperty
				( pComp, this, paramAttenuation, physParam.fpAttenuation, false ) ;
			pEditor->EditScalarProperty
				( pComp, this, paramShrinkable, physParam.fpShrinkable, false ) ;
			pEditor->EditScalarProperty
				( pComp, this, paramElasticity, physParam.fpElasticity, false ) ;
			pEditor->EditScalarProperty
				( pComp, this, paramMinStretch, physParam.fpMinStretch, false ) ;
			pEditor->EditScalarProperty
				( pComp, this, paramMaxStretch, physParam.fpMaxStretch, false ) ;
			pEditor->EditScalarProperty
				( pComp, this, paramHardness, physParam.fpHardness, false ) ;
			pEditor->EditScalarProperty
				( pComp, this, paramEffect, physParam.fpEffect, false ) ;
			pEditor->EditScalarProperty
				( pComp, this, paramLimitedAngle, physParam.fpLimitedAngle, false ) ;
			pEditor->EditScalarProperty
				( pComp, this, paramCollisionRadius, physParam.fpCollisionRadius, false ) ;
			pEditor->EditScalarProperty
				( pComp, this, paramFrictionalResistance, physParam.fpFrictionalResistance, false ) ;
			pEditor->EditIntegerProperty
				( pComp, this, paramPhysExFlags1, physParam.nPhysExFlags1, false ) ;
		}
		else
		{
			m_physMaterial = m_pPhysMaterial->GetPhysMaterial() ;
		}
	}
	for ( size_t i = 0; i < GetChildrenCount(); i ++ )
	{
		S3DBoneSerializer *	pBone =
			ESLTypeCast<S3DBoneSerializer>( GetChildAt( i ) ) ;
		if ( pBone != nullptr )
		{
			pBone->CmdApplyPhysParams( pEditor ) ;
		}
	}
}

// ポーズ登録
//////////////////////////////////////////////////////////////////////////////
void S3DBoneSerializer::CmdRegisterPose
	( S3DSceneComposer::Composition * pComp, S3DModelPose * pPose )
{
	if ( !m_flagBonePhysics || (m_fpPoseWeight > 0.00001) )
	{
		const wchar_t *	pwszBoneID = pComp->GetSceneItemIDOf( this ) ;
		if ( pwszBoneID != nullptr )
		{
			S3DModelPose::JointAnimation *	pja = pPose->GetJointAs( pwszBoneID ) ;
			if ( pja == nullptr )
			{
				pja = new S3DModelPose::JointAnimation ;
				pja->m_vHandle = m_vBoneHandle ;
				pPose->AddJointAs( pwszBoneID, pja ) ;
			}
			pja->m_qRotation = m_matSpaceRotation ;
			pja->m_vZoom = m_vSpaceZoom ;
			pja->m_wPhysBlend = m_fpPoseWeight ;
		}
	}
	for ( size_t i = 0; i < GetChildrenCount(); i ++ )
	{
		S3DBoneSerializer *	pBone =
			ESLTypeCast<S3DBoneSerializer>( GetChildAt( i ) ) ;
		if ( pBone != nullptr )
		{
			pBone->CmdRegisterPose( pComp, pPose ) ;
		}
	}
}

// ポーズ展開
//////////////////////////////////////////////////////////////////////////////
void S3DBoneSerializer::CmdRestorePose
	( S3DSceneComposer::Composition * pComp,
		S3DModelPose * pPose,
		S3DCompositionEditorInterface * pEditor )
{
	const wchar_t *	pwszBoneID = pComp->GetSceneItemIDOf( this ) ;
	if ( pwszBoneID != nullptr )
	{
		S3DModelPose::JointAnimation *	pja = pPose->GetJointAs( pwszBoneID ) ;
		if ( pja != nullptr )
		{
			S3DDMatrix	matRot = pja->m_qRotation ;
			//
			pEditor->EditMatrixProperty
				( pComp, this, paramRotation, matRot, false ) ;
			pEditor->EditVectorProperty
				( pComp, this, paramZoom, pja->m_vZoom, false ) ;
			pEditor->EditScalarProperty
				( pComp, this, paramPoseWeight, pja->m_wPhysBlend, false ) ;
		}
	}
	for ( size_t i = 0; i < GetChildrenCount(); i ++ )
	{
		S3DBoneSerializer *	pBone =
			ESLTypeCast<S3DBoneSerializer>( GetChildAt( i ) ) ;
		if ( pBone != nullptr )
		{
			pBone->CmdRestorePose( pComp, pPose, pEditor ) ;
		}
	}
}



//////////////////////////////////////////////////////////////////////////////
// メッシュ編集アイテム
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DMeshEditorSerializer::m_paramEntries
		[S3DMeshEditorSerializer::paramMeshCount] =
{
	{ L"mesh_editor",
		S3DSceneComposer::typeBinary,
		S3DSceneComposer::attrConstant1,
		L"メッシュエディタ", nullptr },
	{ L"mesh_primitive_type",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration
		| S3DSceneComposer::attrUIOnlyEnumeration,
		L"プリミティブ", nullptr },
	{ L"mesh_edge_method",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration
		| S3DSceneComposer::attrUIOnlyEnumeration,
		L"稜線分割",
		L"稜線の分割方法を指定します\n"
		L"partial_edge: 稜線ごとに法線の分割を判定\n"
		L"all_smooth: 全頂点の法線を分割しない\n"
		L"all_div_points: 全頂点を面毎に分割する（但し法線はスムージング）\n"
		L"all_flat: 全頂点を面毎に分割する（法線はフラット）\n" },
	{ L"mesh_edge_by_angle",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"稜線角度を使用", L"稜線法線を指定角度以上で分割するか指定します" },
	{ L"mesh_edge_angle",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrUIScalarSlider,
		L"稜線角度を使用", L"稜線法線を指定角度以上で分割するか指定します", 0, 180 },
	{ L"mesh_div_ahead",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"メッシュ細分化前処理", L"メッシュ細分化処理をコントローラーより前に行う" },
	{ L"mesh_div_method",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration
		| S3DSceneComposer::attrUIOnlyEnumeration,
		L"メッシュ細分化法",
		L"メッシュの細分化メソッドを指定します\n"
		L"lerp: 線形補完\n"
		L"bezier: ベジェ補完\n" },
	{ L"mesh_div_horz",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1,
		L"メッシュ水平細分化", nullptr },
	{ L"mesh_div_vert",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1,
		L"メッシュ垂直細分化", nullptr },
	{ L"material0",
		S3DSceneComposer::typeCommand,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration,
		L"マテリアル[0]", L"描画に使用するマテリアルを指定します" },
	{ L"material1",
		S3DSceneComposer::typeCommand,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration,
		L"マテリアル[1]", L"描画に使用するマテリアルを指定します" },
	{ L"material2",
		S3DSceneComposer::typeCommand,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration,
		L"マテリアル[2]", L"描画に使用するマテリアルを指定します" },
	{ L"material3",
		S3DSceneComposer::typeCommand,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration,
		L"マテリアル[3]", L"描画に使用するマテリアルを指定します" },
	{ L"enable_collider",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"当たり判定生成", L"メッシュを当たり判定として設定します" },
	{ L"enable_collider_all",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"全メッシュ当たり判定", L"全てのメッシュを当たり判定として設定します" },
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
	{ L"col_div_horz",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1,
		L"当たり判定メッシュ水平細分化", nullptr },
	{ L"col_div_vert",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1,
		L"当たり判定メッシュ垂直細分化", nullptr },
	{ L"freeze_mesh",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"メッシュ固定化", L"メッシュの動的な変形・生成を無効化し、メッシュを固定します" },
	{ L"enable_bone",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"ボーン変形有効", L"ボーン変形を有効化します" },
	{ L"bone_ref",
		S3DSceneComposer::typeCommand,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration,
		L"ボーン参照", L"ボーン変形に参照するルートボーンを指定します" },
	{ L"bone_auto_map",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrEditorCommand,
		L"ボーンから自動的にウェイトマップ作成", nullptr },
	{ L"bone_clear_map",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrEditorCommand,
		L"ボーン用ウェイトマップ削除", nullptr },
	{ L"bone_matrix_map",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrEditorCommand,
		L"ボーン用行列を現在の値に設定", nullptr },
	{ L"bone_normalize_weight",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrEditorCommand,
		L"ボーン・ウェイトマップを正規化", nullptr },
	{ L"bone_optimize_weight",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrEditorCommand,
		L"ボーン・ウェイトマップを最適化", nullptr },
	{ L"instancing",
		S3DSceneComposer::typeBinary,
		S3DSceneComposer::attrConstant1, L"インスタンス", nullptr },
	{ L"sort_instance",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1, L"インスタンス・ソート", nullptr },
	{ L"bake_instance_to_mesh",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrEditorCommand,
		L"インスタンスをメッシュにベイク", L"インスタンスをメッシュにベイクします" },
} ;

const S3DSceneComposer::ParamSetClass
	S3DMeshEditorSerializer::m_pscClass =
{
	&ItemBasicSerializer::m_pscClass,
	S3DMeshEditorSerializer::paramMeshCount,
	&S3DMeshEditorSerializer::m_paramEntries[0]
} ;

const SSystem::SXMLDocument::AttrInteger
	S3DMeshEditorSerializer::m_aiMeshEdgeMethods[5] =
{
	{ L"partial_edge", S3DMeshEditor::flagMeshPartialEdge },
	{ L"all_smooth", S3DMeshEditor::flagMeshAllSmooth },
	{ L"all_div_points", S3DMeshEditor::flagMeshAllDivPoints },
	{ L"all_flat", S3DMeshEditor::flagMeshAllFlat },
	{ nullptr, 0 },
} ;

const SSystem::SXMLDocument::AttrInteger
	S3DMeshEditorSerializer::m_aiMeshDivMethods[3] =
{
	{ L"lerp", S3DMeshEditor::divMethodLerp },
	{ L"bezier", S3DMeshEditor::divMethodBezier },
	{ nullptr, 0 },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO4
	( SakuraGL::S3DMeshEditorSerializer,
		ItemBasicSerializer,
		S3DMeshEditorBridge,
		RenderTarget, S3DInstancingItemInterface )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DMeshEditorSerializer, mesh_editor )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditorSerializer::S3DMeshEditorSerializer( void )
	: ItemBasicSerializer
		( m_ItemClassDescriptor.pwszClassID,
				&S3DMeshEditorSerializer::m_pscClass )
{
	m_flagsBehavior |= S3DScene::itemOwnerBehavior ;
	m_maskClasses |= (1 << S3DScene::classPreRender)
					| (1 << S3DScene::classPreRender2) ;
	//
	m_mesh.AttachExtraMeshInfo( this ) ;
	//
	m_instancing.SetSorting( S3DItemInstancingSerializer::sortNothing ) ;
	m_instancing.SetCulling( S3DItemInstancingSerializer::cullingNothing ) ;
	//
	S4DMatrix	mat4Instance( 1, 1, 1, 1 ) ;
	S3DColor	clrInstance( 0xFFFFFFFF, 0 ) ;
	m_instancing.InsertStaticInstanceAt( 0, mat4Instance, clrInstance, this ) ;
	m_instancing.UpdateStaticInstancingEntriesBase64() ;
	//
	for ( int i = 0; i < paramMaterialCount; i ++ )
	{
		m_pMaterials[i] =
			S3DMaterial::GetDefaultMaterial( S3DMaterial::defaultWhite ) ;
		m_vbEditMesh[i].SetBufferControlFlags
			( m_vbEditMesh[i].GetBufferControlFlags()
					| S3DVertexBuffer::bufferKeepDeviceBuffer ) ;
		m_nMeshBufCount[i] = 0 ;
		m_nColBufCount[i] = 0 ;
	}
	//
	m_flagCollision = false ;
	m_flagCollisionAll = false ;
	m_flagCollisionUpdate = false ;
	m_maskCollisionFlags = S3DCollision::colliderShape ;
	m_nCollisionAlpha = 0xFF ;
	m_nCollisionDivHorz = 1 ;
	m_nCollisionDivVert = 1 ;
	//
	m_flagDivMeshAhead = false ;
	m_flagFreezeMesh = false ;
	m_flagMeshUpdated = false ;
	m_flagLastMeshCtrled = false ;
	m_flagEnableBone = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditorSerializer::~S3DMeshEditorSerializer( void )
{
}

// マテリアル関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::AttachMaterial
	( size_t i, S3DMaterial * pMaterial, const wchar_t * pwszMaterialID )
{
	if ( i < paramMaterialCount )
	{
		m_pMaterials[i] = pMaterial ;
		m_strMaterialIDs[i] = pwszMaterialID ;
	}
}

// マテリアル参照更新
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::UpdateMaterialRef( void )
{
	S3DSceneComposer *	pComposer = nullptr ;
	for ( int i = 0; i < paramMaterialCount; i ++ )
	{
		m_pMaterials[i] = S3DMaterial::GetDefaultMaterial( S3DMaterial::defaultWhite ) ;
		if ( m_strMaterialIDs[i].IsEmpty() )
		{
			continue ;
		}
		if ( pComposer == nullptr )
		{
			pComposer = GetComposer() ;
		}
		if ( pComposer != nullptr )
		{
			m_pMaterials[i] =
				pComposer->GetAssets().
					GetMaterialLibrary().GetMaterialAs( m_strMaterialIDs[i] ) ;
			m_mesh.SetUpdateVertexFlag() ;
		}
	}
}

// ボーン参照更新
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::UpdateBoneRef( void )
{
	if ( m_strBoneRoot.IsEmpty() )
	{
		m_refBoneRoot = nullptr ;
		return ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		m_refBoneRoot = pComp->GetSceneSpaceAs( m_strBoneRoot ) ;
	}
}

// モデルバッファ更新（静的メッシュ）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::UpdateEditorMesh( S3DMeshEditor& mesh )
{
	S3DVertexBufferInterface *	pVB[paramMaterialCount] ;
	for ( int i = 0; i < paramMaterialCount; i ++ )
	{
		m_vbEditMesh[i].AttachDefaultMaterial
			( (m_pMaterials[i] != nullptr)
				? m_pMaterials[i]
				: S3DMaterial::GetDefaultMaterial( S3DMaterial::defaultWhite ) ) ;
		m_vbEditMesh[i].ClearBuffer() ;
		pVB[i] = &(m_vbEditMesh[i]) ;
	}
	const S3DMeshEditor::MeshParam&	mparam = m_mesh.GetMeshParameter() ;
	if ( !m_flagDivMeshAhead && ((mparam.nDivHorz >= 2) || (mparam.nDivVert >= 2)) )
	{
		S3DMeshEditor	meshTemp ;
		meshTemp.RedivideMeshFrom( mesh, mparam ) ;
		meshTemp.RenderVertexBuffers
			( pVB, m_pMaterials, m_nMeshBufCount, paramMaterialCount, mparam, 0 ) ;
	}
	else
	{
		mesh.RenderVertexBuffers
			( pVB, m_pMaterials, m_nMeshBufCount, paramMaterialCount, mparam, 0 ) ;
	}
	if ( m_flagFreezeMesh )
	{
		for ( int i = 0; i < paramMaterialCount; i ++ )
		{
			S3DRenderBuffer *	prbuf =
				ESLTypeCast<S3DRenderBuffer>( m_vbEditMesh[i].GetVertexBuffer() ) ;
			if ( prbuf != nullptr )
			{
				prbuf->RebuildAsSinglePrimitive() ;
			}
		}
	}
	m_flagMeshUpdated = true ;
	//
	if ( m_flagCollision | m_flagCollisionAll )
	{
		UpdateEditorCollision( mesh ) ;
	}
	else
	{
		m_flagCollisionUpdate = true ;
	}
}

void S3DMeshEditorSerializer::UpdateEditorCollision( S3DMeshEditor& mesh )
{
	S3DVertexBufferInterface *	pVB[paramMaterialCount] ;
	for ( int i = 0; i < paramMaterialCount; i ++ )
	{
		m_collision[i].AttachDefaultMaterial( m_pMaterials[i] ) ;
		m_collision[i].ClearBuffer() ;
		m_collision[i].AttachMeshUserData( (Item*) this ) ;
		m_collision[i].SetSceneClassesMask( (1 << m_classItem) | m_maskClasses ) ;
		m_collision[i].SetUserClassesMask( m_maskCollisionFlags ) ;
		m_collision[i].BeginBatchBuild() ;
		pVB[i] = &(m_collision[i]) ;
	}
	if ( !m_flagDivMeshAhead && ((m_nCollisionDivHorz >= 2) || (m_nCollisionDivVert >= 2)) )
	{
		S3DMeshEditor::MeshParam	mparam = m_mesh.GetMeshParameter() ;
		S3DMeshEditor	meshTemp ;
		mparam.nDivHorz = m_nCollisionDivHorz ;
		mparam.nDivVert = m_nCollisionDivVert ;
		meshTemp.RedivideMeshFrom( mesh, mparam ) ;
		meshTemp.RenderVertexBuffers
			( pVB, m_pMaterials, m_nColBufCount,
				paramMaterialCount, mparam,
				(m_flagCollisionAll ? S3DMeshEditor::renderAll
										: S3DMeshEditor::renderCollision) ) ;
	}
	else
	{
		mesh.RenderVertexBuffers
			( pVB, m_pMaterials, m_nColBufCount,
				paramMaterialCount, m_mesh.GetMeshParameter(),
				(m_flagCollisionAll ? S3DMeshEditor::renderAll
										: S3DMeshEditor::renderCollision) ) ;
	}
	for ( int i = 0; i < paramMaterialCount; i ++ )
	{
		m_collision[i].EndBatchBuild() ;
	}
	m_flagCollisionUpdate = false ;
}

// MeshController 処理
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::ProcessMeshControllers( S3DScene& scene )
{
	if ( m_flagFreezeMesh && m_flagMeshUpdated )
	{
		return ;
	}
	S3DMeshEditor *					pMesh = &m_mesh ;
	SSmartPointer<S3DMeshEditor>	pTempMesh ;
	//
	bool	flagUpdateMesh =
				m_flagLastMeshCtrled || m_mesh.GetUpdateVertexFlag() ;
	m_flagLastMeshCtrled = false ;
	//
	if ( MakeMeshWithControllers( scene, pMesh, pTempMesh ) )
	{
		flagUpdateMesh = true ;
	}
	if ( m_flagEnableBone )
	{
		if ( pMesh == &m_mesh )
		{
			pTempMesh = new S3DMeshEditor ;
			pTempMesh->DuplicateMesh( m_mesh ) ;
			pMesh = pTempMesh ;
		}
		if ( ProcessMeshWithBone( *pMesh ) )
		{
			m_flagLastMeshCtrled = true ;
			flagUpdateMesh = true ;
		}
	}
	if ( flagUpdateMesh )
	{
		#if	defined(__DEBUG__)
			pMesh->VerifyUpdateFlagByPatchUpdate() ;
		#endif
		pMesh->UpdateAllPatchs( m_mesh.GetMeshParameter() ) ;
		UpdateEditorMesh( *pMesh ) ;
	}
}

bool S3DMeshEditorSerializer::MakeMeshWithControllers
	( S3DScene& scene, S3DMeshEditor*& pMesh,
		SSystem::SSmartPointer<S3DMeshEditor>& pTempMesh )
{
	SSmartPointer<S3DRenderBuffer>	pTempRenderBuf[paramMaterialCount] ;
	bool	flagUpdateMesh = false ;
	pMesh = &m_mesh ;
	//
	const S3DMeshEditor::MeshParam&	mparam = m_mesh.GetMeshParameter() ;
	if ( m_flagDivMeshAhead && ((mparam.nDivHorz >= 2) || (mparam.nDivVert >= 2)) )
	{
		pTempMesh = new S3DMeshEditor ;
		pTempMesh->DuplicateMesh( m_mesh ) ;
		pMesh = pTempMesh ;
		//
		pTempMesh->RedivideMeshFrom( m_mesh, mparam ) ;
	}
	//
	size_t	nCtrls = GetControllerCount() ;
	size_t	iFirstCtrl = 0 ;
	for ( size_t i = 0; i < nCtrls; i ++ )
	{
		S3DSceneComposer::Controller *	pCtrl = GetControllerAt( nCtrls - i - 1 ) ;
		if ( (pCtrl == nullptr) || pCtrl->IsControllerDisabled() )
		{
			continue ;
		}
		S3DBakedMeshController *
				pMeshCtrl = ESLTypeCast<S3DBakedMeshController>( pCtrl ) ;
		if ( pMeshCtrl != nullptr )
		{
			iFirstCtrl = i ;
			break ;
		}
	}
	for ( size_t i = iFirstCtrl; i < nCtrls; i ++ )
	{
		S3DSceneComposer::Controller *	pCtrl = GetControllerAt( i ) ;
		if ( (pCtrl == nullptr) || pCtrl->IsControllerDisabled() )
		{
			continue ;
		}
		MeshController *	pMeshCtrl = ESLTypeCast<MeshController>( pCtrl ) ;
		if ( pMeshCtrl != nullptr )
		{
			if ( pMesh == &m_mesh )
			{
				pTempMesh = new S3DMeshEditor ;
				pTempMesh->DuplicateMesh( m_mesh ) ;
				pMesh = pTempMesh ;
			}
			pMesh = pMeshCtrl->ModifyMeshEditor( *pMesh, m_mesh ) ;
			m_flagLastMeshCtrled = true ;
			flagUpdateMesh = true ;
		}
		S3DMeshBufferItemSerializer::MeshController *
			pSubMesh = ESLTypeCast<S3DMeshBufferItemSerializer::MeshController>( pCtrl ) ;
		if ( pSubMesh != nullptr )
		{
			S3DRenderBuffer *	pTempBuf[paramMaterialCount] ;
			for ( size_t j = 0; j < paramMaterialCount; j ++ )
			{
				if ( pTempRenderBuf[j] == nullptr )
				{
					pTempRenderBuf[j] = new S3DRenderBuffer ;
				}
				pTempBuf[j] = pTempRenderBuf[j].Ptr() ;
			}
			if ( pMesh == &m_mesh )
			{
				pTempMesh = new S3DMeshEditor ;
				pTempMesh->DuplicateMesh( m_mesh ) ;
				pMesh = pTempMesh ;
			}
			BakeMeshBufferController( *pMesh, *pSubMesh, scene, pTempBuf ) ;
			m_flagLastMeshCtrled = true ;
			flagUpdateMesh = true ;
		}
	}
	return	flagUpdateMesh ;
}

bool S3DMeshEditorSerializer::ProcessMeshWithBone( S3DMeshEditor& mesh ) const
{
	S3DDMatrix	matMesh ;
	S3DDVector	vMesh ;
	GetGlobalTransformation( matMesh, vMesh ) ;
	//
	S3DDMatrix	matIMesh = matMesh.Inverse() ;
	S3DDVector	vIMesh = matIMesh * -vMesh ;
	//
	SPointerArray<const float32_t>	aPtrWeightMap ;
	SArray<size_t>					aWeightLayer ;
	SArray<S3DMatrix>				aBoneBaseIMatrix ;
	SArray<S3DVector>				aBoneBasePos ;
	SArray<S3DMatrix>				aBoneMatrix ;
	SArray<S3DVector>				aBonePos ;
	S3DSceneComposer::Composition *	pComp = nullptr ;
	//
	const size_t	nWeightLayerCount = mesh.GetWeightLayerCount() ;
	for ( size_t i = 0; i < nWeightLayerCount; i ++ )
	{
		const S3DMeshEditor::WeightMapInfo *
						pwmi = mesh.GetWeightLayerInfoAt( i ) ;
		ESLAssert( pwmi != nullptr ) ;
		if ( pwmi->nFlags & S3DMeshEditor::weightBone )
		{
			if ( pComp == nullptr )
			{
				pComp = GetComposition() ;
				if ( pComp == nullptr )
				{
					return	false ;
				}
			}
			S3DScene::Space *	pSpace =
				pComp->GetSceneSpaceAs( mesh.GetWeightLayerIDAt( i ) ) ;
			if ( pSpace != nullptr )
			{
				S3DDMatrix	matdBone ;
				S3DDVector	vdBone ;
				pSpace->CalcGlobalTransformation( matdBone, vdBone ) ;
				//
				aBoneBaseIMatrix.Add( pwmi->mat4Bone.GetMatrix3().Inverse() ) ;
				aBoneBasePos.Add( pwmi->mat4Bone.GetTranslation() ) ;
				aBoneMatrix.Add( S3DMatrix( matIMesh * matdBone ) ) ;
				aBonePos.Add( S3DVector( matIMesh * vdBone + vIMesh ) ) ;
				aWeightLayer.Add( i ) ;
			}
		}
	}
	if ( aWeightLayer.GetLength() == 0 )
	{
		return	false ;
	}
	const size_t		nBoneCount = aWeightLayer.GetLength() ;
	const size_t *		pWeightLayer = aWeightLayer.GetConstArray() ;
	const S3DMatrix *	pBoneBaseIMatrix = aBoneBaseIMatrix.GetConstArray() ;
	const S3DVector *	pBoneBasePos = aBoneBasePos.GetConstArray() ;
	const S3DMatrix *	pBoneMatrix = aBoneMatrix.GetConstArray() ;
	const S3DVector *	pBonePos = aBonePos.GetConstArray() ;
	const float32_t **	ppWeightMap = aPtrWeightMap.GetArray( nBoneCount ) ;
	ESLAssert( aBoneBaseIMatrix.GetLength() == nBoneCount ) ;
	ESLAssert( aBoneBasePos.GetLength() == nBoneCount ) ;
	ESLAssert( aBoneMatrix.GetLength() == nBoneCount ) ;
	ESLAssert( aBonePos.GetLength() == nBoneCount ) ;
	//
	for ( size_t iPatch = 0; iPatch < mesh.GetPatchCount(); iPatch ++ )
	{
		S3DMeshEditor::Patch *	pPatch = mesh.GetPatchAt( iPatch ) ;
		ESLAssert( pPatch != nullptr ) ;
		for ( size_t i = 0; i < nBoneCount; i ++ )
		{
			ppWeightMap[i] = pPatch->GetConstWeightArrayAt( pWeightLayer[i] ) ;
		}
		const size_t	nVertexCount = pPatch->GetTotalVertexCount() ;
		for ( size_t i = 0; i < nVertexCount; i ++ )
		{
			float32_t	wSum = 0.0f ;
			for ( size_t j = 0; j < nBoneCount; j ++ )
			{
				wSum += *(ppWeightMap[j] + i) ;
			}
			if ( wSum == 0.0f )
			{
				continue ;
			}
			S3DVector	vPos = pPatch->GetPointAt( i ) ;
			S3DVector	vMoved( 0, 0, 0 ) ;
			float32_t	fpRcpWeight = 1.0f / wSum ;
			for ( size_t j = 0; j < nBoneCount; j ++ )
			{
				float32_t	w = *(ppWeightMap[j] + i) ;
				if ( w == 0.0f )
				{
					continue ;
				}
				w *= fpRcpWeight ;
				vMoved += (pBoneMatrix[j]
							* (pBoneBaseIMatrix[j]
								* (vPos - pBoneBasePos[j])) + pBonePos[j]) * w ;
			}
			pPatch->SetPointAt( i, vMoved ) ;
		}
	}
	return	true ;
}

void S3DMeshEditorSerializer::BakeMeshBufferController
	( S3DMeshEditor& meshDst,
		S3DMeshBufferItemSerializer::MeshController& ctrl,
		S3DScene& scene,
		S3DRenderBuffer* pTempRenderBuf[paramMaterialCount] )
{
	S3DVertexBufferInterface *	pVB[paramMaterialCount] ;
	for ( size_t j = 0; j < paramMaterialCount; j ++ )
	{
		pVB[j] = pTempRenderBuf[j] ;
		pTempRenderBuf[j]->AttachDefaultMaterial( m_pMaterials[j] ) ;
		pTempRenderBuf[j]->ClearBuffer() ;
	}
	ctrl.AddMesh( scene, this, pVB, paramMaterialCount ) ;
	//
	for ( size_t j = 0; j < paramMaterialCount; j ++ )
	{
		if ( pTempRenderBuf[j]->GetMeshCount() > 0 )
		{
			S3DMeshEditor::Patch *	pPatch = meshDst.NewPatch() ;
			pPatch->ConvertFromVertexBuffer( *(pTempRenderBuf[j]) ) ;
			pPatch->SetMaterialIndex( j ) ;
			meshDst.AddPatch( pPatch ) ;
		}
	}
}

// ボーン有効
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditorSerializer::IsEnabledBone( void ) const
{
	return	m_flagEnableBone ;
}

bool S3DMeshEditorSerializer::IsValidBoneReference( void ) const
{
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp == nullptr )
	{
		return	false ;
	}
	for ( size_t i = 0; i < m_mesh.GetWeightLayerCount(); i ++ )
	{
		const S3DMeshEditor::WeightMapInfo *
					pwmi = m_mesh.GetWeightLayerInfoAt( i ) ;
		if ( (pwmi != nullptr)
			&& (pwmi->nFlags & S3DMeshEditor::weightBone) )
		{
			if ( pComp->GetSceneItemAs( m_mesh.GetWeightLayerIDAt( i ) ) != nullptr )
			{
				return	true ;
			}
		}
	}
	return	false ;
}

void S3DMeshEditorSerializer::EnableBone( bool flagBone )
{
	m_flagEnableBone = flagBone ;
}

// ボーンを列挙
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::EnumerateAllChildBones
	( SSystem::SArray<S3DMeshEditorSerializer::AutoBoneWeightInfo>& aBones,
		S3DSceneComposer::Composition * pComp, S3DBoneSerializer * pBone ) const
{
	AutoBoneWeightInfo	abwi ;
	abwi.pBone = pBone ;
	abwi.pwszID = pComp->GetSceneItemIDOf( pBone ) ;
	abwi.iLayer = -1 ;
	abwi.wTemp = 0.0f ;
	//
	if ( abwi.pwszID == nullptr )
	{
		return ;
	}
	S3DDMatrix	matMesh ;
	S3DDVector	vMesh ;
	CalcGlobalTransformation( matMesh, vMesh ) ;
	//
	S3DDMatrix	matIMesh = matMesh.Inverse() ;
	S3DDMatrix	matBone ;
	S3DDVector	vBone ;
	pBone->CalcGlobalTransformation( matBone, vBone ) ;
	//
	abwi.vBone = matIMesh * (vBone - vMesh) ;
	abwi.vHandleDir = matIMesh * matBone * pBone->GetBoneHandle() ;
	abwi.fpHandleLen = (float32_t) abwi.vHandleDir.Absolute() ;
	if ( abwi.fpHandleLen > 0.0f )
	{
		abwi.vHandleDir *= 1.0f / abwi.fpHandleLen ;
	}
	aBones.Add( abwi ) ;
	//
	for ( size_t i = 0; i < pBone->GetChildItemCount(); i ++ )
	{
		S3DBoneSerializer *	pChild =
			ESLTypeCast<S3DBoneSerializer>( pBone->GetChildItemAt( i ) ) ;
		if ( pChild != nullptr )
		{
			EnumerateAllChildBones( aBones, pComp, pChild ) ;
		}
	}
}

// ボーンウェイトマップを自動的に生成
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::DoAutoMapBoneWeightLayers
	( S3DSceneComposer::Composition * pComp, S3DBoneSerializer * pBoneRoot )
{
	S3DDMatrix	matMesh ;
	S3DDVector	vMesh ;
	CalcGlobalTransformation( matMesh, vMesh ) ;
	//
	S4DMatrix	mat4Mesh( 1, 1, 1, 1 ) ;
	mat4Mesh.SetMatrix3( S3DMatrix( matMesh ) ) ;
	mat4Mesh.SetTranslation( S3DVector( vMesh ) ) ;
	//
	S4DMatrix	mat4IMesh = mat4Mesh.Inverse() ;
	//
	//
	// ボーン列挙
	//
	SArray<AutoBoneWeightInfo>	aBones ;
	EnumerateAllChildBones( aBones, pComp, pBoneRoot ) ;
	//
	AutoBoneWeightInfo *	pBoneInfos = aBones.GetArray() ;
	const size_t			nBoneCount = aBones.GetLength() ;
	if ( nBoneCount == 0 )
	{
		return ;
	}
	//
	// 各頂点毎に処理
	//
	for ( size_t iPatch = 0; iPatch < m_mesh.GetPatchCount(); iPatch ++ )
	{
		S3DMeshEditor::Patch *	pPatch = m_mesh.GetPatchAt( iPatch ) ;
		ESLAssert( pPatch != nullptr ) ;
		//
		const size_t	nVertexCount = pPatch->GetTotalVertexCount() ;
		for ( size_t i = 0; i < nVertexCount; i ++ )
		{
			S3DVector	vPos = pPatch->GetPointAt( i ) ;
			//
			for ( size_t j = 0; j < nBoneCount; j ++ )
			{
				AutoBoneWeightInfo&	abwi = pBoneInfos[j] ;
				S3DVector	vDelta = vPos - abwi.vBone ;
				float32_t	t = vDelta.InnerProduct( abwi.vHandleDir ) ;
				if ( t > 0.0f )
				{
					if ( t < abwi.fpHandleLen )
					{
						vDelta -= abwi.vHandleDir * t ;
					}
					else
					{
						vDelta -= abwi.vHandleDir * abwi.fpHandleLen ;
					}
				}
				abwi.wTemp = (float32_t) vDelta.Absolute() ;
			}
			float32_t	wMin = pBoneInfos[0].wTemp ;
			for ( size_t j = 1; j < nBoneCount; j ++ )
			{
				wMin = esl_fminf( wMin, pBoneInfos[j].wTemp ) ;
			}
			float32_t	wRcp4 = 4.0f / wMin ;
			float32_t	wSum = 0.0f ;
			for ( size_t j = 0; j < nBoneCount; j ++ )
			{
				if ( pBoneInfos[j].wTemp <= wMin )
				{
					pBoneInfos[j].wTemp = 1.0f ;
				}
				else if ( pBoneInfos[j].wTemp < wMin * 1.25f )
				{
					pBoneInfos[j].wTemp =
							1.0f - (pBoneInfos[j].wTemp * wRcp4 - 4.0f) ;
				}
				else
				{
					pBoneInfos[j].wTemp = 0.0f ;
				}
				wSum += pBoneInfos[j].wTemp ;
			}
			float32_t	wRcpSum = 1.0f / wSum ;
			for ( size_t j = 0; j < nBoneCount; j ++ )
			{
				AutoBoneWeightInfo&	abwi = pBoneInfos[j] ;
				if ( abwi.wTemp == 0.0f )
				{
					continue ;
				}
				if ( abwi.iLayer < 0 )
				{
					abwi.iLayer = (ssize_t) m_mesh.GetWeightLayerCount() ;
					m_mesh.InsertWeightLayerAt( abwi.iLayer, abwi.pwszID ) ;
					//
					S3DDMatrix	matBone ;
					S3DDVector	vBone ;
					abwi.pBone->CalcGlobalTransformation( matBone, vBone ) ;
					//
					S4DMatrix	mat4Bone( 1, 1, 1, 1 ) ;
					mat4Bone.SetMatrix3( S3DMatrix( matBone ) ) ;
					mat4Bone.SetTranslation( S3DVector( vBone ) ) ;
					//
					S3DMeshEditor::WeightMapInfo	wmi ;
					wmi.nFlags = S3DMeshEditor::weightBone ;
					wmi.mat4Bone = mat4IMesh * mat4Bone ;
					m_mesh.SetWeightLayerInfoAt( abwi.iLayer, wmi ) ;
				}
				pPatch->SetWeightAt( i, abwi.iLayer, abwi.wTemp * wRcpSum ) ;
			}
		}
	}
}

// ボーン用ウェイトマップを削除
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::DoClearBoneWeightLayers( void )
{
	for ( size_t i = 0; i < m_mesh.GetWeightLayerCount(); i ++ )
	{
		const S3DMeshEditor::WeightMapInfo *
						pwmi = m_mesh.GetWeightLayerInfoAt( i ) ;
		ESLAssert( pwmi != nullptr ) ;
		if ( pwmi->nFlags & S3DMeshEditor::weightBone )
		{
			m_mesh.RemoveWeightLayer( i --, 1 ) ;
		}
	}
}

// ボーン用ウェイトマップの行列を設定
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::DoInitBoneMatrix( S3DSceneComposer::Composition * pComp )
{
	S3DDMatrix	matMesh ;
	S3DDVector	vMesh ;
	CalcGlobalTransformation( matMesh, vMesh ) ;
	//
	S4DMatrix	mat4Mesh( 1, 1, 1, 1 ) ;
	mat4Mesh.SetMatrix3( S3DMatrix( matMesh ) ) ;
	mat4Mesh.SetTranslation( S3DVector( vMesh ) ) ;
	//
	S4DMatrix	mat4IMesh = mat4Mesh.Inverse() ;
	//
	for ( size_t i = 0; i < m_mesh.GetWeightLayerCount(); i ++ )
	{
		const S3DMeshEditor::WeightMapInfo *
						pwmi = m_mesh.GetWeightLayerInfoAt( i ) ;
		ESLAssert( pwmi != nullptr ) ;
		if ( pwmi->nFlags & S3DMeshEditor::weightBone )
		{
			S3DScene::Space *	pSpace =
				pComp->GetSceneSpaceAs( m_mesh.GetWeightLayerIDAt( i ) ) ;
			if ( pSpace != nullptr )
			{
				S3DDMatrix	matBone ;
				S3DDVector	vBone ;
				pSpace->CalcGlobalTransformation( matBone, vBone ) ;
				//
				S4DMatrix	mat4Bone( 1, 1, 1, 1 ) ;
				mat4Bone.SetMatrix3( S3DMatrix( matBone ) ) ;
				mat4Bone.SetTranslation( S3DVector( vBone ) ) ;
				//
				S3DMeshEditor::WeightMapInfo	wmi = *pwmi ;
				wmi.mat4Bone = mat4IMesh * mat4Bone ;
				m_mesh.SetWeightLayerInfoAt( i, wmi ) ;
			}
		}
	}
}

// ボーン用ウェイトマップを正規化
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::DoNormalizeBoneWeightMap( void )
{
	m_mesh.NormalizeBoneWeightMap() ;
}

// ボーン用ウェイトマップを最適化
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::DoOptimizeBoneWeightMap( void )
{
	m_mesh.OptimizeBoneWeightMap() ;
}

// インスタンスをメッシュにベイク
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::DoBakeInstanceToMeshEditor( void )
{
	const size_t	nPatchCount = GetPatchCount() ;
	const size_t	nInstancCount = m_instancing.GetStaticInstanceCount() ;
	S4DMatrix		matrix ;
	S3DColor		color ;
	for ( size_t i = 1; i < nInstancCount; i ++ )
	{
		if ( m_instancing.GetStaticInstanceAt( i, matrix, color ) )
		{
			for ( size_t j = 0; j < nPatchCount; j ++ )
			{
				S3DMeshEditor::Patch *	pPatch0 = GetPatchAt( j ) ;
				ESLAssert( pPatch0 != nullptr ) ;
				S3DMeshEditor::Patch *	pPatch = NewPatch() ;
				pPatch->CopyFrom( *pPatch0 ) ;
				pPatch->SetName( pPatch0->GetName() + SString(i) ) ;
				DoMakeMeshEditorPatchInstance( *pPatch, matrix, color ) ;
				AddPatch( pPatch ) ;
			}
		}
	}
	if ( m_instancing.GetStaticInstanceAt( 0, matrix, color ) )
	{
		for ( size_t j = 0; j < nPatchCount; j ++ )
		{
			S3DMeshEditor::Patch *	pPatch = GetPatchAt( j ) ;
			ESLAssert( pPatch != nullptr ) ;
			DoMakeMeshEditorPatchInstance( *pPatch, matrix, color ) ;
			//
			S3DMeshEditor::SelectPointSet	sps ;
			const size_t	nVertexCount = pPatch->GetTotalVertexCount() ;
			for ( size_t i = 0; i < nVertexCount; i ++ )
			{
				sps.QuickAdd( S3DMeshEditor::SelectPoint( pPatch, i ) ) ;
			}
			sps.SortArray() ;
			NotifyUpdateVertex( sps ) ;
		}
	}
	//
	// インスタンスをリセット
	//
	for ( size_t i = 0; i < nInstancCount; i ++ )
	{
		m_instancing.RemoveStaticInstanceAt( nInstancCount - i - 1, this ) ;
	}
	ESLAssert( m_instancing.GetStaticInstanceCount() == 0 ) ;
	//
	S4DMatrix	mat4Instance( 1, 1, 1, 1 ) ;
	S3DColor	clrInstance( 0xFFFFFFFF, 0 ) ;
	m_instancing.InsertStaticInstanceAt( 0, mat4Instance, clrInstance, this ) ;
	m_instancing.UpdateStaticInstancingEntriesBase64() ;
}

void S3DMeshEditorSerializer::DoMakeMeshEditorPatchInstance
	( S3DMeshEditor::Patch& patch,
		const S4DMatrix& matrix, const S3DColor& color )
{
	const size_t	nVertexCount = patch.GetTotalVertexCount() ;
	S3DMatrix		mat3 = matrix.GetMatrix3() ;
	S3DVector		vt = matrix.GetTranslation() ;
	for ( size_t i = 0; i < nVertexCount; i ++ )
	{
		patch.SetPointAt( i, mat3 * patch.GetPointAt(i) + vt ) ;
		patch.SetNormalAt( i, (mat3 * patch.GetNormalAt(i)).Normalized() ) ;
		patch.SetColorAt( i, color * patch.GetColorAt(i) ) ;
	}
}

// コンポジションをモデルファイルに変換する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMeshEditorSerializer::ConvertModelFromComposition
	( S3DModelBuffer& model,
		S3DSceneComposer::Composition & comp,
		S3DSceneComposer::SpaceSerializer & spaceRoot,
		const S3DMeshEditorSerializer::ConvertModelParam& param )
{
	const S3DSceneComposer::CompositionInfo *
						pcmpInf = comp.GetCompositionInfo() ;
	ESLAssert( pcmpInf != nullptr ) ;
	//
	// シーンのアニメーショントラック
	//
	SSmartPointer<S3DModelPose>	pTempPose ;
	S3DModelPose *				pPose = new S3DModelPose ;
	S3DModelPose::MetaInfo		metaInf = pPose->GetMetaInfo() ;
	metaInf.nFlags = 0 ;
	metaInf.msecDuration =
		(uint32_t) esl_lroundfi
			( pcmpInf->FrameIndexToSecond
				( (double) pcmpInf->GetTotalFrameCount() ) *1000.0 ) ;
	metaInf.fxFrameRatio =
		(uint32_t) esl_lroundfi
			( pcmpInf->FrameIndexFromSecond( 0x10000 ) ) ;
	//
	pPose->SetMetaInfo( metaInf ) ;
	//
	if ( param.nFlags & cvtFlagWithoutAnimation )
	{
		pTempPose = pPose ;
	}
	else
	{
		model.GetPoseLibrary().AddPoseAs( param.pwszCompID, pPose ) ;
	}
	//
	// ボーン変換
	//
	BoneInfoSortMap	mapBoneInfos ;
	if ( !(param.nFlags & cvtFlagWithoutBone) )
	{
		CreateBoneFromComposition
			( model, *pPose, mapBoneInfos,
				comp, spaceRoot, false, param.matBase, param.vBase ) ;
		BuildBoneByInfoMap( model, mapBoneInfos, comp, param.vBase ) ;
	}
	//
	// マテリアル変換
	//
	TextureInfoSortMap	mapTexInfos ;
	if ( param.nFlags & cvtFlagAllMaterials )
	{
		ConvertAllMaterials( model, comp, mapTexInfos, param ) ;
	}
	//
	// 各アイテム変換
	//
	MeshGroupArray		mapMeshGroupArrays ;
	SGLError	err =
		ConvertCompositionItems
			( model, comp, mapBoneInfos,
				mapTexInfos, mapMeshGroupArrays,
				param.matBase, param.vBase, spaceRoot, param ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// モーフィング・ボーン構築
	//
	model.RebuildVertexBuffer() ;
	//
	// マテリアルの統合
	//
	if ( param.nFlags & cvtFlagMergeMaterialByName )
	{
		MergeMaterialByName( model ) ;
	}
	//
	// メッシュの統合
	//
	if ( param.nFlags & cvtFlagMergeMaterials )
	{
		MergeMeshAllMaterials( model ) ;
	}
	else if ( param.nFlags & cvtFlagMergeMeshs )
	{
		MergeMeshEachMeshGroups( model, mapMeshGroupArrays ) ;
	}
	if ( param.nFlags & cvtFlagMergeMeshByName )
	{
		MergeMeshByName( model ) ;
	}
	//
	// 元コンポジション・シリアライズ
	//
	if ( param.nFlags & cvtFlagSerializeComposition )
	{
		const SSystem::SString *	pstrSceneID =
			comp.GetComposer()->GetCompositionID
				( (size_t) comp.GetComposer()->FindCompositionOf
										( comp.GetCompositionInfo() ) ) ;
		//
		S3DSceneComposer	compScene( nullptr ) ;
		S3DSceneComposer::CompositionInfo *	pCompInfo =
			new S3DSceneComposer::CompositionInfo( *(comp.GetCompositionInfo()) ) ;
		//
		comp.FormatItem( compScene, pCompInfo->EditComposition(), 0 ) ;
		compScene.AddCompositionAs
			( ((pstrSceneID != nullptr) ? *pstrSceneID : L"scene"), pCompInfo ) ;
		//
		SXMLDocument&	xmlScene = model.EditSceneComposition() ;
		xmlScene.RemoveAllContents() ;
		compScene.FormatComposeFile( xmlScene ) ;
		//
		xmlScene.SetAttributeAs( L"mesh_editor", L"true" ) ;
	}
	return	sglErrSuccess ;
}

// マテリアル統合
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::MergeMaterialByName( S3DModelBuffer& model )
{
	S3DMaterialLibrary&	libMaterial = model.GetMaterialLibrary() ;
	for ( size_t i = 0; i < libMaterial.GetMaterialCount(); i ++ )
	{
		S3DMaterial *	pMaterial = libMaterial.GetMaterialAt( i ) ;
		const wchar_t *	pwszMaterialID = libMaterial.GetMaterialIdentityAt( i ) ;
		if ( (pMaterial == nullptr)
			|| (pwszMaterialID == nullptr) || (pwszMaterialID[0] == 0) )
		{
			continue ;
		}
		SString	strMaterialID = pwszMaterialID ;
		ssize_t	iSep = strMaterialID.Find( L'@' ) ;
		if ( iSep < 0 )
		{
			continue ;
		}
		SPointerArray<S3DMaterial>	aMaterials ;
		aMaterials.Add( pMaterial ) ;
		//
		SString	strOrgName = strMaterialID ;
		SString	strMergeName = strMaterialID.Middle( (size_t) iSep + 1 ) ;
		for ( size_t j = i + 1; j < libMaterial.GetMaterialCount(); j ++ )
		{
			pMaterial = libMaterial.GetMaterialAt( j ) ;
			strMaterialID = libMaterial.GetMaterialIdentityAt( j ) ;
			if ( (pMaterial == nullptr) || strMaterialID.IsEmpty() )
			{
				continue ;
			}
			iSep = strMaterialID.Find( L'@' ) ;
			if ( iSep < 0 )
			{
				continue ;
			}
			if ( strMaterialID.Middle( (size_t) iSep + 1 ) == strMergeName )
			{
				aMaterials.Add( pMaterial ) ;
			}
		}
		if ( aMaterials.GetLength() >= 2 )
		{
			model.MergeMaterials
				( aMaterials.GetConstArray(), aMaterials.GetLength(),
					strMergeName + L"_", strMergeName + L"_back_" ) ;
		}
		//
		SString	strMergedName = strMergeName ;
		size_t	iOptNum = 1 ;
		while ( libMaterial.GetMaterialAs( strMergedName ) != nullptr )
		{
			strMergedName = strMergeName + L"." + SString( iOptNum ++ ) ;
		}
		libMaterial.RenameMaterialAs( aMaterials.GetAt(0), strMergedName ) ;
		i -- ;
	}
	//
	// 使用されていないテクスチャを削除する
	//
	S3DTextureLibrary&	libTexture = model.GetTextureLibrary() ;
	for ( size_t i = 0; i < libTexture.GetTextureCount(); i ++ )
	{
		SGLImageObject *	pImage = libTexture.GetTextureAt( i ) ;
		if ( pImage == nullptr )
		{
			continue ;
		}
		bool	flagRefTexture = false ;
		for ( size_t j = 0; j < libMaterial.GetMaterialCount(); j ++ )
		{
			S3DMaterial *	pMaterial = libMaterial.GetMaterialAt( j ) ;
			if ( (pMaterial != nullptr)
				&& ((pMaterial->FindTextureOf( pImage ) >= 0)
					|| (pMaterial->FindBackTextureOf( pImage ) >= 0)) )
			{
				flagRefTexture = true ;
				break ;
			}
		}
		if ( !flagRefTexture )
		{
			libTexture.RemoveTextureAs( libTexture.GetTextureIdentityAt(i) ) ;
			i -- ;
		}
	}
}

// メッシュ統合
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::MergeMeshAllMaterials( S3DModelBuffer& model )
{
	S3DMaterialLibrary&	libMaterial = model.GetMaterialLibrary() ;
	for ( size_t i = 0; i < libMaterial.GetMaterialCount(); i ++ )
	{
		S3DMaterial *	pMaterial = libMaterial.GetMaterialAt( i ) ;
		if ( pMaterial != nullptr )
		{
			model.MergeMeshMaterialOf( pMaterial ) ;
		}
	}
}

void S3DMeshEditorSerializer::MergeMeshEachMeshGroups
	( S3DModelBuffer& model,
		const S3DMeshEditorSerializer::MeshGroupArray& mapMeshGroupArrays )
{
	for ( size_t i = 0; i < mapMeshGroupArrays.GetLength(); i ++ )
	{
		SPointerArray<S3DModelData::MeshGroup> *
						pMeshGroup = mapMeshGroupArrays.GetAt( i ) ;
		if ( (pMeshGroup == nullptr) || (pMeshGroup->GetLength() <= 1) )
		{
			continue ;
		}
		SArraySet<size_t>	aMeshs ;
		for ( size_t j = 0; j < pMeshGroup-> GetLength(); j ++ )
		{
			S3DModelData::MeshGroup *	pmg = pMeshGroup->GetAt( j ) ;
			ESLAssert( pmg != nullptr ) ;
			if ( pmg != nullptr )
			{
				for ( size_t k = 0; k < pmg->m_nMeshCount; k ++ )
				{
					aMeshs.AddSorted( pmg->m_iFirstMesh + k ) ;
				}
			}
		}
		if ( aMeshs.GetLength() > 1 )
		{
			model.MergeMeshs( aMeshs.GetConstArray(), aMeshs.GetLength() ) ;
		}
	}
	SStrSortArray<S3DModelBuffer::MeshGroup>	ssaTemp ;
	SStrSortArray<S3DModelBuffer::MeshGroup>&
						ssaMeshGroups = model.GetMeshGroupList() ;
	for ( size_t i = 0; i < ssaMeshGroups.GetLength(); i ++ )
	{
		const SString *				pstrID = ssaMeshGroups.GetTagAt(i) ;
		S3DModelBuffer::MeshGroup *	pmgMeshs = ssaMeshGroups.GetAt(i) ;
		if ( (pstrID != nullptr) && (pmgMeshs != nullptr) )
		{
			SString	strMeshID = pstrID->GetFileTitlePart() ;
			if ( (ssaMeshGroups.GetAs( strMeshID ) != nullptr)
				|| (ssaTemp.GetAs( strMeshID ) != nullptr) )
			{
				strMeshID = *pstrID ;
			}
			ssaTemp.Add( strMeshID, *pmgMeshs ) ;
		}
	}
	ssaMeshGroups.RemoveAll() ;
	//
	for ( size_t i = 0; i < ssaTemp.GetLength(); i ++ )
	{
		const SString *				pstrID = ssaTemp.GetTagAt(i) ;
		S3DModelBuffer::MeshGroup *	pmgMeshs = ssaTemp.GetAt(i) ;
		if ( (pstrID != nullptr) && (pmgMeshs != nullptr) )
		{
			ssaMeshGroups.Add( *pstrID, *pmgMeshs ) ;
		}
	}
}

void S3DMeshEditorSerializer::MergeMeshByName( S3DModelBuffer& model )
{
	for ( size_t i = 0; i < model.GetMeshCount(); i ++ )
	{
		const SSystem::SString *	pstrMeshID = model.GetMeshIdentityAt( i ) ;
		if ( pstrMeshID == nullptr )
		{
			continue ;
		}
		ssize_t	iSep = pstrMeshID->Find( L'@' ) ;
		if ( iSep < 0 )
		{
			continue ;
		}
		SArraySet<size_t>	aMeshs ;
		aMeshs.Add( i ) ;
		//
		SString	strOrgName = *pstrMeshID ;
		SString	strMergeName = pstrMeshID->Middle( (size_t) iSep + 1 ) ;
		for ( size_t j = i + 1; j < model.GetMeshCount(); j ++ )
		{
			pstrMeshID = model.GetMeshIdentityAt( j ) ;
			if ( pstrMeshID == nullptr )
			{
				continue ;
			}
			iSep = pstrMeshID->Find( L'@' ) ;
			if ( iSep < 0 )
			{
				continue ;
			}
			if ( pstrMeshID->Middle( (size_t) iSep + 1 ) == strMergeName )
			{
				aMeshs.Add( j ) ;
			}
		}
		if ( aMeshs.GetLength() >= 2 )
		{
			model.MergeMeshs( aMeshs.GetConstArray(), aMeshs.GetLength() ) ;
		}
		//
		SString	strMergedName = strMergeName ;
		size_t	iOptNum = 1 ;
		while ( model.GetMeshGroupAs( strMergedName ) != nullptr )
		{
			strMergedName = strMergeName + L"." + SString( iOptNum ++ ) ;
		}
		model.ModifyMeshIdentity( strOrgName, strMergedName ) ;
	}
}

// ボーン構築
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DMeshEditorSerializer::CreateBoneFromComposition
	( S3DModelBuffer& model, S3DModelPose& pose,
		S3DMeshEditorSerializer::BoneInfoSortMap& mapBoneInfos,
		S3DSceneComposer::Composition & comp,
		S3DSceneComposer::SpaceSerializer & space, bool flagBoneSpace,
		const S3DDMatrix& matBase, const S3DDVector& vBase )
{
	uint32_t	nSubItemFlags = 0 ;
	for ( size_t i = 0; i < space.GetChildrenCount(); i ++ )
	{
		S3DSceneComposer::SpaceSerializer *	pChild =
			ESLTypeCast<S3DSceneComposer::SpaceSerializer>( space.GetChildAt( i ) ) ;
		if ( pChild != nullptr )
		{
			nSubItemFlags |=
				CreateBoneFromComposition
					( model, pose, mapBoneInfos, comp, *pChild, true, matBase, vBase ) ;
		}
	}
	for ( size_t i = 0; i < space.GetItemCount(); i ++ )
	{
		S3DScene::Item *	pItem = space.GetItemAt( i ) ;
		S3DMeshEditorSerializer *
			pMeshEditor = ESLTypeCast<S3DMeshEditorSerializer>( pItem ) ;
		S3DSceneComposer::CommonSerializer *
			pCmnSer = ESLTypeCast<S3DSceneComposer::CommonSerializer>( pItem ) ;
		if ( (pCmnSer != nullptr)
			&& ((pCmnSer->GetParameterSequencer
					(S3DSceneComposer::CommonSerializer::paramPosition) != nullptr)
				|| (pCmnSer->GetParameterSequencer
					(S3DSceneComposer::CommonSerializer::paramRotation) != nullptr)
				|| (pCmnSer->GetParameterSequencer
					(S3DSceneComposer::CommonSerializer::paramZoom) != nullptr)) )
		{
			if ( (pMeshEditor == nullptr)
				|| !pMeshEditor->IsEnabledBone()
				|| !pMeshEditor->IsValidBoneReference() )
			{
				SSmartPointer<BuildBoneInfo>	pBoneInfo = new BuildBoneInfo ;
				pBoneInfo->pBone = new S3DModelBoneSpace ;
				pBoneInfo->pItem = pCmnSer ;
				//
				S3DModelPose::JointAnimation *	pJoint =
					ConvertBoneTimeline
						( comp, *(pBoneInfo->pBone),
							pCmnSer->GetParameterSequencer
								(S3DSceneComposer::CommonSerializer::paramPosition),
							pCmnSer->GetParameterSequencer
								(S3DSceneComposer::CommonSerializer::paramRotation),
							pCmnSer->GetParameterSequencer
								(S3DSceneComposer::CommonSerializer::paramZoom), nullptr ) ;
				if ( pJoint != nullptr )
				{
					mapBoneInfos.Add( pCmnSer, pBoneInfo.Detach() ) ;
					//
					pBoneInfo->pBone->SetLocalSpacePosition
							( pCmnSer->GetSpaceInfo().m_vCenter ) ;
					//
					const wchar_t *	pwszID = comp.GetSceneItemIDOf( pCmnSer ) ;
					pose.AddJointAs( pwszID, pJoint ) ;
					model.AddBonePropertyAs( pwszID, pBoneInfo->pBone ) ;
				}
			}
		}
		if ( (pMeshEditor != nullptr)
			|| (ESLTypeCast<S3DMeshBufferItemSerializer>( pItem ) != nullptr)
			|| (ESLTypeCast<S3DIndirectMeshBuilderSerializer>( pItem ) != nullptr)
			|| (ESLTypeCast<S3DMultiInstanceSerializer>( pItem ) != nullptr) )
		{
			if ( pCmnSer != nullptr )
			{
				// メッシュの表示／非表示アニメーショントラック
				S3DSceneComposer::Sequencer *	pSeq =
					pCmnSer->GetParameterSequencer
						( S3DSceneComposer::CommonSerializer::paramVisible ) ;
				if ( pSeq != nullptr )
				{
					const wchar_t *	pwszID = comp.GetSceneItemIDOf( pCmnSer ) ;
					if ( pwszID != nullptr )
					{
						pose.AddMeshSelectorAs
							( pwszID, ConvertMeshSelectorTimeline( comp, pSeq ) ) ;
					}
				}
			}
			if ( (pMeshEditor == nullptr)
				|| !pMeshEditor->IsEnabledBone()
				|| !pMeshEditor->IsValidBoneReference() )
			{
				nSubItemFlags |= cvtItemFlagMesh ;
			}
		}
	}
	S3DBoneSerializer *	pBone = ESLTypeCast<S3DBoneSerializer>( &space ) ;
	const bool	flagAnimation =
				(space.GetParameterSequencer
					(S3DSceneComposer::CommonSerializer::paramPosition) != nullptr)
				|| (space.GetParameterSequencer
						(S3DSceneComposer::CommonSerializer::paramRotation) != nullptr)
				|| (space.GetParameterSequencer
						(S3DSceneComposer::CommonSerializer::paramZoom) != nullptr) ;
	if ( flagBoneSpace
		&& ((pBone != nullptr)
			|| flagAnimation
			|| (nSubItemFlags & (cvtItemFlagBone | cvtItemFlagMesh))) )
	{
		BuildBoneInfo *	pBoneInfo = new BuildBoneInfo ;
		pBoneInfo->pBone = new S3DModelBoneSpace ;
		pBoneInfo->pItem = &space ;
		mapBoneInfos.Add( pBoneInfo->pItem, pBoneInfo ) ;
		//
		S3DDMatrix	matLink ;
		S3DDVector	vLinkPos ;
		space.GetItemLinkTransformation( matLink, vLinkPos ) ;
		//
		S3DDMatrix	matBone ;
		S3DDVector	vBonePos ;
		space.GetGlobalTransformation( matBone, vBonePos ) ;
		//
		matLink = matBase * matLink ;
		vLinkPos = matBase * vLinkPos + vBase ;
		matBone = matBase * matBone ;
		vBonePos = matBase * vBonePos + vBase ;
		//
		pBoneInfo->pBone->SetLocalSpacePosition( vBonePos - vLinkPos ) ;
		if ( pBone != nullptr )
		{
			uint32_t	nBoneFlags = 0 ;
			if ( pBone->IsEnabledPhysics() )
			{
				nBoneFlags |= S3DModelBoneSpace::flagBonePhysics ;
			}
			if ( pBone->IsDisabledCollision() )
			{
				nBoneFlags |= S3DModelBoneSpace::flagNoCollision ;
			}
			S3DModelBoneSpace::PhysMaterial	physParam = pBone->GetPhysMaterial() ;
			if ( physParam.nPhysExFlags1 & S3DModelBoneSpace::flagPhysExColliderAll )
			{
				physParam.nPhysExFlags1 |= S3DModelBoneSpace::flagPhysExColliderUseMask ;
			}
			//
			S3DModelBoneSpace::IKParameter	ikparam = pBoneInfo->pBone->GetIKParameter() ;
			if ( pBone->IsEnabledBendDir()
				&& (pBone->GetBendDir().Absolute() > 1.0e-7) )
			{
				ikparam.nIKFlags |= S3DModelBoneSpace::flagIKBendDirection ;
			}
			if ( pBone->GetBendParentAxisFlag()
				&& (pBone->GetBendDir().Absolute() > 1.0e-7) )
			{
				ikparam.nIKFlags |= S3DModelBoneSpace::flagIKParentAxis ;
			}
			if ( pBone->GetIKTerminateFlag() )
			{
				ikparam.nIKFlags |= S3DModelBoneSpace::flagIKTerminate ;
			}
			ikparam.nIKFlags |= S3DModelBoneSpace::flagIKMaxBent ;
			ikparam.fpWeight = (float32_t) pBone->GetBendWeight() ;
			ikparam.degMaxBent = (float32_t) pBone->GetBendMaxAngle() ;
			ikparam.vBendDirection = matLink * pBone->GetBendDir() ;
			//
			pBoneInfo->pBone->SetBoneFlags( nBoneFlags ) ;
			pBoneInfo->pBone->SetBoneHandle( matBone * pBone->GetBoneHandle() ) ;
			pBoneInfo->pBone->SetBonePhysicalMaterial( physParam ) ;
			pBoneInfo->pBone->SetBonePhysicalMaterialID( pBone->GetPhysMaterialRef() ) ;
			pBoneInfo->pBone->SetOriginalBoneMatrix( pBone->GetOrgMatrix() ) ;
			pBoneInfo->pBone->SetIKParameter( ikparam ) ;
		}
		const wchar_t *	pwszID = comp.GetSceneItemIDOf( pBoneInfo->pItem ) ;
		model.AddBonePropertyAs( pwszID, pBoneInfo->pBone ) ;
		//
		S3DModelPose::JointAnimation *	pJoint =
			ConvertBoneTimeline
				( comp, *(pBoneInfo->pBone),
					space.GetParameterSequencer
						(S3DSceneComposer::CommonSerializer::paramPosition),
					space.GetParameterSequencer
						(S3DSceneComposer::CommonSerializer::paramRotation),
					space.GetParameterSequencer
						(S3DSceneComposer::CommonSerializer::paramZoom),
					((pBone == nullptr) ? nullptr
						: pBone->GetParameterSequencer
								( S3DBoneSerializer::paramPoseWeight )) ) ;
		if ( pJoint != nullptr )
		{
			if ( pBone != nullptr )
			{
				pJoint->m_wPhysBlend = pBone->GetPoseWeight() ;
			}
			pose.AddJointAs( pwszID, pJoint ) ;
		}
		nSubItemFlags |= cvtItemFlagBone ;
		if ( flagAnimation )
		{
			nSubItemFlags |= cvtItemFlagAnimation ;
		}
	}
	return	nSubItemFlags ;
}

void S3DMeshEditorSerializer::BuildBoneByInfoMap
	( S3DModelBuffer& model,
		S3DMeshEditorSerializer::BoneInfoSortMap& mapBoneInfos,
		S3DSceneComposer::Composition & comp, const S3DDVector& vBase )
{
	for ( size_t i = 0; i < mapBoneInfos.GetLength(); i ++ )
	{
		BuildBoneInfo *	pBoneInfo = mapBoneInfos.GetAt( i ) ;
		ESLAssert( pBoneInfo != nullptr ) ;
		//
		pBoneInfo->pBone->AttachModel( &model ) ;
		//
		S3DModelBoneSpace *	pParentBone =
			GetParentBoneByInfoMap
				( mapBoneInfos, comp, pBoneInfo->pItem->GetParentSpaceItem() ) ;
		if ( pParentBone != nullptr )
		{
			pParentBone->AddChild( pBoneInfo->pBone ) ;
		}
		else
		{
			S3DDVector	vBonePos ;
			pBoneInfo->pBone->GetLocalSpacePosition( vBonePos ) ;
			vBonePos += vBase ;
			pBoneInfo->pBone->SetLocalSpacePosition( vBonePos ) ;
			model.GetBoneRoot().AddChild( pBoneInfo->pBone ) ;
		}
	}
}

S3DModelBoneSpace * S3DMeshEditorSerializer::GetParentBoneByInfoMap
	( S3DMeshEditorSerializer::BoneInfoSortMap& mapBoneInfos,
		S3DSceneComposer::Composition & comp,
		S3DSceneComposer::ItemSerializer * pItem )
{
	if ( pItem == nullptr )
	{
		return	nullptr ;
	}
	while ( (pItem != nullptr) && (pItem != &comp) )
	{
		BuildBoneInfo *	pbbiParent = mapBoneInfos.GetAs( pItem ) ;
		if ( pbbiParent != nullptr )
		{
			return	pbbiParent->pBone ;
		}
		pItem = pItem->GetParentSpaceItem() ;
	}
	return	nullptr ;
}

S3DMeshEditorSerializer::BuildBoneInfo *
	S3DMeshEditorSerializer::GetParentBuildBoneByInfoMap
		( S3DMeshEditorSerializer::BoneInfoSortMap& mapBoneInfos,
			S3DSceneComposer::Composition & comp,
			S3DSceneComposer::ItemSerializer * pItem )
{
	if ( pItem == nullptr )
	{
		return	nullptr ;
	}
	while ( (pItem != nullptr) && (pItem != &comp) )
	{
		BuildBoneInfo *	pbbiParent = mapBoneInfos.GetAs( pItem ) ;
		if ( pbbiParent != nullptr )
		{
			return	pbbiParent ;
		}
		pItem = pItem->GetParentSpaceItem() ;
	}
	return	nullptr ;
}

S3DModelPose::MeshSelector *
		S3DMeshEditorSerializer::ConvertMeshSelectorTimeline
	( S3DSceneComposer::Composition & comp, S3DSceneComposer::Sequencer * pSeq )
{
	const S3DSceneComposer::CompositionInfo *
						pcmpInf = comp.GetCompositionInfo() ;
	ESLAssert( pcmpInf != nullptr ) ;
	const size_t	nTotalFrame = (size_t) pcmpInf->GetTotalFrameCount() ;
	//
	S3DModelPose::MeshSelector *	pmsel = new S3DModelPose::MeshSelector ;
	uint8_t *	pVisibles = pmsel->m_aVisibles.GetArray( nTotalFrame ) ;
	for ( size_t i = 0; i < nTotalFrame; i ++ )
	{
		pVisibles[i] = pSeq->GetFrameBoolean( (double) i ) ;
	}
	pmsel->m_aVisibles.FinishArray() ;
	return	pmsel ;
}

S3DModelPose::JointAnimation *
	S3DMeshEditorSerializer::ConvertBoneTimeline
		( S3DSceneComposer::Composition & comp,
			S3DModelBoneSpace & bone,
			S3DSceneComposer::Sequencer * pSeqPos,
			S3DSceneComposer::Sequencer * pSeqRot,
			S3DSceneComposer::Sequencer * pSeqZoom,
			S3DSceneComposer::Sequencer * pSeqBlend )
{
	if ( (pSeqPos == nullptr) && (pSeqRot == nullptr) && (pSeqZoom == nullptr) )
	{
		return	nullptr ;
	}
	const S3DSceneComposer::CompositionInfo *
						pcmpInf = comp.GetCompositionInfo() ;
	ESLAssert( pcmpInf != nullptr ) ;
	const size_t	nTotalFrame = (size_t) pcmpInf->GetTotalFrameCount() ;
	//
	S3DModelPose::JointAnimation *	pJoint = new S3DModelPose::JointAnimation ;
	S4DMatrix *	pMatrixs = pJoint->m_aMatrixs.GetArray( nTotalFrame ) ;
	float32_t *	pPhysBlend = nullptr ;
	if ( pSeqBlend != nullptr )
	{
		pPhysBlend = pJoint->m_aPhysBlends.GetArray( nTotalFrame ) ;
	}
	S3DDVector	vBonePos ;
	S3DVector	vBasePos = bone.GetLocalSpacePosition( vBonePos ) ;
	for ( size_t i = 0; i < nTotalFrame; i ++ )
	{
		S4DMatrix	mat4( 1, 1, 1, 1 ) ;
		S3DMatrix	mat3( 1, 1, 1 ) ;
		if ( pSeqRot != nullptr )
		{
			mat3 = pSeqRot->GetFrameMatrix( (double) i ) ;
		}
		if ( pSeqZoom != nullptr )
		{
			mat3.MagnifyByVector
				( S3DVector( pSeqZoom->GetFrameVector( (double) i ) ) ) ;
		}
		if ( pSeqPos != nullptr )
		{
			mat4.SetTranslation
				( S3DVector( pSeqPos->GetFrameVector( (double) i ) ) - vBasePos ) ;
		}
		mat4.SetMatrix3( mat3 ) ;
		pMatrixs[i] = mat4 ;
		//
		if ( pSeqBlend != nullptr )
		{
			pPhysBlend[i] = (float32_t) pSeqBlend->GetFrameScalar( (double) i ) ;
		}
	}
	pJoint->m_aMatrixs.FinishArray() ;
	if ( pSeqBlend != nullptr )
	{
		pJoint->m_aPhysBlends.FinishArray() ;
	}
	return	pJoint ;
}

// 全マテリアル変換
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMeshEditorSerializer::ConvertAllMaterials
	( S3DModelBuffer& model, S3DSceneComposer::Composition & comp,
		S3DMeshEditorSerializer::TextureInfoSortMap& mapTexInfos,
		const S3DMeshEditorSerializer::ConvertModelParam& param )
{
	S3DSceneComposer * pComposer = comp.GetComposer() ;
	if ( pComposer == nullptr )
	{
		return	sglErrInvalidParam ;
	}
	const S3DSceneComposer::ResourceAssets&	assets = pComposer->GetAssets() ;
	for ( size_t iRsrc = 0; iRsrc < assets.GetResourceCount(); iRsrc ++ )
	{
		S3DSceneComposer::ResourceContainer *	prc = assets.GetResourceAt( iRsrc ) ;
		const wchar_t *	pwszRsrcID = assets.GetResourceIdentityAt( iRsrc ) ;
		if ( (prc == nullptr) || (pwszRsrcID == nullptr) )
		{
			continue ;
		}
		S3DMaterialLibrary *	pMaterialLib = prc->GetResource<S3DMaterialLibrary>() ;
		if ( pMaterialLib == nullptr )
		{
			continue ;
		}
		for ( size_t i = 0; i < pMaterialLib->GetMaterialCount(); i ++ )
		{
			S3DMaterial *	pMaterial = pMaterialLib->GetMaterialAt( i ) ;
			const wchar_t *	pwszMaterialID = pMaterialLib->GetMaterialIdentityAt( i ) ;
			if ( (pMaterial == nullptr) || (pwszMaterialID == nullptr) )
			{
				continue ;
			}
			ConvertMeshMaterial
				( model, comp, mapTexInfos,
					pMaterial, pwszMaterialID, param ) ;
		}
	}
	return	sglErrSuccess ;
}

// 各アイテム変換
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMeshEditorSerializer::ConvertCompositionItems
	( S3DModelBuffer& model,
		S3DSceneComposer::Composition & comp,
		S3DMeshEditorSerializer::BoneInfoSortMap& mapBoneInfos,
		S3DMeshEditorSerializer::TextureInfoSortMap& mapTexInfos,
		S3DMeshEditorSerializer::MeshGroupArray& mapMeshGroupArrays,
		const S3DDMatrix& matIBaseSpace,
		const S3DDVector& vIBaseSpace,
		S3DSceneComposer::SpaceSerializer & space,
		const S3DMeshEditorSerializer::ConvertModelParam& param )
{
	for ( size_t i = 0; i < space.GetChildrenCount(); i ++ )
	{
		S3DSceneComposer::SpaceSerializer *	pChild =
			ESLTypeCast<S3DSceneComposer::SpaceSerializer>( space.GetChildAt( i ) ) ;
		if ( pChild == nullptr )
		{
			continue ;
		}
		if ( !(pChild->GetBehaviorFlags()
					& (S3DScene::itemSpaceHidden | S3DScene::itemIgnore))
			|| (param.nFlags & cvtFlagAllSpaces) )
		{
			SGLError	err = ConvertCompositionItems
				( model, comp, mapBoneInfos,
					mapTexInfos, mapMeshGroupArrays,
					matIBaseSpace, vIBaseSpace, *pChild, param ) ;
			if ( err )
			{
				return	err ;
			}
		}
	}
	for ( size_t i = 0; i < space.GetItemCount(); i ++ )
	{
		S3DScene::Item *	pItem = space.GetItemAt( i ) ;
		if ( !(pItem->m_flagsBehavior & S3DScene::itemVisible)
			&& !(param.nFlags & cvtFlagAllItems) )
		{
			continue ;
		}
		S3DMeshEditorSerializer *
			pMeshEditor = ESLTypeCast<S3DMeshEditorSerializer>( pItem ) ;
		if ( pMeshEditor != nullptr )
		{
			SGLError	err = ConvertInstancingMesh
				( model, comp, mapBoneInfos,
					mapTexInfos, mapMeshGroupArrays,
					matIBaseSpace, vIBaseSpace,
					*pMeshEditor, *pMeshEditor, param ) ;
			if ( err )
			{
				return	err ;
			}
			continue ;
		}
		S3DMeshBufferItemSerializer *
			pMeshBuf = ESLTypeCast<S3DMeshBufferItemSerializer>( pItem ) ;
		if ( pMeshBuf != nullptr )
		{
			SGLError	err ;
			if ( pMeshBuf->IsInstancingDraw() )
			{
				err = ConvertInstancingMesh
					( model, comp, mapBoneInfos,
						mapTexInfos, mapMeshGroupArrays,
						matIBaseSpace, vIBaseSpace,
						*pMeshBuf, *pMeshBuf, param ) ;
			}
			else
			{
				S3DDMatrix	matMesh ;
				S3DDVector	vMesh ;
				pMeshBuf->GetGlobalTransformation( matMesh, vMesh ) ;
				//
				S3DDMatrix	matMeshBase = matIBaseSpace * matMesh ;
				S3DDVector	vMeshBase = matIBaseSpace * vMesh + vIBaseSpace ;
				//
				err = ConvertMeshBuffer
					( model, comp, mapBoneInfos,
						mapTexInfos, mapMeshGroupArrays,
						matMeshBase, vMeshBase, *pMeshBuf, 0, nullptr, param ) ;
			}
			if ( err )
			{
				return	err ;
			}
			continue ;
		}
		S3DIndirectMeshBuilderSerializer *
			pInMeshBuf = ESLTypeCast<S3DIndirectMeshBuilderSerializer>( pItem ) ;
		if ( pInMeshBuf != nullptr )
		{
			SGLError	err = ConvertIndirectMeshBuffer
				( model, comp, mapBoneInfos,
					mapTexInfos, mapMeshGroupArrays,
					matIBaseSpace, vIBaseSpace, *pInMeshBuf, param ) ;
			if ( err )
			{
				return	err ;
			}
			continue ;
		}
		S3DMultiInstanceSerializer *
			pInstancing = ESLTypeCast<S3DMultiInstanceSerializer>( pItem ) ;
		if ( pInstancing != nullptr )
		{
			SGLError	err = ConvertInstancingItem
				( model, comp, mapBoneInfos,
					mapTexInfos, mapMeshGroupArrays,
					matIBaseSpace, vIBaseSpace, *pInstancing, param ) ;
			if ( err )
			{
				return	err ;
			}
			continue ;
		}
		S3DBonePhysMaterialSerializer *
			pBonePhysMaterial = ESLTypeCast<S3DBonePhysMaterialSerializer>( pItem ) ;
		if ( pBonePhysMaterial != nullptr )
		{
			ConvertBonePhysMaterialItem
				( model, comp, *pBonePhysMaterial, param ) ;
			continue ;
		}
		S3DMarkerItemSerializer *
			pMarker = ESLTypeCast<S3DMarkerItemSerializer>( pItem ) ;
		if ( pMarker != nullptr )
		{
			ConvertMarkerItem
				( model, comp, mapBoneInfos,
					matIBaseSpace, vIBaseSpace, *pMarker, param ) ;
			continue ;
		}
	}
	return	sglErrSuccess ;
}

S3DMaterial * S3DMeshEditorSerializer::ConvertMeshMaterial
	( S3DModelBuffer& model,
		S3DSceneComposer::Composition & comp,
		S3DMeshEditorSerializer::TextureInfoSortMap& mapTexInfos,
		S3DMaterial * pMaterial, const wchar_t * pwszMaterialID,
		const S3DMeshEditorSerializer::ConvertModelParam& param )
{
	S3DMaterial *	pDstMaterial =
		model.GetMaterialLibrary().GetMaterialAs( pwszMaterialID ) ;
	if ( pDstMaterial != nullptr )
	{
		return	pDstMaterial ;
	}
	if ( param.nFlags & cvtFlagRefMaterial )
	{
		model.GetMaterialLibrary().AddMaterialAs( pwszMaterialID, pMaterial ) ;
		return	pMaterial ;
	}
	//
	// 表面属性
	//
	S3DMaterial *	pNewMaterial = new S3DMaterial ;
	pNewMaterial->SetSurfaceAttribute( pMaterial->m_attrSurface ) ;
	pNewMaterial->SetBackSurfaceAttribute( pMaterial->m_attrBack ) ;
	//
	const bool	flagBackSuf = pMaterial->IsEnabledBackSurfaceAttribute() ;
	pNewMaterial->EnableBackSurfaceAttribute( flagBackSuf ) ;
	//
	model.GetMaterialLibrary().AddSmartMaterialAs( pwszMaterialID, pNewMaterial ) ;
	//
	// 使用テクスチャ登録
	//
	for ( int i = 0; i < S3DMaterial::textureMaxCount; i ++ )
	{
		if ( pMaterial->m_pTexture[i] != nullptr )
		{
			ConvertMaterialTexture
				( model, comp, mapTexInfos,
					pNewMaterial, pMaterial,
					i, false, pwszMaterialID, param ) ;
		}
		else if ( !pMaterial->m_idTexture[i].IsEmpty()
				&& (param.nFlags & cvtFlagValidResourceRef) )
		{
			return	nullptr ;
		}
		if ( flagBackSuf )
		{
			if ( pMaterial->m_pBackTexture[i] != nullptr )
			{
				ConvertMaterialTexture
					( model, comp, mapTexInfos,
						pNewMaterial, pMaterial,
						i, true, pwszMaterialID, param ) ;
			}
			else if ( !pMaterial->m_idBackTexture[i].IsEmpty()
					&& (param.nFlags & cvtFlagValidResourceRef) )
			{
				return	nullptr ;
			}
		}
	}
	return	pNewMaterial ;
}

void S3DMeshEditorSerializer::ConvertMaterialTexture
	( S3DModelBuffer& model,
		S3DSceneComposer::Composition & comp,
		S3DMeshEditorSerializer::TextureInfoSortMap& mapTexInfos,
		S3DMaterial * pDstMaterial,
		S3DMaterial * pSrcMaterial,
		int iTexture, bool flagBack,
		const wchar_t * pwszMaterialID,
		const S3DMeshEditorSerializer::ConvertModelParam& param )
{
	SGLImageObject *	pSrcTexture =
		!flagBack ? pSrcMaterial->m_pTexture[iTexture]
					: pSrcMaterial->m_pBackTexture[iTexture] ;
	TextureInfo *	pTexInfo = mapTexInfos.GetAs( pSrcTexture ) ;
	if ( pTexInfo == nullptr )
	{
		SString	idTexture =
			!flagBack ? pSrcMaterial->m_idTexture[iTexture]
						: pSrcMaterial->m_idBackTexture[iTexture] ;
		if ( idTexture.IsEmpty() )
		{
			idTexture = comp.GetComposer()->GetAssets().
									GetResourceIdentityOf( pSrcTexture ) ;
		}
		if ( idTexture.IsEmpty() )
		{
			for ( int i = 0; i < 900; i ++ )
			{
				idTexture.Format( L"%s.%03d", pwszMaterialID, (iTexture + i) ) ;
				if ( model.GetTextureLibrary().GetTextureAs( idTexture ) == nullptr )
				{
					break ;
				}
			}
		}
		SGLImageObject *	pDstTexture = pSrcTexture ;
		if ( !(param.nFlags & cvtFlagRefTexture) )
		{
			pDstTexture = pSrcTexture->NewReference() ;
			model.GetTextureLibrary().AddSmartTextureAs( idTexture, pDstTexture ) ;
		}
		else
		{
			model.GetTextureLibrary().AddTextureAs( idTexture, pDstTexture ) ;
		}
		pTexInfo = new TextureInfo ;
		pTexInfo->pTexture = pDstTexture ;
		pTexInfo->idTexture = idTexture ;
		mapTexInfos.Add( pSrcTexture, pTexInfo ) ;
	}
	if ( !flagBack )
	{
		pDstMaterial->SetTexture
			( pTexInfo->pTexture, iTexture,
				pSrcMaterial->GetTextureFlags(iTexture),
				pSrcMaterial->GetTextureApplication(iTexture),
				pSrcMaterial->GetTextureParameter(iTexture), pTexInfo->idTexture ) ;
	}
	else
	{
		pDstMaterial->SetBackTexture
			( pTexInfo->pTexture, iTexture,
				pSrcMaterial->GetBackTextureFlags(iTexture),
				pSrcMaterial->GetBackTextureApplication(iTexture),
				pSrcMaterial->GetBackTextureParameter(iTexture), pTexInfo->idTexture ) ;
	}
}

SGLError S3DMeshEditorSerializer::ConvertInstancingMesh
	( S3DModelBuffer& model,
		S3DSceneComposer::Composition & comp,
		S3DMeshEditorSerializer::BoneInfoSortMap& mapBoneInfos,
		S3DMeshEditorSerializer::TextureInfoSortMap& mapTexInfos,
		S3DMeshEditorSerializer::MeshGroupArray& mapMeshGroupArrays,
		const S3DDMatrix& matIBaseSpace,
		const S3DDVector& vIBaseSpace,
		S3DSceneComposer::ItemSerializer & mesh,
		S3DSceneComposer::ItemSerializer & itemOwner,
		const S3DMeshEditorSerializer::ConvertModelParam& param )
{
	S3DInstancingItemInterface *
		pInstancingItem = ESLTypeCast<S3DInstancingItemInterface>( &mesh ) ;
	if ( pInstancingItem == nullptr )
	{
		return	sglErrSuccess ;
	}
	S3DItemInstancingSerializer *
		pInstancing = pInstancingItem->GetInstancing() ;
	if ( pInstancing == nullptr )
	{
		return	sglErrSuccess ;
	}
	return	ConvertInstancingMesh
		( model, comp, mapBoneInfos, mapTexInfos, mapMeshGroupArrays,
			matIBaseSpace, vIBaseSpace, mesh, itemOwner, *pInstancing, param ) ;
}

SGLError S3DMeshEditorSerializer::ConvertInstancingMesh
	( S3DModelBuffer& model,
		S3DSceneComposer::Composition & comp,
		S3DMeshEditorSerializer::BoneInfoSortMap& mapBoneInfos,
		S3DMeshEditorSerializer::TextureInfoSortMap& mapTexInfos,
		S3DMeshEditorSerializer::MeshGroupArray& mapMeshGroupArrays,
		const S3DDMatrix& matIBaseSpace,
		const S3DDVector& vIBaseSpace,
		S3DSceneComposer::ItemSerializer & mesh,
		S3DSceneComposer::ItemSerializer & itemOwner,
		S3DItemInstancingSerializer & instancing,
		const S3DMeshEditorSerializer::ConvertModelParam& param )
{
	const S4DMatrix *	pmatInstancing ;
	const S3DColor *	pclrInstancing ;
	size_t				nInstanceCount ;
	size_t				iInstancing = 0 ;
	if ( instancing.IsNeededDynamicProcess() )
	{
		nInstanceCount = instancing.GetProcessedInstancingArray
									( pmatInstancing, pclrInstancing ) ;
	}
	else
	{
		nInstanceCount =
			instancing.GetStaticInstancingArray
					( pmatInstancing, pclrInstancing ) ;
		if ( nInstanceCount > 0 )
		{
			for ( size_t i = 0; i < nInstanceCount; i ++ )
			{
				SGLError	err = ConvertInstanceMeshAt
					( model, comp, mapBoneInfos, mapTexInfos,
						mapMeshGroupArrays, matIBaseSpace, vIBaseSpace,
						iInstancing ++, pmatInstancing[i], mesh, itemOwner, param ) ;
				if ( err )
				{
					return	err ;
				}
			}
		}
		nInstanceCount =
			instancing.GetDynamicInstancingArray
					( pmatInstancing, pclrInstancing ) ;
	}
	if ( nInstanceCount > 0 )
	{
		for ( size_t i = 0; i < nInstanceCount; i ++ )
		{
			SGLError	err = ConvertInstanceMeshAt
				( model, comp, mapBoneInfos, mapTexInfos,
					mapMeshGroupArrays, matIBaseSpace, vIBaseSpace,
					iInstancing ++, pmatInstancing[i], mesh, itemOwner, param ) ;
			if ( err )
			{
				return	err ;
			}
		}
	}
	return	sglErrSuccess ;
}

SGLError S3DMeshEditorSerializer::ConvertInstanceMeshAt
	( S3DModelBuffer& model,
		S3DSceneComposer::Composition & comp,
		S3DMeshEditorSerializer::BoneInfoSortMap& mapBoneInfos,
		S3DMeshEditorSerializer::TextureInfoSortMap& mapTexInfos,
		S3DMeshEditorSerializer::MeshGroupArray& mapMeshGroupArrays,
		const S3DDMatrix& matIBaseSpace,
		const S3DDVector& vIBaseSpace,
		size_t iInstancing, const S4DMatrix& mat4Instance,
		S3DSceneComposer::ItemSerializer & mesh,
		S3DSceneComposer::ItemSerializer & itemOwner,
		const S3DMeshEditorSerializer::ConvertModelParam& param )
{
	//
	// メッシュ基底行列
	//
	S3DDMatrix	matdMesh ;
	S3DDVector	vdMesh ;
	itemOwner.GetGlobalTransformation( matdMesh, vdMesh ) ;
	//
	S3DMatrix	matMeshBase = matIBaseSpace * matdMesh ;
	S3DVector	vMeshBase = matIBaseSpace * vdMesh + vIBaseSpace ;

	S3DMatrix	matSpace = matMeshBase * mat4Instance.GetMatrix3() ;
	S3DVector	vSpace = matMeshBase * mat4Instance.GetTranslation() + vMeshBase ;
	//
	S3DMeshEditorSerializer *
		pMeshEditor = ESLTypeCast<S3DMeshEditorSerializer>( &mesh ) ;
	if ( pMeshEditor != nullptr )
	{
		return	ConvertMeshEditor
			( model, comp, mapBoneInfos,
				mapTexInfos,
				mapMeshGroupArrays,
				matSpace, vSpace,
				*pMeshEditor, itemOwner, iInstancing, param ) ;
	}
	else
	{
		S3DMeshBufferItemSerializer *
			pMeshBuf = ESLTypeCast<S3DMeshBufferItemSerializer>( &mesh ) ;
		if ( pMeshBuf != nullptr )
		{
			return	ConvertMeshBuffer
				( model, comp, mapBoneInfos,
					mapTexInfos, mapMeshGroupArrays,
					matSpace, vSpace, *pMeshBuf, iInstancing, nullptr, param ) ;
		}
	}
	return	sglErrSuccess ;
}

SGLError S3DMeshEditorSerializer::ConvertMeshEditor
	( S3DModelBuffer& model,
		S3DSceneComposer::Composition & comp,
		S3DMeshEditorSerializer::BoneInfoSortMap& mapBoneInfos,
		S3DMeshEditorSerializer::TextureInfoSortMap& mapTexInfos,
		S3DMeshEditorSerializer::MeshGroupArray& mapMeshGroupArrays,
		const S3DMatrix& matMeshBase, const S3DVector& vMeshBase,
		S3DMeshEditorSerializer & mesh,
		S3DSceneComposer::ItemSerializer & itemOwner, size_t iInstancing,
		const S3DMeshEditorSerializer::ConvertModelParam& param )
{
	//
	// 参照マテリアルを登録
	//
	S3DMaterial *	pMaterial[paramMaterialCount] ;
	for ( size_t i = 0; i < paramMaterialCount; i ++ )
	{
		if ( (mesh.m_pMaterials[i] != nullptr)
			&& !mesh.m_strMaterialIDs[i].IsEmpty() )
		{
			pMaterial[i] =
				ConvertMeshMaterial
					( model, comp, mapTexInfos,
						mesh.m_pMaterials[i],
						mesh.m_strMaterialIDs[i], param ) ;
			if ( (param.nFlags & cvtFlagValidResourceRef)
				&& (pMaterial[i] == nullptr) )
			{
				return	sglErrFailed ;
			}
		}
		else if ( (mesh.m_pMaterials[i] == nullptr)
				&& !mesh.m_strMaterialIDs[i].IsEmpty()
				&& (param.nFlags & cvtFlagValidResourceRef) )
		{
			return	sglErrFailed ;
		}
		else
		{
			pMaterial[i] = nullptr ;
		}
	}
	//
	// メッシュをベイク
	//
	SSmartPointer<S3DMeshEditor>	pTempMesh = new S3DMeshEditor ;
	pTempMesh->DuplicateMesh( mesh.m_mesh ) ;
	//
	S3DMeshEditor *	pMesh = pTempMesh ;
	const size_t	nCtrls = mesh.GetControllerCount() ;
	for ( size_t i = 0; i < nCtrls; i ++ )
	{
		S3DSceneComposer::Controller *	pCtrl = mesh.GetControllerAt( i ) ;
		if ( pCtrl == nullptr )
		{
			continue ;
		}
		MeshController *	pMeshCtrl = ESLTypeCast<MeshController>( pCtrl ) ;
		if ( (pMeshCtrl != nullptr)
			&& (ESLTypeCast<S3DMorphMeshController>( pCtrl ) == nullptr) )
		{
			pMesh = pMeshCtrl->ModifyMeshEditor( *pMesh, mesh.m_mesh ) ;
		}
	}
	const S3DMeshEditor::MeshParam&	mparam = mesh.m_mesh.GetMeshParameter() ;
	SSmartPointer<S3DMeshEditor>	pDivMesh ;
	if ( (mparam.nDivHorz >= 2) || (mparam.nDivVert >= 2) )
	{
		pDivMesh = new S3DMeshEditor ;
		pDivMesh->RedivideMeshFrom( *pMesh, mparam ) ;
		pMesh = pDivMesh ;
	}
	//
	// ウェイトマップのボーン情報
	//
	SPointerArray<S3DModelBoneSpace>	aBoneWeightMap ;
	S3DModelBoneSpace *					pParentBone = nullptr ;
	const size_t	nWeightLayers = pMesh->GetWeightLayerCount() ;
	const bool		flagEnableBone = mesh.IsEnabledBone()
									&& mesh.IsValidBoneReference() ;
	if ( flagEnableBone )
	{
		if ( param.nFlags & cvtFlagOptimizeBoneWeights )
		{
			pMesh->OptimizeBoneWeightMap() ;
		}
		else
		{
			pMesh->NormalizeBoneWeightMap() ;
		}
	}
	else if ( !(param.nFlags & cvtFlagWithoutBone) )
	{
		pParentBone = GetParentBoneByInfoMap( mapBoneInfos, comp, &itemOwner ) ;
	}
	for ( size_t i = 0; i < nWeightLayers; i ++ )
	{
		aBoneWeightMap.SetAt( i, nullptr ) ;
		//
		const S3DMeshEditor::WeightMapInfo *
						pwmi = pMesh->GetWeightLayerInfoAt( i ) ;
		if ( (pwmi != nullptr) && (pwmi->nFlags & S3DMeshEditor::weightBone) )
		{
			if ( flagEnableBone )
			{
				BuildBoneInfo *	pBoneInfo =
					mapBoneInfos.GetAs
						( comp.GetSceneItemAs
							( pMesh->GetWeightLayerIDAt( i ) ) ) ;
				if ( pBoneInfo != nullptr )
				{
					aBoneWeightMap.SetAt( i, pBoneInfo->pBone ) ;
				}
			}
		}
	}
	//
	// メッシュ名取得
	//
	SString	strMeshItemID = comp.GetSceneItemIDOf( &itemOwner ) ;
	if ( strMeshItemID.IsEmpty() )
	{
		int	iMeshNum = 0 ;
		strMeshItemID = L"mesh" ;
		while ( mapMeshGroupArrays.GetAs( strMeshItemID ) != nullptr )
		{
			strMeshItemID = L"mesh" ;
			strMeshItemID += SString( ++ iMeshNum ) ;
		}
		ESLAssert( mapMeshGroupArrays.GetAs( strMeshItemID ) == nullptr ) ;
	}
	//
	// マテリアル毎のメッシュグループ
	//
	SPointerArray<S3DModelData::MeshGroup>	aMeshGroups ;
	SPointerArray<S3DModelData::MeshGroup> *
						pMaterialMeshGroups[paramMaterialCount] ;
	for ( int i = 0; i < paramMaterialCount; i ++ )
	{
		pMaterialMeshGroups[i] = nullptr ;
	}
	//
	// 各パッチを出力
	//
	S3DMeshEditor::MeshBuffer	mbuf ;
	pMesh->UpdateAllPatchs( mparam ) ;
	pMesh->GetExAttrWeightLayers( mbuf.m_bufExAttrIndex ) ;
	//
	for ( size_t iPatch = 0; iPatch < pMesh->GetPatchCount(); iPatch ++ )
	{
		S3DMeshEditor::Patch *	pPatch = pMesh->GetPatchAt( iPatch ) ;
		ESLAssert( pPatch != nullptr ) ;
		aMeshGroups.SetAt( iPatch, nullptr ) ;
		//
		if ( pPatch->GetFlags() & S3DMeshEditor::Patch::flagInvisible )
		{
			continue ;
		}
		//
		size_t	iMaterial = pPatch->GetMaterialIndex() ;
		if ( iMaterial >= paramMaterialCount )
		{
			continue ;
		}
		if ( pMaterial[iMaterial] == nullptr )
		{
			pMaterial[iMaterial] =
				ConvertMeshMaterial
					( model, comp, mapTexInfos,
						S3DMaterial::GetDefaultMaterial( S3DMaterial::defaultWhite ),
						L"@default_material", param ) ;
		}
		//
		// メッシュを出力
		//
		const size_t	iMesh = model.GetMeshCount() ;
		model.AttachDefaultMaterial( pMaterial[iMaterial] ) ;
		//
		const size_t	nAreaSize = pPatch->GetAreaSize() ;
		if ( nAreaSize > 0 )
		{
			pPatch->BuildPatchMesh( mbuf, mparam ) ;
			S3DMeshEditor::RenderQuadMeshBuffer
				( model, pMaterial[iMaterial],
					matMeshBase, vMeshBase, mbuf, mparam ) ;
		}
		else if ( pPatch->GetExVertexCount() > 0 )
		{
			mbuf.m_countVertex = pPatch->GetExVertexCount() ;
			mbuf.m_edegs.RemoveAll() ;
			//
			S3DMeshEditor::RenderExPatchTriangles
				( model, pMaterial[iMaterial],
					matMeshBase, vMeshBase, *pPatch, mbuf, mparam ) ;
			//
			size_t *	pSrcIndex = mbuf.m_bufSrcVertex.GetArray( mbuf.m_countVertex ) ;
			for ( size_t i = 0; i < mbuf.m_countVertex; i ++ )
			{
				pSrcIndex[i] = nAreaSize + i ;
			}
			mbuf.m_bufSrcVertex.FinishArray() ;
		}
		else
		{
			continue ;
		}
		//
		// メッシュ情報
		//
		S3DVector	vMeshMin, vMeshMax ;
		model.GetCircumscribedBoxOfMeshAt( vMeshMin, vMeshMax, iMesh ) ;
		//
		S3DModelData::MeshGroup	meshGroup ;
		meshGroup.m_iFirstMesh = (uint32_t) iMesh ;
		meshGroup.m_nMeshCount = (uint32_t) (model.GetMeshCount() - iMesh) ;
		meshGroup.m_vCenter = (vMeshMin + vMeshMax) * 0.5f ;
		//
		SString	strBaseName = strMeshItemID ;
		SString	strMeshName = strMeshItemID ;
		SString	strMergeName ;
		ssize_t	iSep = strMeshItemID.Find( '@' ) ;
		if ( iSep >= 0 )
		{
			strBaseName = strMeshItemID.Left( (size_t) iSep ) ;
			strMeshName = strBaseName ;
			strMergeName = strMeshItemID.Middle( (size_t) iSep ) ;
		}
		if ( pMesh->GetPatchCount() > 1 )
		{
			strMeshName += L"." ;
			strMeshName += pPatch->GetName() ;
		}
		strMeshName += strMergeName ;
		//
		size_t	iMeshNameNum = iInstancing ;
		while ( model.GetMeshGroupAs( strMeshName ) != nullptr )
		{
			strMeshName = strBaseName + L"."
						+ pPatch->GetName() + L"."
						+ SString(++ iMeshNameNum) + strMergeName ;
		}
		model.GetMeshGroupList().Add( strMeshName, meshGroup ) ;
		//
		// マテリアル毎のメッシュグループ
		//
		S3DModelData::MeshGroup *	pmgMesh = model.GetMeshGroupAs( strMeshName ) ;
		ESLAssert( pmgMesh != nullptr ) ;
		if ( pMaterialMeshGroups[iMaterial] == nullptr )
		{
			SString	strMeshGroupName = strMeshItemID ;
			if ( iMaterial > 0 )
			{
				int	iMeshGroupNameNum = (int) iInstancing ;
				strMeshGroupName += L"." ;
				strMeshGroupName += SString(iMaterial) ;
				while ( mapMeshGroupArrays.GetAs( strMeshGroupName ) != nullptr )
				{
					strMeshGroupName = strMeshItemID + L"."
								+ SString(iMaterial) + L"."
								+ SString(++ iMeshGroupNameNum) ;
				}
			}
			pMaterialMeshGroups[iMaterial] = new SPointerArray<S3DModelData::MeshGroup> ;
			mapMeshGroupArrays.Add
				( strMeshGroupName, pMaterialMeshGroups[iMaterial] ) ;
		}
		pMaterialMeshGroups[iMaterial]->Add( pmgMesh ) ;
		aMeshGroups.SetAt( iPatch, pmgMesh ) ;
		//
		// ボーン用ウェイトマップ
		//
		if ( !(param.nFlags & cvtFlagWithoutBone) )
		for ( size_t iLayer = 0; iLayer < aBoneWeightMap.GetLength(); iLayer ++ )
		{
			S3DModelBoneSpace *	pBone = aBoneWeightMap.GetAt( iLayer ) ;
			if ( pBone == nullptr )
			{
				continue ;
			}
			S3DModelData::MeshObject *	pMeshObj = model.GetMeshObjectAt( iMesh ) ;
			if ( pMeshObj == nullptr )
			{
				continue ;
			}
			//
			// 行列設定
			//
			S3DDMatrix	matdBone ;
			S3DDVector	vdBone ;
			pBone->CalcBoneTransformation( matdBone, vdBone ) ;
			//
			S3DMatrix	matIBone = matdBone.Inverse() ;
			S4DMatrix	mat4IMesh( 1, 1, 1, 1 ) ;
			S4DMatrix	mat4RelMesh( 1, 1, 1, 1 ) ;
			mat4IMesh.SetMatrix3( matIBone ) ;
			mat4IMesh.SetTranslation( matIBone * S3DVector(- vdBone) ) ;
			//
			pBone->AddEffectiveMeshIndex( iMesh, mat4IMesh, mat4RelMesh ) ;
			//
			// ウェイトマップ
			//
			const size_t	nVertexCount = pMeshObj->m_countVertex ;
			ESLAssert( mbuf.m_countVertex == nVertexCount ) ;
			ESLAssert( mbuf.m_bufSrcVertex.GetLength() >= nVertexCount ) ;
			if ( (mbuf.m_countVertex < nVertexCount)
				|| (mbuf.m_bufSrcVertex.GetLength() < nVertexCount) )
			{
				continue ;
			}
			size_t	iMeshVertex = pMeshObj->m_iVertex ;
			size_t	nMeshCount = pMeshObj->m_countVertex ;
			pBone->ExpandBoneWeightBounds( iMeshVertex, nMeshCount ) ;
			//
			const size_t *	pSrcIndex = mbuf.m_bufSrcVertex.GetConstArray() ;
			float32_t *		pfpWeightMap =
								pBone->LockBoneWeightMap( iMeshVertex, nMeshCount ) ;
			ESLAssert( iMeshVertex == pMeshObj->m_iVertex ) ;
			ESLAssert( nMeshCount == pMeshObj->m_countVertex ) ;
			for ( size_t i = 0; i < nVertexCount; i ++ )
			{
				pfpWeightMap[i] = pPatch->GetWeightAt( pSrcIndex[i], iLayer ) ;
			}
			pBone->UnlockBoneWeightMap( true ) ;
		}
		if ( (pParentBone != nullptr)
			&& (model.GetMeshObjectAt( iMesh ) != nullptr) )
		{
			//
			// 行列設定（ボーンの直系子配置）
			//
			S3DDMatrix	matdBone ;
			S3DDVector	vdBone ;
			pParentBone->CalcBoneTransformation( matdBone, vdBone ) ;
			//
			S3DMatrix	matIBone = matdBone.Inverse() ;
			S4DMatrix	mat4IMesh( 1, 1, 1, 1 ) ;
			S4DMatrix	mat4RelMesh( 1, 1, 1, 1 ) ;
			mat4IMesh.SetMatrix3( matIBone ) ;
			mat4IMesh.SetTranslation( matIBone * S3DVector(- vdBone) ) ;
			//
			pParentBone->AddEffectiveMeshIndex( iMesh, mat4IMesh, mat4RelMesh ) ;
			//
			// ウェイトマップ（ボーンの直系子配置）
			//
			S3DModelData::MeshObject *	pMeshObj = model.GetMeshObjectAt( iMesh ) ;
			ESLAssert( pMeshObj != nullptr ) ;
			//
			size_t	iMeshVertex = pMeshObj->m_iVertex ;
			size_t	nMeshCount = pMeshObj->m_countVertex ;
			pParentBone->ExpandBoneWeightBounds( iMeshVertex, nMeshCount ) ;
			//
			float32_t *	pfpWeightMap =
							pParentBone->LockBoneWeightMap( iMeshVertex, nMeshCount ) ;
			for ( size_t i = 0; i < nMeshCount; i ++ )
			{
				pfpWeightMap[i] = 1.0f ;
			}
			pParentBone->UnlockBoneWeightMap( true ) ;
		}
	}
	model.AttachDefaultMaterial( nullptr ) ;
	//
	// モーフィング
	//
	S3DRenderBuffer	render ;
	for ( size_t i = 0; i < nCtrls; i ++ )
	{
		S3DSceneComposer::Controller *	pCtrl = mesh.GetControllerAt( i ) ;
		if ( pCtrl == nullptr )
		{
			continue ;
		}
		S3DMorphMeshController *
			pMorphCtrl = ESLTypeCast<S3DMorphMeshController>( pCtrl ) ;
		if ( pMorphCtrl == nullptr )
		{
			continue ;
		}
		SString	strMorphCtrlID = pMorphCtrl->GetItemIdentity() ;
		//
		// モーフィングターゲットをベイク
		//
		SSmartPointer<S3DMeshEditor>	pMorphTempMesh = new S3DMeshEditor ;
		pMorphTempMesh->DuplicateMesh( pMorphCtrl->MeshEditor() ) ;
		S3DMeshEditor *					pMorph = pMorphTempMesh ;
		//
		for ( size_t j = i + 1; j < nCtrls; j ++ )
		{
			S3DSceneComposer::Controller *	pCtrl2 = mesh.GetControllerAt( j ) ;
			if ( pCtrl2 == nullptr )
			{
				continue ;
			}
			MeshController *	pMeshCtrl2 = ESLTypeCast<MeshController>( pCtrl2 ) ;
			if ( (pMeshCtrl2 != nullptr)
				&& (ESLTypeCast<S3DMorphMeshController>( pCtrl2 ) == nullptr) )
			{
				pMorph = pMeshCtrl2->ModifyMeshEditor
								( *pMorph, pMorphCtrl->MeshEditor() ) ;
			}
		}
		SSmartPointer<S3DMeshEditor>	pMorphDivMesh ;
		if ( (mparam.nDivHorz >= 2) || (mparam.nDivVert >= 2) )
		{
			pMorphDivMesh = new S3DMeshEditor ;
			pMorphDivMesh->RedivideMeshFrom( *pMorph, mparam ) ;
			pMorph = pMorphDivMesh ;
		}
		//
		S3DMeshEditor::MeshBuffer	mbufMorph ;
		pMorph->UpdateAllPatchs( mparam ) ;
		pMorph->GetExAttrWeightLayers( mbufMorph.m_bufExAttrIndex ) ;
		//
		for ( size_t iPatch = 0; iPatch < pMorph->GetPatchCount(); iPatch ++ )
		{
			S3DMeshEditor::Patch *	pMorphPatch = pMorph->GetPatchAt( iPatch ) ;
			ESLAssert( pMorphPatch != nullptr ) ;
			//
			S3DModelData::MeshGroup *	pmgSrcMesh = aMeshGroups.GetAt( iPatch ) ;
			if ( pmgSrcMesh == nullptr )
			{
				continue ;
			}
			S3DModelData::MeshObject *	pSrcMeshObj =
					model.GetMeshObjectAt( pmgSrcMesh->m_iFirstMesh ) ;
			ESLAssert( pSrcMeshObj != nullptr ) ;
			if ( pSrcMeshObj == nullptr )
			{
				continue ;
			}
			//
			// メッシュを出力
			//
			const size_t	nAreaSize = pMorphPatch->GetAreaSize() ;
			if ( nAreaSize > 0 )
			{
				render.ClearBuffer() ;
				pMorphPatch->BuildPatchMesh( mbufMorph, mparam ) ;
				S3DMeshEditor::RenderQuadMeshBuffer
					( render, pMaterial[0],
						matMeshBase, vMeshBase, mbufMorph, mparam ) ;
			}
			else if ( pMorphPatch->GetExVertexCount() > 0 )
			{
				render.ClearBuffer() ;
				mbufMorph.m_edegs.RemoveAll() ;
				S3DMeshEditor::RenderExPatchTriangles
					( render, pMaterial[0],
						matMeshBase, vMeshBase, *pMorphPatch, mbufMorph, mparam ) ;
			}
			else
			{
				continue ;
			}
			S3DRenderBuffer::MeshBuffer	mbufRender ;
			render.GetMeshBufferAsSinglePrimitive( mbufRender ) ;
			//
			if ( pSrcMeshObj->m_countVertex != mbufRender.m_nVertexCount )
			{
				continue ;
			}
			//
			S3DModelBuffer::MorphTargetMesh *
					pMorphTarget = new S3DModelBuffer::MorphTargetMesh ;
			pMorphTarget->m_countVertex = mbufRender.m_nVertexCount ;
			pMorphTarget->m_bufVertex = mbufRender.m_bufVertex ;
			pMorphTarget->m_bufNormal = mbufRender.m_bufNormal ;
			pMorphTarget->m_bufUVMap = mbufRender.m_bufUVMap ;
			pMorphTarget->m_bufColor = mbufRender.m_bufColor ;
			pMorphTarget->m_bufWeight = mbufRender.m_bufExAttr ;
			//
			if ( (mbufRender.m_nExAttrCount > 1)
				&& (pMorphTarget->m_bufWeight.GetLength()
					>= mbufRender.m_nVertexCount * mbufRender.m_nExAttrCount) )
			{
				float32_t *	pfpWeight = pMorphTarget->m_bufWeight.GetArray() ;
				for ( size_t i = 0; i < mbufRender.m_nVertexCount; i ++ )
				{
					pfpWeight[i] = pfpWeight[i * mbufRender.m_nExAttrCount] ;
				}
				pMorphTarget->m_bufWeight.FinishArray() ;
			}
			//
			// モーフターゲット登録
			//
			SString	strMorphName = strMorphCtrlID ;
			if ( pMorph->GetPatchCount() > 1 )
			{
				strMorphName += L"." ;
				strMorphName += pMorphPatch->GetName() ;
			}
			size_t	iMorphNameNum = iInstancing ;
			while ( model.GetMorhTargetAs( strMorphName ) != nullptr )
			{
				strMorphName = strMorphCtrlID + L"."
						+ pMorphPatch->GetName() + L"." + SString(++ iMorphNameNum) ;
			}
			pMorphTarget = model.AddMorhTargetAs( strMorphName, pMorphTarget ) ;
			pSrcMeshObj->m_arrMorphTarget.Add( new SString(strMorphName) ) ;
		}
	}
	return	sglErrSuccess ;
}

SGLError S3DMeshEditorSerializer::ConvertMeshBuffer
	( S3DModelBuffer& model,
		S3DSceneComposer::Composition & comp,
		S3DMeshEditorSerializer::BoneInfoSortMap& mapBoneInfos,
		S3DMeshEditorSerializer::TextureInfoSortMap& mapTexInfos,
		S3DMeshEditorSerializer::MeshGroupArray& mapMeshGroupArrays,
		const S3DMatrix& matMeshBase, const S3DVector& vMeshBase,
		S3DMeshBufferItemSerializer & mesh, size_t iInstancing,
		S3DSceneComposer::ItemSerializer * pOptMesh,
		const S3DMeshEditorSerializer::ConvertModelParam& param )
{
	//
	// 参照マテリアルを登録
	//
	S3DMaterial *	pMaterial[S3DMeshBufferItemSerializer::paramMaterialMaxCount] ;
	size_t	nMaterialCount = mesh.GetMaxMaterialCount() ;
	for ( size_t i = 0; i < nMaterialCount; i ++ )
	{
		SString	strMaterialID = mesh.GetMaterialIDAt( i ) ;
		pMaterial[i] = mesh.GetMaterialAt( i ) ;
		if ( (pMaterial[i] != nullptr)
			&& !strMaterialID.IsEmpty() )
		{
			pMaterial[i] =
				ConvertMeshMaterial
					( model, comp, mapTexInfos,
						pMaterial[i], strMaterialID, param ) ;
			if ( (param.nFlags & cvtFlagValidResourceRef)
				&& (pMaterial[i] == nullptr) )
			{
				return	sglErrFailed ;
			}
		}
		else if ( (pMaterial[i] == nullptr)
				&& !strMaterialID.IsEmpty()
				&& (param.nFlags & cvtFlagValidResourceRef) )
		{
			return	sglErrFailed ;
		}
		else
		{
			pMaterial[i] = nullptr ;
		}
	}
	//
	// メッシュをベイク
	//
	SObjectArray<S3DRenderBuffer>			aVBO ;
	SPointerArray<S3DVertexBufferInterface>	aVBOPtrs ;
	for ( size_t i = 0; i < nMaterialCount; i ++ )
	{
		S3DRenderBuffer *	pBuf = new S3DRenderBuffer ;
		pBuf->AttachDefaultMaterial( pMaterial[i] ) ;
		aVBO.SetAt( i, pBuf ) ;
		aVBOPtrs.SetAt( i, pBuf ) ;
	}
	S3DSceneComposer::ItemSerializer *	pMeshItem = &mesh ;
	S3DScene	scene ;
	if ( pOptMesh == nullptr )
	{
		mesh.RenderToVertexBuffers( scene, aVBOPtrs.GetArray(), nMaterialCount ) ;
	}
	else
	{
		pMeshItem = pOptMesh ;
		mesh.RenderControllersToVertexBuffers
			( scene, *pOptMesh, aVBOPtrs.GetArray(), nMaterialCount ) ;
	}
	//
	// メッシュ名取得
	//
	SString	strMeshItemID = comp.GetSceneItemIDOf( pMeshItem ) ;
	if ( strMeshItemID.IsEmpty() )
	{
		int	iMeshNum = 0 ;
		strMeshItemID = L"mesh" ;
		while ( mapMeshGroupArrays.GetAs( strMeshItemID ) != nullptr )
		{
			strMeshItemID = L"mesh" ;
			strMeshItemID += SString( ++ iMeshNum ) ;
		}
	}
	ESLAssert( mapMeshGroupArrays.GetAs( strMeshItemID ) == nullptr ) ;
	//
	// ボーン取得
	//
	S3DModelBoneSpace *	pParentBone = nullptr ;
	if ( !(param.nFlags & cvtFlagWithoutBone) )
	{
		pParentBone = GetParentBoneByInfoMap( mapBoneInfos, comp, pMeshItem ) ;
	}
	//
	// メッシュ出力
	//
	S3DRenderBuffer::MeshBuffer	meshBuf ;
	for ( size_t i = 0; i < nMaterialCount; i ++ )
	{
		meshBuf.ClearBuffer() ;
		if ( aVBO.At(i).GetMeshBufferAsSinglePrimitive( meshBuf ) )
		{
			continue ;
		}
		if ( meshBuf.m_nVertexCount == 0 )
		{
			continue ;
		}
		meshBuf.Transform( matMeshBase, vMeshBase ) ;
		//
		const size_t	iMesh = model.GetMeshCount() ;
		//
		if ( pMaterial[i] == nullptr )
		{
			pMaterial[i] =
				ConvertMeshMaterial
					( model, comp, mapTexInfos,
						S3DMaterial::GetDefaultMaterial( S3DMaterial::defaultWhite ),
						L"@default_material", param ) ;
		}
		meshBuf.m_pMaterial = pMaterial[i] ;
		meshBuf.RenderToVertexBuffer( model ) ;
		//
		// メッシュ情報
		//
		S3DVector	vMeshMin, vMeshMax ;
		meshBuf.GetCircumscribedBox( vMeshMin, vMeshMax ) ;
		//
		S3DModelData::MeshGroup	meshGroup ;
		meshGroup.m_iFirstMesh = (uint32_t) iMesh ;
		meshGroup.m_nMeshCount = (uint32_t) (model.GetMeshCount() - iMesh) ;
		meshGroup.m_vCenter = (vMeshMin + vMeshMax) * 0.5f ;
		//
		SString	strBaseMeshName = strMeshItemID ;
		if ( nMaterialCount > 1 )
		{
			strBaseMeshName += L"." ;
			strBaseMeshName += SString(i) ;
		}
		SString	strMeshName = strBaseMeshName ;
		size_t	iMeshNameNum = iInstancing ;
		while ( model.GetMeshGroupAs( strMeshName ) != nullptr )
		{
			strMeshName = strBaseMeshName + L"." + SString(++ iMeshNameNum) ;
		}
		model.GetMeshGroupList().Add( strMeshName, meshGroup ) ;
		//
		// ボーンウェイトマップ
		//
		if ( (pParentBone != nullptr)
			&& (model.GetMeshObjectAt( iMesh ) != nullptr) )
		{
			//
			// 行列設定（ボーンの直系子配置）
			//
			S3DDMatrix	matdBone ;
			S3DDVector	vdBone ;
			pParentBone->CalcBoneTransformation( matdBone, vdBone ) ;
			//
			S3DMatrix	matIBone = matdBone.Inverse() ;
			S4DMatrix	mat4IMesh( 1, 1, 1, 1 ) ;
			S4DMatrix	mat4RelMesh( 1, 1, 1, 1 ) ;
			mat4IMesh.SetMatrix3( matIBone ) ;
			mat4IMesh.SetTranslation( matIBone * S3DVector(- vdBone) ) ;
			//
			pParentBone->AddEffectiveMeshIndex( iMesh, mat4IMesh, mat4RelMesh ) ;
			//
			// ウェイトマップ（ボーンの直系子配置）
			//
			S3DModelData::MeshObject *	pMeshObj = model.GetMeshObjectAt( iMesh ) ;
			ESLAssert( pMeshObj != nullptr ) ;
			//
			size_t	iMeshVertex = pMeshObj->m_iVertex ;
			size_t	nMeshCount = pMeshObj->m_countVertex ;
			pParentBone->ExpandBoneWeightBounds( iMeshVertex, nMeshCount ) ;
			//
			float32_t *	pfpWeightMap =
							pParentBone->LockBoneWeightMap( iMeshVertex, nMeshCount ) ;
			for ( size_t i = 0; i < nMeshCount; i ++ )
			{
				pfpWeightMap[i] = 1.0f ;
			}
			pParentBone->UnlockBoneWeightMap( true ) ;
		}
	}
	return	sglErrSuccess ;
}

SGLError S3DMeshEditorSerializer::ConvertIndirectMeshBuffer
	( S3DModelBuffer& model,
		S3DSceneComposer::Composition & comp,
		S3DMeshEditorSerializer::BoneInfoSortMap& mapBoneInfos,
		S3DMeshEditorSerializer::TextureInfoSortMap& mapTexInfos,
		S3DMeshEditorSerializer::MeshGroupArray& mapMeshGroupArrays,
		const S3DDMatrix& matIBaseSpace,
		const S3DDVector& vIBaseSpace,
		S3DIndirectMeshBuilderSerializer & mesh,
		const S3DMeshEditorSerializer::ConvertModelParam& param )
{
	S3DMeshBufferItemSerializer *	pMeshBuf = mesh.GetMeshTarget() ;
	if ( pMeshBuf == nullptr )
	{
		return	sglErrSuccess ;
	}
	S3DDMatrix	matdMesh ;
	S3DDVector	vdMesh ;
	mesh.GetGlobalTransformation( matdMesh, vdMesh ) ;
	//
	S3DMatrix	matMeshBase = matIBaseSpace * matdMesh ;
	S3DVector	vMeshBase = matIBaseSpace * vdMesh + vIBaseSpace ;
	//
	return	ConvertMeshBuffer
		( model, comp, mapBoneInfos, mapTexInfos, mapMeshGroupArrays,
			matMeshBase, vMeshBase, *pMeshBuf, 0, &mesh, param ) ;
}

SGLError S3DMeshEditorSerializer::ConvertInstancingItem
	( S3DModelBuffer& model,
		S3DSceneComposer::Composition & comp,
		S3DMeshEditorSerializer::BoneInfoSortMap& mapBoneInfos,
		S3DMeshEditorSerializer::TextureInfoSortMap& mapTexInfos,
		S3DMeshEditorSerializer::MeshGroupArray& mapMeshGroupArrays,
		const S3DDMatrix& matIBaseSpace,
		const S3DDVector& vIBaseSpace,
		S3DMultiInstanceSerializer & item,
		const S3DMeshEditorSerializer::ConvertModelParam& param )
{
	S3DSceneComposer::ItemSerializer *	pTargetItem = item.GetInstancingTarget() ;
	if ( pTargetItem == nullptr )
	{
		return	sglErrSuccess ;
	}
	return	ConvertInstancingMesh
		( model, comp, mapBoneInfos, mapTexInfos, mapMeshGroupArrays,
			matIBaseSpace, vIBaseSpace,
			*pTargetItem, item, item.Instancing(), param ) ;
}

void S3DMeshEditorSerializer::ConvertBonePhysMaterialItem
	( S3DModelBuffer& model,
		S3DSceneComposer::Composition & comp,
		S3DBonePhysMaterialSerializer& bonePhys,
		const S3DMeshEditorSerializer::ConvertModelParam& param )
{
	SString	strPhysID = comp.GetSceneItemIDOf( &bonePhys ) ;
	if ( !strPhysID.IsEmpty() )
	{
		model.GetPhysMaterialList().SetAs( strPhysID, bonePhys.GetPhysMaterial() ) ;
	}
}

void S3DMeshEditorSerializer::ConvertMarkerItem
	( S3DModelBuffer& model,
		S3DSceneComposer::Composition & comp,
		BoneInfoSortMap& mapBoneInfos,
		const S3DDMatrix& matIBaseSpace,
		const S3DDVector& vIBaseSpace,
		S3DMarkerItemSerializer& marker,
		const S3DMeshEditorSerializer::ConvertModelParam& param )
{
	SString	strMarkerID = comp.GetSceneItemIDOf( &marker ) ;
	if ( strMarkerID.IsEmpty() )
	{
		return ;
	}
	//
	// マーカー情報
	//
	S3DModelData::MarkerInfo *	pMarkerInfo = new S3DModelData::MarkerInfo ;
	marker.GetMarkerInfo( *pMarkerInfo ) ;
	model.GetMarkerInfoList().SetAs( strMarkerID, pMarkerInfo ) ;
	//
	S3DDMatrix	matdMarker ;
	S3DDVector	vdMarker ;
	marker.GetItemLinkTransformation( matdMarker, vdMarker ) ;
	//
	matdMarker = matIBaseSpace * matdMarker ;
	vdMarker = matIBaseSpace * vdMarker + vIBaseSpace ;
	//
	pMarkerInfo->m_vPosition =
		matdMarker * S3DDVector(pMarkerInfo->m_vPosition) + vdMarker ;
	pMarkerInfo->m_vDirection =
		(matdMarker * S3DDVector(pMarkerInfo->m_vDirection)).Normalized() ;
	pMarkerInfo->m_vSize =
		matdMarker * S3DDVector(pMarkerInfo->m_vSize) ;
	//
	// ボーン取得
	//
	BuildBoneInfo *	pbbiBone =
			GetParentBuildBoneByInfoMap( mapBoneInfos, comp, &marker ) ;
	if ( pbbiBone != nullptr )
	{
		S3DDMatrix	matBone ;
		S3DDVector	vBone ;
		pbbiBone->pItem->GetGlobalTransformation( matBone, vBone ) ;
		//
		pMarkerInfo->m_vPosition -= S3DVector(matIBaseSpace * vBone + vIBaseSpace) ;
		pMarkerInfo->m_strRefBone = comp.GetSceneItemIDOf( pbbiBone->pItem ) ;
	}
}

// モデルファイルを編集可能なコンポジションに変換する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMeshEditorSerializer::BuildMeshEditorItemsFromModel
	( S3DSceneComposer::Composition & comp,
		S3DSceneComposer::SpaceSerializer & spaceRoot,
		S3DModelBuffer& model,
		const BuildMeshEditorItemsParam& param )
{
	//
	// ボーンビルド
	//
	SStrSortObjectArray<SSystem::SString>	ssoaBoneMap ;
	BuildEditorBonesFromModel
		( comp, spaceRoot, ssoaBoneMap, model, &(model.GetBoneRoot()), param ) ;
	//
	// ボーン物理演算パラメータパレット
	//
	if ( model.GetPhysMaterialList().GetLength() > 0 )
	{
		S3DSceneComposer::SpaceSerializer *
				pSpace = new S3DSceneComposer::SpaceSerializer ;
		AddEditorItemChild
			( comp, spaceRoot, L"phys_params", pSpace ) ;
		//
		BuildEditorBonePhysFromModel( comp, *pSpace, model, param ) ;
	}
	//
	// マーカー
	//
	if ( model.GetMarkerInfoCount() > 0 )
	{
		S3DSceneComposer::SpaceSerializer *
				pSpace = new S3DSceneComposer::SpaceSerializer ;
		AddEditorItemChild
			( comp, spaceRoot, L"markers", pSpace ) ;
		//
		BuildEditorMarkersFromModel( comp, *pSpace, ssoaBoneMap, model, param ) ;
	}
	//
	// メッシュ
	//
	S3DSceneComposer::SpaceSerializer *
			pSpace = new S3DSceneComposer::SpaceSerializer ;
	AddEditorItemChild
		( comp, spaceRoot, L"meshs", pSpace ) ;
	//
	BuildEditorMeshsFromModel( comp, *pSpace, ssoaBoneMap, model, param ) ;
	//
	return	sglErrSuccess ;
}

bool S3DMeshEditorSerializer::AddEditorItemChild
	( S3DSceneComposer::Composition & comp,
		S3DSceneComposer::SpaceSerializer & spaceParent,
		const wchar_t * pwszID, S3DSceneComposer::ItemSerializer * pChild )
{
	SString	strID = pwszID ;
	comp.NormalizeSceneItemID( strID ) ;
	//
	S3DScene::Space *
		pChildSpace = ESLTypeCast<S3DScene::Space>( pChild ) ;
	if ( pChildSpace != nullptr )
	{
		spaceParent.InsertChild( spaceParent.GetChildrenCount(), pChildSpace ) ;
		comp.RegisterSceneItem( strID, pChild ) ;
		pChild->OnAttachedCompositionTree( comp ) ;
		return	true ;
	}
	S3DScene::Item *
		pChildItem = ESLTypeCast<S3DScene::Item>( pChild ) ;
	if ( pChildItem != nullptr )
	{
		spaceParent.InsertItem( spaceParent.GetItemCount(), pChildItem ) ;
		comp.RegisterSceneItem( strID, pChild ) ;
		pChild->OnAttachedCompositionTree( comp ) ;
		return	true ;
	}
	return	false ;
}

void S3DMeshEditorSerializer::BuildEditorBonesFromModel
	( S3DSceneComposer::Composition & comp,
		S3DSceneComposer::SpaceSerializer & spaceParent,
		SSystem::SStrSortObjectArray<SSystem::SString>& ssoaBoneMap,
		S3DModelBuffer& model,
		S3DModelBoneSpace * pBoneParent,
		const BuildMeshEditorItemsParam& param )
{
	const size_t	nChildrenCount = pBoneParent->GetChildrenCount() ;
	for ( size_t i = 0; i < nChildrenCount; i ++ )
	{
		S3DModelBoneSpace *	pBone =
				ESLTypeCast<S3DModelBoneSpace>( pBoneParent->GetChildAt(i) ) ;
		if ( pBone == nullptr )
		{
			continue ;
		}
		const SString *	pstrBoneID = model.GetBoneIdentityOf( pBone ) ;
		if ( pstrBoneID == nullptr )
		{
			continue ;
		}
		S3DBoneSerializer *	pBoneSer = new S3DBoneSerializer ;
		S3DDVector	vBonePos ;
		pBoneSer->SetItemPositioin( pBone->GetLocalSpacePosition(vBonePos) ) ;
		pBoneSer->SetBoneHandle( pBone->GetBoneHandle() ) ;
		if ( pBone->GetIKParameter().nIKFlags & S3DModelBoneSpace::flagIKBendDirection )
		{
			pBoneSer->EnableBendDir( true ) ;
			pBoneSer->SetBendDir( pBone->GetIKParameter().vBendDirection ) ;
		}
		if ( pBone->GetIKParameter().nIKFlags & S3DModelBoneSpace::flagIKParentAxis )
		{
			pBoneSer->SetBendParentAxisFlag( true ) ;
			pBoneSer->SetBendDir( pBone->GetIKParameter().vBendDirection ) ;
		}
		if ( pBone->GetIKParameter().nIKFlags & S3DModelBoneSpace::flagIKTerminate )
		{
			pBoneSer->SetIKTerminateFlag( true ) ;
		}
		if ( pBone->GetIKParameter().nIKFlags & S3DModelBoneSpace::flagIKMaxBent )
		{
			pBoneSer->SetBendMaxAngle( pBone->GetIKParameter().degMaxBent ) ;
		}
		pBoneSer->SetBendWeight( pBone->GetIKParameter().fpWeight ) ;
		pBoneSer->SetOrgMatrix( pBone->GetOriginalBoneMatrix() ) ;
		pBoneSer->EnablePhysics
			( (pBone->GetBoneFlags() & S3DModelBoneSpace::flagBonePhysics) != 0 ) ;
		pBoneSer->DisableCollision
			( (pBone->GetBoneFlags() & S3DModelBoneSpace::flagNoCollision) != 0 ) ;
		pBoneSer->PhysMaterial() = pBone->GetBonePhysicalMaterial() ;
		pBoneSer->SetPhysMaterialRef( pBone->GetBonePhysicalMaterialID() ) ;
		//
		AddEditorItemChild( comp, spaceParent, *pstrBoneID, pBoneSer ) ;
		//
		ssoaBoneMap.SetAs( *pstrBoneID, new SString(pBoneSer->GetItemIdentity()) ) ;
		//
		BuildEditorBonesFromModel( comp, *pBoneSer, ssoaBoneMap, model, pBone, param ) ;
	}
}

void S3DMeshEditorSerializer::BuildEditorBonePhysFromModel
	( S3DSceneComposer::Composition & comp,
		S3DSceneComposer::SpaceSerializer & spaceParent,
		S3DModelBuffer& model,
		const BuildMeshEditorItemsParam& param )
{
	SStrSortArray<S3DModelBoneSpace::PhysMaterial>&
				ssaPhysMaterials = model.GetPhysMaterialList() ;
	for ( size_t i = 0; i < ssaPhysMaterials.GetLength(); i ++ )
	{
		const SString *	pstrID = ssaPhysMaterials.GetTagAt(i) ;
		S3DModelBoneSpace::PhysMaterial *
						pPhysMat = ssaPhysMaterials.GetAt(i) ;
		if ( (pstrID == nullptr) || (pPhysMat == nullptr) )
		{
			continue ;
		}
		S3DBonePhysMaterialSerializer *
					pPhysMatSer = new S3DBonePhysMaterialSerializer ;
		pPhysMatSer->PhysMaterial() = *pPhysMat ;
		//
		AddEditorItemChild( comp, spaceParent, *pstrID, pPhysMatSer ) ;
	}
}

void S3DMeshEditorSerializer::BuildEditorMarkersFromModel
	( S3DSceneComposer::Composition & comp,
		S3DSceneComposer::SpaceSerializer & spaceParent,
		const SSystem::SStrSortObjectArray<SSystem::SString>& ssoaBoneMap,
		S3DModelBuffer& model,
		const BuildMeshEditorItemsParam& param )
{
	for ( size_t i = 0; i < model.GetMarkerInfoCount(); i ++ )
	{
		const SSystem::SString *	pstrID = model.GetMarkerInfoIdentityAt( i ) ;
		S3DModelData::MarkerInfo *	pMarker = model.GetMarkerInfoAt( i ) ;
		if ( (pstrID == nullptr) || (pMarker == nullptr) )
		{
			continue ;
		}
		S3DMatrix	matRotation ;
		pMarker->m_qRotation.ToMatrix( matRotation ) ;
		//
		S3DMarkerItemSerializer *	pMarkerSer = new S3DMarkerItemSerializer ;
		pMarkerSer->SetMarkerType( pMarker->m_type ) ;
		pMarkerSer->SetShapeType( pMarker->m_shape ) ;
		pMarkerSer->SetColliderClass( pMarker->m_iCollider ) ;
		pMarkerSer->SetMarkerDirection( pMarker->m_vDirection ) ;
		pMarkerSer->SetMarkerRadius( pMarker->m_fpRadius ) ;
		pMarkerSer->SetItemPositioin( pMarker->m_vPosition ) ;
		pMarkerSer->SetItemRotation( S3DDMatrix( matRotation ) ) ;
		pMarkerSer->SetItemZoom( pMarker->m_vSize ) ;
		//
		S3DSceneComposer::SpaceSerializer *	pParentBone = nullptr ;
		if ( !pMarker->m_strRefBone.IsEmpty() )
		{
			SString *	pstrBoneID = ssoaBoneMap.GetAs( pMarker->m_strRefBone ) ;
			if ( pstrBoneID != nullptr )
			{
				pParentBone =
					ESLTypeCast<S3DSceneComposer::SpaceSerializer>
								( comp.GetSceneItemAs( *pstrBoneID ) ) ;
			}
		}
		if ( pParentBone == nullptr )
		{
			pParentBone = &spaceParent ;
		}
		AddEditorItemChild( comp, *pParentBone, *pstrID, pMarkerSer ) ;
	}
}

void S3DMeshEditorSerializer::BuildEditorMeshsFromModel
	( S3DSceneComposer::Composition & comp,
		S3DSceneComposer::SpaceSerializer & spaceParent,
		const SSystem::SStrSortObjectArray<SSystem::SString>& ssoaBoneMap,
		S3DModelBuffer& model,
		const BuildMeshEditorItemsParam& param )
{
	for ( size_t i = 0; i < model.GetMeshCount(); i ++ )
	{
		SString			strMeshID ;
		const SString *	pstrMeshID = model.GetMeshGroupNameIndexOf( i ) ;
		if ( pstrMeshID == nullptr )
		{
			strMeshID.Format( L"mesh%d", i ) ;
			pstrMeshID = &strMeshID ;
		}
		else
		{
			strMeshID = *pstrMeshID ;
		}
		S3DModelBuffer::MeshObject *	pMeshObj = model.GetMeshObjectAt( i ) ;
		if ( pMeshObj == nullptr )
		{
			continue ;
		}
		S3DMeshEditorSerializer *	pMeshSer = new S3DMeshEditorSerializer ;
		AddEditorItemChild( comp, spaceParent, strMeshID, pMeshSer ) ;
		//
		pMeshSer->AttachMaterial
			( 0, nullptr, model.GetMaterialLibrary().
							GetMaterialIdentityOf( pMeshObj->m_pMaterial ) ) ;
		//
		pMeshSer->BuildMeshVertexFromModel( model, strMeshID, *pMeshObj, param ) ;
		if ( pMeshSer->BuildMeshBoneRefFromModel( model, *pMeshObj, ssoaBoneMap, param ) )
		{
			pMeshSer->EnableBone( true ) ;
		}
		pMeshSer->BuildMeshMorphFromModel( model, *pMeshObj, param ) ;
		//
		pMeshSer->UpdateMaterialRef() ;
		pMeshSer->UpdateBoneRef() ;
	}
}

void S3DMeshEditorSerializer::BuildMeshVertexFromModel
	( S3DModelBuffer& model,
		const wchar_t * pwszName,
		const S3DModelBuffer::MeshObject& meshObj,
		const BuildMeshEditorItemsParam& param )
{
	S3DMeshBufferPropertySerializer::MeshBuffer	mesh ;
	mesh.strName = pwszName ;
	mesh.typeMesh = primitiveTriangle ;
	mesh.countPrimitive = (uint32_t) meshObj.m_countPolygon ;
	mesh.countVertex = (uint32_t) meshObj.m_countVertex ;
	mesh.countIndex = (uint32_t) meshObj.m_countPolygon * 3 ;
	mesh.bufVertex.AddArray
		( model.GetVertexBufferAt( meshObj.m_iVertex ), mesh.countVertex ) ;
	mesh.bufNormal.AddArray
		( model.GetNormalBufferAt( meshObj.m_iNormal ), mesh.countVertex ) ;
	mesh.bufUVMap = meshObj.m_bufUVMap ;
	mesh.bufColor = meshObj.m_bufColor ;
	mesh.bufIndex = meshObj.m_bufIndex ;
	//
	S3DMeshEditor::Patch *	pPatch = NewPatch() ;
	pPatch->ConvertFromMeshBuffer( mesh ) ;
	AddPatch( pPatch ) ;
	//
	if ( meshObj.m_bufExAttrElements.GetLength()
				>= meshObj.m_nExAttrElements * mesh.countVertex )
	{
		for ( size_t i = 0; i < meshObj.m_nExAttrElements; i ++ )
		{
			SString	strLayerName ;
			strLayerName.Format( L"ex_attr%d", i ) ;
			m_mesh.InsertWeightLayerAt( i, strLayerName ) ;
			//
			ESLAssert( pPatch->GetWeightLayerCount() > i ) ;
			for ( size_t j = 0; j < mesh.countVertex; j ++ )
			{
				pPatch->SetWeightAt
					( j, i, meshObj.m_bufExAttrElements.At
								( j * meshObj.m_nExAttrElements + i ) ) ;
			}
		}
	}
}

bool S3DMeshEditorSerializer::BuildMeshBoneRefFromModel
	( S3DModelBuffer& model,
		const S3DModelBuffer::MeshObject& meshObj,
		const SSystem::SStrSortObjectArray<SSystem::SString>& ssoaBoneMap,
		const BuildMeshEditorItemsParam& param )
{
	S3DMeshEditor::Patch *	pPatch = m_mesh.GetPatchAt(0) ;
	if ( pPatch == nullptr )
	{
		return	false ;
	}
	bool	flagRefBone = false ;
	for ( size_t i = 0; i < meshObj.m_arrRelBone.GetLength(); i ++ )
	{
		const S3DModelData::BONE_LINK_INFO& bli = meshObj.m_arrRelBone.At(i) ;
		const SString *	pstrBoneID = model.GetBoneIdentityOf( bli.pRelBone ) ;
		if ( pstrBoneID == nullptr )
		{
			continue ;
		}
		pstrBoneID = ssoaBoneMap.GetAs( *pstrBoneID ) ;
		if ( pstrBoneID == nullptr )
		{
			continue ;
		}
		size_t	iLayer = m_mesh.GetWeightLayerCount() ;
		m_mesh.InsertWeightLayerAt( iLayer, *pstrBoneID ) ;
		//
		S3DMeshEditor::WeightMapInfo	wmi ;
		wmi.nFlags = S3DMeshEditor::weightBone ;
		wmi.mat4Bone = bli.matIMesh.Inverse() ;
		m_mesh.SetWeightLayerInfoAt( iLayer, wmi ) ;
		//
		for ( size_t j = 0; j < meshObj.m_countVertex; j ++ )
		{
			pPatch->SetWeightAt
				( j, iLayer,
					bli.pRelBone->GetBoneWeightAt( meshObj.m_iVertex + j ) ) ;
		}
		flagRefBone = true ;
	}
	return	flagRefBone ;
}

void S3DMeshEditorSerializer::BuildMeshMorphFromModel
	( S3DModelBuffer& model,
		const S3DModelBuffer::MeshObject& meshObj,
		const BuildMeshEditorItemsParam& param )
{
	S3DMeshEditor::Patch *	pSrcPatch = m_mesh.GetPatchAt(0) ;
	if ( pSrcPatch == nullptr )
	{
		return ;
	}
	for ( size_t i = 0; i < meshObj.m_arrMorphTarget.GetLength(); i ++ )
	{
		SString *	pstrMorphID = meshObj.m_arrMorphTarget.GetAt(i) ;
		if ( pstrMorphID == nullptr )
		{
			continue ;
		}
		S3DModelBuffer::MorphTargetMesh *
				pmtmMorph = model.GetMorhTargetAs( *pstrMorphID ) ;
		if ( pmtmMorph == nullptr )
		{
			continue ;
		}
		S3DMorphMeshController *	pMorphCtrl = new S3DMorphMeshController ;
		pMorphCtrl->MeshEditor().DuplicateMesh( m_mesh ) ;
		pMorphCtrl->MeshEditor().RemoveWeightLayer
			( 0, pMorphCtrl->GetMeshEditor().GetWeightLayerCount() ) ;
		pMorphCtrl->MeshEditor().InsertWeightLayerAt( 0, L"morph_mask" ) ;
		//
		S3DMeshEditor::Patch *	pDstPatch = pMorphCtrl->MeshEditor().GetPatchAt(0) ;
		if ( (pDstPatch != nullptr)
			&& (pmtmMorph->m_countVertex == pDstPatch->GetTotalVertexCount()) )
		{
			for ( size_t j = 0; j < pmtmMorph->m_countVertex; j ++ )
			{
				if ( j < pmtmMorph->m_bufVertex.GetLength() )
				{
					pDstPatch->SetPointAt( j, pmtmMorph->m_bufVertex.At(j) ) ;
				}
				if ( j < pmtmMorph->m_bufNormal.GetLength() )
				{
					pDstPatch->SetNormalAt( j, pmtmMorph->m_bufNormal.At(j) ) ;
				}
				if ( j < pmtmMorph->m_bufUVMap.GetLength() )
				{
					pDstPatch->SetUVAt( j, pmtmMorph->m_bufUVMap.At(j) ) ;
				}
				if ( j < pmtmMorph->m_bufColor.GetLength() )
				{
					pDstPatch->SetColorAt( j, pmtmMorph->m_bufColor.At(j) ) ;
				}
				if ( j < pmtmMorph->m_bufWeight.GetLength() )
				{
					pDstPatch->SetWeightAt( j, 0, pmtmMorph->m_bufWeight.At(j) ) ;
				}
				else
				{
					pDstPatch->SetWeightAt( j, 0, 1.0f ) ;
				}
			}
			pMorphCtrl->SetMeshAutoSync( true ) ;
		}
		pMorphCtrl->SetItemIdentity( *pstrMorphID ) ;
		AddController( pMorphCtrl ) ;
	}
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
double S3DMeshEditorSerializer::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramMeshEdgeAngle:
		return	m_mesh.GetMeshParameter().fpEdgeAngle ;
	}
	return	ItemBasicSerializer::GetScalarParameter( i ) ;
}

int32_t S3DMeshEditorSerializer::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramMeshDivHorz:
		return	(int32_t) m_mesh.GetMeshParameter().nDivHorz ;
	case	paramMeshDivVert:
		return	(int32_t) m_mesh.GetMeshParameter().nDivVert ;
	case	paramColliderFlags:
		return	(int32_t) m_maskCollisionFlags ;
	case	paramCollisionAlpha:
		return	m_nCollisionAlpha ;
	case	paramColDivHorz:
		return	m_nCollisionDivHorz ;
	case	paramColDivVert:
		return	m_nCollisionDivVert ;
	}
	return	ItemBasicSerializer::GetIntegerParameter( i ) ;
}

bool S3DMeshEditorSerializer::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramMeshEdgeByAngle:
		return	(m_mesh.GetMeshParameter().nFlags
					& S3DMeshEditor::flagMeshEdgeByAngle) != 0 ;
	case	paramMeshDivAhead:
		return	m_flagDivMeshAhead ;
	case	paramFreezeMesh:
		return	m_flagFreezeMesh ;
	case	paramCollision:
		return	m_flagCollision ;
	case	paramCollisionAll:
		return	m_flagCollisionAll ;
	case	paramEnableBone:
		return	m_flagEnableBone ;
	case	paramSortInstance:
		return	(m_instancing.GetSorting() != S3DItemInstancingSerializer::sortNothing) ;
	}
	return	ItemBasicSerializer::GetBooleanParameter( i ) ;
}

const wchar_t * S3DMeshEditorSerializer::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramMeshEditor:
		return	((S3DMeshEditorSerializer*)this)->m_mesh.Serialize() ;
	case	paramMeshPrimitiveType:
		if ( (m_mesh.GetMeshParameter().nFlags
							& S3DMeshEditor::flagMeshPrimitiveType)
			&& (m_mesh.GetMeshParameter().nPrimitiveType == primitivePoint) )
		{
			return	L"point" ;
		}
		else if ( (m_mesh.GetMeshParameter().nFlags
							& S3DMeshEditor::flagMeshPrimitiveType)
			&& (m_mesh.GetMeshParameter().nPrimitiveType == primitiveLine) )
		{
			return	(m_mesh.GetMeshParameter().nFlags
						& S3DMeshEditor::flagMeshEdgeSingleLine)
												? L"edge" : L"line" ;
		}
		else
		{
			return	L"triangle" ;
		}
	case	paramMeshEdgeMethod:
		return	SXMLDocument::GetSymbolAsIntegerOf
				( m_aiMeshEdgeMethods,
					(m_mesh.GetMeshParameter().nFlags
							& S3DMeshEditor::flagMeshEdgeMethodMask) ) ;
	case	paramMeshDivMethod:
		return	SXMLDocument::GetSymbolAsIntegerOf
				( m_aiMeshDivMethods, m_mesh.GetMeshParameter().nDivMethod ) ;
	case	paramMaterial0:
		return	m_strMaterialIDs[0] ;
	case	paramMaterial1:
		return	m_strMaterialIDs[1] ;
	case	paramMaterial2:
		return	m_strMaterialIDs[2] ;
	case	paramMaterial3:
		return	m_strMaterialIDs[3] ;
	case	paramBoneRoot:
		return	m_strBoneRoot ;
	case	paramInstancing:
		return	m_instancing.GetInstancingEntriesBase64() ;
	}
	return	ItemBasicSerializer::GetCommandParameter( i ) ;
}

size_t S3DMeshEditorSerializer::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t i ) const
{
	switch ( i )
	{
	case	paramMeshEditor:
		{
			size_t	nHeaderBytes ;
			size_t	nTotalBytes =
						((S3DMeshEditorSerializer*)this)->
									m_mesh.SerializeBuffer( nHeaderBytes ) ;
			if ( pDst == nullptr )
			{
				return	nTotalBytes ;
			}
			if ( nBufBytes == sizeof(S3DSceneComposer::BinaryHeader) )
			{
				S3DSceneComposer::BinaryHeader *
					pbh = (S3DSceneComposer::BinaryHeader*) pDst ;
				pbh->nType = S3DSceneComposer::binaryMeshEditor ;
				pbh->nSubType = 0 ;
				pbh->nBodyBytes =
					(uint32_t) (nTotalBytes - sizeof(S3DSceneComposer::BinaryHeader)) ;
				pbh->nReserved = 0 ;
				return	sizeof(S3DSceneComposer::BinaryHeader) ;
			}
			if ( nBufBytes == nTotalBytes )
			{
				((S3DMeshEditorSerializer*)this)->
					m_mesh.SerializeBinary
						( (uint8_t*) pDst, nTotalBytes, nHeaderBytes ) ;
				return	nTotalBytes ;
			}
		}
		return	0 ;

	case	paramInstancing:
		if ( pDst == nullptr )
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
void S3DMeshEditorSerializer::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramMeshEdgeAngle:
		if ( m_mesh.GetMeshParameter().fpEdgeAngle != (float32_t) s )
		{
			m_mesh.MeshParameter().fpEdgeAngle = (float32_t) s ;
			//
			if ( m_mesh.GetMeshParameter().nFlags
							& S3DMeshEditor::flagMeshEdgeByAngle )
			{
				m_mesh.SetUpdateVertexFlag() ;
			}
		}
		return ;
	}
	ItemBasicSerializer::SetScalarParameter( i, s ) ;
}

void S3DMeshEditorSerializer::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramMeshDivHorz:
		if ( m_mesh.GetMeshParameter().nDivHorz != (uint32_t) n )
		{
			m_mesh.MeshParameter().nDivHorz = (uint32_t) esl_min( n, 8 ) ;
			m_mesh.SetUpdateVertexFlag() ;
		}
		return ;

	case	paramMeshDivVert:
		if ( m_mesh.GetMeshParameter().nDivVert != (uint32_t) n )
		{
			m_mesh.MeshParameter().nDivVert = (uint32_t) esl_min( n, 8 ) ;
			m_mesh.SetUpdateVertexFlag() ;
		}
		return ;

	case	paramColliderFlags:
		m_maskCollisionFlags = (uint32_t) n ;
		return ;

	case	paramCollisionAlpha:
		m_nCollisionAlpha = n ;
		return ;

	case	paramColDivHorz:
		if ( m_nCollisionDivHorz != (uint32_t) n )
		{
			m_nCollisionDivHorz = (uint32_t) esl_min( n, 8 ) ;
			m_mesh.SetUpdateVertexFlag() ;
		}
		return ;

	case	paramColDivVert:
		if ( m_nCollisionDivVert != (uint32_t) n )
		{
			m_nCollisionDivVert = (uint32_t) esl_min( n, 8 ) ;
			m_mesh.SetUpdateVertexFlag() ;
		}
		return ;
	}
	ItemBasicSerializer::SetIntegerParameter( i, n ) ;
}

void S3DMeshEditorSerializer::SetBooleanParameter( size_t i, bool b )
{
	uint32_t						nFlags ;
	S3DSceneComposer::Composition *	pComp ;
	S3DBoneSerializer *				pBone ;
	switch ( i )
	{
	case	paramMeshEdgeByAngle:
		nFlags = (m_mesh.GetMeshParameter().nFlags
						& ~S3DMeshEditor::flagMeshEdgeByAngle)
					| (b ? S3DMeshEditor::flagMeshEdgeByAngle : 0) ;
		if ( m_mesh.GetMeshParameter().nFlags != nFlags )
		{
			m_mesh.MeshParameter().nFlags = nFlags ;
			m_mesh.SetUpdateVertexFlag() ;
		}
		return ;
	case	paramMeshDivAhead:
		m_flagDivMeshAhead = b ;
		return ;
	case	paramCollision:
		m_flagCollision = b ;
		m_flagCollisionUpdate = true ;
		return ;
	case	paramCollisionAll:
		m_flagCollisionAll = b ;
		m_flagCollisionUpdate = true ;
		return ;
	case	paramFreezeMesh:
		m_flagFreezeMesh = b ;
		if ( !b )
		{
			m_flagMeshUpdated = false ;
		}
		return ;
	case	paramEnableBone:
		m_flagEnableBone = b ;
		if ( b )
		{
			m_mesh.SetUpdateVertexFlag() ;
		}
		return ;
	case	paramSortInstance:
		m_instancing.SetSorting
			( b ? S3DItemInstancingSerializer::sortAllItems
					: S3DItemInstancingSerializer::sortNothing ) ;
		return ;

	case	paramBoneAutoMap:
		pComp = GetComposition() ;
		if ( pComp != nullptr )
		{
			DoClearBoneWeightLayers() ;
			//
			pBone = ESLTypeCast<S3DBoneSerializer>
						( pComp->GetSceneSpaceAs( m_strBoneRoot ) ) ;
			if ( pBone != nullptr )
			{
				DoAutoMapBoneWeightLayers( pComp, pBone ) ;
			}
		}
		return ;

	case	paramBoneClearMap:
		DoClearBoneWeightLayers() ;
		return ;

	case	paramBoneMatrixMap:
		pComp = GetComposition() ;
		if ( pComp != nullptr )
		{
			DoInitBoneMatrix( pComp ) ;
		}
		return ;

	case	paramBoneNormalizeWeight:
		DoNormalizeBoneWeightMap() ;
		return ;

	case	paramBoneOptimizeWeight:
		DoOptimizeBoneWeightMap() ;
		return ;

	case	paramBakeInstanceToMesh:
		DoBakeInstanceToMeshEditor() ;
		return ;
	}
	ItemBasicSerializer::SetBooleanParameter( i, b ) ;
}

void S3DMeshEditorSerializer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	uint32_t	nFlags, nMethod ;
	size_t		iMaterial ;
	switch ( i )
	{
	case	paramMeshEditor:
		m_mesh.Deserialize( pwszCmd ) ;
		m_flagCollisionUpdate = true ;
		return ;

	case	paramMeshPrimitiveType:
		if ( SString::Compare( pwszCmd, L"point" ) == 0 )
		{
			m_mesh.MeshParameter().nFlags |= S3DMeshEditor::flagMeshPrimitiveType ;
			m_mesh.MeshParameter().nPrimitiveType = primitivePoint ;
		}
		else if ( SString::Compare( pwszCmd, L"line" ) == 0 )
		{
			m_mesh.MeshParameter().nFlags |= S3DMeshEditor::flagMeshPrimitiveType ;
			m_mesh.MeshParameter().nFlags &= ~S3DMeshEditor::flagMeshEdgeSingleLine ;
			m_mesh.MeshParameter().nPrimitiveType = primitiveLine ;
		}
		else if ( SString::Compare( pwszCmd, L"edge" ) == 0 )
		{
			m_mesh.MeshParameter().nFlags |= S3DMeshEditor::flagMeshPrimitiveType
											| S3DMeshEditor::flagMeshEdgeSingleLine ;
			m_mesh.MeshParameter().nPrimitiveType = primitiveLine ;
		}
		else
		{
			m_mesh.MeshParameter().nFlags &= ~(S3DMeshEditor::flagMeshPrimitiveType
												| S3DMeshEditor::flagMeshEdgeSingleLine) ;
			m_mesh.MeshParameter().nPrimitiveType = primitiveTriangle ;
		}
		m_mesh.SetUpdateVertexFlag() ;
		m_flagCollisionUpdate = true ;
		return ;

	case	paramMeshEdgeMethod:
		nFlags = (m_mesh.GetMeshParameter().nFlags
						& ~S3DMeshEditor::flagMeshEdgeMethodMask)
				| (uint32_t) SXMLDocument::GetIntegerAsSymbolOf
					( m_aiMeshEdgeMethods, pwszCmd,
						(m_mesh.GetMeshParameter().nFlags
								& S3DMeshEditor::flagMeshEdgeMethodMask) ) ;
		if ( m_mesh.GetMeshParameter().nFlags != nFlags )
		{
			m_mesh.MeshParameter().nFlags = nFlags ;
			m_mesh.SetUpdateVertexFlag() ;
		}
		return ;

	case	paramMeshDivMethod:
		nMethod = (uint32_t) SXMLDocument::GetIntegerAsSymbolOf
					( m_aiMeshDivMethods, pwszCmd,
							m_mesh.GetMeshParameter().nDivMethod ) ;
		if ( m_mesh.GetMeshParameter().nDivMethod != nMethod )
		{
			m_mesh.MeshParameter().nDivMethod = nMethod ;
			m_mesh.SetUpdateVertexFlag() ;
		}
		return ;

	case	paramMaterial0:
	case	paramMaterial1:
	case	paramMaterial2:
	case	paramMaterial3:
		iMaterial = i - paramMaterial0 ;
		if ( m_strMaterialIDs[iMaterial] != pwszCmd )
		{
			m_strMaterialIDs[iMaterial] = pwszCmd ;
			UpdateMaterialRef() ;
		}
		return ;

	case	paramBoneRoot:
		if ( m_strBoneRoot != pwszCmd )
		{
			m_strBoneRoot = pwszCmd ;
			UpdateBoneRef() ;
		}
		return ;

	case	paramInstancing:
		m_instancing.SetInstancingEntriesBase64( pwszCmd ) ;
		m_flagCollisionUpdate = true ;
		return ;
	}
	ItemBasicSerializer::SetCommandParameter( i, pwszCmd ) ;
}

size_t S3DMeshEditorSerializer::SetBinaryParameter
	( size_t i, const void * pSrc, size_t nBufBytes )
{
	switch ( i )
	{
	case	paramMeshEditor:
		m_mesh.DeserializeBinary( pSrc, nBufBytes ) ;
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
bool S3DMeshEditorSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	int	j ;
	S3DSceneComposer::Composition *	pComp ;
	switch ( i )
	{
	case	paramMeshPrimitiveType:
		aStrSet.Add( new SString( L"point" ) ) ;
		aStrSet.Add( new SString( L"edge" ) ) ;
		aStrSet.Add( new SString( L"line" ) ) ;
		aStrSet.Add( new SString( L"triangle" ) ) ;
		return	true ;

	case	paramMeshEdgeMethod:
		for ( j = 0; m_aiMeshEdgeMethods[j].pszSymbol != nullptr; j ++ )
		{
			aStrSet.Add( new SString(m_aiMeshEdgeMethods[j].pszSymbol) ) ;
		}
		return	true ;

	case	paramMeshDivMethod:
		for ( j = 0; m_aiMeshDivMethods[j].pszSymbol != nullptr; j ++ )
		{
			aStrSet.Add( new SString(m_aiMeshDivMethods[j].pszSymbol) ) ;
		}
		return	true ;

	case	paramMaterial0:
	case	paramMaterial1:
	case	paramMaterial2:
	case	paramMaterial3:
		pComp = GetComposition() ;
		if ( pComp != nullptr )
		{
			S3DSceneComposer *	pSceneComp = pComp->GetSceneComposer() ;
			if ( pSceneComp != nullptr )
			{
				pSceneComp->Assets().EnumerateMaterialStringSet( aStrSet ) ;
			}
		}
		return	true ;

	case	paramBoneRoot:
		pComp = GetComposition() ;
		if ( pComp != nullptr )
		{
			pComp->EnumerateItemIDsAs
				( aStrSet, ESL_RUNTIME_CLASS(S3DBoneSerializer) ) ;
		}
		return	true ;
	}
	return	ItemBasicSerializer::EnumerateStringSet( i, aStrSet ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DMeshEditorSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	1:
		return	L"メッシュ" ;
	}
	return	ItemBasicSerializer::GetParameterCategoryName( iCategory ) ;
}

// アイテム作用の追加処理
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::OnUpdateBehavior( S3DScene& scene )
{
	ItemBasicSerializer::OnUpdateBehavior( scene ) ;
	//
	m_instancing.ResetDynamicInstancingEntries() ;
	m_instancing.UpdateDynamicInstancingEntries( this ) ;
	//
	if ( m_flagsBehavior & (S3DScene::itemVisible | S3DScene::itemCollision) )
	{
		ProcessMeshControllers( scene ) ;
	}
}

// アイテムのプライマリモデル取得
//////////////////////////////////////////////////////////////////////////////
S3DVertexBufferInterface * S3DMeshEditorSerializer::GetItemPrimaryModel( void )
{
	return	&m_vbEditMesh[0] ;
}

// アイテムのコリジョンバッファ取得
//////////////////////////////////////////////////////////////////////////////
S3DCollider * S3DMeshEditorSerializer::GetItemPrimaryCollider( void )
{
	return	&m_collision[0] ;
}

// 拡張的な処理の通知
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::OnExtendNotify
	( const wchar_t * pwszCmd, const wchar_t * pwszParam,
		const void * pExParam, size_t nExParamBytes )
{
	ItemBasicSerializer::OnExtendNotify( pwszCmd, pwszParam, pExParam, nExParamBytes ) ;
	//
	if ( SString::Compare( pwszCmd, S3DSceneComposer::CmdInitializeItem ) == 0 )
	{
		S3DSceneComposer::Composition *	pComp = GetComposition() ;
		if ( (pComp != nullptr) && !pComp->IsEditMode() && !m_flagFreezeMesh )
		{
			S3DSceneComposer *	pComposer = pComp->GetComposer() ;
			if ( pComposer != nullptr )
			{
				SString	strMsg ;
				strMsg.Format( L"\'%s\' はメッシュ固定化されていません。\n", GetItemIdentity() ) ;
				pComposer->OutputTraceLog( strMsg ) ;
			}
		}
	}
}

// レンダリングの為のデバイスリソース準備
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::OnPrepareToRender
	( S3DRenderDevice * pDevice, uint32_t nFlags )
{
	ItemBasicSerializer::OnPrepareToRender( pDevice, nFlags ) ;
	//
	if ( m_flagFreezeMesh )
	{
		S3DScene *	pScene = GetScene() ;
		if ( pScene != nullptr )
		{
			ProcessMeshControllers( *pScene ) ;
			//
			for ( int i = 0; i < paramMaterialCount; i ++ )
			{
				if ( m_vbEditMesh[i].GetMeshCount() > 0 )
				{
					pDevice->CommitDeviceVertexBuffer( m_vbEditMesh[i], 10 ) ;
				}
			}
		}
	}
}

// フレームを適用
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::SetFrameParameters
		( double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	ItemBasicSerializer::SetFrameParameters( fpFrame, seek ) ;
	//
	if ( seek == S3DSceneComposer::seekJumpReset )
	{
		m_instancing.ResetDynamicInstancingEntries() ;
		m_instancing.UpdateDynamicInstancingEntries( this ) ;
		m_instancing.NotifyResetPotentialInstance( this ) ;
	}
}

// レンダリング前後処理（全視点共通）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::OnItemRenderEvent
	( S3DScene& scene, S3DScene::ItemClass clsItem )
{
	ItemBasicSerializer::OnItemRenderEvent( scene, clsItem ) ;
	//
	if ( clsItem == S3DScene::classPreRender2 )
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
				( scene, matdModel, vdModel, 0.0f, (float32_t) PI ) ;
		}
	}
}

// 当たり判定追加
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::OnItemRenderCollision
	( const S3DScene& scene, S3DCollision& render )
{
	ItemBasicSerializer::OnItemRenderCollision( scene, render ) ;
	//
	if ( m_flagCollision | m_flagCollisionAll )
	{
		if ( m_flagCollisionUpdate )
		{
			UpdateEditorCollision( m_mesh ) ;
		}
		render.SetUserClassesMask( m_maskCollisionFlags ) ;
		render.BeginBatchBuild() ;
		//
		const S4DMatrix *	pmatStcInstancing ;
		const S3DColor *	pclrStcInstancing ;
		size_t	nStaticInstanceCount =
			m_instancing.GetStaticInstancingArray
					( pmatStcInstancing, pclrStcInstancing ) ;
		//
		const S4DMatrix *	pmatDynInstancing ;
		const S3DColor *	pclrDynInstancing ;
		size_t	nDynamicInstanceCount =
			m_instancing.GetDynamicInstancingArray
					( pmatDynInstancing, pclrDynInstancing ) ;
		//
		size_t	i, j ;
		for ( j = 0; j < paramMaterialCount; j ++ )
		{
			if ( m_nColBufCount[j] == 0 )
			{
				continue ;
			}
			for ( i = 0; i < nStaticInstanceCount; i ++ )
			{
				if ( (int32_t) pclrStcInstancing[i].rgbMul.argb.Alpha >= m_nCollisionAlpha )
				{
					render.AddColliderObject
						( &m_collision[j], pmatStcInstancing + i, i ) ;
				}
			}
			for ( i = 0; i < nDynamicInstanceCount; i ++ )
			{
				if ( (int32_t) pclrDynInstancing[i].rgbMul.argb.Alpha >= m_nCollisionAlpha )
				{
					render.AddColliderObject
						( &m_collision[j],
							pmatDynInstancing + i, nStaticInstanceCount + i ) ;
				}
			}
		}
		render.EndBatchBuild() ;
	}
}

// 表示モデル追加
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::OnItemRenderModel
	( const S3DScene& scene,
		S3DRenderContextInterface& render,
		uint64_t flagsExclusion )
{
	ItemBasicSerializer::OnItemRenderModel( scene, render, flagsExclusion ) ;
	//
	const S4DMatrix *	pmatInstancing ;
	const S3DColor *	pclrInstancing ;
	const S4DMatrix *	pmatDynInstancing ;
	const S3DColor *	pclrDynInstancing ;
	size_t				nInstanceCount, nDynInstanceCount = 0 ;
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
		nDynInstanceCount =
			m_instancing.GetDynamicInstancingArray
					( pmatDynInstancing, pclrDynInstancing ) ;
	}
	for ( int i = 0; i < paramMaterialCount; i ++ )
	{
		if ( (m_nMeshBufCount[i] == 0) || (m_pMaterials[i] == nullptr) )
		{
			continue ;
		}
		if ( m_pMaterials[i]->m_attrSurface.flagsShading & flagsExclusion )
		{
			continue ;
		}
		if ( nInstanceCount > 0 )
		{
			render.AddVertexBuffer
				( m_pMaterials[i], 0, &(m_vbEditMesh[i]), 0, -1,
					nInstanceCount, pmatInstancing, pclrInstancing ) ;
		}
		if ( nDynInstanceCount > 0 )
		{
			render.AddVertexBuffer
				( m_pMaterials[i], 0, &(m_vbEditMesh[i]), 0, -1,
					nDynInstanceCount, pmatDynInstancing, pclrDynInstancing ) ;
		}
	}
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DMeshEditorSerializer::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp, uint32_t nFlags )
{
	uint32_t	nResFlags =
		ItemBasicSerializer::UpdatePropertyReference( comp, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefResource )
	{
		UpdateMaterialRef() ;
	}
	if ( nFlags & S3DSceneComposer::updateRefItem )
	{
		UpdateBoneRef() ;
	}
	return	nResFlags ;
}

// インスタンス処理（classPreRender で呼び出す）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::AddDynamicInstancingEntries
	( const S4DMatrix * pMatrixs,
		const S3DColor * pColors,
		size_t nCount, ESLObject * pSrcItem )
{
	m_csLock.Lock() ;
	m_instancing.AddDynamicInstancingEntries
				( pMatrixs, pColors, nCount, pSrcItem ) ;
	m_csLock.Unlock() ;
}

// S3DItemInstancingSerializer 取得
//////////////////////////////////////////////////////////////////////////////
S3DItemInstancingSerializer * S3DMeshEditorSerializer::GetInstancing( void )
{
	return	&m_instancing ;
}

// S3DMeshEditor 取得
//////////////////////////////////////////////////////////////////////////////
const S3DMeshEditor& S3DMeshEditorSerializer::GetMeshEditor( void ) const
{
	return	m_mesh ;
}

S3DMeshEditor& S3DMeshEditorSerializer::MeshEditor( void )
{
	return	m_mesh ;
}

// 表示用メッシュの更新
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::UpdateViewMesh( void )
{
	S3DScene *	pScene = GetScene() ;
	if ( pScene != nullptr )
	{
		ProcessMeshControllers( *pScene ) ;
	}
}

// 表示用マテリアルの取得
//////////////////////////////////////////////////////////////////////////////
S3DMaterial * S3DMeshEditorSerializer::GetMeshMaterial( size_t iMaterial ) const
{
	if ( iMaterial < paramMaterialCount )
	{
		return	m_pMaterials[iMaterial] ;
	}
	return	nullptr ;
}

// 空間
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::GetMeshItemMatrix( S3DDMatrix& mat, S3DDVector& pos ) const
{
	GetGlobalTransformation( mat, pos ) ;
}

// アニメーション長取得
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditorSerializer::GetTargetAnimationLength( double& secLength ) const
{
	secLength = 0.0 ;
	return	false ;
}

// 全フレーム数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DMeshEditorSerializer::GetTargetAnimationFrames( void ) const
{
	return	1 ;
}

// ターゲット空間（逆変換用）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::GetTargetSpaceTransformation
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
void S3DMeshEditorSerializer::AddParticles
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
		if ( pZooms != nullptr )
		{
			zx = pZooms[i] ;
			zy = zx ;
		}
		if ( pxAspect != nullptr )
		{
			zx *= pxAspect[i] ;
		}
		S3DMatrix	mat3( zx, zy, zy ) ;
		if ( pFaceDirs != nullptr )
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
		if ( pSrcColors != nullptr )
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
bool S3DMeshEditorSerializer::IsUsingIndexedParticles( void )
{
	return	true ;
}

// パーティクル追加（高機能）（classPreRender で呼び出す）
//////////////////////////////////////////////////////////////////////////////
void S3DMeshEditorSerializer::AddIndexedParticles
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
		if ( pFaceDirs != nullptr )
		{
			mat4.SetMatrix3( pFaceDirs[i] ) ;
		}
		mat4.SetTranslation( pvPoints[pIndexes[i].nIndex] ) ;
		pMatrix[i] = mat4 ;
		//
		S3DColor	color( 0xFFFFFFFF, 0 ) ;
		if ( pSrcColors != nullptr )
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

// オーナーアイテム取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ItemSerializer * S3DMeshEditorSerializer::GetOwnerItem( void ) const
{
	return	(S3DMeshEditorSerializer*) this ;
}

// メッシュ同期
//////////////////////////////////////////////////////////////////////////////
bool S3DMeshEditorSerializer::IsSynchronizedMeshEditor( void ) const
{
	return	true ;
}



//////////////////////////////////////////////////////////////////////////////
// マーカーアイテム
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DMarkerItemSerializer::m_paramEntries
		[S3DMarkerItemSerializer::paramMarkerCount] =
{
	{ L"marker_type",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration
			| S3DSceneComposer::attrDynamicValidation,
		L"マーカータイプ", nullptr },
	{ L"shape_type",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration
			| S3DSceneComposer::attrDynamicValidation,
		L"形状タイプ", nullptr },
	{ L"collider_class",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1,
		L"当たり判定クラス", L"-1,0～15\n-1:無指定\n"
		L"0～15:ボーン当たり判定対象番号（ボーン側がビットマスクで対象集合を指定）\n"
		L"ボーン以外の場合；\n"
		L"0:形状・移動障壁, 1:移動障壁, 2:敵当たり判定領域, "
		L"3:敵の攻撃当たり判定, 4:イベント発生, 5～7:予約領域, 8～:ユーザー定義" },
	{ L"direction",
		S3DSceneComposer::typeDirection,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrNoLocalTransform,
		L"方向ベクトル", nullptr },
	{ L"marker_radius",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,
		L"マーカー半径", nullptr },
	{ L"marker_length",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,
		L"マーカー長", nullptr },
} ;

const S3DSceneComposer::ParamSetClass
	S3DMarkerItemSerializer::m_pscClass =
{
	&ItemBasicSerializer::m_pscClass,
	S3DMarkerItemSerializer::paramMarkerCount,
	&S3DMarkerItemSerializer::m_paramEntries[0]
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DMarkerItemSerializer, ItemBasicSerializer )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DMarkerItemSerializer, marker )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DMarkerItemSerializer::S3DMarkerItemSerializer( void )
	: ItemBasicSerializer
		( m_ItemClassDescriptor.pwszClassID,
				&S3DMarkerItemSerializer::m_pscClass ),
		m_type( S3DModelData::MarkerInfo::typeBoneColider ),
		m_shape( S3DModelData::MarkerInfo::shapeSphere ),
		m_iCollider( -1 ), m_vDirection( 0, 0, 1 ),
		m_fpRadius( 1.0 ), m_fpLength( 1.0 )
{
	m_flagsBehavior |= S3DScene::itemCollision ;
	m_classItem = S3DScene::classStaticItem1 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DMarkerItemSerializer::~S3DMarkerItemSerializer( void )
{
}

// マーカータイプ
//////////////////////////////////////////////////////////////////////////////
S3DModelData::MarkerInfo::Type S3DMarkerItemSerializer::GetMarkerType( void ) const
{
	return	m_type ;
}

void S3DMarkerItemSerializer::SetMarkerType( S3DModelData::MarkerInfo::Type type )
{
	m_type = type ;
	if ( type != S3DModelData::MarkerInfo::typePosition )
	{
		m_flagsBehavior |= S3DScene::itemCollision ;
	}
	else
	{
		m_flagsBehavior &= ~S3DScene::itemCollision ;
	}
}

// 形状
//////////////////////////////////////////////////////////////////////////////
S3DModelData::MarkerInfo::Shape S3DMarkerItemSerializer::GetShapeType( void ) const
{
	return	m_shape ;
}

void S3DMarkerItemSerializer::SetShapeType( S3DModelData::MarkerInfo::Shape shape )
{
	m_shape = shape ;
}

// 当たり判定クラス
//////////////////////////////////////////////////////////////////////////////
int32_t S3DMarkerItemSerializer::GetColliderClass( void ) const
{
	return	m_iCollider ;
}

void S3DMarkerItemSerializer::SetColliderClass( int32_t iCollider )
{
	m_iCollider = iCollider ;
}

// 方向
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DMarkerItemSerializer::GetMarkerDirection( void ) const
{
	return	m_vDirection ;
}

void S3DMarkerItemSerializer::SetMarkerDirection( const S3DDVector& vDir )
{
	m_vDirection = vDir ;
}

// 半径
//////////////////////////////////////////////////////////////////////////////
double S3DMarkerItemSerializer::GetMarkerRadius( void ) const
{
	return	m_fpRadius ;
}

void S3DMarkerItemSerializer::SetMarkerRadius( double fpRadius )
{
	m_fpRadius = fpRadius ;
}

// 長さ
//////////////////////////////////////////////////////////////////////////////
double S3DMarkerItemSerializer::GetMarkerLength( void ) const
{
	return	m_fpLength ;
}

void S3DMarkerItemSerializer::SetMarkerLength( double fpLength )
{
	m_fpLength = fpLength ;
}

// マーカー情報
//////////////////////////////////////////////////////////////////////////////
void S3DMarkerItemSerializer::GetMarkerInfo( S3DModelData::MarkerInfo& marker ) const
{
	marker.m_type = m_type ;
	marker.m_shape = m_shape ;
	marker.m_iCollider = m_iCollider ;
	marker.m_qRotation.FromMatrix( S3DMatrix( GetItemRotation() ) ) ;
	marker.m_vPosition = GetItemPosition() ;
	marker.m_vDirection = m_vDirection ;
	marker.m_vSize = GetItemZoom() ;
	marker.m_fpRadius = (float32_t) m_fpRadius ;
	marker.m_fpLength = (float32_t) m_fpLength ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DMarkerItemSerializer::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramDirection:
		return	GetMarkerDirection() ;
	}
	return	ItemBasicSerializer::GetVectorParameter( i ) ;
}

double S3DMarkerItemSerializer::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRadius:
		return	GetMarkerRadius() ;
	case	paramLength:
		return	GetMarkerLength() ;
	}
	return	ItemBasicSerializer::GetScalarParameter( i ) ;
}

int32_t S3DMarkerItemSerializer::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramColliderClass:
		return	GetColliderClass() ;
	}
	return	ItemBasicSerializer::GetIntegerParameter( i ) ;
}

const wchar_t * S3DMarkerItemSerializer::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramMarkerType:
		return	S3DModelData::MarkerInfo::m_pwszTypeTags[m_type] ;

	case	paramShapeType:
		return	S3DModelData::MarkerInfo::m_pwszShapeIDs[m_shape] ;
	}
	return	ItemBasicSerializer::GetCommandParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DMarkerItemSerializer::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramDirection:
		SetMarkerDirection( vec ) ;
		return ;
	}
	ItemBasicSerializer::SetVectorParameter( i, vec ) ;
}

void S3DMarkerItemSerializer::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramRadius:
		SetMarkerRadius( s ) ;
		return ;
	case	paramLength:
		SetMarkerLength( s ) ;
		return ;
	}
	ItemBasicSerializer::SetScalarParameter( i, s ) ;
}

void S3DMarkerItemSerializer::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramColliderClass:
		SetColliderClass( n ) ;
		return ;
	}
	ItemBasicSerializer::SetIntegerParameter( i, n ) ;
}

void S3DMarkerItemSerializer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	size_t	j ;
	switch ( i )
	{
	case	paramMarkerType:
		for ( j = 0; j < S3DModelData::MarkerInfo::typeCount; j ++ )
		{
			if ( SString::Compare
				( S3DModelData::MarkerInfo::m_pwszTypeTags[j], pwszCmd ) == 0 )
			{
				SetMarkerType( (S3DModelData::MarkerInfo::Type) j ) ;
				break ;
			}
		}
		return ;

	case	paramShapeType:
		for ( j = 0; j < S3DModelData::MarkerInfo::shapeCount; j ++ )
		{
			if ( SString::Compare
				( S3DModelData::MarkerInfo::m_pwszShapeIDs[j], pwszCmd ) == 0 )
			{
				SetShapeType( (S3DModelData::MarkerInfo::Shape) j ) ;
				break ;
			}
		}
		return ;
	}
	ItemBasicSerializer::SetCommandParameter( i, pwszCmd ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DMarkerItemSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	size_t	j ;
	switch ( i )
	{
	case	paramMarkerType:
		for ( j = 0; j < S3DModelData::MarkerInfo::typeCount; j ++ )
		{
			aStrSet.Add( new SString
				( S3DModelData::MarkerInfo::m_pwszTypeTags[j] ) ) ;
		}
		return	true ;

	case	paramShapeType:
		for ( j = 0; j < S3DModelData::MarkerInfo::shapeCount; j ++ )
		{
			aStrSet.Add( new SString
				( S3DModelData::MarkerInfo::m_pwszShapeIDs[j] ) ) ;
		}
		return	true ;
	}
	return	ItemBasicSerializer::EnumerateStringSet( i, aStrSet ) ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DMarkerItemSerializer::IsParameterValidation( size_t i ) const
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
//	case	paramUseCollision:
	case	paramGlobalSpace:
	case	paramCameraShift:
	case	paramCameraSpace:
	case	paramHideNear:
	case	paramHideFar:
		return	false ;

	case	paramDirection:
		return	(m_type == S3DModelData::MarkerInfo::typePosition)
				|| (m_shape == S3DModelData::MarkerInfo::shapeTube) ;

	case	paramShapeType:
		return	(m_type != S3DModelData::MarkerInfo::typePosition) ;

	case	paramRadius:
		return	(m_shape == S3DModelData::MarkerInfo::shapeSphere)
				|| (m_shape == S3DModelData::MarkerInfo::shapeTube) ;

	case	paramLength:
		return	(m_shape == S3DModelData::MarkerInfo::shapeTube) ;
	}
	return	ItemBasicSerializer::IsParameterValidation( i ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DMarkerItemSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"マーカー" ;
	}
	return	ItemBasicSerializer::GetParameterCategoryName( iCategory ) ;
}

// 当たり判定追加
//（必要であれば scene.PhysicsScene() へ Actor の追加も行う）
//////////////////////////////////////////////////////////////////////////////
void S3DMarkerItemSerializer::OnItemRenderCollision
	( const S3DScene& scene, S3DCollision& render )
{
	if ( m_iCollider >= 0 )
	{
		render.SetUserClassesMask( 1 << m_iCollider ) ;
	}
	if ( m_shape == S3DModelData::MarkerInfo::shapeSphere )
	{
		S3DVector	vPos( 0, 0, 0 ) ;
		render.AddSolidSphere( vPos, (float32_t) m_fpRadius, 0 ) ;
	}
	else if ( m_shape == S3DModelData::MarkerInfo::shapeCube )
	{
		S3DVector	vPos( 0, 0, 0 ) ;
		S3DVector	vSize( 1, 1, 1 ) ;
		render.AddSolidCube( vPos, vSize, 0 ) ;
	}
	else if ( m_shape == S3DModelData::MarkerInfo::shapeTube )
	{
		S3DVector	vPoints[2] ;
		vPoints[0] = S3DVector( 0, 0, 0 ) ;
		vPoints[1] = m_vDirection.Normalized() * m_fpLength ;
		render.AddSolidTubeList( vPoints, 2, (float32_t) m_fpRadius ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// モーフィング・メッシュコントローラー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DMorphMeshController, MeshController, S3DMeshEditorBridge )
S3D_IMPLEMENT_COMPOSER_ITEM( S3DMorphMeshController, morph_mesh )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DMorphMeshController::S3DMorphMeshController( void )
	: MeshController( m_ItemClassDescriptor.pwszClassID ),
		m_fpBlendParam( 0.0 ), m_flagAutoSync( true )
{
	PrepareParameterEntryCount( paramCount ) ;
	AddParameterEntry
		( L"mesh_editor", S3DSceneComposer::typeBinary,
			S3DSceneComposer::attrConstant,
			L"メッシュエディタ", nullptr ) ;
	AddParameterEntry
		( L"blend_param", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"適用度", nullptr, 0.0, 1.0 ) ;
	AddParameterEntry
		( L"auto_sync_mesh", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"メッシュ同期", nullptr ) ;
	//
	m_mesh.AttachExtraMeshInfo( this ) ;
}

// 適用度
//////////////////////////////////////////////////////////////////////////////
double S3DMorphMeshController::GetBlendParam( void ) const
{
	return	m_fpBlendParam ;
}

void S3DMorphMeshController::SetBlendParam( double fpBlend )
{
	m_fpBlendParam = fpBlend ;
}

// メッシュ同期
//////////////////////////////////////////////////////////////////////////////
bool S3DMorphMeshController::IsMeshAutoSync( void ) const
{
	return	m_flagAutoSync ;
}

void S3DMorphMeshController::SetMeshAutoSync( bool flagSync )
{
	m_flagAutoSync = flagSync ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
double S3DMorphMeshController::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramBlendParam:
		return	m_fpBlendParam ;
	}
	return	0.0 ;
}

bool S3DMorphMeshController::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramAutoSync:
		return	m_flagAutoSync ;
	}
	return	false ;
}

const wchar_t * S3DMorphMeshController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramMorphMesh:
		return	((S3DMorphMeshController*)this)->m_mesh.Serialize() ;
	}
	return	nullptr ;
}

size_t S3DMorphMeshController::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t i ) const
{
	switch ( i )
	{
	case	paramMorphMesh:
		{
			size_t	nHeaderBytes ;
			size_t	nTotalBytes =
						((S3DMorphMeshController*)this)->
									m_mesh.SerializeBuffer( nHeaderBytes ) ;
			if ( pDst == nullptr )
			{
				return	nTotalBytes ;
			}
			if ( nBufBytes == sizeof(S3DSceneComposer::BinaryHeader) )
			{
				S3DSceneComposer::BinaryHeader *
					pbh = (S3DSceneComposer::BinaryHeader*) pDst ;
				pbh->nType = S3DSceneComposer::binaryMeshEditor ;
				pbh->nSubType = 0 ;
				pbh->nBodyBytes =
					(uint32_t) (nTotalBytes - sizeof(S3DSceneComposer::BinaryHeader)) ;
				pbh->nReserved = 0 ;
				return	sizeof(S3DSceneComposer::BinaryHeader) ;
			}
			if ( nBufBytes == nTotalBytes )
			{
				((S3DMorphMeshController*)this)->
					m_mesh.SerializeBinary
						( (uint8_t*) pDst, nTotalBytes, nHeaderBytes ) ;
				return	nTotalBytes ;
			}
		}
		break ;
	}
	return	0 ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DMorphMeshController::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramBlendParam:
		m_fpBlendParam = s ;
		return ;
	}
}

void S3DMorphMeshController::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramAutoSync:
		m_flagAutoSync = b ;
		return ;
	}
}

void S3DMorphMeshController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramMorphMesh:
		m_mesh.Deserialize( pwszCmd ) ;
		return ;
	}
}

size_t S3DMorphMeshController::SetBinaryParameter
	( size_t i, const void * pSrc, size_t nBufBytes )
{
	switch ( i )
	{
	case	paramMorphMesh:
		m_mesh.DeserializeBinary( pSrc, nBufBytes ) ;
		return	nBufBytes ;
	}
	return	0 ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DMorphMeshController::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"モーフィング" ;
	}
	return	nullptr ;
}

// メッシュの変形
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditor * S3DMorphMeshController::ModifyMeshEditor
	( S3DMeshEditor& meshTempBuf, const S3DMeshEditor& meshOrgRef )
{
	if ( fabs(m_fpBlendParam) < 0.00001 )
	{
		return	&meshTempBuf ;
	}
	for ( size_t iPatch = 0; iPatch < meshTempBuf.GetPatchCount(); iPatch ++ )
	{
		if ( (iPatch >= meshOrgRef.GetPatchCount())
			|| (iPatch >= m_mesh.GetPatchCount()) )
		{
			break ;
		}
		S3DMeshEditor::Patch *	pDstPatch = meshTempBuf.GetPatchAt( iPatch ) ;
		S3DMeshEditor::Patch *	pOrgPatch = meshOrgRef.GetPatchAt( iPatch ) ;
		S3DMeshEditor::Patch *	pMorphPatch = m_mesh.GetPatchAt( iPatch ) ;
		ESLAssert( pDstPatch != nullptr ) ;
		ESLAssert( pOrgPatch != nullptr ) ;
		ESLAssert( pMorphPatch != nullptr ) ;
		if ( (pDstPatch == nullptr)
			|| (pOrgPatch == nullptr)
			|| (pMorphPatch == nullptr)
			|| (pOrgPatch->GetFlags() & S3DMeshEditor::Patch::flagDisableModifier) )
		{
			continue ;
		}
		const size_t	nVertexCount = pDstPatch->GetTotalVertexCount() ;
		const size_t	nWeightCount = pMorphPatch->GetWeightLayerCount() ;
		if ( (pOrgPatch->GetTotalVertexCount() != nVertexCount)
			|| (pMorphPatch->GetTotalVertexCount() != nVertexCount) )
		{
			continue ;
		}
		const S3DVector *
			pvDstPoitns = pDstPatch->GetConstPointArray( 0, nVertexCount ) ;
		const S3DVector *
			pvOrgPoitns = pOrgPatch->GetConstPointArray( 0, nVertexCount ) ;
		const S3DVector *
			pvMorphPoitns = pMorphPatch->GetConstPointArray( 0, nVertexCount ) ;
		const S2DVector *
			pvDstUVs = pDstPatch->GetConstUVArray( 0, nVertexCount ) ;
		const S2DVector *
			pvOrgUVs = pOrgPatch->GetConstUVArray( 0, nVertexCount ) ;
		const S2DVector *
			pvMorphUVs = pMorphPatch->GetConstUVArray( 0, nVertexCount ) ;
		//
		for ( size_t i = 0; i < nVertexCount; i ++ )
		{
			float32_t	w = (float32_t) m_fpBlendParam ;
			if ( nWeightCount >= 1 )
			{
				w *= pMorphPatch->GetWeightAt( i, 0 ) ;
			}
			if ( w != 0.0f )
			{
				pDstPatch->SetPointAt
					( i, pvDstPoitns[i]
							+ (pvMorphPoitns[i] - pvOrgPoitns[i]) * w ) ;
				pDstPatch->SetUVAt
					( i, pvDstUVs[i]
							+ (pvMorphUVs[i] - pvOrgUVs[i]) * w ) ;
			}
		}
		meshTempBuf.SetUpdateVertexFlag() ;
	}
	return	&meshTempBuf ;
}

// S3DMeshEditor 取得
//////////////////////////////////////////////////////////////////////////////
const S3DMeshEditor& S3DMorphMeshController::GetMeshEditor( void ) const
{
	return	m_mesh ;
}

S3DMeshEditor& S3DMorphMeshController::MeshEditor( void )
{
	return	m_mesh ;
}

// 表示用メッシュの更新
//////////////////////////////////////////////////////////////////////////////
void S3DMorphMeshController::UpdateViewMesh( void )
{
	S3DMeshEditorSerializer *
		pOwnerMeshEditor =
			ESLTypeCast<S3DMeshEditorSerializer>( GetOwnerItem() ) ;
	if ( pOwnerMeshEditor != nullptr )
	{
		S3DScene *	pScene = pOwnerMeshEditor->GetScene() ;
		if ( pScene != nullptr )
		{
			pOwnerMeshEditor->ProcessMeshControllers( *pScene ) ;
		}
	}
}

// 表示用マテリアルの取得
//////////////////////////////////////////////////////////////////////////////
S3DMaterial * S3DMorphMeshController::GetMeshMaterial( size_t iMaterial ) const
{
	S3DMeshEditorInterface *
		pOwnerMeshEditor =
			ESLTypeCast<S3DMeshEditorInterface>( GetOwnerItem() ) ;
	if ( pOwnerMeshEditor != nullptr )
	{
		return	pOwnerMeshEditor->GetMeshMaterial( iMaterial ) ;
	}
	return	nullptr ;
}

// 空間
//////////////////////////////////////////////////////////////////////////////
void S3DMorphMeshController::GetMeshItemMatrix( S3DDMatrix& mat, S3DDVector& pos ) const
{
	S3DSceneComposer::ItemSerializer *	pOwnerItem = GetOwnerItem() ;
	if ( pOwnerItem != nullptr )
	{
		pOwnerItem->GetGlobalTransformation( mat, pos ) ;
	}
	else
	{
		mat = S3DDMatrix( 1, 1, 1 ) ;
		pos = S3DDVector( 0, 0, 0 ) ;
	}
}

// 値更新通知
//////////////////////////////////////////////////////////////////////////////
void S3DMorphMeshController::NotifyUpdateVertex
		( const S3DMeshEditor::SelectPointSet& selPoints )
{
	for ( size_t i = 0; i < selPoints.GetLength(); i ++ )
	{
		S3DMeshEditor::SelectPoint	sp = selPoints.At(i) ;
		if ( sp.pPatch->GetWeightLayerCount() >= 1 )
		{
			float32_t	w = sp.pPatch->GetWeightAt( sp.iVertex, 0 ) ;
			if ( w < sp.fpWeight )
			{
				sp.pPatch->SetWeightAt( sp.iVertex, 0, sp.fpWeight ) ;
			}
		}
	}
//	S3DMeshEditorBridge::NotifyUpdateVertex( selPoints ) ;
}

// オーナーアイテム取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ItemSerializer * S3DMorphMeshController::GetOwnerItem( void ) const
{
	return	MeshController::GetOwnerItem() ;
}

// メッシュ同期
//////////////////////////////////////////////////////////////////////////////
bool S3DMorphMeshController::IsSynchronizedMeshEditor( void ) const
{
	return	m_flagAutoSync ;
}

// 値更新通知
//////////////////////////////////////////////////////////////////////////////
void S3DMorphMeshController::OnNotifyUpdateVertex
	( const S3DMeshEditor::SelectPointSet& selPointsIndexed,
		const S3DMeshEditor& meshSrc )
{
	S3DMeshEditor::Patch::Elements	el0, el1 ;
	el0.nWeights = 0 ;
	el0.pWeights = nullptr ;
	el1.nWeights = 0 ;
	el1.pWeights = nullptr ;
	//
	ESLAssert( IsSynchronizedMeshEditor() ) ;
	S3DMeshEditor&	meshDst = MeshEditor() ;
	for ( size_t i = 0; i < selPointsIndexed.GetLength(); i ++ )
	{
		S3DMeshEditor::SelectPoint	sp = selPointsIndexed.At(i) ;
		const size_t			iPatch = (size_t) ((ulong_ptr_t) sp.pPatch) ;
		S3DMeshEditor::Patch *	pSrcPatch = meshSrc.GetPatchAt( iPatch ) ;
		S3DMeshEditor::Patch *	pDstPatch = meshDst.GetPatchAt( iPatch ) ;
		if ( (pSrcPatch != nullptr) && (pDstPatch != nullptr) )
		{
			ESLAssert( sp.iVertex < pSrcPatch->GetTotalVertexCount() ) ;
			if ( sp.iVertex < pDstPatch->GetTotalVertexCount() )
			{
				float32_t	w = sp.fpWeight ;
				if ( pDstPatch->GetWeightLayerCount() >= 1 )
				{
					w *= 1.0f - pDstPatch->GetWeightAt( sp.iVertex, 0 ) ;
				}
				if ( w > 0.00001f )
				{
					pSrcPatch->GetElementsAt( el0, sp.iVertex ) ;
					if ( w < 0.99999f )
					{
						pDstPatch->GetElementsAt( el1, sp.iVertex ) ;
						S3DMeshEditor::Patch::LerpElements( el0, el1, 1.0f - w ) ;
					}
					pDstPatch->SetElementsAt( sp.iVertex, el0 ) ;
				}
			}
		}
	}
	meshDst.SetUpdateVertexFlag() ;
}
	


//////////////////////////////////////////////////////////////////////////////
// 法線制御メッシュコントローラー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DNormalMeshController, MeshController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DNormalMeshController, mesh_normal )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DNormalMeshController::S3DNormalMeshController( void )
	: MeshController( m_ItemClassDescriptor.pwszClassID ),
		m_vNormal( 0, 0, -1 ), m_fpCoverage( 1.0 )
{
	PrepareParameterEntryCount( paramCount ) ;
	ESLVerify( paramRefWightMap == AddParameterEntry
		( L"ref_weight_layer", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration,
			L"参照ウェイトマップ", nullptr ) ) ;
	ESLVerify( paramNormal == AddParameterEntry
		( L"normal", S3DSceneComposer::typeDirection, 0,
			L"法線", nullptr ) ) ;
	ESLVerify( paramCoverage == AddParameterEntry
		( L"coverage", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"適用度", nullptr, 0.0, 1.0 ) ) ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DNormalMeshController::GetVectorParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramNormal:
		return	m_vNormal ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DNormalMeshController::GetScalarParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramCoverage:
		return	m_fpCoverage ;
	}
	return	0.0 ;
}

const wchar_t * S3DNormalMeshController::GetCommandParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramRefWightMap:
		return	m_strRefWeight ;
	}
	return	nullptr ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DNormalMeshController::SetVectorParameter( size_t iParam, const S3DDVector& vec )
{
	switch ( iParam )
	{
	case	paramNormal:
		m_vNormal = vec ;
		return ;
	}
}

void S3DNormalMeshController::SetScalarParameter( size_t iParam, double s )
{
	switch ( iParam )
	{
	case	paramCoverage:
		m_fpCoverage = s ;
		return ;
	}
}

void S3DNormalMeshController::SetCommandParameter( size_t iParam, const wchar_t * pwszCmd )
{
	switch ( iParam )
	{
	case	paramRefWightMap:
		m_strRefWeight = pwszCmd ;
		return ;
	}
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DNormalMeshController::EnumerateStringSet
	( size_t iParam, SSystem::SStringArray& aStrSet )
{
	S3DMeshEditorSerializer *	pMeshEditor ;
	switch ( iParam )
	{
	case	paramRefWightMap:
		pMeshEditor = ESLTypeCast<S3DMeshEditorSerializer>( GetOwnerItem() ) ;
		if ( pMeshEditor != nullptr )
		{
			const S3DMeshEditor&	mesh = pMeshEditor->GetMeshEditor() ;
			for ( size_t j = 0; j < mesh.GetWeightLayerCount(); j ++ )
			{
				aStrSet.Add( new SString( mesh.GetWeightLayerIDAt(j) ) ) ;
			}
		}
		return	true ;
	}
	return	false ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DNormalMeshController::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"法線" ;
	}
	return	nullptr ;
}

// メッシュの変形
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditor * S3DNormalMeshController::ModifyMeshEditor
	( S3DMeshEditor& meshTempBuf, const S3DMeshEditor& meshOrgRef )
{
	size_t	iWeightLayer = 0 ;
	for ( size_t i = 0; i < meshOrgRef.GetWeightLayerCount(); i ++ )
	{
		if ( m_strRefWeight == meshOrgRef.GetWeightLayerIDAt(i) )
		{
			iWeightLayer = i ;
			break ;
		}
	}
	if ( iWeightLayer >= meshOrgRef.GetWeightLayerCount() )
	{
		return	&meshTempBuf ;
	}
	const S3DVector	vNormal = m_vNormal ;
	const float32_t	fpCoverage = (float32_t) m_fpCoverage ;
	for ( size_t iPatch = 0; iPatch < meshOrgRef.GetPatchCount(); iPatch ++ )
	{
		if ( iPatch >= meshTempBuf.GetPatchCount() )
		{
			break ;
		}
		S3DMeshEditor::Patch *	pDstPatch = meshTempBuf.GetPatchAt( iPatch ) ;
		S3DMeshEditor::Patch *	pSrcPatch = meshOrgRef.GetPatchAt( iPatch ) ;
		ESLAssert( pDstPatch != nullptr ) ;
		ESLAssert( pSrcPatch != nullptr ) ;
		if ( pSrcPatch->GetFlags() & S3DMeshEditor::Patch::flagDisableModifier )
		{
			continue ;
		}
		const size_t	nVertexCount = pSrcPatch->GetTotalVertexCount() ;
		if ( pDstPatch->GetTotalVertexCount() != nVertexCount )
		{
			continue ;
		}
		pDstPatch->UpdatePatchMesh() ;
		pSrcPatch->SetFlags
			( pSrcPatch->GetFlags() | S3DMeshEditor::Patch::flagFreezeNormal ) ;
		//
		const float32_t *
			pfpWeight = pSrcPatch->GetConstWeightArrayAt( iWeightLayer ) ;
		for ( size_t i = 0; i < nVertexCount; i ++ )
		{
			float32_t	w = esl_fclampf( pfpWeight[i] * fpCoverage, 0.0f, 1.0f ) ;
			S3DVector	vSrc = pDstPatch->GetNormalAt( i ) ;
			S3DVector	vDst = vSrc + (vNormal - vSrc) * w ;
			pDstPatch->SetNormalAt( i, vDst ) ;
		}
	}
	meshTempBuf.SetUpdateVertexFlag() ;
	return	&meshTempBuf ;
}



//////////////////////////////////////////////////////////////////////////////
// ディスプレイスメント・メッシュコントローラー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DDisplacementMeshController, MeshController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DDisplacementMeshController, mesh_displacement )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DDisplacementMeshController::S3DDisplacementMeshController( void )
	: MeshController( m_ItemClassDescriptor.pwszClassID ),
		m_fpHeight( 1.0 ), m_fpBaseWeight( 0.0 )
{
	PrepareParameterEntryCount( paramCount ) ;
	AddParameterEntry
		( L"ref_weight_layer", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration,
			L"参照ウェイトマップ", nullptr ) ;
	AddParameterEntry
		( L"disp_scale", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"押し出しスケール", nullptr, 0.0, 10.0 ) ;
	AddParameterEntry
		( L"base_weight", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"基準ウェイト値", nullptr, 0.0, 1.0 ) ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
double S3DDisplacementMeshController::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramHeight:
		return	m_fpHeight ;
	case	paramBaseWeight:
		return	m_fpBaseWeight ;
	}
	return	0.0 ;
}

const wchar_t * S3DDisplacementMeshController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRefWightMap:
		return	m_strRefWeight ;
	}
	return	nullptr ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DDisplacementMeshController::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramHeight:
		m_fpHeight = s ;
		return ;
	case	paramBaseWeight:
		m_fpBaseWeight = s ;
		return ;
	}
}

void S3DDisplacementMeshController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramRefWightMap:
		m_strRefWeight = pwszCmd ;
		return ;
	}
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DDisplacementMeshController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	S3DMeshEditorSerializer *	pMeshEditor ;
	switch ( i )
	{
	case	paramRefWightMap:
		pMeshEditor = ESLTypeCast<S3DMeshEditorSerializer>( GetOwnerItem() ) ;
		if ( pMeshEditor != nullptr )
		{
			const S3DMeshEditor&	mesh = pMeshEditor->GetMeshEditor() ;
			for ( size_t j = 0; j < mesh.GetWeightLayerCount(); j ++ )
			{
				aStrSet.Add( new SString( mesh.GetWeightLayerIDAt(j) ) ) ;
			}
		}
		return	true ;
	}
	return	false ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DDisplacementMeshController::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"ディスプレイスメント" ;
	}
	return	nullptr ;
}

// メッシュの変形
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditor * S3DDisplacementMeshController::ModifyMeshEditor
	( S3DMeshEditor& meshTempBuf, const S3DMeshEditor& meshOrgRef )
{
	size_t	iWeightLayer = 0 ;
	for ( size_t i = 0; i < meshOrgRef.GetWeightLayerCount(); i ++ )
	{
		if ( m_strRefWeight == meshOrgRef.GetWeightLayerIDAt(i) )
		{
			iWeightLayer = i ;
			break ;
		}
	}
	if ( iWeightLayer >= meshOrgRef.GetWeightLayerCount() )
	{
		return	&meshTempBuf ;
	}
	const float32_t	fpBaseWeight = (float32_t) m_fpBaseWeight ;
	const float32_t	fpHeightScale = (float32_t) m_fpHeight ;
	for ( size_t iPatch = 0; iPatch < meshOrgRef.GetPatchCount(); iPatch ++ )
	{
		if ( iPatch >= meshTempBuf.GetPatchCount() )
		{
			break ;
		}
		S3DMeshEditor::Patch *	pDstPatch = meshTempBuf.GetPatchAt( iPatch ) ;
		S3DMeshEditor::Patch *	pSrcPatch = meshOrgRef.GetPatchAt( iPatch ) ;
		ESLAssert( pDstPatch != nullptr ) ;
		ESLAssert( pSrcPatch != nullptr ) ;
		if ( pSrcPatch->GetFlags() & S3DMeshEditor::Patch::flagDisableModifier )
		{
			continue ;
		}
		const size_t	nVertexCount = pSrcPatch->GetTotalVertexCount() ;
		if ( pDstPatch->GetTotalVertexCount() != nVertexCount )
		{
			continue ;
		}
		pDstPatch->UpdatePatchMesh() ;
		//
		const float32_t *
			pfpWeight = pSrcPatch->GetConstWeightArrayAt( iWeightLayer ) ;
		for ( size_t i = 0; i < nVertexCount; i ++ )
		{
			float32_t	w = (pfpWeight[i] - fpBaseWeight) * fpHeightScale ;
			pDstPatch->SetPointAt
				( i, pDstPatch->GetPointAt(i) + pDstPatch->GetNormalAt(i) * w ) ;
		}
	}
	meshTempBuf.SetUpdateVertexFlag() ;
	return	&meshTempBuf ;
}



//////////////////////////////////////////////////////////////////////////////
// 鏡映反転メッシュコントローラー
//////////////////////////////////////////////////////////////////////////////

const SSystem::SXMLDocument::AttrInteger
	S3DMirrorMeshController::m_aiMirrorAxis[4] =
{
	{ L"x", mirrorX },
	{ L"y", mirrorY },
	{ L"z", mirrorZ },
	{ nullptr, 0 },
} ;

const SSystem::SXMLDocument::AttrInteger
	S3DMirrorMeshController::m_aiNameRule[4] =
{
	{ L"inclusive", ruleInclusive },
	{ L"match_lead", ruleMatchLead },
	{ L"match_end", ruleMatchEnd },
	{ nullptr, 0 },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DMirrorMeshController, MeshController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DMirrorMeshController, mesh_mirror )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DMirrorMeshController::S3DMirrorMeshController( void )
	: MeshController( m_ItemClassDescriptor.pwszClassID ),
		m_vBasePos( 0, 0, 0 ), m_mirrorAxis( mirrorX ),
		m_flagSeam( true ), m_fpSeamGap( 0.0 ),
		m_flagMirrorWeight( true ), m_ruleWightName( ruleInclusive ),
		m_strRightName( L"右" ), m_strLeftName( L"左" )
{
	PrepareParameterEntryCount( paramCount ) ;
	ESLVerify( paramBasePosition == AddParameterEntry
		( L"base_position", S3DSceneComposer::typePosition,
			S3DSceneComposer::attrConstant,
			L"基準座標", nullptr ) ) ;
	ESLVerify( paramMirrorAxis == AddParameterEntry
		( L"mirror_axis", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration,
			L"反転軸", nullptr ) ) ;
	ESLVerify( paramSeamFlag == AddParameterEntry
		( L"seam_flag", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"近接頂点縮退", nullptr ) ) ;
	ESLVerify( paramSeamGap == AddParameterEntry
		( L"seam_gap", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"縮退許容距離", nullptr, 0.0, 1.0 ) ) ;
	ESLVerify( paramMirrorWeight == AddParameterEntry
		( L"mirror_weight", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrDynamicValidation,
			L"ウェイトマップ自動反転", nullptr ) ) ;
	ESLVerify( paramWeightNameRule == AddParameterEntry
		( L"weight_name_rule", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration,
			L"反転名規則", nullptr ) ) ;
	ESLVerify( paramWeightRightName == AddParameterEntry
		( L"weight_right_name", S3DSceneComposer::typeCommand,
			S3DSceneComposer::attrConstant,
			L"ウェイト右側名", nullptr ) ) ;
	ESLVerify( paramWeightLeftName == AddParameterEntry
		( L"weight_left_name", S3DSceneComposer::typeCommand,
			S3DSceneComposer::attrConstant,
			L"ウェイト左側名", nullptr ) ) ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DMirrorMeshController::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramBasePosition:
		return	m_vBasePos ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DMirrorMeshController::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramSeamGap:
		return	m_fpSeamGap ;
	}
	return	0.0 ;
}

bool S3DMirrorMeshController::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramSeamFlag:
		return	m_flagSeam ;
	case	paramMirrorWeight:
		return	m_flagMirrorWeight ;
	}
	return	false ;
}

const wchar_t * S3DMirrorMeshController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramMirrorAxis:
		return	SXMLDocument::GetSymbolAsIntegerOf( m_aiMirrorAxis, m_mirrorAxis ) ;
	case	paramWeightNameRule:
		return	SXMLDocument::GetSymbolAsIntegerOf( m_aiNameRule, m_ruleWightName ) ;
	case	paramWeightRightName:
		return	m_strRightName ;
	case	paramWeightLeftName:
		return	m_strLeftName ;
	}
	return	nullptr ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DMirrorMeshController::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramBasePosition:
		m_vBasePos = vec ;
		return ;
	}
}

void S3DMirrorMeshController::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramSeamGap:
		m_fpSeamGap = s ;
		return ;
	}
}

void S3DMirrorMeshController::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramSeamFlag:
		m_flagSeam = b ;
		return ;
	case	paramMirrorWeight:
		m_flagMirrorWeight = b ;
		return ;
	}
}

void S3DMirrorMeshController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramMirrorAxis:
		m_mirrorAxis = (MirrorAxis)
			SXMLDocument::GetIntegerAsSymbolOf
				( m_aiMirrorAxis, pwszCmd, m_mirrorAxis ) ;
		return ;
	case	paramWeightNameRule:
		m_ruleWightName = (WeightNameRule)
			SXMLDocument::GetIntegerAsSymbolOf
				( m_aiNameRule, pwszCmd, m_ruleWightName ) ;
		return ;
	case	paramWeightRightName:
		m_strRightName = pwszCmd ;
		return ;
	case	paramWeightLeftName:
		m_strLeftName = pwszCmd ;
		return ;
	}
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DMirrorMeshController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	switch ( i )
	{
	case	paramMirrorAxis:
		{
			for ( size_t j = 0; m_aiMirrorAxis[j].pszSymbol != nullptr; j ++ )
			{
				aStrSet.Add( new SString(m_aiMirrorAxis[j].pszSymbol) ) ;
			}
		}
		return	true ;

	case	paramWeightNameRule:
		{
			for ( size_t j = 0; m_aiNameRule[j].pszSymbol != nullptr; j ++ )
			{
				aStrSet.Add( new SString(m_aiNameRule[j].pszSymbol) ) ;
			}
		}
		return	true ;
	}
	return	false ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DMirrorMeshController::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramWeightNameRule:
	case	paramWeightRightName:
	case	paramWeightLeftName:
		return	m_flagMirrorWeight ;
	}
	return	true ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DMirrorMeshController::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"鏡映反転" ;
	}
	return	nullptr ;
}

// メッシュの変形
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditor * S3DMirrorMeshController::ModifyMeshEditor
	( S3DMeshEditor& meshTempBuf, const S3DMeshEditor& meshOrgRef )
{
	//
	// 反転行列
	//
	S3DMatrix	matMirror( 1, 1, 1 ) ;
	S3DVector	vBasePos = m_vBasePos ;
	switch ( m_mirrorAxis )
	{
	case	mirrorX:
	default:
		matMirror.m[0][0] = - matMirror.m[0][0] ;
		break ;
	case	mirrorY:
		matMirror.m[1][1] = - matMirror.m[1][1] ;
		break ;
	case	mirrorZ:
		matMirror.m[2][2] = - matMirror.m[2][2] ;
		break ;
	}
	S3DMatrix	matIMirror = matMirror.Inverse() ;
	vBasePos -= matMirror * vBasePos ;
	//
	// ボーンマップ反転準備
	//
	SArray<size_t>	aMirrorLayerPair ;
	if ( m_flagMirrorWeight && (GetOwnerItem() != nullptr) )
	{
		S3DDMatrix	matMesh ;
		S3DDVector	vMesh ;
		GetOwnerItem()->GetGlobalTransformation( matMesh, vMesh ) ;
		//
		S4DMatrix	mat4Mesh( 1, 1, 1, 1 ) ;
		mat4Mesh.SetMatrix3( S3DMatrix( matMesh ) ) ;
		mat4Mesh.SetTranslation( S3DVector( vMesh ) ) ;
		//
		S4DMatrix	mat4IMesh = mat4Mesh.Inverse() ;
		//
		size_t	nLayerCount = meshTempBuf.GetWeightLayerCount() ;
		for ( size_t i = 0; i < nLayerCount; i ++ )
		{
			SString	strLayerOrg = meshTempBuf.GetWeightLayerIDAt( i ) ;
			SString	strLayerMirror = MirrorWeightName( strLayerOrg ) ;
			if ( strLayerOrg == strLayerMirror )
			{
				continue ;
			}
			ssize_t	iFindMirror = meshTempBuf.FindWeightLayerAs( strLayerMirror ) ;
			if ( iFindMirror >= 0 )
			{
				if ( ((size_t) iFindMirror > i)
					&& ((size_t) iFindMirror < nLayerCount) )
				{
					aMirrorLayerPair.Add( i ) ;
					aMirrorLayerPair.Add( (size_t) iFindMirror ) ;
				}
				continue ;
			}
			size_t	iMirrorLayer = meshTempBuf.GetWeightLayerCount() ;
			meshTempBuf.InsertWeightLayerAt( iMirrorLayer, strLayerMirror ) ;
			//
			const S3DMeshEditor::WeightMapInfo *
						pwmi = meshTempBuf.GetWeightLayerInfoAt( i ) ;
			ESLAssert( pwmi != nullptr ) ;
			//
			S4DMatrix	mat4Bone = mat4Mesh * pwmi->mat4Bone ;
			//
			S3DMatrix	matBone = mat4Bone.GetMatrix3() ;
			S3DVector	vBone = mat4Bone.GetTranslation() ;
			//
			S4DMatrix	mat4MirrorBone( 1, 1, 1, 1 ) ;
			mat4MirrorBone.SetMatrix3( matIMirror * matBone * matMirror ) ;
			mat4MirrorBone.SetTranslation( matMirror * vBone + vBasePos ) ;
			//
			S3DMeshEditor::WeightMapInfo	wmiMirror = *pwmi ;
			wmiMirror.mat4Bone = mat4IMesh * mat4MirrorBone ;
			meshTempBuf.SetWeightLayerInfoAt( iMirrorLayer, wmiMirror ) ;
			//
			aMirrorLayerPair.Add( i ) ;
			aMirrorLayerPair.Add( iMirrorLayer ) ;
		}
	}
	//
	// 各パッチの反転
	//
	S3DMeshEditor::PatchPointSet		ppsSeam ;
	S3DMeshEditor::SeamPatchCollection	spcTemp = meshTempBuf.GetSeamPatch() ;
	const size_t	nPatchCount = meshTempBuf.GetPatchCount() ;
	for ( size_t iPatch = 0; iPatch < nPatchCount; iPatch ++ )
	{
		S3DMeshEditor::Patch *	pSrcPatch = meshTempBuf.GetPatchAt( iPatch ) ;
		ESLAssert( pSrcPatch != nullptr ) ;
		if ( (pSrcPatch == nullptr)
			|| (pSrcPatch->GetFlags() & S3DMeshEditor::Patch::flagDisableModifier) )
		{
			spcTemp.RemovePointIndex( pSrcPatch, 0, pSrcPatch->GetTotalVertexCount() ) ;
			continue ;
		}
		S3DMeshEditor::Patch *	pDstPatch = meshTempBuf.NewPatch() ;
		pDstPatch->CopyFrom( *pSrcPatch ) ;
		pDstPatch->SetName( pSrcPatch->GetName() + L"_mirror" ) ;
		meshTempBuf.AddPatch( pDstPatch ) ;
		//
		spcTemp.RepointerPatch( pSrcPatch, pDstPatch ) ;
		//
		const size_t	nVertexCount = pDstPatch->GetTotalVertexCount() ;
		const S3DVector *
				pvVertex = pSrcPatch->GetConstPointArray( 0, nVertexCount ) ;
		const S3DVector *
				pvNormal = pSrcPatch->GetConstNormalArray( 0, nVertexCount ) ;
		//
		for ( size_t i = 0; i < nVertexCount; i ++ )
		{
			S3DVector	vMirrorPos = matMirror * pvVertex[i] + vBasePos ;
			S3DVector	vMirrorNormal = matMirror * pvNormal[i] ;
			if ( m_flagSeam
				&& (vMirrorPos - pvVertex[i]).Absolute() <= m_fpSeamGap )
			{
				vMirrorPos = (vMirrorPos + pvVertex[i]) * 0.5f ;
				vMirrorNormal = (vMirrorNormal + pvNormal[i]) * 0.5f ;
				vMirrorNormal.Normalize() ;
				//
				pSrcPatch->SetPointAt( i, vMirrorPos ) ;
				pSrcPatch->SetNormalAt( i, vMirrorNormal ) ;
				pDstPatch->SetPointAt( i, vMirrorPos ) ;
				pDstPatch->SetNormalAt( i, vMirrorNormal ) ;
				//
				ppsSeam.RemoveAll() ;
				ppsSeam.Add( S3DMeshEditor::PatchPoint( pSrcPatch, i ) ) ;
				ppsSeam.Add( S3DMeshEditor::PatchPoint( pDstPatch, i ) ) ;
				meshTempBuf.ShrinkPoints( ppsSeam, 0 ) ;
			}
			else
			{
				pDstPatch->SetPointAt( i, vMirrorPos ) ;
				pDstPatch->SetNormalAt( i, vMirrorNormal ) ;
			}
			if ( m_flagMirrorWeight )
			{
				const size_t *	pMirrorLayerPair = aMirrorLayerPair.GetConstArray() ;
				const size_t	nMirrorLayerCount = aMirrorLayerPair.GetLength() ;
				for ( size_t j = 0; j < nMirrorLayerCount; j += 2 )
				{
					ESLAssert( j + 1 < nMirrorLayerCount ) ;
					size_t		z0 = pMirrorLayerPair[j] ;
					size_t		z1 = pMirrorLayerPair[j + 1] ;
					float32_t	w0 = pDstPatch->GetWeightAt( i, z0 ) ;
					float32_t	w1 = pDstPatch->GetWeightAt( i, z1 ) ;
					pDstPatch->SetWeightAt( i, z1, w0 ) ;
					pDstPatch->SetWeightAt( i, z0, w1 ) ;
				}
			}
		}
		//
		const size_t	nFaceCount = pDstPatch->GetTotalFaceCount() ;
		const int8_t *	pFaces = pSrcPatch->GetConstFaceArray( 0, nFaceCount ) ;
		const uint8_t *	pFaceShifter = pSrcPatch->GetConstFaceShifterArray( 0, nFaceCount ) ;
		for ( size_t i = 0; i < nFaceCount; i ++ )
		{
			pDstPatch->SetFaceAt( i, - pFaces[i] ) ;
			pDstPatch->SetFaceShifterAt( i, pFaceShifter[i] ^ 1 ) ;
		}
	}
	meshTempBuf.SeamPatch().Merge( spcTemp ) ;
	meshTempBuf.SetUpdateVertexFlag() ;
	return	&meshTempBuf ;
}

// ウェイトマップ名反転
//////////////////////////////////////////////////////////////////////////////
SSystem::SString
	S3DMirrorMeshController::MirrorWeightName( const SSystem::SString& strName ) const
{
	if ( m_ruleWightName == ruleMatchLead )
	{
		if ( strName.CompareLeft( m_strRightName ) == 0 )
		{
			return	m_strLeftName + strName.Middle( m_strRightName.GetLength() ) ;
		}
		if ( strName.CompareLeft( m_strLeftName ) == 0 )
		{
			return	m_strRightName + strName.Middle( m_strLeftName.GetLength() ) ;
		}
	}
	else if ( m_ruleWightName == ruleMatchEnd )
	{
		if ( (strName.GetLength() >= m_strRightName.GetLength())
			&& (strName.Right( m_strRightName.GetLength() ) == m_strRightName) )
		{
			return	strName.Left( strName.GetLength() - m_strRightName.GetLength() ) + m_strLeftName ;
		}
		if ( (strName.GetLength() >= m_strLeftName.GetLength())
			&& (strName.Right( m_strLeftName.GetLength() ) == m_strLeftName) )
		{
			return	strName.Left( strName.GetLength() - m_strLeftName.GetLength() ) + m_strRightName ;
		}
	}
	else
	{
		ssize_t	i = strName.Find( m_strRightName ) ;
		if ( i >= 0 )
		{
			return	strName.Left( (size_t) i ) + m_strLeftName
					+ strName.Middle( (size_t) i + m_strRightName.GetLength() ) ;
		}
		i = strName.Find( m_strLeftName ) ;
		if ( i >= 0 )
		{
			return	strName.Left( (size_t) i ) + m_strRightName
					+ strName.Middle( (size_t) i + m_strLeftName.GetLength() ) ;
		}
	}
	return	strName ;
}



//////////////////////////////////////////////////////////////////////////////
// 配列複製メッシュコントローラー
//////////////////////////////////////////////////////////////////////////////

const SSystem::SXMLDocument::AttrInteger
	S3DArrayMeshController::m_aiRotationAxis[4] =
{
	{ L"x", rotationOnX },
	{ L"y", rotationOnY },
	{ L"z", rotationOnZ },
	{ nullptr, 0 },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DArrayMeshController, MeshController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DArrayMeshController, mesh_array )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DArrayMeshController::S3DArrayMeshController( void )
	: MeshController( m_ItemClassDescriptor.pwszClassID ),
		m_vBasePos( 0, 0, 0 ), m_matBaseRotate( 1, 1, 1 ), m_vBaseZoom( 1, 1, 1 ),
		m_nCopyCount( 0 ), m_flagKeepOrg( true ), m_flagDuplicateOrg( false ),
		m_vMoveDelta( 0, 0, 0 ), m_vRotationCenter( 0, 0, 0 ),
		m_rotationAxis( rotationOnY ), m_degRotationAngle( 0.0 ),
		m_vZoomDelta( 1, 1, 1 ), m_flagSeam( true ), m_fpSeamGap( 0.0 )
{
	PrepareParameterEntryCount( paramCount ) ;
	AddParameterEntry
		( L"position", S3DSceneComposer::typePosition,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrNoLocalTransform,
			L"基準座標", nullptr ) ;
	AddParameterEntry
		( L"rotation", S3DSceneComposer::typeRotation,
			S3DSceneComposer::attrConstant,
			L"基準回転", nullptr ) ;
	AddParameterEntry
		( L"zoom", S3DSceneComposer::typeZoom,
			S3DSceneComposer::attrConstant,
			L"基準拡大率", nullptr ) ;
	AddParameterEntry
		( L"array_count", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"配列数", nullptr ) ;
	AddParameterEntry
		( L"keep_original", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"元メッシュを保持", nullptr ) ;
	AddParameterEntry
		( L"copy_original", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"元メッシュを配列", nullptr ) ;
	AddParameterEntry
		( L"move_delta", S3DSceneComposer::typePosition,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrNoLocalTransform,
			L"移動量", nullptr ) ;
	AddParameterEntry
		( L"rot_center", S3DSceneComposer::typePosition,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrNoLocalTransform,
			L"回転中心", nullptr ) ;
	AddParameterEntry
		( L"rot_axis", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration,
			L"回転軸", nullptr ) ;
	AddParameterEntry
		( L"rot_angle", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"回転角", nullptr, 0.0, 180.0 ) ;
	AddParameterEntry
		( L"zoom_delta", S3DSceneComposer::typeZoom,
			S3DSceneComposer::attrConstant,
			L"拡大率差分", nullptr ) ;
	AddParameterEntry
		( L"seam_flag", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"近接頂点縮退", nullptr ) ;
	AddParameterEntry
		( L"seam_gap", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrUIScalarSlider,
			L"縮退許容距離", nullptr, 0.0, 1.0 ) ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DArrayMeshController::GetMatrixParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramBaseRotate:
		return	m_matBaseRotate ;
	}
	return	S3DDMatrix( 1, 1, 1 ) ;
}

S3DDVector S3DArrayMeshController::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramBasePosition:
		return	m_vBasePos ;
	case	paramBaseZoom:
		return	m_vBaseZoom ;
	case	paramMoveDelta:
		return	m_vMoveDelta ;
	case	paramRotationCenter:
		return	m_vRotationCenter ;
	case	paramZoomDelta:
		return	m_vZoomDelta ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DArrayMeshController::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRotationDelta:
		return	m_degRotationAngle ;
	case	paramSeamGap:
		return	m_fpSeamGap ;
	}
	return	0.0 ;
}

int32_t S3DArrayMeshController::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramCopyCount:
		return	m_nCopyCount ;
	}
	return	0 ;
}

bool S3DArrayMeshController::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramKeepOriginal:
		return	m_flagKeepOrg ;
	case	paramDuplicateOriginal:
		return	m_flagDuplicateOrg ;
	case	paramSeamFlag:
		return	m_flagSeam ;
	}
	return	false ;
}

const wchar_t * S3DArrayMeshController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRotationAxis:
		return	SXMLDocument::GetSymbolAsIntegerOf( m_aiRotationAxis, m_rotationAxis ) ;
	}
	return	nullptr ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DArrayMeshController::SetMatrixParameter( size_t i, const S3DDMatrix& mat )
{
	switch ( i )
	{
	case	paramBaseRotate:
		m_matBaseRotate = mat ;
		return ;
	}
}

void S3DArrayMeshController::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramBasePosition:
		m_vBasePos = vec ;
		return ;
	case	paramBaseZoom:
		m_vBaseZoom = vec ;
		return ;
	case	paramMoveDelta:
		m_vMoveDelta = vec ;
		return ;
	case	paramRotationCenter:
		m_vRotationCenter = vec ;
		return ;
	case	paramZoomDelta:
		m_vZoomDelta = vec ;
		return ;
	}
}

void S3DArrayMeshController::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramRotationDelta:
		m_degRotationAngle = s ;
		return ;
	case	paramSeamGap:
		m_fpSeamGap = s ;
		return ;
	}
}

void S3DArrayMeshController::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramCopyCount:
		m_nCopyCount = n ;
		return ;
	}
}

void S3DArrayMeshController::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramKeepOriginal:
		m_flagKeepOrg = b ;
		return ;
	case	paramDuplicateOriginal:
		m_flagDuplicateOrg = b ;
		return ;
	case	paramSeamFlag:
		m_flagSeam = b ;
		return ;
	}
}

void S3DArrayMeshController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramRotationAxis:
		m_rotationAxis = (RotationAxis)
			SXMLDocument::GetIntegerAsSymbolOf
				( m_aiRotationAxis, pwszCmd, m_rotationAxis ) ;
		return ;
	}
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DArrayMeshController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	switch ( i )
	{
	case	paramRotationAxis:
		{
			for ( size_t i = 0; m_aiRotationAxis[i].pszSymbol != nullptr; i ++ )
			{
				aStrSet.Add( new SString(m_aiRotationAxis[i].pszSymbol) ) ;
			}
		}
		return	true ;
	}
	return	false ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DArrayMeshController::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"複製配列" ;
	}
	return	nullptr ;
}

// メッシュの変形
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditor * S3DArrayMeshController::ModifyMeshEditor
	( S3DMeshEditor& meshTempBuf, const S3DMeshEditor& meshOrgRef )
{
	if ( m_nCopyCount <= 0 )
	{
		return	&meshTempBuf ;
	}
	//
	// 複製元
	//
	const S3DMeshEditor *	pSrcMesh = &meshTempBuf ;
	S3DMeshEditor			meshDupTempBuf ;
	S3DMeshEditor::SeamPatchCollection	spcSrcBase = meshTempBuf.GetSeamPatch() ;
	SPointerArray<S3DMeshEditor::Patch>	pSeamSrcPatchs ;
	if ( !m_flagKeepOrg )
	{
		spcSrcBase.RemoveAll() ;
		if ( m_flagDuplicateOrg )
		{
			spcSrcBase.Merge( meshOrgRef.GetSeamPatch() ) ;
			for ( size_t i = 0; i < meshOrgRef.GetPatchCount(); i ++ )
			{
				pSeamSrcPatchs.Add( meshOrgRef.GetPatchAt(i) ) ;
			}
			pSrcMesh = &meshOrgRef ;
		}
		else
		{
			meshDupTempBuf.DuplicateMesh( meshTempBuf ) ;
			pSrcMesh = &meshDupTempBuf ;
		}
		while ( meshTempBuf.GetPatchCount() > 0 )
		{
			meshTempBuf.RemovePatchAt( 0 ) ;
		}
	}
	else
	{
		for ( size_t i = 0; i < pSrcMesh->GetPatchCount(); i ++ )
		{
			pSeamSrcPatchs.Add( pSrcMesh->GetPatchAt(i) ) ;
		}
		if ( m_flagDuplicateOrg )
		{
			pSrcMesh = &meshOrgRef ;
		}
	}
	//
	// 座標変換と差分計算
	//
	S3DMatrix	matCopy = m_matBaseRotate ;
	matCopy.MagnifyByVector( S3DVector(m_vBaseZoom) ) ;
	S3DVector	vCopyMove = m_vBasePos ;
	S3DMatrix	matRotate( 1, 1, 1 ) ;
	double		radRotate = m_degRotationAngle * PI / 180.0 ;
	switch ( m_rotationAxis )
	{
	case	rotationOnX:
		matRotate.RevolveOnX( sin(radRotate), cos(radRotate) ) ;
		break ;
	case	rotationOnY:
		matRotate.RevolveOnY( sin(radRotate), cos(radRotate) ) ;
		break ;
	case	rotationOnZ:
		matRotate.RevolveOnZ( sin(radRotate), cos(radRotate) ) ;
		break ;
	}
	S3DVector	vZoomDelta = m_vZoomDelta ;
	S3DVector	vRotateOffset = m_vRotationCenter ;
	vRotateOffset -= matRotate * vRotateOffset ;
	vRotateOffset += S3DVector( m_vMoveDelta ) ;
	//
	// 縮退情報構築
	//
	SeamEntryMap	mapSeam ;
	if ( m_flagSeam && (m_nCopyCount >= 2) )
	{
		S3DMatrix	matCopy1 = matRotate * matCopy ;
		matCopy1.MagnifyByVector( vZoomDelta ) ;
		S3DVector	vCopy1 = matRotate * vCopyMove + vRotateOffset ;
		MakeSeamEntryMap
			( mapSeam, *pSrcMesh, matCopy, vCopyMove, matCopy1, vCopy1 ) ;
	}
	//
	// 順次処理
	//
	const size_t	nPatchCount = pSrcMesh->GetPatchCount() ;
	const size_t	iFirstDstPatchBase = meshTempBuf.GetPatchCount() ;
	size_t			iLastDstPatchBase = 0 ;
	for ( int32_t iCopy = 0; iCopy < m_nCopyCount; iCopy ++ )
	{
		S3DMeshEditor::SeamPatchCollection	spcTemp = spcSrcBase ;
		const size_t	iDstPatchBase = meshTempBuf.GetPatchCount() ;
		for ( size_t iPatch = 0; iPatch < nPatchCount; iPatch ++ )
		{
			S3DMeshEditor::Patch *	pSrcPatch = pSrcMesh->GetPatchAt( iPatch ) ;
			ESLAssert( pSrcPatch != nullptr ) ;
			//
			// Patch 複製
			//
			S3DMeshEditor::Patch *	pDstPatch = meshTempBuf.NewPatch() ;
			if ( !(pSrcPatch->GetFlags() & S3DMeshEditor::Patch::flagDisableModifier) )
			{
				pDstPatch->CopyFrom( *pSrcPatch ) ;
			}
			pDstPatch->SetName( pSrcPatch->GetName() + L"." + SString(iCopy + 1) ) ;
			meshTempBuf.AddPatch( pDstPatch ) ;
			//
			spcTemp.RepointerPatch( pSeamSrcPatchs.GetAt(iPatch), pDstPatch ) ;
			//
			if ( pSrcPatch->GetFlags() & S3DMeshEditor::Patch::flagDisableModifier )
			{
				continue ;
			}
			//
			// 座標変換
			//
			const size_t	nVertexCount = pDstPatch->GetTotalVertexCount() ;
			const S3DVector *
					pvVertex = pSrcPatch->GetConstPointArray( 0, nVertexCount ) ;
			const S3DVector *
					pvNormal = pSrcPatch->GetConstNormalArray( 0, nVertexCount ) ;
			for ( size_t i = 0; i < nVertexCount; i ++ )
			{
				pDstPatch->SetPointAt( i, matCopy * pvVertex[i] + vCopyMove ) ;
				pDstPatch->SetNormalAt( i, (matCopy * pvNormal[i]).Normalized() ) ;
			}
		}
		meshTempBuf.SeamPatch().Merge( spcTemp ) ;
		meshTempBuf.SetUpdateVertexFlag() ;
		//
		if ( m_flagSeam && (iCopy >= 1) )
		{
			AddBuildBySeamEntries
				( meshTempBuf, iLastDstPatchBase, iDstPatchBase, mapSeam, false ) ;
		}
		//
		// 次の座標変換
		//
		matCopy = matRotate * matCopy ;
		matCopy.MagnifyByVector( vZoomDelta ) ;
		vCopyMove = matRotate * vCopyMove + vRotateOffset ;
		iLastDstPatchBase = iDstPatchBase ;
	}
	if ( m_flagSeam && (m_nCopyCount >= 2) )
	{
		AddBuildBySeamEntries
			( meshTempBuf, iLastDstPatchBase, iFirstDstPatchBase, mapSeam, true ) ;
	}
	return	&meshTempBuf ;
}

// 縮退情報構築
//////////////////////////////////////////////////////////////////////////////
void S3DArrayMeshController::MakeSeamEntryMap
	( S3DArrayMeshController::SeamEntryMap& sem, const S3DMeshEditor& mesh,
		const S3DMatrix& matCopy0, const S3DVector& vCopy0,
		const S3DMatrix& matCopy1, const S3DVector& vCopy1 ) const
{
	const size_t	nPatchCount = mesh.GetPatchCount() ;
	const float32_t	fpSqrSeamGap = (float32_t) (m_fpSeamGap * m_fpSeamGap) ;
	for ( size_t iPatch1 = 0; iPatch1 < nPatchCount; iPatch1 ++ )
	{
		S3DMeshEditor::Patch *	pPatch1 = mesh.GetPatchAt( iPatch1 ) ;
		ESLAssert( pPatch1 != nullptr ) ;
		if ( pPatch1->GetFlags() & S3DMeshEditor::Patch::flagDisableModifier )
		{
			continue ;
		}
		const size_t	nVertexCount1 = pPatch1->GetTotalVertexCount() ;
		for ( size_t i = 0; i < nVertexCount1; i ++ )
		{
			//
			// 移動後座標 vPos1 に最も近い移動前座標を検索
			//
			S3DVector	vPos1 = matCopy1 * pPatch1->GetPointAt( i ) + vCopy1 ;
			float32_t	fpNearest = fpSqrSeamGap + 1.0f ;
			PointEntry	peNearest ;
			//
			for ( size_t iPatch0 = 0; iPatch0 < nPatchCount; iPatch0 ++ )
			{
				S3DMeshEditor::Patch *	pPatch0 = mesh.GetPatchAt( iPatch0 ) ;
				ESLAssert( pPatch0 != nullptr ) ;
				if ( pPatch0->GetFlags() & S3DMeshEditor::Patch::flagDisableModifier )
				{
					continue ;
				}
				const size_t	nVertexCount0 = pPatch0->GetTotalVertexCount() ;
				const S3DVector *
						pvVertex = pPatch0->GetConstPointArray( 0, nVertexCount0 ) ;
				for ( size_t j = 0; j < nVertexCount0; j ++ )
				{
					S3DVector	vPos0 = matCopy0 * pvVertex[j] + vCopy0 ;
					S3DVector	vDelta = vPos1 - vPos0 ;
					float32_t	fpSqrLen = vDelta.InnerProduct( vDelta ) ;
					if ( fpSqrLen < fpNearest )
					{
						fpNearest = fpSqrLen ;
						peNearest.iPatch = iPatch0 ;
						peNearest.iVertex = j ;
					}
				}
			}
			if ( fpNearest <= fpSqrSeamGap )
			{
				//
				// 結合すべき頂点を追加
				//
				SeamEntry *	pSeam = sem.GetAs( peNearest ) ;
				if ( pSeam == nullptr )
				{
					pSeam = new SeamEntry ;
					pSeam->m_peSrc = peNearest ;
					sem.SetAs( peNearest, pSeam ) ;
				}
				PointEntry	peDst ;
				peDst.iPatch = iPatch1 ;
				peDst.iVertex = i ;
				pSeam->Add( peDst ) ;
			}
		}
	}
}

// 縮退設定
//////////////////////////////////////////////////////////////////////////////
void S3DArrayMeshController::AddBuildBySeamEntries
	( S3DMeshEditor& mesh,
		size_t iLastBasePatch, size_t iCurBasePatch,
		const S3DArrayMeshController::SeamEntryMap& sem, bool flagSeamTest )
{
	S3DMeshEditor::PatchPointSet	ppsTemp ;
	for ( size_t i = 0; i < sem.GetLength(); i ++ )
	{
		SeamEntry *	pSeam = sem.GetAt( i ) ;
		if ( pSeam == nullptr )
		{
			continue ;
		}
		ppsTemp.RemoveAll() ;
		//
		S3DMeshEditor::PatchPoint	ppSrc
			( mesh.GetPatchAt
				( iLastBasePatch
					+ pSeam->m_peSrc.iPatch ), pSeam->m_peSrc.iVertex ) ;
		if ( ppSrc.pPatch == nullptr )
		{
			continue ;
		}
		S3DVector	vPoint = ppSrc.pPatch->GetPointAt( ppSrc.iVertex ) ;
		S3DVector	vNormal = ppSrc.pPatch->GetNormalAt( ppSrc.iVertex ) ;
		ppsTemp.AddSorted( ppSrc ) ;
		//
		const PointEntry *	ppePoints = pSeam->GetConstArray() ;
		const size_t		nPoints = pSeam->GetLength() ;
		for ( size_t j = 0; j < nPoints; j ++ )
		{
			S3DMeshEditor::PatchPoint	pp
				( mesh.GetPatchAt
					( iCurBasePatch + ppePoints[j].iPatch ), ppePoints[j].iVertex ) ;
			if ( pp.pPatch == nullptr )
			{
				continue ;
			}
			if ( flagSeamTest )
			{
				S3DVector	v = pp.pPatch->GetPointAt( pp.iVertex ) ;
				if ( (v - vPoint).Absolute() > m_fpSeamGap )
				{
					continue ;
				}
			}
			pp.pPatch->SetPointAt( pp.iVertex, vPoint ) ;
			pp.pPatch->SetNormalAt( pp.iVertex, vNormal ) ;
			ppsTemp.AddSorted( pp ) ;
		}
		if ( ppsTemp.GetLength() > 1 )
		{
			mesh.ShrinkPoints( ppsTemp, 0 ) ;
		}
	}
}



//////////////////////////////////////////////////////////////////////////////
// ブーリアン・モデリング
//////////////////////////////////////////////////////////////////////////////

// 面と稜線交差情報 S3DBooleanMeshEditor::MeshCrossEdgeList
//////////////////////////////////////////////////////////////////////////////
bool S3DBooleanMeshEditor::MeshCrossEdgeList::IsEmpty( void ) const
{
	return	(pNextList == nullptr) && (pFirst == nullptr) && (pLast == nullptr) ;
}

void S3DBooleanMeshEditor::MeshCrossEdgeList::AddFirst
				( S3DBooleanMeshEditor::MeshCrossEdge * pmce )
{
	ESLAssert( pmce->pNextEdge == nullptr ) ;
	if ( pFirst != nullptr )
	{
		pmce->pNextEdge = pFirst ;
		pFirst = pmce ;
	}
	else
	{
		ESLAssert( pLast == nullptr ) ;
		pFirst = pmce ;
		pLast = pmce ;
	}
}

void S3DBooleanMeshEditor::MeshCrossEdgeList::AddLast
				( S3DBooleanMeshEditor::MeshCrossEdge * pmce )
{
	ESLAssert( pmce->pNextEdge == nullptr ) ;
	if ( pLast != nullptr )
	{
		pLast->pNextEdge = pmce ;
		pLast = pmce ;
	}
	else
	{
		ESLAssert( pFirst == nullptr ) ;
		pFirst = pmce ;
		pLast = pmce ;
	}
}

void S3DBooleanMeshEditor::MeshCrossEdgeList::Reverse( void )
{
	if ( pFirst == nullptr )
	{
		return ;
	}
	MeshCrossEdge *	pPrev = pFirst ;
	pLast = pPrev ;
	//
	MeshCrossEdge *	pNext = pPrev->pNextEdge ;
	pPrev->pNextEdge = nullptr ;
	while ( pNext != nullptr )
	{
		MeshCrossEdge *	pCur = pNext ;
		pNext = pCur->pNextEdge ;
		pCur->pNextEdge = pPrev ;
		pPrev = pCur ;
	}
	//
	pFirst = pPrev ;

#if defined(__DEBUG__)
	VerifyList() ;
#endif
}

void S3DBooleanMeshEditor::MeshCrossEdgeList::AddLastFrom
		( S3DBooleanMeshEditor::MeshCrossEdgeList * pmcel )
{
	if ( pLast != nullptr )
	{
		ESLAssert( pLast->pNextEdge == nullptr ) ;
		pLast->pNextEdge = pmcel->pFirst ;
		if ( pmcel->pLast != nullptr )
		{
			pLast = pmcel->pLast ;
		}
	}
	else
	{
		ESLAssert( pFirst == nullptr ) ;
		pFirst = pmcel->pFirst ;
		pLast = pmcel->pLast ;
	}
#if defined(__DEBUG__)
	VerifyList() ;
#endif
}

void S3DBooleanMeshEditor::MeshCrossEdgeList::AddLastReverseFrom
		( S3DBooleanMeshEditor::MeshCrossEdgeList * pmcelRev )
{
	while ( pmcelRev->pFirst != nullptr )
	{
		ESLAssert( pmcelRev->pLast != nullptr ) ;
		if ( pmcelRev->pFirst != pmcelRev->pLast )
		{
			MeshCrossEdge *	pPrevLast = pmcelRev->pFirst ;
			while ( pPrevLast->pNextEdge != pmcelRev->pLast )
			{
				ESLAssert( pPrevLast->pNextEdge != nullptr ) ;
				pPrevLast = pPrevLast->pNextEdge ;
			}
			AddLast( pmcelRev->pLast ) ;
			pPrevLast->pNextEdge = nullptr ;
			pmcelRev->pLast = pPrevLast ;
		}
		else
		{
			AddLast( pmcelRev->pLast ) ;
			pmcelRev->pLast = nullptr ;
			pmcelRev->pFirst = nullptr ;
			break ;
		}
	}
#if defined(__DEBUG__)
	VerifyList() ;
#endif
}

void S3DBooleanMeshEditor::MeshCrossEdgeList::AddFirstReverseFrom
		( S3DBooleanMeshEditor::MeshCrossEdgeList * pmcelRev )
{
	while ( pmcelRev->pFirst != nullptr )
	{
		MeshCrossEdge *	pmce = pmcelRev->pFirst ;
		pmcelRev->pFirst = pmce->pNextEdge ;
		pmce->pNextEdge = nullptr ;
		AddFirst( pmce ) ;
	}
	pmcelRev->pLast = nullptr ;

#if defined(__DEBUG__)
	VerifyList() ;
#endif
}

void S3DBooleanMeshEditor::MeshCrossEdgeList::VerifyList( void )
{
#if defined(__DEBUG__)
	SPointerArray<MeshCrossEdge>	aList ;
	MeshCrossEdge *	pmceNext = pFirst ;
	MeshCrossEdge *	pmceLast = nullptr ;
	while ( pmceNext != nullptr )
	{
		ESLAssert( aList.FindPtr( pmceNext ) < 0 ) ;
		aList.Add( pmceNext ) ;
		pmceLast = pmceNext ;
		pmceNext = pmceNext->pNextEdge ;
	}
	ESLAssert( pmceLast == pLast ) ;
#endif // defined(__DEBUG__)
}


// メッシュ S3DBooleanMeshEditor::MeshWorkBuffer
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DBooleanMeshEditor::MeshWorkBuffer, ESLObject )

S3DBooleanMeshEditor::MeshWorkBuffer::MeshWorkBuffer( void )
{
}

S3DBooleanMeshEditor::MeshWorkBuffer::~MeshWorkBuffer( void )
{
}

// ブーリアン開始
//////////////////////////////////////////////////////////////////////////////
void S3DBooleanMeshEditor::ResetBoolean( void )
{
	m_bufWork.FreeAll() ;
}

// オブジェクト構築
//////////////////////////////////////////////////////////////////////////////
void S3DBooleanMeshEditor::BuildObject
	( S3DBooleanMeshEditor::BooleanObject& bobj,
		S3DMeshEditor& mesh,
		const S3DMeshEditor::MeshParam& param, uint32_t nFlags,
		S3DMaterial*const* ppMaterials, size_t nCount )
{
	//
	// メッシュをベイク
	//
	SArray<size_t>							aExistingCount ;
	SObjectArray<S3DRenderBuffer>			aRenderBuf ;
	SPointerArray<S3DVertexBufferInterface>	aPtrVBOs ;
	//
	aRenderBuf.SetLimit( nCount ) ;
	aPtrVBOs.SetLimit( nCount ) ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		S3DRenderBuffer *	pRender = new S3DRenderBuffer ;
		aRenderBuf.SetAt( i, pRender ) ;
		aPtrVBOs.SetAt( i, pRender ) ;
	}
	mesh.RenderVertexBuffers
		( aPtrVBOs.GetConstArray(), ppMaterials,
			aExistingCount.GetArray( nCount ), nCount, param, nFlags ) ;
	aExistingCount.FinishArray() ;
	//
	// メッシュを統合してコリジョンを構築
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		MeshWorkBuffer *	pmwb = new MeshWorkBuffer ;
		bobj.m_buffers.Add( pmwb ) ;
		aRenderBuf.At(i).GetMeshBufferAsSinglePrimitive( pmwb->m_mesh ) ;
		//
		const size_t	nFaceCount = pmwb->m_mesh.m_nIndexCount / 3 ;
		pmwb->m_fcet.SetLength( nFaceCount ) ;
		pmwb->m_vbt.SetLength( pmwb->m_mesh.m_nVertexCount ) ;
		//
		if ( !pmwb->m_mesh.IsEmpty() )
		{
			bobj.m_collision.AttachMeshUserData( pmwb ) ;
			pmwb->m_mesh.Render( bobj.m_collision ) ;
		}
	}
	//
	// FaceCrossEdgeTable を構築
	//
	BuildFaceCrossEdgeTable( bobj ) ;
}

void S3DBooleanMeshEditor::BuildFaceCrossEdgeTable( BooleanObject& bobj )
{
	for ( size_t iMesh = 0; iMesh < bobj.m_buffers.GetLength(); iMesh ++ )
	{
		MeshWorkBuffer *	pmwb = bobj.m_buffers.GetAt( iMesh ) ;
		ESLAssert( pmwb != nullptr ) ;
		if ( pmwb == nullptr )
		{
			continue ;
		}
		ESLAssert( pmwb->m_mesh.m_bufVertex.GetLength() >= pmwb->m_mesh.m_nVertexCount ) ;
		ESLAssert( pmwb->m_mesh.m_bufIndex.GetLength() >= pmwb->m_mesh.m_nIndexCount ) ;
		const size_t		nPolyCount = pmwb->m_mesh.m_nIndexCount / 3 ;
		const uint32_t *	pPolyIndex = pmwb->m_mesh.m_bufIndex.GetConstArray() ;
		for ( size_t iPoly = 0; iPoly < nPolyCount; iPoly ++, pPolyIndex += 3 )
		{
			size_t	iVertex[4] =
			{
				(size_t) pPolyIndex[0],
				(size_t) pPolyIndex[1],
				(size_t) pPolyIndex[2],
				(size_t) pPolyIndex[0],
			} ;
			for ( size_t i = 0; i < 3; i ++ )
			{
				S3DMeshEditor::Edge	edge( iVertex[i], iVertex[i + 1] ) ;
				FacePair *	pfp = pmwb->m_efm.GetAs( edge ) ;
				if ( pfp != nullptr )
				{
					pfp->iFace[1] = (ssize_t) iPoly ;
					continue ;
				}
				FacePair	fp ;
				fp.iFace[0] = (ssize_t) iPoly ;
				fp.iFace[1] = -1 ;
				pmwb->m_efm.SetAs( edge, fp ) ;
			}
		}
	}
}

// 交差処理
//////////////////////////////////////////////////////////////////////////////
void S3DBooleanMeshEditor::CrossTest
	( S3DBooleanMeshEditor::BooleanObject& bobj0,
		S3DBooleanMeshEditor::BooleanObject& bobj1 )
{
	S3DVector	vMin0, vMin1 ;
	S3DVector	vMax0, vMax1 ;
	bobj0.m_collision.GetCircumscribedParallelepiped( vMin0, vMax0 ) ;
	bobj1.m_collision.GetCircumscribedParallelepiped( vMin1, vMax1 ) ;
	//
	vMin0.x = esl_fminf( vMin0.x, vMin1.x ) ;
	vMin0.y = esl_fminf( vMin0.y, vMin1.y ) ;
	vMin0.z = esl_fminf( vMin0.z, vMin1.z ) ;
	vMax0.x = esl_fmaxf( vMax0.x, vMax1.x ) ;
	vMax0.y = esl_fmaxf( vMax0.y, vMax1.y ) ;
	vMax0.z = esl_fmaxf( vMax0.z, vMax1.z ) ;
	//
	CrossTestAgainst( bobj0, bobj1, vMin0, vMax0 ) ;
	CrossTestAgainst( bobj1, bobj0, vMin0, vMax0 ) ;
}

void S3DBooleanMeshEditor::CrossTestAgainst
	( S3DBooleanMeshEditor::BooleanObject& bobjBoolean,
		S3DBooleanMeshEditor::BooleanObject& bobjTest,
		const S3DVector& vMin, const S3DVector& vMax )
{
	const float32_t		fpErrorGap = (float32_t) (vMax - vMin).Absolute() * 0.0001f ;
	for ( size_t iMesh = 0; iMesh < bobjTest.m_buffers.GetLength(); iMesh ++ )
	{
		MeshWorkBuffer *	pmwb = bobjTest.m_buffers.GetAt( iMesh ) ;
		ESLAssert( pmwb != nullptr ) ;
		if ( pmwb == nullptr )
		{
			continue ;
		}
		ESLAssert( pmwb->m_mesh.m_bufVertex.GetLength() >= pmwb->m_mesh.m_nVertexCount ) ;
		ESLAssert( pmwb->m_mesh.m_bufIndex.GetLength() >= pmwb->m_mesh.m_nIndexCount ) ;
		for ( size_t iEdge = 0; iEdge < pmwb->m_efm.GetLength(); iEdge ++ )
		{
			const S3DMeshEditor::Edge *	pEdge = pmwb->m_efm.GetTagAt( iEdge ) ;
			const FacePair *			pfp = pmwb->m_efm.GetAt( iEdge ) ;
			ESLAssert( pEdge != nullptr ) ;
			ESLAssert( pfp != nullptr ) ;
			if ( pmwb->m_vbt.At(pEdge->iVertex0) == elementUnknown )
			{
				CrossTestEdgeAgainst
					( bobjBoolean, pmwb,
						pEdge->iVertex0, pEdge->iVertex1,
						pfp, vMin, vMax, fpErrorGap ) ;
			}
			else
			{
				CrossTestEdgeAgainst
					( bobjBoolean, pmwb,
						pEdge->iVertex1, pEdge->iVertex0,
						pfp, vMin, vMax, fpErrorGap ) ;
			}
		}
	}
}

void S3DBooleanMeshEditor::CrossTestEdgeAgainst
	( S3DBooleanMeshEditor::BooleanObject& bobjBoolean,
		S3DBooleanMeshEditor::MeshWorkBuffer * pmwb,
		size_t iVertex0, size_t iVertex1,
		const S3DBooleanMeshEditor::FacePair * pfp,
		const S3DVector& vMin, const S3DVector& vMax, float32_t fpErrorGap )
{
	//
	// 交差判定
	//
	S3DDVector	vPos0 = pmwb->m_mesh.m_bufVertex.At( iVertex0 ) ;
	S3DDVector	vPos1 = pmwb->m_mesh.m_bufVertex.At( iVertex1 ) ;
	S3DVector	vDir = (vPos1 - vPos0).Normalized() ;
	//
	CrossTestEdgeAgainst_Param
			cteap( *this, bobjBoolean, pmwb,
				iVertex0, iVertex1, pfp, vMin, vMax, fpErrorGap ) ;
	S3DCollisionResult	rsCross ;
	rsCross.fpDistance = (float32_t) (vPos1 - vPos0).Absolute() ;
	rsCross.pMesh = nullptr ;
	rsCross.pqpcHit = nullptr ;
	rsCross.pfnOnHitCollider = &S3DBooleanMeshEditor::CrossTestEdgeAgainst_OnHitCollider ;
	rsCross.ptrOnHitInstance = &cteap ;
	//
	bobjBoolean.m_collision.IsSegmentCrossing( vPos0, vPos1, fpErrorGap, rsCross ) ;
	if ( cteap.m_aHitResult.GetLength() == 0 )
	{
		//
		// 稜線が交差していない時は、別途内外判定
		//
		if ( pmwb->m_vbt.At(iVertex0) == elementUnknown )
		{
			pmwb->m_vbt.At(iVertex0) =
				TestVertexBooleanAgainst
					( bobjBoolean, vPos0, vMin, vMax, fpErrorGap ) ;
		}
		return ;
	}
	const CrossEdgeDesc&	ced0 = cteap.m_aHitResult.At(0) ;
	VertexBoolean	vbElement1 = elementUnknown ;
	//
	// 内外判定
	//
	if ( ced0.vHitNormal.InnerProduct( vDir ) < 0.0f )
	{
		if ( pmwb->m_vbt.At(iVertex0) == elementUnknown )
		{
			pmwb->m_vbt.At(iVertex0) = elementOuter ;
		}
		vbElement1 = elementInner ;
	}
	else
	{
		if ( pmwb->m_vbt.At(iVertex0) == elementUnknown )
		{
			pmwb->m_vbt.At(iVertex0) = elementInner ;
		}
		vbElement1 = elementOuter ;
	}
	double	fpLastHit = 0.0 ;
	for ( size_t i = 0; i < cteap.m_aHitResult.GetLength(); i ++ )
	{
		const CrossEdgeDesc&	ced = cteap.m_aHitResult.At(i) ;
		//
		// 当たり面情報
		//
		S3DBooleanMeshEditor::MeshWorkBuffer *	pmwbCross = ced.pmwbCross ;
		const size_t	iCrossFace = ced.iCrossFace ;
		//
		// 交差情報追加
		//
		S3DMeshEditor::Edge	edge( iVertex0, iVertex1 ) ;
		ESLAssert( pmwb->m_efm.GetAs( edge ) == pfp ) ;
		if ( pfp != nullptr )
		{
			ESLAssert( pfp->iFace[0] >= 0 ) ;
			AddFaceEdgeCrossPoint
				( pmwb, (size_t) pfp->iFace[0],
					FaceEdgeIndexOf( pmwb, (size_t) pfp->iFace[0], edge ),
					ced.vHitGlobal, pmwbCross, iCrossFace ) ;
			//
			if ( pfp->iFace[1] >= 0 )
			{
				AddFaceEdgeCrossPoint
					( pmwb, (size_t) pfp->iFace[1],
						FaceEdgeIndexOf( pmwb, (size_t) pfp->iFace[1], edge ),
						ced.vHitGlobal, pmwbCross, iCrossFace ) ;
			}
		}
		//
		// 交差対象面に交差情報追加
		//
		AddCrossEdgePointAtFace
			( pmwbCross, iCrossFace,
				ced.vHitGlobal, ced.vHitCoord, pmwb, edge, pfp ) ;
		//
		// 次の交差点へ
		//
		double	fpGap = ced.fpHit - fpLastHit ;
		if ( ced.vHitNormal.InnerProduct( vDir ) < 0.0f )
		{
			vbElement1 = elementInner ;
		}
		else
		{
			vbElement1 = elementOuter ;
		}
	}
	//
	// 反対側頂点の内外
	//
	if ( (pmwb->m_vbt.At(iVertex1) == elementUnknown) || (vbElement1 == elementOuter) )
	{
		pmwb->m_vbt.At(iVertex1) = vbElement1 ;
	}
	else
	{
//		ESLAssert( pmwb->m_vbt.At(iVertex1) == vbElement1 ) ;
	}
	//
/*	if ( !bobjBoolean.m_collision.IsSegmentCrossing
						( vPos0, vPos1, fpErrorGap, rsCross ) )
	{
		//
		// 稜線が交差していない時は、別途内外判定
		//
		if ( pmwb->m_vbt.At(iVertex0) == elementUnknown )
		{
			pmwb->m_vbt.At(iVertex0) =
				TestVertexBooleanAgainst
					( bobjBoolean, vPos0, vMin, vMax, fpErrorGap ) ;
		}
		return ;
	}
	VertexBoolean	vbElement1 = elementUnknown ;
	if ( pmwb->m_vbt.At(iVertex0) == elementUnknown )
	{
		// 内外判定
		if ( rsCross.vNormal.InnerProduct( S3DVector(vPos1 - vPos0) ) < 0.0f )
		{
			pmwb->m_vbt.At(iVertex0) = elementOuter ;
			vbElement1 = elementInner ;
		}
		else
		{
			pmwb->m_vbt.At(iVertex0) = elementInner ;
			vbElement1 = elementOuter ;
		}
	}
	for ( ; ; )
	{
		//
		// 当たり面情報
		//
		S3DBooleanMeshEditor::MeshWorkBuffer *
				pmwbCross = ESLTypeCast<S3DBooleanMeshEditor::MeshWorkBuffer>
													( rsCross.pMesh->pUserData ) ;
		size_t	iCrossFace = rsCross.iPolygon ;
		//
		rsCross.ComputeHitLocalCoord() ;
		//
		// 交差情報追加
		//
		S3DMeshEditor::Edge	edge( iVertex0, iVertex1 ) ;
		ESLAssert( pmwb->m_efm.GetAs( edge ) == pfp ) ;
		if ( pfp != nullptr )
		{
			ESLAssert( pfp->iFace[0] >= 0 ) ;
			AddFaceEdgeCrossPoint
				( pmwb, (size_t) pfp->iFace[0],
					FaceEdgeIndexOf( pmwb, (size_t) pfp->iFace[0], edge ),
					rsCross.vHitGlobal, pmwbCross, iCrossFace ) ;
			//
			if ( pfp->iFace[1] >= 0 )
			{
				AddFaceEdgeCrossPoint
					( pmwb, (size_t) pfp->iFace[1],
						FaceEdgeIndexOf( pmwb, (size_t) pfp->iFace[1], edge ),
						rsCross.vHitGlobal, pmwbCross, iCrossFace ) ;
			}
		}
		//
		// 交差対象面に交差情報追加
		//
		AddCrossEdgePointAtFace
			( pmwbCross, iCrossFace,
				rsCross.vHitGlobal, rsCross.vLocalCoord, pmwb, edge, pfp ) ;
		//
		// 次の交差点を検索
		//
		vPos0 = rsCross.vHitGlobal ;
		if ( (vPos1 - vPos0).Absolute() <= fpErrorGap * 5.0 )
		{
			break ;
		}
		vPos0 += vDir * (fpErrorGap * 2.5) ;
		rsCross.fpDistance = (float32_t) (vPos1 - vPos0).Absolute() ;
		//
		rsCross.pMesh = nullptr ;
		rsCross.pqpcHit = nullptr ;
		//
		if ( !bobjBoolean.m_collision.IsSegmentCrossing
							( vPos0, vPos1, fpErrorGap, rsCross ) )
		{
			break ;
		}
		if ( rsCross.vNormal.InnerProduct( S3DVector(vPos1 - vPos0) ) < 0.0f )
		{
			vbElement1 = elementInner ;
		}
		else
		{
			vbElement1 = elementOuter ;
		}
	}
	//
	// 反対側頂点の内外
	//
	if ( pmwb->m_vbt.At(iVertex1) == elementUnknown )
	{
		pmwb->m_vbt.At(iVertex1) = vbElement1 ;
	}
*/
}

S3DBooleanMeshEditor::CrossTestEdgeAgainst_Param::CrossTestEdgeAgainst_Param
	( S3DBooleanMeshEditor& bme,
		BooleanObject& bobjBoolean, MeshWorkBuffer * pmwb,
		size_t iVertex0, size_t iVertex1, const FacePair * pfp,
		const S3DVector& vMin, const S3DVector& vMax, float32_t fpErrorGap )
	: m_pmwb(pmwb)
{
	m_vPos0 = pmwb->m_mesh.m_bufVertex.At( iVertex0 ) ;
	m_vPos1 = pmwb->m_mesh.m_bufVertex.At( iVertex1 ) ;
	m_vDir = (m_vPos1 - m_vPos0).Normalized() ;
}

S3DCollision::HitColliderCallback
	S3DBooleanMeshEditor::CrossTestEdgeAgainst_OnHitCollider
		( const S3DCollisionResult& rsHit,
			const S3DVector& vHitPos, const S3DVector& vHitNormal,
			const S3DCollision::MeshCollision * pMesh, size_t iPolygon )
{
	CrossTestEdgeAgainst_Param *	pcteap =
		reinterpret_cast<CrossTestEdgeAgainst_Param*>( rsHit.ptrOnHitInstance ) ;
	S3DBooleanMeshEditor::MeshWorkBuffer *	pmwb = pcteap->m_pmwb ;

	S3DCollision::HitColliderGlobalInfo	hcgi ;
	rsHit.GetHitColliderGlobalInfo( hcgi, pMesh ) ;

	S3DCollisionResult	rsCross( rsHit ) ;
	rsCross.SetParamOnHitCollider( hcgi, vHitPos, vHitNormal, pMesh, iPolygon ) ;
	rsCross.ComputeHitLocalCoord() ;
	//
	S3DDVector		vGlobalHitPos = hcgi.matToGlobal * vHitPos + hcgi.vToGlobal ;
	S3DDVector		vGlobalNormal = hcgi.matToGlobal * vHitNormal ;
	double			fpHit = (vGlobalHitPos - pcteap->m_vPos0).Absolute() ;

	CrossEdgeDesc *	pced = new CrossEdgeDesc ;
	pced->fpHit = fpHit ;
	pced->pmwbCross = ESLTypeCast<S3DBooleanMeshEditor::MeshWorkBuffer>( pMesh->pUserData ) ;
	pced->iCrossFace = iPolygon ;
	pced->vHitGlobal = vGlobalHitPos ;
	pced->vHitNormal = vGlobalNormal ;
	pced->vHitCoord = rsCross.vLocalCoord ;
	//
	for ( size_t i = 0; i < pcteap->m_aHitResult.GetLength(); i ++ )
	{
		CrossEdgeDesc *	pcedResult = pcteap->m_aHitResult.GetAt( i ) ;
		if ( fpHit < pcedResult->fpHit )
		{
			pcteap->m_aHitResult.InsertAt( i, pced ) ;
			return	S3DCollision::hitColliderNext ;
		}
	}
	pcteap->m_aHitResult.Add( pced ) ;
	return	S3DCollision::hitColliderNext ;
}

// 頂点の内外判定
//////////////////////////////////////////////////////////////////////////////
S3DBooleanMeshEditor::VertexBoolean
		S3DBooleanMeshEditor::TestVertexBooleanAgainst
	( S3DBooleanMeshEditor::BooleanObject& bobjBoolean,
		const S3DDVector& vVertex,
		const S3DVector& vMin, const S3DVector& vMax, float32_t fpErrorGap )
{
	double		fpLength = (vMax - vMin).Absolute() * 1.1 ;
	S3DDVector	vTarget[6] =
	{
		vVertex + S3DDVector( -fpLength, 0, 0 ),
		vVertex + S3DDVector( fpLength, 0, 0 ),
		vVertex + S3DDVector( 0, -fpLength, 0 ),
		vVertex + S3DDVector( 0, fpLength, 0 ),
		vVertex + S3DDVector( 0, 0, -fpLength ),
		vVertex + S3DDVector( 0, 0, fpLength ),
	} ;
	for ( int i = 0; i < 6; i ++ )
	{
		S3DCollisionResult	rsCross ;
		rsCross.fpDistance = (float32_t) (vTarget[i] - vVertex).Absolute() ;
		rsCross.pMesh = nullptr ;
		rsCross.pqpcHit = nullptr ;
		//
		if ( bobjBoolean.m_collision.IsSegmentCrossing
					( vVertex, vTarget[i], fpErrorGap, rsCross ) )
		{
			if ( rsCross.vNormal.InnerProduct
						( S3DVector(vTarget[i] - vVertex) ) < 0.0f )
			{
				return	elementOuter ;
			}
			else
			{
				return	elementInner ;
			}
		}
	}
	return	elementOuter ;
}

// 面の稜線番号を取得
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DBooleanMeshEditor::FaceEdgeIndexOf
	( MeshWorkBuffer * pmwb, size_t iFace, const S3DMeshEditor::Edge& edge )
{
	const uint32_t *
		pPolyIndex = pmwb->m_mesh.m_bufIndex.GetConstArray() + iFace * 3 ;
	size_t	iVertex[4] =
	{
		(size_t) pPolyIndex[0],
		(size_t) pPolyIndex[1],
		(size_t) pPolyIndex[2],
		(size_t) pPolyIndex[0],
	} ;
	for ( size_t i = 0; i < 3; i ++ )
	{
		if ( S3DMeshEditor::Edge( iVertex[i], iVertex[i + 1] ) == edge )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

// 面の稜線の交差点を追加
//////////////////////////////////////////////////////////////////////////////
void S3DBooleanMeshEditor::AddFaceEdgeCrossPoint
	( S3DBooleanMeshEditor::MeshWorkBuffer * pmwb, size_t iFace,
		size_t iFaceEdge, const S3DVector& vCross,
		S3DBooleanMeshEditor::MeshWorkBuffer * pmwbCross, size_t iCrossFace )
{
	MeshCrossEdgeList *	pmcel = pmwb->m_fcet.GetAt( iFace ) ;
	ESLAssert( pmcel != nullptr ) ;
	ESLAssert( iFaceEdge < 3 ) ;
	//
	// 交差点情報生成
	//
	MeshCrossEdge *	pmceNew =
		(MeshCrossEdge*) m_bufWork.Allocate( sizeof(MeshCrossEdge) ) ;
	eslFillMemory( pmceNew, 0, sizeof(MeshCrossEdge) ) ;
	pmceNew->iFaceEdge = (ssize_t) iFaceEdge ;
	pmceNew->pMesh = pmwbCross ;
	pmceNew->iCrossFace = (ssize_t) iCrossFace ;
	pmceNew->vCross = vCross ;
	//
	const uint32_t *	pFaceIndex = pmwb->m_mesh.m_bufIndex.GetConstArray() + iFace * 3 ;
	const S3DVector4 *	pvVertex = pmwb->m_mesh.m_bufVertex.GetConstArray() ;
	S3DVector	v[3] =
	{
		pvVertex[pFaceIndex[0]],
		pvVertex[pFaceIndex[1]],
		pvVertex[pFaceIndex[2]],
	} ;
	S3DVector	vBA, vCA, vPA, vXBC ;
	vBA = v[1] - v[0] ;
	vCA = v[2] - v[0] ;
	vPA = vCross - v[0] ;
	vXBC = vBA * vCA ;
	//
	float32_t	d = vXBC.InnerProduct( vXBC ) ;
	if ( d >= 1.0e-8 )
	{
		vXBC *= 1.0f / d ;
	}
	pmceNew->vCoord.x = (float32_t) vXBC.InnerProduct( vPA * vCA ) ;
	pmceNew->vCoord.y = (float32_t) vXBC.InnerProduct( vBA * vPA ) ;
	//
	// 交差面と同一の交差点があったら最後に追加する
	//
	MeshCrossEdgeList *	pmcelLast = pmcel ;
	do
	{
		if ( pmcel->pLast != nullptr )
		{
			MeshCrossEdge *	pmce = pmcel->pLast ;
			ESLAssert( pmce->pNextEdge == nullptr ) ;
			if ( pmce->pMesh == pmwbCross )
			{
				if ( pmce->iFaceEdge < 0 )
				{
					const FacePair *	pfp = pmce->pFaces ;
					if ( (pfp != nullptr)
						&& ((pfp->iFace[0] == (ssize_t) iCrossFace)
							|| (pfp->iFace[1] == (ssize_t) iCrossFace)) )
					{
						ESLAssert( pmcel->pFirst->iCrossFace != (ssize_t) iCrossFace ) ;
						pmcel->AddLast( pmceNew ) ;
						return ;
					}
				}
				else if ( pmce->iCrossFace == (ssize_t) iCrossFace )
				{
					if ( (pmcel->pFirst == pmcel->pLast)
						&& (pmce->iFaceEdge != pmceNew->iFaceEdge) )
					{
						pmcel->AddLast( pmceNew ) ;
						return ;
					}
				}
			}
		}
		if ( (pmcel->pFirst != nullptr) && (pmcel->pLast != pmcel->pFirst) )
		{
			MeshCrossEdge *	pmce = pmcel->pFirst ;
			if ( pmce->pMesh == pmwbCross )
			{
				if ( pmce->iFaceEdge < 0 )
				{
					const FacePair *	pfp = pmce->pFaces ;
					if ( (pfp != nullptr)
						&& ((pfp->iFace[0] == (ssize_t) iCrossFace)
							|| (pfp->iFace[1] == (ssize_t) iCrossFace)) )
					{
						pmcel->AddFirst( pmceNew ) ;
						return ;
					}
				}
				else
				{
//					ESLAssert( pmce->iCrossFace != (ssize_t) iCrossFace ) ;
				}
			}
		}
		pmcelLast = pmcel ;
		pmcel = pmcel->pNextList ;
	}
	while ( pmcel != nullptr ) ;
	//
	if ( pmcelLast->pFirst == nullptr )
	{
		pmcelLast->AddLast( pmceNew ) ;
		return ;
	}
	//
	// 新たなリストを追加する
	//
	pmcel = (MeshCrossEdgeList*) m_bufWork.Allocate( sizeof(MeshCrossEdgeList) ) ;
	eslFillMemory( pmcel, 0, sizeof(MeshCrossEdgeList) ) ;
	pmcelLast->pNextList = pmcel ;
	pmcel->AddLast( pmceNew ) ;
}

// 面に交差する稜線を追加（稜線に貫かれる側の面）
//////////////////////////////////////////////////////////////////////////////
void S3DBooleanMeshEditor::AddCrossEdgePointAtFace
	( S3DBooleanMeshEditor::MeshWorkBuffer * pmwb, size_t iFace,
		const S3DVector& vCross, const S2DVector& vCoord,
		S3DBooleanMeshEditor::MeshWorkBuffer * pmwbCross,
		const S3DMeshEditor::Edge& edgeCross,
		const S3DBooleanMeshEditor::FacePair * pfpEdgeFaces )
{
	MeshCrossEdgeList *	pmcel = pmwb->m_fcet.GetAt( iFace ) ;
	ESLAssert( pmcel != nullptr ) ;
	//
	// 交差点情報生成
	//
	MeshCrossEdge *	pmceNew =
		(MeshCrossEdge*) m_bufWork.Allocate( sizeof(MeshCrossEdge) ) ;
	eslFillMemory( pmceNew, 0, sizeof(MeshCrossEdge) ) ;
	pmceNew->iFaceEdge = -1 ;
	pmceNew->pMesh = pmwbCross ;
	pmceNew->iCrossFace = -1 ;
	pmceNew->edge = edgeCross ;
	pmceNew->pFaces = pfpEdgeFaces ;
	pmceNew->vCross = vCross ;
	pmceNew->vCoord = vCoord ;
	//
	// 稜線を含む面
	//
	ESLAssert( pmwbCross->m_efm.GetAs( edgeCross ) == pfpEdgeFaces ) ;
	ssize_t		iEdgeFace[2] = { -1, -1 } ;
	if ( pfpEdgeFaces != nullptr )
	{
		iEdgeFace[0] = pfpEdgeFaces->iFace[0] ;
		iEdgeFace[1] = pfpEdgeFaces->iFace[1] ;
	}
	//
	// 同じ面に属する稜線があったらリストに結合する
	//
	MeshCrossEdgeList *	pmcelLast = pmcel ;
	MeshCrossEdgeList *	pmcelAdded = nullptr ;
	do
	{
		if ( pmcel->pLast != nullptr )
		{
			MeshCrossEdge *	pmce = pmcel->pLast ;
			ESLAssert( pmce->pNextEdge == nullptr ) ;
			if ( pmce->pMesh == pmwbCross )
			{
				if ( pmce->iFaceEdge < 0 )
				{
					const FacePair *	pfp = pmce->pFaces ;
					if ( (pfp != nullptr)
						&& ((pfp->iFace[0] == iEdgeFace[0])
							|| (pfp->iFace[1] == iEdgeFace[0])
							|| (pfp->iFace[0] == iEdgeFace[1])
							|| ((pfp->iFace[1] == iEdgeFace[1])
										&& (iEdgeFace[1] != -1))) )
					{
						pmcel->AddLast( pmceNew ) ;
						pmcelAdded = pmcel ;
						break ;
					}
				}
				else if ( (pmcel->pFirst == pmcel->pLast)
					&& ((pmce->iCrossFace == iEdgeFace[0])
						|| (pmce->iCrossFace == iEdgeFace[1])) )
				{
					pmcel->AddLast( pmceNew ) ;
					pmcelAdded = pmcel ;
					break ;
				}
			}
		}
		if ( (pmcel->pFirst != nullptr) && (pmcel->pFirst != pmcel->pLast) )
		{
			MeshCrossEdge *	pmce = pmcel->pFirst ;
			if ( pmce->pMesh == pmwbCross )
			{
				if ( pmce->iFaceEdge < 0 )
				{
					const FacePair *	pfp = pmce->pFaces ;
					if ( (pfp != nullptr)
						&& ((pfp->iFace[0] == iEdgeFace[0])
							|| (pfp->iFace[1] == iEdgeFace[0])
							|| (pfp->iFace[0] == iEdgeFace[1])
							|| ((pfp->iFace[1] == iEdgeFace[1])
										&& (iEdgeFace[1] != -1))) )
					{
						pmcel->AddFirst( pmceNew ) ;
						pmcelAdded = pmcel ;
						break ;
					}
				}
			}
		}
		pmcelLast = pmcel ;
		pmcel = pmcel->pNextList ;
	}
	while ( pmcel != nullptr ) ;
	//
	if ( pmcelAdded == nullptr )
	{
		//
		// 結合できる交差点が見つからなかった場合は新規リストを追加する
		//
		if ( pmcelLast->pFirst == nullptr )
		{
			pmcelLast->AddLast( pmceNew ) ;
			return ;
		}
		pmcel = (MeshCrossEdgeList*) m_bufWork.Allocate( sizeof(MeshCrossEdgeList) ) ;
		eslFillMemory( pmcel, 0, sizeof(MeshCrossEdgeList) ) ;
		pmcelLast->pNextList = pmcel ;
		pmcel->AddLast( pmceNew ) ;
		return ;
	}
	//
	// 結合できる交差点を見つけた場合は、反対側への結合点を見つける
	//
	for ( ; ; )
	{
		pmcelLast = pmcel ;
		pmcel = pmcel->pNextList ;
		if ( pmcel == nullptr )
		{
			break ;
		}
		if ( pmcel->pLast != nullptr )
		{
			MeshCrossEdge *	pmce = pmcel->pLast ;
			ESLAssert( pmce->pNextEdge == nullptr ) ;
			if ( pmce->pMesh == pmwbCross )
			{
				bool	flagMerge = false ;
				if ( pmce->iFaceEdge < 0 )
				{
					const FacePair *	pfp = pmce->pFaces ;
					if ( (pfp != nullptr)
						&& ((pfp->iFace[0] == iEdgeFace[0])
							|| (pfp->iFace[1] == iEdgeFace[0])
							|| (pfp->iFace[0] == iEdgeFace[1])
							|| ((pfp->iFace[1] == iEdgeFace[1])
										&& (iEdgeFace[1] != -1))) )
					{
						flagMerge = true ;
					}
				}
				else if ( (pmcel->pFirst == pmcel->pLast)
					&& ((pmce->iCrossFace == iEdgeFace[0])
						|| (pmce->iCrossFace == iEdgeFace[1])) )
				{
					flagMerge = true ;
				}
				if ( flagMerge )
				{
					pmcelLast->pNextList = pmcel->pNextList ;	// pmcel をリストから外す
					//
					if ( pmcelAdded->pLast == pmceNew )
					{
						pmcelAdded->AddLastReverseFrom( pmcel ) ;
					}
					else
					{
						ESLAssert( pmcelAdded->pFirst == pmceNew ) ;
						pmcel->AddLastFrom( pmcelAdded ) ;
						pmcelAdded->pFirst = pmcel->pFirst ;
						pmcelAdded->pLast = pmcel->pLast ;
					}
					break ;
				}
			}
		}
		if ( (pmcel->pFirst != nullptr) && (pmcel->pFirst != pmcel->pLast) )
		{
			MeshCrossEdge *	pmce = pmcel->pFirst ;
			if ( pmce->pMesh == pmwbCross )
			{
				if ( pmce->iFaceEdge < 0 )
				{
					const FacePair *	pfp = pmce->pFaces ;
					if ( (pfp != nullptr)
						&& ((pfp->iFace[0] == iEdgeFace[0])
							|| (pfp->iFace[1] == iEdgeFace[0])
							|| (pfp->iFace[0] == iEdgeFace[1])
							|| ((pfp->iFace[1] == iEdgeFace[1])
										&& (iEdgeFace[1] != -1))) )
					{
						pmcelLast->pNextList = pmcel->pNextList ;	// pmcel をリストから外す
						//
						if ( pmcelAdded->pLast == pmceNew )
						{
							pmcelAdded->AddLastFrom( pmcel ) ;
						}
						else
						{
							ESLAssert( pmcelAdded->pFirst == pmceNew ) ;
							pmcelAdded->AddFirstReverseFrom( pmcel ) ;
						}
						break ;
					}
				}
			}
		}
	}
}

// 交差処理されたメッシュを出力する
//////////////////////////////////////////////////////////////////////////////
void S3DBooleanMeshEditor::BuildBooleanMesh
	( S3DBooleanMeshEditor::BooleanObject& bobj,
		S3DBooleanMeshEditor::VertexBoolean vboolLogic, bool faceBack )
{
	S3DVector	vMin, vMax ;
	bobj.m_collision.GetCircumscribedParallelepiped( vMin, vMax ) ;
	//
	float32_t	fpErrorGap = (float32_t) (vMax - vMin).Absolute() * 0.0001f ;
	//
	for ( size_t i = 0; i < bobj.m_buffers.GetLength(); i ++ )
	{
		BuildBooleanMeshBuffer( bobj.m_buffers.At(i), vboolLogic, faceBack, fpErrorGap ) ;
	}
}

void S3DBooleanMeshEditor::BuildBooleanMeshBuffer
	( S3DBooleanMeshEditor::MeshWorkBuffer& mwbuf,
		S3DBooleanMeshEditor::VertexBoolean vboolLogic,
		bool faceBack, float32_t fpErrorGap )
{
	//
	// 交差リストの最終的な結合
	//
	const size_t	nFaceCount = mwbuf.m_fcet.GetLength() ;
	size_t			nAddVertex = 0 ;
	for ( size_t iFace = 0; iFace < nFaceCount; iFace ++ )
	{
		MeshCrossEdgeList *	pmcel = mwbuf.m_fcet.GetAt( iFace ) ;
		ESLAssert( pmcel != nullptr ) ;
		MergeMeshCrossEdgeList( pmcel, fpErrorGap ) ;
		nAddVertex += CountCrossPointOfEdgeList( pmcel ) ;
	}
	//
	// バッファの準備
	//
	ESLAssert( mwbuf.m_mesh.m_bufIndex.GetLength() >= nFaceCount * 3 ) ;
	m_bufIndex = mwbuf.m_mesh.m_bufIndex ;
	mwbuf.m_mesh.m_bufIndex.SetLength(0) ;
	//
	ESLAssert( mwbuf.m_mesh.m_bufVertex.GetLength() == mwbuf.m_mesh.m_nVertexCount ) ;
	mwbuf.m_mesh.m_bufVertex.SetLimit( mwbuf.m_mesh.m_nVertexCount + nAddVertex * 2 ) ;
	mwbuf.m_mesh.m_bufNormal.SetLimit( mwbuf.m_mesh.m_nVertexCount + nAddVertex * 2 ) ;
	mwbuf.m_mesh.m_bufUVMap.SetLimit( mwbuf.m_mesh.m_nVertexCount + nAddVertex * 2 ) ;
	mwbuf.m_mesh.m_bufColor.SetLimit( mwbuf.m_mesh.m_nVertexCount + nAddVertex * 2 ) ;
	mwbuf.m_mesh.m_bufExAttr.SetLimit
		( (mwbuf.m_mesh.m_nVertexCount + nAddVertex * 2) * mwbuf.m_mesh.m_nExAttrCount ) ;
	mwbuf.m_mesh.m_bufIndex.SetLimit( nFaceCount * 3 + nAddVertex * 3 ) ;
	mwbuf.m_mesh.m_nIndexCount = 0 ;
	//
	// 各面を出力
	//
	const uint32_t *	pFaceIndex = m_bufIndex.GetConstArray() ;
	const int8_t *		pVBoolTable = mwbuf.m_vbt.GetConstArray() ;
	for ( size_t iFace = 0; iFace < nFaceCount; iFace ++, pFaceIndex += 3 )
	{
		MeshCrossEdgeList *	pmcel = mwbuf.m_fcet.GetAt( iFace ) ;
		ESLAssert( pmcel != nullptr ) ;
		if ( pmcel->IsEmpty() )
		{
			if ( (pVBoolTable[pFaceIndex[0]]
					+ pVBoolTable[pFaceIndex[1]]
					+ pVBoolTable[pFaceIndex[2]]) * vboolLogic < 0 )
			{
				// 出力しない面
				continue ;
			}
			// 分割しない面を出力
			if ( faceBack )
			{
				mwbuf.m_mesh.m_bufIndex.Add( pFaceIndex[2] ) ;
				mwbuf.m_mesh.m_bufIndex.Add( pFaceIndex[1] ) ;
				mwbuf.m_mesh.m_bufIndex.Add( pFaceIndex[0] ) ;
			}
			else
			{
				mwbuf.m_mesh.m_bufIndex.AddArray( pFaceIndex, 3 ) ;
			}
			mwbuf.m_mesh.m_nIndexCount += 3 ;
			continue ;
		}
		BuildMeshAtCrossingFace
			( mwbuf, pmcel, pFaceIndex, vboolLogic, faceBack ) ;
	}
	ESLAssert( mwbuf.m_mesh.m_bufVertex.GetLength() == mwbuf.m_mesh.m_nVertexCount ) ;
	ESLAssert( mwbuf.m_mesh.m_bufIndex.GetLength() == mwbuf.m_mesh.m_nIndexCount ) ;
	//
	if ( faceBack )
	{
		S3DVector4 *	pvNormal =
			mwbuf.m_mesh.m_bufNormal.GetArray( mwbuf.m_mesh.m_nVertexCount ) ;
		for ( size_t i = 0; i < mwbuf.m_mesh.m_nVertexCount; i ++ )
		{
			pvNormal[i] = - pvNormal[i] ;
		}
		mwbuf.m_mesh.m_bufNormal.FinishArray() ;
	}
}

// MeshCrossEdgeList を最終的に結合する
//////////////////////////////////////////////////////////////////////////////
void S3DBooleanMeshEditor::MergeMeshCrossEdgeList
	( S3DBooleanMeshEditor::MeshCrossEdgeList * pmcel, float32_t fpErrorGap )
{
	if ( pmcel->pNextList == nullptr )
	{
		return ;
	}
	do
	{
		while ( (pmcel->pLast != nullptr)
			&& (pmcel->pLast->iFaceEdge < 0) )
		{
			MeshCrossEdgeList *	pmcelLast = pmcel ;
			MeshCrossEdge *		pmceNear =
				FindNearCrossPoint( pmcelLast, pmcel->pLast->vCross, fpErrorGap ) ;
			if ( pmceNear == nullptr )
			{
				break ;
			}
			MeshCrossEdgeList *	pmcelNext = pmcelLast->pNextList ;
			ESLAssert( pmcelNext != nullptr ) ;
			pmcelLast->pNextList = pmcelNext->pNextList ;
			//
			if ( pmcelNext->pFirst == pmceNear )
			{
				pmcel->AddLastFrom( pmcelNext ) ;
			}
			else
			{
				ESLAssert( pmcelNext->pLast == pmceNear ) ;
				pmcel->AddLastReverseFrom( pmcelNext ) ;
			}
		}
		while ( (pmcel->pFirst != nullptr)
			&& (pmcel->pFirst->iFaceEdge < 0) )
		{
			MeshCrossEdgeList *	pmcelLast = pmcel ;
			MeshCrossEdge *		pmceNear =
				FindNearCrossPoint( pmcelLast, pmcel->pFirst->vCross, fpErrorGap ) ;
			if ( pmceNear == nullptr )
			{
				break ;
			}
			MeshCrossEdgeList *	pmcelNext = pmcelLast->pNextList ;
			ESLAssert( pmcelNext != nullptr ) ;
			pmcelLast->pNextList = pmcelNext->pNextList ;
			//
			if ( pmcelNext->pFirst == pmceNear )
			{
				pmcel->AddFirstReverseFrom( pmcelNext ) ;
			}
			else
			{
				ESLAssert( pmcelNext->pLast == pmceNear ) ;
				pmcelNext->AddLastFrom( pmcel ) ;
				pmcel->pFirst = pmcelNext->pFirst ;
				pmcel->pLast = pmcelNext->pLast ;
			}
		}
		pmcel = pmcel->pNextList ;
	}
	while ( pmcel != nullptr ) ;
}

S3DBooleanMeshEditor::MeshCrossEdge *
	S3DBooleanMeshEditor::FindNearCrossPoint
		( S3DBooleanMeshEditor::MeshCrossEdgeList*& pmcelLast,
			const S3DVector& vPos, float32_t fpErrorGap )
{
	S3DBooleanMeshEditor::MeshCrossEdgeList *	pmcel = pmcelLast->pNextList ;
	while ( pmcel != nullptr )
	{
		if ( (pmcel->pFirst != nullptr)
			&& (pmcel->pFirst->iFaceEdge < 0)
			&& ((pmcel->pFirst->vCross - vPos).Absolute() < fpErrorGap) )
		{
			return	pmcel->pFirst ;
		}
		if ( (pmcel->pLast != nullptr)
			&& (pmcel->pLast->iFaceEdge < 0)
			&& ((pmcel->pLast->vCross - vPos).Absolute() < fpErrorGap) )
		{
			return	pmcel->pLast ;
		}
		pmcelLast = pmcel ;
		pmcel = pmcel->pNextList ;
	}
	return	nullptr ;
}

// 交差頂点数計算
//////////////////////////////////////////////////////////////////////////////
size_t S3DBooleanMeshEditor::CountCrossPointOfEdgeList
			( S3DBooleanMeshEditor::MeshCrossEdgeList * pmcel )
{
	size_t	nCount = 0 ;
	while ( pmcel != nullptr )
	{
		MeshCrossEdge *	pmce = pmcel->pFirst ;
		while ( pmce != nullptr )
		{
			nCount ++ ;
			pmce = pmce->pNextEdge ;
		}
		pmcel = pmcel->pNextList ;
	}
	return	nCount ;
}

// 交差面を分割して出力
//////////////////////////////////////////////////////////////////////////////
void S3DBooleanMeshEditor::BuildMeshAtCrossingFace
	( S3DBooleanMeshEditor::MeshWorkBuffer& mwbuf,
		S3DBooleanMeshEditor::MeshCrossEdgeList * pmcel,
		const uint32_t * pFaceIndex,
		S3DBooleanMeshEditor::VertexBoolean vboolLogic, bool faceBack )
{
	const S3DVector4 *	pvVertex = mwbuf.m_mesh.m_bufVertex.GetConstArray() ;
	const int8_t *		pVBoolTable = mwbuf.m_vbt.GetConstArray() ;
	int8_t	vbVertex[3] =
	{
		pVBoolTable[pFaceIndex[0]],
		pVBoolTable[pFaceIndex[1]],
		pVBoolTable[pFaceIndex[2]],
	} ;
	bool	flagValidVertex[3] =
	{
		vbVertex[0] == vboolLogic,
		vbVertex[1] == vboolLogic,
		vbVertex[2] == vboolLogic,
	} ;
	bool		flagCornerVertex[3] ;
	S3DVector	vVertexPoint[3] ;
	S3DVector	vNextVertexPoint[3] =
	{
		pvVertex[pFaceIndex[1]],
		pvVertex[pFaceIndex[2]],
		pvVertex[pFaceIndex[0]],
	} ;
	for ( ; ; )
	{
		pvVertex = mwbuf.m_mesh.m_bufVertex.GetConstArray() ;
		for ( int i = 0; i < 3; i ++ )
		{
			vbVertex[i] = flagValidVertex[i] ? vboolLogic : elementUnknown ;
			flagCornerVertex[i] = flagValidVertex[i] ;
			vVertexPoint[i] = pvVertex[pFaceIndex[i]] ;
		}
		m_bufPolyIndex.RemoveAll() ;
		//
		ssize_t	iFaceEdge = 0 ;
		while ( (size_t) iFaceEdge < 3 )
		{
			//
			// 指定辺の交差点で、指定頂点に最近の交差点を検索する
			//
			S3DVector			vVertex = vVertexPoint[iFaceEdge] ;
			MeshCrossEdgeList *	pmcelLast = pmcel ;
			MeshCrossEdgeList *	pmcelCur = pmcel ;
			MeshCrossEdge *		pmceEdge =
				FindNearEdgeCrossPoint
					( pmcelCur, pmcelLast, iFaceEdge,
						vVertex, vNextVertexPoint[iFaceEdge] ) ;
			if ( pmceEdge == nullptr )
			{
				// この辺に交差点はない
				if ( flagValidVertex[iFaceEdge] && flagCornerVertex[iFaceEdge] )
				{
					// 表示頂点追加
					m_bufPolyIndex.Add( pFaceIndex[iFaceEdge] ) ;
					flagValidVertex[iFaceEdge] = false ;
				}
				iFaceEdge ++ ;
				continue ;
			}
			if ( vbVertex[iFaceEdge] == vboolLogic )
			{
				// 頂点を含むポリゴン
				if ( flagValidVertex[iFaceEdge] && flagCornerVertex[iFaceEdge] )
				{
					m_bufPolyIndex.Add( pFaceIndex[iFaceEdge] ) ;
					flagValidVertex[iFaceEdge] = false ;
				}
				bool	flagChain = false ;
				if ( pmcelCur->pFirst != pmceEdge )
				{
					ESLAssert( pmcelCur->pLast == pmceEdge ) ;
					if ( pmcelCur->pFirst->iFaceEdge >= 0 )
					{
						pmcelCur->Reverse() ;
						flagChain = true ;
					}
				}
				else
				{
					ESLAssert( pmcelCur->pFirst == pmceEdge ) ;
					if ( pmcelCur->pLast->iFaceEdge >= 0 )
					{
						flagChain = true ;
					}
				}
				ssize_t	iNextFaceEdge = iFaceEdge ;
				if ( flagChain )
				{
					AppendCrossPoints
						( mwbuf.m_mesh, m_bufPolyIndex, pFaceIndex, pmcelCur->pFirst ) ;
					//
					ESLAssert( pmcelCur->pLast->iFaceEdge >= 0 ) ;
					iNextFaceEdge = pmcelCur->pLast->iFaceEdge ;
				}
				else
				{
					AppendCrossPoints
						( mwbuf.m_mesh, m_bufPolyIndex,
							pFaceIndex, pmceEdge, pmceEdge->pNextEdge ) ;
				}
				vbVertex[iFaceEdge] = elementUnknown ;
				vVertexPoint[iFaceEdge] = pmcelCur->pFirst->vCross ;
				//
				iFaceEdge = iNextFaceEdge ;
				//
				vbVertex[iFaceEdge] = vboolLogic ;
				flagCornerVertex[iFaceEdge] = false ;
				vVertexPoint[iFaceEdge] = pmcelCur->pLast->vCross ;
			}
			else
			{
				// 頂点を含まないポリゴン
				bool	flagChain = false ;
				if ( pmcelCur->pLast != pmceEdge )
				{
					ESLAssert( pmcelCur->pFirst == pmceEdge ) ;
					if ( pmcelCur->pFirst->iFaceEdge >= 0 )
					{
						pmcelCur->Reverse() ;
						flagChain = true ;
					}
				}
				else
				{
					ESLAssert( pmcelCur->pLast == pmceEdge ) ;
					if ( pmcelCur->pFirst->iFaceEdge >= 0 )
					{
						flagChain = true ;
					}
				}
				if ( flagChain )
				{
					AppendCrossPoints
						( mwbuf.m_mesh, m_bufPolyIndex, pFaceIndex, pmcelCur->pFirst ) ;
				}
				else
				{
					AppendCrossPoints
						( mwbuf.m_mesh, m_bufPolyIndex,
							pFaceIndex, pmceEdge, pmceEdge->pNextEdge ) ;
				}
				//
				vbVertex[iFaceEdge] = vboolLogic ;
				flagCornerVertex[iFaceEdge] = false ;
				vVertexPoint[iFaceEdge] = pmcelCur->pLast->vCross ;
			}
			//
			// リストから削除
			//
			if ( pmcelLast != nullptr )
			{
				pmcelLast->pNextList = pmcelCur->pNextList ;
			}
			else
			{
				ESLAssert( pmcelCur == pmcel ) ;
				MeshCrossEdgeList *	pNextList = pmcel->pNextList ;
				if ( pNextList != nullptr )
				{
					*pmcel = *pNextList ;
				}
				else
				{
					pmcel->pFirst = nullptr ;
					pmcel->pLast = nullptr ;
				}
			}
		}
		//
		// ポリゴンの三角化
		//
		const size_t	nPolyVertexCount = m_bufPolyIndex.GetLength() ;
		if ( nPolyVertexCount < 3 )
		{
			break ;
		}
		if ( nPolyVertexCount == 3 )
		{
			m_bufTrianglizeIndex.SetLength( 3 ) ;
			m_bufTrianglizeIndex.SetAt( 0, 0 ) ;
			m_bufTrianglizeIndex.SetAt( 1, 1 ) ;
			m_bufTrianglizeIndex.SetAt( 2, 2 ) ;
		}
		else
		{
			//
			// 多角形（凹な形状可）を三角形に変換する
			//
			S3DVector4 *	pvTrianglize =
					m_bufTrianglizeVertex.GetArray( nPolyVertexCount ) ;
			for ( size_t i = 0; i < nPolyVertexCount; i ++ )
			{
				pvTrianglize[i] = mwbuf.m_mesh.m_bufVertex.At( m_bufPolyIndex.At(i) ) ;
			}
			m_bufTrianglizeIndex.RemoveAll() ;
			m_bufTrianglizeWork.RemoveAll() ;
			m_bufTrianglizeIndex.SetLimit( nPolyVertexCount * 3 ) ;
			//
			S3DMeshShaper::MakeTriangleIndexedList
				( m_bufTrianglizeIndex, m_bufTrianglizeWork,
									pvTrianglize, nPolyVertexCount ) ;
			m_bufTrianglizeVertex.FinishArray() ;
		}
		//
		// くり抜き判定
		//
		pvVertex = mwbuf.m_mesh.m_bufVertex.GetConstArray() ;
		MeshCrossEdgeList *	pmcelGouge =
			GetGougeOutEdgeList
				( pmcel, pvVertex,
					m_bufPolyIndex.GetConstArray(),
					m_bufTrianglizeIndex.GetConstArray(),
					m_bufTrianglizeIndex.GetLength() ) ;
		if ( pmcelGouge != nullptr )
		{
			AppendGougeOutEdgePoints
				( mwbuf.m_mesh, m_bufPolyIndex,
					m_bufTrianglizeIndex.GetConstArray(),
					m_bufTrianglizeIndex.GetLength(), pFaceIndex, pmcelGouge ) ;
		}
		//
		// 出力
		//
		const uint32_t *	pTriangleIndex = m_bufTrianglizeIndex.GetConstArray() ;
		const size_t		nTriangleIndexCount = m_bufTrianglizeIndex.GetLength() ;
		for ( size_t i = 0; i + 2 < nTriangleIndexCount; i += 3 )
		{
			if ( faceBack )
			{
				mwbuf.m_mesh.m_bufIndex.Add( m_bufPolyIndex.At( pTriangleIndex[i + 2] ) ) ;
				mwbuf.m_mesh.m_bufIndex.Add( m_bufPolyIndex.At( pTriangleIndex[i + 1] ) ) ;
				mwbuf.m_mesh.m_bufIndex.Add( m_bufPolyIndex.At( pTriangleIndex[i] ) ) ;
			}
			else
			{
				mwbuf.m_mesh.m_bufIndex.Add( m_bufPolyIndex.At( pTriangleIndex[i] ) ) ;
				mwbuf.m_mesh.m_bufIndex.Add( m_bufPolyIndex.At( pTriangleIndex[i + 1] ) ) ;
				mwbuf.m_mesh.m_bufIndex.Add( m_bufPolyIndex.At( pTriangleIndex[i + 2] ) ) ;
			}
			mwbuf.m_mesh.m_nIndexCount += 3 ;
		}
	}
}

// 指定辺の交差点で、指定頂点に最近の交差点を検索する
//////////////////////////////////////////////////////////////////////////////
S3DBooleanMeshEditor::MeshCrossEdge *
	S3DBooleanMeshEditor::FindNearEdgeCrossPoint
		( S3DBooleanMeshEditor::MeshCrossEdgeList*& pmcelCur,
			S3DBooleanMeshEditor::MeshCrossEdgeList*& pmcelLast,
			ssize_t iFaceEdge,
			const S3DVector& vPos, const S3DVector& vNextPos )
{
	MeshCrossEdgeList *	pmcel = pmcelCur ;
	MeshCrossEdgeList *	pmcelTempLast = nullptr ;
	MeshCrossEdgeList *	pmcelNearestLast = nullptr ;
	MeshCrossEdgeList *	pmcelNearestCur = nullptr ;
	MeshCrossEdge *		pmceNearest = nullptr ;
	S3DVector			vNextDir = vNextPos - vPos ;
	float32_t			fpNearest = (float32_t) vNextDir.Absolute() ;
	if ( fpNearest < 0.000001f )
	{
		return	nullptr ;
	}
	vNextDir *= 1.0f / fpNearest ;
	fpNearest += 1.0f ;
	//
	while ( pmcel != nullptr )
	{
		if ( (pmcel->pFirst != nullptr) /*&& (pmcel->pFirst->iFaceEdge >= 0)*/
			&& (pmcel->pLast != nullptr) /*&& (pmcel->pLast->iFaceEdge >= 0)*/ )
		{
			if ( pmcel->pFirst->iFaceEdge == iFaceEdge )
			{
				float32_t	d = vNextDir.InnerProduct( pmcel->pFirst->vCross - vPos ) ;
				if ( (d > 0.0f) && (d < fpNearest) )
				{
					pmcelNearestLast = pmcelTempLast ;
					pmcelNearestCur = pmcel ;
					pmceNearest = pmcel->pFirst ;
					fpNearest = d ;
				}
			}
			if ( pmcel->pLast->iFaceEdge == iFaceEdge )
			{
				float32_t	d = vNextDir.InnerProduct( pmcel->pLast->vCross - vPos ) ;
				if ( (d > 0.0f) && (d < fpNearest) )
				{
					pmcelNearestLast = pmcelTempLast ;
					pmcelNearestCur = pmcel ;
					pmceNearest = pmcel->pLast ;
					fpNearest = d ;
				}
			}
		}
		pmcelTempLast = pmcel ;
		pmcel = pmcel->pNextList ;
	}
	pmcelLast = pmcelNearestLast ;
	pmcelCur = pmcelNearestCur ;
	return	pmceNearest ;
}

// くり抜き交差点リストを検索し、リストから分離する
//////////////////////////////////////////////////////////////////////////////
S3DBooleanMeshEditor::MeshCrossEdgeList *
	S3DBooleanMeshEditor::GetGougeOutEdgeList
	( S3DBooleanMeshEditor::MeshCrossEdgeList * pmcel,
		const S3DVector4 * pvVertex, const uint32_t * pPolyIndex,
		const uint32_t * pTriangleIndex, size_t nTriangleIndexCount )
{
	MeshCrossEdgeList *	pmcelFirst = pmcel ;
	MeshCrossEdgeList *	pmcelLast = nullptr ;
	while ( pmcel != nullptr )
	{
		if ( (pmcel->pFirst != nullptr) && (pmcel->pFirst->iFaceEdge < 0)
			&& (pmcel->pLast != nullptr) && (pmcel->pLast->iFaceEdge < 0) )
		{
			MeshCrossEdge *	pmceNext = pmcel->pFirst ;
			while ( pmceNext != nullptr )
			{
				for ( size_t i = 0; i + 2 < nTriangleIndexCount; i += 3 )
				{
					if ( IsPointInTriangle
						( pvVertex[pPolyIndex[pTriangleIndex[i]]],
							pvVertex[pPolyIndex[pTriangleIndex[i+1]]],
							pvVertex[pPolyIndex[pTriangleIndex[i+2]]],
							pmceNext->vCross ) )
					{
						// リストから分離して返却
						if ( pmcelLast != nullptr )
						{
							pmcelLast->pNextList = pmcel->pNextList ;
							return	pmcel ;
						}
						else
						{
							ESLAssert( pmcelFirst == pmcel ) ;
							MeshCrossEdgeList	mcelTemp = *pmcel ;
							MeshCrossEdgeList *	pmcelNext = pmcel->pNextList ;
							if ( pmcelNext != nullptr )
							{
								*pmcelFirst = *pmcelNext ;
							}
							else
							{
								pmcelNext = (MeshCrossEdgeList*)
										m_bufWork.Allocate( sizeof(MeshCrossEdgeList) ) ;
								eslFillMemory( pmcelNext, 0, sizeof(MeshCrossEdgeList) ) ;
								pmcelFirst->pNextList = nullptr ;
								pmcelFirst->pLast = nullptr ;
								pmcelFirst->pLast = nullptr ;
							}
							*pmcelNext = mcelTemp ;
							return	pmcelNext ;
						}
					}
				}
				pmceNext = pmceNext->pNextEdge ;
			}
		}
		pmcelLast = pmcel ;
		pmcel = pmcel->pNextList ;
	}
	return	nullptr ;
}

bool S3DBooleanMeshEditor::IsPointInTriangle
	( const S3DVector& v0, const S3DVector& v1,
		const S3DVector& v2, const S3DVector& vPos )
{
	S3DVector	vx0 = (v1 - v0) * (vPos - v0) ;
	S3DVector	vx1 = (v2 - v1) * (vPos - v1) ;
	S3DVector	vx2 = (v0 - v2) * (vPos - v2) ;
	return	((vx0.InnerProduct(vx1) >= 0.0f)
			&& (vx0.InnerProduct(vx2) >= 0.0f)) ;
}

// くり抜き処理
//////////////////////////////////////////////////////////////////////////////
void S3DBooleanMeshEditor::AppendGougeOutEdgePoints
	( S3DRenderBuffer::MeshBuffer& mesh,
		SSystem::SArray<uint32_t>& bufIndex,
		const uint32_t * pTriangleIndex, size_t nTriangleIndexCount,
		const uint32_t * pFaceIndex,
		S3DBooleanMeshEditor::MeshCrossEdgeList * pmcelGouge )
{
	//
	// 面の裏表判定
	//
	S3DVector	vFaceDir( 0, 0, 0 ) ;
	for ( size_t i = 0; i + 2 < nTriangleIndexCount; i += 3 )
	{
		S3DVector	v0 = mesh.m_bufVertex.At( bufIndex.At( pTriangleIndex[i] ) ) ;
		S3DVector	v1 = mesh.m_bufVertex.At( bufIndex.At( pTriangleIndex[i + 1] ) ) ;
		S3DVector	v2 = mesh.m_bufVertex.At( bufIndex.At( pTriangleIndex[i + 2] ) ) ;
		vFaceDir += (v1 - v0) * (v2 - v0) ;
	}
	//
	// 頂点収集
	//
	MeshCrossEdge *	pmce = pmcelGouge->pFirst ;
	m_bufGougeVertex.RemoveAll() ;
	while ( pmce != nullptr )
	{
		m_bufGougeVertex.Add( pmce->vCross ) ;
		pmce = pmce->pNextEdge ;
	}
	if ( m_bufGougeVertex.GetLength() < 3 )
	{
		return ;
	}
	//
	// くり抜きの頂点順序判定
	//
	m_bufGougeTrianglizeIndex.RemoveAll() ;
	m_bufTrianglizeWork.RemoveAll() ;
	//
	S3DMeshShaper::MakeTriangleIndexedList
		( m_bufGougeTrianglizeIndex, m_bufTrianglizeWork,
			m_bufGougeVertex.GetConstArray(), m_bufGougeVertex.GetLength() ) ;
	//
	S3DVector	vGougeFaceDir( 0, 0, 0 ) ;
	for ( size_t i = 0; i + 2 < m_bufGougeTrianglizeIndex.GetLength(); i += 3 )
	{
		S3DVector	v0 = m_bufGougeVertex.At( m_bufGougeTrianglizeIndex.At( i ) ) ;
		S3DVector	v1 = m_bufGougeVertex.At( m_bufGougeTrianglizeIndex.At( i + 1 ) ) ;
		S3DVector	v2 = m_bufGougeVertex.At( m_bufGougeTrianglizeIndex.At( i + 2 ) ) ;
		vGougeFaceDir += (v1 - v0) * (v2 - v0) ;
	}
	if ( vFaceDir.InnerProduct( vGougeFaceDir ) >= 0.0f )
	{
		pmcelGouge->Reverse() ;
	}
	//
	// 切れ込み点を検索
	//
	MeshCrossEdge *	pmceLast = nullptr ;
	MeshCrossEdge *	pmceNearest = nullptr ;
	MeshCrossEdge *	pmceNearestLast = nullptr ;
	double			fpNearest = 1.0e+9 ;
	S3DVector		vCutPoint = mesh.m_bufVertex.At( bufIndex.At(0) ) ;
	pmce = pmcelGouge->pFirst ;
	while ( pmce != nullptr )
	{
		double	d = (pmce->vCross - vCutPoint).Absolute() ;
		if ( d < fpNearest )
		{
			pmceNearest = pmce ;
			pmceNearestLast = pmceLast ;
			fpNearest = d ;
		}
		pmceLast = pmce ;
		pmce = pmce->pNextEdge ;
	}
	if ( pmceNearestLast != nullptr )
	{
		ESLAssert( pmceNearestLast->pNextEdge == pmceNearest ) ;
		pmceLast = pmcelGouge->pLast ;
		pmceLast->pNextEdge = pmcelGouge->pFirst ;
		pmcelGouge->pFirst = pmceNearest ;
		pmcelGouge->pLast = pmceNearestLast ;
		pmceNearestLast->pNextEdge = nullptr ;
	}
	//
	// 切り抜き輪郭
	//
	bufIndex.Add( bufIndex.At(0) ) ;
	//
	size_t	iCutFirst = mesh.m_nVertexCount ;
	AppendCrossPoints( mesh, bufIndex, pFaceIndex, pmcelGouge->pFirst ) ;
	//
	if ( iCutFirst < mesh.m_nVertexCount )
	{
		bufIndex.Add( (uint32_t) iCutFirst ) ;
	}
	//
	// ポリゴン三角化
	//
	const size_t	nPolyVertexCount = bufIndex.GetLength() ;
	S3DVector4 *	pvTrianglize =
			m_bufTrianglizeVertex.GetArray( nPolyVertexCount ) ;
	for ( size_t i = 0; i < nPolyVertexCount; i ++ )
	{
		pvTrianglize[i] = mesh.m_bufVertex.At( bufIndex.At(i) ) ;
	}
	m_bufTrianglizeIndex.RemoveAll() ;
	m_bufTrianglizeWork.RemoveAll() ;
	m_bufTrianglizeIndex.SetLimit( nPolyVertexCount * 3 ) ;
	//
	S3DMeshShaper::MakeTriangleIndexedList
		( m_bufTrianglizeIndex, m_bufTrianglizeWork,
							pvTrianglize, nPolyVertexCount ) ;
	m_bufTrianglizeVertex.FinishArray() ;
}

// 交差点を頂点として追加する
//////////////////////////////////////////////////////////////////////////////
void S3DBooleanMeshEditor::AppendCrossPoints
	( S3DRenderBuffer::MeshBuffer& mesh,
		SSystem::SArray<uint32_t>& bufIndex,
		const uint32_t * pFaceIndex,
		S3DBooleanMeshEditor::MeshCrossEdge * pmce,
		S3DBooleanMeshEditor::MeshCrossEdge * pmceEnd )
{
	//
	// 頂点をサンプリング
	//
	S3DVector	vVertex[3] =
	{
		mesh.m_bufVertex.At( pFaceIndex[0] ),
		mesh.m_bufVertex.At( pFaceIndex[1] ),
		mesh.m_bufVertex.At( pFaceIndex[2] ),
	} ;
	S3DVector	vNormal[3] =
	{
		mesh.m_bufNormal.At( pFaceIndex[0] ),
		mesh.m_bufNormal.At( pFaceIndex[1] ),
		mesh.m_bufNormal.At( pFaceIndex[2] ),
	} ;
	S2DVector	vUV[3] =
	{
		mesh.m_bufUVMap.At( pFaceIndex[0] ),
		mesh.m_bufUVMap.At( pFaceIndex[1] ),
		mesh.m_bufUVMap.At( pFaceIndex[2] ),
	} ;
	S3DColor	clrVertex[3] =
	{
		mesh.m_bufColor.At( pFaceIndex[0] ),
		mesh.m_bufColor.At( pFaceIndex[1] ),
		mesh.m_bufColor.At( pFaceIndex[2] ),
	} ;
	float32_t *	pfpExAttrs[3] ;
	m_bufExAttrTemp.SetLength( mesh.m_nExAttrCount * 3 ) ;
	pfpExAttrs[0] = m_bufExAttrTemp.GetArray() ;
	pfpExAttrs[1] = pfpExAttrs[0] + mesh.m_nExAttrCount ;
	pfpExAttrs[2] = pfpExAttrs[1] + mesh.m_nExAttrCount ;
	//
	vVertex[1] -= vVertex[0] ;
	vVertex[2] -= vVertex[0] ;
	//
	vNormal[1] -= vNormal[0] ;
	vNormal[2] -= vNormal[0] ;
	//
	vUV[1] -= vUV[0] ;
	vUV[2] -= vUV[0] ;
	//
	for ( size_t i = 0; i < mesh.m_nExAttrCount; i ++ )
	{
		pfpExAttrs[0][i] =
			mesh.m_bufExAttr.At( pFaceIndex[0] * mesh.m_nExAttrCount+ i ) ;
		pfpExAttrs[1][i] =
			mesh.m_bufExAttr.At( pFaceIndex[1] * mesh.m_nExAttrCount+ i ) ;
		pfpExAttrs[2][i] =
			mesh.m_bufExAttr.At( pFaceIndex[2] * mesh.m_nExAttrCount+ i ) ;
		//
		pfpExAttrs[1][i] -= pfpExAttrs[0][i] ;
		pfpExAttrs[2][i] -= pfpExAttrs[0][i] ;
	}
	//
	const MeshCrossEdge *	pmceFirst = pmce ;
	while ( pmce != pmceEnd )
	{
		ESLAssert( pmce != nullptr ) ;
		//
		// 各頂点属性を補完して追加
		//
		const size_t	iVertex = mesh.m_nVertexCount ++ ;
		bufIndex.Add( (uint32_t) iVertex ) ;
		//
		mesh.m_bufVertex.Add( pmce->vCross ) ;
		mesh.m_bufNormal.Add
			( (vNormal[0] + vNormal[1] * pmce->vCoord.x
							+ vNormal[2] * pmce->vCoord.y).Normalized() ) ;
		mesh.m_bufUVMap.Add
			( vUV[0] + vUV[1] * pmce->vCoord.x
							+ vUV[2] * pmce->vCoord.y ) ;
		//
		uint32_t	fx0 = (uint32_t) 
			esl_clampi( esl_roundfi
				( (1.0f - pmce->vCoord.x - pmce->vCoord.y) * 256.0f ), 0, 0x100 ) ;
		uint32_t	fx1 = (uint32_t) 
			esl_clampi( esl_roundfi( pmce->vCoord.x * 256.0f ), 0, 0x100 ) ;
		uint32_t	fx2 = (uint32_t) 
			esl_clampi( esl_roundfi( pmce->vCoord.y * 256.0f ), 0, 0x100 ) ;
		//
		mesh.m_bufColor.Add
			( clrVertex[0].imul(fx0) + clrVertex[1].imul(fx1) + clrVertex[2].imul(fx2) ) ;
		//
		for ( size_t i = 0; i < mesh.m_nExAttrCount; i ++ )
		{
			mesh.m_bufExAttr.Add
				( pfpExAttrs[0][i] + pfpExAttrs[1][i] * pmce->vCoord.x
									+ pfpExAttrs[2][i] * pmce->vCoord.y ) ;
		}
		//
		pmce = pmce->pNextEdge ;
		ESLAssert( pmceFirst != pmce ) ;
	}
}

// S3DMeshEditor へ変換する
//////////////////////////////////////////////////////////////////////////////
void S3DBooleanMeshEditor::ConvertToMeshEditor
	( S3DMeshEditor& mesh,
		const S3DBooleanMeshEditor::BooleanObject& bobj,
		const wchar_t * pwszBaseName,
		uint32_t nPatchFlags, ssize_t iMaterial )
{
	for ( size_t i = 0; i < bobj.m_buffers.GetLength(); i ++ )
	{
		MeshWorkBuffer&	mwb = bobj.m_buffers.At(i) ;
		if ( mwb.m_mesh.IsEmpty() )
		{
			continue ;
		}
		SString	strPatchName ;
		strPatchName.Format( L"%s%d", pwszBaseName, i ) ;
		//
		S3DMeshEditor::Patch *	pPatch = mesh.NewPatch() ;
		pPatch->SetName( strPatchName ) ;
		pPatch->SetFlags
			( pPatch->GetFlags() | nPatchFlags
					| S3DMeshEditor::Patch::flagFreezeNormal ) ;
		if ( iMaterial >= 0 )
		{
			pPatch->SetMaterialIndex( (size_t) iMaterial ) ;
		}
		else
		{
			pPatch->SetMaterialIndex( i ) ;
		}
		//
		ConvertToMeshEditorPatch( *pPatch, mwb ) ;
		//
		mesh.AddPatch( pPatch ) ;
	}
}

// S3DMeshEditor::Patch へ変換する
//////////////////////////////////////////////////////////////////////////////
void S3DBooleanMeshEditor::ConvertToMeshEditorPatch
	( S3DMeshEditor::Patch& patch, const S3DBooleanMeshEditor::MeshWorkBuffer& mwb )
{
	patch.MakeTriangleMeshFrom( mwb.m_mesh ) ;
}



//////////////////////////////////////////////////////////////////////////////
// ブーリアン・メッシュコントローラー
//////////////////////////////////////////////////////////////////////////////

const SSystem::SXMLDocument::AttrInteger
	S3DBooleanMeshController::m_aiBooleanType[booleanTypeCount+1] =
{
	{ L"nothing", booleanNothing },
	{ L"cut", booleanCut },
	{ L"cut_out", booleanCutOut },
	{ L"and", booleanAnd },
	{ L"gouge_out", booleanGougeOut },
	{ L"or", booleanOr },
	{ L"material", booleanMaterial },
	{ nullptr, 0 },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DBooleanMeshController, MeshController, S3DMeshEditorInterface )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DBooleanMeshController, boolean_mesh )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DBooleanMeshController::S3DBooleanMeshController( void )
	: MeshController( m_ItemClassDescriptor.pwszClassID ),
		m_type( booleanNothing ), m_flagEachOp( false ), m_iMaterial( 1 )
{
	PrepareParameterEntryCount( paramCount ) ;
	AddParameterEntry
		( L"mesh_editor", S3DSceneComposer::typeBinary,
			S3DSceneComposer::attrConstant,
			L"メッシュ", nullptr ) ;
	AddParameterEntry
		( L"boolean_type", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration,
			L"ブーリアン", nullptr ) ;
	AddParameterEntry
		( L"each_operation", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"当たり判定別処理", nullptr ) ;
	AddParameterEntry
		( L"replace_material", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"置き換えマテリアル", nullptr ) ;
	//
	m_mesh.AttachExtraMeshInfo( this ) ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
bool S3DBooleanMeshController::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramEachOperation:
		return	m_flagEachOp ;
	}
	return	false ;
}

int32_t S3DBooleanMeshController::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramReplaceMaterial:
		return	m_iMaterial ;
	}
	return	0 ;
}

const wchar_t * S3DBooleanMeshController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramBooleanMesh:
		return	((S3DBooleanMeshController*)this)->m_mesh.Serialize() ;

	case	paramBooleanType:
		return	SXMLDocument::GetSymbolAsIntegerOf( m_aiBooleanType, m_type ) ;
	}
	return	nullptr ;
}

size_t S3DBooleanMeshController::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t i ) const
{
	switch ( i )
	{
	case	paramBooleanMesh:
		{
			size_t	nHeaderBytes ;
			size_t	nTotalBytes =
						((S3DBooleanMeshController*)this)->
									m_mesh.SerializeBuffer( nHeaderBytes ) ;
			if ( pDst == nullptr )
			{
				return	nTotalBytes ;
			}
			if ( nBufBytes == sizeof(S3DSceneComposer::BinaryHeader) )
			{
				S3DSceneComposer::BinaryHeader *
					pbh = (S3DSceneComposer::BinaryHeader*) pDst ;
				pbh->nType = S3DSceneComposer::binaryMeshEditor ;
				pbh->nSubType = 0 ;
				pbh->nBodyBytes =
					(uint32_t) (nTotalBytes - sizeof(S3DSceneComposer::BinaryHeader)) ;
				pbh->nReserved = 0 ;
				return	sizeof(S3DSceneComposer::BinaryHeader) ;
			}
			if ( nBufBytes == nTotalBytes )
			{
				((S3DBooleanMeshController*)this)->
					m_mesh.SerializeBinary
						( (uint8_t*) pDst, nTotalBytes, nHeaderBytes ) ;
				return	nTotalBytes ;
			}
		}
		break ;
	}
	return	0 ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DBooleanMeshController::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramEachOperation:
		m_flagEachOp = b ;
		return ;
	}
}

void S3DBooleanMeshController::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramReplaceMaterial:
		m_iMaterial = n ;
		return ;
	}
}

void S3DBooleanMeshController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramBooleanMesh:
		m_mesh.Deserialize( pwszCmd ) ;
		return ;

	case	paramBooleanType:
		m_type = (BooleanType)
			SXMLDocument::GetIntegerAsSymbolOf( m_aiBooleanType, pwszCmd, m_type ) ;
		return ;
	}
}

size_t S3DBooleanMeshController::SetBinaryParameter
	( size_t i, const void * pSrc, size_t nBufBytes )
{
	switch ( i )
	{
	case	paramBooleanMesh:
		m_mesh.DeserializeBinary( pSrc, nBufBytes ) ;
		return	nBufBytes ;
	}
	return	0 ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DBooleanMeshController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	switch ( i )
	{
	case	paramBooleanType:
		{
			for ( size_t j = 0; m_aiBooleanType[j].pszSymbol != nullptr; j ++ )
			{
				aStrSet.Add( new SString(m_aiBooleanType[j].pszSymbol) ) ;
			}
		}
		return	true ;
	}
	return	false ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DBooleanMeshController::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"ブーリアン" ;
	}
	return	nullptr ;
}

// メッシュの変形
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditor * S3DBooleanMeshController::ModifyMeshEditor
	( S3DMeshEditor& meshTempBuf, const S3DMeshEditor& meshOrgRef )
{
	//
	// ソース MeshEditor 取得
	//
	S3DMeshEditorSerializer *	pMeshEditorItem =
			ESLTypeCast<S3DMeshEditorSerializer>( GetOwnerItem() ) ;
	if ( pMeshEditorItem == nullptr )
	{
		return	&meshTempBuf ;
	}
	S3DMeshEditorObject *	pMeshEditorObj =
		ESLTypeCast<S3DMeshEditorObject>( &(pMeshEditorItem->MeshEditor()) ) ;
	if ( pMeshEditorObj == nullptr )
	{
		return	&meshTempBuf ;
	}
	const S3DMeshEditor::MeshParam&
			mparam = pMeshEditorObj->GetMeshParameter() ;
	if ( (mparam.nFlags & S3DMeshEditor::flagMeshPrimitiveType)
		&& (mparam.nPrimitiveType != primitiveTriangle) )
	{
		return	&meshTempBuf ;
	}
	if ( m_type == booleanNothing )
	{
		m_mesh.UpdateAllPatchs( mparam ) ;
		//
		for ( size_t i = 0; i < m_mesh.GetPatchCount(); i ++ )
		{
			S3DMeshEditor::Patch *	pNewPatch = meshTempBuf.NewPatch() ;
			S3DMeshEditor::Patch *	pSrcPatch = m_mesh.GetPatchAt(i) ;
			pNewPatch->CopyFrom( *pSrcPatch ) ;
			meshTempBuf.AddPatch( pNewPatch ) ;
		}
		return	&meshTempBuf ;
	}
	//
	S3DMaterial *	pMaterials[S3DMeshEditorSerializer::paramMaterialCount] ;
	for ( size_t i = 0; i < S3DMeshEditorSerializer::paramMaterialCount; i ++ )
	{
		pMaterials[i] = pMeshEditorItem->GetMeshMaterial( i ) ;
	}
	//
	// 演算対象メッシュ構築
	//
	static const S3DBooleanMeshEditor::VertexBoolean	s_vbLogicDst[booleanTypeCount] =
	{
		S3DBooleanMeshEditor::elementUnknown,
		S3DBooleanMeshEditor::elementOuter,
		S3DBooleanMeshEditor::elementInner,
		S3DBooleanMeshEditor::elementInner,
		S3DBooleanMeshEditor::elementOuter,
		S3DBooleanMeshEditor::elementOuter,
		S3DBooleanMeshEditor::elementOuter,
	} ;
	static const bool	s_faceBackDst[booleanTypeCount] =
	{
		false, false, false, false, false, false, false,
	} ;
	static const S3DBooleanMeshEditor::VertexBoolean	s_vbLogicDst2[booleanTypeCount] =
	{
		S3DBooleanMeshEditor::elementUnknown,
		S3DBooleanMeshEditor::elementUnknown,
		S3DBooleanMeshEditor::elementUnknown,
		S3DBooleanMeshEditor::elementUnknown,
		S3DBooleanMeshEditor::elementUnknown,
		S3DBooleanMeshEditor::elementUnknown,
		S3DBooleanMeshEditor::elementInner,
	} ;
	static const bool	s_faceBackDst2[booleanTypeCount] =
	{
		false, false, false, false, false, false, false,
	} ;
	static const S3DBooleanMeshEditor::VertexBoolean	s_vbLogicSrc[booleanTypeCount] =
	{
		S3DBooleanMeshEditor::elementUnknown,
		S3DBooleanMeshEditor::elementUnknown,
		S3DBooleanMeshEditor::elementUnknown,
		S3DBooleanMeshEditor::elementInner,
		S3DBooleanMeshEditor::elementInner,
		S3DBooleanMeshEditor::elementOuter,
		S3DBooleanMeshEditor::elementUnknown,
	} ;
	static const bool	s_faceBackSrc[booleanTypeCount] =
	{
		false, false, false, false, true, false, false,
	} ;
	S3DBooleanMeshEditor::BooleanObject	bobjSrc ;
	S3DBooleanMeshEditor::BooleanObject	bobjDst ;
	S3DBooleanMeshEditor::BooleanObject	bobjDst2 ;
	S3DBooleanMeshEditor::BooleanObject	bobjCol ;
	uint32_t	nDstBuildFlags = S3DMeshEditor::renderModifiable ;
	uint32_t	nColBuildFlags = nDstBuildFlags | S3DMeshEditor::renderCollision ;
	m_boolean.ResetBoolean() ;
	m_boolean.BuildObject
		( bobjSrc, m_mesh, mparam, 0,
			pMaterials, S3DMeshEditorSerializer::paramMaterialCount ) ;
	m_boolean.BuildObject
		( bobjDst, meshTempBuf, mparam, nDstBuildFlags,
			pMaterials, S3DMeshEditorSerializer::paramMaterialCount ) ;
	if ( s_vbLogicDst2[m_type] != S3DBooleanMeshEditor::elementUnknown )
	{
		m_boolean.BuildObject
			( bobjDst2, meshTempBuf, mparam, nDstBuildFlags,
				pMaterials, S3DMeshEditorSerializer::paramMaterialCount ) ;
	}
	if ( m_flagEachOp )
	{
		m_boolean.BuildObject
			( bobjCol, meshTempBuf, mparam, nColBuildFlags,
				pMaterials, S3DMeshEditorSerializer::paramMaterialCount ) ;
	}
	//
	// 交差処理
	//
	m_boolean.CrossTest( bobjDst, bobjSrc ) ;
	//
	if ( s_vbLogicDst2[m_type] != S3DBooleanMeshEditor::elementUnknown )
	{
		m_boolean.CrossTest( bobjDst2, bobjSrc ) ;
	}
	if ( m_flagEachOp )
	{
		m_boolean.CrossTest( bobjCol, bobjSrc ) ;
	}
	//
	// ブーリアン処理対象を削除する
	//
	bool	flagAllModifiable = true ;
	for ( size_t i = 0; i < meshTempBuf.GetPatchCount(); i ++ )
	{
		if ( meshTempBuf.GetPatchAt(i)->GetFlags()
				& S3DMeshEditor::Patch::flagDisableModifier )
		{
			flagAllModifiable = false ;
			break ;
		}
	}
	if ( flagAllModifiable )
	{
		meshTempBuf.RemoveAllPatchs() ;
	}
	else
	{
		size_t	iPatch = 0 ;
		while ( iPatch < meshTempBuf.GetPatchCount() )
		{
			if ( meshTempBuf.GetPatchAt(iPatch)->GetFlags()
						& S3DMeshEditor::Patch::flagDisableModifier )
			{
				iPatch ++ ;
			}
			else
			{
				meshTempBuf.RemovePatchAt( iPatch ) ;
			}
		}
	}
	//
	// メッシュを構築
	//
	m_boolean.BuildBooleanMesh
		( bobjDst, s_vbLogicDst[m_type], s_faceBackDst[m_type] ) ;
	m_boolean.ConvertToMeshEditor( meshTempBuf, bobjDst, L"bool_dst", 0 ) ;
	//
	if ( m_flagEachOp )
	{
		m_boolean.BuildBooleanMesh
			( bobjCol, s_vbLogicDst[m_type], s_faceBackDst[m_type] ) ;
		m_boolean.ConvertToMeshEditor
			( meshTempBuf, bobjCol, L"bool_col_dst",
				S3DMeshEditor::Patch::flagInvisible
					| S3DMeshEditor::Patch::flagCollision ) ;
	}
	//
	if ( s_vbLogicSrc[m_type] != S3DBooleanMeshEditor::elementUnknown )
	{
		m_boolean.BuildBooleanMesh
			( bobjSrc, s_vbLogicSrc[m_type], s_faceBackSrc[m_type] ) ;
		m_boolean.ConvertToMeshEditor( meshTempBuf, bobjSrc, L"bool_src", 0 ) ;
		//
		if ( m_flagEachOp )
		{
			m_boolean.ConvertToMeshEditor
				( meshTempBuf, bobjSrc, L"bool_col_src",
					S3DMeshEditor::Patch::flagInvisible
					| S3DMeshEditor::Patch::flagCollision ) ;
		}
	}
	if ( s_vbLogicDst2[m_type] != S3DBooleanMeshEditor::elementUnknown )
	{
		m_boolean.BuildBooleanMesh
			( bobjDst2, s_vbLogicDst2[m_type], s_faceBackDst2[m_type] ) ;
		m_boolean.ConvertToMeshEditor
			( meshTempBuf, bobjDst2, L"bool_dst2", 0, m_iMaterial ) ;
		//
		if ( m_flagEachOp )
		{
			m_boolean.ConvertToMeshEditor
				( meshTempBuf, bobjDst2, L"bool_col_src",
					S3DMeshEditor::Patch::flagInvisible
					| S3DMeshEditor::Patch::flagCollision ) ;
		}
	}
	return	&meshTempBuf ;
}

// S3DMeshEditor 取得
//////////////////////////////////////////////////////////////////////////////
const S3DMeshEditor& S3DBooleanMeshController::GetMeshEditor( void ) const
{
	return	m_mesh ;
}

S3DMeshEditor& S3DBooleanMeshController::MeshEditor( void )
{
	return	m_mesh ;
}

// 表示用メッシュの更新
//////////////////////////////////////////////////////////////////////////////
void S3DBooleanMeshController::UpdateViewMesh( void )
{
	S3DMeshEditorSerializer *
		pOwnerMeshEditor =
			ESLTypeCast<S3DMeshEditorSerializer>( GetOwnerItem() ) ;
	if ( pOwnerMeshEditor != nullptr )
	{
		S3DScene *	pScene = pOwnerMeshEditor->GetScene() ;
		if ( pScene != nullptr )
		{
			pOwnerMeshEditor->ProcessMeshControllers( *pScene ) ;
		}
	}
}

// 表示用マテリアルの取得
//////////////////////////////////////////////////////////////////////////////
S3DMaterial * S3DBooleanMeshController::GetMeshMaterial( size_t iMaterial ) const
{
	S3DMeshEditorInterface *
		pOwnerMeshEditor =
			ESLTypeCast<S3DMeshEditorInterface>( GetOwnerItem() ) ;
	if ( pOwnerMeshEditor != nullptr )
	{
		return	pOwnerMeshEditor->GetMeshMaterial( iMaterial ) ;
	}
	return	nullptr ;
}

// 空間
//////////////////////////////////////////////////////////////////////////////
void S3DBooleanMeshController::GetMeshItemMatrix( S3DDMatrix& mat, S3DDVector& pos ) const
{
	S3DSceneComposer::ItemSerializer *	pOwnerItem = GetOwnerItem() ;
	if ( pOwnerItem != nullptr )
	{
		pOwnerItem->GetGlobalTransformation( mat, pos ) ;
	}
	else
	{
		mat = S3DDMatrix( 1, 1, 1 ) ;
		pos = S3DDVector( 0, 0, 0 ) ;
	}
}




//////////////////////////////////////////////////////////////////////////////
// 簡易メッシュ・リダクション・コントローラー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DSimpleMeshReductionController, MeshController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DSimpleMeshReductionController, simple_mesh_reductor )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSimpleMeshReductionController::S3DSimpleMeshReductionController( void )
	: MeshController( m_ItemClassDescriptor.pwszClassID ),
		m_fpReducing( 0.5 ), m_nRepetition( 1 )
{
	ESLVerify( paramReducing == AddParameterEntry
		( L"reducing", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"削減閾値", nullptr, 0.0, 1.0 ) ) ;
	ESLVerify( paramRepetition == AddParameterEntry
		( L"repetition", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"反復回数", nullptr ) ) ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
double S3DSimpleMeshReductionController::GetScalarParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramReducing:
		return	m_fpReducing ;
	}
	return	MeshController::GetScalarParameter( iParam ) ;
}

int32_t S3DSimpleMeshReductionController::GetIntegerParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramRepetition:
		return	m_nRepetition ;
	}
	return	MeshController::GetIntegerParameter( iParam ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSimpleMeshReductionController::SetScalarParameter( size_t iParam, double s )
{
	switch ( iParam )
	{
	case	paramReducing:
		m_fpReducing = s ;
		return ;
	}
	MeshController::SetScalarParameter( iParam, s ) ;
}

void S3DSimpleMeshReductionController::SetIntegerParameter( size_t iParam, int32_t n )
{
	switch ( iParam )
	{
	case	paramRepetition:
		m_nRepetition = n ;
		return ;
	}
	MeshController::SetIntegerParameter( iParam, n ) ;
}

// メッシュの変形
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditor * S3DSimpleMeshReductionController::ModifyMeshEditor
	( S3DMeshEditor& meshTempBuf, const S3DMeshEditor& meshOrgRef )
{
	S3DMeshEditorSerializer *	pMeshEditorItem =
			ESLTypeCast<S3DMeshEditorSerializer>( GetOwnerItem() ) ;
	if ( pMeshEditorItem == nullptr )
	{
		return	&meshTempBuf ;
	}
	S3DMeshEditorObject *	pMeshEditorObj =
		ESLTypeCast<S3DMeshEditorObject>( &(pMeshEditorItem->MeshEditor()) ) ;
	if ( pMeshEditorObj == nullptr )
	{
		return	&meshTempBuf ;
	}
	//
	// メッシュをベイク
	//
	const size_t				nBufCount = S3DMeshEditorSerializer::paramMaterialCount ;
	S3DRenderBuffer				renderBufs[nBufCount] ;
	S3DVertexBufferInterface *	pVBOs[nBufCount] ;
	S3DMaterial *				pMaterials[nBufCount] ;
	for ( size_t i = 0; i < nBufCount; i ++ )
	{
		pVBOs[i] = &renderBufs[i] ;
		pMaterials[i] = pMeshEditorItem->GetMeshMaterial( i ) ;
	}
	const S3DMeshEditor::MeshParam&
					mparam = pMeshEditorObj->GetMeshParameter() ;
	SArray<size_t>	aExistingCount ;
	meshTempBuf.RenderVertexBuffers
		( pVBOs, pMaterials,
			aExistingCount.GetArray( nBufCount ),
			nBufCount, mparam, 0 ) ;
	aExistingCount.FinishArray() ;
	//
	// 出力先をクリア
	//
	meshTempBuf.RemoveAllPatchs() ;
	//
	// マテリアル毎の処理
	//
	for ( size_t iBuf = 0; iBuf < nBufCount; iBuf ++ )
	{
		if ( aExistingCount.At(iBuf) == 0 )
		{
			continue ;
		}
		//
		// メッシュを統合
		//
		S3DRenderBuffer::MeshBuffer	mbuf ;
		renderBufs[iBuf].GetMeshBufferAsSinglePrimitive( mbuf ) ;
		//
		// 減ポリゴン処理
		//
		S3DSimpleMeshReduction	reductor ;
		reductor.SetupTargetMesh( &mbuf ) ;
		//
		for ( int i = 0; (i < m_nRepetition) && (i < 8); i ++ )
		{
			if ( i > 0 )
			{
				reductor.RecycleMesh() ;
			}
			reductor.ReduceTriangles( m_fpReducing ) ;
		}
		//
		mbuf.m_nIndexCount = reductor.GetReducedIndexCount() ;
		mbuf.m_bufIndex.SetLength( 0 ) ;
		mbuf.m_bufIndex.AddArray
			( reductor.GetReducedIndexList(), mbuf.m_nIndexCount ) ;
		//
		// データとして追加
		//
		SString	strPatchName ;
		strPatchName.Format( L"reducted_%d", iBuf ) ;
		//
		S3DMeshEditor::Patch *	pPatch = meshTempBuf.NewPatch() ;
		pPatch->SetName( strPatchName ) ;
		pPatch->ModifyFlags( S3DMeshEditor::Patch::flagFreezeNormal, 0 ) ;
		pPatch->SetMaterialIndex( iBuf ) ;
		//
		pPatch->MakeTriangleMeshFrom( mbuf ) ;
		//
		meshTempBuf.AddPatch( pPatch ) ;
	}
	return	&meshTempBuf ;
}



//////////////////////////////////////////////////////////////////////////////
// メッシュデーター上書き・コントローラー（ベイク済みデータ保持用）
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DBakedMeshController, MeshController, S3DMeshEditorInterface )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DBakedMeshController, baked_mesh )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DBakedMeshController::S3DBakedMeshController( void )
	: MeshController( m_ItemClassDescriptor.pwszClassID )
{
	PrepareParameterEntryCount( paramCount ) ;
	AddParameterEntry
		( L"mesh_editor", S3DSceneComposer::typeBinary,
			S3DSceneComposer::attrConstant,
			L"メッシュ", nullptr ) ;
	//
	m_mesh.AttachExtraMeshInfo( this ) ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DBakedMeshController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramBakedMesh:
		return	const_cast<S3DBakedMeshController*>(this)->m_mesh.Serialize() ;
	}
	return	nullptr ;
}

size_t S3DBakedMeshController::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t i ) const
{
	switch ( i )
	{
	case	paramBakedMesh:
		{
			size_t	nHeaderBytes ;
			size_t	nTotalBytes =
						const_cast<S3DBakedMeshController*>(this)->
									m_mesh.SerializeBuffer( nHeaderBytes ) ;
			if ( pDst == nullptr )
			{
				return	nTotalBytes ;
			}
			if ( nBufBytes == sizeof(S3DSceneComposer::BinaryHeader) )
			{
				S3DSceneComposer::BinaryHeader *
					pbh = (S3DSceneComposer::BinaryHeader*) pDst ;
				pbh->nType = S3DSceneComposer::binaryMeshEditor ;
				pbh->nSubType = 0 ;
				pbh->nBodyBytes =
					(uint32_t) (nTotalBytes - sizeof(S3DSceneComposer::BinaryHeader)) ;
				pbh->nReserved = 0 ;
				return	sizeof(S3DSceneComposer::BinaryHeader) ;
			}
			if ( nBufBytes == nTotalBytes )
			{
				const_cast<S3DBakedMeshController*>(this)->
					m_mesh.SerializeBinary
						( (uint8_t*) pDst, nTotalBytes, nHeaderBytes ) ;
				return	nTotalBytes ;
			}
		}
		break ;
	}
	return	0 ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DBakedMeshController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramBakedMesh:
		m_mesh.Deserialize( pwszCmd ) ;
		return ;
	}
}

size_t S3DBakedMeshController::SetBinaryParameter
	( size_t i, const void * pSrc, size_t nBufBytes )
{
	switch ( i )
	{
	case	paramBakedMesh:
		m_mesh.DeserializeBinary( pSrc, nBufBytes ) ;
		return	nBufBytes ;
	}
	return	0 ;
}

// メッシュの変形
//////////////////////////////////////////////////////////////////////////////
S3DMeshEditor * S3DBakedMeshController::ModifyMeshEditor
	( S3DMeshEditor& meshTempBuf, const S3DMeshEditor& meshOrgRef )
{
//	S3DMeshEditor::MeshParam	mparam ;
//	m_mesh.UpdateAllPatchs( mparam ) ;
	//
	meshTempBuf.DuplicateMesh( m_mesh ) ;
	return	&meshTempBuf ;
}

// S3DMeshEditor 取得
//////////////////////////////////////////////////////////////////////////////
const S3DMeshEditor& S3DBakedMeshController::GetMeshEditor( void ) const
{
	return	m_mesh ;
}

S3DMeshEditor& S3DBakedMeshController::MeshEditor( void )
{
	return	m_mesh ;
}

// 表示用メッシュの更新
//////////////////////////////////////////////////////////////////////////////
void S3DBakedMeshController::UpdateViewMesh( void )
{
	S3DMeshEditorSerializer *
		pOwnerMeshEditor =
			ESLTypeCast<S3DMeshEditorSerializer>( GetOwnerItem() ) ;
	if ( pOwnerMeshEditor != nullptr )
	{
		S3DScene *	pScene = pOwnerMeshEditor->GetScene() ;
		if ( pScene != nullptr )
		{
			pOwnerMeshEditor->ProcessMeshControllers( *pScene ) ;
		}
	}
}

// 表示用マテリアルの取得
//////////////////////////////////////////////////////////////////////////////
S3DMaterial * S3DBakedMeshController::GetMeshMaterial( size_t iMaterial ) const
{
	S3DMeshEditorInterface *
		pOwnerMeshEditor =
			ESLTypeCast<S3DMeshEditorInterface>( GetOwnerItem() ) ;
	if ( pOwnerMeshEditor != nullptr )
	{
		return	pOwnerMeshEditor->GetMeshMaterial( iMaterial ) ;
	}
	return	nullptr ;
}

// 空間
//////////////////////////////////////////////////////////////////////////////
void S3DBakedMeshController::GetMeshItemMatrix( S3DDMatrix& mat, S3DDVector& pos ) const
{
	S3DSceneComposer::ItemSerializer *	pOwnerItem = GetOwnerItem() ;
	if ( pOwnerItem != nullptr )
	{
		pOwnerItem->GetGlobalTransformation( mat, pos ) ;
	}
	else
	{
		mat = S3DDMatrix( 1, 1, 1 ) ;
		pos = S3DDVector( 0, 0, 0 ) ;
	}
}


