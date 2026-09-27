#include <app/sakura2vm.h>
#include "legacy_runner.h"
#include "launcher/psb_key_runtime.h"
#include "platform/log.h"
#include <exception>

int sglMain(const wchar_t *arguments) {
    try {
        const int result = RunLegacyGame(arguments);
        if (entis::launcher::HasGamePsbKeyError()) {
            study::platform::LogWrite(study::platform::LogPriority::Error, "EntisGLS", entis::launcher::GamePsbKeyError().c_str());
            return 40;
        }
        return result;
    } catch (const std::exception& error) {
        study::platform::LogWrite(study::platform::LogPriority::Error, "EntisGLS", error.what());
        return 1;
    }
}
SGLError sglStaticInitialize() { return sglErrSuccess; }
SGLError sglStaticFinalize() { return sglErrSuccess; }
void sglAbortVM() { AbortLegacyGame(); }
