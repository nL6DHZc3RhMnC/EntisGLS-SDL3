
/*****************************************************************************
				Entis Generalized Library System version 3
													last update 2002/08/21
 ----------------------------------------------------------------------------
		Copyright (c) 1998-2002 Leshade Entis. All rights reserved.
 ****************************************************************************/


#include <gls.h>



//////////////////////////////////////////////////////////////////////////////
// MIDI ストリーム再生オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
///////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EMidiMusic, ESLObject )

// 構築関数
///////////////////////////////////////////////////////////////////////////////
EMidiMusic::EMidiMusic( void )
{
	m_hMidiStream = NULL ;
	//
	m_fPlaying = FALSE ;
	m_fPaused = FALSE ;
	m_fRepeat = FALSE ;
	//
	m_dwTotalVolume = 0xFFFF ;
	//
	m_hDonePlaying = ::CreateEvent( NULL, TRUE, TRUE, FALSE ) ;
	::InitializeCriticalSection( &m_cs ) ;
}

// 消滅関数
///////////////////////////////////////////////////////////////////////////////
EMidiMusic::~EMidiMusic( void )
{
	DeleteContents( ) ;
	::CloseHandle( m_hDonePlaying ) ;
	::DeleteCriticalSection( &m_cs ) ;
}

// 内容を削除
///////////////////////////////////////////////////////////////////////////////
void EMidiMusic::DeleteContents( void )
{
	CloseMidi( ) ;
	//
	Lock( ) ;
	m_listMidiBlock.RemoveAll( ) ;
	Unlock( ) ;
}

// MIDI トラック情報
///////////////////////////////////////////////////////////////////////////////
struct	MIDI_TRACK_INFO
{
	DWORD	dwDeltaTime ;
	DWORD	dwLastMsg ;
	BYTE *	pMidiData ;
} ;

