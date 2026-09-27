
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// バッファ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSBuffer, ECSObject )

// new Buffer
//////////////////////////////////////////////////////////////////////////////
ECS_EXPORT ECSSakura2::Object *
	ecs_new_object_Buffer
		( ECSSakura2Processor::Context * context, int cls_id )
{
	return	new ECSBuffer ;
}

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSBuffer::ECSBuffer( void )
{
	m_vtType = csvtBuffer ;
	m_pbytBuf = NULL ;
	m_nBufSize = 0 ;
	m_nBufBase = 0 ;
	m_nBufLimit = 0 ;
}

ECSBuffer::ECSBuffer( const ECSBuffer & buf )
{
	m_vtType = csvtBuffer ;
	m_pbytBuf = NULL ;
	m_nBufSize = 0 ;
	m_nBufBase = 0 ;
	m_nBufLimit = 0 ;
	//
	CopyBufferFrom( buf ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSBuffer::~ECSBuffer( void )
{
	FreeBuffer() ;
}

// バッファ生成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSBuffer::CreateBuffer( int nBytes, int nBase )
{
	FreeBuffer() ;
	//
	m_nBufSize = nBytes ;
	m_nBufBase = nBase ;
	m_nBufLimit = (nBytes + 0x0F) & ~0x0F ;
	m_pbytBuf =
		(BYTE*) eslHeapAllocate( NULL, m_nBufLimit, ESL_HEAP_ZERO_INIT ) ;
	//
	if ( m_pbytBuf == NULL )
	{
		return	eslErrFailed ;
	}
	return	eslErrSuccess ;
}

// バッファリサイズ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSBuffer::ResizeBuffer( int nBytes, int nBase )
{
	if ( nBytes > m_nBufLimit )
	{
		m_nBufSize = nBytes ;
		m_nBufBase = nBase ;
		m_nBufLimit = (nBytes + 0x0F) & ~0x0F ;
		m_pbytBuf =
			(BYTE*) eslHeapReallocate
						( NULL, m_pbytBuf, m_nBufLimit, ESL_HEAP_ZERO_INIT ) ;
	}
	else
	{
		m_nBufSize = nBytes ;
		m_nBufBase = nBase ;
	}
	return	eslErrSuccess ;
}

// バッファリミット設定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSBuffer::ResizeBufferLimit( int nLimit )
{
	if ( nLimit >= m_nBufSize )
	{
		nLimit = (nLimit + 0x0F) & ~0x0F ;
		if ( nLimit != m_nBufLimit )
		{
			m_pbytBuf =
				(BYTE*) eslHeapReallocate
					( NULL, m_pbytBuf, nLimit, ESL_HEAP_ZERO_INIT ) ;
			m_nBufLimit = nLimit ;
		}
	}
	return	eslErrSuccess ;
}

// バッファ解放
//////////////////////////////////////////////////////////////////////////////
void ECSBuffer::FreeBuffer( void )
{
	if ( m_pbytBuf != NULL )
	{
		eslHeapFree( NULL, m_pbytBuf, 0 ) ;
		m_pbytBuf = NULL ;
	}
	m_nBufSize = 0 ;
	m_nBufLimit = 0 ;
}

// バッファ複製
//////////////////////////////////////////////////////////////////////////////
void ECSBuffer::CopyBufferFrom( const ECSBuffer & buf )
{
	m_pClassInf = buf.m_pClassInf ;
	//
	CreateBuffer( buf.m_nBufSize ) ;
	//
	if ( m_pbytBuf != NULL )
	{
		eslMoveMemory( m_pbytBuf, buf.m_pbytBuf, buf.m_nBufSize ) ;
	}
}

// 書き出しバッファ確保
//////////////////////////////////////////////////////////////////////////////
void * ECSBuffer::PutBuffer( int nSize )
{
	if ( m_nBufSize + nSize > m_nBufLimit )
	{
		int	nLimit = m_nBufLimit + (m_nBufLimit >> 2) ;
		if ( nLimit < m_nBufSize + nSize )
		{
			nLimit = m_nBufSize + nSize ;
		}
		ResizeBufferLimit( nLimit ) ;
	}
	return	m_pbytBuf + m_nBufSize ;
}

// 書き出しバッファ確定
//////////////////////////////////////////////////////////////////////////////
void ECSBuffer::Flush( int nSize )
{
	if ( m_nBufSize + nSize > m_nBufLimit )
	{
		m_nBufSize = m_nBufLimit ;
	}
	else
	{
		m_nBufSize += nSize ;
	}
}

// バッファ変更
//////////////////////////////////////////////////////////////////////////////
void * ECSBuffer::ModifyBuffer( int nPos, int nSize )
{
	return	ECSBuffer::GetBuffer( nPos, nSize, true ) ;
}

// バッファ結合
//////////////////////////////////////////////////////////////////////////////
void ECSBuffer::MergeBuffer( const void * ptrBuf, int nSize )
{
	void *	ptrDst = PutBuffer( nSize ) ;
	::eslMoveMemory( ptrDst, ptrBuf, nSize ) ;
	Flush( nSize ) ;
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSBuffer::GetTypeName( void ) const
{
	return	L"Buffer" ;
}

ECSObject * ECSBuffer::GetTypeOf( const wchar_t * pwszTypeName )
{
	if ( EWideString::Compare( L"Buffer", pwszTypeName ) == 0 )
	{
		return	this ;
	}
	return	NULL ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSBuffer::Duplicate( void )
{
	return	new ECSBuffer( *this ) ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSBuffer::Move
	( ECSContext & context, ECSObject * obj )
{
	ECSBuffer *	pSrcBuf = ESLTypeCast<ECSBuffer>( ECSObject::GetEntity( obj ) ) ;
	if ( pSrcBuf == NULL )
	{
		return	ESLErrorMsg( "Buffer への不正な代入です" ) ;
	}
	CopyBufferFrom( *pSrcBuf ) ;
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSBuffer::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	return	ESLErrorMsg( "Buffer の定義されていない単項演算子です" ) ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSBuffer::Operate
	( ECSContext & context,
		CSOperatorType csopType, ECSObject * obj )
{
	return	ESLErrorMsg( "Buffer の定義されていない演算子です" ) ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSBuffer::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	return	ESLErrorMsg( "Buffer の定義されていない比較演算子です" ) ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSBuffer::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	nIndex = m_staFuncName->FindIndex( pwszName ) ;
	if ( nIndex < 0 )
	{
		return	ESLErrorMsg
			( "定義されていないメンバ関数を呼び出しています。" ) ;
	}
	return	eslErrSuccess ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSBuffer::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	if ( (unsigned int) nIndex >= m_staFuncName->GetSize() )
	{
		return	ESLErrorMsg( "不正なメンバ関数を呼び出そうとしました。" ) ;
	}
	return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
}

// 特殊演算子 : sizeof
//////////////////////////////////////////////////////////////////////////////
ESLError ECSBuffer::OperateSizeOf( INT64 & nSize )
{
	nSize = m_nBufSize ;
	return	eslErrSuccess ;
}

// 特殊演算子 : typeof
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSBuffer::OperateTypeOf( void ) const
{
	return	L"Buffer" ;
}

// 内部バッファインターフェース
//////////////////////////////////////////////////////////////////////////////
void * ECSBuffer::GetBuffer( int iOffset, int nSize, bool fWritable )
{
	iOffset -= m_nBufBase ;
	if ( (iOffset >= 0) & ((UINT)(iOffset + nSize) <= (UINT)m_nBufSize) )
	{
		return	m_pbytBuf + iOffset ;
	}
	return	NULL ;
}

ECSSakura2Processor::LinearAddressCache *
	ECSBuffer::GetSegmentBuffer( ECSSakura2Processor::LinearAddressCache & seg )
{
	seg.baseOffset = m_nBufBase ;
	seg.limitSegment = m_nBufSize ;
	seg.pbytBuffer = m_pbytBuf ;
	return	&seg ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSBuffer::Save( ESLFileObject & file, ECSContext & context )
{
	file.Write( &m_nBufBase, sizeof(m_nBufBase) ) ;
	//
	if ( file.Write( &m_nBufSize, sizeof(m_nBufSize) ) < sizeof(m_nBufSize) )
	{
		return	eslErrFailed ;
	}
	if ( m_nBufSize > 0 )
	{
		file.Write( m_pbytBuf, m_nBufSize ) ;
	}
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSBuffer::Load( ESLFileObject & file, ECSContext & context )
{
	int	nBufSize, nBufBase ;
	file.Read( &nBufBase, sizeof(nBufBase) ) ;
	if ( file.Read( &nBufSize, sizeof(nBufSize) ) < sizeof(nBufSize) )
	{
		return	eslErrFailed ;
	}
	if ( nBufSize > 0 )
	{
		CreateBuffer( nBufSize, nBufBase ) ;
		file.Read( m_pbytBuf, nBufSize ) ;
	}
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSBuffer::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	EString	strDump ;
	strDump += EString( m_nBufSize ) + " [bytes]" ;
	buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	return	eslErrSuccess ;
}

// スクリプトのデストラクタ
//////////////////////////////////////////////////////////////////////////////
void ECSBuffer::OnDestruction( ECSContext & context )
{
	FreeBuffer() ;
}

// メンバ関数ポインタ
//////////////////////////////////////////////////////////////////////////////
ECSStrTagArray *	ECSBuffer::m_staFuncName = NULL ;
const wchar_t *		ECSBuffer::m_pwszFuncName[4] =
{
	L"CreateBuffer", L"FreeBuffer", L"ResizeBuffer", NULL
} ;
const ECSBuffer::PFUNC_CALL	ECSBuffer::m_pfnCallFunc[3] =
{
	&ECSBuffer::Call_CreateBuffer,
	&ECSBuffer::Call_FreeBuffer,
	&ECSBuffer::Call_ResizeBuffer,
} ;

// メンバ関数 : Error CreateBuffer( Integer nBytes )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSBuffer::Call_CreateBuffer
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	int	nBytes ;
	err = context.GetArgumentAsInt( nBytes, lstArg, 1, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = CreateBuffer( nBytes ) ;
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : Error FreeBuffer()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSBuffer::Call_FreeBuffer
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	//
	FreeBuffer() ;
	//
	return	context.PushObject( context.new_CSInteger() ) ;
}

// メンバ関数 : Error ResizeBuffer( Integer nBytes )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSBuffer::Call_ResizeBuffer
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	int	nBytes ;
	err = context.GetArgumentAsInt( nBytes, lstArg, 1, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = ResizeBuffer( nBytes ) ;
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}


//////////////////////////////////////////////////////////////////////////////
// （スクリプト）読み込み専用バッファ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSReadOnlyBuffer, ECSBuffer )

// 内部バッファインターフェース
//////////////////////////////////////////////////////////////////////////////
void * ECSReadOnlyBuffer::GetBuffer( int iOffset, int nSize, bool fWritable )
{
	if ( !fWritable )
	{
		return	ECSBuffer::GetBuffer( iOffset, nSize, fWritable ) ;
	}
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// コード専用バッファ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSCodeBuffer, ECSReadOnlyBuffer )
// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSCodeBuffer::ECSCodeBuffer( void )
{
	m_pbytShadow = NULL ;
}

ECSCodeBuffer::ECSCodeBuffer( const ECSBuffer & buf )
	 : ECSReadOnlyBuffer( buf )
{
	m_pbytShadow = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSCodeBuffer::~ECSCodeBuffer( void )
{
	FreeBuffer() ;
}

// バッファリサイズ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCodeBuffer::ResizeBuffer( int nBytes, int nBase )
{
	if ( nBytes > m_nBufLimit )
	{
		m_nBufSize = nBytes ;
		m_nBufBase = nBase ;
		m_nBufLimit = (nBytes + 0x0F) & ~0x0F ;
		m_pbytBuf =
			(BYTE*) eslHeapReallocate
						( NULL, m_pbytBuf, m_nBufLimit, ESL_HEAP_ZERO_INIT ) ;
		//
		if ( m_pbytShadow != NULL )
		{
			m_pbytShadow =
				(BYTE*) eslHeapReallocate
					( NULL, m_pbytShadow, m_nBufLimit, ESL_HEAP_ZERO_INIT ) ;
		}
	}
	else
	{
		m_nBufSize = nBytes ;
		m_nBufBase = nBase ;
	}
	return	eslErrSuccess ;
}

// バッファリミット設定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCodeBuffer::ResizeBufferLimit( int nLimit )
{
	if ( nLimit >= m_nBufSize )
	{
		nLimit = (nLimit + 0x0F) & ~0x0F ;
		if ( nLimit != m_nBufLimit )
		{
			m_pbytBuf =
				(BYTE*) eslHeapReallocate
					( NULL, m_pbytBuf, nLimit, ESL_HEAP_ZERO_INIT ) ;
			m_nBufLimit = nLimit ;
			//
			if ( m_pbytShadow != NULL )
			{
				m_pbytShadow =
					(BYTE*) eslHeapReallocate
						( NULL, m_pbytShadow, m_nBufLimit, ESL_HEAP_ZERO_INIT ) ;
			}
		}
	}
	return	eslErrSuccess ;
}

// バッファ解放
//////////////////////////////////////////////////////////////////////////////
void ECSCodeBuffer::FreeBuffer( void )
{
	if ( m_pbytShadow != NULL )
	{
		eslHeapFree( NULL, m_pbytShadow, 0 ) ;
		m_pbytShadow = NULL ;
	}
	ECSBuffer::FreeBuffer() ;
}

// シャドウバッファ生成
//////////////////////////////////////////////////////////////////////////////
void ECSCodeBuffer::CreateShadowBuffer( void )
{
	if ( m_pbytShadow == NULL )
	{
		m_pbytShadow =
			(BYTE*) eslHeapAllocate
				( NULL, m_nBufLimit, ESL_HEAP_ZERO_INIT ) ;
	}
}

// 内部バッファインターフェース
//////////////////////////////////////////////////////////////////////////////
BYTE * ECSCodeBuffer::GetSegmentShadowBuffer( int iShadow )
{
	return	m_pbytShadow ;
}


