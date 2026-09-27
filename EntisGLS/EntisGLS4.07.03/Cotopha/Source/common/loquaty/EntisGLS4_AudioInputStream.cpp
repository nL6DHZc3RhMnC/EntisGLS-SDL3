
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_AudioInputStream.h>



// const EntisGLS4.SoundFormat* getAudioFormat( )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioInputStream_getAudioFormat)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioInputStream, pThis ) ;
	SGLAudioInputStream *	pAudio = pThis->GetRef<SGLAudioInputStream>() ;
	LQT_VERIFY_NULL_PTR( pAudio ) ;

	LEntisGLS4_SoundFormat	valRet ;
	pAudio->GetAudioFormat( valRet ) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// EntisGLS4.MediaOptionalInfo getAudioOptinalInfo( )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioInputStream_getAudioOptinalInfo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioInputStream, pThis ) ;
	SGLAudioInputStream *	pAudio = pThis->GetRef<SGLAudioInputStream>() ;
	LQT_VERIFY_NULL_PTR( pAudio ) ;

	SGLMediaOptionalInfo	info ;
	if ( pAudio->GetAudioOptinalInfo( info ) != sglErrSuccess )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LClass *	pInfoClass =
		_context.VM().GetClassPathAs( L"EntisGLS4.MediaOptionalInfo" ) ;
	if ( pInfoClass == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}

	LObjPtr	valRet( pInfoClass->CreateInstance() ) ;
	valRet->SetElementLongAs( L"flags", info.m_nFlags ) ;
	valRet->SetElementLongAs( L"loopStart", info.m_nLoopStart ) ;
	valRet->SetElementLongAs( L"loopEnd", info.m_nLoopEnd ) ;
	valRet->SetElementStringAs( L"title", info.m_strTitle ) ;
	valRet->SetElementStringAs( L"player", info.m_strPlayer ) ;
	valRet->SetElementStringAs( L"composer", info.m_strComposer ) ;
	valRet->SetElementStringAs( L"arranger", info.m_strArranger ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// long getAudioLength( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_AudioInputStream_getAudioLength)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioInputStream, pThis ) ;
	SGLAudioInputStream *	pAudio = pThis->GetRef<SGLAudioInputStream>() ;
	LQT_VERIFY_NULL_PTR( pAudio ) ;

	LQT_RETURN_LONG( pAudio->GetAudioLength() ) ;
}

// ulong readAudio( void* buf, ulong samples )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioInputStream_readAudio)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioInputStream, pThis ) ;
	SGLAudioInputStream *	pAudio = pThis->GetRef<SGLAudioInputStream>() ;
	LQT_VERIFY_NULL_PTR( pAudio ) ;
	LEntisGLS4_SoundFormat	fmt ;
	pAudio->GetAudioFormat( fmt ) ;
	LQT_FUNC_ARG_POINTER_N( uint8_t, buf, fmt.SamplesToBytes(LQT_ARG_LONG(2)) ) ;
	LQT_VERIFY_NULL_PTR( buf ) ;
	LQT_FUNC_ARG_ULONG( samples ) ;

	LQT_RETURN_ULONG( pAudio->ReadAudio( buf, (size_t) samples ) ) ;
}

// boolean seekAudio( ulong samples )
IMPL_LOQUATY_FUNC(EntisGLS4_AudioInputStream_seekAudio)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_AudioInputStream, pThis ) ;
	SGLAudioInputStream *	pAudio = pThis->GetRef<SGLAudioInputStream>() ;
	LQT_VERIFY_NULL_PTR( pAudio ) ;
	LQT_FUNC_ARG_ULONG( samples ) ;

	LQT_RETURN_BOOL( pAudio->SeekAudio( samples ) == sglErrSuccess ) ;
}



