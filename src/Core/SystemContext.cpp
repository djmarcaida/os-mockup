#include "SystemContext.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <random>
#include <cctype>
#include <functional>
#include <limits>

namespace OS {

namespace {

char toLowerChar(unsigned char value) {
    return static_cast<char>(std::tolower(value));
}

bool equalsIgnoreCase(const std::string& left, const std::string& right) {
    if (left.size() != right.size()) return false;
    return std::equal(left.begin(), left.end(), right.begin(), [](unsigned char a, unsigned char b) {
        return std::tolower(a) == std::tolower(b);
    });
}

std::string currentDateIso() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t timeNow = std::chrono::system_clock::to_time_t(now);
    std::tm tmNow{};
#if defined(_WIN32)
    localtime_s(&tmNow, &timeNow);
#else
    localtime_r(&timeNow, &tmNow);
#endif
    std::ostringstream out;
    out << std::setfill('0') << std::setw(4) << (1900 + tmNow.tm_year) << '-'
        << std::setw(2) << (tmNow.tm_mon + 1) << '-'
        << std::setw(2) << tmNow.tm_mday;
    return out.str();
}

template <typename FileType>
FileType* resolveVirtualPath(FileType* root, FileType* currentDirectory,
                             const std::string& currentDirectoryPath, const std::string& path) {
    if (!root || !currentDirectory || path.empty()) return nullptr;

    std::string normalized = path;
    std::replace(normalized.begin(), normalized.end(), '/', '\\');

    std::string combinedPath;
    const bool hasDrive = normalized.size() >= 2 && normalized[1] == ':';
    if (hasDrive) {
        if (std::tolower(static_cast<unsigned char>(normalized[0])) != 'c') return nullptr;
        const std::string driveRelativePath = normalized.substr(2);
        if (driveRelativePath.empty()) return root;
        if (driveRelativePath[0] == '\\') {
            combinedPath = driveRelativePath;
        } else {
            combinedPath = currentDirectoryPath.substr(2) + "\\" + driveRelativePath;
        }
    } else if (normalized[0] == '\\') {
        combinedPath = normalized;
    } else {
        combinedPath = currentDirectoryPath.substr(2) + "\\" + normalized;
    }

    std::vector<std::string> components;
    std::stringstream pathStream(combinedPath);
    std::string component;
    while (std::getline(pathStream, component, '\\')) {
        if (!component.empty()) components.push_back(component);
    }

    std::vector<FileType*> directoryStack{root};
    for (size_t index = 0; index < components.size(); ++index) {
        const std::string& part = components[index];
        if (part == ".") continue;
        if (part == "..") {
            if (directoryStack.size() > 1) directoryStack.pop_back();
            continue;
        }

        FileType* parent = directoryStack.back();
        auto found = std::find_if(parent->children.begin(), parent->children.end(), [&part](const auto& child) {
            return equalsIgnoreCase(child.name, part);
        });
        if (found == parent->children.end()) return nullptr;

        const bool isLast = index + 1 == components.size();
        if (!found->isDirectory) {
            return isLast ? &*found : nullptr;
        }
        directoryStack.push_back(&*found);
    }
    return directoryStack.back();
}

} // namespace

SystemContext::SystemContext() {
    m_cpuHistory.resize(60, 5.0f);
    m_memoryHistory.resize(60, 1420.0f);

    initProcesses();
    refreshProcessMetrics();
    initVFS();

    // Bind the initial editor to its actual virtual source file.
    openFileInNotepad(findFileByPath("C:\\CSOPESY\\welcome.txt"));
    showNotepad = false;

    // Initialize terminal greeting
    m_terminalLines.push_back({"CSOPESY Command Prompt [Version 2.4.19-release]", 0xFF44FF44, false});
    m_terminalLines.push_back({"(C) 2026 CSOPESY Operating Systems Engineering. All rights reserved.", 0xFFAAAAAA, false});
    m_terminalLines.push_back({"Type 'help' to view available commands.", 0xFFFFFF88, false});
    m_terminalLines.push_back({"", 0xFFFFFFFF, false});
}

void SystemContext::setState(BootState state) {
    m_state = state;
    m_stateTimer = 0.0f;
}

void SystemContext::reboot() {
    setState(BootState::BIOS_POST);
    showTaskManager = false;
    showFileExplorer = false;
    showTerminal = false;
    showSystemInfo = false;
    showNotepad = false;
    showStartMenu = false;
    showPowerDialog = false;
    showVolumePopup = false;
    showNetworkPopup = false;
}

