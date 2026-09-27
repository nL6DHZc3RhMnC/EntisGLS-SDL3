
DeclareType	InputFilter As Class
DeclareType	Sprite As Class
DeclareType	ResourceManager As Class
DeclareType	ToneFilter As Class
DeclareType	ModelJoint As Class
DeclareType	RenderSprite As Class

; File シークフラグ
Constant	FromBegin := 0, FromCurrent := 1, FromEnd := 2

Enumerator	SeekType<Integer>
	FromBegin := 0, FromCurrent := 1, FromEnd := 2
EndEnum


; File オープンフラグ
Constant	modeCreateFlag	:= 0001H,
			modeCreate		:= 0005H,
			modeRead		:= 0002H,
			modeWrite		:= 0004H,
			modeReadWrite	:= 0006H,
			shareRead		:= 0010H,
			shareWrite		:= 0020H

Class	Native File
Public
	Enumerator	Mode<Integer>
		CreateFlag	:= 0001H
		Create		:= 0005H
		Read		:= 0002H
		Write		:= 0004H
		ReadWrite	:= 0006H
		ShareRead	:= 0010H
		ShareWrite	:= 0020H
		NoCacheURL	:= 0100H
	EndEnum

Public
	Prototype	File& operator := ( Integer num )
	Prototype	File& operator += ( Integer num )
	Prototype	File& operator += ( Real num )
	Prototype	File& operator += ( String str )
	;
	Prototype	operator Boolean () const
	;
	Prototype	Error Open(
					String sFileName,
					Integer nFlags := File::Mode::Read \
									| File::Mode::ShareRead )
	Prototype	Error OpenURL(
					String sFileName,
					String sDownloadFile := "", Integer nFlags := 0 )
	Prototype	Error CreateMemoryFile()
	Prototype	Error CreateMemoryFile( Integer nInitBufSize )
	Prototype	Error Close()
	Prototype	Error OpenArchive()
	Prototype	Error CloseArchive()
	Prototype	Error OpenArchiveFile(
				String sFilepath, String sPassword, Boolean fStream )
	Prototype	Error CloseArchiveFile()
	Prototype	String GetCharacterEncoding() const
	Prototype	Error SetCharacterEncoding( String sType )
	Prototype	Boolean IsEndOfFile() const
	Prototype	Error SetEndOfFile()
	Prototype	Integer GetCurrentDownloaded() const
	Prototype	Integer GetDownloadingFileLength() const
	Prototype	Boolean IsFileDownloaded() const
	Prototype	Boolean IsFileDownloadFailed() const
	Prototype	Error CancelFileDownloading()
	Prototype	Integer GetLength() const
	Prototype	Integer GetPosition() const
	Prototype	Integer Seek( Integer nPos,
						Integer nOrigin := SeekType::FromBegin )
	Prototype	Error ReadText( Integer& num )
	Prototype	Error ReadText( Real& num )
	Prototype	Error ReadText( String& str )
	Prototype	Integer WriteText( Integer num )
	Prototype	Integer WriteText( Real num )
	Prototype	Integer WriteText( String str )
	Prototype	Integer Read( void* buf, Integer nBytes )
	Prototype	Integer Read( Integer& num, Integer nBytes := 0 )
	Prototype	Integer Read( Real& num, Integer nBytes := 0 )
	Prototype	Integer Read( String& str, Integer nBytes )
	Prototype	Integer Read( File& file, Integer nBytes )
	Prototype	Integer Write( void* buf, Integer nBytes := 0 )
	Prototype	Integer Write( Integer num, Integer nBytes := 0 )
	Prototype	Integer Write( Real num, Integer nBytes := 0 )
	Prototype	Integer Write( String str, Integer nBytes := 0 )
	Prototype	Integer Write( const File& file, Integer nBytes )
	Prototype	Error GetFileTime(
			Time & ftCreate, Time & ftLastAccess, Time & ftLastWrite ) const
	Prototype	Error LoadContextTitle( Reference object )
	Prototype	Error LoadObject( Reference object )
	Prototype	Error LoadContext()
	@IF	!PLATFORM_ANDROID
	Prototype	Error LoadContext( Boolean fCompressed )
	@ENDIF
	Prototype	Error SaveThumbnailImage( Sprite& sprThumbnail )
	Prototype	Error SaveThumbnailImage(
			Sprite& sprThumbnail, Integer nWidth, Integer nHeight )
	Prototype	Error SaveObject( Reference object )
	Prototype	Error SaveObject( Reference object, Reference title )
	Prototype	Error SaveContext()
	Prototype	Error SaveContext( Reference title )
	@IF	!PLATFORM_ANDROID
	Prototype	Error SaveContext( Reference title, Boolean fCompress )
	@ENDIF
	Prototype	Error DumpObject( Reference object )
	Prototype	Error DumpContext()
	Prototype	Boolean IsExisting( String sFileName ) const
	Prototype	Error Rename( String sOldName, String sNewName ) const
	Prototype	Error FindFile(
					String[]& refFileArray, String sFileName ) const
	Prototype	Error FindDirectory(
					String[]& refDirArray, String sDirName ) const
	Prototype	String FilterFilePath( String sFilePath ) const
EndClass


Class	Native	Thread
Public
	@IF	!PLATFORM_ANDROID
	Prototype	Error BeginThread( String sFuncName, ... )
	@ENDIF
	Prototype	Error BeginThread( ThreadProcedure& thproc )
	Prototype	Boolean IsThreadRunning() const
	Prototype	Reference GetThreadResult( Integer nTimeout := INFINITE )
	@IF	!PLATFORM_ANDROID
	Prototype	Error SetExceptionHandler( String sFuncName )
	@ENDIF
EndClass


Class	Native	ThreadEvent
Public
	Prototype	Error Create( Integer fInitState := 0 )
	Prototype	Error Delete()
	Prototype	Error Wait( Integer nTimeout := INFINITE )
	Prototype	Error Set( Integer nValue := 1 )
	Prototype	Error Reset()
	Prototype	Integer Value() const
EndClass



Class	Native	ThreadMutex
Public
	Prototype	Error Create( Integer fInitState := 0 )
	Prototype	Error Delete()
	Prototype	Error Wait( Integer nTimeout )
	Prototype	Error Reset()
	Prototype	Integer Value() const
EndClass



; Resource（音声再生モードフラグ）
Constant	ptfMusic :=0, ptfSE :=1, ptfVoice :=2, ptfSystem := 3
Constant	ptfDevice := 80000000H

