#pragma once
#include "tjs.h"
#include <vector>
#include <cstdint>
namespace motion {class EmoteEngine;}
namespace studysteady::motion {
struct SavedActorState {tTJSVariant engine;float baseScale=1,userScale=1;};
std::vector<uint8_t> encodeActorState(::motion::EmoteEngine &,const ttstr &project,float baseScale,float userScale);
SavedActorState decodeActorState(const void *,size_t,::motion::EmoteEngine &schema,const ttstr &project);
void validateActorStateEnvelope(const void *,size_t,const ttstr &project);
}