void SystemContext::requestShutdown() {
    m_shouldCloseApp = true;
}

void SystemContext::update(float deltaTime) {
    m_stateTimer += deltaTime;
    m_totalUptime += deltaTime;

    if (m_state == BootState::BIOS_POST) {
        if (m_stateTimer >= m_postDuration) {
            setState(BootState::BOOT_SPLASH);
        }
    } else if (m_state == BootState::BOOT_SPLASH) {
        if (m_stateTimer >= m_splashDuration) {
            setState(BootState::DESKTOP);
            // Open task manager by default for immediate wow factor
            showTaskManager = true;
        }
    } else if (m_state == BootState::DESKTOP) {
        updateProcesses(deltaTime);
        updatePerformanceHistory(deltaTime);
    }
}

uint32_t SystemContext::getBiosMemoryKB() const {
    const uint32_t targetKB = 262144; // 256 MB in KB
    if (m_state != BootState::BIOS_POST) return targetKB;

    float ratio = m_stateTimer / (m_postDuration * 0.75f);
    if (ratio >= 1.0f) return targetKB;
    return static_cast<uint32_t>(ratio * targetKB);
}

std::string SystemContext::getCurrentTimeString() const {
    auto now = std::chrono::system_clock::now();
    std::time_t timeNow = std::chrono::system_clock::to_time_t(now);
    std::tm tmNow{};
#if defined(_WIN32)
    localtime_s(&tmNow, &timeNow);
#else
    localtime_r(&timeNow, &tmNow);
#endif
    std::ostringstream ss;
    ss << std::setfill('0') << std::setw(2) << tmNow.tm_hour << ":"
       << std::setfill('0') << std::setw(2) << tmNow.tm_min << ":"
       << std::setfill('0') << std::setw(2) << tmNow.tm_sec;
    return ss.str();
}

std::string SystemContext::getCurrentDateString() const {
    auto now = std::chrono::system_clock::now();
    std::time_t timeNow = std::chrono::system_clock::to_time_t(now);
    std::tm tmNow{};
#if defined(_WIN32)
    localtime_s(&tmNow, &timeNow);
#else
    localtime_r(&timeNow, &tmNow);
#endif

    const char* days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    const char* months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

    std::ostringstream ss;
    ss << days[tmNow.tm_wday] << ", " << months[tmNow.tm_mon] << " " << tmNow.tm_mday << ", " << (1900 + tmNow.tm_year);
    return ss.str();
}

std::string SystemContext::getUptimeString() const {
    int totalSec = static_cast<int>(m_totalUptime);
    int hours = totalSec / 3600;
    int mins = (totalSec % 3600) / 60;
    int secs = totalSec % 60;

    std::ostringstream ss;
    ss << std::setfill('0') << std::setw(2) << hours << ":"
       << std::setfill('0') << std::setw(2) << mins << ":"
       << std::setfill('0') << std::setw(2) << secs;
    return ss.str();
}

void SystemContext::initProcesses() {
    m_processes.clear();
    m_processes.push_back({0,   "idle_process",       "SYSTEM", 78.4f, 78.0f, 0.1f,   0.1f,   0.0f,  "Running", true});
    m_processes.push_back({4,   "csopesy_kernel.exe", "SYSTEM",  6.2f,  6.0f, 84.5f,  84.0f,  24.1f, "Running", true});
    m_processes.push_back({108, "compositor.srv",     "SYSTEM",  4.8f,  5.0f, 132.8f, 130.0f, 12.4f, "Running", true});
    m_processes.push_back({212, "shell_ui.exe",       "admin",   3.1f,  3.0f, 96.2f,  95.0f,  5.8f,  "Running", false});
    m_processes.push_back({260, "dwm_core.dll",       "admin",   2.4f,  2.5f, 64.0f,  64.0f,  1.2f,  "Running", true});
    m_processes.push_back({344, "vfs_service.exe",    "SYSTEM",  0.9f,  1.0f, 42.1f,  42.0f,  45.6f, "Running", true});
    m_processes.push_back({416, "pagefile_mgr.exe",   "SYSTEM",  0.3f,  0.5f, 28.4f,  28.0f,  8.0f,  "Running", true});
    m_processes.push_back({528, "audio_daemon.sys",   "SYSTEM",  0.5f,  0.5f, 18.2f,  18.0f,  0.0f,  "Running", false});
    m_processes.push_back({680, "net_stack.srv",      "SYSTEM",  0.7f,  0.8f, 34.6f,  34.0f,  14.2f, "Running", false});
    m_processes.push_back({812, "taskmgr.exe",        "admin",   1.8f,  1.8f, 48.0f,  48.0f,  2.0f,  "Running", false});
    m_processes.push_back({944, "csopesy_sh.exe",     "admin",   0.2f,  0.2f, 16.5f,  16.0f,  0.5f,  "Running", false});
}