Class	Native	Resource
Public
	Enumerator	PlayType<Integer>
		Music := 0, SE := 1, Voice := 2, System := 3
		Movie := 4, User0 := 5, User1 := 6, User2 := 7, User3 := 8
		Device := 80000000H
	EndEnum

Public
	Prototype	Error LoadImage( String filename )
	Prototype	Error LoadImage( File& file )
	Prototype	Error LoadSound( String filename, Integer nThreshold := -1 )
	Prototype	Error LoadSound( File& filen, Integer nThreshold := -1 )
	@IF	!PLATFORM_ANDROID
	Prototype	Error LoadMidi( String filename )
	Prototype	Error LoadMidi( File& file )
	@ENDIF
	Prototype	Error SaveImage(
					String filename,
					String sMimeType := "image/x-eri", Integer nQuality := -1 )
	Prototype	Error Release()
	Prototype	Error AttachSound( Resource& refSound )
	Prototype	Error Play(
					Integer nIntroSample := -1,
					Integer flag := Resource::PlayType::Music )
	Prototype	Error PlayFrom(
			Integer nStartPos := 0, Integer nEndPos := -1,
			Boolean fRepeat := false,
			Integer nRewindPos := -1, Integer flag := ptfMusic )
	Prototype	Error SetRewindingPortion(
			Integer nRewindPos := -1,
			Integer nEndPos := -1, Boolean fRepeat := true )
	Prototype	Error Stop()
	Prototype	Error Pause()
	Prototype	Error Restart()
	Prototype	Error GetVolume( Real& rLeftVol, Real& rRightVol )
	Prototype	Error SetVolume( Real rLeftVol, Real rRightVol )
	Prototype	Error SetVolumeEnvelope(
						const Vector2D[]& bezier, Integer nDurationTime )
	Prototype	Error CancelVolumeEnvelope()
	Prototype	Boolean IsPendingEnvelope() const
	Prototype	Real GetTotalVolume( Integer flag ) const
	Prototype	Error SetTotalVolume( Integer flag, Real rTotalVol )
	Prototype	Boolean IsPlaying() const
	Prototype	Integer GetPlayingPosition() const
	Prototype	Reference GetInfo() const
	Prototype	ImageInfo& GetImageInfo() const
	Prototype	SoundInfo& GetSoundInfo() const
	Prototype	Integer GetPixel( Integer x, Integer y ) const
	Prototype	Error GetPixelRect( Integer[]& aPixels,
			Integer x, Integer y, Integer width, Integer height ) const
	Prototype	Error GetPixelRect( uint32* aPixels,
			Integer x, Integer y, Integer width, Integer height ) const
	Prototype	Error SetPixelRect( const Integer[]& aPixels,
			Integer x, Integer y, Integer width, Integer height )
	Prototype	Error SetPixelRect( const uint32* aPixels,
			Integer x, Integer y, Integer width, Integer height )
	@IF	!PLATFORM_ANDROID
	Prototype	Integer GetWaveData(
			void* pWaveData,
			Integer nStartSamples, Integer nLengthSamples ) const
	@ENDIF
EndClass



Enumerator	AnimationLoopType<Integer>
	Once, Loop, Turn
EndEnum

Class	Native	Sprite	: Public Resource
Public
	Enumerator	AnimationFlag<Integer>
		Normal	:= 1
		Action	:= 2
		Effect	:= 4
	EndEnum

