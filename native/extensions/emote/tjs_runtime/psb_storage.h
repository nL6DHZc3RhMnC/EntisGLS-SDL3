#pragma once
#include "tjs.h"
#include "extensions/emote/tjs_runtime/motion_apk_runtime.h"
void motionSetPsbHeaderSeed(uint32_t seed);
void motionSetPsbKeyResolver(StudyMotionPsbKeyResolver resolver,void *user);
void motionSetStorageReader(StudyMotionRead read,StudyMotionRelease release,void *user);
ttstr TVPGetPlacedPath(const ttstr &name);
bool TVPIsExistentStorage(const ttstr &name);
