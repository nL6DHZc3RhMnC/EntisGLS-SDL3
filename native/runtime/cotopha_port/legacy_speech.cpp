#include "compatibility/sdk/legacy/gls.h"
#include "runtime/cotopha_port/legacy_speech.h"
#include "platform/log.h"

IMPLEMENT_CLASS_INFO(ECSSpeachVoiceGenerator, ECSObject)
IMPLEMENT_CLASS_INFO(ECSSpeachVoicePlayer, ECSResource)

namespace {
constexpr const wchar_t* generatorMethods[] = {
    L"Initialize", L"Release", L"GetSpeakerVolume", L"SetSpeakerVolume",
    L"IsSpeakerMute", L"SetSpeakerMute", L"GetConfig", L"SetConfig", L"DoConfigDialog"
};
constexpr int playerMethodBase = 6144;

void ReportUnavailable(const char* operation) {
    study::platform::LogPrint(study::platform::LogPriority::Warn, "LegacyCotopha",
        "%s: speech synthesis backend is unavailable", operation);
}
}

ECSSpeachVoiceGenerator::ECSSpeachVoiceGenerator() { m_vtType = csvtObject; }
const wchar_t* ECSSpeachVoiceGenerator::GetTypeName() const { return L"SpeachVoiceGenerator"; }
ECSObject* ECSSpeachVoiceGenerator::GetTypeOf(const wchar_t* name) {
    return name && !EWideString::Compare(name, GetTypeName()) ? this : ECSObject::GetTypeOf(name);
}
ECSObject* ECSSpeachVoiceGenerator::Duplicate() { return new ECSSpeachVoiceGenerator; }
ESLError ECSSpeachVoiceGenerator::Move(ECSContext& context, ECSObject* object) {
    if (!ESLTypeCast<ECSSpeachVoiceGenerator>(ECSObject::GetEntity(object)))
        return ESLErrorMsg("SpeachVoiceGenerator assignment requires SpeachVoiceGenerator");
    context.delete_CSObject(object);
    return eslErrSuccess;
}
ESLError ECSSpeachVoiceGenerator::UnaryOperate(ECSContext&, CSUnaryOperatorType) {
    return ESLErrorMsg("SpeachVoiceGenerator has no unary operator");
}
ESLError ECSSpeachVoiceGenerator::Operate(ECSContext&, CSOperatorType, ECSObject*) {
    return ESLErrorMsg("SpeachVoiceGenerator has no arithmetic operator");
}
ESLError ECSSpeachVoiceGenerator::Compare(ECSContext&, int&, CSCompareType, ECSObject&) {
    return ESLErrorMsg("SpeachVoiceGenerator has no value comparison");
}
ESLError ECSSpeachVoiceGenerator::GetFunction(ECSContext&, int& index, const wchar_t* name) {
    for (index = 0; index < 9; ++index)
        if (name && !EWideString::Compare(name, generatorMethods[index])) return eslErrSuccess;
    index = -1;
    return ESLErrorMsg("Unknown SpeachVoiceGenerator method");
}
ESLError ECSSpeachVoiceGenerator::CallFunction(ECSContext& context, int index, ECSObjArray<ECSObject>& args) {
    if (index < 0 || index >= 9) return ESLErrorMsg("Invalid SpeachVoiceGenerator method index");
    constexpr int counts[] = {1, 1, 2, 3, 2, 3, 1, 2, 2}; // Include the receiver.
    if (const auto error = context.VerifyArgumentCount(args, counts[index], counts[index])) return error;
    const auto push = [&](INT64 value) { return context.PushObject(new ECSInteger(value)); };
    if (index == 0) {
        ReportUnavailable("SpeachVoiceGenerator.Initialize");
        return push(eslErrNotSupported);
    }
    if (index == 1) return push(eslErrSuccess); // No backend resources remain to release.
    if (index >= 2 && index <= 5) {
        int speaker = 0;
        if (const auto error = context.GetArgumentAsInt(speaker, args, 1, 0)) return error;
        if (index == 2) return context.PushObject(new ECSReal(0));
        // All speakers have no output on an unavailable backend. This lets
        // callers skip synthesis instead of repeatedly requesting silence.
        if (index == 4) return push(-1);
        if (index == 3) {
            double volume = 0;
            if (const auto error = context.GetArgumentAsReal(volume, args, 2, 0)) return error;
        } else {
            int mute = 0;
            if (const auto error = context.GetArgumentAsInt(mute, args, 2, 0)) return error;
        }
        return push(eslErrNotSupported);
    }
    // There is no installed synthesizer or configuration to serialize. Do not
    // fabricate speaker names, a Windows settings XML, or a successful update.
    if (index == 6) return context.PushObject(new ECSString(L""));
    if (index == 7) {
        EWideString config;
        if (const auto error = context.GetArgumentAsStr(config, args, 1, L"")) return error;
    }
    if (index == 8) ReportUnavailable("SpeachVoiceGenerator.DoConfigDialog");
    return push(eslErrNotSupported);
}

