
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_queue_buffer.h>
#include <sakura/ssys_module.h>
#include <glscs/glscs_sakura2_jit_x86_compiler.h>
#include <glscs/glscs_sakura2_jit_sse2_compiler.h>
#include <glscs/glscs_sakura2_jit_arm_compiler.h>

using	namespace SSystem ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2JIT ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// 詞葉モジュール
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSSakura2::ExecutableModule, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ExecutableModule::ExecutableModule( void )
{
	m_iModule = -1 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ExecutableModule::~ExecutableModule( void )
{
}

// コード・リアロケーション
//////////////////////////////////////////////////////////////////////////////
void ExecutableModule::ReallocateModule
	( int iModule, NewObjectVector& vectorClass,
					SystemCallVector& vectorSysCall )
{
	//
	// コード・データ参照アドレス・リアロケーション
	//
	m_iModule = iModule ;
	ReallocateRefAddress( m_reallcRefCode ) ;
	ReallocateRefAddress( m_reallcRefGlobal ) ;
	ReallocateRefAddress( m_reallcRefConst ) ;
	ReallocateRefAddress( m_reallcRefShared ) ;
	//
	// クラス ID リアロケーション
	//
	ReallocateRefIdentity
		( vectorClass.GetEntryIndex(),
			m_indexClass, m_reallcRefClassId ) ;
	//
	// システムコール ID リアロケーション
	//
	ReallocateRefIdentity
		( vectorSysCall.GetEntryIndex(),
			m_indexSysCall, m_reallcRefSysCallId ) ;
	//
	// シンボル情報アドレス・リアロケーション
	//
	const size_t	nSymCount = m_symbolData.GetLength() ;
	for ( size_t i = 0; i < nSymCount; i ++ )
	{
		SYMBOL_INFO *	pSymInf = m_symbolData.GetAt( i ) ;
		if ( pSymInf != NULL )
		{
			pSymInf->nAddress =
				(pSymInf->nAddress & 0xFF000000FFFFFFFF)
					| (((INT64)((DWORD) iModule & 0x00FFFFFF)) << 32) ;
		}
	}
}

// 参照アドレス・リアロケーション
//////////////////////////////////////////////////////////////////////////////
void ExecutableModule::ReallocateRefAddress
	( const ExecutableModule::ReallocationArray& reallcRef )
{
	BYTE *			pbytCode = m_bufCode.GetBuffer() ;
	const size_t	nCount = reallcRef.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		DWORD	dwRefAddr = reallcRef.At(i) ;
		ESLAssert( dwRefAddr + sizeof(INT64) <= m_bufCode.GetLength() ) ;
		DWORD *	pdwHighAddr =
			(DWORD*) (pbytCode + dwRefAddr + sizeof(DWORD)) ;
		*pdwHighAddr =
			(*pdwHighAddr & 0xFF000000)
				| ((DWORD) m_iModule & 0x00FFFFFF) ;
	}
}

// ID リアロケーション
//////////////////////////////////////////////////////////////////////////////
void ExecutableModule::ReallocateRefIdentity
	( SIndexedArray
		<SString,const wchar_t*>& indexDstId,
			const ExecutableModule::StringIndexedArray& indexSrcId,
			const ExecutableModule::ReallocationArray& reallcRef )
{
	//
	// ID ベクタの結合とリアロケーションテーブル生成
	//
	SArray<DWORD>	remapID ;
	const size_t	nSrcCount = indexSrcId.GetLength() ;
	size_t			i ;
	remapID.SetLength( nSrcCount ) ;
	for ( i = 0; i < nSrcCount; i ++ )
	{
		SString *	pstrID = indexSrcId.GetAt( i ) ;
		if ( pstrID != NULL )
		{
			ssize_t	idDst = indexDstId.FindIndex( *pstrID ) ;
			if ( idDst < 0 )
			{
				idDst = (ssize_t) indexDstId.Add( new SString( *pstrID ) ) ;
			}
			remapID.SetAt( i, (DWORD) idDst ) ;
		}
	}
	//
	// 参照 ID のリアロケーション
	//
	BYTE *			pbytCode = m_bufCode.GetBuffer() ;
	const size_t	nRefCount = reallcRef.GetLength() ;
	for ( i = 0; i < nRefCount; i ++ )
	{
		DWORD	dwRefAddr = reallcRef.At(i) ;
		ESLAssert( dwRefAddr + sizeof(DWORD) <= m_bufCode.GetLength() ) ;
		DWORD *	pdwRefID = (DWORD*) (pbytCode + dwRefAddr) ;
		ESLAssert( *pdwRefID <= remapID.GetLength() ) ;
		if ( *pdwRefID <= remapID.GetLength() )
		{
			*pdwRefID = remapID.At(*pdwRefID);
		}
	}
}

