#include "memory_info.h"

#include <SDL3/SDL_cpuinfo.h>
#include <algorithm>

#if defined(__APPLE__)
#include <mach/mach.h>
#include <sys/sysctl.h>
#elif defined(__linux__)
#include <sys/sysinfo.h>
#endif

namespace study::platform::sdl {

bool QueryMemoryInfo(MemoryInfo& result) {
    result = {};
    const int totalMegabytes = SDL_GetSystemRAM();
    if (totalMegabytes > 0) result.totalPhysical = std::uint64_t(totalMegabytes) * 1024 * 1024;

#if defined(__APPLE__)
    std::uint64_t physicalBytes = 0;
    std::size_t physicalSize = sizeof(physicalBytes);
    if (sysctlbyname("hw.memsize", &physicalBytes, &physicalSize, nullptr, 0) == 0) {
        result.totalPhysical = physicalBytes;
    }
    const mach_port_t host = mach_host_self();
    vm_size_t pageSize = 0;
    vm_statistics64_data_t statistics{};
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    if (host_page_size(host, &pageSize) == KERN_SUCCESS &&
        host_statistics64(host, HOST_VM_INFO64, reinterpret_cast<host_info64_t>(&statistics), &count) == KERN_SUCCESS) {
        result.availablePhysical = std::min(result.totalPhysical, std::uint64_t(statistics.free_count) * pageSize);
        result.availablePhysicalKnown = true;
    }
    mach_port_deallocate(mach_task_self(), host);
    xsw_usage swap{};
    std::size_t swapSize = sizeof(swap);
    if (sysctlbyname("vm.swapusage", &swap, &swapSize, nullptr, 0) == 0) {
        result.totalSwap = swap.xsu_total;
        result.availableSwap = swap.xsu_avail;
        result.swapKnown = true;
    }
#elif defined(__linux__)
    struct sysinfo info{};
    if (sysinfo(&info) == 0) {
        result.totalPhysical = std::uint64_t(info.totalram) * info.mem_unit;
        result.availablePhysical = std::uint64_t(info.freeram) * info.mem_unit;
        result.totalSwap = std::uint64_t(info.totalswap) * info.mem_unit;
        result.availableSwap = std::uint64_t(info.freeswap) * info.mem_unit;
        result.availablePhysicalKnown = true;
        result.swapKnown = true;
    }
#endif
    return result.totalPhysical != 0;
}

} // namespace study::platform::sdl
