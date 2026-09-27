
#include <cotopha.cc>
#include <objected/xml_document.cc>
#include "mkffiles.h"

File	cout ;
cout.Open( "", File::Mode::Write ) ;

Hash<XMLDocument>	m_mapFile ;


int main( String[] arg )
{
	MakeFragmentFilesApp	app ;
	app.m_strSrcDir = "src" ;
	app.m_strDstDir = "dst" ;
	app.m_strAssetsDir = app.m_strDstDir + "\\assets" ;
	app.m_strOnlinesDir = app.m_strDstDir + "\\onlines" ;
	app.m_strOnlinesURL = "http://server_ip/htdoc_path/" ;
	app.m_strDefPassword = "" ;

	for ( a : arg )
	{
		int	iEqu = a.Find( "=" ) ;
		if ( iEqu >= 0 )
		{
			String	sName = a.Left( iEqu ) ;
			String	sValue = a.Middle( iEqu + 1 ) ;
			if ( sName == "url" )
			{
				app.m_strOnlinesURL = sValue ;
			}
			else if ( sName == "pass" )
			{
				app.m_strDefPassword = sValue ;
			}
		}
	}

	XMLDocument	xmlFileList ;
	if ( xmlFileList.LoadDocument( "file_list.xml", xmlFileList ) == eslErrSuccess )
	{
		const XMLDocument&	xmlFiles = xmlFileList.GetElementTagAs( "files" ) ;
		if ( xmlFiles !== null )
		{
			for ( int i = 0; i < xmlFiles.GetElementsCount(); i ++ )
			{
				const XMLDocument&	xmlTag = xmlFiles.GetElementAt( i ) ;
				if ( (xmlTag === null)
					|| (xmlTag.GetTag() != "archive") )
				{
					continue ;
				}
				String	strFileName = xmlTag.GetAttrStringAs( "file" ) ;
				if ( strFileName != "" )
				{
					m_mapFile[strFileName] = xmlTag ;
				}
			}
		}
	}

	cout += "fragment files;" ;
	int	nErrors = app.FragmentFiles( "" ) ;
	if ( nErrors == 0 )
	{
		cout += "\narchive files;" ;
		nErrors = app.FragmentArchives
			( "archives", MakeFragmentFilesApp::Mode::AssetsOnly ) ;
		if ( nErrors == 0 )
		{
			cout += "\ncopy files;" ;
			nErrors = app.FragmentArchives
				( "copies", MakeFragmentFilesApp::Mode::CopyFromAssets ) ;
			if ( nErrors == 0 )
			{
				cout += "\nonline archive files;" ;
				nErrors = app.FragmentArchives
					( "onlines", MakeFragmentFilesApp::Mode::Download ) ;
				if ( nErrors == 0 )
				{
					cout += "" ;
					nErrors = app.SaveEnvTemplate() ;
				}
			}
		}
	}

	return	nErrors ;
}


