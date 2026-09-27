#pragma once
class ECSContext;
class ECSObject;
using LegacyPlatformObjectFactory = ECSObject *(*)(ECSContext &, const wchar_t *);
void LegacySetPlatformObjectFactory(LegacyPlatformObjectFactory factory);
ECSObject *LegacyCreatePlatformObject(ECSContext &, const wchar_t *);
unsigned LegacyProcessorFeatures();
void LegacyLogScriptError(const char* message, unsigned instruction);
