

import "xml_document.rs" ;


//////////////////////////////////////////////////////////////////////////////
// Antirrhinum スクリプト・コンパイラー
//////////////////////////////////////////////////////////////////////////////

class	AGLCompiler
{
	// リスナ
	class	Listener
	{
		public abstract void initialize( AGLCompiler compiler, XMLDocument xmlTag ) ;
		public abstract void onBeginCompile( AGLCompiler compiler, String strScriptFile ) ;
		public abstract void onBlankLine( AGLCompiler compiler ) ;
		public abstract boolean isExtendCommand( AGLCompiler compiler, StringParser line ) ;
		public abstract void beforeAddCommand( AGLCompiler compiler, XMLDocument cmd ) ;
		public abstract void onEndCompile( AGLCompiler compiler ) ;
	}

	// コマンド引数書式
	public static const int	ARG_TYPE_ARRAY			= 0 ;	// 配列タイプ引数
	public static const int	ARG_TYPE_NAMED_PARAM	= 1 ;	// 名前付き配列タイプ引数
	public static const int	ARG_TYPE_USAGE			= 2 ;	// 正規表現書式

	// コマンド書式フラグ
	public static const int	ARG_FLAG_VAR_NAMED	= 0x0001 ;	// 自由記述名前付き配列

	// コマンド・プロトタイプ
	class	CmdPrototype
	{
		public String	m_cmd ;
		public boolean	m_vararg = false ;
		public String[]	m_args ;
		public String[]	m_params ;
		public String[]	m_values ;
	} ;

	// コマンド変換ディスクリプタ
	class	CmdDesc
	{
		public String								m_cmd ;
		public int									m_argType ;
		public int									m_argFlags ;
		public UsageMatcher							m_argUsage ;
		public String[]								m_argNames ;
		public String[]								m_argDefault ;
		public AGLCompiler.CmdPrototype[]			m_argPrototype ;
		public Function<void,AGLCompiler,HashMap>	m_argProc ;

		public CmdDesc
			( String cmd, int argType, int argFlags,
				String argUsage, String[] argNames,
				String[] argDefault = null,
				AGLCompiler.CmdPrototype[] argPrototype = null,
				Function<void,AGLCompiler,HashMap> argProc = null )
		{
			m_cmd = cmd ;
			m_argType = argType ;
			m_argFlags = argFlags ;
			m_argUsage = null ;
			m_argNames = argNames ;
			m_argDefault = argDefault ;
			m_argPrototype = argPrototype ;
			m_argProc = argProc ;
			//
			if ( argUsage != null )
			{
				m_argUsage = new UsageMatcher( argUsage ) ;
			}
		}

		public void release()
		{
			m_argProc = null ;
		}

	} ;

	protected HashMap<CmdDesc>	m_cmdDesc ;

	protected String[]			m_aIncludeDirs = new String[] ;
	protected HashMap			m_mapLoadedUsageDefFiles = new HashMap() ;

	// マクロ引数
	class	MacroArg
	{
		public static const int	TYPE_TOKEN	= 0 ;
		public static const int	TYPE_STRING	= 1 ;

		public int		nType ;
		public String	sName ;
		public String	sDefault ;
	}

	// マクロ定義
	class	MacroDef
	{
		protected AGLCompiler.MacroArg[]
							m_aArgDecl = new AGLCompiler.MacroArg[] ;	// 引数定義
		protected String[]	m_aLines = new String[] ;					// マクロ文実体

		// 引数解釈
		public String parsePrototype( StringParser sparsArg )
		{
			int[]	fTokenType = new int[1] ;
			while ( sparsArg.passSpace() )
			{
				char	chQuote = sparsArg.hasToComeChar( "\"\'" ) ;
				AGLCompiler.MacroArg
						argDecl = new AGLCompiler.MacroArg() ;
				m_aArgDecl.add( argDecl ) ;
				//
				if ( (chQuote == '\"') || (chQuote == '\'') )
				{
					String	sArgName = sparsArg.getEnclosedString( chQuote ) ;
					argDecl.nType = AGLCompiler.MacroArg.TYPE_STRING ;
					argDecl.sName = sArgName ;
					//
					if ( sparsArg.hasToComeChar( "=" ) == '=' )
					{
						if ( sparsArg.hasToComeChar
								( new String( [ chQuote ] ) ) != chQuote )
						{
							return	"文字列引数のデフォルト値が文字列ではありません" ;
						}
						argDecl.sDefault =
							sparsArg.getEnclosedString( chQuote ).
												decodeCLangString() ;
					}
				}
				else
				{
					String	sArgName = sparsArg.getToken( fTokenType ) ;
					if ( fTokenType[0] != StringParser.tokenNormal )
					{
						return	"マクロ引数名が不正です" ;
					}
					argDecl.nType = AGLCompiler.MacroArg.TYPE_TOKEN ;
					argDecl.sName = sArgName ;
					//
					if ( sparsArg.hasToComeChar( "=" ) == '=' )
					{
						argDecl.sDefault =
							sparsArg.getEnclosedString( ',' ).trim() ;
					}
				}
				if ( sparsArg.passSpace()
					&& (sparsArg.hasToComeChar( "," ) != ',') )
				{
					return	"マクロ引数は \',\' で区切ってください" ;
				}
			}
			return	null ;
		}

		// マクロ定義本体を追加する
		public void addMacroLine( String sLine )
		{
			m_aLines.add( sLine ) ;
		}

		// マクロ定義本体を取得する
		public const String[] getMacroLines()
		{
			return	m_aLines ;
		}

		// マクロ引数を解釈する
		public const String parseArgumentList
			( HashMap mapArgValues, StringParser sparsArg )
		{
			int	iArg = 0 ;
			while ( sparsArg.passSpace() )
			{
				if ( iArg >= m_aArgDecl.length() )
				{
					return	"マクロ引数が多すぎます" ;
				}
				const AGLCompiler.MacroArg 	argDecl = m_aArgDecl[iArg] ;
				if ( (argDecl.nType == AGLCompiler.MacroArg.TYPE_STRING)
					&& (sparsArg.hasToComeChar( "\"" ) == '\"') )
				{
					String	sValue = sparsArg.getEnclosedString( '\"' ) ;
					mapArgValues[argDecl.sName] = sValue.decodeCLangString() ;
					if ( sparsArg.hasToComeChar( "," ) != ',' )
					{
						if ( sparsArg.passSpace()
								&& (iArg < m_aArgDecl.length()) )
						{
							return	"マクロ引数が \',\' で区切られていません" ;
						}
					}
				}
				else if ( sparsArg.hasToComeChar( "<" ) == '<' )
				{
					String	sValue = "" ;
					for ( ; ; )
					{
						char	c = sparsArg.getCharacter() ;
						if ( (c == '>') || (c == 0) )
						{
							break ;
						}
						if ( c == '\\' )
						{
							sValue += sparsArg.getCharacter() ;
						}
						else
						{
							sValue += new String( [ c ] ) ;
						}
					}
					mapArgValues[argDecl.sName] = sValue ;
					if ( sparsArg.hasToComeChar( "," ) != ',' )
					{
						if ( sparsArg.passSpace()
								&& (iArg < m_aArgDecl.length()) )
						{
							return	"マクロ引数が \',\' で区切られていません" ;
						}
					}
				}
				else
				{
					String	sValue = sparsArg.getEnclosedString( ',' ) ;
					mapArgValues[argDecl.sName] = sValue.trim() ;
				}
				iArg ++ ;
			}
			while ( iArg < m_aArgDecl.length() )
			{
				const AGLCompiler.MacroArg 	argDecl = m_aArgDecl[iArg ++] ;
				if ( argDecl.sDefault === null )
				{
					return	"省略できないマクロ引数が記述されていません" ;
				}
				mapArgValues[argDecl.sName] = argDecl.sDefault ;
			}
			return	null ;
		}

