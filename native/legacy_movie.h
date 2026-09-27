#pragma once
#include "legacy_sprite.h"
class ECSMovieSprite:public ECSSprite {
public:
    DECLARE_CLASS_INFO(ECSMovieSprite,ECSSprite)
    ECSMovieSprite();
    ~ECSMovieSprite() override;
    const wchar_t *GetTypeName() const override;
    ECSObject *GetTypeOf(const wchar_t *) override;
    ECSObject *Duplicate() override;
    ESLError Move(ECSContext &,ECSObject *) override;
    ESLError GetFunction(ECSContext &,int &,const wchar_t *) override;
    ESLError CallFunction(ECSContext &,int,ECSObjArray<ECSObject> &) override;
    ESLError Save(ESLFileObject &,ECSContext &) override;
    ESLError Load(ESLFileObject &,ECSContext &) override;
    ESLError CommitAllReference(ECSContext &) override;
    bool IsPlaying() override;
    SakuraGL::SGLImageObject *GetImage() const override;
    ESLError Release() override;
    ESLError OpenMovieFile(const wchar_t *,ECSEnvironment *);
    ESLError CloseMovie();
    ESLError PlayMovie(uint32_t flags=0,int playType=-1);
    ESLError StopMovie();
    bool IsMoviePlaying() const;
    ESLError SeekFrame(uint64_t);
    uint64_t GetCurrentFrame() const;
    uint64_t GetTotalFrame() const;
    uint64_t GetTotalTime() const;
private:
    class MovieNative;
    MovieNative &Movie() const;
    ECSEnvironment *environment_=nullptr;
    uint32_t movieFlags_=0,loopStart_=0,loopEnd_=UINT32_MAX;
    uint32_t restoredStatus_=0,restoredFrame_=UINT32_MAX;
    bool movieRestorePending_=false;
};
bool CheckLegacyMovie(ECSEnvironment &);
