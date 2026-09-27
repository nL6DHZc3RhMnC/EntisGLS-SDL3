
; ----------------------------------------------------------------------------
;	一般的な定数
; ----------------------------------------------------------------------------

; エラーコード
Constant	eslErrSuccess		:= 0,
			eslErrNotSupported	:= -1,
			eslErrGeneral		:= 1,
			eslErrFailed		:= 1,
			eslErrAbort			:= 2,
			eslErrInvalidParam	:= 3,
			eslErrTimeout		:= 4,
			eslErrPending		:= 5,
			eslErrContinue		:= 6

; タイムアウト（永遠に）
Constant	INFINITE := -1

; ヌル
Constant	NULL := 0

; システム定義コマンドID
Enumerator	SysCommandId<String>
	AppExit		:= "ID_APP_EXIT"
	AppBack		:= "ID_APP_BACK"
	AppSuspend	:= "ID_APP_SUSPEND"
EndEnum

; 円周率
Constant	PI := 3.1415926535897932384626433832795


; ----------------------------------------------------------------------------
;	構造体の定義
; ----------------------------------------------------------------------------

DeclareType	Size As Structure

; 座標
Structure	Point
	Integer	x
	Integer	y

	Prototype	void Point( Integer x, Integer y )
	Prototype	void Point( const Point& pos )
	Prototype	void Point( const Size& size )
	Prototype	Point operator + ( const Point& pos ) const
	Prototype	Point operator - ( const Point& pos ) const
	Prototype	const Point& operator += ( const Point& pos )
	Prototype	const Point& operator -= ( const Point& pos )
EndStruct

; サイズ
Structure	Size
	Integer	w
	Integer	h

	Prototype	void Size( Integer w, Integer h )
	Prototype	void Size( const Size& size )
	Prototype	void Size( const Point& pos )
	Prototype	Size operator + ( const Size& size ) const
	Prototype	Size operator - ( const Size& size ) const
	Prototype	const Size& operator += ( const Size& size )
	Prototype	const Size& operator -= ( const Size& size )
EndStruct

; 矩形
Structure	Rect
	Integer	left
	Integer	top
	Integer	right
	Integer	bottom

	Prototype	void Rect( Integer x0, Integer y0, Integer x1, Integer y1 )
	Prototype	void Rect( const Rect& rect )
	Prototype	void Rect( const Point& pos, const Size& size )
	Prototype	Rect operator + ( const Point& pos ) const
	Prototype	Rect operator - ( const Point& pos ) const
	Prototype	const Rect& operator += ( const Point& pos )
	Prototype	const Rect& operator -= ( const Point& pos )
EndStruct


; スプライト情報
Structure	SpriteParam
	Integer		nFlags					; 描画フラグ
	Point		ptDstPos				; 出力基準座標
	Point		ptRevCenter				; 回転中心座標
	Real		rHorzUnit := 1.0		; ｘ軸拡大率
	Real		rVertUnit := 1.0		; ｙ軸拡大率
	Real		rRevAngle				; 回転角度 [deg]
	Real		rCrossingAngle := 90.0	; ｘｙ軸交差角 [deg]
	Integer		rgbDimColor				; α描画色
	Integer		rgbLightColor := 00FFFFFFH
	Integer		nTransparency			; 透明度
	Real		rZOrder					; ｚ値
	Integer		rgbColorParam1			; 色パラメータ
	Real		rZScale					; ｚスケール
EndStruct

; 描画フラグ
Constant	EGL_DRAW_BLEND_ALPHA	:= 0001H
Constant	EGL_DRAW_GLOW_LIGHT		:= 0002H
Constant	EGL_WITH_Z_ORDER		:= 0004H
Constant	EGL_SMOOTH_STRETCH		:= 0010H
Constant	EGL_UNSMOOTH_STRETCH	:= 0020H
Constant	EGL_FIXED_POSITION		:= 0040H
Constant	EGL_PARAM_Z_SCALE		:= 8000H
Constant	EGL_MULTI_THREAD		:= 4000H

Constant	EGL_APPLY_C_ADD			:= 00800000H
Constant	EGL_APPLY_C_MUL			:= 00820000H
Constant	EGL_APPLY_A_MUL			:= 00880000H
Constant	EGL_APPLY_C_MASK		:= 00890000H
Constant	EGL_DRAW_F_ADD			:= 80000000H
Constant	EGL_DRAW_F_SUB			:= 81000000H
Constant	EGL_DRAW_F_MUL			:= 82000000H
Constant	EGL_DRAW_F_DIV			:= 83000000H
Constant	EGL_DRAW_F_MAX			:= 84000000H
Constant	EGL_DRAW_F_MIN			:= 85000000H
Constant	EGL_DRAW_F_SCREEN		:= 86000000H
Constant	EGL_DRAW_A_MOVE			:= 88000000H
Constant	EGL_DRAW_A_MUL			:= 89000000H
Constant	EGL_DRAW_DST_A_MASK		:= 8A000000H