		// マクロ文字列を展開する
		public const String evalMacroText
			( String sResult, StringParser sparsLine,
				HashMap mapArgValues, HashMap mapMacroVar )
		{
			boolean	fQuoted = false ;
			String	sEvalText = "" ;
			while ( !sparsLine.isIndexOverflow() )
			{
				char	c = sparsLine.currentCharacter() ;
				if ( c <= ' ' )
				{
					sEvalText += new String( (char[]) [ sparsLine.getCharacter() ] ) ;
				}
				else if ( (c == '\"') || (c == '\'') )
				{
					fQuoted = !fQuoted ;
					sEvalText += new String( (char[]) [ sparsLine.getCharacter() ] ) ;
				}
				else if ( c == '%' )
				{
					char	chPrefix = sparsLine.getCharacter() ;
					char	chQuote = sparsLine.getCharacter() ;
					if ( (chQuote == '\"') || (chQuote == '\'') )
					{
						String	sQuote = new String( (char[]) [ chQuote ] ) ;
						String	sName = sparsLine.getEnclosedString( chQuote ) ;
						if ( mapArgValues[sName] !== null )
						{
							sEvalText +=
								sQuote + mapArgValues[sName].encodeCLangString() + sQuote ;
						}
						else if ( mapMacroVar[sName] !== null )
						{
							sEvalText +=
								sQuote + mapMacroVar[sName].toString().
												encodeCLangString() + sQuote ;
						}
						else
						{
							return	sName + " はマクロ引数ではありません" ;
						}
					}
					else if ( chQuote == '<' )
					{
						String	sName = sparsLine.getEnclosedString( '>' ) ;
						if ( mapArgValues[sName] !== null )
						{
							sEvalText += mapArgValues[sName] ;
						}
						else if ( mapMacroVar[sName] !== null )
						{
							sEvalText += mapMacroVar[sName].toString() ;
						}
						else
						{
							return	sName + " はマクロ引数ではありません" ;
						}
					}
					else if ( chQuote == '(' )
					{
						StringParser	sparsExpr = new StringParser() ;
						sparsExpr.attachString( sparsLine.getEnclosedString( ')' ) ) ;
						String	sEvalExpr = new String() ;
						String	sErr =
							evalMacroText
								( sEvalExpr, sparsExpr, mapArgValues, mapMacroVar ) ;
						if ( sErr != null )
						{
							return	sErr ;
						}
						try
						{
							with( mapMacroVar )
							{
								Object	obj = System.eval( sEvalExpr ) ;
								if ( obj !== null )
								{
									sEvalText += obj.toString() ;
								}
							}
						}
						catch ( Exception e )
						{
							return	"%(" + sEvalExpr + ") の文字列変換エラー" ;
						}
					}
					else if ( chQuote == '%' )
					{
						sEvalText += "%" ;
					}
					else
					{
						sEvalText += new String( [ chPrefix, chQuote ] ) ;
					}
				}
				else if ( StringParser.isPunctuation( c ) )
				{
					sEvalText += new String( (char[]) [ sparsLine.getCharacter() ] ) ;
				}
				else
				{
					String	sToken = sparsLine.getToken() ;
					if ( fQuoted || (mapArgValues[sToken] === null) )
					{
						sEvalText += sToken ;
					}
					else
					{
						sEvalText += mapArgValues[sToken] ;
					}
				}
			}
			sResult.setString( sEvalText ) ;
			return	null ;
		}

	}

	// マクロ定義
	protected HashMap<MacroDef>
							m_mapMacroDefs = null ;
	protected HashMap		m_mapMacroVars ;
	protected MacroDef		m_macroInDef = null ;
	protected MacroDef		m_macroInExp = null ;
	protected int			m_iMacroExpNext = 0 ;

	// 設定ファイル
	protected XMLDocument	m_xmlConfig = null ;

	// メッセージ文パーサー
	protected UsageMatcher	m_umMsgUsage = null ;
	protected String[]		m_argMsgNames = null ;
	protected Function<boolean,AGLCompiler,HashMap,StringParser>
							m_pfnMsgParser = null ;
	protected Function<String,AGLCompiler,HashMap>
							m_pfnMsgProc = null ;
	protected Listener[]	m_aListeners = new Listener[] ;
	protected HashMap		m_mapParserContext = new HashMap() ;

	// 処理中情報
	protected String		m_strSrcFile = null ;
	protected StringParser	m_sparsSrc = null ;
	protected int			m_nSrcLineIndex = 0 ;

	protected int			m_nError = 0 ;

	// 出力
	protected XMLDocument	m_xmlDoc = null ;
	protected XMLDocument[]	m_xmlNest = null ;
	protected XMLDocument	m_xmlCode = null ;

	// スクリプト・ラベル
	class	ScriptLabel
	{
		public String	m_strScript = null ;
		public String	m_strLabel = null ;
	} ;


	// 構築関数
	public AGLCompiler( void )
	{
		m_cmdDesc = new HashMap<AGLCompiler.CmdDesc>() ;

		m_mapMacroDefs = new HashMap<AGLCompiler.MacroDef>() ;
		m_mapMacroVars = new HashMap() ;
		m_mapMacroVars.freeze() ;

		HashMap<String>	mapEnvVar = System.getEnvironmentVariables() ;
		String	strAntirrhinumHome = mapEnvVar.get( "ANTIRRHINUM_HOME" ) ;
		if ( strAntirrhinumHome != null )
		{
			m_aIncludeDirs.push
				( strAntirrhinumHome.offsetFilePath( "Tools" ) ) ;
		}

		addCommand( "@wait", "wait", ARG_TYPE_ARRAY, 0, null, ["time"] ) ;
		addCommand( "@wait_for", "wait_for", ARG_TYPE_USAGE, 0,
								"(%x) [, (%x)]\\", ["cond", "timeout"] ) ;
		addCommand( "@start_timer", "start_timer", ARG_TYPE_ARRAY, 0, null, (String[]) [] ) ;
		addCommand( "@wait_timer", "wait_timer", ARG_TYPE_ARRAY, 0, null, ["time"] ) ;
		addCommand( "@terminate", "terminate", ARG_TYPE_ARRAY, 0, null, ["name"] ) ;
		addCommand( "@permit_skip", "permit_skip", ARG_TYPE_ARRAY, 0, null, ["flags"] ) ;
		addCommand( "@prohibit_skip", "prohibit_skip", ARG_TYPE_ARRAY, 0, null, ["flags"] ) ;
		addCommand( "@dis_interrupt", "dis_interrupt",
					ARG_TYPE_ARRAY, 0, null, ["flag"], ["1"] ) ;
		addCommand( "@trace", "trace", ARG_TYPE_ARRAY, 0, null, ["text"] ) ;
	}

	// ユーザー定義コマンド書式追加
	public void addCommand
		( String strLead, String cmd,
			int argType, int argFlags,
			String argUsage, String[] argNames,
			String[] argDefault = null,
			AGLCompiler.CmdPrototype[] argPrototype = null,
			Function<void,AGLCompiler,HashMap> argProc = null )
	{
		m_cmdDesc[strLead] =
			new AGLCompiler.CmdDesc
				( cmd, argType, argFlags, argUsage,
					argNames, argDefault, argPrototype, argProc ) ;
	}

	// メッセージ文パーサー設定
	public void setMsgParser
			( String strUsage, String[] aArgNames,
				Function<boolean,AGLCompiler,HashMap,StringParser> pfnMsgParser )
	{
		if ( strUsage != null )
		{
			m_umMsgUsage = new UsageMatcher( strUsage ) ;
		}
		m_argMsgNames = aArgNames ;
		m_pfnMsgParser = pfnMsgParser ;
	}

	// メッセージ文処理関数設定
	public void setMsgProcessor
			( Function<String,AGLCompiler,HashMap> pfnMsgProc )
	{
		m_pfnMsgProc = pfnMsgProc ;
	}

	// コンパイル・リスナ設定
	public void attachCompileListener( Listener listener )
	{
		m_aListeners.add( listener ) ;
	}

	// コンパイル・リスナ取得
	public Listener getCompileListener( Class cls )
	{
		for ( int i = 0; i < m_aListeners.length(); i ++ )
		{
			if ( m_aListeners[i] instanceof cls )
			{
				return	m_aListeners[i] ;
			}
		}
		return	null ;
	}