const wchar_t* ECSSpeachVoicePlayer::GetTypeName() const { return L"SpeachVoicePlayer"; }
ECSObject* ECSSpeachVoicePlayer::GetTypeOf(const wchar_t* name) {
    return name && !EWideString::Compare(name, GetTypeName()) ? this : ECSResource::GetTypeOf(name);
}
ECSObject* ECSSpeachVoicePlayer::Duplicate() {
    auto* copy = new ECSSpeachVoicePlayer;
    copy->m_image = m_image;
    copy->m_imageStream = m_imageStream;
    {
        std::lock_guard<std::mutex> lock(m_volumeMutex);
        if (m_sound) copy->m_sound = std::make_shared<SakuraGL::SGLAudioPlayer>(m_sound->ClonePlayer(), true);
    }
    copy->m_wstrFileName = m_wstrFileName;
    copy->m_volume[0] = m_volume[0].load();
    copy->m_volume[1] = m_volume[1].load();
    copy->m_nPlayType = m_nPlayType.load();
    copy->m_nThreshold = m_nThreshold;
    copy->m_nStartPos = m_nStartPos;
    copy->m_nEndPos = m_nEndPos;
    copy->m_nRewindPos = m_nRewindPos;
    copy->m_nRepeatPlaying = m_nRepeatPlaying;
    return copy;
}
ESLError ECSSpeachVoicePlayer::GetFunction(ECSContext& context, int& index, const wchar_t* name) {
    if (name && !EWideString::Compare(name, L"AttachVoiceGenerator")) { index = playerMethodBase; return eslErrSuccess; }
    if (name && !EWideString::Compare(name, L"MakeVoice")) { index = playerMethodBase + 1; return eslErrSuccess; }
    return ECSResource::GetFunction(context, index, name);
}
ESLError ECSSpeachVoicePlayer::CallFunction(ECSContext& context, int index, ECSObjArray<ECSObject>& args) {
    if (index < playerMethodBase) return ECSResource::CallFunction(context, index, args);
    if (index > playerMethodBase + 1) return ESLErrorMsg("Invalid SpeachVoicePlayer method index");
    const int count = index == playerMethodBase ? 2 : 3;
    if (const auto error = context.VerifyArgumentCount(args, count, count)) return error;
    if (index == playerMethodBase) {
        if (!context.GetArgumentObjectAs(args, 1, L"SpeachVoiceGenerator"))
            return context.PushObject(new ECSInteger(eslErrInvalidParam));
    } else {
        EWideString text;
        int speaker = 0;
        if (const auto error = context.GetArgumentAsStr(text, args, 1, L"")) return error;
        if (const auto error = context.GetArgumentAsInt(speaker, args, 2, 0)) return error;
        // Never leave previously loaded audio looking like newly generated
        // speech after this request fails.
        ECSResource::Release();
        ReportUnavailable("SpeachVoicePlayer.MakeVoice");
    }
    return context.PushObject(new ECSInteger(eslErrNotSupported));
}
