
/*****************************************************************************
				Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2003-2017 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// フォーム読み込みクラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EFormResourceManager, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EFormResourceManager::EFormResourceManager( void )
{
	m_nPageNest = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EFormResourceManager::~EFormResourceManager( void )
{
}

// 内容削除
//////////////////////////////////////////////////////////////////////////////
void EFormResourceManager::DeleteContents( void )
{
	m_wstaResource.RemoveAll( ) ;
	m_wstaStyle.RemoveAll( ) ;
	m_wstaPage.RemoveAll( ) ;
	m_nPageNest = 0 ;
}

// エラー出力
//////////////////////////////////////////////////////////////////////////////
void EFormResourceManager::OutputError( const char * pszErrMsg )
{
	ESLTrace( pszErrMsg ) ;
	ESLTrace( "\n" ) ;
}

// スキンファイルを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError EFormResourceManager::ReadSkinFile
	( ESLFileObject & file, EDescription & dscSkin )
{
	//
	// ファイルを開く
	//
	ERISAArchive	arcfile ;
	if ( arcfile.Open( &file ) )
	{
		ESLTrace( "NOA 書庫ファイルを開けませんでした。\n" ) ;
		return	ESLErrorMsg( "NOA 書庫ファイルを開けませんでした" ) ;
	}
	//
	// system.inf を読み込む
	//
	DWORD			dwBytes ;
	EStreamBuffer	buf ;
	if ( arcfile.DescendFile( "system.inf" ) )
	{
		ESLTrace( "system.inf を開けませんでした。\n" ) ;
		return	ESLErrorMsg( "system.inf を開けませんでした" ) ;
	}
	dwBytes = arcfile.GetLength( ) ;
	arcfile.Read( buf.PutBuffer(dwBytes), dwBytes ) ;
	buf.Flush( dwBytes ) ;
	arcfile.AscendFile( ) ;
	dscSkin.ReadDescription( buf ) ;
	//
	// <skin> タグを取得する
	//
	EDescription *	pdscSkin =
		dscSkin.GetContentTagAs( 0, L"skin" ) ;
	if ( pdscSkin == NULL )
	{
		ESLTrace( "<skin> タグが見つかりません。\n" ) ;
		return	ESLErrorMsg( "<skin> タグが見つかりません" ) ;
	}
	//
	// リソース読み込み
	//
	EDescription *	pdscRsrc =
		pdscSkin->GetContentTagAs( 0, L"resource" ) ;
	if ( pdscRsrc != NULL )
	{
		if ( ReadResourceSection( *pdscRsrc, arcfile ) )
		{
			ESLTrace( "リソースの読み込みに失敗しました。\n" ) ;
			return	ESLErrorMsg( "リソースの読み込みに失敗しました" ) ;
		}
	}
	//
	// スタイル読み込み
	//
	EDescription *	pdscStyle =
		pdscSkin->GetContentTagAs( 0, L"declare_style" ) ;
	if ( pdscStyle != NULL )
	{
		if ( ReadStyleSection( *pdscStyle ) )
		{
			ESLTrace( "スタイルの読み込みに失敗しました。\n" ) ;
			return	ESLErrorMsg( "スタイルの読み込みに失敗しました" ) ;
		}
	}
	//
	// ページリスト定義
	//
	RemovePageList( ) ;
	AddPageList( *pdscSkin ) ;
	//
	arcfile.Close( ) ;
	return	eslErrSuccess ;
}

// リソースセクション読み込み
//////////////////////////////////////////////////////////////////////////////
ESLError EFormResourceManager::ReadResourceSection
	( EDescription & descRes, ERISAArchive & file )
{
	int	i, nCount ;
	nCount = descRes.GetContentTagCount( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		EDescription *	pTag = descRes.GetContentTagAt( i ) ;
		if ( pTag != NULL )
		{
			ReadResourceTag( *pTag, file ) ;
		}
	}
	return	eslErrSuccess ;
}

// リソース読み込み
//////////////////////////////////////////////////////////////////////////////
ESLError EFormResourceManager::ReadResourceTag
	( EDescription & descRes, ERISAArchive & file )
{
	if ( descRes.Tag() == L"image" )
	{
		//
		// 画像リソース
		//
		ESLError	errResult = eslErrSuccess ;
		EWideString	wstrID = descRes.GetAttrString( L"id", L"" ) ;
		EString		strFile = descRes.GetAttrString( L"src", L"" ) ;
		EGLAnimation *	pAnime = new EGLAnimation ;
		//
		if ( file.OpenFile( strFile ) )
		{
			OutputError
				( "image タグ : "
					+ strFile + " を開けませんでした。" ) ;
			errResult = eslErrGeneral ;
		}
		else
		{
			if ( pAnime->ReadImageFile( file ) )
			{
				OutputError
					( "image タグ : "
						+ strFile + " を読み込めませんでした。" ) ;
				errResult = eslErrGeneral ;
			}
			file.AscendFile( ) ;
		}
		//
		if ( errResult )
		{
			pAnime->CreateImage( EIF_RGBA_BITMAP, 16, 16, 32 ) ;
		}
		//
		AddResource( wstrID, pAnime ) ;
		return	errResult ;
	}
	else if ( descRes.Tag() == L"sound" )
	{
		//
		// 音声リソース
		//
		EWideString	wstrID = descRes.GetAttrString( L"id", L"" ) ;
		EString		strFile = descRes.GetAttrString( L"src", L"" ) ;
		EWideString	wstrWay = descRes.GetAttrString( L"way", L"auto" ) ;
		if ( file.OpenFile( strFile ) )
		{
			OutputError
				( "sound タグ : "
					+ strFile + " を開けませんでした。" ) ;
			return	eslErrGeneral ;
		}
		//
		MIOSoundStream::PlayMode	pmMode ;
		if ( wstrWay == L"static" )
		{
			pmMode = MIOSoundStream::pmStaticPlay ;
		}
		else if ( wstrWay == L"dynamic" )
		{
			pmMode = MIOSoundStream::pmDynamicPlay ;
		}
		else
		{
			ERIFile	erif ;
			if ( erif.Open( &file, ERIFile::otReadHeader ) )
			{
				OutputError
					( "sound タグ : "
						+ strFile + " を読み込めませんでした。" ) ;
				return	eslErrGeneral ;
			}
			if ( !(erif.m_fdwReadMask & ERIFile::rmSoundInfo) )
			{
				OutputError
					( "sound タグ : "
						+ strFile + " を読み込めませんでした。" ) ;
				return	eslErrGeneral ;
			}
			if ( erif.m_MIOInfHdr.dwAllSampleCount < 100000 )
			{
				pmMode = MIOSoundStream::pmStaticPlay ;
			}
			else
			{
				pmMode = MIOSoundStream::pmDynamicPlay ;
			}
			erif.Close( ) ;
			file.Seek( 0, ESLFileObject::FromBegin ) ;
		}
		MIOSoundStream *	pSound = new MIOSoundStream ;
		if ( pSound->Open( file, pmMode ) )
		{
			OutputError
				( "sound タグ : "
					+ strFile + " を読み込めませんでした。" ) ;
			delete	pSound ;
			return	eslErrGeneral ;
		}
		//
		return	AddResource( wstrID, pSound ) ;
	}
	else
	{
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

// リソース追加
//////////////////////////////////////////////////////////////////////////////
ESLError EFormResourceManager::AddResource
		( const wchar_t * pwszID, ESLObject * pRes )
{
	m_wstaResource.Add( pwszID, pRes ) ;
	return	eslErrSuccess ;
}

// リソース取得
//////////////////////////////////////////////////////////////////////////////
ESLObject * EFormResourceManager::GetResourceAs( const wchar_t * pwszID )
{
	return	m_wstaResource.GetAs( pwszID ) ;
}

// スタイルセクション読み込み
//////////////////////////////////////////////////////////////////////////////
ESLError EFormResourceManager::ReadStyleSection( EDescription & descStyle )
{
	int	i, nCount ;
	nCount = descStyle.GetContentTagCount( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		EDescription *	pTag = descStyle.GetContentTagAt( i ) ;
		if ( pTag != NULL )
		{
			AddStyle( *pTag ) ;
		}
	}
	return	eslErrSuccess ;
}

// スタイル追加
//////////////////////////////////////////////////////////////////////////////
ESLError EFormResourceManager::AddStyle( EDescription & descStyle )
{
	EWideString	wstrID = descStyle.GetAttrString( L"id", L"" ) ;
	m_wstaStyle.Add( wstrID, new EDescription( descStyle ) ) ;
	return	eslErrSuccess ;
}

// スタイル取得
//////////////////////////////////////////////////////////////////////////////
EDescription * EFormResourceManager::GetStyleAs( const wchar_t * pwszID )
{
	return	m_wstaStyle.GetAs( pwszID ) ;
}

// ページ作成
//////////////////////////////////////////////////////////////////////////////
ESLError EFormResourceManager::CreateFormedPage
	( ESpriteInterface & siPage, EDescription & descPage )
{
	EWideString	wstrBG = descPage.GetAttrString( L"bg", NULL ) ;
	siPage.SetID( descPage.GetAttrString( L"id", NULL ) ) ;
	//
	if ( !wstrBG.IsEmpty() )
	{
		//
		// 背景画像を指定する
		//
		EGL_IMAGE_RECT	irectBG ;
		EGLAnimation *	pAnimation ;
		EGL_IMAGE_RECT *	pRectBG ;
		pRectBG = GetImageResource( wstrBG, pAnimation, irectBG ) ;
		if ( pAnimation == NULL )
		{
			OutputError( EString(wstrBG) + "画像リソースが見つかりません。" ) ;
			return	eslErrGeneral ;
		}
		siPage.DuplicateImage( *pAnimation ) ;
		//
		// 背景画像を設定する
		//
		EAnimationSprite *	pBGSprite = new EAnimationSprite ;
		if ( pRectBG == NULL )
		{
			pBGSprite->CreateAnimation( pAnimation ) ;
		}
		else
		{
			EGLRect	rectView = *pRectBG ;
			pBGSprite->SetImageView( *pAnimation, &rectView ) ;
		}
		//
		EImageSprite::PARAMETER	param ;
		pBGSprite->GetParameter( param ) ;
		param.dwFlags = 0 ;
		pBGSprite->SetParameter( param ) ;
		siPage.EnableFillBack( false ) ;
		//
		pBGSprite->SetID( L"ID_BG" ) ;
		siPage.AddSprite( 0x7FFFFFFF, pBGSprite ) ;
		pBGSprite->SetVisible( true ) ;
	}
	else
	{
		//
		// 背景画像は指定しない
		//
		int	nWidth = descPage.GetAttrInteger( L"width", 128 ) ;
		int	nHeight = descPage.GetAttrInteger( L"height", 128 ) ;
		siPage.CreateImage( EIF_RGBA_BITMAP, nWidth, nHeight, 32, 0 ) ;
		siPage.ReverseVertically( ) ;
		siPage.EnableFillBack( true ) ;
	}
	//
	EImageSprite::PARAMETER	param ;
	siPage.GetParameter( param ) ;
	param.ptDstPos.x = descPage.GetAttrInteger( L"x", 0 ) ;
	param.ptDstPos.y = descPage.GetAttrInteger( L"y", 0 ) ;
	param.ptRevCenter.x = 0 ;
	param.ptRevCenter.y = 0 ;
	param.rHorzUnit = 1 ;
	param.rVertUnit = 1 ;
	siPage.SetParameter( param ) ;
	siPage.m_pfrmRsrc = this ;
	//
	return	eslErrSuccess ;
}

// フォームセクション読み込み
//////////////////////////////////////////////////////////////////////////////
ESLError EFormResourceManager::ReadFormSection
	( ESpriteInterface & siPage, EDescription & descPage )
{
	int	i, nCount ;
	nCount = descPage.GetContentTagCount( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		EDescription *	pTag = descPage.GetContentTagAt( i ) ;
		if ( pTag != NULL )
		{
			ReadFormTag( siPage, *pTag ) ;
		}
	}
	return	eslErrSuccess ;
}

// フォーム読み込み
//////////////////////////////////////////////////////////////////////////////
ESLError EFormResourceManager::ReadFormTag
	( ESpriteInterface & siPage, EDescription & descTag )
{
	//
	// アイテム作成
	//
	ESpriteInterface *	pSprite = ReadTagObject( siPage, descTag ) ;
	//
	// アイテム設定
	//
	if ( pSprite == NULL )
	{
		return	eslErrGeneral ;
	}
	DWORD	dwFlags = pSprite->GetFunctionFlags( ) ;
	//
	EWideString	wstrGroup = descTag.GetAttrString( L"group", L"" ) ;
	if ( wstrGroup == L"true" )
		dwFlags |= ESpriteInterface::ffGroup ;
	else if ( wstrGroup == L"false" )
		dwFlags &= ~ESpriteInterface::ffGroup ;
	//
	EWideString	wstrTabStop = descTag.GetAttrString( L"tab_stop", L"" ) ;
	if ( wstrTabStop == L"true" )
		dwFlags |= ESpriteInterface::ffTabStop ;
	else if ( wstrTabStop == L"false" )
		dwFlags &= ~ESpriteInterface::ffTabStop ;
	//
	pSprite->SetFunctionFlags( dwFlags ) ;
	//
	pSprite->MovePosition
		( EGLPoint( descTag.GetAttrInteger( L"x", 0 ),
					descTag.GetAttrInteger( L"y", 0 ) ) ) ;
	siPage.AddSpriteItem
		( descTag.GetAttrString( L"id", L"" ), pSprite ) ;
	pSprite->SetVisible( true ) ;
	//
	return	eslErrSuccess ;
}

// オブジェクト作成
//////////////////////////////////////////////////////////////////////////////
ESpriteInterface * EFormResourceManager::ReadTagObject
	( ESpriteInterface & siPage, EDescription & descTag )
{
	//
	// アイテム生成
	//
	const EWideString &	wstrTag = descTag.Tag( ) ;
	ESpriteInterface *	pSprite ;
	//
	if ( wstrTag == L"image" )
	{
		pSprite = ReadImageTag( siPage, descTag ) ;
	}
	else if ( wstrTag == L"static_frame" )
	{
		pSprite = ReadStaticFrameTag( siPage, descTag ) ;
	}
	else if ( wstrTag == L"static_text" )
	{
		pSprite = ReadStaticTextTag( siPage, descTag ) ;
	}
	else if ( wstrTag == L"progress_bar" )
	{
		pSprite = ReadProgressBarTag( siPage, descTag ) ;
	}
	else if ( wstrTag == L"button" )
	{
		pSprite = ReadButtonTag( siPage, descTag ) ;
	}
	else if ( wstrTag == L"scroll_bar" )
	{
		pSprite = ReadScrollBarTag( siPage, descTag ) ;
	}
	else if ( wstrTag == L"edit_text" )
	{
		pSprite = ReadEditTextTag( siPage, descTag ) ;
	}
	else if ( wstrTag == L"list_view" )
	{
		pSprite = ReadListViewTag( siPage, descTag ) ;
	}
	else if ( wstrTag == L"object" )
	{
		pSprite = ReadObjectTag( siPage, descTag ) ;
	}
	else
	{
		pSprite = ReadExtendedFromTag( siPage, descTag ) ;
	}
	//
	// コマンド実行
	//
	EDescription *	pdscCmd = descTag.GetContentTagAs( 0, L"command" ) ;
	if ( pdscCmd != NULL )
	{
		pSprite->SendCommand( *pdscCmd ) ;
	}
	//
	return	pSprite ;
}

// image フォーム読み込み
//////////////////////////////////////////////////////////////////////////////
ESpriteInterface * EFormResourceManager::ReadImageTag
	( ESpriteInterface & siPage, EDescription & descTag )
{
	//
	// 画像リソースを取得
	//
	EGL_IMAGE_RECT	irectClip ;
	EGLAnimation *	pImage ;
	EWideString	wstrImage = descTag.GetAttrString( L"rsrc", L"" ) ;
	EGL_IMAGE_RECT *	pClip =
		GetImageResource( wstrImage, pImage, irectClip ) ;
	if ( pImage == NULL )
	{
		OutputError( "image タグ : " + EString(wstrImage) ) ;
		return	NULL ;
	}
	//
	// 画像を設定
	//
	EAnimationSprite *	pAnime = new EAnimationSprite ;
	if ( pClip == NULL )
	{
		pAnime->CreateAnimation( pImage ) ;
	}
	else
	{
		EGL_IMAGE_INFO	eiiClipped ;
		if ( ::eglGetClippedImageInfo( &eiiClipped, *pImage, pClip ) )
		{
			OutputError
				( "image タグ : " + EString(wstrImage)
							+ " 矩形が画像サイズを超えています。" ) ;
			delete	pAnime ;
			return	NULL ;
		}
		pAnime->DuplicateImage( &eiiClipped ) ;
		pAnime->EnableFillBack( false ) ;
	}
	//
	return	pAnime ;
}

// static_frame フォーム読み込み
//////////////////////////////////////////////////////////////////////////////
ESpriteInterface * EFormResourceManager::ReadStaticFrameTag
	( ESpriteInterface & siPage, EDescription & descTag )
{
	//
	// スタイル読み込み
	//
	EGL_IMAGE_INFO	eiiBuffer[IMGBUF_SIZE] ;
	EStaticFrameSprite::FRAME_STYLE	style ;
	EWideString	wstrStyle = descTag.GetAttrString( L"style", L"" ) ;
	EDescription *	pStyle = GetStyleAs( wstrStyle ) ;
	if ( pStyle == NULL )
	{
		OutputError
			( "static_frame : "
				+ EString(wstrStyle) + " スタイルが見つかりません。" ) ;
		return	NULL ;
	}
	if ( GetStaticFrameStyle( style, *pStyle, eiiBuffer ) )
	{
		return	NULL ;
	}
	//
	// オブジェクト作成
	//
	EStaticFrameSprite *	pFrame = new EStaticFrameSprite ;
	EGL_SIZE	sizeFrame ;
	sizeFrame.w = descTag.GetAttrInteger( L"width", 16 ) ;
	sizeFrame.h = descTag.GetAttrInteger( L"height", 16 ) ;
	//
	if ( pFrame->CreateStaticFrame( style, sizeFrame.w, sizeFrame.h ) )
	{
		delete	pFrame ;
		return	NULL ;
	}
	//
	return	pFrame ;
}

// static_text フォーム読み込み
//////////////////////////////////////////////////////////////////////////////
ESpriteInterface * EFormResourceManager::ReadStaticTextTag
	( ESpriteInterface & siPage, EDescription & descTag )
{
	//
	// スタイル読み込み
	//
	EStaticTextSprite::TEXT_STYLE	style ;
	EWideString	wstrStyle = descTag.GetAttrString( L"style", L"" ) ;
	EDescription *	pStyle = GetStyleAs( wstrStyle ) ;
	if ( pStyle == NULL )
	{
		OutputError
			( "static_text : "
				+ EString(wstrStyle) + " スタイルが見つかりません。" ) ;
		return	NULL ;
	}
	if ( GetStaticTextStyle( style, *pStyle ) )
	{
		return	NULL ;
	}
	//
	// オブジェクト作成
	//
	EStaticTextSprite *	pText = new EStaticTextSprite ;
	EWideString	wstrText ;
	EGL_SIZE	sizeText ;
	sizeText.w = descTag.GetAttrInteger( L"width", 16 ) ;
	sizeText.h = descTag.GetAttrInteger( L"height", 16 ) ;
	wstrText = descTag.GetAttrString( L"text", L"" ) ;
	style.rectExt.left = 0 ;
	style.rectExt.top = 0 ;
	style.rectExt.right = sizeText.w - 1 ;
	style.rectExt.bottom = sizeText.h - 1 ;
	style.pwszText = wstrText ;
	//
	if ( pText->CreateText( style, sizeText.w, sizeText.h ) )
	{
		delete	pText ;
		return	NULL ;
	}
	//
	return	pText ;
}

// progress_bar フォーム読み込み
//////////////////////////////////////////////////////////////////////////////
ESpriteInterface * EFormResourceManager::ReadProgressBarTag
	( ESpriteInterface & siPage, EDescription & descTag )
{
	//
	// スタイル読み込み
	//
	EGL_IMAGE_INFO	eiiBuffer[IMGBUF_SIZE] ;
	EProgressBarSprite::BAR_STYLE	style ;
	EWideString	wstrStyle = descTag.GetAttrString( L"style", L"" ) ;
	EDescription *	pStyle = GetStyleAs( wstrStyle ) ;
	if ( pStyle == NULL )
	{
		OutputError
			( "progress_bar : "
				+ EString(wstrStyle) + " スタイルが見つかりません。" ) ;
		return	NULL ;
	}
	if ( GetProgressBarStyle( style, *pStyle, eiiBuffer ) )
	{
		return	NULL ;
	}
	//
	// オブジェクト作成
	//
	EProgressBarSprite *	pBar = new EProgressBarSprite ;
	if ( style.pbtType == EProgressBarSprite::pbtVert )
	{
		style.sizeExt.h = descTag.GetAttrInteger( L"width", 16 ) ;
		style.sizeExt.w = style.pFrameWay->dwImageWidth ;
	}
	else
	{
		style.sizeExt.w = descTag.GetAttrInteger( L"width", 16 ) ;
		style.sizeExt.h = style.pFrameWay->dwImageHeight ;
	}
	if ( pBar->CreateProgressBar( style ) )
	{
		delete	pBar ;
		return	NULL ;
	}
	return	pBar ;
}

// button フォーム読み込み
//////////////////////////////////////////////////////////////////////////////
ESpriteInterface * EFormResourceManager::ReadButtonTag
	( ESpriteInterface & siPage, EDescription & descTag )
{
	//
	// スタイル読み込み
	//
	EGL_IMAGE_INFO	eiiBuffer[IMGBUF_SIZE] ;
	EButtonSprite::BUTTON_STYLE	style ;
	EWideString	wstrStyle = descTag.GetAttrString( L"style", L"" ) ;
	EDescription *	pStyle = GetStyleAs( wstrStyle ) ;
	if ( pStyle == NULL )
	{
		OutputError
			( "button : "
				+ EString(wstrStyle) + " スタイルが見つかりません。" ) ;
		return	NULL ;
	}
	if ( GetButtonStyle( style, *pStyle, eiiBuffer ) )
	{
		return	NULL ;
	}
	//
	// オブジェクト作成
	//
	EButtonSprite *	pButton = new EButtonSprite ;
	EWideString	wstrText ;
	int			i ;
	ESLError	errResult ;
	if ( style.btType > EButtonSprite::btTextButton )
	{
		style.sizeExt.w = descTag.GetAttrInteger( L"width", 16 ) ;
		style.sizeExt.h = descTag.GetAttrInteger( L"height", 16 ) ;
	}
	wstrText = descTag.GetAttrString( L"text", L"" ) ;
	for ( i = 0; i < EButtonSprite::bsMax; i ++ )
	{
		style.tsTextStyle[i].pwszText = wstrText ;
	}
	errResult = pButton->CreateButton( style ) ;
	if ( errResult )
	{
		delete	pButton ;
		return	NULL ;
	}
	//
	EWideString	wstrSE ;
	wstrSE = descTag.GetAttrString( L"focus_se", L"" ) ;
	if ( !wstrSE.IsEmpty() )
	{
		pButton->SetSoundOnFocus( wstrSE ) ;
	}
	wstrSE = descTag.GetAttrString( L"pushed_se", L"" ) ;
	if ( !wstrSE.IsEmpty() )
	{
		pButton->SetSoundOnPushed( wstrSE ) ;
	}
	//
	return	pButton ;
}

// scroll_bar フォーム読み込み
//////////////////////////////////////////////////////////////////////////////
ESpriteInterface * EFormResourceManager::ReadScrollBarTag
	( ESpriteInterface & siPage, EDescription & descTag )
{
	//
	// スタイル読み込み
	//
	EGL_IMAGE_INFO	eiiBuffer[IMGBUF_SIZE] ;
	EScrollBarSprite::BAR_STYLE	style ;
	EWideString	wstrStyle = descTag.GetAttrString( L"style", L"" ) ;
	EDescription *	pStyle = GetStyleAs( wstrStyle ) ;
	if ( pStyle == NULL )
	{
		OutputError
			( "scroll_bar : "
				+ EString(wstrStyle) + " スタイルが見つかりません。" ) ;
		return	NULL ;
	}
	if ( GetScrollBarStyle( style, *pStyle, eiiBuffer ) )
	{
		return	NULL ;
	}
	//
	// オブジェクト作成
	//
	EWideString	wstrRelativeID = descTag.GetAttrString( L"id", L"" ) ;
	EScrollBarSprite *	pBar = new EScrollBarSprite ;
	if ( style.sbtBarType == EScrollBarSprite::sbtVert )
	{
		style.sizeBarExt.h =
			descTag.GetAttrInteger( L"width", style.sizeBarExt.h ) ;
		style.nColumnWidth =
			style.sizeBarExt.h
				- style.bsPrevButton.sizeExt.h
				- style.bsNextButton.sizeExt.h ;
		style.ptNextButton.y =
			style.sizeBarExt.h - style.bsNextButton.sizeExt.h ;
		//
		if ( wstrRelativeID.Right(4) == L"_VSB" )
		{
			wstrRelativeID =
				wstrRelativeID.Left( wstrRelativeID.GetLength() - 4 ) ;
		}
		else
		{
			wstrRelativeID = L"" ;
		}
	}
	else
	{
		style.sizeBarExt.w =
			descTag.GetAttrInteger( L"width", style.sizeBarExt.w ) ;
		style.nColumnWidth =
			style.sizeBarExt.w
				- style.bsPrevButton.sizeExt.w
				- style.bsNextButton.sizeExt.w ;
		style.ptNextButton.x =
			style.sizeBarExt.w - style.bsNextButton.sizeExt.w ;
		//
		if ( wstrRelativeID.Right(4) == L"_HSB" )
		{
			wstrRelativeID =
				wstrRelativeID.Left( wstrRelativeID.GetLength() - 4 ) ;
		}
		else
		{
			wstrRelativeID = L"" ;
		}
	}
	if ( pBar->CreateScrollBar( style ) )
	{
		delete	pBar ;
		return	NULL ;
	}
	//
	// スクロールバー関連付け
	//
	if ( !wstrRelativeID.IsEmpty() )
	{
		ESpriteInterface *	pItem =
			siPage.GetSpriteItemAs( wstrRelativeID ) ;
		if ( pItem != NULL )
		{
			pBar->AttachScrollItem( pItem ) ;
		}
	}
	//
	return	pBar ;
}

// edit_text フォーム読み込み
//////////////////////////////////////////////////////////////////////////////
ESpriteInterface * EFormResourceManager::ReadEditTextTag
	( ESpriteInterface & siPage, EDescription & descTag )
{
	//
	// スタイル読み込み
	//
	EGL_IMAGE_INFO	eiiBuffer[IMGBUF_SIZE] ;
	ETextEditSprite::EDIT_STYLE	style ;
	EWideString	wstrStyle = descTag.GetAttrString( L"style", L"" ) ;
	EDescription *	pStyle = GetStyleAs( wstrStyle ) ;
	if ( pStyle == NULL )
	{
		OutputError
			( "edit_text : "
				+ EString(wstrStyle) + " スタイルが見つかりません。" ) ;
		return	NULL ;
	}
	if ( GetEditTextStyle( style, *pStyle, eiiBuffer ) )
	{
		return	NULL ;
	}
	//
	// オブジェクト作成
	//
	ETextEditSprite *	pEdit = new ETextEditSprite ;
	style.sizeExt.w =
		descTag.GetAttrInteger( L"width", style.sizeExt.w ) ;
	style.sizeExt.h =
		descTag.GetAttrInteger( L"height", style.sizeExt.h ) ;
	if ( pEdit->CreateEdit( style ) )
	{
		delete	pEdit ;
		return	NULL ;
	}
	pEdit->SetEditText( descTag.GetAttrString( L"text", L"" ) ) ;
	return	pEdit ;
}

// list_view フォーム読み込み
//////////////////////////////////////////////////////////////////////////////
ESpriteInterface * EFormResourceManager::ReadListViewTag
	( ESpriteInterface & siPage, EDescription & descTag )
{
	//
	// スタイル読み込み
	//
	EListViewSprite::LIST_STYLE	style ;
	EWideString	wstrStyle = descTag.GetAttrString( L"style", L"" ) ;
	EDescription *	pStyle = GetStyleAs( wstrStyle ) ;
	if ( pStyle == NULL )
	{
		OutputError
			( "list_view : "
				+ EString(wstrStyle) + " スタイルが見つかりません。" ) ;
		return	NULL ;
	}
	if ( GetListViewStyle( style, *pStyle ) )
	{
		return	NULL ;
	}
	//
	// オブジェクト作成
	//
	ESLError	errResult ;
	EListViewSprite *	pList = new EListViewSprite ;
	style.sizeExt.w = descTag.GetAttrInteger( L"width", 16 ) ;
	style.sizeExt.h = descTag.GetAttrInteger( L"height", 16 ) ;
	errResult = pList->CreateListView( style ) ;
	if ( errResult )
	{
		delete	pList ;
		return	NULL ;
	}
	return	pList ;
}

// combo_list フォーム読み込み
//////////////////////////////////////////////////////////////////////////////
ESpriteInterface * EFormResourceManager::ReadComboListTag
	( ESpriteInterface & siPage, EDescription & descTag )
{
	//
	// スタイル読み込み
	//
	EComboListSprite::COMBO_STYLE	style ;
	EWideString	wstrStyle = descTag.GetAttrString( L"style", L"" ) ;
	EDescription *	pStyle = GetStyleAs( wstrStyle ) ;
	if ( pStyle == NULL )
	{
		OutputError
			( "combo_list : "
				+ EString(wstrStyle) + " スタイルが見つかりません。" ) ;
		return	NULL ;
	}
	EStreamBuffer	buf ;
	if ( GetComboListStyle( style, *pStyle, buf ) )
	{
		return	NULL ;
	}
	//
	// オブジェクト作成
	//
	ESLError	errResult ;
	EComboListSprite *	pCombo = new EComboListSprite ;
	style.sizeExt.w = descTag.GetAttrInteger( L"width", 16 ) ;
	style.sizeExt.h = descTag.GetAttrInteger( L"height", 16 ) ;
	errResult = pCombo->CreateCombo( style ) ;
	//
	return	pCombo ;
}

// object フォーム読み込み
//////////////////////////////////////////////////////////////////////////////
ESpriteInterface * EFormResourceManager::ReadObjectTag
	( ESpriteInterface & siPage, EDescription & descTag )
{
	//
	// スタイル取得
	//
	EWideString	wstrStyle = descTag.GetAttrString( L"style", NULL ) ;
	EDescription *	pStyle = GetPageFormAs( wstrStyle ) ;
	if ( pStyle == NULL )
	{
		OutputError
			( "object : "
				+ EString(wstrStyle) + " ページ識別子が見つかりません。" ) ;
		return	NULL ;
	}
	//
	// オブジェクト作成
	//
	if ( m_nPageNest >= 16 )
	{
		OutputError( "object : ネスト回数が１６回を超えました。" ) ;
		return	NULL ;
	}
	m_nPageNest ++ ;
	//
	ESpriteInterface *	pObject = new ESpriteInterface ;
	if ( CreateFormedPage( *pObject, *pStyle ) )
	{
		m_nPageNest -- ;
		delete	pObject ;
		return	NULL ;
	}
	if ( ReadFormSection( *pObject, *pStyle ) )
	{
		m_nPageNest -- ;
		delete	pObject ;
		return	NULL ;
	}
	m_nPageNest -- ;
	//
	return	pObject ;
}

// 未知のフォーム読み込み
//////////////////////////////////////////////////////////////////////////////
ESpriteInterface * EFormResourceManager::ReadExtendedFromTag
	( ESpriteInterface & siPage, EDescription & descTag )
{
	return	NULL ;
}

// ページリストを設定
//////////////////////////////////////////////////////////////////////////////
ESLError EFormResourceManager::AddPageList( EDescription & descTag )
{
	unsigned int	i, nCount ;
	nCount = descTag.GetContentTagCount( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		EDescription *	pTag = descTag.GetContentTagAt( i ) ;
		if ( pTag == NULL )
			continue ;
		if ( (pTag->Tag() != L"window") && (pTag->Tag() != L"page") )
			continue ;
		//
		EWideString	wstrID = pTag->GetAttrString( L"id", NULL ) ;
		m_wstaPage.SetAs( wstrID, new EDescription( *pTag ) ) ;
	}
	return	eslErrSuccess ;
}

// ページリストを削除
//////////////////////////////////////////////////////////////////////////////
void EFormResourceManager::RemovePageList( void )
{
	m_wstaPage.RemoveAll( ) ;
	m_nPageNest = 0 ;
}

// ページのフォームを取得
//////////////////////////////////////////////////////////////////////////////
EDescription *
	EFormResourceManager::GetPageFormAs( const wchar_t * pwszPageID )
{
	return	m_wstaPage.GetAs( pwszPageID ) ;
}

// フォントを列挙する
//////////////////////////////////////////////////////////////////////////////
static int CALLBACK EFormResourceManager_EnumFontFamProc
	( ENUMLOGFONTEX *lpelfe,
		NEWTEXTMETRICEX *lpntme, int FontType, LPARAM lParam )
{
	DWORD *	pdwFlags = (DWORD*) lParam ;
	*pdwFlags |= (FontType == TRUETYPE_FONTTYPE) ;
	return	1 ;
}

// static_frame 用スタイル読み込み
//////////////////////////////////////////////////////////////////////////////
ESLError EFormResourceManager::GetStaticFrameStyle
	( EStaticFrameSprite::FRAME_STYLE & style,
		EDescription & descStyle, EGL_IMAGE_INFO eiiBuffer[IMGBUF_SIZE] )
{
	//
	// 構造体初期化
	//////////////////////////////////////////////////////////////////////////
	::eslFillMemory( &style, 0, sizeof(EStaticFrameSprite::FRAME_STYLE) ) ;
	//
	// 画像取得
	//////////////////////////////////////////////////////////////////////////
	static const wchar_t *	pwszType[EStaticFrameSprite::ftMax] =
	{
		L"upper_left", L"upper", L"upper_right",
		L"left", L"pane", L"right",
		L"under_left", L"under", L"under_right"
	} ;
	EDescription *	pdscImage = descStyle.GetContentTagAs( 0, L"image" ) ;
	if ( pdscImage == NULL )
	{
		OutputError( "static_frame スタイル : image タグが見つかりません。" ) ;
		return	eslErrGeneral ;
	}
	for ( int i = 0; i < EStaticFrameSprite::ftMax; i ++ )
	{
		style.pFrame[i] =
			GetStillImageResource
				( pdscImage->GetAttrString
					( pwszType[i], NULL ), &eiiBuffer[i] ) ;
	}
	return	eslErrSuccess ;
}

// static_text 用スタイル読み込み
//////////////////////////////////////////////////////////////////////////////
ESLError EFormResourceManager::GetStaticTextStyle
	( EStaticTextSprite::TEXT_STYLE & style, EDescription & descStyle )
{
	//
	// 構造体初期化
	//////////////////////////////////////////////////////////////////////////
	::eslFillMemory( &style, 0, sizeof(EStaticTextSprite::TEXT_STYLE) ) ;
	//
	// アレンジメント設定
	//////////////////////////////////////////////////////////////////////////
	EDescription *	pTag ;
	int				iTag ;
	iTag = descStyle.FindContentTag( 0, L"arrange" ) ;
	if ( iTag >= 0 )
	{
		pTag = descStyle.GetContentTagAt( iTag ) ;
		ESLAssert( pTag != NULL ) ;
		//
		// 文字整列方法
		EWideString	wstrAlign = pTag->GetAttrString( L"align", L"left" ) ;
		if ( wstrAlign == L"left" )
			style.taAlign = EStaticTextSprite::taLeft ;
		else if ( wstrAlign == L"top" )
			style.taAlign = EStaticTextSprite::taTop ;
		else if ( wstrAlign == L"right" )
			style.taAlign = EStaticTextSprite::taRight ;
		else if ( wstrAlign == L"center" )
			style.taAlign = EStaticTextSprite::taCenter ;
		else
			style.taAlign = EStaticTextSprite::taAccordance ;
		//
		// 行間
		style.nLineHeight = pTag->GetAttrInteger( L"line_height", 16 ) ;
		//
		// インデント
		style.nIndent = pTag->GetAttrInteger( L"indent", 0 ) ;
		//
		// ピッチ
		style.nFontPitch = pTag->GetAttrInteger( L"pitch", 0 ) ;
		//
		// 表示矩形
		style.rectExt.left = pTag->GetAttrInteger( L"left", 0 ) ;
		style.rectExt.top = pTag->GetAttrInteger( L"top", 0 ) ;
		style.rectExt.right =
			style.rectExt.left
				+ pTag->GetAttrInteger( L"width", 640 ) - 1 ;
		style.rectExt.bottom =
			style.rectExt.top
				+ pTag->GetAttrInteger( L"height", 480 ) - 1 ;
	}
	else
	{
		style.taAlign = EStaticTextSprite::taLeft ;
		style.nLineHeight = 16 ;
		style.rectExt.right = 639 ;
		style.rectExt.bottom = 479 ;
	}
	//
	// フォント設定
	//////////////////////////////////////////////////////////////////////////
	LOGFONT	lfFont ;
	iTag = descStyle.FindContentTag( 0, L"font" ) ;
	::eslFillMemory( &lfFont, 0, sizeof(lfFont) ) ;
	lfFont.lfHeight = 16 ;
	lfFont.lfCharSet = DEFAULT_CHARSET ;
	lfFont.lfOutPrecision = OUT_DEFAULT_PRECIS ;
	lfFont.lfClipPrecision = CLIP_DEFAULT_PRECIS ;
	lfFont.lfQuality = DEFAULT_QUALITY ;
	lfFont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE ;
	if ( iTag >= 0 )
	{
		pTag = descStyle.GetContentTagAt( iTag ) ;
		ESLAssert( pTag != NULL ) ;
		//
		// サイズ
		lfFont.lfHeight = pTag->GetAttrInteger( L"size", 16 ) ;
		//
		// フォントフェース
		EString	strFace = pTag->GetAttrString( L"face", L"ＭＳ Ｐゴシック" ) ;
		int		iLast = 0 ;
		int		iFind = strFace.Find( ';', 0 ) ;
		while ( iFind > 0 )
		{
			EString	strFaceName = strFace.Middle( iLast, iFind - iLast ) ;
			DWORD	dwFlags = 0 ;
			HDC	hdc = ::CreateCompatibleDC( NULL ) ;
			LOGFONT	lfEnum ;
			::eslFillMemory( &lfEnum, 0, sizeof(lfEnum) ) ;
			lfEnum.lfCharSet = DEFAULT_CHARSET ;
			::eslMoveMemory
				( lfEnum.lfFaceName, strFace.CharPtr(),
					__min( strFaceName.GetLength(), LF_FACESIZE ) ) ;
			::EnumFontFamiliesEx
				( hdc, &lfEnum,
					(FONTENUMPROCA) &EFormResourceManager_EnumFontFamProc,
														(LPARAM) &dwFlags, 0 ) ;
			::DeleteDC( hdc ) ;
			if ( dwFlags )
			{
				strFace = strFaceName ;
				break ;
			}
			iLast = ++ iFind ;
			iFind = strFace.Find( ';', iLast ) ;
			if ( iFind < 0 )
			{
				strFace = strFace.Middle( iLast ) ;
				break ;
			}
		}
		::eslMoveMemory
			( lfFont.lfFaceName, strFace.CharPtr(),
				__min( strFace.GetLength() + 1, LF_FACESIZE ) ) ;
		//
		// 太字指定
		if ( pTag->GetAttrString( L"bold", L"" ) == L"true" )
		{
			lfFont.lfWeight = FW_BOLD ;
		}
		//
		// 斜体指定
		if ( pTag->GetAttrString( L"italic", L"" ) == L"true" )
		{
			lfFont.lfItalic = TRUE ;
		}
	}
	style.lfFont = lfFont ;
	//
	// 文字属性設定
	//////////////////////////////////////////////////////////////////////////
	iTag = descStyle.FindContentTag( 0, L"text" ) ;
	if ( iTag >= 0 )
	{
		pTag = descStyle.GetContentTagAt( iTag ) ;
		ESLAssert( pTag != NULL ) ;
		//
		// 文字色
		style.rgbColor.dwPixelCode = pTag->GetAttrInteger( L"color", 0 ) ;
		//
		// 文字透明度
		style.nTransparency = pTag->GetAttrInteger( L"transparency", 0 ) ;
	}
	//
	// 文字影属性設定
	//////////////////////////////////////////////////////////////////////////
	iTag = descStyle.FindContentTag( 0, L"shadow" ) ;
	if ( iTag >= 0 )
	{
		pTag = descStyle.GetContentTagAt( iTag ) ;
		ESLAssert( pTag != NULL ) ;
		//
		// オフセット座標
		style.ptShadowOffset.x = pTag->GetAttrInteger( L"x", 1 ) ;
		style.ptShadowOffset.y = pTag->GetAttrInteger( L"y", 1 ) ;
		//
		// 影色
		style.rgbShadow.dwPixelCode = pTag->GetAttrInteger( L"color", 0 ) ;
		//
		// 影透明度
		style.nShadowTrans = pTag->GetAttrInteger( L"transparency", 0x100 ) ;
	}
	else
	{
		style.nShadowTrans = 0x100 ;
	}
	//
	// 文字縁取り属性設定
	//////////////////////////////////////////////////////////////////////////
	iTag = descStyle.FindContentTag( 0, L"border" ) ;
	if ( iTag >= 0 )
	{
		pTag = descStyle.GetContentTagAt( iTag ) ;
		ESLAssert( pTag != NULL ) ;
		//
		style.nFlags |= EStaticTextSprite::txfBordering ;
		//
		// 縁取り色
		style.rgbBorder.dwPixelCode = pTag->GetAttrInteger( L"color", 0 ) ;
		//
		// 縁取り透明度
		style.nBorderTrans = pTag->GetAttrInteger( L"transparency", 0x100 ) ;
	}
	else
	{
		style.nBorderTrans = 0x100 ;
	}
	//
	return	eslErrSuccess ;
}

// progress_bar 用スタイル読み込み
//////////////////////////////////////////////////////////////////////////////
ESLError EFormResourceManager::GetProgressBarStyle
	( EProgressBarSprite::BAR_STYLE & style,
		EDescription & descStyle,
		EGL_IMAGE_INFO eiiBuffer[EFormResourceManager::IMGBUF_SIZE] )
{
	//
	// 構造体初期化
	//////////////////////////////////////////////////////////////////////////
	::eslFillMemory( &style, 0, sizeof(EProgressBarSprite::BAR_STYLE) ) ;
	//
	// アレンジメント設定
	//////////////////////////////////////////////////////////////////////////
	EDescription *	pTag ;
	int				iTag ;
	iTag = descStyle.FindContentTag( 0, L"arrange" ) ;
	if ( iTag >= 0 )
	{
		pTag = descStyle.GetContentTagAt( iTag ) ;
		ESLAssert( pTag != NULL ) ;
		//
		// タイプ
		EWideString	wstrType = pTag->GetAttrString( L"type", L"horz" ) ;
		if ( wstrType == L"vert" )
			style.pbtType = EProgressBarSprite::pbtVert ;
		else
			style.pbtType = EProgressBarSprite::pbtHorz ;
		//
		// バーのオフセット座標
		style.ptBarOffset.x = pTag->GetAttrInteger( L"bar_x", 0 ) ;
		style.ptBarOffset.y = pTag->GetAttrInteger( L"bar_y", 0 ) ;
	}
	//
	// フレーム画像
	//////////////////////////////////////////////////////////////////////////
	iTag = descStyle.FindContentTag( 0, L"frame" ) ;
	if ( iTag < 0 )
	{
		OutputError( "progress_bar スタイル : frame タグが見つかりません。" ) ;
		return	eslErrGeneral ;
	}
	pTag = descStyle.GetContentTagAt( iTag ) ;
	ESLAssert( pTag != NULL ) ;
	//
	style.pFrameLeft =
		GetStillImageResource
			( pTag->GetAttrString( L"left", L"" ), &eiiBuffer[0] ) ;
	style.pFrameRight =
		GetStillImageResource
			( pTag->GetAttrString( L"right", L"" ), &eiiBuffer[1] ) ;
	style.pFrameWay =
		GetStillImageResource
			( pTag->GetAttrString( L"way", L"" ), &eiiBuffer[2] ) ;
	//
	if ( !style.pFrameLeft || !style.pFrameRight || !style.pFrameWay )
	{
		return	eslErrGeneral ;
	}
	//
	// バー画像
	//////////////////////////////////////////////////////////////////////////
	iTag = descStyle.FindContentTag( 0, L"bar" ) ;
	if ( iTag < 0 )
	{
		OutputError( "progress_bar スタイル : bar タグが見つかりません。" ) ;
		return	eslErrGeneral ;
	}
	pTag = descStyle.GetContentTagAt( iTag ) ;
	ESLAssert( pTag != NULL ) ;
	//
	style.pBarLeft =
		GetStillImageResource
			( pTag->GetAttrString( L"left", L"" ), &eiiBuffer[3] ) ;
	style.pBarRight =
		GetStillImageResource
			( pTag->GetAttrString( L"right", L"" ), &eiiBuffer[4] ) ;
	style.pBarWay =
		GetStillImageResource
			( pTag->GetAttrString( L"way", L"" ), &eiiBuffer[5] ) ;
	//
	if ( !style.pBarLeft || !style.pBarRight || !style.pBarWay )
	{
		return	eslErrGeneral ;
	}
	//
	// デフォルトサイズ算出
	//////////////////////////////////////////////////////////////////////////
	if ( style.pbtType == EProgressBarSprite::pbtVert )
	{
		style.sizeExt.w = style.pFrameLeft->dwImageWidth ;
		if ( style.sizeExt.w < (int) style.pFrameRight->dwImageWidth )
			style.sizeExt.w = (int) style.pFrameRight->dwImageWidth ;
		if ( style.sizeExt.w < (int) style.pFrameWay->dwImageWidth )
			style.sizeExt.w = (int) style.pFrameWay->dwImageWidth ;
		//
		style.sizeExt.h =
			style.pFrameLeft->dwImageHeight
				+ style.pFrameRight->dwImageHeight
				+ style.pFrameWay->dwImageHeight ;
	}
	else
	{
		style.sizeExt.w =
			style.pFrameLeft->dwImageWidth
				+ style.pFrameRight->dwImageWidth
				+ style.pFrameWay->dwImageWidth ;
		//
		style.sizeExt.h = style.pFrameLeft->dwImageHeight ;
		if ( style.sizeExt.h < (int) style.pFrameRight->dwImageHeight )
			style.sizeExt.h = (int) style.pFrameRight->dwImageHeight ;
		if ( style.sizeExt.h < (int) style.pFrameWay->dwImageHeight )
			style.sizeExt.h = (int) style.pFrameWay->dwImageHeight ;
	}
	//
	return	eslErrSuccess ;
}

// button 用スタイル読み込み
//////////////////////////////////////////////////////////////////////////////
ESLError EFormResourceManager::GetButtonStyle
	( EButtonSprite::BUTTON_STYLE & style,
		EDescription & descStyle,
		EGL_IMAGE_INFO eiiBuffer[EFormResourceManager::IMGBUF_SIZE] )
{
	//
	// 構造体初期化
	//////////////////////////////////////////////////////////////////////////
	::eslFillMemory( &style, 0, sizeof(EButtonSprite::BUTTON_STYLE) ) ;
	//
	// アレンジメント設定
	//////////////////////////////////////////////////////////////////////////
	EDescription *	pTag ;
	int				iTag ;
	iTag = descStyle.FindContentTag( 0, L"arrange" ) ;
	if ( iTag >= 0 )
	{
		pTag = descStyle.GetContentTagAt( iTag ) ;
		ESLAssert( pTag != NULL ) ;
		//
		// タイプ
		EWideString	wstrType = pTag->GetAttrString( L"type", L"button" ) ;
		if ( wstrType == L"button" )
			style.btType = EButtonSprite::btTextButton ;
		else if ( wstrType == L"check" )
			style.btType = EButtonSprite::btCheckBox ;
		else
			style.btType = EButtonSprite::btRadioButton ;
	}
	//
	// 画像設定
	//////////////////////////////////////////////////////////////////////////
	static const wchar_t *	pwszTag[EButtonSprite::bsMax] =
	{
		L"normal", L"focus", L"pushed", L"pushed_focus",
		L"disabled", L"push_disabled", L"active_pushed"
	} ;
	int		i, j = 0 ;
	for ( i = 0; i < EButtonSprite::bsMax; i ++ )
	{
		iTag = descStyle.FindContentTag( 0, pwszTag[i] ) ;
		if ( iTag < 0 )
		{
			if ( i == EButtonSprite::bsPushedFocus )
			{
				iTag = descStyle.FindContentTag
							( 0, pwszTag[EButtonSprite::bsPushed] ) ;
			}
			else if ( i == EButtonSprite::bsPushDisabled )
			{
				iTag = descStyle.FindContentTag
							( 0, pwszTag[EButtonSprite::bsDisabled] ) ;
				if ( iTag < 0 )
				{
					iTag = descStyle.FindContentTag
								( 0, pwszTag[EButtonSprite::bsPushed] ) ;
				}
			}
			else if ( i == EButtonSprite::bsActivePushed )
			{
				iTag = descStyle.FindContentTag
							( 0, pwszTag[EButtonSprite::bsPushedFocus] ) ;
				if ( iTag < 0 )
				{
					iTag = descStyle.FindContentTag
								( 0, pwszTag[EButtonSprite::bsPushed] ) ;
				}
			}
			if ( iTag < 0 )
			{
				iTag = descStyle.FindContentTag( 0, pwszTag[0] ) ;
			}
			if ( iTag < 0 )
			{
				style.pImage[i] = NULL ;
				::eslFillMemory
					( &(style.tsTextStyle[i]),
						0, sizeof(style.tsTextStyle[i]) ) ;
				continue ;
//				OutputError
//					( "button スタイル : "
//						"通常状態の画像の指定がありません。" ) ;
//				return	eslErrGeneral ;
			}
		}
		pTag = descStyle.GetContentTagAt( iTag ) ;
		ESLAssert( pTag != NULL ) ;
		//
		EWideString	wstrImage = pTag->GetAttrString( L"image", L"" ) ;
		style.pImage[i] =
			GetStillImageResource( wstrImage, &eiiBuffer[j ++] ) ;
		if ( style.pImage[i] == NULL )
		{
			::eslFillMemory
				( &(style.tsTextStyle[i]),
					0, sizeof(style.tsTextStyle[i]) ) ;
			continue ;
//			OutputError( "button スタイル : "
//					+ EString(wstrImage) + " は無効な画像識別子です。" ) ;
//			return	eslErrGeneral ;
		}
		if ( style.sizeExt.w < (int) style.pImage[i]->dwImageWidth )
			style.sizeExt.w = style.pImage[i]->dwImageWidth ;
		if ( style.sizeExt.h < (int) style.pImage[i]->dwImageHeight )
			style.sizeExt.h = style.pImage[i]->dwImageHeight ;
		//
		if ( GetStaticTextStyle( style.tsTextStyle[i], *pTag ) )
		{
			return	eslErrGeneral ;
		}
	}
	//
	// マスク画像指定
	//////////////////////////////////////////////////////////////////////////
	pTag = descStyle.GetContentTagAs( 0, L"mask" ) ;
	if ( pTag != NULL )
	{
		style.pHitTestMask =
			GetStillImageResource
				( pTag->GetAttrString( L"image", NULL ), &eiiBuffer[j ++] ) ;
		if ( style.pHitTestMask != NULL )
		{
			if ( style.sizeExt.w < (int) style.pHitTestMask->dwImageWidth )
				style.sizeExt.w = style.pHitTestMask->dwImageWidth ;
			if ( style.sizeExt.h < (int) style.pHitTestMask->dwImageHeight )
				style.sizeExt.h = style.pHitTestMask->dwImageHeight ;
		}
		if ( pTag->GetAttrString( L"rect", L"false" ) == L"true" )
		{
			style.dwFlags |= EButtonSprite::bfHitExtRect ;
		}
	}
	//
	return	eslErrSuccess ;
}

// scroll_bar 用スタイル読み込み
//////////////////////////////////////////////////////////////////////////////
ESLError EFormResourceManager::GetScrollBarStyle
	( EScrollBarSprite::BAR_STYLE & style,
		EDescription & descStyle,
		EGL_IMAGE_INFO eiiBuffer[EFormResourceManager::IMGBUF_SIZE] )
{
	int			iBuf = 0 ;
	ESLError	errResult ;
	//
	// 構造体初期化
	//////////////////////////////////////////////////////////////////////////
	::eslFillMemory( &style, 0, sizeof(EScrollBarSprite::BAR_STYLE) ) ;
	//
	// アレンジメント設定
	//////////////////////////////////////////////////////////////////////////
	EDescription *	pTag ;
	int				iTag ;
	iTag = descStyle.FindContentTag( 0, L"arrange" ) ;
	if ( iTag >= 0 )
	{
		pTag = descStyle.GetContentTagAt( iTag ) ;
		ESLAssert( pTag != NULL ) ;
		//
		// タイプ
		EWideString	wstrType = pTag->GetAttrString( L"type", L"vert" ) ;
		if ( wstrType == L"vert" )
			style.sbtBarType = EScrollBarSprite::sbtVert ;
		else
			style.sbtBarType = EScrollBarSprite::sbtHorz ;
		//
		// バーのオフセット
		if ( style.sbtBarType == EScrollBarSprite::sbtVert )
			style.ptColumnPos.x = pTag->GetAttrInteger( L"bar_offset", 0 ) ;
		else
			style.ptColumnPos.y = pTag->GetAttrInteger( L"bar_offset", 0 ) ;
	}
	//
	// アレンジメント設定
	//////////////////////////////////////////////////////////////////////////
	pTag = descStyle.GetContentTagAs( 0, L"track" ) ;
	if ( pTag != NULL )
	{
		style.rctTrackSpace.left = pTag->GetAttrInteger( L"left", 0 ) ;
		style.rctTrackSpace.top = pTag->GetAttrInteger( L"top", 0 ) ;
		style.rctTrackSpace.right = pTag->GetAttrInteger( L"right", 0 ) ;
		style.rctTrackSpace.bottom = pTag->GetAttrInteger( L"bottom", 0 ) ;
	}
	//
	// 上ボタン設定
	//////////////////////////////////////////////////////////////////////////
	iTag = descStyle.FindContentTag( 0, L"up_button" ) ;
	if ( iTag >= 0 )
	do
	{
		pTag = descStyle.GetContentTagAt( iTag ) ;
		ESLAssert( pTag != NULL ) ;
		//
		EWideString	wstrStyle = pTag->GetAttrString( L"style", L"" ) ;
		EDescription *	pStyle = GetStyleAs( wstrStyle ) ;
		if ( pStyle != NULL )
		{
			errResult = GetButtonStyle
				( style.bsPrevButton, *pStyle, &eiiBuffer[iBuf] ) ;
		}
		else if ( pTag->GetContentTagCount() != 0 )
		{
			errResult = GetButtonStyle
				( style.bsPrevButton, *pTag, &eiiBuffer[iBuf] ) ;
		}
		else
		{
			::eslFillMemory
				( &(style.bsPrevButton),
					0, sizeof(style.bsPrevButton) ) ;
			style.bsPrevButton.btType = EButtonSprite::btMax ;
			break ;
		}
		iBuf += EButtonSprite::bsMax ;
		if ( errResult )
		{
			return	errResult ;
		}
	}
	while ( false ) ;
	//
	// 下ボタン設定
	//////////////////////////////////////////////////////////////////////////
	iTag = descStyle.FindContentTag( 0, L"down_button" ) ;
	if ( iTag >= 0 )
	do
	{
		pTag = descStyle.GetContentTagAt( iTag ) ;
		ESLAssert( pTag != NULL ) ;
		//
		EWideString	wstrStyle = pTag->GetAttrString( L"style", L"" ) ;
		EDescription *	pStyle = GetStyleAs( wstrStyle ) ;
		if ( pStyle != NULL )
		{
			errResult = GetButtonStyle
				( style.bsNextButton, *pStyle, &eiiBuffer[iBuf] ) ;
		}
		else if ( pTag->GetContentTagCount() != 0 )
		{
			errResult = GetButtonStyle
				( style.bsNextButton, *pTag, &eiiBuffer[iBuf] ) ;
		}
		else
		{
			::eslFillMemory
				( &(style.bsNextButton),
					0, sizeof(style.bsNextButton) ) ;
			style.bsNextButton.btType = EButtonSprite::btMax ;
			break ;
		}
		iBuf += EButtonSprite::bsMax ;
		if ( errResult )
		{
			return	errResult ;
		}
	}
	while ( false ) ;
	//
	// つまみ画像
	//////////////////////////////////////////////////////////////////////////
	static const wchar_t *	pwszTag[EScrollBarSprite::bsMax] =
	{
		L"normal", L"focus", L"tracking", L"disabled"
	} ;
	for ( int i = 0; i < EScrollBarSprite::bsMax; i ++ )
	{
		iTag = descStyle.FindContentTag( 0, pwszTag[i] ) ;
		if ( iTag < 0 )
		{
			OutputError( "scroll_bar スタイル : "
				+ EString(pwszTag[i]) + " タグが見つかりません。" ) ;
			return	eslErrGeneral ;
		}
		pTag = descStyle.GetContentTagAt( iTag ) ;
		ESLAssert( pTag != NULL ) ;
		//
		EWideString	wstrBar = pTag->GetAttrString( L"bar", L"" ) ;
		style.pBarImage[i] =
			GetStillImageResource( wstrBar, &eiiBuffer[iBuf ++] ) ;
		if ( style.pBarImage[i] == NULL )
		{
			OutputError( "scroll_bar スタイル : "
				+ EString(wstrBar) + " つまみ画像の指定が不正です。" ) ;
			return	eslErrGeneral ;
		}
		//
		EWideString	wstrColumn = pTag->GetAttrString( L"column", L"" ) ;
		style.pColumnImage[i] =
			GetStillImageResource( wstrColumn, &eiiBuffer[iBuf ++] ) ;
/*		if ( style.pColumnImage[i] == NULL )
		{
			OutputError( "scroll_bar スタイル : "
				+ EString(wstrColumn) + " 軌道カラム画像の指定が不正です。" ) ;
			return	eslErrGeneral ;
		}
*/		//
		EWideString	wstrProgress = pTag->GetAttrString( L"progress", L"" ) ;
		style.pProgressImage[i] =
			GetStillImageResource( wstrProgress, &eiiBuffer[iBuf ++] ) ;
		//
		if ( style.sizeBarExt.w < (int) style.pBarImage[i]->dwImageWidth )
			style.sizeBarExt.w = (int) style.pBarImage[i]->dwImageWidth ;
		if ( style.sizeBarExt.h < (int) style.pBarImage[i]->dwImageHeight )
			style.sizeBarExt.h = (int) style.pBarImage[i]->dwImageHeight ;
		if ( style.pColumnImage[i] != NULL )
		{
			if ( style.sizeBarExt.w < (int) style.pColumnImage[i]->dwImageWidth )
				style.sizeBarExt.w = (int) style.pColumnImage[i]->dwImageWidth ;
			if ( style.sizeBarExt.h < (int) style.pColumnImage[i]->dwImageHeight )
				style.sizeBarExt.h = (int) style.pColumnImage[i]->dwImageHeight ;
		}
	}
	//
	if ( style.sizeBarExt.w < style.bsPrevButton.sizeExt.w )
		style.sizeBarExt.w = style.bsPrevButton.sizeExt.w ;
	if ( style.sizeBarExt.w < style.bsNextButton.sizeExt.w )
		style.sizeBarExt.w = style.bsNextButton.sizeExt.w ;
	if ( style.sizeBarExt.h < style.bsPrevButton.sizeExt.h )
		style.sizeBarExt.h = style.bsPrevButton.sizeExt.h ;
	if ( style.sizeBarExt.h < style.bsNextButton.sizeExt.h )
		style.sizeBarExt.h = style.bsNextButton.sizeExt.h ;
	//
	if ( style.sbtBarType == EScrollBarSprite::sbtVert )
		style.ptColumnPos.y = style.bsPrevButton.sizeExt.h ;
	else
		style.ptColumnPos.x = style.bsPrevButton.sizeExt.w ;
	//
	return	eslErrSuccess ;
}