	// 書式定義ファイル読み込み
	public boolean loadUsageDefFile( String strFile )
	{
		if ( !m_mapLoadedUsageDefFiles.isEmpty( strFile ) )
		{
			return	true ;
		}
		String	strBaseDir = strFile.getFileDirectoryPart() ;
		XMLDocument	xmlDoc = new XMLDocument() ;
		if ( !xmlDoc.loadDocument( strFile, new ParserErrorTracer() ) )
		{
			xmlDoc = null ;
			for ( int i = 0; i < m_aIncludeDirs.length(); i ++ )
			{
				xmlDoc = new XMLDocument() ;
				String	strEnvPath =
					m_aIncludeDirs[i].offsetFilePath( strFile.getFileNamePart() ) ;
				if ( xmlDoc.loadDocument( strEnvPath, new ParserErrorTracer() ) )
				{
					strBaseDir = m_aIncludeDirs[i] ;
					break ;
				}
				xmlDoc = null ;
			}
			if ( xmlDoc == null )
			{
				outputError( "\'" + strFile + "\' の読み込みに失敗しました" ) ;
				return	false ;
			}
		}
		XMLDocument	xmlUsages = xmlDoc.getElementTagAs( "agl_usage_definition" ) ;
		if ( xmlUsages == null )
		{
			outputError( "\'" + strFile + "\' <agl_usage_definition> タグが見つかりません" ) ;
			return	false ;
		}
		m_mapLoadedUsageDefFiles.put( strFile, true ) ;

		for ( int i = 0; i < xmlUsages.getElementsCount(); i ++ )
		{
			XMLDocument	xmlTag = xmlUsages.getElementAt(i) ;
			if ( (xmlTag == null)
				|| (xmlTag.getType() != XMLDocument.DocumentType.Tag) )
			{
				continue ;
			}
			const String	strTag = xmlTag.getTag() ;
			if ( strTag == "command" )
			{
				// コマンド定義
				parseCommandDef( xmlTag ) ;
			}
			else if ( strTag == "message" )
			{
				// メッセージ構文定義
				parseMessageDef( xmlTag ) ;
			}
			else if ( strTag == "import" )
			{
				// 定義ファイル・インポート
				const String	strImportFile = xmlTag.getAttributeAs( "file" ) ;
				if ( strImportFile != null )
				{
					loadUsageDefFile( strBaseDir.offsetFilePath(strImportFile) ) ;
				}
			}
			else if ( strTag == "script" )
			{
				// スクリプトファイル・インポート
				const String	strScriptFile = xmlTag.getAttributeAs( "file" ) ;
				const String	strListenerClass = xmlTag.getAttributeAs( "listener" ) ;
				if ( strScriptFile != null )
				{
					importScript
						( strBaseDir.offsetFilePath(strScriptFile),
											strListenerClass, xmlTag ) ;
				}
			}
			else if ( strTag == "macro" )
			{
				// マクロスクリプト・インポート
				const String	strScriptFile = xmlTag.getAttributeAs( "file" ) ;
				if ( strScriptFile != null )
				{
					initialize() ;
					loadSourceFile
						( strBaseDir.offsetFilePath(strScriptFile), true ) ;
				}
			}
		}
		return	true ;
	}

	public void importScript
		( String strScriptFile, String strListenerClass, XMLDocument xmlTag )
	{
		import strScriptFile ;
		//
		if ( strListenerClass != null )
		{
			try
			{
				AGLCompiler.Listener	listener =
					(AGLCompiler.Listener)
						System.eval( "new " + strListenerClass + "()" ) ;
				listener.initialize( this, xmlTag ) ;
				attachCompileListener( listener ) ;
			}
			catch ( Exception e )
			{
				outputError( e.toString() ) ;
			}
		}
	}

	public boolean parseCommandDef( XMLDocument xmlCmdTag )
	{
		const String	strLead = xmlCmdTag.getAttributeAs( "lead" ) ;
		if ( (strLead == null) || (strLead == "") )
		{
			outputError( "lead 属性の無い <command> タグは処理されません" ) ;
			return	false ;
		}
		const String	strCmd = xmlCmdTag.getAttributeAs( "cmd" ) ;
		if ( (strCmd == null) || (strCmd == "") )
		{
			outputError( "cmd 属性の無い <command> タグは処理されません" ) ;
			return	false ;
		}
		String	strArgType = xmlCmdTag.getAttrStringAs( "arg_type", "array" ) ;
		int 	argType = AGLCompiler.ARG_TYPE_ARRAY ;
		int		argFlags = 0 ;
		if ( strArgType == "named" )
		{
			argType = AGLCompiler.ARG_TYPE_NAMED_PARAM ;
		}
		else if ( strArgType == "var_named" )
		{
			argType = AGLCompiler.ARG_TYPE_NAMED_PARAM ;
			argFlags = AGLCompiler.ARG_FLAG_VAR_NAMED ;
		}
		else if ( strArgType == "usage" )
		{
			argType = AGLCompiler.ARG_TYPE_USAGE ;
		}
		const String	strUsage = xmlCmdTag.getAttributeAs( "usage" ) ;
		String[]		argNames = new String[] ;
		String[]		argDefault = null ;
		XMLDocument		xmlArgList = xmlCmdTag.getElementTagAs( "arg_list" ) ;
		if ( xmlArgList != null )
		{
			int	iArg = 0 ;
			for ( int i = 0; i < xmlArgList.getElementsCount(); i ++ )
			{
				XMLDocument	xmlArg = xmlArgList.getElementAt( i ) ;
				if ( (xmlArg == null)
					|| (xmlArg.getTag() != "arg") )
				{
					continue ;
				}
				const String	strName = xmlArg.getAttributeAs( "name" ) ;
				if ( strName == null )
				{
					continue ;
				}
				argNames[iArg] = strName ;
				//
				const String	strDefValue = xmlArg.getAttributeAs( "default" ) ;
				if ( strDefValue != null )
				{
					if ( argDefault == null )
					{
						argDefault = new String[] ;
					}
					argDefault[iArg] = strDefValue ;
				}
				iArg ++ ;
			}
		}
		AGLCompiler.CmdPrototype[]	argPrototype = null ;
		XMLDocument	xmlProtoList = xmlCmdTag.getElementTagAs( "proto_list" ) ;
		if ( xmlProtoList != null )
		{
			argPrototype = new AGLCompiler.CmdPrototype[] ;
			for ( int i = 0; i < xmlProtoList.getElementsCount(); i ++ )
			{
				XMLDocument	xmlProto = xmlProtoList.getElementAt( i ) ;
				if ( (xmlProto == null)
					|| (xmlProto.getTag() != "prototype") )
				{
					continue ;
				}
				AGLCompiler.CmdPrototype
						cmdProto = parserCommandProto( xmlProto ) ;
				if ( cmdProto == null )
				{
					continue ;
				}
				argPrototype.push( cmdProto ) ;
			}
		}
		Function<void,AGLCompiler,HashMap>	argProc = null ;
		XMLDocument	xmlProc = xmlCmdTag.getElementTagAs( "proc" ) ;
		if ( xmlProc != null )
		{
			const String	strTextProc = xmlProc.getTextElement() ;
			if ( strTextProc != null )
			{
				try
				{
					argProc = (Function<void,AGLCompiler,HashMap>)
										System.eval( strTextProc ) ;
				}
				catch ( Exception e )
				{
					outputError( "<" + strCmd + "> : proc 処理中に例外が発生しました : " + e.toString() ) ;
				}
			}
		}
		addCommand
			( strLead, strCmd,
				argType, argFlags, strUsage,
				argNames, argDefault, argPrototype, argProc ) ;
		argProc = null ;
		return	true ;
	}

