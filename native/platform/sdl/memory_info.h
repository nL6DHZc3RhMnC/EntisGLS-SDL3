#pragma once

#include <cstdint>

namespace study::platform::sdl {

struct MemoryInfo {
    std::uint64_t totalPhysical = 0;
    std::uint64_t availablePhysical = 0;
    std::uint64_t totalSwap = 0;
    std::uint64_t availableSwap = 0;
    bool availablePhysicalKnown = false;
    bool swapKnown = false;
};

// True means totalPhysical is known. The separate flags distinguish a measured
// zero from data that SDL or the host does not expose. "Available" follows the
// existing Linux sysinfo freeram field: free pages, excluding reclaimable cache.
bool QueryMemoryInfo(MemoryInfo& result);

} // namespace study::platform::sdl
