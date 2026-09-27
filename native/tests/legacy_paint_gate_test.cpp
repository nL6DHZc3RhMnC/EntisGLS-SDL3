#include "../legacy_paint_gate.h"
#include <chrono>
#include <future>
#include <iostream>

namespace {
bool Check(bool value, const char* description) {
    std::cout << (value ? "PASS: " : "FAIL: ") << description << std::endl;
    return value;
}
struct Window {};
}

int main() {
    using namespace std::chrono_literals;
    LegacyPaintGate<Window> gate;
    Window window, other;
    std::mutex uiMutex;
    unsigned drawn = 0;
    gate.Freeze(&window);
    gate.Freeze(&window);
    std::unique_lock<std::mutex> scriptOwnsUI(uiMutex);
    auto frozenDraw = std::async(std::launch::async, [&] {
        gate.Draw(&window, [&](Window*) {
            std::lock_guard<std::mutex> ui(uiMutex);
            ++drawn;
        });
    });
    const bool returnedWhileUILocked = frozenDraw.wait_for(1s) == std::future_status::ready;
    // Always release the simulated UI lock before joining, including failures.
    scriptOwnsUI.unlock();
    frozenDraw.get();
    if (!Check(returnedWhileUILocked && drawn == 0,
        "frozen production gate returns while another thread owns the UI mutex")) return 1;
    if (!Check(gate.Unfreeze(&window) && gate.IsFrozen(&window),
        "first unfreeze retains nested suppression")) return 1;
    gate.Draw(&window, [&](Window*) {++drawn;});
    if (!Check(drawn == 0 && gate.Unfreeze(&window) && !gate.IsFrozen(&window) && !gate.Unfreeze(&window),
        "last unfreeze resumes and unmatched unfreeze is rejected")) return 1;

    scriptOwnsUI.lock();
    std::promise<void> enteredDraw;
    auto entered = enteredDraw.get_future();
    auto liveDraw = std::async(std::launch::async, [&] {
        gate.Draw(&window, [&](Window*) {
            enteredDraw.set_value();
            std::lock_guard<std::mutex> ui(uiMutex);
            ++drawn;
        });
    });
    const bool enteredCallback = entered.wait_for(1s) == std::future_status::ready;
    const bool waitedForUI = liveDraw.wait_for(0s) == std::future_status::timeout;
    scriptOwnsUI.unlock();
    liveDraw.get();
    if (!Check(enteredCallback && waitedForUI && drawn == 1,
        "unfrozen gate invokes normal drawing and its UI synchronization")) return 1;

    gate.Draw(&window, [&](Window* target) {
        gate.Freeze(target);
        gate.Unfreeze(target);
        ++drawn;
    });
    if (!Check(drawn == 2, "drawing callbacks run without holding the registry mutex")) return 1;
    gate.Freeze(&window);
    gate.Freeze(&window);
    gate.Draw(&other, [&](Window*) {++drawn;});
    gate.Draw(nullptr, [&](Window*) {++drawn;});
    if (!Check(drawn == 3, "freezes are per-window and null draws are ignored")) return 1;
    gate.Reset(&window);
    if (!Check(!gate.IsFrozen(&window) && !gate.Unfreeze(&window),
        "close/reset removes all nesting state")) return 1;
    gate.Draw(&window, [&](Window*) {++drawn;});
    if (!Check(drawn == 4, "reset window can draw again")) return 1;
    return 0;
}