	public CmdPrototype parserCommandProto( XMLDocument xmlCmdProto )
	{
		AGLCompiler.CmdPrototype
				cmdProto = new AGLCompiler.CmdPrototype() ;
		cmdProto.m_cmd = xmlCmdProto.getAttributeAs( "cmd" ) ;
		if ( cmdProto.m_cmd == null )
		{
			return	null ;
		}
		cmdProto.m_vararg = (xmlCmdProto.getAttrIntegerAs( "vararg", 0 ) != 0) ;

		int	iArg = 0 ;
		for ( int i = 0; i < xmlCmdProto.getElementsCount(); i ++ )
		{
			XMLDocument	xmlArg = xmlCmdProto.getElementAt( i ) ;
			if ( (xmlArg == null)
				|| (xmlArg.getTag() != "arg") )
			{
				continue ;
			}
			const String	strArg = xmlArg.getAttributeAs( "name" ) ;
			if ( strArg == null )
			{
				continue ;
			}
			if ( cmdProto.m_args == null )
			{
				cmdProto.m_args = new String[] ;
			}
			cmdProto.m_args[iArg] = strArg ;
			//
			const String	strSrcParam = xmlArg.getAttributeAs( "src" ) ;
			if ( strSrcParam != null )
			{
				if ( cmdProto.m_params == null )
				{
					cmdProto.m_params = new String[] ;
				}
				cmdProto.m_params[iArg] = strSrcParam ;
			}
			//
			const String	strValue = xmlArg.getAttributeAs( "value" ) ;
			if ( strValue != null )
			{
				if ( cmdProto.m_values == null )
				{
					cmdProto.m_values = new String[] ;
				}
				cmdProto.m_values[iArg] = strValue ;
			}
			iArg ++ ;
		}
		return	cmdProto ;
	}

	public boolean parseMessageDef( XMLDocument xmlMsgTag )
	{
		const String	strUsage = xmlMsgTag.getAttributeAs( "usage" ) ;
		 String			strParser = null ;
		XMLDocument		xmlParser = xmlMsgTag.getElementTagAs( "parser" ) ;
		if ( xmlParser != null )
		{
			strParser = xmlParser.getTextElement() ;
		}
		if ( (strUsage == null) && (strParser == null) )
		{
			outputError( "usage 属性の無い <message> タグは処理されません" ) ;
			return	false ;
		}
		Function<boolean,AGLCompiler,HashMap,StringParser>	pfnMsgParser = null ;
		if ( strParser != null )
		{
			try
			{
				pfnMsgParser =
					(Function<boolean,AGLCompiler,HashMap,StringParser>)
											System.eval( strParser ) ;
			}
			catch ( Exception e )
			{
				outputError( "message parser : " + e.toString() ) ;
			}
		}
		XMLDocument	xmlArgList = xmlMsgTag.getElementTagAs( "arg_list" ) ;
		String[]	aArgNames = new String[] ;
		if ( xmlArgList != null )
		{
			for ( int i = 0; i < xmlArgList.getElementsCount(); i ++ )
			{
				XMLDocument	xmlArg = xmlArgList.getElementAt( i ) ;
				if ( (xmlArg == null)
					|| (xmlArg.getTag() != "arg") )
				{
					continue ;
				}
				const String	strName = xmlArg.getAttributeAs( "name" ) ;
				if ( strName != null )
				{
					aArgNames.push( strName ) ;
				}
			}
		}
		setMsgParser( strUsage, aArgNames, pfnMsgParser ) ;
		pfnMsgParser = null ;
		//
		XMLDocument		xmlProc = xmlMsgTag.getElementTagAs( "proc" ) ;
		if ( xmlProc != null )
		{
			const String	strProc = xmlProc.getTextElement() ;
			if ( strProc != null )
			{
				Function<String,AGLCompiler,HashMap>	pfnMsgProc = null ;
				try
				{
					pfnMsgProc =
						(Function<String,AGLCompiler,HashMap>)
										System.eval( strProc ) ;
				}
				catch ( Exception e )
				{
					outputError( "message proc : " + e.toString() ) ;
				}
				setMsgProcessor( pfnMsgProc ) ;
				pfnMsgProc = null ;
			}
		}
		return	true ;
	}

	// コンフィグファイル読み込み
	public boolean loadConfigFile( String strCfgFile )
	{
		ParserErrorTracer	perr = new ParserErrorTracer() ;
		XMLDocument	xmlDoc = new XMLDocument() ;
		if ( !xmlDoc.loadDocument( strCfgFile, perr ) )
		{
			outputError( "\'" + strCfgFile + "\' の読み込みに失敗しました" ) ;
			perr.printAllErrors() ;
			return	false ;
		}
		m_xmlConfig = xmlDoc ;
		return	true ;
	}

	// コンフィグ取得
	public const XMLDocument getConfig()
	{
		return	m_xmlConfig ;
	}

	// コンパイル実行
	public int run( String strSrcPath, String strDstPath )
	{
		initialize() ;
		//
		if ( loadSourceFile( strSrcPath ) )
		{
			return	m_nError ;
		}
		saveModuleFile( strDstPath ) ;
		//
		return	m_nError ;
	}

	// リソース破棄
	public void initialize( void )
	{
		m_macroInDef = null ;
		m_strSrcFile = null ;
		m_sparsSrc = null ;
		m_nError = 0 ;
		//
		m_xmlDoc = new XMLDocument() ;
		m_xmlDoc.setTag( "aglx_script" ) ;
		//
		m_xmlCode = m_xmlDoc.createElementTagAs( "code" ) ;
		m_xmlNest = new XMLDocument[] ;
		m_xmlNest.push( m_xmlCode ) ;
	}

	// 設定等解放
	public void release( void )
	{
		for ( int i = 0; i < m_cmdDesc.size(); i ++ )
		{
			m_cmdDesc[i].release() ;
		}
		m_cmdDesc = null ;
		m_mapLoadedUsageDefFiles = null ;

		m_mapMacroDefs = null ;
		m_mapMacroVars = null ;

		m_pfnMsgParser = null ;
		m_pfnMsgProc = null ;
		m_aListeners = null ;
		m_mapParserContext = null ;
	}


	// ファイル処理・変換
	public int loadSourceFile( String strPath, boolean flagNoSuccessReport = false )
	{
		StringParser	sparsSrc = new StringParser() ;
		if ( !sparsSrc.loadTextFile( strPath ) )
		{
			outputError( "\'" + strPath + "\' の読み込みに失敗しました" ) ;
			return	m_nError ;
		}
		return	compileSource( sparsSrc, strPath, flagNoSuccessReport ) ;
	}

	public int compileSource
		( StringParser sparsSrc, String strPath, boolean flagNoSuccessReport = false )
	{
		m_sparsSrc = sparsSrc ;
		m_strSrcFile = strPath ;
		//
		const int	nListeners = m_aListeners.length() ;
		for ( int i = 0; i < nListeners; i ++ )
		{
			m_aListeners[i].onBeginCompile( this, strPath ) ;
		}
		//
		String			sLine ;
		StringParser	sparsLine = new StringParser() ;
		while ( !m_sparsSrc.isIndexOverflow() )
		{
			m_nSrcLineIndex = m_sparsSrc.getIndex() ;
			sLine = m_sparsSrc.getLine() ;
			compileSourceLine( sparsLine, sLine ) ;
		}
		for ( int i = 0; i < nListeners; i ++ )
		{
			m_aListeners[i].onEndCompile( this ) ;
		}
		while ( m_xmlNest.length() > 1 )
		{
			outputError( "." + m_xmlCode.getTag() + " は閉じられていません" ) ;
			m_xmlNest.pop() ;
			m_xmlCode = m_xmlNest[m_xmlNest.length() - 1] ;
		}
		m_sparsSrc = null ;
		m_strSrcFile = null ;
		//
		if ( !flagNoSuccessReport || (m_nError > 0) )
		{
			System.console().printf( "\n%s: %d errors\n", strPath, m_nError ) ;
		}
		return	m_nError ;
	}

