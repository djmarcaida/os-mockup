#include "DesktopBackground.h"
#include <imgui.h>
#include <iomanip>
#include <sstream>

namespace OS {

void DesktopBackground::render(SystemContext& ctx, float deltaTime) {
    (void)deltaTime;
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    // 1. Full-screen background window
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGuiWindowFlags bgFlags = ImGuiWindowFlags_NoDecoration |
                               ImGuiWindowFlags_NoMove |
                               ImGuiWindowFlags_NoResize |
                               ImGuiWindowFlags_NoSavedSettings |
                               ImGuiWindowFlags_NoBringToFrontOnFocus |
                               ImGuiWindowFlags_NoNavFocus |
                               ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (ImGui::Begin("##DESKTOP_WALLPAPER_LAYER", nullptr, bgFlags)) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 pMin = viewport->Pos;
        ImVec2 pMax = ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y);

        // Draw dynamic theme gradient wallpaper
        ThemeMode currentTheme = ctx.getTheme();
        if (currentTheme == ThemeMode::CYBER_DARK) {
            drawList->AddRectFilledMultiColor(
                pMin, pMax,
                IM_COL32(11, 19, 43, 255),   // Top-Left: Deep midnight
                IM_COL32(28, 37, 65, 255),   // Top-Right: Navy slate
                IM_COL32(42, 60, 90, 255),   // Bottom-Right: Steel twilight
                IM_COL32(15, 23, 42, 255)    // Bottom-Left: Abyss
            );
            // Subtle ambient grid pattern
            const float gridSize = 64.0f;
            for (float x = pMin.x; x < pMax.x; x += gridSize) {
                drawList->AddLine(ImVec2(x, pMin.y), ImVec2(x, pMax.y), IM_COL32(255, 255, 255, 6), 1.0f);
            }
            for (float y = pMin.y; y < pMax.y; y += gridSize) {
                drawList->AddLine(ImVec2(pMin.x, y), ImVec2(pMax.x, y), IM_COL32(255, 255, 255, 6), 1.0f);
            }
        } else if (currentTheme == ThemeMode::CLASSIC_98) {
            // Nostalgic Windows 98 Teal
            drawList->AddRectFilled(pMin, pMax, IM_COL32(0, 128, 128, 255));
        } else if (currentTheme == ThemeMode::RETRO_AMBER) {
            drawList->AddRectFilledMultiColor(
                pMin, pMax,
                IM_COL32(32, 18, 0, 255),
                IM_COL32(48, 28, 0, 255),
                IM_COL32(64, 38, 2, 255),
                IM_COL32(24, 12, 0, 255)
            );
        } else { // MATRIX_GREEN
            drawList->AddRectFilledMultiColor(
                pMin, pMax,
                IM_COL32(0, 15, 5, 255),
                IM_COL32(0, 25, 8, 255),
                IM_COL32(0, 35, 12, 255),
                IM_COL32(0, 10, 3, 255)
            );
        }

        // Watermark in bottom right (above taskbar)
        const char* watermark = "CSOPESY Workstation OS v2.4 (Enterprise Edition)";
        float wmWidth = ImGui::CalcTextSize(watermark).x;
        drawList->AddText(
            ImVec2(pMax.x - wmWidth - 20.0f, pMax.y - 70.0f),
            IM_COL32(255, 255, 255, 40),
            watermark
        );

        // 2. Desktop Shortcut Icons Grid (Left side)
        float iconStartX = pMin.x + 24.0f;
        float iconStartY = pMin.y + 24.0f;
        float iconW = 100.0f;
        float iconH = 80.0f;
        float iconGapY = 16.0f;

        struct DesktopShortcut {
            const char* label;
            const char* tag;
            bool* targetBool;
        };

        DesktopShortcut shortcuts[] = {
            {"[ Taskmgr ]\nTask Manager", "##sc_taskmgr", &ctx.showTaskManager},
            {"[ Explorer ]\nFile Explorer", "##sc_explorer", &ctx.showFileExplorer},
            {"[ Terminal ]\nCommand Line", "##sc_terminal", &ctx.showTerminal},
            {"[ System ]\nSystem Info", "##sc_sysinfo", &ctx.showSystemInfo},
            {"[ Readme ]\nReadme.txt", "##sc_readme", &ctx.showNotepad}
        };

