package com.entis.android.entisgls4 ;

import java.net.* ;
import java.util.* ;
import java.io.* ;

public class	HttpFile
{
	protected HttpURLConnection				m_con = null ;
	protected InputStream					m_is = null ;
	protected String						m_url = null ;
	protected String						m_method = null ;
	protected boolean						m_flagPost = false ;
	protected byte[]						m_bufPost = null ;
	protected int							m_statusCode = -1 ;
	protected Map<String, List<String> >	m_mapHeaders = null ;

	// URL 設定
	//////////////////////////////////////////////////////////////////////////
	public boolean setRequest( String strURL, String strMethod )
	{
		try
		{
			URL				url = new URL( strURL ) ;
			URLConnection	con = url.openConnection() ;
			if ( con instanceof HttpURLConnection )
			{
				m_con = (HttpURLConnection) con ;
				m_con.setRequestMethod( strMethod ) ;
				m_url = strURL ;
				m_method = strMethod ;
				if ( (strMethod != null)
					&& (strMethod.compareToIgnoreCase("POST") == 0) )
				{
					m_flagPost = true ;
					m_con.setDoOutput( true ) ;
				}
				m_con.setDoInput( true ) ;
				return	true ;
			}
		}
		catch ( Exception e )
		{
			EntisGLS.logError( e.getMessage() );
		}
		return	false ;
	}
	// ヘッダ追加
	//////////////////////////////////////////////////////////////////////////
	public boolean addHeader( String strField, String strValue )
	{
		if ( m_con != null )
		{
			try
			{
				m_con.addRequestProperty( strField, strValue ) ;
				return	true ;
			}
			catch ( Exception e )
			{
			}
		}
		return	false ;
	}
	// 送信データ設定
	//////////////////////////////////////////////////////////////////////////
	public void setSendData( byte[] buf )
	{
		m_bufPost = buf ;
	}
	// 接続
	//////////////////////////////////////////////////////////////////////////
	public boolean connect()
	{
		if ( m_con != null )
		{
			try
			{
				m_con.connect() ;
				//
				if ( m_flagPost && (m_bufPost != null) )
				{
					OutputStream	os = m_con.getOutputStream() ;
					if ( os != null )
					{
						os.write( m_bufPost ) ;
						m_bufPost = null ;
						os.flush() ;
					}
				}
				m_statusCode = m_con.getResponseCode() ;
				m_mapHeaders = m_con.getHeaderFields() ;
				m_is = m_con.getInputStream() ;
				return	(m_is != null) ;
			}
			catch ( Exception e )
			{
				EntisGLS.logError( e.getMessage() );
			}
		}
		return	false ;
	}
	// 閉じる
	//////////////////////////////////////////////////////////////////////////
	public void close()
	{
		m_is = null ;
		if ( m_con != null )
		{
			try
			{
				m_con.disconnect() ;
			}
			catch ( Exception e )
			{
			}
			m_con = null ;
		}
	}
	// ステータスコード取得
	//////////////////////////////////////////////////////////////////////////
	public int getStatusCode()
	{
		return	m_statusCode ;
	}
	// 受信ヘッダ取得
	//////////////////////////////////////////////////////////////////////////
	public String getReceiveHeader( String strField )
	{
		try
		{
			if ( m_mapHeaders != null )
			{
				List<String>	listHeader = m_mapHeaders.get( strField ) ;
				if ( listHeader != null )
				{
					return	listHeader.get( 0 ) ;
				}
			}
		}
		catch ( Exception e )
		{
		}
		return	null ;
	}
	// 読み込み
	//////////////////////////////////////////////////////////////////////////
	public int read( byte[] buf )
	{
		if ( m_is != null )
		{
			try
			{
				return	m_is.read( buf ) ;
			}
			catch ( Exception e )
			{
			}
		}
		return	0 ;
	}

}

