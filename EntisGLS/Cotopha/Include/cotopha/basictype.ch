
@IF	!@IsDefined("__GLS_H__")
TypeDef		Error := Integer
DeclareType	Boolean, Int8, Uint8, Int16, Uint16, Int32, Uint32, Int64
DeclareType	File As Class
@ENDIF


Class	Native Integer
Public
	Prototype	void Integer( Integer num )
	Prototype	void Integer( Real num )
	@IF	!@IsDefined("__GLS_H__")
	Prototype	void Integer( String str )
	@ENDIF
	;
	Prototype	Integer operator + ( Integer num ) const
	Prototype	Integer operator - ( Integer num ) const
	Prototype	Integer operator * ( Integer num ) const
	Prototype	Integer operator / ( Integer num ) const
	Prototype	Real operator + ( Real num ) const
	Prototype	Real operator - ( Real num ) const
	Prototype	Real operator * ( Real num ) const
	Prototype	Real operator / ( Real num ) const
	Prototype	Integer operator % ( Integer num ) const
	Prototype	Integer operator & ( Integer num ) const
	Prototype	Integer operator | ( Integer num ) const
	Prototype	Integer operator ^ ( Integer num ) const
	Prototype	Integer operator >> ( Integer num ) const
	Prototype	Integer operator << ( Integer num ) const
	Prototype	Boolean operator == ( Integer num ) const
	Prototype	Boolean operator != ( Integer num ) const
	Prototype	Boolean operator < ( Integer num ) const
	Prototype	Boolean operator <= ( Integer num ) const
	Prototype	Boolean operator > ( Integer num ) const
	Prototype	Boolean operator >= ( Integer num ) const
	Prototype	Boolean operator == ( Real num ) const
	Prototype	Boolean operator != ( Real num ) const
	Prototype	Boolean operator < ( Real num ) const
	Prototype	Boolean operator <= ( Real num ) const
	Prototype	Boolean operator > ( Real num ) const
	Prototype	Boolean operator >= ( Real num ) const
	Prototype	Integer operator + () const
	Prototype	Integer operator - () const
	Prototype	Integer operator ~ () const
	Prototype	Boolean operator ! () const
	Prototype	Integer operator ++ ()
	Prototype	Integer operator -- ()
	Prototype	Integer operator ++ ( Integer )
	Prototype	Integer operator -- ( Integer )
	Prototype	Integer operator := ( Integer num )
	@IF	!@IsDefined("__GLS_H__")
	Prototype	Integer operator := ( const File& file )
	@ENDIF
	Prototype	Integer operator += ( Integer num )
	Prototype	Integer operator -= ( Integer num )
	Prototype	Integer operator *= ( Integer num )
	Prototype	Integer operator /= ( Integer num )
	Prototype	Integer operator %= ( Integer num )
	Prototype	Integer operator &= ( Integer num )
	Prototype	Integer operator |= ( Integer num )
	Prototype	Integer operator ^= ( Integer num )
	Prototype	Integer operator >>= ( Integer num )
	Prototype	Integer operator <<= ( Integer num )
	;
	@IF	!@IsDefined("__GLS_H__")
	Prototype	String Char() const
	Prototype	String Format( Integer nRadix := 0, Integer nCol := 0 ) const
	Prototype	Integer Abs() const
	Prototype	Boolean TestBit( Integer i ) const
	Prototype	Integer SetBit( Integer i )
	Prototype	Integer ResetBit( Integer i )
	Prototype	Integer RotateLeft( Integer i )
	Prototype	Integer RotateRight( Integer i )
	@ENDIF
EndClass