	public void compileSourceLine( StringParser sparsLine, String sLine )
	{
		sLine = sLine.trimRight() ;
		if ( m_macroInDef != null )
		{
			sparsLine.attachString( sLine ) ;
			sparsLine.seekIndex( 0 ) ;
			if ( (sparsLine.hasToComeChar( "." ) == '.')
				&& sparsLine.hasToComeToken( "endm" ) )
			{
				m_macroInDef = null ;
			}
			else
			{
				m_macroInDef.addMacroLine( sLine ) ;
			}
			return ;
		}
		if ( sLine == "" )
		{
			const int	nListeners = m_aListeners.length() ;
			for ( int i = 0; i < nListeners; i ++ )
			{
				m_aListeners[i].onBlankLine( this ) ;
			}
			return ;
		}
		sparsLine.attachString( sLine ) ;
		sparsLine.seekIndex( 0 ) ;
		//
		char	cLead = sparsLine.hasToComeChar( ".$#;" ) ;
		if ( cLead != 0 )
		{
			if ( cLead == '.' )
			{
				parseDirective( sparsLine ) ;
			}
			else if ( cLead == '$' )
			{
				parseStatement( sparsLine ) ;
			}
			else if ( cLead == '#' )
			{
				parseLabel( sparsLine ) ;
			}
		}
		else
		{
			String	sCmd = sparsLine.getString() ;
			if ( m_mapMacroDefs[sCmd] != null )
			{
				AGLCompiler.MacroDef
						macroDef = m_mapMacroDefs[sCmd] ;
				HashMap	mapArgs = new HashMap() ;
				String	sErr = macroDef.parseArgumentList( mapArgs, sparsLine ) ;
				if ( sErr != null )
				{
					outputError( sErr ) ;
					return ;
				}
				String[]		aMacroLines = macroDef.getMacroLines() ;
				StringParser	sparsMacroLine = new StringParser() ;
				MacroDef		macroInExpSave = m_macroInExp ;
				int				iMacroExpNextSave = m_iMacroExpNext ;
				m_macroInExp = macroDef ;
				m_iMacroExpNext = 0 ;
				while ( m_iMacroExpNext < aMacroLines.length() )
				{
					sparsMacroLine.attachString( aMacroLines[m_iMacroExpNext ++] ) ;
					sparsMacroLine.seekIndex( 0 ) ;
					//
					String	sMacroLine = new String() ;
					sErr = macroDef.evalMacroText
						( sMacroLine, sparsMacroLine, mapArgs, m_mapMacroVars ) ;
					if ( sErr != null )
					{
						outputError( sErr ) ;
					}
					else
					{
						compileSourceLine( sparsLine, sMacroLine ) ;
					}
				}
				m_macroInExp = macroInExpSave ;
				m_iMacroExpNext = iMacroExpNextSave ;
			}
			else if ( m_cmdDesc[sCmd] != null )
			{
				parseUserCommand( sparsLine, m_cmdDesc[sCmd] ) ;
			}
			else
			{
				const int	nListeners = m_aListeners.length() ;
				boolean		flagProcessed = false ;
				for ( int i = 0; i < nListeners; i ++ )
				{
					sparsLine.seekIndex( 0 ) ;
					if ( m_aListeners[i].isExtendCommand( this, sparsLine ) )
					{
						flagProcessed = true ;
						break ;
					}
				}
				if ( !flagProcessed )
				{
					sparsLine.seekIndex( 0 ) ;
					parseMessage( sparsLine ) ;
				}
			}
		}
	}

	// ファイル書き出し
	public boolean saveModuleFile( String strPath )
	{
		System.console().printf( " --> %s\n", strPath ) ;
		if ( !m_xmlDoc.saveDocument( strPath ) )
		{
			outputError( "\'" + strPath + "\' への書き出しに失敗しました" ) ;
			return	false ;
		}
		return	true ;
	}

	public SmartBufferFile formatModuleFile()
	{
		SmartBufferFile	buf = new SmartBufferFile() ;
		OutputStream	stream = buf.getOutputStream() ;
		if ( !m_xmlDoc.writeDocument( stream ) )
		{
			return	null ;
		}
		stream.flush() ;
		buf.seek( 0 ) ;
		return	buf ;
	}

