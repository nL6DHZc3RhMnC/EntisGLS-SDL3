package com.entis.android.entisgls4 ;

import java.util.* ;
import android.view.* ;


public class MenuData
{
	// メニューアイテム情報
	public static class	ItemInfo
	{
		public long			nFlags ;
		public int			nID ;
		public String		strText ;
		public ItemInfo[]	listSubMenu ;
		public MenuItem		menuItem ;
	}
	public static final int		flagChecked			= 0x0001 ;
	public static final int		flagDisabled		= 0x0002 ;
	public static final int		flagCheckItem		= 0x0010 ;
	public static final int		flagRadioItem		= 0x0020 ;
	public static final int		flagSeparator		= 0x1000 ;
	public static final int		flagSubMenu			= 0x2000 ;

	protected boolean		m_flagNewMenu = false ;
	protected ItemInfo[]	m_listMenuInfo = null ;
	protected HashMap<Integer,ItemInfo>
							m_mapItemInfo = new HashMap<Integer,ItemInfo>() ;
	protected int			m_nNextGroup = Menu.FIRST ;

	// アイテム追加
	public void setMenuInfo( ItemInfo[] listMenuInfo )
	{
		m_flagNewMenu = true ;
		m_listMenuInfo = listMenuInfo ;
		//
		m_mapItemInfo = new HashMap<Integer,ItemInfo>() ;
		registerItemInfo( listMenuInfo );
	}
	protected void registerItemInfo( ItemInfo[] listMenuInfo )
	{
		if ( listMenuInfo != null )
		{
			for ( int i = 0; i < listMenuInfo.length; i ++ )
			{
				ItemInfo	item = listMenuInfo[i] ;
				if ( item != null )
				{
					if ( item.nID != 0 )
					{
						m_mapItemInfo.put( item.nID, item ) ;
					}
					if ( (item.nFlags & flagSubMenu) != 0 )
					{
						registerItemInfo( item.listSubMenu ) ;
					}
				}
			}
		}
	}
	// メニューを新規生成する必要があるか？
	public boolean isNewMenu()
	{
		return	m_flagNewMenu ;
	}

	// メニュー生成
	public void createMenu( Menu menu )
	{
		menu.clear() ;
		m_flagNewMenu = false ;
		m_nNextGroup = Menu.FIRST ;
		createSubMenu( menu, m_listMenuInfo ) ;
	}
	protected void createSubMenu( Menu menu, ItemInfo[] listItems )
	{
		if ( listItems != null )
		{
			boolean	flagLastRadio = false ;
			for ( int i = 0; i < listItems.length; i ++ )
			{
				ItemInfo	item = listItems[i] ;
				if ( item != null )
				{
					if ( flagLastRadio
						&& ((item.nFlags & flagRadioItem) == 0) )
					{
						menu.setGroupCheckable( m_nNextGroup, true, true ) ;
						m_nNextGroup ++ ;
						flagLastRadio = false ;
					}
					if ( (item.nFlags & flagSeparator) != 0 )
					{
						m_nNextGroup ++ ;
					}
					else if ( (item.nFlags & flagSubMenu) != 0 )
					{
						SubMenu	submenu = menu.addSubMenu( item.strText ) ;
						createSubMenu( submenu, item.listSubMenu ) ;
					}
					else
					{
						item.menuItem = menu.add
							( m_nNextGroup,
								item.nID, Menu.NONE, item.strText ) ;
						if ( (item.nFlags & flagRadioItem) != 0 )
						{
							flagLastRadio = true ;
							item.menuItem.setCheckable( true ) ;
							item.menuItem.setChecked
								( (item.nFlags & flagChecked) != 0 ) ;
						}
						else if ( (item.nFlags & flagCheckItem) != 0 )
						{
							item.menuItem.setCheckable( true ) ;
							item.menuItem.setChecked
								( (item.nFlags & flagChecked) != 0 ) ;
						}
						if ( (item.nFlags & flagDisabled) != 0 )
						{
							item.menuItem.setEnabled( false ) ;
						}
					}
				}
			}
			if ( flagLastRadio )
			{
				menu.setGroupCheckable( m_nNextGroup, true, true ) ;
				m_nNextGroup ++ ;
			}
		}
	}

	// メニューポップアップ表示
	public void showPopupMenu()
	{
		EntisGLSurfaceView	glsufview = EntisGLS.getMainSurfaceView() ;
		if ( glsufview != null )
		{
			glsufview.showPopupMenu( this ) ;
		}
	}

	// 有効状態変更
	protected static class	EnablerRunnable implements Runnable
	{
		protected ItemInfo	m_item ;
		protected boolean	m_enabled ;

		public EnablerRunnable( ItemInfo item )
		{
			m_item = item ;
		}
		public void run()
		{
			if ( m_item.menuItem != null )
			{
				m_item.menuItem.setEnabled
					( (m_item.nFlags & flagDisabled) == 0 ) ;
			}
		}
	}
	public synchronized boolean enableMenuItem( int id, boolean fEnabled )
	{
		ItemInfo	item = m_mapItemInfo.get( id ) ;
		if ( item != null )
		{
			item.nFlags &= ~flagDisabled ;
			if ( !fEnabled )
			{
				item.nFlags |= flagDisabled ;
			}
			EntisGLS.procedureOnUIThread
				( new EnablerRunnable( item ), false ) ;
			return	true ;
		}
		return	false ;
	}

	// チェック状態変更
	protected static class	CheckerRunnable implements Runnable
	{
		protected ItemInfo	m_item ;

		public CheckerRunnable( ItemInfo item )
		{
			m_item = item ;
		}
		public void run()
		{
			if ( m_item.menuItem != null )
			{
				m_item.menuItem.setChecked
					( (m_item.nFlags & flagChecked) != 0 ) ;
			}
		}
	}
	public synchronized boolean checkMenuItem( int id, boolean fChecked )
	{
		ItemInfo	item = m_mapItemInfo.get( id ) ;
		if ( item != null )
		{
			item.nFlags &= ~flagChecked ;
			if ( fChecked )
			{
				item.nFlags |= flagChecked ;
			}
			if ( fChecked || ((item.nFlags & flagRadioItem) == 0) )
			{
				EntisGLS.procedureOnUIThread
					( new CheckerRunnable( item ), false ) ;
			}
			return	true ;
		}
		return	false ;
	}

	// テキスト変更
	protected static class	TextSetterRunnable implements Runnable
	{
		protected ItemInfo	m_item ;

		public TextSetterRunnable( ItemInfo item )
		{
			m_item = item ;
		}
		public void run()
		{
			if ( m_item.menuItem != null )
			{
				m_item.menuItem.setTitle( m_item.strText ) ;
			}
		}
	}
	public synchronized boolean setMenuItemText( int id, String strText )
	{
		ItemInfo	item = m_mapItemInfo.get( id ) ;
		if ( item != null )
		{
			item.strText = strText ;
			EntisGLS.procedureOnUIThread
				( new TextSetterRunnable( item ), false ) ;
			return	true ;
		}
		return	false ;
	}

}


