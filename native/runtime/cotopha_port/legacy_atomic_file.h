#pragma once
#include "runtime/cotopha_port/legacy_file.h"
#include "runtime/cotopha_port/legacy_atomic_path.h"
class ECSEnvironment;

class LegacyAtomicSaveFile final : public ESLFileObject {
public:
    static LegacyAtomicSaveFile *TryOpen(ECSEnvironment *,const wchar_t *,unsigned,bool &candidate);
    LegacyAtomicSaveFile(std::shared_ptr<LegacyAtomicPath>,unsigned);
    ESLFileObject *Duplicate() const override;
    unsigned long Read(void *,unsigned long) override;
    unsigned long Write(const void *,unsigned long) override;
    unsigned long GetLength() const override;
    unsigned long GetPosition() const override;
    unsigned long Seek(long,SeekOrigin) override;
    UINT64 GetLargeLength() const override;
    UINT64 GetLargePosition() const override;
    UINT64 SeekLarge(INT64,SeekOrigin) override;
    ESLError SetEndOfFile() override;
    void BeginSave();
    ESLError StagePrefix(const void *,size_t);
    ESLError Replace(const void *,size_t,UINT64 prefix);
    ESLError FinishClose();
private:
    std::shared_ptr<LegacyAtomicPath> file_;
    UINT64 position_ = 0;
};
