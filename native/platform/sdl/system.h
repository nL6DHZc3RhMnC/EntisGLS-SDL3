#pragma once

#include <cstdint>
#include <cstddef>
#include <string>

struct SDL_Window;
namespace SSystem {
class SFileOpener;
class SString;
struct MEMORY_STATUS;
struct PLATFORM_INFORMATION;
struct DATE_TIME;
}

namespace study::platform::sdl {

// UTF-8 paths supplied by the application, before SakuraGL::Initialize().
// storageRoot resolves storage://; gameRoot can override its virtual game/
// directory. localRoot contains savedata/. An empty assetsRoot explicitly
// selects SDL's packaged-asset lookup (for example an Android APK).
// All non-empty roots are absolute paths. Optional schemes are not registered
// unless their roots are supplied; no host paths or public-storage assumptions
// are baked into the backend.
struct SystemPaths {
    std::string assetsRoot;
    std::string storageRoot;
    std::string localRoot;
    std::string gameRoot;   // optional storage://game override; no resource copy
    std::string sharedRoot; // optional compatibility sd://
    std::string dataRoot;   // optional compatibility data://
};

bool ConfigureSystemPaths(const SystemPaths& paths);
void RequireSystemPaths();
void RegisterSystemSchemes();
void ReleaseSystemPaths();
void InstallStdoutLogging();
void SetDialogParent(SDL_Window* window);

// SDL lacks an API for resolving statically linked symbols in the current
// executable. The application can supply its native-registration resolver.
using SymbolResolver = void* (*)(const char* symbol, void* userdata);
void SetSymbolResolver(SymbolResolver resolver, void* userdata = nullptr);
std::uintptr_t ResolveSymbol(const wchar_t* name, const wchar_t* reserved);
int ResolveDefaultDirectory(SSystem::SString& directory, const wchar_t* placement, const wchar_t* option);

void FillMemoryStatus(SSystem::MEMORY_STATUS& status);
void FillPlatformInformation(SSystem::PLATFORM_INFORMATION& information);
void FillLocalDate(SSystem::DATE_TIME& date);
std::int32_t LocalTimeDifference(wchar_t* name, std::size_t capacity);
int CPUFamily();
std::uint64_t CPUFeatures();
int ShowMessageBox(const wchar_t* message, const wchar_t* caption, int style);

} // namespace study::platform::sdl
