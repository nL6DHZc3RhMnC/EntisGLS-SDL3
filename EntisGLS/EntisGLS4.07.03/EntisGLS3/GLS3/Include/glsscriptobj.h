
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#if	!defined(__GLSSCRIPTOBJ_H__)
#define	__GLSSCRIPTOBJ_H__


//////////////////////////////////////////////////////////////////////////////
// スクリプト用文字列
//////////////////////////////////////////////////////////////////////////////

typedef	EWideString	ECSWideString ;


//////////////////////////////////////////////////////////////////////////////
// 数値配列クラス
//////////////////////////////////////////////////////////////////////////////

template <class T>	class	ECSNumArray	: public	ENumArray<T>	{ } ;


//////////////////////////////////////////////////////////////////////////////
// オブジェクト配列
//////////////////////////////////////////////////////////////////////////////

template <class T>	class	ECSObjArray	: public SSystem::SObjectArray<T>
{
public:
	ECSObjArray( void ) {}
	ECSObjArray
		( ECSObjArray & array,
			unsigned int nFirst = 0, unsigned int nCount = -1 )
	{
		if ( nFirst < array.GetLength() )
		{
			if ( nCount == -1 )
			{
				nCount = array.m_nLength - nFirst ;
			}
			Merge( 0, array, nFirst, nCount ) ;
			array.Detach( nFirst, nCount ) ;
		}
	}
	unsigned int GetSize( void ) const
	{
		return	m_nLength ;
	}
	void SetSize( unsigned int nLength )
	{
		SetLength( nLength ) ;
	}
	void RemoveBetween( unsigned int nFirst, unsigned int nCount )
	{
		Remove( nFirst, nCount ) ;
	}
	void DetachBetween( unsigned int nFirst, unsigned int nCount )
	{
		Detach( nFirst, nCount ) ;
	}
	bool IsEqual( const SSystem::SObjectArray<T> & array ) const
	{
		if ( GetLength() != array.GetLength() )
		{
			return	false ;
		}
		const size_t nCount = GetLength() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			T *	pSrc1 = GetAt( i ) ;
			T *	pSrc2 = array.GetAt( i ) ;
			if ( (pSrc1 == NULL) || (pSrc2 == NULL) )
			{
				if ( pSrc1 != pSrc2 )
				{
					return	false ;
				}
			}
			if ( *pSrc1 != *pSrc2 )
			{
				return	false ;
			}
		}
		return	true ;
	}
	const ECSObjArray<T> & operator = ( const ECSObjArray<T> & array )
	{
		using namespace SSystem ;
		SObjectArray<T>::operator = ( array ) ;
		return	*this ;
	}
	bool operator == ( const SSystem::SObjectArray<T> & array ) const
	{
		return	IsEqual( array ) ;
	}
	bool operator != ( const SSystem::SObjectArray<T> & array ) const
	{
		return	!IsEqual( array ) ;
	}

} ;


//////////////////////////////////////////////////////////////////////////////
// 連想配列
//////////////////////////////////////////////////////////////////////////////

template <class T>	class	ECSWStrTagArray
					: public	ETagSortArray<ECSWideString,T> { } ;


//////////////////////////////////////////////////////////////////////////////
// 検索用インデックス付き文字列配列（静的文字列バッファ・追加のみ）
//////////////////////////////////////////////////////////////////////////////

class	ECSStrTagArray	: public	EPtrObjArray<const wchar_t>
{
public:
	// 構築関数
	ECSStrTagArray( void ) { }
	// 消滅関数
	virtual ~ECSStrTagArray( void ) { }
	// クラス情報
	DECLARE_CLASS_INFO( ECSStrTagArray, EPtrArray )

protected:
	ECSNumArray<int>	m_idxSorted ;

public:
	// 要素追加
	int Add( const wchar_t * pwszStr ) ;
	// インデックスを検索する
	int FindIndex( const wchar_t * pwszStr ) const ;
	// 全要素削除
	void RemoveAll( void ) ;
	// 全ての要素をコピー
	const ECSStrTagArray & operator = ( const ECSStrTagArray & array )
		{
			EPtrArray::operator = ( array ) ;
			m_idxSorted = array.m_idxSorted ;
			return	*this ;
		}
	// 全配列をデバッグ出力する
	void TraceDebugOutput( void ) ;

public:
	// 配列保存
	ESLError SaveArray( ESLFileObject & file ) ;
	// 配列復元
	ESLError LoadArray( ESLFileObject & file, ECSContext & context ) ;

public:		// メモリアロケーション
	// メモリ確保
	static void * operator new ( size_t stObj ) ;
	static void * operator new
		( size_t stObj, const char * pszFileName, int nLine ) ;
	// メモリ解放
	static void operator delete( void * ptrObj ) ;

} ;