// MIDI ファイルを読み込む
///////////////////////////////////////////////////////////////////////////////
ESLError EMidiMusic::ReadMidi( ESLFileObject & file )
{
	// 現在の内容を削除する
	//////////////////////////////////////////////////////////////////////////
	DeleteContents( ) ;
	m_fRepeat = FALSE ;

	// ヘッダを読み込む
	///////////////////////////////////////////////////////////////////////////
	char	cMidiHdr[0x10] ;
	if ( file.Read( &cMidiHdr[0], 4 ) < 4 )
	{
		ESLTrace( "MIDI ファイルの読み込みに失敗しました。\n" ) ;
		return	ESLErrorMsg( "MIDI ファイルの読み込みに失敗しました。" ) ;
	}
	if ( (cMidiHdr[0] != 'M') || (cMidiHdr[1] != 'T')
		|| (cMidiHdr[2] != 'h') || (cMidiHdr[3] != 'd') )
	{
		file.Seek( (0x80 - 4), ESLFileObject::FromCurrent ) ;
		file.Read( &cMidiHdr[0], 4 ) ;
		if ( (cMidiHdr[0] != 'M') || (cMidiHdr[1] != 'T')
			|| (cMidiHdr[2] != 'h') || (cMidiHdr[3] != 'd' ) )
		{
			ESLTrace( "MIDI ファイルの読み込みに失敗しました。\n" ) ;
			return	ESLErrorMsg( "MIDI ファイルの読み込みに失敗しました。" ) ;
		}
	}
	//
	file.Read( &cMidiHdr[0], 4 ) ;
	DWORD	dwBlockLength =
		(((DWORD) cMidiHdr[0] & 0xFF) << 24)
			+ (((DWORD) cMidiHdr[1] & 0xFF) << 16)
			+ (((DWORD) cMidiHdr[2] & 0xFF) << 8)
			+ (((DWORD) cMidiHdr[3] & 0xFF)) ;
	if ( dwBlockLength != 6 )
	{
		return	ESLErrorMsg( "MIDI ファイルの読み込みに失敗しました。" ) ;
	}
	//
	file.Read( &cMidiHdr[0], 4 ) ;
	unsigned int	nTrackCount ;
	if ( cMidiHdr[1] == 0 )
	{
		nTrackCount = 1 ;
	}
	else
	{
		nTrackCount =
			(((DWORD) cMidiHdr[2] & 0xFF) << 8)
				+ ((DWORD) cMidiHdr[3] & 0xFF) ;
	}
	//
	file.Read( &cMidiHdr[0], 2 ) ;
	m_nDivClock =
		(((DWORD) cMidiHdr[0] & 0xFF) << 8)
			+ ((DWORD) cMidiHdr[1] & 0xFF) ;
	//
	DWORD	dwFilePointer = file.GetPosition( ) ;

	// MIDI データを読み込む
	///////////////////////////////////////////////////////////////////////////
	unsigned int	i ;
	EObjArray<EStreamBuffer>	listMidiData ;
	EObjArray<MIDI_TRACK_INFO>	listTrackInfo ;
	for ( i = 0; i < nTrackCount; i ++ )
	{
		//
		// トラックヘッダを読み込む
		//
		file.Read( &cMidiHdr[0], 8 ) ;
		if ( (cMidiHdr[0] != 'M') || (cMidiHdr[1] != 'T')
			|| (cMidiHdr[2] != 'r') || (cMidiHdr[3] != 'k') )
		{
			return	ESLErrorMsg( "MIDI ファイルの読み込みに失敗しました。" ) ;
		}
		//
		// トラックデータを読み込む
		//
		DWORD	dwTrackLength =
			(((DWORD) cMidiHdr[4] & 0xFF) << 24)
				+ (((DWORD) cMidiHdr[5] & 0xFF) << 16)
				+ (((DWORD) cMidiHdr[6] & 0xFF) << 8)
				+ (((DWORD) cMidiHdr[7] & 0xFF)) ;
		EStreamBuffer *	pBuf = new EStreamBuffer ;
		void *	ptrData = pBuf->PutBuffer( dwTrackLength ) ;
		if ( file.Read( ptrData, dwTrackLength ) < dwTrackLength )
		{
			return	ESLErrorMsg( "MIDI ファイルの読み込みに失敗しました。" ) ;
		}
		pBuf->Flush( dwTrackLength ) ;
		listMidiData.Add( pBuf ) ;
		//
		// トラック情報を初期化
		//
		MIDI_TRACK_INFO *	pTrackInf = new MIDI_TRACK_INFO ;
		BYTE *	pbyData = (BYTE*) ptrData ;
		DWORD	dwDeltaTime = 0, dwDeltaFlag ;
		do
		{
			dwDeltaFlag = (DWORD) *(pbyData ++) ;
			dwDeltaTime = (dwDeltaTime << 7) | (dwDeltaFlag & 0x7F) ;
		}
		while ( dwDeltaFlag & 0x80 ) ;
		//
		pTrackInf->dwDeltaTime = dwDeltaTime ;
		pTrackInf->dwLastMsg = NULL ;
		pTrackInf->pMidiData = pbyData ;
		listTrackInfo.Add( pTrackInf ) ;
	}

	// MIDI ストリームへ変換
	///////////////////////////////////////////////////////////////////////////
	MIDI_TRACK_INFO *		pTrackInf ;
	DWORD					dwMinDeltaTime, dwLastDeltaTime = 0 ;
	int						nMidiStrmLength = 0, nMinIndex ;
	unsigned int			nEndTrackCount = 0 ;
	EStreamBuffer *			pMidiBuf = new EStreamBuffer ;
	EPtrObjArray<MIDIEVENT>	listMidiEvent ;
	m_iLoopBlockPoint = 0 ;
	//
	while ( nTrackCount > nEndTrackCount )
	{
		//
		// 最小デルタ時間のトラックを探す
		//
		dwMinDeltaTime = 0xFFFFFFFF ;
		nMinIndex = 0 ;
		for ( i = 0; i < nTrackCount; i ++ )
		{
			pTrackInf = listTrackInfo.GetAt( i ) ;
			if ( (pTrackInf->pMidiData != NULL)
				&& (pTrackInf->dwDeltaTime < dwMinDeltaTime) )
			{
				dwMinDeltaTime = pTrackInf->dwDeltaTime ;
				nMinIndex = i ;
			}
		}
		//
		// デルタ時間を差し引く
		//
		if ( dwMinDeltaTime != 0 )
		{
			//
			// デルタ時間を減算
			//
			for ( i = 0; i < nTrackCount; i ++ )
			{
				listTrackInfo.GetAt(i)->dwDeltaTime -= dwMinDeltaTime ;
			}
			//
			// MIDI イベントを出力
			//
			AddMidiStream( pMidiBuf, listMidiEvent, dwLastDeltaTime ) ;
			dwLastDeltaTime = dwMinDeltaTime ;
		}
		i = nMinIndex ;
		//
		// MIDI ストリームへ変換する
		//
		BYTE	bytMidiCode ;
		pTrackInf = listTrackInfo.GetAt( i ) ;
		BYTE *	pMidi = pTrackInf->pMidiData ;
		DWORD	dwLength, dwTempo ;
		MIDIEVENT *	pMidiEvent ;
		do
		{
			//
			// MIDI メッセージ
			//
			bytMidiCode = *pMidi ;
			if ( bytMidiCode == 0xFF )
			{
				// メタイベント
				///////////////////////////////////////////////////////////////
				pMidi ++ ;
				switch ( *(pMidi ++) )
				{
				case	0x2f:	// End of track --> exit
					pTrackInf->pMidiData = NULL ;
					nEndTrackCount ++ ;
					break ;

				case	0x51:	// テンポ
					pMidi ++ ;
					dwTempo =
						( (((DWORD) pMidi[0]) << 16)
						+ (((DWORD) pMidi[1]) << 8)
						+  ((DWORD) pMidi[2]) ) ;
					pMidi += 3 ;
					//
					pMidiEvent = (MIDIEVENT*) ::eslHeapAllocate
						( NULL, sizeof(DWORD) * 3, ESL_HEAP_ZERO_INIT ) ;
					pMidiEvent->dwEvent =
							dwTempo | MEVT_F_CALLBACK | (MEVT_TEMPO << 24) ;
					listMidiEvent.Add( pMidiEvent ) ;
					break ;

				default:		// その他
					dwLength = (DWORD) *(pMidi ++) ;
					pMidi += dwLength ;
					break ;
				}
				if ( pTrackInf->pMidiData == NULL )
				{
					break ;
				}
			}
			else if ( (bytMidiCode == 0xF0) || (bytMidiCode == 0xF7) )
			{
				// エクスクルーシブメッセージ
				///////////////////////////////////////////////////////////////
				pMidi ++ ;
				dwLength = (DWORD) *(pMidi ++) ;
				if ( (dwLength != 0)
					&& ((dwLength != 1) || (*pMidi != 0xF7)) )
				{
					//
					// Exclusive message
					//
					if( bytMidiCode == 0xF0 )
					{
						//
						// FO event
						dwLength ++ ;
						pMidiEvent = (MIDIEVENT*) ::eslHeapAllocate
							( NULL, sizeof(DWORD) * 3
								+ ((dwLength + 3) & ~0x03), ESL_HEAP_ZERO_INIT ) ;
						pMidiEvent->dwEvent = (dwLength | MEVT_F_LONG) ;
						*((BYTE*)&pMidiEvent->dwParms[0]) = bytMidiCode ;
						::memcpy( ((BYTE*)&pMidiEvent->dwParms[0]) + 1,
													pMidi, dwLength - 1 ) ;
						pMidi += dwLength - 1 ;
					}
					else
					{
						//
						// F7 event
						pMidiEvent = (MIDIEVENT*) ::eslHeapAllocate
							( NULL, sizeof(DWORD) * 3
								+ ((dwLength + 3) & ~0x03), ESL_HEAP_ZERO_INIT ) ;
						pMidiEvent->dwEvent = (dwLength | MEVT_F_LONG) ;
						::memcpy( &pMidiEvent->dwParms[0], pMidi, dwLength ) ;
						pMidi += dwLength ;
					}
					listMidiEvent.Add( pMidiEvent ) ;
				}
				else
				{
					//
					// ループポイント設定
					//
					if( (bytMidiCode == 0xF0) &&
							(dwLength == 1) && (*pMidi == 0xF7) )
					{
						pMidi ++ ;
					}
					if ( !m_fRepeat )
					{
						if ( pMidiBuf->GetLength() > 0 )
						{
							m_listMidiBlock.Add( pMidiBuf ) ;
							pMidiBuf = new EStreamBuffer ;
						}
						m_iLoopBlockPoint = m_listMidiBlock.GetSize( ) ;
						m_fRepeat = TRUE ;
					}
				}
			}
			else if ( (bytMidiCode & 0xF0) == 0xF0 )
			{
				// コモンメッセージ　／　リアルタイムメッセージ
				///////////////////////////////////////////////////////////////
				pMidiEvent = (MIDIEVENT*) ::eslHeapAllocate
						( NULL, sizeof(DWORD) * 3, ESL_HEAP_ZERO_INIT ) ;
				pMidiEvent->dwEvent = ((DWORD)bytMidiCode | MEVT_F_SHORT) ;
				listMidiEvent.Add( pMidiEvent ) ;
				pMidi ++ ;
			}
			else
			{
				// MIDI メッセージ
				///////////////////////////////////////////////////////////////
				DWORD	dwMidiShortMsg ;
				//
				if ( bytMidiCode <= 0x7F )
				{
					dwMidiShortMsg = pTrackInf->dwLastMsg ;
				}
				else
				{
					pTrackInf->dwLastMsg = (DWORD) bytMidiCode ;
					dwMidiShortMsg = (DWORD) bytMidiCode ;
					pMidi ++ ;
				}
				//
				switch ( dwMidiShortMsg & 0x70 )
				{
				case	0x00:	// note off
				case	0x10:	// note on
				case	0x20:	// polyphonic key pressure
				case	0x30:	// control change
				case	0x60:	// pitch bend change
					dwMidiShortMsg |= ( ((DWORD)*((WORD*)pMidi)) << 8 ) ;
					pMidi += 2 ;
					break ;
				case	0x40:	// program change
				case	0x50:	// channel pressure
					dwMidiShortMsg |= ( ((DWORD)(*pMidi)) << 8 ) ;
					pMidi ++ ;
					break ;
				}
				//
				if ( (dwMidiShortMsg & 0xFFF0) == 0x07B0 )
				{
					// part level change
					pMidiEvent = (MIDIEVENT*) ::eslHeapAllocate
							( NULL, sizeof(DWORD) * 3, ESL_HEAP_ZERO_INIT ) ;
					pMidiEvent->dwEvent =
								dwMidiShortMsg | MEVT_F_SHORT |
								MEVT_F_CALLBACK | (MEVT_NOP << 24) ;
					listMidiEvent.Add( pMidiEvent ) ;
				}
				else
				{
					pMidiEvent = (MIDIEVENT*) ::eslHeapAllocate
							( NULL, sizeof(DWORD) * 3, ESL_HEAP_ZERO_INIT ) ;
					pMidiEvent->dwEvent = ((DWORD) dwMidiShortMsg | MEVT_F_SHORT) ;
					listMidiEvent.Add( pMidiEvent ) ;
				}
			}
			//
			// 次のデルタ時間を取得
			//
			dwMinDeltaTime = 0 ;
			do
			{
				bytMidiCode = *(pMidi ++) ;
				dwMinDeltaTime =
					(dwMinDeltaTime << 7) | ((DWORD) bytMidiCode & 0x7F) ;
			}
			while ( bytMidiCode >= 0x80 ) ;
		}
		while ( dwMinDeltaTime == 0 ) ;
		//
		if ( pTrackInf->pMidiData != NULL )
		{
			pTrackInf->pMidiData = pMidi ;
			pTrackInf->dwDeltaTime = dwMinDeltaTime ;
		}
	}

	// MIDI ストリーム確定
	///////////////////////////////////////////////////////////////////////////
	AddMidiStream( pMidiBuf, listMidiEvent, dwLastDeltaTime ) ;
	//
	if ( pMidiBuf->GetLength() > 0 )
	{
		m_listMidiBlock.Add( pMidiBuf ) ;
	}
	else
	{
		delete	pMidiBuf ;
	}

	return	eslErrSuccess ;
}

