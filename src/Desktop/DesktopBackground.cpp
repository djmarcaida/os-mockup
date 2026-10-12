#include "DesktopBackground.h"
#include "UiHelpers.h"
#include "WallpaperSlideshow.h"
#include <imgui.h>
#include <iomanip>
#include <sstream>

namespace OS {

void DesktopBackground::render(SystemContext& ctx, float deltaTime) {
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
                               ImGuiWindowFlags_NoBackground |
                               ImGuiWindowFlags_NoScrollbar;

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

        WallpaperSlideshow::render(drawList, pMin, viewport->Size, deltaTime);

        // Watermark in bottom right (above taskbar)
        const char* watermark = "CSOPESY Workstation OS v2.4 (Enterprise Edition)";
        float wmWidth = ImGui::CalcTextSize(watermark).x;
        drawList->AddText(
            ImVec2(pMax.x - wmWidth - 20.0f, pMax.y - 70.0f),
            IM_COL32(255, 255, 255, 40),
            watermark
        );

        // 2. Desktop Shortcut Icons Grid (Left side)
        const float iconStartX = pMin.x + 24.0f;
        const float iconStartY = pMin.y + 20.0f;
        const float iconW = 104.0f;
        const float iconH = 72.0f;
        const float iconStep = 84.0f;
        const float usableHeight = viewport->Size.y - Ui::TaskbarHeight - 76.0f;
        const int rows = std::max(1, static_cast<int>(usableHeight / iconStep));

        struct DesktopShortcut {
            const char* label;
            const char* tag;
            bool* targetBool;
            Ui::Icon icon;
        };

        DesktopShortcut shortcuts[] = {
            {"Task Manager", "##sc_taskmgr", &ctx.showTaskManager, Ui::Icon::Tasks},
            {"File Explorer", "##sc_explorer", &ctx.showFileExplorer, Ui::Icon::Folder},
            {"Terminal", "##sc_terminal", &ctx.showTerminal, Ui::Icon::Terminal},
            {"System Info", "##sc_sysinfo", &ctx.showSystemInfo, Ui::Icon::System},
            {"Readme.txt", "##sc_readme", &ctx.showNotepad, Ui::Icon::Note}
        };

        for (size_t i = 0; i < sizeof(shortcuts)/sizeof(shortcuts[0]); ++i) {
            const int index = static_cast<int>(i);
            ImGui::SetCursorScreenPos(ImVec2(iconStartX + (index / rows) * 116.0f,
                                            iconStartY + (index % rows) * iconStep));
            ImGui::PushID(shortcuts[i].tag);
            
            // Stylized desktop icon button
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.18f, 0.28f, 0.55f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.45f, 0.75f, 0.85f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.35f, 0.65f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.94f, .97f, 1.0f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

            if (ImGui::Button("##shortcut", ImVec2(iconW, iconH))) {
                if (shortcuts[i].icon == Ui::Icon::Note) {
                    const VirtualFile* readme = ctx.findFileByPath("C:\\Users\\Administrator\\Desktop\\readme.txt");
                    if (readme && !readme->isDirectory) ctx.openFileInNotepad(readme);
                    else *shortcuts[i].targetBool = true;
                } else {
                    *shortcuts[i].targetBool = true;
                }
            }
            const ImVec2 itemMin = ImGui::GetItemRectMin();
            Ui::drawIcon(drawList, shortcuts[i].icon, ImVec2(itemMin.x + 38.0f, itemMin.y + 10.0f),
                         28.0f, IM_COL32(224, 241, 255, 255));
            const float labelWidth = ImGui::CalcTextSize(shortcuts[i].label).x;
            drawList->AddText(ImVec2(itemMin.x + (iconW - labelWidth) * .5f, itemMin.y + 48.0f),
                              IM_COL32(240, 247, 255, 255), shortcuts[i].label);
            if (ImGui::IsItemHovered() || ImGui::IsItemFocused()) ImGui::SetTooltip("Open %s", shortcuts[i].label);

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(4);
            ImGui::PopID();
        }

        // 3. Top-Left Quick System Bar (Visible PWR Button)
        ImGui::SetCursorScreenPos(ImVec2(pMin.x + 24.0f, pMax.y - Ui::TaskbarHeight - 48.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.18f, 0.18f, 0.85f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.23f, 0.23f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.60f, 0.10f, 0.10f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

        if (ImGui::Button("PWR (Exit)", ImVec2(100.0f, 34.0f))) {
            ctx.showPowerDialog = true;
        }
        if (ImGui::IsItemHovered() || ImGui::IsItemFocused()) ImGui::SetTooltip("Power options (Alt+F4)");

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);

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
        ImGui::SetCursorScreenPos(ImVec2(clockPos.x + 168.0f, clockPos.y + 68.0f));
        ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.3f, 1.0f), "CPU: %.1f%%", ctx.getTotalCpuUsage());

        ImGui::SetCursorScreenPos(ImVec2(clockPos.x + 16.0f, clockPos.y + 88.0f));
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 0.8f), "RAM: %d MB / %d MB", 
            static_cast<int>(ctx.getTotalMemoryMB()), static_cast<int>(ctx.getMaxMemoryMB()));
    }
    ImGui::End();

    ImGui::PopStyleVar(2);

}

} // namespace OS
