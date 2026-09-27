
#include <cotopha.cc>

File	cout ;
cout.Open( "", modeWrite ) ;


constant	ECS_DECLARE_EXPORT_NEW_OBJECT = "ECS_DECLARE_EXPORT_NEW_OBJECT" ;
constant	ECS_DECLARE_EXPORT_SYSCALL = "ECS_DECLARE_EXPORT_SYSCALL" ;
constant	ECS_LIB_DECLARE_EXPORT_NEW_OBJECT = "ECS_LIB_DECLARE_EXPORT_NEW_OBJECT" ;
constant	ECS_LIB_DECLARE_EXPORT_SYSCALL = "ECS_LIB_DECLARE_EXPORT_SYSCALL" ;

Hash<String>	g_mapNewObject ;
Hash<String>	g_mapSysCall ;


int main( String[]& arg )
{
	if ( (sizeof(arg) == 0) || (arg[0] == "") )
	{
		cout += "ソースディレクトリが指定されていません" ;
		return	1 ;
	}
	for ( int i = 0; i < sizeof(arg); i ++ )
	{
		String		sSrcDir = arg[i] ;
		String[]	aFiles ;
		File.FindFile( aFiles, sSrcDir.OffsetFilePath( "*.*" ) ) ;

		for ( name : aFiles : i )
		{
			String	sSrcFile = sSrcDir.OffsetFilePath( name ) ;
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
				if ( (sToken == ECS_DECLARE_EXPORT_NEW_OBJECT)
					|| (sToken == ECS_DECLARE_EXPORT_SYSCALL)
					|| (sToken == ECS_LIB_DECLARE_EXPORT_NEW_OBJECT)
					|| (sToken == ECS_LIB_DECLARE_EXPORT_SYSCALL) )
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
					}
					else
					{
						String	sSymbol = sLine.NextToken() ;
						if ( (sToken == ECS_DECLARE_EXPORT_NEW_OBJECT)
							|| (sToken == ECS_LIB_DECLARE_EXPORT_NEW_OBJECT) )
						{
							g_mapNewObject[sSymbol] =
									"\tENTRY_NEW_OBJECT(" + sSymbol + ")" ;
						}
						else
						{
							g_mapSysCall[sSymbol] =
									"\tENTRY_SYS_CALL(" + sSymbol + ")" ;
						}
					}
				}
			}
		}
	}

	File	fileDst ;
	if ( fileDst.Open( "glscs_sakura2_new_object.h", modeCreate ) == eslErrSuccess )
	{
		for ( e : g_mapNewObject )
		{
			fileDst += e + "," ;
		}
	}
	else
	{
		cout += "glscs_sakura2_new_object.h を開けませんでした" ;
	}
	fileDst.Close() ;

	if ( fileDst.Open( "glscs_sakura2_sys_call.h", modeCreate ) == eslErrSuccess )
	{
		for ( e : g_mapSysCall )
		{
			fileDst += e + "," ;
		}
	}
	else
	{
		cout += "glscs_sakura2_sys_call.h を開けませんでした" ;
	}
	fileDst.Close() ;

	return	0 ;
}


