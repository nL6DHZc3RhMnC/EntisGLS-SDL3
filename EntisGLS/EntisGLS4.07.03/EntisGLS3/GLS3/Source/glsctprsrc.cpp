
/*****************************************************************************
               Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
   Copyright (c) 2003-2013 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// リソーススクリプトインターフェース
//////////////////////////////////////////////////////////////////////////////

REAL32	ECSResource::m_rTotalVol[ECSResource::ptfMax] =
{
	1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
} ;
EWaveMixingServer *			ECSResource::m_pWaveDev = NULL ;
EPtrObjArray<ECSResource> *	ECSResource::m_plstPlayRsrc = NULL ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO2( ECSResource, ECSObject, EGLSThread )
IMPLEMENT_CLASS_INFO( ECSResource::ESoundResource, MIOSoundStream )

// ECSResource::ESoundResource 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSResource::ESoundResource::ESoundResource( void )
{
	m_pOwnFile = NULL ;
}

// ECSResource::ESoundResource 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSResource::ESoundResource::~ESoundResource( void )
{
	Close( ) ;
	delete	m_pOwnFile ;
}

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSResource::ECSResource( ESLObject * pRsrc )
{
	m_vtType = csvtObject ;
	m_fOwnRsrc = rofNothing ;
	m_nPlayType = ptfNothing ;
	m_pRsrc = pRsrc ;
	m_rVolume[0] = 1.0F ;
	m_rVolume[1] = 1.0F ;
	m_fEnvelope = false ;
	m_hThreadReady = NULL ;
	m_nThreshold = -1 ;
	m_nRewindPos = -1 ;
	m_nStartPos = 0 ;
	m_nEndPos = -1 ;
	m_nRepeatPlaying = 2 ;
	//
	m_ppir = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSResource::~ECSResource( void )
{
	Release( ) ;
	//
	if ( m_ppir != NULL )
	{
		::eslHeapFree( NULL, m_ppir ) ;
	}
}

// 画像リソースを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::LoadImageFile
	( const wchar_t * pwszFilePath, ECSContext * pContext )
{
	Release( ) ;
	//
	ESLFileObject *	pfile = OpenResourceFile( pwszFilePath, pContext ) ;
	if ( pfile == NULL )
	{
		return	eslErrGeneral ;
	}
	ESLError	err = ReadImageFile( *pfile ) ;
	m_wstrFileName = pwszFilePath ;
	delete	pfile ;
	return	err ;
}

ESLError ECSResource::ReadImageFile( ESLFileObject & file )
{
	EGLMediaLoader *	pImage = new EGLMediaLoader ;
	ESLError	err = pImage->ReadMediaFile( file ) ;
	if ( err )
	{
		delete	pImage ;
		return	eslErrGeneral ;
	}
	m_fOwnRsrc = rofImage ;
	m_pRsrc = pImage ;
	//
	return	eslErrSuccess ;
}

// 音声リソースを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::LoadSoundFile
	( const wchar_t * pwszFilePath,
		unsigned int nThreshold, ECSContext * pContext )
{
	Release( ) ;
	//
	ESLFileObject *	pfile = OpenResourceFile( pwszFilePath, pContext ) ;
	if ( pfile == NULL )
	{
		return	eslErrGeneral ;
	}
	ESLError	err =
		ReadSoundFile( *pfile, nThreshold ) ;
	m_wstrFileName = pwszFilePath ;
	delete	pfile ;
	return	err ;
}

ESLError ECSResource::ReadSoundFile
	( ESLFileObject & file, unsigned int nThreshold )
{
	if ( (signed int) nThreshold < 0 )
	{
		nThreshold = 100000 ;
	}
	MIOSoundStream::PlayMode	pmMode = MIOSoundStream::pmDynamicPlay ;
	DWORD	dwFilePos = file.GetPosition( ) ;
	if ( nThreshold > 0 )
	do
	{
		ERIFile	erif ;
		if ( erif.Open( &file, ERIFile::otReadHeader ) )
		{
			break ;
		}
		if ( !(erif.m_fdwReadMask & ERIFile::rmSoundInfo) )
		{
			break ;
		}
		if ( erif.m_MIOInfHdr.dwAllSampleCount < nThreshold )
		{
			pmMode = MIOSoundStream::pmStaticPlay ;
		}
		else if ( (erif.m_MIOInfHdr.dwAllSampleCount < nThreshold * 4)
								|| (file.GetLength() < 0x40000) )
		{
			pmMode = MIOSoundStream::pmDynamicPlay ;
		}
		else
		{
			pmMode = MIOSoundStream::pmDynamicRead ;
		}
		erif.Close( ) ;
		file.Seek( dwFilePos, ESLFileObject::FromBegin ) ;
	}
	while ( false ) ;
	//
	ESLFileObject *	pTargetFile = &file ;
	ESoundResource *	pSound = new ESoundResource ;
	if ( pmMode == MIOSoundStream::pmDynamicRead )
	{
		pSound->m_pOwnFile = file.Duplicate( ) ;
		pTargetFile = pSound->m_pOwnFile ;
	}
	ESLError	err = pSound->Open( *pTargetFile, pmMode ) ;
	if ( err )
	{
		file.Seek( dwFilePos, ESLFileObject::FromBegin ) ;
		err = pSound->ReadWave( file ) ;
		if ( err )
		{
			delete	pSound ;
			return	eslErrGeneral ;
		}
	}
	m_fOwnRsrc = rofSound ;
	m_pRsrc = pSound ;
	m_nThreshold = nThreshold ;
	//
	return	eslErrSuccess ;
}

// MIDI リソースを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::LoadMidiFile
	( const wchar_t * pwszFilePath, ECSContext * pContext )
{
	Release( ) ;
	//
	ESLFileObject *	pfile = OpenResourceFile( pwszFilePath, pContext ) ;
	if ( pfile == NULL )
	{
		return	eslErrGeneral ;
	}
	ESLError	err = ReadMidiFile( *pfile ) ;
	delete	pfile ;
	m_wstrFileName = pwszFilePath ;
	return	err ;
}

ESLError ECSResource::ReadMidiFile( ESLFileObject & file )
{
	EMidiMusic *	pMidi = new EMidiMusic ;
	ESLError	err = pMidi->ReadMidi( file ) ;
	if ( err )
	{
		delete	pMidi ;
		return	eslErrGeneral ;
	}
	m_fOwnRsrc = rofMidi ;
	m_pRsrc = pMidi ;
	//
	return	eslErrSuccess ;
}

// 音声関連付け
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::AttachSound( ECSResource * pRsrc )
{
	ESLError	err ;
	EWaveSound *	pWave = ESLTypeCast<EWaveSound>( pRsrc->m_pRsrc ) ;
	if ( pWave != NULL )
	{
		MIOSoundStream *	pSound = new MIOSoundStream ;
		err = pSound->AttachWaveSound( *pWave ) ;
		if ( !err )
		{
			err = SetResource( pSound, rofSound, NULL ) ;
			if ( !err )
			{
				m_refAttachSound.SetReference( pRsrc ) ;
			}
			else
			{
				delete	pSound ;
			}
		}
		else
		{
			delete	pSound ;
		}
	}
	else
	{
		err = eslErrGeneral ;
	}
	return	err ;
}

// リソースを設定する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::SetResource
	( ESLObject * pRsrc,
		ECSResource::ResourceOwnFlag rofType, const wchar_t * pwszFileName )
{
	Release( ) ;
	if ( pRsrc == NULL )
	{
		return	eslErrSuccess ;
	}
	//
	switch ( rofType )
	{
	case	rofImage:
		ESLAssert( pRsrc->IsKindOf( ESL_RUNTIME_CLASS(EGLAnimation) ) ) ;
		break ;
	case	rofSound:
		ESLAssert( pRsrc->IsKindOf( ESL_RUNTIME_CLASS(MIOSoundStream) ) ) ;
		break ;
	case	rofMidi:
		ESLAssert( pRsrc->IsKindOf( ESL_RUNTIME_CLASS(EMidiMusic) ) ) ;
		break ;
	default:
		return	eslErrGeneral ;
	}
	//
	m_wstrFileName = pwszFileName ;
	m_fOwnRsrc = rofType ;
	m_pRsrc = pRsrc ;
	//
	return	eslErrSuccess ;
}

// リソースを解放する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Release( void )
{
	CancelVolumeEnvelope( ) ;
	if ( IsPlaying() )
	{
		Stop( ) ;
	}
//	if ( m_nPlayType != ptfNothing )
	{
		if ( m_plstPlayRsrc != NULL )
		{
			ECotophaScript::Lock( ) ;
			for ( int i = 0; i < (int) m_plstPlayRsrc->GetSize(); i ++ )
			{
				if ( m_plstPlayRsrc->GetAt(i) == this )
				{
					m_plstPlayRsrc->RemoveAt( i -- ) ;
				}
			}
			m_nPlayType = ptfNothing ;
			ECotophaScript::Unlock( ) ;
		}
	}
	if ( m_fOwnRsrc && m_pRsrc )
	{
		delete	m_pRsrc ;
	}
	m_fOwnRsrc = rofNothing ;
	m_pRsrc = NULL ;
	m_wstrFileName.FreeString( ) ;
	m_refAttachSound.SetReference( NULL ) ;
	//
	return	eslErrSuccess ;
}

// 画像リソースを保存する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::SaveImageFile
	( const wchar_t * pwszFilePath,
		const wchar_t * pwszMimeType,
		int nQuality, ECSContext * pContext ) const
{
	if ( !EWideString::Compare( pwszMimeType, L"video/avi" ) )
	{
		EGLMediaLoader *	pImage =
			ESLTypeCast<EGLMediaLoader>( GetResource() ) ;
		if ( pImage != NULL )
		{
			ECSEnvironment *	pEnv = NULL ;
			EString				strFilePath = pwszFilePath ;
			if ( pContext != NULL )
			{
				pEnv = pContext->GetEnvironment() ;
				if ( pEnv != NULL )
				{
					strFilePath =
						pEnv->m_strSaveDir.OffsetFilePath( strFilePath ) ;
				}
			}
			return	pImage->SaveAviFile( strFilePath ) ;
		}
		else
		{
			return	eslErrGeneral ;
		}
	}
	ESLFileObject *	pfile = NULL ;
	if ( pContext != NULL )
	{
		pfile = pContext->OpenFileOnScript
				( pwszFilePath, ESLFileObject::modeCreate ) ;
	}
	else
	{
		ERawFile *	pRawFile = new ERawFile ;
		if ( !pRawFile->Open
			( EString(pwszFilePath), ESLFileObject::modeCreate ) )
		{
			pfile = pRawFile ;
		}
		else
		{
			delete	pRawFile ;
		}
	}
	if ( pfile == NULL )
	{
		return	eslErrGeneral ;
	}
	ESLError	err = WriteImageFile( *pfile, pwszMimeType, nQuality ) ;
	delete	pfile ;
	return	err ;
}

ESLError ECSResource::WriteImageFile
	( ESLFileObject & file, const wchar_t * pwszMimeType, int nQuality ) const
{
	EGLMediaLoader	imgTemp ;
	EGLMediaLoader *	pImage =
		ESLTypeCast<EGLMediaLoader>( GetResource() ) ;
	if ( pImage == NULL )
	{
		EGLImage *	pEGLImage = ESLTypeCast<EGLImage>( GetResource() ) ;
		if ( pEGLImage == NULL )
		{
			return	eslErrGeneral ;
		}
		PEGL_IMAGE_INFO	pImageBuf = ::eglDuplicateImageBuffer( *pEGLImage ) ;
		imgTemp.AddFrame( pImageBuf ) ;
		imgTemp.AttachImage( pImageBuf ) ;
		pImage = &imgTemp ;
	}
	if ( !EWideString::Compare( pwszMimeType, L"image/x-eri" ) )
	{
		return	pImage->WriteImageFile
					( file, EGLImage::ctfCompatibleFormat ) ;
	}
	if ( !EWideString::Compare( pwszMimeType, L"image/x-erina" ) )
	{
		return	pImage->WriteImageFile
					( file, EGLImage::ctfExtendedFormat ) ;
	}
	if ( !EWideString::Compare( pwszMimeType, L"image/x-erisa" ) )
	{
		return	pImage->WriteImageFile
					( file, EGLImage::ctfSuperiorArchitecure ) ;
	}
	if ( !EWideString::Compare( pwszMimeType, L"image/bmp" ) )
	{
		return	pImage->WriteBitmapFile( file ) ;
	}
	if ( !EWideString::Compare( pwszMimeType, L"image/x-photoshop" ) )
	{
		return	pImage->WritePhotoshopPSDFile( file ) ;
	}
	if ( !EWideString::Compare( pwszMimeType, L"image/x-icon" ) )
	{
		EWin32IconFile	icof ;
		BITMAPINFO *	pbmi = pImage->CreatePackedDIB( ) ;
		pbmi->bmiHeader.biHeight *= 2 ;
		//
		DWORD	dwImageSize = pbmi->bmiHeader.biSizeImage ;
		DWORD	dwPalBytes =
				pbmi->bmiHeader.biClrUsed * sizeof(RGBQUAD) ;
		if ( (dwPalBytes == 0) && (pbmi->bmiHeader.biBitCount <= 8) )
		{
			dwPalBytes = sizeof(RGBQUAD) << pbmi->bmiHeader.biBitCount ;
		}
		DWORD	dwBufSize = sizeof(BITMAPINFOHEADER) + dwPalBytes + dwImageSize ;
		//
		EStreamBuffer	buf ;
		void *		ptrBuf = buf.PutBuffer( dwBufSize + dwImageSize ) ;
		::memset( ptrBuf, 0, dwBufSize + dwImageSize ) ;
		::memmove( ptrBuf, pbmi, dwBufSize ) ;
		dwBufSize += dwImageSize ;
		icof.AddImage( (BITMAPINFOHEADER*) ptrBuf, dwBufSize ) ;
		::eslHeapFree( NULL, pbmi ) ;
		//
		return	icof.WriteIconFile( file ) ;
	}
	return	pImage->WriteWithGDIplus( file, pwszMimeType, nQuality ) ;
}

// リソースを取得
//////////////////////////////////////////////////////////////////////////////
PEGL_IMAGE_INFO ECSResource::GetImageInfo( void ) const
{
	EGLAnimation *	pImage = GetImage() ;
	if ( pImage != NULL )
	{
		return	*pImage ;
	}
	return	NULL ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLFileObject * ECSResource::OpenResourceFile
	( const wchar_t * pwszFilePath, ECSContext * pContext )
{
	if ( pContext != NULL )
	{
		return	pContext->OpenFileOnScript( pwszFilePath ) ;
	}
	ERawFile *	pfile = new ERawFile ;
	if ( pfile->Open
		( EString(pwszFilePath),
			ESLFileObject::modeRead | ESLFileObject::shareRead ) )
	{
		delete	pfile ;
		return	NULL ;
	}
	return	pfile ;
}

// 音声リソースを再生する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Play( unsigned int nIntroSamples, int fPlayType )
{
	Stop( ) ;
	//
	MIOSoundStream *	pSound = ESLTypeCast<MIOSoundStream>( m_pRsrc ) ;
	EMidiMusic *	pMidi ;
	bool	fRepeatPlaying = false ;
	if ( pSound != NULL )
	{
		if ( m_pWaveDev == NULL )
		{
			return	eslErrGeneral ;
		}
		m_nPlayType = fPlayType ;
		pSound->AttachWaveDevice( m_pWaveDev ) ;
		SetVolume( m_rVolume[0], m_rVolume[1] ) ;
		const ERIFile &	erif = pSound->GetMIOPlayer().GetERIFile() ;
		ULONG			nRewindPos = 0 ;
		unsigned int	nLoopEndPos	= (unsigned int) -1 ;
		bool			fLoopPoint = false ;
		if ( erif.m_fdwReadMask & erif.rmDescription )
		{
			ERIFile::ETagInfo	taginf ;
			taginf.CreateTagInfo( erif.m_wstrDescription ) ;
			const wchar_t *	pwszRewindPoint =
				taginf.GetTagContents( erif.tagRewindPoint ) ;
			if ( pwszRewindPoint != NULL )
			{
				nRewindPos = taginf.GetRewindPoint() ;
				nLoopEndPos = taginf.GetLoopEndPoint() ;
				fRepeatPlaying = true ;
			}
		}
		if ( (signed int) nIntroSamples >= 0 )
		{
			nRewindPos = nIntroSamples ;
			fRepeatPlaying = true ;
		}
		else if ( (signed int) nIntroSamples == -2 )
		{
			fRepeatPlaying = false ;
		}
		ESLError	err =
			pSound->PlayFrom( 0, nLoopEndPos, fRepeatPlaying, nRewindPos ) ;
		if ( err )
		{
			return	err ;
		}
	}
	else if ( (pMidi = GetMidiMusic()) != NULL )
	{
		m_nPlayType = fPlayType ;
		SetVolume( m_rVolume[0], m_rVolume[1] ) ;
		fRepeatPlaying = ((signed int) nIntroSamples >= 0) ;
		ESLError	err = pMidi->PlayMidi( fRepeatPlaying ) ;
		if ( err )
		{
			return	err ;
		}
	}
	else
	{
		return	eslErrGeneral ;
	}
	//
	ESLAssert( m_plstPlayRsrc != NULL ) ;
	if ( m_plstPlayRsrc == NULL )
	{
		ECotophaScript::Initialize( ) ;
	}
	ECotophaScript::Lock( ) ;
	m_nRewindPos = nIntroSamples ;
	m_nStartPos = 0 ;
	m_nEndPos = -1 ;
	m_nRepeatPlaying = fRepeatPlaying ;
	m_nPlayType = fPlayType ;
	m_plstPlayRsrc->Add( this ) ;
	ECotophaScript::Unlock( ) ;
	//
	return	eslErrSuccess ;
}

ESLError ECSResource::PlayFrom
	( unsigned int nStartPos, unsigned int nPlayEnd,
		bool fRepeat, unsigned int nRewindPos, int fPlayType )
{
	Stop( ) ;
	//
	MIOSoundStream *	pSound = ESLTypeCast<MIOSoundStream>( m_pRsrc ) ;
	EMidiMusic *	pMidi ;
	if ( pSound != NULL )
	{
		if ( m_pWaveDev == NULL )
		{
			return	eslErrGeneral ;
		}
		m_nPlayType = fPlayType ;
		pSound->AttachWaveDevice( m_pWaveDev ) ;
		SetVolume( m_rVolume[0], m_rVolume[1] ) ;
		//
		ESLError	err =
			pSound->PlayFrom( nStartPos, nPlayEnd, fRepeat, nRewindPos ) ;
		if ( err )
		{
			return	err ;
		}
	}
	else if ( (pMidi = GetMidiMusic()) != NULL )
	{
		m_nPlayType = fPlayType ;
		SetVolume( m_rVolume[0], m_rVolume[1] ) ;
		ESLError	err = pMidi->PlayMidi( fRepeat ) ;
		if ( err )
		{
			return	err ;
		}
	}
	else
	{
		return	eslErrGeneral ;
	}
	//
	ESLAssert( m_plstPlayRsrc != NULL ) ;
	if ( m_plstPlayRsrc == NULL )
	{
		ECotophaScript::Initialize( ) ;
	}
	ECotophaScript::Lock( ) ;
	m_nRewindPos = nRewindPos ;
	m_nStartPos = nStartPos ;
	m_nEndPos = nPlayEnd ;
	m_nRepeatPlaying = fRepeat ? 0 : 1 ;
	m_nPlayType = fPlayType ;
	m_plstPlayRsrc->Add( this ) ;
	ECotophaScript::Unlock( ) ;
	//
	return	eslErrSuccess ;
}

// 再生ループポイント設定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::SetRewindingPortion
	( unsigned int nRewindPos, unsigned int nEndPos, bool fRepeat )
{
	ESLError	err = eslErrSuccess ;
	MIOSoundStream *	pSound = ESLTypeCast<MIOSoundStream>( m_pRsrc ) ;
	if ( pSound != NULL )
	{
		if ( (signed int) nRewindPos < 0 )
		{
			nRewindPos = pSound->GetRewoundPosition() ;
		}
		if ( nEndPos == (unsigned int) -1 )
		{
			nEndPos = pSound->GetWaveSamples() ;
			if ( nEndPos == 0 )
			{
				nEndPos = pSound->GetMIOPlayer().GetTotalSampleCount() ;
			}
		}
		err = pSound->SetRewindingPortion( nRewindPos, nEndPos, fRepeat ) ;
	}
	else
	{
		err = eslErrGeneral ;
	}
	return	err ;
}

// 音声リソースの再生を停止する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Stop( void )
{
	ESLAssert( m_plstPlayRsrc != NULL ) ;
	if ( m_plstPlayRsrc != NULL )
	{
		ECotophaScript::Lock( ) ;
		for ( int i = 0; i < (int) m_plstPlayRsrc->GetSize(); i ++ )
		{
			if ( m_plstPlayRsrc->GetAt(i) == this )
			{
				m_plstPlayRsrc->RemoveAt( i -- ) ;
			}
		}
		m_nPlayType = ptfNothing ;
		ECotophaScript::Unlock( ) ;
	}
	//
	MIOSoundStream *	pSound = ESLTypeCast<MIOSoundStream>( m_pRsrc ) ;
	EMidiMusic *	pMidi ;
	if ( pSound != NULL )
	{
		return	pSound->StopWave( ) ;
	}
	else if ( (pMidi = GetMidiMusic()) != NULL )
	{
		return	pMidi->StopMidi( ) ;
	}
	return	eslErrGeneral ;
}

// 音声リソースの再生を一時停止する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Pause( void )
{
	MIOSoundStream *	pSound = ESLTypeCast<MIOSoundStream>( m_pRsrc ) ;
	EMidiMusic *	pMidi ;
	if ( pSound != NULL )
	{
		return	pSound->PauseWave( ) ;
	}
	else if ( (pMidi = GetMidiMusic()) != NULL )
	{
		return	pMidi->PauseMidi( ) ;
	}
	return	eslErrGeneral ;
}

// 音声リソースの再生を再開する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Restart( void )
{
	MIOSoundStream *	pSound = ESLTypeCast<MIOSoundStream>( m_pRsrc ) ;
	EMidiMusic *	pMidi ;
	if ( pSound != NULL )
	{
		return	pSound->RestartWave( ) ;
	}
	else if ( (pMidi = GetMidiMusic()) != NULL )
	{
		return	pMidi->RestartMidi( ) ;
	}
	return	eslErrGeneral ;
}

// 音量を取得する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::GetVolume( REAL32 & rLeftVol, REAL32 & rRightVol )
{
	rLeftVol = m_rVolume[0] ;
	rRightVol = m_rVolume[1] ;
	return	eslErrSuccess ;
}

// 音量を設定する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::SetVolume( REAL32 rLeftVol, REAL32 rRightVol )
{
	m_rVolume[0] = rLeftVol ;
	m_rVolume[1] = rRightVol ;
	//
	if ( (m_nPlayType > ptfNothing) && (m_nPlayType < ptfMax) )
	{
		MIOSoundStream *	pSound = ESLTypeCast<MIOSoundStream>( m_pRsrc ) ;
		EMidiMusic *	pMidi ;
		if ( pSound != NULL )
		{
			pSound->SetVolume
				( rLeftVol * m_rTotalVol[m_nPlayType],
					rRightVol * m_rTotalVol[m_nPlayType] ) ;
		}
		else if ( (pMidi = GetMidiMusic()) != NULL )
		{
			double	rVol =
				(rLeftVol * m_rTotalVol[m_nPlayType]
					+ rRightVol * m_rTotalVol[m_nPlayType]) * 0.5 ;
			DWORD	dwVol =
				(DWORD) ::eriRoundR64ToLInt( rVol * 0xFFFF ) ;
			pMidi->SetVolume( dwVol ) ;
		}
	}
	return	eslErrSuccess ;
}

// 再生中か調べる
//////////////////////////////////////////////////////////////////////////////
bool ECSResource::IsPlaying( void )
{
	MIOSoundStream *	pSound = ESLTypeCast<MIOSoundStream>( m_pRsrc ) ;
	EMidiMusic *	pMidi ;
	if ( pSound != NULL )
	{
		return	(pSound->IsPlaying() != 0) ;
	}
	else if ( (pMidi = GetMidiMusic()) != NULL )
	{
		return	(pMidi->IsPlaying() != 0) ;
	}
	return	false ;
}

// 再生中の位置を取得する
//////////////////////////////////////////////////////////////////////////////
UINT64 ECSResource::GetPlayingPosition( void )
{
	MIOSoundStream *	pSound = ESLTypeCast<MIOSoundStream>( m_pRsrc ) ;
	if ( (pSound != NULL) && (m_pWaveDev != NULL) )
	{
		return	m_pWaveDev->GetCurrentSample( pSound ) ;
	}
	return	0 ;
}

// 音量エンベロープを設定する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::SetVolumeEnvelope
	( const EBezierCurves<E3D_VECTOR_2D> & bezier,
						unsigned int nDurationTime )
{
	if ( (ESLTypeCast<EWaveSound>( m_pRsrc ) == NULL)
							&& (GetMidiMusic() == NULL) )
	{
		return	eslErrGeneral ;
	}
	//
	CancelVolumeEnvelope( ) ;
	//
	if ( nDurationTime == 0 )
	{
		SetVolume( bezier[3].x, bezier[3].y ) ;
	}
	else
	{
		m_bzVolume = bezier ;
		m_hThreadReady = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
		BeginThread( ) ;
		::WaitForSingleObject( m_hThreadReady, INFINITE ) ;
		::CloseHandle( m_hThreadReady ) ;
		m_hThreadReady = NULL ;
		m_fEnvelope = true ;
		PostThreadMessage( WM_USER + 1, nDurationTime, 0 ) ;
	}
	//
	return	eslErrSuccess ;
}

// 音量エンベロープをキャンセルする
//////////////////////////////////////////////////////////////////////////////
void ECSResource::CancelVolumeEnvelope( void )
{
	if ( Handle() != NULL )
	{
		PostThreadMessage( WM_QUIT, 0, 0 ) ;
		::WaitForSingleObject( Handle(), 10000 ) ;
		CloseThread( ) ;
	}
}

// 音量エンベロープ実行中か調べる
//////////////////////////////////////////////////////////////////////////////
bool ECSResource::IsPendingEnvelope( void )
{
	return	m_fEnvelope ;
}

// 全体音量を取得する
//////////////////////////////////////////////////////////////////////////////
REAL32 ECSResource::GetTotalVolume( int fPlayType )
{
	if ( fPlayType == ptfDevice )
	{
		if ( m_pWaveDev != NULL )
		{
			unsigned int	nDevVol[2] = { 0xFFFF, 0xFFFF } ;
			if ( !m_pWaveDev->GetTotalVolume( nDevVol ) )
			{
				return	(REAL32)
					((double) (nDevVol[0] + nDevVol[1]) / 0x1FFFE) ;
			}
		}
	}
	if ( (fPlayType > ptfNothing) && (fPlayType < ptfMax) )
	{
		return	m_rTotalVol[fPlayType] ;
	}
	return	1.0 ;
}

// 全体音量を設定する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::SetTotalVolume( int fPlayType, REAL32 rVolume )
{
	if ( fPlayType == ptfDevice )
	{
		if ( m_pWaveDev != NULL )
		{
			unsigned int	nDevVol[2] ;
			nDevVol[0] = ::eriRoundR32ToInt( (REAL32) (rVolume * 0xFFFF) ) ;
			nDevVol[1] = nDevVol[0] ;
			return	m_pWaveDev->SetTotalVolume( nDevVol ) ;
		}
	}
	if ( (fPlayType > ptfNothing) && (fPlayType < ptfMax) )
	{
		m_rTotalVol[fPlayType] = rVolume ;
		//
		ESLAssert( m_plstPlayRsrc != NULL ) ;
		if ( m_plstPlayRsrc != NULL )
		{
			ECotophaScript::Lock( ) ;
			for ( int i = 0; i < (int) m_plstPlayRsrc->GetSize(); i ++ )
			{
				ECSResource *	prsSound = m_plstPlayRsrc->GetAt( i ) ;
				if ( prsSound == NULL )
					continue ;
				if ( prsSound->m_nPlayType == fPlayType )
				{
					prsSound->SetVolume
						( prsSound->m_rVolume[0], prsSound->m_rVolume[1] ) ;
				}
			}
			ECotophaScript::Unlock( ) ;
		}
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
DWORD ECSResource::ThreadProc( void )
{
	MSG		msg ;
	::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ;
	::SetTimer( NULL, 0, 66, NULL ) ;
	::SetEvent( m_hThreadReady ) ;
	//
	DWORD	dwExit = EGLSThread::ThreadProc( ) ;
	m_fEnvelope = false ;
	return	dwExit ;
}

// スレッドメッセージ処理
//////////////////////////////////////////////////////////////////////////////
void ECSResource::DispatchMessage( const MSG & msg )
{
	if ( msg.hwnd == NULL )
	{
		if ( msg.message == WM_USER + 1 )
		{
			BeginTime( msg.wParam, 0x10000 ) ;
			m_fEnvelope = true ;
		}
		if ( m_fEnvelope )
		{
			int		nOffsetTime = GetOffsetTime( ) ;
			E3DVector2D	v =
				m_bzVolume.pt( (double) nOffsetTime / 0x10000 ) ;
			SetVolume( v.x, v.y ) ;
			//
			if ( nOffsetTime >= 0x10000 )
			{
				PostThreadMessage( WM_QUIT, 0, 0 ) ;
			}
		}
	}
	else
	{
		EGLSThread::DispatchMessage( msg ) ;
	}
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSResource::GetTypeName( void ) const
{
	return	L"Resource" ;
}

ECSObject * ECSResource::GetTypeOf( const wchar_t * pwszTypeName )
{
	if ( !EWideString::Compare( pwszTypeName, L"Resource" ) )
	{
		return	this ;
	}
	return	ECSObject::GetTypeOf( pwszTypeName ) ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSResource::Duplicate( void )
{
	ECSResource *	pRsrc = new ECSResource ;
	if ( m_fOwnRsrc == rofNothing )
	{
		pRsrc->m_pRsrc = m_pRsrc ;
	}
	return	pRsrc ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Move( ECSContext & context, ECSObject * obj )
{
	ECSResource *	pRsrc =
		ESLTypeCast<ECSResource>( ECSObject::GetEntity( obj ) ) ;
	if ( pRsrc == NULL )
	{
		return	ESLErrorMsg
			( "Resource オブジェクトに Resource 以外の"
					"オブジェクトを代入しようとしました。" ) ;
	}
	//
	Release( ) ;
	//
	if ( pRsrc->m_fOwnRsrc == rofNothing )
	{
		m_pRsrc = pRsrc->m_pRsrc ;
	}
	context.delete_CSObject( obj ) ;
	//
	return	eslErrSuccess ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	return	ESLErrorMsg( "Resource 型には単項演算子は定義されていません。" ) ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	return	ESLErrorMsg( "Resource 型には二項演算子は定義されていません。" ) ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	return	ESLErrorMsg( "Resource 型には比較演算子は定義されていません。" ) ;
}

// メンバ変数取得
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSResource::GetVariableAt( int nIndex )
{
	if ( nIndex == -1 )
	{
		return	&m_refAttachSound ;
	}
	return	NULL ;
}

// メンバ変数設定
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSResource::SetVariableAt( int nIndex, ECSObject * obj )
{
	return	NULL ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	nIndex = m_staFuncName->FindIndex( pwszName ) ;
	if ( nIndex >= 0 )
	{
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg
		( "Resource 型の定義されていないメンバ関数を呼び出そうとしています。" ) ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	if ( (nIndex < 0) || (nIndex >= (int) m_staFuncName->GetSize()) )
	{
		return	ESLErrorMsg
			( "Resource 型の定義されていないメンバ関数を呼び出そうとしています。" ) ;
	}
	return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
}

// 全てのメンバ変数にインデックスを振る
//////////////////////////////////////////////////////////////////////////////
void ECSResource::IndexAllMember( void )
{
	m_refAttachSound.IndexAllMember( ) ;
	m_refAttachSound.m_pParent = this ;
	m_refAttachSound.m_nIndex = -1 ;
}

// 全てのメンバ変数の参照を解消する
//////////////////////////////////////////////////////////////////////////////
void ECSResource::CleanupAllReference( ECSContext & context )
{
	m_refAttachSound.CleanupAllReference( context ) ;
	//
	ECSObject::CleanupAllReference( context ) ;
}

// 全てのメンバ変数の参照を解決する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::CommitAllReference( ECSContext & context )
{
	ESLError	err = m_refAttachSound.CommitAllReference( context ) ;
	if ( err )
	{
//		return	err ;
	}
	int	nPlayType = m_nPlayType ;
	int	nRewindPos = m_nRewindPos ;
	int	nStartPos = m_nStartPos ;
	int	nEndPos = m_nEndPos ;
	int	nRepeatPlaying = m_nRepeatPlaying ;
	ECSResource *	pRsrc =
		ESLTypeCast<ECSResource>( m_refAttachSound.m_pRef ) ;
	if ( pRsrc != NULL )
	{
		err = AttachSound( pRsrc ) ;
	}
	if ( (nPlayType != ptfNothing)
		&& (nRepeatPlaying || (nPlayType == ptfMusic)) )
	{
		err = PlayFrom
			( nStartPos, nEndPos,
				(nRepeatPlaying != 0), nRewindPos, nPlayType ) ;
	}
	return	err ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Save( ESLFileObject & file, ECSContext & context )
{
	ESLError	err ;
	REAL32	rVolume[2] = { m_rVolume[0], m_rVolume[1] } ;
	if ( m_fEnvelope )
	{
//		CancelVolumeEnvelope() ;
		//
		E3DVector2D	v = m_bzVolume.pt( 1.0 ) ;
//		SetVolume( v.x, v.y ) ;
		rVolume[0] = v.x ;
		rVolume[1] = v.y ;
	}
	//
	if ( m_nPlayType != ptfNothing )
	{
		if ( !IsPlaying() )
		{
			m_nPlayType = ptfNothing ;
		}
		MIOSoundStream *
			pSound = ESLTypeCast<MIOSoundStream>( m_pRsrc ) ;
		if ( pSound != NULL )
		{
			m_nRepeatPlaying = pSound->IsRepeated( ) ;
			if ( !m_nRepeatPlaying )
			{
				m_nStartPos = pSound->GetCurrentOutput( ) ;
			}
		}
	}
	file.Write( &m_fOwnRsrc, sizeof(m_fOwnRsrc) ) ;
	file.Write( &m_nPlayType, sizeof(m_nPlayType) ) ;
	file.Write( &m_nThreshold, sizeof(m_nThreshold) ) ;
	file.Write( &m_nRewindPos, sizeof(m_nRewindPos) ) ;
	file.Write( &m_nStartPos, sizeof(m_nStartPos) ) ;
	file.Write( &m_nEndPos, sizeof(m_nEndPos) ) ;
	file.Write( &m_nRepeatPlaying, sizeof(m_nRepeatPlaying) ) ;
	file.Write( rVolume, sizeof(rVolume) ) ;
	//
	DWORD	dwLength = m_wstrFileName.GetLength() ;
	file.Write( &dwLength, sizeof(dwLength) ) ;
	if ( dwLength > 0 )
	{
		file.Write( m_wstrFileName.CharPtr(), dwLength * sizeof(wchar_t) ) ;
	}
	err = m_refAttachSound.Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Load( ESLFileObject & file, ECSContext & context )
{
	ESLError		err ;
	ResourceOwnFlag	fOwnRsrc ;
	int				nPlayType ;
	unsigned int	nThreshold ;
	unsigned int	nRewindPos, nStartPos, nEndPos, nRepeatPlaying ;
	file.Read( &fOwnRsrc, sizeof(fOwnRsrc) ) ;
	file.Read( &nPlayType, sizeof(nPlayType) ) ;
	file.Read( &nThreshold, sizeof(nThreshold) ) ;
	file.Read( &nRewindPos, sizeof(nRewindPos) ) ;
	file.Read( &nStartPos, sizeof(nStartPos) ) ;
	file.Read( &nEndPos, sizeof(nEndPos) ) ;
	file.Read( &nRepeatPlaying, sizeof(nRepeatPlaying) ) ;
	file.Read( m_rVolume, sizeof(m_rVolume) ) ;
	//
	DWORD	dwLength ;
	if ( file.Read( &dwLength, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	eslErrGeneral ;
	}
	if ( dwLength > 0 )
	{
		ECSWideString	wstrFileName ;
		file.Read
			( wstrFileName.GetBuffer(dwLength),
						dwLength * sizeof(wchar_t) ) ;
		wstrFileName.ReleaseBuffer( dwLength ) ;
		//
		err = eslErrSuccess ;
		if ( fOwnRsrc == rofImage )
		{
			err = LoadImageFile( wstrFileName, &context ) ;
		}
		else if ( fOwnRsrc == rofSound )
		{
			err = LoadSoundFile( wstrFileName, nThreshold, &context ) ;
		}
		else if ( fOwnRsrc == rofMidi )
		{
			err = LoadMidiFile( wstrFileName, &context ) ;
		}
		if ( err )
		{
			// ロード時にファイルがなくてもエラーとはしない
//			return	err ;
		}
	}
	else
	{
		m_wstrFileName.FreeString( ) ;
	}
	err = m_refAttachSound.Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	m_nPlayType = nPlayType ;
	m_nRewindPos = nRewindPos ;
	m_nStartPos = nStartPos ;
	m_nEndPos = nEndPos ;
	m_nRepeatPlaying = nRepeatPlaying ;
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	EString	strDump ;
	if ( m_fOwnRsrc == rofImage )
	{
		strDump = "Image : " + EString(m_wstrFileName) ;
	}
	else if ( m_fOwnRsrc == rofSound )
	{
		strDump = "Sound : " + EString(m_wstrFileName) ;
	}
	else
	{
		strDump = "[no-resource]" ;
	}
	buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	return	eslErrSuccess ;
}

// メンバ関数ポインタ
//////////////////////////////////////////////////////////////////////////////
ECSStrTagArray *	ECSResource::m_staFuncName = NULL ;
const wchar_t *	ECSResource::m_pwszFuncName[29] =
{
	L"LoadImage", L"LoadSound", L"LoadMidi", L"SaveImage",
	L"Release", L"AttachSound",
	L"Play", L"PlayFrom", L"SetRewindingPortion", L"Stop", L"Pause",
	L"Restart", L"GetVolume", L"SetVolume", L"GetTotalVolume", L"SetTotalVolume",
	L"IsPlaying", L"GetPlayingPosition",
	L"GetInfo", L"GetImageInfo", L"GetSoundInfo",
	L"SetVolumeEnvelope", L"CancelVolumeEnvelope",
	L"IsPendingEnvelope", L"GetPixel", L"GetPixelRect", L"SetPixelRect",
	L"GetWaveData",
	NULL
} ;
const ECSResource::PFUNC_CALL	ECSResource::m_pfnCallFunc[28] =
{
	&ECSResource::Call_LoadImage,
	&ECSResource::Call_LoadSound,
	&ECSResource::Call_LoadMidi,
	&ECSResource::Call_SaveImage,
	&ECSResource::Call_Release,
	&ECSResource::Call_AttachSound,
	&ECSResource::Call_Play,
	&ECSResource::Call_PlayFrom,
	&ECSResource::Call_SetRewindingPortion,
	&ECSResource::Call_Stop,
	&ECSResource::Call_Pause,
	&ECSResource::Call_Restart,
	&ECSResource::Call_GetVolume,
	&ECSResource::Call_SetVolume,
	&ECSResource::Call_GetTotalVolume,
	&ECSResource::Call_SetTotalVolume,
	&ECSResource::Call_IsPlaying,
	&ECSResource::Call_GetPlayingPosition,
	&ECSResource::Call_GetInfo,
	&ECSResource::Call_GetImageInfo,
	&ECSResource::Call_GetSoundInfo,
	&ECSResource::Call_SetVolumeEnvelope,
	&ECSResource::Call_CancelVolumeEnvelope,
	&ECSResource::Call_IsPendingEnvelope,
	&ECSResource::Call_GetPixel,
	&ECSResource::Call_GetPixelRect,
	&ECSResource::Call_SetPixelRect,
	&ECSResource::Call_GetWaveData,
} ;

// メンバ関数 : Integer LoadImage( String filename )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_LoadImage
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrFileName ;
	err = context.GetArgumentAsStr( wstrFileName, lstArg, 1, NULL ) ;
	if ( err )
	{
		ECSFile *	pfile =
			ESLTypeCast<ECSFile>
				( context.GetArgumentObjectAs( lstArg, 1, L"File" ) ) ;
		if ( (pfile == NULL)
			|| (pfile->GetFileInterface() == NULL) )
		{
			return	err ;
		}
		err = ReadImageFile( *(pfile->GetFileInterface()) ) ;
	}
	else
	{
		err = LoadImageFile( wstrFileName, &context ) ;
	}
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer LoadSound( String filename, Integer nThreshold := -1 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_LoadSound
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrFileName ;
	int				nThreshold ;
	err = context.GetArgumentAsInt( nThreshold, lstArg, 2, -1 ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsStr( wstrFileName, lstArg, 1, NULL ) ;
	if ( err )
	{
		ECSFile *	pfile =
			ESLTypeCast<ECSFile>
				( context.GetArgumentObjectAs( lstArg, 1, L"File" ) ) ;
		if ( (pfile == NULL)
			|| (pfile->GetFileInterface() == NULL) )
		{
			return	err ;
		}
		err = ReadSoundFile( *(pfile->GetFileInterface()), nThreshold ) ;
	}
	else
	{
		err = LoadSoundFile( wstrFileName, nThreshold, &context ) ;
	}
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer LoadMidi( String filename )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_LoadMidi
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrFileName ;
	err = context.GetArgumentAsStr( wstrFileName, lstArg, 1, NULL ) ;
	if ( err )
	{
		ECSFile *	pfile =
			ESLTypeCast<ECSFile>
				( context.GetArgumentObjectAs( lstArg, 1, L"File" ) ) ;
		if ( (pfile == NULL)
			|| (pfile->GetFileInterface() == NULL) )
		{
			return	err ;
		}
		err = ReadMidiFile( *(pfile->GetFileInterface()) ) ;
	}
	else
	{
		err = LoadMidiFile( wstrFileName, &context ) ;
	}
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer SaveImage
//		( String filename,
//			String sMimeType := "image/x-eri", Integer nQuality := -1 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_SaveImage
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 4 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrFileName ;
	ECSWideString	wstrMimeType ;
	int				nQuality ;
	err = context.GetArgumentAsStr( wstrFileName, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsStr( wstrMimeType, lstArg, 2, L"image/x-eri" ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( nQuality, lstArg, 3, -1 ) ;
	if ( err )
	{
		return	err ;
	}
	err = SaveImageFile( wstrFileName, wstrMimeType, nQuality, &context ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Release()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_Release
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	Release( ) ;
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : AttachSound( sound )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_AttachSound
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSResource *	pRsrc =
		ESLTypeCast<ECSResource>
			( context.GetArgumentObjectAs( lstArg, 1, L"Resource" ) ) ;
	if ( pRsrc != NULL )
	{
		err = AttachSound( pRsrc ) ;
	}
	else
	{
		err = eslErrGeneral ;
	}
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 :
//	Integer Play( Integer nIntroSample := -1, Integer flag := ptfMusic )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_Play
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 3 ) ;
	if ( err )
		return	err ;
	//
	int		nPlayType, nIntroSample ;
	err = context.GetArgumentAsInt( nIntroSample,lstArg, 1, -1 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nPlayType, lstArg, 2, ptfMusic ) ;
	if ( err )
		return	err ;
	//
	err = Play( nIntroSample, nPlayType ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : PlayFrom
//		( Integer nStartPos := 0, Integer nEndPos := -1,
//			Integer fRepeat := false, Integer nRewindPos := -1,
//			Integer nPlayFlag := ptfMusic )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_PlayFrom
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 6 ) ;
	if ( err )
		return	err ;
	//
	int		nStartPos, nEndPos, fRepeat, nRewindPos, nPlayFlag ;
	err = context.GetArgumentAsInt( nStartPos,lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nEndPos, lstArg, 2, -1 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( fRepeat, lstArg, 3, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nRewindPos, lstArg, 4, -1 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nPlayFlag, lstArg, 5, ptfMusic ) ;
	if ( err )
		return	err ;
	//
	err = PlayFrom
		( nStartPos, nEndPos, (fRepeat != 0), nRewindPos, nPlayFlag ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : SetRewindingPortion
//		( Integer nRewindPos := -1,
//			Integer nEndPos := -1, Integer fRepeat := true )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_SetRewindingPortion
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 4 ) ;
	if ( err )
		return	err ;
	//
	int		nRewindPos, nEndPos, fRepeat ;
	err = context.GetArgumentAsInt( nRewindPos, lstArg, 1, -1 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nEndPos, lstArg, 2, -1 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( fRepeat, lstArg, 3, 0 ) ;
	if ( err )
		return	err ;
	//
	err = SetRewindingPortion( nRewindPos, nEndPos, (fRepeat != 0) ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Stop()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_Stop
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	CancelVolumeEnvelope( ) ;
	//
	return	context.PushObject( new ECSInteger( Stop() ) ) ;
}

// メンバ関数 : Pause()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_Pause
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( Pause() ) ) ;
}

// メンバ関数 : Restart()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_Restart
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( Restart() ) ) ;
}

// メンバ関数 : GetVolume( Reference rLeftVol, Reference rRightVol )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_GetVolume
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSReal *	pLeftVol =
		ESLTypeCast<ECSReal>
			( context.GetArgumentObjectAs( lstArg, 1, L"Real" ) ) ;
	if ( pLeftVol == NULL )
		return	ESLErrorMsg( "引数の型が不正です。" ) ;
	ECSReal *	pRightVol =
		ESLTypeCast<ECSReal>
			( context.GetArgumentObjectAs( lstArg, 2, L"Real" ) ) ;
	if ( pRightVol == NULL )
		return	ESLErrorMsg( "引数の型が不正です。" ) ;
	//
	pLeftVol->m_varReal = m_rVolume[0] ;
	pRightVol->m_varReal = m_rVolume[1] ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : SetVolume( Real rLeftVol, Real rRightVol )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_SetVolume
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	double	rLeftVol, rRightVol ;
	err = context.GetArgumentAsReal( rLeftVol, lstArg, 1, 1.0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsReal( rRightVol, lstArg, 2, 1.0 ) ;
	if ( err )
		return	err ;
	//
	err = SetVolume( (REAL32) rLeftVol, (REAL32) rRightVol ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Real GetTotalVolume( Integer flag )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_GetTotalVolume
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int		nFlag ;
	err = context.GetArgumentAsInt( nFlag, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSReal( GetTotalVolume( nFlag ) ) ) ;
}

// メンバ関数 : SetTotalVolume( Integer flag, Real rTotalVol )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_SetTotalVolume
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	int		nFlag ;
	double	rTotalVol ;
	err = context.GetArgumentAsInt( nFlag, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsReal( rTotalVol, lstArg, 2, 1.0 ) ;
	if ( err )
		return	err ;
	//
	err = SetTotalVolume( nFlag, (REAL32) rTotalVol ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer IsPlaying()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_IsPlaying
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( IsPlaying() ? -1 : 0 ) ) ;
}

// メンバ関数 : Integer GetPlayingPosition()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_GetPlayingPosition
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSInteger( (long int) GetPlayingPosition() ) ) ;
}

// メンバ関数 : ImageInfo GetInfo() 
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_GetInfo
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	EGLAnimation *	pImage = ESLTypeCast<EGLAnimation>( m_pRsrc ) ;
	if ( pImage != NULL )
	{
		return	Call_GetImageInfo( context, lstArg ) ;
	}
	else
	{
		EWaveSound *	pSound = ESLTypeCast<EWaveSound>( m_pRsrc ) ;
		if ( pSound != NULL )
		{
			return	Call_GetSoundInfo( context, lstArg ) ;
		}
	}
	return	context.PushObject( context.new_CSReference() ) ;
}

// メンバ関数 : ImageInfo GetImageInfo()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_GetImageInfo
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	ECSStructureInterface *	pImageInfo = NULL ;
	EGLAnimation *	pImage = ESLTypeCast<EGLAnimation>( m_pRsrc ) ;
	if ( (pImage != NULL) && (pImage->GetInfo() != NULL) )
	{
		pImageInfo = context.CreateUserStructure( L"ImageInfo" ) ;
		//
		PEGL_IMAGE_INFO	pInfo = *pImage ;
		pImageInfo->SetMemberAsInt
			( L"nFormatType", pInfo->fdwFormatType ) ;
		pImageInfo->SetMemberAsInt
			( L"nImageWidth", pInfo->dwImageWidth ) ;
		pImageInfo->SetMemberAsInt
			( L"nImageHeight", pInfo->dwImageHeight ) ;
		pImageInfo->SetMemberAsInt
			( L"nBitsPerPixel", pInfo->dwBitsPerPixel ) ;
		pImageInfo->SetMemberAsInt
			( L"nFrameCount", pImage->GetTotalFrameCount() ) ;
		pImageInfo->SetMemberAsInt
			( L"xHotSpot", pImage->GetHotSpot().x ) ;
		pImageInfo->SetMemberAsInt
			( L"yHotSpot", pImage->GetHotSpot().y ) ;
		pImageInfo->SetMemberAsInt
			( L"nResourceBytes",
				abs( pInfo->dwSizeOfImage )
					* pImage->GetTotalFrameCount() ) ;
		return	context.PushObject( *pImageInfo ) ;
	}
	return	context.PushObject( context.new_CSReference() ) ;
}

// メンバ関数 : SoundInfo GetSoundInfo()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_GetSoundInfo
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	ECSStructureInterface *	pSoundInfo = NULL ;
	EWaveSound *	pSound = ESLTypeCast<EWaveSound>( m_pRsrc ) ;
	if ( pSound != NULL )
	{
		const WAVEFORMATEX *	pwfx = pSound->GetWaveFormat( ) ;
		if ( pwfx != NULL )
		{
			pSoundInfo = context.CreateUserStructure( L"SoundInfo" ) ;
			//
			unsigned int	nWaveLength =  pSound->GetWaveLength() ;
			unsigned int	nSampleCount = nWaveLength ;
			if ( pwfx->nChannels * pwfx->wBitsPerSample != 0 )
			{
				nSampleCount =
					(int) ((INT64) nWaveLength * 8
						/ (pwfx->nChannels * pwfx->wBitsPerSample)) ;
			}
			//
			pSoundInfo->SetMemberAsInt
				( L"nSamplesPerSec", pwfx->nSamplesPerSec ) ;
			pSoundInfo->SetMemberAsInt
				( L"nChannelCount", pwfx->nChannels ) ;
			pSoundInfo->SetMemberAsInt
				( L"nBitsPerSample", pwfx->wBitsPerSample ) ;
			pSoundInfo->SetMemberAsInt
				( L"nResourceBytes", nWaveLength ) ;
			//
			MIOSoundStream *
				pMIO = ESLTypeCast<MIOSoundStream>( pSound ) ;
			if ( (pMIO != NULL) && (nSampleCount == 0) )
			{
				const ERIFile &	erif = pMIO->GetMIOPlayer().GetERIFile() ;
				if ( erif.m_fdwReadMask & erif.rmSoundInfo )
				{
					nSampleCount = erif.m_MIOInfHdr.dwAllSampleCount ;
				}
			}
			pSoundInfo->SetMemberAsInt
				( L"nSampleCount", nSampleCount ) ;
			//
			if ( pMIO != NULL )
			{
				pSoundInfo->SetMemberAsInt
					( L"nRewoundPosition", pMIO->GetRewoundPosition() ) ;
			}
			else
			{
				pSoundInfo->SetMemberAsInt
					( L"nRewoundPosition", 0 ) ;
			}
			return	context.PushObject( *pSoundInfo ) ;
		}
	}
	return	context.PushObject( context.new_CSReference() ) ;
}

// メンバ関数 : SetVolumeEnvelope( Array bezier, Integer nDurationTime )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_SetVolumeEnvelope
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSArray *	pArray =
		ESLTypeCast<ECSArray>
			( context.GetArgumentObjectAs( lstArg, 1, L"Array" ) ) ;
	if ( pArray == NULL )
	{
		return	ESLErrorMsg( "引数が指定されていません。" ) ;
	}
	int		nDurationTime ;
	err = context.GetArgumentAsInt( nDurationTime, lstArg, 2, 1000 ) ;
	if ( err )
		return	err ;
	//
	EBezierCurves<E3D_VECTOR_2D>	bezier ;
	bezier.SetCount( pArray->m_varArray.GetSize() ) ;
	for ( unsigned int i = 0; i < pArray->m_varArray.GetSize(); i ++ )
	{
		ECSStructureInterface *	pHash =
			ESLTypeCast<ECSStructureInterface>( pArray->m_varArray.GetAt( i ) ) ;
		if ( pHash == NULL )
		{
			return	ESLErrorMsg( "Array 型の引数の要素が不正です。" ) ;
		}
		bezier[i].x = (REAL32) pHash->GetMemberAsReal( L"x", 1.0 ) ;
		bezier[i].y = (REAL32) pHash->GetMemberAsReal( L"y", 1.0 ) ;
	}
	//
	err = SetVolumeEnvelope( bezier, nDurationTime ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : CancelVolumeEnvelope()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_CancelVolumeEnvelope
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	CancelVolumeEnvelope( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer IsPendingEnvelope()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_IsPendingEnvelope
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSInteger( IsPendingEnvelope() ? -1 : 0 ) ) ;
}

// メンバ関数 : Integer GetPixel( Integer x, Integer y )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_GetPixel
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	int	x, y ;
	err = context.GetArgumentAsInt( x, lstArg, 1, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( y, lstArg, 2, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	EGLAnimation *	pImage = GetImage( ) ;
	DWORD	dwPixel = 0 ;
	if ( pImage && pImage->GetInfo() )
	{
		dwPixel = pImage->GetPixel( x, y ).dwPixelCode ;
	}
	//
	return	context.PushObject( context.new_CSInteger( dwPixel ) ) ;
}

// メンバ関数 : Integer GetPixelRect
//		( Reference aPixels as Integer,
//			Integer x, Integer y, Integer width, Integer height )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_GetPixelRect
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 6 ) ;
	if ( err )
		return	err ;
	//
	ECSPointer *	pPixelPtr =
		ESLTypeCast<ECSPointer>
			( context.GetArgumentObjectAs( lstArg, 1, L"Pointer" ) ) ;
	ECSArray *	pPixels = NULL ;
	if ( pPixelPtr == NULL )
	{
		pPixels = ESLTypeCast<ECSArray>
				( context.GetArgumentObjectAs( lstArg, 1, L"Array" ) ) ;
		if ( pPixels == NULL )
		{
			return	ESLErrorMsg
				( "引数に Array 型オブジェクトが指定されていません。" ) ;
		}
	}
	int	x, y, w, h ;
	err = context.GetArgumentAsInt( x, lstArg, 2, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( y, lstArg, 3, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( w, lstArg, 4, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( h, lstArg, 5, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	do
	{
		err = eslErrGeneral ;
		//
		EGLAnimation *	pImage = GetImage( ) ;
		if ( pImage == NULL )
		{
			break ;
		}
		PEGL_IMAGE_INFO	pImageInf = pImage->GetInfo() ;
		if ( pImageInf == NULL )
		{
			break ;
		}
		if ( pPixelPtr != NULL )
		{
			const DWORD	dwBufSize = w * h * sizeof(DWORD) ;
			void *	ptrBuf = pPixelPtr->GetBuffer( 0, dwBufSize, true ) ;
			if ( ptrBuf != NULL )
			{
				EGL_IMAGE_INFO	infDstBuf ;
				eslFillMemory( &infDstBuf, 0, sizeof(EGL_IMAGE_INFO) ) ;
				infDstBuf.dwInfoSize = sizeof(EGL_IMAGE_INFO) ;
				infDstBuf.fdwFormatType = EIF_RGBA_BITMAP ;
				infDstBuf.dwImageWidth = w ;
				infDstBuf.dwImageHeight = h ;
				infDstBuf.dwBitsPerPixel = 32 ;
				infDstBuf.dwBytesPerLine = w * sizeof(DWORD) ;
				infDstBuf.dwSizeOfImage = dwBufSize ;
				infDstBuf.ptrImageArray = ptrBuf ;
				//
				EGL_IMAGE_INFO	infSrc ;
				EGLImageRect	rectClip( x, y, w, h ) ;
				infSrc.dwInfoSize = sizeof(EGL_IMAGE_INFO) ;
				if ( !eglGetClippedImageInfo
					( &infSrc, pImageInf, &rectClip ) )
				{
					err = eglConvertFormat( &infDstBuf, &infSrc ) ;
				}
				//
				pPixelPtr->FlushBuffer( 0, dwBufSize, ptrBuf, true ) ;
			}
		}
		else
		{
			for ( int i = 0; i < h; i ++ )
			{
				for ( int j = 0; j < w; j ++ )
				{
					const int	k = i * w + j ;
					DWORD	dwPixelCode =
						pImage->GetPixel( x + j, y + i ).dwPixelCode ;
					//
					ECSObject *	pObj = pPixels->m_varArray.GetAt( k ) ;
					if ( pObj == NULL )
					{
						pPixels->m_varArray.SetAt
							( k, new ECSInteger( dwPixelCode ) ) ;
					}
					else if ( pObj->m_vtType == csvtInteger )
					{
						((ECSInteger*)pObj)->SetValue( dwPixelCode ) ;
					}
					else
					{
						pPixels->m_varArray.SetAt
							( k, new ECSInteger( dwPixelCode ) ) ;
					}
				}
			}
			err = eslErrSuccess ;
		}
	}
	while ( false ) ;
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : Integer SetPixelRect
//		( Reference aPixels as Integer,
//			Integer x, Integer y, Integer width, Integer height )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_SetPixelRect
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 6 ) ;
	if ( err )
		return	err ;
	//
	ECSPointer *	pPixelPtr =
		ESLTypeCast<ECSPointer>
			( context.GetArgumentObjectAs( lstArg, 1, L"Pointer" ) ) ;
	ECSArray *	pPixels = NULL ;
	if ( pPixelPtr == NULL )
	{
		pPixels = ESLTypeCast<ECSArray>
				( context.GetArgumentObjectAs( lstArg, 1, L"Array" ) ) ;
		if ( pPixels == NULL )
		{
			return	ESLErrorMsg
				( "引数に Array 型オブジェクトが指定されていません。" ) ;
		}
	}
	int	x, y, w, h ;
	err = context.GetArgumentAsInt( x, lstArg, 2, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( y, lstArg, 3, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( w, lstArg, 4, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( h, lstArg, 5, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	do
	{
		err = eslErrGeneral ;
		//
		EGLAnimation *	pImage = GetImage( ) ;
		if ( pImage == NULL )
		{
			break ;
		}
		PEGL_IMAGE_INFO	pImageInf = pImage->GetInfo() ;
		if ( pImageInf == NULL )
		{
			break ;
		}
		if ( pPixelPtr != NULL )
		{
			const DWORD	dwBufSize = w * h * sizeof(DWORD) ;
			void *	ptrBuf = pPixelPtr->GetBuffer( 0, dwBufSize, false ) ;
			if ( ptrBuf != NULL )
			{
				EGL_IMAGE_INFO	infSrcBuf ;
				eslFillMemory( &infSrcBuf, 0, sizeof(EGL_IMAGE_INFO) ) ;
				infSrcBuf.dwInfoSize = sizeof(EGL_IMAGE_INFO) ;
				infSrcBuf.fdwFormatType = EIF_RGBA_BITMAP ;
				infSrcBuf.dwImageWidth = w ;
				infSrcBuf.dwImageHeight = h ;
				infSrcBuf.dwBitsPerPixel = 32 ;
				infSrcBuf.dwBytesPerLine = w * sizeof(DWORD) ;
				infSrcBuf.dwSizeOfImage = dwBufSize ;
				infSrcBuf.ptrImageArray = ptrBuf ;
				//
				EGL_IMAGE_INFO	infDst ;
				EGLImageRect	rectClip( x, y, w, h ) ;
				infDst.dwInfoSize = sizeof(EGL_IMAGE_INFO) ;
				if ( !eglGetClippedImageInfo
					( &infDst, pImageInf, &rectClip ) )
				{
					err = eglConvertFormat( &infDst, &infSrcBuf ) ;
				}
				//
				pPixelPtr->FlushBuffer( 0, dwBufSize, ptrBuf, false ) ;
			}
		}
		else
		{
			for ( int i = 0; i < h; i ++ )
			{
				for ( int j = 0; j < w; j ++ )
				{
					const int	k = i * w + j ;
					ECSObject *	pObj = pPixels->m_varArray.GetAt( k ) ;
					if ( pObj && (pObj->m_vtType == csvtInteger) )
					{
						EGL_PALETTE	rgbPixel ;
						rgbPixel.dwPixelCode =
								(DWORD) ((ECSInteger*)pObj)->GetInt() ;
						pImage->SetPixel( x + j, y + i, rgbPixel ) ;
					}
				}
			}
			err = eslErrSuccess ;
		}
	}
	while ( false ) ;
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : Integer GetWaveData
//		( Pointer aPCMData,
//			Integer samplesStart, Integer samplesCount )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResource::Call_GetWaveData
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 4 ) ;
	if ( err )
		return	err ;
	//
	ECSPointer *	pWavePtr =
		ESLTypeCast<ECSPointer>
			( context.GetArgumentObjectAs( lstArg, 1, L"Pointer" ) ) ;
	ECSArray *	pPixels = NULL ;
	if ( pWavePtr == NULL )
	{
		return	ESLErrorMsg
			( "引数に Pointer 型オブジェクトが指定されていません。" ) ;
	}
	int	nStart, nCount ;
	err = context.GetArgumentAsInt( nStart, lstArg, 2, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( nCount, lstArg, 3, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	MIOSoundStream *	pSound = ESLTypeCast<MIOSoundStream>( m_pRsrc ) ;
	if ( pSound == NULL )
	{
		return	context.PushObject( context.new_CSInteger( 0 ) ) ;
	}
	const WAVEFORMATEX *	pwfx = pSound->GetWaveFormat() ;
	if ( pwfx == NULL )
	{
		return	context.PushObject( context.new_CSInteger( 0 ) ) ;
	}
	unsigned int	nStartBytes = pSound->SampleToBytes( nStart ) ;
	unsigned int	nLengthBytes = pSound->SampleToBytes( nCount ) ;
	if ( nStartBytes >= pSound->GetWaveLength() )
	{
		return	context.PushObject( context.new_CSInteger( 0 ) ) ;
	}
	if ( nStartBytes + nLengthBytes >= pSound->GetWaveLength() )
	{
		nLengthBytes = pSound->GetWaveLength() - nStartBytes ;
	}
	const void *	ptrWaveBuf = pSound->GetWaveBuffer() ;
	if ( ptrWaveBuf == NULL )
	{
		return	context.PushObject( context.new_CSInteger( 0 ) ) ;
	}
	void *	ptrBuf = pWavePtr->GetBuffer( 0, nLengthBytes, true ) ;
	if ( pWavePtr != NULL )
	{
		eslMoveMemory( ptrBuf, ptrWaveBuf, nLengthBytes ) ;
		pWavePtr->FlushBuffer( 0, nLengthBytes, ptrBuf, true ) ;
	}
	else
	{
		nLengthBytes = 0 ;
	}
	return	context.PushObject
		( context.new_CSInteger( pSound->BytesToSample(nLengthBytes) ) ) ;
}

// プラグインインターフェースを取得する
//////////////////////////////////////////////////////////////////////////////
void * ECSResource::GetObjectInterface( const wchar_t * pwszType )
{
	if ( !EWideString::CompareNoCase
			( pwszType, L"ECS_RESOURCE_INTERFACE" ) )
	{
		if ( m_ppir == NULL )
		{
			m_ppir = (PLUGIN_RESOURCE*)
				::eslHeapAllocate( NULL, sizeof(PLUGIN_RESOURCE), 0 ) ;
			m_ppir->pBackLink = this ;
			m_ppir->pfnLoadImageFile = PIC_LoadImageFile ;
			m_ppir->pfnLoadSoundFile = PIC_LoadSoundFile ;
			m_ppir->pfnLoadMidiFile = PIC_LoadMidiFile ;
			m_ppir->pfnAttachSound = PIC_AttachSound ;
			m_ppir->pfnRelease = PIC_Release ;
			m_ppir->pfnPlayFrom = PIC_PlayFrom ;
			m_ppir->pfnStop = PIC_Stop ;
			m_ppir->pfnPause = PIC_Pause ;
			m_ppir->pfnRestart = PIC_Restart ;
			m_ppir->pfnGetVolume = PIC_GetVolume ;
			m_ppir->pfnSetVolume = PIC_SetVolume ;
			m_ppir->pfnIsPlaying = PIC_IsPlaying ;
			m_ppir->pfnSetVolumeEnvelope = PIC_SetVolumeEnvelope ;
			m_ppir->pfnCancelVolumeEnvelope = PIC_CancelVolumeEnvelope ;
			m_ppir->pfnIsPendingEnvelope = PIC_IsPendingEnvelope ;
			m_ppir->pfnGetTotalVolume = PIC_GetTotalVolume ;
			m_ppir->pfnSetTotalVolume = PIC_SetTotalVolume ;
			m_ppir->pfnSetResource = PIC_SetResource ;
			m_ppir->pfnGetResource = PIC_GetResource ;
		}
		return	(ECS_RESOURCE_INTERFACE*) m_ppir ;
	}
	return	ECSObject::GetObjectInterface( pwszType ) ;
}

ESLError __stdcall ECSResource::PIC_LoadImageFile
	( ECS_RESOURCE_INTERFACE * instance,
		const wchar_t * pwszFilePath, ECS_CONTEXT * pContext )
{
	ECSContext *	context = NULL ;
	if ( pContext != NULL )
	{
		context = ECSContext::ContextFromPlugin( pContext ) ;
	}
	return	ResourceFromPlugin(instance)->
				LoadImageFile( pwszFilePath, context ) ;
}

ESLError __stdcall ECSResource::PIC_LoadSoundFile
	( ECS_RESOURCE_INTERFACE * instance,
		const wchar_t * pwszFilePath,
		unsigned int nThreshold, ECS_CONTEXT * pContext )
{
	ECSContext *	context = NULL ;
	if ( pContext != NULL )
	{
		context = ECSContext::ContextFromPlugin( pContext ) ;
	}
	return	ResourceFromPlugin(instance)->
				LoadSoundFile( pwszFilePath, nThreshold, context ) ;
}

ESLError __stdcall ECSResource::PIC_LoadMidiFile
	( ECS_RESOURCE_INTERFACE * instance,
		const wchar_t * pwszFilePath, ECS_CONTEXT * pContext )
{
	ECSContext *	context = NULL ;
	if ( pContext != NULL )
	{
		context = ECSContext::ContextFromPlugin( pContext ) ;
	}
	return	ResourceFromPlugin(instance)->
				LoadMidiFile( pwszFilePath, context ) ;
}

ESLError __stdcall ECSResource::PIC_AttachSound
	( ECS_RESOURCE_INTERFACE * instance, ECS_RESOURCE_INTERFACE * pRsrc )
{
	return	ResourceFromPlugin(instance)->
				AttachSound( ResourceFromPlugin( pRsrc ) ) ;
}

void __stdcall ECSResource::PIC_Release( ECS_RESOURCE_INTERFACE * instance )
{
	ResourceFromPlugin(instance)->Release( ) ;
}

ESLError __stdcall ECSResource::PIC_PlayFrom
	( ECS_RESOURCE_INTERFACE * instance,
		unsigned int nStartPos, unsigned int nEndPos,
		int fRepeat, unsigned int nRewindPos, int fPlayType )
{
	return	ResourceFromPlugin(instance)->PlayFrom
		( nStartPos, nEndPos, (fRepeat != 0), nRewindPos, fPlayType ) ;
}

ESLError __stdcall ECSResource::PIC_Stop
	( ECS_RESOURCE_INTERFACE * instance )
{
	return	ResourceFromPlugin(instance)->Stop( ) ;
}

ESLError __stdcall ECSResource::PIC_Pause
	( ECS_RESOURCE_INTERFACE * instance )
{
	return	ResourceFromPlugin(instance)->Pause( ) ;
}

ESLError __stdcall ECSResource::PIC_Restart
	( ECS_RESOURCE_INTERFACE * instance )
{
	return	ResourceFromPlugin(instance)->Restart( ) ;
}

ESLError __stdcall ECSResource::PIC_GetVolume
	( ECS_RESOURCE_INTERFACE * instance,
		REAL32 & rLeftVol, REAL32 & rRightVol )
{
	return	ResourceFromPlugin(instance)->GetVolume( rLeftVol, rRightVol ) ;
}

ESLError __stdcall ECSResource::PIC_SetVolume
	( ECS_RESOURCE_INTERFACE * instance,
		REAL32 rLeftVol, REAL32 rRightVol )
{
	return	ResourceFromPlugin(instance)->SetVolume( rLeftVol, rRightVol ) ;
}

int __stdcall ECSResource::PIC_IsPlaying
	( ECS_RESOURCE_INTERFACE * instance )
{
	return	ResourceFromPlugin(instance)->IsPlaying( ) ;
}

ESLError __stdcall ECSResource::PIC_SetVolumeEnvelope
	( ECS_RESOURCE_INTERFACE * instance,
		const E3D_VECTOR_2D * bezier, unsigned int nDurationTime )
{
	EBezierCurves<E3D_VECTOR_2D>	bzVol ;
	bzVol.SetCount( 4 ) ;
	for ( int i = 0; i < 4; i ++ )
	{
		bzVol[i] = bezier[i] ;
	}
	return	ResourceFromPlugin(instance)->
				SetVolumeEnvelope( bzVol, nDurationTime ) ;
}

void __stdcall ECSResource::PIC_CancelVolumeEnvelope
	( ECS_RESOURCE_INTERFACE * instance )
{
	ResourceFromPlugin(instance)->CancelVolumeEnvelope( ) ;
}

int __stdcall ECSResource::PIC_IsPendingEnvelope
	( ECS_RESOURCE_INTERFACE * instance )
{
	return	ResourceFromPlugin(instance)->IsPendingEnvelope( ) ;
}

REAL32 __stdcall ECSResource::PIC_GetTotalVolume
	( ECS_RESOURCE_INTERFACE * instance, int fPlayType )
{
	return	ResourceFromPlugin(instance)->GetTotalVolume( fPlayType ) ;
}

ESLError __stdcall ECSResource::PIC_SetTotalVolume
	( ECS_RESOURCE_INTERFACE * instance, int fPlayType, REAL32 rVolume )
{
	return	ResourceFromPlugin(instance)->SetTotalVolume( fPlayType, rVolume ) ;
}

ESLError __stdcall ECSResource::PIC_SetResource
	( ECS_RESOURCE_INTERFACE * instance, ESLObject * pRsrc,
		ECS_RESOURCE_INTERFACE::ResourceOwnFlag rofType,
								const wchar_t * pwszFileName )
{
	return	ResourceFromPlugin(instance)->
		SetResource( pRsrc, (ResourceOwnFlag) rofType, pwszFileName ) ;
}

ESLObject * __stdcall ECSResource::PIC_GetResource
	( ECS_RESOURCE_INTERFACE * instance )
{
	return	ResourceFromPlugin(instance)->GetResource( ) ;
}



//////////////////////////////////////////////////////////////////////////////
// リソース管理スクリプトインターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO2( ECSResourceManager, ECSGlobal, EFormResourceManager )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSResourceManager::ECSResourceManager( void )
{
	m_vtType = csvtObject ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSResourceManager::~ECSResourceManager( void )
{
	Release( ) ;
}

// スキンファイルを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResourceManager::LoadSkinFile
	( const wchar_t * pwszFileName, ECSContext * pContext )
{
	//
	// 現在の内容を削除
	//
	DeleteContents( ) ;
	//
	// ファイルを開く
	//
	ESLFileObject *	pfile = OpenResourceFile( pwszFileName, pContext ) ;
	if ( pfile == NULL )
	{
		return	eslErrGeneral ;
	}
	//
	// スキンを読み込む
	//
	EDescription	dscSkin ;
	ESLError	errResult = ReadSkinFile( *pfile, dscSkin ) ;
	m_wstrFileName = pwszFileName ;
	//
	// 終了
	//
	delete	pfile ;
	return	errResult ;
}

// ページフォームを読み込んでページを作成する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResourceManager::CreateFormPage
	( ECSSprite & siPage, const wchar_t * pwszPageID )
{
	EDescription *	pdscPage = GetPageFormAs( pwszPageID ) ;
	if ( pdscPage == NULL )
		return	eslErrGeneral ;
	//
	siPage.Release( ) ;
	siPage.SetBaseItemPriority( 0 ) ;
	//
	if ( CreateFormedPage( siPage, *pdscPage ) )
		return	eslErrGeneral ;
	if ( ReadFormSection( siPage, *pdscPage ) )
		return	eslErrGeneral ;
	//
	siPage.m_wstrPageID = pwszPageID ;
	siPage.m_refRsrcManager.SetReference( this ) ;
	siPage.m_hashItemStatus.m_varArray.RemoveAll( ) ;
	//
	return	eslErrSuccess ;
}

// リソース追加
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResourceManager::AddResource
	( const wchar_t * pwszID, ESLObject * pRes )
{
	ECSGlobal::AddVariable( pwszID, new ECSResource( pRes ) ) ;
	return	EFormResourceManager::AddResource( pwszID, pRes ) ;
}

// 内容削除
//////////////////////////////////////////////////////////////////////////////
void ECSResourceManager::Release( void )
{
	ECSGlobal::RemoveAllVariable( ) ;
	EFormResourceManager::DeleteContents( ) ;
	m_wstrFileName.FreeString( ) ;
}

// 内容削除
//////////////////////////////////////////////////////////////////////////////
void ECSResourceManager::DeleteContents( void )
{
	ECSGlobal::RemoveAllVariable( ) ;
	EFormResourceManager::DeleteContents( ) ;
	m_wstrFileName.FreeString( ) ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLFileObject * ECSResourceManager::OpenResourceFile
	( const wchar_t * pwszFilePath, ECSContext * pContext )
{
	if ( pContext != NULL )
	{
		return	pContext->OpenFileOnScript( pwszFilePath ) ;
	}
	ERawFile *	pfile = new ERawFile ;
	if ( pfile->Open
		( EString(pwszFilePath),
			ESLFileObject::modeRead | ESLFileObject::shareRead ) )
	{
		delete	pfile ;
		return	NULL ;
	}
	return	pfile ;
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSResourceManager::GetTypeName( void ) const
{
	return	L"ResourceManager" ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSResourceManager::Duplicate( void )
{
	return	new ECSResourceManager ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResourceManager::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	nIndex = m_staFuncName->FindIndex( pwszName ) ;
	if ( nIndex >= 0 )
	{
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg
		( "ResourceManager 型の定義されていない"
				"メンバ関数を呼び出そうとしています。" ) ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResourceManager::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	if ( (nIndex < 0) || (nIndex >= (int) m_staFuncName->GetSize()) )
	{
		return	ESLErrorMsg
			( "ResourceManager 型の定義されていない"
					"メンバ関数を呼び出そうとしました。" ) ;
	}
	return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResourceManager::Save( ESLFileObject & file, ECSContext & context )
{
	DWORD	dwLength = m_wstrFileName.GetLength( ) ;
	file.Write( &dwLength, sizeof(dwLength) ) ;
	if ( dwLength > 0 )
	{
		file.Write( m_wstrFileName.CharPtr(), dwLength * sizeof(wchar_t) ) ;
	}
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResourceManager::Load( ESLFileObject & file, ECSContext & context )
{
	DWORD	dwLength ;
	if ( file.Read( &dwLength, sizeof(dwLength) ) < sizeof(dwLength) )
	{
		return	eslErrGeneral ;
	}
	if ( dwLength > 0 )
	{
		ECSWideString	wstrFileName ;
		file.Read
			( wstrFileName.GetBuffer(dwLength),
						dwLength * sizeof(wchar_t) ) ;
		wstrFileName.ReleaseBuffer( dwLength ) ;
		return	LoadSkinFile( wstrFileName, &context ) ;
	}
	else
	{
		DeleteContents( ) ;
	}
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResourceManager::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	EString	strDump ;
	strDump = "skin file = \"" ;
	strDump += EString(m_wstrFileName) ;
	strDump += "\"" ;
	buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	return	eslErrSuccess ;
}

// メンバ関数ポインタ
//////////////////////////////////////////////////////////////////////////////
ECSStrTagArray *	ECSResourceManager::m_staFuncName = NULL ;
const wchar_t *		ECSResourceManager::m_pwszFuncName[4] =
{
	L"LoadResource", L"DeleteContents", L"CreateFormPage", NULL
} ;
const ECSResourceManager::PFUNC_CALL
	ECSResourceManager::m_pfnCallFunc[3] =
{
	&ECSResourceManager::Call_LoadResource,
	&ECSResourceManager::Call_DeleteContents,
	&ECSResourceManager::Call_CreateFormPage
} ;

// メンバ関数 : Integer LoadResource( String filename )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResourceManager::Call_LoadResource
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSFile *	pfile =
		ESLTypeCast<ECSFile>
			( context.GetArgumentObjectAs( lstArg, 1, L"File" ) ) ;
	if ( (pfile == NULL)
		|| (pfile->GetFileInterface() == NULL) )
	{
		ECSWideString	wstrFileName ;
		err = context.GetArgumentAsStr( wstrFileName, lstArg, 1, NULL ) ;
		if ( err )
		{
			return	err ;
		}
		err = LoadSkinFile( wstrFileName, &context ) ;
	}
	else
	{
		EDescription	dscSkin ;
		err = ReadSkinFile( *(pfile->GetFileInterface()), dscSkin ) ;
	}
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : DeleteContents()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResourceManager::Call_DeleteContents
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	Release( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer CreateFormPage( Reference refSprite, String strPageID )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSResourceManager::Call_CreateFormPage
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSSprite *	pSprite =
		ESLTypeCast<ECSSprite>
			( context.GetArgumentObjectAs( lstArg, 1, L"Sprite" ) ) ;
	if ( pSprite == NULL )
	{
		return	ESLErrorMsg( "Sprite オブジェクトが指定されていません。" ) ;
	}
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 2, NULL ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSInteger( CreateFormPage( *pSprite, wstrID ) ) ) ;
}


//////////////////////////////////////////////////////////////////////////////
// スプライトフィルター
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSToneFilter, ECSObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSToneFilter::ECSToneFilter( void )
{
	m_dwFlags = 0 ;
	SetGeneralTone
		( 0, EGL_TONE_BRIGHTNESS,
			0, EGL_TONE_BRIGHTNESS,
			0, EGL_TONE_BRIGHTNESS, 0, EGL_TONE_BRIGHTNESS ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSToneFilter::~ECSToneFilter( void )
{
}

// フィルタファイルを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError ECSToneFilter::LoadFilterFile
	( const wchar_t * pwszFileName, ECSContext * pContext )
{
	ESLFileObject *	pfile = OpenFilterFile( pwszFileName, pContext ) ;
	if ( pfile == NULL )
	{
		return	eslErrGeneral ;
	}
	EMCFile	emcfile ;
	if ( emcfile.Open( pfile ) )
	{
		delete	pfile ;
		return	eslErrGeneral ;
	}
	m_dwFlags = 0 ;
	while ( !emcfile.DescendRecord() )
	{
		UINT64	nRecID = emcfile.GetRecordID( ) ;
		if ( nRecID == *((UINT64*)"blue    ") )
		{
			emcfile.Read( m_bytBlue, sizeof(m_bytBlue) ) ;
		}
		else if ( nRecID == *((UINT64*)"green   ") )
		{
			emcfile.Read( m_bytGreen, sizeof(m_bytGreen) ) ;
		}
		else if ( nRecID == *((UINT64*)"red     ") )
		{
			emcfile.Read( m_bytRed, sizeof(m_bytRed) ) ;
		}
		else if ( nRecID == *((UINT64*)"alpha   ") )
		{
			emcfile.Read( m_bytAlpha, sizeof(m_bytAlpha) ) ;
		}
		else if ( nRecID == *((UINT64*)"info    ") )
		{
			emcfile.Read( &m_dwFlags, sizeof(m_dwFlags) ) ;
		}
		emcfile.AscendRecord( ) ;
	}
	emcfile.Close( ) ;
	delete	pfile ;
	return	eslErrSuccess ;
}

// トーンテーブルを取得
//////////////////////////////////////////////////////////////////////////////
void ECSToneFilter::GetToneTables
	( void * pRed, void * pGreen,
		void * pBlue, void * pAlpha )
{
	if ( pRed != NULL )
		::eslMoveMemory( pRed, m_bytRed, 0x100 ) ;
	if ( pGreen != NULL )
		::eslMoveMemory( pGreen, m_bytGreen, 0x100 ) ;
	if ( pBlue != NULL )
		::eslMoveMemory( pBlue, m_bytBlue, 0x100 ) ;
	if ( pAlpha != NULL )
		::eslMoveMemory( pAlpha, m_bytAlpha, 0x100 ) ;
}

// 複雑なフィルタを設定
//////////////////////////////////////////////////////////////////////////////
void ECSToneFilter::SetGeneralTone
	( int nRedTone, int nRedFlag,
		int nGreenTone, int nGreenFlag,
		int nBlueTone, int nBlueFlag,
		int nAlphaTone, int nAlphaFlag, DWORD dwFlags )
{
	::eglCalculateToneTable( m_bytRed, nRedTone, nRedFlag ) ;
	::eglCalculateToneTable( m_bytGreen, nGreenTone, nGreenFlag ) ;
	::eglCalculateToneTable( m_bytBlue, nBlueTone, nBlueFlag ) ;
	::eglCalculateToneTable( m_bytAlpha, nAlphaTone, nAlphaFlag ) ;
	//
	m_dwFlags = dwFlags ;
	if ( dwFlags & tffFixZero )
	{
		m_bytRed[0] = 0 ;
		m_bytGreen[0] = 0 ;
		m_bytBlue[0] = 0 ;
		m_bytAlpha[0] = 0 ;
	}
}

// トーンテーブルを設定
//////////////////////////////////////////////////////////////////////////////
void ECSToneFilter::SetToneTables
	( const void * pRed, const void * pGreen,
		const void * pBlue, const void * pAlpha, DWORD dwFlags )
{
	if ( pRed != NULL )
		::eslMoveMemory( m_bytRed, pRed, 0x100 ) ;
	if ( pGreen != NULL )
		::eslMoveMemory( m_bytGreen, pGreen, 0x100 ) ;
	if ( pBlue != NULL )
		::eslMoveMemory( m_bytBlue, pBlue, 0x100 ) ;
	if ( pAlpha != NULL )
		::eslMoveMemory( m_bytAlpha, pAlpha, 0x100 ) ;
	//
	m_dwFlags = dwFlags ;
	if ( dwFlags & tffFixZero )
	{
		m_bytRed[0] = 0 ;
		m_bytGreen[0] = 0 ;
		m_bytBlue[0] = 0 ;
		m_bytAlpha[0] = 0 ;
	}
}

// フィルタ補完
//////////////////////////////////////////////////////////////////////////////
void ECSToneFilter::MorphingFilter
	( const ECSToneFilter & filter1,
		const ECSToneFilter & filter2, unsigned int nDegree )
{
	BYTE	bytSrc[0x100], bytDst[0x100] ;
	m_dwFlags = filter1.m_dwFlags ;
	//
	if ( nDegree >= 0x100 )
		nDegree = 0x100 ;
	::eglCalculateToneTable
		( bytSrc, - (int) nDegree, EGL_TONE_BRIGHTNESS ) ;
	::eglCalculateToneTable
		( bytDst, (int) nDegree - 0x100, EGL_TONE_BRIGHTNESS ) ;
	//
	for ( int i = 0; i < 0x100; i ++ )
	{
		m_bytBlue[i] = bytSrc[filter1.m_bytBlue[i]]
						+ bytDst[filter2.m_bytBlue[i]] ;
		m_bytGreen[i] = bytSrc[filter1.m_bytGreen[i]]
						+ bytDst[filter2.m_bytGreen[i]] ;
		m_bytRed[i] = bytSrc[filter1.m_bytRed[i]]
						+ bytDst[filter2.m_bytRed[i]] ;
		m_bytAlpha[i] = bytSrc[filter1.m_bytAlpha[i]]
						+ bytDst[filter2.m_bytAlpha[i]] ;
	}
}

// 代入演算子
//////////////////////////////////////////////////////////////////////////////
const ECSToneFilter &
	ECSToneFilter::operator = ( const ECSToneFilter & filter )
{
	m_dwFlags = filter.m_dwFlags ;
	::eslMoveMemory( m_bytBlue, filter.m_bytBlue, 0x100 ) ;
	::eslMoveMemory( m_bytGreen, filter.m_bytGreen, 0x100 ) ;
	::eslMoveMemory( m_bytRed, filter.m_bytRed, 0x100 ) ;
	::eslMoveMemory( m_bytAlpha, filter.m_bytAlpha, 0x100 ) ;
	return	*this ;
}

// トーンフィルタ処理
//////////////////////////////////////////////////////////////////////////////
void ECSToneFilter::ApplyToneFilter( PEGL_IMAGE_INFO pImage )
{
SSystem::STimeCounter	timer ;
	ParallelFilterData	pfd[8] ;
	void *	pInstance[8] =
	{
		&pfd[0], &pfd[1], &pfd[2], &pfd[3],
		&pfd[4], &pfd[5], &pfd[6], &pfd[7]
	} ;
	size_t	nThread =
		(size_t) esl_clampi( (int) SSystem::g_cpuLogicalCount, 1, 8 ) ;
	ParallelFilterProc	prllProc( *this, pImage ) ;
	prllProc.Start( pInstance, nThread ) ;
ESLTrace( "filter time %f[ms]\n", timer.GetRealTime() ) ;
}

// ParallelFilterProc 構築関数
ECSToneFilter::ParallelFilterProc::ParallelFilterProc
	( ECSToneFilter& filter, PEGL_IMAGE_INFO pImage )
{
	m_dwFlags = filter.m_dwFlags ;
	m_bytBlue = filter.m_bytBlue ;
	m_bytGreen = filter.m_bytGreen ;
	m_bytRed = filter.m_bytRed ;
	m_bytAlpha = filter.m_bytAlpha ;
	m_pImage = pImage ;
	//
	DWORD	dwLineBytes = pImage->dwImageWidth
							* pImage->dwBitsPerPixel / 8 ;
	m_dwStepLines = 0 ;
	if ( dwLineBytes != 0 )
	{
		m_dwStepLines = 0x10000 / dwLineBytes ;
	}
	if ( m_dwStepLines == 0 )
	{
		m_dwStepLines = 1 ;
	}
	if ( !(m_dwFlags & (tffGrayFilter | tffYUVFilter | tffMaskWithAlpha)) )
	{
		m_dwStepLines = pImage->dwImageHeight ;
	}
	m_dwNextLine = 0 ;
}

// ループ処理／終了判定関数
bool ECSToneFilter::ParallelFilterProc::Continue( void * pInstance )
{
	if ( m_pImage->dwImageHeight <= m_dwNextLine )
	{
		return	false ;
	}
	DWORD	dwBlockHeight = m_pImage->dwImageHeight - m_dwNextLine ;
	if ( dwBlockHeight > m_dwStepLines )
	{
		dwBlockHeight = m_dwStepLines ;
	}
	EGL_IMAGE_RECT	rect ;
	rect.x = 0 ;
	rect.y = (SDWORD) m_dwNextLine ;
	rect.w = (SDWORD) m_pImage->dwImageWidth ;
	rect.h = (SDWORD) dwBlockHeight ;
	//
	ParallelFilterData *	ppfd = (ParallelFilterData*) pInstance ;
	::eglGetClippedImageInfo( &(ppfd->eiiBlock), m_pImage, &rect ) ;
	//
	m_dwNextLine += dwBlockHeight ;
	return	true ;
}

// 並列処理関数
void ECSToneFilter::ParallelFilterProc::RunParallel( void * pInstance )
{
	ParallelFilterData *	ppfd = (ParallelFilterData*) pInstance ;
	if ( m_dwFlags & tffGrayFilter )
	{
		::eglMakeGrayTone( &(ppfd->eiiBlock), &(ppfd->eiiBlock) ) ;
	}
	else if ( m_dwFlags & tffYUVFilter )
	{
		EGLImageRect
			irClip( 0, 0, ppfd->eiiBlock.dwImageWidth,
							ppfd->eiiBlock.dwImageHeight ) ;
		EGL_IMAGE_INFO	eiiYUV ;
		::eglGetClippedImageInfo( &eiiYUV, &(ppfd->eiiBlock), &irClip ) ;
		eiiYUV.fdwFormatType = EIF_YUV_BITMAP ;
		::eglConvertFormat( &eiiYUV, &(ppfd->eiiBlock) ) ;
	}
	::eglApplyToneTable
		( &(ppfd->eiiBlock), &(ppfd->eiiBlock),
			m_bytBlue, m_bytGreen, m_bytRed, m_bytAlpha ) ;
	if ( m_dwFlags & tffYUVFilter )
	{
		EGLImageRect
			irClip( 0, 0, ppfd->eiiBlock.dwImageWidth,
							ppfd->eiiBlock.dwImageHeight ) ;
		EGL_IMAGE_INFO	eiiYUV ;
		::eglGetClippedImageInfo( &eiiYUV, &(ppfd->eiiBlock), &irClip ) ;
		eiiYUV.fdwFormatType = EIF_YUV_BITMAP ;
		::eglConvertFormat( &(ppfd->eiiBlock), &eiiYUV ) ;
	}
	if ( (m_dwFlags & tffMaskWithAlpha)
		&& (ppfd->eiiBlock.fdwFormatType & EIF_WITH_ALPHA) )
	{
		::eglBlendAlphaChannel
			( &(ppfd->eiiBlock), NULL, NULL, EGL_BAC_MULTIPLY ) ;
	}
}

// フィルタファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLFileObject * ECSToneFilter::OpenFilterFile
	( const wchar_t * pwszFileName, ECSContext * pContext )
{
	if ( pContext != NULL )
	{
		return	pContext->OpenFileOnScript( pwszFileName ) ;
	}
	ERawFile *	pfile = new ERawFile ;
	if ( pfile->Open
		( EString(pwszFileName),
			ESLFileObject::modeRead | ESLFileObject::shareRead ) )
	{
		delete	pfile ;
		return	NULL ;
	}
	return	pfile ;
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSToneFilter::GetTypeName( void ) const
{
	return	L"ToneFilter" ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSToneFilter::Duplicate( void )
{
	ECSToneFilter *	pFilter = new ECSToneFilter ;
	pFilter->operator = ( *this ) ;
	return	pFilter ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSToneFilter::Move( ECSContext & context, ECSObject * obj )
{
	ECSToneFilter *	pFilter =
		ESLTypeCast<ECSToneFilter>( ECSObject::GetEntity( obj ) ) ;
	if ( pFilter == NULL )
	{
		return	ESLErrorMsg( "ToneFilter オブジェクトへ変換できません。" ) ;
	}
	operator = ( *pFilter ) ;
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSToneFilter::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	return	ESLErrorMsg( "ToneFilter に単項演算子は定義されていません。" ) ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSToneFilter::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	return	ESLErrorMsg( "ToneFilter に二項演算子は定義されていません。" ) ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSToneFilter::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	return	ESLErrorMsg( "ToneFilter に比較演算子は定義されていません。" ) ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSToneFilter::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	nIndex = m_staFuncName->FindIndex( pwszName ) ;
	if ( nIndex >= 0 )
	{
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg
		( "ToneFilter の定義されていない"
			"メンバ関数を呼び出そうとしています。" ) ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSToneFilter::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	if ( (nIndex < 0) || (nIndex >= (int) m_staFuncName->GetSize()) )
	{
		return	ESLErrorMsg
			( "ToneFilter の定義されていない"
				"メンバ関数を呼び出そうとしました。" ) ;
	}
	return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSToneFilter::Save( ESLFileObject & file, ECSContext & context )
{
	file.Write( &m_dwFlags, sizeof(DWORD) ) ;
	file.Write( m_bytBlue, sizeof(m_bytBlue) ) ;
	file.Write( m_bytGreen, sizeof(m_bytGreen) ) ;
	file.Write( m_bytRed, sizeof(m_bytRed) ) ;
	file.Write( m_bytAlpha, sizeof(m_bytAlpha) ) ;
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSToneFilter::Load( ESLFileObject & file, ECSContext & context )
{
	file.Read( &m_dwFlags, sizeof(DWORD) ) ;
	file.Read( m_bytBlue, sizeof(m_bytBlue) ) ;
	file.Read( m_bytGreen, sizeof(m_bytGreen) ) ;
	file.Read( m_bytRed, sizeof(m_bytRed) ) ;
	file.Read( m_bytAlpha, sizeof(m_bytAlpha) ) ;
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSToneFilter::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	return	eslErrSuccess ;
}

// メンバ関数ポインタ
//////////////////////////////////////////////////////////////////////////////
ECSStrTagArray *	ECSToneFilter::m_staFuncName = NULL ;
const wchar_t *		ECSToneFilter::m_pwszFuncName[4] =
{
	L"LoadFilterFile", L"SetGeneralTone", L"MorphingFilter", NULL
} ;
const ECSToneFilter::PFUNC_CALL	ECSToneFilter::m_pfnCallFunc[3] =
{
	&ECSToneFilter::Call_LoadFilterFile,
	&ECSToneFilter::Call_SetGeneralTone,
	&ECSToneFilter::Call_MorphingFilter
} ;

// メンバ関数 : Integer LoadFilterFile( String filename )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSToneFilter::Call_LoadFilterFile
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrFileName ;
	err = context.GetArgumentAsStr( wstrFileName, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	err = LoadFilterFile( wstrFileName, &context ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 :
//	SetGeneralTone
//		( Integer nRedTone, Integer nRedFlag,
//			Integer nGreenTone, Integer nGreenFlag,
//			Integer nBlueTone, Integer nBlueFlag,
//			Integer nAlphaTone, Integer nAlphaFlag, Integer nFlags := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSToneFilter::Call_SetGeneralTone
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 9, 10 ) ;
	if ( err )
		return	err ;
	//
	int	nTone[9] ;
	for ( int i = 0; i < 9; i ++ )
	{
		err = context.GetArgumentAsInt( nTone[i], lstArg, i + 1, NULL ) ;
		if ( err )
			return	err ;
	}
	//
	SetGeneralTone
		( nTone[0], nTone[1], nTone[2], nTone[3],
			nTone[4], nTone[5], nTone[6], nTone[7], nTone[8] ) ;
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 :
//	MorphingFilter( Reference filter1, Reference filter2, Integer degree )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSToneFilter::Call_MorphingFilter
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 4 ) ;
	if ( err )
		return	err ;
	//
	ECSToneFilter *	pFilter1 =
		ESLTypeCast<ECSToneFilter>
			( context.GetArgumentObjectAs( lstArg, 1, L"ToneFilter" ) ) ;
	ECSToneFilter *	pFilter2 =
		ESLTypeCast<ECSToneFilter>
			( context.GetArgumentObjectAs( lstArg, 2, L"ToneFilter" ) ) ;
	if ( (pFilter1 == NULL) || (pFilter2 == NULL) )
	{
		return	ESLErrorMsg
			( "引数に ToneFilter オブジェクトが渡されていません。" ) ;
	}
	int		nDegree ;
	err = context.GetArgumentAsInt( nDegree, lstArg, 3, 0 ) ;
	if ( err )
		return	err ;
	//
	MorphingFilter( *pFilter1, *pFilter2, nDegree ) ;
	return	context.PushObject( new ECSInteger ) ;
}


//////////////////////////////////////////////////////////////////////////////
// 入力フィルタ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO2( ECSInputFilter, ECSObject, EInputFilter )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSInputFilter::ECSInputFilter( void )
{
	m_dwFilterMode = 0 ;
	m_ppiif = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSInputFilter::~ECSInputFilter( void )
{
	if ( m_ppiif != NULL )
	{
		::eslHeapFree( NULL, m_ppiif, 0 ) ;
	}
}

// フィルタファイルを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::LoadInputFilter
	( const wchar_t * pwszFileName, ECSContext * pContext )
{
	ESLFileObject *	pfile = OpenFilterFile( pwszFileName, pContext ) ;
	if ( pfile == NULL )
	{
		return	eslErrGeneral ;
	}
	//
	EStreamBuffer	buf ;
	DWORD	dwBytes = pfile->GetLength( ) ;
	pfile->Read( buf.PutBuffer(dwBytes), dwBytes ) ;
	buf.Flush( dwBytes ) ;
	delete	pfile ;
	//
	EDescription	dscFilter ;
	dscFilter.ReadDescription( buf, EDescription::dftXML ) ;
	//
	return	EInputFilter::LoadInputFilter( dscFilter ) ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLFileObject * ECSInputFilter::OpenFilterFile
	( const wchar_t * pwszFilePath, ECSContext * pContext )
{
	if ( pContext != NULL )
	{
		return	pContext->OpenFileOnScript( pwszFilePath ) ;
	}
	ERawFile *	pfile = new ERawFile ;
	if ( pfile->Open
		( EString(pwszFilePath),
			ESLFileObject::modeRead | ESLFileObject::shareRead ) )
	{
		delete	pfile ;
		return	NULL ;
	}
	return	pfile ;
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSInputFilter::GetTypeName( void ) const
{
	return	L"InputFilter" ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSInputFilter::Duplicate( void )
{
	ECSInputFilter *	pFilter = new ECSInputFilter ;
	EDescription	dscFilter ;
	SaveInputFilter( dscFilter ) ;
	pFilter->LoadInputFilter( dscFilter ) ;
	return	pFilter ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::Move( ECSContext & context, ECSObject * obj )
{
	ECSInputFilter *	pEntity =
		ESLTypeCast<ECSInputFilter>( ECSObject::GetEntity( obj ) ) ;
	if ( pEntity == NULL )
	{
		return	ESLErrorMsg
			( "InputFilter へ代入するオブジェクトが存在しません。" ) ;
	}
	EDescription	dscFilter ;
	pEntity->SaveInputFilter( dscFilter ) ;
	LoadInputFilter( dscFilter ) ;
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	return	ESLErrorMsg
		( "InputFilter への単項演算子は定義されていません。" ) ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	return	ESLErrorMsg
		( "InputFilter への二項演算子は定義されていません。" ) ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	return	ESLErrorMsg
		( "InputFilter への比較演算子は定義されていません。" ) ;
}

// メンバ変数取得
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSInputFilter::GetVariableAt( int nIndex )
{
	return	NULL ;
}

// メンバ変数設定
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSInputFilter::SetVariableAt( int nIndex, ECSObject * obj )
{
	return	NULL ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	nIndex = m_staFuncName->FindIndex( pwszName ) ;
	if ( nIndex >= 0 )
	{
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg
		( "InputFilter の定義されていない"
				"メンバ関数を呼び出そうとしています。" ) ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	if ( (nIndex < 0) || (nIndex >= (int) m_staFuncName->GetSize()) )
	{
		return	ESLErrorMsg
			( "InputFilter の定義されていない"
					"メンバ関数を呼び出そうとしました。" ) ;
	}
	return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
}

// 全てのメンバ変数にインデックスを振る
//////////////////////////////////////////////////////////////////////////////
void ECSInputFilter::IndexAllMember( void )
{
	m_refWindow.IndexAllMember( ) ;
}

// 全てのメンバ変数の参照を解消する
//////////////////////////////////////////////////////////////////////////////
void ECSInputFilter::CleanupAllReference( ECSContext & context )
{
	m_refWindow.CleanupAllReference( context ) ;
	CloseFilter( ) ;
	//
	ECSObject::CleanupAllReference( context ) ;
}

// 全てのメンバ変数の参照を解決する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::CommitAllReference( ECSContext & context )
{
	ESLError	err ;
	err = m_refWindow.CommitAllReference( context ) ;
	if ( err )
	{
//		return	err ;
	}
	ECSWindow *	pWindow = ESLTypeCast<ECSWindow>( m_refWindow.m_pRef ) ;
	if ( pWindow != NULL )
	{
		EWindowSpriteInterface *	pInterface = pWindow->GetInterface( ) ;
		if ( pInterface != NULL )
		{
			if ( m_dwFilterMode & 0x02 )
			{
				OpenFilter( pInterface ) ;
			}
			else if ( m_dwFilterMode & 0x01 )
			{
				OpenFilter( pInterface->GetWindow() ) ;
			}
		}
	}
	return	eslErrSuccess ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::Save( ESLFileObject & file, ECSContext & context )
{
	ESLError	err ;
	err = m_refWindow.Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	file.Write( &m_dwFilterMode, sizeof(m_dwFilterMode) ) ;
	//
	EDescription	dscFilter ;
	EStreamFileBuffer	fbuf ;
	SaveInputFilter( dscFilter ) ;
	dscFilter.WriteDescription( fbuf, 0, EDescription::dftXML ) ;
	EPtrBuffer	ptrbuf = fbuf.GetBuffer( ) ;
	DWORD	dwLength = ptrbuf.GetLength( ) ;
	file.Write( &dwLength, sizeof(dwLength) ) ;
	file.Write( ptrbuf, dwLength ) ;
	file.Write( &m_nInputLimit, sizeof(int) ) ;
	FlushInputQueue( m_nInputLimit ) ;
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::Load( ESLFileObject & file, ECSContext & context )
{
	ESLError	err ;
	err = m_refWindow.Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	file.Read( &m_dwFilterMode, sizeof(m_dwFilterMode) ) ;
	//
	DWORD	dwLength ;
	if ( file.Read( &dwLength, sizeof(dwLength) ) < sizeof(dwLength) )
	{
		return	eslErrGeneral ;
	}
	EStreamBuffer	buf ;
	EDescription	dscFilter ;
	file.Read( buf.PutBuffer(dwLength), dwLength ) ;
	buf.Flush( dwLength ) ;
	dscFilter.ReadDescription( buf, EDescription::dftXML ) ;
	LoadInputFilter( dscFilter ) ;
	//
	unsigned int	nInputLimit = 0 ;
	if ( file.Read( &nInputLimit, sizeof(int) ) < sizeof(int) )
	{
		return	eslErrGeneral ;
	}
	FlushInputQueue( nInputLimit ) ;
	//
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	return	eslErrSuccess ;
}

// メンバ関数ポインタ
//////////////////////////////////////////////////////////////////////////////
ECSStrTagArray *	ECSInputFilter::m_staFuncName = NULL ;
const wchar_t *		ECSInputFilter::m_pwszFuncName[19] =
{
	L"LoadInputFilter", L"DeleteInputFilter",
	L"OpenFilter", L"CloseFilter",
	L"GetInputEvent", L"FlushInputQueue",
	L"GetCapturedJoyStick", L"GetStickPosition",
	L"IsJoyButtonPushing", L"GetJoyButtonPushed",
	L"FlushJoyButtonPushed", L"ResetJoyButtonPushing",
	L"GetCursorPos", L"MoveCursorPos",
	L"AddFilter", L"RemoveFilter", L"GetFilter", L"DispatchEvent",
	NULL
} ;
const ECSInputFilter::PFUNC_CALL	ECSInputFilter::m_pfnCallFunc[18] =
{
	&ECSInputFilter::Call_LoadInputFilter,
	&ECSInputFilter::Call_DeleteInputFilter,
	&ECSInputFilter::Call_OpenFilter,
	&ECSInputFilter::Call_CloseFilter,
	&ECSInputFilter::Call_GetInputEvent,
	&ECSInputFilter::Call_FlushInputQueue,
	&ECSInputFilter::Call_GetCapturedJoyStick,
	&ECSInputFilter::Call_GetStickPosition,
	&ECSInputFilter::Call_IsJoyButtonPushing,
	&ECSInputFilter::Call_GetJoyButtonPushed,
	&ECSInputFilter::Call_FlushJoyButtonPushed,
	&ECSInputFilter::Call_ResetJoyButtonPushing,
	&ECSInputFilter::Call_GetCursorPos,
	&ECSInputFilter::Call_MoveCursorPos,
	&ECSInputFilter::Call_AddFilter,
	&ECSInputFilter::Call_RemoveFilter,
	&ECSInputFilter::Call_GetFilter,
	&ECSInputFilter::Call_DispatchEvent,
} ;

// メンバ関数 : Integer LoadInputFilter( String filename )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::Call_LoadInputFilter
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrFileName ;
	err = context.GetArgumentAsStr( wstrFileName, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSInteger( LoadInputFilter( wstrFileName, &context ) ) ) ;
}

// メンバ関数 : DeleteInputFilter()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::Call_DeleteInputFilter
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	DeleteInputFilter( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : OpenFilter( Integer nType, Reference window )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::Call_OpenFilter
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSWindow *	pWindow =
		ESLTypeCast<ECSWindow>
			( context.GetArgumentObjectAs( lstArg, 2, L"Window" ) ) ;
	if ( pWindow == NULL )
	{
		if ( context.m_pcsxi != NULL )
		{
			pWindow = ESLTypeCast<ECSWindow>
				( context.m_pcsxi->GetGlobalObject( L"screen" ) ) ;
		}
		if ( pWindow == NULL )
		{
			return	ESLErrorMsg( "Window オブジェクトが見つかりません。" ) ;
		}
	}
	int		nOpenType ;
	err = context.GetArgumentAsInt( nOpenType, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	m_dwFilterMode = (nOpenType == 0) ? 1 : 2 ;
	m_refWindow.SetReference( pWindow, &context ) ;
	//
	EWindowSpriteInterface *	pInterface = pWindow->GetInterface( ) ;
	if ( pInterface != NULL )
	{
		if ( nOpenType == 0 )
		{
			pInterface->SetInputFilter( NULL ) ;
			OpenFilter( pInterface->GetWindow() ) ;
		}
		else
		{
			OpenFilter( pInterface ) ;
		}
	}
	else
	{
		pWindow->m_pInputFilter = this ;
	}
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : CloseFilter()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::Call_CloseFilter
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	m_refWindow.SetReference( NULL, &context ) ;
	CloseFilter( ) ;
	m_dwFilterMode = 0 ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer GetInputEvent( Reference refEvent, Integer nTimeout )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::Call_GetInputEvent
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSStructure *	pHash =
		ESLTypeCast<ECSStructure>( ECSObject::GetEntity( lstArg.GetAt(1) ) ) ;
	if ( pHash == NULL )
	{
		return	ESLErrorMsg( "引数に構造体への参照が指定されていません。" ) ;
	}
	int		nTimeout ;
	err = context.GetArgumentAsInt( nTimeout, lstArg, 2, INFINITE ) ;
	if ( err )
		return	err ;
	//
	INPUT_EVENT	ie ;
	if ( (unsigned int) nTimeout >= 1000 )
	{
		DWORD	dwBeginTime = ::timeGetTime( ) ;
		while ( context.GetStatus() == context.xsExecution )
		{
			err = GetInputEvent( ie, 10, false ) ;
			if ( err == eslErrSuccess )
			{
				break ;
			}
			if ( ::timeGetTime() - dwBeginTime >= (DWORD) nTimeout )
			{
				break ;
			}
		}
	}
	else
	{
		err = GetInputEvent( ie, nTimeout, false ) ;
	}
	if ( !err )
	{
		pHash->SetMemberAsInt( L"idType", ie.idType ) ;
		pHash->SetMemberAsInt( L"iDevNum", ie.iDevNum ) ;
		pHash->SetMemberAsInt( L"iKeyNum", ie.iKeyNum ) ;
		pHash->SetMemberAsStr( L"strCommand", ie.wstrCommand ) ;
	}
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : FlushInputQueue( Integer nLimit )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::Call_FlushInputQueue
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int		nLimit ;
	err = context.GetArgumentAsInt( nLimit, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	FlushInputQueue( nLimit ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ変数：Integer GetCapturedJoyStick()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::Call_GetCapturedJoyStick
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( GetCapturedJoyStick() ) ) ;
}

// メンバ関数 :
//	Integer GetStickPosition( Reference refPos, Integer iDevNum := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::Call_GetStickPosition
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSStructureInterface *	pPos =
		ESLTypeCast<ECSStructureInterface>( ECSObject::GetEntity( lstArg.GetAt(1) ) ) ;
	if ( pPos == NULL )
	{
		return	ESLErrorMsg( "引数に構造体への参照が指定されていません。" ) ;
	}
	int		iDevNum ;
	err = context.GetArgumentAsInt( iDevNum, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	//
	E3D_VECTOR	vPos ;
	err = GetStickPosition( vPos, iDevNum ) ;
	if ( !err )
	{
		pPos->SetMemberAsReal( L"x", vPos.x ) ;
		pPos->SetMemberAsReal( L"y", vPos.y ) ;
		pPos->SetMemberAsReal( L"z", vPos.z ) ;
	}
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 :
//	Integer IsJoyButtonPushing( Integer iKeyNum, Integer iDevNum := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::Call_IsJoyButtonPushing
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	int		iKeyNum, iDevNum ;
	err = context.GetArgumentAsInt( iKeyNum, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( iDevNum, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	//
	bool	fPushing = IsJoyButtonPushing( iKeyNum, iDevNum ) ;
	return	context.PushObject( new ECSInteger( fPushing ? -1 : 0 ) ) ;
}

// メンバ関数 :
//	Integer GetJoyButtonPushed( Integer iKeyNum, Integer iDevNum := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::Call_GetJoyButtonPushed
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	int		iKeyNum, iDevNum ;
	err = context.GetArgumentAsInt( iKeyNum, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( iDevNum, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	//
	int		nPushed = GetJoyButtonPushed( iKeyNum, iDevNum ) ;
	return	context.PushObject( new ECSInteger( nPushed ) ) ;
}

// メンバ関数 : Integer FlushJoyButtonPushed
//		( Integer iDevNum := 0, Integer iKeyNum := -1 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::Call_FlushJoyButtonPushed
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 3 ) ;
	if ( err )
		return	err ;
	//
	int		iDevNum, iKeyNum ;
	err = context.GetArgumentAsInt( iDevNum, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( iKeyNum, lstArg, 2, -1 ) ;
	if ( err )
		return	err ;
	//
	err = FlushJoyButtonPushed( iDevNum, iKeyNum ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer ResetJoyButtonPushing
//		( Integer iDevNum := 0, Integer iKeyNum := -1 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::Call_ResetJoyButtonPushing
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 3 ) ;
	if ( err )
		return	err ;
	//
	int		iDevNum, iKeyNum ;
	err = context.GetArgumentAsInt( iDevNum, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( iKeyNum, lstArg, 2, -1 ) ;
	if ( err )
		return	err ;
	//
	err = ResetJoyButtonPushing( iDevNum, iKeyNum ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Point GetCursorPos()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::Call_GetCursorPos
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	POINT	ptCursor ;
	::GetCursorPos( &ptCursor ) ;
	EWindowSpriteInterface *	pItf = GetWindowInterface( ) ;
	if ( pItf != NULL )
	{
		EGLPoint	ptCursorPos( ptCursor.x, ptCursor.y ) ;
		pItf->ScreenToClient( ptCursorPos ) ;
		ptCursor.x = ptCursorPos.x ;
		ptCursor.y = ptCursorPos.y ;
	}
	else
	{
		EWindow *	pWnd = GetWindow( ) ;
		if ( pWnd != NULL )
		{
			pWnd->ScreenToClient( &ptCursor ) ;
		}
	}
	ECSStructureInterface *	pHash = context.CreateUserStructure( L"Point" ) ;
	pHash->SetMemberAsInt( L"x", ptCursor.x ) ;
	pHash->SetMemberAsInt( L"y", ptCursor.y ) ;
	//
	return	context.PushObject( *pHash ) ;
}

// メンバ関数 : MoveCursorPos( Integer x, Integer y )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::Call_MoveCursorPos
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	int		x, y ;
	err = context.GetArgumentAsInt( x, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( y, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	//
	EWindowSpriteInterface *	pItf = GetWindowInterface( ) ;
	if ( pItf != NULL )
	{
		EGLPoint	ptCursorPos( x, y ) ;
		pItf->ClientToScreen( ptCursorPos ) ;
		::SetCursorPos( ptCursorPos.x, ptCursorPos.y ) ;
	}
	else
	{
		POINT	ptCursor ;
		ptCursor.x = x ;
		ptCursor.y = y ;
		//
		EWindow *	pWnd = GetWindow( ) ;
		if ( pWnd != NULL )
		{
			pWnd->ClientToScreen( &ptCursor ) ;
		}
		::SetCursorPos( ptCursor.x, ptCursor.y ) ;
	}
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer AddFilter( Reference evInput, Reference evOutput )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::Call_AddFilter
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSStructure *	pHashInput =
		ESLTypeCast<ECSStructure>( ECSObject::GetEntity( lstArg.GetAt(1) ) ) ;
	if ( pHashInput == NULL )
	{
		return	ESLErrorMsg( "引数に構造体への参照が指定されていません。" ) ;
	}
	ECSStructure *	pHashOutput =
		ESLTypeCast<ECSStructure>( ECSObject::GetEntity( lstArg.GetAt(2) ) ) ;
	if ( pHashOutput == NULL )
	{
		return	ESLErrorMsg( "引数に構造体への参照が指定されていません。" ) ;
	}
	//
	INPUT_EVENT	evInput, evOutput ;
	evInput.idType =
		(InputDevice) pHashInput->GetMemberAsInt( L"idType", idKeyboard ) ;
	evInput.iDevNum = pHashInput->GetMemberAsInt( L"iDevNum", 0 ) ;
	evInput.iKeyNum = pHashInput->GetMemberAsInt( L"iKeyNum", 0 ) ;
	evInput.wstrCommand = pHashInput->GetMemberAsStr( L"strCommand", NULL ) ;
	evInput.nPriority =
			(evInput.idType == idSignalCommand)
				? EWndSpriteCmd::priorityHigh
					: EWndSpriteCmd::priorityNormal ;
	//
	evOutput.idType =
		(InputDevice) pHashOutput->GetMemberAsInt( L"idType", idKeyboard ) ;
	evOutput.iDevNum = pHashOutput->GetMemberAsInt( L"iDevNum", 0 ) ;
	evOutput.iKeyNum = pHashOutput->GetMemberAsInt( L"iKeyNum", 0 ) ;
	evOutput.wstrCommand = pHashOutput->GetMemberAsStr( L"strCommand", NULL ) ;
	evOutput.nPriority =
			(evOutput.idType == idSignalCommand)
				? EWndSpriteCmd::priorityHigh
					: EWndSpriteCmd::priorityNormal ;
	//
	err = AddFilter( evInput, evOutput ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer RemoveFilter( Reference evInput )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::Call_RemoveFilter
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSStructure *	pHashInput =
		ESLTypeCast<ECSStructure>( ECSObject::GetEntity( lstArg.GetAt(1) ) ) ;
	if ( pHashInput == NULL )
	{
		return	ESLErrorMsg( "引数に構造体への参照が指定されていません。" ) ;
	}
	//
	INPUT_EVENT	evInput ;
	evInput.idType =
		(InputDevice) pHashInput->GetMemberAsInt( L"idType", idKeyboard ) ;
	evInput.iDevNum = pHashInput->GetMemberAsInt( L"iDevNum", 0 ) ;
	evInput.iKeyNum = pHashInput->GetMemberAsInt( L"iKeyNum", 0 ) ;
	evInput.wstrCommand = pHashInput->GetMemberAsStr( L"strCommand", NULL ) ;
	//
	err = RemoveFilter( evInput ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : InputEvent GetFilter( Reference evInput )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::Call_GetFilter
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSStructure *	pHashInput =
		ESLTypeCast<ECSStructure>( ECSObject::GetEntity( lstArg.GetAt(1) ) ) ;
	if ( pHashInput == NULL )
	{
		return	ESLErrorMsg( "引数に構造体への参照が指定されていません。" ) ;
	}
	//
	INPUT_EVENT	evInput ;
	evInput.idType =
		(InputDevice) pHashInput->GetMemberAsInt( L"idType", idKeyboard ) ;
	evInput.iDevNum = pHashInput->GetMemberAsInt( L"iDevNum", 0 ) ;
	evInput.iKeyNum = pHashInput->GetMemberAsInt( L"iKeyNum", 0 ) ;
	evInput.wstrCommand = pHashInput->GetMemberAsStr( L"strCommand", NULL ) ;
	//
	INPUT_EVENT *	pevOutput = GetFilter( evInput ) ;
	if ( pevOutput == NULL )
	{
		pevOutput = &evInput ;
	}
	ECSStructure *	pStruct = context.CreateUserStructureObject( L"InputEvent" ) ;
	pStruct->SetMemberAsInt( L"idType", pevOutput->idType ) ;
	pStruct->SetMemberAsInt( L"iDevNum", pevOutput->iDevNum ) ;
	pStruct->SetMemberAsInt( L"iKeyNum", pevOutput->iKeyNum ) ;
	pStruct->SetMemberAsStr( L"strCommand", pevOutput->wstrCommand ) ;
	//
	return	context.PushObject( pStruct ) ;
}

// メンバ関数 : Boolean DispatchEvent( Reference evInput, Boolean fPushed )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInputFilter::Call_DispatchEvent
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSStructure *	pHashInput =
		ESLTypeCast<ECSStructure>( ECSObject::GetEntity( lstArg.GetAt(1) ) ) ;
	if ( pHashInput == NULL )
	{
		return	ESLErrorMsg( "引数に構造体への参照が指定されていません。" ) ;
	}
	//
	int	fPushed ;
	err = context.GetArgumentAsInt( fPushed, lstArg, 2, 1 ) ;
	if ( err )
		return	err ;
	//
	INPUT_EVENT	evInput ;
	evInput.idType =
		(InputDevice) pHashInput->GetMemberAsInt( L"idType", idKeyboard ) ;
	evInput.iDevNum = pHashInput->GetMemberAsInt( L"iDevNum", 0 ) ;
	evInput.iKeyNum = pHashInput->GetMemberAsInt( L"iKeyNum", 0 ) ;
	evInput.wstrCommand = pHashInput->GetMemberAsStr( L"strCommand", NULL ) ;
	//
	bool	result = ProcessEvent( evInput, (fPushed != 0) ) ;
	//
	return	context.PushObject( new ECSInteger( result ? -1 : 0 ) ) ;
}

// プラグインインターフェースを取得する
//////////////////////////////////////////////////////////////////////////////
void * ECSInputFilter::GetObjectInterface( const wchar_t * pwszType )
{
	if ( !EWideString::CompareNoCase
			( pwszType, L"ECS_INPUT_FILTER_INTERFACE" ) )
	{
		if ( m_ppiif == NULL )
		{
			m_ppiif = (PLUGIN_INPUT_FILTER*)
				::eslHeapAllocate( NULL, sizeof(PLUGIN_INPUT_FILTER), 0 ) ;
			m_ppiif->pBackLink = this ;
			m_ppiif->pfnLoadInputFilter = PIC_LoadInputFilter ;
			m_ppiif->pfnDeleteInputFilter = PIC_DeleteInputFilter ;
			m_ppiif->pfnOpenFilter = PIC_OpenFilter ;
			m_ppiif->pfnCloseFilter = PIC_CloseFilter ;
			m_ppiif->pfnGetInputEvent = PIC_GetInputEvent ;
			m_ppiif->pfnFlushInputQueue = PIC_FlushInputQueue ;
			m_ppiif->pfnGetCapturedJoyStick = PIC_GetCapturedJoyStick ;
			m_ppiif->pfnGetStickPosition = PIC_GetStickPosition ;
			m_ppiif->pfnIsJoyButtonPushing = PIC_IsJoyButtonPushing ;
			m_ppiif->pfnGetJoyButtonPushed = PIC_GetJoyButtonPushed ;
			m_ppiif->pfnFlushJoyButtonPushed = PIC_FlushJoyButtonPushed ;
			m_ppiif->pfnGetCursorPos = PIC_GetCursorPos ;
			m_ppiif->pfnMoveCursorPos = PIC_MoveCursorPos ;
		}
		return	(ECS_INPUT_FILTER_INTERFACE*) m_ppiif ;
	}
	return	ECSObject::GetObjectInterface( pwszType ) ;
}

ESLError __stdcall ECSInputFilter::PIC_LoadInputFilter
	( ECS_INPUT_FILTER_INTERFACE * instance,
		const wchar_t * pwszFileName, ECS_CONTEXT * pContext )
{
	ECSContext *	context = NULL ;
	if ( pContext != NULL )
	{
		context = ECSContext::ContextFromPlugin( pContext ) ;
	}
	return	InputFilterFromPlugin(instance)->
				LoadInputFilter( pwszFileName, context ) ;
}

void __stdcall ECSInputFilter::PIC_DeleteInputFilter
	( ECS_INPUT_FILTER_INTERFACE * instance )
{
	InputFilterFromPlugin(instance)->DeleteInputFilter( ) ;
}

ESLError __stdcall ECSInputFilter::PIC_OpenFilter
	( ECS_INPUT_FILTER_INTERFACE * instance,
					int nType, ECS_OBJECT * pWnd )
{
	ECSWindow *	pWindow = ESLTypeCast<ECSWindow>( ObjectFromPlugin(pWnd) ) ;
	if ( pWindow == NULL )
	{
		return	eslErrGeneral ;
	}
	ECSInputFilter *	pInputFilter = InputFilterFromPlugin(instance) ;
	EWindowSpriteInterface *	pInterface = pWindow->GetInterface( ) ;
	pInputFilter->m_dwFilterMode = (nType == 0) ? 1 : 2 ;
	if ( pInterface != NULL )
	{
		if ( nType == 0 )
		{
			pInterface->SetInputFilter( NULL ) ;
			return	pInputFilter->OpenFilter( pInterface->GetWindow() ) ;
		}
		else
		{
			return	pInputFilter->OpenFilter( pInterface ) ;
		}
	}
	else
	{
		pWindow->m_pInputFilter = pInputFilter ;
	}
	return	eslErrSuccess ;
}

ESLError __stdcall ECSInputFilter::PIC_CloseFilter
	( ECS_INPUT_FILTER_INTERFACE * instance )
{
	ECSInputFilter *	pInputFilter = InputFilterFromPlugin(instance) ;
	pInputFilter->m_dwFilterMode = 0 ;
	return	pInputFilter->CloseFilter( ) ;
}

ESLError __stdcall ECSInputFilter::PIC_GetInputEvent
	( ECS_INPUT_FILTER_INTERFACE * instance,
		ECS_INPUT_FILTER_INTERFACE::INPUT_EVENT & ieEvent, DWORD dwTimeout )
{
	INPUT_EVENT	ie ;
	ESLError	err =
		InputFilterFromPlugin(instance)->GetInputEvent( ie, dwTimeout ) ;
	if ( !err )
	{
		ieEvent.idType = (ECS_INPUT_FILTER_INTERFACE::InputDevice) ie.idType ;
		ieEvent.iDevNum = ie.iDevNum ;
		ieEvent.iKeyNum = ie.iKeyNum ;
	}
	return	err ;
}

ESLError __stdcall ECSInputFilter::PIC_FlushInputQueue
	( ECS_INPUT_FILTER_INTERFACE * instance, int nLimit )
{
	return	InputFilterFromPlugin(instance)->FlushInputQueue( nLimit ) ;
}

DWORD __stdcall ECSInputFilter::PIC_GetCapturedJoyStick
	( ECS_INPUT_FILTER_INTERFACE * instance )
{
	return	InputFilterFromPlugin(instance)->GetCapturedJoyStick( ) ;
}

ESLError __stdcall ECSInputFilter::PIC_GetStickPosition
	( ECS_INPUT_FILTER_INTERFACE * instance,
				E3D_VECTOR & vPos, int iDevNum )
{
	return	InputFilterFromPlugin(instance)->
				GetStickPosition( vPos, iDevNum ) ;
}

int __stdcall ECSInputFilter::PIC_IsJoyButtonPushing
	( ECS_INPUT_FILTER_INTERFACE * instance, int iKeyNum, int iDevNum )
{
	return	InputFilterFromPlugin(instance)->
				IsJoyButtonPushing( iKeyNum, iDevNum ) ;
}

int __stdcall ECSInputFilter::PIC_GetJoyButtonPushed
	( ECS_INPUT_FILTER_INTERFACE * instance, int iKeyNum, int iDevNum )
{
	return	InputFilterFromPlugin(instance)->
				GetJoyButtonPushed( iKeyNum, iDevNum ) ;
}

ESLError __stdcall ECSInputFilter::PIC_FlushJoyButtonPushed
	( ECS_INPUT_FILTER_INTERFACE * instance, int iDevNum, int iKeyNum )
{
	return	InputFilterFromPlugin(instance)->
					FlushJoyButtonPushed( iDevNum, iKeyNum ) ;
}

void __stdcall ECSInputFilter::PIC_GetCursorPos
	( ECS_INPUT_FILTER_INTERFACE * instance, EGL_POINT & ptCursor )
{
	POINT	ptCur ;
	::GetCursorPos( &ptCur ) ;
	EWindow *	pWnd = InputFilterFromPlugin(instance)->GetWindow( ) ;
	if ( pWnd != NULL )
	{
		pWnd->ScreenToClient( &ptCur ) ;
	}
	ptCursor.x = ptCur.x ;
	ptCursor.y = ptCur.y ;
}

void __stdcall ECSInputFilter::PIC_MoveCursorPos
	( ECS_INPUT_FILTER_INTERFACE * instance,
					long int xPos, long int yPos )
{
	POINT	ptCursor ;
	ptCursor.x = xPos ;
	ptCursor.y = yPos ;
	//
	EWindow *	pWnd = InputFilterFromPlugin(instance)->GetWindow( ) ;
	if ( pWnd != NULL )
	{
		pWnd->ClientToScreen( &ptCursor ) ;
	}
	::SetCursorPos( ptCursor.x, ptCursor.y ) ;
}


