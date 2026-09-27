#pragma once
// Reuses the supplied GLS3 compiler's actual expression evaluator.
ESLError LegacyCalculateString(ECSString &,ECSContext &,ECSObjArray<ECSObject> &);
class ECSEnvironment;
bool CheckLegacyCompiler(ECSEnvironment &);
