
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2013 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// Cotopha Script コンパイル時型情報
//////////////////////////////////////////////////////////////////////////////

// タイプオブジェクト複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSTypeInfo::DuplicateType( ECSObject * pType )
{
	ECSObject *	pDupType = NULL ;
	if ( pType != NULL )
	{
		if ( pType->m_vtType == csvtPointer )
		{
			ECSPointer *	pPtr = new ECSPointer ;
			pPtr->SetOwnObject
				( DuplicateType( ((ECSPointer*)pType)->m_pRef ) ) ;
			pPtr->m_fReadOnly = ((ECSPointer*)pType)->m_fReadOnly ;
			pDupType = pPtr ;
		}
		else if ( pType->m_vtType == csvtReference )
		{
			ECSReference *	pRef = new ECSReference ;
			pRef->SetOwnObject
				( DuplicateType( ((ECSReference*)pType)->m_pRef ) ) ;
			pDupType = pRef ;
		}
		else
		{
			pDupType = pType->Duplicate() ;
		}
	}
	return	pDupType ;
}

// 参照型生成
//////////////////////////////////////////////////////////////////////////////
void ECSTypeInfo::MakeReferenceOf( const ECSTypeInfo & typeinf )
{
	ECSReference *	pRef = new ECSReference ;
	if ( typeinf.m_pValue != NULL )
	{
		pRef->SetOwnObject( DuplicateType( typeinf.m_pValue ) ) ;
	}
	SetTypeValue( pRef, typeinf.m_dwFlags ) ;
}

// ポインタ型生成
//////////////////////////////////////////////////////////////////////////////
void ECSTypeInfo::MakePointerOf( const ECSTypeInfo & typeinf )
{
	ECSPointer *	pPtr = new ECSPointer ;
	if ( typeinf.m_pValue != NULL )
	{
		pPtr->SetOwnObject( DuplicateType( typeinf.GetNakedType() ) ) ;
	}
	pPtr->m_fReadOnly = ((typeinf.m_dwFlags & flagConstant) != 0) ;
	//
	SetTypeValue( pPtr, (typeinf.m_dwFlags & ~flagConstant) ) ;
}

// 参照解除した型情報生成
//////////////////////////////////////////////////////////////////////////////
void ECSTypeInfo::MakeNakedOf( const ECSTypeInfo & typeinf )
{
	SetTypeValue
		( DuplicateType( typeinf.GetNakedType() ), typeinf.m_dwFlags ) ;
}

// ポインタ解除した型情報生成
//////////////////////////////////////////////////////////////////////////////
void ECSTypeInfo::MakeNakedPointerOf( const ECSTypeInfo & typeinf )
{
	ECSObject *	pObj = typeinf.GetNakedType() ;
	if ( pObj != NULL )
	{
		if ( pObj->m_vtType == csvtPointer )
		{
			ECSPointer *	pPtr = (ECSPointer*) pObj ;
			SetTypeValue
				( DuplicateType( pPtr->m_pRef ),
					(typeinf.m_dwFlags & ~flagConstant)
						| (pPtr->m_fReadOnly ? flagConstant : 0) ) ;
		}
		else
		{
			SetTypeValue( DuplicateType( pObj ), typeinf.m_dwFlags ) ;
		}
	}
	else
	{
		SetTypeValue( NULL, typeinf.m_dwFlags ) ;
	}
}

// 整数定数値の型を最小サイズに正規化する
//////////////////////////////////////////////////////////////////////////////
void ECSTypeInfo::NormalzieImmediateIntegerType( void )
{
	if ( (m_pValue == NULL) || !(m_dwFlags & flagDeterministic) )
	{
		return ;
	}
	if ( m_pValue->m_vtType != csvtInteger )
	{
		return ;
	}
	ECSInteger *	pValue = (ECSInteger*) m_pValue ;
	INT64	maskInt = pValue->GetValueMask() ;
	INT64	nValue = pValue->GetValue() ;
	if ( (-1 <= nValue) && (nValue <= 0) )
	{
		maskInt = ECSInteger::m_maskBoolean ;
	}
	else if ( (-0x80 <= nValue) && (nValue <= 0x7F) )
	{
		if ( (maskInt != ECSInteger::m_maskBoolean)
			&& (maskInt != ECSInteger::m_maskUint8) )
		{
			maskInt = ECSInteger::m_maskInt8 ;
		}
	}
	else if ( (0 <= nValue) && (nValue <= 0xFF) )
	{
		if ( maskInt != ECSInteger::m_maskInt16 )
		{
			maskInt = ECSInteger::m_maskUint8 ;
		}
	}
	else if ( (-0x8000 <= nValue) && (nValue <= 0x7FFF) )
	{
		maskInt = ECSInteger::m_maskInt16 ;
	}
	else if ( (0 <= nValue) && (nValue <= 0xFFFF) )
	{
		if ( maskInt != ECSInteger::m_maskInt32 )
		{
			maskInt = ECSInteger::m_maskUint16 ;
		}
	}
	else if ( (-(INT64) 0x80000000L <= nValue) && (nValue <= 0x7FFFFFFF) )
	{
		maskInt = ECSInteger::m_maskInt32 ;
	}
	else if ( (0 <= nValue) && (nValue <= (UINT64) 0xFFFFFFFFUL) )
	{
		maskInt = ECSInteger::m_maskUint32 ;
	}
	else if ( nValue < 0 )
	{
		maskInt = ECSInteger::m_maskInt64 ;
	}
	pValue->SetValueMask( maskInt ) ;
}

// 比較
//////////////////////////////////////////////////////////////////////////////
bool ECSTypeInfo::IsTypeEqual( const ECSTypeInfo & typeinf ) const
{
	const DWORD	dwTypeFlags = flagConstant | flagNakedBuffer ;
	if ( (m_dwFlags & dwTypeFlags) == (typeinf.m_dwFlags & dwTypeFlags) )
	{
		return	IsTypeEqual( m_pValue, typeinf.m_pValue ) ;
	}
	return	false ;
}

bool ECSTypeInfo::IsTypeEqual
		( const ECSObject * pDstObj, const ECSObject * pSrcObj )
{
	for ( ; ; )
	{
		if ( pDstObj == NULL )
		{
			if ( pSrcObj == NULL )
			{
				return	true ;
			}
			return	false ;
		}
		else if ( pSrcObj == NULL )
		{
			return	false ;
		}
		if ( EWideString::Compare
			( pDstObj->GetTypeName(), pSrcObj->GetTypeName() ) )
		{
			return	false ;
		}
		if ( pDstObj->m_vtType == csvtReference )
		{
			ESLAssert( pSrcObj->m_vtType == csvtReference ) ;
			if ( pSrcObj->m_vtType != csvtReference )
			{
				return	false ;
			}
			pDstObj = ((ECSReference*)pDstObj)->m_pRef ;
			pSrcObj = ((ECSReference*)pSrcObj)->m_pRef ;
			continue ;
		}
		if ( EWideString::Compare
			( pDstObj->OperateTypeOf(), pSrcObj->OperateTypeOf() ) )
		{
			return	false ;
		}
		if ( pDstObj->m_vtType == csvtArray )
		{
			ESLAssert( pSrcObj->m_vtType == csvtArray ) ;
			if ( pSrcObj->m_vtType != csvtArray )
			{
				return	false ;
			}
			const ECSArray *	pDstArray = (const ECSArray *) pDstObj ;
			const ECSArray *	pSrcArray = (const ECSArray *) pSrcObj ;
			int	nDstDim = pDstArray->GetDimension() ;
			int	nSrcDim = pSrcArray->GetDimension() ;
			unsigned int *	pDstBounds = new unsigned int[nDstDim] ;
			unsigned int *	pSrcBounds = new unsigned int[nSrcDim] ;
			nDstDim = pDstArray->GetDimensionSize( pDstBounds, nDstDim ) ;
			nSrcDim = pSrcArray->GetDimensionSize( pSrcBounds, nSrcDim ) ;
			if ( nDstDim != nSrcDim )
			{
				delete [] pDstBounds ;
				delete [] pSrcBounds ;
				return	false ;
			}
			for ( int i = 0; i < nDstDim; i ++ )
			{
				if ( pDstBounds[i] != pSrcBounds[i] )
				{
					delete [] pDstBounds ;
					delete [] pSrcBounds ;
					return	false ;
				}
			}
			delete [] pDstBounds ;
			delete [] pSrcBounds ;
			//
			pDstObj = pDstArray->GetEndDefaultElement() ;
			pSrcObj = pSrcArray->GetEndDefaultElement() ;
			continue ;
		}
		else if ( pDstObj->m_vtType == csvtHash )
		{
			ESLAssert( pSrcObj->m_vtType == csvtHash ) ;
			if ( pSrcObj->m_vtType != csvtHash )
			{
				return	false ;
			}
			const ECSHash *	pDstHash = (const ECSHash *) pDstObj ;
			const ECSHash *	pSrcHash = (const ECSHash *) pSrcObj ;
			pDstObj = pDstHash->m_pDefObj ;
			pSrcObj = pSrcHash->m_pDefObj ;
			continue ;
		}
		else if ( pDstObj->m_vtType == csvtPointer )
		{
			ESLAssert( pSrcObj->m_vtType == csvtPointer ) ;
			if ( pSrcObj->m_vtType != csvtPointer )
			{
				return	false ;
			}
			const ECSPointer *	pDstPtr = (const ECSPointer *) pDstObj ;
			const ECSPointer *	pSrcPtr = (const ECSPointer *) pSrcObj ;
			if ( pDstPtr->m_fReadOnly != pSrcPtr->m_fReadOnly )
			{
				return	false ;
			}
			pDstObj = pDstPtr->m_pRef ;
			pSrcObj = pSrcPtr->m_pRef ;
			continue ;
		}
		else if ( pDstObj->m_vtType == csvtFunction )
		{
			ESLAssert( pSrcObj->m_vtType == csvtFunction ) ;
			if ( pSrcObj->m_vtType != csvtFunction )
			{
				return	false ;
			}
			const ECSFunction *	pDstFunc = (const ECSFunction *) pDstObj ;
			const ECSFunction *	pSrcFunc = (const ECSFunction *) pSrcObj ;
			const DWORD	dwProtoMask =
				ECSTypeInfo::flagConstant | ECSTypeInfo::flagNativeObject
					| ECSTypeInfo::flagNakedCall | ECSTypeInfo::flagThisCall ;
			if ( (pDstFunc->m_prototype.GetAttribute() & dwProtoMask)
				!= (pSrcFunc->m_prototype.GetAttribute() & dwProtoMask) )
			{
				return	false ;
			}
			if ( pDstFunc->m_prototype.GetReturnType()
						!= pSrcFunc->m_prototype.GetReturnType() )
			{
				return	false ;
			}
			if ( !pDstFunc->m_prototype.IsArgumentEqual
					( pSrcFunc->m_prototype.GetArgument() ) )
			{
				return	false ;
			}
			if ( pDstFunc->m_pThisCall != NULL )
			{
				if ( pSrcFunc->m_pThisCall == NULL )
				{
					return	false ;
				}
				if ( pDstFunc->m_pThisCall->GetGlobalName()
						!= pSrcFunc->m_pThisCall->GetGlobalName() )
				{
					return	false ;
				}
			}
			else if ( pSrcFunc->m_pThisCall != NULL )
			{
				return	false ;
			}
			return	true ;
		}
		ESLAssert( pDstObj->m_vtType == pSrcObj->m_vtType ) ;
		return	true ;
	}
	return	false ;
}