	// ディレクティブ処理
	public void parseDirective( StringParser sparsLine )
	{
		String	sCmd = sparsLine.getToken() ;
		if ( sCmd == null )
		{
			outputError( "不正なディレクティブ" ) ;
			return ;
		}
		String	sExpr ;
		sCmd = sCmd.toLowerCase() ;
		if ( sCmd == "end" )
		{
			// .end
			if ( m_xmlNest.length() <= 1 )
			{
				outputError( "不正な入れ子閉じ指定" ) ;
				return ;
			}
			m_xmlNest.pop() ;
			m_xmlCode = m_xmlNest[m_xmlNest.length() - 1] ;
		}
		else if ( sCmd == "if" )
		{
			// .if <expr>
			XMLDocument	xmlIf = new XMLDocument() ;
			xmlIf.setTag( "if" ) ;
			addCodeCommand( xmlIf ) ;
			//
			sparsLine.passSpace() ;
			sExpr = sparsLine.subString( sparsLine.getIndex() ).trim() ;
			sparsLine.seekToNextLine() ;
			xmlIf.setAttributeAs( "cond", sExpr ) ;
			//
			m_xmlNest.push( xmlIf ) ;
			m_xmlCode = xmlIf ;
		}
		else if ( sCmd == "elseif" )
		{
			// .elseif <expr>
			if ( (m_xmlCode.getTag() != "if")
				&& (m_xmlCode.getTag() != "elseif") )
			{
				outputError( ".elseif が .if と対応しません" ) ;
				return ;
			}
			m_xmlNest.pop() ;
			m_xmlCode = m_xmlNest[m_xmlNest.length() - 1] ;
			//
			XMLDocument	xmlElseIf = new XMLDocument() ;
			xmlElseIf.setTag( "elseif" ) ;
			addCodeCommand( xmlElseIf ) ;
			//
			sparsLine.passSpace() ;
			sExpr = sparsLine.subString( sparsLine.getIndex() ).trim() ;
			sparsLine.seekToNextLine() ;
			xmlElseIf.setAttributeAs( "cond", sExpr ) ;
			//
			m_xmlNest.push( xmlElseIf ) ;
			m_xmlCode = xmlElseIf ;
		}
		else if ( sCmd == "else" )
		{
			// .else
			if ( (m_xmlCode.getTag() != "if")
				&& (m_xmlCode.getTag() != "elseif") )
			{
				outputError( ".else が .if と対応しません" ) ;
				return ;
			}
			m_xmlNest.pop() ;
			m_xmlCode = m_xmlNest[m_xmlNest.length() - 1] ;
			//
			XMLDocument	xmlElse = new XMLDocument() ;
			xmlElse.setTag( "else" ) ;
			addCodeCommand( xmlElse ) ;
			//
			m_xmlNest.push( xmlElse ) ;
			m_xmlCode = xmlElse ;
		}
		else if ( sCmd == "while" )
		{
			// .while <expr>
			XMLDocument	xmlWhile = new XMLDocument() ;
			xmlWhile.setTag( "while" ) ;
			addCodeCommand( xmlWhile ) ;
			//
			sparsLine.passSpace() ;
			sExpr = sparsLine.subString( sparsLine.getIndex() ).trim() ;
			sparsLine.seekToNextLine() ;
			xmlWhile.setAttributeAs( "cond", sExpr ) ;
			//
			m_xmlNest.push( xmlWhile ) ;
			m_xmlCode = xmlWhile ;
		}
		else if ( sCmd == "return" )
		{
			// .return [if <expr>]
			XMLDocument	xmlReturn = new XMLDocument() ;
			xmlReturn.setTag( "return" ) ;
			addCodeCommand( xmlReturn ) ;
			//
			if ( sparsLine.hasToComeNoCaseString( "if" ) )
			{
				sparsLine.passSpace() ;
				sExpr = sparsLine.subString( sparsLine.getIndex() ).trim() ;
				sparsLine.seekToNextLine() ;
				xmlReturn.setAttributeAs( "cond", sExpr ) ;
			}
		}
		else if ( sCmd == "break" )
		{
			// .break [if <expr>]
			XMLDocument	xmlBreak = new XMLDocument() ;
			xmlBreak.setTag( "break" ) ;
			addCodeCommand( xmlBreak ) ;
			//
			if ( sparsLine.hasToComeNoCaseString( "if" ) )
			{
				sparsLine.passSpace() ;
				sExpr = sparsLine.subString( sparsLine.getIndex() ).trim() ;
				sparsLine.seekToNextLine() ;
				xmlBreak.setAttributeAs( "cond", sExpr ) ;
			}
			if ( findNest( "while" ) < 0 )
			{
				outputError( ".break に対応する反復文がありません" ) ;
			}
		}
		else if ( sCmd == "continue" )
		{
			// .continue [if <expr>]
			XMLDocument	xmlContinue = new XMLDocument() ;
			xmlContinue.setTag( "continue" ) ;
			addCodeCommand( xmlContinue ) ;
			//
			if ( sparsLine.hasToComeNoCaseString( "if" ) )
			{
				sparsLine.passSpace() ;
				sExpr = sparsLine.subString( sparsLine.getIndex() ).trim() ;
				sparsLine.seekToNextLine() ;
				xmlContinue.setAttributeAs( "cond", sExpr ) ;
			}
			if ( findNest( "while" ) < 0 )
			{
				outputError( ".continue に対応する反復文がありません" ) ;
			}
		}
		else if ( sCmd == "call" )
		{
			// .call <script#label> [if <expr>]
			AGLCompiler.ScriptLabel	scrLabel =
					parseScriptLabel( sparsLine.getStringTerm() ) ;
			//
			XMLDocument	xmlCall = new XMLDocument() ;
			xmlCall.setTag( "call" ) ;
			addCodeCommand( xmlCall ) ;
			//
			if ( scrLabel.m_strScript != null )
			{
				xmlCall.setAttributeAs( "file", scrLabel.m_strScript ) ;
			}
			if ( scrLabel.m_strLabel != null )
			{
				xmlCall.setAttributeAs( "label", scrLabel.m_strLabel ) ;
			}
			if ( sparsLine.hasToComeNoCaseString( "if" ) )
			{
				sparsLine.passSpace() ;
				sExpr = sparsLine.subString( sparsLine.getIndex() ).trim() ;
				sparsLine.seekToNextLine() ;
				xmlCall.setAttributeAs( "cond", sExpr ) ;
			}
		}
		else if ( sCmd == "jump" )
		{
			// .jump <script#label> [if <expr>]
			AGLCompiler.ScriptLabel	scrLabel =
					parseScriptLabel( sparsLine.getStringTerm() ) ;
			//
			XMLDocument	xmlJump = new XMLDocument() ;
			xmlJump.setTag( "jump" ) ;
			addCodeCommand( xmlJump ) ;
			//
			if ( scrLabel.m_strScript != null )
			{
				xmlJump.setAttributeAs( "file", scrLabel.m_strScript ) ;
			}
			if ( scrLabel.m_strLabel != null )
			{
				xmlJump.setAttributeAs( "label", scrLabel.m_strLabel ) ;
			}
			if ( sparsLine.hasToComeNoCaseString( "if" ) )
			{
				sparsLine.passSpace() ;
				sExpr = sparsLine.subString( sparsLine.getIndex() ).trim() ;
				sparsLine.seekToNextLine() ;
				xmlJump.setAttributeAs( "cond", sExpr ) ;
			}
		}
		else if ( sCmd == "thread" )
		{
			// .thread [as <name>] [if <expr>]]
			// .thread <script#label> [as <name>] [if <expr>]
			XMLDocument	xmlThread = new XMLDocument() ;
			xmlThread.setTag( "thread" ) ;
			addCodeCommand( xmlThread ) ;
			//
			if ( sparsLine.hasToComeNoCaseString( "as" ) )
			{
				sparsLine.passSpace() ;
				xmlThread.setAttributeAs( "name", sparsLine.getStringTerm() ) ;
			}
			if ( sparsLine.hasToComeNoCaseString( "if" ) )
			{
				sparsLine.passSpace() ;
				sExpr = sparsLine.subString( sparsLine.getIndex() ).trim() ;
				sparsLine.seekToNextLine() ;
				xmlThread.setAttributeAs( "cond", sExpr ) ;
			}
			if ( !sparsLine.passSpace() )
			{
				m_xmlNest.push( xmlThread ) ;
				m_xmlCode = xmlThread ;
			}
			else
			{
				AGLCompiler.ScriptLabel	scrLabel =
						parseScriptLabel( sparsLine.getStringTerm() ) ;
				//
				if ( scrLabel.m_strScript != null )
				{
					xmlThread.setAttributeAs( "file", scrLabel.m_strScript ) ;
				}
				if ( scrLabel.m_strLabel != null )
				{
					xmlThread.setAttributeAs( "label", scrLabel.m_strLabel ) ;
				}
				if ( sparsLine.hasToComeNoCaseString( "as" ) )
				{
					sparsLine.passSpace() ;
					xmlThread.setAttributeAs( "name", sparsLine.getStringTerm() ) ;
				}
				if ( sparsLine.hasToComeNoCaseString( "if" ) )
				{
					sparsLine.passSpace() ;
					sExpr = sparsLine.subString( sparsLine.getIndex() ).trim() ;
					sparsLine.seekToNextLine() ;
					xmlThread.setAttributeAs( "cond", sExpr ) ;
				}
			}
		}
		else if ( sCmd == "fwait" )
		{
			// .fwait
			XMLDocument	xmlFWait = new XMLDocument() ;
			xmlFWait.setTag( "fwait" ) ;
			addCodeCommand( xmlFWait ) ;
		}
		else if ( sCmd == "mlet" )
		{
			// .mlet <macro-var> = <macro-expr>
			int[]	fTokenType = new int[1] ;
			String	sVarName = sparsLine.getToken( fTokenType ) ;
			if ( fTokenType[0] != StringParser.tokenNormal )
			{
				outputError( "マクロ変数名が不正です" ) ;
				return ;
			}
			if ( sparsLine.hasToComeChar( "=" ) != '=' )
			{
				outputError( ".mlet 構文が不正です" ) ;
				return ;
			}
			try
			{
				Object	obj ;
				with( m_mapMacroVars )
				{
					obj = System.eval( sparsLine.subString( sparsLine.getIndex() ) ) ;
				}
				m_mapMacroVars.put( sVarName, obj ) ;
			}
			catch ( Exception e )
			{
				outputError( e.toString() ) ;
			}
			return ;
		}
		else if ( sCmd == "mexpr" )
		{
			// .mexpr <macro-expr>
			try
			{
				with( m_mapMacroVars )
				{
					System.eval( sparsLine.subString( sparsLine.getIndex() ) ) ;
				}
			}
			catch ( Exception e )
			{
				outputError( e.toString() ) ;
			}
			return ;
		}
		else if ( sCmd == "macro" )
		{
			// .macro <macro-name> [: <arg-name> [= <default-value>] [, ...]]
			int[]	fTokenType = new int[1] ;
			String	sMacroName = sparsLine.getToken( fTokenType ) ;
			if ( fTokenType[0] != StringParser.tokenNormal )
			{
				outputError
					( "マクロ名が記述されていないか不正なマクロ名です" ) ;
				return ;
			}
			if ( m_macroInDef != null )
			{
				outputError
					( ".macro を入れ子にすることはできません" ) ;
				return ;
			}
			m_macroInDef = new AGLCompiler.MacroDef() ;
			m_mapMacroDefs[sMacroName] = m_macroInDef ;
			//
			if ( sparsLine.hasToComeChar( ":" ) == ':' )
			{
				String	sErr = m_macroInDef.parsePrototype( sparsLine ) ;
				if ( sErr != null )
				{
					outputError( sErr ) ;
				}
			}
			else if ( sparsLine.passSpace() )
			{
				outputError
					( "マクロ引数は \':\' で区切って記述してください" ) ;
			}
			return ;
		}
		else
		{
			outputError( "未定義ディレクティブ ." + sCmd ) ;
			return ;
		}
		if ( sparsLine.passSpace() )
		{
			if ( sparsLine.currentCharacter() != ';' )
			{
				outputError( "." + sCmd + " 末尾に処理されない記述" ) ;
			}
		}
	}