; ステレオ立体視情報
Structure	View3DInfo
	Real	zFocus						; 焦点距離
	Real	xParallax					; 視差
	Real	zOffset						; z 座標オフセット
EndStruct


; 画像リソース情報
Structure	ImageInfo
	Integer	nFormatType
	Integer	nImageWidth
	Integer	nImageHeight
	Integer	nBitsPerPixel	; 通常 32 固定
	Integer	nFrameCount		; 静止画の場合 1
	Integer	xHotSpot
	Integer	yHotSpot
	Integer	nResourceBytes
EndStruct

; 音声リソース情報
Structure	SoundInfo
	Integer	nSampleCount
	Integer	nSamplesPerSec
	Integer	nChannelCount
	Integer	nBitsPerSample
	Integer	nRewoundPosition
	Integer	nResourceBytes
EndStruct

; 画像フォーマットフラグ
Constant	EIF_RGB_BITMAP		:= 00000001H
Constant	EIF_RGBA_BITMAP		:= 04000001H
Constant	EIF_GRAY_BITMAP		:= 00000002H
Constant	EIF_YUV_BITMAP		:= 00000004H
Constant	EIF_HSB_BITMAP		:= 00000006H
Constant	EIF_Z_BUFFER_R4		:= 00002005H
Constant	EIF_TYPE_MASK		:= 00FFFFFFH
Constant	EIF_WITH_PALETTE	:= 01000000H
Constant	EIF_WITH_CLIPPING	:= 02000000H
Constant	EIF_WITH_ALPHA		:= 04000000H


; スキンコマンド
Structure	WndSpriteCmd
	String	strID			; アイテム識別子
	String	strFullID
	Integer	nNotification	; 通知コード
	Integer	nParameter		; パラメータ

	Enumerator	Priority<Integer>
		Lowest		:= -10
		Low			:= -5
		Normal		:= 0
		High		:= 5
		Highest		:= 10
		Critical	:= 7FFFFFFFH
	EndEnum
EndStruct


; 入力イベント
Structure	InputEvent
	Integer	idType		; デバイスタイプ
	Integer	iDevNum		; デバイス番号（ジョイスティックのみ）
	Integer	iKeyNum		; 仮想キーコード／仮想ジョイスティックボタン
	String	strCommand	; コマンドID
EndStruct

; デバイスタイプ
Constant	idKeyboard		:= 0,
			idMouse			:= 1,
			idJoyStick		:= 2,
			idCommand		:= 3,
			idSignalCommand	:= 4

