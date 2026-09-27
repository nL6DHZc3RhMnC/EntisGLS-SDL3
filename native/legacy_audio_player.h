#pragma once
#include "legacy_resource.h"
#include <vector>

class ECSAudioPlayer:public ECSResource {
public:
    DECLARE_CLASS_INFO(ECSAudioPlayer,ECSResource)
    struct Mark { uint64_t position=0,length=0; };
    ECSAudioPlayer()=default;
    const wchar_t *GetTypeName() const override;
    ECSObject *GetTypeOf(const wchar_t *) override;
    ECSObject *Duplicate() override;
    ESLError Move(ECSContext &,ECSObject *) override;
    ESLError Release() override;
    ESLError Save(ESLFileObject &,ECSContext &) override;
    ESLError Load(ESLFileObject &,ECSContext &) override;
    ESLError GetFunction(ECSContext &,int &,const wchar_t *) override;
    ESLError CallFunction(ECSContext &,int,ECSObjArray<ECSObject> &) override;
    ESLError ReplaceMarkPortion(ECSContext &,const wchar_t *,const std::vector<EWideString> &);
    ESLError ReplaceMarkPortionArray(ECSContext &,const std::vector<Mark> &,const std::vector<EWideString> &);
    struct PCM { SakuraGL::SGLSoundFormat format; std::vector<uint8_t> data; };
    static ESLError Decode(SakuraGL::SGLAudioPlayerInterface &,PCM &);
    static ESLError Compose(PCM &,const PCM &,const std::vector<Mark> &,const std::vector<PCM> &);
    ESLError LoadPCM(const PCM &);
private:
    // Original AudioPlayer wire retains a source MIO plus a single replacement
    // recipe; it never serializes the transient composed player as a filename.
    EWideString markFile_;
    std::vector<EWideString> replacementFiles_;
    std::vector<Mark> replacementMarks_;
    void ClearRecipe();
};
class ECSEnvironment;
bool CheckLegacyAudioPlayer(ECSEnvironment &);