// edit_text 用スタイル読み込み
//////////////////////////////////////////////////////////////////////////////
ESLError EFormResourceManager::GetEditTextStyle
	( ETextEditSprite::EDIT_STYLE & style,
		EDescription & descStyle,
		EGL_IMAGE_INFO eiiBuffer[EFormResourceManager::IMGBUF_SIZE] )
{
	int		iBuf = 0 ;
	//
	// 構造体初期化
	//////////////////////////////////////////////////////////////////////////
	EDescription *	pTag ;
	int				iTag ;
	::eslFillMemory( &style, 0, sizeof(ETextEditSprite::EDIT_STYLE) ) ;
	style.etType = ETextEditSprite::etSingleLine ;
	pTag = descStyle.GetContentTagAs( 0, L"arrange" ) ;
	if ( pTag != NULL )
	{
		if ( pTag->GetAttrString( L"type", L"single" ) == L"multiline" )
		{
			style.etType = ETextEditSprite::etMultiLine ;
		}
	}
	//
	// フレーム画像設定
	//////////////////////////////////////////////////////////////////////////
	iTag = descStyle.FindContentTag( 0, L"frame" ) ;
	if ( iTag < 0 )
	{
		OutputError( "edit_text スタイル : frame タグが見つかりません。" ) ;
		return	eslErrGeneral ;
	}
	pTag = descStyle.GetContentTagAt( iTag ) ;
	ESLAssert( pTag != NULL ) ;
	//
	// 左端画像
	EWideString	wstrImage ;
	wstrImage = pTag->GetAttrString( L"left_image", L"" ) ;
	style.pLeftSide =
		GetStillImageResource( wstrImage, &eiiBuffer[iBuf ++] ) ;
/*	if ( style.pLeftSide == NULL )
	{
		OutputError( "edit_text スタイル : "
			+ EString(wstrImage) + " 左端画像の指定が無効です。" ) ;
		return	eslErrGeneral ;
	}
*/	//
	// 右端画像
	wstrImage = pTag->GetAttrString( L"right_image", L"" ) ;
	style.pRightSide =
		GetStillImageResource( wstrImage, &eiiBuffer[iBuf ++] ) ;
/*	if ( style.pRightSide == NULL )
	{
		OutputError( "edit_text スタイル : "
			+ EString(wstrImage) + " 右端画像の指定が無効です。" ) ;
		return	eslErrGeneral ;
	}
*/	//
	// 中央画像
	wstrImage = pTag->GetAttrString( L"text_way_image", L"" ) ;
	style.pTextWay =
		GetStillImageResource( wstrImage, &eiiBuffer[iBuf ++] ) ;
/*	if ( style.pTextWay == NULL )
	{
		OutputError( "edit_text スタイル : "
			+ EString(wstrImage) + " 中央画像の指定が無効です。" ) ;
		return	eslErrGeneral ;
	}
*/	//
	if ( style.pLeftSide && style.pRightSide && style.pTextWay )
	{
		style.sizeExt.w =
			style.pLeftSide->dwImageWidth
				+ style.pRightSide->dwImageWidth
				+ style.pTextWay->dwImageWidth ;
		//
		style.sizeExt.h = style.pLeftSide->dwImageHeight ;
		if ( style.sizeExt.h < (int) style.pRightSide->dwImageHeight )
			style.sizeExt.h = (int) style.pRightSide->dwImageHeight ;
		if ( style.sizeExt.h < (int) style.pTextWay->dwImageHeight )
			style.sizeExt.h = (int) style.pTextWay->dwImageHeight ;
	}
	else
	{
		style.sizeExt.w = 1 ;
		style.sizeExt.h = 1 ;
	}
	//
	style.nEditTop = 0 ;
	style.nEditBottom = style.sizeExt.h - 1 ;
	//
	// カレット設定
	//////////////////////////////////////////////////////////////////////////
	iTag = descStyle.FindContentTag( 0, L"caret" ) ;
	if ( iTag >= 0 )
	{
		pTag = descStyle.GetContentTagAt( iTag ) ;
		ESLAssert( pTag != NULL ) ;
		//
		// カレットサイズ
		style.sizeCaret.w = pTag->GetAttrInteger( L"width", 0 ) ;
		style.sizeCaret.h = pTag->GetAttrInteger( L"height", 0 ) ;
		//
		// 点滅間隔
		style.nCaretInterval = pTag->GetAttrInteger( L"interval", 500 ) ;
		//
		// カレット色
		style.rgbCaretColor.dwPixelCode =
						pTag->GetAttrInteger( L"color", 500 ) ;
	}
	//
	// フォント設定
	//////////////////////////////////////////////////////////////////////////
	iTag = descStyle.FindContentTag( 0, L"font" ) ;
	if ( iTag >= 0 )
	{
		pTag = descStyle.GetContentTagAt( iTag ) ;
		ESLAssert( pTag != NULL ) ;
		//
		GetEditTextFontStyle( style.lfEditFont, *pTag ) ;
	}
	style.fIMEFont = false ;
	style.lfIMEFont = style.lfEditFont ;
	//
	iTag = descStyle.FindContentTag( 0, L"ime_font" ) ;
	if ( iTag >= 0 )
	{
		pTag = descStyle.GetContentTagAt( iTag ) ;
		ESLAssert( pTag != NULL ) ;
		//
		GetEditTextFontStyle( style.lfIMEFont, *pTag ) ;
		style.fIMEFont = true ;
	}
	//
	// 編集設定
	//////////////////////////////////////////////////////////////////////////
	iTag = descStyle.FindContentTag( 0, L"edit" ) ;
	if ( iTag >= 0 )
	{
		pTag = descStyle.GetContentTagAt( iTag ) ;
		ESLAssert( pTag != NULL ) ;
		//
		// 編集領域
		style.nEditTop =
			pTag->GetAttrInteger( L"top", style.nEditTop ) ;
		style.nEditBottom =
			pTag->GetAttrInteger( L"bottom", style.nEditBottom ) ;
		//
		// 文字色
		style.rgbTextColor.dwPixelCode =
						pTag->GetAttrInteger( L"color", 0 ) ;
		style.rgbSelTextColor.dwPixelCode =
						pTag->GetAttrInteger( L"sel_color", 0 ) ;
	}
	//
	return	eslErrSuccess ;
}

