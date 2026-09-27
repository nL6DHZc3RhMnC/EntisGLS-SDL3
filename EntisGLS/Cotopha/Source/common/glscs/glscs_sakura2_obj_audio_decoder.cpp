
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <sakuragl/sgl_media.h>
#include <glscs/glscs_sakura2_obj_audio_decoder.h>

using	namespace SSystem ;
using	namespace SakuraGL ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// AudioDecoder オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO_CAST( ECSSakura2::AudioDecoderObject, ECSVolatileObject, m_decoder )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
AudioDecoderObject::AudioDecoderObject
	( const wchar_t * pwszType,
		SakuraGL::SGLAudioDecoderInterface * decoder, bool flagOwner )
{
	m_pwszType = pwszType ;
	m_decoder = decoder ;
	m_flagOwner = flagOwner ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
AudioDecoderObject::~AudioDecoderObject( void )
{
	if ( m_flagOwner )
	{
		delete	m_decoder ;
	}
	m_flagOwner = false ;
	m_decoder = NULL ;
}

// デコーダー関連付け
//////////////////////////////////////////////////////////////////////////////
void AudioDecoderObject::AttachAudioDecoder
	( SakuraGL::SGLAudioDecoderInterface * decoder, bool flagOwner )
{
	if ( m_flagOwner )
	{
		delete	m_decoder ;
	}
	m_decoder = decoder ;
	m_flagOwner = flagOwner ;
}

// メモリマッピング
//////////////////////////////////////////////////////////////////////////////
LinearAddressCache *
	AudioDecoderObject::GetSegmentBuffer( LinearAddressCache & seg )
{
	seg.baseOffset = 0 ;
	seg.limitSegment = (DWORD) m_bufOptionalInf.GetLength() ;
	seg.pbytBuffer = (BYTE*) m_bufOptionalInf.GetConstArray() ;
	return	&seg ;
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * AudioDecoderObject::GetTypeName( void ) const
{
	return	m_pwszType ;
}

// オプショナルフォーマット構築
//////////////////////////////////////////////////////////////////////////////
void AudioDecoderObject::MakeOptionalInfo
	( AudioDecoderObject::OPTIONAL_INFO& optDst,
		const SGLAudioDecoderInterface::OptionalInfo& optSrc )
{
	m_bufOptionalInf.SetLength( 0 ) ;
	//
	optDst.nFlags = optSrc.nFlags ;
	optDst.nLoopStart = optSrc.nLoopStart ;
	optDst.nLoopEnd = optSrc.nLoopEnd ;
	//
	if ( (optSrc.pszTitle != NULL)
		&& (optDst.nFlags & SGLAudioDecoderInterface::flagTitle) )
	{
		optDst.pszTitle = AddOptionalInfoString( optSrc.pszTitle ) ;
	}
	if ( (optSrc.pszVocalPlayer != NULL)
		&& (optDst.nFlags & SGLAudioDecoderInterface::flagVocalPlayer) )
	{
		optDst.pszVocalPlayer = AddOptionalInfoString( optSrc.pszVocalPlayer ) ;
	}
	if ( (optSrc.pszComposer != NULL)
		&& (optDst.nFlags & SGLAudioDecoderInterface::flagComposer) )
	{
		optDst.pszComposer = AddOptionalInfoString( optSrc.pszComposer ) ;
	}
	if ( (optSrc.pszArranger != NULL)
		&& (optDst.nFlags & SGLAudioDecoderInterface::flagArranger) )
	{
		optDst.pszArranger = AddOptionalInfoString( optSrc.pszArranger ) ;
	}
}

uint64_t AudioDecoderObject::AddOptionalInfoString( const uint16_t * pszInfo )
{
	uint64_t	addrInfo =
		(((uint64_t) m_dwHighAddr) << 32) | m_bufOptionalInf.GetLength() ;
	//
	size_t	nLength = 0 ;
	while ( pszInfo[nLength] != 0 )
	{
		nLength ++ ;
	}
	m_bufOptionalInf.AddArray
		( (const uint8_t*) pszInfo, nLength * sizeof(uint16_t) ) ;
	//
	return	addrInfo ;
}


//////////////////////////////////////////////////////////////////////////////
// AudioDecoder スタブ
//////////////////////////////////////////////////////////////////////////////

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SakuraGL::AudioDecoder
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT(SakuraGL_AudioDecoder, context, cls_id)
{
	return	new AudioDecoderObject( L"SakuraGL::AudioDecoder", NULL, false ) ;
}

// SGLError AudioDecoder::Open( const wchar_t * pwszFilePath ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioDecoder_Open, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, AudioDecoderObject, pDecoder, arg, AudioDecoder::Open ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, const uint16_t, pszFilePath,
				arg[1].i, pszFilePath at AudioDecoder::Open ) ;
	//
	context->m_regset[regAcc].i = sglErrFailed ;
	//
	SString	strFilePath = pszFilePath ;
	SFileInterface *	file =
		vm->NewOpenFile( strFilePath, SFileOpener::shareRead ) ;
	if ( file == NULL )
	{
		return	NULL ;
	}
	SGLAudioDecoderInterface *
		decoder = SGLAudioDecoderManager::CreateDecoder( file, true ) ;
	if ( decoder == NULL )
	{
		delete	file ;
		return	NULL ;
	}
	pDecoder->AttachAudioDecoder( decoder, true ) ;
	context->m_regset[regAcc].i = sglErrSuccess ;
	return	NULL ;
}