; 仮想キーコード
; '0'～'9'、'A'～'Z' は ASCII コードと同じ
Constant	VK_LBUTTON        := 01H
Constant	VK_RBUTTON        := 02H
Constant	VK_CANCEL         := 03H
Constant	VK_MBUTTON        := 04H
Constant	VK_BACK           := 08H
Constant	VK_TAB            := 09H
Constant	VK_CLEAR          := 0CH
Constant	VK_RETURN         := 0DH
Constant	VK_SHIFT          := 10H
Constant	VK_CONTROL        := 11H
Constant	VK_MENU           := 12H
Constant	VK_PAUSE          := 13H
Constant	VK_CAPITAL        := 14H
Constant	VK_KANA           := 15H
Constant	VK_HANGEUL        := 15H
Constant	VK_HANGUL         := 15H
Constant	VK_JUNJA          := 17H
Constant	VK_FINAL          := 18H
Constant	VK_HANJA          := 19H
Constant	VK_KANJI          := 19H
Constant	VK_ESCAPE         := 1BH
Constant	VK_CONVERT        := 1CH
Constant	VK_NONCONVERT     := 1DH
Constant	VK_ACCEPT         := 1EH
Constant	VK_MODECHANGE     := 1FH
Constant	VK_SPACE          := 20H
Constant	VK_PRIOR          := 21H
Constant	VK_NEXT           := 22H
Constant	VK_END            := 23H
Constant	VK_HOME           := 24H
Constant	VK_LEFT           := 25H
Constant	VK_UP             := 26H
Constant	VK_RIGHT          := 27H
Constant	VK_DOWN           := 28H
Constant	VK_SELECT         := 29H
Constant	VK_PRINT          := 2AH
Constant	VK_EXECUTE        := 2BH
Constant	VK_SNAPSHOT       := 2CH
Constant	VK_INSERT         := 2DH
Constant	VK_DELETE         := 2EH
Constant	VK_HELP           := 2FH
Constant	VK_LWIN           := 5BH
Constant	VK_RWIN           := 5CH
Constant	VK_APPS           := 5DH
Constant	VK_SLEEP          := 5FH
Constant	VK_NUMPAD0        := 60H
Constant	VK_NUMPAD1        := 61H
Constant	VK_NUMPAD2        := 62H
Constant	VK_NUMPAD3        := 63H
Constant	VK_NUMPAD4        := 64H
Constant	VK_NUMPAD5        := 65H
Constant	VK_NUMPAD6        := 66H
Constant	VK_NUMPAD7        := 67H
Constant	VK_NUMPAD8        := 68H
Constant	VK_NUMPAD9        := 69H
Constant	VK_MULTIPLY       := 6AH
Constant	VK_ADD            := 6BH
Constant	VK_SEPARATOR      := 6CH
Constant	VK_SUBTRACT       := 6DH
Constant	VK_DECIMAL        := 6EH
Constant	VK_DIVIDE         := 6FH
Constant	VK_F1             := 70H
Constant	VK_F2             := 71H
Constant	VK_F3             := 72H
Constant	VK_F4             := 73H
Constant	VK_F5             := 74H
Constant	VK_F6             := 75H
Constant	VK_F7             := 76H
Constant	VK_F8             := 77H
Constant	VK_F9             := 78H
Constant	VK_F10            := 79H
Constant	VK_F11            := 7AH
Constant	VK_F12            := 7BH
Constant	VK_F13            := 7CH
Constant	VK_F14            := 7DH
Constant	VK_F15            := 7EH
Constant	VK_F16            := 7FH
Constant	VK_F17            := 80H
Constant	VK_F18            := 81H
Constant	VK_F19            := 82H
Constant	VK_F20            := 83H
Constant	VK_F21            := 84H
Constant	VK_F22            := 85H
Constant	VK_F23            := 86H
Constant	VK_F24            := 87H

; コンテキストキーマスク
Constant	ckmShift		:= 1000H,
			ckmControl		:= 2000H,
			ckmMenu			:= 4000H,
			ckmCaptal		:= 8000H,
			ckmKeyCodeMask	:= 00FFH

; 物理ジョイスティックデバイスID
Constant	joyStickId1		:= 0,
			joyStickId2		:= 1,
			joyStickXInput1	:= 2,
			joyStickXInput2	:= 3,
			joyStickXInput3	:= 4,
			joyStickXInput4	:= 6

; 仮想ジョイスティックボタン
Constant	jbUp		:= 0,
			jbDown		:= 1,
			jbLeft		:= 2,
			jbRight		:= 3,
			jbButton1	:= 4,
			jbButton2	:= 5,
			jbButton3	:= 6,
			jbButton4	:= 7,
			jbButtonMax	:= jbButton1 + 32

; XInput ボタン番号
Constant	xbuttonDPadUp			:= 0,
			xbuttonDPadDown			:= 1,
			xbuttonDPadLeft			:= 2,
			xbuttonDPadRight		:= 3,
			xbuttonStart			:= 4,
			xbuttonBack				:= 5,
			xbuttonLeftThumb		:= 6,
			xbuttonRightThumb		:= 7,
			xbuttonLeftShoulder		:= 8,
			xbuttonRightShoulder	:= 9,
			xbuttonA				:= 12,
			xbuttonB				:= 13,
			xbuttonX				:= 14,
			xbuttonY				:= 15,
			xbuttonLeftTrigger		:= 16,
			xbuttonRightTrigger		:= 17



;  2次元ベクトル
Structure	Vector2D
	float	x
	float	y

	Prototype	void Vector2D( Real x, Real y )
	Prototype	void Vector2D( const Vector2D& vec )
	Prototype	Vector2D operator + ( const Vector2D& vec ) const
	Prototype	Vector2D operator - ( const Vector2D& vec ) const
	Prototype	Vector2D operator * ( Real r ) const
	Prototype	Vector2D operator / ( Real r ) const
	Prototype	const Vector2D& operator += ( const Vector2D& vec )
	Prototype	const Vector2D& operator -= ( const Vector2D& vec )
	Prototype	const Vector2D& operator *= ( Real r )
	Prototype	const Vector2D& operator /= ( Real r )
	Prototype	Vector2D Rotate( Real r ) const
	Prototype	Vector2D Revolve( Real r ) const
	Prototype	Real Abs() const
	Prototype	void Normalize()
	Prototype	void RoundTo1()