void EFormResourceManager::GetEditTextFontStyle
	( LOGFONT& lfFont, EDescription & descFont )
{
	::eslFillMemory( &lfFont, 0, sizeof(LOGFONT) ) ;
	lfFont.lfCharSet = DEFAULT_CHARSET ;
	lfFont.lfOutPrecision = OUT_DEFAULT_PRECIS ;
	lfFont.lfClipPrecision = CLIP_DEFAULT_PRECIS ;
	lfFont.lfQuality = DEFAULT_QUALITY ;
	lfFont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE ;
	//
	// サイズ
	lfFont.lfHeight = descFont.GetAttrInteger( L"size", 16 ) ;
	//
	// フォントフェース
	EString	strFace = descFont.GetAttrString( L"face", L"ＭＳ Ｐゴシック" ) ;
	::eslMoveMemory
		( lfFont.lfFaceName, strFace.CharPtr(),
				__max( strFace.GetLength(), LF_FACESIZE ) ) ;
	//
	// 太字指定
	if ( descFont.GetAttrString( L"bold", L"" ) == L"true" )
	{
		lfFont.lfWeight = FW_BOLD ;
	}
	//
	// 斜体指定
	if ( descFont.GetAttrString( L"italic", L"" ) == L"true" )
	{
		lfFont.lfItalic = TRUE ;
	}
}

