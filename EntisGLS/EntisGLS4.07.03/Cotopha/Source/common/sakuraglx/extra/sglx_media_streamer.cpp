
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <sakuracl/erisa/sgl_erisa_md5_context.h>
#include <sakuraglx/extra/sglx_media_streamer.h>
#include <sakuraglx/sprite/sglx_sprite_cursor.h>

using namespace SSystem ;
using namespace SakuraGL ;
using namespace SakuraCL ;


//////////////////////////////////////////////////////////////////////////////
// ストリーミング・サーバー
//////////////////////////////////////////////////////////////////////////////

// DCT ジグザグ走査
//////////////////////////////////////////////////////////////////////////////
const size_t	SGLMediaStreamingServer::m_indexZigzagTable[64] =
{
	 0,  1,  5,  6, 14, 15, 27, 28,
	 2,  4,  7, 13, 16, 26, 29, 42,
	 3,  8, 12, 17, 25, 30, 41, 43,
	 9, 11, 18, 24, 31, 40, 44, 53,
	10, 19, 23, 32, 39, 45, 52, 54,
	20, 22, 33, 38, 46, 51, 55, 60,
	21, 34, 37, 47, 50, 56, 59, 61,
	35, 36, 48, 49, 57, 58, 62, 63
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLMediaStreamingServer, ESLObject, SProcedure )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLMediaStreamingServer::SGLMediaStreamingServer( void )
{
	m_flagListening = false ;
	//
	m_nLimitConnection = 1 ;
	m_msecSoundKeyInterval = 4000 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLMediaStreamingServer::~SGLMediaStreamingServer( void )
{
}

// セットアップ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaStreamingServer::SetupServer
			( uint32_t nTCPPort, uint32_t nUDPPort )
{
	if ( m_flagListening )
	{
		return	sglErrFailed ;
	}
	m_nListenTCPPort = nTCPPort ;
	m_nListenUDPPort = nUDPPort ;
	m_eventExit.Initialize( false ) ;
	m_procUDP = new UDPProcedure( this ) ;
	if ( m_threadUDP.BeginThread( m_procUDP ) )
	{
		m_procUDP = NULL ;
		m_threadUDP.Delete() ;
		m_eventExit.Delete() ;
		return	sglErrFailed ;
	}
	if ( m_threadListen.BeginThread( this ) )
	{
		m_eventExit.SetSignal() ;
		m_threadUDP.Wait() ;
		m_procUDP = NULL ;
		m_threadUDP.Delete() ;
		m_eventExit.Delete() ;
		return	sglErrFailed ;
	}
	m_flagListening = true ;
	return	sglErrSuccess ;
}

// 終了
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaStreamingServer::Shutdown( void )
{
	if ( !m_flagListening )
	{
		return	sglErrFailed ;
	}
	m_eventExit.SetSignal() ;
	{
		SSyncSocket	socket ;
		socket.Create() ;
		if ( socket.Connect
			( L"localhost", m_nListenTCPPort ) == errSuccess )
		{
			socket.WriteEncodedString
				( L"shutdown\n", -1, Charset::encodingUTF8 ) ;
			socket.Close() ;
		}
	}
	m_threadListen.Wait() ;
	m_threadListen.Delete() ;
	m_threadUDP.Wait() ;
	m_threadUDP.Delete() ;
	m_eventExit.Delete() ;
	m_flagListening = false ;
	//
#if	defined(__PLATFORM_WINDOWS__)
	m_csSync.Lock() ;
	for ( size_t i = 0; i < m_lstInstance.GetLength(); i ++ )
	{
		Instance *	pInstance = m_lstInstance.GetAt( i ) ;
		if ( pInstance != NULL )
		{
			pInstance->AsyncAbort() ;
		}
	}
	m_csSync.Unlock() ;
	//
	while ( m_lstInstance.GetLength() != 0 )
	{
		SleepMilliSec( 10 ) ;
	}
#endif
	return	sglErrSuccess ;
}

// 最大接続数設定
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::SetMaxConnection( size_t nLimitConnection )
{
	m_nLimitConnection = nLimitConnection ;
}

// パスワード設定
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::SetPassword( const wchar_t * pwszPassword )
{
	m_strLoginPass = pwszPassword ;
}

// 録音デバイス設定
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::SetSoundRecorderDevice( const wchar_t * pwszDevName )
{
	m_strSoundRecDevice = pwszDevName ;
}

#if	defined(__PLATFORM_WINDOWS__)

// インスタンス追加
//////////////////////////////////////////////////////////////////////////////
bool SGLMediaStreamingServer::AddInstance
		( SGLMediaStreamingServer::Instance * pInstance )
{
	bool	fConnect = false ;
	m_csSync.Lock() ;
	if ( m_lstInstance.GetLength() < m_nLimitConnection )
	{
		m_lstInstance.Add( pInstance ) ;
		fConnect = true ;
	}
	m_csSync.Unlock() ;
	return	fConnect ;
}

// インスタンス削除
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::RemoveInstance
		( SGLMediaStreamingServer::Instance * pInstance )
{
	m_csSync.Lock() ;
	ssize_t	i = m_lstInstance.FindPtr( pInstance ) ;
	if ( i >= 0 )
	{
		m_lstInstance.DetachAt( (size_t) i ) ;
		pInstance->ReleaseRef() ;
	}
	m_csSync.Unlock() ;
}

#endif

// 拒否接続元か？
//////////////////////////////////////////////////////////////////////////////
bool SGLMediaStreamingServer::IsDenyClient( const wchar_t * pwszClientAddr ) const
{
	bool	fDeny = false ;
	m_csSync.Lock() ;
	//
	DenyConnection *	pdc = m_ssaDenyClient.GetAs( pwszClientAddr ) ;
	if ( pdc != NULL )
	{
		fDeny = (pdc->nFailedLogin >= 5) ;
	}
	//
	m_csSync.Unlock() ;
	return	fDeny ;
}

// ログイン結果反映
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::ClientLoginResult
	( const wchar_t * pwszClientAddr, bool fSuccessed )
{
	m_csSync.Lock() ;
	if ( fSuccessed )
	{
		m_ssaDenyClient.RemoveAs( pwszClientAddr ) ;
	}
	else
	{
		DenyConnection *	pdc = m_ssaDenyClient.GetAs( pwszClientAddr ) ;
		if ( pdc == NULL )
		{
			DenyConnection	dc ;
			dc.nFailedLogin = 0 ;
			m_ssaDenyClient.Add( pwszClientAddr, dc ) ;
			//
			pdc = m_ssaDenyClient.GetAs( pwszClientAddr ) ;
		}
		if ( pdc != NULL )
		{
			pdc->nFailedLogin ++ ;
		}
	}
	m_csSync.Unlock() ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::Run( void )
{
	if ( m_socketListen.Create( m_nListenTCPPort ) != errSuccess )
	{
		ESLTrace( "failed to create TCP/IP soket (SGLMediaStreamingServer).\n" ) ;
		return ;
	}
	for ( ; ; )
	{
		if ( m_socketListen.Listen() == errSuccess )
		{
			#if	defined(__PLATFORM_WINDOWS__)
			Instance *	pInstance = new Instance( this ) ;
			if ( pInstance->Start( m_socketListen ) )
			{
				pInstance->ReleaseRef() ;
			}
			#endif
		}
		if ( m_eventExit.Wait( 10 ) == errSuccess )
		{
			break ;
		}
	}
}

// UDP 受信データ処理
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::DispatchUDPCommand
	( const uint8_t * pbytData, size_t nDataBytes,
		const uint8_t * pbytIP, size_t nIPBytes )
{
	if ( !VerifyUDPData( pbytData, nDataBytes ) )
	{
		return ;
	}
	SMemoryReferenceFile	memfile ;
	memfile.AttachMemory
		( (uint8_t*) pbytData + sizeof(RecvUDPHeader),
					nDataBytes - sizeof(RecvUDPHeader) ) ;
	//
	SXMLDocument	xmlCmds ;
	xmlCmds.ReadDocument( memfile, xmlCmds ) ;
	//
	SXMLDocument *	pxmlCommands = xmlCmds.GetElementTagAs( L"commands" ) ;
	if ( pxmlCommands == NULL )
	{
		return ;
	}
	//
#if	defined(__PLATFORM_WINDOWS__)
	m_csSync.Lock() ;
	Instance *	pInstance = GetInstanceUDPIPOf( pbytIP, nIPBytes ) ;
	if ( pInstance != NULL )
	{
		pInstance->AddRef() ;
	}
	m_csSync.Unlock() ;
	//
	for ( size_t i = 0; i < pxmlCommands->GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlCmd = pxmlCommands->GetElementAt( i ) ;
		if ( pxmlCmd == NULL )
		{
			continue ;
		}
		if ( pInstance == NULL )
		{
			if ( pxmlCmd->GetTag() == L"login" )
			{
				SString	strToken = pxmlCmd->GetAttrStringAs( L"token" ) ;
				for ( size_t i = 0; i < m_lstInstance.GetLength(); i ++ )
				{
					Instance *	p = m_lstInstance.GetAt( i ) ;
					if ( p && p->IsEqualToken( strToken ) )
					{
						p->DispatchUDPCommand
							( *pxmlCmd, pbytIP, nIPBytes ) ;
						break ;
					}
				}
			}
		}
		else
		{
			pInstance->DispatchUDPCommand
				( *pxmlCmd, pbytIP, nIPBytes ) ;
		}
	}
	if ( pInstance != NULL )
	{
		pInstance->ReleaseRef() ;
	}
#endif
}

#if	defined(__PLATFORM_WINDOWS__)

// IP が一致する接続取得
//////////////////////////////////////////////////////////////////////////////
SGLMediaStreamingServer::Instance *
	SGLMediaStreamingServer::GetInstanceUDPIPOf
		( const uint8_t * pbytIP, size_t nIPBytes ) const
{
	m_csSync.Lock() ;
	for ( size_t i = 0; i < m_lstInstance.GetLength(); i ++ )
	{
		Instance *	pInstance = m_lstInstance.GetAt( i ) ;
		if ( pInstance
			&& pInstance->IsEqualUDPIP( pbytIP, nIPBytes ) )
		{
			m_csSync.Unlock() ;
			return	pInstance ;
		}
	}
	m_csSync.Unlock() ;
	return	NULL ;
}

#endif

// UDP 受信データ検証
//////////////////////////////////////////////////////////////////////////////
bool SGLMediaStreamingServer::VerifyUDPData
	( const uint8_t * pbytData, size_t nDataBytes )
{
	RecvUDPHeader *	pHeader = (RecvUDPHeader*) pbytData ;
	if ( nDataBytes < sizeof(RecvUDPHeader) )
	{
		return	false ;
	}
	if ( nDataBytes != pHeader->nBodyBytes + sizeof(RecvUDPHeader) )
	{
		return	false ;
	}
	SakuraCL::CRC32Context	crc32 ;
	crc32.Stream( pbytData + sizeof(RecvUDPHeader), pHeader->nBodyBytes ) ;
	return	(pHeader->nBodyCRC32 == crc32.GetCRC32()) ;
}

// UDP データ送信
//////////////////////////////////////////////////////////////////////////////
size_t SGLMediaStreamingServer::SendUDPData
	( const void * ptrBuf, size_t nBytes, void * ptrAddrTo, size_t nAddrBytes )
{
	size_t	nSentBytes ;
	m_csSync.Lock() ;
	nSentBytes = m_socketUDP.SendTo( ptrBuf, nBytes, ptrAddrTo, nAddrBytes ) ;
	m_csSync.Unlock() ;
	return	nSentBytes ;
}


//////////////////////////////////////////////////////////////////////////////
// UDP 受信スレッド
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
( SakuraGL::SGLMediaStreamingServer::UDPProcedure, ESLObject, SProcedure )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLMediaStreamingServer::UDPProcedure::UDPProcedure( SGLMediaStreamingServer * pmss )
{
	m_pmss = pmss ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLMediaStreamingServer::UDPProcedure::~UDPProcedure( void )
{
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::UDPProcedure::Run( void )
{
	if ( m_pmss->m_socketUDP.Create
		( m_pmss->m_nListenUDPPort, SSocket::typeDatagram ) != errSuccess )
	{
		ESLTrace( "failed to create UDP/IP soket (SGLMediaStreamingServer).\n" ) ;
		return ;
	}
	SArray<uint8_t>	bufData ;
	bufData.SetLength( 0x10000 ) ;
	for ( ; ; )
	{
		int64_t	msecTimeout = 10 ;
		while ( m_pmss->m_socketUDP.Poll
					( SSocket::pollIn, msecTimeout ) & SSocket::pollIn )
		{
			uint8_t	bufIP[0x100] ;
			size_t	bytesIP = 0x100 ;
			size_t	nRecvBytes ;
			m_pmss->m_csSync.Lock() ;
			nRecvBytes =
				m_pmss->m_socketUDP.ReceiveFrom
					( bufData.GetArray(), 0x10000, &bufIP[0], bytesIP ) ;
			bufData.FinishArray() ;
			m_pmss->m_csSync.Unlock() ;
			if ( nRecvBytes != 0 )
			{
				m_pmss->DispatchUDPCommand
					( bufData.GetConstArray(), nRecvBytes, &bufIP[0], bytesIP ) ;
			}
			msecTimeout = 0 ;
		}
		if ( m_pmss->m_eventExit.Wait( 1 ) == errSuccess )
		{
			break ;
		}
	}
}


#if	defined(__PLATFORM_WINDOWS__)

//////////////////////////////////////////////////////////////////////////////
// 音声ストリーム用スレッド関数
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLMediaStreamingServer::SoundProcedure::SoundProcedure( Instance * instance )
{
	m_instance = instance ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::SoundProcedure::Run( void )
{
	while ( m_instance->m_flagSound )
	{
		m_instance->OnStreamSound() ;
		::SleepMilliSec( 5 ) ;
	}
}

// 完了後の処理
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::SoundProcedure::Finalize( void )
{
	delete	this ;
}


//////////////////////////////////////////////////////////////////////////////
// 映像ストリーム用スレッド関数
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLMediaStreamingServer::VideoProcedure::VideoProcedure( Instance * instance )
{
	m_instance = instance ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::VideoProcedure::Run( void )
{
	while ( m_instance->m_flagImage )
	{
		uint32_t	msecNextWait = m_instance->OnStreamVideo() ;
		::SleepMilliSec( msecNextWait ) ;
	}
}

// 完了後の処理
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::VideoProcedure::Finalize( void )
{
	delete	this ;
}


//////////////////////////////////////////////////////////////////////////////
// 接続インスタンス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLMediaStreamingServer::Instance, ESLObject, SProcedure )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLMediaStreamingServer::Instance::Instance( SGLMediaStreamingServer * pmss )
{
	m_countRef = 1 ;
	m_pmss = pmss ;
	m_flagAbort = false ;

	m_flagSound = false ;
	m_nSoundPacketID = 0 ;
	m_msecResendLimit = 0 ;

	m_flagImage = false ;
	m_idFrameID = 1 ;
	m_idCursorID = 1 ;
	m_flagIFrame = false ;
	m_flagShouldIFrame = true ;

#if	defined(__PLATFORM_WINDOWS__)
	m_hwndCapture = NULL ;
	m_rectCapture.x = 0 ;
	m_rectCapture.y = 0 ;
	m_rectCapture.w = ::GetSystemMetrics( SM_CXSCREEN ) ;
	m_rectCapture.h = ::GetSystemMetrics( SM_CYSCREEN ) ;
	m_hLastCursor = NULL ;
	m_nNoChangeCursor = 0 ;
	m_scrnCapture.SetCaptureTarget( NULL ) ;
	m_scrnCapture.SetCapturePosition( m_rectCapture.x, m_rectCapture.y ) ;
	m_scrnCapture.SetCaptureSize( m_rectCapture.w, m_rectCapture.h ) ;
#endif
	m_nImageScale = 0 ;
	m_limitMinDiffBlock = 3 * 3 * 64 ;
	m_limitMaxDiffBlock = 32 * 32 * 64 ;
	m_msecFrameInterval = 33 ;
	m_msecRefreshInterval = 3000 ;
	m_bytesTargetIFrame = 0x07FFFFFF ;
	m_bytesIFrameSend = 0 ;
	m_msecIFrameSend = 0 ;
	m_bytesSendMinLimitRate = 166 * 1024 ;
	m_bytesBestSendRate = 0 ;
	m_nPScaleLastIFrame = 0 ;
	m_countTryHQIFrame = 0 ;
	//
	for ( int i = 0; i < 4; i ++ )
	{
		for ( int j = 0; j < 64; j ++ )
		{
			m_fxDCTParam[i][j] = 0x20 ;
		}
	}
}

SGLMediaStreamingServer::Instance::Instance( const Instance& inst )
{
	m_countRef = 1 ;
	m_pmss = inst.m_pmss ;
	m_flagAbort = false ;
	m_flagSound = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLMediaStreamingServer::Instance::~Instance( void )
{
}

// 参照カウンタ加算
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::Instance::AddRef( void )
{
	AtomicAdd( &m_countRef, 1 ) ;
}

// 参照カウンタ減少・削除
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::Instance::ReleaseRef( void )
{
	if ( AtomicSub( &m_countRef, 1 ) <= 0 )
	{
		delete	this ;
	}
}

// 開始
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaStreamingServer::Instance::Start( SSystem::SSocket& socket )
{
	if ( socket.Accept( m_socketTCP ) )
	{
		return	sglErrFailed ;
	}
	if ( !socket.GetAcceptedCleintIP( m_strClientIP ) )
	{
		if ( m_pmss->IsDenyClient( m_strClientIP ) )
		{
			ESLTrace( "SGLMediaStreamingServer: deny connect %s.\n",
								m_strClientIP.ToCharArray().GetConstArray() ) ;
			m_socketTCP.WriteEncodedString
				( L"999 bye\n", -1, Charset::encodingUTF8 ) ;
			m_socketTCP.WaitToSendAll( 1000 ) ;
			m_socketTCP.Close() ;
			return	sglErrFailed ;
		}
	}
	if ( m_pmss->AddInstance( this ) )
	{
		ESLTrace( "SGLMediaStreamingServer: connect client %s.\n",
								m_strClientIP.ToCharArray().GetConstArray() ) ;
		if ( m_threadTCP.BeginThread( this ) )
		{
			m_pmss->RemoveInstance( this ) ;
			return	sglErrFailed ;
		}
	}
	else
	{
		ESLTrace( "SGLMediaStreamingServer: too many connection %s.\n",
								m_strClientIP.ToCharArray().GetConstArray() ) ;
		m_socketTCP.WriteEncodedString
			( L"999 bye\n", -1, Charset::encodingUTF8 ) ;
		m_socketTCP.WaitToSendAll( 1000 ) ;
		m_socketTCP.Close() ;
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// ランダムトーイン一致判定
//////////////////////////////////////////////////////////////////////////////
bool SGLMediaStreamingServer::Instance::IsEqualToken
				( const wchar_t * pwszToken ) const
{
	return	(m_strRandomToken == pwszToken) ;
}

// IP 一致判定
//////////////////////////////////////////////////////////////////////////////
bool SGLMediaStreamingServer::Instance::IsEqualUDPIP
				( const uint8_t * pbytIP, size_t nIPBytes ) const
{
	if ( m_bufUDPIP.GetLength() != nIPBytes )
	{
		return	false ;
	}
	const uint8_t *	pbytUDPIP = m_bufUDPIP.GetConstArray() ;
	for ( size_t i = 0; i < nIPBytes; i ++ )
	{
		if ( pbytIP[i] != pbytUDPIP[i] )
		{
			return	false ;
		}
	}
	return	true ;
}

// 強制停止
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::Instance::AsyncAbort( void )
{
	m_flagAbort = true ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::Instance::Run( void )
{
	//
	// ログイン
	//
	SCLRandomizer	random ;
	random.InitializeSeed() ;
	uint8_t	buf[0x10] ;
	size_t	nHelloLen = 12 ;
	//
	SString	strHello = L"100 hello " ;
	SString	strRandToken ;
	for ( size_t i = 0; i < 12; i ++ )
	{
		buf[i] = (uint8_t) random.Randomize( 0x100 ) ;
	}
	Charset::EncodeBase64( strRandToken, &buf[0], nHelloLen ) ;
	strHello += strRandToken ;
	m_strRandomToken = strRandToken ;
	//
	SendMessageLine( strHello ) ;
	//
	STimeCounter	timerLogin ;
	for ( ; ; )
	{
		SString	strLine ;
		if ( !RecvMessageLine( strLine ) )
		{
			SObjectArray<SString>	arrParam ;
			if ( SUsageMatcher::Match
					( strLine, L"pass (%t)\\", &arrParam ) )
			{
				SendMessageLine( L"900 error" ) ;
				m_socketTCP.WaitToSendAll( 1000 ) ;
				m_socketTCP.Close() ;
				m_pmss->ClientLoginResult( m_strClientIP, false ) ;
				return ;
			}
			SArray<uint8_t>	bufPass ;
			Charset::Encode
				( bufPass, Charset::encodingUTF8, m_pmss->m_strLoginPass ) ;
			//
			MD5Context	md5 ;
			md5.Stream( bufPass.GetConstArray(), bufPass.GetLength() ) ;
			md5.Stream( &buf[0], nHelloLen ) ;
			md5.Flush() ;
			//
			SString	strHexDigest ;
			if ( arrParam.At(0) != md5.GetMD5DigestHex( strHexDigest ) )
			{
				SendMessageLine( L"900 error" ) ;
				m_socketTCP.WaitToSendAll( 1000 ) ;
				m_socketTCP.Close() ;
				m_pmss->ClientLoginResult( m_strClientIP, false ) ;
				return ;
			}
			m_pmss->ClientLoginResult( m_strClientIP, true ) ;
			break ;
		}
		else if ( m_flagAbort )
		{
			return ;
		}
		if ( m_socketTCP.WaitToClose( 10 ) == errSuccess )
		{
			return ;
		}
		if ( timerLogin.GetTime() >= 60000 )
		{
			return ;
		}
	}
	SendMessageLine( L"100 connected TCP." ) ;
	//
	// コマンド受付ループ
	//
	QuickLock() ;
	m_timerLastRecv.Reset() ;
	QuickUnlock() ;
	//
	while ( !m_flagAbort )
	{
		SString	strLine ;
		while ( !RecvMessageLine( strLine ) )
		{
			DispatchCommand( strLine ) ;
			//
			QuickLock() ;
			m_timerLastRecv.Reset() ;
			QuickUnlock() ;
		}
		m_socketTCP.WaitToReceive( 10 ) ;
		if ( m_socketTCP.WaitToClose( 0 ) == errSuccess )
		{
			break ;
		}
		bool	fTimeout = false ;
		QuickLock() ;
		fTimeout = (m_timerLastRecv.GetTime() > 30 * 60 * 1000) ;
		QuickUnlock() ;
		if ( fTimeout )
		{
			break ;
		}
	}
	m_socketTCP.Close() ;
	//
	if ( m_flagSound )
	{
		EndSound() ;
	}
	if ( m_flagImage )
	{
		EndVideo() ;
	}
}

// 完了後の処理
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::Instance::Finalize( void )
{
	ESLTrace( "SGLMediaStreamingServer: disconnect.\n" ) ;
	m_pmss->RemoveInstance( this ) ;
}

// サウンドストリーム処理
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::Instance::OnStreamSound( void )
{
	for ( ; ; )
	{
		void *	ptrBuf = m_qbufRecSound.PutBuffer( 0x10000 ) ;
		size_t	nRecBytes = m_recSound.Read( ptrBuf, 0x10000 ) ;
		m_qbufRecSound.FlushBuffer( nRecBytes ) ;
		if ( nRecBytes == 0 )
		{
			break ;
		}
	}
	bool	fDelaySound = false ;
	m_csSyncSound.Lock() ;
	while ( m_queSoundData.GetLength() > 0 )
	{
		SArray<uint8_t> *	pData = m_queSoundData.GetAt( 0 ) ;
		if ( pData != NULL )
		{
			fDelaySound = true ;
			if ( m_timerResendLimit.GetRealTime() < m_msecResendLimit )
			{
				break ;
			}
			if ( m_pmss->SendUDPData
				( pData->GetConstArray(), pData->GetLength(),
					m_bufUDPIP.GetArray(), m_bufUDPIP.GetLength() ) == 0 )
			{
				m_bufUDPIP.FinishArray() ;
				break ;
			}
			m_bufUDPIP.FinishArray() ;
			m_timerResendLimit.Reset() ;
			break ;
		}
	}
	m_csSyncSound.Unlock() ;
	//
	if ( m_qbufRecSound.GetLength()
			>= (int64_t) (m_nSoundInterval * m_mioHeader.dwChannelCount * 2) )
	{
		if ( fDelaySound )
		{
			m_csSyncSound.Lock() ;
			while ( m_queSoundData.GetLength() >= 4 )
			{
				m_queSoundData.RemoveAt( 0 ) ;
			}
			m_csSyncSound.Unlock() ;
		}
		size_t	nSamples =
					(size_t) m_qbufRecSound.GetLength()
						/ (m_mioHeader.dwChannelCount * 2) ;
		nSamples -= nSamples % m_nSoundInterval ;
		//
		size_t	nBytes = nSamples * m_mioHeader.dwChannelCount * 2 ;
		const void *
				ptrWaveBuf = m_qbufRecSound.GetBuffer( nBytes ) ;
		//
		// データ圧縮
		//
		SSmartBuffer			sbufFile ;
		ERISA::MIO_DATA_HEADER	mdh ;
		mdh.bytVersion = 1 ;
		// mdh.bytFlags = m_flagKeySound ? ERISA::mioDataLeadBlock : 0 ;
		mdh.bytFlags = ERISA::mioDataLeadBlock ;
		mdh.bytReserved1 = 0 ;
		mdh.bytReserved2 = 0 ;
		mdh.dwSampleCount = (DWORD) nSamples ;
		//
		sbufFile.Write( &mdh, sizeof(ERISA::MIO_DATA_HEADER) ) ;
		if ( mdh.bytFlags & ERISA::mioDataLeadBlock )
		{
			sbufFile.Write
				( &m_mioHeader, sizeof(ERISA::MIO_INFO_HEADER) ) ;
		}
		//
		ERISA::SGLEncodeBitStream	bstream( 0x10000 ) ;
		bstream.AttachOutputStream( &sbufFile ) ;
		if ( !m_encSound.EncodeSound( bstream, mdh, ptrWaveBuf ) )
		{
			SArray<uint8_t>	bufData ;
			bufData.SetLength
				( sizeof(UDPDataHeader)
						+ (size_t) sbufFile.GetLength() ) ;
			//
			UDPDataHeader *
				pHeader = (UDPDataHeader*) bufData.GetArray() ;
			pHeader->typeData = udpDataSound ;
			pHeader->bytesData = (uint32_t) sbufFile.GetLength() ;
			pHeader->idSerial = ++ m_nSoundPacketID ;
			//
			sbufFile.Seek( 0 ) ;
			sbufFile.Read
				( pHeader + 1, (size_t) pHeader->bytesData ) ;
			//
			SakuraCL::CRC32Context	crc32 ;
			crc32.Stream( (const uint8_t*) (pHeader + 1), pHeader->bytesData ) ;
			pHeader->nCRC32 = crc32.GetCRC32() ;
			//
			bufData.FinishArray() ;
			//
			// データ送信
			//
			m_csSyncSound.Lock() ;
			SArray<uint8_t> *	pData = new SArray<uint8_t> ;
			pData->AddArray
				( (const uint8_t*) pHeader, bufData.GetLength() ) ;
			m_queSoundData.Add( pData ) ;
			//
			if ( m_queSoundData.GetLength() == 1 )
			{
				if ( m_pmss->SendUDPData
					( pHeader, bufData.GetLength(),
						m_bufUDPIP.GetArray(), m_bufUDPIP.GetLength() ) == 0 )
				{
					m_msecResendLimit = 0 ;
				}
				else
				{
					m_msecResendLimit =
						(uint32_t) (m_nSoundInterval * 1000
										/ m_mioHeader.dwSamplesPerSec / 3) ;
				}
				m_bufUDPIP.FinishArray() ;
				m_timerResendLimit.Reset() ;
			}
			m_csSyncSound.Unlock() ;
			//
			// キーフレーム判定（現エンコーディングでは無意味）
			//
			m_flagKeySound = false ;
			m_nSoundStreamSamples += nSamples ;
			if ( m_nSoundStreamSamples >
					m_mioHeader.dwSamplesPerSec
						* m_pmss->m_msecSoundKeyInterval / 1000 )
			{
				m_flagKeySound = true ;
				m_nSoundStreamSamples = 0 ;
			}
		}
		//
		m_qbufRecSound.ReleaseBuffer( (ssize_t) nBytes ) ;
	}
}

// 映像ストリーム処理
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLMediaStreamingServer::Instance::OnStreamVideo( void )
{
	OnStreamCursor() ;
	//
	// 未送信データを送信
	//                                
	double	msecInterval = 1000.0 / m_msecRefreshInterval ;
	if ( !SendVideoStream() )
	{
		if ( m_flagIFrame
			|| (m_timerLastFrame.GetRealTime() < m_msecRefreshInterval) )
		{
			return	1 ;
		}
	}
	//
	// フレーム経過時間判定
	//
	if ( (m_msecFrameInterval != 0)
		&& (m_timerLastFrame.GetRealTime() < m_msecFrameInterval) )
	{
		double	fpLeftTime = msecInterval - m_timerLastFrame.GetRealTime() ;
		if ( fpLeftTime < 1.0 )
		{
			return	1 ;
		}
		uint32_t	msecLeftTime =
				(uint32_t) eslRoundR32ToInt( (float32_t) fpLeftTime ) ;
		if ( msecLeftTime < 10 )
		{
			return	msecLeftTime ;
		}
		return	10 ;
	}
	//
	// キャプチャウィンドウサイズ判定
	//
	m_csSyncFrame.Lock() ;
	//
	bool	flagIFrame = false ;
	SGLSize	sizeCapture ;
	if ( m_hwndCapture != NULL )
	{
		RECT	rectWindow ;
		if ( ::IsWindow( m_hwndCapture )
			&& ::GetWindowRect( m_hwndCapture, &rectWindow ) )
		{
			m_scrnCapture.SetCaptureSize
				( rectWindow.right - rectWindow.left,
					rectWindow.bottom - rectWindow.top ) ;
			ESLTrace( "Set capture rect : %d, %d, %d, %d.\n",
					rectWindow.left, rectWindow.top, rectWindow.right, rectWindow.bottom ) ;
		}
		else
		{
			ESLTrace( "Capture target window is lost.\n" ) ;
			m_hwndCapture = NULL ;
			m_rectCapture.x = 0 ;
			m_rectCapture.y = 0 ;
			m_rectCapture.w = ::GetSystemMetrics( SM_CXSCREEN ) ;
			m_rectCapture.h = ::GetSystemMetrics( SM_CYSCREEN ) ;
			m_scrnCapture.SetCaptureTarget( NULL, false ) ;
			m_scrnCapture.SetCapturePosition( 0, 0 ) ;
			m_scrnCapture.SetCaptureSize
					( m_rectCapture.w, m_rectCapture.h ) ;
		}
	}
	else
	{
		sizeCapture = m_rectCapture.GetSize() ;
	}
	//
	// キャプチャ実行
	//
	SGLImageObject *	pImage = m_scrnCapture.Capture() ;
	if ( pImage == NULL )
	{
		m_csSyncFrame.Unlock() ;
		return	10 ;
	}
	sizeCapture = pImage->GetImageSize() ;
	SGLImageBuffer	imgCapture ;
	imgCapture.ptrBuffer =
			pImage->LockBuffer( imgCapture, SGLImageObject::lockRead ) ;
	//
	SGLSize	sizeFrameBuf = m_imgLastFrame.GetImageSize() ;
	SGLSize	sizeInBlock ;
	if ( m_nImageScale > 0 )
	{
		SGLSize	sizeHalf( (sizeCapture.w >> 1), (sizeCapture.h >> 1) ) ;
		if ( sizeHalf.IsEmpty() )
		{
			m_csSyncFrame.Unlock() ;
			return	10 ;
		}
		sizeCapture = sizeHalf ;
		sizeInBlock.w = (sizeHalf.w + 0x0F) >> 4 ;
		sizeInBlock.h = (sizeHalf.h + 0x0F) >> 4 ;
		if ( m_imgCurFrameTemp.GetImageSize() != sizeHalf )
		{
			m_imgCurFrameTemp.CreateImage
				( sizeHalf.w, sizeHalf.h, formatImageRGB, 32 ) ;
		}
		if ( sizeFrameBuf != sizeHalf )
		{
			m_imgLastFrame.CreateImage
				( sizeHalf.w, sizeHalf.h, formatImageRGB, 32 ) ;
			m_bufVideoDiffBlock.SetLength( sizeInBlock.w * sizeInBlock.h ) ;
			flagIFrame = true ;
		}
		SGLImageBuffer	imgTemp ;
		imgTemp.ptrBuffer =
			m_imgCurFrameTemp.LockBuffer
				( imgTemp, SGLImageObject::lockWrite ) ;
		if ( sglEnlargeHalfImageBuffer( imgTemp, imgCapture ) )
		{
			pImage->UnlockBuffer( SGLImageObject::lockRead ) ;
			m_csSyncFrame.Unlock() ;
			return	10 ;
		}
		pImage->UnlockBuffer( SGLImageObject::lockRead ) ;
		//
		pImage = &m_imgCurFrameTemp ;
		imgCapture.ptrBuffer =
				pImage->LockBuffer( imgCapture, SGLImageObject::lockRead ) ;
	}
	else
	{
		sizeInBlock.w = (sizeCapture.w + 0x0F) >> 4 ;
		sizeInBlock.h = (sizeCapture.h + 0x0F) >> 4 ;
		if ( sizeFrameBuf != sizeCapture )
		{
			m_imgLastFrame.CreateImage
				( sizeCapture.w, sizeCapture.h, formatImageRGB, 32 ) ;
			m_bufVideoDiffBlock.SetLength( sizeInBlock.w * sizeInBlock.h ) ;
			flagIFrame = true ;
		}
	}
	SGLImageBuffer	imgLastFrame ;
	imgLastFrame.ptrBuffer =
			m_imgLastFrame.LockBuffer
					( imgLastFrame, SGLImageObject::lockReadWrite ) ;
	//
	//
	// 差分フレーム判定
	//
	if ( m_timerLastKeyFrame.GetRealTime() >= m_msecRefreshInterval )
	{
		flagIFrame = true ;
	}
	if ( flagIFrame )
	{
		// 独立フレームは未送信のデータを消去
		m_queVideoData.RemoveAll() ;
		m_timerLastKeyFrame.Reset() ;
		eslFillMemory
			( m_bufVideoDiffBlock.GetArray(),
					0, m_bufVideoDiffBlock.GetLength() ) ;
		m_bufVideoDiffBlock.FinishArray() ;

		// 送信フレームレート制御用パラメータ初期化
		m_timerSendCurrent.Reset() ;
		m_bytesSendCurrent = 0 ;
	}
	m_timerLastFrame.Reset() ;
	//
	// フレーム圧縮
	//
	UDPImageHeader	imgHeader ;
	//
	ERISA::SGLEncodeBitStream		bstream( 0x4000 ) ;
	ERISA::SGLGammaEncodeContext	encoder( &bstream ) ;
	m_sbufVideoTemp.Seek( 0 ) ;
	m_sbufVideoTemp.SetEndOfFile() ;
	bstream.AttachOutputStream( &m_sbufVideoTemp ) ;
	//
	ImageBlockset444	bs444 ;
	ImageBlockset444	bsLast ;
	ImageBlockset411	bs411 ;
	//
	eslFillMemory( &imgHeader, 0, sizeof(UDPImageHeader) ) ;
	imgHeader.sizeFrame = sizeCapture ;
	imgHeader.nStartBlock = 0 ;
	imgHeader.nSubframeBlocks = 0 ;
	imgHeader.nFlags = 0 ;
	imgHeader.nParamScale = m_nPScaleLastIFrame ;
	//
	if ( !flagIFrame )
	{
		imgHeader.nFlags |= udpImageDifferential ;
		imgHeader.nParamScale >>= 1 ;
	}
	//
	uint32_t	bytesFrame = 0 ;
	uint8_t *	pbytDiffBlock = m_bufVideoDiffBlock.GetArray() ;
	uint8_t *	pbytReqIFrame = m_bufVideoReqIFrame.GetArray() ;
	size_t		nReqIFrame = m_bufVideoReqIFrame.GetLength() ;
	//
	size_t	i = 0 ;
	for ( int y = 0; y < sizeInBlock.h; y ++ )
	{
		for ( int x = 0; x < sizeInBlock.w; x ++, i ++ )
		{
			bool		fIBlock = true ;
			bool		fNBlock = false ;
			int			iDCTParam = 0 ;
			uint32_t	bHeader = 0x00000000 ;
			SampleImageBlockset
				( bs444, imgCapture,
					(uint32_t) (x << 4), (uint32_t) (y << 4) ) ;
			if ( !flagIFrame && (pbytDiffBlock[i] < 10) )
			{
				if ( (i < nReqIFrame) && pbytReqIFrame[i] )
				{
					pbytReqIFrame[i] = 0 ;
					pbytDiffBlock[i] = 0 ;
				}
				else
				{
					uint32_t	nDiff, nMaxDiff ;
					SampleImageBlockset
						( bsLast, imgLastFrame,
							(uint32_t) (x << 4), (uint32_t) (y << 4) ) ;
					nDiff = DifferenceImageBlockset( nMaxDiff, bs444, bsLast ) ;
					if ( nDiff <= m_limitMinDiffBlock * 12 )
					{
						fNBlock = true ;
						bHeader = 0x80000000 ;
					}
					else if ( (nMaxDiff > m_limitMaxDiffThreshold)
							|| (nDiff >= m_limitMaxDiffBlock * 12) )
					{
						IDifferenceImageBlockset( bs444, bsLast ) ;
						pbytDiffBlock[i] = 0 ;
					}
					else
					{
						fIBlock = false ;
						iDCTParam = 2 ;
						bHeader = 0x40000000 ;
						pbytDiffBlock[i] ++ ;
					}
				}
			}
			else
			{
				pbytDiffBlock[i] = 0 ;
			}
			bstream.OutNBits( bHeader, 2 ) ;
			//
			if ( !fNBlock )
			{
				ConvertBlocksetRGBtoYUV( bs444 ) ;
				ConvertBlockset444to411( bs411, bs444 ) ;
				//
				EncodeBlockset411
					( encoder, bs411, imgHeader.nParamScale, iDCTParam ) ;
			}
			//
			imgHeader.nSubframeBlocks ++ ;
			//
			if ( m_sbufVideoTemp.GetLength() >= 0x1000 )
			{
				encoder.FinishEncoding() ;
				//
				bytesFrame += (uint32_t) m_sbufVideoTemp.GetLength() ;
				SendVideoSubframe( imgHeader, m_sbufVideoTemp ) ;
				//
				m_sbufVideoTemp.Seek( 0 ) ;
				m_sbufVideoTemp.SetEndOfFile() ;
				imgHeader.nStartBlock = y * sizeInBlock.w + x + 1 ;
				imgHeader.nSubframeBlocks = 0 ;
			}
		}
	}
	m_bufVideoDiffBlock.FinishArray() ;
	m_bufVideoReqIFrame.FinishArray() ;
	m_bufVideoReqIFrame.RemoveAll() ;
	//
	if ( imgHeader.nStartBlock < (uint32_t) (sizeInBlock.w * sizeInBlock.h) )
	{
		encoder.FinishEncoding() ;
		bytesFrame += (uint32_t) m_sbufVideoTemp.GetLength() ;
		SendVideoSubframe( imgHeader, m_sbufVideoTemp ) ;
	}
	if ( bytesFrame > 166 * 1024 )
	{
		m_bytesSendMinLimitRate = bytesFrame ;
	}
	//
	sglCopyImageBuffer( imgLastFrame, imgCapture ) ;
	//
	pImage->UnlockBuffer( SGLImageObject::lockRead ) ;
	m_imgLastFrame.UnlockBuffer( SGLImageObject::lockReadWrite ) ;
	//
	m_idFrameID ++ ;
	m_flagIFrame = flagIFrame ;
	//
	if ( flagIFrame )
	{
		m_bytesLastIFrame = bytesFrame ;
		//
		if ( m_bytesLastIFrame >= m_bytesTargetIFrame )
		{
			if ( ++ m_nPScaleLastIFrame >= 2 )
			{
				m_nPScaleLastIFrame = 2 ;
				//
				if ( ++ m_countTryHQIFrame >= 10 )
				{
					m_bytesTargetIFrame <<= 1 ;
					m_nPScaleLastIFrame -- ;
					m_countTryHQIFrame = 0 ;
				}
			}
		}
		else
		{
			m_countTryHQIFrame = 0 ;
			//
			if ( m_nPScaleLastIFrame > 0 )
			{
				m_nPScaleLastIFrame -- ;
			}
		}
	}
	m_csSyncFrame.Unlock() ;
	//
//	OnStreamCursor() ;
	//
	if ( !SendVideoStream() )
	{
		return	1 ;
	}
	double	fpLeftTime = msecInterval - m_timerLastFrame.GetRealTime() ;
	if ( fpLeftTime < 1.0 )
	{
		return	1 ;
	}
	uint32_t	msecLeftTime =
			(uint32_t) eslRoundR32ToInt( (float32_t) fpLeftTime ) ;
	if ( msecLeftTime < 10 )
	{
		return	msecLeftTime ;
	}
	return	10 ;
}

// カーソルデータ処理
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::Instance::OnStreamCursor( void )
{
	if ( m_timerLastCursor.GetRealTime() < m_msecFrameInterval )
	{
		return ;
	}
	CURSORINFO	ci ;
	eslFillMemory( &ci, 0, sizeof(CURSORINFO) ) ;
	ci.cbSize = sizeof(CURSORINFO) ;
	if ( !::GetCursorInfo( &ci ) )
	{
		return ;
	}
	//
	// カーソル情報取得
	//
	if ( m_hwndCapture )
	{
		::ScreenToClient( m_hwndCapture, &(ci.ptScreenPos) ) ;
	}
	else
	{
		ci.ptScreenPos.x -= m_rectCapture.x ;
		ci.ptScreenPos.y -= m_rectCapture.y ;
	}
	UDPCursorHeader	curHeader ;
	eslFillMemory( &curHeader, 0, sizeof(UDPCursorHeader) ) ;
	curHeader.ptCursor.x = ci.ptScreenPos.x >> m_nImageScale ;
	curHeader.ptCursor.y = ci.ptScreenPos.y >> m_nImageScale ;
	curHeader.nFlags = 0 ;
	curHeader.idCursor = (uint32_t) ((ulong_ptr_t)ci.hCursor) ;
	//
	SArray<uint8_t>	bufImage ;
	//
	if ( ci.hCursor != NULL )
	{
		if ( ci.flags & CURSOR_SHOWING )
		{
			curHeader.nFlags |= udpCursorShow ;
		}
		if ( (++ m_nNoChangeCursor >= 30)
			|| (m_hLastCursor != ci.hCursor) )
		{
			m_nNoChangeCursor = 0 ;
			//
			// カーソル画像取得
			//
			SGLImageBuffer *	pimgColor =
				SGLSpriteCursor::ConvertHCURSORtoImageBuffer( ci.hCursor ) ;
			if ( pimgColor == NULL )
			{
				return ;
			}
			curHeader.nFlags |= udpCursorImage ;
			curHeader.sizeCursor.w = pimgColor->width ;
			curHeader.sizeCursor.h = pimgColor->height ;
			curHeader.ptHotspot.x = pimgColor->ptOrigin.x ;
			curHeader.ptHotspot.y = pimgColor->ptOrigin.y ;
			//
			uint8_t *	pbytPixels = 
				bufImage.GetArray
					( pimgColor->width * pimgColor->height * 4 ) ;
			uint8_t *	pbytSrcLine = pimgColor->ptrBuffer ;
			for ( int y = 0; y < curHeader.sizeCursor.h; y ++ )
			{
				eslMoveMemory
					( pbytPixels, pbytSrcLine,
							curHeader.sizeCursor.w * 4 ) ;
				pbytSrcLine += pimgColor->pitchLine ;
				pbytPixels += pimgColor->width * 4 ;
			}
			//
			bufImage.FinishArray() ;
			sglReleaseImageBuffer( pimgColor ) ;
		}
	}
	m_hLastCursor = ci.hCursor ;
	//
	// UDP 送信データ生成
	//
	const size_t	nDataBytes = bufImage.GetLength() ;
	const size_t	nTotalBytes =
			sizeof(UDPDataHeader) + sizeof(UDPCursorHeader) + nDataBytes ;
	UDPDataHeader *	pHeader =
		(UDPDataHeader*) m_bufVideoTemp.GetArray( nTotalBytes ) ;
	pHeader->typeData = updDataCursor ;
	pHeader->bytesData = (uint32_t) (sizeof(UDPCursorHeader) + nDataBytes) ;
	pHeader->idSerial = m_idCursorID ++ ;
	//
	UDPCursorHeader *	pCursorHdr = (UDPCursorHeader*) (pHeader + 1) ;
	*pCursorHdr = curHeader ;
	//
	if ( nDataBytes > 0 )
	{
		eslMoveMemory( pCursorHdr + 1, bufImage.GetConstArray(), nDataBytes ) ;
	}
	//
	SakuraCL::CRC32Context	crc32 ;
	crc32.Stream( (const uint8_t*) (pHeader + 1), pHeader->bytesData ) ;
	pHeader->nCRC32 = crc32.GetCRC32() ;
	m_bufVideoTemp.FinishArray() ;
	//
	// 未送信のカーソルデータを削除する
	//
	const size_t	nQueCount = m_queVideoData.GetLength() ;
	bool			flagTrimData = false ;
	for ( size_t i = 0; i < nQueCount; i ++ )
	{
		SArray<uint8_t> *	pData = m_queVideoData.GetAt( i ) ;
		if ( pData != NULL )
		{
			const UDPDataHeader *	pDataHeader =
				(const UDPDataHeader*) pData->GetConstArray() ;
			if ( (pDataHeader != NULL)
				&& (pDataHeader->typeData == updDataCursor) )
			{
				m_queVideoData.SetAt( i, NULL ) ;
				flagTrimData = true ;
			}
		}
	}
	if ( flagTrimData )
	{
		m_queVideoData.TrimEmpty() ;
	}
	//
	// カーソルデータを送信する
	//
	if ( m_pmss->SendUDPData
		( pHeader, nTotalBytes,
			m_bufUDPIP.GetArray(), m_bufUDPIP.GetLength() ) == 0 )
	{
		SArray<uint8_t> *	pData = new SArray<uint8_t> ;
		pData->AddArray( (const uint8_t*) pHeader, nTotalBytes ) ;
		m_queVideoData.InsertAt( 0, pData ) ;
	}
	m_bufUDPIP.FinishArray() ;
	m_timerLastCursor.Reset() ;
}

// 未送信の映像データを送信
//////////////////////////////////////////////////////////////////////////////
bool SGLMediaStreamingServer::Instance::SendVideoStream( void )
{
	while ( m_queVideoData.GetLength() > 0 )
	{
		if ( m_timerSendCurrent.GetRealTime() >= 1000.0 )
		{
			m_timerSendCurrent.Reset() ;
			m_bytesSendCurrent = 0 ;
		}
		if ( m_bytesIFrameSend )
		{
			uint32_t	msecIFrame = m_msecIFrameSend ;
			if ( msecIFrame > m_msecFrameInterval * 4 )
			{
				msecIFrame = m_msecFrameInterval * 4 ;
				if ( msecIFrame == 0 )
				{
					msecIFrame = 1 ;
				}
			}
			double	fpSendRate =
				(double) m_bytesIFrameSend * 1000.0 / msecIFrame ;
			if ( m_bytesBestSendRate > fpSendRate )
			{
				fpSendRate = (fpSendRate + m_bytesBestSendRate) * 0.5 ;
			}
			if ( fpSendRate < m_bytesSendMinLimitRate )
			{
				fpSendRate = m_bytesSendMinLimitRate ;
			}
			if ( m_bytesSendCurrent * 1000.0
						/ m_timerSendCurrent.GetRealTime() >= fpSendRate )
			{
				return	false ;
			}
		}
		SArray<uint8_t> *	pData = m_queVideoData.GetAt( 0 ) ;
		if ( pData != NULL )
		{
			if ( m_pmss->SendUDPData
				( pData->GetConstArray(), pData->GetLength(),
					m_bufUDPIP.GetArray(), m_bufUDPIP.GetLength() ) == 0 )
			{
				m_bufUDPIP.FinishArray() ;
				return	false ;
			}
			m_bufUDPIP.FinishArray() ;
			m_bytesSendCurrent += (uint32_t) pData->GetLength() ;
		}
		m_queVideoData.RemoveAt( 0 ) ;
	}
	return	true ;
}

// HBITMAP を RGB32 画像へ変換
//////////////////////////////////////////////////////////////////////////////
SGLImageBuffer *
	SGLMediaStreamingServer::Instance::
			ConvertHBITMAPtoImageBuffer( HBITMAP hBitmap )
{
	return	SGLSpriteCursor::ConvertHBITMAPtoImageBuffer( hBitmap ) ;
}

// カーソルのカラー画像にマスク画像を合成
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::Instance::MakeBlendCursorImage
	( SGLImageBuffer * pimgColor, SGLImageBuffer * pimgMask )
{
	SGLSpriteCursor::MakeBlendCursorImage( pimgColor, pimgMask ) ;
}

// サンプリング
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::Instance::SampleImageBlockset
	( SGLMediaStreamingServer::ImageBlockset444& bsRGB,
		const SGLImageBuffer& imgFrame, uint32_t xPos, uint32_t yPos )
{
	const int32_t	pitchLine = imgFrame.pitchLine ;
	const int32_t	pitchPixel = imgFrame.pitchPixel ;
	if ( (xPos + 16 <= imgFrame.width)
		&& (yPos + 16 <= imgFrame.height) )
	{
		for ( int yBlock = 0; yBlock < 2; yBlock ++ )
		{
			for ( int xBlock = 0; xBlock < 2; xBlock ++ )
			{
				size_t		j = (yBlock << 1) + xBlock ;
				int16_t *	pB = &(bsRGB.b[j][0]) ;
				int16_t *	pG = &(bsRGB.g[j][0]) ;
				int16_t *	pR = &(bsRGB.r[j][0]) ;
				//
				uint8_t *	pbytBlock =
					imgFrame.ptrBuffer
						+ ((yPos + (yBlock << 3)) * pitchLine
							+ ((xPos + (xBlock << 3)) * pitchPixel)) ;
				//
				for ( int y = 0; y < 8; y ++ )
				{
					uint8_t *	pbytNext = pbytBlock ;
					for ( int x = 0; x < 8; x ++ )
					{
						*(pB ++) = pbytNext[0] ;
						*(pG ++) = pbytNext[1] ;
						*(pR ++) = pbytNext[2] ;
						pbytNext += pitchPixel ;
					}
					pbytBlock += pitchLine ;
				}
			}
		}
	}
	else
	{
		eslFillMemory( &bsRGB, 0, sizeof(ImageBlockset444) ) ;
		//
		for ( int ySub = 0; ySub < 2; ySub ++ )
		{
			for ( int xSub = 0; xSub < 2; xSub ++ )
			{
				size_t		j = (ySub << 1) + xSub ;
				int16_t *	pB = &(bsRGB.b[j][0]) ;
				int16_t *	pG = &(bsRGB.g[j][0]) ;
				int16_t *	pR = &(bsRGB.r[j][0]) ;
				//
				int	xBlock = xPos + (xSub << 3) ;
				int	yBlock = yPos + (ySub << 3) ;
				int	wBlock = (int) imgFrame.width - xBlock ;
				int	hBlock = (int) imgFrame.height - yBlock ;
				if ( wBlock > 8 )
				{
					wBlock = 8 ;
				}
				if ( hBlock > 8 )
				{
					hBlock = 8 ;
				}
				//
				uint8_t *	pbytBlock =
					imgFrame.ptrBuffer
						+ (yBlock * pitchLine + xBlock * pitchPixel) ;
				//
				for ( int y = 0; y < hBlock; y ++ )
				{
					uint8_t *	pbytNext = pbytBlock ;
					for ( int x = 0; x < wBlock; x ++ )
					{
						*(pB ++) = pbytNext[0] ;
						*(pG ++) = pbytNext[1] ;
						*(pR ++) = pbytNext[2] ;
						pbytNext += pitchPixel ;
					}
					pbytBlock += pitchLine ;
				}
			}
		}
	}
}

// 差分処理
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLMediaStreamingServer::Instance::DifferenceImageBlockset
	( uint32_t& nMaxDiffPow2,
		SGLMediaStreamingServer::ImageBlockset444& bsDiff,
		const SGLMediaStreamingServer::ImageBlockset444& bsLast )
{
	int16_t *		pDst = &(bsDiff.b[0][0]) ;
	const int16_t *	pSrc = &(bsLast.b[0][0]) ;
	const int		nCount = 64 * 4 * 3 ;
	uint32_t		nSumDiff = 0 ;
	uint32_t		nMaxDiff = 0 ;
	uint32_t		nMask ;
	//
	for ( int i = 0; i < nCount; i ++ )
	{
		const int	n = pDst[i] - pSrc[i] ;
		pDst[i] = (int16_t) n ;
		uint32_t	nDiff = (uint32_t) (n * n) ;
		nSumDiff += nDiff ;
		nMask = (uint32_t) (- (int) (nDiff > nMaxDiff)) ;
		nMaxDiff = (n & nMask) | (nMaxDiff & ~nMask) ; 
	}
	//
	nMaxDiffPow2 = nMaxDiff ;
	return	nSumDiff ;
}

void SGLMediaStreamingServer::Instance::IDifferenceImageBlockset
	( SGLMediaStreamingServer::ImageBlockset444& bsDiff,
		const SGLMediaStreamingServer::ImageBlockset444& bsLast )
{
	int16_t *		pDst = &(bsDiff.b[0][0]) ;
	const int16_t *	pSrc = &(bsLast.b[0][0]) ;
	const int		nCount = 64 * 4 * 3 ;
	for ( int i = 0; i < nCount; i ++ )
	{
		pDst[i] += pSrc[i] ;
	}
}

// RGB -> YUV 変換
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::Instance::ConvertBlocksetRGBtoYUV
	( SGLMediaStreamingServer::ImageBlockset444& bsRGB )
{
	int16_t *	pB = &(bsRGB.b[0][0]) ;
	int16_t *	pG = &(bsRGB.g[0][0]) ;
	int16_t *	pR = &(bsRGB.r[0][0]) ;
	//
	for ( size_t i = 0; i < 64 * 4; i ++ )
	{
		// (Y)   (  7/24  7/12   1/8 ) (R)
		// (U) = ( -1/6  -1/3    1/2 ) (G)
		// (V)   ( 17/36 -7/18 -1/12 ) (B)
		int	b = pB[i] ;
		int	g = pG[i] ;
		int	r = pR[i] ;
		int	y = 75*r + 149*g + 32*b ;
		int	u = -43*r - 85*g + 128*b ;
		int	v = 121*r - 100*g - 21*b ;
		pB[i] = (int16_t) ((y + 0x80) >> 8) ;
		pG[i] = (int16_t) ((u + 0x7F) >> 8) ;
		pR[i] = (int16_t) ((v + 0x7F) >> 8) ;
	}
}

// 4:4:4 -> 4:1:1 変換
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::Instance::ConvertBlockset444to411
	( SGLMediaStreamingServer::ImageBlockset411& bs411,
		const SGLMediaStreamingServer::ImageBlockset444& bs444 )
{
	eslMoveMemory
		( &(bs411.y[0][0]),
			&(bs444.b[0][0]), 64 * 4 * sizeof(int16_t) ) ;
	//
	for ( int y = 0; y < 8; y ++ )
	{
		for ( int x = 0; x < 8; x ++ )
		{
			int	i = (y << 3) + x ;
			int	j = ((y >> 2) << 1) + (x >> 2) ;
			int	k = ((y & 0x03) << 4) + ((x & 0x03) << 1) ;
			bs411.u[i] =
				(bs444.g[j][k] + bs444.g[j][k + 1]
					+ bs444.g[j][k + 8] + bs444.g[j][k + 9] + 1) >> 2 ;
			bs411.v[i] =
				(bs444.r[j][k] + bs444.r[j][k + 1]
					+ bs444.r[j][k + 8] + bs444.r[j][k + 9] + 1) >> 2 ;
		}
	}
}

// DCT 変換・量子化・ジグザグ走査・符号化
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::Instance::EncodeBlockset411
	( ERISA::SGLGammaEncodeContext& encoder,
		SGLMediaStreamingServer::ImageBlockset411& bs411,
		uint8_t nParamScale, int iDCTParam )
{
	int16_t		bufDst[6][64] ;
	int16_t *	pBlock = &(bs411.y[0][0]) ;
	int16_t *	pDst = &(bufDst[0][0]) ;
	int16_t *	pDCTParam[6] =
	{
		&m_fxDCTParam[iDCTParam][0],
		&m_fxDCTParam[iDCTParam][0],
		&m_fxDCTParam[iDCTParam][0],
		&m_fxDCTParam[iDCTParam][0],
		&m_fxDCTParam[iDCTParam + 1][0],
		&m_fxDCTParam[iDCTParam + 1][0],
	} ;
	//
	// DCT 変換
	//
	for ( size_t i = 0; i < 6; i ++ )
	{
		ERISA::sclwFastDCT8x8( pBlock ) ;
		pBlock += 64 ;
	}
	//
	// DC 成分差分
	//
	bs411.y[1][0] -= bs411.y[0][0] ;
	bs411.y[3][0] -= bs411.y[2][0] ;
	bs411.y[2][0] -= bs411.y[0][0] ;
	bs411.y[3][0] -= bs411.y[1][0] ;
	//
	// 量子化・ジグザグ走査
	//
	int	nScale = nParamScale + 8 ;
	int	nRoundOdd = 1 << (nScale - 1) ;
	pBlock = &(bs411.y[0][0]) ;
	for ( size_t i = 0; i < 6; i ++ )
	{
		int16_t *	pParam = pDCTParam[i] ;
		for ( size_t j = 0; j < 64; j ++ )
		{
			int	n = ((int) pBlock[j] * (int) pParam[j] + nRoundOdd) >> nScale ;
			pDst[m_indexZigzagTable[j]] = (int16_t) n ;
		}
		pDst += 64 ;
		pBlock += 64 ;
	}
	//
	// 符号化
	//
	encoder.EncodeGammaCodeWords( &(bufDst[0][0]), 64 * 6 ) ;
}

// 送信
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::Instance::SendVideoSubframe
	( const SGLMediaStreamingServer::UDPImageHeader& imgHeader, SSmartBuffer& sbuf )
{
	const size_t	nDataBytes = (size_t) m_sbufVideoTemp.GetLength() ;
	const size_t	nTotalBytes =
			sizeof(UDPDataHeader) + sizeof(UDPImageHeader) + nDataBytes ;
	UDPDataHeader *	pHeader =
		(UDPDataHeader*) m_bufVideoTemp.GetArray( nTotalBytes ) ;
	pHeader->typeData = udpDataImage ;
	pHeader->bytesData = (uint32_t) (sizeof(UDPImageHeader) + nDataBytes) ;
	pHeader->idSerial = m_idFrameID ;
	//
	UDPImageHeader *	pImageHdr = (UDPImageHeader*) (pHeader + 1) ;
	*pImageHdr = imgHeader ;
	//
	sbuf.Seek( 0 ) ;
	sbuf.Read( pImageHdr + 1, nDataBytes ) ;
	//
	SakuraCL::CRC32Context	crc32 ;
	crc32.Stream( (const uint8_t*) (pHeader + 1), pHeader->bytesData ) ;
	pHeader->nCRC32 = crc32.GetCRC32() ;
	m_bufVideoTemp.FinishArray() ;
	//
	if ( m_queVideoData.GetLength() == 0 )
	{
		if ( m_bytesIFrameSend && m_msecIFrameSend )
		{
			double	fpSendRate =
				(double) m_bytesIFrameSend * 1000.0 / m_msecIFrameSend ;
			if ( m_bytesBestSendRate > fpSendRate )
			{
				fpSendRate = (fpSendRate + m_bytesBestSendRate) * 0.5 ;
			}
			if ( fpSendRate < m_bytesSendMinLimitRate )
			{
				fpSendRate = m_bytesSendMinLimitRate ;
			}
			if ( m_bytesSendCurrent * 1000.0
						/ m_timerSendCurrent.GetRealTime() >= fpSendRate )
			{
				SArray<uint8_t> *	pData = new SArray<uint8_t> ;
				pData->AddArray( (const uint8_t*) pHeader, nTotalBytes ) ;
				m_queVideoData.Add( pData ) ;
				return ;
			}
		}
		if ( m_pmss->SendUDPData
			( pHeader, nTotalBytes,
				m_bufUDPIP.GetArray(), m_bufUDPIP.GetLength() ) == 0 )
		{
			SArray<uint8_t> *	pData = new SArray<uint8_t> ;
			pData->AddArray( (const uint8_t*) pHeader, nTotalBytes ) ;
			m_queVideoData.Add( pData ) ;
		}
		else
		{
			m_bytesSendCurrent += (uint32_t) nTotalBytes ;
		}
		m_bufUDPIP.FinishArray() ;
	}
	else
	{
		SArray<uint8_t> *	pData = new SArray<uint8_t> ;
		pData->AddArray( (const uint8_t*) pHeader, nTotalBytes ) ;
		m_queVideoData.Add( pData ) ;
	}
}

// UDP コマンド処理
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::Instance::DispatchUDPCommand
	( const SSystem::SXMLDocument& xmlCmd,
		const uint8_t * pbytIP, size_t nIPBytes )
{
	if ( xmlCmd.GetTag() == L"login" )
	{
		m_bufUDPIP.RemoveAll() ;
		m_bufUDPIP.AddArray( pbytIP, nIPBytes ) ;
		//
		SakuraCL::CRC32Context	crc32 ;
		UDPDataHeader	udpData ;
		eslFillMemory( &udpData, 0, sizeof(UDPDataHeader) ) ;
		udpData.typeData = udpDataNull ;
		udpData.bytesData = 0 ;
		udpData.nCRC32 = crc32.GetCRC32() ;
		udpData.idSerial = 0 ;
		//
		while ( m_pmss->SendUDPData
				( &udpData, sizeof(UDPDataHeader),
					m_bufUDPIP.GetArray(), m_bufUDPIP.GetLength() ) == 0 )
		{
			SleepMilliSec( 10 ) ;
		}
		m_bufUDPIP.FinishArray() ;
		return ;
	}
	else if ( xmlCmd.GetTag() == L"sent_sound" )
	{
		uint64_t	idPacked = (uint64_t) xmlCmd.GetAttrHexIntegerAs( L"id" ) ;
		m_csSyncSound.Lock() ;
		m_queSoundData.TrimEmpty() ;
		for ( size_t i = 0; i < m_queSoundData.GetLength(); i ++ )
		{
			SArray<uint8_t> *	pData = m_queSoundData.GetAt( 0 ) ;
			ESLAssert( pData != NULL ) ;
			const UDPDataHeader *	pHeader =
				(const UDPDataHeader*) pData->GetConstArray() ;
			if ( pHeader->idSerial == idPacked )
			{
				m_queSoundData.RemoveAt( i ) ;
				m_msecResendLimit = 0 ;
				break ;
			}
		}
		m_csSyncSound.Unlock() ;
		//
		QuickLock() ;
		m_timerLastRecv.Reset() ;
		QuickUnlock() ;
		return ;
	}
}

// コマンド処理
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::Instance::DispatchCommand( const SSystem::SString& strLine )
{
	SStringParser	sparsLine ;
	sparsLine.AttachString( strLine ) ;
	//
	SString	strCmd ;
	if ( !sparsLine.NextString( strCmd ) )
	{
		return ;
	}
	SStrSortObjectArray<SString>	mapParam ;
	while ( sparsLine.PassSpace() )
	{
		SString	strName, strValue ;
		if ( (sparsLine.NextToken( strName ) == SStringParser::tokenNormal)
			&& (sparsLine.HasToComeChar( L"=" ) == L'=')
			&& sparsLine.NextString( strValue ) )
		{
			mapParam.SetAs( strName, new SString( strValue ) ) ;
		}
	}
	//
	if ( strCmd == L"mouse_move" )
	{
		if ( m_flagImage )
		{
			int	x = 0, y = 0 ;
			SString *	pstrX = mapParam.GetAs( L"x" ) ;
			if ( pstrX != NULL )
			{
				x = (int) pstrX->AsInteger() << m_nImageScale ;
			}
			SString *	pstrY = mapParam.GetAs( L"y" ) ;
			if ( pstrY != NULL )
			{
				y = (int) pstrY->AsInteger() << m_nImageScale ;
			}
			m_scrnCapture.MouseMove( x, y, true ) ;
		}
		return ;
	}
	else if ( strCmd == L"mouse_pos" )
	{
		if ( m_flagImage )
		{
			int	x = 0, y = 0 ;
			SString *	pstrX = mapParam.GetAs( L"x" ) ;
			if ( pstrX != NULL )
			{
				x = (int) pstrX->AsInteger() << m_nImageScale ;
			}
			SString *	pstrY = mapParam.GetAs( L"y" ) ;
			if ( pstrY != NULL )
			{
				y = (int) pstrY->AsInteger() << m_nImageScale ;
			}
			if ( m_hwndCapture && ::IsWindow( m_hwndCapture ) )
			{
				POINT	ptCursor = { x, y } ;
				::ClientToScreen( m_hwndCapture, &ptCursor ) ;
				m_scrnCapture.MouseMove( ptCursor.x, ptCursor.y, false ) ;
			}
			else
			{
				m_scrnCapture.MouseMove
					( m_rectCapture.x + x, m_rectCapture.y + y, false ) ;
			}
		}
		return ;
	}
	else if ( strCmd == L"key_press" )
	{
		SString *	pstrCode = mapParam.GetAs( L"code" ) ;
		if ( m_flagImage && (pstrCode != NULL) )
		{
			m_scrnCapture.KeyboardEvent( pstrCode->AsInteger(), 0, false ) ;
		}
		return ;
	}
	else if ( strCmd == L"key_release" )
	{
		SString *	pstrCode = mapParam.GetAs( L"code" ) ;
		if ( m_flagImage && (pstrCode != NULL) )
		{
			m_scrnCapture.KeyboardEvent( pstrCode->AsInteger(), 0, true ) ;
		}
		return ;
	}
	else if ( strCmd == L"mouse_event" )
	{
		int			nDelta = 0 ;
		SString *	pstrDelta = mapParam.GetAs( L"delta" ) ;
		if ( pstrDelta != NULL )
		{
			nDelta = (int) pstrDelta->AsInteger() ;
		}
		SString *	pstrCode = mapParam.GetAs( L"code" ) ;
		if ( m_flagImage && (pstrCode != NULL) )
		{
			SGLScreenCapture::MouseEventCode	code =
				(SGLScreenCapture::MouseEventCode) pstrCode->AsInteger() ;
			bool	fEvent = true ;
			switch ( code )
			{
			case	SGLScreenCapture::mouseLeftDown:
			case	SGLScreenCapture::mouseRightDown:
			case	SGLScreenCapture::mouseMiddleDown:
			case	SGLScreenCapture::mouseXButton1Down:
			case	SGLScreenCapture::mouseXButton2Down:
				if ( m_hwndCapture != NULL )
				{
					fEvent = false ;
					//
					POINT	ptCursor ;
					RECT	rectWindow ;
					::GetCursorPos( &ptCursor ) ;
					if ( ::GetWindowRect( m_hwndCapture, &rectWindow ) )
					{
						fEvent = (rectWindow.left <= ptCursor.x)
								&& (ptCursor.x < rectWindow.right)
								&& (rectWindow.top <= ptCursor.y)
								&& (ptCursor.y < rectWindow.bottom) ;
					}
				}
				break ;
			}
			if ( fEvent )
			{
				m_scrnCapture.MouseEvent( code, nDelta ) ;
			}
		}
		return ;
	}
	else if ( strCmd == L"frame_rate" )
	{
		SString *	pstrBytes = mapParam.GetAs( L"bytes" ) ;
		SString *	pstrDuration = mapParam.GetAs( L"duration" ) ;
		if ( pstrBytes && pstrDuration )
		{
			m_bytesIFrameSend = (uint32_t) pstrBytes->AsInteger() ;
			m_msecIFrameSend = (uint32_t) pstrDuration->AsInteger() ;
			ESLTrace( "frame rate %d / %d [bytes/ms]\n",
							m_bytesIFrameSend, m_msecIFrameSend ) ;
			if ( m_msecIFrameSend > 1000 )
			{
				m_msecIFrameSend = 1000 ;
			}
			if ( m_msecIFrameSend <= 0 )
			{
				m_msecIFrameSend = m_msecFrameInterval / 2 ;
			}
			if ( m_msecIFrameSend > 0 )
			{
				size_t	nTargetBytes =
					m_bytesIFrameSend * m_msecFrameInterval
											* 6 / m_msecIFrameSend ;
				m_bytesTargetIFrame = (uint32_t) nTargetBytes ;
				//
				uint32_t	bytesRate =
					m_bytesIFrameSend * 1000 / m_msecIFrameSend ;
				if ( bytesRate > m_bytesBestSendRate )
				{
					m_bytesBestSendRate = bytesRate ;
				}
				nTargetBytes =
					m_bytesBestSendRate * 2 * m_msecFrameInterval / 1000 ;
				if ( nTargetBytes > m_bytesTargetIFrame )
				{
					m_bytesTargetIFrame =
						(uint32_t) ((m_bytesTargetIFrame + nTargetBytes) >> 1) ;
				}
			}
			m_bytesSendMinLimitRate = 166 * 1024 ;
		}
		SString *	pstrReqBlock = mapParam.GetAs( L"req_iblock" ) ;
		size_t	nMaskBytes = 0 ;
		if ( (pstrReqBlock != NULL)
			&& ((nMaskBytes = (size_t) pstrReqBlock->AsInteger()) != 0) )
		{
			SArray<uint8_t>	buf ;
			uint8_t *	pbytBuf = buf.GetArray( nMaskBytes ) ;
			size_t		nRecvBytes = 0 ;
			while ( nRecvBytes < nMaskBytes )
			{
				if ( m_socketTCP.WaitToReceive( 10 ) == sglErrTimeout )
				{
					if ( m_socketTCP.WaitToClose(0) == sglErrSuccess )
					{
						break ;
					}
				}
				else
				{
					nRecvBytes +=
						m_socketTCP.Receive
							( pbytBuf + nRecvBytes,
								nMaskBytes - nRecvBytes ) ;
				}
			}
			buf.FinishArray() ;
			//
			uint8_t *	pbytReq ;
			size_t		nLength = nRecvBytes << 3 ;
			m_csSyncFrame.Lock() ;
			//
			m_bufVideoReqIFrame.SetLength( nRecvBytes * 8 ) ;
			pbytReq = m_bufVideoReqIFrame.GetArray() ;
			for ( size_t i = 0; i < nRecvBytes; i ++ )
			{
				uint8_t	m = 0x80 ;
				uint8_t	b = pbytBuf[i] ;
				size_t	j = i * 8 ;
				for ( size_t k = 0; k < 8; k ++ )
				{
					pbytReq[j + k] = b & m ;
					m >>= 1 ;
				}
			}
			m_bufVideoReqIFrame.FinishArray() ;
			//
			m_csSyncFrame.Unlock() ;
		}
	}
	else if ( strCmd == L"update_video_param" )
	{
		SString *	pstrLimIBlock = mapParam.GetAs( L"lim_i_block" ) ;
		if ( pstrLimIBlock != NULL )
		{
			m_limitMaxDiffBlock = (uint32_t) pstrLimIBlock->AsInteger() ;
		}
		SString *	pstrThrsIBlock = mapParam.GetAs( L"threshold_i_block" ) ;
		if ( pstrThrsIBlock != NULL )
		{
			m_limitMaxDiffThreshold = (uint32_t) pstrThrsIBlock->AsInteger() ;
		}
		SString *	pstrLimDiff = mapParam.GetAs( L"lim_diff_block" ) ;
		if ( pstrLimDiff != NULL )
		{
			m_limitMinDiffBlock = (uint32_t) pstrLimDiff->AsInteger() ;
		}
		SString *	pstrInterval = mapParam.GetAs( L"frame_interval" ) ;
		if ( pstrInterval != NULL )
		{
			m_msecFrameInterval = (uint32_t) pstrInterval->AsInteger() ;
		}
		SString *	pstrRefresh = mapParam.GetAs( L"refresh_interval" ) ;
		if ( pstrRefresh != NULL )
		{
			m_msecRefreshInterval = (uint32_t) pstrRefresh->AsInteger() ;
		}
		SendMessageLine( L"100 successed to update video param." ) ;
	}
	else if ( strCmd == L"update_video_dct" )
	{
		SString *	pstrIndex = mapParam.GetAs( L"index" ) ;
		if ( pstrIndex != NULL )
		{
			UpdateDCTParam( (int) pstrIndex->AsInteger() ) ;
		}
		else
		{
			SendMessageLine( L"900 failed to update DCT param." ) ;
		}
	}
	else if ( strCmd == L"video_scale" )
	{
		SString *	pstrScale = mapParam.GetAs( L"scale" ) ;
		if ( pstrScale != NULL )
		{
			m_csSyncFrame.Lock() ;
			m_nImageScale = (int) pstrScale->AsInteger() ;
			if ( m_nImageScale > 1 )
			{
				m_nImageScale = 1 ;
			}
			m_csSyncFrame.Unlock() ;
		}
		SendMessageLine( L"100 successed to update video scale." ) ;
	}
	else if ( strCmd == L"begin_sound" )
	{
		int	nRate = 40000 ;
		int	nBufSamples = 0 ;
		int	nFrequency = 22050 ;
		int	nChannels = 2 ;
		//
		SString *	pstrFreq = mapParam.GetAs( L"freq" ) ;
		if ( pstrFreq != NULL )
		{
			nFrequency = (int) pstrFreq->AsInteger() ;
		}
		SString *	pstrChannels = mapParam.GetAs( L"ch" ) ;
		if ( pstrChannels != NULL )
		{
			nChannels = (int) pstrChannels->AsInteger() ;
		}
		SString *	pstrRate = mapParam.GetAs( L"rate" ) ;
		if ( pstrFreq != NULL )
		{
			nRate = (int) pstrRate->AsInteger() ;
		}
		SString *	pstrBuffer = mapParam.GetAs( L"buf" ) ;
		if ( pstrBuffer != NULL )
		{
			nBufSamples = (int) pstrBuffer->AsInteger() ;
		}
		if ( nBufSamples <= 0 )
		{
			nBufSamples = 1024 ;
		}
		if ( BeginSound( nRate, nBufSamples, nFrequency, nChannels ) )
		{
			SendMessageLine( L"100 successed to begin sound." ) ;
		}
		else
		{
			SendMessageLine( L"900 failed to begin sound." ) ;
		}
	}
	else if ( strCmd == L"end_sound" )
	{
		if ( EndSound() )
		{
			SendMessageLine( L"100 successed to end sound." ) ;
		}
		else
		{
			SendMessageLine( L"900 failed to end sound." ) ;
		}
	}
	else if ( strCmd == L"begin_video" )
	{
		if ( BeginVideo() )
		{
			SendMessageLine( L"100 successed to begin video." ) ;
		}
		else
		{
			SendMessageLine( L"900 failed to begin video." ) ;
		}
	}
	else if ( strCmd == L"end_video" )
	{
		if ( EndVideo() )
		{
			SendMessageLine( L"100 successed to end video." ) ;
		}
		else
		{
			SendMessageLine( L"900 failed to end video." ) ;
		}
	}
	else if ( strCmd == L"query_window_list" )
	{
		SendMessageLine( L"100 return window list." ) ;
		//
		m_lstQueryWindow.RemoveAll() ;
		//
		SArray<SGLDisplayMode::MonitorHandle>	lstMonitors ;
		SGLDisplayMode	dispMode ;
		dispMode.EnumerateDisplay( lstMonitors ) ;
		if ( lstMonitors.GetLength() > 0 )
		{
			for ( size_t i = 0; i < lstMonitors.GetLength(); i ++ )
			{
				SGLImageRect	rectMonitor, rectVirtual ;
				if ( !dispMode.GetMonitorRect
					( lstMonitors.At(i), rectMonitor, rectVirtual ) )
				{
					SString	strDispName ;
					strDispName = L"Display #" ;
					strDispName += SString( i + 1 ) ;
					//
					WindowEntry	we ;
					we.hwnd = /*::GetDesktopWindow()*/ NULL ;
					we.rectDisplay = rectMonitor ;
					m_lstQueryWindow.Add( we ) ;
					SendMessageLine( strDispName ) ;
					//
					ESLTrace( "Display #%d : %d, %d, %dx%d\n",
						i + 1, rectMonitor.x, rectMonitor.y, rectMonitor.w, rectMonitor.h ) ;
				}
			}
		}
		else
		{
			WindowEntry	we ;
			we.hwnd = NULL ;
			we.rectDisplay.x = 0 ;
			we.rectDisplay.y = 0 ;
			we.rectDisplay.w = ::GetSystemMetrics( SM_CXSCREEN ) ;
			we.rectDisplay.h = ::GetSystemMetrics( SM_CYSCREEN ) ;
			m_lstQueryWindow.Add( we ) ;
			SendMessageLine( L"Display #1" ) ;
		}
		HWND	hWnd = ::GetTopWindow( NULL ) ;
		while ( hWnd != NULL )
		{
			LONG	lStyle = GetWindowLong( hWnd, GWL_STYLE ) ;
			if ( (lStyle & WS_VISIBLE)
				/*&& (lStyle & WS_SYSMENU)*/
				&& !(lStyle & (/*WS_POPUP |*/ WS_CHILD /*| WS_MINIMIZE*/)) )
			{
				char	szBuf[0x100] ;
				int	nLen = ::GetWindowText( hWnd, szBuf, 0x100 ) ;
				if ( (nLen > 0) && (nLen < 0x100) )
				{
					SString	strName( szBuf, nLen ) ;
					ssize_t	iLine = strName.Find( L'\n' ) ;
					if ( iLine >= 0 )
					{
						strName = strName.Left( (size_t) iLine ) ;
					}
					SString	strWndName = L"Window: " ;
					strWndName += strName ;
					//
					WindowEntry	we ;
					we.hwnd = hWnd ;
					m_lstQueryWindow.Add( we ) ;
					SendMessageLine( strWndName ) ;
				}
			}
			hWnd = ::GetNextWindow( hWnd, GW_HWNDNEXT ) ;
		}
		SendMessageLine( L"" ) ;
	}
	else if ( strCmd == L"select_window" )
	{
		SString *	pstrIndex = mapParam.GetAs( L"index" ) ;
		bool		fSuccess = false ;
		if ( pstrIndex != NULL )
		{
			size_t	nIndex = (size_t) pstrIndex->AsInteger() ;
			if ( nIndex < m_lstQueryWindow.GetLength() )
			{
				WindowEntry *	pwe = m_lstQueryWindow.GetAt( nIndex ) ;
				if ( pwe )
				{
					m_csSyncFrame.Lock() ;
					m_scrnCapture.SetCaptureTarget( pwe->hwnd, false ) ;
					m_scrnCapture.SetCapturePosition
						( pwe->rectDisplay.x, pwe->rectDisplay.y ) ;
					m_scrnCapture.SetCaptureSize
						( pwe->rectDisplay.w, pwe->rectDisplay.h ) ;
					m_hwndCapture = pwe->hwnd ;
					m_rectCapture = pwe->rectDisplay ;
					m_csSyncFrame.Unlock() ;
					ESLTrace( "select window : %d, %d, %dx%d\n",
						m_rectCapture.x, m_rectCapture.y, m_rectCapture.w, m_rectCapture.h ) ;
					//
					LONG	lStyle = GetWindowLong( m_hwndCapture, GWL_STYLE ) ;
					if ( lStyle & WS_MINIMIZE )
					{
						::ShowWindow( m_hwndCapture, SW_SHOWNORMAL ) ;
					}
					::SetForegroundWindow( m_hwndCapture ) ;
					fSuccess = true ;
				}
			}
		}
		if ( fSuccess )
		{
			SendMessageLine( L"100 successed." ) ;
		}
		else
		{
			SendMessageLine( L"900 failed." ) ;
		}
	}
}

// 音声ストリーミング開始
//////////////////////////////////////////////////////////////////////////////
bool SGLMediaStreamingServer::Instance::BeginSound
	( int nRate, int nBufSamples, int nFrequency, int nChannels )
{
	if ( m_flagSound )
	{
		return	false ;
	}
	//
	// 録音デバイス選択
	//
	SArray<uint16_t>	bufNameBuf ;
	uint16_t *	pwNameBuf = bufNameBuf.GetArray( 0x10000 ) ;
	size_t		nDevCount = m_recSound.EnumerateDevices( pwNameBuf, 0x10000 ) ;
	size_t		iDevRec = 0 ;
	for ( size_t i = 0; i < nDevCount; i ++ )
	{
		SString	strDevName = pwNameBuf ;
		if ( strDevName == m_pmss->m_strSoundRecDevice )
		{
			iDevRec = i ;
			break ;
		}
		pwNameBuf += strDevName.GetLength() + 1 ;
	}
	bufNameBuf.FinishArray() ;
	//
	m_fmtSound.format = formatSoundLinearPCM ;
	m_fmtSound.frequency = (uint32_t) nFrequency ;
	m_fmtSound.channels = (uint32_t) nChannels ;
	m_fmtSound.bitsPerSample = 16 ;
	//
	// 圧縮設定
	//
	static const int	nPresetRate[9] =
	{
		235, 176, 156, 141, 128, 117, 94, 78, 70
	} ;
	static const ERISA::SGLSoundEncoder::PresetParameter	nPresetIndex[9] =
	{
		ERISA::SGLSoundEncoder::ppVBR235kbps,
		ERISA::SGLSoundEncoder::ppVBR176kbps,
		ERISA::SGLSoundEncoder::ppVBR156kbps,
		ERISA::SGLSoundEncoder::ppVBR141kbps,
		ERISA::SGLSoundEncoder::ppVBR128kbps,
		ERISA::SGLSoundEncoder::ppVBR117kbps,
		ERISA::SGLSoundEncoder::ppVBR94kbps,
		ERISA::SGLSoundEncoder::ppVBR78kbps,
		ERISA::SGLSoundEncoder::ppVBR70kbps,
	} ;
	ERISA::SGLSoundEncoder::PresetParameter
				ppIndex = ERISA::SGLSoundEncoder::ppVBR70kbps ;
	int	nRateScale = nFrequency * nChannels * 0x100 / (44100 * 2) ;
	for ( size_t i = 0; i < 9; i ++ )
	{
		if ( nRate >= nPresetRate[i] * nRateScale / 0x100 )
		{
			ppIndex = nPresetIndex[i] ;
			break ;
		}
	}
	ERISA::SGLSoundEncoder::Parameter	encParam ;
	eslFillMemory( &m_mioHeader, 0, sizeof(ERISA::MIO_INFO_HEADER) ) ;
	m_mioHeader.dwChannelCount = m_fmtSound.channels ;
	m_mioHeader.dwSamplesPerSec = m_fmtSound.frequency ;
	m_mioHeader.dwBitsPerSample = m_fmtSound.bitsPerSample ;
	encParam.LoadPresetParam( ppIndex, m_mioHeader ) ;
	m_mioHeader.dwArchitecture = ERISA::erisaRunlengthGamma ;
	//
	if ( m_encSound.Initialize( m_mioHeader ) )
	{
		return	false ;
	}
	m_encSound.SetCompressionParameter( encParam ) ;
	//
	size_t	nSubbandDegree = ((size_t) 1 << m_mioHeader.dwSubbandDegree) ;
	m_nSoundInterval = ((size_t) nBufSamples + nSubbandDegree - 1) ;
	m_nSoundInterval -= (m_nSoundInterval % nSubbandDegree) ;
	if ( m_nSoundInterval == 0 )
	{
		m_nSoundInterval = nSubbandDegree ;
	}
	m_flagKeySound = true ;
//	m_nSoundPacketID = 0 ;
	m_nSoundStreamSamples = 0 ;
	//
	// 録音開始
	//
	if ( m_recSound.Open( iDevRec, m_fmtSound ) )
	{
		return	false ;
	}
	if ( m_recSound.PrepareStream() )
	{
		return	false ;
	}
	if ( m_recSound.Start() )
	{
		return	false ;
	}
	m_flagSound = true ;
	m_queSoundData.RemoveAll() ;
	//
	m_threadSound.BeginThread( new SoundProcedure( this ) ) ;
	return	true ;
}

// 音声ストリーミング終了
//////////////////////////////////////////////////////////////////////////////
bool SGLMediaStreamingServer::Instance::EndSound( void )
{
	if ( m_flagSound )
	{
		m_flagSound = false ;
		m_threadSound.Wait() ;
		m_threadSound.Delete() ;
		m_recSound.Stop() ;
		m_queSoundData.RemoveAll() ;
	}
	return	true ;
}

// 映像DCT係数設定
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::Instance::UpdateDCTParam( int nIndex )
{
	size_t	i = 0 ;
	while ( !m_flagAbort )
	{
		SString	strLine ;
		if ( !RecvMessageLine( strLine ) )
		{
			SStringParser	sparsLine ;
			sparsLine.AttachString( strLine ) ;
			//
			while ( sparsLine.PassSpace() && (i < 64) )
			{
				int	type = sparsLine.IsNextNumber() ;
				if ( type == SStringParser::numberInvalid )
				{
					break ;
				}
				m_fxDCTParam[nIndex][i ++] =
						(int16_t) sparsLine.NextInteger( type ) ;
			}
			SendMessageLine( L"100 successed to update DCT param." ) ;
			break ;
		}
		if ( m_socketTCP.WaitToClose( 10 ) == errSuccess )
		{
			break ;
		}
	}
}

// 映像ストリーミング開始
//////////////////////////////////////////////////////////////////////////////
bool SGLMediaStreamingServer::Instance::BeginVideo( void )
{
	if ( !m_flagImage )
	{
		m_flagImage = true ;
		m_flagShouldIFrame = true ;
	//	m_idFrameID = 1 ;
		m_flagIFrame = false ;
		m_bytesTargetIFrame = 0x07FFFFFF ;
		m_nPScaleLastIFrame = 0 ;
		//
		#if	defined(__PLATFORM_WINDOWS__)
		m_hLastCursor = NULL ;
		#endif
		//
		m_timerLastFrame.Reset() ;
		m_timerLastKeyFrame.Reset() ;
		m_timerLastCursor.Reset() ;
		m_queVideoData.RemoveAll() ;
		//
		m_bytesIFrameSend = 0 ;
		m_msecIFrameSend = 0 ;
		//
		m_idFrameID ++ ;
		m_idCursorID ++ ;
		m_threadImage.BeginThread( new VideoProcedure( this ) ) ;
	}
	return	true ;
}

// 映像ストリーミング終了
//////////////////////////////////////////////////////////////////////////////
bool SGLMediaStreamingServer::Instance::EndVideo( void )
{
	if ( m_flagImage )
	{
		m_flagImage = false ;
		m_threadImage.Wait() ;
		m_threadImage.Delete() ;
		m_queVideoData.RemoveAll() ;
		m_idFrameID ++ ;
		m_idCursorID ++ ;
	}
	return	true ;
}

// TCP/IP で1行送信
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingServer::Instance::SendMessageLine( const wchar_t * pwszMsg )
{
	SArray<uint8_t>	bufMsg ;
	Charset::Encode( bufMsg, Charset::encodingUTF8, pwszMsg ) ;
	if ( (bufMsg.GetLength() == 0)
		|| (bufMsg.At(bufMsg.GetLength() - 1) != '\n') )
	{
		bufMsg.Add( (uint8_t) '\n' ) ;
	}
	m_socketTCP.Write( bufMsg.GetConstArray(), bufMsg.GetLength() ) ;
}

// TCP/IP で1行受信
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaStreamingServer::Instance::RecvMessageLine( SSystem::SString& strLine )
{
	uint8_t	bufTemp[0x100] ;
	size_t	nLine = m_socketTCP.ReadLine( &bufTemp[0], 0x100 ) ;
	m_bufLine.AddArray( &bufTemp[0], nLine ) ;
	//
	if ( (nLine != 0) && (bufTemp[nLine - 1] == '\n') )
	{
		Charset::Decode
			( strLine, Charset::encodingUTF8,
				m_bufLine.GetConstArray(), (ssize_t) m_bufLine.GetLength() ) ;
		m_bufLine.RemoveAll() ;
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

#endif


//////////////////////////////////////////////////////////////////////////////
// ストリーミング・クライアント・リスナ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLMediaStreamingClient::Listener, ESLObject ) ;

// サウンド出力準備
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingClient::Listener::PrepareSoundStream( const SGLSoundFormat& fmt )
{
}

// サウンドストリーム
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingClient::Listener::OnSoundStream
	( const void * ptrWavePCM, size_t nBytes, bool fHeader )
{
}

// 映像ストリーム
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingClient::Listener::OnVideoUpdate( SGLImage& imgFrame )
{
}

// マウスカーソル
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingClient::Listener::OnMouseCursor
	( const SGLMediaStreamingClient::CursorDataHeader& hdrCursor, SGLImage * pimgCursor )
{
}

// 切断した
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingClient::Listener::OnDisconnected( void )
{
}


//////////////////////////////////////////////////////////////////////////////
// 映像デコードスレッド
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLMediaStreamingClient::VideoDecoderThread, SProcedure )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLMediaStreamingClient::VideoDecoderThread::VideoDecoderThread
	( SGLMediaStreamingClient * pmsc, bool flagPrimary )
{
	m_pmsc = pmsc ;
	m_flagPrimary = flagPrimary ;
	m_flagExit = false ;
	//
	m_thread.BeginThread( this ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLMediaStreamingClient::VideoDecoderThread::~VideoDecoderThread( void )
{
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingClient::VideoDecoderThread::Run( void )
{
	while ( !m_flagExit )
	{
		if ( m_pmsc->m_signRecvVideoFrames.Wait( 10 ) == errSuccess )
		{
			UDPRecvData *	pData ;
			m_pmsc->m_csSync.Lock() ;
			if ( !m_flagPrimary )
			{
				pData = m_pmsc->m_queRecvVideoFrames.GetAt( 0 ) ;
				if ( pData != NULL )
				{
					RecvUDPHeader *	pHeader = pData->GetHeader() ;
					if ( pHeader->idSerial != m_pmsc->m_idVideoPacketID )
					{
						m_pmsc->m_csSync.Unlock() ;
						SleepMilliSec( 1 ) ;
						continue ;
					}
				}
			}
			pData = m_pmsc->m_queRecvVideoFrames.DetachAt( 0 ) ;
			if ( m_pmsc->m_queRecvVideoFrames.GetLength() == 0 )
			{
				m_pmsc->m_signRecvVideoFrames.ResetSignal() ;
			}
			m_pmsc->m_csSync.Unlock() ;
			//
			if ( pData != NULL )
			{
				m_pmsc->OnStreamVideo( pData->GetHeader(), m_flagPrimary ) ;
				delete	pData ;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
// ストリーミング・クライアント
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLMediaStreamingClient, ESLObject, SProcedure )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLMediaStreamingClient::SGLMediaStreamingClient( void )
{
	m_flagConnectTCP = false ;
	m_flagConnectUDP = false ;
	m_flagThread = false ;
	m_initDecSound = false ;
	m_flagKeySound = false ;
	m_idSoundPacketID = 0 ;
	m_idVideoPacketID = 0 ;
	m_flagVideoIFrame = false ;
	m_flagPostViewUpdate = false ;
	m_bytesLastIFrame = 0 ;
	m_msecLastIFrameDuration = 0 ;
	m_flagBlockReqIFrame = false ;
	m_countDecodingVideo = 0 ;
	m_idCursorPacketID = 0 ;
	//
	for ( int i = 0; i < 4; i ++ )
	{
		for ( int j = 0; j < 64; j ++ )
		{
			m_fxDCTParam[i][j] = 0x20 ;
			m_fxIDCTParam[i][j] = 0x10000 / 0x20 ;
		}
	}
	//
	m_pListener = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLMediaStreamingClient::~SGLMediaStreamingClient( void )
{
	Close() ;
}

// リスナ設定
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingClient::AttachListener
		( SGLMediaStreamingClient::Listener * pListener )
{
	m_pListener = pListener ;
}

// 接続
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaStreamingClient::Connect
	( const wchar_t * pwszHostAddr,
		const wchar_t * pwszPassword,
		uint32_t nTCPPort, uint32_t nUDPPort )
{
	Close() ;
	//
	// 接続
	//
	m_socketTCP.Create() ;
	if ( m_socketTCP.Connect( pwszHostAddr, nTCPPort ) )
	{
		return	sglErrFailed ;
	}
	//
	// ログイン
	//
	SString		strHelloLine ;
	SGLError	err = ResponseToError( RecvTCPResponse( strHelloLine ) ) ;
	if ( err )
	{
		m_socketTCP.Close() ;
		return	err ;
	}
	SStringParser	sparsHelloLine ;
	sparsHelloLine.AttachString( strHelloLine ) ;
	if ( !sparsHelloLine.HasToComeToken( L"hello" ) )
	{
		m_socketTCP.Close() ;
		return	sglErrFailed ;
	}
	SString	strLoginToken = sparsHelloLine.GetString() ;
	//
	SArray<uint8_t>	bufLoginToken ;
	Charset::DecodeBase64( bufLoginToken, strLoginToken ) ;
	//
	SArray<uint8_t>	bufPass ;
	Charset::Encode
		( bufPass, Charset::encodingUTF8, pwszPassword ) ;
	//
	MD5Context	md5 ;
	md5.Stream( bufPass.GetConstArray(), bufPass.GetLength() ) ;
	md5.Stream( bufLoginToken.GetConstArray(), bufLoginToken.GetLength() ) ;
	md5.Flush() ;
	//
	SString	strHexDigest ;
	md5.GetMD5DigestHex( strHexDigest ) ;
	//
	SString	strPassCmd = L"pass " ;
	strPassCmd += strHexDigest ;
	strPassCmd += L"\n" ;
	//
	m_socketTCP.WriteEncodedString( strPassCmd, Charset::encodingUTF8 ) ;
	//
	SString	strMsg ;
	err = ResponseToError( RecvTCPResponse( strMsg ) ) ;
	if ( err )
	{
		m_socketTCP.Close() ;
		return	err ;
	}
	m_flagConnectTCP = true ;
	//
	// UDP 接続
	//
	m_socketUDP.Create( nUDPPort, SSocket::typeDatagram ) ;
	if ( m_socketUDP.Connect( pwszHostAddr, nUDPPort ) )
	{
		Close() ;
		return	sglErrFailed ;
	}
	//
	SXMLDocument	xmlCmdLogin ;
	SXMLDocument *	pxmlCmd ;
	xmlCmdLogin.SetTag( L"commands" ) ;
	pxmlCmd = new SXMLDocument ;
	pxmlCmd->SetTag( L"login" ) ;
	pxmlCmd->SetAttributeAs( L"token", strLoginToken ) ;
	xmlCmdLogin.AddElement( pxmlCmd ) ;
	//
	STimeCounter	timer ;
	for ( ; ; )
	{
		if ( SendUDPCommand( xmlCmdLogin ) )
		{
			Close() ;
			return	sglErrFailed ;
		}
		if ( m_socketUDP.Poll( SSocket::pollIn, 10 ) & SSocket::pollIn )
		{
			SArray<uint8_t>	bufData ;
			const size_t	nDataBufSize = 0x10000 ;
			uint8_t *		pbytData = bufData.GetArray( nDataBufSize ) ;
			size_t			nRecvBytes ;
			nRecvBytes = m_socketUDP.Receive( pbytData, nDataBufSize ) ;
			bufData.FinishArray() ;
			//
			if ( (nRecvBytes != 0)
				&& VerifyUDPData( pbytData, nRecvBytes ) )
			{
				break;
			}
		}
		if ( timer.GetTime() >= 10000 )
		{
			m_socketUDP.Close() ;
			Close() ;
			return	sglErrTimeout ;
		}
	}
	m_flagConnectUDP = true ;
	//
	// スレッド起動
	//
	m_idSoundPacketID = 0 ;
	m_idVideoPacketID = 0 ;
	m_idCursorPacketID = 0 ;
	m_bytesLastIFrame = 0 ;
	m_flagBlockReqIFrame = false ;
	//
	m_countDecodingVideo = 0 ;
	m_signFreeDecodingVideo.Initialize( true ) ;
	//
	m_signRecvVideoFrames.Initialize( false ) ;
	m_queRecvVideoFrames.RemoveAll() ;
	//
	m_eventExit.Initialize( false ) ;
	//
	if ( m_thread.BeginThread( this ) )
	{
		Close() ;
		return	sglErrFailed ;
	}
	m_flagThread = true ;
	//
	size_t	nThreads = GetLogicalProcessorCount() ;
	if ( nThreads > 2 )
	{
		nThreads -- ;
	}
	for ( size_t i = 0; i < nThreads; i ++ )
	{
		m_lstVideoThreads.Add( new VideoDecoderThread( this, (i == 0) ) ) ;
	}
	//
	return	sglErrSuccess ;
}

// 切断
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaStreamingClient::Close( void )
{
	for ( size_t i = 0; i < m_lstVideoThreads.GetLength(); i ++ )
	{
		VideoDecoderThread *	pThread = m_lstVideoThreads.GetAt( i ) ;
		if ( pThread != NULL )
		{
			pThread->m_flagExit = true ;
			pThread->m_thread.Wait() ;
		}
	}
	if ( m_flagThread )
	{
		m_eventExit.SetSignal() ;
		m_thread.Wait() ;
		m_signFreeDecodingVideo.Delete() ;
		m_signRecvVideoFrames.Delete() ;
		m_eventExit.Delete() ;
		m_thread.Delete() ;
		m_flagThread = false ;
	}
	m_lstVideoThreads.RemoveAll() ;
	//
	if ( m_flagConnectUDP )
	{
		m_socketUDP.Close() ;
		m_flagConnectUDP = false ;
	}
	if ( m_flagConnectTCP )
	{
		m_socketTCP.Close() ;
		m_flagConnectTCP = false ;
	}
	return	sglErrSuccess ;
}

// サウンドストリーミング開始
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaStreamingClient::BeginSound
	( int nRate, int nBufSamples, int nFrequency, int nChannels )
{
	if ( !m_flagConnectTCP )
	{
		return	sglErrFailed ;
	}
	m_idSoundPacketID = 0 ;
	m_flagSoundFirst = true ;
	//
	SString	strCmd ;
	strCmd.Format
		( L"begin_sound freq=%d ch=%d rate=%d buf=%d\n",
			nFrequency, nChannels, nRate, nBufSamples ) ;
	m_socketTCP.WriteEncodedString( strCmd, Charset::encodingUTF8 ) ;
	//
	SString	strMsg ;
	return	ResponseToError( RecvTCPResponse( strMsg ) ) ;
}

// サウンドストリーミング停止
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaStreamingClient::EndSound( void )
{
	if ( !m_flagConnectTCP )
	{
		return	sglErrFailed ;
	}
	m_socketTCP.WriteEncodedString
		( L"end_sound\n", -1, Charset::encodingUTF8 ) ;
	//
	SString	strMsg ;
	return	ResponseToError( RecvTCPResponse( strMsg ) ) ;
}

// 映像ストリーミング開始
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaStreamingClient::BeginVideo( void )
{
	if ( !m_flagConnectTCP )
	{
		return	sglErrFailed ;
	}
	m_idVideoPacketID = 0 ;
	m_idCursorPacketID = 0 ;

	m_socketTCP.WriteEncodedString
		( L"begin_video\n", -1, Charset::encodingUTF8 ) ;
	//
	SString	strMsg ;
	return	ResponseToError( RecvTCPResponse( strMsg ) ) ;
}

// 映像ストリーミング終了
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaStreamingClient::EndVideo( void )
{
	if ( !m_flagConnectTCP )
	{
		return	sglErrFailed ;
	}
	m_socketTCP.WriteEncodedString
		( L"end_video\n", -1, Charset::encodingUTF8 ) ;
	//
	SString	strMsg ;
	return	ResponseToError( RecvTCPResponse( strMsg ) ) ;
}

// 映像DCT係数設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaStreamingClient::UpdateDCTParam( int nIndex, const int * pParamDCT )
{
	if ( !m_flagConnectTCP )
	{
		return	sglErrFailed ;
	}
	SString	strCmdLines = L"update_video_dct index=" ;
	strCmdLines += SString(nIndex) ;
	strCmdLines += "\n" ;
	//
	for ( int i = 0; i < 64; i ++ )
	{
		strCmdLines += SString( pParamDCT[i] ) ;
		strCmdLines += L" " ;
	}
	strCmdLines += "\n" ;
	//
	m_socketTCP.WriteEncodedString( strCmdLines, Charset::encodingUTF8 ) ;
	//
	SString		strMsg ;
	SGLError	err = ResponseToError( RecvTCPResponse( strMsg ) ) ;
	if ( err )
	{
		return	err ;
	}
	for ( int i = 0; i < 64; i ++ )
	{
		m_fxDCTParam[nIndex][i] = pParamDCT[i] ;
		if ( pParamDCT[i] > 0 )
		{
			m_fxIDCTParam[nIndex][i] =
				((0x10000 + (pParamDCT[i] >> 1)) / pParamDCT[i]) ;
		}
		else
		{
			m_fxIDCTParam[nIndex][i] = 0 ;
		}
	}
	return	sglErrSuccess ;
}

// 映像パラメータ送信
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaStreamingClient::SendVideoParam
	( int msecKeyFrame, int msecFrame,
		int minDiffBlock, int maxDiffBlock, int thresholdDiffBlock )
{
	if ( !m_flagConnectTCP )
	{
		return	sglErrFailed ;
	}
	SString	strCmdLines = L"update_video_param refresh_interval=" ;
	strCmdLines += SString(msecKeyFrame) ;
	//
	if ( msecFrame > 0 )
	{
		strCmdLines += L" frame_interval=" ;
		strCmdLines += SString(msecFrame) ;
	}
	if ( minDiffBlock > 0 )
	{
		strCmdLines += L" lim_diff_block=" ;
		strCmdLines += SString(minDiffBlock) ;
	}
	if ( maxDiffBlock > 0 )
	{
		strCmdLines += L" lim_i_block=" ;
		strCmdLines += SString(maxDiffBlock) ;
	}
	if ( thresholdDiffBlock > 0 )
	{
		strCmdLines += L" threshold_i_block=" ;
		strCmdLines += SString(thresholdDiffBlock) ;
	}
	strCmdLines += "\n" ;
	//
	m_socketTCP.WriteEncodedString( strCmdLines, Charset::encodingUTF8 ) ;
	//
	SString	strMsg ;
	return	ResponseToError( RecvTCPResponse( strMsg ) ) ;
}

// 映像スケール設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaStreamingClient::SetVideoScale( int nScale )
{
	if ( !m_flagConnectTCP )
	{
		return	sglErrFailed ;
	}
	SString	strCmdLines = L"video_scale scale=" ;
	strCmdLines += SString(nScale) ;
	strCmdLines += "\n" ;
	//
	m_socketTCP.WriteEncodedString( strCmdLines, Charset::encodingUTF8 ) ;
	//
	SString	strMsg ;
	return	ResponseToError( RecvTCPResponse( strMsg ) ) ;
}

// ウィンドウリスト取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaStreamingClient::QueryWindowList
	( SSystem::SObjectArray<SSystem::SString>& lstWindowName )
{
	if ( !m_flagConnectTCP )
	{
		return	sglErrFailed ;
	}
	SString	strCmdLines = L"query_window_list\n" ;
	m_socketTCP.WriteEncodedString( strCmdLines, Charset::encodingUTF8 ) ;
	//
	SString		strMsg ;
	SGLError	err = ResponseToError( RecvTCPResponse( strMsg ) ) ;
	if ( err )
	{
		return	err ;
	}
	for ( ; ; )
	{
		SString		strLine ;
		SGLError	err = RecvResponseLine( strLine ) ;
		if ( err )
		{
			return	err ;
		}
		strLine.TrimRight() ;
		if ( strLine.IsEmpty() )
		{
			break ;
		}
		lstWindowName.Add( new SString( strLine ) ) ;
	}
	return	sglErrSuccess ;
}

// ウィンドウ選択
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaStreamingClient::SelectWindow( int iWindow )
{
	if ( !m_flagConnectTCP )
	{
		return	sglErrFailed ;
	}
	SString	strCmdLines = L"select_window index=" ;
	strCmdLines += SString( iWindow ) ;
	strCmdLines += "\n" ;
	//
	m_socketTCP.WriteEncodedString( strCmdLines, Charset::encodingUTF8 ) ;
	//
	SString	strMsg ;
	return	ResponseToError( RecvTCPResponse( strMsg ) ) ;
}

// マウス座標送信
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingClient::PostMousePoint( int xPos, int yPos, bool fDelta )
{
	if ( fDelta && (xPos == 0) && (yPos == 0) )
	{
		return ;
	}
	SString	strCmdLines ;
	if ( fDelta )
	{
		strCmdLines = L"mouse_move x=" ;
	}
	else
	{
		strCmdLines = L"mouse_pos x=" ;
	}
	strCmdLines += SString( xPos ) ;
	strCmdLines += L" y=" ;
	strCmdLines += SString( yPos ) ;
	strCmdLines += "\n" ;
	//
	m_socketTCP.WriteEncodedString( strCmdLines, Charset::encodingUTF8 ) ;
}

// キーイベント送信
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingClient::PostKeyboardEvent
	( int nVirtKey, int nFlags, bool fRelease )
{
	SString	strCmdLines ;
	if ( !fRelease )
	{
		strCmdLines = L"key_press code=" ;
	}
	else
	{
		strCmdLines = L"key_release code=" ;
	}
	strCmdLines += SString( nVirtKey ) ;
	strCmdLines += "\n" ;
	//
	m_socketTCP.WriteEncodedString( strCmdLines, Charset::encodingUTF8 ) ;
}

// マウスイベント送信
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingClient::PostMouseEvent( int code, int nDelta )
{
	SString	strCmdLines = L"mouse_event code=" ;
	strCmdLines += SString( code ) ;
	strCmdLines += L" delta=" ;
	strCmdLines += SString( nDelta ) ;
	strCmdLines += "\n" ;
	//
	m_socketTCP.WriteEncodedString( strCmdLines, Charset::encodingUTF8 ) ;
}

// TCP 送信待ち行列データバイト数取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLMediaStreamingClient::GetPendingPostQueueBytes( void ) const
{
	return	m_socketTCP.GetNotSentDataBytes() ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingClient::Run( void )
{
	SArray<uint8_t>	bufData ;
	const size_t	nDataBufSize = 0x10000 ;
	uint8_t *		pbytData = bufData.GetArray( nDataBufSize ) ;
	//
	for ( ; ; )
	{
		int64_t	msecTimeout = 1 ;
		while ( m_socketUDP.Poll
					( SSocket::pollIn, msecTimeout ) & SSocket::pollIn )
		{
			size_t	nRecvBytes ;
			nRecvBytes = m_socketUDP.Receive( pbytData, nDataBufSize ) ;
			if ( (nRecvBytes != 0)
				&& VerifyUDPData( pbytData, nRecvBytes ) )
			{
				RecvUDPHeader *	pHeader = (RecvUDPHeader*) pbytData ;
				if ( pHeader->typeData
							== SGLMediaStreamingServer::udpDataSound )
				{
					OnStreamSound( pHeader ) ;
				}
				else if ( pHeader->typeData
							== SGLMediaStreamingServer::udpDataImage )
				{
					AddVideoFrameQueue( pHeader ) ;
				}
				else if ( pHeader->typeData
							== SGLMediaStreamingServer::updDataCursor )
				{
					OnMouseCursor( pHeader ) ;
				}
			}
			msecTimeout = 0 ;
		}
		if ( m_socketTCP.WaitToClose( msecTimeout ) == errSuccess )
		{
			if ( m_pListener != NULL )
			{
				m_pListener->OnDisconnected() ;
			}
			break ;
		}
		if ( m_eventExit.Wait( 0 ) == errSuccess )
		{
			break ;
		}
		while ( m_queDelaySendUDP.GetLength() > 0 )
		{
			UDPSendData *	pUDPData = m_queDelaySendUDP.GetAt( 0 ) ;
			if ( pUDPData != NULL )
			{
				if ( m_socketUDP.Send
					( pUDPData->GetConstArray(), pUDPData->GetLength() ) == 0 )
				{
					break ;
				}
			}
			m_queDelaySendUDP.RemoveAt( 0 ) ;
		}
	}
	bufData.FinishArray() ;
}

// 映像フレームデータ追加
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingClient::AddVideoFrameQueue
	( SGLMediaStreamingClient::RecvUDPHeader * pHeader )
{
	UDPRecvData *	pData = new UDPRecvData ;
	pData->AddArray
		( (const uint8_t*) pHeader,
			pHeader->bytesData + sizeof(RecvUDPHeader) ) ;
	//
	m_csSync.Lock() ;
	if ( m_queRecvVideoFrames.GetLength() > 0 )
	{
		SGLMediaStreamingServer::UDPImageHeader *
			pImageHdr = (SGLMediaStreamingServer::UDPImageHeader*) (pHeader + 1) ;
		if ( !(pImageHdr->nFlags
				& SGLMediaStreamingServer::udpImageDifferential) )
		{
			UDPRecvData *	prdFirst = m_queRecvVideoFrames.GetAt( 0 ) ;
			uint64_t		idException = 0 ;
			if ( prdFirst != NULL )
			{
				RecvUDPHeader *	pudpHeader = prdFirst->GetHeader() ;
				if ( pudpHeader != NULL )
				{
					SGLMediaStreamingServer::UDPImageHeader *
						pFirstImageHdr =
							(SGLMediaStreamingServer::UDPImageHeader*) (pudpHeader + 1) ;
					if ( !(pFirstImageHdr->nFlags
							& SGLMediaStreamingServer::udpImageDifferential) )
					{
						idException = pudpHeader->idSerial ;
					}
				}
			}
			if ( idException < pHeader->idSerial )
			{
				size_t	nCount = m_queRecvVideoFrames.GetLength() ;
				for ( size_t i = 0; i < nCount; i ++ )
				{
					UDPRecvData *	prd = m_queRecvVideoFrames.GetAt( (size_t) i ) ;
					uint64_t	id = prd->GetHeader()->idSerial ;
					if ( (id != idException) && (id != pHeader->idSerial) )
					{
						m_queRecvVideoFrames.SetAt( i, NULL ) ;
					}
				}
				m_queRecvVideoFrames.TrimEmpty() ;
			}
		}
		ssize_t i = (ssize_t) m_queRecvVideoFrames.GetLength() - 1 ;
		while ( i >= 0 )
		{
			UDPRecvData *	prd = m_queRecvVideoFrames.GetAt( (size_t) i ) ;
			if ( prd->GetHeader()->idSerial <= pHeader->idSerial )
			{
				break ;
			}
			i -- ;
		}
		m_queRecvVideoFrames.InsertAt( (size_t) (i + 1), pData ) ;
		//
		/*
		if ( m_timerTrace.GetTime() > 1000 )
		{
			Trace( "queue video frames %d\n", m_queRecvVideoFrames.GetLength() ) ;
			m_timerTrace.Reset() ;
		}
		*/
	}
	else
	{
		m_queRecvVideoFrames.Add( pData ) ;
	}
	m_signRecvVideoFrames.SetSignal() ;
	m_csSync.Unlock() ;
}

// TCP レスポンス受信
//////////////////////////////////////////////////////////////////////////////
int SGLMediaStreamingClient::RecvTCPResponse
		( SSystem::SString& strMsg, uint32_t msecTimeout )
{
	SString	strLine ;
	if ( RecvResponseLine( strLine, msecTimeout ) )
	{
		return	responseTimeout ;
	}
	SStringParser	sparsLine ;
	sparsLine.AttachString( strLine ) ;
	int	type = sparsLine.IsNextNumber() ;
	if ( type == SStringParser::numberInvalid )
	{
		return	responseInvalid ;
	}
	int	num = (int) sparsLine.NextInteger( type ) ;
	sparsLine.PassSpace() ;
	strMsg = sparsLine.SubString( sparsLine.GetIndex() ) ;
	return	num ;
}

// TCP １行受信
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaStreamingClient::RecvResponseLine
	( SSystem::SString& strLine, uint32_t msecTimeout )
{
	SArray<uint8_t>	bufMsg ;
	uint8_t			buf[0x100] ;
	STimeCounter	timer ;
	for ( ; ; )
	{
		size_t	nLine = m_socketTCP.ReadLine( &buf[0], 0x100 ) ;
		bufMsg.AddArray( &buf[0], nLine ) ;
		if ( (nLine > 0) && (buf[nLine - 1] == '\n') )
		{
			break ;
		}
		if ( m_socketTCP.WaitToClose(0) == errSuccess )
		{
			return	sglErrAbort ;
		}
		if ( timer.GetTime() > msecTimeout )
		{
			return	sglErrTimeout ;
		}
		m_socketTCP.WaitToReceive( 100 ) ;
	}
	Charset::Decode
		( strLine, Charset::encodingUTF8,
			bufMsg.GetConstArray(), (ssize_t) bufMsg.GetLength() ) ;
	return	sglErrSuccess ;
}

// TCP レスポンスをエラーコードに変換
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaStreamingClient::ResponseToError( int nResCode )
{
	switch ( nResCode )
	{
	case	responseSuccess:
		return	sglErrSuccess ;
	case	responseError:
		return	sglErrFailed ;
	case	responseTimeout:
		return	sglErrTimeout ;
	}
	return	sglErrInvalidParam ;
}

// UDP コマンド送信
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMediaStreamingClient::SendUDPCommand( SSystem::SXMLDocument& xmlCmd )
{
	SArray<uint8_t>	bufData ;
	MakeUDPCommand( bufData, xmlCmd ) ;
	//
	if ( m_socketUDP.Send( bufData.GetConstArray(), bufData.GetLength() ) == 0 )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// UDP コマンド生成
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingClient::MakeUDPCommand
	( SSystem::SArray<uint8_t>& bufData, SSystem::SXMLDocument& xmlCmd )
{
	SSmartBuffer	sbuf ;
	xmlCmd.WriteDocument( sbuf ) ;
	//
	SByteBuffer	bbuf ;
	sbuf.Seek( 0 ) ;
	bbuf.ReadFromFile( sbuf ) ;
	//
	SendUDPHeader	hdr ;
	hdr.nBodyBytes = (uint32_t) bbuf.GetLength() ;
	//
	SakuraCL::CRC32Context	crc32 ;
	crc32.Stream( bbuf.GetConstArray(), hdr.nBodyBytes ) ;
	//
	hdr.nBodyCRC32 = crc32.GetCRC32() ;
	//
	bufData.AddArray( (const uint8_t*) &hdr, sizeof(SendUDPHeader) ) ;
	bufData.AddArray( bbuf.GetConstArray(), hdr.nBodyBytes ) ;
}

// UDP 受信データ検証
//////////////////////////////////////////////////////////////////////////////
bool SGLMediaStreamingClient::VerifyUDPData
	( const uint8_t * pbytData, size_t nDataBytes )
{
	RecvUDPHeader *	pHeader = (RecvUDPHeader*) pbytData ;
	if ( nDataBytes < sizeof(RecvUDPHeader) )
	{
		return	false ;
	}
	if ( nDataBytes != pHeader->bytesData + sizeof(RecvUDPHeader) )
	{
		return	false ;
	}
//	SakuraCL::CRC32Context	crc32 ;
//	crc32.Stream( pbytData + sizeof(RecvUDPHeader), pHeader->bytesData ) ;
//	return	(pHeader->nCRC32 == crc32.GetCRC32()) ;
	return	true ;
}

// 音声ストリームデータ処理
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingClient::OnStreamSound
		( SGLMediaStreamingClient::RecvUDPHeader * pHeader )
{
	//
	// 受信通知
	//
	SXMLDocument	xmlCmds ;
	SXMLDocument *	pxmlCmd ;
	xmlCmds.SetTag( L"commands" ) ;
	pxmlCmd = new SXMLDocument ;
	pxmlCmd->SetTag( L"sent_sound" ) ;
	pxmlCmd->SetAttrHexIntegerAs( L"id", pHeader->idSerial ) ;
	xmlCmds.AddElement( pxmlCmd ) ;
	//
	if ( SendUDPCommand( xmlCmds ) )
	{
		UDPSendData *	pUDPData = new UDPSendData ;
		MakeUDPCommand( *pUDPData, xmlCmds ) ;
		m_queDelaySendUDP.Add( pUDPData ) ;
	}
	//
	if ( m_idSoundPacketID >= pHeader->idSerial )
	{
		m_flagKeySound = true ;
		return ;
	}
	//
	// フォーマット判定
	//
	ERISA::MIO_DATA_HEADER *
					pmdh = (ERISA::MIO_DATA_HEADER*) (pHeader + 1) ;
	const void *	ptrData = NULL ;
	size_t			nSrcBytes =
						pHeader->bytesData- sizeof(ERISA::MIO_DATA_HEADER) ;
	//
	if ( !m_initDecSound || m_flagKeySound
		|| (pmdh->bytFlags & ERISA::mioDataLeadBlock) )
	{
		if ( !(pmdh->bytFlags & ERISA::mioDataLeadBlock) )
		{
			return ;
		}
		ERISA::MIO_INFO_HEADER *
			pmih = (ERISA::MIO_INFO_HEADER*) (pmdh + 1) ;
		bool	fPrepareFormat = false ;
		if ( !m_initDecSound )
		{
			if ( m_decSound.Initialize( *pmih ) )
			{
				return ;
			}
			m_initDecSound = true ;
			m_mioHeader = *pmih ;
			fPrepareFormat = true ;
		}
		else
		{
			if ( (m_mioHeader.fdwTransformation != pmih->fdwTransformation)
				|| (m_mioHeader.dwArchitecture != pmih->dwArchitecture)
				|| (m_mioHeader.dwChannelCount != pmih->dwChannelCount)
				|| (m_mioHeader.dwSamplesPerSec != pmih->dwSamplesPerSec)
				|| (m_mioHeader.dwBitsPerSample != pmih->dwBitsPerSample)
				|| (m_mioHeader.dwSubbandDegree != pmih->dwSubbandDegree) )
			{
				if ( !m_decSound.Initialize( *pmih ) )
				{
					m_initDecSound = false ;
					return ;
				}
				m_mioHeader = *pmih ;
				fPrepareFormat = true ;
			}
		}
		if ( fPrepareFormat && m_pListener )
		{
			SGLSoundFormat	fmt ;
			fmt.format = formatSoundLinearPCM ;
			fmt.frequency = (uint32_t) pmih->dwSamplesPerSec ;
			fmt.channels = (uint32_t) pmih->dwChannelCount ;
			fmt.bitsPerSample = (uint32_t) pmih->dwBitsPerSample ;
			//
			m_pListener->PrepareSoundStream( fmt ) ;
		}
		ptrData = (pmih + 1) ;
		nSrcBytes -= sizeof(ERISA::MIO_INFO_HEADER) ;
	}
	else
	{
		if ( m_idSoundPacketID + 1 < pHeader->idSerial )
		{
			m_flagKeySound = true ;
			return ;
		}
		ptrData = (pmdh + 1) ;
	}
	//
	// デコード
	//
	ERISA::SGLDecodeBitStream	bstream( 0x1000 ) ;
	SMemoryReferenceFile		memfile ;
	memfile.AttachMemory( (void*) ptrData, nSrcBytes ) ;
	bstream.AttachInputStream( &memfile ) ;
	//
	SArray<uint8_t>	bufWavePCM ;
	size_t	nPCMBytes = pmdh->dwSampleCount
						* m_mioHeader.dwChannelCount
						* (m_mioHeader.dwBitsPerSample / 8) ;
	//
	if ( !m_decSound.DecodeSound
		( bstream, *pmdh, bufWavePCM.GetArray(nPCMBytes) ) )
	{
		bufWavePCM.FinishArray() ;
		//
		if ( m_pListener )
		{
			m_pListener->OnSoundStream
				( bufWavePCM.GetConstArray(), nPCMBytes,
					(m_flagSoundFirst
						|| (m_idSoundPacketID + 1 < pHeader->idSerial)) ) ;
		}
	}
	else
	{
		bufWavePCM.FinishArray() ;
	}
	m_flagSoundFirst = false ;
	m_idSoundPacketID = pHeader->idSerial ;
}

// 映像ストリームデータ処理
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingClient::OnStreamVideo
	( SGLMediaStreamingClient::RecvUDPHeader * pHeader, bool flagPrimary )
{
	if ( m_idVideoPacketID > pHeader->idSerial )
	{
		return ;
	}
	SGLMediaStreamingServer::UDPImageHeader *
		pImageHdr = (SGLMediaStreamingServer::UDPImageHeader*) (pHeader + 1) ;
	//
	LockTrace( __FILE__, __LINE__ ) ;
	m_csSync.Lock() ;
	SGLSize	sizeFrame = m_imgLastFrame.GetImageSize() ;
	SGLSize	sizeInBlock ;
	sizeInBlock.w = (sizeFrame.w + 0x0F) >> 4 ;
	sizeInBlock.h = (sizeFrame.h + 0x0F) >> 4 ;
	if ( pImageHdr->sizeFrame != sizeFrame )
	{
		ESLAssert( flagPrimary ) ;
		while ( m_countDecodingVideo != 0 )
		{
			m_csSync.Unlock() ;
			Unlock() ;
			m_signFreeDecodingVideo.Wait() ;
			LockTrace( __FILE__, __LINE__ ) ;
			m_csSync.Lock() ;
		}
		m_imgLastFrame.CreateImage
			( (uint32_t) pImageHdr->sizeFrame.w,
				(uint32_t) pImageHdr->sizeFrame.h,
				formatImageRGB, 32 ) ;
		sizeFrame = m_imgLastFrame.GetImageSize() ;
		sizeInBlock.w = (sizeFrame.w + 0x0F) >> 4 ;
		sizeInBlock.h = (sizeFrame.h + 0x0F) >> 4 ;
		//
		m_bufBlockDiffCounter.SetLength( sizeInBlock.w * sizeInBlock.h ) ;
		m_bufBlockReqArray.SetLength
				( (sizeInBlock.w * sizeInBlock.h + 0x07) >> 3 ) ;
		eslFillMemory
			( m_bufBlockDiffCounter.GetArray(),
					0x40, m_bufBlockDiffCounter.GetLength() ) ;
		m_bufBlockDiffCounter.FinishArray() ;
		//
		if ( !(pImageHdr->nFlags & SGLMediaStreamingServer::udpImageDifferential) )
		{
			if ( m_idVideoPacketID != pHeader->idSerial )
			{
				m_timerIFrameRecv.Reset() ;
			}
		}
	}
	else if ( !(pImageHdr->nFlags & SGLMediaStreamingServer::udpImageDifferential) )
	{
		if ( m_idVideoPacketID != pHeader->idSerial )
		{
			eslFillMemory
				( m_bufBlockDiffCounter.GetArray(),
						0x40, m_bufBlockDiffCounter.GetLength() ) ;
			m_bufBlockDiffCounter.FinishArray() ;
			m_timerIFrameRecv.Reset() ;
		}
	}
	m_countDecodingVideo ++ ;
	m_signFreeDecodingVideo.ResetSignal() ;
	m_csSync.Unlock() ;
	//
	SMemoryReferenceFile	memfile ;
	memfile.AttachMemory
		( (void*) (pImageHdr + 1),
			pHeader->bytesData
				- sizeof(SGLMediaStreamingServer::UDPImageHeader) ) ;
	//
	ERISA::SGLDecodeBitStream		bstream( 0x4000 ) ;
	ERISA::SGLGammaDecodeContext	decoder( &bstream ) ;
	bstream.AttachInputStream( &memfile ) ;
	//
	ImageBlockset444	bs444 ;
	ImageBlockset411	bs411 ;
	//
	SGLImageBuffer	imgFrame ;
	SGLImage		imgLastFrame ;
	imgLastFrame.DuplicateOf( m_imgLastFrame.GetImage() ) ;
	imgFrame.ptrBuffer =
		imgLastFrame.LockBuffer( imgFrame, SGLImageObject::lockWrite ) ;
	Unlock() ;
	//
	uint8_t *	pbytBlockDiffCounter = m_bufBlockDiffCounter.GetArray() ;
	pbytBlockDiffCounter += pImageHdr->nStartBlock ;
	//
	if ( pImageHdr->nStartBlock
		+ pImageHdr->nSubframeBlocks >= m_bufBlockDiffCounter.GetLength() )
	{
		if ( pImageHdr->nStartBlock < m_bufBlockDiffCounter.GetLength() )
		{
			pImageHdr->nSubframeBlocks =
				(uint32_t) m_bufBlockDiffCounter.GetLength()
										- pImageHdr->nStartBlock ;
		}
		else
		{
			pImageHdr->nSubframeBlocks = 0 ;
		}
	}
	//
	int	x = pImageHdr->nStartBlock % sizeInBlock.w ;
	int	y = (pImageHdr->nStartBlock - x) / sizeInBlock.w ;
	for ( uint32_t i = 0; i < pImageHdr->nSubframeBlocks; i ++ )
	{
		UINT	ctrl = bstream.GetNBits( 2 ) ;
		if ( ctrl != 0x02 )
		{
			DecodeBlockset411
				( decoder, bs411, pImageHdr->nParamScale, 0 ) ;
			ConvertBlockset411to444( bs444, bs411 ) ;
			ConvertBlocksetYUVtoRGB( bs444 ) ;
			if ( ctrl == 0x00 )
			{
				pbytBlockDiffCounter[i] = 0 ;
				RestoreIFrameBlockset
					( imgFrame, bs444,
						(uint32_t) (x << 4), (uint32_t) (y << 4) ) ;
			}
			else
			{
				pbytBlockDiffCounter[i] ++ ;
				RestorePFrameBlockset
					( imgFrame, bs444,
						(uint32_t) (x << 4), (uint32_t) (y << 4) ) ;
			}
		}
		if ( ++ x >= sizeInBlock.w )
		{
			x = 0 ;
			y ++ ;
		}
	}
	//
	m_bufBlockDiffCounter.FinishArray() ;
	imgLastFrame.UnlockBuffer( SGLImageObject::lockWrite ) ;
	//
	m_csSync.Lock() ;
	if ( !(pImageHdr->nFlags & SGLMediaStreamingServer::udpImageDifferential) )
	{
		if ( m_idVideoPacketID == pHeader->idSerial )
		{
			m_bytesLastIFrame += pHeader->bytesData ;
		}
		else
		{
			m_bytesLastIFrame = pHeader->bytesData ;
		}
		m_msecLastIFrameDuration =
				(uint32_t) m_timerIFrameRecv.GetTime() ;
		m_flagVideoIFrame = true ;
	}
	else
	{
		if ( m_idVideoPacketID != pHeader->idSerial )
		{
			MakeRequestIFrameEncoded() ;
		}
		if ( m_flagBlockReqIFrame )
		{
			if ( m_flagVideoIFrame
				|| (m_socketTCP.GetNotSentDataBytes() < 1000) )
			{
				SendIFrameBytes() ;
				m_flagBlockReqIFrame = false ;
			}
		}
		m_flagVideoIFrame = false ;
	}
	bool	fUpdateFrame = (m_idVideoPacketID != pHeader->idSerial) ;
	if ( (m_pListener != NULL)
		&& (fUpdateFrame
			|| (pImageHdr->nStartBlock
				+ pImageHdr->nSubframeBlocks
						>= (size_t) (sizeInBlock.w * sizeInBlock.h))) )
	{
		m_flagPostViewUpdate = true ;
	}
	if ( -- m_countDecodingVideo <= 0 )
	{
		m_signFreeDecodingVideo.SetSignal() ;
		m_countDecodingVideo = 0 ;
	}
	m_idVideoPacketID = pHeader->idSerial ;
	m_csSync.Unlock() ;
	//
	if ( m_flagPostViewUpdate )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		m_csSync.Lock() ;
		while ( m_countDecodingVideo != 0 )
		{
			m_csSync.Unlock() ;
			Unlock() ;
			m_signFreeDecodingVideo.Wait() ;
			LockTrace( __FILE__, __LINE__ ) ;
			m_csSync.Lock() ;
		}
		if ( m_flagPostViewUpdate )
		{
			m_pListener->OnVideoUpdate( m_imgLastFrame ) ;
			m_flagPostViewUpdate = false ;
		}
		m_csSync.Unlock() ;
		Unlock() ;
	}
}

// Iフレーム受信容量送信
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingClient::SendIFrameBytes( void )
{
	if ( m_bytesLastIFrame == 0 )
	{
		return ;
	}
	SString	strCmd ;
	size_t	nReqIBlock = 0 ;
	if ( m_flagBlockReqIFrame )
	{
		nReqIBlock = m_bufBlockReqArray.GetLength() ;
	}
	strCmd.Format
		( L"frame_rate bytes=%d duration=%d req_iblock=%d\n",
			m_bytesLastIFrame, m_msecLastIFrameDuration, nReqIBlock ) ;
	//
	SArray<uint8_t>	bufCmd ;
	Charset::Encode
		( bufCmd, Charset::encodingUTF8,
			strCmd, (ssize_t) strCmd.GetLength() ) ;
	//
	if ( nReqIBlock > 0 )
	{
		bufCmd.AddArray( m_bufBlockReqArray.GetConstArray(), nReqIBlock ) ;
	}
	m_socketTCP.Send( bufCmd.GetConstArray(), bufCmd.GetLength() ) ;
	//
	m_bytesLastIFrame = 0 ;
	m_flagBlockReqIFrame = false ;
}

// Iフレーム再送信要求設定
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingClient::MakeRequestIFrameEncoded( void )
{
	const uint8_t *	pbytSrc = m_bufBlockDiffCounter.GetConstArray() ;
	uint8_t *		pbytDst = m_bufBlockReqArray.GetArray() ;
	size_t			nBlocks = m_bufBlockDiffCounter.GetLength() ;
	ESLAssert( m_bufBlockReqArray.GetLength() * 8 >= nBlocks ) ;
	//
	bool	fReqIFrame = false ;
	eslFillMemory( pbytDst, 0, m_bufBlockReqArray.GetLength() ) ;
	for ( size_t i = 0; i < nBlocks; i ++ )
	{
		if ( pbytSrc[i] >= 0x0C )
		{
			pbytDst[i >> 3] |= (uint8_t) 0x80 >> (i & 0x07) ;
			fReqIFrame = true ;
		}
	}
	m_bufBlockReqArray.FinishArray() ;
	m_flagBlockReqIFrame = fReqIFrame ;
}

// 復号・逆量子化・ジグザグ走査・DCT 変換
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingClient::DecodeBlockset411
	( ERISA::SGLGammaDecodeContext& decoder,
		SGLMediaStreamingClient::ImageBlockset411& bs411,
		uint8_t nParamScale, int iDCTParam )
{
	int16_t		bufSrc[6][64] ;
	int16_t *	pBlock = &(bs411.y[0][0]) ;
	int16_t *	pSrc = &(bufSrc[0][0]) ;
	int *		pIDCTParam[6] =
	{
		&m_fxIDCTParam[iDCTParam][0],
		&m_fxIDCTParam[iDCTParam][0],
		&m_fxIDCTParam[iDCTParam][0],
		&m_fxIDCTParam[iDCTParam][0],
		&m_fxIDCTParam[iDCTParam + 1][0],
		&m_fxIDCTParam[iDCTParam + 1][0],
	} ;
	//
	// 復号
	//
	decoder.InitGammaContext() ;
	decoder.DecodeGammaCodeWords( &(bufSrc[0][0]), 64 * 6 ) ;
	//
	// ジグザグ走査・逆量子化
	//
	int	nScale = 8 - nParamScale ;
	int	nRoundOdd = (1 << nScale) >> 1 ;
	pBlock = &(bs411.y[0][0]) ;
	for ( size_t i = 0; i < 6; i ++ )
	{
		for ( size_t j = 0; j < 64; j ++ )
		{
			size_t	k = SGLMediaStreamingServer::m_indexZigzagTable[j] ;
			pBlock[j] = pSrc[k] ;
		}
		int *	pParam = pIDCTParam[i] ;
		for ( size_t j = 0; j < 64; j ++ )
		{
			pBlock[j] =
				(int16_t) (((int)pBlock[j] * pParam[j]
											+ nRoundOdd) >> nScale) ;
		}
		pSrc += 64 ;
		pBlock += 64 ;
	}
	//
	// DC 成分差分
	//
	bs411.y[1][0] += bs411.y[0][0] ;
	bs411.y[3][0] += bs411.y[2][0] ;
	bs411.y[2][0] += bs411.y[0][0] ;
	bs411.y[3][0] += bs411.y[1][0] ;
	//
	// 逆 DCT 変換
	//
	pBlock = &(bs411.y[0][0]) ;
	for ( size_t i = 0; i < 6; i ++ )
	{
		ERISA::sclwFastIDCT8x8( pBlock ) ;
		pBlock += 64 ;
	}
}

// 4:1:1 -> 4:4:4 変換
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingClient::ConvertBlockset411to444
	( SGLMediaStreamingClient::ImageBlockset444& bs444,
		const SGLMediaStreamingClient::ImageBlockset411& bs411 )
{
	eslMoveMemory
		( &(bs444.b[0][0]),
			&(bs411.y[0][0]), 64 * 4 * sizeof(int16_t) ) ;
	//
	for ( int y = 0; y < 8; y ++ )
	{
		for ( int x = 0; x < 8; x ++ )
		{
			int		i = (y << 3) + x ;
			int		j = ((y >> 2) << 1) + (x >> 2) ;
			int		k = ((y & 0x03) << 4) + ((x & 0x03) << 1) ;
			int16_t	u = bs411.u[i] ;
			int16_t	v = bs411.v[i] ;
			bs444.g[j][k] = u ;
			bs444.g[j][k + 1] = u ;
			bs444.g[j][k + 8] = u ;
			bs444.g[j][k + 9] = u ;
			bs444.r[j][k] = v ;
			bs444.r[j][k + 1] = v ;
			bs444.r[j][k + 8] = v ;
			bs444.r[j][k + 9] = v ;
		}
	}
}

// YUV -> RGB 変換
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingClient::ConvertBlocksetYUVtoRGB
	( SGLMediaStreamingClient::ImageBlockset444& bsYUV )
{
	int16_t *	pY = &(bsYUV.b[0][0]) ;
	int16_t *	pU = &(bsYUV.g[0][0]) ;
	int16_t *	pV = &(bsYUV.r[0][0]) ;
	//
	for ( size_t i = 0; i < 64 * 4; i ++ )
	{
		// (R)   (  1    0    3/2 ) (Y)
		// (G) = (  1  -3/8  -3/4 ) (U)
		// (B)   (  1   7/4    0  ) (V)
		int	y = pY[i] ;
		int	u = pU[i] ;
		int	v = pV[i] ;
		int	u3 = (u << 1) + u ;
		int	v3 = (v << 1) + v ;
		int	u7 = (u << 3) - u ;
		int	b = y + (u7 >> 2) ;
		int	g = y - (u3 >> 3) - (v3 >> 2) ;
		int	r = y + (v3 >> 1) ;
		pY[i] = (int16_t) b ;
		pU[i] = (int16_t) g ;
		pV[i] = (int16_t) r ;
	}
}

// 画像復帰
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingClient::RestoreIFrameBlockset
	( const SGLImageBuffer& imgFrame,
		SGLMediaStreamingClient::ImageBlockset444& bsRGB,
		uint32_t xPos, uint32_t yPos )
{
	const int32_t	pitchLine = imgFrame.pitchLine ;
	const int32_t	pitchPixel = imgFrame.pitchPixel ;
	int16_t *		pwRGB = &(bsRGB.b[0][0]) ;
	for ( size_t i = 0; i < 64 * 4 * 3; i ++ )
	{
		if ( (uint16_t) pwRGB[i] >= 0x100 )
		{
			pwRGB[i] = ~(pwRGB[i] >> 15) & 0xFF ;
		}
	}
	if ( (xPos + 16 <= imgFrame.width)
		&& (yPos + 16 <= imgFrame.height) )
	{
		for ( int iBlock = 0; iBlock < 4; iBlock ++ )
		{
			const int	xBlock = iBlock & 0x01 ;
			const int	yBlock = (iBlock >> 1) ;
			int16_t *	pB = &(bsRGB.b[iBlock][0]) ;
			int16_t *	pG = &(bsRGB.g[iBlock][0]) ;
			int16_t *	pR = &(bsRGB.r[iBlock][0]) ;
			//
			uint8_t *	pbytBlock =
				imgFrame.ptrBuffer
					+ ((yPos + (yBlock << 3)) * pitchLine
						+ ((xPos + (xBlock << 3)) * pitchPixel)) ;
			//
			for ( int y = 0; y < 8; y ++ )
			{
				uint8_t *	pbytNext = pbytBlock ;
				for ( int x = 0; x < 8; x ++ )
				{
					pbytNext[0] = (uint8_t) *(pB ++) ;
					pbytNext[1] = (uint8_t) *(pG ++) ;
					pbytNext[2] = (uint8_t) *(pR ++) ;
					pbytNext += pitchPixel ;
				}
				pbytBlock += pitchLine ;
			}
		}
	}
	else
	{
		for ( int iBlock = 0; iBlock < 4; iBlock ++ )
		{
			const int	xSub = iBlock & 0x01 ;
			const int	ySub = (iBlock >> 1) ;
			int16_t *	pB = &(bsRGB.b[iBlock][0]) ;
			int16_t *	pG = &(bsRGB.g[iBlock][0]) ;
			int16_t *	pR = &(bsRGB.r[iBlock][0]) ;
			//
			int	xBlock = xPos + (xSub << 3) ;
			int	yBlock = yPos + (ySub << 3) ;
			int	wBlock = (int) imgFrame.width - xBlock ;
			int	hBlock = (int) imgFrame.height - yBlock ;
			if ( wBlock > 8 )
			{
				wBlock = 8 ;
			}
			if ( hBlock > 8 )
			{
				hBlock = 8 ;
			}
			//
			uint8_t *	pbytBlock =
				imgFrame.ptrBuffer
					+ (yBlock * pitchLine + xBlock * pitchPixel) ;
			//
			for ( int y = 0; y < hBlock; y ++ )
			{
				uint8_t *	pbytNext = pbytBlock ;
				for ( int x = 0; x < wBlock; x ++ )
				{
					pbytNext[0] = (uint8_t) *(pB ++) ;
					pbytNext[1] = (uint8_t) *(pG ++) ;
					pbytNext[2] = (uint8_t) *(pR ++) ;
					pbytNext += pitchPixel ;
				}
				pbytBlock += pitchLine ;
			}
		}
	}
}

void SGLMediaStreamingClient::RestorePFrameBlockset
	( const SGLImageBuffer& imgFrame,
		SGLMediaStreamingClient::ImageBlockset444& bsRGB,
		uint32_t xPos, uint32_t yPos )
{
	const int32_t	pitchLine = imgFrame.pitchLine ;
	const int32_t	pitchPixel = imgFrame.pitchPixel ;
	int16_t *		pwRGB = &(bsRGB.b[0][0]) ;
	for ( int iBlock = 0; iBlock < 4; iBlock ++ )
	{
		const int	xSub = iBlock & 0x01 ;
		const int	ySub = (iBlock >> 1) ;
		int16_t *	pB = &(bsRGB.b[iBlock][0]) ;
		int16_t *	pG = &(bsRGB.g[iBlock][0]) ;
		int16_t *	pR = &(bsRGB.r[iBlock][0]) ;
		//
		int	xBlock = xPos + (xSub << 3) ;
		int	yBlock = yPos + (ySub << 3) ;
		int	wBlock = (int) imgFrame.width - xBlock ;
		int	hBlock = (int) imgFrame.height - yBlock ;
		if ( wBlock > 8 )
		{
			wBlock = 8 ;
		}
		if ( hBlock > 8 )
		{
			hBlock = 8 ;
		}
		//
		uint8_t *	pbytBlock =
			imgFrame.ptrBuffer
				+ (yBlock * pitchLine + xBlock * pitchPixel) ;
		//
		for ( int y = 0; y < hBlock; y ++ )
		{
			uint8_t *	pbytNext = pbytBlock ;
			for ( int x = 0; x < wBlock; x ++ )
			{
				int16_t	b = *(pB ++) + pbytNext[0] ;
				int16_t	g = *(pG ++) + pbytNext[1] ;
				int16_t	r = *(pR ++) + pbytNext[2] ;
				pbytNext[0] = (uint8_t) ((b & 0xFF00) ? ~(b >> 15) : b) ;
				pbytNext[1] = (uint8_t) ((g & 0xFF00) ? ~(g >> 15) : g) ;
				pbytNext[2] = (uint8_t) ((r & 0xFF00) ? ~(r >> 15) : r) ;
				pbytNext += pitchPixel ;
			}
			pbytBlock += pitchLine ;
		}
	}
}

// カーソルデータ処理
//////////////////////////////////////////////////////////////////////////////
void SGLMediaStreamingClient::OnMouseCursor
		( SGLMediaStreamingClient::RecvUDPHeader * pHeader )
{
	if ( m_idCursorPacketID > pHeader->idSerial )
	{
		return ;
	}
	LockTrace( __FILE__, __LINE__ ) ;
	CursorDataHeader *	pCurHeader = (CursorDataHeader*) (pHeader + 1) ;
	SGLImage *	pimgCursor = NULL ;
	if ( pCurHeader->nFlags & udpCursorImage )
	{
		pimgCursor = &m_imgLastCursor ;
		//
		if ( m_imgLastCursor.GetImageSize() != pCurHeader->sizeCursor )
		{
			m_imgLastCursor.CreateImage
				( pCurHeader->sizeCursor.w,
					pCurHeader->sizeCursor.h,
					formatImageARGB, 32 ) ;
		}
		SGLImageInfo	imginf ;
		uint8_t *	pbytImage = m_imgLastCursor.LockBuffer( imginf ) ;
		uint32_t *	pdwSrc = (uint32_t*) (pCurHeader + 1) ;
		for ( uint32_t y = 0; y < imginf.height; y ++ )
		{
			eslMoveMemory( pbytImage, pdwSrc, imginf.width * 4 ) ;
			pbytImage += imginf.pitchLine ;
			pdwSrc += imginf.width ;
		}
		m_imgLastCursor.UnlockBuffer() ;
	}
	if ( m_pListener != NULL )
	{
		m_pListener->OnMouseCursor( *pCurHeader, pimgCursor ) ;
	}
	Unlock() ;
}



//////////////////////////////////////////////////////////////////////////////
// ウィンドウ同期動画キャプチャー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( SakuraGL::SGLSyncWindowCapture, ESLObject, SProcedure )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSyncWindowCapture::SGLSyncWindowCapture( void )
{
	m_pWindow = NULL ;
	m_pVideoOut = NULL ;
	m_pAudioOut = NULL ;
	//
	m_flagCapturing = false ;
	m_flagExitCapture = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSyncWindowCapture::~SGLSyncWindowCapture( void )
{
	if ( m_flagCapturing )
	{
		EndCapture() ;
	}
}

// ターゲットウィンドウ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSyncWindowCapture::AttachTargetWindow( SGLWindowSprite * pWindow )
{
	m_pWindow = pWindow ;
}

// 出力先設定
//////////////////////////////////////////////////////////////////////////////
void SGLSyncWindowCapture::AttachOutputStream
	( SGLVideoOutputStream * pVideo, SGLAudioOutputStream * pAudio )
{
	m_pVideoOut = pVideo ;
	m_pAudioOut = pAudio ;
}

// キャプチャー開始
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSyncWindowCapture::BeginCapture
	( uint32_t nFlags,
		size_t nMilliSecPerFrame,
		const SGLImageInfo& infVideo,
		const SGLSoundFormat& fmtSoundOut )
{
	if ( m_flagCapturing )
	{
		return	sglErrFailed ;
	}
	m_flagExitCapture = false ;
	m_nFlags = nFlags ;
	m_nMilliSecPerFrame = nMilliSecPerFrame ;
	m_infVideo = infVideo ;
	m_sizeVideo.w = (int32_t) infVideo.width ;
	m_sizeVideo.h = (int32_t) infVideo.height ;
	m_fmtMixOut = fmtSoundOut ;
	m_mixSound.SetOutputFormat( m_fmtMixOut ) ;
	//
	if ( m_thread.BeginThread( this ) )
	{
		return	sglErrFailed ;
	}
	m_flagCapturing = true ;
	SGLSoundPlayer::SetPlayerCreator
		( &SGLSyncWindowCapture::new_SoundPlayer, this ) ;
	return	sglErrSuccess ;
}

// キャプチャー終了
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSyncWindowCapture::EndCapture( void )
{
	if ( m_flagCapturing )
	{
		m_flagExitCapture = true ;
		m_thread.Wait() ;
		m_thread.Delete() ;
		m_flagCapturing = false ;
		SGLSoundPlayer::SetPlayerCreator( NULL, NULL ) ;
	}
	return	sglErrSuccess ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLSyncWindowCapture::Run( void )
{
	STimeCounter::SetVirtualTimerMode( true ) ;
	if ( !(m_nFlags & captureRealTime) )
	{
		m_pWindow->EnableSpriteTimer( false ) ;
	}
	SArray<uint8_t>	bufWave ;
	SGLImage		imgCapture ;
	imgCapture.CreateImage
		( m_sizeVideo.w, m_sizeVideo.h, m_infVideo.format, m_infVideo.depth ) ;
	//
	SDWORD	dwDelayTime = 0 ;
	while ( !m_flagExitCapture )
	{
		#if	defined(__PLATFORM_WINDOWS__)
		DWORD	dwFrameTime = ::GetCurrentTime() ;
		#endif
		//
		Lock() ;
		if ( !(m_nFlags & captureRealTime) )
		{
			m_pWindow->AdvanceTime( (uint32_t) m_nMilliSecPerFrame ) ;
		}
		Unlock() ;
		//
		m_mixSound.MixSound( (uint32_t) m_nMilliSecPerFrame ) ;
		if ( !(m_nFlags & captureRealTime) )
		{
			m_pWindow->UpdateWindow() ;
		}
		STimeCounter::AdvanceVirtualTimer( (uint32_t) m_nMilliSecPerFrame ) ;
		//
		#if	defined(__PLATFORM_WINDOWS__)
		HWND	hWnd = m_pWindow->GetWindowHandle() ;
		SGLImageWin32DIBitmap *
				pDIB = SGLImageWin32DIBitmap::CommitDIB( &imgCapture ) ;
		if ( pDIB != NULL )
		{
			HDC	hdc = ::GetDC( hWnd ) ;
			::BitBlt
				( pDIB->m_hDC, 0, 0,
					m_sizeVideo.w, m_sizeVideo.h, hdc, 0, 0, SRCCOPY ) ;
			::ReleaseDC( hWnd, hdc ) ;
			imgCapture.ReflectImageObject( imageObjectWin32DIBitmap ) ;
		}
		#endif
		//
		if ( m_pVideoOut != NULL )
		{
			SGLImageInfo	infCapture ;
			uint8_t *		pbytCapture =
				imgCapture.LockBuffer( infCapture, SGLImageObject::lockRead ) ;
			m_pVideoOut->WriteFrame( infCapture, pbytCapture ) ;
			imgCapture.UnlockBuffer( SGLImageObject::lockRead ) ;
		}
		//
		size_t	nWaveBytes = m_mixSound.GetOutputDataBytes() ;
		if ( nWaveBytes > 0x10000 )
		{
			uint8_t *	pbytWave = bufWave.GetArray( nWaveBytes ) ;
			size_t	nBytes = m_mixSound.ReadOutputData( pbytWave, nWaveBytes ) ;
			if ( m_pAudioOut != NULL )
			{
				m_pAudioOut->WriteAudio
					( pbytWave, nBytes / (m_fmtMixOut.channels
											* m_fmtMixOut.bitsPerSample / 8) ) ;
			}
			bufWave.FinishArray() ;
		}
		//
		#if	defined(__PLATFORM_WINDOWS__)
		if ( !(m_nFlags & captureNoWait) )
		{
			DWORD	dwEndTime = ::GetCurrentTime() ;
			SDWORD	dwPastTime = (SDWORD) (dwEndTime - dwFrameTime) ;
			if ( dwPastTime + dwDelayTime < (SDWORD) m_nMilliSecPerFrame )
			{
				SleepMilliSec
					( (int) m_nMilliSecPerFrame
						- (int) (dwPastTime + dwDelayTime) ) ;
			}
			dwDelayTime +=
				(SDWORD) (::GetCurrentTime() - dwFrameTime)
									- (SDWORD) m_nMilliSecPerFrame ;
			if ( dwDelayTime > (SDWORD) m_nMilliSecPerFrame )
			{
				dwDelayTime = (SDWORD) m_nMilliSecPerFrame ;
			}
		}
		#endif
	}
	size_t	nWaveBytes = m_mixSound.GetOutputDataBytes() ;
	if ( nWaveBytes > 0 )
	{
		uint8_t *	pbytWave = bufWave.GetArray( nWaveBytes ) ;
		size_t	nBytes = m_mixSound.ReadOutputData( pbytWave, nWaveBytes ) ;
		if ( m_pAudioOut != NULL )
		{
			m_pAudioOut->WriteAudio
				( pbytWave, nBytes / (m_fmtMixOut.channels
										* m_fmtMixOut.bitsPerSample / 8) ) ;
		}
		bufWave.FinishArray() ;
	}
	STimeCounter::SetVirtualTimerMode( false ) ;
	if ( !(m_nFlags & captureRealTime) )
	{
		m_pWindow->EnableSpriteTimer( true ) ;
	}
}

// ミキサ出力サウンドプレイヤー生成関数
//////////////////////////////////////////////////////////////////////////////
SGLSoundPlayerInterface * SGLSyncWindowCapture::new_SoundPlayer( void * pInstance )
{
	SGLSyncWindowCapture *	pswc = (SGLSyncWindowCapture*) pInstance ;
	return	new SGLSoundMixerLinePlayer( &(pswc->m_mixSound) ) ;
}


