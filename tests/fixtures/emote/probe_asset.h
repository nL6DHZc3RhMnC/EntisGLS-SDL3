#pragma once
#include "extensions/emote/tjs_runtime/tjs_host.h"
#include "psbfile/PSBDispatch.h"
#include "MotionDispatch.h"
#include "extensions/emote/tjs_runtime/psb_storage.h"
#include <functional>
#include <stdexcept>
#include <spdlog/sinks/stdout_color_sinks.h>

inline tTJSVariant probeDispatch(const PSB::PSBRawNode &raw) {
    auto *d=new PSB::PSBValueDispatch(raw.GetFile_guess(),raw.GetNode());
    tTJSVariant out(d,d);d->Release();return out;
}
inline void probeMotions(const PSB::PSBFile &file,
    const std::function<void(const std::string &,const tTJSVariant &)> &visit) {
    const auto objects=file.GetRoot().GetDictionaryValueStrict("object");
    for(const auto &object:objects.GetDictionaryKeys()) {
        const auto motions=objects.GetDictionaryValueStrict(object.c_str()).GetDictionaryValueStrict("motion");
        for(const auto &motion:motions.GetDictionaryKeys())
            visit(object+"/"+motion,probeDispatch(motions.GetDictionaryValueStrict(motion.c_str())));
    }
}
inline void probeLayers(const tTJSVariant &layers,const std::string &path,
    const std::function<void(const std::string &,const tTJSVariant &)> &visit) {
    using namespace motion::detail;
    if(layers.Type()==tvtVoid)return;
    for(int i=0;i<motionPropGetCount(layers);++i) {
        const auto layer=motionPropGetByNum(layers,i);
        const auto here=path+"/"+motionPropGetString(layer,TJS_W("label")).AsStdString();
        visit(here,layer);
        probeLayers(motionPropGet(layer,TJS_W("children")),here,visit);
    }
}
