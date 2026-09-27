

int main( String[] arg )
{
	//
	// ソースファイル名取得
	//
	String	sMovieFile ;
	if ( arg.length() == 0 )
	{
		sMovieFile = new String() ;
		if ( Dialog.browseOpenFile
			( sMovieFile, "動画ファイル", null,
				["movie files", "avi;mpg;wmv;mei"] ) != Dialog.msgboxResultOk )
		{
			return	1 ;
		}
	}
	else
	{
		sMovieFile = arg[0] ;
	}
	//
	// ファイルを開く
	//
	MovieSprite	movie = new MovieSprite() ;
	if ( !movie.openMovieFile( sMovieFile ) )
	{
		Dialog.messageBox( sMovieFile + " を開けませんでした", "エラー" ) ;
		return	1 ;
	}
	Size	sizeMovie = movie.getMovieSize() ;
	//
	// ウィンドウ作成
	//
	WindowSprite	window = new WindowSprite() ;
	VirtualInput 	input = new VirtualInput() ;
	window.setOptionalFlags
		( window.getOptionalFlags()
			| WindowSprite.flagAllowClose
			| WindowSprite.flagAllowMinimize
			| WindowSprite.flagAllowMinimize
			| WindowSprite.flagVariableWindowSize ) ;
	window.createDisplay
		( sMovieFile, WindowSprite.modeWindow , sizeMovie.w, sizeMovie.h ) ;
	input.attachPostListenerToWindow( window ) ;
	//
	// 再生開始
	//
	window.addChild( movie ) ;
	movie.setVolume( (float[]) [ 0.3, 0.3 ], 2 ) ;
	movie.playMovie() ;
	//
	while ( movie.isMoviePlaying() )
	{
		VirtualInput.Command	cmd = new VirtualInput.Command() ;
		if ( input.getCommand( cmd ) )
		{
			if ( cmd.strID == VirtualInput.Command.AppExit )
			{
				break ;
			}
		}
		Thread.sleep( 10 ) ;
	}
	//
	// 終了
	//
	window.detachChild( movie ) ;
	movie.closeMovieFile() ;
	//
	input.detachPostListenerToWindow( window ) ;
	window.closeDisplay() ;

	return	1 ;
}

