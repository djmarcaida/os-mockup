#include "WindowCompositor.h"
#include "../Desktop/DesktopBackground.h"
#include "../Desktop/Taskbar.h"
#include "../Apps/TaskManagerWindow.h"
#include "../Apps/FileExplorerWindow.h"
#include "../Apps/TerminalWindow.h"
#include "../Apps/SystemInfoWindow.h"
#include "../Apps/NotepadWindow.h"
#include <imgui.h>

namespace OS {

void WindowCompositor::render(SystemContext& ctx, float deltaTime) {
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
}

void WindowCompositor::handleShortcuts(SystemContext& ctx) {
    ImGuiIO& io = ImGui::GetIO();

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