class	ECSStrBufTagArray	: public ECSStrTagArray
{
public:
	// 構築関数
	ECSStrBufTagArray( void ) { }

protected:
	ECSObjArray<ECSWideString>	m_lstStrTagBuf ;

public:
	// 要素追加
	int Add( const wchar_t * pwszStr )
		{
			ECSWideString *	pwstrBuf = new EWideString( pwszStr ) ;
			m_lstStrTagBuf.Add( pwstrBuf ) ;
			return	ECSStrTagArray::Add( pwstrBuf->CharPtr() ) ;
		}
	// 全要素削除
	void RemoveAll( void )
		{
			m_lstStrTagBuf.RemoveAll() ;
			ECSStrTagArray::RemoveAll() ;
		}
	// 全ての要素をコピー
	const ECSStrTagArray & operator = ( const ECSStrBufTagArray & array )
		{
			m_lstStrTagBuf = array.m_lstStrTagBuf ;
			m_idxSorted = array.m_idxSorted ;
			//
			unsigned int	i, nCount ;
			nCount = m_lstStrTagBuf.GetSize() ;
			SetSize( nCount ) ;
			for ( i = 0; i < nCount; i ++ )
			{
				ESLAssert( m_lstStrTagBuf.GetAt(i) != NULL ) ;
				SetAt( i, m_lstStrTagBuf.GetAt(i)->CharPtr() ) ;
			}
			return	*this ;
		}
	// 比較
	bool operator == ( const ECSStrBufTagArray & array ) const
		{
			return	(m_lstStrTagBuf == array.m_lstStrTagBuf) ;
		}
	bool operator != ( const ECSStrBufTagArray & array ) const
		{
			return	(m_lstStrTagBuf != array.m_lstStrTagBuf) ;
		}

} ;


//////////////////////////////////////////////////////////////////////////////
// グローバル関数（簡易）インポート
//////////////////////////////////////////////////////////////////////////////

typedef	ESLError (*API_ECS_FUNC)
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;


//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script ソース解析オブジェクト
//////////////////////////////////////////////////////////////////////////////

class	ECSSourceStream	: public	EStreamWideString
{
public:
	// 構築関数
	ECSSourceStream( void ) ;
	ECSSourceStream( const ECSSourceStream & cssSrc ) ;
	ECSSourceStream( const wchar_t * pwszString ) ;
	ECSSourceStream( const char * pszString ) ;
	// 消滅関数
	virtual ~ECSSourceStream( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSSourceStream, EStreamWideString )

public:
	// 代入操作
	const ECSSourceStream &
				operator = ( const ECSSourceStream & cssSrc ) ;
	const ECSSourceStream &
				operator = ( const wchar_t * pwszString ) ;
	const ECSSourceStream &
				operator = ( const char * pszString ) ;

public:
	// 動作フラグ
	enum	ExpressionFlags
	{
		flagNormal				= 0,
		flagQuoteNakedString	= 0x00000002,
		flagDisableExpression	= 0x80000000,
	} ;
	// 現在の文字列（区切り記号は無視）を通過する
	virtual void PassEnclosedString
		( wchar_t wchClose, int flagCtrlCode = flagNormal ) ;
	// 現在のトークンを通過する
	virtual void PassAToken( int * pTokenType = NULL ) ;
	virtual void PassAExpressionTerm( int flagCtrlCode = flagNormal ) ;

public:		// メモリアロケーション
	// 文字列用バッファ確保
	virtual void AllocString( unsigned int nLength ) ;
	virtual void FreeString( void ) ;
	// メモリ確保
	static void * operator new ( size_t stObj ) ;
	static void * operator new
		( size_t stObj, const char * pszFileName, int nLine ) ;
	// メモリ解放
	static void operator delete( void * ptrObj ) ;

public:
	// オブジェクトの複製
	virtual ECSSourceStream * Duplicate( void ) const ;

} ;


//////////////////////////////////////////////////////////////////////////////
// SSystem::SFileInterface と ESLFileObject の変換
//////////////////////////////////////////////////////////////////////////////

#include "glscs_inter_file.h"


//////////////////////////////////////////////////////////////////////////////
// 詞葉オブジェクトクラス
//////////////////////////////////////////////////////////////////////////////

#include "glscsobj_object.h"
#include "glscsobj_reference.h"
#include "glscsobj_pointer.h"
#include "glscsobj_integer.h"
#include "glscsobj_real.h"
#include "glscsobj_string.h"
#include "glscsobj_array.h"
#include "glscsobj_hash.h"

#include "glscs_classinf.h"

#include "glscsobj_structure.h"
#include "glscsobj_buffer.h"
#include "glscsobj_buffer_structure.h"
#include "glscsobj_stack.h"
#include "glscsobj_global.h"
#include "glscsobj_function.h"


#if	!defined(_DEBUG)
const ECSObject * ECSObject::GetEntity( const ECSObject * pObj )
{
	while ( pObj != NULL )
	{
		if ( (pObj->m_vtType != csvtReference)
			/*& (pObj->m_vtType != csvtPointerReference)*/ )
		{
			return	pObj ;
		}
		pObj = ((const ECSReference *) pObj)->m_pRef ;
	}
	return	NULL ;
}

ECSObject * ECSObject::GetEntity( ECSObject * pObj )
{
	while ( pObj != NULL )
	{
		if ( (pObj->m_vtType != csvtReference)
			/*& (pObj->m_vtType != csvtPointerReference)*/ )
		{
			return	pObj ;
		}
		pObj = ((ECSReference*)pObj)->m_pRef ;
	}
	return	NULL ;
}

inline ECSObject * ECSObject::GetObjectEntity( void )
{
	ECSObject *	pObj = this ;
	while ( pObj != NULL )
	{
		if ( (pObj->m_vtType != csvtReference)
			/*& (pObj->m_vtType != csvtPointerReference)*/ )
		{
			return	pObj ;
		}
		pObj = ((ECSReference*)pObj)->m_pRef ;
	}
	return	NULL ;
}

inline const ECSObject * ECSObject::GetObjectEntity( void ) const
{
	const ECSObject *	pObj = this ;
	while ( pObj != NULL )
	{
		if ( (pObj->m_vtType != csvtReference)
			/*& (pObj->m_vtType != csvtPointerReference)*/ )
		{
			return	pObj ;
		}
		pObj = ((const ECSReference *) pObj)->m_pRef ;
	}
	return	NULL ;
}
#endif


#endif
