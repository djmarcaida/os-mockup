#include "WindowCompositor.h"
#include "../Desktop/DesktopBackground.h"
#include "../Desktop/Taskbar.h"
#include "../Desktop/UiHelpers.h"
#include "../Apps/TaskManagerWindow.h"
#include "../Apps/FileExplorerWindow.h"
#include "../Apps/TerminalWindow.h"
#include "../Apps/SystemInfoWindow.h"
#include "../Apps/NotepadWindow.h"
#include <imgui.h>

namespace OS {

void WindowCompositor::render(SystemContext& ctx, float deltaTime) {
    const ImVec2 size = ImGui::GetMainViewport()->Size;
    if (size.x <= 0.0f || size.y <= 0.0f) return;
    // One authoritative theme application covers Settings and terminal commands alike.
    SystemInfoWindow::applyThemeToImGui(ctx.getTheme());
    handleShortcuts(ctx);

    // Layer 1: Desktop Background (Base layer wallpaper, desktop icons, clock widget, PWR button)
    DesktopBackground::render(ctx, deltaTime);

    // Layer 2: Application Windows (Movable, focusable)
    TaskManagerWindow::render(ctx, deltaTime);
    FileExplorerWindow::render(ctx, deltaTime);
    TerminalWindow::render(ctx, deltaTime);
    SystemInfoWindow::render(ctx, deltaTime);
    NotepadWindow::render(ctx, deltaTime);

    // Layer 3: Fixed Taskbar Dock (Anchored along bottom edge, Start menu, system tray)
    Taskbar::render(ctx, deltaTime);

    // The modal is submitted last so every launch and native-close path shares it.
    renderPowerDialog(ctx);
}

void WindowCompositor::renderPowerDialog(SystemContext& ctx) {
    const char* popup = "Power Options##shutdown_modal";
    if (ctx.showPowerDialog && !ImGui::IsPopupOpen(popup)) ImGui::OpenPopup(popup);
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float width = std::min(480.0f, viewport->Size.x - 32.0f);
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(.5f, .5f));
    ImGui::SetNextWindowSize(ImVec2(width, 220.0f));
    if (ImGui::BeginPopupModal(popup, &ctx.showPowerDialog,
                              ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::Text("Power options");
        ImGui::Spacing();
        ImGui::TextWrapped("Virtual files and edits are kept only for this session. Shutting down discards them; restarting keeps them in memory.");
        if (ctx.notepadModified) ImGui::TextWrapped("Notepad has unsaved edits.");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        const float buttonWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2.0f) / 3.0f;
        if (ImGui::Button("Shut down", ImVec2(buttonWidth, 34.0f))) {
            ctx.requestShutdown();
            ctx.showPowerDialog = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Restart", ImVec2(buttonWidth, 34.0f))) {
            ctx.reboot();
            ctx.showPowerDialog = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 34.0f)) || ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
            ctx.showPowerDialog = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::EndPopup();
    }
}

void WindowCompositor::handleShortcuts(SystemContext& ctx) {
    ImGuiIO& io = ImGui::GetIO();
    if (ctx.showPowerDialog) return;

    // Ctrl + Shift + Esc: Toggle Task Manager
    if (io.KeyCtrl && io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
        ctx.showTaskManager = !ctx.showTaskManager;
    }

    // Alt + F4: Show Shutdown Modal
    if (io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_F4, false)) {
        ctx.showPowerDialog = true;
    }

    // F1: Toggle System Info
    if (ImGui::IsKeyPressed(ImGuiKey_F1, false)) {
        ctx.showSystemInfo = !ctx.showSystemInfo;
    }

    // ` (Tilde) or F12: Quick Terminal toggle
    if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent, false) && io.KeyCtrl) {
        ctx.showTerminal = !ctx.showTerminal;
    }
}

} // namespace OS