// 型変換可能判定
//////////////////////////////////////////////////////////////////////////////
ECSTypeInfo::TypeMatchResult
	 ECSTypeInfo::IsMatchType
		( const ECSTypeInfo & typeSrc, TypeMatchResult matchLimit ) const
{
	if ( IsTypeReference()
		&& typeSrc.IsAbstractType()
		&& (typeSrc.m_dwFlags & ECSTypeInfo::flagDeterministic) )
	{
		return	typeMatch ;
	}
	if ( (typeSrc.m_dwFlags & flagConstant) && !(m_dwFlags & flagConstant) )
	{
		if ( IsTypeReference() )
		{
			return	typeNoMatch ;
		}
	}
	if ( IsAbstractType() )
	{
		return	typeMatch ;
	}
	if ( typeSrc.IsAbstractType() )
	{
		if ( (typeSrc.m_dwFlags & flagDeterministic) && IsTypePointer() )
		{
			return	typeNatualMatch ;
		}
		return	typeLooseMatch ;
	}
	const ECSObject *	pDstObj = m_pValue ;
	const ECSObject *	pSrcObj = typeSrc.m_pValue ;
	if ( pDstObj == NULL )
	{
		if ( pSrcObj == NULL )
		{
			return	typeNoMatch ;
		}
	}
	else if ( pSrcObj == NULL )
	{
		return	typeNoMatch ;
	}
	bool	fNeedEqual = false ;
	for ( ; ; )
	{
		if ( pDstObj->m_vtType == csvtReference )
		{
			pDstObj = ECSObject::GetEntity( pDstObj ) ;
			if ( pDstObj == NULL )
			{
				return	typeMatch ;
			}
			if ( fNeedEqual && (pSrcObj->m_vtType != csvtReference) )
			{
				return	typeNoMatch ;
			}
		}
		if ( pSrcObj->m_vtType == csvtReference )
		{
			pSrcObj = ECSObject::GetEntity( pSrcObj ) ;
			if ( pSrcObj == NULL )
			{
				return	typeLooseMatch ;
			}
			continue ;
		}
		if ( pSrcObj->m_vtType == csvtObject )
		{
			const ECSStructure *	pClassObj =
					ESLTypeCast<ECSStructure,ECSObject>( pSrcObj ) ;
			if ( (pClassObj == NULL)
				|| (pClassObj->m_pClassInf == NULL) )
			{
				return	(EWideString::Compare
							( pDstObj->OperateTypeOf(),
								pSrcObj->OperateTypeOf() ) == 0)
											? typeMatch : typeNoMatch ;
			}
			if ( EWideString::Compare
					( pDstObj->OperateTypeOf(),
						pSrcObj->OperateTypeOf() ) == 0 )
			{
				return	typeMatch ;
			}
			if ( !fNeedEqual && (matchLimit == typeNoMatch) )
			{
				const ECSClassInfo *	pClassInf = pClassObj->m_pClassInf ;
				ESLAssert( pClassInf != NULL ) ;
				return	pClassInf->CanCastTypeTo( *this )
										? typeCastableMatch : typeNoMatch ;
			}
			else
			{
				return	typeNoMatch ;
			}
		}
		if ( pDstObj->m_vtType == csvtPointer )
		{
			if ( !fNeedEqual
				&& (m_dwFlags & flagNakedBuffer)
				&& (typeSrc.m_dwFlags & flagNakedBuffer)
				&& (pSrcObj->m_vtType == csvtArray) )
			{
				const ECSPointer *	pDstPtr = (const ECSPointer *) pDstObj ;
				if ( !(pDstPtr->m_fReadOnly)
					&& (typeSrc.m_dwFlags & flagConstant) )
				{
					return	typeNoMatch ;
				}
				const ECSObject *	pDstPtrType = pDstPtr->m_pRef ;
				if ( pDstPtrType == NULL )
				{
					return	typeMatch ;
				}
				if ( IsTypeNakedArrayEqual( pDstPtrType, pSrcObj ) )
				{
					return	typeMatch ;
				}
				return	typeNoMatch ;
			}
			else if ( !fNeedEqual && (pSrcObj->m_vtType != csvtPointer) )
			{
				if ( (pSrcObj->m_vtType == csvtInteger)
					&& (typeSrc.m_dwFlags & flagDeterministic) )
				{
					INT64	nValue = ((ECSInteger*)pSrcObj)->GetValue() ;
					if ( nValue == 0 )
					{
						return	typeMatch ;
					}
				}
				if ( (pSrcObj->m_vtType == csvtReference)
					&& (typeSrc.m_dwFlags & flagDeterministic) )
				{
					if ( ((ECSReference*)pSrcObj)->m_pRef == NULL )
					{
						return	typeNatualMatch ;
					}
				}
				if ( (pSrcObj->m_vtType == csvtInteger)
							&& (m_dwFlags & flagNakedBuffer) )
				{
					return	typeLooseMatch ;
				}
				const ECSPointer *	pDstPtr = (const ECSPointer *) pDstObj ;
				const ECSObject *	pDstPtrType = pDstPtr->m_pRef ;
				if ( (pSrcObj->m_vtType == csvtString)
					&& (pDstPtrType != NULL)
					&& (pDstPtrType->m_vtType == csvtInteger)
					&& (((ECSInteger*)pDstPtrType)->GetIntegerType() == csvtUint16) )
				{
					return	typeNatualMatch ;
				}
				return	typeNoMatch ;
			}
			ECSTypeInfo::TypeMatchResult	macthResult = typeMatch ;
			if ( (m_dwFlags & flagNakedBuffer)
				&& !(typeSrc.m_dwFlags & flagNakedBuffer) )
			{
				macthResult = typeNatualMatch ;
			}
			const ECSPointer *	pDstPtr = (const ECSPointer *) pDstObj ;
			const ECSPointer *	pSrcPtr = (const ECSPointer *) pSrcObj ;
			if ( !(pDstPtr->m_fReadOnly) && pSrcPtr->m_fReadOnly )
			{
				return	typeNoMatch ;
			}
			const ECSObject *	pDstPtrType = pDstPtr->m_pRef ;
			const ECSObject *	pSrcPtrType = pSrcPtr->m_pRef ;
			if ( !fNeedEqual && (pDstPtrType == NULL) )
			{
				return	macthResult ;
			}
			if ( pSrcPtrType == NULL )
			{
				return	(pDstPtrType == NULL) ? typeMatch : typeLooseMatch ;
			}
			if ( IsTypeNakedArrayPointerMatch( pDstPtrType, pSrcPtrType ) )
			{
				return	macthResult ;
			}
			if ( !fNeedEqual
				&& (pSrcPtrType->m_vtType == csvtObject)
				&& (pSrcPtrType->m_pClassInf != NULL) )
			{
				//
				// 親クラスポインタへのキャスト
				//
				const ECSClassInfo *
					pSrcPtrClassInf = pSrcPtrType->m_pClassInf ;
				ECSClassInfo::CastInfo *	pCastInf =
					pSrcPtrClassInf->GetCastParentClassAs
								( pDstPtrType->OperateTypeOf() ) ;
				if ( (pCastInf != NULL) && (pCastInf->pClassInf != NULL) )
				{
					if ( pSrcPtrClassInf->IsNakedMemoryClass()
						!= pCastInf->pClassInf->IsNakedMemoryClass() )
					{
						return	typeNoMatch ;
					}
					return	typeNatualMatch ;
				}
			}
			return	typeNoMatch ;
		}
		else if ( pDstObj->m_vtType == csvtInteger )
		{
			if ( pSrcObj->m_vtType == csvtInteger )
			{
				int		nDstBits = ((ECSInteger*)pDstObj)->SizeOf() ;
				int		nSrcBits = ((ECSInteger*)pSrcObj)->SizeOf() ;
				bool	fDstSign = ((ECSInteger*)pDstObj)->IsSign() ;
				bool	fSrcSign = ((ECSInteger*)pSrcObj)->IsSign() ;
				if ( typeSrc.m_dwFlags & flagDeterministic )
				{
					INT64	nValue = ((ECSInteger*)pSrcObj)->GetValue() ;
					if ( nValue < 0 )
					{
						/*
						if ( !fDstSign )
						{
							return	typeLooseMatch ;
						}
						*/
						fSrcSign = true ;
						if ( nValue == -1 )
						{
							fSrcSign = fDstSign ;
							nSrcBits = 1 ;
						}
						else if ( nValue >= -0x80 )
						{
							nSrcBits = 8 ;
						}
						else if ( nValue >= -0x8000 )
						{
							nSrcBits = 16 ;
						}
						else if ( nValue >= - (INT64) 0x80000000 )
						{
							nSrcBits = 32 ;
						}
					}
					else
					{
						fSrcSign = false ;
						if ( nValue == 0 )
						{
							fSrcSign = fDstSign ;
							nSrcBits = 1 ;
						}
						else if ( nValue <= 0x7F )
						{
							nSrcBits = 7 ;
						}
						else if ( nValue <= 0xFF )
						{
							nSrcBits = 8 ;
						}
						else if ( nValue <= 0x7FFF )
						{
							nSrcBits = 15 ;
						}
						else if ( nValue <= 0xFFFF )
						{
							nSrcBits = 16 ;
						}
						else if ( nValue <= 0x7FFFFFFF )
						{
							nSrcBits = 31 ;
						}
						else if ( nValue <= 0xFFFFFFFF )
						{
							nSrcBits = 32 ;
						}
					}
				}
				if ( nDstBits >= nSrcBits )
				{
					return	typeNatualMatch ;
				}
				else if ( nDstBits < nSrcBits )
				{
					return	typeLooseMatch ;
				}
				if ( (fDstSign == fSrcSign) || (nDstBits == 64) )
				{
					return	typeMatch ;
				}
				return	typeLooseMatch ;
			}
			else if ( fNeedEqual )
			{
				return	typeNoMatch ;
			}
			else if ( pSrcObj->m_vtType == csvtReal )
			{
				return	typeLooseMatch ;
			}
			else if ( pSrcObj->m_vtType == csvtString )
			{
				return	typeLooseMatch ;
			}
			else if ( pSrcObj->m_vtType == csvtArray )
			{
				return	typeLooseMatch ;
			}
			else if ( pSrcObj->m_vtType == csvtPointer )
			{
				if ( typeSrc.m_dwFlags & ECSTypeInfo::flagNakedBuffer )
				{
					return	typeLooseMatch ;
				}
			}
			return	typeNoMatch ;
		}
		else if ( pDstObj->m_vtType == csvtReal )
		{
			switch ( pSrcObj->m_vtType )
			{
			case	csvtReal:
				return	typeMatch ;
			case	csvtInteger:
				if ( !fNeedEqual )
				{
					return	typeNatualMatch ;
				}
			case	csvtString:
			default:
				break ;
			}
			return	typeNoMatch ;
		}
		else if ( pDstObj->m_vtType == csvtString )
		{
			switch ( pSrcObj->m_vtType )
			{
			case	csvtInteger:
			case	csvtReal:
			default:
				break ;
			case	csvtString:
				return	typeMatch ;
			}
			return	typeNoMatch ;
		}
		else if ( pDstObj->m_vtType == csvtArray )
		{
			if ( pSrcObj->m_vtType != csvtArray )
			{
				return	typeNoMatch ;
			}
			const ECSArray *	pDstArray = (const ECSArray *) pDstObj ;
			const ECSArray *	pSrcArray = (const ECSArray *) pSrcObj ;
			int	nDstDim = pDstArray->GetDimension() ;
			int	nSrcDim = pSrcArray->GetDimension() ;
			unsigned int *	pDstBounds = new unsigned int[nDstDim] ;
			unsigned int *	pSrcBounds = new unsigned int[nSrcDim] ;
			nDstDim = pDstArray->GetDimensionSize( pDstBounds, nDstDim ) ;
			nSrcDim = pSrcArray->GetDimensionSize( pSrcBounds, nSrcDim ) ;
			ECSObject *	pDstElement = pDstArray->GetEndDefaultElement() ;
			ECSObject *	pSrcElement = pSrcArray->GetEndDefaultElement() ;
			if ( nDstDim != nSrcDim )
			{
				delete [] pDstBounds ;
				delete [] pSrcBounds ;
				//
				if ( pSrcElement == NULL )
				{
					return	typeLooseMatch ;
				}
				return	typeNoMatch ;
			}
			for ( int i = 0; i < nDstDim; i ++ )
			{
				if ( pDstBounds[i] != pSrcBounds[i] )
				{
					delete [] pDstBounds ;
					delete [] pSrcBounds ;
					//
					if ( pSrcElement == NULL )
					{
						return	typeLooseMatch ;
					}
					return	typeNoMatch ;
				}
			}
			delete [] pDstBounds ;
			delete [] pSrcBounds ;
			//
			pDstObj = pDstElement ;
			pSrcObj = pSrcElement ;
			//
			if ( pDstObj == NULL )
			{
				return	typeMatch ;
			}
			else if ( pSrcObj == NULL )
			{
				return	typeLooseMatch ;
			}
			if ( IsTypeEqual( pDstObj, pSrcObj ) )
			{
				return	typeMatch ;
			}
			return	typeNoMatch ;
		}
		else if ( pDstObj->m_vtType == csvtHash )
		{
			if ( pSrcObj->m_vtType != csvtHash )
			{
				return	typeNoMatch ;
			}
			const ECSHash *	pDstHash = (const ECSHash *) pDstObj ;
			const ECSHash *	pSrcHash = (const ECSHash *) pSrcObj ;
			if ( IsTypeEqual( pDstHash->m_pDefObj, pSrcHash->m_pDefObj ) )
			{
				return	typeMatch ;
			}
			if ( pSrcHash->m_pDefObj == NULL )
			{
				return	typeLooseMatch ;
			}
			return	typeNoMatch ;
		}
		else
		{
			const ECSStructure *	pClassObj =
				ESLTypeCast<ECSStructure,ECSObject>( pDstObj ) ;
			if ( (pClassObj == NULL)
				|| (pClassObj->m_pClassInf == NULL) )
			{
				return	(EWideString::Compare
							( pDstObj->OperateTypeOf(),
								pSrcObj->OperateTypeOf() ) == 0)
											? typeMatch : typeNoMatch ;
			}
			if ( fNeedEqual )
			{
				return	typeNoMatch ;
			}
			const ECSClassInfo *	pClassInf = pClassObj->m_pClassInf ;
			ESLAssert( pClassInf != NULL ) ;
			if ( pClassInf->IsIntegerEnumeratorType()
				|| pClassInf->IsRealEnumeratorType() )
			{
				if ( pClassInf->IsMatchEnumeratorValue( pSrcObj ) )
				{
					return	typeNatualMatch ;
				}
			}
			if ( matchLimit == typeNoMatch )
			{
				return	pClassInf->CanMoveTypeFrom( pSrcObj )
										? typeNatualMatch : typeNoMatch ;
			}
			else
			{
				return	typeNoMatch ;
			}
		}
	}
	return	typeNoMatch ;
}

// naked ポインタの配列キャスト判定 type* <- type[][][]...*
//////////////////////////////////////////////////////////////////////////////
bool ECSTypeInfo::IsTypeNakedArrayEqual
	( const ECSObject * pDstObj, const ECSObject * pSrcObj )
{
	for ( ; ; )
	{
		if ( IsTypeEqual( pDstObj, pSrcObj ) )
		{
			return	true ;
		}
		if ( pSrcObj->m_vtType == csvtArray )
		{
			pSrcObj = ((ECSArray*)pSrcObj)->m_pDefObj ;
			if ( pSrcObj == NULL )
			{
				break ;
			}
		}
		else
		{
			break ;
		}
	}
	return	false ;
}

bool ECSTypeInfo::IsTypeNakedArrayPointerMatch
	( const ECSObject * pDstObj, const ECSObject * pSrcObj )
{
	for ( ; ; )
	{
		if ( IsTypeEqual( pDstObj, pSrcObj ) )
		{
			return	true ;
		}
		if ( pSrcObj->m_vtType == csvtArray )
		{
			pSrcObj = ((ECSArray*)pSrcObj)->m_pDefObj ;
			if ( pSrcObj == NULL )
			{
				break ;
			}
		}
		else if ( (pDstObj->m_vtType == csvtPointer)
				&& (pSrcObj->m_vtType == csvtPointer) )
		{
			ECSPointer *	pPtrDst = (ECSPointer*) pDstObj ;
			ECSPointer *	pPtrSrc = (ECSPointer*) pSrcObj ;
			if ( !(pPtrDst->m_fReadOnly) && pPtrSrc->m_fReadOnly )
			{
				break ;
			}
			pDstObj = pPtrDst->m_pRef ;
			pSrcObj = pPtrSrc->m_pRef ;
			if ( (pDstObj == NULL) || (pSrcObj == NULL) )
			{
				return	(pDstObj == NULL) && (pSrcObj == NULL) ;
			}
		}
		else
		{
			break ;
		}
	}
	return	false ;
}

// 型表現フォーマット
//////////////////////////////////////////////////////////////////////////////
EWideString ECSTypeInfo::GetFormatTypeString( void ) const
{
	EWideString	wstrType ;
	FormatTypeString( wstrType ) ;
	return	wstrType ;
}

void ECSTypeInfo::FormatTypeString( EWideString & wstrTypeFormat ) const
{
	EWideString	wstrTypeDec ;
	const ECSObject *	pType = FormatTypeDecoration( wstrTypeDec ) ;
	if ( (pType != NULL) && (pType->m_vtType == csvtFunction) )
	{
		EWideString	wstrRetType ;
		EWideString	wstrArgList ;
		ECSFunction *	pFunc = (ECSFunction*) pType ;
		pFunc->m_prototype.GetReturnType().FormatTypeString( wstrRetType ) ;
		FormatArgumentTypeList( wstrArgList, pFunc->m_prototype ) ;
		//
		wstrTypeFormat = wstrRetType + L" (" ;
		if ( (pFunc->m_pThisCall != NULL)
			&& (pFunc->m_prototype.GetAttribute() & ECSTypeInfo::flagThisCall) )
		{
			wstrTypeFormat += pFunc->m_pThisCall->GetGlobalName() ;
			wstrTypeFormat += L"::" ;
		}
		wstrTypeFormat += wstrTypeDec + L")(" + wstrArgList + L")" ;
		if ( pFunc->m_prototype.GetAttribute() & ECSTypeInfo::flagConstant )
		{
			wstrTypeFormat += L" const" ;
		}
		if ( pFunc->m_prototype.GetAttribute() & ECSTypeInfo::flagNakedCall )
		{
			wstrTypeFormat += L" naked" ;
		}
		if ( pFunc->m_prototype.GetAttribute() & ECSTypeInfo::flagNativeObject )
		{
			wstrTypeFormat += L" native" ;
		}
		return ;
	}
	else
	{
		FormatTypeString( wstrTypeFormat, m_pValue ) ;
		//
		if ( m_dwFlags & flagConstant )
		{
			if ( (m_pValue != NULL) && (m_pValue->m_vtType == csvtPointer) )
			{
				if ( wstrTypeFormat.GetAt
						( wstrTypeFormat.GetLength() - 1 ) != L'*' )
				{
					wstrTypeFormat += L" " ;
				}
				wstrTypeFormat += L"const" ;
			}
			else
			{
				wstrTypeFormat = L"const " + wstrTypeFormat ;
			}
		}
		if ( m_dwFlags & flagNakedBuffer )
		{
			if ( (m_pValue != NULL) && (m_pValue->m_vtType == csvtPointer) )
			{
				if ( wstrTypeFormat.GetAt
						( wstrTypeFormat.GetLength() - 1 ) != L'*' )
				{
					wstrTypeFormat += L" " ;
				}
				wstrTypeFormat += L"naked" ;
			}
			else
			{
				wstrTypeFormat = L"naked " + wstrTypeFormat ;
			}
		}
	}
}

void ECSTypeInfo::FormatTypeString
		( EWideString & wstrTypeFormat, const ECSObject * pType )
{
	EWideString	wstrTypeName ;
	EWideString	wstrTypeDec ;
	pType = FormatTypeDecoration( wstrTypeDec, pType ) ;
	if ( pType == NULL )
	{
		wstrTypeName = L"void" ;
	}
	else if ( pType->m_vtType == csvtFunction )
	{
		EWideString	wstrRetType ;
		EWideString	wstrArgList ;
		ECSFunction *	pFunc = (ECSFunction*) pType ;
		pFunc->m_prototype.GetReturnType().FormatTypeString( wstrRetType ) ;
		FormatArgumentTypeList( wstrArgList, pFunc->m_prototype ) ;
		//
		wstrTypeFormat = wstrRetType + L" (" ;
		if ( (pFunc->m_pThisCall != NULL)
			&& (pFunc->m_prototype.GetAttribute() & ECSTypeInfo::flagThisCall) )
		{
			wstrTypeFormat += pFunc->m_pThisCall->GetGlobalName() ;
			wstrTypeFormat += L"::" ;
		}
		wstrTypeFormat += wstrTypeDec + L")(" + wstrArgList + L")" ;
		if ( pFunc->m_prototype.GetAttribute() & ECSTypeInfo::flagNakedCall )
		{
			wstrTypeFormat += L" naked" ;
		}
		return ;
	}
	else if ( pType->m_vtType == csvtReference )
	{
		wstrTypeName = L"Reference" ;
	}
	else if ( pType->m_vtType == csvtHash )
	{
		wstrTypeName = L"Hash" ;
		//
		const ECSHash *	pHash = (const ECSHash *) pType ;
		ECSObject *		pElementType = pHash->m_pDefObj ;
		if ( pElementType != NULL )
		{
			EWideString	wstrElementType ;
			FormatTypeString( wstrElementType, pElementType ) ;
			wstrTypeDec += L"<" + wstrElementType + L">" ;
		}
	}
	else
	{
		wstrTypeName = pType->OperateTypeOf() ;
	}
	wstrTypeFormat = wstrTypeName + wstrTypeDec ;
}

