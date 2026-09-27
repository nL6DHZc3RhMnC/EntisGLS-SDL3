
class	StdMessageCompilerListener	implements AGLCompiler.Listener
{
	class	CharConfig
	{
		public int		m_nIndex ;			// キャラクタ・インデックス
		public String	m_idChar ;			// キャラクタ識別子（表示またはスクリプト記述用）
		public String	m_strName ;			// キャラクタ名（表示用）
		public String	m_strNameEn ;		// 英語用キャラクタ名
		public String	m_strFaceLead ;		// フェイス画像ファイル名先頭共通部
		public String	m_strBSLead ;		// 立ち絵ファイル先頭共通部
		public String	m_strVoiceLead ;	// ボイスファイル先頭共通部

		public String	m_strCurFaceOpt = "" ;

		public void parseConfig( XMLDocument xmlTag )
		{
			m_nIndex = xmlTag.getAttrIntegerAs( "index" ) ;
			m_idChar = xmlTag.getAttrStringAs( "id" ) ;
			m_strName = xmlTag.getAttrStringAs( "name" ) ;
			m_strNameEn = xmlTag.getAttrStringAs( "name_en" ) ;
			m_strFaceLead = xmlTag.getAttrStringAs( "face_file_lead" ) ;
			m_strBSLead = xmlTag.getAttrStringAs( "bs_file_lead" ) ;
			m_strVoiceLead = xmlTag.getAttrStringAs( "voice_file_lead" ) ;
		}
	}


	protected boolean		m_flagMsgStatement = true ;
	protected boolean		m_flagStrictName = false ;
	protected boolean		m_flagMustNameEncloser = false ;
	protected String		m_sNameStarters = null ;
	protected String		m_sNameClosers = null ;
	protected int			m_iMsgIndex = 0 ;
	protected int			m_iMenuIndex = 0 ;
	protected CharConfig[]	m_aCharConfigs ;
	protected String		m_sWarnMsgLeadChars ;
	protected int			m_nMsgFadeTime = 300 ;

	protected String		m_strNameEn = null ;
	protected String		m_strMessageEn = null ;