	// script#label パラメータ書式解釈
	public ScriptLabel parseScriptLabel( String sScriptLabel )
	{
		AGLCompiler.ScriptLabel	scrLabel = new AGLCompiler.ScriptLabel() ;
		int	iLabel = sScriptLabel.indexOf( "#" ) ;
		if ( iLabel >= 0 )
		{
			scrLabel.m_strLabel = sScriptLabel.substring( iLabel + 1 ) ;
			if ( iLabel > 0 )
			{
				scrLabel.m_strScript = sScriptLabel.substring( 0, iLabel ) ;
			}
			else
			{
				scrLabel.m_strScript = null ;
			}
		}
		else
		{
			scrLabel.m_strScript = sScriptLabel ;
			scrLabel.m_strLabel = null ;
		}
		if ( (scrLabel.m_strScript != null)
			&& (scrLabel.m_strScript.getFileNamePart().indexOf(".") < 0) )
		{
			scrLabel.m_strScript += ".xmlagl" ;
		}
		return	scrLabel ;
	}

	// ネスト検索
	public int findNest( String sTag )
	{
		int	count = m_xmlNest.length() ;
		for ( int i = 0; i < count; i ++ )
		{
			if ( m_xmlNest[count - i - 1].getTag() == sTag )
			{
				return	count - i - 1 ;
			}
		}
		return	-1 ;
	}

	// rosetta 文処理
	public void parseStatement( StringParser sparsLine )
	{
		String	sSrc = "" ;
		if ( sparsLine.hasToComeChar( "{" ) == '{' )
		{
			sparsLine.passSpace() ;
			int	iBegin = sparsLine.getIndex() ;
			sparsLine.passExpression( "}" ) ;
			sSrc = sparsLine.subString( iBegin, sparsLine.getIndex() - iBegin ) ;
			//
			if ( sparsLine.hasToComeChar( "}" ) != '}' )
			{
				if ( m_macroInExp != null )
				{
					boolean		flagClosed = false ;
					String[]	aMacroLines = m_macroInExp.getMacroLines() ;
					while ( m_iMacroExpNext < aMacroLines.length() )
					{
						StringParser	sparsMacro = new StringParser() ;
						sparsMacro.attachString( aMacroLines[m_iMacroExpNext ++] ) ;
						sparsMacro.seekIndex( 0 ) ;
						sparsMacro.passExpression( "}" ) ;
						sSrc += sparsMacro.subString( 0, sparsMacro.getIndex() ) ;
						//
						if ( m_sparsSrc.hasToComeChar( "}" ) == '}' )
						{
							flagClosed = true ;
							break ;
						}
					}
					if ( !flagClosed )
					{
						outputError( "文の閉じ括弧 \'}\' がありません" ) ;
					}
				}
				else
				{
					iBegin = m_sparsSrc.getIndex() ;
					m_sparsSrc.passExpression( "}" ) ;
					sSrc += m_sparsSrc.subString( iBegin, m_sparsSrc.getIndex() - iBegin ) ;
					//
					if ( m_sparsSrc.hasToComeChar( "}" ) != '}' )
					{
						outputError( "文の閉じ括弧 \'}\' がありません" ) ;
					}
				}
			}
		}
		else
		{
			sSrc = sparsLine.subString( sparsLine.getIndex() ).trim() ;
		}
		XMLDocument	xmlEval = new XMLDocument() ;
		xmlEval.setTag( "eval" ) ;
		xmlEval.setAttributeAs( "src", sSrc ) ;
		addCodeCommand( xmlEval ) ;
	}

	// ラベル文処理
	public void parseLabel( StringParser sparsLine )
	{
		String	sLabel = sparsLine.getString() ;
		if ( (sLabel == null) || (sLabel == "") )
		{
			return ;
		}
		XMLDocument	xmlLabel = new XMLDocument() ;
		xmlLabel.setTag( "label" ) ;
		xmlLabel.setAttributeAs( "id", sLabel ) ;
		addCodeCommand( xmlLabel ) ;
	}

