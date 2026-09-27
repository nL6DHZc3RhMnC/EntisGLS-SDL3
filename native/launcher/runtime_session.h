#pragma once
#include <string>
namespace entis::launcher { struct GameLaunchConfig; }
int RunLegacyGame(const wchar_t *arguments, const entis::launcher::GameLaunchConfig* config = nullptr);
std::string LegacyGameLastError();
void AbortLegacyGame();
