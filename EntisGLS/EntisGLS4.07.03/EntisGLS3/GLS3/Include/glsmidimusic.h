
/*****************************************************************************
             Entis Generalized Library System version 3
													last update 2002/08/21
 ----------------------------------------------------------------------------
		Copyright (c) 1998-2002 Leshade Entis. All rights reserved.
 ****************************************************************************/


#if	!defined(__MIDI_MUSIC_H__)
#define	__MIDI_MUSIC_H__	1


//////////////////////////////////////////////////////////////////////////////
// MIDI ストリーム再生オブジェクト
//////////////////////////////////////////////////////////////////////////////

class	EMidiMusic	: public	ESLObject
{
public:
	// 構築関数
	EMidiMusic( void ) ;
	// 消滅関数
	virtual ~EMidiMusic( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EMidiMusic, ESLObject )

protected:
	HMIDISTRM		m_hMidiStream ;			// MIDI 出力デバイスハンドル
	MIDIHDR			m_midihdr ;				// MIDI ヘッダ
	UINT			m_nDeviceID	;			// MIDI デバイス識別子

	unsigned int	m_fPlaying ;			// 再生中か？
	unsigned int	m_fPaused ;				// 一時停止中か？
	unsigned int	m_fRepeat ;				// 反復再生フラグ

	int				m_nDivClock ;			// [a quarter note / clock]
	DWORD			m_dwDefTempo ;			// デフォルトテンポ
	DWORD			m_dwTotalVolume ;		// トータルボリューム
	DWORD			m_dwChannelVolume[16] ;	// チャネルボリューム
	DWORD			m_dwLastVolume[16] ;	// 最近変更されたチャネルボリューム

	EObjArray<EStreamBuffer>
					m_listMidiBlock ;		// MIDI ストリームデータ配列
	unsigned int	m_iCurrentPoint ;		// 現在の MIDI ストリーム指標
	unsigned int	m_iLoopBlockPoint ;		// 巻き戻し MIDI ストリーム指標

	HANDLE			m_hDonePlaying ;		// 再生終了イベント

	CRITICAL_SECTION	m_cs ;

public:
	// 内容を削除
	void DeleteContents( void ) ;
	// MIDI ファイルを読み込む
	ESLError ReadMidi( ESLFileObject & file ) ;

protected:
	// MIDI ストリームにデータを追加する
	void AddMidiStream
		( EStreamBuffer *& pMidiBuf,
			EPtrObjArray<MIDIEVENT> & listMidiEvent, DWORD dwLastDeltaTime ) ;

public:
	// MIDI デバイスを開く
	ESLError OpenMidi( void ) ;
	// MIDI デバイスを閉じる
	ESLError CloseMidi( void ) ;
	// MIDI ストリームを再生する
	ESLError PlayMidi( unsigned int fRepeat = FALSE ) ;
	// MIDI ストリームを停止する
	ESLError StopMidi( void ) ;
	// MIDI ストリームを一時停止する
	ESLError PauseMidi( void ) ;
	// MIDI ストリームを再開する
	ESLError RestartMidi( void ) ;

	// トータルボリュームを設定する
	ESLError SetVolume( DWORD dwVolume ) ;
	// チャネルボリュームを設定する
	ESLError SetChannelVolume( DWORD dwChannel, DWORD dwVolume ) ;
	// ショートメッセージを出力する
	ESLError OutShortMsg( DWORD dwMsg ) ;
	// ロングメッセージを出力する
	ESLError OutLongMsg( LPSTR pLongMsg, DWORD dwMsgLength ) ;

	// MIDI ストリームを再生中か？
	unsigned int IsPlaying( void ) const
		{
			return	m_fPlaying ;
		}
	// リピートフラグを取得
	unsigned int GetRepeatFlag( void ) const
		{
			return	m_fRepeat ;
		}

public:
	// 再生が終了するまで待機
	ESLError WaitUntilPlayed( DWORD dwTimeout ) ;

	// スレッド排他アクセス
	void Lock( void ) const ;
	void Unlock( void ) const ;

	// MIDI デバイスはインストールされているか？
	static bool IsDeviceInstalled( void ) ;

protected:
	// MIDI ストリームコールバック関数
	static void CALLBACK MidiCallbackProc
		( HMIDISTRM hMidiStream, UINT uMsg,
			DWORD dwInstance, DWORD dwParam1, DWORD dwParam2 ) ;
	void MidiStreamProc
		( HMIDISTRM hMidiStream, UINT uMsg, MIDIHDR * pMidiHdr ) ;

};

#endif
