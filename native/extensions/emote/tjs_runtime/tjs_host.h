#pragma once
#include "tjs.h"
#include "tjsError.h"

// Real engine services consumed by NCB. This boundary owns no fake variants or
// dispatch objects; every operation delegates to the active TJS engine.
void motionSetTjsEngine(tTJS *engine);
iTJSDispatch2 *TVPGetScriptDispatch();
void TVPExecuteExpression(const ttstr &content, tTJSVariant *result = nullptr);
void TVPAddLog(const ttstr &line);
[[noreturn]] void TVPThrowExceptionMessage(const tjs_char *message);
[[noreturn]] void TVPThrowExceptionMessage(const tjs_char *message, const ttstr &parameter);