Public
	Prototype	Error AttachImage(
			const Resource& refImage,
			Integer nFrameNum := -1, const Rect& rcClip := null )
	Prototype	Error CreateSprite(
			Integer format, Integer width, Integer height )
	Prototype	Error SetAlphaImage(
			const Resource& refAlpha, Integer nAlphaRange )
	Prototype	Error Release()
	Prototype	Error SetBackColor( Integer rgbBack, Boolean fEnableBack )
	Prototype	Boolean EnableDynamicMode( Boolean fDynamicMode )
	Prototype	Error CreateZBuffer()
	Prototype	Error DeleteZBuffer()
	@IF	!PLATFORM_ANDROID
	Prototype	Error CreateStereoBuffer( const View3DInfo& v3dInfo )
	Prototype	Error SetStereoViewInfo( const View3DInfo& v3dInfo )
	Prototype	Error DeleteStereoBuffer()
	@ENDIF
	Prototype	ImageInfo& GetImageInfo() const
	Prototype	Error Set3DViewCamera(
			const Vector& vCamera,
			const Vector& vTarget, Real rRevCameraZ := 0.0 )
	Prototype	Error Enable3DViewCamera( Boolean fEnableCamera )
	Prototype	Error Get3DViewCamera(
			Vector& vCamera,
			Vector& vTarget, Real& rRevCameraZ ) const
	Prototype	Vector GetScreenPosition() const
	Prototype	Error SetScreenPosition( Real x, Real y, Real z )
	Prototype	Integer GetDrawFunctionFlags() const
	Prototype	Error SetDrawFunctionFlags( Integer nFlags )
	Prototype	Integer GetRenderFunctionFlags() const
	Prototype	Error SetRenderFunctionFlags( Integer nFlags )
	Prototype	Sprite& GetParent() const
	Prototype	Boolean IsVisible() const
	Prototype	Error SetVisible( Boolean f )
	Prototype	Rect GetRectangle( String strID := "" ) const
	Prototype	Error MovePosition( Integer x, Integer y )
	Prototype	Point GetPosition() const
	Prototype	Integer GetTransparency( String strID := "" ) const
	Prototype	Error SetTransparency( Integer t, String strID := "" )
	Prototype	Error SetZPosition( Real zPos )
	Prototype	Error SetZPosition( Real zPos, Real zScale )
	Prototype	Real GetZPosition() const
	Prototype	Error GetParameter( SpriteParam& param ) const
	Prototype	Error SetParameter( const SpriteParam& param )
	Prototype	Error CopyParameters( const Sprite& src )
	Prototype	Error UpdateRect()
	Prototype	Error UpdateRect( const Rect& rect )
	Prototype	Error Refresh()
	Prototype	Integer GetPriority( String strID := "" ) const
	Prototype	Error ChangePriority( Integer priority, String strID := "" )
	Prototype	Error AddSprite( Integer priority, const Sprite& sprite )
	Prototype	Error DetachSprite( const Sprite& sprite )
	Prototype	Error DetachAllSprite()
	Prototype	Error DrawImage(
			const Resource& rImage, const SpriteParam& param,
			Integer iFrame := 0, const Rect& rcClip := null )
	Prototype	Error FillRect(
			const Rect& rcFill, Integer rgbaFill,
			Integer nTransparency := 0, Integer nFlags := 0 )
	Prototype	Integer DrawText( const DrawTextParam& dtp, String strText )
	;
	Prototype	String GetSpriteID() const
	Prototype	Error SetSpriteID( String strID )
	Prototype	Error Enable( Boolean fEnable, String strID := "" )
	Prototype	Boolean IsEnabled( String strID := "" ) const
	Prototype	String GetSpriteText( String strID ) const
	Prototype	Error SetSpriteText( String strID, String strText )
	Prototype	Error SetSpriteFontFace( String strID, String strFont )
	Prototype	Error SetSpriteImage( String strID, String strImageID )
	;
	Prototype	Boolean IsHitSprite(
			Integer x, Integer y, String strID := "" ) const
	Prototype	String GetSpriteAtPoint( Integer x, Integer y ) const
	Prototype	String GetFocus() const
	Prototype	Error SetFocus( String strID := "" )
	Prototype	Error KillFocus( String strID := "" )
	Prototype	Error MoveFocus( Boolean fNext := true )
	Prototype	Error SetCapture( String strID := "" )
	Prototype	Error ReleaseCapture( String strID := "" )
	Prototype	Integer GetVertScrollPos( String strID := "" ) const
	Prototype	Error SetVertScrollPos( Integer nPos, String strID := "" )
	Prototype	Integer GetVertScrollRange( String strID := "" ) const
	Prototype	Error SetVertScrollRange( Integer nRange, String strID := "" )
	Prototype	Integer GetHorzScrollPos( String strID := "" ) const
	Prototype	Error SetHorzScrollPos( Integer nPos, String strID := "" )
	Prototype	Integer GetHorzScrollRange( String strID := "" ) const
	Prototype	Error SetHorzScrollRange( Integer nRange, String strID := "" )
	Prototype	Boolean IsButtonChecked( String strID ) const
	Prototype	Error CheckButton( String strID, Boolean fCheck )
	Prototype	Integer GetButtonViewStyle( String strID ) const
	Prototype	Integer SendCommand( String strID, String strCmd )
	Prototype	Integer SendCommand(
			String strID, String strCmd, String& strResult )
	;
	Prototype	Error SetHitTestProcedure( SpriteHitTestProcedure& proc )
	Prototype	Error SetTimerProcedure( SpriteTimerProcedure& proc )
	Prototype	Error SetMouseInterface( SpriteMouseInterface& hook )
	Prototype	Error SetKeyInterface( SpriteKeyInterface& hook )
	;
	Prototype	Integer ModifyAnimationFlags(
			Integer nAddFlags := 0, Integer nRemoveFlags := 0 )
	Prototype	Error BeginAnimation(
			Integer nLoopCount := 1, Integer nBeginFrame := 0,
			Integer nAnimationTime := -1,
			Integer nRewindSequence := 0, Integer nTurnSequence := -1 )
	Prototype	Error EndAnimation()
	Prototype	Boolean IsDuringAnimation() const
	Prototype	Integer GetBlendDegree() const
	Prototype	Error SetBlendDegree( Integer nDegree )
	Prototype	Error SetBlendingEnvelope( Integer nTargetDegree )
	Prototype	Error SetBlendingEnvelope( const Real[]& bzEnvelope )
	Prototype	Error SetBezierCurve( const Vector2D[]& bzCurve := null,
							const Real[]& bzRev := null,
							const Vector2D[]& bzMagnify := null )
	Prototype	Error SetBezierCurve( const Vector[]& bzCurve,
							const Real[]& bzRev := null,
							const Vector2D[]& bzMagnify := null )
	Prototype	Error SetCameraCurve( const Vector[]& bzCamera,
					const Vector[]& bzTarget, const Real[]& bzRevZ := null )
	Prototype	Error BeginActivation(
			Integer nDurationTime,
			Integer nActionType := AnimationLoopType::Once )
	Prototype	Error BeginActivation(
			const Integer[]& nDurationList,
			Integer nActionType := AnimationLoopType::Once )
	Prototype	Error CancelActivation()
	Prototype	Error FlushActivation()
	Prototype	Boolean IsActivation() const
	Prototype	Error AttachToneFilter()
	Prototype	Error AttachToneFilter( const ToneFilter& filter )
EndClass

; Sprite（アニメーションフラグ）
Constant	animeNormal	:= 1,
			animeAction	:= 2,
			animeEffect	:= 4

; Sprite（アイテム通知コード）
Constant	bsNormal := 0,
			bsFocus := 1,
			bsPushed := 2,
			bsPushedFocus := 3,
			bsDisabled := 4,
			bsPushDisabled := 5,
			bsActivePushed := 6
Constant	ncGeneric := 0,
			ncLineUp := 1,
			ncLineDown := 2,
			ncClickColumn := 3,
			ncTracking := 4,
			ncEndTracking := 5
Constant	ncChange := 0,
			ncKillFocus := 1

Enumerator	ButtonStatus<Integer>
	Normal := 0
	Focus := 1
	Pushed := 2
	PushedFocus := 3
	Disabled := 4
	PushDisabled := 5
	ActivePushed := 6
EndEnum

Enumerator	ScrollNotification<Integer>
	Generic := 0
	LineUp := 1
	LineDown := 2
	ClickColumn := 3
	Tracking := 4
	EndTracking := 5
	OnMouse := 6
	OnLeave := 7
EndEnum

Enumerator	EditNotification<Integer>
	Change := 0
	KillFocus := 1
EndEnum


