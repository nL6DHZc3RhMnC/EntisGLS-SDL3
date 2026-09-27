#include "compatibility/sdk/legacy/gls.h"
#include "legacy_volume_envelope_probe.h"
#include "platform/log.h"
#include <chrono>
#include <cmath>
#include <memory>

namespace {
bool Check(bool ok, const char *stage) {
    if (!ok) study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady", "Legacy volume envelope probe FAIL: %s", stage);
    return ok;
}
bool Near(float a, float b) { return std::abs(a-b) < 0.0001f; }
class VolumeProbeResource final : public ECSResource {
public:
    VolumeProbeResource() { m_nPlayType = ptfNothing; }
    LegacyVolumePoint Volume() const {
        std::lock_guard<std::mutex> lock(m_volumeMutex);
        return {m_volume[0].load(), m_volume[1].load()};
    }
    bool BackendVolume(LegacyVolumePoint &point) const {
        std::lock_guard<std::mutex> lock(m_volumeMutex);
        float value[2] = {};
        if (!m_sound || !m_sound->GetPlayer() || m_sound->GetPlayer()->GetVolume(value, 2)) return false;
        point = {value[0], value[1]};
        return true;
    }
};
bool Invoke(ECSContext &context, ECSResource &resource, const wchar_t *name,
            ECSObjArray<ECSObject> &arguments, INT64 expected) {
    int index = -1;
    if (resource.GetFunction(context, index, name) || resource.CallFunction(context, index, arguments)) return false;
    std::unique_ptr<ECSObject> value(context.PopObject());
    INT64 integer = 0;
    return value && !value->OperateInteger(integer) && integer == expected;
}
bool InvokeSimple(ECSContext &context, ECSResource &resource, const wchar_t *name, INT64 expected) {
    ECSObjArray<ECSObject> args;
    args.Add(new ECSReference(&resource));
    return Invoke(context, resource, name, args, expected);
}
}

