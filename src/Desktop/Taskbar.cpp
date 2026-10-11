#include "Taskbar.h"
#include <imgui.h>

namespace OS {

void Taskbar::render(SystemContext& ctx, float deltaTime) {
    (void)deltaTime;
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    const float taskbarHeight = 44.0f;
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - taskbarHeight));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, taskbarHeight));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                           ImGuiWindowFlags_NoMove |
                           ImGuiWindowFlags_NoResize |
                           ImGuiWindowFlags_NoSavedSettings |
                           ImGuiWindowFlags_NoScrollbar |
                           ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 6.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

    // Taskbar styling based on theme
    ThemeMode theme = ctx.getTheme();
    ImVec4 taskbarBg(0.08f, 0.12f, 0.20f, 0.95f);
    ImVec4 borderColor(0.2f, 0.3f, 0.45f, 0.6f);

    if (theme == ThemeMode::CLASSIC_98) {
        taskbarBg = ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
        borderColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    } else if (theme == ThemeMode::RETRO_AMBER) {
        taskbarBg = ImVec4(0.12f, 0.08f, 0.02f, 0.95f);
        borderColor = ImVec4(0.5f, 0.3f, 0.1f, 0.6f);
    } else if (theme == ThemeMode::MATRIX_GREEN) {
        taskbarBg = ImVec4(0.02f, 0.08f, 0.04f, 0.95f);
        borderColor = ImVec4(0.1f, 0.5f, 0.2f, 0.6f);
    }

    ImGui::PushStyleColor(ImGuiCol_WindowBg, taskbarBg);
    ImGui::PushStyleColor(ImGuiCol_Border, borderColor);

    if (ImGui::Begin("##FIXED_TASKBAR_DOCK", nullptr, flags)) {
        // 1. Start Button
        bool isStartActive = ctx.showStartMenu;
        if (isStartActive) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.45f, 0.8f, 1.0f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.35f, 0.65f, 0.9f));
        }

        if (ImGui::Button("  CSOPESY Start  ", ImVec2(130.0f, 32.0f))) {
            ctx.showStartMenu = !ctx.showStartMenu;
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();

        // 2. Application Quick Launch & Switcher Buttons
        auto renderAppButton = [](const char* name, bool& isOpen, const char* iconTag) {
            (void)iconTag;
            if (isOpen) {
                // Highlighted active state
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.40f, 0.65f, 0.95f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.9f, 1.0f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.18f, 0.26f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.85f, 0.85f, 0.9f));
            }

            if (ImGui::Button(name, ImVec2(130.0f, 32.0f))) {
                isOpen = !isOpen;
            }

            ImGui::PopStyleColor(2);
            ImGui::SameLine();
        };

        // a) Task Manager button
        renderAppButton("Task Manager", ctx.showTaskManager, "taskmgr");
        // b) Custom Screen 1: File Explorer
        renderAppButton("File Explorer", ctx.showFileExplorer, "explorer");
        // c) Custom Screen 1 (alt): Terminal / Console
        renderAppButton("Terminal (sh)", ctx.showTerminal, "terminal");
        // d) Custom Screen 2: System Settings / System Info
        renderAppButton("System Info", ctx.showSystemInfo, "sysinfo");
        // e) Notes / Editor
        renderAppButton("Notepad", ctx.showNotepad, "notepad");

        // 3. System Tray (Right edge alignment)
        float trayWidth = 270.0f;
        float trayStartX = viewport->Size.x - trayWidth - 12.0f;
        ImGui::SetCursorPosX(trayStartX);

        // Volume Indicator Button
        std::string volLabel = ctx.isMuted ? "Mute" : ("Vol " + std::to_string(ctx.volumeLevel) + "%");
        if (ImGui::Button(volLabel.c_str(), ImVec2(72.0f, 32.0f))) {
            ctx.showVolumePopup = !ctx.showVolumePopup;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Audio: %d%% (%s)\nClick to adjust volume", 
                ctx.volumeLevel, ctx.isMuted ? "Muted" : "Active");
        }

        ImGui::SameLine();

        // Network Indicator Button
        if (ImGui::Button("LAN", ImVec2(48.0f, 32.0f))) {
            ctx.showNetworkPopup = !ctx.showNetworkPopup;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Network: Virtual Ethernet\nIP: %s\nStatus: Connected", ctx.ipAddress.c_str());
        }

        ImGui::SameLine();

        // System Tray Clock Button
        std::string clockStr = ctx.getCurrentTimeString();
        if (ImGui::Button(clockStr.c_str(), ImVec2(80.0f, 32.0f))) {
            // Clicking toggles System Info
            ctx.showSystemInfo = !ctx.showSystemInfo;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s\nUptime: %s", ctx.getCurrentDateString().c_str(), ctx.getUptimeString().c_str());
        }

        ImGui::SameLine();

        // Shutdown / PWR Button on taskbar
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.25f, 0.25f, 1.0f));
        if (ImGui::Button("PWR", ImVec2(42.0f, 32.0f))) {
            ctx.showPowerDialog = true;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Power Options / Clean Exit");
        }
        ImGui::PopStyleColor(2);
    }
    ImGui::End();

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);

    // 4. Volume Popup
    if (ctx.showVolumePopup) {
        ImGui::SetNextWindowPos(ImVec2(viewport->Size.x - 220.0f, viewport->Size.y - taskbarHeight - 140.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(200.0f, 130.0f), ImGuiCond_Always);
        if (ImGui::Begin("Volume Control##tray_vol", &ctx.showVolumePopup, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
            ImGui::Text("Master Volume");
            ImGui::SliderInt("##volume_slider", &ctx.volumeLevel, 0, 100, "%d%%");
            ImGui::Checkbox("Mute Audio", &ctx.isMuted);
            ImGui::Spacing();
            if (ImGui::Button("Close", ImVec2(60.0f, 22.0f))) {
                ctx.showVolumePopup = false;
            }
        }
        ImGui::End();
    }

    // 5. Network Popup
    if (ctx.showNetworkPopup) {
        ImGui::SetNextWindowPos(ImVec2(viewport->Size.x - 260.0f, viewport->Size.y - taskbarHeight - 160.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(250.0f, 150.0f), ImGuiCond_Always);
        if (ImGui::Begin("Network Status##tray_net", &ctx.showNetworkPopup, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "Connected");
            ImGui::Separator();
            ImGui::Text("Adapter: CSOPESY Virtual vNIC 0");
            ImGui::Text("IP Address: %s", ctx.ipAddress.c_str());
            ImGui::Text("Subnet: 255.255.255.0");
            ImGui::Text("Gateway: 192.168.1.1");
            if (ImGui::Button("Close", ImVec2(60.0f, 22.0f))) {
                ctx.showNetworkPopup = false;
            }
        }
        ImGui::End();
    }

    // 6. Start Menu Popup
    if (ctx.showStartMenu) {
        renderStartMenu(ctx);
    }
}

void Taskbar::renderStartMenu(SystemContext& ctx) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float menuWidth = 320.0f;
    const float menuHeight = 390.0f;
    const float taskbarHeight = 44.0f;

    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + 8.0f, viewport->Pos.y + viewport->Size.y - taskbarHeight - menuHeight - 4.0f));
    ImGui::SetNextWindowSize(ImVec2(menuWidth, menuHeight));

    ImGuiWindowFlags menuFlags = ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoCollapse |
                                 ImGuiWindowFlags_NoTitleBar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.14f, 0.22f, 0.98f));

    if (ImGui::Begin("##CSOPESY_START_MENU", &ctx.showStartMenu, menuFlags)) {
        // User profile banner
        ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "CSOPESY Administrator");
        ImGui::TextDisabled("Workstation Session - Local Host");
        ImGui::Separator();
        ImGui::Spacing();

        // Application list
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "APPLICATIONS");
        ImGui::Spacing();

        if (ImGui::Selectable("  [Task Manager] System Processes", false, 0, ImVec2(0, 26))) {
            ctx.showTaskManager = true;
            ctx.showStartMenu = false;
        }
        if (ImGui::Selectable("  [File Explorer] Virtual Disk (C:)", false, 0, ImVec2(0, 26))) {
            ctx.showFileExplorer = true;
            ctx.showStartMenu = false;
        }
        if (ImGui::Selectable("  [Terminal] Command Prompt (sh)", false, 0, ImVec2(0, 26))) {
            ctx.showTerminal = true;
            ctx.showStartMenu = false;
        }
        if (ImGui::Selectable("  [System Info] Hardware & Themes", false, 0, ImVec2(0, 26))) {
            ctx.showSystemInfo = true;
            ctx.showStartMenu = false;
        }
        if (ImGui::Selectable("  [Notepad] Text Editor", false, 0, ImVec2(0, 26))) {
            ctx.showNotepad = true;
            ctx.showStartMenu = false;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Power Options
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.65f, 0.2f, 0.2f, 0.85f));
        if (ImGui::Button("Shut Down...", ImVec2(130.0f, 30.0f))) {
            ctx.showPowerDialog = true;
            ctx.showStartMenu = false;
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();

        if (ImGui::Button("Restart...", ImVec2(130.0f, 30.0f))) {
            ctx.reboot();
            ctx.showStartMenu = false;
        }
    }
    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

} // namespace OS