void SystemContext::updateProcesses(float deltaTime) {
    if (!std::isfinite(deltaTime) || deltaTime <= 0.0f) return;
    m_processTimer += deltaTime;
    if (m_processTimer < 0.25f) return;
    m_processTimer = std::fmod(m_processTimer, 0.25f);

    static std::mt19937 rng(1337);
    std::uniform_real_distribution<float> jitter(-0.8f, 0.8f);
    std::uniform_real_distribution<float> memJitter(-0.4f, 0.4f);

    for (auto& proc : m_processes) {
        if (proc.pid == 0) continue;

        const float baseCpu = std::isfinite(proc.baseCpu) ? std::max(0.0f, std::min(100.0f, proc.baseCpu)) : 0.0f;
        const float baseMemory = std::isfinite(proc.baseMemoryMB) ? std::max(0.0f, std::min(m_maxMemoryMB, proc.baseMemoryMB)) : 0.0f;
        const float diskIO = std::isfinite(proc.diskIOKB) ? proc.diskIOKB : 0.0f;
        proc.cpuUsage = std::max(0.1f, std::min(45.0f, baseCpu + jitter(rng)));
        proc.memoryMB = std::max(5.0f, std::min(m_maxMemoryMB, baseMemory + memJitter(rng)));
        proc.diskIOKB = std::max(0.0f, diskIO + jitter(rng) * 2.0f);
    }

    refreshProcessMetrics(true);
}

void SystemContext::updatePerformanceHistory(float deltaTime) {
    if (!std::isfinite(deltaTime) || deltaTime <= 0.0f) return;
    m_historyTimer += deltaTime;
    if (!std::isfinite(m_historyTimer) || m_historyTimer < 1.0f) return;

    const size_t capacity = std::max(m_cpuHistory.size(), m_memoryHistory.size());
    if (capacity > 0 && m_historyTimer >= static_cast<float>(capacity)) {
        std::fill(m_cpuHistory.begin(), m_cpuHistory.end(), m_totalCpuUsage);
        std::fill(m_memoryHistory.begin(), m_memoryHistory.end(), m_totalMemoryMB);
        m_historyTimer = std::fmod(m_historyTimer, 1.0f);
        return;
    }

    const size_t samples = static_cast<size_t>(m_historyTimer);
    m_historyTimer -= static_cast<float>(samples);
    const auto appendSamples = [samples](std::vector<float>& history, float value) {
        if (history.empty()) return;
        if (samples >= history.size()) {
            std::fill(history.begin(), history.end(), value);
            return;
        }
        history.erase(history.begin(), history.begin() + static_cast<std::ptrdiff_t>(samples));
        history.insert(history.end(), samples, value);
    };
    appendSamples(m_cpuHistory, m_totalCpuUsage);
    appendSamples(m_memoryHistory, m_totalMemoryMB);
}