Class	Native	Window	: Public Sprite
Public
	Enumerator	Copperation<Integer>
		Window			:= 0000H
		Normal			:= 0001H
		FullScreen		:= 0003H
		Exclusive		:= 0007H
	EndEnum

	Enumerator	Flag<Integer>
		UseDblClick			:= 00000001H
		AllowClose			:= 00000002H
		BlackBack			:= 00000004H
		EnableIME			:= 00000008H
		AllowMinimize		:= 00000010H
		GrantScreenSave		:= 00000020H
		GrantMonitorSave	:= 00000040H
		GrantPowerSuspend	:= 00000080H
		VariableWindowSize	:= 00000100H
		AllowMaximize		:= 00000200H
		AutoWindowAscept	:= 00000400H
		NoAutoFitSize		:= 00000800H
		ChildWindow			:= 00001000H
		PopupWindow			:= 00002000H
		InvisibleWindow		:= 00004000H
		NoNormalizePos		:= 00008000H
		OpenIME				:= 00010000H
		DoMinimize			:= 00020000H
		DoMaximize			:= 00040000H
	EndEnum

	Enumerator	Stereo3D<String>
		AnaglyphView		:= "AnaglyphView"
		DDStereoscopic		:= "DDStereoscopic"
		OpenGLQuadBuffer	:= "OpenGLQuadBuffer"
		NVStereoBLT			:= "NVStereoBLT"
		Mono				:= ""
	EndEnum

	Enumerator	MsgBoxStyle<Integer>
		Ok, OkCancel, YesNo, YesNoCancel, RetryCancel
	EndEnum

	Enumerator	MsgBoxResult<Integer>
		Ok, Cancel, Yes, No, Retry
	EndEnum

	Enumerator	Layout<Integer>
		Nothing			:= 0
		OffsetClient	:= 1
		DockingLeft		:= 2
		DockingRight	:= 3
		DockingUpper	:= 4
		DockingUnder	:= 5
		AlignLeft		:= 00H
		AlignTop		:= 00H
		AlignCenter		:= 10H
		AlignRight		:= 20H
		AlignBottom		:= 20H
		AlignAccording	:= 30H
		TypeClient		:= 00H
		TypeWindow		:= 40H
	EndEnum

	Enumerator	ExteriorFrameType<Integer>
		FillColor	:= 01H
		Stretch		:= 02H
	EndEnum

Public
	Prototype	Integer MessageBox( String sMessage )
	Prototype	Integer MessageBox(
					String sMessage, String sCaption,
					Integer nStyle := MsgBoxStyle::Ok )

	Prototype	Error CreateDisplay( String strWindowName )
	Prototype	Error CreateDisplay(
					String strWindowName, Integer fCopperationLevel )
	Prototype	Error CreateDisplay(
					String strWindowName, Integer fCopperationLevel,
					Integer nWidth, Integer nHeight )
	Prototype	Error CreateDisplay(
					String strWindowName, Integer fCopperationLevel,
					Integer nWidth, Integer nHeight,
					Integer nBitsPerPixel, Integer nFrequency )
	Prototype	Error CloseDisplay()
	Prototype	Integer GetOptionalFuncFlag() const
	Prototype	Error SetOptionalFuncFlag( Integer nFlags )
	Prototype	Error ChangeCooperationLevel()
	Prototype	Error ChangeCooperationLevel( Integer fCooperationLevel )
	Prototype	Error ChangeDisplaySize()
	Prototype	Error ChangeDisplaySize( Integer nWidth, Integer nHeight )
	Prototype	Error ChangeDisplaySize( Integer nWidth, Integer nHeight,
						Integer nBitsPerPixel, Integer nFrequency )
	Prototype	Boolean SetChangeDisplayModeFlag(
							Boolean fWithChangeMode := true )
	Prototype	Error SetStereoDisplayMode( String sViewID )
	Prototype	Error SetStereoDisplayMode( String sViewID, Integer nSubParam )
	Prototype	Boolean IsSupportedStereoDisplayMode( String sViewID ) const

	Prototype	Size GetDisplaySize() const
	Prototype	Error UpdateWindow()
	Prototype	Error ProcessUserInput( Integer nTimeout := 30 )
	Prototype	Boolean IsWindowActive() const
	Prototype	Error InitWindowPosition(
					Integer xPos := 80000000H, Integer yPos := 80000000H )
	Prototype	Boolean GetNormalWindowPosition( Point& ptWindow )
	@IF	!PLATFORM_ANDROID
	Prototype	Error InitWindowPosition(
					Integer xPos, Integer yPos,
					Integer nWidth, Integer nHeight )
	Prototype	Boolean GetNormalWindowPosition(
							Point& ptWindow, Size& sizeWindow )
	Prototype	Error GetPhysicalMonitorSize( Size& sizeWindow )
	@ENDIF
	Prototype	Error SetExteriorBackgroundFrame(
					Integer nFlags, Integer rgbColor, Resource& rsTile,
					Resource& rsLeft := null, Resource& rsRight := null,
					Resource& rsUpper := null, Resource& rsUnder := null )

	@IF	!PLATFORM_ANDROID
	Prototype	Error CreateWindow(
					String strWindowName,
					Integer nWidth, Integer nHeight,
					Window& wndParent := null )
	Prototype	Error CloseWindow()
	Prototype	Error ChangeWindowSize( Integer nWidth, Integer nHeight )
	Prototype	Error SetLayeredWindow( Boolean fLayered )
	Prototype	Error SetWindowLayout(
					Integer nFlags, Integer xPos := 0, Integer yPos := 0 )
	@ENDIF

	Prototype	Error EnableCommandQueue( Boolean fQueueCommand := true )
	Prototype	Error FlushCommandQueue(
					Boolean fQueueCommand := true,
					Integer nPriority := WndSpriteCmd::Priority::Highest )
	Prototype	Error QueueCommand( String strID,
					Integer nNotification := 0, Integer nParameter := 0,
					Integer nPriority := WndSpriteCmd::Priority::Normal,
					Boolean fOverwrite := false )
	Prototype	Error GetCommand( WndSpriteCmd& wscCmd,
					Integer nTimeout, Boolean fRemove := true )
	Prototype	Error CallMouseMove()
	Prototype	Error Lock( Integer nTimeout := INFINITE ) const
	Prototype	Error Unlock() const
	Prototype	Error FreezePaint() const
	Prototype	Error UnfreezePaint() const
	Prototype	Error SyncTimePaint( Integer nTimeout ) const
	Prototype	Error AsyncTimePaint() const

	Prototype	Error ShowCursor( Boolean fShow := true )
	Prototype	Boolean IsShowCursor() const
EndClass

; ウィンドウ協調レベル
Constant	levelWindow		:= 0000H,
			levelNormal		:= 0001H,
			levelFullScreen	:= 0003H,
			levelExclusive	:= 0007H

; ウィンドウ機能フラグ
Constant	optfUseDblClick			:= 00000001H,
			optfAllowClose			:= 00000002H,
			optfBlackBack			:= 00000004H,
			optfEnableIME			:= 00000008H,
			optfAllowMinimize		:= 00000010H,
			optfGrantScreenSave		:= 00000020H,
			optfGrantMonitorSave	:= 00000040H,
			optfGrantPowerSuspend	:= 00000080H,
			optfVariableWindowSize	:= 00000100H,
			optfAllowMaximize		:= 00000200H,
			optfOpenIME				:= 00010000H