Class	Native Real
Public
	Prototype	void Real( Integer num )
	Prototype	void Real( Real num )
	@IF	!@IsDefined("__GLS_H__")
	Prototype	void Real( String str )
	@ENDIF
	;
	Prototype	Real operator + ( Real num ) const
	Prototype	Real operator - ( Real num ) const
	Prototype	Real operator * ( Real num ) const
	Prototype	Real operator / ( Real num ) const
	Prototype	Real operator + ( Integer num ) const
	Prototype	Real operator - ( Integer num ) const
	Prototype	Real operator * ( Integer num ) const
	Prototype	Real operator / ( Integer num ) const
	Prototype	Boolean operator == ( Real num ) const
	Prototype	Boolean operator != ( Real num ) const
	Prototype	Boolean operator < ( Real num ) const
	Prototype	Boolean operator <= ( Real num ) const
	Prototype	Boolean operator > ( Real num ) const
	Prototype	Boolean operator >= ( Real num ) const
	Prototype	Boolean operator == ( Integer num ) const
	Prototype	Boolean operator != ( Integer num ) const
	Prototype	Boolean operator < ( Integer num ) const
	Prototype	Boolean operator <= ( Integer num ) const
	Prototype	Boolean operator > ( Integer num ) const
	Prototype	Boolean operator >= ( Integer num ) const
	Prototype	Real operator + () const
	Prototype	Real operator - () const
	Prototype	Real operator := ( Real num )
	Prototype	Real operator += ( Real num )
	Prototype	Real operator -= ( Real num )
	Prototype	Real operator *= ( Real num )
	Prototype	Real operator /= ( Real num )
	Prototype	Real operator := ( Integer num )
	Prototype	Real operator += ( Integer num )
	Prototype	Real operator -= ( Integer num )
	Prototype	Real operator *= ( Integer num )
	Prototype	Real operator /= ( Integer num )
	;
	@IF	!@IsDefined("__GLS_H__")
	Prototype	Real Pi() const
	Prototype	Real Abs() const
	Prototype	Real Log( Real r := 10.0 ) const
	Prototype	Real Power( Real r ) const
	Prototype	Real Sqrt() const
	Prototype	Real Sin() const
	Prototype	Real Cos() const
	Prototype	Real Tan() const
	Prototype	Real ASin() const
	Prototype	Real ACos() const
	Prototype	Real ATan() const
	Prototype	Real ATan( Real r ) const
	Prototype	Integer Round() const
	Prototype	Integer Floor() const
	@ENDIF
EndClass


Class	Native String
Public
	Prototype	void String( Integer num )
	Prototype	void String( Real num )
	Prototype	void String( String str )
	;
	Prototype	String operator + ( String str ) const
	Prototype	String operator * ( Integer num ) const
	Prototype	Boolean operator == ( String str ) const
	Prototype	Boolean operator != ( String str ) const
	Prototype	Boolean operator < ( String str ) const
	Prototype	Boolean operator <= ( String str ) const
	Prototype	Boolean operator > ( String str ) const
	Prototype	Boolean operator >= ( String str ) const
	Prototype	String operator := ( String str )
@IF	!@IsDefined("__GLS_H__")
	Prototype	String operator := ( const File& file )
@ENDIF
	Prototype	String operator += ( String str )
@IF	!@IsDefined("__GLS_H__")
	Prototype	String operator += ( const File& file )
@ENDIF
	Prototype	String operator *= ( Integer num )
	;