void SystemContext::refreshProcessMetrics(bool updateIdleProcess) {
    double activeCpuSum = 0.0;
    double totalMemory = 0.0;
    ProcessInfo* idleProcess = nullptr;

    for (auto& proc : m_processes) {
        proc.cpuUsage = std::isfinite(proc.cpuUsage) ? std::max(0.0f, std::min(100.0f, proc.cpuUsage)) : 0.0f;
        proc.memoryMB = std::isfinite(proc.memoryMB) ? std::max(0.0f, std::min(m_maxMemoryMB, proc.memoryMB)) : 0.0f;
        proc.diskIOKB = std::isfinite(proc.diskIOKB) ? std::max(0.0f, proc.diskIOKB) : 0.0f;
        if (proc.pid == 0) {
            idleProcess = &proc;
            totalMemory += proc.memoryMB;
            continue;
        }
        activeCpuSum += proc.cpuUsage;
        totalMemory += proc.memoryMB;
    }

    if (activeCpuSum > 100.0) {
        const float scale = static_cast<float>(100.0 / activeCpuSum);
        for (auto& proc : m_processes) {
            if (proc.pid != 0) proc.cpuUsage *= scale;
        }
        activeCpuSum = 100.0;
    }

    // Saturate the simulated workload while keeping table rows and totals consistent.
    if (totalMemory > m_maxMemoryMB) {
        const float scale = static_cast<float>(m_maxMemoryMB / totalMemory);
        for (auto& proc : m_processes) proc.memoryMB *= scale;
        totalMemory = m_maxMemoryMB;
    }
    m_totalCpuUsage = static_cast<float>(std::max(0.0, std::min(100.0, activeCpuSum)));
    m_totalMemoryMB = static_cast<float>(totalMemory);
    if (updateIdleProcess && idleProcess) {
        idleProcess->cpuUsage = std::max(0.0f, 100.0f - m_totalCpuUsage);
    }
}

bool SystemContext::killProcess(int pid, std::string& outMessage) {
    auto it = std::find_if(m_processes.begin(), m_processes.end(), [pid](const ProcessInfo& p) {
        return p.pid == pid;
    });

    if (it == m_processes.end()) {
        outMessage = "Process ID not found.";
        return false;
    }

    if (it->isSystem) {
        outMessage = "Access Denied: '" + it->name + "' is a protected critical system service!";
        return false;
    }

    outMessage = "Successfully terminated process '" + it->name + "' (PID: " + std::to_string(pid) + ").";
    m_processes.erase(it);
    refreshProcessMetrics(true);
    return true;
}

void SystemContext::spawnProcess(const std::string& name, float cpu, float mem) {
    const float safeCpu = std::isfinite(cpu) ? std::max(0.0f, std::min(100.0f, cpu)) : 0.0f;
    const float safeMemory = std::isfinite(mem) ? std::max(0.0f, std::min(m_maxMemoryMB, mem)) : 0.0f;
    ProcessInfo p;
    p.pid = m_nextPid++;
    p.name = name;
    p.user = "admin";
    p.baseCpu = safeCpu;
    p.cpuUsage = safeCpu;
    p.baseMemoryMB = safeMemory;
    p.memoryMB = safeMemory;
    p.diskIOKB = 5.0f;
    p.status = "Running";
    p.isSystem = false;
    m_processes.push_back(p);
    refreshProcessMetrics(true);
}