	// メッセージ文解釈
	// [<char-id>[/<name>][.<face-opt>]][{+|-|&a|&t=<timeout>|&f=<fadeout>}*][{\| }]<message>[|<voice>]*
	public boolean parseMessageStatement( AGLCompiler compiler, StringParser line )
	{
		if ( !m_flagMsgStatement )
		{
			return	false ;
		}
		XMLDocument	xmlMsg = new XMLDocument() ;
		xmlMsg.setTag( "msg" ) ;
		//
		boolean	flagWithChar = false ;
		char	c = line.currentCharacter() ;
		if ( (m_sNameStarters != null) && (c != 0)
			&& (m_sNameStarters.indexOf
					( line.subString( line.getIndex(), 1 ) ) >= 0) )
		{
			// キャラクター指定構文判定
			flagWithChar = parseMessageCharId( xmlMsg, compiler, line ) ;
			c = line.currentCharacter() ;
		}
		else if ( !m_flagMustNameEncloser
				&& (c != '\\') && (c != '+') && (c != '-') && (c != '&') )
		{
			// キャラクター指定構文判定
			flagWithChar = parseMessageCharId( xmlMsg, compiler, line ) ;
			c = line.currentCharacter() ;
		}
		// オプション引数解釈
		for ( ; ; )
		{
			if ( c == '+' )
			{
				line.getCharacter() ;
				xmlMsg.setAttrIntegerAs( "add_msg", 1 ) ;
			}
			else if ( c == '-' )
			{
				line.getCharacter() ;
				xmlMsg.setAttrIntegerAs( "keep_msg", 1 ) ;
				xmlMsg.setAttrIntegerAs( "keep_voice", 1 ) ;
			}
			else if ( c == '&' )
			{
				line.getCharacter() ;
				c = line.getCharacter() ;
				if ( c == 'a' )
				{
					xmlMsg.setAttrIntegerAs( "async", 1 ) ;
				}
				else if ( c == 't' )
				{
					line.hasToComeChar( "=" ) ;
					xmlMsg.setAttrIntegerAs( "timeout", line.nextInteger() ) ;
				}
				else if ( c == 'f' )
				{
					line.hasToComeChar( "=" ) ;
					xmlMsg.setAttrIntegerAs( "fadeout", line.nextInteger() ) ;
				}
				else
				{
					compiler.outputError
						( String.format( "メッセージ文の &%c は不正な構文です", c ) ) ;
				}
			}
			else
			{
				break ;
			}
			c = line.currentCharacter() ;
		}
		if ( (c == '\\') || (c == ' ') || (c == '\t') )
		{
			line.getCharacter() ;
		}
		if ( m_sNameClosers != null )
		{
			line.hasToComeChar( m_sNameClosers ) ;
		}
		// メッセージ本文取得
		String	strMsgText ;
		int		iMsgMarkIndex = line.getIndex() ;
		line.markIndex() ;
		if ( line.seekAnyCharacters( "|" ) )
		{
			strMsgText= line.subStringFromMark() ;
			xmlMsg.setAttributeAs( "text", strMsgText ) ;
			//
			line.hasToComeChar( "|" ) ;
			xmlMsg.setAttributeAs
				( "voices", line.subString( line.getIndex() ) ) ;
		}
		else
		{
			strMsgText = line.subString( line.getIndex() ) ;
			xmlMsg.setAttributeAs( "text", strMsgText ) ;
		}
		if ( (m_sWarnMsgLeadChars != null)
			// && (iMsgMarkIndex == 0)
			&& (strMsgText != null) && (strMsgText.length() >= 1)
			&& (m_sWarnMsgLeadChars.indexOf(strMsgText.charAt(0)) >= 0) )
		{
			compiler.outputError
				( "\"" + strMsgText + "\" はメッセージ文として処理されました" ) ;
		}

		// メッセージ・インデックス
		xmlMsg.setAttrIntegerAs( "index", m_iMsgIndex ++ ) ;

		// 英文設定
		if ( m_strNameEn != null )
		{
			xmlMsg.setAttributeAs( "name_en", m_strNameEn ) ;
			m_strNameEn = null ;
		}
		if ( m_strMessageEn != null )
		{
			xmlMsg.setAttributeAs( "text_en", m_strMessageEn ) ;
			m_strMessageEn = null ;
		}

		compiler.addCodeCommand( xmlMsg ) ;
		return	true ;
	}

