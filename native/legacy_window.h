#pragma once
#include "legacy_sprite.h"
#include <sakuraglx/sprite/sglx_sprite_window.h>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <string>

class ECSInputFilter;

// Traditional Window methods operate on the SDK's actual Android virtual
// window. Its root contains ECSSprite::sprite_, just as GLS3 EInterface did.
class ECSWindow : public ECSSprite {
public:
    DECLARE_CLASS_INFO(ECSWindow, ECSSprite)
    ECSWindow();
    ~ECSWindow() override;
    const wchar_t *GetTypeName() const override;
    ECSObject *GetTypeOf(const wchar_t *) override;
    ECSObject *Duplicate() override;
    ESLError GetFunction(ECSContext &, int &, const wchar_t *) override;
    ESLError CallFunction(ECSContext &, int, ECSObjArray<ECSObject> &) override;
    ESLError Save(ESLFileObject &, ECSContext &) override;
    ESLError Load(ESLFileObject &, ECSContext &) override;
    void OnDestruction(ECSContext &) override;
    SakuraGL::SGLImageObject *GetImage() const override;

    SakuraGL::SGLWindowSprite *GetWindow() const;
    SakuraGL::SGLWindowSprite *NativeWindow() const { return GetWindow(); }
    ESLError CloseDisplay();
    ESLError MoveLogicalCursor(int x,int y);
    void AttachInputFilter(ECSInputFilter *, int mode);
    void DetachInputFilter(ECSInputFilter *);
    void QueueInputCommand(const wchar_t *, int priority, bool signal);
    void QueueCommand(const wchar_t *, int64_t notification = 0,
                      int64_t parameter = 0, int priority = 0, bool overwrite = false);

private:
    friend bool CheckLegacyWindowCommandQueue();
    void QueueCommandImpl(const wchar_t*,int64_t,int64_t,int,bool,bool);
    class WindowBridge;
    std::unique_ptr<WindowBridge> window_;
    std::unique_ptr<SakuraGL::SGLSprite::Buffer> snapshot_;
    ESLError RefreshSnapshot(int width=0,int height=0);
    struct Command {
        std::wstring id, fullID;
        int64_t notification = 0, parameter = 0;
        int priority = 0;
    };
    std::mutex queueMutex_;
    std::condition_variable queueChanged_;
    std::deque<Command> commands_;
    bool queueEnabled_ = false;
    bool closed_ = true;
    // SDK callbacks and filter detachment share this lock, so the callback
    // cannot retain a pointer after DetachInputFilter has returned.
    std::recursive_mutex filterMutex_;
    ECSInputFilter *inputFilter_ = nullptr;
    int filterMode_ = 0;
    bool NotifyCommand(const wchar_t *, int64_t, int64_t, int, bool);
    ESLError GetCommand(ECSContext &, Command &, int64_t timeout, bool remove);
    void WindowReady();
};

// Uses actual native button transitions; no Android display is created.
bool CheckLegacyWindowCommandQueue();