Class	Native	MessageSprite	: Public Sprite
Public
	Prototype	Error CreateMessage( Integer nWidth, Integer nHeight )
	Prototype	Error CreateMessage( Integer nWidth, Integer nHeight,
					const Rect& rctMsgView, Boolean fMessageInSize := false )
	Prototype	Error OutputMessage( String strMsg )
	Prototype	Error FlushMessage()
	Prototype	Error ClearMessage()
	Prototype	Boolean IsMessagePending() const
	Prototype	Error AttachMessageStyle(
					const ResourceManager& refStyle, String strDefaultStyle )
	Prototype	Error SetDefaultMsgSpeed(
					Integer nCharSpeed,
					Integer nFadeSpeed, Integer nSpeedRatio := 100H )
	Prototype	Error SetMessageEffect(
					Real x, Real y, Real mx, Real my, Real rev )
	Prototype	Error SetShadowTransparency( Integer nTransparency )
	Prototype	Error SetFontBordering( Boolean fBordering )
	Prototype	Error SetFontStyle( String strMsgStyle )
	Prototype	Error SetFontFace( String strFontNames )
	Prototype	Error SetFontColor( Integer rgbColor, Integer rgbShadow := 0 )
	Prototype	Point GetCursorPos() const
	Prototype	Error MoveCursorPos( Integer x, Integer y )
	Prototype	Integer GetCharacterCount() const
	@IF	!PLATFORM_ANDROID
	Prototype	Rect GetMessageRect() const
	@ENDIF
EndClass


Class	Native	SuperSprite	: Public Sprite
Public
	Prototype	Error SetEffectParameter( const EffectParam& efprm )
	Prototype	Error SetEffectParameter(
					const EffectParam& efprm, const Resource& refMaskImage )
	Prototype	Error SetMeshWarpEffect(
			const Real[]& aMeshList, Integer nMeshListCount,
			Integer nMeshWidth, Integer nMeshHeight )
	Prototype	Error SetMeshWarpEffect(
			const Real[]& aMeshList, Integer nMeshListCount,
			Integer nMeshWidth, Integer nMeshHeight, const Real[]& aBaseMesh )
	@IF	!PLATFORM_ANDROID
	Prototype	Error SetMeshWarpEffect(
			const double * pMeshList, Integer nMeshListCount,
			Integer nMeshWidth, Integer nMeshHeight, const double * pBaseMesh )
	@ENDIF
EndClass



@IF	!PLATFORM_ANDROID

Class	Native	ParticleSprite	: Public Sprite
Public
	Enumerator	GenerationFlag<Integer>
		NegativeMask		:= 01H
		GenerationPoints	:= 02H
		RaySide				:= 04H
	EndEnum

Public
	Prototype	Error SetParticleImageLimit( Integer nLimit )
	Prototype	Error SetParticleImage( const Resource& rsImage )
	Prototype	Error SetParticleImage(
			const Resource& rsImage, const Point& ptHotspot )
	Prototype	Error SetParticleImage(
			const Resource& rsImage, const Point& ptHotspot, Integer nIndex )
	Prototype	Error SetParticleParameter( const ParticleParam& param )
	Prototype	Error SetParticleGeneratorMask(
					const Resource& rsMask,
					const Point& ptCenter := null,
					const Size& szZoom := null, const Size& szStep := null,
					Integer nFlags := 0, Integer nGenPoints := 0,
					const Vector2D& vRay := null )
	Prototype	Error SetParticleRectangle( const Rect& rctValidated )
	Prototype	Error CreateParticle( Integer nCount )
	Prototype	Error SetParticleGenerator( Integer nCount )
EndClass

; ParticleSprite（生成フラグ）
Constant	gfNegativeMask		:= 01H,
			gfGenerationPoints	:= 02H,
			gfRaySide			:= 04H


@ENDIF	; !PLATFORM_ANDROID


Class	Native	MovieSprite	: Public Sprite
Public
	Enumerator	PlayFlag<Integer>
		DirectDraw		:= 0001H
		LoopPlay		:= 0002H
		NoSkipFrame		:= 0004H
		NoLoopFilter	:= 0400H
		UseLoopFilter	:= 0800H
	EndEnum

Public
	Prototype	Error OpenMovie( String sFileName )
	Prototype	Error CloseMovie()
	Prototype	Error PlayMovie( Integer nFlags := 0, Integer fPlayType := -1 )
	Prototype	Error StopMovie()
	Prototype	Boolean IsMoviePlaying() const
	Prototype	Error SeekFrame( Integer nFrame )
	Prototype	Integer GetCurrentFrame() const
	Prototype	Integer GetTotalFrame() const
	Prototype	Integer GetTotalTime() const
EndClass

; MovieSprite（動画再生モードフラグ）
Constant	mpfDirectDraw		:= 01H,
			mpfLoopPlay			:= 02H,
			mpfNoLoopFilter		:= 0400H,
			mpfUseLoopFilter	:= 0800H


Class	Native	ResourceManager
Public
	Prototype	Resource& operator [] ( String index )
	Prototype	Error LoadResource( String filename )
	Prototype	Error LoadResource( File& file )
	Prototype	Error DeleteContents()
	Prototype	Error CreateFormPage( Sprite& refSprite, String strPageID )
EndClass



Class	Native	ToneFilter
Public
	Enumerator	Tone<Integer>
		Brightness	:= 0000H
		Inversion	:= 0001H
		Light		:= 0002H
	EndEnum
	Enumerator	Flag<Integer>
		FixZero			:= 0001H
		MaskWithAlpha	:= 0002H
		YUVFilter		:= 0004H
		GrayFilter		:= 0008H
	EndEnum

Public
	Prototype	void ToneFilter( const ToneFilter& filter )
	Prototype	const ToneFilter& operator := ( const ToneFilter& filter )
	;
	Prototype	Error LoadFilterFile( String filename )
	Prototype	Error SetGeneralTone(
					Integer nRedTone, Integer nRedFlag,
					Integer nGreenTone, Integer nGreenFlag,
					Integer nBlueTone, Integer nBlueFlag,
					Integer nAlphaTone, Integer nAlphaFlag,
					Integer nFlags := 0 )
	Prototype	Error MorphingFilter( const ToneFilter& filter1,
					const ToneFilter& filter2, Integer degree )
