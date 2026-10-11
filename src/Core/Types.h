#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <cstdint>

namespace OS {

enum class BootState {
    BIOS_POST,
    BOOT_SPLASH,
    DESKTOP,
    SHUTTING_DOWN
};

enum class ThemeMode {
    CYBER_DARK,
    CLASSIC_98,
    RETRO_AMBER,
    MATRIX_GREEN
};

struct ProcessInfo {
    int pid;
    std::string name;
    std::string user;
    float cpuUsage;       // In percent (0.0 to 100.0)
    float baseCpu;        // Base CPU target for fluctuation
    float memoryMB;       // Memory in MB
    float baseMemoryMB;
    float diskIOKB;       // Disk I/O KB/s
    std::string status;   // "Running", "Suspended", "Idle"
    bool isSystem;        // Protected kernel processes
};

struct VirtualFile {
    std::string name;
    bool isDirectory = false;
    size_t sizeBytes = 0;
    std::string modifiedDate;
    std::string content;
    std::vector<VirtualFile> children;

    VirtualFile() = default;
    VirtualFile(std::string n, bool isDir, size_t sz, std::string mod, std::string cnt)
        : name(std::move(n)), isDirectory(isDir), sizeBytes(sz), modifiedDate(std::move(mod)), content(std::move(cnt)) {}
};

struct TerminalLine {
    std::string text;
    uint32_t color = 0xFFFFFFFF; // ABGR/RGBA ImU32 compatible
    bool isCommand = false;
};

} // namespace OS