	public boolean parseMessageCharId
			( XMLDocument xmlMsg, AGLCompiler compiler, StringParser line )
	{
		String	sNameCloser = "" ;
		if ( m_sNameClosers != null )
		{
			sNameCloser = m_sNameClosers ;
		}
		// char-id 取得
		boolean	flagNameStarted = false ;
		int	nIndex = line.getIndex() ;
		line.markIndex() ;
		if ( m_sNameStarters != null )
		{
			if ( line.hasToComeChar( m_sNameStarters ) != 0 )
			{
				flagNameStarted = true ;
				line.markIndex() ;
			}
		}
		if ( !line.seekAnyCharacters( " \t/,+-&\\" + sNameCloser ) )
		{
			return	false ;
		}
		String	idChar = line.subStringFromMark() ;
		boolean	flagNullChar = false ;
		//
		StdMessageCompilerListener.CharConfig
				cfgChar = getCharConfigById( idChar ) ;
		if ( cfgChar == null )
		{
			cfgChar = getCharConfigByName( idChar ) ;
			if ( cfgChar == null )
			{
				if ( m_flagStrictName )
				{
					compiler.outputError
						( "「" + idChar + "」は定義されていないキャラクタ識別子です。" ) ;
				}
				cfgChar = getCharConfigById( "" ) ;
				if ( cfgChar == null )
				{
					if ( !flagNameStarted )
					{
						line.seekIndex( nIndex ) ;
						return	false ;
					}
				}
				xmlMsg.setAttributeAs( "name", idChar ) ;
				flagNullChar = true ;
			}
		}
		if ( cfgChar != null )
		{
			xmlMsg.setAttrIntegerAs( "char_idx", cfgChar.m_nIndex ) ;
			//
			char	c = line.hasToComeChar( "/," ) ;
			if ( c == '/' )
			{
				// キャラ表示名取得
				line.markIndex() ;
				if ( line.seekAnyCharacters( " \t,+-&\\" + sNameCloser ) )
				{
					xmlMsg.setAttributeAs( "name", line.subStringFromMark() ) ;
				}
				else
				{
					compiler.outputError
						( "メッセージ文のキャラクタ表示名の記述が不正です" ) ;
				}
				c = line.hasToComeChar( "," ) ;
			}
			if ( c == ',' )
			{
				// フェイス表示画像取得
				line.markIndex() ;
				if ( line.seekAnyCharacters( " \t+-&\\" + sNameCloser ) )
				{
					String	strFaceOpt = line.subStringFromMark() ;
					if ( strFaceOpt != "" )
					{
						xmlMsg.setAttributeAs
							( "face", cfgChar.m_strFaceLead + strFaceOpt ) ;
					}
					cfgChar.m_strCurFaceOpt = strFaceOpt ;
				}
				else
				{
					compiler.outputError
						( "メッセージ文のフェイス画像の記述が不正です" ) ;
				}
			}
			else
			{
				if ( cfgChar.m_strCurFaceOpt != "" )
				{
					xmlMsg.setAttributeAs
						( "face", cfgChar.m_strFaceLead + cfgChar.m_strCurFaceOpt ) ;
				}
			}
		}
		if ( m_sNameClosers != null )
		{
			line.hasToComeChar( m_sNameClosers ) ;
		}
		return	true ;
	}

	// 初期設定
	public void initialize( AGLCompiler compiler, XMLDocument xmlTag )
	{
		m_aCharConfigs = new CharConfig[] ;
		m_flagMsgStatement =
			(xmlTag.getAttrIntegerAs( "disable_msg_statement", 0 ) == 0) ;

		XMLDocument	xmlConfig = compiler.getConfig() ;
		if ( xmlConfig != null )
		{
			xmlConfig = xmlConfig.getElementTagAs( "config" ) ;
		}
		if ( xmlConfig != null )
		{
			XMLDocument	xmlCharDef = xmlConfig.getElementTagAs( "character_definition" ) ;
			if ( xmlCharDef != null )
			{
				parseConfigCharDef( xmlCharDef ) ;
			}
			XMLDocument	xmlMsgCfg = xmlConfig.getElementTagAs( "message" ) ;
			if ( xmlMsgCfg != null )
			{
				parseConfigMessage( xmlMsgCfg ) ;
			}
		}
	}

	public void parseConfigCharDef( XMLDocument xmlCharDef )
	{
		m_flagStrictName =
			(xmlCharDef.getAttrIntegerAs( "strict_char_id", 0 ) != 0) ;

		for ( int i = 0; i < xmlCharDef.getElementsCount(); i ++ )
		{
			XMLDocument	xmlChar = xmlCharDef.getElementAt(i) ;
			if ( (xmlChar != null)
				&& (xmlChar.getTag() == "character") )
			{
				StdMessageCompilerListener.CharConfig
					cfgChar = new StdMessageCompilerListener.CharConfig() ;
				cfgChar.parseConfig( xmlChar ) ;
				m_aCharConfigs.add( cfgChar ) ;
			}
		}
	}