// SGLError AudioDecoder::Create( SSystem::File * pFile ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioDecoder_Create, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, AudioDecoderObject, pDecoder, arg, AudioDecoder::Create ) ;
	ECS_DECLARE_SYSCALL_OBJECT
		( vm, SFileInterface, pFile, arg[1].i, pFile of AudioDecoder::Create ) ;
	//
	SGLAudioDecoderInterface *
		decoder = SGLAudioDecoderManager::CreateDecoder( pFile, false ) ;
	if ( decoder == NULL )
	{
		context->m_regset[regAcc].i = sglErrFailed ;
		return	NULL ;
	}
	pDecoder->AttachAudioDecoder( decoder, true ) ;
	context->m_regset[regAcc].i = sglErrSuccess ;
	return	NULL ;
}

// SGLError AudioDecoder::Close( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioDecoder_Close, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, AudioDecoderObject, pDecoder, arg, AudioDecoder::Close ) ;
	//
	pDecoder->AttachAudioDecoder( NULL, false ) ;
	//
	context->m_regset[regAcc].i = sglErrSuccess ;
	return	NULL ;
}

// SGLError AudioDecoder::GetFormat( SGLSoundFormat & fmt ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioDecoder_GetFormat, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAudioDecoderInterface,
				pDecoder, arg, AudioDecoder::GetFormat ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SGLSoundFormat, pFormat,
					arg[1].i, AudioDecoder::GetFormat ) ;
	//
	context->m_regset[regAcc].i = pDecoder->GetFormat( *pFormat ) ;
	//
	return	NULL ;
}

// SGLError AudioDecoder::GetOptinalInfo( OptionalInfo & optinf ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioDecoder_GetOptinalInfo, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, AudioDecoderObject,
				pDecoder, arg, AudioDecoder::GetOptinalInfo ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, AudioDecoderObject::OPTIONAL_INFO, pOptInfo,
							arg[1].i, AudioDecoder::GetOptinalInfo ) ;
	//
	SGLAudioDecoderInterface::OptionalInfo	optinf ;
	SGLAudioDecoderInterface *
		decoder = ESLTypeCast<SGLAudioDecoderInterface>( pDecoder ) ;
	if ( decoder != NULL )
	{
		context->m_regset[regAcc].i = decoder->GetOptinalInfo( optinf ) ;
		pDecoder->MakeOptionalInfo( *pOptInfo, optinf ) ;
	}
	else
	{
		context->m_regset[regAcc].i = sglErrFailed ;
	}
	return	NULL ;
}

// uint64_t AudioDecoder::GetTotalLength( void ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioDecoder_GetTotalLength, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAudioDecoderInterface,
				pDecoder, arg, AudioDecoder::GetTotalLength ) ;
	//
	context->m_regset[regAcc].i = pDecoder->GetTotalLength() ;
	//
	return	NULL ;
}

// SGLError AudioDecoder::SeekPosition( uint64_t nPos ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioDecoder_SeekPosition, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAudioDecoderInterface,
				pDecoder, arg, AudioDecoder::SeekPosition ) ;
	//
	context->m_regset[regAcc].i = pDecoder->SeekPosition( arg[1].i ) ;
	//
	return	NULL ;
}

// size_t AudioDecoder::DecodeNext( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioDecoder_DecodeNext, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAudioDecoderInterface,
				pDecoder, arg, AudioDecoder::DecodeNext ) ;
	//
	context->m_regset[regAcc].i = pDecoder->DecodeNext() ;
	//
	return	NULL ;
}

// size_t AudioDecoder::ReadDecodedBuffer
//		( void * ptrPCM, size_t nBytes, size_t nOffset = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioDecoder_ReadDecodedBuffer, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAudioDecoderInterface,
				pDecoder, arg, AudioDecoder::ReadDecodedBuffer ) ;
	ECS_DECLARE_SYSCALL_ARRAYVAR
		( context, uint8_t, ptrPCM, arg[1].i,
					arg[2].i, AudioDecoder::ReadDecodedBuffer ) ;
	//
	context->m_regset[regAcc].i =
		pDecoder->ReadDecodedBuffer
			( ptrPCM, (size_t) arg[2].i, (size_t) arg[3].i ) ;
	//
	return	NULL ;
}

#endif