const ECSObject * ECSTypeInfo::FormatTypeDecoration( EWideString & wstrTypeFormat ) const
{
	const ECSObject *	pPureType =
		FormatTypeDecoration( wstrTypeFormat, m_pValue ) ;
	//
	if ( m_dwFlags & flagConstant )
	{
		if ( wstrTypeFormat.GetAt
				( wstrTypeFormat.GetLength() - 1 ) != L'*' )
		{
			wstrTypeFormat += L" " ;
		}
		wstrTypeFormat += L"const" ;
	}
	if ( m_dwFlags & flagNakedBuffer )
	{
		if ( wstrTypeFormat.GetAt
				( wstrTypeFormat.GetLength() - 1 ) != L'*' )
		{
			wstrTypeFormat += L" " ;
		}
		wstrTypeFormat += L"naked" ;
	}
	return	pPureType ;
}

const ECSObject * ECSTypeInfo::FormatTypeDecoration
		( EWideString & wstrTypeFormat, const ECSObject * pType )
{
	const ECSObject *	pPureType = NULL ;
	if ( pType == NULL )
	{
		return	NULL ;
	}
	if ( pType->m_vtType == csvtReference )
	{
		if ( ((ECSReference*)pType)->m_pRef == NULL )
		{
			pPureType = pType ;
		}
		else
		{
			pPureType = FormatTypeDecoration
				( wstrTypeFormat, ((ECSReference*)pType)->m_pRef ) ;
			wstrTypeFormat += L"&" ;
		}
	}
	else if ( pType->m_vtType == csvtPointer )
	{
		const ECSPointer *	pPtrType = (const ECSPointer*) pType ;
		pPureType =
			FormatTypeDecoration( wstrTypeFormat, pPtrType->m_pRef ) ;
		if ( pPtrType->m_fReadOnly )
		{
			wstrTypeFormat += L" const" ;
		}
		wstrTypeFormat += L"*" ;
	}
	else if ( pType->m_vtType == csvtArray )
	{
		const ECSArray *	pArray = (const ECSArray *) pType ;
		ECSObject *			pElementType = pArray->GetEndDefaultElement() ;
		if ( pElementType == NULL )
		{
			return	pArray ;
		}
		int				nDim = pArray->GetDimension() ;
		unsigned int *	pBounds = new unsigned int[nDim] ;
		nDim = pArray->GetDimensionSize( pBounds, nDim ) ;
		//
		pPureType =
			FormatTypeDecoration( wstrTypeFormat, pElementType ) ;
		//
		for ( int i = 0; i < nDim; i ++ )
		{
			wstrTypeFormat += L'[' ;
			if ( !(pBounds[i] & 0x80000000) )
			{
				wstrTypeFormat += EWideString( (int) pBounds[i] ) ;
			}
			wstrTypeFormat += L']' ;
		}
		//
		delete []	pBounds ;
	}
	else
	{
		pPureType = pType ;
	}
	return	pPureType ;
}

// プロトタイプの引数型表現フォーマット
//////////////////////////////////////////////////////////////////////////////
void ECSTypeInfo::FormatArgumentTypeList
	( EWideString & wstrArgList, const ECSPrototypeInfo & proto )
{
	for ( unsigned int i = 0; i < proto.GetArgumentCount(); i ++ )
	{
		if ( i != 0 )
		{
			wstrArgList += L"," ;
		}
		ECSTypeInfo *	pArgType = proto.GetArgumentAt( i ) ;
		if ( pArgType != NULL )
		{
			EWideString	wstrType ;
			pArgType->FormatTypeString( wstrType ) ;
			wstrArgList += wstrType ;
		}
	}
}

// クラス参照をカウント
//////////////////////////////////////////////////////////////////////////////
void ECSTypeInfo::EnumerateReferenceClasses
	( ENumArray<DWORD>& lstClassUsed, ECSExecutionImage * pcsxi )
{
	ECSObject *	pType = m_pValue ;
	while ( pType != NULL )
	{
		switch ( pType->m_vtType )
		{
		case	csvtReference:
			pType = ((ECSReference*) pType)->m_pRef ;
			break ;
		case	csvtPointer:
			pType = ((ECSPointer*) pType)->m_pRef ;
			break ;
		case	csvtArray:
			pType = ((ECSArray*) pType)->GetEndDefaultElement() ;
			break ;
		case	csvtHash:
			pType = ((ECSHash*) pType)->m_pDefObj ;
			break ;
		case	csvtFunction:
			{
				ECSFunction *	pFuncType = (ECSFunction*) pType ;
				if ( pFuncType->m_pThisCall != NULL )
				{
					pFuncType->m_pThisCall->
						EnumerateReferenceClasses( lstClassUsed, pcsxi ) ;
				}
				pFuncType->m_prototype.
					EnumerateReferenceClasses( lstClassUsed, pcsxi ) ;
			}
			return ;
		case	csvtObject:
			{
				ECSClassInfo *	pClassInf =
					pcsxi->GetClassInfoAs( pType->GetTypeName() ) ;
				if ( pClassInf != NULL )
				{
					pClassInf->EnumerateReferenceClasses( lstClassUsed, pcsxi ) ;
				}
			}
		default:
			return ;
		}
	}
}

// デバッグ出力
//////////////////////////////////////////////////////////////////////////////
void ECSTypeInfo::DebugTrace( const char * pszHeader ) const
{
	EWideString	wstrTrace = pszHeader ;
	char	szRegName[0x20] ;
	if ( m_flagThisReg )
	{
		wstrTrace += L"this=" ;
		wstrTrace +=
			EWideString( ECSSakura2Processor::GetRegisterName
									( szRegName, 0x20, m_regThis ) ) ;
		wstrTrace += L"," ;
	}
	if ( m_flagLoadReg )
	{
		wstrTrace += L"loaded=" ;
		wstrTrace +=
			EWideString( ECSSakura2Processor::GetRegisterName
									( szRegName, 0x20, m_regLoaded ) ) ;
		wstrTrace += L"," ;
	}
	if ( m_flagAddress )
	{
		wstrTrace += L"addr=[" ;
		wstrTrace +=
			EWideString( ECSSakura2Processor::GetRegisterName
									( szRegName, 0x20, m_regBase ) ) ;
		if ( m_regIndex >= 0 )
		{
			wstrTrace += L"+" ;
			wstrTrace +=
				EWideString( ECSSakura2Processor::GetRegisterName
										( szRegName, 0x20, m_regIndex ) ) ;
			if ( m_scaleIndex > 0 )
			{
				wstrTrace += L"*" ;
				wstrTrace += EWideString( 1 << m_scaleIndex ) ;
			}
			if ( m_addrOffset > 0 )
			{
				wstrTrace += L"+" ;
				wstrTrace += EWideString( m_addrOffset ) ;
			}
			else if ( m_addrOffset < 0 )
			{
				wstrTrace += EWideString( m_addrOffset ) ;
			}
		}
		wstrTrace += L"]," ;
	}
	wstrTrace += GetFormatTypeString() ;
	//
	ESLTrace( EString( wstrTrace ) + "\n" ) ;
}

// ピュアな型か？（配列や参照型でない）
//////////////////////////////////////////////////////////////////////////////
bool ECSTypeInfo::IsPureType( void ) const
{
	if ( m_pValue != NULL )
	{
		if ( m_pValue->m_vtType == csvtReference )
		{
			return	(((ECSReference*)m_pValue)->m_pRef == NULL) ;
		}
		else if ( m_pValue->m_vtType == csvtArray )
		{
			return	(((ECSArray*)m_pValue)->m_pDefObj == NULL) ;
		}
		else if ( m_pValue->m_vtType == csvtHash )
		{
			return	(((ECSHash*)m_pValue)->m_pDefObj == NULL) ;
		}
		else if ( m_pValue->m_vtType == csvtPointer )
		{
			return	false ;
		}
	}
	return	true ;
}

// Boolean 型か？
//////////////////////////////////////////////////////////////////////////////
bool ECSTypeInfo::IsTypeBoolean( const ECSObject * pType )
{
	if ( (pType != NULL)
		&& (pType->m_vtType == csvtInteger) )
	{
		return	(((const ECSInteger *)pType)->GetValueMask()
										== ECSInteger::m_maskBoolean) ;
	}
	return	false ;
}

// 整数値取得
//////////////////////////////////////////////////////////////////////////////
INT64 ECSTypeInfo::GetValueInteger( void ) const
{
	ESLAssert( m_dwFlags & flagDeterministic ) ;
	ESLAssert( m_pValue != NULL ) ;
	if ( m_pValue != NULL )
	{
		ESLAssert( m_pValue->m_vtType == csvtInteger ) ;
		if ( m_pValue->m_vtType == csvtInteger )
		{
			return	((ECSInteger*)m_pValue)->GetValue() ;
		}
	}
	return	0 ;
}

// 実数値取得
//////////////////////////////////////////////////////////////////////////////
double ECSTypeInfo::GetValueReal( void ) const
{
	ESLAssert( m_dwFlags & flagDeterministic ) ;
	ESLAssert( m_pValue != NULL ) ;
	if ( m_pValue != NULL )
	{
		ESLAssert( m_pValue->m_vtType == csvtReal ) ;
		if ( m_pValue->m_vtType == csvtReal )
		{
			return	((ECSReal*)m_pValue)->m_varReal ;
		}
	}
	return	0 ;
}

// 参照型か？（type & 形式か？）
//////////////////////////////////////////////////////////////////////////////
bool ECSTypeInfo::IsTypeReference( const ECSObject * pType )
{
	if ( (pType != NULL)
		&& (pType->m_vtType == csvtReference) )
	{
		return	(((const ECSReference *)pType)->m_pRef != NULL) ;
	}
	return	false ;
}

// 参照型か？（type& 形式の左辺式か？）
//////////////////////////////////////////////////////////////////////////////
bool ECSTypeInfo::IsTypeReference2( const ECSObject * pType )
{
	if ( (pType != NULL)
		&& (pType->m_vtType == csvtReference) )
	{
		return	IsTypeReference( ((const ECSReference *)pType)->m_pRef ) ;
	}
	return	false ;
}

// 多次元配列型か？（type[] 形式か？）
//////////////////////////////////////////////////////////////////////////////
bool ECSTypeInfo::IsTypeArray( const ECSObject * pType )
{
	if ( (pType != NULL)
		&& (pType->m_vtType == csvtArray) )
	{
		return	(((const ECSArray *)pType)->m_pDefObj != NULL) ;
	}
	return	false ;
}

// ハッシュコンテナ型か？（Hash<Type> 形式か？）
//////////////////////////////////////////////////////////////////////////////
bool ECSTypeInfo::IsTypeHashArray( const ECSObject * pType )
{
	if ( (pType != NULL)
		&& (pType->m_vtType == csvtHash) )
	{
		return	(((const ECSHash *)pType)->m_pDefObj != NULL) ;
	}
	return	false ;
}

// 関数ポインタ型か？
//////////////////////////////////////////////////////////////////////////////
ECSFunction * ECSTypeInfo::GetTypeFunctionPointer( void ) const
{
	ECSObject *	pType = GetNakedType() ;
	if ( pType == NULL )
	{
		return	NULL ;
	}
	if ( pType->m_vtType != csvtPointer )
	{
		if ( pType->m_vtType == csvtFunction )
		{
			return	(ECSFunction*) pType ;
		}
		return	NULL ;
	}
	ECSPointer *	pPtrType = (ECSPointer*) pType ;
	if ( pPtrType->m_pRef == NULL )
	{
		return	NULL ;
	}
	if ( pPtrType->m_pRef->m_vtType != csvtFunction )
	{
		return	NULL ;
	}
	return	(ECSFunction*) (pPtrType->m_pRef) ;
}

// 実行時に Integer で表現される特殊な型か？
//////////////////////////////////////////////////////////////////////////////
bool ECSTypeInfo::IsRuntimeIntegerType( void ) const
{
	if ( IsTypePointerNaked() )
	{
		return	true ;
	}
	ECSObject *	pValue = GetNakedType() ;
	if ( (pValue != NULL) && (pValue->m_vtType == csvtObject) )
	{
		const ECSClassInfo *	pClassInf = pValue->m_pClassInf ;
		if ( pClassInf != NULL )
		{
			if ( pClassInf->IsIntegerEnumeratorType() )
			{
				return	true ;
			}
		}
	}
	return	false ;
}

// 参照を除去した型を取得
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSTypeInfo::GetNakedType( void ) const
{
	ECSObject *	pObj = m_pValue ;
	while ( pObj != NULL )
	{
		if ( pObj->m_vtType == csvtReference )
		{
			if ( ((ECSReference*)pObj)->m_pRef != NULL )
			{
				pObj = ((ECSReference*)pObj)->m_pRef ;
			}
			else
			{
				break ;
			}
		}
		else
		{
			break ;
		}
	}
	return	pObj ;
}

ECSObject * ECSTypeInfo::GetSingleNakedType( void ) const
{
	ECSObject *	pObj = m_pValue ;
	if ( pObj != NULL )
	{
		if ( pObj->m_vtType == csvtReference )
		{
			if ( ((ECSReference*)pObj)->m_pRef != NULL )
			{
				pObj = ((ECSReference*)pObj)->m_pRef ;
			}
		}
	}
	return	pObj ;
}

// ポインタを除去した型を取得
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSTypeInfo::GetNakedPointerType( void ) const
{
	ECSObject *	pObj = GetNakedType() ;
	if ( pObj != NULL )
	{
		if ( pObj->m_vtType == csvtPointer )
		{
			return	((ECSPointer*)pObj)->m_pRef ;
		}
	}
	return	pObj ;
}

// 配列型 [] の要素型を取得
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSTypeInfo::GetArrayElementType( void ) const
{
	ECSObject *	pObj = GetNakedType() ;
	if ( pObj != NULL )
	{
		if ( pObj->m_vtType == csvtArray )
		{
			return	((ECSArray*)pObj)->GetEndDefaultElement() ;
		}
		else if ( pObj->m_vtType == csvtHash )
		{
			return	((ECSHash*)pObj)->m_pDefObj ;
		}
	}
	return	NULL ;
}

// ピュアな型を取得
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSTypeInfo::GetPureType( void ) const
{
	ECSObject *	pObj = m_pValue ;
	while ( pObj != NULL )
	{
		while ( pObj->m_vtType == csvtReference )
		{
			if ( ((ECSReference*)pObj)->m_pRef != NULL )
			{
				pObj = ((ECSReference*)pObj)->m_pRef ;
			}
			else
			{
				break ;
			}
		}
		if ( pObj->m_vtType == csvtArray )
		{
			ECSObject *	pElementType =
				((ECSArray*)pObj)->GetEndDefaultElement() ;
			if ( pElementType != NULL )
			{
				pObj = pElementType ;
			}
			else
			{
				break ;
			}
		}
		else if ( pObj->m_vtType == csvtHash )
		{
			ECSObject *	pElementType = ((ECSHash*)pObj)->m_pDefObj ;
			if ( pElementType != NULL )
			{
				pObj = pElementType ;
			}
			else
			{
				break ;
			}
		}
		else
		{
			break ;
		}
	}
	return	pObj ;
}

// クラス情報取得
//////////////////////////////////////////////////////////////////////////////
const ECSClassInfo * ECSTypeInfo::GetClassInfo( void ) const
{
	ECSObject *	pPureType = GetPureType() ;
	if ( pPureType != NULL )
	{
		return	pPureType->m_pClassInf ;
	}
	return	NULL ;
}

// naked メモリ上でのサイズ
//////////////////////////////////////////////////////////////////////////////
int ECSTypeInfo::SizeOfOnNakedMemory( void ) const
{
	int	nSize, nObjCount ;
	GetNakedMemorySize( nSize, nObjCount ) ;
	return	nSize ;
}

ESLError ECSTypeInfo::GetNakedMemorySize( int& nSize, int& nObjCount ) const
{
	/*
	if ( !(m_dwFlags & flagNakedBuffer) )
	{
		nSize = 8 ;
		nObjCount = 1 ;
		return	eslErrSuccess ;
	}
	*/
	return	GetNakedMemorySize( nSize, nObjCount, m_pValue ) ;
}