void SystemContext::initVFS() {
    m_vfsRoot = std::make_unique<VirtualFile>();
    m_vfsRoot->name = "C:";
    m_vfsRoot->isDirectory = true;
    m_vfsRoot->modifiedDate = "2026-10-11";

    // CSOPESY system folder
    VirtualFile csopesyDir;
    csopesyDir.name = "CSOPESY";
    csopesyDir.isDirectory = true;
    csopesyDir.modifiedDate = "2026-10-11";

    VirtualFile welcomeTxt;
    welcomeTxt.name = "welcome.txt";
    welcomeTxt.isDirectory = false;
    welcomeTxt.sizeBytes = 840;
    welcomeTxt.modifiedDate = "2026-10-11";
    welcomeTxt.content = 
        "CSOPESY Workstation OS v2.4 (Enterprise Edition)\n"
        "Engineered for high performance operating system simulation.\n"
        "Features:\n"
        " - Microkernel architecture simulation\n"
        " - OpenGL 3.3 accelerated Desktop Window Compositor\n"
        " - Dear ImGui reactive widget pipeline\n"
        " - Simulated process metrics and performance graphs\n";
    welcomeTxt.sizeBytes = welcomeTxt.content.size();
    csopesyDir.children.push_back(welcomeTxt);

    VirtualFile kernelCfg;
    kernelCfg.name = "kernel.cfg";
    kernelCfg.isDirectory = false;
    kernelCfg.sizeBytes = 320;
    kernelCfg.modifiedDate = "2026-10-10";
    kernelCfg.content = 
        "# CSOPESY Kernel Boot Configuration\n"
        "boot_target=desktop\n"
        "scheduler=mlfq\n"
        "quantum_ms=20\n"
        "paging_enabled=true\n"
        "vfs_mount=ext4_vfs\n"
        "graphics_driver=opengl3_compositor\n"
        "debug_level=2\n";
    csopesyDir.children.push_back(kernelCfg);

    // System32 folder
    VirtualFile sys32Dir;
    sys32Dir.name = "System32";
    sys32Dir.isDirectory = true;
    sys32Dir.modifiedDate = "2026-10-11";

    VirtualFile halDll;
    halDll.name = "hal.dll";
    sys32Dir.children.push_back({halDll.name, false, 245760, "2026-09-15", "[Binary HAL Dynamic Library]"});
    VirtualFile kernelExe;
    kernelExe.name = "csopesy_kernel.exe";
    sys32Dir.children.push_back({kernelExe.name, false, 819200, "2026-10-01", "[CSOPESY Kernel Executable]"});
    VirtualFile compositorSrv;
    compositorSrv.name = "compositor.srv";
    sys32Dir.children.push_back({compositorSrv.name, false, 412000, "2026-10-08", "[Compositor Service]"});

    // Users folder
    VirtualFile usersDir;
    usersDir.name = "Users";
    usersDir.isDirectory = true;
    usersDir.modifiedDate = "2026-10-11";

    VirtualFile adminDir;
    adminDir.name = "Administrator";
    adminDir.isDirectory = true;
    adminDir.modifiedDate = "2026-10-11";

    VirtualFile desktopDir;
    desktopDir.name = "Desktop";
    desktopDir.isDirectory = true;
    desktopDir.modifiedDate = "2026-10-11";

    VirtualFile readmeTxt;
    readmeTxt.name = "readme.txt";
    readmeTxt.isDirectory = false;
    readmeTxt.sizeBytes = 512;
    readmeTxt.modifiedDate = "2026-10-11";
    readmeTxt.content = 
        "Welcome to the Desktop!\n\n"
        "Quick Tips:\n"
        "1. Click the Taskbar buttons or Desktop Icons to open apps.\n"
        "2. Launch Terminal to run commands ('help', 'ps', 'mem', 'reboot').\n"
        "3. Click PWR or Start -> Shutdown to exit cleanly.\n"
        "4. Switch themes in Settings/System Info window!\n";
    desktopDir.children.push_back(readmeTxt);

    VirtualFile docsDir;
    docsDir.name = "Documents";
    docsDir.isDirectory = true;
    docsDir.modifiedDate = "2026-10-11";
    docsDir.children.push_back({"notes.txt", false, 180, "2026-10-11", "Operating Systems Project Notes:\nReview Round Robin and MLFQ CPU scheduling algorithms."});

    adminDir.children.push_back(desktopDir);
    adminDir.children.push_back(docsDir);
    usersDir.children.push_back(adminDir);

    // Logs folder
    VirtualFile logsDir;
    logsDir.name = "Logs";
    logsDir.isDirectory = true;
    logsDir.modifiedDate = "2026-10-11";

    VirtualFile bootLog;
    bootLog.name = "boot.log";
    bootLog.isDirectory = false;
    bootLog.sizeBytes = 1240;
    bootLog.modifiedDate = "2026-10-11";
    bootLog.content = 
        "[0.000] POST memory check complete: 262144 KB OK\n"
        "[0.150] IDE Primary Master: CSOPESY-VHD 40GB detected\n"
        "[0.300] Loading CSOPESY Stage 2 Bootloader\n"
        "[0.650] Kernel uncompressed, starting execution\n"
        "[1.200] Hardware Abstraction Layer initialized\n"
        "[1.800] OpenGL 3.3 Compositor mounted\n"
        "[2.400] Desktop shell ready.\n";
    logsDir.children.push_back(bootLog);

    m_vfsRoot->children.push_back(csopesyDir);
    m_vfsRoot->children.push_back(sys32Dir);
    m_vfsRoot->children.push_back(usersDir);
    m_vfsRoot->children.push_back(logsDir);

    m_currentDir = m_vfsRoot.get();
    m_currentDirPath = "C:\\";
}

bool SystemContext::navigateTo(const std::string& folderName) {
    const VirtualFile* directory = findFileByPath(folderName);
    return directory && directory->isDirectory && navigateToDirectory(directory);
}

bool SystemContext::navigateToDirectory(const VirtualFile* directory) {
    std::string canonicalPath;
    if (!directory || !getCanonicalPath(directory, canonicalPath)) return false;
    VirtualFile* resolvedDirectory = resolveVirtualPath(m_vfsRoot.get(), m_currentDir, m_currentDirPath, canonicalPath);
    if (!resolvedDirectory || !resolvedDirectory->isDirectory) return false;

    m_currentDir = resolvedDirectory;
    m_currentDirPath = std::move(canonicalPath);
    return true;
}