// 書庫ファイルを順次処理する
//////////////////////////////////////////////////////////////////////////////
int MakeFragmentFilesApp::FragmentArchives
	( String sSrcDir, MakeFragmentFilesApp::Mode mode )
{
	sSrcDir = m_strSrcDir.OffsetFilePath( sSrcDir ) ;
	//
	String[]	aFiles ;
	File.FindFile( aFiles, sSrcDir.OffsetFilePath( "*.*" ) ) ;
	for ( src : aFiles )
	{
		if ( (mode == Mode::AssetsOnly)
			|| (mode == Mode::CopyFromAssets) )
		{
			cout += "  " + src + "..." ;
			//
			// assets 内に分割形式で収める
			//
			String	sDstFile =
				m_strAssetsDir.OffsetFilePath( "archives\\" + src ) ;
			//
			Setup	setup ;
			if ( setup.ExecuteProcess
				( "", "ffragment /nologo \""
					+ sSrcDir.OffsetFilePath( src )
					+ "\" \"" + sDstFile + "\"" ) != eslErrSuccess )
			{
				cout += "\nffragment の起動に失敗しました" ;
				return	1 ;
			}
			int	nExitCode ;
			setup.GetExecuteExitCode( INFINITE, nExitCode ) ;
			if ( nExitCode != 0 )
			{
				return	1 ;
			}
			//
			XMLDocument&	xmlArc = XMLDocument ;
			xmlArc.SetTag( "archive" ) ;
			if ( mode == Mode::AssetsOnly )
			{
				xmlArc.SetAttributeAs( "path", "assets://archives/" + src ) ;
				xmlArc.SetAttributeAs( "fragment", "true" ) ;
			}
			else
			{
				xmlArc.SetAttributeAs( "path", "sd://Android/data/$(APPID)/files/" + src ) ;
				xmlArc.SetAttributeAs( "display_name", src.ParseFileTitle() ) ;
				//
				File	srcfile ;
				if ( srcfile.Open
					( sSrcDir.OffsetFilePath( src ),
						File::Mode::Read | File::Mode::ShareRead ) != eslErrSuccess )
				{
					cout += "\n開けませんでした。" ;
					return	1 ;
				}
				XMLDocument&	xmlFile = XMLDocument ;
				xmlFile.SetTag( "file" ) ;
				xmlFile.SetAttributeAs( "path", "fragments://archives/" + src ) ;
				xmlFile.SetAttrIntegerAs( "size", srcfile.GetLength() ) ;
				xmlFile.SetAttributeAs
						( "crc", setup.CalcCRC32(srcfile).Format(16,8) ) ;
				xmlArc.AddElement( xmlFile ) ;
			}
			if ( m_strDefPassword != "" )
			{
				xmlArc.SetAttributeAs( "key", m_strDefPassword ) ;
			}
			xmlArc.SetAttributeAs( "id", src.ParseFileTitle() ) ;
			m_xmlEnv.AddElement( xmlArc ) ;
			//
			OverrideArchiveTag( xmlArc, src ) ;
		}
		else
		{
			//
			// HTTP 上にファイルを設置する
			//
			cout += "  " + src + "..." ;
			Setup	setup ;
			/*
			if ( setup.ExecuteProcess
				( "", "sigcrc /nologo \""
					+ sSrcDir.OffsetFilePath( src )
					+ "\" \"" + m_strOnlinesDir + "\"" ) != eslErrSuccess )
			{
				cout += "\nsigcrc の起動に失敗しました" ;
				return	1 ;
			}
			int	nExitCode ;
			setup.GetExecuteExitCode( INFINITE, nExitCode ) ;
			if ( nExitCode != 0 )
			{
				cout += "" ;
				return	1 ;
			}
			*/
			//
			File	srcfile ;
			if ( srcfile.Open
				( sSrcDir.OffsetFilePath( src ),
					File::Mode::Read | File::Mode::ShareRead ) != eslErrSuccess )
			{
				cout += "\n開けませんでした。" ;
				return	1 ;
			}
			File	dstfile ;
			if ( dstfile.Open
				( m_strOnlinesDir.OffsetFilePath( src ),
								File::Mode::Create ) != eslErrSuccess )
			{
				cout += "\n" + m_strOnlinesDir.OffsetFilePath( src )
												+ " を開けませんでした。" ;
				return	1 ;
			}
			dstfile.Write( srcfile, srcfile.GetLength() ) ;
			srcfile.Seek( 0 ) ;
			//
			XMLDocument&	xmlArc = XMLDocument ;
			xmlArc.SetTag( "archive" ) ;
			xmlArc.SetAttributeAs( "path", "sd://Android/data/$(APPID)/files/" + src ) ;
			xmlArc.SetAttributeAs( "display_name", src.ParseFileTitle() ) ;
			xmlArc.SetAttributeAs( "id", src.ParseFileTitle() ) ;
			/*
			xmlArc.SetAttributeAs( "updatable", "true" ) ;
			xmlArc.SetAttributeAs
				( "download", m_strOnlinesURL + src.ParseFileTitle() + ".xml" ) ;
			*/
			if ( m_strDefPassword != "" )
			{
				xmlArc.SetAttributeAs( "key", m_strDefPassword ) ;
			}
			m_xmlEnv.AddElement( xmlArc ) ;
			//
			XMLDocument&	xmlFile = XMLDocument ;
			xmlFile.SetTag( "file" ) ;
			xmlFile.SetAttributeAs( "path", m_strOnlinesURL + src ) ;
			xmlFile.SetAttrIntegerAs( "size", srcfile.GetLength() ) ;
			xmlFile.SetAttributeAs
					( "crc", setup.CalcCRC32(srcfile).Format(16,8) ) ;
			xmlArc.AddElement( xmlFile ) ;
			//
			OverrideArchiveTag( xmlArc, src ) ;
		}
	}
	return	0 ;
}