ESLError ECSTypeInfo::GetNakedMemorySize
			( int& nSize, int& nObjCount, ECSObject * pType )
{
	nSize = 0 ;
	nObjCount = 0 ;
	if ( pType == NULL )
	{
		return	ESLErrorMsg( "void はメモリ上に配置できません" ) ;
	}
	switch ( pType->m_vtType )
	{
	case	csvtInteger:
		nSize = (((ECSInteger*)pType)->SizeOf() + 7) / 8 ;
		break ;
	case	csvtReal:
		nSize = ((ECSReal*)pType)->SizeOf() / 8 ;
		break ;
	case	csvtArray:
		{
			ECSArray *	pArray = (ECSArray*) pType ;
			if ( !pArray->IsBounds() )
			{
				return	ESLErrorMsg( "配列サイズが指定されていません" ) ;
			}
			if ( pArray->m_pDefObj == NULL )
			{
				return	ESLErrorMsg( "配列要素型が指定されていません" ) ;
			}
			int			nElementSize, nElementObjCount ;
			ESLError	err =
				GetNakedMemorySize
					( nElementSize, nElementObjCount, pArray->m_pDefObj ) ;
			if ( err )
			{
				return	err ;
			}
			nSize = nElementSize * pArray->GetBounds() ;
			nObjCount = nElementObjCount * pArray->GetBounds() ;
		}
		break ;
	case	csvtPointer:
	case	csvtReference:
	case	csvtFunction:
		nSize = 8 ;
		break ;
	case	csvtObject:
		{
			const ECSClassInfo *	pClassInf = pType->m_pClassInf ;
			if ( pClassInf == NULL )
			{
				return	ESLErrorMsg
					( "naked メモリ上に配置する構造体情報がありません" ) ;
			}
			if ( !(pClassInf->GetAttribute() & ECSTypeInfo::flagNakedBuffer) )
			{
				if ( pClassInf->IsIntegerEnumeratorType() )
				{
					nSize = 8 ;
					break ;
				}
				nSize = 8 ;
				nObjCount = 1 ;
				return	ESLErrorMsg
					( "naked メモリ上に配置する構造体が naked ではありません" ) ;
			}
			nSize = pClassInf->GetNakedMemorySize() ;
		}
		break ;
	default:
		nSize = 8 ;
		nObjCount ++ ;
		return	ESLErrorMsg( "naked メモリ上に配置できないデータです" ) ;
	}
	return	eslErrSuccess ;
}

// naked メモリ上でのアライメント取得
//////////////////////////////////////////////////////////////////////////////
int ECSTypeInfo::GetNakedAlignment( void ) const
{
	return	GetNakedAlignment( m_pValue ) ;
}

int ECSTypeInfo::GetNakedAlignment( ECSObject * pType )
{
	if ( pType == NULL )
	{
		return	8 ;
	}
	switch ( pType->m_vtType )
	{
	case	csvtInteger:
		return	(((ECSInteger*)pType)->SizeOf() + 7) / 8 ;

	case	csvtReal:
		return	((ECSReal*)pType)->SizeOf() / 8 ;

	case	csvtArray:
		{
			ECSArray *	pArray = (ECSArray*) pType ;
			if ( pArray->m_pDefObj == NULL )
			{
				break ;
			}
			return	GetNakedAlignment( pArray->m_pDefObj ) ;
		}

	case	csvtPointer:
	case	csvtReference:
	case	csvtFunction:
		return	8 ;

	case	csvtObject:
		{
			const ECSClassInfo *	pClassInf = pType->m_pClassInf ;
			if ( pClassInf == NULL )
			{
				break ;
			}
			return	pClassInf->GetNakedAlignment() ;
		}

	default:
		break ;
	}
	return	8 ;
}

// naked メモリ上の配列サイズを計算
//////////////////////////////////////////////////////////////////////////////
int ECSTypeInfo::CalcNakedMemoryArraySize( void ) const
{
	return	CalcNakedMemoryArraySize( m_pValue ) ;
}

int ECSTypeInfo::CalcNakedMemoryArraySize( ECSObject * pObj )
{
	if ( pObj == NULL )
	{
		return	0 ;
	}
	if ( pObj->m_vtType == csvtArray )
	{
		ECSArray *	pArray = (ECSArray*) pObj ;
		if ( !pArray->IsBounds() )
		{
			return	0 ;
		}
		if ( pArray->m_pDefObj == NULL )
		{
			return	0 ;
		}
		return	pArray->GetBounds()
					* CalcNakedMemoryArraySize( pArray->m_pDefObj ) ;
	}
	return	1 ;
}

// naked メモリ上のアクセス型
//////////////////////////////////////////////////////////////////////////////
CSVariableType ECSTypeInfo::GetNakedMemoryType( void ) const
{
	return	GetNakedMemoryType( m_pValue ) ;
}

CSVariableType ECSTypeInfo::GetNakedMemoryType( ECSObject * pObj )
{
	if ( pObj == NULL )
	{
		return	csvtObject ;
	}
	switch ( pObj->m_vtType )
	{
	case	csvtInteger:
		return	((ECSInteger*)pObj)->GetIntegerType() ;

	case	csvtReal:
		return	((ECSReal*)pObj)->m_vtRealType ;

	case	csvtPointer:
	case	csvtReference:
	case	csvtFunction:
		return	csvtInteger ;

	case	csvtObject:
		if ( pObj->m_pClassInf != NULL )
		{
			if ( pObj->m_pClassInf->IsIntegerEnumeratorType() )
			{
				return	csvtInteger ;
			}
			if ( pObj->m_pClassInf->IsRealEnumeratorType() )
			{
				return	csvtReal ;
			}
		}
		break ;

	default:
		break ;
	}
	return	csvtObject ;
}

// naked メモリ上のオブジェクトか判定
//////////////////////////////////////////////////////////////////////////////
bool ECSTypeInfo::IsNakedMemoryObject( bool fNakedMemoryMode ) const
{
	return	IsNakedMemoryObject
		( m_pValue, (fNakedMemoryMode || (m_dwFlags & flagNakedBuffer)) ) ;
}

bool ECSTypeInfo::IsNakedMemoryObject
	( const ECSObject * pType, bool fNakedMemoryMode )
{
	if ( pType == NULL )
	{
		return	false ;
	}
	switch ( pType->m_vtType )
	{
	case	csvtInteger:
		return	true ;

	case	csvtReal:
		return	true ;

	case	csvtPointer:
	case	csvtReference:
		return	true ;

	case	csvtArray:
		return	fNakedMemoryMode ;

	case	csvtFunction:
		return	true ;

	default:
		if ( pType->m_pClassInf != NULL )
		{
			const ECSClassInfo *	pClassInf = pType->m_pClassInf ;
			if ( pClassInf->GetAttribute() & ECSTypeInfo::flagNakedBuffer )
			{
				return	true ;
			}
			if ( pClassInf->IsIntegerEnumeratorType()
					|| pClassInf->IsRealEnumeratorType() )
			{
				return	true ;
			}
		}
		break ;
	}
	return	false ;
}

// naked モードレジスタで表現可能な型か判定
//////////////////////////////////////////////////////////////////////////////
bool ECSTypeInfo::IsNakedPrimitiveDataType( void ) const
{
	return	IsNakedPrimitiveDataType( m_pValue ) ;
}

bool ECSTypeInfo::IsNakedPrimitiveDataType( const ECSObject * pObj )
{
	if ( pObj != NULL )
	{
		switch ( pObj->m_vtType )
		{
		case	csvtInteger:
		case	csvtReal:
		case	csvtPointer:
		case	csvtReference:
		case	csvtFunction:
			return	true ;
		default:
			if ( pObj->m_pClassInf != NULL )
			{
				const ECSClassInfo *	pClassInf = pObj->m_pClassInf ;
				if ( pClassInf->IsIntegerEnumeratorType()
						|| pClassInf->IsRealEnumeratorType() )
				{
					return	true ;
				}
			}
		}
	}
	return	false ;
}

// naked クラスかその配列型なら、そのクラス情報を取得
//////////////////////////////////////////////////////////////////////////////
const ECSClassInfo * ECSTypeInfo::GetNakedMemoryClassInfo( void ) const
{
	ECSObject *	pObj = m_pValue ;
	if ( (pObj != NULL)
		&& (pObj->m_vtType == csvtArray) )
	{
		pObj = ((ECSArray*)pObj)->GetEndDefaultElement() ;
	}
	if ( !IsNakedPrimitiveDataType( pObj ) )
	{
		const ECSClassInfo *	pClassInf = pObj->m_pClassInf ;
		if ( pClassInf != NULL )
		{
			if ( pClassInf->IsNakedMemoryClass() )
			{
				return	pClassInf ;
			}
		}
	}
	return	NULL ;
}

// naked メモリ上で object かその配列型なら、そのクラス情報を取得
//////////////////////////////////////////////////////////////////////////////
const ECSClassInfo * ECSTypeInfo::GetObjectClassInfo
						( const ECSCompiler& compiler ) const
{
	ECSObject *	pObj = m_pValue ;
	if ( (pObj != NULL)
		&& (pObj->m_vtType == csvtArray) )
	{
		pObj = ((ECSArray*)pObj)->GetEndDefaultElement() ;
	}
	return	GetObjectClassInfo( pObj, compiler ) ;
}