// MIDI ストリームにデータを追加する
///////////////////////////////////////////////////////////////////////////////
void EMidiMusic::AddMidiStream
	( EStreamBuffer *& pMidiBuf,
		EPtrObjArray<MIDIEVENT> & listMidiEvent, DWORD dwLastDeltaTime )
{
	for ( int i = 0; i < 4; i ++ )
	{
		unsigned int	j = 0 ;
		while ( j < listMidiEvent.GetSize() )
		{
			MIDIEVENT *	pMidiEvent = listMidiEvent.GetAt( j ) ;
			DWORD	dwEventType = MEVT_EVENTTYPE(pMidiEvent->dwEvent) ;
			bool	fOutput = false ;
			switch ( i )
			{
			case	0:
				// GM リセットは他の設定より先に行う
				if ( (dwEventType == MEVT_LONGMSG)
					&& (MEVT_EVENTPARM(pMidiEvent->dwEvent) == 6) )
				{
					if ( ((pMidiEvent->dwParms[0] & 0xFF00FFFF) == 0x09007EF0)
						&& ((pMidiEvent->dwParms[1] & 0xFFFF) == 0xF701))
					{
						fOutput = true ;
					}
				}
				break ;

			case	1:
				if ( dwEventType == MEVT_SHORTMSG )
				{
					DWORD	dwMidiMsg = MEVT_EVENTPARM(pMidiEvent->dwEvent) ;
					if ( (dwMidiMsg & 0xF0) == 0xC0 )
					{
						fOutput = true ;
					}
					else if ( (dwMidiMsg & 0xF0) == 0xB0 )
					{
						fOutput = ((dwMidiMsg & 0xFF00) == 0) ||
									((dwMidiMsg & 0xFF00) == 0x2000) ;
					}
				}
				break ;

			case	2:
				fOutput = (dwEventType == MEVT_SHORTMSG) ||
							(dwEventType == MEVT_LONGMSG) ;
				break ;

			case	3:
			default:
				fOutput = true ;
				break ;
			}
			//
			if ( fOutput )
			{
				//
				// MIDI メッセージを出力する
				//
				pMidiEvent->dwDeltaTime = dwLastDeltaTime ;
				dwLastDeltaTime = 0 ;
				DWORD	dwEventBytes = sizeof(DWORD) * 3 ;
				if ( pMidiEvent->dwEvent & MEVT_F_LONG )
				{
					dwEventBytes +=
						(MEVT_EVENTPARM(pMidiEvent->dwEvent)
											+ 0x03) & (~0x03) ;
				}
				pMidiBuf->Write( pMidiEvent, dwEventBytes ) ;
				//
				// MIDI ストリームブロックを確定する
				//
				if ( pMidiBuf->GetLength() >= (0x10000 - 0x200) )
				{
					m_listMidiBlock.Add( pMidiBuf ) ;
					pMidiBuf = new EStreamBuffer ;
				}
				//
				// MIDI イベントを削除
				//
				::eslHeapFree( NULL, pMidiEvent ) ;
				listMidiEvent.RemoveAt( j ) ;
			}
			else
			{
				j ++ ;
			}
		}
	}
	ESLAssert( listMidiEvent.GetSize() == 0 ) ;
}

