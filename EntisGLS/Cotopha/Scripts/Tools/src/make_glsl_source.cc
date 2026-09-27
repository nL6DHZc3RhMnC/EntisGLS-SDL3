
#include <cotopha.cc>

File	cout ;
cout.Open( "", File::Mode::Write ) ;

void DumpCSourceBinaryFile( File& fileDst, File& fileSrc ) ;


int main( String[] arg )
{
	String	sDstHFile = arg[0] ;
	String	sDstCppFiles = arg[1] ;
	String	sSrcFiles = arg[2] ;
	if ( sDstHFile == "" )
	{
		cout += "出力 .h ファイルが指定されていません" ;
		return	1 ;
	}
	if ( sDstCppFiles == "" )
	{
		cout += "出力 .cpp ファイルが指定されていません" ;
		return	1 ;
	}
	if ( sSrcFiles == "" )
	{
		cout += "入力ファイルが指定されていません" ;
		return	1 ;
	}
	File	fileHDst ;
	if ( fileHDst.Open
		( sDstHFile, File::Mode::Create ) != eslErrSuccess )
	{
		cout += sDstHFile + " を開けませんでした" ;
		return	1 ;
	}
	fileHDst += "namespace SakuraGL" ;
	fileHDst += "{" ;
	//
	File	fileCppDst ;
	if ( fileCppDst.Open
		( sDstCppFiles, File::Mode::Create ) != eslErrSuccess )
	{
		cout += sDstCppFiles + " を開けませんでした" ;
		return	1 ;
	}
	fileCppDst += "" ;
	//
	String		sSrcDir = sSrcFiles.ParseFileDirectory() ;
	String[]	aFiles ;
	File.FindFile( aFiles, sSrcFiles ) ;
	for ( src : aFiles : i )
	{
		cout += src + "..." ;
		//
		String	sFileName = src ;
		if ( (sFileName.ParseFileExtension().CompareNoCase( "bin" ) == 0)
			&& (sFileName.GetLength() - 4 > 0) )
		{
			sFileName = sFileName.Left( sFileName.GetLength() - 4 ) ;
		}
		for ( int j = 0; j < sizeof(sFileName); j ++ )
		{
			String	c = sFileName.Middle( j, 1 ) ;
			if ( !((c >= "A") && (c <= "Z")
				|| (c >= "a") && (c <= "z")
				|| (c >= "0") && (c <= "9")) )
			{
				sFileName.SetChar( j, "_" ) ;
			}
		}
		File	fileSrc ;
		if ( fileSrc.Open
			( sSrcDir.OffsetFilePath( src ),
				File::Mode::Read | File::Mode::ShareRead ) != eslErrSuccess )
		{
			cout += src + " を開けませんでした" ;
			continue ;
		}
		int	nLength = fileSrc.GetLength() ;
		//
		fileHDst += "	extern const uint8_t	g_" + sFileName
										+ "[" + String(nLength) + "] ;" ;
		fileCppDst += "const uint8_t	SakuraGL::g_"
							+ sFileName + "[" + String(nLength) + "] =" ;
		fileCppDst += "{" ;
		DumpCSourceBinaryFile( fileCppDst, fileSrc ) ;
		fileCppDst += "} ;" ;
		fileCppDst += "" ;
	}
	fileHDst += "}" ;
	return	0 ;
}


void DumpCSourceBinaryFile( File& fileDst, File& fileSrc )
{
	String	sHex = "" ;
	int		iDump = 0 ;
	for ( ; ; )
	{
		int	b ;
		if ( fileSrc.Read( &b, 1 ) == 0 )
		{
			break ;
		}
		sHex += "0x" + b.Format(16,2) + ", " ;
		if ( ++ iDump >= 8 )
		{
			fileDst += "\t" + sHex ;
			sHex = "" ;
			iDump = 0 ;
		}
	}
	if ( iDump > 0 )
	{
		fileDst += "\t" + sHex ;
	}
}