EndStruct

; 3 次元ベクトル
Structure	Vector
	float	x
	float	y
	float	z

	Prototype	void Vector( Real x, Real y, Real z )
	Prototype	void Vector( const Vector& vec )
	Prototype	Vector operator + ( const Vector& vec ) const
	Prototype	Vector operator - ( const Vector& vec ) const
	Prototype	Vector operator * ( Real r ) const
	Prototype	Vector operator / ( Real r ) const
	Prototype	const Vector& operator += ( const Vector& vec )
	Prototype	const Vector& operator -= ( const Vector& vec )
	Prototype	const Vector& operator *= ( Real r )
	Prototype	const Vector& operator /= ( Real r )
	Prototype	Real Abs() const
	Prototype	void Normalize()
	Prototype	void RoundTo1()
EndStruct

; 4 元数
Structure	Quaternion
	float	q0, q1, q2, q3

	Prototype	void Quaternion( Real q0, Real q1, Real q2, Real q3 )
	Prototype	void Quaternion( const Quaternion& q )
	Prototype	Quaternion operator + ( const Quaternion& q ) const
	Prototype	Quaternion operator - ( const Quaternion& q ) const
	Prototype	Quaternion operator * ( Real r ) const
	Prototype	const Quaternion& operator += ( const Quaternion& q )
	Prototype	const Quaternion& operator -= ( const Quaternion& q )
	Prototype	const Quaternion& operator *= ( Real r )
	Prototype	void Normalize()
EndStruct

; ベジェ曲線
Structure	Bezier1D
	Real[]	m_cp

	Prototype	void Bezier1D( const Bezier1D & bzSrc )
	Prototype	Integer operator sizeof ( void ) const
	Prototype	operator const Real[]&( void ) const
	Prototype	operator Real[]&( void )
	Prototype	Real& operator [] ( Integer nIndex )
	Prototype	void SetLinear( Real r0, Real r1 )
	Prototype	void SetAcceleration( Real a0, Real a1 )
	Prototype	void SetLine( Real p0, Real p1, Real v0, Real v1 )
	Prototype	void AddLine( Real p1, Real v0, Real v1 )
	Prototype	void SetCurve(
						Real p0, Real p1, Real p2, Real v0, Real v1, Real v2 )
	Prototype	Real pt( Real t, Integer base := 0 ) const
	Prototype	void DivideBezier( Real t, Integer base := 0 )
EndStruct

Structure	Bezier2D
	Vector2D[]	m_cp

	Prototype	void Bezier2D( const Bezier2D & bzSrc )
	Prototype	Integer operator sizeof ( void ) const
	Prototype	operator const Vector2D[]&( void ) const
	Prototype	operator Vector2D[]&( void )
	Prototype	Vector2D& operator [] ( Integer nIndex )
	Prototype	void SetLinear( Real x0, Real y0, Real x1, Real y1 )
	Prototype	void SetAcceleration( Real a0, Real a1 )
	Prototype	void SetLine(
					Real x0, Real y0, Real x1, Real y1, Real v0, Real v1 )
	Prototype	void AddLine( Real x1, Real y1, Real v0, Real v1 )
	Prototype	void SetCurve(
					Real x0, Real y0, Real x1, Real y1,
					Real x2, Real y2, Real v0, Real v1, Real v2 )
	Prototype	Vector2D pt( Real t, Integer base := 0 ) const
	Prototype	void DivideBezier( Real t, Integer base := 0 )
EndStruct

Structure	Bezier3D
	Vector[]	m_cp

	Prototype	void Bezier3D( const Bezier3D & bzSrc )
	Prototype	Integer operator sizeof ( void ) const
	Prototype	operator const Vector[]&( void ) const
	Prototype	operator Vector[]&( void )
	Prototype	Vector& operator [] ( Integer nIndex )
	Prototype	void SetLinear(
					Real x0, Real y0, Real z0, Real x1, Real y1, Real z1 )
	Prototype	void SetAcceleration( Real a0, Real a1 )
	Prototype	void SetLine(
					Real x0, Real y0, Real z0,
					Real x1, Real y1, Real z1, Real v0, Real v1 )
	Prototype	void AddLine( Real x1, Real y1, Real z1, Real v0, Real v1 )
	Prototype	void SetCurve(
					Real x0, Real y0, Real z0,
					Real x1, Real y1, Real z1,
					Real x2, Real y2, Real z2,
					Real v0, Real v1, Real v2 )
	Prototype	Vector pt( Real t, Integer base := 0 ) const
	Prototype	void DivideBezier( Real t, Integer base := 0 )
EndStruct