bool SystemContext::navigateUp() {
    if (!m_currentDir || m_currentDir == m_vfsRoot.get()) return false;
    VirtualFile* parent = resolveVirtualPath(m_vfsRoot.get(), m_currentDir, m_currentDirPath, "..");
    return parent && parent != m_currentDir && navigateToDirectory(parent);
}

const VirtualFile* SystemContext::findFileByPath(const std::string& path) const {
    return resolveVirtualPath(m_vfsRoot.get(), m_currentDir, m_currentDirPath, path);
}

bool SystemContext::getCanonicalPath(const VirtualFile* file, std::string& outPath) const {
    if (!m_vfsRoot || !file) return false;

    std::function<bool(const VirtualFile*, const std::string&)> findPath =
        [&](const VirtualFile* current, const std::string& currentPath) {
            if (current == file) {
                outPath = currentPath;
                return true;
            }
            for (const auto& child : current->children) {
                const std::string childPath = current == m_vfsRoot.get()
                    ? currentPath + child.name
                    : currentPath + "\\" + child.name;
                if (findPath(&child, childPath)) return true;
            }
            return false;
        };

    return findPath(m_vfsRoot.get(), "C:\\");
}

void SystemContext::openFileInNotepad(const std::string& title, const std::string& content) {
    notepadTitle = title + " - Notepad";
    notepadBuffer = content;
    notepadModified = false;
    m_notepadFilePath.clear();
    m_pendingNotepadFilePath.clear();
    showNotepad = true;
}

void SystemContext::openFileInNotepad(const VirtualFile* file) {
    std::string canonicalPath;
    if (!file || !getCanonicalPath(file, canonicalPath) || file->isDirectory) return;

    showNotepad = true;
    if (notepadModified) {
        // Reopening the current source must not erase its unsaved buffer.
        if (canonicalPath != m_notepadFilePath) m_pendingNotepadFilePath = std::move(canonicalPath);
        return;
    }
    notepadTitle = file->name + " - Notepad";
    notepadBuffer = file->content;
    notepadModified = false;
    m_notepadFilePath = std::move(canonicalPath);
    m_pendingNotepadFilePath.clear();
}

bool SystemContext::confirmNotepadOpen() {
    const VirtualFile* target = findFileByPath(m_pendingNotepadFilePath);
    if (!target || target->isDirectory) return false;
    // Resolve the path again after Save, which may reallocate directory children.
    notepadTitle = target->name + " - Notepad";
    notepadBuffer = target->content;
    notepadModified = false;
    m_notepadFilePath = std::move(m_pendingNotepadFilePath);
    m_pendingNotepadFilePath.clear();
    showNotepad = true;
    return true;
}

void SystemContext::cancelNotepadOpen() {
    m_pendingNotepadFilePath.clear();
}

void SystemContext::newNotepadDocument() {
    notepadTitle = "Untitled - Notepad";
    notepadBuffer.clear();
    notepadModified = false;
    m_notepadFilePath.clear();
    m_pendingNotepadFilePath.clear();
    showNotepad = true;
}

bool SystemContext::saveNotepadDocument(std::string& outMessage) {
    if (!m_vfsRoot || !m_currentDir || !m_currentDir->isDirectory) {
        outMessage = "Save failed: current directory is unavailable.";
        return false;
    }

    const std::string modifiedDate = currentDateIso();
    if (!m_notepadFilePath.empty()) {
        VirtualFile* source = resolveVirtualPath(m_vfsRoot.get(), m_currentDir, m_currentDirPath, m_notepadFilePath);
        if (!source || source->isDirectory) {
            outMessage = "Save failed: source file no longer exists.";
            return false;
        }
        if (!notepadModified) {
            outMessage = "No changes to save.";
            return true;
        }

        source->content = notepadBuffer;
        source->sizeBytes = notepadBuffer.size();
        source->modifiedDate = modifiedDate;
        notepadModified = false;
        outMessage = "Saved changes to " + m_notepadFilePath + ".";
        return true;
    }

    size_t untitledNumber = 1;
    std::string fileName;
    while (true) {
        fileName = "Untitled-" + std::to_string(untitledNumber) + ".txt";
        const auto existing = std::find_if(m_currentDir->children.begin(), m_currentDir->children.end(), [&fileName](const VirtualFile& item) {
            return equalsIgnoreCase(item.name, fileName);
        });
        if (existing == m_currentDir->children.end()) break;
        ++untitledNumber;
    }

    m_currentDir->children.emplace_back(fileName, false, notepadBuffer.size(), modifiedDate, notepadBuffer);
    m_notepadFilePath = m_currentDirPath == "C:\\"
        ? m_currentDirPath + fileName
        : m_currentDirPath + "\\" + fileName;
    notepadTitle = fileName + " - Notepad";
    notepadModified = false;
    outMessage = "Saved new document as " + m_notepadFilePath + ".";
    return true;
}

