
#include <cotopha.cc>
#include <imagecontext.h>

File	cin, cout ;
cin.Open( "", File::Mode::Read ) ;
cout.Open( "", File::Mode::Write ) ;

int SaveIcon
	( String sPath, String sMIME,
		ImageContext& imgctx, int nWidth, int nHeight ) ;

int main( String[] arg )
{
	if ( (arg[0] == "") || (arg[1] == "") )
	{
		cout += "make_android_icons <image-file> <android-res-directory>" ;
		return	1 ;
	}
	ImageContext	imgctx ;
	if ( imgctx.load( arg[0] ) != eslErrSuccess )
	{
		cout += arg[0] + " ‚Ì“Ç‚Ýž‚Ý‚ÉŽ¸”s‚µ‚Ü‚µ‚½" ;
		return	1 ;
	}
	String	sDstDir = arg[1] ;
	int		nErrors = 0 ;
	if ( (imgctx.size.w >= 192) && (imgctx.size.h >= 192) )
	{
		nErrors += SaveIcon
			( sDstDir.OffsetFilePath
				( "drawable-xxxhdpi\\ic_launcher.png" ),
					"image/png", imgctx, 192, 192 ) ;
	}
	if ( (imgctx.size.w >= 144) && (imgctx.size.h >= 144) )
	{
		nErrors += SaveIcon
			( sDstDir.OffsetFilePath
				( "drawable-xxhdpi\\ic_launcher.png" ),
					"image/png", imgctx, 144, 144 ) ;
	}
	if ( (imgctx.size.w >= 96) && (imgctx.size.h >= 96) )
	{
		nErrors += SaveIcon
			( sDstDir.OffsetFilePath
				( "drawable-xhdpi\\ic_launcher.png" ),
					"image/png", imgctx, 96, 96 ) ;
	}
	nErrors += SaveIcon
		( sDstDir.OffsetFilePath
			( "drawable-hdpi\\ic_launcher.png" ),
				"image/png", imgctx, 72, 72 ) ;
	nErrors += SaveIcon
		( sDstDir.OffsetFilePath
			( "drawable-mdpi\\ic_launcher.png" ),
				"image/png", imgctx, 48, 48 ) ;
	nErrors += SaveIcon
		( sDstDir.OffsetFilePath
			( "drawable-ldpi\\ic_launcher.png" ),
				"image/png", imgctx, 36, 36 ) ;
	return	nErrors ;
}


int SaveIcon
	( String sPath, String sMIME,
		ImageContext& imgctx, int nWidth, int nHeight )
{
	ImageContext	ictxTemp ;
	ictxTemp.merge( imgctx ) ;
	//
	while ( (ictxTemp.size.w >= nWidth * 2)
			&& (ictxTemp.size.h >= nHeight * 2) )
	{
		ictxTemp.resize( ictxTemp.size.w / 2, ictxTemp.size.h / 2 ) ;
	}
	if ( (ictxTemp.size.w != nWidth) && (ictxTemp.size.h != nHeight) )
	{
		ictxTemp.resize( nWidth, nHeight ) ;
	}
	cout += sPath + "..." ;
	//
	File	fileTemp ;
	fileTemp.Open( sPath, File::Mode::Create ) ;
	fileTemp.Close() ;
	fileTemp.Rename( sPath, "" ) ;
	//
	if ( ictxTemp.save( sPath, sMIME ) != eslErrSuccess )
	{
		cout += "failed!" ;
		return	1 ;
	}
	return	0 ;
}