Structure	Bezier4D
	Quaternion[]	m_cp

	Prototype	void Bezier4D( const Bezier4D & bzSrc )
	Prototype	Integer operator sizeof ( void ) const
	Prototype	operator const Quaternion[]&( void ) const
	Prototype	operator Quaternion[]&( void )
	Prototype	Quaternion& operator [] ( Integer nIndex )
	Prototype	void SetLinear(
					const Quaternion& q0, const Quaternion& q1 )
	Prototype	void SetAcceleration( Real a0, Real a1 )
	Prototype	void SetLine(
					const Quaternion& q0,
					const Quaternion& q1, Real v0, Real v1 )
	Prototype	void SetCurve(
					const Quaternion& q0, const Quaternion& q1,
					const Quaternion& q2, Real v0, Real v1, Real v2 )
	Prototype	Quaternion pt( Real t, Integer base := 0 ) const
	Prototype	void DivideBezier( Real t, Integer base := 0 )
EndStruct

; 表面属性
Structure	SurfaceAttribute
	Integer		nShadingFlags
	Integer		rgbColorMul := 0FFFFFFH
	Integer		rgbColorAdd
	Integer		nAmbient
	Integer		nDiffusion
	Integer		nSpecular
	Integer		nSpecularSize
	Integer		nTransparency
	Integer		nDeepness
	Integer		rgbShadeMul
	Integer		rgbShadeAdd
	Integer		nReflection
	Real		nRefraction
EndStruct

; 色情報
Structure	E3DColor
	uint32	rgbMul := 0FFFFFFH
	uint32	rgbAdd
EndStruct

; 光源
Structure	LightEntry
	uint32		nLightType
	uint32		rgbColor
	float		rBrightness
	float		rFogDeepness
	float		rFogDistance
	Vector		vecLight
EndStruct

Constant	E3DSAF_NO_SHADING		:= 00000000H,	; シェーディング無し
			E3DSAF_FLAT_SHADE		:= 00000001H,	; フラットシェーディング（未使用）
			E3DSAF_GOURAUD_SHADE	:= 00000002H,	; グーローシェーディング
			E3DSAF_PHONG_SHADE		:= 00000003H,	; フォンシェーディング（未使用）
			E3DSAF_PHONG_SHADE_BEFORE_TEXTURE	:= 00000006H,
			E3DSAF_RAY_SHADOWING	:= 00000010H,	; 陰を落とす (ver.3.09 以降)
			E3DSAF_RAY_REFLECTING	:= 00000020H,	; 反射を有効にする (ver.3.09 以降)
			E3DSAF_RAY_REFRACTING	:= 00000040H,	; 屈折を有効にする (ver.3.09 以降)
			E3DSAF_RAY_TRACING		:= 00000074H,	; レイトレーシング (ver.3.09 以降)
			E3DSAF_RAY_TRACING_BEFORE_TEXTURE	:= 00000076H,
			E3DSAF_SHADING_MASK		:= 000000FFH,
			E3DSAF_TEXTURE_TILING	:= 00000100H,	; テクスチャをタイリング
			E3DSAF_TEXTURE_TRIM		:= 00000200H,	; トリミングテクスチャ
			E3DSAF_TEXTURE_SMOOTH	:= 00000400H,	; テクスチャ補完拡大
			E3DSAF_TEXTURE_MAPPING	:= 00001000H,	; テクスチャマッピング
			E3DSAF_ENVIRONMENT_MAP	:= 00002000H,	; 環境マッピング
			E3DSAF_GENVIRONMENT_MAP	:= 00004000H,	; グローバル環境マッピング
			E3DSAF_SINGLE_SIDE_PLANE:= 00010000H,	; 片面ポリゴン
			E3DSAF_NO_ZBUFFER		:= 00020000H,	; ｚ比較を行わないで描画
			E3DSAF_ZBUF_ONLY_COMPARE:= 00040000H,	; ｚ比較のみ（書き込まない）			E3DSAF_NO_SHADOW_OBJECT	:= 01000000H,	; 他のオブジェクトに陰を落とさない（レイトレーシングモードのみ）
			E3DSAF_NO_SALF_SHADOW	:= 02000000H,	; 自分自身のメッシュに陰を落とさない（レイトレーシングモードのみ）
			E3DSAF_NO_REFLECT_OBJECT:= 04000000H,	; 他のオブジェクトに映りこまない（レイトレーシングモードのみ）
			E3DSAF_GLOBAL_REFLECT_OBJECT	:= 08000000H	; 距離に関係なく他のオブジェクトに映りこむ（レイトレーシングモードのみ）