	// ユーザー定義コマンド処理
	public void parseUserCommand( StringParser sparsLine, CmdDesc cmdDesc )
	{
		HashMap<String>	mapParam = new HashMap<String>() ;
		if ( cmdDesc.m_argType == ARG_TYPE_ARRAY )
		{
			// 引数順固定
			int	iArg = 0 ;
			while ( sparsLine.passSpace() )
			{
				String	sValue ;
				boolean	fComma = false ;
				char	c = sparsLine.currentCharacter() ;
				if ( (c == '\'') || (c == '\"') )
				{
					sValue = sparsLine.getStringTerm().decodeCLangString() ;
				}
				else
				{
					sparsLine.markIndex() ;
					sValue = sparsLine.getString() ;
					if ( sValue.indexOf(",") > 0 )
					{
						sparsLine.seekToMark() ;
						sValue = sparsLine.getEnclosedString( ',' ) ;
						fComma = true ;
					}
				}
				if ( iArg >= cmdDesc.m_argNames.length() )
				{
					outputError( cmdDesc.m_cmd + ": 引数が多すぎます" ) ;
					break ;
				}
				mapParam[cmdDesc.m_argNames[iArg]] = sValue ;
				iArg ++ ;
				//
				if ( !fComma )
				{
					sparsLine.hasToComeChar( "," ) ;
				}
			}
			if ( cmdDesc.m_argDefault != null )
			{
				while ( iArg < cmdDesc.m_argDefault.length() )
				{
					if ( iArg >= cmdDesc.m_argNames.length() )
					{
						outputError( cmdDesc.m_cmd + ": デフォルト引数の名前が未定義です" ) ;
						break ;
					}
					if ( cmdDesc.m_argDefault[iArg] == null )
					{
						outputError( cmdDesc.m_cmd + ": デフォルト引数が未定義です" ) ;
						iArg ++ ;
						continue ;
					}
					mapParam[cmdDesc.m_argNames[iArg]] = cmdDesc.m_argDefault[iArg] ;
					iArg ++ ;
				}
			}
		}
		else if ( cmdDesc.m_argType == ARG_TYPE_NAMED_PARAM )
		{
			// 引数順不同
			int	iArg = 0 ;
			while ( sparsLine.passSpace() )
			{
				char	c = sparsLine.currentCharacter() ;
				if ( (c == '\'') || (c == '\"') )
				{
					String	sValue = sparsLine.getStringTerm().decodeCLangString() ;
					if ( iArg >= cmdDesc.m_argNames.length() )
					{
						outputError( cmdDesc.m_cmd + ": 引数名が未指定です" ) ;
						break ;
					}
					mapParam[cmdDesc.m_argNames[iArg]] = sValue ;
				}
				else
				{
					int		iTerm = sparsLine.getIndex() ;
					String	sName = sparsLine.getToken() ;
					if ( sparsLine.hasToComeChar( "=" ) == '=' )
					{
						String	sValue ;
						sparsLine.passSpace() ;
						c = sparsLine.currentCharacter() ;
						if ( (c == '\'') || (c == '\"') )
						{
							sValue = sparsLine.getStringTerm() ;
						}
						else
						{
							sValue = sparsLine.getStringTerm().decodeCLangString() ;
						}
						if ( !(cmdDesc.m_argFlags & AGLCompiler.ARG_FLAG_VAR_NAMED) )
						{
							const int	nArgCount = cmdDesc.m_argNames.length() ;
							boolean		flagValidName = false ;
							for ( int i = 0; i < nArgCount; i ++ )
							{
								if ( cmdDesc.m_argNames[i] == sName )
								{
									flagValidName = true ;
									break ;
								}
							}
							if ( !flagValidName )
							{
								outputError
									( cmdDesc.m_cmd + ": 引数名 " + sName
											+ " は定義されていない名前です" ) ;
							}
						}
						mapParam[sName] = sValue ;
					}
					else if ( cmdDesc.m_argNames != null )
					{
						if ( iArg >= cmdDesc.m_argNames.length() )
						{
							outputError( cmdDesc.m_cmd + ": 引数名が未指定です" ) ;
							break ;
						}
						sparsLine.seekIndex( iTerm ) ;
						//
						String	sValue = sparsLine.getStringTerm() ;
						if ( sValue.charLastCodeAt(0) == ',' )
						{
							sValue = sValue.chopRight(1) ;
						}
						mapParam[cmdDesc.m_argNames[iArg]] = sValue ;
					}
					else
					{
						outputError( cmdDesc.m_cmd + ": 引数名が未指定です" ) ;
						break ;
					}
				}
				sparsLine.hasToComeChar( "," ) ;
				iArg ++ ;
			}
			if ( (cmdDesc.m_argNames != null)
				&& (cmdDesc.m_argDefault != null) )
			{
				for ( int i = 0; i < cmdDesc.m_argDefault.length(); i ++ )
				{
					if ( (cmdDesc.m_argNames[i] == null)
						|| (cmdDesc.m_argDefault[i] == null) )
					{
						continue ;
					}
					if ( mapParam[cmdDesc.m_argNames[i]] == null )
					{
						mapParam[cmdDesc.m_argNames[i]] = cmdDesc.m_argDefault[i] ;
					}
				}
			}
		}
		else if ( cmdDesc.m_argType == ARG_TYPE_USAGE )
		{
			// 書式
			if ( cmdDesc.m_argUsage == null )
			{
				outputError( cmdDesc.m_cmd + ": 書式が未定義です" ) ;
				return ;
			}
			String[]	aParams = new String[] ;
			sparsLine.passSpace() ;
			String	sErr = cmdDesc.m_argUsage.parseNext( sparsLine, aParams ) ;
			//
			if ( sErr != null )
			{
				outputError( cmdDesc.m_cmd + ": " + sErr ) ;
				return ;
			}
			for ( int i = 0; i < aParams.length(); i ++ )
			{
				if ( i >= cmdDesc.m_argNames.length() )
				{
					outputError( cmdDesc.m_cmd + ": 引数名が未定義です" ) ;
					break ;
				}
				if ( (aParams[i] == "")
					&& (cmdDesc.m_argDefault != null)
					&& (i < cmdDesc.m_argDefault.length()) )
				{
					mapParam[cmdDesc.m_argNames[i]] = cmdDesc.m_argDefault[i] ;
				}
				else
				{
					mapParam[cmdDesc.m_argNames[i]] = aParams[i] ;
				}
			}
		}
		if ( sparsLine.passSpace() )
		{
			if ( sparsLine.currentCharacter() != ';' )
			{
				outputError( "." + cmdDesc.m_cmd + " 末尾に処理されない記述" ) ;
			}
		}
		if ( cmdDesc.m_argProc != null )
		{
			(this.*(cmdDesc.m_argProc))( mapParam ) ;
		}
		if ( cmdDesc.m_argPrototype != null )
		{
			for ( int i = 0; i < cmdDesc.m_argPrototype.length(); i ++ )
			{
				AGLCompiler.CmdPrototype	cmdProto = cmdDesc.m_argPrototype[i] ;
				if ( cmdProto == null )
				{
					continue ;
				}
				XMLDocument	xmlCmd = new XMLDocument() ;
				xmlCmd.setTag( cmdProto.m_cmd ) ;
				//
				if ( cmdProto.m_args != null )
				{
					for ( int j = 0; j < cmdProto.m_args.length(); j ++ )
					{
						if ( cmdProto.m_args[j] == null )
						{
							continue ;
						}
						String	strParam = cmdProto.m_args[j] ;
						if ( (cmdProto.m_values != null)
							&& (j < cmdProto.m_values.length())
							&& (cmdProto.m_values[j] != null) )
						{
							xmlCmd.setAttributeAs
								( cmdProto.m_args[j], cmdProto.m_values[j] ) ;
						}
						else
						{
							if ( (cmdProto.m_params != null)
								&& (j < cmdProto.m_params.length())
								&& (cmdProto.m_params[j] != null) )
							{
								strParam = cmdProto.m_params[j] ;
							}
							if ( mapParam[strParam] != null )
							{
								xmlCmd.setAttributeAs
									( cmdProto.m_args[j], mapParam[strParam] ) ;
							}
						}
					}
				}
				if ( cmdProto.m_vararg )
				{
					for ( int i = 0; i < mapParam.size(); i ++ )
					{
						String	strAttr = mapParam.keyAt(i) ;
						if ( xmlCmd.getAttributeAs( strAttr ) == null )
						{
							xmlCmd.setAttributeAs( strAttr, mapParam[i] ) ;
						}
					}
				}
				addCodeCommand( xmlCmd ) ;
			}
		}
		else
		{
			XMLDocument	xmlCmd = new XMLDocument() ;
			xmlCmd.setTag( cmdDesc.m_cmd ) ;
			//
			for ( int i = 0; i < mapParam.size(); i ++ )
			{
				xmlCmd.setAttributeAs( mapParam.keyAt(i), mapParam[i] ) ;
			}
			addCodeCommand( xmlCmd ) ;
		}
	}

	// メッセージ文処理
	public void parseMessage( StringParser sparsLine )
	{
		HashMap<String>	mapParam = new HashMap<String>() ;
		if ( m_pfnMsgParser != null )
		{
			if ( !(this.*m_pfnMsgParser)( mapParam, sparsLine ) )
			{
				outputError( "不正な構文です" ) ;
				return ;
			}
		}
		else if ( m_umMsgUsage != null )
		{
			String[]	aParams = new String[] ;
			String	sErr = m_umMsgUsage.parseNext( sparsLine, aParams ) ;
			if ( sErr != null )
			{
				outputError( "不正な構文です(" + sErr + ")" ) ;
				return ;
			}
			for ( int i = 0; i < aParams.length(); i ++ )
			{
				if ( i >= m_argMsgNames.length() )
				{
					outputError( "引数名が未定義です" ) ;
					break ;
				}
				mapParam[m_argMsgNames[i]] = aParams[i] ;
			}
		}
		else
		{
			outputError( "不正な構文です" ) ;
			return ;
		}

		// パラメータ処理
		String	sCmd = processMessageParam( mapParam ) ;
		if ( sCmd == null )
		{
			return ;
		}

		// コマンド
		XMLDocument	xmlCmd = new XMLDocument() ;
		xmlCmd.setTag( sCmd ) ;
		addCodeCommand( xmlCmd ) ;
		//
		for ( int i = 0; i < mapParam.size(); i ++ )
		{
			xmlCmd.setAttributeAs( mapParam.keyAt(i), mapParam[i] ) ;
		}
	}

	// メッセージ処理
	public String processMessageParam( HashMap<String> mapParam )
	{
		if ( m_pfnMsgProc != null )
		{
			return	m_pfnMsgProc( mapParam ) ;
		}
		return	"msg" ;
	}

	// 直前のコードタグ取得
	public const XMLDocument getLastCodeCommand()
	{
		int	nElements = m_xmlCode.getElementsCount() ;
		if ( nElements >= 1 )
		{
			return	m_xmlCode.getElementAt( nElements - 1 ) ;
		}
		return	null ;
	}

	// コードコマンド追加
	public void addCodeCommand( XMLDocument xmlCmd )
	{
		const int	nListeners = m_aListeners.length() ;
		for ( int i = 0; i < nListeners; i ++ )
		{
			m_aListeners[i].beforeAddCommand( this, xmlCmd ) ;
		}
		if ( xmlCmd.getTag() != "" )
		{
			m_xmlCode.addElement( xmlCmd ) ;
		}
	}

	// パーサーコンテキスト取得
	public const HashMap getParserContext()
	{
		return	m_mapParserContext ;
	}

	// エラー出力
	public void outputError( String sErr )
	{
		if ( (m_strSrcFile == null) || (m_sparsSrc == null) )
		{
			System.console().printf( "error: %s\n", sErr ) ;
		}
		else
		{
			int	nLine = m_sparsSrc.getLineNumberOf( m_nSrcLineIndex ) ;
			System.console().printf( "%s(%d): %s\n", m_strSrcFile, nLine, sErr ) ;
		}
		m_nError ++ ;
	}
} ;