EndClass

; トーンフィルターフラグ
Constant	EGL_TONE_BRIGHTNESS	:= 0000H,
			EGL_TONE_INVERSION	:= 0001H,
			EGL_TONE_LIGHT		:= 0002H
Constant	tffFixZero			:= 0001H,
			tffMaskWithAlpha	:= 0002H,
			tffYUVFilter		:= 0004H


Class	Native	InputFilter
Public
	Enumerator	Type<Integer>
		Above	:= 0
		Below	:= 1
	EndEnum
Public
	Prototype	void InputFilter( const InputFilter& filter )
	Prototype	const InputFilter& operator := ( const InputFilter& filter )
	;
	Prototype	Error LoadInputFilter( String filename )
	Prototype	Error DeleteInputFilter()
	Prototype	Error OpenFilter(
					Integer nFilterType := Type::Above,
					const Window& refWindow := null )
	Prototype	Error CloseFilter()
	Prototype	Error GetInputEvent( InputEvent& refEvent, Integer nTimeout )
	Prototype	Error FlushInputQueue( Integer nLimit )
	Prototype	Integer GetCapturedJoyStick() const
	Prototype	Error GetStickPosition(
					Vector& refPos, Integer iDevNum := 0 ) const
	Prototype	Boolean IsJoyButtonPushing(
					Integer iKeyNum, Integer iDevNum := 0 ) const
	Prototype	Integer GetJoyButtonPushed(
					Integer iKeyNum, Integer iDevNum := 0 ) const
	Prototype	Error FlushJoyButtonPushed(
					Integer iDevNum := 0, Integer iKeyNum := -1 )
	Prototype	Error ResetJoyButtonPushing(
					Integer iDevNum := 0, Integer iKeyNum := -1 )
	Prototype	Point GetCursorPos() const
	Prototype	Error MoveCursorPos( Integer x, Integer y )
	Prototype	Error AddFilter(
					const InputEvent& evInput, const InputEvent& evOutput )
	Prototype	Error RemoveFilter( const InputEvent& evInput )
	Prototype	InputEvent GetFilter( const InputEvent& evInput ) const
	@IF	!PLATFORM_ANDROID
	Prototype	Error DispatchEvent(
					const InputEvent& evInput, Boolean fPushed )
	@ENDIF
EndClass



Class	Native	PolygonModel
Public
	Prototype	Error LoadModel( String strFileName )
	Prototype	Error DeleteModel()
	Prototype	Error CreateImagePrimitive(
			const Resource& rImage, Integer iFrame := 0,
			const Rect& rectView := null,
			const Vector2D& vCenter := null,
			const Vector2D& vEnlarge := null )
	Prototype	Error CreateImagePolygon(
			const Resource& rImage, Integer iFrame := 0,
			const Rect& rectView := null, const Vector2D& vCenter := null,
			const SurfaceAttribute& sfAttr := null,
			Real rFogDeepness := 0.0, Integer rgbFogColor := 0 )
	Prototype	Error AttachImagePolygon(
			const Resource& rImage, Integer iFrame := 0,
			const Rect& rectView := null, const Vector2D& vCenter := null )
	;
	Enumerator	GridType<Integer>
		HorzLoop	:= 0001H
		VertLoop	:= 0002H
		TopTip		:= 0010H
		BottomTip	:= 0020H
		AutoSmooth	:= 8000H
	EndEnum
	;
	Prototype	String RegisterSurfaceAttribute(
					const SurfaceAttribute& sufattr )
	Prototype	String RegisterTextureAttribute(
					Resource& rsTexture, const SurfaceAttribute& sufattr )
	Prototype	Integer AddGridMeshPrimitive(
					String idAttr,
					Integer nMeshWidth, Integer nMeshHeight, Integer nFlags,
					float[3]* pvVertex, float[3]* pvNormal := null,
					float[2]* pvUVMap := null, uint32[2]* pvColor := null )
	Prototype	Error ModifyGridMeshPrimitive(
					Integer idPrimitive,
					Integer nMeshWidth, Integer nMeshHeight, Integer nFlags,
					float[3]* pvVertex := null, float[3]* pvNormal := null,
					float[2]* pvUVMap := null, uint32[2]* pvColor := null )
	Prototype	Integer AddTriangleStripPrimitive(
					String idAttr,
					Integer nTriangleStripCount, Integer nFlags,
					float[3]* pvVertex, float[3]* pvNormal := null,
					float[2]* pvUVMap := null, uint32[2]* pvColor := null )
	Prototype	Error ModifyTriangleStripPrimitive(
					Integer idPrimitive,
					Integer nTriangleStripCount, Integer nFlags,
					float[3]* pvVertex := null, float[3]* pvNormal := null,
					float[2]* pvUVMap := null, uint32[2]* pvColor := null )
	Prototype	Integer AddTriangleListPrimitive(
					String idAttr,
					Integer nTriangleCount, Integer nFlags,
					float[3]* pvVertex, float[3]* pvNormal := null,
					float[2]* pvUVMap := null, uint32[2]* pvColor := null )
	Prototype	Error ModifyTriangleListPrimitive(
					Integer idPrimitive,
					Integer nTriangleCount, Integer nFlags,
					float[3]* pvVertex := null, float[3]* pvNormal := null,
					float[2]* pvUVMap := null, uint32[2]* pvColor := null )
	;
	@IF	!PLATFORM_ANDROID
	Prototype	Error TransformAccordingAsBone()
	Prototype	ModelJoint& FindBoneAs( String strBoneName ) const
	@ENDIF
EndClass