// ファイル関連属性を設定する
//////////////////////////////////////////////////////////////////////////////
void MakeFragmentFilesApp::OverrideArchiveTag( XMLDocument& xmlArc, String src )
{
	if ( !m_mapFile.IsEmpty( src ) )
	{
		XMLDocument&	xmlTemp = m_mapFile[src] ;
		for ( int i = 0; i < xmlTemp.GetAttributeCount(); i ++ )
		{
			String	strAttrName = xmlTemp.GetAttributeNameAt( i ) ;
			if ( strAttrName != "file" )
			{
				xmlArc.SetAttributeAs
					( strAttrName, xmlTemp.GetAttributeValueAt( i ) ) ;
			}
		}
	}
}

// ファイルを順次処理する
//////////////////////////////////////////////////////////////////////////////
int MakeFragmentFilesApp::FragmentFiles( String sOffsetDir )
{
	//
	// ファイルを順次処理
	//
	String[]	aFiles ;
	String		sSrcDir = m_strSrcDir.OffsetFilePath( "files\\" + sOffsetDir ) ;
	File.FindFile( aFiles, sSrcDir.OffsetFilePath( "*.*" ) ) ;
	for ( src : aFiles )
	{
		cout += "  " + sOffsetDir.OffsetFilePath( src ) + "..." ;
		//
		File	srcfile ;
		if ( srcfile.Open
			( sSrcDir.OffsetFilePath( src ),
				File::Mode::Read | File::Mode::ShareRead ) != eslErrSuccess )
		{
			cout += "\n開けませんでした。" ;
			return	1 ;
		}
		if ( srcfile.GetLength() < 1024 * 1024 )
		{
			File	dstfile ;
			String	sDstFile =
				m_strAssetsDir.OffsetFilePath
					( "files\\" + sOffsetDir ).OffsetFilePath( src ) ;
			if ( dstfile.Open( sDstFile, File::Mode::Create ) != eslErrSuccess )
			{
				cout += "\n" + sDstFile + " を開けませんでした。" ;
				return	1 ;
			}
			dstfile.Write( srcfile, srcfile.GetLength() ) ;
		}
		else
		{
			srcfile.Close() ;
			//
			String	sDstFile =
				m_strAssetsDir.OffsetFilePath
					( "fragments\\" + sOffsetDir ).OffsetFilePath( src ) ;
			//
			Setup	setup ;
			if ( setup.ExecuteProcess
				( "", "ffragment /nologo \""
					+ sSrcDir.OffsetFilePath( src )
					+ "\" \"" + sDstFile + "\"" ) != eslErrSuccess )
			{
				cout += "\nffragment の起動に失敗しました" ;
				return	1 ;
			}
			int	nExitCode ;
			setup.GetExecuteExitCode( INFINITE, nExitCode ) ;
			if ( nExitCode != 0 )
			{
				return	1 ;
			}
		}
	}

	//
	// サブディレクトリを順次処理
	//
	String[]	aDirs ;
	File.FindDirectory( aDirs, sSrcDir.OffsetFilePath( "*.*" ) ) ;
	for ( src : aDirs )
	{
		if ( (src != ".") && (src != "..") )
		{
			int	nErrors = FragmentFiles( sOffsetDir.OffsetFilePath( src ) ) ;
			if ( nErrors != 0 )
			{
				return	nErrors ;
			}
		}
	}

	return	0 ;
}

// 環境設定ファイルのテンプレートを書き出す
//////////////////////////////////////////////////////////////////////////////
int MakeFragmentFilesApp::SaveEnvTemplate( void )
{
	m_xmlEnv.SetTag( "script" ) ;
	//
	XMLDocument&	xmlFrag = XMLDocument ;
	xmlFrag.SetTag( "file" ) ;
	xmlFrag.SetAttributeAs( "path", "assets://fragments/" ) ;
	xmlFrag.SetAttributeAs( "fragment", "true" ) ;
	m_xmlEnv.AddElement( xmlFrag ) ;
	//
	XMLDocument&	xmlFiles = XMLDocument ;
	xmlFiles.SetTag( "file" ) ;
	xmlFiles.SetAttributeAs( "path", "assets://files/" ) ;
	m_xmlEnv.AddElement( xmlFiles ) ;
	//
	m_xmlEnv.SaveDocument( m_strAssetsDir.OffsetFilePath( "cotopha.xml" ) ) ;
	return	0 ;
}



