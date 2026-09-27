
#include <cotopha.cc>

File	cout, cin ;

cout.Open( "", File::Mode::Write ) ;
cin.Open( "", File::Mode::Read ) ;


int main( String[] arg )
{
	String	sMovieFile ;
	if ( sizeof(arg) == 0 )
	{
		cout += "再生する動画ファイル:" ;
		sMovieFile = cin ;
	}
	else
	{
		sMovieFile = arg[0] ;
	}
	if ( sMovieFile.ParseFileExtension() == "" )
	{
		sMovieFile += ".mei" ;
	}

	MovieSprite	movie ;
	if ( movie.OpenMovie( sMovieFile ) != eslErrSuccess )
	{
		cout += sMovieFile + " ファイルを開けませんでした。" ;
		return	1 ;
	}

	ImageInfo&	imginf = movie.GetImageInfo() ;
	if ( imginf === null )
	{
		cout += "画像情報を取得できませんでした。" ;
		return	1 ;
	}

	Window		screen ;
	InputFilter	input ;
	screen.CreateDisplay
		( sMovieFile, Window::Copperation::Window,
				imginf.nImageWidth, imginf.nImageHeight ) ;
	screen.SetOptionalFuncFlag
		( screen.GetOptionalFuncFlag()
			| Window::Flag::AllowClose | Window::Flag::AllowMinimize
			| Window::Flag::VariableWindowSize | Window::Flag::AllowMaximize ) ;
	screen.EnableCommandQueue( true ) ;
	input.OpenFilter( InputFilter::Type::Above, screen ) ;
	input.FlushInputQueue( 0x10 ) ;

	double	rVolume = 1.0 ;
	bool	flagPaused = false ;
	int		nTotalFrame = movie.GetTotalFrame() ;
	int		nTotalTime = movie.GetTotalTime() ;
	int		nFramePerSec = 1 ;
	if ( nTotalTime > 0 )
	{
		nFramePerSec = nTotalFrame * 1000 / nTotalTime ;
	}

	screen.Lock() ;
	screen.AddSprite( 0, movie ) ;
	movie.SetVisible( true ) ;
	movie.PlayMovie
		( MovieSprite::PlayFlag::DirectDraw, Resource::PlayType::Music ) ;
	screen.Unlock() ;

	WndSpriteCmd	wscmd ;
	for ( ; ; )
	{
		if ( !screen.GetCommand( wscmd, 33 ) )
		{
			break	if ( wscmd.strID == "ID_APP_EXIT" ) ;
		}
		InputEvent	ev ;
		if ( input.GetInputEvent( ev, 0 ) == eslErrSuccess )
		{
			bool	fSeekFrame = false ;
			int		nSeekFrame = 0 ;
			if ( ev.idType == idKeyboard )
			{
				break	if ( ev.iKeyNum == VK_ESCAPE ) ;
				if ( ev.iKeyNum == VK_SPACE )
				{
					screen.Lock() ;
					if ( flagPaused )
					{
						movie.PlayMovie
							( MovieSprite::PlayFlag::DirectDraw,
										Resource::PlayType::Music ) ;
						flagPaused = false ;
						cout += "START" ;
					}
					else
					{
						movie.StopMovie() ;
						flagPaused = true ;
						cout += "PAUSE" ;
					}
					screen.Unlock() ;
				}
				else if ( ev.iKeyNum == VK_HOME )
				{
					screen.Lock() ;
					movie.StopMovie() ;
					movie.SeekFrame( 0 ) ;
					flagPaused = true ;
					cout += "SEEK HOME" ;
					screen.Unlock() ;
				}
				else if ( ev.iKeyNum == VK_LEFT )
				{
					nSeekFrame = movie.GetCurrentFrame() - nFramePerSec ;
					fSeekFrame = true ;
				}
				else if ( ev.iKeyNum == VK_RIGHT )
				{
					nSeekFrame = movie.GetCurrentFrame() + nFramePerSec ;
					fSeekFrame = true ;
				}
				else if ( ev.iKeyNum == VK_PRIOR )
				{
					nSeekFrame = movie.GetCurrentFrame() - nFramePerSec * 10 ;
					fSeekFrame = true ;
				}
				else if ( ev.iKeyNum == VK_NEXT )
				{
					nSeekFrame = movie.GetCurrentFrame() + nFramePerSec * 10 ;
					fSeekFrame = true ;
				}
				else if ( ev.iKeyNum == VK_UP )
				{
					rVolume += 0.125 ;
					if ( rVolume >= 1.0 )
					{
						rVolume = 1.0 ;
					}
					movie.SetVolume( rVolume, rVolume ) ;
					cout += "VOLUME " + String(rVolume) ;
				}
				else if ( ev.iKeyNum == VK_DOWN )
				{
					rVolume -= 0.125 ;
					if ( rVolume <= 0.0 )
					{
						rVolume = 0.0 ;
					}
					movie.SetVolume( rVolume, rVolume ) ;
					cout += "VOLUME " + String(rVolume) ;
				}
			}
			else if ( ev.idType == idMouse )
			{
				if ( ev.iKeyNum == VK_LBUTTON )
				{
					screen.Lock() ;
					if ( flagPaused )
					{
						movie.PlayMovie
							( MovieSprite::PlayFlag::DirectDraw,
										Resource::PlayType::Music ) ;
						flagPaused = false ;
						cout += "START" ;
					}
					else
					{
						movie.StopMovie() ;
						flagPaused = true ;
						cout += "PAUSE" ;
					}
					screen.Unlock() ;
				}
			}
			if ( fSeekFrame )
			{
				if ( nSeekFrame < 0 )
				{
					nSeekFrame = 0 ;
				}
				else if ( nSeekFrame >= nTotalFrame )
				{
					nSeekFrame = nTotalFrame ;
				}
				screen.Lock() ;
				movie.StopMovie() ;
				movie.SeekFrame( nSeekFrame ) ;
				cout += "SEEK " + String(nSeekFrame) ;
				flagPaused = true ;
				screen.Unlock() ;
			}
		}
		if ( !flagPaused )
		{
			if ( !movie.IsMoviePlaying() )
			{
				screen.Lock() ;
				movie.StopMovie() ;
				movie.SeekFrame( 0 ) ;
				movie.PlayMovie
					( MovieSprite::PlayFlag::DirectDraw,
								Resource::PlayType::Music ) ;
				cout += "LOOP" ;
				flagPaused = false ;
				screen.Unlock() ;
			}
		}
	}

	screen.Lock() ;
	movie.CloseMovie() ;
	screen.DetachSprite( movie ) ;
	screen.Unlock() ;

	screen.CloseDisplay() ;
	return	0 ;
}