Class	Native	ModelJoint
Public
	Prototype	void ModelJoint( const ModelJoint& joint )
	Prototype	const ModelJoint& operator := ( const ModelJoint& joint )
	Prototype	ModelJoint& operator [] ( Integer index ) const
	;
	Prototype	Vector& Position()
	Prototype	Error InitializeMatrix()
	Prototype	Error RotateOnX( Real rDeg )
	Prototype	Error RotateOnY( Real rDeg )
	Prototype	Error RotateOnZ( Real rDeg )
	Prototype	Error RotateByAngleOn( Real x, Real y, Real z )
	Prototype	Error RevolveOnX( Real rDeg )
	Prototype	Error RevolveOnY( Real rDeg )
	Prototype	Error RevolveOnZ( Real rDeg )
	Prototype	Error RevolveByAngleOn( Real x, Real y, Real z )
	Prototype	Error MagnifyByVector( Real x, Real y, Real z )
	Prototype	Quaternion GetQuaternion()
	Prototype	Error SetQuaternion( const Quaternion& q )
	;
	Prototype	Error AddModelRef( const PolygonModel& rModel )
	Prototype	Error ClearModelRef()
	Prototype	Integer AddSubJoint()
	Prototype	Integer AddSubJoint( ModelJoint& joint )
	Prototype	ModelJoint& CreateSubJoint()
	Prototype	Integer GetLength() const
	Prototype	Error RemoveSubJoint( const ModelJoint& joint )
	Prototype	Error RemoveSubJoint( Integer nIndex )
	Prototype	Error RemoveAllSubJoint()
	Prototype	E3DColor GetColorAttribute() const
	Prototype	Integer GetTransparency() const
	Prototype	Error SetColorAttribute()
	Prototype	Error SetColorAttribute( const E3DColor& color )
	Prototype	Error SetTransparency( Integer nTransparency )
	Prototype	Error SetColorMorphing( const E3DColor& color )
	Prototype	Error SetColorMorphing(
			const E3DColor& color := null, Integer nTransparency := 0 )
	Prototype	Error SetBezierCurve(
			const Vector[]& aCurve := null,
			const Vector[]& aRoation := null,
			const Vector[]& aMagnification := null )
	Prototype	Error SetRotationBezier( const Quaternion[]& aRotation )
	Prototype	Error BeginActivation(
			Integer nDuration, Integer nActionType := AnimationLoopType::Once )
	Prototype	Error FlushActivation()
	Prototype	Boolean IsActivation() const
EndClass



@IF	!PLATFORM_ANDROID

Class	Native	ParticleModel	: Public ModelJoint
Public
	Prototype	Error SetParticleImageLimit( Integer nLimit )
	Prototype	Error SetParticleImage(
			const Resource& rImage, const Vector2D& vHotspot := null )
	Prototype	Error SetParticleImage(
			const Resource& rImage,
			const Vector2D& vHotspot := null, Integer nIndex := 0 )
	Prototype	Error SetParticleImage(
			const PolygonModel& rImage, const Vector2D& vHotspot := null )
	Prototype	Error SetParticleImage(
			const PolygonModel& rImage,
			const Vector2D& vHotspot := null, Integer nIndex := 0 )
	Prototype	Error SetParticleParameter( const ParticleParam3D& param )
	Prototype	Error CreateParticle( Integer nCount )
	Prototype	Error SetParticleGenerator( Integer nCount )
	Prototype	Error AdvanceParticleTime( Integer nPastTime )
	Prototype	Error AddModelToRenderer( RenderSprite& render ) const
EndClass

@ENDIF	; !PLATFORM_ANDROID


Class	Native	RenderSprite
Public
	Enumerator	CameraCurveFlag<Integer>
		OnlyAngle	:= 01H
	EndEnum

Public
	Prototype	Error Initialize(
			Sprite& refParent, Integer nPriority,
			Integer nHeapSize := 100000H,
			Integer nPolyLimit := 8000H,
			Integer nFlag := 0, const Rect& rectClip := null )
	Prototype	Error Release()
	Prototype	Error SetViewPoint(
			const Vector& vViewPoint, const Vector& vTarget, Real rDegAngleZ )
	Prototype	Error SetEndOfViewCurve(
			const Vector& vViewPoint, const Vector& vTarget,
				Real rDegAngleZ, Integer nFlags := 0 )
	Prototype	Error SetViewOnCurve( Real t )
	Prototype	Error BeginViewAnimation( Integer nDuration )
	Prototype	Error FlushViewAnimation()
	Prototype	Boolean IsViewAnimation() const
	Prototype	Error FlushActivation()
	Prototype	Boolean IsActivation() const
	Prototype	Error SetZClipRange( Real rMinZ, Real rMaxZ )
	Prototype	Error SetLightEntries( const LightEntry[]& aLights )
	Prototype	Integer GetSortingFlags() const
	Prototype	Error SetSortingFlags( Integer nFlags )
	Prototype	Error AddModel(
			ModelJoint& rJoint,
			const E3DColor& color, Integer nTransparency := 0 )
	Prototype	Error PrepareRendering()
	Prototype	Error FlushAllPolygon()
	Prototype	Error AttachRootJoint( ModelJoint& rRootJoint )
	Prototype	Error UpdateRendering()
EndClass

; RenderSprite（カメラアニメーションフラグ）
Constant	ctOnlyAngle	:= 01H


@IF	!PLATFORM_ANDROID

Class	Native	Setup
Public
	Enumerator	FontEnumFlag<Integer>
		AllCharset		:= 00H
		DefaultCharset	:= 01H
		ANSI			:= 02H
		ShiftJIS		:= 04H
		Symbol			:= 08H
	EndEnum