// シンボル・インポート
//////////////////////////////////////////////////////////////////////////////
int ExecutableModule::ImportSymbols
	( const SPointerArray<ExecutableModule> & modules )
{
	int	nUnsolved = 0 ;
	nUnsolved += ImportFunctionSymbols( modules, m_importRefCode ) ;
	nUnsolved += ImportDataSymbols( modules, m_importRefGlobal ) ;
	nUnsolved += ImportDataSymbols( modules, m_importRefConst ) ;
	nUnsolved += ImportDataSymbols( modules, m_importRefShared ) ;
	return	nUnsolved ;
}

// 関数アドレス・インポート解決
//////////////////////////////////////////////////////////////////////////////
int ExecutableModule::ImportFunctionSymbols
	( const SPointerArray<ExecutableModule> & modules,
			const ExecutableModule::TaggedImportArray& importRefCode )
{
	BYTE *			pbytCode = m_bufCode.GetBuffer() ;
	const size_t	nModuleCount = modules.GetLength() ;
	const size_t	nImpCount = importRefCode.GetLength() ;
	int				nUnsolved = 0 ;
	for ( size_t iImp = 0; iImp < nImpCount; iImp ++ )
	{
		//
		// 関数検索
		//
		const SString *	pstrTag = importRefCode.GetTagAt( iImp ) ;
		ESLAssert( pstrTag != NULL ) ;
		if ( pstrTag == NULL )
		{
			continue ;
		}
		FUNC_ENTRY *	pFunc = NULL ;
		size_t	iModule ;
		for ( iModule = 0; iModule < nModuleCount; iModule ++ )
		{
			ExecutableModule *	pModule = modules.GetAt( iModule ) ;
			if ( pModule == NULL )
			{
				continue ;
			}
			pFunc = pModule->m_symbolCode.GetAs( *pstrTag ) ;
			if ( pFunc != NULL )
			{
				break ;
			}
		}
		if ( pFunc == NULL )
		{
			nUnsolved ++ ;
			continue ;
		}
		//
		// 解決
		//
		ReallocationArray *	preallcRef = importRefCode.GetAt( iImp ) ;
		ESLAssert( preallcRef != NULL ) ;
		if ( preallcRef == NULL )
		{
			continue ;
		}
		const size_t	nRefCount = preallcRef->GetLength() ;
		for ( size_t iRef = 0; iRef < nRefCount; iRef ++ )
		{
			DWORD	dwRefAddr = preallcRef->At(iRef) ;
			ESLAssert( dwRefAddr + sizeof(INT64) <= m_bufCode.GetLength() ) ;
			DWORD *	pdwFuncAddr = (DWORD*) (pbytCode + dwRefAddr) ;
			pdwFuncAddr[0] = pFunc->dwAddress ;
			pdwFuncAddr[1] = (VirtualMachine::roasCode << 24)
								| ((DWORD) iModule & 0x00FFFFFF) ;
		}
	}
	return	nUnsolved ;
}