Constant	E3D_FOG_LIGHT		:= 00000000H,		; 擬似フォッグ
			E3D_AMBIENT_LIGHT	:= 00000001H,		; 環境光
			E3D_VECTOR_LIGHT	:= 00000002H,		; 無限遠光源
			E3D_POINT_LIGHT		:= 00000004H		; 点光源

; レンダリング機能フラグ
Constant	E3D_FLAG_ANTIALIAS_SIDE_EDGE	:= 0001H,
			E3D_FLAG_TEXTURE_SMOOTHING		:= 0002H,
			E3D_FLAG_PHONG_SHADING			:= 0004H,
			E3D_FLAG_RAY_SHADOWING			:= 0010H,
			E3D_FLAG_RAY_REFLECTING			:= 0020H,
			E3D_FLAG_RAY_REFRACTING			:= 0040H,
			E3D_FLAG_RAY_TRACING			:= 0074H,
			E3D_FLAG_ENABLE_SSE2			:= 0100H,
			E3D_FLAG_OPENGL_SHADING			:= 10000000H

; ソートフラグ
Constant	E3D_SORT_TRANSPARENT	:= 0001H,
			E3D_SORT_OPAQUE			:= 0002H


; 時刻
Structure	Time
	Integer	nYear
	Integer	nMonth
	Integer	nDay
	Integer	nWeek
	Integer	nHour
	Integer	nMinute
	Integer	nSecond

	Prototype	Integer Compare( const Time & time ) const
EndStruct


; メモリステータス
Structure	MemoryStatus
	Integer	nTotalPhys
	Integer	nAvailPhys
	Integer	nTotalVirtual
	Integer	nAvailVirtual
EndStruct


; 文字描画パラメータ
Structure	DrawTextParam
	Rect	rcArea
	Point	ptCurPos
	Integer	nFlags
	Integer	rgbColor
	Integer	nTransparency
	Integer	nLineHeight
	Integer	nIndentWidth
	Integer	nFontSize
	String	strFontFace
EndStruct

Constant	DTPF_VERTICAL		:= 0001H	; 縦書き
Constant	DTPF_NOSMOOTHING	:= 0002H	; アンチエイリアス無効化
Constant	DTPF_LEFT			:= 0000H	; 左寄せ（デフォルト）
Constant	DTPF_CENTER			:= 0010H	; 中央寄せ（単一行）
Constant	DTPF_RIGHT			:= 0020H	; 右寄せ（単一行）
Constant	DTPF_ACCORDING		:= 0030H	; 左右の幅調整（単一行）
Constant	DTPF_ALIGN_MASK		:= 0030H


; エフェクトパラメータ
Structure	EffectParam
	String	strType
	Integer	nFlags
	Integer	nInterval, nDegreeStep
	Integer	nShakingWidth, nMeshSize, nMeshDivision, nFrequency
	Size	sizeView
	Point	ptSpeed
	Integer	nAlphaRange, nMilliSecPerDegree
	Point	ptSmashPoint
	Real	rSmashDelay, rSmashPower, rRandomPower, rDeceleration
	Vector	vVelocity, vGravity, vRevSpeed, vRevRandom
EndStruct

Constant	effTransition := 0001H

; パーティクルパラメータ
Structure	ParticleFlick
	Real	rAmplitude			; 揺らぎ幅 [pixel]
	Real	rAmplitudeRange		; 揺らぎ幅（乱数） [pixel]
	Real	rFrequency			; 揺らぎ周期 [sec]
	Real	rFrequencyRange		; 揺らぎ周期（乱数） [sec]
EndStruct

Structure	ParticleParam
	Integer		nFlags				; フラグ
	Integer		nDuration			; 寿命 [ms]
	Integer		nAnimationSpeed := 100H	; アニメーション速度比 x100H
									;（アニメ画像パーティクル用）
	Integer		nFadein				; フェードイン時間 [ms]
	Integer		nFadeout			; フェードアウト時間 [ms]
	Integer		nFadeTransparency	; フェードアウト透明度
	Real		rFadeZoom := 1.0	; フェードアウト時の拡大比率
	Real		rGenWidth			; 発生幅 [pixel]
	Real		rGenHeight
	Real		rGenAngle			; 発生角 [deg]
	Real		rGenAngleRange		; 発生角の幅（乱数） [deg]
	Real		rGenVelocity		; （中心から遠ざかる）初速 [pixel/sec]
	Real		rGenVelocityRange	; 初速の幅（乱数） [pixel/sec]
	Real		rShrink				; 初速減速率 [/sec]
	Real		rRevSpeed			; 回転速度 [deg/sec]
	Real		rRevSpeedRange		; 回転速度の幅（乱数）[deg/sec]
	Real		rZoom := 1.0			; 拡大率
	Real		rZoomRange			; 拡大率の幅（乱数）]
	ParticleFlick[2] pfFlickness	; 揺らぎ
	Vector2D	vGenSpeed			; 初速ベクトル [pixel/sec]
	Real		rGenSpeedRange		; 初速の幅（乱数）[pixel/sec]
	Vector2D	vStream				; 流速 [pixel/sec]
	Vector2D	vGravity			; 重力加速度 [pixel/sec/sec]
