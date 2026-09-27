
int main( String[] arg )
{
	for ( int i = 0; i < arg.length(); i ++ )
	{
		MD5DigestContext	md5 = new MD5DigestContext() ;
		Uint8Pointer	bin = arg[i].encodeTo( String.encodingUTF8 ) ;
		md5.stream( bin, bin.getBytes() ) ;
		md5.flush() ;
		System.console().printf( "%s: (MD5) %s\n", arg[i], md5.getDigestHex() ) ;
	}
	return	0 ;
}


