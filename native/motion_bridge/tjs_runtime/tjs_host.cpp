#include "tjs_host.h"
#include <stdexcept>
#include <spdlog/spdlog.h>
#include "platform/log.h"

namespace { tTJS *activeEngine = nullptr; }
void motionSetTjsEngine(tTJS *engine) { activeEngine = engine; }
iTJSDispatch2 *TVPGetScriptDispatch() {
    if(!activeEngine) throw std::runtime_error("NCB has no active TJS engine");
    return activeEngine->GetGlobal();
}
void TVPExecuteExpression(const ttstr &content, tTJSVariant *result) {
    if(!activeEngine) throw std::runtime_error("NCB has no active TJS engine");
    activeEngine->EvalExpression(content, result);
}
void TVPAddLog(const ttstr &line) { spdlog::info("{}", line.AsStdString()); }
namespace TJS {
void TVPConsoleLog(const tTJSString& line) {
    const auto text = line.AsStdString();
    study::platform::LogWrite(study::platform::LogPriority::Info, "TJS", text.c_str());
}
}
[[noreturn]] void TVPThrowExceptionMessage(const tjs_char *message) {
    TJS_eTJSError(message);
    throw std::runtime_error("TJS exception routine unexpectedly returned");
}
[[noreturn]] void TVPThrowExceptionMessage(const tjs_char *message, const ttstr &parameter) {
    auto text = ttstr(message);
    text.Replace(TJS_W("%1"), parameter);
    TVPThrowExceptionMessage(text.c_str());
}