// list_view 用スタイル読み込み
//////////////////////////////////////////////////////////////////////////////
ESLError EFormResourceManager::GetListViewStyle
	( EListViewSprite::LIST_STYLE & style, EDescription & descStyle )
{
	//
	// 構造体初期化
	//////////////////////////////////////////////////////////////////////////
	::eslFillMemory( &style, 0, sizeof(EListViewSprite::LIST_STYLE) ) ;
	//
	// アレンジメント設定
	//////////////////////////////////////////////////////////////////////////
	EDescription *	pTag ;
	pTag = descStyle.GetContentTagAs( 0, L"arrange" ) ;
	if ( pTag != NULL )
	{
		EWideString	wstrType = pTag->GetAttrString( L"type", NULL ) ;
		if ( wstrType == L"sel_multiple" )
		{
			style.nTypeFlags = EListViewSprite::ltMultiSelect ;
		}
		else
		{
			style.nTypeFlags = EListViewSprite::ltSingleSelect ;
		}
		style.nLineHeight = pTag->GetAttrInteger( L"line_height", 16 ) ;
	}
	//
	// テキストスタイル設定
	//////////////////////////////////////////////////////////////////////////
	static const wchar_t *	pwszTextStyle[EListViewSprite::lsMax] =
	{
		L"normal", L"focus", L"pushed", L"pushed_focus"
	} ;
	for ( int i = 0; i < EListViewSprite::lsMax; i ++ )
	{
		pTag = descStyle.GetContentTagAs( 0, pwszTextStyle[i] ) ;
		if ( pTag == NULL )
		{
			pTag = descStyle.GetContentTagAs( 0, pwszTextStyle[0] ) ;
			if ( pTag == NULL )
			{
				return	eslErrGeneral ;
			}
		}
		style.lsLine[i].rgbaBackColor.dwPixelCode =
					pTag->GetAttrInteger( L"back_color", 0 ) ;
		if ( GetStaticTextStyle( style.lsLine[i].tsText, *pTag ) )
		{
			return	eslErrGeneral ;
		}
	}
	return	eslErrSuccess ;
}