	public void parseConfigMessage( XMLDocument xmlMsgCfg )
	{
		m_sNameStarters = xmlMsgCfg.getAttributeAs( "name_start_chars" ) ;
		m_sNameClosers = xmlMsgCfg.getAttributeAs( "name_close_chars" ) ;
		m_sWarnMsgLeadChars = xmlMsgCfg.getAttributeAs( "warn_msg_lead_chars" ) ;
		m_flagMustNameEncloser =
			(xmlMsgCfg.getAttrIntegerAs( "must_name_encloser", 0 ) != 0) ;
		m_nMsgFadeTime =
			xmlMsgCfg.getAttrIntegerAs
				( "msg_window_fade_time", m_nMsgFadeTime ) ;
	}

	// キャラ識別子からキャラ設定取得
	public const CharConfig getCharConfigById( String idChar )
	{
		if ( idChar == null )
		{
			return	null ;
		}
		for ( int i = 0; i < m_aCharConfigs.length(); i ++ )
		{
			if ( m_aCharConfigs[i].m_idChar == idChar )
			{
				return	m_aCharConfigs[i] ;
			}
		}
		return	null ;
	}

	// キャラ名からキャラ設定取得
	public const CharConfig getCharConfigByName( String strName )
	{
		if ( strName == null )
		{
			return	null ;
		}
		for ( int i = 0; i < m_aCharConfigs.length(); i ++ )
		{
			if ( m_aCharConfigs[i].m_strName == strName )
			{
				return	m_aCharConfigs[i] ;
			}
		}
		return	null ;
	}

	// スクリプト開始時処理
	public void onBeginCompile( AGLCompiler compiler, String strScriptFile )
	{
		m_iMsgIndex = 0 ;
	}

	// 空白行処理
	public void onBlankLine( AGLCompiler compiler )
	{
	}

	// 任意構文判定
	public boolean isExtendCommand( AGLCompiler compiler, StringParser line )
	{
		return	parseMessageStatement( compiler, line ) ;
	}

	// メッセージ文修飾
	public void effectMessage
		( HashMap params, String strCharID, String strName )
	{
		StdMessageCompilerListener.CharConfig
				cfgChar = getCharConfigById( strCharID ) ;
		if ( cfgChar == null )
		{
			cfgChar = getCharConfigByName( strName ) ;
		}
		if ( cfgChar != null )
		{
			params["char_idx"] = cfgChar.m_nIndex ;
			//
			if ( (strName == null) || (strName == "") )
			{
				params["name"] = cfgChar.m_strName ;
				params["name_en"] = cfgChar.m_strNameEn ;
			}
		}
		else if ( strCharID != null )
		{
			params["char_id"] = strCharID ;
		}
		else
		{
			params["char_idx"] = "-1" ;
		}
		params["index"] = (String) (m_iMsgIndex ++) ;
		//
		effectEnglishMessage( params ) ;
	}

	// 英語メッセージ設定
	public void setEnglishMessage( HashMap params )
	{
		if ( params["name"] != null )
		{
			m_strNameEn = params["name"] ;
		}
		if ( params["text"] != null )
		{
			m_strMessageEn = params["text"] ;
		}
	}

	// 英語メッセージ反映
	public void effectEnglishMessage( HashMap params )
	{
		if ( m_strNameEn != null )
		{
			params["name_en"] = m_strNameEn ;
			m_strNameEn = null ;
		}
		if ( m_strMessageEn != null )
		{
			params["text_en"] = m_strMessageEn ;
			m_strMessageEn = null ;
		}
	}

	// 選択肢開始
	public void beginSelector()
	{
		m_iMenuIndex = 0 ;
	}

	// 選択肢追加
	public void effectSelectorItem( HashMap params )
	{
		params.value = m_iMenuIndex ++ ;
		params.msg_index = m_iMsgIndex ++ ;
	}

	// コマンド追加時処理
	public void beforeAddCommand( AGLCompiler compiler, XMLDocument cmd )
	{
	}

	// スクリプト終了時処理
	public void onEndCompile( AGLCompiler compiler )
	{
	}


}