bool CheckLegacyVolumeEnvelope(ECSEnvironment &environment) {
    const LegacyVolumeCurve nonlinear{{0,1},{0,0},{0,0},{1,0}};
    const LegacyVolumeCurve piecewise{{0,1},{0,0},{0,0},{1,0},{1,0},{1,0},{0,1}};
    const auto mid = LegacyEvaluateVolumeCurve(nonlinear, 0.5);
    const auto quarter = LegacyEvaluateVolumeCurve(piecewise, 0.25);
    const auto threeQuarters = LegacyEvaluateVolumeCurve(piecewise, 0.75);
    if (!Check(Near(mid.left, 0.125f) && Near(mid.right, 0.125f) &&
        Near(quarter.left, 0.125f) && Near(threeQuarters.left, 0.875f) && Near(threeQuarters.right, 0.125f),
        "cubic channel curves and equal-duration multiple segments")) return false;
    VolumeProbeResource resource;
    std::unique_ptr<ESLFileObject> audio(environment.OpenFileObject("se517.mio"));
    if (!Check(audio && !resource.ReadSoundFile(*audio), "real NOA player for envelope")) return false;
    if (!Check(resource.SetVolumeEnvelope({{0,0},{1,1}}, 100) == eslErrInvalidParam &&
        resource.SetVolumeEnvelope({{0,0},{-1,0},{1,1},{1,1}}, 100) == eslErrInvalidParam,
        "unsupported negative controls and malformed curves report errors")) return false;

    ECSContext context;
    // Use real dynamic Structure objects and the exact native Array/Real ABI.
    auto *array = new ECSArray;
    for (const auto &point : nonlinear) {
        auto *vector = new ECSStructure;
        vector->SetMemberAsReal(L"x", point.left);
        vector->SetMemberAsReal(L"y", point.right);
        array->m_varArray.Add(static_cast<ECSArray *>(vector));
    }
    {
        ECSObjArray<ECSObject> args;
        args.Add(new ECSReference(&resource)); args.Add(array); args.Add(new ECSInteger(396));
        if (!Check(Invoke(context, resource, L"SetVolumeEnvelope", args, 0) &&
            InvokeSimple(context, resource, L"IsPendingEnvelope", -1), "native scheduling and legacy -1 pending result")) return false;
    }
    bool observedCurve = false;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (resource.IsPendingEnvelope() && std::chrono::steady_clock::now() < deadline) {
        LegacyVolumePoint backend;
        if (!Check(resource.BackendVolume(backend), "read actual Android player volumes")) return false;
        if (backend.left > 0.001f && backend.left < 0.7f && backend.right > 0.001f) {
            // On this curve L=t^3, R=(1-t)^3. This relation distinguishes the
            // real cubic from a linear fade without depending on tick timing.
            if (!Check(std::abs(std::cbrt(backend.left)+std::cbrt(backend.right)-1) < 0.002,
                "actual backend follows nonlinear stereo curve")) return false;
            observedCurve = true;
        }
        Sleep(10);
    }
    auto final = resource.Volume();
    LegacyVolumePoint backend;
    if (!Check(observedCurve && !resource.IsPendingEnvelope() && !resource.GetVolumeEnvelopeError() &&
        Near(final.left,1) && Near(final.right,0) && resource.BackendVolume(backend) &&
        Near(backend.left,1) && Near(backend.right,0), "natural completion writes exact endpoint to backend")) return false;

    if (!Check(!resource.SetVolumeEnvelope(piecewise, 0), "zero-duration multiple segments")) return false;
    final = resource.Volume();
    if (!Check(Near(final.left,1) && Near(final.right,0) && !resource.IsPendingEnvelope(),
        "zero duration retains GLS3 point-three behavior")) return false;
    if (!Check(!resource.SetVolumeEnvelope(nonlinear, 1000), "start cancellable envelope")) return false;
    Sleep(80);
    if (!Check(InvokeSimple(context, resource, L"CancelVolumeEnvelope", 0), "native cancellation joins worker")) return false;
    const auto cancelled = resource.Volume(); Sleep(90); final = resource.Volume();
    if (!Check(!resource.IsPendingEnvelope() && Near(final.left,cancelled.left) && Near(final.right,cancelled.right),
        "cancel does not jump to endpoint or continue writing")) return false;
    if (!Check(!resource.SetVolumeEnvelope(nonlinear, 1000) && !resource.SetVolumeEnvelope(piecewise, 0),
        "replacement cancels prior worker")) return false;
    Sleep(90); final = resource.Volume();
    if (!Check(Near(final.left,1) && Near(final.right,0), "old worker cannot overwrite replacement")) return false;
    if (!Check(!resource.SetVolumeEnvelope(nonlinear, 1000) &&
        InvokeSimple(context, resource, L"Stop", 0) && !resource.IsPendingEnvelope(),
        "script Stop also cancels the envelope per GLS3")) return false;
    if (!Check(!resource.SetVolumeEnvelope(nonlinear, 1000) && !resource.Release() &&
        !resource.IsPendingEnvelope() && !resource.GetSound(), "release joins before destroying audio target")) return false;

    auto source = std::make_unique<ECSResource>();
    audio->SeekLarge(0, ESLFileObject::FromBegin);
    if (!Check(!source->ReadSoundFile(*audio), "source for hidden audio reference")) return false;
    {
        ECSObjArray<ECSObject> args;
        args.Add(new ECSReference(&resource)); args.Add(new ECSReference(source.get()));
        if (!Check(Invoke(context, resource, L"AttachSound", args, 0), "audio attachment records actual reference")) return false;
    }
    resource.IndexAllMember();
    auto *hidden = ESLTypeCast<ECSReference>(resource.GetVariableAt(-1));
    if (!Check(hidden && hidden->m_pRef == source.get() && hidden->m_pParent == &resource && hidden->m_nIndex == -1,
        "hidden attachment reference has stable object graph index")) return false;
    source.reset();
    if (!Check(!hidden->m_pRef && resource.GetSound(), "destroyed attachment source clears script backlink and keeps cloned audio")) return false;
    study::platform::LogWrite(study::platform::LogPriority::Info, "StudySteady",
        "Legacy volume envelope probe PASS: cubic stereo, piecewise timing, Android volume, native ABI, endpoint, cancellation, replacement, release and hidden references");
    return true;
}