// MIDI デバイスを開く
///////////////////////////////////////////////////////////////////////////////
ESLError EMidiMusic::OpenMidi( void )
{
	if ( m_hMidiStream != NULL )
	{
		return	eslErrSuccess ;
	}

	Lock( ) ;
	int		nError ;
	m_nDeviceID = MIDI_MAPPER ;
	nError = ::midiStreamOpen
		( &m_hMidiStream, &m_nDeviceID, 1,
			(DWORD) &EMidiMusic::MidiCallbackProc,
			(DWORD) this, CALLBACK_FUNCTION ) ;
	Unlock( ) ;

	if ( nError != MMSYSERR_NOERROR )
	{
		return	ESLErrorMsg( "MIDI ストリームデバイスを開けませんでした。" ) ;
	}
	::midiStreamRestart( m_hMidiStream ) ;

	return	eslErrSuccess ;
}

// MIDI デバイスを閉じる
///////////////////////////////////////////////////////////////////////////////
ESLError EMidiMusic::CloseMidi( void )
{
	StopMidi( ) ;
	//
	Lock( ) ;
	if ( m_hMidiStream != NULL )
	{
		::midiStreamClose( m_hMidiStream ) ;
		m_hMidiStream = NULL ;
	}
	Unlock( ) ;

	return	eslErrSuccess ;
}

