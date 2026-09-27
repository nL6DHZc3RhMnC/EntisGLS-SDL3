
#include <cotopha.cc>
#include <objected/xml_document.cc>
#include <psd_utilities.cc>

constant	MODE_EACH_LAYER = 0 ;
constant	MODE_RICH_LAYER = 1 ;
constant	MODE_SKIN_LAYER = 2 ;

int		modeConvertor = MODE_EACH_LAYER ;
String	sSrcFiles = "src\\*.*" ;
String	sDstDir = "dst" ;
String	sMIME = "image/x-erina" ;
int		nQuality = -1 ;
String	sConjunction = "_" ;
int		alignCut = 1 ;
int		marginCut = 0 ;
bool	keepHotSpot = true ;

int main( String[] arglist )
{
	//
	// 引数解釈
	//
	for ( arg : arglist : i )
	{
		int	iEqu = arg.Find( "=" ) ;
		if ( iEqu >= 0 )
		{
			String	sName = arg.Left( iEqu ) ;
			String	sValue = arg.Middle( iEqu + 1 ) ;
			if ( sName == "mode" )
			{
				if ( sValue == "each_layer" )
				{
					modeConvertor = MODE_EACH_LAYER ;
					keepHotSpot = true ;
				}
				else if ( sValue == "rich_layer" )
				{
					modeConvertor = MODE_RICH_LAYER ;
					keepHotSpot = true ;
				}
				else if ( sValue == "skin_layer" )
				{
					modeConvertor = MODE_SKIN_LAYER ;
					keepHotSpot = false ;
				}
				else
				{
					print( "mode 引数 \'", sValue, "\' は不正です\n" ) ;
				}
			}
			else if ( sName == "src" )
			{
				sSrcFiles = sValue ;
			}
			else if ( sName == "dst" )
			{
				sDstDir = sValue ;
			}
			else if ( sName == "mime" )
			{
				sMIME = sValue ;
			}
			else if ( sName == "quality" )
			{
				nQuality = int( sValue ) ;
			}
			else if ( sName == "conjunction" )
			{
				sConjunction = sValue ;
			}
			else if ( sName == "keep_pos" )
			{
				keepHotSpot = (sValue == "true") ;
			}
			else if ( sName == "align" )
			{
				alignCut = int( sValue ) ;
			}
			else if ( sName == "margin" )
			{
				marginCut = int( sValue ) ;
			}
			else
			{
				print( "引数 \'", arg, "\' は不正です\n" ) ;
				return	1 ;
			}
		}
		else
		{
			print( "引数 \'", arg, "\' は不正です\n" ) ;
			return	1 ;
		}
	}

	//
	// ファイル列挙
	//
	String[]	aFiles ;
	String		sSrcDir = sSrcFiles.ParseFileDirectory() ;
	File.FindFile( aFiles, sSrcFiles ) ;

	for ( filename : aFiles )
	{
		//
		// ファイル読み込み
		//
		ImageContext	imgctx ;
		print( filename, "..." ) ;
		if ( imgctx.load
			( sSrcDir.OffsetFilePath( filename ) ) == eslErrSuccess )
		{
			print( "\n" ) ;
			//
			// ファイル変換
			//
			if ( modeConvertor == MODE_EACH_LAYER )
			{
				SaveImageEachLayer
					( imgctx, sDstDir, sMIME, nQuality,
						sConjunction, keepHotSpot, alignCut, marginCut ) ;
			}
			else if ( modeConvertor == MODE_RICH_LAYER )
			{
				SaveAnimationRichLayer
					( imgctx, sDstDir, sMIME, nQuality,
								keepHotSpot, alignCut, marginCut ) ;
			}
			else if ( modeConvertor == MODE_SKIN_LAYER )
			{
				SkinContext	skin ;
				String		sFileName = filename.ParseFileTitle() ;
				String		sSystemInf =
						sDstDir.OffsetFilePath( sFileName + "\\system.inf" ) ;
				File.Open( sSystemInf, File::Mode::Create ) ;
				//
				int	nError =
					skin.MakeSkin
						( imgctx, sDstDir.OffsetFilePath(sFileName),
								sMIME, keepHotSpot, alignCut, marginCut, 0 ) ;
				if ( nError == 0 )
				{
					skin.SaveSkinDescription( sSystemInf ) ;
				}
			}
		}
		else
		{
			print( "開けませんでした\n" ) ;
		}
	}
	return	0 ;
}


