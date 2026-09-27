
class	MakeFragmentFilesApp
{
public:
	enum	Mode<int>
	{
		AssetsOnly,
		CopyFromAssets,
		Download,
	} ;
	String	m_strSrcDir ;
	String	m_strDstDir ;
	String	m_strAssetsDir ;
	String	m_strOnlinesDir ;
	String	m_strOnlinesURL ;
	String	m_strDefPassword ;

	XMLDocument	m_xmlEnv ;

public:
	// 書庫ファイルを順次処理する
	int FragmentArchives( String sSrcDir, Mode mode ) ;
	// ファイル関連属性を設定する
	void OverrideArchiveTag( XMLDocument& xmlArc, String src ) ;
	// ファイルを順次処理する
	int FragmentFiles( String sOffsetDir ) ;
	// 環境設定ファイルのテンプレートを書き出す
	int SaveEnvTemplate( void ) ;

} ;