// MIDI ストリームを再生する
///////////////////////////////////////////////////////////////////////////////
ESLError EMidiMusic::PlayMidi( unsigned int fRepeat )
{
	if( m_listMidiBlock.GetSize() == 0 )
	{
		return	ESLErrorMsg( "MIDI データがありません。" ) ;
	}

	//
	// MIDI デバイスを開く
	//
	if ( m_hMidiStream != NULL )
	{
		ESLError	err = StopMidi( ) ;
		if ( err != eslErrSuccess )
		{
			return	err ;
		}
	}
	else
	{
		ESLError	err = OpenMidi( ) ;
		if ( err != eslErrSuccess )
		{
			return	err ;
		}
	}
	::midiStreamPause( m_hMidiStream ) ;

	//
	// ボリュームを初期化
	//
	Lock( ) ;
	for ( int i = 0; i < 0x10; i ++ )
	{
		m_dwChannelVolume[i] = 100 ;
		m_dwLastVolume[i] = 0xFFFFFFFF ;
	}

	//
	// タイムディビジョンを設定
	//
	MIDIPROPTIMEDIV	MidiTimeDiv ;
	MidiTimeDiv.cbStruct = sizeof(MIDIPROPTIMEDIV) ;
	MidiTimeDiv.dwTimeDiv = m_nDivClock ;
	::midiStreamProperty
		( m_hMidiStream, (LPBYTE) &MidiTimeDiv,
				(MIDIPROP_SET | MIDIPROP_TIMEDIV) ) ;

	//
	// テンポを取得
	//
	MIDIPROPTEMPO	MidiTempo ;
	MidiTempo.cbStruct = sizeof(MIDIPROPTEMPO) ;
	::midiStreamProperty
		( m_hMidiStream, (LPBYTE) &MidiTempo,
				(MIDIPROP_GET | MIDIPROP_TEMPO) );
	m_dwDefTempo = MidiTempo.dwTempo ;

	//
	// 出力準備
	//
	m_iCurrentPoint = 0 ;
	::memset( &m_midihdr, 0, sizeof(MIDIHDR) ) ;
	EStreamBuffer *	pMidiBuf = m_listMidiBlock.GetAt( 0 ) ;
	EPtrBuffer	ptrbuf = pMidiBuf->GetBuffer( ) ;
	m_midihdr.lpData = (LPSTR) ptrbuf.GetBuffer( ) ;
	m_midihdr.dwBufferLength = ptrbuf.GetLength( ) ;
	m_midihdr.dwBytesRecorded = ptrbuf.GetLength( ) ;
	if ( ::midiOutPrepareHeader
		( (HMIDIOUT) m_hMidiStream,
			&m_midihdr, sizeof(MIDIHDR) ) != MMSYSERR_NOERROR )
	{
		Unlock( ) ;
		return	ESLErrorMsg( "midiOutPrepareHeader 関数が失敗しました。" ) ;
	}

	//
	// MIDI ストリーム出力
	//
	m_fRepeat = fRepeat ;
	::midiStreamRestart( m_hMidiStream ) ;
	if ( ::midiStreamOut
		( m_hMidiStream, &m_midihdr, sizeof(MIDIHDR) ) != MMSYSERR_NOERROR )
	{
		Unlock( ) ;
		return	ESLErrorMsg( "MIDI ストリームの出力に失敗しました。" ) ;
	}

	::ResetEvent( m_hDonePlaying ) ;
	m_fPaused = FALSE ;
	m_fPlaying = TRUE ;
	Unlock( ) ;
	return	eslErrSuccess ;
}

