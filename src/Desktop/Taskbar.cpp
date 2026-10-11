#include "Taskbar.h"
#include "UiHelpers.h"
#include <imgui.h>

namespace OS {

void Taskbar::render(SystemContext& ctx, float deltaTime) {
    (void)deltaTime;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float bottom = viewport->Pos.y + viewport->Size.y;
    const bool labels = viewport->Size.x >= 1180.0f;
    const float gap = 6.0f;
    const float volumeWidth = labels ? 96.0f : 42.0f;
    const float trayWidth = volumeWidth + 42.0f + 80.0f + 48.0f + gap * 3.0f;
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, bottom - Ui::TaskbarHeight));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, Ui::TaskbarHeight));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 6.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(gap, 6.0f));
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar;

    if (ImGui::Begin("##FIXED_TASKBAR_DOCK", nullptr, flags)) {
        if (Ui::iconButton("##start", "CSOPESY Start", Ui::Icon::Start,
                           ImVec2(labels ? 140.0f : 42.0f, 32.0f), ctx.showStartMenu, labels))
            ctx.showStartMenu = !ctx.showStartMenu;

        struct App { const char* id; const char* name; Ui::Icon icon; bool* visible; };
        const App apps[] = {
            {"##tasks", "Task Manager", Ui::Icon::Tasks, &ctx.showTaskManager},
            {"##explorer", "File Explorer", Ui::Icon::Folder, &ctx.showFileExplorer},
            {"##terminal", "Terminal", Ui::Icon::Terminal, &ctx.showTerminal},
            {"##system", "System Info", Ui::Icon::System, &ctx.showSystemInfo},
            {"##notepad", "Notepad", Ui::Icon::Note, &ctx.showNotepad}
        };
        for (const App& app : apps) {
            ImGui::SameLine();
            const float width = labels ? ImGui::CalcTextSize(app.name).x + 48.0f : 42.0f;
            if (Ui::iconButton(app.id, app.name, app.icon, ImVec2(width, 32.0f), *app.visible, labels))
                *app.visible = !*app.visible;
        }

        // Reserve tray space explicitly; compact icon controls fit at 640 pixels.
        ImGui::SameLine();
        ImGui::SetCursorPosX(viewport->Size.x - trayWidth - 8.0f);
        if (Ui::iconButton("##volume", "Volume", Ui::Icon::Volume, ImVec2(volumeWidth, 32.0f),
                           ctx.showVolumePopup, labels)) ctx.showVolumePopup = !ctx.showVolumePopup;
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Simulated audio: %d%% (%s)", ctx.volumeLevel, ctx.isMuted ? "Muted" : "Active");
        ImGui::SameLine();
        if (Ui::iconButton("##network", "Virtual network", Ui::Icon::Network, ImVec2(42.0f, 32.0f),
                           ctx.showNetworkPopup)) ctx.showNetworkPopup = !ctx.showNetworkPopup;
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Virtual Ethernet\nIP: %s\n%s", ctx.ipAddress.c_str(),
                              ctx.networkConnected ? "Connected (simulated)" : "Disconnected (simulated)");
        ImGui::SameLine();
        const std::string clockLabel = ctx.getCurrentTimeString() + "###tray_clock";
        if (ImGui::Button(clockLabel.c_str(), ImVec2(80.0f, 32.0f)))
            ctx.showSystemInfo = !ctx.showSystemInfo;
        if (ImGui::IsItemHovered() || ImGui::IsItemFocused())
            ImGui::SetTooltip("%s\nUptime: %s\nOpen System Info", ctx.getCurrentDateString().c_str(), ctx.getUptimeString().c_str());
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.65f, .17f, .17f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(.8f, .23f, .23f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
        if (ImGui::Button("PWR", ImVec2(48.0f, 32.0f))) ctx.showPowerDialog = true;
        if (ImGui::IsItemHovered() || ImGui::IsItemFocused()) ImGui::SetTooltip("Power options (Alt+F4)");
        ImGui::PopStyleColor(3);
    }
    ImGui::End();
    ImGui::PopStyleVar(3);

    if (ctx.showVolumePopup) {
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + viewport->Size.x - 224.0f, bottom - Ui::TaskbarHeight - 156.0f));
        ImGui::SetNextWindowSize(ImVec2(216.0f, 148.0f));
        if (ImGui::Begin("Volume Control##tray_vol", &ctx.showVolumePopup,
                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings)) {
            ImGui::TextDisabled("Simulated audio controls");
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::SliderInt("##volume_slider", &ctx.volumeLevel, 0, 100, "%d%%");
            ImGui::Checkbox("Mute Audio", &ctx.isMuted);
            if (ImGui::Button("Close")) ctx.showVolumePopup = false;
        }
        ImGui::End();
    }
    if (ctx.showNetworkPopup) {
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + viewport->Size.x - 288.0f, bottom - Ui::TaskbarHeight - 196.0f));
        ImGui::SetNextWindowSize(ImVec2(280.0f, 188.0f));
        if (ImGui::Begin("Network Status##tray_net", &ctx.showNetworkPopup,
                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings)) {
            ImGui::Text("%s (simulated)", ctx.networkConnected ? "Connected" : "Disconnected");
            ImGui::Separator();
            ImGui::TextWrapped("Adapter: CSOPESY Virtual vNIC 0");
            ImGui::Text("IP Address: %s", ctx.ipAddress.c_str());
            ImGui::Text("Subnet: 255.255.255.0");
            ImGui::Text("Gateway: 192.168.1.1");
            if (ImGui::Button("Close")) ctx.showNetworkPopup = false;
        }
        ImGui::End();
    }
    if (ctx.showStartMenu) renderStartMenu(ctx);
}

void Taskbar::renderStartMenu(SystemContext& ctx) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float width = std::min(320.0f, viewport->Size.x - 16.0f);
    const float height = std::min(390.0f, viewport->Size.y - Ui::TaskbarHeight - 16.0f);
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + 8.0f,
                                  viewport->Pos.y + viewport->Size.y - Ui::TaskbarHeight - height - 8.0f));
    ImGui::SetNextWindowSize(ImVec2(width, height));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
    if (ImGui::Begin("##CSOPESY_START_MENU", &ctx.showStartMenu,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::Text("CSOPESY Administrator");
        ImGui::TextDisabled("Local simulation session");
        ImGui::Separator();
        ImGui::Spacing();
        struct App { const char* label; bool* visible; };
        const App apps[] = {
            {"Task Manager", &ctx.showTaskManager}, {"File Explorer", &ctx.showFileExplorer},
            {"Terminal", &ctx.showTerminal}, {"System Info & Themes", &ctx.showSystemInfo},
            {"Notepad", &ctx.showNotepad}
        };
        for (const App& app : apps) {
            if (ImGui::Selectable(app.label, false, 0, ImVec2(0, 30.0f))) {
                *app.visible = true;
                ctx.showStartMenu = false;
            }
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        if (ImGui::Button("Power options...", ImVec2(-1.0f, 32.0f))) {
            ctx.showPowerDialog = true;
            ctx.showStartMenu = false;
        }
        ImGui::TextWrapped("Shut down or restart the virtual workstation.");
        if (ImGui::IsKeyPressed(ImGuiKey_Escape, false)) ctx.showStartMenu = false;
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

} // namespace OS
