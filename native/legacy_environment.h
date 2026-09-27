#pragma once
#include "legacy_file.h"
#include <sakuraglx/sakuraglx.h>

class ECSContext;
struct ECS_PLUGIN_ENTRY_TABLE;

// The traditional interpreter uses the current Android engine's real resource
// environment. It does not load Windows DLLs or emulate their export tables.
class ECSEnvironment : public ECSSakura2::EnvironmentVM, public EDescription {
public:
    using ECSSakura2::EnvironmentVM::operator new;
    using ECSSakura2::EnvironmentVM::operator delete;
    using SSystem::SEnvironment::QueryFileState;
    struct EPlugin {
        bool m_fStartup = false;
        EString m_strModuleName;
        HMODULE m_hModule = nullptr;
        ECS_PLUGIN_ENTRY_TABLE *m_ppiet = nullptr;
    };
    bool m_fAcceptOtherSaveDir = false;
    EObjArray<EPlugin> m_lstModule;

    SSystem::SError InitEnvironment() override;
    ESLFileObject *OpenFileObject(const char *name,
        long flags = ESLFileObject::modeRead | ESLFileObject::shareRead);
    EWideString FilterFilePath(const wchar_t *path);
    FARPROC FindPluginedFunction(const char *name);
    ESLError AddModule(const wchar_t *name, ECSContext *context = nullptr);
    ESLError AddFileArchive(const wchar_t *path, const char *password,
                           const wchar_t *id = nullptr);
    ESLError EnableFilePath(const wchar_t *id, bool enable);
};