// MIDI ストリームを停止する
///////////////////////////////////////////////////////////////////////////////
ESLError EMidiMusic::StopMidi( void )
{
	if ( (m_hMidiStream != NULL) && m_fPlaying )
	{
		Lock( ) ;
		m_fPlaying = FALSE ;
		::midiStreamStop( m_hMidiStream ) ;
		for ( DWORD i = 0x007BB0; i <= 0x007BBF; i ++ )
		{
			::midiOutShortMsg( (HMIDIOUT) m_hMidiStream, i ) ;
		}
		Unlock( ) ;
		//
		WaitUntilPlayed( INFINITE ) ;
	}
	return	eslErrSuccess ;
}

// MIDI ストリームを一時停止する
///////////////////////////////////////////////////////////////////////////////
ESLError EMidiMusic::PauseMidi( void )
{
	if ( !m_fPlaying || (m_hMidiStream == NULL) )
	{
		return	ESLErrorMsg( "MIDI ストリームは再生されていません。" ) ;
	}
	if ( m_fPaused )
	{
		return	eslErrSuccess ;
	}

	Lock( ) ;
	::midiStreamPause( m_hMidiStream ) ;
	m_fPaused = TRUE ;
	Unlock( ) ;

	return	eslErrSuccess ;
}

// MIDI ストリームを再開する
///////////////////////////////////////////////////////////////////////////////
ESLError EMidiMusic::RestartMidi( void )
{
	if( !m_fPlaying || (m_hMidiStream == NULL) )
	{
		return	ESLErrorMsg( "MIDI ストリームは再生されていません。" ) ;
	}
	if ( !m_fPaused )
	{
		return	eslErrSuccess ;
	}

	Lock( ) ;
	::midiStreamRestart( m_hMidiStream ) ;
	m_fPaused = FALSE ;
	Unlock( ) ;

	return	eslErrSuccess ;
}

// トータルボリュームを設定する
///////////////////////////////////////////////////////////////////////////////
ESLError EMidiMusic::SetVolume( DWORD dwVolume )
{
	Lock( ) ;
	m_dwTotalVolume = (dwVolume < 0xFFFF) ? dwVolume : 0xFFFF ;
	if ( m_hMidiStream != NULL )
	{
		for ( int i = 0; i < 0x10; i ++ )
		{
			SetChannelVolume( i, dwVolume ) ;
		}
	}
	Unlock( ) ;
	return	eslErrSuccess ;
}

// チャネルボリュームを設定する
///////////////////////////////////////////////////////////////////////////////
ESLError EMidiMusic::SetChannelVolume( DWORD dwChannel, DWORD dwVolume )
{
	if ( m_hMidiStream == NULL )
	{
		return	eslErrSuccess ;
	}

	Lock( ) ;
	dwChannel &= 0x0F ;
	dwVolume = m_dwChannelVolume[dwChannel]
		* ((dwVolume < 0xFFFF) ? dwVolume : 0xFFFF) / 0xFFFF ;
	if ( m_dwLastVolume[dwChannel] == dwVolume )
	{
		Unlock( ) ;
		return	eslErrSuccess ;
	}
	m_dwLastVolume[dwChannel] = dwVolume ;
	if ( ::midiOutShortMsg
		( (HMIDIOUT) m_hMidiStream,
			(0x07B0 | dwChannel | (dwVolume << 16)) ) != MMSYSERR_NOERROR )
	{
		Unlock( ) ;
		return	ESLErrorMsg( "チャネルボリュームの設定に失敗しました。" ) ;
	}
	Unlock( ) ;

	return	eslErrSuccess ;
}

