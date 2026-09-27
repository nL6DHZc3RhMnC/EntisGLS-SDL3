#include "android_game_files.h"
#include "platform/game_files.h"
#include <SDL3/SDL.h>
#include <jni.h>
#include <codecvt>
#include <cstdio>
#include <locale>
#include <memory>
#include <stdexcept>
#include <string>
#include <fcntl.h>
#include <unistd.h>

namespace study::platform::sdl {
namespace {
using entis::io::FileInfo;
JNIEnv* Environment() {
    auto* env = static_cast<JNIEnv*>(SDL_GetAndroidJNIEnv());
    if (!env) throw std::runtime_error("Android document access is unavailable");
    return env;
}
std::string Utf8(JNIEnv* env, jstring text) {
    if (!text) return {};
    const jchar* chars = env->GetStringChars(text, nullptr);
    if (!chars) throw std::runtime_error("Cannot read document provider result");
    const std::u16string value(reinterpret_cast<const char16_t*>(chars), env->GetStringLength(text));
    env->ReleaseStringChars(text, chars);
    return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(value);
}
void Check(JNIEnv* env) {
    if (!env->ExceptionCheck()) return;
    jthrowable error = env->ExceptionOccurred();
    env->ExceptionClear();
    std::string message = "Cannot access the selected game folder; select it again in the launcher";
    jclass type = env->GetObjectClass(error);
    jmethodID method = type ? env->GetMethodID(type, "getMessage", "()Ljava/lang/String;") : nullptr;
    if (method) {
        auto detail = static_cast<jstring>(env->CallObjectMethod(error, method));
        if (!env->ExceptionCheck() && detail) message += ": " + Utf8(env, detail);
    }
    env->ExceptionClear();
    throw std::runtime_error(message);
}
struct Frame {
    JNIEnv* env = Environment();
    Frame() { if (env->PushLocalFrame(32) < 0) { env->ExceptionClear(); throw std::runtime_error("Cannot allocate document access frame"); } }
    ~Frame() { env->PopLocalFrame(nullptr); }
    jstring String(const std::string& text) {
        const auto value = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(text);
        auto result = env->NewString(reinterpret_cast<const jchar*>(value.data()), static_cast<jsize>(value.size()));
        Check(env);
        if (!result) throw std::runtime_error("Cannot allocate document path");
        return result;
    }
};

class DocumentBackend final : public entis::io::Backend {
public:
    DocumentBackend() {
        Frame frame;
        auto* env = frame.env;
        jobject activity = static_cast<jobject>(SDL_GetAndroidActivity());
        if (!activity) throw std::runtime_error("Android game activity is unavailable");
        jclass activityType = env->GetObjectClass(activity);
        jmethodID getter = env->GetMethodID(activityType, "getDocumentTreeAccess",
            "()Lio/entisgls/launcher/sdl/DocumentTreeAccess;");
        Check(env);
        jobject access = env->CallObjectMethod(activity, getter);
        Check(env);
        if (!access) throw std::runtime_error("The selected game folder has no document-tree grant");
        jclass type = env->GetObjectClass(access);
        stat_ = env->GetMethodID(type, "stat", "(Ljava/lang/String;)[J"); Check(env);
        list_ = env->GetMethodID(type, "list", "(Ljava/lang/String;)[Ljava/lang/String;"); Check(env);
        open_ = env->GetMethodID(type, "open", "(Ljava/lang/String;Ljava/lang/String;)I"); Check(env);
        mkdir_ = env->GetMethodID(type, "mkdir", "(Ljava/lang/String;)V"); Check(env);
        remove_ = env->GetMethodID(type, "remove", "(Ljava/lang/String;Z)V"); Check(env);
        rename_ = env->GetMethodID(type, "rename", "(Ljava/lang/String;Ljava/lang/String;)V"); Check(env);
        object_ = env->NewGlobalRef(access); Check(env);
        if (!object_) throw std::runtime_error("Cannot retain document-tree access");
    }
    ~DocumentBackend() override {
        if (auto* env = static_cast<JNIEnv*>(SDL_GetAndroidJNIEnv())) env->DeleteGlobalRef(object_);
    }
    FileInfo Stat(const std::string& path) override {
        Frame frame;
        auto value = static_cast<jlongArray>(frame.env->CallObjectMethod(object_, stat_, frame.String(path)));
        Check(frame.env);
        if (!value || frame.env->GetArrayLength(value) != 4) throw std::runtime_error("Invalid document metadata");
        jlong data[4]{};
        frame.env->GetLongArrayRegion(value, 0, 4, data); Check(frame.env);
        if (data[0] < 0 || data[0] > 2 || data[1] < 0) throw std::runtime_error("Invalid document kind or length");
        FileInfo result;
        result.kind = data[0] == 1 ? FileInfo::Kind::File : data[0] == 2 ? FileInfo::Kind::Directory : FileInfo::Kind::Missing;
        result.size = static_cast<std::uint64_t>(data[1]);
        result.modifiedMs = data[2]; result.writable = data[3] != 0;
        return result;
    }
    std::vector<entis::io::Entry> List(const std::string& path) override {
        Frame frame;
        auto names = static_cast<jobjectArray>(frame.env->CallObjectMethod(object_, list_, frame.String(path)));
        Check(frame.env);
        if (!names) throw std::runtime_error("Cannot enumerate selected game folder");
        std::vector<entis::io::Entry> result;
        for (jsize i = 0; i < frame.env->GetArrayLength(names); ++i) {
            auto name = static_cast<jstring>(frame.env->GetObjectArrayElement(names, i));
            Check(frame.env);
            auto text = Utf8(frame.env, name);
            frame.env->DeleteLocalRef(name);
            const auto child = path.empty() ? text : path + "/" + text;
            result.push_back({text, Stat(child)});
        }
        return result;
    }
    FILE* Open(const std::string& path, const char* mode) override {
        const std::string value(mode);
        if (value.empty() || (value.front() != 'r' && value.front() != 'w' && value.front() != 'a'))
            throw std::runtime_error("Unsupported document stream mode");
        const auto existing = Stat(path);
        if (value.front() == 'r' && existing.kind != FileInfo::Kind::File)
            throw std::runtime_error("Game document does not exist: " + path);
        const bool appendUpdate = value.front() == 'a' && value.find('+') != std::string::npos;
        const char* javaMode = value.find('w') != std::string::npos ? "rwt" :
            appendUpdate ? (existing.kind == FileInfo::Kind::Missing ? "rwt" : "rw") :
            value.find('a') != std::string::npos ? "wa" : value.find('+') != std::string::npos ? "rw" : "r";
        Frame frame;
        const int fd = frame.env->CallIntMethod(object_, open_, frame.String(path), frame.String(javaMode));
        Check(frame.env);
        if (fd < 0) throw std::runtime_error("Cannot open selected game document: " + path);
        if (value.front() == 'a') {
            const int flags = ::fcntl(fd, F_GETFL);
            if (flags < 0 || ::fcntl(fd, F_SETFL, flags | O_APPEND) < 0) {
                ::close(fd); throw std::runtime_error("Cannot append to game document: " + path);
            }
        }
        FILE* file = ::fdopen(fd, mode);
        if (!file) { ::close(fd); throw std::runtime_error("Cannot open document stream: " + path); }
        return file;
    }
    void CreateDirectory(const std::string& path) override {
        Frame frame; frame.env->CallVoidMethod(object_, mkdir_, frame.String(path)); Check(frame.env);
    }
    void Remove(const std::string& path, bool directory) override {
        Frame frame; frame.env->CallVoidMethod(object_, remove_, frame.String(path), static_cast<jboolean>(directory)); Check(frame.env);
    }
    void Rename(const std::string& oldPath, const std::string& newPath) override {
        Frame frame; frame.env->CallVoidMethod(object_, rename_, frame.String(oldPath), frame.String(newPath)); Check(frame.env);
    }
private:
    jobject object_ = nullptr;
    jmethodID stat_{}, list_{}, open_{}, mkdir_{}, remove_{}, rename_{};
};
}
void ConfigureAndroidGameFiles(const std::string& root) {
    if (root.compare(0, 15, "/__entis_saf__/") == 0)
        entis::io::SetBackend(root, std::make_shared<DocumentBackend>());
}
}
