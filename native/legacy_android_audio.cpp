#include "legacy_android_audio.h"
#include <jni.h>
#include <mutex>
#include <algorithm>
#include <cmath>

namespace {
std::mutex audioMutex;
JavaVM* audioVM = nullptr;
jobject audioManager = nullptr;
jmethodID getVolume = nullptr, getMaxVolume = nullptr, setVolume = nullptr;
struct AttachedEnv {
    JNIEnv* env = nullptr;
    bool attached = false;
    AttachedEnv() {
        if (!audioVM) return;
        if (audioVM->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_EDETACHED)
            attached = audioVM->AttachCurrentThread(&env, nullptr) == JNI_OK;
    }
    ~AttachedEnv() { if (attached) audioVM->DetachCurrentThread(); }
    bool Failed() {
        if (!env || env->ExceptionCheck()) {
            if (env) { env->ExceptionDescribe(); env->ExceptionClear(); }
            return true;
        }
        return false;
    }
};
constexpr jint musicStream = 3;
}

extern "C" JNIEXPORT void JNICALL
Java_io_entisgls_launcher_GameActivity_nativeConfigureAudio(JNIEnv* env, jclass, jobject manager) {
    std::lock_guard<std::mutex> lock(audioMutex);
    if (audioManager) env->DeleteGlobalRef(audioManager);
    audioManager = env->NewGlobalRef(manager);
    env->GetJavaVM(&audioVM);
    jclass cls = env->GetObjectClass(manager);
    getVolume = env->GetMethodID(cls, "getStreamVolume", "(I)I");
    getMaxVolume = env->GetMethodID(cls, "getStreamMaxVolume", "(I)I");
    setVolume = env->GetMethodID(cls, "setStreamVolume", "(III)V");
    env->DeleteLocalRef(cls);
}

bool LegacyGetDeviceVolume(double& volume) {
    std::lock_guard<std::mutex> lock(audioMutex);
    AttachedEnv call;
    if (!call.env || !audioManager || !getVolume || !getMaxVolume) return false;
    const jint current = call.env->CallIntMethod(audioManager, getVolume, musicStream);
    if (call.Failed()) return false;
    const jint maximum = call.env->CallIntMethod(audioManager, getMaxVolume, musicStream);
    if (call.Failed() || maximum <= 0) return false;
    volume = double(current) / maximum;
    return true;
}

bool LegacySetDeviceVolume(double volume) {
    if (!std::isfinite(volume)) return false;
    std::lock_guard<std::mutex> lock(audioMutex);
    AttachedEnv call;
    if (!call.env || !audioManager || !setVolume || !getMaxVolume) return false;
    const jint maximum = call.env->CallIntMethod(audioManager, getMaxVolume, musicStream);
    if (call.Failed() || maximum <= 0) return false;
    const jint value = std::lround(std::clamp(volume, 0.0, 1.0) * maximum);
    call.env->CallVoidMethod(audioManager, setVolume, musicStream, value, 0);
    return !call.Failed();
}