const ECSClassInfo * ECSTypeInfo::GetObjectClassInfo
			( const ECSObject * pObj, const ECSCompiler& compiler )
{
	if ( pObj == NULL )
	{
		return	NULL ;
	}
	if ( !IsNakedPrimitiveDataType( pObj ) )
	{
		const ECSClassInfo *	pClassInf = pObj->m_pClassInf ;
		if ( pClassInf != NULL )
		{
			if ( !pClassInf->IsNakedMemoryClass() )
			{
				return	pClassInf ;
			}
		}
		else
		{
			switch ( pObj->m_vtType )
			{
			case	csvtArray:
				return	compiler.GetClassInfoAs( L"Array" ) ;
			case	csvtHash:
				return	compiler.GetClassInfoAs( L"Hash" ) ;
			case	csvtString:
				return	compiler.GetClassInfoAs( L"String" ) ;
			default:
				break ;
			}
		}
	}
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// Cotopha Script 関数プロトタイプ情報
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSPrototypeInfo, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSPrototypeInfo::ECSPrototypeInfo( void )
{
	m_dwFlags = 0 ;
}

ECSPrototypeInfo::ECSPrototypeInfo( const ECSPrototypeInfo & proto )
{
	operator = ( proto ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSPrototypeInfo::~ECSPrototypeInfo( void )
{
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const ECSPrototypeInfo &
	ECSPrototypeInfo::operator = ( const ECSPrototypeInfo & proto )
{
	m_dwFlags = proto.m_dwFlags ;
	m_typeReturn = proto.m_typeReturn ;
	m_wstrFuncName = proto.m_wstrFuncName ;
	m_wstrGlobalName = proto.m_wstrGlobalName ;
	m_lstArgument = proto.m_lstArgument ;
	m_lstArgName = proto.m_lstArgName ;
	m_lstThrows = proto.m_lstThrows ;
	//
	int	i, nCount ;
	nCount = proto.m_lstArgDefault.GetSize() ;
	m_lstArgDefault.SetSize( nCount ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSObject *	pObj = proto.GetArgumentDefaultAt( i ) ;
		if ( pObj != NULL )
		{
			m_lstArgDefault.SetAt( i, pObj->Duplicate() ) ;
		}
		else
		{
			m_lstArgDefault.SetAt( i, NULL ) ;
		}
	}
	return	*this ;
}

// 比較
//////////////////////////////////////////////////////////////////////////////
bool ECSPrototypeInfo::IsPrototypeEqual( const ECSPrototypeInfo & proto ) const
{
	if ( m_dwFlags != proto.m_dwFlags )
	{
		return	false ;
	}
	if ( m_typeReturn != proto.m_typeReturn )
	{
		return	false ;
	}
	return	IsArgumentEqual( proto.m_lstArgument ) ;
}

bool ECSPrototypeInfo::IsArgumentEqual( const EPtrObjArray<ECSTypeInfo> & arg ) const
{
	unsigned int	i, nCount ;
	nCount = m_lstArgument.GetSize() ;
	if ( arg.GetSize() != nCount )
	{
		return	false ;
	}
	for ( i = 0; i < nCount; i ++ )
	{
		ESLAssert( m_lstArgument.GetAt( i ) != NULL ) ;
		ESLAssert( arg.GetAt( i ) != NULL ) ;
//		if ( m_lstArgument[i] != arg[i] )
		if ( !ECSTypeInfo::IsTypeEqual
			( m_lstArgument[i].m_pValue, arg[i].m_pValue ) )
		{
			return	false ;
		}
	}
	return	true ;
}

// 引数の適合判定
//////////////////////////////////////////////////////////////////////////////
ECSTypeInfo::TypeMatchResult
	ECSPrototypeInfo::IsMatchArgument
		( const EPtrObjArray<ECSTypeInfo> & lstArg,
			ECSTypeInfo::TypeMatchResult matchLimit ) const
{
	unsigned int	i, nCount ;
	nCount = m_lstArgument.GetSize() ;
	if ( (nCount < lstArg.GetSize())
		&& !(m_dwFlags & ECSTypeInfo::flagVarArgument) )
	{
		return	ECSTypeInfo::typeNoMatch ;
	}
	ECSTypeInfo::TypeMatchResult	matchResult = ECSTypeInfo::typeMatch ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSTypeInfo *	pArg = m_lstArgument.GetAt( i ) ;
		ESLAssert( pArg != NULL ) ;
		bool	fDefaultArg = true ;
		if ( i < lstArg.GetSize() )
		{
			ECSTypeInfo *	pParam = lstArg.GetAt( i ) ;
			if ( pParam != NULL )
			{
				ECSTypeInfo::TypeMatchResult
					typeMatch = pArg->IsMatchType( *pParam, matchLimit ) ;
				if ( typeMatch == ECSTypeInfo::typeNoMatch )
				{
					return	ECSTypeInfo::typeNoMatch ;
				}
				else if ( typeMatch != ECSTypeInfo::typeMatch )
				{
					if ( matchResult < typeMatch )
					{
						matchResult = typeMatch ;
					}
				}
				fDefaultArg = false ;
			}
		}
		if ( fDefaultArg )
		{
			ECSObject *	pDefValue = m_lstArgDefault.GetAt( i ) ;
			if ( pDefValue == NULL )
			{
				if ( !(pArg->m_dwFlags & ECSTypeInfo::flagDeterministic) )
				{
					return	ECSTypeInfo::typeNoMatch ;
				}
			}
			else
			{
				ECSTypeInfo::TypeMatchResult
					typeMatch = pArg->IsMatchType
						( ECSTypeInfo( pDefValue->Duplicate(),
										ECSTypeInfo::flagDeterministic ) ) ;
				if ( typeMatch == ECSTypeInfo::typeNoMatch )
				{
					return	ECSTypeInfo::typeNoMatch ;
				}
				else if ( typeMatch > ECSTypeInfo::typeNatualMatch )
				{
					if ( matchResult < typeMatch )
					{
						matchResult = typeMatch ;
					}
				}
			}
		}
	}
	return	matchResult ;
}

// 属性設定
//////////////////////////////////////////////////////////////////////////////
void ECSPrototypeInfo::SetAttribute( DWORD dwFlags )
{
	m_dwFlags = dwFlags ;
}

// 返り値型設定
//////////////////////////////////////////////////////////////////////////////
void ECSPrototypeInfo::SetReturnType( const ECSTypeInfo & typeinf )
{
	m_typeReturn = typeinf ;
}

// 関数名設定
//////////////////////////////////////////////////////////////////////////////
void ECSPrototypeInfo::SetName( const wchar_t * pwszName )
{
	m_wstrFuncName = pwszName ;
}

// 関数名（ファイルスコープ用）設定
//////////////////////////////////////////////////////////////////////////////
void ECSPrototypeInfo::SetGlobalName( const wchar_t * pwszName )
{
	m_wstrGlobalName = pwszName ;
}

// 名前空間名を取得
//////////////////////////////////////////////////////////////////////////////
EWideString ECSPrototypeInfo::GetNameSpace( void ) const
{
	unsigned int	nFuncLen = m_wstrFuncName.GetLength() ;
	if ( m_wstrGlobalName.Right( nFuncLen ) == m_wstrFuncName )
	{
		unsigned int	nGlobalLen = m_wstrGlobalName.GetLength() ;
		if ( m_wstrGlobalName.Middle
				( nGlobalLen - nFuncLen - 2, 2 ) == L"::" )
		{
			return	m_wstrGlobalName.Left( nGlobalLen - nFuncLen - 2 ) ;
		}
	}
	ECSCompiler::SYMBOL_NAMESPACE	snsSymbol ;
	snsSymbol.ParseSymbol( m_wstrGlobalName ) ;
	return	snsSymbol.wstrNamespace ;
	/*
	int	iLast = 0 ;
	for ( ; ; )
	{
		int	iNext = m_wstrGlobalName.Find
						( L"::", ((iLast == 0) ? 0 : iLast + 2) ) ;
		if ( iNext < 0 )
		{
			break ;
		}
		iLast = iNext ;
	}
	return	m_wstrGlobalName.Left( iLast ) ;
	*/
}

// 引数追加
//////////////////////////////////////////////////////////////////////////////
int ECSPrototypeInfo::AddArgument
		( ECSTypeInfo * pArg, const wchar_t * pwszName )
{
	int	nIndex = m_lstArgument.Add( pArg ) ;
	m_lstArgName.Add( new EWideString( pwszName ) ) ;
	return	nIndex ;
}

// 引数設定
//////////////////////////////////////////////////////////////////////////////
void ECSPrototypeInfo::SetArgumentAt
			( unsigned int nIndex, ECSTypeInfo * pArg )
{
	m_lstArgument.SetAt( nIndex, pArg ) ;
}

// 引数名設定
//////////////////////////////////////////////////////////////////////////////
void ECSPrototypeInfo::SetArgumentNameAt
		( unsigned int nIndex, const wchar_t * pwszName )
{
	m_lstArgName.SetAt( nIndex, new EWideString( pwszName ) ) ;
}

// 引数デフォルト値設定
//////////////////////////////////////////////////////////////////////////////
void ECSPrototypeInfo::SetArgumentDefaultAt
			( unsigned int nIndex, ECSObject * pDefValue )
{
	m_lstArgDefault.SetAt( nIndex, pDefValue ) ;
}

// 引数の数設定
//////////////////////////////////////////////////////////////////////////////
void ECSPrototypeInfo::SetArgumentCount( unsigned int nCount )
{
	m_lstArgument.SetSize( nCount ) ;
	m_lstArgName.SetSize( nCount ) ;
	m_lstArgDefault.SetSize( nCount ) ;
}

// naked 関数の属性修飾
//////////////////////////////////////////////////////////////////////////////
void ECSPrototypeInfo::MakeNakedAttribute( void )
{
	if ( m_typeReturn.IsTypePointer() | m_typeReturn.IsTypeReference() )
	{
		bool	fCastOperator = false ;
		if ( m_wstrFuncName.CompareLeft( L"operator " ) == 0 )
		{
			EWideString	wstrCastType ;
			m_typeReturn.FormatTypeString( wstrCastType ) ;
			if ( m_wstrFuncName == L"operator " + wstrCastType )
			{
				fCastOperator = true ;
			}
		}
		m_typeReturn.m_dwFlags |= ECSTypeInfo::flagNakedBuffer ;
		if ( fCastOperator )
		{
			EWideString	wstrNameSpace = GetNameSpace() ;
			//
			EWideString	wstrCastType ;
			m_typeReturn.FormatTypeString( wstrCastType ) ;
			m_wstrFuncName = L"operator " + wstrCastType ;
			//
			if ( !wstrNameSpace.IsEmpty() )
			{
				wstrNameSpace += L"::" ;
			}
			m_wstrGlobalName = wstrNameSpace + m_wstrFuncName ;
		}
	}
	if ( (GetNameSpace() == L"")
		&& !(m_dwFlags & ECSTypeInfo::flagNativeObject) )
	{
		m_wstrGlobalName = EWideString( L"naked " + m_wstrGlobalName ) ;
	}
	for ( int i = 0; i < (int) m_lstArgument.GetSize(); i ++ )
	{
		ECSTypeInfo *	pArgType = m_lstArgument.GetAt( i ) ;
		if ( !pArgType->IsNakedPrimitiveDataType() )
		{
			ECSTypeInfo *	pRefArgType = new ECSTypeInfo ;
			pRefArgType->MakeReferenceOf( *pArgType ) ;
			m_lstArgument.SetAt( i, pRefArgType ) ;
		}
		else if ( pArgType->IsTypeReference()
					|| pArgType->IsTypePointer() )
		{
			pArgType->m_dwFlags |= ECSTypeInfo::flagNakedBuffer ;
		}
	}
}

// クラス参照をカウント
//////////////////////////////////////////////////////////////////////////////
void ECSPrototypeInfo::EnumerateReferenceClasses
	( ENumArray<DWORD>& lstClassUsed, ECSExecutionImage * pcsxi )
{
	m_typeReturn.EnumerateReferenceClasses( lstClassUsed, pcsxi ) ;
	//
	unsigned int	nArgCount = m_lstArgument.GetSize() ;
	for ( unsigned int i = 0; i < nArgCount; i ++ )
	{
		ECSTypeInfo *	pTypeArg = m_lstArgument.GetAt( i ) ;
		if ( pTypeArg != NULL )
		{
			pTypeArg->EnumerateReferenceClasses( lstClassUsed, pcsxi ) ;
		}
	}
}

// プロトタイプ書式
//////////////////////////////////////////////////////////////////////////////
EWideString ECSPrototypeInfo::FormatPrototype( void ) const
{
	EWideString	wstrPrototype ;
	if ( m_dwFlags & ECSTypeInfo::flagVirtual )
	{
		wstrPrototype += L"virtual " ;
	}
	if ( m_dwFlags & ECSTypeInfo::flagNativeObject )
	{
		wstrPrototype += L"native " ;
	}
	//
	EWideString	wstrReturn ;
	m_typeReturn.FormatTypeString( wstrReturn ) ;
	wstrPrototype += wstrReturn ;
	wstrPrototype += L" " ;
	wstrPrototype += m_wstrGlobalName ;
	//
	EWideString	wstrArgList ;
	ECSTypeInfo::FormatArgumentTypeList( wstrArgList, *this ) ;
	wstrPrototype += L"(" ;
	wstrPrototype += wstrArgList ;
	wstrPrototype += L")" ;
	//
	if ( m_dwFlags & ECSTypeInfo::flagNakedCall )
	{
		wstrPrototype += L" naked" ;
	}
	if ( m_dwFlags & ECSTypeInfo::flagConstant )
	{
		wstrPrototype += L" const" ;
	}
	return	wstrPrototype ;
}


//////////////////////////////////////////////////////////////////////////////
// Cotopha Script クラス情報
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSClassInfo, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSClassInfo::ECSClassInfo( void )
{
	m_phase = phaseEmpty ;
	m_dwFlags = 0 ;
	m_nNakedSize = 0 ;
	m_nUnnakedObjects = 0 ;
	m_flagVirtualOffset = false ;
}

ECSClassInfo::ECSClassInfo( const ECSClassInfo & clsinf )
{
	operator = ( clsinf ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSClassInfo::~ECSClassInfo( void )
{
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const ECSClassInfo & ECSClassInfo::operator = ( const ECSClassInfo & clsinf )
{
	m_dwFlags = clsinf.m_dwFlags ;
	m_wstrClassName = clsinf.m_wstrClassName ;
	m_wstrGlobalName = clsinf.m_wstrGlobalName ;
	m_lstParent = clsinf.m_lstParent ;
	m_wstaCastInf = clsinf.m_wstaCastInf ;
	m_lstVariable = clsinf.m_lstVariable ;
	m_staVariable = clsinf.m_staVariable ;
	m_lstVarNakedOffset = clsinf.m_lstVarNakedOffset ;
	m_lstFuncProto = clsinf.m_lstFuncProto ;
	m_staFuncName = clsinf.m_staFuncName ;
	//
	m_nNakedSize = clsinf.m_nNakedSize ;
	m_nUnnakedObjects = clsinf.m_nUnnakedObjects ;
	m_flagVirtualOffset = clsinf.m_flagVirtualOffset ;
	//
	unsigned int	i, nCount ;
	nCount = m_lstFuncProto.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		MemberFunction *	pFunc = m_lstFuncProto.GetAt( i ) ;
		if ( pFunc != NULL )
		{
			pFunc->m_pClassCast =
				m_wstaCastInf.GetAs( pFunc->m_wstrClass ) ;
		}
	}
	return	*this ;
}

// 比較
//////////////////////////////////////////////////////////////////////////////
bool ECSClassInfo::IsClassEqual( const ECSClassInfo & clsinf ) const
{
	if ( m_wstrGlobalName != clsinf.m_wstrGlobalName )
	{
		return	false ;
	}
	if ( m_lstParent != clsinf.m_lstParent )
	{
		return	false ;
	}
	if ( m_lstVariable != clsinf.m_lstVariable )
	{
		return	false ;
	}
	if ( m_staVariable != clsinf.m_staVariable )
	{
		return	false ;
	}
	if ( m_lstFuncProto != clsinf.m_lstFuncProto )
	{
		return	false ;
	}
	if ( m_staFuncName != clsinf.m_staFuncName )
	{
		return	false ;
	}
	return	true ;
}

// 基本型判定
//////////////////////////////////////////////////////////////////////////////
CSVariableType ECSClassInfo::IsBasicType( void ) const
{
	static const wchar_t *	pwszTypeName[] =
	{
		L"Integer", L"Real", L"String",
		L"Reference", L"Array", L"Hash",
		L"Pointer",
		NULL
	} ;
	static const CSVariableType	vtTypeIndex[] =
	{
		csvtInteger, csvtReal, csvtString,
		csvtReference, csvtArray, csvtHash,
		csvtPointer,
	} ;
	for ( int i = 0; pwszTypeName[i]; i ++ )
	{
		if ( m_wstrGlobalName.CompareNoCase( pwszTypeName[i] ) == 0 )
		{
			return	vtTypeIndex[i] ;
		}
	}
	return	csvtObject ;
}

// フラグ設定
//////////////////////////////////////////////////////////////////////////////
void ECSClassInfo::SetAttribute( DWORD dwFlags )
{
	m_dwFlags = dwFlags ;
}

// クラス名設定
//////////////////////////////////////////////////////////////////////////////
void ECSClassInfo::SetName( const wchar_t * pwszName )
{
	m_wstrClassName = pwszName ;
}

// グローバルクラス名設定
//////////////////////////////////////////////////////////////////////////////
void ECSClassInfo::SetGlobalName( const wchar_t * pwszName )
{
	m_wstrGlobalName = pwszName ;
}

// 親クラス追加
//////////////////////////////////////////////////////////////////////////////
void ECSClassInfo::AddParentClassInfo( ParentClass * pParentClass )
{
	m_lstParent.Add( pParentClass ) ;
}

// 親クラスキャスト情報構築
//////////////////////////////////////////////////////////////////////////////
void ECSClassInfo::BuildAllClassCast( void )
{
	m_wstaCastInf.RemoveAll() ;
	m_lstVariable.RemoveAll() ;
	m_staVariable.RemoveAll() ;
	m_lstVarNakedOffset.RemoveAll() ;
	m_lstFuncProto.RemoveAll() ;
	m_staFuncName.RemoveAll() ;
	m_nNakedSize = 0 ;
	m_nUnnakedObjects = 0 ;
	m_flagVirtualOffset = false ;
	//
	CastInfo *	pciCast =
		new CastInfo( ECS_CAST_INTERFACE( NULL ), 0, this ) ;
	m_wstaCastInf.Add( GetGlobalName(), pciCast ) ;
	//
	unsigned int	i, j, nCount ;
	int				nVarCount = 0 ;
	int				nFuncCount = 0 ;
	nCount = m_lstParent.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ParentClass *	pParentClass = m_lstParent.GetAt( i ) ;
		if ( (pParentClass != NULL)
			&& (pParentClass->pClassInf != NULL) )
		{
			ECSClassInfo *	pClassInf = pParentClass->pClassInf ;
			ECS_CAST_INTERFACE	ciParent
				( NULL, nVarCount,
						pClassInf->GetVariableCount(), nFuncCount ) ;
			BuildClassCast
				( pParentClass->pClassInf,
					((GetAttribute() & ECSTypeInfo::flagNativeObject) != 0),
					(i == 0), true, ciParent, pParentClass->dwFlags ) ;
			//
			if ( pClassInf->GetAttribute() & ECSTypeInfo::flagNativeObject )
			{
				if ( !(GetAttribute() & ECSTypeInfo::flagNativeObject) )
				{
					nVarCount ++ ;
					m_nNakedSize += 8 ;
					m_nUnnakedObjects ++ ;
				}
			}
			else
			{
				nVarCount += pClassInf->GetVariableCount() ;
			}
			//
			for ( j = 0; j < pClassInf->GetFunctionCount(); j ++ )
			{
				MemberFunction *	pFunc = pClassInf->GetFunctionAt( j ) ;
				ESLAssert( pFunc != NULL ) ;
				DWORD	dwFuncProtected = pFunc->GetProtectedAttribute() ;
				if ( dwFuncProtected == ECSTypeInfo::flagPrivate )
				{
					dwFuncProtected = ECSTypeInfo::flagPrivate2 ;
				}
				MemberFunction *	pAddFunc = new MemberFunction ;
				*pAddFunc = *pFunc ;
				pAddFunc->SetAttribute
					( (pAddFunc->GetAttribute()
							& ~ECSTypeInfo::flagProtectedMask)
						| __max( dwFuncProtected, pParentClass->dwFlags ) ) ;
				pAddFunc->m_wstrClass = pFunc->m_wstrClass ;
				pAddFunc->m_pClassCast =
						GetCastClassInfoAs( pAddFunc->m_wstrClass ) ;
				pAddFunc->m_fpFuncPointer = pFunc->m_fpFuncPointer ;
				pAddFunc->m_fpFuncPointer.m_castThis = ciParent ;
				//
				AddFunction( pAddFunc ) ;
			}
			nFuncCount += pClassInf->GetFunctionCount() ;
		}
	}
}

void ECSClassInfo::BuildClassCast
	( ECSClassInfo * pParentClass,
		bool fNativeParent, bool fRootParent, bool fVirtualOffset,
		const ECS_CAST_INTERFACE & ciParent, DWORD dwScopeFlags )
{
	const bool	fVirtualFunc = (pParentClass->GetVirtualFunctionCount() > 0) ;
	if ( fVirtualFunc )
	{
		m_nNakedSize = (m_nNakedSize + 0x07) & ~0x07 ;
	}
	//
	// キャスト情報登録
	//
	if ( m_wstaCastInf.GetAs( pParentClass->GetGlobalName() ) == NULL )
	{
		CastInfo *	pciCast ;
		if ( pParentClass->GetAttribute() & ECSTypeInfo::flagNativeObject )
		{
			ECS_CAST_INTERFACE	ci( NULL ) ;
			ci.iNativeParent = ciParent.iVarOffset ;
			pciCast = new CastInfo( ci, dwScopeFlags, pParentClass ) ;
			pciCast->nNakedOffset = m_nNakedSize ;
			m_wstaCastInf.Add( pParentClass->GetGlobalName(), pciCast ) ;
		}
		else
		{
			pciCast = new CastInfo( ciParent, dwScopeFlags, pParentClass ) ;
			pciCast->nNakedOffset = m_nNakedSize ;
			m_wstaCastInf.Add( pParentClass->GetGlobalName(), pciCast ) ;
		}
	}
	//
	// 親クラス処理
	//
	unsigned int	i, nCount ;
	int				nVarCount = 0 ;
	int				nFuncCount = 0 ;
	nCount = pParentClass->GetParentClassCount() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ParentClass *	pClass = pParentClass->GetParentClassAt( i ) ;
		if ( (pClass != NULL)
			&& (pClass->pClassInf != NULL) )
		{
			ECSClassInfo *	pClassInf = pClass->pClassInf ;
			if ( (i == 0) && fVirtualFunc && fVirtualOffset )
			{
				if ( (pClassInf->GetVirtualFunctionCount() == 0) )
				{
					m_nNakedSize += 8 ;
					if ( fRootParent )
					{
						m_flagVirtualOffset = true ;
					}
				}
				fVirtualOffset = false ;
			}
			ECS_CAST_INTERFACE	ciParent
				( NULL, ciParent.iVarOffset + nVarCount,
						pClassInf->GetVariableCount(),
						ciParent.iFuncOffset + nFuncCount ) ;
			BuildClassCast
				( pClass->pClassInf,
					((pParentClass->GetAttribute()
						& ECSTypeInfo::flagNativeObject) != 0),
					(fRootParent && (i == 0)), true,
					ciParent, __max( pClass->dwFlags, dwScopeFlags ) ) ;
			//
			if ( pClassInf->GetAttribute() & ECSTypeInfo::flagNativeObject )
			{
				if ( !fNativeParent
					&& !(GetAttribute() & ECSTypeInfo::flagNativeObject) )
				{
					nVarCount ++ ;
				}
			}
			else
			{
				nVarCount += pClassInf->GetVariableCount() ;
			}
			nFuncCount += pClassInf->GetFunctionCount() ;
		}
	}
	//
	// メンバ変数登録
	//
	if ( fVirtualFunc && fVirtualOffset )
	{
		m_nNakedSize += 8 ;
		if ( fRootParent )
		{
			m_flagVirtualOffset = true ;
		}
	}
	if ( pParentClass->GetAttribute() & ECSTypeInfo::flagNativeObject )
	{
		if ( !fNativeParent
			&& !(GetAttribute() & ECSTypeInfo::flagNativeObject) )
		{
			AddVariable
				( pParentClass->GetGlobalName(),
					new ECSTypeInfo
						( new ECSStructure( pParentClass ), dwScopeFlags ) ) ;
		}
	}
	else
	{
		for ( i = nVarCount; i < pParentClass->GetVariableCount(); i ++ )
		{
			ECSTypeInfo *	pVarType = pParentClass->GetVariableAt( i ) ;
			ESLAssert( pVarType != NULL ) ;
			ECSTypeInfo *	pNewParentVar = new ECSTypeInfo( *pVarType ) ;
			DWORD	dwVarProtected = pNewParentVar->GetProtectedAttribute() ;
			if ( dwVarProtected == ECSTypeInfo::flagPrivate )
			{
				dwVarProtected = ECSTypeInfo::flagPrivate2 ;
			}
			pNewParentVar->m_dwFlags &= ~ECSTypeInfo::flagProtectedMask ;
			pNewParentVar->m_dwFlags |= __max( dwVarProtected, dwScopeFlags ) ;
			AddVariable
				( pParentClass->GetVariableNameAt(i), pNewParentVar ) ;
		}
	}
}

// 親クラスキャスト追加
//////////////////////////////////////////////////////////////////////////////
void ECSClassInfo::AddCastClassInfo
	( const wchar_t * pwszName, ECSClassInfo::CastInfo * pCastClass )
{
	m_wstaCastInf.Add( pwszName, pCastClass ) ;
}

// 親クラスキャスト情報取得
//////////////////////////////////////////////////////////////////////////////
ECSClassInfo::CastInfo *
	ECSClassInfo::GetCastClassInfoAs( const wchar_t * pwszName ) const
{
	return	m_wstaCastInf.GetAs( pwszName ) ;
}

// 親クラスメンバ変数総数取得
//////////////////////////////////////////////////////////////////////////////
unsigned int ECSClassInfo::GetParentVariableCount( void ) const
{
	unsigned int	i, nCount ;
	unsigned int	nVarCount = 0 ;
	nCount = m_lstParent.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ParentClass *	pParentClass = m_lstParent.GetAt( i ) ;
		if ( (pParentClass != NULL)
			&& (pParentClass->pClassInf != NULL) )
		{
			if ( pParentClass->pClassInf->GetAttribute()
								& ECSTypeInfo::flagNativeObject )
			{
				nVarCount ++ ;
			}
			else
			{
				nVarCount += pParentClass->pClassInf->GetVariableCount() ;
			}
		}
	}
	return	nVarCount ;
}

// 保存用クラス情報以外を削除する
//////////////////////////////////////////////////////////////////////////////
void ECSClassInfo::CleanupClassInfo( void )
{
	if ( IsNamespace() )
	{
		m_lstVariable.RemoveAll() ;
		m_staVariable.RemoveAll() ;
		m_lstVarNakedOffset.RemoveAll() ;
		m_nNakedSize = 0 ;
		m_nUnnakedObjects = 0 ;
		//
		m_lstFuncProto.RemoveAll() ;
		m_staFuncName.RemoveAll() ;
		m_lstDestructors.RemoveAll() ;
	}
}

// メンバ変数追加
//////////////////////////////////////////////////////////////////////////////
int ECSClassInfo::AddVariable
		( const wchar_t * pwszName, ECSTypeInfo * pVarType )
{
	ESLAssert( m_lstVariable.GetSize() == m_staVariable.GetSize() ) ;
	int	iVar = m_lstVariable.GetSize() ;
	m_lstVariable.Add( pVarType ) ;
	m_staVariable.Add( pwszName ) ;
	if ( !IsUnion() )
	{
		if ( !(GetAttribute() & ECSTypeInfo::flagStructure) )
		{
			int	nAlign = pVarType->GetNakedAlignment() ;
			m_nNakedSize += nAlign - 1 ;
			m_nNakedSize -= m_nNakedSize % nAlign ;
		}
		m_lstVarNakedOffset.Add( m_nNakedSize ) ;
	}
	else
	{
		m_lstVarNakedOffset.Add( 0 ) ;
	}
	if ( !(GetAttribute() & ECSTypeInfo::flagNativeObject) )
	{
		int	nSize, nObjCount ;
		if ( !pVarType->GetNakedMemorySize( nSize, nObjCount ) )
		{
			pVarType->m_dwFlags |=
					(GetAttribute() & ECSTypeInfo::flagNakedBuffer) ;
			if ( !IsUnion() )
			{
				m_nNakedSize += nSize ;
			}
			else
			{
				if ( m_nNakedSize < nSize )
				{
					m_nNakedSize = nSize ;
				}
			}
		}
		else
		{
			pVarType->m_dwFlags &= ~ECSTypeInfo::flagNakedBuffer ;
			if ( !IsUnion() )
			{
				m_nNakedSize += 8 ;
			}
			else
			{
				if ( m_nNakedSize < 8 )
				{
					m_nNakedSize = 8 ;
				}
			}
			m_nUnnakedObjects ++ ;
		}
	}
	//
	return	iVar ;
}

// メンバ関数追加
//////////////////////////////////////////////////////////////////////////////
int ECSClassInfo::AddFunction( MemberFunction * pPrototype )
{
	ESLAssert( m_lstFuncProto.GetSize() == m_staFuncName.GetSize() ) ;
	//
	int	iFunc = m_lstFuncProto.GetSize() ;
	m_lstFuncProto.Add( pPrototype ) ;
	m_staFuncName.Add( pPrototype->GetName() ) ;
	//
	return	iFunc ;
}

// 関数オーバーライド追加
//////////////////////////////////////////////////////////////////////////////
ESLError ECSClassInfo::AddOverrideFunction
				( const ECSPrototypeInfo & prototype )
{
	CastInfo *		pciThis = GetCastClassInfoAs( GetGlobalName() ) ;
	if ( pciThis == NULL )
	{
		return	ESLErrorMsg
			( "内部エラー：this クラスのキャスト情報が見つかりません。" ) ;
	}
	unsigned int	i, nCount ;
	unsigned int	nParentFuncCount = GetParentFunctionCount() ;
	bool			fOverrided = false ;
	const bool		fNativeClass =
						(pciThis->pClassInf != NULL)
							&& (pciThis->pClassInf->GetAttribute()
									& ECSTypeInfo::flagNativeObject) ;
	nCount = GetFunctionCount() ;
	//
	// naked デストラクタ関数名補正
	//
	EWideString	wstrMatchingFuncName = prototype.GetName() ;
	if ( IsNakedMemoryClass() )
	{
		if ( wstrMatchingFuncName == L"~" + m_wstrClassName )
		{
			wstrMatchingFuncName = L"<destructor>" ;
		}
	}
	//
	// ファイルリンク用関数名決定
	//
	int				nFuncIndex = 0 ;
	EWideString		wstrExternFuncName = prototype.GetGlobalName() ;
	for ( i = 0; i < nCount; i ++ )
	{
		MemberFunction *	pFunc = GetFunctionAt( i ) ;
		if ( (pFunc != NULL)
			&& (pFunc->m_wstrClass == GetGlobalName())
			&& (pFunc->GetName() == prototype.GetName())
			&& (!fNativeClass
				|| (pFunc->IsNakedCall() == prototype.IsNakedCall())) )
		{
			nFuncIndex ++ ;
		}
	}
	if ( nFuncIndex > 0 )
	{
		wstrExternFuncName += L"@" ;
		wstrExternFuncName += EWideString( nFuncIndex ) ;
	}
	//
	// 関数オーバーライド
	//
	for ( i = 0; i < nCount; i ++ )
	{
		MemberFunction *	pFunc = GetFunctionAt( i ) ;
		if ( (pFunc != NULL)
			&& (pFunc->GetName() == wstrMatchingFuncName)
			&& ((pFunc->GetAttribute() & ECSTypeInfo::flagConstant)
				== (prototype.GetAttribute() & ECSTypeInfo::flagConstant))
			&& (!fNativeClass
				|| (pFunc->IsNakedCall() == prototype.IsNakedCall()))
			&& pFunc->IsArgumentEqual( prototype.GetArgument() ) )
		{
			const DWORD	dwAttr = pFunc->GetAttribute() ;
			if ( i >= nParentFuncCount )
			{
				return	ESLErrorMsg
					( "既に定義された関数を再定義しようとしています。" ) ;
			}
			else if ( !(dwAttr & ECSTypeInfo::flagVirtual) )
			{
				continue ;
			}
			if ( pFunc->GetReturnType() != prototype.GetReturnType() )
			{
				return	ESLErrorMsg
					( "オーバーライド関数の返り値が一致していません。" ) ;
			}
			if ( prototype.GetAttribute() & ECSTypeInfo::flagStatic )
			{
				return	ESLErrorMsg
					( "static 関数で仮想関数をオーバーライドしようとしています" ) ;
			}
			pFunc = new MemberFunction ;
			*pFunc = prototype ;
			pFunc->SetAttribute
				( (pFunc->GetAttribute() | ECSTypeInfo::flagVirtual)
					& ~(ECSTypeInfo::flagNakedCallGate
								| ECSTypeInfo::flagObjectCallGate) ) ;
			if ( dwAttr & ECSTypeInfo::flagNakedCall )
			{
				if ( !(prototype.GetAttribute() & ECSTypeInfo::flagNakedCall) )
				{
					if ( IsNakedMemoryClass() )
					{
						pFunc->SetAttribute
							( pFunc->GetAttribute()
								| ECSTypeInfo::flagNakedCall
								| ECSTypeInfo::flagObjectCallGate ) ;
					}
					else
					{
						return	ESLErrorMsg
							( "object クラスで object モード関数を"
								" naked 関数でオーバーライドしようとしています" ) ;
					}
				}
			}
			else
			{
				if ( prototype.GetAttribute() & ECSTypeInfo::flagNakedCall )
				{
					if ( IsNakedMemoryClass() )
					{
						pFunc->SetAttribute
							( pFunc->GetAttribute()
								& ~ECSTypeInfo::flagNakedCall
								| ECSTypeInfo::flagNakedCallGate ) ;
					}
					else
					{
						return	ESLErrorMsg
							( "object クラスで naked モード関数を"
								" object 関数でオーバーライドしようとしています" ) ;
					}
				}
			}
			pFunc->SetName( wstrMatchingFuncName ) ;
			pFunc->SetGlobalName( wstrExternFuncName ) ;
			pFunc->m_wstrClass = GetGlobalName() ;
			pFunc->m_pClassCast = pciThis ;
			pFunc->m_fpFuncPointer.m_ftType =
							ECS_FUNCTION_POINTER::funcScriptCall ;
			pFunc->m_fpFuncPointer.m_castThis = *pciThis ;
			pFunc->m_fpFuncPointer.m_varFunc.addrScript = 0 ;
			//
			m_lstFuncProto.SetAt( i, pFunc ) ;
			//
			fOverrided = true ;
		}
	}
	//
	// 非オーバーライド関数
	//
	if ( !fOverrided )
	{
		MemberFunction *	pFunc = new MemberFunction ;
		*pFunc = prototype ;
		pFunc->SetName( wstrMatchingFuncName ) ;
		pFunc->SetGlobalName( wstrExternFuncName ) ;
		pFunc->m_wstrClass = GetGlobalName() ;
		pFunc->m_pClassCast = pciThis ;
		pFunc->m_fpFuncPointer.m_ftType =
						ECS_FUNCTION_POINTER::funcScriptCall ;
		pFunc->m_fpFuncPointer.m_castThis = *pciThis ;
		pFunc->m_fpFuncPointer.m_varFunc.addrScript = 0 ;
		//
		AddFunction( pFunc ) ;
	}
	return	eslErrSuccess ;
}

// オーバーライド関数指標検索
//////////////////////////////////////////////////////////////////////////////
int ECSClassInfo::FindOverrideFunction
	( const ECSPrototypeInfo & prototype, int iFirst, int iEndBounds )
{
	if ( iEndBounds < 0 )
	{
		iEndBounds = GetFunctionCount() ;
	}
	for ( int i = iFirst; i < iEndBounds; i ++ )
	{
		MemberFunction *	pFunc = GetFunctionAt( i ) ;
		if ( pFunc == NULL )
		{
			continue ;
		}
		if ( pFunc->GetName() != prototype.GetName() )
		{
			continue ;
		}
		if ( !pFunc->IsArgumentEqual( prototype.GetArgument() ) )
		{
			continue ;
		}
		const DWORD	dwFlagMask =
				ECSTypeInfo::flagConstant | ECSTypeInfo::flagStatic ;
		if ( (pFunc->GetAttribute() & dwFlagMask)
					== (prototype.GetAttribute() & dwFlagMask) )
		{
			return	i ;
		}
	}
	return	-1 ;
}

// 親クラスメンバ関数総数取得
//////////////////////////////////////////////////////////////////////////////
unsigned int ECSClassInfo::GetParentFunctionCount( void ) const
{
	unsigned int	i, nCount ;
	unsigned int	nFuncCount = 0 ;
	nCount = m_lstParent.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ParentClass *	pParentClass = m_lstParent.GetAt( i ) ;
		if ( (pParentClass != NULL)
			&& (pParentClass->pClassInf != NULL) )
		{
			nFuncCount += pParentClass->pClassInf->GetFunctionCount() ;
		}
	}
	return	nFuncCount ;
}

// 仮想関数の総数取得
//////////////////////////////////////////////////////////////////////////////
unsigned int ECSClassInfo::GetVirtualFunctionCount( int nFuncCount ) const
{
	int	i, nCount = m_lstFuncProto.GetSize() ;
	int	nVirtFuncCount = 0 ;
	if ( nFuncCount >= 0 )
	{
		nCount = nFuncCount ;
	}
	for ( i = 0; i < nCount; i ++ )
	{
		MemberFunction *	pFunc = m_lstFuncProto.GetAt( i ) ;
		if ( (pFunc != NULL)
			&& (pFunc->GetAttribute() & ECSTypeInfo::flagVirtual) )
		{
			nVirtFuncCount ++ ;
		}
	}
	return	nVirtFuncCount ;
}

// 仮想関数インデックスを取得
//////////////////////////////////////////////////////////////////////////////
int ECSClassInfo::GetVirtualFunctionIndex( const MemberFunction * pFunc ) const
{
	int	nFuncIndex = FindFunctionIndex( pFunc ) ;
	if ( nFuncIndex >= 0 )
	{
		return	GetVirtualFunctionCount( nFuncIndex ) ;
	}
	return	-1 ;
}

// このクラスのメンバ関数取得
//////////////////////////////////////////////////////////////////////////////
ECSClassInfo::MemberFunction *
	ECSClassInfo::GetThisFunctionAs( const ECSPrototypeInfo & proto ) const
{
	//
	// naked デストラクタ関数名補正
	//
	EWideString	wstrMatchingFuncName = proto.GetName() ;
	if ( IsNakedMemoryClass() )
	{
		if ( wstrMatchingFuncName == L"~" + m_wstrClassName )
		{
			wstrMatchingFuncName = L"<destructor>" ;
		}
	}
	//
	// 関数検索
	//
	unsigned int	i, nCount ;
	nCount = m_lstFuncProto.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		MemberFunction *	pMember = m_lstFuncProto.GetAt( i ) ;
		ESLAssert( pMember != NULL ) ;
		const DWORD	dwMaskAttr =
				ECSTypeInfo::flagConstant | ECSTypeInfo::flagNakedCall ;
		if ( (pMember->m_wstrClass == GetGlobalName())
			&& (pMember->GetName() == wstrMatchingFuncName)
			&& ((pMember->GetAttribute() & dwMaskAttr)
					== (proto.GetAttribute() & dwMaskAttr))
			&& pMember->IsArgumentEqual( proto.GetArgument() ) )
		{
			return	pMember ;
		}
	}
	return	NULL ;
}