EndStruct

Constant	pfAnimationLoop := 01H

Structure	ParticleParam3D
	Integer		nFlags				; フラグ
	Integer		nDuration			; 寿命 [ms]
	Integer		nAnimationSpeed := 100H		; アニメーション速度比 x100H
									;（アニメ画像パーティクル用）
	Integer		nFadein				; フェードイン時間 [ms]
	Integer		nFadeout			; フェードアウト時間 [ms]
	Integer		nFadeTransparency	; フェードアウト透明度
	Real		rFadeZoom := 1.0	; フェードアウト時の拡大比率
	Vector		vGenWidth			; 発生幅
	Vector		vGenAngle			; 発生角 [deg]
	Real		rGenAngleRange		; 発生角の幅（乱数） [deg]
	Real		rGenVelocity		; （中心から遠ざかる）初速 [pixel/sec]
	Real		rGenVelocityRange	; 初速の幅（乱数） [pixel/sec]
	Real		rShrink				; 初速減速率 [/sec]
	Vector		vRevBaseAxis		; 回転軸
	Real		rRevSpeed			; 回転速度 [deg/sec]
	Real		rRevSpeedRange		; 回転速度の幅（乱数）[deg/sec]
	Vector		vRevRevAxis			; 回転軸を回転させる基底（ランダム用）
	Real		rRevRevRange		; 回転軸の回転幅（乱数）[deg]
	Real		rZoom := 1.0			; 拡大率
	Real		rZoomRange			; 拡大率の幅（乱数）
	ParticleFlick[2] pfFlickness	; 揺らぎ
	Vector		vGenSpeed			; 初速ベクトル [pixel/sec]
	Real		rGenSpeedRange		; 初速の幅（乱数）[pixel/sec]
	Vector		vStream				; 流速 [pixel/sec]
	Vector		vGravity			; 重力加速度 [pixel/sec/sec]
EndStruct

; レイトレーシング情報
Structure	RenderRayTraceParam
	Integer		nFlags					; フラグ
	Real		rShadowingDistance		; 影を落とす有効距離
	Real		rRayTracingDistance		; 光線追跡有効距離
	Integer		nRayReflectCount		; 光線追跡回数
EndStruct

Constant	E3D_RAYTRACE_SHADOWING			:= 0001H	; 影を落とす
Constant	E3D_RAYTRACE_SHADOW_ALPHA		:= 0002H	; 影に透明度を考慮する
Constant	E3D_RAYTRACE_REFLECTION			:= 0010H	; 反射を有効にする
Constant	E3D_RAYTRACE_REFRACTION			:= 0020H	; 屈折を有効にする
Constant	E3D_RAYTRACE_ONLY_GLOBAL_REF	:= 0040H	; 大域反射屈折のみ有効


; アンインストール情報
Structure	UninstallInfo
	String	strDisplayName
	String	strDisplayIcon
	String	strUninstallCmdLine
	String	strUninstallPath
	String	strInstallLocation
	String	strPublisher
	String	strVersionMajor
	String	strVersionMinor
EndStruct


; ----------------------------------------------------------------------------
;	算術関数
; ----------------------------------------------------------------------------

Prototype	Real fabs( Real x )
Prototype	Real log( Real x )
Prototype	Real log10( Real x )
Prototype	Real pow( Real x, Real y )
Prototype	Real sqrt( Real x )
Prototype	Real sin( Real x )
Prototype	Real cos( Real x )
Prototype	Real tan( Real x )
Prototype	Real asin( Real x )
Prototype	Real acos( Real x )
Prototype	Real atan( Real x )
Prototype	Real atan2( Real x, Real y )
Prototype	Integer round( Real x )
Prototype	Integer floor( Real x )

Prototype	Real fabs( Real x ) naked native
Prototype	Real log( Real x ) naked native
Prototype	Real log10( Real x ) naked native
Prototype	Real pow( Real x, Real y ) naked native
Prototype	Real sqrt( Real x ) naked native
Prototype	Real sin( Real x ) naked native
Prototype	Real cos( Real x ) naked native
Prototype	Real tan( Real x ) naked native
Prototype	Real asin( Real x ) naked native
Prototype	Real acos( Real x ) naked native
Prototype	Real atan( Real x ) naked native
Prototype	Real atan2( Real x, Real y ) naked native
Prototype	Integer round( Real x ) naked native
Prototype	Integer floor( Real x ) naked native



