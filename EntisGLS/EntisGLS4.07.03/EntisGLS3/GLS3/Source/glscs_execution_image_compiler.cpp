
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// ECSExecutionImageCompiler::RegisterContext
//////////////////////////////////////////////////////////////////////////////

// レジスタのメモリへの書き出しが必要
//////////////////////////////////////////////////////////////////////////////
bool ECSExecutionImageCompiler::RegisterContext::IsModifiedRegister( void ) const
{
	for ( int i = 0; i < regAssignMax; i ++ )
	{
		if ( regAssigns[i].flagLoaded && regAssigns[i].flagModified )
		{
			return	true ;
		}
	}
	return	false ;
}

// マージ可能か？（レジスタ割り当てが一致しているか？）
//////////////////////////////////////////////////////////////////////////////
bool ECSExecutionImageCompiler::RegisterContext::IsMergableContextFrom
	( const RegisterContext & context )
{
	for ( int i = 0; i < regAssignMax; i ++ )
	{
		if ( regAssigns[i].flagLoaded )
		{
			if ( !context.regAssigns[i].flagLoaded
				|| (regAssigns[i].offsetLocal != context.regAssigns[i].offsetLocal)
				|| (regAssigns[i].typeLocal != context.regAssigns[i].typeLocal) )
			{
				return	false ;
			}
		}
	}
	return	true ;
}

