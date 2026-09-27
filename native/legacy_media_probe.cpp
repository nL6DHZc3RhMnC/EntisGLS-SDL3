#include "legacy_compat/gls.h"
#include "legacy_media_probe.h"
#include "legacy_sprite.h"
#include "legacy_resource_manager.h"
#include "legacy_device_volume.h"
#include "platform/log.h"
#include <memory>
#include <vector>

namespace {
bool Check(bool success, const char *stage) {
    if (!success) study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady", "Legacy media probe FAIL: %s", stage);
    return success;
}
bool Invoke(ECSContext &context, ECSSprite &sprite, const wchar_t *method,
            std::initializer_list<INT64> integers) {
    int index;
    if (sprite.GetFunction(context, index, method)) return false;
    ECSObjArray<ECSObject> args;
    args.Add(new ECSReference(&sprite));
    for (auto value : integers) args.Add(new ECSInteger(value));
    if (sprite.CallFunction(context, index, args)) return false;
    std::unique_ptr<ECSObject> result(context.PopObject());
    auto *integer = ESLTypeCast<ECSInteger>(result.get());
    return integer && integer->GetValue() == 0;
}
}

bool CheckLegacyMedia(ECSEnvironment &environment) {
    study::platform::LogWrite(study::platform::LogPriority::Info, "StudySteady", "Legacy media probe: real ERI/MIO and Sprite");
    ECSResource image;
    std::unique_ptr<ESLFileObject> imageFile(environment.OpenFileObject("particle_light1.eri"));
    if (!Check(imageFile && !image.ReadImageFile(*imageFile) && image.GetImage(), "ERI image load")) return false;
    SakuraGL::SGLImageInfo info;
    SakuraGL::SGLPalette pixel;
    if (!Check(!image.GetImage()->GetImageInfo(info) && info.width && info.height &&
        !image.GetImage()->GetPixelRGBA(pixel, 0, 0), "ERI image metadata and pixel")) return false;
    study::platform::LogPrint(study::platform::LogPriority::Info, "StudySteady", "Legacy ERI decoded: %ux%u depth=%u", info.width, info.height, info.depth);

    ECSResource sound;
    std::unique_ptr<ESLFileObject> soundFile(environment.OpenFileObject("se517.mio"));
    if (!Check(soundFile && !sound.ReadSoundFile(*soundFile) && sound.GetSound(), "MIO audio load")) return false;
    if (!Check(sound.GetSound()->GetSampleFrequency() && sound.GetSound()->GetTotalLength(), "MIO audio metadata")) return false;
    study::platform::LogPrint(study::platform::LogPriority::Info, "StudySteady", "Legacy MIO decoded: rate=%u samples=%llu",
        sound.GetSound()->GetSampleFrequency(), static_cast<unsigned long long>(sound.GetSound()->GetTotalLength()));
    if (!Check(!sound.SetVolume(0, 0) && !sound.Play() && !sound.Stop(), "muted native audio play/stop")) return false;

    ECSContext context;
    ECSSprite sprite, child;
    if (!Check(Invoke(context, sprite, L"CreateSprite", {1, 4, 3}) && sprite.GetImage() &&
        !sprite.GetImage()->GetImageInfo(info) && info.width == 4 && info.height == 3,
        "script Sprite.CreateSprite allocates a real image")) return false;
    if (!Check(Invoke(context, sprite, L"MovePosition", {17, -9}) &&
        Invoke(context, sprite, L"SetTransparency", {128}) && Invoke(context, sprite, L"SetVisible", {0}) &&
        sprite.NativeSprite().GetPosition().x == 17 && sprite.NativeSprite().GetPosition().y == -9 &&
        sprite.NativeSprite().GetTransparency() == 128 && !sprite.NativeSprite().IsVisible(),
        "script Sprite properties reach native SGLSprite")) return false;
    int addIndex;
    if (!Check(!sprite.GetFunction(context, addIndex, L"AddSprite"), "Sprite.AddSprite binding")) return false;
    {
        ECSObjArray<ECSObject> args;
        args.Add(new ECSReference(&sprite));
        args.Add(new ECSInteger(5));
        args.Add(new ECSReference(&child));
        if (!Check(!sprite.CallFunction(context, addIndex, args) &&
            child.NativeSprite().GetParent() == &sprite.NativeSprite() &&
            child.NativeSprite().GetPriority() == 5, "native Sprite parenting")) return false;
        delete context.PopObject();
    }
    if (!Check(Invoke(context, sprite, L"DetachAllSprite", {}) && child.NativeSprite().GetParent() == nullptr,
        "native Sprite detach")) return false;
    double deviceVolume;
    if (!Check(LegacyGetDeviceVolume(deviceVolume) && deviceVolume >= 0 && deviceVolume <= 1,
#if defined(STUDYSTEADY_PLATFORM_SDL3)
        "actual SDL application master gain (all active streams)")) return false;
#else
        "actual Android media stream volume")) return false;
#endif
    ECSResourceManager manager;
    std::unique_ptr<ESLFileObject> skinFile(environment.OpenFileObject("wm_langpicker.noa"));
    if (!Check(skinFile && !manager.ReadSkinFile(*skinFile) && manager.m_varArray.GetSize() == 5,
        "actual language picker skin resources")) return false;
    ECSSprite page;
    if (!Check(!page.BuildFormPage(manager.GetSkin(), L"ID_LANGPICKER_FRAME") &&
        page.NativeSprite().GetItemAs(L"ID_LANGPICKER_JA") &&
        page.NativeSprite().GetItemAs(L"ID_LANGPICKER_BG"), "actual language picker form and buttons")) return false;
    // A page retains its native skin even when the script manager is released.
    manager.Release();
    if (!Check(page.NativeSprite().GetItemAs(L"ID_LANGPICKER_JA") && !page.Release(),
        "formed page owns resource lifetime")) return false;
    study::platform::LogWrite(study::platform::LogPriority::Info, "StudySteady", "Legacy skin probe PASS: five real language-picker resources, page/buttons, ownership");
    study::platform::LogWrite(study::platform::LogPriority::Info, "StudySteady", "Legacy media probe PASS: ERI decode, muted MIO play/stop, Sprite image/properties/hierarchy");
    return true;
}
