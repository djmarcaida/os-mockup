#pragma once

#include "Types.h"
#include <vector>
#include <string>
#include <memory>
#include <ctime>

namespace OS {

class SystemContext {
public:
    SystemContext();
    ~SystemContext() = default;

    // State Machine
    BootState getState() const { return m_state; }
    void setState(BootState state);
    float getStateTimer() const { return m_stateTimer; }
    float getPostDuration() const { return m_postDuration; }
    float getSplashDuration() const { return m_splashDuration; }
    void update(float deltaTime);
    void reboot();
    void requestShutdown();
    bool shouldCloseApp() const { return m_shouldCloseApp; }
    void setShouldCloseApp(bool val) { m_shouldCloseApp = val; }

    // Memory counter for POST screen
    uint32_t getBiosMemoryKB() const;

    // Time & Clock
    std::string getCurrentTimeString() const;
    std::string getCurrentDateString() const;
    std::string getUptimeString() const;
    float getTotalUptime() const { return m_totalUptime; }

    // Process Manager (Task Manager backend)
    const std::vector<ProcessInfo>& getProcesses() const { return m_processes; }
    bool killProcess(int pid, std::string& outMessage);
    void spawnProcess(const std::string& name, float cpu, float mem);
    float getTotalCpuUsage() const { return m_totalCpuUsage; }
    float getTotalMemoryMB() const { return m_totalMemoryMB; }
    float getMaxMemoryMB() const { return m_maxMemoryMB; }
    const float* getCpuHistory() const { return m_cpuHistory.data(); }
    const float* getMemoryHistory() const { return m_memoryHistory.data(); }
    int getHistoryCapacity() const { return static_cast<int>(m_cpuHistory.size()); }

    // Virtual File System
    const VirtualFile* getRootFileSystem() const { return m_vfsRoot.get(); }
    const VirtualFile* getCurrentDirectory() const { return m_currentDir; }
    const std::string& getCurrentDirPath() const { return m_currentDirPath; }
    bool navigateTo(const std::string& folderName);
    bool navigateUp();
    const VirtualFile* findFileByPath(const std::string& path) const;

    // Terminal Shell
    const std::vector<TerminalLine>& getTerminalLines() const { return m_terminalLines; }
    void executeTerminalCommand(const std::string& commandLine);
    void addTerminalLine(const std::string& text, uint32_t color = 0xFFFFFFFF, bool isCmd = false);
    void clearTerminal();

    // Text Editor / Notepad
    std::string notepadTitle = "Untitled - Notepad";
    std::string notepadBuffer;
    bool notepadModified = false;
    void openFileInNotepad(const std::string& title, const std::string& content);

    // Window Visibility Toggles
    bool showTaskManager = false;
    bool showFileExplorer = false;
    bool showTerminal = false;
    bool showSystemInfo = false;
    bool showNotepad = false;
    bool showStartMenu = false;
    bool showPowerDialog = false;
    bool showVolumePopup = false;
    bool showNetworkPopup = false;

    // Audio & Network State
    int volumeLevel = 75;
    bool isMuted = false;
    bool networkConnected = true;
    std::string ipAddress = "192.168.1.105";

    // Theme & Visuals
    ThemeMode getTheme() const { return m_theme; }
    void setTheme(ThemeMode theme);

    // Hardware Specs (Mock / Real)
    std::string cpuModel = "Intel(R) Pentium(R) III 733MHz (Coppermine)";
    std::string osVersion = "CSOPESY Workstation OS v2.4 (Enterprise Edition)";
    std::string kernelVersion = "CSOPESY-x86 SMP Hybrid Kernel 2.4.19";
    std::string buildNumber = "Build 2026.10.11-RELEASE";

private:
    void initProcesses();
    void initVFS();
    void updateProcesses(float deltaTime);
    void updatePerformanceHistory(float deltaTime);

    BootState m_state = BootState::BIOS_POST;
    float m_stateTimer = 0.0f;
    float m_postDuration = 3.2f;
    float m_splashDuration = 3.0f;
    float m_totalUptime = 0.0f;
    bool m_shouldCloseApp = false;

    // Processes
    std::vector<ProcessInfo> m_processes;
    float m_totalCpuUsage = 0.0f;
    float m_totalMemoryMB = 0.0f;
    const float m_maxMemoryMB = 8192.0f;
    std::vector<float> m_cpuHistory;
    std::vector<float> m_memoryHistory;
    float m_historyTimer = 0.0f;

    // VFS
    std::unique_ptr<VirtualFile> m_vfsRoot;
    VirtualFile* m_currentDir = nullptr;
    std::string m_currentDirPath = "C:\\";

    // Terminal
    std::vector<TerminalLine> m_terminalLines;
    std::vector<std::string> m_commandHistory;

    // Theme
    ThemeMode m_theme = ThemeMode::CYBER_DARK;
};

} // namespace OS