// マージ（レジスタの変更フラグを優先して複製）
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::RegisterContext::MergeContextFrom
	( const RegisterContext & context )
{
	for ( int i = 0; i < regAssignMax; i ++ )
	{
		ESLAssert( regAssigns[i].flagLoaded
						== context.regAssigns[i].flagLoaded ) ;
		regAssigns[i].flagModified =
			(regAssigns[i].flagModified
					|| context.regAssigns[i].flagModified) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 実行イメージ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSExecutionImageCompiler, ECSExecutionImageLinker )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSExecutionImageCompiler::ECSExecutionImageCompiler
	( ECSExecutionImageCompiler * pcsxiMaster )
{
	m_pcsxiMaster = pcsxiMaster ;
	//
	m_dwLastInstructionPos = 0 ;
	m_icLastInstruction = ECSSakura2Processor::codeInvalid ;
	//
	m_dwOptimizeStart = 0 ;
	m_dwOptimizeEnd = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSExecutionImageCompiler::~ECSExecutionImageCompiler( void )
{
}

// 実行イメージを消去
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::DeleteImage( void )
{
	ECSExecutionImage::DeleteImage() ;
	//
	m_dwLastInstructionPos = 0 ;
	m_icLastInstruction = ECSSakura2Processor::codeInvalid ;
}

// クラスを検索
//////////////////////////////////////////////////////////////////////////////
int ECSExecutionImageCompiler::GetClassInfoIndex
						( const wchar_t * pwszClassName ) const
{
	if ( m_pcsxiMaster != NULL )
	{
		int	iClassID = m_pcsxiMaster->GetClassInfoIndex( pwszClassName ) ;
		if ( iClassID >= 0 )
		{
			return	iClassID ;
		}
	}
	return	ECSExecutionImageLinker::GetClassInfoIndex( pwszClassName ) ;
}

// 最後に出力した命令コードをリセットする
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::FenceInstruction( void )
{
	FlushAllRegisterAssigns() ;
	ResetAllRegisterAssigns() ;
	m_icLastInstruction = ECSSakura2Processor::codeInvalid ;
}

// レジスタ割り当てを初期化
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::InitializeAllRegisterAssigns( void )
{
	for ( int i = 0; i < regAssignMax; i ++ )
	{
		if ( m_regAssigns[i].flagLoaded )
		{
			m_regAssigns[i].flagLoaded = false ;
			m_regAssigns[i].flagUnloaded = false ;
			m_regAssigns[i].countLocked = 0 ;
		}
	}
}

// 全てのレジスタ割り当てを解除
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::ResetAllRegisterAssigns( void )
{
	for ( int i = 0; i < regAssignMax; i ++ )
	{
		if ( m_regAssigns[i].flagLoaded )
		{
			if ( m_regAssigns[i].countLocked == 0 )
			{
				m_regAssigns[i].flagLoaded = false ;
			}
			else
			{
				m_regAssigns[i].flagUnloaded = true ;
			}
		}
	}
}

void ECSExecutionImageCompiler::ResetLocalBoundsAssignedRegisters
									( int offsetFirst, int offsetEnd )
{
	for ( int i = 0; i < regAssignMax; i ++ )
	{
		if ( m_regAssigns[i].flagLoaded )
		{
			if ( (offsetFirst <= m_regAssigns[i].offsetLocal)
				&& (m_regAssigns[i].offsetLocal < offsetEnd) )
			{
				if ( m_regAssigns[i].countLocked == 0 )
				{
					m_regAssigns[i].flagLoaded = false ;
				}
				else
				{
					m_regAssigns[i].flagUnloaded = true ;
				}
			}
		}
	}
}

// unloaded 状態のレジスタを再ロード
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::ReloadAllRegisterAssigns( void )
{
	for ( int i = 0; i < regAssignMax; i ++ )
	{
		if ( m_regAssigns[i].flagLoaded
			&& m_regAssigns[i].flagUnloaded )
		{
			DWORD	dwOffset = m_regAssigns[i].offsetLocal ;
			WriteSakuraInstructionCode
				( ECSSakura2Processor::codeLoadLocalImm32 ) ;
			WriteByteCode
				( (BYTE) m_regAssigns[i].typeLocal ) ;
			WriteByteCode( (BYTE) i ) ;
			WriteCodeData( &dwOffset, sizeof(DWORD) ) ;
			//
			m_regAssigns[i].flagUnloaded = false ;
//			m_regAssigns[i].countLocked = 0 ;
		}
	}
}

// レジスタへの変更をローカルメモリに反映させる
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::FlushAllRegisterAssigns( void )
{
	for ( int i = 0; i < regAssignMax; i ++ )
	{
		FlushRegisterAssign( i ) ;
	}
}

void ECSExecutionImageCompiler::FlushLocalBoundsAssignedRegisters
	( int offsetFirst, int offsetEnd )
{
	for ( int i = 0; i < regAssignMax; i ++ )
	{
		if ( m_regAssigns[i].flagLoaded )
		{
			if ( (offsetFirst <= m_regAssigns[i].offsetLocal)
				&& (m_regAssigns[i].offsetLocal < offsetEnd) )
			{
				FlushRegisterAssign( i ) ;
				m_regAssigns[i].flagLoaded = false ;
			}
		}
	}
}

void ECSExecutionImageCompiler::FlushRegisterAssign( int regNum )
{
	ESLAssert( (regNum >= 0) && (regNum < regAssignMax) ) ;
	if ( m_regAssigns[regNum].flagLoaded )
	{
		if ( m_regAssigns[regNum].flagModified )
		{
			ESLAssert( !m_regAssigns[regNum].flagUnloaded ) ;
			if ( !m_regAssigns[regNum].flagUnloaded )
			{
				DWORD	dwOffset = m_regAssigns[regNum].offsetLocal ;
				WriteSakuraInstructionCode
					( ECSSakura2Processor::codeStoreLocalImm32 ) ;
				WriteByteCode
					( (BYTE) m_regAssigns[regNum].typeLocal ) ;
				WriteByteCode( (BYTE) regNum ) ;
				WriteCodeData( &dwOffset, sizeof(DWORD) ) ;
			}
			m_regAssigns[regNum].flagModified = false ;
			m_icLastInstruction = ECSSakura2Processor::codeInvalid ;
		}
	}
}

// レジスタ・コンテキスト取得
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::GetRegisterContext( RegisterContext & context )
{
	for ( int i = 0; i < regAssignMax; i ++ )
	{
		context.regAssigns[i] = m_regAssigns[i] ;
	}
}

// レジスタ・コンテキスト復元
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::RestoreRegisterContext
	( const RegisterContext & context )
{
	for ( int i = 0; i < regAssignMax; i ++ )
	{
		bool	fModified = m_regAssigns[i].flagModified
								| context.regAssigns[i].flagModified ;
		m_regAssigns[i] = context.regAssigns[i] ;
		m_regAssigns[i].flagModified =
							fModified && m_regAssigns[i].flagLoaded ;
	}
}

// レジスタ・コンテキストをマージ可能か？
//////////////////////////////////////////////////////////////////////////////
bool ECSExecutionImageCompiler::IsMergableContextTo
	( const RegisterContext & context ) const
{
	for ( int i = 0; i < regAssignMax; i ++ )
	{
		if ( context.regAssigns[i].flagLoaded )
		{
			if ( !m_regAssigns[i].flagLoaded
				|| (context.regAssigns[i].offsetLocal != m_regAssigns[i].offsetLocal)
				|| (context.regAssigns[i].typeLocal != m_regAssigns[i].typeLocal) )
			{
				return	false ;
			}
		}
	}
	return	true ;
}

// レジスタ・コンテキストをマージするためのメモリ処理
//////////////////////////////////////////////////////////////////////////////
bool ECSExecutionImageCompiler::FlushMergableContextTo( RegisterContext & context )
{
	bool	fMergable = true ;
	for ( int i = 0; i < regAssignMax; i ++ )
	{
		if ( context.regAssigns[i].flagLoaded )
		{
			if ( !m_regAssigns[i].flagLoaded
				|| (context.regAssigns[i].offsetLocal != m_regAssigns[i].offsetLocal)
				|| (context.regAssigns[i].typeLocal != m_regAssigns[i].typeLocal) )
			{
				fMergable = false ;
			}
		}
		else
		{
			FlushRegisterAssign( i ) ;
		}
	}
	return	fMergable ;
}

// 割り当てられたレジスタを取得
//////////////////////////////////////////////////////////////////////////////
int ECSExecutionImageCompiler::FindAssignedRegister
		( int offseLocal, ECSSakura2Processor::DataType typeLocal ) const
{
	for ( int i = 0; i < regAssignMax; i ++ )
	{
		if ( m_regAssigns[i].flagLoaded )
		{
			if ( (m_regAssigns[i].offsetLocal == offseLocal)
				&& (m_regAssigns[i].typeLocal == typeLocal) )
			{
				return	i ;
			}
		}
	}
	return	-1 ;
}

// レジスタにローカル変数が割り当てられているか？
//////////////////////////////////////////////////////////////////////////////
bool ECSExecutionImageCompiler::IsRegisterAssigned( int regNum ) const
{
	ESLAssert( (regNum >= 0) && (regNum < regAssignMax) ) ;
	if ( (regNum >= 0) && (regNum < regAssignMax) )
	{
		return	m_regAssigns[regNum].flagLoaded ;
	}
	return	false ;
}

// レジスタの割り当てローカルアドレスを取得
//////////////////////////////////////////////////////////////////////////////
int ECSExecutionImageCompiler::GetAssignedLocalOffset( int regNum ) const
{
	ESLAssert( (regNum >= 0) && (regNum < regAssignMax) ) ;
	if ( (regNum >= 0) && (regNum < regAssignMax) )
	{
		ESLAssert( m_regAssigns[regNum].flagLoaded ) ;
		return	m_regAssigns[regNum].offsetLocal ;
	}
	return	0 ;
}

// レジスタにローカル変数を割り当てる
//（必要であれば古いレジスタはライトバックする）
//////////////////////////////////////////////////////////////////////////////
int ECSExecutionImageCompiler::AssignLocalToRegister
			( int offsetLocal, ECSSakura2Processor::DataType typeLocal )
{
	int	regNum = FindAssignedRegister( offsetLocal, typeLocal ) ;
	if ( regNum < 0 )
	{
		//
		// 割り当てのためのレジスタスロットを検索する
		//
		int	recentAccess = -1 ;
		for ( int i = 1; i < regAssignMax; i ++ )
		{
			if ( m_regAssigns[i].flagLoaded )
			{
				if ( (recentAccess < m_regAssigns[i].lastAccess)
						&& (m_regAssigns[i].countLocked == 0) )
				{
					recentAccess = m_regAssigns[i].lastAccess ;
					regNum = i ;
				}
			}
			else
			{
				regNum = i ;
				break ;
			}
		}
		if ( regNum < 0 )
		{
			return	-1 ;
		}
		ESLAssert( regNum > 0 ) ;
		FlushRegisterAssign( regNum ) ;
		//
		m_regAssigns[regNum].flagLoaded = true ;
		m_regAssigns[regNum].flagModified = false ;
		m_regAssigns[regNum].lastAccess = 0 ;
		m_regAssigns[regNum].offsetLocal = offsetLocal ;
		m_regAssigns[regNum].typeLocal = typeLocal ;
	}
	//
	// アクセス頻度を更新する
	//
	AccessAssignedRegister( regNum ) ;
	//
	return	regNum ;
}

// レジスタのアクセス履歴を更新する
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::AccessAssignedRegister( int regNum )
{
	for ( int i = 0; i < regAssignMax; i ++ )
	{
		if ( m_regAssigns[i].flagLoaded )
		{
			if ( regNum == i )
			{
				m_regAssigns[i].lastAccess = 0 ;
			}
			else
			{
				m_regAssigns[i].lastAccess ++ ;
			}
		}
	}
}

// レジスタの割り当てを解放する（ライトバックはしない）
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::FreeAssignLocalToRegister
		( int offsetLocal, ECSSakura2Processor::DataType typeLocal )
{
	int	regNum = FindAssignedRegister( offsetLocal, typeLocal ) ;
	if ( regNum >= 0 )
	{
		ESLAssert( m_regAssigns[regNum].countLocked == 0 ) ;
		m_regAssigns[regNum].flagLoaded = false ;
		m_regAssigns[regNum].flagModified = false ;
		m_regAssigns[regNum].countLocked = 0 ;
	}
}

// レジスタの割り当てをロックする
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::LockAssignedRegister( int regNum )
{
	ESLAssert( (regNum >= 0) && (regNum < regAssignMax) ) ;
	if ( (regNum >= 0) && (regNum < regAssignMax) )
	{
		ESLAssert( m_regAssigns[regNum].flagLoaded ) ;
		m_regAssigns[regNum].countLocked ++ ;
	}
}

// レジスタの割り当てのロックを1回アンロックする
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::UnlockAssignedRegister( int regNum )
{
	ESLAssert( (regNum >= 0) && (regNum < regAssignMax) ) ;
	if ( (regNum >= 0) && (regNum < regAssignMax) )
	{
//		ESLAssert( m_regAssigns[regNum].flagLoaded ) ;
		if ( m_regAssigns[regNum].flagLoaded )
		{
			if ( m_regAssigns[regNum].countLocked > 0 )
			{
				m_regAssigns[regNum].countLocked -- ;
			}
			if ( m_regAssigns[regNum].countLocked <= 0 )
			{
				if ( m_regAssigns[regNum].flagUnloaded )
				{
					ESLAssert( !m_regAssigns[regNum].flagModified ) ;
					m_regAssigns[regNum].flagLoaded = false ;
					m_regAssigns[regNum].flagUnloaded = false ;
				}
			}
		}
		else
		{
			ESLAssert( m_regAssigns[regNum].countLocked == 0 ) ;
		}
	}
}

// １バイト書き出し
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::WriteByteCode( BYTE nCode )
{
	BYTE	bytCode = (BYTE) nCode ;
	m_bufImage.MergeBuffer( &bytCode, sizeof(BYTE) ) ;
}

void ECSExecutionImageCompiler::WriteInstructionCode( CSInstructionCode icode )
{
	m_dwLastInstructionPos = m_bufImage.GetLength() ;
	m_icLastInstruction = ECSSakura2Processor::codeInvalid ;
	//
	WriteByteCode( icode ) ;
}

// 文字列書き出し
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::WriteConstantString( const ECSWideString & wstrData )
{
	DWORD	dwLength = 0x80000000 ;
	m_bufImage.MergeBuffer( &dwLength, sizeof(dwLength) ) ;
	//
	unsigned int		nIndex ;
	ENumArray<DWORD> *	pList = m_extConstStr.GetAs( wstrData, &nIndex ) ;
	if ( pList == NULL )
	{
		pList = new ENumArray<DWORD> ;
		nIndex = m_extConstStr.SetAs( wstrData, pList ) ;
		m_lstConstStr.InsertAt( nIndex, new ECSString( wstrData ) ) ;
	}
	pList->Add( m_bufImage.GetLength() ) ;
	//
	DWORD	dwDummy = nIndex ;
	m_bufImage.MergeBuffer( &dwDummy, sizeof(dwDummy) ) ;
}

// クラスインデックス書き出し
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::WriteClassIndex( DWORD dwClassIndex )
{
	m_extClassIndexRef.Add( m_bufImage.GetLength() ) ;
	m_bufImage.MergeBuffer( &dwClassIndex, sizeof(DWORD) ) ;
}

// ネイティブ関数インデックス書き出し
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::WriteNativeFunctionIndex
					( const wchar_t * pwszFuncName )
{
	int	iFuncIndex = m_staNativeFuncName.FindIndex( pwszFuncName ) ;
	if ( iFuncIndex < 0 )
	{
		iFuncIndex = m_staNativeFuncName.Add( pwszFuncName ) ;
	}
	//
	m_impNativeFunc.Add( m_bufImage.GetLength() ) ;
	//
	DWORD	dwIndex = iFuncIndex ;
	m_bufImage.MergeBuffer( &dwIndex, sizeof(DWORD) ) ;
}

void ECSExecutionImageCompiler::WriteNakedNativeFunctionIndex
					( const wchar_t * pwszFuncName )
{
//	int	iFuncIndex = m_staNakedNativeFuncName.FindIndex( pwszFuncName ) ;
	int	iFuncIndex = m_vectorSysCall.FindEntry( pwszFuncName ) ;
	if ( iFuncIndex < 0 )
	{
//		iFuncIndex = m_staNakedNativeFuncName.Add( pwszFuncName ) ;
		iFuncIndex = m_vectorSysCall.AddEntry( pwszFuncName ) ;
	}
	//
	m_impNakedNativeFunc.Add( m_bufImage.GetLength() ) ;
	//
	DWORD	dwIndex = iFuncIndex ;
	m_bufImage.MergeBuffer( &dwIndex, sizeof(DWORD) ) ;
}

// ネイティブ関数インデックス生成
//////////////////////////////////////////////////////////////////////////////
int ECSExecutionImageCompiler::MakeNakedNativeFunctionIndex
					( const wchar_t * pwszFuncName )
{
	int	iFuncIndex = m_vectorSysCall.FindEntry( pwszFuncName ) ;
	if ( iFuncIndex < 0 )
	{
		iFuncIndex = m_vectorSysCall.AddEntry( pwszFuncName ) ;
	}
	return	iFuncIndex ;
}

// 関数アドレス（参照）書き出し
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::WriteFunctionAddress
			( const wchar_t * pwszGlobalFuncName )
{
	AddCodeRefFunctionAddress( pwszGlobalFuncName, m_bufImage.GetLength() ) ;
	//
	DWORD	dwAddrDummy = 0 ;
	m_bufImage.MergeBuffer( &dwAddrDummy, sizeof(DWORD) ) ;
}

// n バイト書き出し
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::WriteCodeData
	( const void * ptrData, unsigned int nBytes )
{
	m_bufImage.MergeBuffer( ptrData, nBytes ) ;
}

// イメージを確定する
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::CommitImage( void )
{
	m_dwImageSize = m_bufImage.GetLength( ) ;
	m_pImage = (BYTE*) m_bufImage.ModifyBuffer( 0, m_dwImageSize ) ;
}


//////////////////////////////////////////////////////////////////////////
// 詞葉 3.0 Sakura2 仮想マシン用コード出力
//////////////////////////////////////////////////////////////////////////

// 命令コード出力
//////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::WriteSakuraInstructionCode
		( ECSSakura2Processor::InstructionCode code )
{
	m_dwLastInstructionPos = m_bufImage.GetLength() ;
	m_icLastInstruction = code ;
	//
	WriteByteCode( (BYTE) code ) ;
}

// CSVariableType から ECSSakura2Processor::DataType へ変換
//////////////////////////////////////////////////////////////////////////
ECSSakura2Processor::DataType
	ECSExecutionImageCompiler::DataTypeFromVariableType
										( CSVariableType csvtType )
{
	switch ( csvtType )
	{
	case	csvtInteger:
	case	csvtInteger64:
	case	csvtObject:
	case	csvtReference:
	case	csvtPointer:
	case	csvtReal:
	case	csvtReal64:
	default:
		return	ECSSakura2Processor::dataInt64 ;
	case	csvtReal32:
		return	ECSSakura2Processor::dataFloat ;
	case	csvtBoolean:
	case	csvtInt8:
		return	ECSSakura2Processor::dataInt8 ;
	case	csvtUint8:
		return	ECSSakura2Processor::dataUint8 ;
	case	csvtInt16:
		return	ECSSakura2Processor::dataInt16 ;
	case	csvtUint16:
		return	ECSSakura2Processor::dataUint16 ;
	case	csvtInt32:
		return	ECSSakura2Processor::dataInt32 ;
	case	csvtUint32:
		return	ECSSakura2Processor::dataUint32 ;
	}
}

// ECSTypeInfo からアドレッシングモード取得
//////////////////////////////////////////////////////////////////////////
ECSSakura2Processor::AddressingMode
	ECSExecutionImageCompiler::AddressingModeFromTypeInfo
										( const ECSTypeInfo & typeVar )
{
	if ( typeVar.m_regIndex < 0 )
	{
		if ( typeVar.m_addrOffset == 0 )
		{
			return	ECSSakura2Processor::addrBase ;
		}
		else
		{
			return	ECSSakura2Processor::addrBaseOffset32 ;
		}
	}
	else
	{
		if ( typeVar.m_addrOffset == 0 )
		{
			return	ECSSakura2Processor::addrBaseIndex ;
		}
		else
		{
			return	ECSSakura2Processor::addrBaseIndexOffset32 ;
		}
	}
}

// ロード・ストア命令
//////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImageCompiler::WriteSakuraMoveMemory
	( bool fStore,
		ECSSakura2Processor::AddressingMode mode,
		ECSSakura2Processor::DataType type, int& regDst,
		int regBase, int offset32,
		int regIndex, int scaleIndex, bool fNoRegCache )
{
	if ( regBase >= 0x10 )
	{
		if ( mode == ECSSakura2Processor::addrBase )
		{
			mode = ECSSakura2Processor::addrBaseIndex ;
			regIndex = regBase ;
			scaleIndex = 0 ;
			regBase = ECSSakura2Processor::regZeroPtr ;
		}
		else if ( mode == ECSSakura2Processor::addrBaseOffset32 )
		{
			mode = ECSSakura2Processor::addrBaseIndexOffset32 ;
			regIndex = regBase ;
			scaleIndex = 0 ;
			regBase = ECSSakura2Processor::regZeroPtr ;
		}
		else
		{
			return	ESLErrorMsg
				( "内部エラー：メモリアクセスのベースレジスタが不正です" ) ;
		}
	}
	/*
	if ( (mode == ECSSakura2Processor::addrBase)
		|| (mode == ECSSakura2Processor::addrBaseOffset32) )
	{
		if ( (m_icLastInstruction == ECSSakura2Processor::codeAddImm32)
			&& (m_regLastDst == regBase) )
		{
			if ( mode == ECSSakura2Processor::addrBase )
			{
				offset32 = 0 ;
			}
			INT64	nOffset = (INT64) offset32 + m_immLastSrc32 ;
			if ( (nOffset >= -(INT64) 0x80000000) && (nOffset <= 0x7FFFFFFF) )
			{
				m_bufImage.ResizeBuffer( m_dwLastInstructionPos ) ;
				m_icLastInstruction = ECSSakura2Processor::codeInvalid ;
				//
				regBase = m_regLastSrc ;
				offset32 += m_immLastSrc32 ;
				if ( offset32 != 0 )
				{
					mode = ECSSakura2Processor::addrBaseOffset32 ;
				}
			}
		}
	}
	*/
	if ( regBase == ECSSakura2Processor::regBP )
	{
		//
		// ローカルメモリ命令へ変換
		//
		switch ( mode )
		{
		case	ECSSakura2Processor::addrBase:
			ESLAssert( offset32 == 0 ) ;
			return	WriteSakuraMoveLocal
				( fStore, ECSSakura2Processor::addrLocalOffset32,
						type, regDst, 0, 0, 0, fNoRegCache ) ;

		case	ECSSakura2Processor::addrBaseOffset32:
			return	WriteSakuraMoveLocal
				( fStore, ECSSakura2Processor::addrLocalOffset32,
						type, regDst, offset32, 0, 0, fNoRegCache ) ;

		case	ECSSakura2Processor::addrBaseIndex:
			ESLAssert( offset32 == 0 ) ;
			return	WriteSakuraMoveLocal
				( fStore, ECSSakura2Processor::addrLocalIndexOffset32,
						type, regDst, 0, regIndex, scaleIndex, fNoRegCache ) ;

		case	ECSSakura2Processor::addrBaseIndexOffset32:
			return	WriteSakuraMoveLocal
				( fStore, ECSSakura2Processor::addrLocalIndexOffset32,
						type, regDst, offset32, regIndex, scaleIndex, fNoRegCache ) ;

		default:
			return	ESLErrorMsg
				( "内部エラー：アドレッシング・モードが不正です" ) ;
		}
	}
	if ( fStore )
	{
		WriteSakuraInstructionCode
			( (ECSSakura2Processor::InstructionCode)
				(ECSSakura2Processor::codeStoreMem | mode) ) ;
	}
	else
	{
		WriteSakuraInstructionCode
			( (ECSSakura2Processor::InstructionCode)
				(ECSSakura2Processor::codeLoadMem | mode) ) ;
	}
	if ( (regBase < 0) || (regBase > 0x0F) )
	{
		return	ESLErrorMsg( "内部エラー：ベースレジスタが不正です" ) ;
	}
	if ( (scaleIndex < 0) || (scaleIndex > 0x03) )
	{
		return	ESLErrorMsg( "内部エラー：スケーリングファクタが不正です" ) ;
	}
	if ( (!fStore && ((regDst < 0) || (regDst > 0x7F)))
		|| (fStore && ((regDst < 0) || (regDst > 0xFF))) )
	{
		return	ESLErrorMsg( "内部エラー：レジスタ番号が不正です" ) ;
	}
	const DWORD	dwOffset = offset32 ;
	switch ( mode )
	{
	case	ECSSakura2Processor::addrBase:
		ESLAssert( offset32 == 0 ) ;
		WriteByteCode( (BYTE) (type | (regBase << 3)) ) ;
		WriteByteCode( (BYTE) regDst ) ;
		break ;

	case	ECSSakura2Processor::addrBaseOffset32:
		WriteByteCode( (BYTE) (type | (regBase << 3)) ) ;
		WriteByteCode( (BYTE) regDst ) ;
		WriteCodeData( &dwOffset, sizeof(DWORD) ) ;
		break; 

	case	ECSSakura2Processor::addrBaseIndex:
		ESLAssert( (regIndex >= 0) && (regIndex <= 0x7F) ) ;
		if ( (regIndex < 0) || (regIndex > 0x7F) )
		{
			return	ESLErrorMsg( "内部エラー：インデックスレジスタ番号が不正です" ) ;
		}
		ESLAssert( offset32 == 0 ) ;
		WriteByteCode
			( (BYTE) (type | (regBase << 3)
						| ((scaleIndex & 0x02) << 6)) ) ;
		WriteByteCode
			( (BYTE) (regIndex | ((scaleIndex & 0x01) << 7)) ) ;
		WriteByteCode( (BYTE) regDst ) ;
		break ;

	case	ECSSakura2Processor::addrBaseIndexOffset32:
		ESLAssert( (regIndex >= 0) && (regIndex <= 0x7F) ) ;
		if ( (regIndex < 0) || (regIndex > 0x7F) )
		{
			return	ESLErrorMsg( "内部エラー：インデックスレジスタ番号が不正です" ) ;
		}
		WriteByteCode
			( (BYTE) (type | (regBase << 3)
						| ((scaleIndex & 0x02) << 6)) ) ;
		WriteByteCode
			( (BYTE) (regIndex | ((scaleIndex & 0x01) << 7)) ) ;
		WriteByteCode( (BYTE) regDst ) ;
		WriteCodeData( &dwOffset, sizeof(DWORD) ) ;
		break ;

	default:
		return	ESLErrorMsg
			( "内部エラー：アドレッシング・モードが不正です" ) ;
	}
	return	eslErrSuccess ;
}

ESLError ECSExecutionImageCompiler::WriteSakuraLoadMemory
	( int& regDst, const ECSTypeInfo & typeVar,
			bool fRefType, int iOffset, bool fNoRegCache )
{
	ECSObject *	pType = typeVar.m_pValue ;
	if ( !fRefType && (pType != NULL) && (pType->m_vtType == csvtReference) )
	{
		pType = ((ECSReference*)pType)->m_pRef ;
	}
	ECSSakura2Processor::DataType
			type = DataTypeFromVariableType
						( ECSTypeInfo::GetNakedMemoryType( pType ) ) ;
	//
	if ( typeVar.IsAddressingInfo() )
	{
		ECSSakura2Processor::AddressingMode
				mode = AddressingModeFromTypeInfo( typeVar ) ;
		//
		return	WriteSakuraLoadMemory
			( mode, type, regDst,
				typeVar.m_regBase, typeVar.m_addrOffset + iOffset,
				typeVar.m_regIndex, typeVar.m_scaleIndex, fNoRegCache ) ;
	}
	if ( typeVar.IsLoadedRegister() )
	{
		return	WriteSakuraLoadMemory
			( ECSSakura2Processor::addrBaseIndex, type, regDst,
				ECSSakura2Processor::regZeroPtr, 0,
				typeVar.GetLoadedRegister(), 0, fNoRegCache ) ;
	}
	return	ESLErrorMsg( "内部エラー：アドレッシング情報がありません" ) ;
}

ESLError ECSExecutionImageCompiler::WriteSakuraStoreMemory
	( int regSrc, const ECSTypeInfo & typeVar,
		bool fRefType, int iOffset, bool fNoRegCache )
{
	ECSObject *	pType = typeVar.m_pValue ;
	if ( !fRefType && (pType != NULL) && (pType->m_vtType == csvtReference) )
	{
		pType = ((ECSReference*)pType)->m_pRef ;
	}
	ECSSakura2Processor::DataType
			type = DataTypeFromVariableType
						( ECSTypeInfo::GetNakedMemoryType( pType ) ) ;
	//
	if ( typeVar.IsAddressingInfo() )
	{
		ECSSakura2Processor::AddressingMode
				mode = AddressingModeFromTypeInfo( typeVar ) ;
		//
		return	WriteSakuraStoreMemory
			( mode, type, regSrc,
				typeVar.m_regBase, typeVar.m_addrOffset + iOffset,
				typeVar.m_regIndex, typeVar.m_scaleIndex, fNoRegCache ) ;
	}
	if ( typeVar.IsLoadedRegister() )
	{
		return	WriteSakuraStoreMemory
			( ECSSakura2Processor::addrBaseIndex, type, regSrc,
				ECSSakura2Processor::regZeroPtr, 0,
				typeVar.GetLoadedRegister(), 0, fNoRegCache ) ;
	}
	return	ESLErrorMsg( "内部エラー：アドレッシング情報がありません" ) ;
}

// ローカルメモリ命令
//////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImageCompiler::WriteSakuraMoveLocal
	( bool fStore,
		ECSSakura2Processor::LocalAddressingMode mode,
		ECSSakura2Processor::DataType type, int& regDst,
		int offset32, int regIndex, int scaleIndex, bool fNoRegCache )
{
	if ( (!fStore && ((regDst < 0) || (regDst > 0x7F)))
		|| (fStore && ((regDst < 0) || (regDst > 0xFF))) )
	{
		return	ESLErrorMsg( "内部エラー：レジスタ番号が不正です" ) ;
	}
	const DWORD	dwOffset = offset32 ;
	if ( (mode == ECSSakura2Processor::addrLocalOffset32)
		/*&& (type == ECSSakura2Processor::dataInt64)*/ && !fNoRegCache )
	{
		//
		// 既にレジスタに割り当てられている場合、レジスタ操作を行う
		//
		int	regAssigned = FindAssignedRegister( offset32, type ) ;
		if ( regAssigned >= 0 )
		{
			ESLAssert( m_regAssigns[regAssigned].flagLoaded ) ;
			if ( !fStore )
			{
				if ( m_regAssigns[regAssigned].flagUnloaded )
				{
					WriteSakuraInstructionCode
						( ECSSakura2Processor::codeLoadLocalImm32 ) ;
					WriteByteCode( (BYTE) type ) ;
					WriteByteCode( (BYTE) regAssigned ) ;
					WriteCodeData( &dwOffset, sizeof(DWORD) ) ;
					//
					m_regAssigns[regAssigned].flagUnloaded = false ;
					m_regAssigns[regAssigned].countLocked = 0 ;
					m_regAssigns[regAssigned].flagModified = false ;
				}
				if ( regAssigned != regDst )
				{
					WriteSakuraMoveRegReg( regDst, regAssigned ) ;
				}
			}
			else
			{
				if ( regAssigned != regDst )
				{
					WriteSakuraMoveRegReg( regAssigned, regDst ) ;
				}
				m_regAssigns[regAssigned].flagUnloaded = false ;
				m_regAssigns[regAssigned].countLocked = 0 ;
				m_regAssigns[regAssigned].flagModified = true ;
			}
			AccessAssignedRegister( regAssigned ) ;
			return	eslErrSuccess ;
		}
		//
		// 新規にレジスタを割り当てる
		//
		regAssigned = AssignLocalToRegister( offset32, type ) ;
		if ( regAssigned >= 0 )
		{
			ESLAssert( regAssigned >= 0 ) ;
			ESLAssert( m_regAssigns[regAssigned].flagLoaded ) ;
			//
			if ( !fStore )
			{
				WriteSakuraInstructionCode
					( ECSSakura2Processor::codeLoadLocalImm32 ) ;
				WriteByteCode( (BYTE) type ) ;
				WriteByteCode( (BYTE) regAssigned ) ;
				WriteCodeData( &dwOffset, sizeof(DWORD) ) ;
				//
				m_regAssigns[regAssigned].flagModified = false ;
				regDst = regAssigned ;
			}
			else
			{
				WriteSakuraMoveRegReg( regAssigned, regDst ) ;
				//
				m_regAssigns[regAssigned].flagModified = true ;
			}
			return	eslErrSuccess ;
		}
	}
	if ( (scaleIndex < 0) || (scaleIndex > 0x07) )
	{
		return	ESLErrorMsg( "内部エラー：スケーリングファクタが不正です" ) ;
	}
	if ( (mode == ECSSakura2Processor::addrLocalIndexOffset32)
								&& ((regIndex < 0) || (regIndex > 0x7F)) )
	{
		return	ESLErrorMsg( "内部エラー：インデックスレジスタ番号が不正です" ) ;
	}
	//
	// メモリ命令出力
	//
	if ( !fStore )
	{
		WriteSakuraInstructionCode
			( (ECSSakura2Processor::InstructionCode)
				(ECSSakura2Processor::codeLoadLocal | mode) ) ;
	}
	else
	{
		WriteSakuraInstructionCode
			( (ECSSakura2Processor::InstructionCode)
				(ECSSakura2Processor::codeStoreLocal | mode) ) ;
	}
	WriteByteCode( (BYTE) (type | (scaleIndex << 5)) ) ;
	if ( mode == ECSSakura2Processor::addrLocalIndexOffset32 )
	{
		WriteByteCode( (BYTE) regIndex ) ;
	}
	WriteByteCode( (BYTE) regDst ) ;
	WriteCodeData( &dwOffset, sizeof(DWORD) ) ;
	//
	return	eslErrSuccess ;
}

// １オペランド（reg）形式命令
//////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImageCompiler::WriteSakuraOperandReg
	( ECSSakura2Processor::InstructionCode code, int reg )
{
	if ( (reg < 0) || (reg > 0xFF) )
	{
		return	ESLErrorMsg( "内部エラー：レジスタ番号が不正です" ) ;
	}
	WriteSakuraInstructionCode( code ) ;
	WriteByteCode( (BYTE) reg ) ;
	//
	m_regLastDst = reg ;
	return	eslErrSuccess ;
}

ESLError ECSExecutionImageCompiler::WriteSakuraPushReg( int reg )
{
	return	WriteSakuraOperandReg
				( ECSSakura2Processor::codePushReg, reg ) ;
}

ESLError ECSExecutionImageCompiler::WriteSakuraPopReg( int reg )
{
	return	WriteSakuraOperandReg
				( ECSSakura2Processor::codePopReg, reg ) ;
}

// ２オペランド（reg,reg）形式命令
//////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImageCompiler::WriteSakuraOperandRegReg
	( ECSSakura2Processor::InstructionCode code, int dstreg, int srcreg )
{
	if ( (dstreg < 0) || (dstreg > 0xF0) )
	{
		return	ESLErrorMsg( "内部エラー：出力レジスタ番号が不正です" ) ;
	}
	if ( (srcreg < 0) || (srcreg > 0xFF) )
	{
		return	ESLErrorMsg( "内部エラー：入力レジスタ番号が不正です" ) ;
	}
	WriteSakuraInstructionCode( code ) ;
	WriteByteCode( (BYTE) dstreg ) ;
	WriteByteCode( (BYTE) srcreg ) ;
	//
	m_regLastDst = dstreg ;
	m_regLastSrc = srcreg ;
	return	eslErrSuccess ;
}

ESLError ECSExecutionImageCompiler::WriteSakuraSIMD64OperandRegReg
	( ECSSakura2Processor::SIMDPacked2OpInstructionCode code, int dstreg, int srcreg )
{
	if ( (dstreg < 0) || (dstreg > 0xF0) )
	{
		return	ESLErrorMsg( "内部エラー：出力レジスタ番号が不正です" ) ;
	}
	if ( (srcreg < 0) || (srcreg > 0xFF) )
	{
		return	ESLErrorMsg( "内部エラー：入力レジスタ番号が不正です" ) ;
	}
	WriteSakuraInstructionCode( ECSSakura2Processor::codeSIMD64Extension2Op ) ;
	WriteByteCode( (BYTE) code ) ;
	WriteByteCode( (BYTE) dstreg ) ;
	WriteByteCode( (BYTE) srcreg ) ;
	//
	m_icLastInstruction = ECSSakura2Processor::codeInvalid ;
	return	eslErrSuccess ;
}

ESLError ECSExecutionImageCompiler::WriteSakuraCvt2IntRegReg( int dstreg, int srcreg )
{
	return	WriteSakuraOperandRegReg
		( ECSSakura2Processor::codeCvtFloat2Int, dstreg, srcreg ) ;
}

ESLError ECSExecutionImageCompiler::WriteSakuraCvt2FloatRegReg( int dstreg, int srcreg )
{
	return	WriteSakuraOperandRegReg
		( ECSSakura2Processor::codeCvtInt2Float, dstreg, srcreg ) ;
}

ESLError ECSExecutionImageCompiler::WriteSakuraMoveRegReg
									( int dstreg, int srcreg )
{
	return	WriteSakuraOperandRegReg
		( ECSSakura2Processor::codeMoveReg, dstreg, srcreg ) ;
}

ESLError ECSExecutionImageCompiler::WriteSakuraAddRegReg( int dstreg, int srcreg )
{
	return	WriteSakuraOperandRegReg
		( ECSSakura2Processor::codeAddReg, dstreg, srcreg ) ;
}

ESLError ECSExecutionImageCompiler::WriteSakuraSubRegReg( int dstreg, int srcreg )
{
	return	WriteSakuraOperandRegReg
		( ECSSakura2Processor::codeSubReg, dstreg, srcreg ) ;
}

ESLError ECSExecutionImageCompiler::WriteSakuraAndRegReg( int dstreg, int srcreg )
{
	return	WriteSakuraOperandRegReg
		( ECSSakura2Processor::codeAndReg, dstreg, srcreg ) ;
}

ESLError ECSExecutionImageCompiler::WriteSakuraXorRegReg( int dstreg, int srcreg )
{
	return	WriteSakuraOperandRegReg
		( ECSSakura2Processor::codeXorReg, dstreg, srcreg ) ;
}

ESLError ECSExecutionImageCompiler::WriteSakuraCmpNeRegReg( int dstreg, int srcreg )
{
	return	WriteSakuraOperandRegReg
		( ECSSakura2Processor::codeCmpNeReg, dstreg, srcreg ) ;
}

ESLError ECSExecutionImageCompiler::WriteSakuraCmpEqRegReg( int dstreg, int srcreg )
{
	if ( (m_regLastDst == dstreg)
		&& (srcreg == ECSSakura2Processor::regIntZero) )
	{
		ECSSakura2Processor::InstructionCode
				icNotLogical = ECSSakura2Processor::codeInvalid ;
		switch ( m_icLastInstruction )
		{
		case	ECSSakura2Processor::codeCmpNeReg:
			icNotLogical = ECSSakura2Processor::codeCmpEqReg ;
			break ;
		case	ECSSakura2Processor::codeCmpEqReg:
			icNotLogical = ECSSakura2Processor::codeCmpNeReg ;
			break ;
		case	ECSSakura2Processor::codeCmpLtReg:
			icNotLogical = ECSSakura2Processor::codeCmpGeReg ;
			break ;
		case	ECSSakura2Processor::codeCmpGtReg:
			icNotLogical = ECSSakura2Processor::codeCmpLeReg ;
			break ;
		}
		if ( icNotLogical != ECSSakura2Processor::codeInvalid )
		{
			m_bufImage.ResizeBuffer( m_dwLastInstructionPos ) ;
			return	WriteSakuraOperandRegReg
				( icNotLogical, m_regLastDst, m_regLastSrc ) ;
		}
	}
	return	WriteSakuraOperandRegReg
		( ECSSakura2Processor::codeCmpEqReg, dstreg, srcreg ) ;
}

ESLError ECSExecutionImageCompiler::WriteSakuraCmpLtRegReg( int dstreg, int srcreg )
{
	return	WriteSakuraOperandRegReg
		( ECSSakura2Processor::codeCmpLtReg, dstreg, srcreg ) ;
}

ESLError ECSExecutionImageCompiler::WriteSakuraCmpGtRegReg( int dstreg, int srcreg )
{
	return	WriteSakuraOperandRegReg
		( ECSSakura2Processor::codeCmpGtReg, dstreg, srcreg ) ;
}

// ２オペランド（reg,imm8）形式命令
//////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImageCompiler::WriteSakuraOperandRegImm8
	( ECSSakura2Processor::InstructionCode code, int reg, int imm8 )
{
	if ( (reg < 0) || (reg > 0xF0) )
	{
		return	ESLErrorMsg( "内部エラー：レジスタ番号が不正です" ) ;
	}
	if ( (imm8 < -0x80) || (imm8 > 0x7F) )
	{
		return	ESLErrorMsg( "内部エラー：8ビット即値が不正です" ) ;
	}
	WriteSakuraInstructionCode( code ) ;
	WriteByteCode( (BYTE) reg ) ;
	WriteByteCode( (BYTE) imm8 ) ;
	//
	m_regLastDst = reg ;
	m_immLastSrc32 = imm8 ;
	return	eslErrSuccess ;
}

ESLError ECSExecutionImageCompiler::WriteSakuraPushRegsImm8( int reg, int imm8 )
{
	return	WriteSakuraOperandRegImm8
				( ECSSakura2Processor::codePushRegs, reg, imm8 ) ;
}

ESLError ECSExecutionImageCompiler::WriteSakuraPopRegsImm8( int reg, int imm8 )
{
	return	WriteSakuraOperandRegImm8
				( ECSSakura2Processor::codePopRegs, reg, imm8 ) ;
}

// ３オペランド（reg,reg,imm8）形式命令
//////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImageCompiler::WriteSakuraOperandRegRegImm8
	( ECSSakura2Processor::InstructionCode code,
						int dstreg, int srcreg, int imm8 )
{
	if ( (dstreg < 0) || (dstreg > 0xF0) )
	{
		return	ESLErrorMsg( "内部エラー：出力レジスタ番号が不正です" ) ;
	}
	if ( (srcreg < 0) || (srcreg > 0xFF) )
	{
		return	ESLErrorMsg( "内部エラー：入力レジスタ番号が不正です" ) ;
	}
	if ( (imm8 < -0x80) || (imm8 > 0x7F) )
	{
		return	ESLErrorMsg( "内部エラー：8ビット即値が不正です" ) ;
	}
	WriteSakuraInstructionCode( code ) ;
	WriteByteCode( (BYTE) dstreg ) ;
	WriteByteCode( (BYTE) srcreg ) ;
	WriteByteCode( (BYTE) imm8 ) ;
	//
	m_regLastDst = dstreg ;
	m_regLastSrc = srcreg ;
	m_immLastSrc32 = imm8 ;
	return	eslErrSuccess ;
}

ESLError ECSExecutionImageCompiler::WriteSakuraSrlRegRegImm8
	( int dstreg, int srcreg, int imm8 )
{
	if ( imm8 > 0 )
	{
		return	WriteSakuraOperandRegRegImm8
				( ECSSakura2Processor::codeSrlImm8, dstreg, srcreg, imm8 ) ;
	}
	return	eslErrSuccess ;
}

ESLError ECSExecutionImageCompiler::WriteSakuraSraRegRegImm8
	( int dstreg, int srcreg, int imm8 )
{
	if ( imm8 > 0 )
	{
		return	WriteSakuraOperandRegRegImm8
				( ECSSakura2Processor::codeSraImm8, dstreg, srcreg, imm8 ) ;
	}
	return	eslErrSuccess ;
}

ESLError ECSExecutionImageCompiler::WriteSakuraSllRegRegImm8
	( int dstreg, int srcreg, int imm8 )
{
	if ( imm8 > 0 )
	{
		return	WriteSakuraOperandRegRegImm8
				( ECSSakura2Processor::codeSllImm8, dstreg, srcreg, imm8 ) ;
	}
	return	WriteSakuraMoveRegReg( dstreg, srcreg ) ;
}


// ３オペランド（reg,reg,imm32）形式命令
//////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImageCompiler::WriteSakuraOperandRegRegImm32
	( ECSSakura2Processor::InstructionCode code,
						int dstreg, int srcreg, int imm32 )
{
	if ( (dstreg < 0) || (dstreg > 0xF0) )
	{
		return	ESLErrorMsg( "内部エラー：出力レジスタ番号が不正です" ) ;
	}
	if ( (srcreg < 0) || (srcreg > 0xFF) )
	{
		return	ESLErrorMsg( "内部エラー：入力レジスタ番号が不正です" ) ;
	}
	DWORD	dwImm32 = imm32 ;
	WriteSakuraInstructionCode( code ) ;
	WriteByteCode( (BYTE) dstreg ) ;
	WriteByteCode( (BYTE) srcreg ) ;
	WriteCodeData( &dwImm32, sizeof(DWORD) ) ;
	//
	m_regLastDst = dstreg ;
	m_regLastSrc = srcreg ;
	m_immLastSrc32 = imm32 ;
	return	eslErrSuccess ;
}

// ３オペランド（reg,reg,reg）形式命令
//////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImageCompiler::WriteSakuraMaskMoveRegRegReg
				( int dstreg, int srcreg, int maskreg )
{
	if ( (dstreg < 0) || (dstreg > 0xFF) )
	{
		return	ESLErrorMsg( "内部エラー：出力レジスタ番号が不正です" ) ;
	}
	if ( (srcreg < 0) || (srcreg > 0xFF) )
	{
		return	ESLErrorMsg( "内部エラー：入力レジスタ番号が不正です" ) ;
	}
	if ( (maskreg < 0) || (maskreg > 0xFF) )
	{
		return	ESLErrorMsg( "内部エラー：入力レジスタ番号が不正です" ) ;
	}
	WriteSakuraInstructionCode( ECSSakura2Processor::codeMaskMove ) ;
	WriteByteCode( (BYTE) dstreg ) ;
	WriteByteCode( (BYTE) srcreg ) ;
	WriteByteCode( (BYTE) maskreg ) ;
	//
	return	eslErrSuccess ;
}

// 整数即値演算命令
//////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImageCompiler::WriteSakuraAddRegRegImm32
	( int dstreg, int srcreg, int imm32 )
{
	if ( (dstreg == srcreg)
		&& (m_icLastInstruction == ECSSakura2Processor::codeMoveReg)
		&& (m_regLastDst == srcreg) )
	{
		m_bufImage.ResizeBuffer( m_dwLastInstructionPos ) ;
		srcreg = m_regLastSrc ;
	}
	if ( imm32 == 0 )
	{
		return	WriteSakuraMoveRegReg( dstreg, srcreg ) ;
	}
	return	WriteSakuraOperandRegRegImm32
		( ECSSakura2Processor::codeAddImm32, dstreg, srcreg, imm32 ) ;
}

ESLError ECSExecutionImageCompiler::WriteSakuraMulRegRegImm32
	( int dstreg, int srcreg, int imm32 )
{
	if ( (dstreg == srcreg)
		&& (m_icLastInstruction == ECSSakura2Processor::codeMoveReg)
		&& (m_regLastDst == srcreg) )
	{
		m_bufImage.ResizeBuffer( m_dwLastInstructionPos ) ;
		srcreg = m_regLastSrc ;
	}
	if ( imm32 >= 0 )
	{
		int	scale = -1, mask = 1 ;
		for ( int i = 0; i < 31; i ++, mask <<= 1 )
		{
			if ( imm32 == mask )
			{
				scale = i ;
				break ;
			}
		}
		if ( scale >= 0 )
		{
			return	WriteSakuraSllRegRegImm8( dstreg, srcreg, scale ) ;
		}
	}
	return	WriteSakuraOperandRegRegImm32
		( ECSSakura2Processor::codeMulImm32, dstreg, srcreg, imm32 ) ;
}

// スタックレジスタ加算命令
//////////////////////////////////////////////////////////////////////////
DWORD ECSExecutionImageCompiler::WriteSakuraAddSP( int imm32 )
{
	if ( m_icLastInstruction == ECSSakura2Processor::codeAddSPImm32 )
	{
		DWORD *	pdwImm32 =
			(DWORD*) m_bufImage.ModifyBuffer
					( m_dwLastInstructionPos + 1, sizeof(DWORD) ) ;
		*pdwImm32 += imm32 ;
		m_immLastSrc32 = *pdwImm32 ;
		return	m_dwLastInstructionPos + 1 ;
	}
	DWORD	dwImm32 = imm32 ;
	WriteSakuraInstructionCode( ECSSakura2Processor::codeAddSPImm32 ) ;
	DWORD	dwImmediateAddress = m_bufImage.GetLength() ;
	WriteCodeData( &dwImm32, sizeof(DWORD) ) ;
	//
	m_immLastSrc32 = imm32 ;
	return	dwImmediateAddress ;
}

// 即値ロード命令
//////////////////////////////////////////////////////////////////////////
DWORD ECSExecutionImageCompiler::WriteSakuraLoadInt64( int reg, INT64 imm64 )
{
	WriteSakuraInstructionCode( ECSSakura2Processor::codeLoadImm64 ) ;
	WriteByteCode( (BYTE) reg ) ;
	//
	DWORD	dwImmediateAddress = m_bufImage.GetLength() ;
	WriteCodeData( &imm64, sizeof(INT64) ) ;
	//
	m_flagLastImmediateReal = false ;
	return	dwImmediateAddress ;
}

DWORD ECSExecutionImageCompiler::WriteSakuraLoadReal64( int reg, REAL64 imm64 )
{
	WriteSakuraInstructionCode( ECSSakura2Processor::codeLoadImm64 ) ;
	WriteByteCode( (BYTE) reg ) ;
	//
	DWORD	dwImmAddr = m_bufImage.GetLength() ;
	WriteCodeData( &imm64, sizeof(REAL64) ) ;
	//
	m_flagLastImmediateReal = true ;
	return	dwImmAddr ;
}

DWORD ECSExecutionImageCompiler::WriteSakuraLoadInt64_FuncPtr
	( int reg, const wchar_t * pwszFuncName )
{
	DWORD	dwRefAddr =
		WriteSakuraLoadInt64
			( reg, ((INT64) ECSExecutionImage::roasCode << 56) ) ;
	AddCodeRefFunctionAddress64( pwszFuncName, dwRefAddr ) ;
	return	dwRefAddr ;
}

DWORD ECSExecutionImageCompiler::WriteSakuraLoadInt64_ClassID
	( int reg, const wchar_t * pwszClassName )
{
	int	nClassIndex = GetClassInfoIndex( pwszClassName ) ;
	ESLAssert( nClassIndex >= 0 ) ;
	//
	DWORD	dwIDAddr = WriteSakuraLoadInt64( reg, nClassIndex ) ;
	//
	m_extClassIndexRef.Add( dwIDAddr ) ;
	return	dwIDAddr ;
}

DWORD ECSExecutionImageCompiler::WriteSakuraLoadInt64_CStrPtr
							( int reg, const wchar_t * pwszString )
{
	DWORD	dwCodeAddr =
		WriteSakuraLoadInt64
			( reg, AllocateNakedConstString( pwszString ) ) ;
	m_extNakedConstRef.Add( dwCodeAddr ) ;
	return	dwCodeAddr ;
}

DWORD ECSExecutionImageCompiler::WriteSakuraLoadInt64_VarAddr( int reg, INT64 nAddr )
{
	DWORD	dwCodeAddr = WriteSakuraLoadInt64( reg, nAddr ) ;
	//
	switch ( (int) (nAddr >> 56) & 0xFF )
	{
	case	roasCode:
		m_extCodeRef.Add( dwCodeAddr ) ;
		break ;
	case	roasNakedGlobal:
		m_extNakedGlobalRef.Add( dwCodeAddr ) ;
		break ;
	case	roasNakedConst:
		m_extNakedConstRef.Add( dwCodeAddr ) ;
		break ;
	case	roasNakedShared:
		m_extNakedSharedRef.Add( dwCodeAddr ) ;
		break ;
	}
	return	dwCodeAddr ;
}

DWORD ECSExecutionImageCompiler::WriteSakuraLoadInt64_GlobalVarAddr
	( int reg, const wchar_t * pwszString )
{
	DWORD	dwCodeAddr = WriteSakuraLoadInt64( reg, 0 ) ;
	//
	ENumArray<DWORD> *	pList = m_impNakedGlobalRef.GetAs( pwszString ) ;
	if ( pList == NULL )
	{
		pList = new ENumArray<DWORD> ;
		m_impNakedGlobalRef.Add( pwszString, pList ) ;
	}
	pList->Add( dwCodeAddr ) ;
	//
	return	dwCodeAddr ;
}

DWORD ECSExecutionImageCompiler::WriteSakuraLoadInt64_SharedVarAddr
	( int reg, const wchar_t * pwszString )
{
	DWORD	dwCodeAddr = WriteSakuraLoadInt64( reg, 0 ) ;
	//
	ENumArray<DWORD> *	pList = m_impNakedSharedRef.GetAs( pwszString ) ;
	if ( pList == NULL )
	{
		pList = new ENumArray<DWORD> ;
		m_impNakedSharedRef.Add( pwszString, pList ) ;
	}
	pList->Add( dwCodeAddr ) ;
	//
	return	dwCodeAddr ;
}

DWORD ECSExecutionImageCompiler::WriteSakuraLoadInt64_ConstVarAddr
	( int reg, const wchar_t * pwszString )
{
	DWORD	dwCodeAddr = WriteSakuraLoadInt64( reg, 0 ) ;
	//
	ENumArray<DWORD> *	pList = m_impNakedConstRef.GetAs( pwszString ) ;
	if ( pList == NULL )
	{
		pList = new ENumArray<DWORD> ;
		m_impNakedConstRef.Add( pwszString, pList ) ;
	}
	pList->Add( dwCodeAddr ) ;
	//
	return	dwCodeAddr ;
}

// 相対ジャンプ命令出力
//////////////////////////////////////////////////////////////////////////
DWORD ECSExecutionImageCompiler::WriteSakuraJumpOffset32( int imm32 )
{
	FlushAllRegisterAssigns() ;
	ResetAllRegisterAssigns() ;
	//
	DWORD	dwImm32 = imm32 ;
	DWORD	dwImmAddr ;
	WriteSakuraInstructionCode( ECSSakura2Processor::codeJumpOffset32 ) ;
	//
	dwImmAddr = m_bufImage.GetLength() ;
	WriteCodeData( &dwImm32, sizeof(DWORD) ) ;
	//
	m_icLastInstruction = ECSSakura2Processor::codeInvalid ;
	return	dwImmAddr ;
}

DWORD ECSExecutionImageCompiler::WriteSakuraCNJumpOffset32( int reg, int imm32 )
{
//	FlushAllRegisterAssigns() ;
	//
	DWORD	dwImm32 = imm32 ;
	DWORD	dwImmAddr ;
	WriteSakuraInstructionCode( ECSSakura2Processor::codeCNJumpOffset32 ) ;
	WriteByteCode( (BYTE) reg ) ;
	//
	dwImmAddr = m_bufImage.GetLength() ;
	WriteCodeData( &dwImm32, sizeof(DWORD) ) ;
	//
	m_icLastInstruction = ECSSakura2Processor::codeInvalid ;
	return	dwImmAddr ;
}

DWORD ECSExecutionImageCompiler::WriteSakuraCJumpOffset32( int reg, int imm32 )
{
//	FlushAllRegisterAssigns() ;
	//
	DWORD	dwImm32 = imm32 ;
	DWORD	dwImmAddr ;
	WriteSakuraInstructionCode( ECSSakura2Processor::codeCJumpOffset32 ) ;
	WriteByteCode( (BYTE) reg ) ;
	//
	dwImmAddr = m_bufImage.GetLength() ;
	WriteCodeData( &dwImm32, sizeof(DWORD) ) ;
	//
	m_icLastInstruction = ECSSakura2Processor::codeInvalid ;
	return	dwImmAddr ;
}

// 間接ジャンプ命令出力
//////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImageCompiler::WriteSakuraJumpReg( int reg )
{
	FlushAllRegisterAssigns() ;
	ResetAllRegisterAssigns() ;
	//
	WriteSakuraInstructionCode( ECSSakura2Processor::codeJumpReg ) ;
	WriteByteCode( (BYTE) reg ) ;
	//
	m_icLastInstruction = ECSSakura2Processor::codeInvalid ;
	//
	if ( (reg < 0) || (reg > 0x7F) )
	{
		return	ESLErrorMsg( "内部エラー：レジスタ番号が不正です" ) ;
	}
	return	eslErrSuccess ;
}

// コール命令出力
//////////////////////////////////////////////////////////////////////////
DWORD ECSExecutionImageCompiler::WriteSakuraCallImm32( int imm32 )
{
	FlushAllRegisterAssigns() ;
	ResetAllRegisterAssigns() ;
	//
	DWORD	dwImm32 = imm32 ;
	DWORD	dwImmAddr ;
	WriteSakuraInstructionCode( ECSSakura2Processor::codeCallImm32 ) ;
	//
	dwImmAddr = m_bufImage.GetLength() ;
	WriteCodeData( &dwImm32, sizeof(DWORD) ) ;
	//
	m_icLastInstruction = ECSSakura2Processor::codeInvalid ;
	return	dwImmAddr ;
}

DWORD ECSExecutionImageCompiler::WriteSakuraCallFunction
				( const wchar_t * pwszFuncName )
{
	FlushAllRegisterAssigns() ;
	ResetAllRegisterAssigns() ;
	//
	WriteSakuraInstructionCode( ECSSakura2Processor::codeCallImm32 ) ;
	//
	DWORD	dwImmAddr = m_bufImage.GetLength() ;
	WriteFunctionAddress( pwszFuncName ) ;
	//
	m_icLastInstruction = ECSSakura2Processor::codeInvalid ;
	return	dwImmAddr ;
}

// 間接コール命令出力
//////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImageCompiler::WriteSakuraCallReg( int reg )
{
	FlushAllRegisterAssigns() ;
	ResetAllRegisterAssigns() ;
	//
	WriteSakuraInstructionCode( ECSSakura2Processor::codeCallReg ) ;
	WriteByteCode( (BYTE) reg ) ;
	//
	m_icLastInstruction = ECSSakura2Processor::codeInvalid ;
	//
	if ( (reg < 0) || (reg > 0x7F) )
	{
		return	ESLErrorMsg( "内部エラー：レジスタ番号が不正です" ) ;
	}
	return	eslErrSuccess ;
}

// システムコール命令出力
//////////////////////////////////////////////////////////////////////////
DWORD ECSExecutionImageCompiler::WriteSakuraSysCallImm32( int imm32 )
{
	FlushAllRegisterAssigns() ;
	//
	DWORD	dwImm32 = imm32 ;
	DWORD	dwImmAddr ;
	WriteSakuraInstructionCode( ECSSakura2Processor::codeSysCallImm32 ) ;
	//
	dwImmAddr = m_bufImage.GetLength() ;
	WriteCodeData( &dwImm32, sizeof(DWORD) ) ;
	//
	m_icLastInstruction = ECSSakura2Processor::codeInvalid ;
	return	dwImmAddr ;
}

DWORD ECSExecutionImageCompiler::WriteSakuraSysCallFunction
				( const wchar_t * pwszFuncName )
{
	FlushAllRegisterAssigns() ;
	//
	DWORD	dwImmAddr ;
	WriteSakuraInstructionCode( ECSSakura2Processor::codeSysCallImm32 ) ;
	//
	dwImmAddr = m_bufImage.GetLength() ;
	WriteNakedNativeFunctionIndex( pwszFuncName ) ;
	//
	m_icLastInstruction = ECSSakura2Processor::codeInvalid ;
	return	dwImmAddr ;
}

void ECSExecutionImageCompiler::WriteSakuraSysCallIndirect( int reg )
{
	FlushAllRegisterAssigns() ;
	//
	WriteSakuraInstructionCode( ECSSakura2Processor::codeSysCallReg ) ;
	WriteByteCode( (BYTE) reg ) ;
	//
	m_icLastInstruction = ECSSakura2Processor::codeInvalid ;
}

// リターン命令出力
//////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::WriteSakuraReturn( void )
{
	FlushAllRegisterAssigns() ;
	ResetAllRegisterAssigns() ;
	//
	WriteSakuraInstructionCode( ECSSakura2Processor::codeReturn ) ;
	//
	m_icLastInstruction = ECSSakura2Processor::codeInvalid ;
}

// naked 無名不変文字配列データ登録
//////////////////////////////////////////////////////////////////////////
INT64 ECSExecutionImageCompiler::AllocateNakedConstString
			( const wchar_t * pwszString )
{
	if ( m_bufNakedConst.GetLength() & 0x01 )
	{
		BYTE	bytDummy = 0 ;
		m_bufNakedConst.MergeBuffer( &bytDummy, sizeof(BYTE) ) ;
	}
	INT64	nAddress = m_bufNakedConst.GetLength()
							| ((INT64)roasNakedConst << 56) ;
	//
	int	i = 0 ;
	if ( pwszString != NULL )
	{
		for ( ; pwszString[i] != 0; i ++ )
		{
		}
		m_bufNakedConst.MergeBuffer
			( pwszString, (i + 1) * sizeof(WORD) ) ;
	}
	else
	{
		WORD	wZero = 0 ;
		m_bufNakedConst.MergeBuffer( &wZero, sizeof(WORD) ) ;
	}
	return	nAddress ;
}

// naked データシンボル追加
//////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::AddNakedSymbolInfo
	( const wchar_t * pwszSymbol, INT64 nAddress )
{
	NAKED_SYMBOL_INFO *	pSymbol = new NAKED_SYMBOL_INFO ;
	pSymbol->nAddress = nAddress ;
	m_wstaSymbols.SetAs( pwszSymbol, pSymbol ) ;
}

// naked データ外部参照追加
//////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::AddCodeRefNakedGlobalAddress
	( const wchar_t * pwszSymbol, DWORD dwCodeAddr )
{
	ENumArray<DWORD> *	pList = m_impNakedGlobalRef.GetAs( pwszSymbol ) ;
	if ( pList == NULL )
	{
		pList = new ENumArray<DWORD> ;
		m_impNakedGlobalRef.Add( pwszSymbol, pList ) ;
	}
	pList->Add( dwCodeAddr ) ;
}

void ECSExecutionImageCompiler::AddCodeRefNakedConstAddress
	( const wchar_t * pwszSymbol, DWORD dwCodeAddr )
{
	ENumArray<DWORD> *	pList = m_impNakedConstRef.GetAs( pwszSymbol ) ;
	if ( pList == NULL )
	{
		pList = new ENumArray<DWORD> ;
		m_impNakedConstRef.Add( pwszSymbol, pList ) ;
	}
	pList->Add( dwCodeAddr ) ;
}

void ECSExecutionImageCompiler::AddCodeRefNakedSharedAddress
	( const wchar_t * pwszSymbol, DWORD dwCodeAddr )
{
	ENumArray<DWORD> *	pList = m_impNakedSharedRef.GetAs( pwszSymbol ) ;
	if ( pList == NULL )
	{
		pList = new ENumArray<DWORD> ;
		m_impNakedSharedRef.Add( pwszSymbol, pList ) ;
	}
	pList->Add( dwCodeAddr ) ;
}

// ネイティブ関数インデックス参照追加
//////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::AddCodeRefNakedSystemCallID( DWORD dwCodeAddr )
{
	m_impNakedNativeFunc.Add( dwCodeAddr ) ;
}

// クラスインデックス参照追加
//////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::AddCodeRefClassID( DWORD dwCodeAddr )
{
	m_extClassIndexRef.Add( dwCodeAddr ) ;
}


//////////////////////////////////////////////////////////////////////////
// 詞葉 3.0 Sakura2 仮想マシン用コード最適化
//////////////////////////////////////////////////////////////////////////

// 最適化領域開始
//////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::BeginSakura2Optimize( void )
{
	m_dwOptimizeStart = m_bufImage.GetLength() ;
}

// 最適化領域開始
//////////////////////////////////////////////////////////////////////////
void ECSExecutionImageCompiler::FinishSakura2Optimize
		( int regTempFirst, int regTempEnd )
{
	m_dwOptimizeEnd = m_bufImage.GetLength() ;
	//
	ECSExecutionOptimizer	optimizer( this ) ;
	optimizer.BeginOptimize
		( m_dwOptimizeStart, m_dwOptimizeEnd, regTempFirst, regTempEnd ) ;
	while ( optimizer.PerformOptimize() > 0 )
	{
	}
	optimizer.FinishOptimize() ;
	//
	m_dwOptimizeStart = 0 ;
	m_dwOptimizeEnd = 0 ;
}