// 変数アドレス・インポート解決
//////////////////////////////////////////////////////////////////////////////
int ExecutableModule::ImportDataSymbols
	( const SPointerArray<ExecutableModule> & modules,
			const ExecutableModule::TaggedImportArray& importRefData )
{
	BYTE *			pbytCode = m_bufCode.GetBuffer() ;
	const size_t	nModuleCount = modules.GetLength() ;
	const size_t	nImpCount = importRefData.GetLength() ;
	int				nUnsolved = 0 ;
	for ( size_t iImp = 0; iImp < nImpCount; iImp ++ )
	{
		//
		// シンボル検索
		//
		const SString *	pstrTag = importRefData.GetTagAt( iImp ) ;
		ESLAssert( pstrTag != NULL ) ;
		if ( pstrTag == NULL )
		{
			continue ;
		}
		SYMBOL_INFO *	pSymbol = NULL ;
		for ( size_t iModule = 0; iModule < nModuleCount; iModule ++ )
		{
			ExecutableModule *	pModule = modules.GetAt( iModule ) ;
			if ( pModule == NULL )
			{
				continue ;
			}
			pSymbol = pModule->m_symbolData.GetAs( *pstrTag ) ;
			if ( pSymbol != NULL )
			{
				break ;
			}
		}
		if ( pSymbol == NULL )
		{
			nUnsolved ++ ;
			continue ;
		}
		//
		// 解決
		//
		ReallocationArray *	preallcRef = importRefData.GetAt( iImp ) ;
		ESLAssert( preallcRef != NULL ) ;
		if ( preallcRef == NULL )
		{
			continue ;
		}
		const size_t	nRefCount = preallcRef->GetLength() ;
		for ( size_t iRef = 0; iRef < nRefCount; iRef ++ )
		{
			DWORD	dwRefAddr = preallcRef->At(iRef) ;
			ESLAssert( dwRefAddr + sizeof(INT64) <= m_bufCode.GetLength() ) ;
			INT64 *	pqwDataAddr = (INT64*) (pbytCode + dwRefAddr) ;
			*pqwDataAddr = pSymbol->nAddress ;
		}
	}
	return	nUnsolved ;
}

// リソースを削除する
//////////////////////////////////////////////////////////////////////////////
void ExecutableModule::DeleteModule( void )
{
	m_bufCode.FreeBuffer() ;
	m_bufGlobal.FreeBuffer() ;
	m_bufConst.FreeBuffer() ;
	m_bufShared.FreeBuffer() ;
	//
	m_vectorPrologue.FreeArray() ;
	m_vectorEpilogue.FreeArray() ;
	//
	m_indexClass.FreeArray() ;
	m_indexSysCall.FreeArray() ;
	m_symbolCode.FreeArray() ;
	m_symbolData.FreeArray() ;
	//
	m_reallcRefCode.FreeArray() ;
	m_reallcRefGlobal.FreeArray() ;
	m_reallcRefConst.FreeArray() ;
	m_reallcRefShared.FreeArray() ;
	m_reallcRefClassId.FreeArray() ;
	m_reallcRefSysCallId.FreeArray() ;
	//
	m_importRefCode.FreeArray() ;
	m_importRefGlobal.FreeArray() ;
	m_importRefConst.FreeArray() ;
	m_importRefShared.FreeArray() ;
	//
	m_bufNativeCodes = NULL ;
	m_bufNativeGates = NULL ;
}