void SystemContext::addTerminalLine(const std::string& text, uint32_t color, bool isCmd) {
    m_terminalLines.push_back({text, color, isCmd});
    if (m_terminalLines.size() > 500) {
        m_terminalLines.erase(m_terminalLines.begin(), m_terminalLines.begin() + 100);
    }
}

void SystemContext::clearTerminal() {
    m_terminalLines.clear();
}

void SystemContext::executeTerminalCommand(const std::string& commandLine) {
    if (commandLine.empty() || std::all_of(commandLine.begin(), commandLine.end(), [](unsigned char c) {
            return std::isspace(c) != 0;
        })) return;

    addTerminalLine("admin@csopesy:~$ " + commandLine, 0xFFFFFFFF, true);
    m_commandHistory.push_back(commandLine);

    std::stringstream ss(commandLine);
    std::string cmd;
    ss >> cmd;
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), toLowerChar);

    if (cmd == "help") {
        addTerminalLine("Available CSOPESY Shell Commands:", 0xFFFFFF55);
        addTerminalLine("  help           - Show this help summary", 0xFFCCCCCC);
        addTerminalLine("  ver            - Display OS and kernel build version", 0xFFCCCCCC);
        addTerminalLine("  ps             - Print active process table", 0xFFCCCCCC);
        addTerminalLine("  kill <pid>     - Terminate process by PID", 0xFFCCCCCC);
        addTerminalLine("  mem            - Display physical and virtual memory usage", 0xFFCCCCCC);
        addTerminalLine("  uptime         - Show total system runtime duration", 0xFFCCCCCC);
        addTerminalLine("  date / time    - Print current system date and clock", 0xFFCCCCCC);
        addTerminalLine("  dir / ls       - List contents of current directory", 0xFFCCCCCC);
        addTerminalLine("  cat <file>     - Display contents of a text file", 0xFFCCCCCC);
        addTerminalLine("  echo <text>    - Print text to console", 0xFFCCCCCC);
        addTerminalLine("  cls / clear    - Clear terminal screen buffer", 0xFFCCCCCC);
        addTerminalLine("  taskmgr        - Open Windows Task Manager window", 0xFFCCCCCC);
        addTerminalLine("  explorer       - Open File Explorer window", 0xFFCCCCCC);
        addTerminalLine("  theme <name>   - Switch theme (dark, 98, amber, green)", 0xFFCCCCCC);
        addTerminalLine("  reboot         - Reboot system back to BIOS POST", 0xFFCCCCCC);
        addTerminalLine("  shutdown       - Cleanly power off system", 0xFFCCCCCC);
    } else if (cmd == "ver") {
        addTerminalLine(osVersion, 0xFF44FF44);
        addTerminalLine(kernelVersion + " [" + buildNumber + "]", 0xFFAAAAAA);
    } else if (cmd == "ps") {
        std::ostringstream head;
        head << std::left << std::setw(6) << "PID" 
             << std::setw(22) << "NAME" 
             << std::setw(10) << "CPU %" 
             << std::setw(12) << "MEM (MB)" 
             << "STATUS";
        addTerminalLine(head.str(), 0xFFFFFF55);
        for (const auto& p : m_processes) {
            std::ostringstream row;
            row << std::left << std::setw(6) << p.pid 
                << std::setw(22) << p.name 
                << std::setw(10) << std::fixed << std::setprecision(1) << p.cpuUsage
                << std::setw(12) << std::fixed << std::setprecision(1) << p.memoryMB 
                << p.status;
            addTerminalLine(row.str(), p.isSystem ? 0xFF88AAFF : 0xFFCCCCCC);
        }
    } else if (cmd == "kill") {
        int pidToKill = -1;
        if (ss >> pidToKill) {
            std::string msg;
            bool ok = killProcess(pidToKill, msg);
            addTerminalLine(msg, ok ? 0xFF44FF44 : 0xFFFF4444);
        } else {
            addTerminalLine("Usage: kill <pid>", 0xFFFF8888);
        }
    } else if (cmd == "mem") {
        std::ostringstream oss;
        oss << "Memory Statistics:\n"
            << "  Physical RAM : " << static_cast<int>(m_totalMemoryMB) << " MB used / " 
            << static_cast<int>(m_maxMemoryMB) << " MB total (" 
            << std::fixed << std::setprecision(1) << (m_totalMemoryMB / m_maxMemoryMB * 100.0f) << "%)\n"
            << "  Swap/Pagefile: 512 MB allocated / 4096 MB reserved";
        addTerminalLine(oss.str(), 0xFF44FFFF);
    } else if (cmd == "uptime") {
        addTerminalLine("System Uptime: " + getUptimeString(), 0xFF44FF44);
    } else if (cmd == "date" || cmd == "time") {
        addTerminalLine(getCurrentDateString() + "  " + getCurrentTimeString(), 0xFFFFFF88);
    } else if (cmd == "cls" || cmd == "clear") {
        clearTerminal();
    } else if (cmd == "dir" || cmd == "ls") {
        addTerminalLine("Directory listing for " + m_currentDirPath + ":", 0xFFFFFF55);
        if (m_currentDir) {
            for (const auto& item : m_currentDir->children) {
                std::ostringstream oss;
                oss << "  " << (item.isDirectory ? "<DIR> " : "      ")
                    << std::left << std::setw(24) << item.name
                    << (item.isDirectory ? "" : (std::to_string(item.sizeBytes) + " bytes"));
                addTerminalLine(oss.str(), item.isDirectory ? 0xFF55FFFF : 0xFFFFFFFF);
            }
        }
    } else if (cmd == "cat") {
        std::string filename;
        if (ss >> filename) {
            const VirtualFile* file = findFileByPath(filename);
            if (file && file->isDirectory) {
                addTerminalLine("Cannot read a directory: " + filename, 0xFFFF4444);
            } else if (file) {
                addTerminalLine("--- " + file->name + " ---", 0xFFFFFF88);
                std::stringstream fileSS(file->content);
                std::string line;
                while (std::getline(fileSS, line)) {
                    addTerminalLine(line, 0xFFDDDDDD);
                }
            } else {
                addTerminalLine("File not found: " + filename, 0xFFFF4444);
            }
        } else {
            addTerminalLine("Usage: cat <filename>", 0xFFFF8888);
        }
    } else if (cmd == "echo") {
        std::string rest;
        std::getline(ss, rest);
        if (!rest.empty() && rest[0] == ' ') rest = rest.substr(1);
        addTerminalLine(rest, 0xFFFFFFFF);
    } else if (cmd == "taskmgr") {
        showTaskManager = true;
        addTerminalLine("Task Manager window opened.", 0xFF44FF44);
    } else if (cmd == "explorer") {
        showFileExplorer = true;
        addTerminalLine("File Explorer window opened.", 0xFF44FF44);
    } else if (cmd == "theme") {
        std::string t;
        if (ss >> t) {
            if (t == "dark") setTheme(ThemeMode::CYBER_DARK);
            else if (t == "98" || t == "classic") setTheme(ThemeMode::CLASSIC_98);
            else if (t == "amber") setTheme(ThemeMode::RETRO_AMBER);
            else if (t == "green" || t == "matrix") setTheme(ThemeMode::MATRIX_GREEN);
            else addTerminalLine("Unknown theme. Options: dark, 98, amber, green", 0xFFFF8888);
        }
    } else if (cmd == "reboot") {
        addTerminalLine("Initiating system reboot...", 0xFFFFFF55);
        reboot();
    } else if (cmd == "shutdown" || cmd == "exit") {
        addTerminalLine("Shutting down CSOPESY Workstation OS...", 0xFFFF5555);
        requestShutdown();
    } else {
        addTerminalLine("'" + cmd + "' is not recognized as an internal command. Type 'help' for command list.", 0xFFFF5555);
    }
}

void SystemContext::setTheme(ThemeMode theme) {
    m_theme = theme;
}

} // namespace OS