        for (size_t i = 0; i < sizeof(shortcuts)/sizeof(shortcuts[0]); ++i) {
            ImGui::SetCursorScreenPos(ImVec2(iconStartX, iconStartY + i * (iconH + iconGapY)));
            ImGui::PushID(shortcuts[i].tag);
            
            // Stylized desktop icon button
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.18f, 0.28f, 0.55f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.45f, 0.75f, 0.85f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.35f, 0.65f, 0.95f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

            if (ImGui::Button(shortcuts[i].label, ImVec2(iconW, iconH))) {
                *shortcuts[i].targetBool = !(*shortcuts[i].targetBool);
            }

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);
            ImGui::PopID();
        }

        // 3. Top-Left Quick System Bar (Visible PWR Button)
        ImGui::SetCursorScreenPos(ImVec2(pMin.x + 24.0f, pMin.y + 540.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.18f, 0.18f, 0.85f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.60f, 0.10f, 0.10f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

        if (ImGui::Button("PWR (Exit)", ImVec2(100.0f, 34.0f))) {
            ctx.showPowerDialog = true;
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        // 4. Real-time System Clock & Stats Widget (Anchored in Fixed Top-Right Corner)
        float widgetW = 280.0f;
        float widgetH = 110.0f;
        ImVec2 clockPos = ImVec2(pMax.x - widgetW - 24.0f, pMin.y + 24.0f);

        // Background card for clock widget
        drawList->AddRectFilled(
            clockPos,
            ImVec2(clockPos.x + widgetW, clockPos.y + widgetH),
            IM_COL32(15, 23, 42, 200),
            10.0f
        );
        drawList->AddRect(
            clockPos,
            ImVec2(clockPos.x + widgetW, clockPos.y + widgetH),
            IM_COL32(56, 189, 248, 80),
            10.0f, 0, 1.5f
        );

        // Render digital clock text inside the card
        ImGui::SetCursorScreenPos(ImVec2(clockPos.x + 16.0f, clockPos.y + 12.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.35f, 0.85f, 1.0f, 1.0f));
        std::string timeStr = ctx.getCurrentTimeString();
        ImGui::SetWindowFontScale(1.4f);
        ImGui::Text("%s", timeStr.c_str());
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();

        ImGui::SetCursorScreenPos(ImVec2(clockPos.x + 16.0f, clockPos.y + 44.0f));
        ImGui::TextColored(ImVec4(0.85f, 0.9f, 0.95f, 0.9f), "%s", ctx.getCurrentDateString().c_str());

        ImGui::SetCursorScreenPos(ImVec2(clockPos.x + 16.0f, clockPos.y + 68.0f));
        ImGui::TextColored(ImVec4(0.6f, 0.8f, 0.6f, 0.9f), "Uptime: %s", ctx.getUptimeString().c_str());
        ImGui::SameLine(160.0f);
        ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.3f, 0.9f), "CPU: %.1f%%", ctx.getTotalCpuUsage());

        ImGui::SetCursorScreenPos(ImVec2(clockPos.x + 16.0f, clockPos.y + 88.0f));
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 0.8f), "RAM: %d MB / %d MB", 
            static_cast<int>(ctx.getTotalMemoryMB()), static_cast<int>(ctx.getMaxMemoryMB()));
    }
    ImGui::End();

    ImGui::PopStyleVar(2);

    // 5. Graceful Power / Shutdown Confirmation Modal Dialog
    if (ctx.showPowerDialog) {
        ImGui::OpenPopup("Power Options##shutdown_modal");
    }

    ImVec2 center = viewport->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(380.0f, 190.0f), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("Power Options##shutdown_modal", &ctx.showPowerDialog, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Shut down CSOPESY Workstation OS?");
        ImGui::Spacing();
        ImGui::TextWrapped("All running processes, file locks, and compositor state will be saved and cleanly closed.");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Shut Down (Clean Exit)", ImVec2(160.0f, 32.0f))) {
            ctx.requestShutdown();
            ImGui::CloseCurrentPopup();
            ctx.showPowerDialog = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Restart (Reboot)", ImVec2(110.0f, 32.0f))) {
            ctx.reboot();
            ImGui::CloseCurrentPopup();
            ctx.showPowerDialog = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(75.0f, 32.0f))) {
            ImGui::CloseCurrentPopup();
            ctx.showPowerDialog = false;
        }

        ImGui::EndPopup();
    }
}

} // namespace OS