// モジュールファイルを読み込む
//////////////////////////////////////////////////////////////////////////////
SError ExecutableModule::ReadModule( SFileInterface * pFile )
{
	//
	// 以前のモジュール情報を消去
	//
	DeleteModule() ;
	//
	// ヘッダを読み込む
	//
	SChunkFile	cfModule ;
	SError	err ;
	err = cfModule.OpenChunkFile( pFile ) ;
	if ( err != errSuccess )
	{
		return	errFailed ;
	}
	for ( ; ; )
	{
		if ( cfModule.DescendChunk() != errSuccess )
		{
			break ;
		}
		if ( cfModule.IsEqualCurrentChunkID( "header  " ) )
		{
			//
			// ヘッダ
			//
			eslFillMemory( &m_exmHeader, 0, sizeof(HEADER) ) ;
			m_exmHeader.nStackSize = 0x1000 ;
			m_exmHeader.nHeapSize = 0x1000 ;
			m_exmHeader.fnEntryPoint = -1 ;
			m_exmHeader.fnStaticInitialize = -1 ;
			m_exmHeader.fnResumePrepare = -1 ;
			//
			cfModule.Read( &m_exmHeader, sizeof(HEADER) ) ;
		}
		else if ( cfModule.IsEqualCurrentChunkID( "image   " ) )
		{
			//
			// コードイメージ
			//
			DWORD	dwLength = (DWORD) cfModule.GetLength( ) ;
			m_bufCode.CreateBuffer( dwLength ) ;
			cfModule.Read( m_bufCode.GetBuffer(), dwLength ) ;
		}
		else if ( cfModule.IsEqualCurrentChunkID( "imgglobl" ) )
		{
			//
			// グローバル領域イメージ
			//
			DWORD	dwLength = (DWORD) cfModule.GetLength( ) ;
			m_bufGlobal.CreateBuffer( dwLength ) ;
			cfModule.Read( m_bufGlobal.GetBuffer(), dwLength ) ;
		}
		else if ( cfModule.IsEqualCurrentChunkID( "imgconst" ) )
		{
			//
			// 不変グローバル領域イメージ
			//
			DWORD	dwLength = (DWORD) cfModule.GetLength( ) ;
			m_bufConst.CreateBuffer( dwLength ) ;
			cfModule.Read( m_bufConst.GetBuffer(), dwLength ) ;
		}
		else if ( cfModule.IsEqualCurrentChunkID( "imgshare" ) )
		{
			//
			// 共有グローバル領域イメージ
			//
			DWORD	dwLength = (DWORD) cfModule.GetLength( ) ;
			m_bufShared.CreateBuffer( dwLength ) ;
			cfModule.Read( m_bufShared.GetBuffer(), dwLength ) ;
		}
		else if ( cfModule.IsEqualCurrentChunkID( "classinf" ) )
		{
			//
			// クラス情報（クラス名のみ）
			//
			err = ReadWideStringArray( cfModule, m_indexClass ) ;
			if ( err )
			{
				return	err ;
			}
			// ※実行時型情報は読み込まない
		}
		else if ( cfModule.IsEqualCurrentChunkID( "initnfnc" ) )
		{
			//
			// naked 初期化関数リスト
			//
			err = ReadDWordArray( cfModule, m_vectorPrologue ) ;
			if ( err )
			{
				return	err ;
			}
			err = ReadDWordArray( cfModule, m_vectorEpilogue ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else if ( cfModule.IsEqualCurrentChunkID( "funcinfo" ) )
		{
			//
			// 関数情報リスト
			//
			DWORD	dwFuncCount ;
			if ( cfModule.Read
				( &dwFuncCount, sizeof(DWORD) ) < sizeof(DWORD) )
			{
				return	errFailed ;
			}
			SString		strFuncName ;
			FUNC_ENTRY	fnEntry ;
			for ( DWORD i = 0; i < dwFuncCount; i ++ )
			{
				if ( cfModule.Read
					( (FUNC_ENTRY_HEADER*) &fnEntry,
						sizeof(FUNC_ENTRY_HEADER) ) < sizeof(FUNC_ENTRY_HEADER) )
				{
					return	errFailed ;
				}
				err = ReadWideString( cfModule, strFuncName ) ;
				if ( err )
				{
					return	err ;
				}
				if ( fnEntry.dwReserved != 0 )
				{
					SQueueBuffer	qbuf ;
					qbuf.ReadFromStream( cfModule, fnEntry.dwReserved ) ;
					//
					while ( qbuf.GetLength() != 0 )
					{
						DWORD	dwID, dwBytes ;
						if ( qbuf.Read( &dwID, sizeof(DWORD) ) < sizeof(DWORD) )
						{
							break ;
						}
						if ( qbuf.Read( &dwBytes, sizeof(DWORD) ) < sizeof(DWORD) )
						{
							break ;
						}
						FUNC_ENTRY_EXTENDED *	pfex = new FUNC_ENTRY_EXTENDED ;
						fnEntry.listExtended.Add( pfex ) ;
						//
						pfex->dwID = dwID ;
						pfex->dwBytes = dwBytes ;
						pfex->bufData.SetLength( dwBytes ) ;
						qbuf.Read( pfex->bufData.GetArray(), dwBytes ) ;
						pfex->bufData.FinishArray() ;
					}
				}
				m_symbolCode.SetAs( strFuncName, fnEntry ) ;
			}
		}
		else if ( cfModule.IsEqualCurrentChunkID( "symblinf" ) )
		{
			//
			// naked シンボル情報リスト
			//
			DWORD	dwSymCount ;
			if ( cfModule.Read
				( &dwSymCount, sizeof(DWORD) ) < sizeof(DWORD) )
			{
				return	errFailed ;
			}
			SString		strSymbol ;
			SYMBOL_INFO	infSymbol ;
			for ( DWORD i = 0; i < dwSymCount; i ++ )
			{
				if ( cfModule.Read
					( &infSymbol, sizeof(SYMBOL_INFO) ) < sizeof(SYMBOL_INFO) )
				{
					return	errFailed ;
				}
				err = ReadWideString( cfModule, strSymbol ) ;
				if ( err )
				{
					return	err ;
				}
				m_symbolData.SetAs( strSymbol, infSymbol ) ;
			}
		}
		else if ( cfModule.IsEqualCurrentChunkID( "linkex64" ) )
		{
			//
			// リアロケーション情報
			//
			DWORD	dwFlags = 0;
			cfModule.Read( &dwFlags, sizeof(DWORD) ) ;
			//
			err = ReadDWordArray( cfModule, m_reallcRefGlobal ) ;
			if ( err )
			{
				return	err ;
			}
			err = ReadDWordArray( cfModule, m_reallcRefConst ) ;
			if ( err )
			{
				return	err ;
			}
			err = ReadDWordArray( cfModule, m_reallcRefShared ) ;
			if ( err )
			{
				return	err ;
			}
			if ( dwFlags & 0x08 )
			{
				err = ReadDWordArray( cfModule, m_reallcRefCode ) ;
				if ( err )
				{
					return	err ;
				}
			}
			//
			// インポート情報
			//
			err = ReadTaggedDWordArray( cfModule, m_importRefGlobal ) ;
			if ( err )
			{
				return	err ;
			}
			err = ReadTaggedDWordArray( cfModule, m_importRefConst ) ;
			if ( err )
			{
				return	err ;
			}
			err = ReadTaggedDWordArray( cfModule, m_importRefShared ) ;
			if ( err )
			{
				return	err ;
			}
			if ( dwFlags & 0x08 )
			{
				err = ReadTaggedDWordArray( cfModule, m_importRefCode ) ;
				if ( err )
				{
					return	err ;
				}
			}
		}
		else if ( cfModule.IsEqualCurrentChunkID( "refclass" ) )
		{
			//
			// クラス情報参照リスト
			//
			err = ReadDWordArray( cfModule, m_reallcRefClassId ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else if ( cfModule.IsEqualCurrentChunkID( "impnativ" ) )
		{
			for ( ; ; )
			{
				if ( cfModule.DescendChunk() != errSuccess )
				{
					break ;
				}
				if ( cfModule.IsEqualCurrentChunkID( "nakedfnc" ) )
				{
					//
					// ネイティブ関数参照
					//
					err = ReadWideStringArray( cfModule, m_indexSysCall ) ;
					if ( err )
					{
						return	err ;
					}
					err = ReadDWordArray( cfModule, m_reallcRefSysCallId ) ;
					if ( err )
					{
						return	err ;
					}
				}
				cfModule.AscendChunk() ;
			}
		}
		cfModule.AscendChunk() ;
	}
	cfModule.Close() ;
	return	errSuccess ;
}

// ネイティブコード化
//////////////////////////////////////////////////////////////////////////////
void ExecutableModule::CompileToNativeCode
		( bool fNoBoundary, uint64_t maskCpuFeatures )
{
	//
	// アセンブラ生成
	//
	SSmartPointer<Sakura2Assembler>	pAsmCodes ;
	SSmartPointer<Sakura2Assembler>	pAsmGates ;
	//
	CPU_Family	cpuFamily = GetCPUFamily() ;
	uint64_t	cpuFeatures = GetCPUFeatures() & maskCpuFeatures ;
#if	defined(__PROCESSOR_INTEL_X86__)
	if ( cpuFamily == cpuFamily_X86 )
	{
		Trace( "JIT compiling for x86 family CPU\n" ) ;
		//
		X86GenericAssembler *	px86AsmCodes ;
		X86GenericAssembler *	px86AsmGates ;
		if ( cpuFeatures & cpuX86_Feature_SSE2 )
		{
			Trace( "enabled SSE2 instruction set\n" ) ;
			px86AsmCodes = new X86SSE2Assembler ;
			px86AsmGates = new X86SSE2Assembler ;
		}
		else
		{
			px86AsmCodes = new X86GenericAssembler ;
			px86AsmGates = new X86GenericAssembler ;
		}
		#if	defined(_MSC_VER)
			// MS-C++ fastcall
			px86AsmCodes->SetCallingABI( X86GenericAssembler::fastcallMSstyle ) ;
			px86AsmGates->SetCallingABI( X86GenericAssembler::fastcallMSstyle ) ;
		#else
			// C call (standard, no fastcall)
			px86AsmCodes->SetCallingABI( X86GenericAssembler::fastcallCstyle ) ;
			px86AsmGates->SetCallingABI( X86GenericAssembler::fastcallCstyle ) ;
		#endif
		pAsmCodes = px86AsmCodes ;
		pAsmGates = px86AsmGates ;
		m_bufNativeCodes = new X86CodeBuffer ;
		m_bufNativeGates = new X86CodeBuffer ;
	}
	else
	{
		return ;
	}
#elif	defined(__PROCESSOR_ARM__)
	if ( cpuFamily == cpuFamily_ARM )
	{
		Trace( "JIT compiling for ARM family CPU\n" ) ;
		//
		ARMGenericAssembler *	parmAsmCodes = new ARMGenericAssembler ;
		ARMGenericAssembler *	parmAsmGates = new ARMGenericAssembler ;
		ARMCodeBuffer *			pbufCodes = new ARMCodeBuffer ;
		ARMCodeBuffer *			pbufGates = new ARMCodeBuffer ;
		//
		int		armVersion = 5, vfpVersion = 0 ;
		bool	vfpNEON = false, modeThumb = false ;
		if ( cpuFeatures & cpuARM_Feature_ARMv7 )
		{
			Trace( "enabled ARMv7 instruction set\n" ) ;
			armVersion = 7 ;
		}
		if ( cpuFeatures & cpuARM_Feature_VFPv3 )
		{
			Trace( "enabled VFPv3 instruction set\n" ) ;
			vfpVersion = 3 ;
		}
		if ( cpuFeatures & cpuARM_Feature_NEON )
		{
			Trace( "enabled NEON instruction set\n" ) ;
			vfpNEON = true ;
		}
		parmAsmCodes->SelectARMInstruction
				( armVersion, vfpVersion, vfpNEON, modeThumb ) ;
		parmAsmGates->SelectARMInstruction
				( armVersion, vfpVersion, vfpNEON, modeThumb ) ;
		pbufCodes->SelectARMInstruction( armVersion, modeThumb ) ;
		pbufGates->SelectARMInstruction( armVersion, modeThumb ) ;
		//
		pAsmCodes = parmAsmCodes ;
		pAsmGates = parmAsmGates ;
		m_bufNativeCodes = pbufCodes ;
		m_bufNativeGates = pbufGates ;
	}
	else
	{
		return ;
	}
#else
	return ;
#endif
#if	defined(__PROCESSOR_INTEL_X86__) || defined(__PROCESSOR_ARM__)
	pAsmCodes->AttachCodeBuffer( m_bufNativeCodes, m_bufNativeGates ) ;
	pAsmGates->AttachCodeBuffer( m_bufNativeGates ) ;
	pAsmCodes->SetNoBoundaryWithAddressTranslation( fNoBoundary ) ;
	pAsmGates->SetNoBoundaryWithAddressTranslation( fNoBoundary ) ;
	//
	// シャドウバッファ生成
	//
	BYTE *	pCodeBuf ;
	BYTE *	pTrickBuf ;
	BYTE *	pCodeShadow ;
	m_bufCode.CreateShadowBuffer() ;
	pCodeBuf = m_bufCode.GetBuffer() ;
	pTrickBuf = m_bufCode.GetSegmentShadowBuffer(0) ;
	pCodeShadow = m_bufCode.GetSegmentShadowBuffer(1) ;
	if ( pCodeBuf && pCodeShadow )
	{
		eslMoveMemory( pCodeShadow, pCodeBuf, m_bufCode.GetLength() ) ;
	}
	//
	// 実行時コンパイル
	//
	ECSSakura2JIT::NativeCompiler	ncompiler ;
	ncompiler.AttachCodeAssembler( pAsmCodes, pAsmGates ) ;
	//
	Trace( "start JIT compiling..." ) ;
	STimeCounter	timer ;
	const size_t	countFunc = m_symbolCode.GetLength() ;
	for ( size_t i = 0; i < countFunc; i ++ )
	{
		FUNC_ENTRY *	pFunc = m_symbolCode.GetAt( i ) ;
		if ( (pFunc == NULL)
			|| (pFunc->dwBytes == (DWORD) -1)
			|| !(pFunc->dwFlags & flagNakedCall) )
		{
			continue ;
		}
		ncompiler.AttachFunction
			( pCodeBuf + pFunc->dwAddress,
				pTrickBuf + pFunc->dwAddress,
				pFunc->dwAddress, pFunc->dwBytes ) ;
		if ( ncompiler.PreprocessFunction() )
		{
			ncompiler.CompileFunction() ;
		}
	}
	pAsmCodes->CommitAllCodes() ;
	pAsmGates->CommitAllCodes() ;
	Trace( "finished %d [ms]\n", (int) timer.GetTime() ) ;
#endif
}

// JIT コンパイラ機能評価
//////////////////////////////////////////////////////////////////////////////
uint32_t ExecutableModule::GetJITCompilerFeatures( void )
{
	SSystem::CPU_Family	cpuFamily = SSystem::GetCPUFamily() ;
	uint64_t			cpuFeatures = SSystem::GetCPUFeatures() ;
	uint32_t			jitFeatures = 0 ;
	if ( cpuFamily == cpuFamily_X86 )
	{
		jitFeatures |= SSystem::jitFeature_Compiler
						| SSystem::jitFeature_Float ;
		if ( cpuFeatures & cpuX86_Feature_SSE2 )
		{
			jitFeatures |= SSystem::jitFeature_Saturation
							| SSystem::jitFeature_SIMD64
							| SSystem::jitFeature_SIMD128 ;
		}
	}
	else if ( cpuFamily == cpuFamily_ARM )
	{
		jitFeatures |= SSystem::jitFeature_Compiler ;
		if ( cpuFeatures & cpuARM_Feature_ARMv7 )
		{
			jitFeatures |= SSystem::jitFeature_Saturation
							| SSystem::jitFeature_SIMD64 ;
		}
		if ( cpuFeatures & cpuARM_Feature_VFPv3 )
		{
			jitFeatures |= SSystem::jitFeature_Float ;
		}
		if ( cpuFeatures & cpuARM_Feature_NEON )
		{
			jitFeatures |= SSystem::jitFeature_Saturation
							| SSystem::jitFeature_SIMD64
							| SSystem::jitFeature_SIMD128 ;
		}
	}
	return	jitFeatures ;
}

// DWORD 配列を読み込む
//////////////////////////////////////////////////////////////////////////////
SError ExecutableModule::ReadDWordArray
	( SFileInterface & file, SSystem::SArray<DWORD> & arrayDWords )
{
	DWORD	dwCount ;
	if ( file.Read( &dwCount, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	errFailed ;
	}
	if ( dwCount != 0 )
	{
		const size_t	nArrayBytes = dwCount * sizeof(DWORD) ;
		arrayDWords.SetLength( dwCount ) ;
		if ( file.Read
			( arrayDWords.GetArray(), nArrayBytes ) < nArrayBytes )
		{
			arrayDWords.FinishArray() ;
			return	errFailed ;
		}
		arrayDWords.FinishArray() ;
	}
	return	errSuccess ;
}

// 文字列を読み込む
//////////////////////////////////////////////////////////////////////////////
SError ExecutableModule::ReadWideString
	( SFileInterface & file, SSystem::SString & strSymbol )
{
	DWORD	dwLength ;
	if ( file.Read( &dwLength, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	errFailed ;
	}
	const size_t	nStrBytes = dwLength * sizeof(uint16_t) ;
	uint16_t *	pwStr = strSymbol.LockBuffer( dwLength ) ;
	if ( file.Read( pwStr, nStrBytes ) < nStrBytes )
	{
		return	errFailed ;
	}
	strSymbol.UnlockBuffer( dwLength ) ;
	return	errSuccess ;
}

// 文字列配列を読み込む
//////////////////////////////////////////////////////////////////////////////
SError ExecutableModule::ReadWideStringArray
	( SFileInterface & file, StringIndexedArray & arrayStrings )
{
	DWORD	dwCount ;
	if ( file.Read( &dwCount, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	errFailed ;
	}
	for ( DWORD i = 0; i < dwCount; i ++ )
	{
		SString *	pStr = new SString ;
		SError	err = ReadWideString( file, *pStr ) ;
		if ( err )
		{
			delete	pStr ;
			return	err ;
		}
		arrayStrings.Add( pStr ) ;
	}
	return	errSuccess ;
}

// インポート参照配列を読み込む
//////////////////////////////////////////////////////////////////////////////
SError ExecutableModule::ReadTaggedDWordArray
	( SFileInterface & file, TaggedImportArray & importRefs )
{
	DWORD	dwCount ;
	if ( file.Read( &dwCount, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	errFailed ;
	}
	for ( DWORD i = 0; i < dwCount; i ++ )
	{
		SString	strSymbol ;
		SError	err = ReadWideString( file, strSymbol ) ;
		if ( err )
		{
			return	err ;
		}
		ReallocationArray *	preallcDWords = new ReallocationArray ;
		err = ReadDWordArray( file, *preallcDWords ) ;
		if ( err )
		{
			delete	preallcDWords ;
			return	err ;
		}
		importRefs.SetAs( strSymbol, preallcDWords ) ;
	}
	return	errSuccess ;
}

// アドレスを含む関数を検索
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ExecutableModule::SearchFunctionAtAddress
	( DWORD dwAddress, ExecutableModule::FUNC_ENTRY** ppFuncEntry )
{
	const size_t	countFunc = m_symbolCode.GetLength() ;
	if ( ppFuncEntry != NULL )
	{
		*ppFuncEntry = NULL ;
	}
	for ( size_t i = 0; i < countFunc; i ++ )
	{
		FUNC_ENTRY *	pFunc = m_symbolCode.GetAt( i ) ;
		if ( (pFunc != NULL)
			&& !(pFunc->dwBytes & 0x80000000)
			&& (pFunc->dwAddress <= dwAddress)
			&& (dwAddress - pFunc->dwAddress < pFunc->dwBytes) )
		{
			const SString *	pstrName = m_symbolCode.GetTagAt(i) ;
			ESLAssert( pstrName != NULL ) ;
			if ( pstrName != NULL )
			{
				if ( ppFuncEntry != NULL )
				{
					*ppFuncEntry = pFunc ;
				}
				return	*pstrName ;
			}
		}
	}
	return	NULL ;
}

// コード逆アセンブル・デバッグ出力
//////////////////////////////////////////////////////////////////////////////
void ExecutableModule::DebugTraceDisassemble
				( DWORD dwAddress, DWORD dwBytes ) const
{
#if	defined(__DEBUG__)
	BYTE *	pbytCode = m_bufCode.GetCodeShadowBuffer() ;
	for ( DWORD i = 0; i < dwBytes;  )
	{
		ECSSakura2Processor::MnemonicInfo	minf ;
		memset( &minf, 0, sizeof(ECSSakura2Processor::MnemonicInfo) ) ;
		minf.nFlags |= ECSSakura2Processor::flagMnemonic ;
		//
		ECSSakura2Processor::GetInstructionInfo
					( &minf, pbytCode + (dwAddress + i) ) ;
		if ( pbytCode[dwAddress + i] == codeSysCallImm32 )
		{
			SString *	pstrSysCall =
				m_indexSysCall.GetAt
					( *((DWORD*)(pbytCode + (dwAddress + i + 1))) ) ;
			if ( pstrSysCall != NULL )
			{
				Trace( "%08X:  %s  %s (%s)\n",
					(dwAddress + i),
					&minf.szMnemonic[0], &minf.szOperand[0],
					pstrSysCall->ToCharArray().GetConstArray() ) ;
			}
			else
			{
				Trace( "%08X:  %s  %s\n",
					(dwAddress + i), &minf.szMnemonic[0], &minf.szOperand[0] ) ;
			}
		}
		else if ( pbytCode[dwAddress + i] == codeJumpOffset32 )
		{
			Trace( "%08X:  %s  %s (%08X)\n",
				(dwAddress + i),
				&minf.szMnemonic[0], &minf.szOperand[0],
				(dwAddress + i + minf.nBytes)
					+ *((SDWORD*)(pbytCode + (dwAddress + i + 1))) ) ;
		}
		else if ( (pbytCode[dwAddress + i] == codeCNJumpOffset32)
				|| (pbytCode[dwAddress + i] == codeCJumpOffset32) )
		{
			Trace( "%08X:  %s  %s (%08X)\n",
				(dwAddress + i),
				&minf.szMnemonic[0], &minf.szOperand[0],
				(dwAddress + i + minf.nBytes)
					+ *((SDWORD*)(pbytCode + (dwAddress + i + 2))) ) ;
		}
		else
		{
			Trace( "%08X:  %s  %s\n",
				(dwAddress + i), &minf.szMnemonic[0], &minf.szOperand[0] ) ;
		}
		i += minf.nBytes ;
	}
	Trace( "\n" ) ;
#endif
}

