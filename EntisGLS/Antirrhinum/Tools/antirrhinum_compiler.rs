
import "antirrhinum.rs" ;


int main( String[] arg )
{
	AGLCompiler	compiler = new AGLCompiler() ;

	String[]	aSrcFiles = new String[] ;
	String		strDstDir = null ;
	String		strDstFile = null ;
	String[]	aDefFiles = new String[] ;
	String		strMacroFile = null ;
	boolean		flagCmpDate = false ;

	String	strOpt = null ;
	for ( int i = 0; i < arg.length(); i ++ )
	{
		if ( arg[i].charCodeAt(0) == '/' )
		{
			if ( arg[i] == "/?" )
			{
				String	strHelp =
					"options;\n" +
					"	[/src] <file>  : ソースファイル（複数可）\n" +
					"	/dst <file>    : 出力ファイル（省略可）\n" +
					"	/dst_dir <dir> : 出力ディレクトリ\n" +
					"	/def <file>    : 書式定義ファイル（複数可）\n" +
					"	/macro <file>  : マクロ定義ファイル\n" +
					"	/cfg <file>    : 設定ファイル\n" +
					"	/cmp_date      : ファイル日付比較する\n" ;
				strOpt = null ;
				System.console().printf( "%s\n", strHelp ) ;
				return	0 ;
			}
			else if ( arg[i] == "/cmp_date" )
			{
				flagCmpDate = true ;
			}
			else
			{
				strOpt = arg[i] ;
			}
			continue ;
		}
		if ( (strOpt == null) || (strOpt == "/src") )
		{
			aSrcFiles.add( arg[i] ) ;
		}
		else if ( strOpt == "/dst" )
		{
			strDstFile = arg[i] ;
		}
		else if ( strOpt == "/dst_dir" )
		{
			strDstDir = arg[i] ;
		}
		else if ( strOpt == "/def" )
		{
			aDefFiles.add( arg[i] ) ;
		}
		else if ( strOpt == "/macro" )
		{
			strMacroFile = arg[i] ;
		}
		else if ( strOpt == "/cfg" )
		{
			compiler.loadConfigFile( arg[i] ) ;
		}
		else
		{
			System.console().printf( "%s は処理出来ない引数です\n", strOpt ) ;
			return	1 ;
		}
		strOpt = null ;
	}

	if ( aSrcFiles.length() == 0 )
	{
		System.console().printf( "ソースファイルが指定されていません\n" ) ;
		return	1 ;
	}
	if ( (aSrcFiles.length() >= 2) && (strDstFile != null) )
	{
		System.console().printf
			( "ソースファイルが複数指定されているときに、"
				+ "出力ファイルの指定は無効です\n" ) ;
		return	1 ;
	}
	for ( int i = 0; i < aDefFiles.length(); i ++ )
	{
		compiler.loadUsageDefFile( aDefFiles[i] ) ;
	}

	int	nErrors = 0 ;
	if ( strMacroFile != null )
	{
		compiler.initialize() ;
		nErrors += compiler.loadSourceFile( strMacroFile, true ) ;
	}
	for ( int i = 0; i < aSrcFiles.length(); i ++ )
	{
		String	strSrcFile = aSrcFiles[i] ;
		if ( strDstFile == null )
		{
			strDstFile = strSrcFile.getFileTitlePart() + ".xmlagl" ;
			if ( strDstDir != null )
			{
				strDstFile = strDstDir.offsetFilePath( strDstFile ) ;
			}
			else
			{
				strDstFile = strSrcFile.getFileDirectoryPart().
										offsetFilePath( strDstFile ) ;
			}
		}
		if ( flagCmpDate )
		{
			File	fileSrc = new File( strSrcFile ) ;
			File	fileDst = new File( strDstFile ) ;
			if ( fileSrc.exists() && fileDst.exists()
				&& (fileSrc.lastModified() <= fileDst.lastModified()) )
			{
				continue ;
			}
		}
		nErrors += compiler.run( strSrcFile, strDstFile ) ;
		//
		strDstFile = null ;
	}
	compiler.release() ;
	return	nErrors ;
}


