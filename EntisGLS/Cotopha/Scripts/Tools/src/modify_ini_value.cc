
#include <cotopha.cc>

File	cin, cout ;
cin.Open( "", File::Mode::Read ) ;
cout.Open( "", File::Mode::Write ) ;


String					sIniFilePath ;
String[]				aIniLines ;
Hash< Hash<String> >	mapValues ;


// エントリポイント
//////////////////////////////////////////////////////////////////////////////
int main( String[] arg )
{
	//
	// ini を読み込み
	//
	sIniFilePath = arg[0] ;
	if ( sIniFilePath == "" )
	{
		cout += "modify_ini_value <ini-file> <var-list>" ;
		cout += "var-list: <var-item> ..." ;
		cout += "var-item: { '[' <section> ']' | <var-name> '=' <value> }" ;
		return	1 ;
	}
	//
	// 置き換え変数を解釈
	//
	String	sCurSec = "" ;
	for ( int i = 1; i < sizeof(arg); i ++ )
	{
		String		sVarItem = arg[i] ;
		String[]	aParam ;
		if ( sVarItem.IsMatchUsage( "(%s)\\=(%s)\\", aParam ) == "" )
		{
			mapValues[sCurSec][aParam[0]] = aParam[1] ;
		}
		else if ( sVarItem.IsMatchUsage( "\\[(%s)\\]\\", aParam ) == "" )
		{
			sCurSec = aParam[0] ;
		}
		else
		{
			cout += "\'" + sVarItem + "\' は不正な構文です" ;
		}
	}
	//
	// ini ファイルを読み込み
	//
	File	fileIni ;
	if ( fileIni.Open
		( sIniFilePath,
			File::Mode::Read | File::Mode::ShareRead ) != eslErrSuccess )
	{
		cout += sIniFilePath + " を開けませんでした" ;
		return	1 ;
	}
	sCurSec = "" ;
	while ( !fileIni.IsEndOfFile() )
	{
		String		sLine = fileIni ;
		String[]	aParam ;
		if ( sLine.IsMatchUsage( " \\#", aParam ) == "" )
		{
		}
		else if ( sLine.IsMatchUsage( " (%s)\\=(%s)\\", aParam ) == "" )
		{
			if ( !mapValues[sCurSec].IsEmpty( aParam[0] ) )
			{
				sLine = aParam[0] + "=" + mapValues[sCurSec][aParam[0]] ;
			}
		}
		else if ( sLine.IsMatchUsage( " \\[(%s)\\]", aParam ) == "" )
		{
			sCurSec = aParam[0] ;
		}
		else
		{
		}
		aIniLines += sLine ;
	}
	fileIni.Close() ;
	//
	// ini ファイルを書き出し
	//
	if ( fileIni.Open
		( sIniFilePath, File::Mode::Create ) != eslErrSuccess )
	{
		cout += sIniFilePath + " へ書き出せませんでした" ;
		return	1 ;
	}
	for ( ini : aIniLines )
	{
		fileIni += ini ;
	}
	//
	return	0 ;
}


