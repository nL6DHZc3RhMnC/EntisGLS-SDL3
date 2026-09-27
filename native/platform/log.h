#pragma once

#include <cstdarg>

#if defined(STUDYSTEADY_PLATFORM_SDL3)
#include <SDL3/SDL_log.h>
#include <string>
#elif defined(__ANDROID__)
#include <android/log.h>
#else
#include <cstdio>
#include <mutex>
#endif

namespace study::platform {

enum class LogPriority { Verbose, Debug, Info, Warn, Error, Fatal };

inline void LogVPrint(LogPriority priority, const char* tag, const char* format,
                      std::va_list args) {
#if defined(STUDYSTEADY_PLATFORM_SDL3)
    SDL_LogPriority nativePriority = SDL_LOG_PRIORITY_INFO;
    switch (priority) {
        case LogPriority::Verbose: nativePriority = SDL_LOG_PRIORITY_VERBOSE; break;
        case LogPriority::Debug: nativePriority = SDL_LOG_PRIORITY_DEBUG; break;
        case LogPriority::Info: nativePriority = SDL_LOG_PRIORITY_INFO; break;
        case LogPriority::Warn: nativePriority = SDL_LOG_PRIORITY_WARN; break;
        case LogPriority::Error: nativePriority = SDL_LOG_PRIORITY_ERROR; break;
        case LogPriority::Fatal: nativePriority = SDL_LOG_PRIORITY_CRITICAL; break;
    }
    // SDL has numeric categories instead of Android's string tags. Keep the
    // original tag in the message, escaping '%' before it becomes format text.
    std::string taggedFormat;
    for (const char* current = tag ? tag : ""; *current; ++current) {
        taggedFormat += *current;
        if (*current == '%') taggedFormat += '%';
    }
    taggedFormat += ": ";
    taggedFormat += format;
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, nativePriority,
                    taggedFormat.c_str(), args);
#elif defined(__ANDROID__)
    int nativePriority = ANDROID_LOG_INFO;
    switch (priority) {
        case LogPriority::Verbose: nativePriority = ANDROID_LOG_VERBOSE; break;
        case LogPriority::Debug: nativePriority = ANDROID_LOG_DEBUG; break;
        case LogPriority::Info: nativePriority = ANDROID_LOG_INFO; break;
        case LogPriority::Warn: nativePriority = ANDROID_LOG_WARN; break;
        case LogPriority::Error: nativePriority = ANDROID_LOG_ERROR; break;
        case LogPriority::Fatal: nativePriority = ANDROID_LOG_FATAL; break;
    }
    __android_log_vprint(nativePriority, tag, format, args);
#else
    const char* level = "INFO";
    switch (priority) {
        case LogPriority::Verbose: level = "VERBOSE"; break;
        case LogPriority::Debug: level = "DEBUG"; break;
        case LogPriority::Info: level = "INFO"; break;
        case LogPriority::Warn: level = "WARN"; break;
        case LogPriority::Error: level = "ERROR"; break;
        case LogPriority::Fatal: level = "FATAL"; break;
    }
    static std::mutex outputMutex;
    const std::lock_guard<std::mutex> lock(outputMutex);
    std::fprintf(stderr, "%s/%s: ", level, tag ? tag : "");
    std::vfprintf(stderr, format, args);
    std::fputc('\n', stderr);
#endif
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 3, 4)))
#endif
inline void LogPrint(LogPriority priority, const char* tag, const char* format, ...) {
    std::va_list args;
    va_start(args, format);
    LogVPrint(priority, tag, format, args);
    va_end(args);
}

inline void LogWrite(LogPriority priority, const char* tag, const char* message) {
    LogPrint(priority, tag, "%s", message ? message : "");
}

} // namespace study::platform
