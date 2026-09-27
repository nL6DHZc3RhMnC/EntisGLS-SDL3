#pragma once
#include <sakuragl/sakuragl.h>
#include <sakuraglx/ui/sglx_joy_stick.h>
#include <array>
#include <deque>
#include <map>
#include <mutex>
#include <string>

class ECSWindow;

// Legacy InputFilter keeps its mapped-event queue and 32-bit virtual-pad state.
// Physical gamepads are sampled through the SDK's actual Android input backend.
class ECSInputFilter : public ECSObject {
public:
    DECLARE_CLASS_INFO(ECSInputFilter, ECSObject)
    enum Device { Keyboard, Mouse, JoyStick, Command, Signal };
    struct InputEvent {
        int device = Keyboard, number = 0, key = 0, priority = 0;
        std::wstring command;
        bool operator<(const InputEvent &) const;
    };
    ECSInputFilter();
    ~ECSInputFilter() override;
    const wchar_t *GetTypeName() const override;
    ECSObject *GetTypeOf(const wchar_t *) override;
    ECSObject *Duplicate() override;
    ESLError Move(ECSContext &, ECSObject *) override;
    ESLError UnaryOperate(ECSContext &, CSUnaryOperatorType) override;
    ESLError Operate(ECSContext &, CSOperatorType, ECSObject *) override;
    ESLError Compare(ECSContext &, int &, CSCompareType, ECSObject &) override;
    ESLError GetFunction(ECSContext &, int &, const wchar_t *) override;
    ESLError CallFunction(ECSContext &, int, ECSObjArray<ECSObject> &) override;
    void IndexAllMember() override;
    void CleanupAllReference(ECSContext &) override;
    ESLError CommitAllReference(ECSContext &) override;
    ESLError Save(ESLFileObject &, ECSContext &) override;
    ESLError Load(ESLFileObject &, ECSContext &) override;
    void OnDestruction(ECSContext &) override;

    ESLError LoadInputFilter(const wchar_t *, ECSContext *);
    ESLError LoadInputFilter(EDescription &);
    ESLError SaveInputFilter(EDescription &);
    void DeleteInputFilter();
    ESLError OpenFilter(ECSWindow *, int mode, ECSContext * = nullptr);
    void CloseFilter();
    bool ProcessEvent(const InputEvent &, bool down);
    ESLError GetInputEvent(InputEvent &, uint32_t timeout = 0, ECSContext * = nullptr);
    ESLError FlushInputQueue(int limit);
    ESLError AddFilter(const InputEvent &, const InputEvent &);
    ESLError RemoveFilter(const InputEvent &);
    bool GetFilter(InputEvent &result, const InputEvent &input);
    bool IsJoyButtonPushing(int key, int device = 0);
    int GetJoyButtonPushed(int key, int device = 0);
    ESLError FlushJoyButtonPushed(int device = 0, int key = -1);
    ESLError ResetJoyButtonPushing(int device = 0, int key = -1);
    uint32_t GetCapturedJoyStick();
    ESLError GetStickPosition(SakuraGL::S4DVector &, int device = 0);

    // Window calls with logical client coordinates and legacy VK mouse codes.
    bool OnKey(int64_t key, int64_t flags, bool down);
    bool OnMouseButton(int code, double x, double y, bool down);
    void OnMouseMove(double x, double y);
    bool OnMouseWheel(int32_t delta, double x, double y);
    void OnWindowDestroyed(ECSWindow *);
    void OnWindowReady(ECSWindow *);
    void OnFocusLost();
    void GetCursorPosition(double &x, double &y);
    ESLError MoveCursorPosition(int x,int y);

    static const wchar_t *m_pwszFuncName[19];
private:
    std::recursive_mutex mutex_;
    std::map<InputEvent, InputEvent> filters_;
    std::deque<InputEvent> queue_;
    std::array<std::array<uint32_t,36>,6> buttons_{};
    std::array<uint64_t,6> physicalButtons_{};
    std::array<SakuraGL::UI::SGLJoyStickState,6> physicalState_;
    SakuraGL::UI::SGLJoyStick joystick_;
    bool capturing_ = false, capsHeld_ = false;
    int64_t modifierFlags_ = 0;
    uint32_t captured_ = 0, flags_ = 0, mode_ = 0;
    int joyCount_ = 6, limit_ = 0;
    double threshold_ = 0.01, cursorX_ = 0, cursorY_ = 0;
    HANDLE ready_;
    ECSReference windowReference_;
    ECSWindow *window_ = nullptr;
    void PollJoyStick();
    bool Consume(bool mapped);
    static ESLError ReadEvent(InputEvent &, EDescription &);
    static void WriteEvent(EDescription &, const InputEvent &);
    static InputEvent FromStructure(ECSStructure &);
    static void ToStructure(ECSStructure &, const InputEvent &);
};
