
#include <cotopha.cc>

File	cout ;
cout.Open( "", modeWrite ) ;


constant	SGL_IMPLEMENT_CLASS_INFO = "SGL_IMPLEMENT_CLASS_INFO" ;
constant	SGL_IMPLEMENT_CLASS_INFO2 = "SGL_IMPLEMENT_CLASS_INFO2" ;
constant	SGL_IMPLEMENT_CLASS_INFO3 = "SGL_IMPLEMENT_CLASS_INFO3" ;
constant	ESL_IMPLEMENT_CLASS_NEW_OBJECT = "ESL_IMPLEMENT_CLASS_NEW_OBJECT" ;

Hash<String>	g_mapIncludeFiles ;
Hash<String>	g_mapNewObject ;
int				g_errors = 0 ;

int dumpSubDirectory( String sBaseDir, String sOffsetDir ) ;

int main( String[]& arg )
{
	if ( (sizeof(arg) == 0) || (arg[0] == "") )
	{
		cout += "ソースディレクトリが指定されていません" ;
		return	1 ;
	}

	for ( int i = 0; i < sizeof(arg); i ++ )
	{
		dumpSubDirectory( arg[i], "" ) ;
	}

	File	fileDst ;
/*
	if ( fileDst.Open( "sglx_new_object_classes.h", modeCreate ) == eslErrSuccess )
	{
		for ( e : g_mapIncludeFiles )
		{
			fileDst += e ;
		}
	}
	else
	{
		cout += "sglx_new_object_classes.h を開けませんでした" ;
	}
	fileDst.Close() ;
*/
	if ( fileDst.Open( "sglx_new_object_entries.h", modeCreate ) == eslErrSuccess )
	{
		for ( e : g_mapNewObject )
		{
			fileDst += e ;
		}
	}
	else
	{
		cout += "sglx_new_object_entries.h を開けませんでした" ;
	}
	fileDst.Close() ;

	return	g_errors ;
}


int dumpSubDirectory( String sBaseDir, String sOffsetDir )
{
	String[]	aFiles ;
	File.FindFile
		( aFiles,
			sBaseDir.OffsetFilePath(sOffsetDir).OffsetFilePath("*.*") ) ;
	for ( name : aFiles : i )
	{
		String	sSrcOffsetPath = sOffsetDir.OffsetFilePath( name ) ;
		String	sSrcFile = sBaseDir.OffsetFilePath( sSrcOffsetPath ) ;
		cout += sSrcFile + "..." ;
		//
		File	fileSrc ;
		if ( fileSrc.Open( sSrcFile, modeRead | shareRead ) != eslErrSuccess )
		{
			cout += "ファイルを開けませんでした" ;
			continue ;
		}
		String	strSource ;
		int		nLineNum = 0 ;
		while ( !fileSrc.IsEndOfFile() )
		{
			String	sLine = fileSrc ;
			nLineNum ++ ;
			//
			String	sToken = sLine.NextToken() ;
			if ( (sToken == SGL_IMPLEMENT_CLASS_INFO)
				|| (sToken == SGL_IMPLEMENT_CLASS_INFO2)
				|| (sToken == SGL_IMPLEMENT_CLASS_INFO3)
				|| (sToken == ESL_IMPLEMENT_CLASS_NEW_OBJECT) )
			{
				bool	fError = true ;
				for ( ; ; )
				{
					if ( sLine.NextNeedChar( "(" ) == "(" )
					{
						fError = false ;
						break ;
					}
					if ( sLine.GetIndex() < sLine.GetLength() )
					{
						break ;
					}
					if ( fileSrc.IsEndOfFile() )
					{
						break ;
					}
					sLine = fileSrc ;
					nLineNum ++ ;
				}
				if ( fError )
				{
					cout += "(" + String(nLineNum)
								+ "): \'(\' が見つかりません" ;
					g_errors ++ ;
				}
				else
				{
					String	sSymbol = sLine.NextToken() ;
					for ( ; ; )
					{
						sToken = sLine.NextToken() ;
						if ( sToken == "::" )
						{
							sSymbol += "::" + sLine.NextToken() ;
						}
						else if ( sToken == "," )
						{
							break ;
						}
						else if ( sToken == ")" )
						{
							break ;
						}
						else
						{
							fError = true ;
							break ;
						}
					}
					if ( fError )
					{
						cout += "(" + String(nLineNum)
									+ "): クラス名の指定が不正です" ;
						g_errors ++ ;
					}
					else
					{
						String	sSrcSlashPath = sSrcOffsetPath ;
						sSrcSlashPath.Replace( "\\", "/" ) ;
						g_mapIncludeFiles[sSrcOffsetPath] =
							"#include <" + sSrcSlashPath + ">" ;
						g_mapNewObject[sSymbol] =
							"\t{ L\"" + sSymbol.GetCEncoded()
								+ "\", (ulong_ptr_t) &" + sSymbol + "::NewESLObject }," ;
					}
				}
			}
		}
	}
	String[]	aDirs ;
	File.FindDirectory
		( aDirs,
			sBaseDir.OffsetFilePath(sOffsetDir).OffsetFilePath("*.*") ) ;
	for ( name : aDirs : i )
	{
		if ( (name != ".") && (name != "..") )
		{
			dumpSubDirectory( sBaseDir, sOffsetDir.OffsetFilePath(name) ) ;
		}
	}
	return	g_errors ;
}