// combo_list 用スタイル読み込み
//////////////////////////////////////////////////////////////////////////////
ESLError EFormResourceManager::GetComboListStyle
	( EComboListSprite::COMBO_STYLE & style,
		EDescription & descStyle, EStreamBuffer bufImage )
{
	//
	// 構造体初期化
	//////////////////////////////////////////////////////////////////////////
	::eslFillMemory( &style, 0, sizeof(EComboListSprite::COMBO_STYLE) ) ;
	//
	// アレンジメント設定
	//////////////////////////////////////////////////////////////////////////
	EDescription *	pTag ;
	pTag = descStyle.GetContentTagAs( 0, L"arrange" ) ;
	if ( pTag != NULL )
	{
		EWideString	wstrType = pTag->GetAttrString( L"type", NULL ) ;
		if ( wstrType == L"combolist" )
		{
			style.nType = EComboListSprite::ctComboList ;
		}
		else if ( wstrType == L"dropdown" )
		{
			style.nType = EComboListSprite::ctDropDown ;
		}
		else
		{
			style.nType = EComboListSprite::ctDropDownList ;
		}
	}
	//
	// エディットテキストスタイル
	//////////////////////////////////////////////////////////////////////////
	EDescription *	pdscStyle ;
	PEGL_IMAGE_INFO	pInfBuf ;
	EWideString	wstrStyleID ;
	const int	nBufSize = sizeof(EGL_IMAGE_INFO) * IMGBUF_SIZE ;
	pTag = descStyle.GetContentTagAs( 0, L"edit_text" ) ;
	if ( pTag != NULL )
	{
		pdscStyle = GetStyleAs( pTag->GetAttrString( L"style", NULL ) ) ;
		if ( pdscStyle == NULL )
		{
			pdscStyle = pTag ;
		}
		style.nFlags |= EComboListSprite::sfEditCtrl ;
		pInfBuf = (PEGL_IMAGE_INFO) bufImage.PutBuffer( nBufSize ) ;
		if ( GetEditTextStyle( style.esEdit, *pdscStyle, pInfBuf ) )
		{
			return	eslErrGeneral ;
		}
		bufImage.Flush( nBufSize ) ;
	}
	//
	// ボタンスタイル
	//////////////////////////////////////////////////////////////////////////
	pTag = descStyle.GetContentTagAs( 0, L"drop_button" ) ;
	if ( pTag != NULL )
	{
		pdscStyle = GetStyleAs( pTag->GetAttrString( L"style", NULL ) ) ;
		if ( pdscStyle == NULL )
		{
			pdscStyle = pTag ;
		}
		style.nFlags |= EComboListSprite::sfDropDownBtn ;
		pInfBuf = (PEGL_IMAGE_INFO) bufImage.PutBuffer( nBufSize ) ;
		if ( GetButtonStyle( style.bsButton, *pdscStyle, pInfBuf ) )
		{
			return	eslErrGeneral ;
		}
		bufImage.Flush( nBufSize ) ;
	}
	//
	// リストビュースタイル
	//////////////////////////////////////////////////////////////////////////
	pTag = descStyle.GetContentTagAs( 0, L"list_view" ) ;
	if ( pTag != NULL )
	{
		pdscStyle = GetStyleAs( pTag->GetAttrString( L"style", NULL ) ) ;
		if ( pdscStyle == NULL )
		{
			pdscStyle = pTag ;
		}
		style.nFlags |= EComboListSprite::sfListView ;
		if ( GetListViewStyle( style.lsList, *pdscStyle ) )
		{
			return	eslErrGeneral ;
		}
	}
	//
	// フレームスタイル
	//////////////////////////////////////////////////////////////////////////
	pTag = descStyle.GetContentTagAs( 0, L"list_frame" ) ;
	if ( pTag != NULL )
	{
		pdscStyle = GetStyleAs( pTag->GetAttrString( L"style", NULL ) ) ;
		if ( pdscStyle == NULL )
		{
			pdscStyle = pTag ;
		}
		style.nFlags |= EComboListSprite::sfListFrame ;
		pInfBuf = (PEGL_IMAGE_INFO) bufImage.PutBuffer( nBufSize ) ;
		if ( GetStaticFrameStyle( style.fsFrame, *pdscStyle, pInfBuf ) )
		{
			return	eslErrGeneral ;
		}
		bufImage.Flush( nBufSize ) ;
	}
	//
	// スクロールスタイル
	//////////////////////////////////////////////////////////////////////////
	pTag = descStyle.GetContentTagAs( 0, L"list_scroll" ) ;
	if ( pTag != NULL )
	{
		pdscStyle = GetStyleAs( pTag->GetAttrString( L"style", NULL ) ) ;
		if ( pdscStyle == NULL )
		{
			pdscStyle = pTag ;
		}
		style.nFlags |= EComboListSprite::sfListScroll ;
		pInfBuf = (PEGL_IMAGE_INFO) bufImage.PutBuffer( nBufSize ) ;
		if ( GetScrollBarStyle( style.bsScroll, *pdscStyle, pInfBuf ) )
		{
			return	eslErrGeneral ;
		}
		bufImage.Flush( nBufSize ) ;
	}
	//
	return	eslErrSuccess ;
}

