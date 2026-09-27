
#include <cotopha.cc>

File	cin, cout ;
cin.Open( "", File::Mode::Read ) ;
cout.Open( "", File::Mode::Write ) ;


// Android.mk ファイル
String		sAndroidMkPath ;
String		sMkBeforeBeginSrc ;
String		sMkAfterEndSrc ;
String[]	aMkLocalSrcFiles ;


// エントリポイント
//////////////////////////////////////////////////////////////////////////////
int main( String[] arg )
{
	//
	// Android.mk を読み込み
	//
	sAndroidMkPath = arg[0] ;
	if ( sAndroidMkPath == "" )
	{
		cout += "Android.mk パスを入力してください" ;
		sAndroidMkPath = cin ;
		sAndroidMkPath.TrimRight() ;
		if ( sAndroidMkPath == "" )
		{
			return	1 ;
		}
		if ( sAndroidMkPath.
				ParseFileExtension().CompareNoCase( "mk" ) != 0 )
		{
			sAndroidMkPath = sAndroidMkPath.OffsetFilePath( "Android.mk" ) ;
		}
	}
	File	fileMk ;
	if ( fileMk.Open
		( sAndroidMkPath,
			File::Mode::Read | File::Mode::ShareRead ) != eslErrSuccess )
	{
		cout += sAndroidMkPath + " を開けませんでした" ;
		return	1 ;
	}
	int	nMkPos = 0 ;
	while ( !fileMk.IsEndOfFile() )
	{
		String	sLine = fileMk ;
		if ( sLine.CompareLeft( "#BEGIN_LOCAL_SRC_FILE" ) == 0 )
		{
			sMkBeforeBeginSrc += sLine + "\r\n" ;
			nMkPos = 1 ;
		}
		else if ( sLine.CompareLeft( "#END_LOCAL_SRC_FILE" ) == 0 )
		{
			sMkAfterEndSrc += sLine + "\r\n" ;
			nMkPos = 2 ;
		}
		else
		{
			if ( nMkPos == 0 )
			{
				sMkBeforeBeginSrc += sLine + "\r\n" ;
			}
			else if ( nMkPos == 1 )
			{
				sLine.TrimRight() ;
				if ( sLine != "" )
				{
					aMkLocalSrcFiles += sLine ;
				}
			}
			else if ( nMkPos == 2 )
			{
				sMkAfterEndSrc += sLine + "\r\n" ;
			}
		}
	}
	fileMk.Close() ;
	//
	// ファイルリスト更新
	//
	bool	fUpdateSrcFiles = false ;
	int		iSrcFile = 0 ;
	for ( int i = 1; i < sizeof(arg); i ++ )
	{
		String[]	aFiles ;
		File.FindFile( aFiles, arg[i] ) ;
		for ( src : aFiles )
		{
			String	sLine = "LOCAL_SRC_FILES += app/" + src.ParseFileName() ;
			if ( (iSrcFile >= sizeof(aMkLocalSrcFiles))
				|| (aMkLocalSrcFiles[iSrcFile] != sLine) )
			{
				fUpdateSrcFiles = true ;
			}
			aMkLocalSrcFiles[iSrcFile ++] = sLine ;
		}
	}
	if ( fUpdateSrcFiles )
	{
		//
		// Android.mk へ書き出し開始
		//
		if ( fileMk.Open
			( sAndroidMkPath, File::Mode::Create ) != eslErrSuccess )
		{
			cout += sAndroidMkPath + " へ書き出せません" ;
			return	1 ;
		}
		fileMk += sMkBeforeBeginSrc ;
		//
		for ( src : aMkLocalSrcFiles )
		{
			fileMk += src ;
		}
		//
		fileMk += "" ;
		fileMk += sMkAfterEndSrc ;
	}
	return	0 ;
}


