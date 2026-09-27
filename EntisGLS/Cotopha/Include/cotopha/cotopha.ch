
@IF	@IsDefined("__COTOPHA_CH__") == 0

Constant	__COTOPHA_CH__ := -1

@IF	@IsDefined("PLATFORM_ANDROID") == 0
	Constant	PLATFORM_ANDROID := 0
@ENDIF

@IF	@IsDefined("PLATFORM_WINDOWS") == 0
	Constant	PLATFORM_WINDOWS := !PLATFORM_ANDROID
@ENDIF

Include "basictype.ch"
Include "stddefs.ch"
Include "stdtype.ch"


@ENDIF

