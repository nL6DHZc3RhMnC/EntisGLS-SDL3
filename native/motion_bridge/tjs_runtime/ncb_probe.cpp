#include "ncbind.hpp"
#include <iostream>
#include <stdexcept>
#include <spdlog/sinks/stdout_color_sinks.h>
#define NCB_MODULE_NAME TJS_W("motion_bridge_probe.dll")

// The production NCB constructor, property and method adapters are exercised
// against a simple C++ object so no renderer or fabricated script dispatch is
// needed to validate the binding boundary.
class NativeAccumulator {
    tjs_int value = 0;
public:
    tjs_int getValue() const { return value; }
    void setValue(tjs_int v) { value = v; }
    tjs_int add(tjs_int v) { value += v; return value; }
};
NCB_REGISTER_CLASS(NativeAccumulator) {
    NCB_CONSTRUCTOR(());
    NCB_PROPERTY(value, getValue, setValue);
    NCB_METHOD(add);
}

int main() {
    try {
        spdlog::stdout_color_mt("tjs2");
        spdlog::stdout_color_mt("core");
        auto *engine = new tTJS();
        motionSetTjsEngine(engine);
        ncbAutoRegister::AllRegist();
        if(!ncbAutoRegister::LoadModule(TJS_W("motion_bridge_probe.dll"))) throw std::runtime_error("NCB registration failed");
        tTJSVariant result;
        engine->ExecScript(TJS_W("var native = new NativeAccumulator(); native.value = 40; native.add(2);"), nullptr);
        engine->EvalExpression(TJS_W("native.value"), &result);
        if(result.AsInteger() != 42) throw std::runtime_error("Native method/property result mismatch");
        std::cout << "NCB class/constructor/property/method=42: PASS\n";
        result.Clear();
        engine->ExecScript(TJS_W("invalidate native; delete native;"), nullptr);
        ncbAutoRegister::AllUnregist();
        engine->Shutdown(); engine->Release();
        motionSetTjsEngine(nullptr);
        return 0;
    } catch(const eTJSError &e) {
        std::cerr << "TJS error: " << e.GetMessage().AsStdString() << '\n'; return 1;
    } catch(const std::exception &e) {
        std::cerr << e.what() << '\n'; return 1;
    }
}