@IF	!@IsDefined("__GLS_H__")
	Prototype	operator Boolean () const
	;
	Prototype	Integer GetLength() const
	Prototype	Integer Char( Integer i := 0 ) const
	Prototype	Error SetChar( Integer i, Integer c )
	Prototype	Error SetChar( Integer i, String c )
	Prototype	String Left( Integer i ) const
	Prototype	String Right( Integer i ) const
	Prototype	String Middle( Integer i, Integer c := -1 ) const
	Prototype	Error MakeUpper()
	Prototype	Error MakeLower()
	Prototype	Error TrimLeft()
	Prototype	Error TrimRight()
	Prototype	Integer Compare( String sText ) const
	Prototype	Integer CompareNoCase( String sText ) const
	Prototype	Integer CompareLeft( String sText ) const
	Prototype	Integer CompareLeftNoCase( String sText ) const
	Prototype	String OffsetFilePath( String sOffsetPath ) const
	Prototype	String ParseFileDrive() const
	Prototype	String ParseFileDirectory() const
	Prototype	String ParseFileName() const
	Prototype	String ParseFileTitle() const
	Prototype	String ParseFileExtension() const
	;
	Prototype	String GetCEncoded() const
	Prototype	String GetCDecoded() const
	Prototype	String GetXMLEncoded() const
	Prototype	String GetXMLDecoded() const
	;
	Prototype	Integer Find( String s, Integer i := 0 ) const
	Prototype	String Replace(
					String strOld, String strNew, Integer nFlag := 0 ) const
	Prototype	Error Separate( String[]& rArray, String strSep := "" ) const
	@IF	!PLATFORM_ANDROID
	Prototype	Reference Calculate( String& rErrMsg := null ) const
	Prototype	Reference Execute( String& rErrMsg := null ) const
	@ENDIF
	Prototype	String IsMatchUsage(
					String strUsage,
					String[]& aParam := null, Integer& nIndex := null ) const
	Prototype	Integer GetIndex() const
	Prototype	Integer SeekIndex( Integer nIndex )
	Prototype	Boolean SeekNext()
	Prototype	String NextChar()
	Prototype	Integer NextInteger( Integer& fError := null )
	Prototype	Real NextRealNumber( Integer& fError := null )
	Prototype	String NextNeedChar( String sNext )
	Prototype	String NextString()
	Prototype	String NextToken()
	Prototype	String NextToken( Integer& fTokenType )
	Prototype	String NextEnclosedString( String sClose )
	Prototype	String NextMatchUsage(
					String strUsage, String[]& aParam := null )
	Prototype	Reference Call( ... ) const
@ENDIF
	;
	; naked mode extension
	Prototype	Integer GetLength( void ) const naked
	Prototype	const uint16 * GetBuffer( void ) const naked
	Prototype	void SetString(
					const uint16 * pszStr, Integer nStrLen := -1 ) naked
	Prototype	void AppendString(
					const uint16 * pszStr, Integer nStrLen := -1 ) naked
	Prototype	void AppendInteger( Integer nNum, Integer nCol := 0 ) naked
	Prototype	void AppendHexInteger( Integer nNum, Integer nCol := 0 ) naked
	Prototype	void AppendReal( Real rNum, Integer nPrec := 0 ) naked
	Prototype	uint16 * LockBuffer( Integer nSize ) naked
	Prototype	void UnlockBuffer( Integer nStrLen := -1 ) naked
EndClass



@IF	!@IsDefined("__GLS_H__")


Class	Native Array
Public
	Prototype	void Array( const Array & a )
	;
	Prototype	Array operator + ( Reference x ) const
	Prototype	const Array& operator := ( const Array & a )
	Prototype	const Array& operator += ( Reference x )
	;
	Prototype	Integer GetLength() const
	Prototype	Boolean IsEmpty( Integer nIndex ) const
	Prototype	Integer Find( Reference object, Integer nIndex := 0 ) const
	Prototype	Error Swap( Integer i, Integer j )
	Prototype	Integer Push( Reference object )
	Prototype	Reference Pop()
	Prototype	Error Insert( Integer i, Reference object )
	Prototype	Error Remove( Integer i := 0, Integer n := -1 )
	Prototype	Reference Detach( Integer i )
	Prototype	Error Merge( const Array& a )
EndClass



Class	Native Hash
Public
	Prototype	void Hash( const Hash & a )
	;
	Prototype	Hash operator + ( const Hash & a ) const
	Prototype	const Hash& operator := ( const Hash & a )
	Prototype	const Hash& operator += ( const Hash & x )
	;
	Prototype	Integer GetLength() const
	Prototype	Boolean IsEmpty( Integer index ) const
	Prototype	Boolean IsEmpty( String index ) const
	Prototype	String GetTagName( Integer index ) const
	Prototype	Integer FindTagIndex( String tag ) const
	Prototype	Integer OrderIndex( String tag ) const
	Prototype	Reference Detach( String tag )
	Prototype	Error Remove( String tag )
	Prototype	Error RemoveAll()
EndClass



Class	Native Global
Public
	Prototype	Reference operator [] ( Integer index )
	Prototype	Reference operator [] ( String index )
	Prototype	Integer GetLength() const
	Prototype	String GetTagName( Integer nIndex ) const
	Prototype	Boolean IsEmpty( String tag ) const
EndClass


@ENDIF