// 消滅関数をリストする
//////////////////////////////////////////////////////////////////////////////
void ECSClassInfo::UpdateDestructorList( void )
{
	m_lstDestructors.RemoveAll() ;
	AddDestructorList( this ) ;
}

// 指定クラスの消滅関数を追加する
//////////////////////////////////////////////////////////////////////////////
void ECSClassInfo::AddDestructorList( ECSClassInfo * pClassInf )
{
	bool		fDestructor = false ;
	bool		fNakedClass = false ;
	EWideString	wstrName ;
	if ( pClassInf->IsNakedMemoryClass() )
	{
		wstrName = L"<destructor>" ;
		fNakedClass = true ;
	}
	else
	{
		wstrName = L"~" + pClassInf->GetName() ;
	}
	int	i, nCount = m_lstFuncProto.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		MemberFunction *	pFunc = m_lstFuncProto.GetAt( i ) ;
		if ( pFunc == NULL )
		{
			continue ;
		}
		if ( pFunc->GetName() != wstrName )
		{
			continue ;
		}
		if ( (pFunc->m_pClassCast != NULL)
			&& (pFunc->m_pClassCast->pClassInf == pClassInf)
			&& (pFunc->m_fpFuncPointer.m_ftType
					== ECS_FUNCTION_POINTER::funcScriptCall) )
		{
			m_lstDestructors.Add( pFunc ) ;
			fDestructor = true ;
			//
			if ( fNakedClass )
			{
				break ;
			}
		}
	}
