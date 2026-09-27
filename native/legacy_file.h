#pragma once
#include <windows.h>
#include <esl.h>
#include <sakura/sakura.h>

// Owns the current engine's file handle; the optional opener is borrowed for the
// lifetime of its environment. This keeps archive decoding in the Android SDK.
class LegacyFileAdapter final : public ESLFileObject {
public:
    LegacyFileAdapter(SSystem::SFileInterface *file, int flags,
                      SSystem::SFileOpener *opener = nullptr);
    ESLFileObject *Duplicate() const override;
    unsigned long Read(void *buffer, unsigned long bytes) override;
    unsigned long Write(const void *buffer, unsigned long bytes) override;
    unsigned long GetLength() const override;
    unsigned long Seek(long offset, SeekOrigin origin) override;
    unsigned long GetPosition() const override;
    UINT64 GetLargeLength() const override;
    UINT64 SeekLarge(INT64 offset, SeekOrigin origin) override;
    UINT64 GetLargePosition() const override;
    ESLError SetEndOfFile() override;
    ESLFileObject *OpenFileObject(const wchar_t *path, int flags) override;
private:
    SSystem::SSmartPointer<SSystem::SFileInterface> file_;
    SSystem::SFileOpener *opener_;
};