// ショートメッセージを出力する
///////////////////////////////////////////////////////////////////////////////
ESLError EMidiMusic::OutShortMsg( DWORD dwMsg )
{
	if ( m_hMidiStream == NULL )
	{
		return	eslErrSuccess ;
	}

	Lock( ) ;
	if ( ::midiOutShortMsg
		( (HMIDIOUT) m_hMidiStream, dwMsg ) != MMSYSERR_NOERROR )
	{
		Unlock( ) ;
		return	ESLErrorMsg( "MIDI ショートメッセージの送信に失敗しました。" ) ;
	}
	Unlock( ) ;

	return	eslErrSuccess ;
}

// ロングメッセージを出力する
///////////////////////////////////////////////////////////////////////////////
ESLError EMidiMusic::OutLongMsg( LPSTR pLongMsg, DWORD dwMsgLength )
{
	if ( m_hMidiStream == NULL )
	{
		return	eslErrSuccess ;
	}

	Lock( ) ;
	MIDIHDR *	pMidiHdr =
		(MIDIHDR*) ::eslHeapAllocate
			( NULL, sizeof(MIDIHDR) + sizeof(MIDIEVENT)
				+ dwMsgLength + 0x10, ESL_HEAP_ZERO_INIT ) ;
	pMidiHdr->lpData = ((LPSTR) pMidiHdr) + sizeof(MIDIHDR) ;
	MIDIEVENT *	pMidiEvent = (MIDIEVENT*) pMidiHdr->lpData ;
	pMidiEvent->dwDeltaTime = 0 ;
	pMidiEvent->dwStreamID = 0 ;
	pMidiEvent->dwEvent = dwMsgLength | MEVT_F_LONG ;
	::memmove( &pMidiEvent->dwParms[0], pLongMsg, dwMsgLength ) ;
	dwMsgLength = (dwMsgLength + 0x03) & (~0x03) ;
	pMidiHdr->dwBufferLength = dwMsgLength + sizeof(DWORD) * 3 ;
	pMidiHdr->dwBytesRecorded = dwMsgLength + sizeof(DWORD) * 3 ;
	if ( ::midiOutPrepareHeader
		( (HMIDIOUT) m_hMidiStream,
			pMidiHdr, sizeof(MIDIHDR) ) == MMSYSERR_NOERROR )
	{
		if ( ::midiStreamOut
			( m_hMidiStream, pMidiHdr, sizeof(MIDIHDR) ) == MMSYSERR_NOERROR )
		{
			Unlock( ) ;
			return	eslErrSuccess ;
		}
	}
	Unlock( ) ;
	::eslHeapFree( NULL, pMidiHdr ) ;

	return	ESLErrorMsg( "ロングメッセージの出力に失敗しました。" ) ;
}

// 再生が終了するまで待機
///////////////////////////////////////////////////////////////////////////////
ESLError EMidiMusic::WaitUntilPlayed( DWORD dwTimeout )
{
	DWORD	dwResult = ::WaitForSingleObject( m_hDonePlaying, dwTimeout ) ;
	if ( dwResult == WAIT_OBJECT_0 )
		return	eslErrSuccess ;
	else if ( dwResult == WAIT_TIMEOUT )
		return	eslErrTimeout ;
	return	eslErrGeneral ;
}

// スレッド排他アクセス
///////////////////////////////////////////////////////////////////////////////
void EMidiMusic::Lock( void ) const
{
	::EnterCriticalSection( (LPCRITICAL_SECTION) &m_cs ) ;
}

void EMidiMusic::Unlock( void ) const
{
	::LeaveCriticalSection( (LPCRITICAL_SECTION) &m_cs ) ;
}

// MIDI デバイスはインストールされているか？
///////////////////////////////////////////////////////////////////////////////
bool EMidiMusic::IsDeviceInstalled( void )
{
	unsigned int	nMaxDevID = ::midiOutGetNumDevs( ) ;
	unsigned int	nDeviceID = MIDI_MAPPER ;
	MIDIOUTCAPS		MidiOutCaps ;
	MMRESULT		mmresult ;
	mmresult = ::midiOutGetDevCaps
		( nDeviceID, &MidiOutCaps, sizeof(MIDIOUTCAPS) ) ;
	if ( (mmresult == MMSYSERR_NOERROR)
		&& (MidiOutCaps.dwSupport & MIDICAPS_STREAM) )
	{
		return	true ;
	}
	for ( nDeviceID = 0; nDeviceID < nMaxDevID; nDeviceID ++ )
	{
		mmresult = ::midiOutGetDevCaps
			( nDeviceID, &MidiOutCaps, sizeof(MIDIOUTCAPS) ) ;
		if( (mmresult == MMSYSERR_NOERROR)
			&& (MidiOutCaps.dwSupport & MIDICAPS_STREAM) )
		{
			return	true ;
		}
	}
	return	false ;
}

