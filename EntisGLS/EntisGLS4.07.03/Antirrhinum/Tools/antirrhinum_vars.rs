
class	AGLVariables
{
	public HashMap	f ;
	public HashMap	s ;

	public AGLVariables()
	{
		this.f = {} ;
		this.s = {} ;
	}

	public void clearAllFlags()
	{
		this.f = {} ;
	}

	public void initSFlag( String name, int value )
	{
		if ( this.s[name] === undefined )
		{
			this.s[name] = value ;
		}
	}

	public int getSFlag( String name )
	{
		if ( this.s[name] === undefined )
		{
			return	0 ;
		}
		return	this.s[name] ;
	}

	public int setSFlag( String name, int value )
	{
		return	this.s[name] = value ;
	}

	public int addSFlag( String name, int value = 1 )
	{
		if ( this.s[name] === undefined )
		{
			this.s[name] = value ;
		}
		else
		{
			this.s[name] += value ;
		}
		return	this.s[name] ;
	}

	public int getDeedFlag( String name )
	{
		if ( this.s[".deed"] === undefined )
		{
			return	0 ;
		}
		if ( this.s[".deed"][name] === undefined )
		{
			return	0 ;
		}
		return	this.s[".deed"][name] ;
	}

	public int addDeedFlag( String name, int value = 1 )
	{
		if ( this.s[".deed"] === undefined )
		{
			this.s[".deed"] = new HashMap<int>() ;
		}
		if ( this.s[".deed"][name] === undefined )
		{
			this.s[".deed"][name] = 0 ;
		}
		this.s[".deed"][name] += value ;
		return	this.s[".deed"][name] ;
	}

}


AGLVariables	g_aglVars = new AGLVariables() ;