// 画像リソースを参照する
//////////////////////////////////////////////////////////////////////////////
EGL_IMAGE_RECT * EFormResourceManager::GetImageResource
	( const wchar_t * pwszRes,
		EGLAnimation *& pAnime, EGL_IMAGE_RECT & irectClip )
{
	//
	// 画像識別子取得
	//
	EStreamWideString	sws = pwszRes ;
	sws.DisregardSpace( ) ;
	EWideString	wstrID = sws.GetEnclosedString( L':' ) ;
	EGLAnimation *	pImage =
		ESLTypeCast<EGLAnimation>( GetResourceAs( wstrID ) ) ;
	//
	pAnime = NULL ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	if ( !sws.HasToComeToken( L"RECT" ) )
	{
		pAnime = pImage ;
		return	NULL ;
	}
	//
	// 矩形取得
	//
	int		radix ;
	if ( sws.HasToComeChar( L"(" ) != L'(' )
		return	NULL ;
	radix = sws.GetNumberRadix( ) ;
	if ( radix < 0 )
		return	NULL ;
	irectClip.x = sws.GetInteger( radix ) ;
	//
	if ( sws.HasToComeChar( L"," ) != L',' )
		return	NULL ;
	radix = sws.GetNumberRadix( ) ;
	if ( radix < 0 )
		return	NULL ;
	irectClip.y = sws.GetInteger( radix ) ;
	//
	if ( sws.HasToComeChar( L"," ) != L',' )
		return	NULL ;
	radix = sws.GetNumberRadix( ) ;
	if ( radix < 0 )
		return	NULL ;
	irectClip.w = sws.GetInteger( radix ) ;
	//
	if ( sws.HasToComeChar( L"," ) != L',' )
		return	NULL ;
	radix = sws.GetNumberRadix( ) ;
	if ( radix < 0 )
		return	NULL ;
	irectClip.h = sws.GetInteger( radix ) ;
	//
	if ( sws.HasToComeChar( L")" ) != L')' )
		return	NULL ;
	//
	pAnime = pImage ;
	return	&irectClip ;
}

// 静画像リソースを参照する
//////////////////////////////////////////////////////////////////////////////
PEGL_IMAGE_INFO EFormResourceManager::GetStillImageResource
	( const wchar_t * pwszRes, PEGL_IMAGE_INFO pImage )
{
	EGL_IMAGE_RECT	irectClip ;
	EGLAnimation *	pAnime ;
	EGL_IMAGE_RECT *	pClip =
		GetImageResource( pwszRes, pAnime, irectClip ) ;
	if ( pAnime == NULL )
	{
		return	NULL ;
	}
	if ( pClip == NULL )
	{
		return	*pAnime ;
	}
	if ( pAnime->GetInfo() == NULL )
	{
		return	NULL ;
	}
	if ( ::eglGetClippedImageInfo( pImage, *pAnime, pClip ) )
	{
		return	NULL ;
	}
	return	pImage ;
}