// MIDI ストリームコールバック関数
///////////////////////////////////////////////////////////////////////////////
void CALLBACK EMidiMusic::MidiCallbackProc
	( HMIDISTRM hMidiStream, UINT uMsg,
		DWORD dwInstance, DWORD dwParam1, DWORD dwParam2 )
{
	MIDIHDR *		pMidiHdr = (MIDIHDR*) dwParam1 ;
	EMidiMusic *	pMidiMusic = (EMidiMusic*) dwInstance ;
	pMidiMusic->MidiStreamProc( hMidiStream, uMsg, pMidiHdr ) ;
}

void EMidiMusic::MidiStreamProc
	( HMIDISTRM hMidiStream, UINT uMsg, MIDIHDR * pMidiHdr )
{
	if ( hMidiStream != m_hMidiStream )
	{
		ESLTrace( "警告：不正な MIDI ストリームハンドルです。\n" ) ;
	}
	MIDIEVENT *	pMidiEvent ;

	switch ( uMsg )
	{
	case	MOM_DONE:
	case	MM_STREAM_DONE:
		// ストリームブロックの再生が完了した
		///////////////////////////////////////////////////////////////////////
		::midiOutUnprepareHeader
			( (HMIDIOUT)hMidiStream, pMidiHdr, sizeof(MIDIHDR) ) ;
		if ( &m_midihdr != pMidiHdr )
		{
			// MIDI Long message か？
			if(	((DWORD)pMidiHdr->lpData)
					== (((DWORD)pMidiHdr) + sizeof(MIDIHDR)) )
			{
				::eslHeapFree( NULL, pMidiHdr ) ;
				return ;
			}
		}
		m_listMidiBlock.GetAt(m_iCurrentPoint)->Release( 0 ) ;
		//
		if ( !m_fPlaying )
		{
			::SetEvent( m_hDonePlaying ) ;
			return ;
		}
		m_iCurrentPoint ++ ;
		if( !m_fRepeat &&
			(m_iCurrentPoint >= m_listMidiBlock.GetSize()) )
		{
			::SetEvent( m_hDonePlaying ) ;
			m_fPlaying = FALSE ;
			return ;
		}
		else
		{
			// 次のストリームブロック
			if ( m_iCurrentPoint >= m_listMidiBlock.GetSize() )
			{
				m_iCurrentPoint = m_iLoopBlockPoint ;
			}
			::memset( &m_midihdr, 0, sizeof(MIDIHDR) ) ;
			EStreamBuffer *	pMidiBuf = m_listMidiBlock.GetAt( m_iCurrentPoint ) ;
			EPtrBuffer	ptrbuf = pMidiBuf->GetBuffer( ) ;
			m_midihdr.lpData = (LPSTR) ptrbuf.GetBuffer( ) ;
			m_midihdr.dwBufferLength = ptrbuf.GetLength( ) ;
			m_midihdr.dwBytesRecorded = ptrbuf.GetLength( ) ;
			::midiOutPrepareHeader
				( (HMIDIOUT) hMidiStream, &m_midihdr, sizeof(MIDIHDR) ) ;
			::midiStreamOut( hMidiStream, &m_midihdr, sizeof(MIDIHDR) ) ;
		}
		break ;

	case	MOM_POSITIONCB:
		// NOP Callback (volume update)
		///////////////////////////////////////////////////////////////////////
		pMidiEvent = (MIDIEVENT*) (pMidiHdr->lpData + pMidiHdr->dwOffset) ;
		do
		{
			if ( pMidiEvent->dwEvent & MEVT_F_LONG )
			{
				break ;
			}
			if ( (pMidiEvent->dwEvent) & (MEVT_TEMPO << 24) )
			{
				// change tempo
			}
			else if ( ((pMidiEvent->dwEvent) & (MEVT_NOP << 24)) &&
						((pMidiEvent->dwEvent) & 0xFFF0) == 0x07B0 )
			{
				// change channel volume
				m_dwChannelVolume[pMidiEvent->dwEvent & 0x0F]
						= ( (pMidiEvent->dwEvent) >> 16 ) & 0x7F ;
				SetChannelVolume
					( ((pMidiEvent->dwEvent) & 0x0F), m_dwTotalVolume ) ;
			}
			pMidiEvent = (MIDIEVENT*) (((BYTE*)pMidiEvent) + sizeof(DWORD) * 3) ;
		}
		while ( pMidiEvent->dwDeltaTime == 0 ) ;
		break ;

	default:
		break ;

	}
}

