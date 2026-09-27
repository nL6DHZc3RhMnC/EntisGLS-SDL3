package com.entis.android.entisgls4 ;

import java.util.* ;
import	android.graphics.Typeface ;


public class FontManager
{
	// スタイル
	public static final int	styleItalic		= 0x00000001 ;
	public static final int	styleBold		= 0x00000002 ;

	// 定義
	protected static Map<String,Typeface>
						m_mapTypeface = new HashMap<String,Typeface>() ;

	// タイプフェース取得
	//////////////////////////////////////////////////////////////////////////
	public static Typeface getTypeFaceAs( String sFontFace, int nStyle )
	{
		Typeface	tf = m_mapTypeface.get( sFontFace ) ;
		if ( tf != null )
		{
			return	tf ;
		}
		switch ( nStyle )
		{
		case	0:
		default:
			nStyle = Typeface.NORMAL ;
			break ;
		case	styleItalic:
			nStyle = Typeface.ITALIC ;
			break ;
		case	styleBold:
			nStyle = Typeface.BOLD ;
			break ;
		case	(styleItalic | styleBold):
			nStyle = Typeface.BOLD_ITALIC ;
			break ;
		}
		if ( (sFontFace != null)
			&& !sFontFace.equals("") && !sFontFace.equals("Default") )
		{
			tf = Typeface.create( sFontFace, nStyle ) ;
			if ( tf != null )
			{
				return	tf ;
			}
			EntisGLS.logError
				( "failed to create font " + sFontFace + ", " + nStyle ) ;
		}
		switch ( nStyle )
		{
		case	Typeface.BOLD:
		case	Typeface.BOLD_ITALIC:
			tf = Typeface.DEFAULT_BOLD ;
			break ;
		case	Typeface.ITALIC:
			tf = Typeface.SERIF ;
			break ;
		case	Typeface.NORMAL:
		default:
			tf = Typeface.DEFAULT ;
			break; 
		}
		return	tf ;
	} ;

	// フォント追加
	//////////////////////////////////////////////////////////////////////////
	public static void addTypeFaceAs( String sFontFace, Typeface tfFace )
	{
		if ( tfFace != null )
		{
			m_mapTypeface.put( sFontFace, tfFace ) ;
		}
	}
	public static boolean addFontFromAsset( String sFontFace, String sPath )
	{
		Typeface	tf =
			Typeface.createFromAsset
				( EntisGLS.getActivity().getAssets(), sPath ) ;
		if ( tf != null )
		{
			addTypeFaceAs( sFontFace, tf ) ;
			return	true ;
		}
		return	false ;
	}

}

