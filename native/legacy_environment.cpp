#include <gls.h>
#include "legacy_environment.h"
#include "platform/log.h"

SSystem::SError ECSEnvironment::InitEnvironment() {
    const auto result = ECSSakura2::EnvironmentVM::InitEnvironment();
    RegisterEnvironmentString(L"CURRENT", L"storage://game");
    return result;
}

EWideString ECSEnvironment::FilterFilePath(const wchar_t *path) {
    SSystem::SString value(path);
    FilterEnvironmentString(value);
    return EWideString(static_cast<const wchar_t *>(value));
}

ESLFileObject *ECSEnvironment::OpenFileObject(const char *name, long flags) {
    if (!name) return nullptr;
    const EWideString wideName(name);
    const EWideString filtered = FilterFilePath(wideName);
    SSystem::SEnvironment::AcceptAllFileForWriting(m_fAcceptOtherSaveDir);
    auto *file = ECSSakura2::EnvironmentVM::NewOpenFile(filtered, flags);
    return file ? new LegacyFileAdapter(file, flags) : nullptr;
}

ESLError ECSEnvironment::AddFileArchive(const wchar_t *path, const char *password,
                                      const wchar_t *id) {
    if (!path) return eslErrInvalidParam;
    const EWideString filtered = FilterFilePath(path);
    const EWideString widePassword(password ? password : "");
    auto *opener = CreateArchiveOpener(filtered, widePassword);
    if (!opener) return eslErrGeneral;
    // SEnvironment retains ownership and appends, matching the original GLS3
    // file search precedence. Dynamic patches stay in script-defined order.
    AddFileOpener(opener, id ? id : L"");
    study::platform::LogPrint(study::platform::LogPriority::Info, "StudySteady", "Legacy archive mounted: %s",
        SSystem::SString(static_cast<const wchar_t *>(filtered)).ToCharArray().GetConstArray());
    return eslErrSuccess;
}

ESLError ECSEnvironment::EnableFilePath(const wchar_t *id, bool enable) {
    if (!id || FindFileOpenerAs(id) < 0) return eslErrGeneral;
    EnableFileOpener(id, enable);
    return eslErrSuccess;
}

ESLError ECSEnvironment::AddModule(const wchar_t *name, ECSContext *) {
    study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady", "Windows Cotopha plugin unsupported: %s",
        SSystem::SString(name ? name : L"").ToCharArray().GetConstArray());
    return eslErrNotSupported;
}

FARPROC ECSEnvironment::FindPluginedFunction(const char *) {
    // No binary plug-ins have been accepted by AddModule.
    return nullptr;
}

void ECSExecutionImage::AttachCSEnvironment(ECSEnvironment *environment) {
    m_pEnv = environment;
    ECSSakura2::StandardVM::AttachEnvironment(environment);
}
