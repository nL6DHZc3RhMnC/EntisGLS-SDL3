#pragma once

#include <cstdint>

namespace study::platform::sdl {

struct GameFileOperationMetrics {
    std::uint64_t calls = 0;
    std::uint64_t bytes = 0; // Actual returned bytes for Read; zero for other operations.
    std::uint64_t totalNs = 0;
    std::uint64_t maxNs = 0;
};

struct GameFileMetricsSnapshot {
    bool enabled = false;
    GameFileOperationMetrics duplicate, read, seek, getLength;
};

// Explicit diagnostic startup opt-in; disabled by default. Toggling affects new
// operations and does not reset totals or change any file access behavior.
void SetGameFileMetricsEnabled(bool enabled);
bool GameFileMetricsEnabled();

// Process-wide totals of completed game-stream operations. Shared read cursors
// are timed once; nested operations on their underlying stream are not counted
// again. Calls, bytes and totalNs can be subtracted at stage boundaries.
// maxNs is the cumulative peak,
// not an interval delta. Concurrent operations may complete during the snapshot;
// counters deliberately do not serialize the application's file operations.
GameFileMetricsSnapshot SnapshotGameFileMetrics();

} // namespace study::platform::sdl
