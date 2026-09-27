#include "tjs.h"
#include "tjsError.h"
#include "tjsArray.h"
#include "tjsDictionary.h"
#include <iostream>
#include <stdexcept>
#include <spdlog/sinks/stdout_color_sinks.h>

int main() {
    try {
        spdlog::stdout_color_mt("tjs2");
        spdlog::stdout_color_mt("core");
        auto *engine = new TJS::tTJS();
        // Regression: a character constructor supplies exactly one UTF-16
        // code unit. The bounded copy must not read a terminator after it.
        const TJS::ttstr single(TJS_W('/'));
        if(single!=TJS_W("/"))throw std::runtime_error("single-character string failed");
        tjs_char bounded[2];TJS::TJS_strcpy_maxlen(bounded,nullptr,0);
        if(bounded[0]!=0)throw std::runtime_error("zero-length string copy failed");
        TJS::tTJSVariant result;
        engine->ExecScript(TJS_W("var d = %[ name: 'StudySteady', values: [1, 2, 3] ]; d.values.add(4); var value = d.values[0] + d.values[3];"), nullptr);
        engine->EvalExpression(TJS_W("d.name + ':' + value + ':' + d.values.count"), &result);
        const TJS::ttstr value(result);
        std::cout << "TJS dictionary/array/execution=" << value.AsStdString() << '\n';
        if(value != TJS_W("StudySteady:5:4")) throw std::runtime_error("Unexpected script result");
        engine->EvalExpression(TJS_W("new Math.RandomGenerator(12345)"), &result);
        if(result.Type() != TJS::tvtObject || !result.AsObjectNoAddRef()) throw std::runtime_error("RandomGenerator construction failed");
        result.Clear();
        engine->Shutdown();
        engine->Release();
        std::cout << "real TJS runtime: PASS\n";
        return 0;
    } catch(const TJS::eTJSError &e) {
        std::cerr << "TJS error: " << e.GetMessage().AsStdString() << '\n'; return 1;
    } catch(const std::exception &e) {
        std::cerr << e.what() << '\n'; return 1;
    }
}