Public
	Prototype	Error CreateInstallationDialog(
			String& sInstDir, Integer& nOptionFlags := null,
			String sCaption := "", Window& window := null,
			String sDefSubDir := "" )
	Prototype	Error CloseInstallationDialog()
	Prototype	Boolean IsInstallationDialogCanceled() const
	Prototype	Error SetInstallationDialogFileText( String sText )
	Prototype	Error SetInstallationDialogProgress(
			Integer nFile, Integer nTotal )
	Prototype	Integer InstallationMessageBox(
			String sText, String sCaption, Integer nType, Window& window )
	Prototype	Integer GetFontList(
			String[]& aFontList, Integer nFlags := 0 ) const
	Prototype	String GetWindowsProductID() const
	Prototype	String MakeMD5Digest( String sText ) const
	Prototype	String MakeMD5Digest( File& file ) const
	Prototype	Integer CalcCRC32( String sText ) const
	Prototype	Integer CalcCRC32( File& file ) const
	Prototype	Integer CheckSum32( String sText ) const
	Prototype	Integer CheckSum32( File& file ) const
	Prototype	String GetDesktopDirectory() const
	Prototype	String GetStartMenuDirectory() const
	Prototype	String GetAppDataDirectory() const
	Prototype	String GetWindowsDirectory() const
	Prototype	String GetCurrentModulePath() const
	Prototype	String GetEnvironmentVariable( Hash<String>& mapEnv ) const
	Prototype	String FilterEnvironmentPath( String sPath ) const
	Prototype	Error GetDiskVolumeName( String sDrv, String& sVolName ) const
	Prototype	Error GetDiskSerialNumber(
			String sDrv, Integer& nSerialNum ) const
	Prototype	Error GetDiskFreeSpace(
			String sDrv, Integer& nFreeAvailable,
			Integer& nTotalBytes, Integer& nFreeSpace ) const
	Prototype	Integer ShellExecute(
			String sVerb, String sFile, String sParameters := "" ) const
	Prototype	Error ExecuteProcess(
			String sAppFile, String sCmdLine, Integer nFlags := 0,
				String sEnvironment := "", String sCurrentDirectory := "" )
	Prototype	Error GetExecuteExitCode(
			Integer nTimeout, Integer & nExitCode := null ) const
	Prototype	Error BrowseForFolder(
			String& sDir, String sCaption, Window& rMainWindow := null )
	Prototype	Error BrowseForFolder(
			String& sDir, String sCaption,
			Window& rMainWindow, Integer nParentWnd )
	Prototype	Error BrowseFileDialog(
			String& sPath, Boolean fSaveFile, String sCaption,
			String aFilters, Window& rMainWindow := null )
	Prototype	Error ReadInstalledLog( String filename )
	Prototype	Error ReadInstalledLog( File& file )
	Prototype	Error WriteInstalledLog( String filename )
	Prototype	Error WriteInstalledLog( File& file )
	Prototype	Error MeasureInstallSize( Integer& nSize ) const
	Prototype	Error AddInstallDirectory( String sDstPath )
	Prototype	Error AddInstallArchiveDirectory(
			String sDstPath, String sPassword := "", Integer nType := 0 )
	Prototype	Error AddInstallFile( String sDstPath, String sSrcPath )
	Prototype	Error AddInstallArchiveTree(
			String sDstPath, String sSrcArchiveFile, String sPassword := "" )
	Prototype	Error AddInstallDirectoryTree(
			String sDstPath, String sSrcPath )
	Prototype	Boolean BootCheck(
			String sCheckName, Boolean fDisableBoot := false )
	Prototype	Error ReleaseBootCheck()
	Prototype	Error GetLastErrorMsg( String& refErrMsg ) const
	Prototype	Error BeginInstall( String sInstDir )
	Prototype	Boolean IsFinishedInstall() const
	Prototype	Error InstallNextFile(
			String& refDstFilePath, String& refSrcFilePath )
	Prototype	Integer GetCurrentCopiedBytes(
			Integer& nFileSize := null ) const
	Prototype	Integer GetTotalCopiedBytes(
			Integer& nTotalSize := null ) const
	Prototype	Error WaitForCurrentCopy( Integer nTimeout ) const
	Prototype	Error EndInstall()
	Prototype	Error InstallCreateDirectory( String sDirPath )
	Prototype	Error AddInstallFileLog( String sDstFilePath )
	Prototype	Error InstallCreateShortcutFile(
			String sDstDir, String sName,
			String sLinkTarget, String sArg := "" )
	Prototype	Error GetUninstallInfo(
			String sRegName, UninstallInfo& info ) const
	Prototype	Error RegisterUninstall(
			String sRegName, const UninstallInfo& info )
	Prototype	Integer GetRegUninstallInteger32(
			String sRegName, String sValueName, Integer nDefValue := 0 ) const
	Prototype	Integer GetRegUninstallInteger64(
			String sRegName, String sValueName, Integer nDefValue := 0 ) const
	Prototype	String GetRegUninstallString(
			String sRegName, String sValueName, String sDefValue := "" ) const
	Prototype	Error SetRegUninstallInteger32(
			String sRegName, String sValueName, Integer nValue )
	Prototype	Error SetRegUninstallInteger64(
			String sRegName, String sValueName, Integer nValue )
	Prototype	Error SetRegUninstallString(
			String sRegName, String sValueName, String strValue)
	Prototype	Error Uninstall( String sInstDir )
	Prototype	Error DeleteInstalledFile( String sFilePath )
	Prototype	Boolean IsNecessaryRebootToDelete() const
	Prototype	Error UnRegisterUninstall( String sRegName )
	Prototype	Error RebootWindows()
EndClass

; インストーラー標準スタイルダイアログオプションフラグ
Constant	instoptShortCutDesktop	:= 0001H,
			instoptShortCutPrograms	:= 0002H,
			instoptDisableDesktop	:= 0100H,
			instoptDisablePrograms	:= 0200H,
			instoptDisableInstDir	:= 0400H

; メッセージボックススタイル
Constant	MB_OK					:= 00000000H,
			MB_OKCANCEL				:= 00000001H,
			MB_ABORTRETRYIGNORE		:= 00000002H,
			MB_YESNOCANCEL			:= 00000003H,
			MB_YESNO				:= 00000004H,
			MB_RETRYCANCEL			:= 00000005H,
			MB_CANCELTRYCONTINUE	:= 00000006H,
			MB_ICONHAND				:= 00000010H,
			MB_ICONQUESTION			:= 00000020H,
			MB_ICONEXCLAMATION		:= 00000030H,
			MB_ICONASTERISK			:= 00000040H,
			MB_USERICON				:= 00000080H,
			MB_ICONWARNING			:= MB_ICONEXCLAMATION,
			MB_ICONERROR			:= MB_ICONHAND,
			MB_ICONINFORMATION		:= MB_ICONASTERISK,
			MB_ICONSTOP				:= MB_ICONHAND

; メッセージボックス返り値
Constant	IDOK		:= 1,
			IDCANCEL	:= 2,
			IDABORT		:= 3,
			IDRETRY		:= 4,
			IDIGNORE	:= 5,
			IDYES		:= 6,
			IDNO		:= 7,
			IDCLOSE		:= 8,
			IDHELP		:= 9,
			IDTRYAGAIN	:= 10,
			IDCONTINUE	:= 11

; プロセス起動フラグ
Constant	DETACHED_PROCESS			:= 00000008H,
			CREATE_NEW_CONSOLE			:= 00000010H,
			NORMAL_PRIORITY_CLASS		:= 00000020H,
			IDLE_PRIORITY_CLASS			:= 00000040H,
			HIGH_PRIORITY_CLASS			:= 00000080H,
			REALTIME_PRIORITY_CLASS		:= 00000100H,
			CREATE_NEW_PROCESS_GROUP	:= 00000200H,
			BELOW_NORMAL_PRIORITY_CLASS	:= 00004000H,
			ABOVE_NORMAL_PRIORITY_CLASS	:= 00008000H,
			CREATE_NO_WINDOW			:= 08000000H


@ENDIF	; !PLATFORM_ANDROID


