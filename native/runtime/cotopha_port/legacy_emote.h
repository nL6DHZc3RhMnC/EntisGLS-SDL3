#pragma once
#include <memory>
#include <deque>
#include <string>
#include <vector>

class LegacyMotionService;

class ECSEmoteDevice : public ECSObject {
public:
    DECLARE_CLASS_INFO(ECSEmoteDevice, ECSObject)
    explicit ECSEmoteDevice(ECSEnvironment& environment);
    ~ECSEmoteDevice() override;
    const wchar_t* GetTypeName() const override;
    ECSObject* GetTypeOf(const wchar_t*) override;
    ECSObject* Duplicate() override;
    ESLError Move(ECSContext&, ECSObject*) override;
    ESLError UnaryOperate(ECSContext&, CSUnaryOperatorType) override;
    ESLError Operate(ECSContext&, CSOperatorType, ECSObject*) override;
    ESLError Compare(ECSContext&, int&, CSCompareType, ECSObject&) override;
    ESLError GetFunction(ECSContext&, int&, const wchar_t*) override;
    ESLError CallFunction(ECSContext&, int, ECSObjArray<ECSObject>&) override;
    ECSObject* GetVariableAt(int index) override;
    void IndexAllMember() override;
    void CleanupAllReference(ECSContext&) override;
    ESLError CommitAllReference(ECSContext&) override;
    ESLError Save(ESLFileObject&, ECSContext&) override;
    ESLError Load(ESLFileObject&, ECSContext&) override;
    std::shared_ptr<LegacyMotionService> Service() const { return service_; }
private:
    ECSEnvironment& environment_;
    std::shared_ptr<LegacyMotionService> service_;
    ECSReference window_;
};

class ECSEmoteSprite : public ECSSprite {
public:
    DECLARE_CLASS_INFO(ECSEmoteSprite, ECSSprite)
    ECSEmoteSprite();
    ~ECSEmoteSprite() override;
    const wchar_t* GetTypeName() const override;
    ECSObject* GetTypeOf(const wchar_t*) override;
    ECSObject* Duplicate() override;
    ESLError Move(ECSContext&, ECSObject*) override;
    ESLError GetFunction(ECSContext&, int&, const wchar_t*) override;
    ESLError CallFunction(ECSContext&, int, ECSObjArray<ECSObject>&) override;
    ESLError Release() override;
    ECSObject* GetVariableAt(int index) override;
    void IndexAllMember() override;
    void CleanupAllReference(ECSContext&) override;
    ESLError CommitAllReference(ECSContext&) override;
    ESLError Save(ESLFileObject&, ECSContext&) override;
    ESLError Load(ESLFileObject&, ECSContext&) override;
private:
    class MotionNative;
    void ReleaseMotionPlayer();
    ESLError OpenPlayer(ECSContext&, ECSEmoteDevice&, const wchar_t*);
    void AdvanceMotion(uint32_t milliseconds);
    void DrawMotion();
    void TraceMotion(const char* stage);
    std::shared_ptr<LegacyMotionService> service_;
    uint64_t project_ = 0;
    uint64_t player_ = 0;
    uint32_t width_ = 1280, height_ = 1024;
    std::wstring playerPath_;
    float scale_ = 1, coordX_ = 0, coordY_ = 0;
    bool restorePlayer_ = false;
    std::string timeline_;
    std::string defaultTimeline_;
    std::deque<std::string> timelineQueue_;
    double physicsWeight_ = 1;
    bool physicsAnimation_ = true;
    bool pendingPhysicsDraw_ = false;
    std::vector<uint8_t> framePixels_;
    ECSReference device_, voice_;
    std::string voiceVariable_;
    std::vector<double> voiceCurve_;
    uint32_t voiceSamplesPerFrame_ = 0;
    double voiceGain_ = 5;
    bool voiceWasPlaying_ = false;
    bool renderFailed_ = false;
    uint32_t motionTraceFrames_ = 0;
    uint32_t motionTraceAdvances_ = 0;
};

bool CheckLegacyMotionOwner(ECSEnvironment& environment);
bool CheckLegacyEmoteWindow(ECSContext&, ECSEnvironment&);