//	if ( !fDestructor )
	if ( !fNakedClass )
	{
		nCount = pClassInf->GetParentClassCount() ;
		for ( i = 0; i < nCount; i ++ )
		{
			ParentClass *	pParentClass = pClassInf->GetParentClassAt( i ) ;
			if ( pParentClass == NULL )
			{
				continue ;
			}
			if ( pParentClass->pClassInf != NULL )
			{
				AddDestructorList( pParentClass->pClassInf ) ;
			}
		}
	}
}

// 親クラスキャスト判定
//////////////////////////////////////////////////////////////////////////////
ECSClassInfo::CastInfo *
	ECSClassInfo::GetCastParentClassAs( const wchar_t * pwszClassName ) const
{
	return	m_wstaCastInf.GetAs( pwszClassName ) ;
}

// 関数プロトタイプ検索
//////////////////////////////////////////////////////////////////////////////
bool ECSClassInfo::SearchFunctinoAs
	( ECSClassInfo::ListMemberFunction & lstFunc,
		const wchar_t * pwszFuncName,
		const EPtrObjArray<ECSTypeInfo> & lstArg,
		DWORD dwFuncFlags, bool fNoParentClass,
		bool fStrictMatch, bool fPrimaryNakedCall ) const
{
	DWORD	dwMatchFlags = 0 ;
	if ( fNoParentClass )
	{
		dwMatchFlags |= matchNoParentClass ;
	}
	if ( fStrictMatch )
	{
		dwMatchFlags |= matchMatchArgument ;
	}
	if ( fPrimaryNakedCall )
	{
		dwFuncFlags |= ECSTypeInfo::flagNakedCall ;
	}
	else
	{
		dwFuncFlags &= ~ECSTypeInfo::flagNakedCall ;
	}
	int	i ;
	for ( i = 0; i < 2; i ++ )
	{
		/*
		//
		// 完全一致引数検索
		//
		if ( GetMatchEachParentClassFunctinoAs
			( lstFunc, pwszFuncName, lstArg, dwFuncFlags,
				(dwMatchFlags | matchEqualArgument),
								fNoParentClass, this ) > 0 )
		{
			break ;
		}
		//
		// 適合引数関数検索
		//
		if ( GetMatchEachParentClassFunctinoAs
			( lstFunc, pwszFuncName, lstArg, dwFuncFlags,
				(dwMatchFlags | matchMatchArgument),
								fNoParentClass, this ) > 0 )
		{
			break ;
		}
		//
		// ルーズ適合関数を検索する
		//
		if ( !fStrictMatch )
		{
			if ( GetMatchEachParentClassFunctinoAs
				( lstFunc, pwszFuncName, lstArg, dwFuncFlags,
					dwMatchFlags, fNoParentClass, this ) > 0 )
			{
				break ;
			}
		}
		*/
		if ( GetMatchEachParentClassFunctinoAs
			( lstFunc, pwszFuncName, lstArg, dwFuncFlags,
				dwMatchFlags, fNoParentClass, this ) > 0 )
		{
			break ;
		}
		dwFuncFlags ^= ECSTypeInfo::flagNakedCall ;
	}
	if ( i >= 2 )
	{
		return	false ;
	}
	return	(lstFunc.GetSize() == 1)
				|| (GetAttribute() & ECSTypeInfo::flagNativeObject) ;
}

// 親クラスのプロトタイプ検索（SearchFunctinoAs のサブ関数）
//////////////////////////////////////////////////////////////////////////////
int ECSClassInfo::GetMatchEachParentClassFunctinoAs
	( ListMemberFunction & lstFunc,
		const wchar_t * pwszFuncName,
		const EPtrObjArray<ECSTypeInfo> & lstArg,
		DWORD dwFuncFlags, DWORD dwMatchFlags,
		bool fNoParentClass, const ECSClassInfo * pParentInf ) const
{
	int	nCount =
		GetMatchFunctinoAs
			( lstFunc, pwszFuncName, lstArg,
				dwFuncFlags, (dwMatchFlags | matchNoParentClass),
				pParentInf->GetGlobalName() ) ;
	if ( nCount > 0 )
	{
		return	nCount ;
	}
	if ( fNoParentClass )
	{
		return	0 ;
	}
	unsigned int	nParentCount = pParentInf->GetParentClassCount() ;
	for ( unsigned int iParent = 0; iParent < nParentCount; iParent ++ )
	{
		ParentClass *	pParent = pParentInf->GetParentClassAt( iParent ) ;
		if ( (pParent == NULL)
			|| (pParent->pClassInf == NULL) )
		{
			continue ;
		}
		nCount =
			GetMatchEachParentClassFunctinoAs
				( lstFunc, pwszFuncName, lstArg,
					dwFuncFlags, dwMatchFlags,
					fNoParentClass, pParent->pClassInf ) ;
		if ( nCount > 0 )
		{
			return	nCount ;
		}
	}
	return	GetMatchFunctinoAs
				( lstFunc, pwszFuncName, lstArg,
					dwFuncFlags, dwMatchFlags, pParentInf->GetGlobalName() ) ;
}

// 関数検索（SearchFunctinoAs のサブ関数）
//////////////////////////////////////////////////////////////////////////////
int ECSClassInfo::GetMatchFunctinoAs
	( ListMemberFunction & lstFunc,
		const wchar_t * pwszFuncName,
		const EPtrObjArray<ECSTypeInfo> & lstArg,
		DWORD dwFuncFlags, DWORD dwMatchFlags,
		const wchar_t * pwszMatchClassName ) const
{
	if ( pwszMatchClassName == NULL )
	{
		pwszMatchClassName = GetGlobalName() ;
	}
	unsigned int	i, nCount ;
	int	nMatchLevel = ECSTypeInfo::typeLooseMatch ;
	if ( dwMatchFlags & matchEqualArgument )
	{
		nMatchLevel = ECSTypeInfo::typeNatualMatch ;
	}
	else if ( dwMatchFlags & matchMatchArgument )
	{
		nMatchLevel = ECSTypeInfo::typeCastableMatch ;
	}
	nCount = m_lstFuncProto.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		MemberFunction *	pProto = m_lstFuncProto.GetAt( i ) ;
		if ( pProto == NULL )
		{
			continue ;
		}
		if ( pProto->GetName() != pwszFuncName )
		{
			continue ;
		}
		if ( (dwMatchFlags & matchNoParentClass)
			&& (pProto->m_wstrClass != pwszMatchClassName) )
		{
			continue ;
		}
		if ( (dwFuncFlags & ECSTypeInfo::flagConstant)
			&& !(pProto->GetAttribute() & ECSTypeInfo::flagConstant) )
		{
			if ( !(pProto->GetAttribute() & ECSTypeInfo::flagStatic) )
			{
				continue ;
			}
		}
		if ( (dwFuncFlags & ECSTypeInfo::flagStatic)
			&& !(pProto->GetAttribute() & ECSTypeInfo::flagStatic) )
		{
			continue ;
		}
		if ( (dwFuncFlags & ECSTypeInfo::flagNakedCall)
			!= (pProto->GetAttribute() & ECSTypeInfo::flagNakedCall) )
		{
			continue ;
		}
		if ( dwMatchFlags & matchEqualArgument )
		{
			if ( (dwFuncFlags & ECSTypeInfo::flagConstant)
				!= (pProto->GetAttribute() & ECSTypeInfo::flagConstant) )
			{
				continue ;
			}
		}
		ECSTypeInfo::TypeMatchResult
			match = pProto->IsMatchArgument( lstArg ) ;
		if ( match <= nMatchLevel )
		{
			int	nProtoMatch = match ;
			if ( (dwFuncFlags & ECSTypeInfo::flagConstant)
				 != (pProto->GetAttribute() & ECSTypeInfo::flagConstant) )
			{
				if ( nProtoMatch < ECSTypeInfo::typeCastableMatch )
				{
					nProtoMatch ++ ;
				}
			}
			if ( nProtoMatch <= nMatchLevel )
			{
				if ( nProtoMatch < nMatchLevel )
				{
					lstFunc.RemoveAll() ;
					nMatchLevel = nProtoMatch ;
				}
				if ( FindEqualFunction( lstFunc, pProto ) < 0 )
				{
					lstFunc.Add( pProto ) ;
				}
			}
		}
	}
	return	lstFunc.GetSize() ;
}

// 単項演算子オペレーター検索
//////////////////////////////////////////////////////////////////////////////
bool ECSClassInfo::SearchUnaryOperatorAs
	( ListMemberFunction & lstFunc,
		CSUnaryOperatorType uoptUnary,
		const EPtrObjArray<ECSTypeInfo> & lstArg, bool fPrimaryNakedCall ) const
{
	static const wchar_t *	pwszOperator[csuotMax] =
	{
		L"+", L"-", L"not", L"lnot", L"++", L"--", L"++", L"--",
	} ;
	EWideString	wstrFuncName = L"operator " ;
	wstrFuncName += pwszOperator[uoptUnary] ;
	//
	return	SearchFunctinoAs
		( lstFunc, wstrFuncName, lstArg, 0, false, false, fPrimaryNakedCall ) ;
}

// 特殊単項演算子オペレーター検索
//////////////////////////////////////////////////////////////////////////////
bool ECSClassInfo::SearchExUnaryOperatorAs
	( ListMemberFunction & lstFunc,
		CSExtraUniOperatorType xuoptExUnary,
		const EPtrObjArray<ECSTypeInfo> & lstArg, bool fPrimaryNakedCall ) const
{
	static const wchar_t *	pwszOperator[csxuotMax] =
	{
		NULL, L"boolean", L"sizeof", NULL, NULL, NULL, NULL, NULL, NULL, L"&", L"*",
	} ;
	if ( pwszOperator[xuoptExUnary] == NULL )
	{
		return	false ;
	}
	EWideString	wstrFuncName = L"operator " ;
	wstrFuncName += pwszOperator[xuoptExUnary] ;
	//
	return	SearchFunctinoAs
		( lstFunc, wstrFuncName, lstArg, 0, false, false, fPrimaryNakedCall ) ;
}

// 二項演算子オペレーター検索
//////////////////////////////////////////////////////////////////////////////
bool ECSClassInfo::SearchOperatorAs
	( ListMemberFunction & lstFunc,
		CSOperatorType optOperator,
		const EPtrObjArray<ECSTypeInfo> & lstArg, bool fPrimaryNakedCall ) const
{
	static const wchar_t *	pwszOperator[csotMax] =
	{
		L"+", L"-", L"*", L"/", L"mod",
		L"and", L"or", L"xor", 
		L"land", L"lor", L"shift_right", L"shift_left",
	} ;
	EWideString	wstrFuncName = L"operator " ;
	wstrFuncName += pwszOperator[optOperator] ;
	//
	return	SearchFunctinoAs
		( lstFunc, wstrFuncName, lstArg, 0, false, false, fPrimaryNakedCall ) ;
}

// 代入演算子オペレーター検索
//////////////////////////////////////////////////////////////////////////////
bool ECSClassInfo::SearchMoveOperatorAs
	( ListMemberFunction & lstFunc,
		CSOperatorType optOperator,
		const EPtrObjArray<ECSTypeInfo> & lstArg,
		bool fStrictMatch, bool fPrimaryNakedCall ) const
{
	static const wchar_t *	pwszOperator[csotMax] =
	{
		L"+=", L"-=", L"*=", L"/=", L"%=",
		L"&=", L"|=", L"^=", 
		L"&&=", L"||=", L">>=", L"<<=",
	} ;
	EWideString	wstrFuncName = L"operator " ;
	if ( optOperator != csotNop )
	{
		wstrFuncName += pwszOperator[optOperator] ;
	}
	else
	{
		wstrFuncName += L":=" ;
	}
	return	SearchFunctinoAs
		( lstFunc, wstrFuncName, lstArg,
			0, false, fStrictMatch, fPrimaryNakedCall ) ;
}

// 比較演算子オペレーター検索
//////////////////////////////////////////////////////////////////////////////
bool ECSClassInfo::SearchComparatorAs
	( ListMemberFunction & lstFunc,
		CSCompareType cptCompare,
		const EPtrObjArray<ECSTypeInfo> & lstArg, bool fPrimaryNakedCall ) const
{
	static const wchar_t *	pwszOperator[csctMax] =
	{
		L"!=", L"==", L"<", L"<=", L">", L">=", L"!==", L"===",
	} ;
	EWideString	wstrFuncName = L"operator " ;
	wstrFuncName += pwszOperator[cptCompare] ;
	//
	return	SearchFunctinoAs
		( lstFunc, wstrFuncName, lstArg, 0, false, false, fPrimaryNakedCall ) ;
}

// 同名の関数を検索
//////////////////////////////////////////////////////////////////////////////
int ECSClassInfo::FindEqualFunction
	( const ECSClassInfo::ListMemberFunction & lstFunc,
						ECSClassInfo::MemberFunction * pFunc )
{
	for ( int i = 0; i < (int) lstFunc.GetSize(); i ++ )
	{
		MemberFunction *	p = lstFunc.GetAt( i ) ;
		if ( (p != NULL) && (p->GetGlobalName() == pFunc->GetGlobalName()) )
		{
			return	i ;
		}
	}
	return	-1 ;
}

// 列挙型の列挙子値判定
//////////////////////////////////////////////////////////////////////////////
bool ECSClassInfo::IsMatchEnumeratorValue( const ECSObject * pValue ) const
{
	if ( !(m_dwFlags & ECSTypeInfo::flagEnumerator) )
	{
		return	false ;
	}
	CSVariableType	csvtSrcType = pValue->m_vtType ;
	const int	nCount = GetVariableCount() ;
	for ( int i = 0; i < nCount; i ++ )
	{
		ECSTypeInfo *	pEnumInfo = GetVariableAt( i ) ;
		if ( (pEnumInfo == NULL) || (pEnumInfo->m_pValue == NULL) )
		{
			continue ;
		}
		ECSObject *	pEnumValue = pEnumInfo->m_pValue ;
		if ( pEnumValue->m_vtType != csvtSrcType )
		{
			continue ;
		}
		switch ( csvtSrcType )
		{
		case	csvtInteger:
			if ( ((ECSInteger*)pEnumValue)->GetValue()
					== ((ECSInteger*)pValue)->GetValue() )
			{
				return	true ;
			}
			break ;
		case	csvtReal:
			if ( ((ECSReal*)pEnumValue)->m_varReal
					== ((ECSReal*)pValue)->m_varReal )
			{
				return	true ;
			}
			break ;
		case	csvtString:
			if ( ((ECSString*)pEnumValue)->m_varStr
					== ((ECSString*)pValue)->m_varStr )
			{
				return	true ;
			}
			break ;
		}
	}
	return	false ;
}

// 特定型へのキャスト可能か？
//////////////////////////////////////////////////////////////////////////////
bool ECSClassInfo::CanCastTypeTo( const ECSTypeInfo& typeinf ) const
{
	const ECSObject *	pEntity = ECSObject::GetEntity( typeinf.m_pValue ) ;
	if ( !ECSTypeInfo::IsTypeArray( pEntity )
		&& !ECSTypeInfo::IsTypeHashArray( pEntity ) )
	{
		if ( m_wstaCastInf.GetAs( pEntity->GetTypeName() ) != NULL )
		{
			return	true ;
		}
		if ( m_wstrGlobalName == pEntity->GetTypeName() )
		{
			return	true ;
		}
	}
	EWideString	wstrTypeName ;
	typeinf.FormatTypeString( wstrTypeName ) ;
	EWideString	wstrFuncName = L"operator " + wstrTypeName ;
	//
	EPtrObjArray<ECSTypeInfo>	lstArg ;
	ListMemberFunction			lstFunc ;
	if ( SearchFunctinoAs( lstFunc, wstrFuncName, lstArg, 0 ) )
	{
		return	true ;
	}
	if ( !typeinf.IsTypeReference() )
	{
		wstrFuncName += L"&" ;
		return	SearchFunctinoAs( lstFunc, wstrFuncName, lstArg, 0 ) ;
	}
	return	false ;
}