; ----------------------------------------------------------------------------
;	スレッドプロシージャ基底クラスの定義
; ----------------------------------------------------------------------------

Class	ThreadProcedure
Public
	Prototype	virtual Reference Run() abstract
	Prototype	virtual Integer ExceptionHandler( String sErrorMessage ) abstract
EndClass


; ----------------------------------------------------------------------------
;	Sprite インターフェース
; ----------------------------------------------------------------------------

DeclareType	Sprite As Class

Class	SpriteHitTestProcedure
Public
	Prototype	virtual Boolean IsHitSprite(
					Sprite& sprite, Integer xPos, Integer yPos ) abstract
EndClass

Class	SpriteTimerProcedure
Public
	Prototype	virtual void OnTimer(
					Sprite& sprite, Integer nPastTime ) abstract
EndClass

Class	SpriteMouseInterface
Public
	Prototype	virtual Boolean OnMouseMove(
					Sprite& sprite, Integer xPos, Integer yPos ) abstract
	Prototype	virtual void OnMouseLeave( Sprite& sprite ) abstract
	Prototype	virtual Boolean OnMouseWheel(
					Sprite& sprite,
					Integer zDelta, Integer xPos, Integer yPos ) abstract
	Prototype	virtual Boolean OnLButtonDown(
					Sprite& sprite, Integer xPos, Integer yPos ) abstract
	Prototype	virtual Boolean OnLButtonUp(
					Sprite& sprite, Integer xPos, Integer yPos ) abstract
	Prototype	virtual Boolean OnLButtonDblClk(
					Sprite& sprite, Integer xPos, Integer yPos ) abstract
	Prototype	virtual Boolean OnRButtonDown(
					Sprite& sprite, Integer xPos, Integer yPos ) abstract
	Prototype	virtual Boolean OnRButtonUp(
					Sprite& sprite, Integer xPos, Integer yPos ) abstract
	Prototype	virtual Boolean OnRButtonDblClk(
					Sprite& sprite, Integer xPos, Integer yPos ) abstract
EndClass

Class	SpriteKeyInterface
Public
	Prototype	virtual Boolean OnKeyDown(
					Sprite& sprite, Integer nVirtKey ) abstract
	Prototype	virtual Boolean OnKeyUp(
					Sprite& sprite, Integer nVirtKey ) abstract
EndClass



; ----------------------------------------------------------------------------
;	システム関数
; ----------------------------------------------------------------------------

Prototype	Integer GetSystemPerformance( Integer nType := 0 ) native
Prototype	Integer SetSystemPerformance(
							Integer nType, Integer nFlags ) native
Prototype	Integer GetCurrentTime() native
Prototype	Time GetLocalTime() native
Prototype	MemoryStatus GetMemoryStatus() native
@IF	!PLATFORM_ANDROID
Prototype	Integer AddModule( String sModuleName ) native
Prototype	Integer OpenToAddArchiveFile(
						String sFilePath,
						String sPassword := "", String sID := "" ) native
Prototype	Integer EnableArchiveFilePath( String sID, Boolean fEnable ) native
@ELSE
Prototype	Integer OpenToAddArchiveFile(
						String sFilePath, String sPassword := "" ) native
@ENDIF
Prototype	Error Sleep( Integer nMilliSec ) native
Prototype	Error Exit() native
Prototype	Error Suspend( String sContextSaveFile := "" ) native
Prototype	Error Trace( String sTrace ) native
Prototype	void * memmove( void * dst, const void * src, Integer bytes ) native

Prototype	void * memmove(
				void * dst, const void * src, Integer bytes ) naked native
Prototype	void * memset( void * dst, Integer c, Integer bytes ) naked native
Prototype	void * malloc( Integer bytes ) naked native
Prototype	void * realloc( void * memblock, Integer bytes ) naked native
Prototype	void free( void * memblock ) naked native



; GetSystemPerformance タイプ
Constant	sysProcessorType			:= 00H,
			sysFontSmoothing			:= 01H,
			sysAcceptOtherSaveDir		:= 02H,
			sysLogicalProcessorCount	:= 03H

; CPU タイプ
Constant	GLS_USE_MMX_PENTIUM	:= 0002H,
			GLS_USE_XMM_SSE		:= 0008H,
			GLS_USE_XMM_SSE2	:= 0010H


