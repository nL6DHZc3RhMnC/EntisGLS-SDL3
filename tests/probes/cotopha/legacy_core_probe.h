#pragma once
class ECSEnvironment;
// Requires ECotophaScript::Initialize and the Android file environment to exist.
// Uses a synthetic object-only image; no game initialization or graphics run.
bool CheckLegacyCore(ECSEnvironment& environment);