// 特定型からの代入可能か？（operator の判定）
//////////////////////////////////////////////////////////////////////////////
bool ECSClassInfo::CanMoveTypeFrom( const ECSObject * pType ) const
{
	const wchar_t *	pwszFuncName = L"operator :=" ;
	EObjArray<ECSTypeInfo>	lstArg ;
	lstArg.Add( new ECSTypeInfo( ((ECSObject*)pType)->Duplicate() ) ) ;
	//
	unsigned int	i, nCount ;
	nCount = m_lstFuncProto.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSPrototypeInfo *	pProto = m_lstFuncProto.GetAt( i ) ;
		if ( pProto == NULL )
		{
			continue ;
		}
		if ( pProto->GetName() != pwszFuncName )
		{
			continue ;
		}
		if ( pProto->IsMatchArgument
			( lstArg, ECSTypeInfo::typeNatualMatch ) == ECSTypeInfo::typeMatch )
		{
			return	true ;
		}
	}
	return	false ;
}

// メンバ関数に純粋仮想関数があるか調べる
//////////////////////////////////////////////////////////////////////////////
bool ECSClassInfo::TestAbstractFunction( void ) const
{
	for ( unsigned int i = 0; i < GetFunctionCount(); i ++ )
	{
		ECSClassInfo::MemberFunction *	pFunc = GetFunctionAt( i ) ;
		if ( (pFunc != NULL)
			&& (pFunc->GetAttribute() & ECSTypeInfo::flagAbstract))
		{
			return	true ;
		}
	}
	return	false ;
}

// メンバ変数にクラス自体が含まれていないか（無限入れ子）チェック
//////////////////////////////////////////////////////////////////////////////
bool ECSClassInfo::TestMemberVariable( const ECSClassInfo & clsinf ) const
{
	unsigned int	i, nCount ;
	nCount = GetVariableCount() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSTypeInfo *	pVarType = GetVariableAt( i ) ;
		if ( (pVarType == NULL)
			|| (pVarType->m_pValue == NULL) )
		{
			continue ;
		}
		if ( pVarType->IsTypeArray() || pVarType->IsTypeHashArray() )
		{
			ECSObject *	pElement = pVarType->GetArrayElementType() ;
			if ( pElement != NULL )
			{
				const ECSClassInfo *
						pElementClass = pElement->m_pClassInf ;
				if ( pElementClass != NULL )
				{
					if ( pElementClass->GetCastParentClassAs
								( clsinf.GetGlobalName() ) != NULL )
					{
						return	true ;
					}
					if ( pElementClass->TestMemberVariable( clsinf ) )
					{
						return	true ;
					}
				}
			}
		}
		const ECSClassInfo *
				pVarClass = pVarType->m_pValue->m_pClassInf ;
		if ( pVarClass == NULL )
		{
			continue ;
		}
		if ( pVarClass->GetCastParentClassAs
					( clsinf.GetGlobalName() ) != NULL )
		{
			return	true ;
		}
		if ( pVarClass->TestMemberVariable( clsinf ) )
		{
			return	true ;
		}
	}
	return	false ;
}

// クラス詳細情報を削除（メンバ変数とメンバ関数の情報を削除）
//////////////////////////////////////////////////////////////////////////////
void ECSClassInfo::CleanupClassMember( void )
{
	m_lstVariable.RemoveAll() ;
	m_staVariable.RemoveAll() ;
	//
	m_lstFuncProto.RemoveAll() ;
	m_staFuncName.RemoveAll() ;
	m_lstDestructors.RemoveAll() ;
}

// クラス参照をカウント
//////////////////////////////////////////////////////////////////////////////
void ECSClassInfo::EnumerateReferenceClasses
	( ENumArray<DWORD>& lstClassUsed, ECSExecutionImage * pcsxi )
{
	int	idClass = pcsxi->GetClassIdentity( m_wstrGlobalName ) ;
	if ( idClass >= 0 )
	{
		if ( lstClassUsed.GetAt(idClass) > 0 )
		{
			return ;
		}
		lstClassUsed.SetAt( idClass, lstClassUsed.GetAt(idClass) + 1 ) ;
	}
	//
	// 親クラスをカウント
	//
	unsigned int	nParentClass = m_lstParent.GetSize() ;
	unsigned int	i ;
	for ( i = 0; i < nParentClass; i ++ )
	{
		ParentClass *	pParentClass = m_lstParent.GetAt( i ) ;
		if ( (pParentClass != NULL) && (pParentClass->pClassInf != NULL) )
		{
			pParentClass->pClassInf->
					EnumerateReferenceClasses( lstClassUsed, pcsxi ) ;
		}
	}
	//
	// メンバ変数をカウント
	//
	unsigned int	nMemberVar = m_lstVariable.GetSize() ;
	for ( i = 0; i < nMemberVar; i ++ )
	{
		ECSTypeInfo *	pVarType = m_lstVariable.GetAt( i ) ;
		if ( pVarType != NULL )
		{
			pVarType->EnumerateReferenceClasses( lstClassUsed, pcsxi ) ;
		}
	}
	//
	// メンバ関数をカウント
	//
	unsigned int	nMemberFunc = m_lstFuncProto.GetSize() ;
	for ( i = 0; i < nMemberFunc; i ++ )
	{
		MemberFunction *	pFunc = m_lstFuncProto.GetAt( i ) ;
		if ( pFunc != NULL )
		{
			pFunc->EnumerateReferenceClasses( lstClassUsed, pcsxi ) ;
		}
	}
}

// 全てのメンバ関数をデバッグ出力
//////////////////////////////////////////////////////////////////////////////
void ECSClassInfo::DebugTraceFunctions( void ) const
{
	ESLTrace( "%s member functions;\n", EString(m_wstrGlobalName).CharPtr() ) ;
	unsigned int	nCount = GetFunctionCount() ;
	for ( unsigned int i = 0; i < nCount; i ++ )
	{
		MemberFunction *	pFunc = GetFunctionAt( i ) ;
		if ( pFunc == NULL )
		{
			continue ;
		}
		ESLTrace( "%s\n", EString(pFunc->FormatPrototype()).CharPtr() ) ;
	}
}

// 列挙型の場合、列挙子型を取得
//////////////////////////////////////////////////////////////////////////////
ECSClassInfo * ECSClassInfo::GetEnumeratorObjectType( void ) const
{
	if ( GetAttribute() & ECSTypeInfo::flagEnumerator )
	{
		ParentClass *	pParentType = GetParentClassAt( 0 ) ;
		if ( (pParentType != NULL) && (pParentType->pClassInf != NULL) )
		{
			return	pParentType->pClassInf ;
		}
	}
	return	NULL ;
}

// 整数の列挙型か？
//////////////////////////////////////////////////////////////////////////////
bool ECSClassInfo::IsIntegerEnumeratorType( void ) const
{
	ECSClassInfo *	pClassInf = GetEnumeratorObjectType() ;
	if ( pClassInf != NULL )
	{
		return	(pClassInf->GetGlobalName() == L"Integer") ;
	}
	return	false ;
}

// 実数の列挙型か？
//////////////////////////////////////////////////////////////////////////////
bool ECSClassInfo::IsRealEnumeratorType( void ) const
{
	ECSClassInfo *	pClassInf = GetEnumeratorObjectType() ;
	if ( pClassInf != NULL )
	{
		return	(pClassInf->GetGlobalName() == L"Real") ;
	}
	return	false ;
}

// フレンドクラス追加
//////////////////////////////////////////////////////////////////////////////
void ECSClassInfo::AddFriendClass( const wchar_t * pwszGlobalName )
{
	if ( !IsFriendClass( pwszGlobalName ) )
	{
		m_lstFriendClass.Add( new EWideString( pwszGlobalName ) ) ;
	}
}

// フレンドクラス判定
//////////////////////////////////////////////////////////////////////////////
bool ECSClassInfo::IsFriendClass( const wchar_t * pwszGlobalName ) const
{
	for ( int i = 0; i < (int) m_lstFriendClass.GetSize(); i ++ )
	{
		EWideString *	pwstrClass = m_lstFriendClass.GetAt( i ) ;
		if ( (pwstrClass != NULL) && (*pwstrClass == pwszGlobalName) )
		{
			return	true ;
		}
	}
	return	false ;
}

// naked メモリ上でのアライメント取得
//////////////////////////////////////////////////////////////////////////////
int ECSClassInfo::GetNakedAlignment( void ) const
{
	if ( GetVirtualFunctionCount() > 0 )
	{
		return	8 ;
	}
	unsigned int	i, nCount ;
	int				nMaxAlign = 1 ;
	nCount = GetVariableCount() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSTypeInfo *	pVarType = GetVariableAt( i ) ;
		if ( pVarType != NULL )
		{
			int	nAlign = pVarType->GetNakedAlignment() ;
			if ( nAlign > nMaxAlign )
			{
				nMaxAlign = nAlign ;
			}
		}
	}
	return	nMaxAlign ;
}

// naked クラスのデフォルトコンストラクタは必要か？
//////////////////////////////////////////////////////////////////////////////
bool ECSClassInfo::IsNeedsNakedClassConstruction
		( bool fNoCheckThisConstructor ) const
{
	if ( !IsNakedMemoryClass() )
	{
		return	false ;	// naked クラスではない
	}
	if ( !fNoCheckThisConstructor )
	{
		ListMemberFunction			lstFunc ;
		EPtrObjArray<ECSTypeInfo>	lstArg ;
		SearchFunctinoAs
			( lstFunc, m_wstrClassName, lstArg, 0, true, true ) ;
		if ( lstFunc.GetSize() > 0 )
		{
			return	true ;	// デフォルトコンストラクタがある
		}
	}
	//
	// 親クラス判定
	//
	int	i, nCount ;
	nCount = GetParentClassCount() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ParentClass *	pParentClass = GetParentClassAt( i ) ;
		if ( (pParentClass != NULL)
			&& (pParentClass->pClassInf != NULL) )
		{
			if ( pParentClass->pClassInf->
						IsNeedsNakedClassConstruction( false ) )
			{
				return	true ;	// 親クラスにコンストラクタが必要
			}
		}
	}
	//
	// メンバ変数判定
	//
	nCount = GetVariableCount() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSTypeInfo *	pVarType = GetVariableAt( i ) ;
		if ( (pVarType != NULL)
			&& (pVarType->m_pValue != NULL) )
		{
			if ( IsNeedsNakedVariableConstruction( pVarType->m_pValue ) )
			{
				return	true ;	// メンバ変数にコンストラクタが必要
			}
		}
	}
	return	false ;
}

bool ECSClassInfo::IsNeedsNakedVariableConstruction( ECSObject * pVarType )
{
	if ( pVarType != NULL )
	{
		if ( pVarType->m_vtType == csvtObject )
		{
			const ECSClassInfo *	pClassInf = pVarType->m_pClassInf ;
			if ( pClassInf != NULL )
			{
				if ( pClassInf->IsNeedsNakedClassConstruction( false ) )
				{
					return	true ;
				}
			}
		}
		else if ( pVarType->m_vtType == csvtArray )
		{
			return	IsNeedsNakedVariableConstruction
						( ((ECSArray*)pVarType)->GetEndDefaultElement() ) ;
		}
	}
	return	false ;
}

// naked クラスのデストラクタ呼び出しが必要か？
//////////////////////////////////////////////////////////////////////////////
bool ECSClassInfo::IsNeedsNakedClassDestruction( bool fNeedsNoNaked ) const
{
	if ( IsIntegerEnumeratorType() || IsRealEnumeratorType() )
	{
		return	false ;
	}
	if ( !IsNakedMemoryClass() )
	{
		return	fNeedsNoNaked ;	// naked クラスではない
	}
	if ( FindFunctionAs( L"<destructor>" ) >= 0 )
	{
		return	true ;	// デストラクタがある
	}
	//
	// 親クラス判定
	//
	int	i, nCount ;
	nCount = GetParentClassCount() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ParentClass *	pParentClass = GetParentClassAt( i ) ;
		if ( (pParentClass != NULL)
			&& (pParentClass->pClassInf != NULL) )
		{
			if ( pParentClass->pClassInf->
					IsNeedsNakedClassDestruction( false ) )
			{
				return	true ;	// 親クラスにデストラクタが必要
								// ※上の FindFunctionAs で
								// 　見つかるか、メンバ変数は下の
								// 　チェックで見つかるはずではあるが
			}
		}
	}
	//
	// メンバ変数判定
	//
	nCount = GetVariableCount() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSTypeInfo *	pVarType = GetVariableAt( i ) ;
		if ( (pVarType != NULL)
			&& (pVarType->m_pValue != NULL) )
		{
			if ( IsNeedsNakedVariableDestruction
						( pVarType->m_pValue, false ) )
			{
				return	true ;
			}
		}
	}
	return	false ;
}

bool ECSClassInfo::IsNeedsNakedVariableDestruction( ECSObject * pVarType, bool fNeedsNoNaked )
{
	if ( pVarType != NULL )
	{
		if ( pVarType->m_vtType == csvtObject )
		{
			const ECSClassInfo *	pClassInf = pVarType->m_pClassInf ;
			if ( pClassInf != NULL )
			{
				if ( pClassInf->IsNeedsNakedClassDestruction( fNeedsNoNaked ) )
				{
					return	true ;
				}
			}
		}
		else if ( pVarType->m_vtType == csvtArray )
		{
			return	IsNeedsNakedVariableDestruction
						( ((ECSArray*)pVarType)->GetEndDefaultElement(), fNeedsNoNaked ) ;
		}
		else if ( (pVarType->m_vtType == csvtString)
				|| (pVarType->m_vtType == csvtHash) )
		{
			return	true ;
		}
	}
	return	false ;
}

// 仮想関数ベクタを含んだメンバ変数オフセットアドレスに修正
//////////////////////////////////////////////////////////////////////////////
void ECSClassInfo::NormalizeNakedOffsetForVirtualVector( void )
{
	if ( !m_flagVirtualOffset )
	{
		if ( GetVirtualFunctionCount() > 0 )
		{
			OffsetNakedVariable( 0, 8 ) ;
		}
		m_flagVirtualOffset = true ;
	}
	/*
	int	iLastOffset = -1 ;
	for ( ; ; )
	{
		int	iMinCastBase = -1 ;
		int	i, nCastCount = m_wstaCastInf.GetSize() ;
		for ( i = 0; i < nCastCount; i ++ )
		{
			CastInfo *	pCastInf = m_wstaCastInf.GetObjectAt( i ) ;
			if ( (pCastInf != NULL)
				&& (pCastInf->pClassInf != NULL)
				&& (pCastInf->pClassInf->GetVirtualFunctionCount() > 0) )
			{
				if ( ((iMinCastBase < 0)
						|| (pCastInf->nNakedOffset < iMinCastBase))
					&& (pCastInf->nNakedOffset > iLastOffset) )
				{
					iMinCastBase = pCastInf->nNakedOffset ;
				}
			}
		}
		if ( iMinCastBase < 0 )
		{
			break ;
		}
		OffsetNakedVariable( iMinCastBase, 8 ) ;
		iLastOffset = iMinCastBase ;
	}
	*/
}

// クラスの naked サイズをアライメントで調整
//////////////////////////////////////////////////////////////////////////////
void ECSClassInfo::NormalizeNakedClassSize( void )
{
	if ( IsNakedMemoryClass()
		&& !(GetAttribute() & ECSTypeInfo::flagStructure) )
	{
		int	nAlign = GetNakedAlignment() ;
		m_nNakedSize += nAlign - 1 ;
		m_nNakedSize -= m_nNakedSize % nAlign ;
	}
}

// メンバの naked オフセットアドレスを加算
//////////////////////////////////////////////////////////////////////////////
void ECSClassInfo::OffsetNakedVariable( int iStart, int iOffset )
{
	//
	// 親クラスキャストオフセット
	//
	int	i, nCount ;
	nCount = m_wstaCastInf.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		CastInfo *	pCastInf = m_wstaCastInf.GetObjectAt( i ) ;
		if ( pCastInf != NULL )
		{
			ESLAssert( pCastInf->pClassInf != NULL ) ;
			if ( (pCastInf->pClassInf != NULL)
				&& (pCastInf->pClassInf->GetVirtualFunctionCount() > 0) )
			{
				if ( pCastInf->nNakedOffset > iStart )
				{
					pCastInf->nNakedOffset += iOffset ;
				}
			}
			else
			{
				if ( pCastInf->nNakedOffset >= iStart )
				{
					pCastInf->nNakedOffset += iOffset ;
				}
			}
		}
	}
	//
	// メンバ変数オフセット
	//
	nCount = m_lstVarNakedOffset.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		int	iVar = m_lstVarNakedOffset.GetAt( i ) ;
		if ( iVar >= iStart )
		{
			m_lstVarNakedOffset.SetAt( i, iVar + iOffset ) ;
		}
	}
	m_nNakedSize += iOffset ;
}
