#include <jni.h>
#include <android/log.h>
#include "psbfile/PSBRawFile.h"
#include "tjsUtils.h"
#include "psb_header.h"
#include "launcher/psb_key_resolver.h"
#include <filesystem>
#include "EmoteVarController.h"
#include <fstream>
#include <sstream>
#include <iterator>

extern "C" JNIEXPORT jstring JNICALL
Java_io_entisgls_launcher_GameActivity_nativeCheckPsb(JNIEnv *env, jclass, jstring path) {
    const char *utf = env->GetStringUTFChars(path, nullptr);
    if (!utf) return nullptr;
    const std::string filename(utf);
    env->ReleaseStringUTFChars(path, utf);
    std::string message;
    try {
        std::ifstream input(filename, std::ios::binary | std::ios::ate);
        if (!input) return env->NewStringUTF("No PSB test resource installed");
        const auto length = input.tellg();
        if (length < 56 || length > 256 * 1024 * 1024)
            throw std::runtime_error("PSB resource size out of range");
        input.seekg(0);
        std::vector<uint8_t> bytes(static_cast<size_t>(length));
        input.read(reinterpret_cast<char *>(bytes.data()), bytes.size());
        if (!input) throw std::runtime_error("PSB read failed");
        const auto gameDir = std::filesystem::path(filename).parent_path();
        entis::launcher::PsbKeyResolver keys(gameDir, gameDir.parent_path() / "psb-probe-cache");
        studysteady::decodePsbHeader(bytes, keys.Resolve(bytes.data(), bytes.size()));
        PSB::PSBFile file;
        auto *owned = static_cast<uint8_t*>(TJS::TJSAlignedAlloc(bytes.size(), 4));
        if (!owned) throw std::bad_alloc();
        std::memcpy(owned, bytes.data(), bytes.size());
        if (!file.Adopt(owned, bytes.size())) {
            TJS::TJSAlignedDealloc(owned);
            throw std::runtime_error("PSB parsing failed");
        }
        const auto root = file.GetRoot();
        const auto spec = root.GetDictionaryValueStrict("spec").GetString();
        const auto version = root.GetDictionaryValueStrict("version").GetDouble();
        const auto objects = root.GetDictionaryValueStrict("object").GetDictionaryKeys();
        motion::EmoteVarController controller(2);
        const float target[2] = {10.f, 20.f};
        motion::EmoteVarController_setTarget_guess(&controller, target, 1.f, 1.f, false);
        float actual[2];
        motion::EmoteVarController_step(&controller, actual, .5f);
        if (actual[0] != 5.f || actual[1] != 10.f)
            throw std::runtime_error("MotionPlayer interpolation failed");
        std::ostringstream result;
        result << "PSB header/read + MotionPlayer interpolation PASS; spec=" << spec
               << ", motion version=" << version << ", object groups=" << objects.size()
               << ". CPU format/interpolation probe; game rendering is initialized separately.";
        message = result.str();
        __android_log_print(ANDROID_LOG_INFO, "StudySteady", "%s", message.c_str());
    } catch (const std::exception &e) {
        message = e.what();
        __android_log_print(ANDROID_LOG_ERROR, "StudySteady", "%s", message.c_str());
    }
    return env->NewStringUTF(message.c_str());
}
