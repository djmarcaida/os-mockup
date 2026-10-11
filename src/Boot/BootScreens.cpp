#include "BootScreens.h"
#include <imgui.h>
#include <cmath>
#include <string>

namespace OS {

void BootScreens::renderBiosPost(SystemContext& ctx, float deltaTime) {
    (void)deltaTime;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | 
                           ImGuiWindowFlags_NoMove | 
                           ImGuiWindowFlags_NoResize | 
                           ImGuiWindowFlags_NoSavedSettings | 
                           ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.02f, 0.02f, 0.02f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24.0f, 24.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 4.0f));

    if (ImGui::Begin("##BIOS_POST_SCREEN", nullptr, flags)) {
        // Draw Energy Star ASCII logo on top-right using DrawList or columns
        float contentWidth = ImGui::GetContentRegionAvail().x;
        
        ImGui::Columns(2, "##bios_header_col", false);
        ImGui::SetColumnWidth(0, contentWidth - 280.0f);

        // Column 1: Megatrends header
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "CSOPESY Modular BIOS v4.51PG, An Energy Star Ally");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Copyright (C) 1984-2026, CSOPESY Megatrends, Inc.");
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 1.0f, 1.0f), "P3B-F ACPI BIOS Revision 1006");
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "Intel(R) Pentium(R) III CPU at 733MHz");

        ImGui::NextColumn();

        // Column 2: Energy Star Logo
        ImGui::TextColored(ImVec4(1.0f, 0.95f, 0.2f, 1.0f), "  .------------------------------.");
        ImGui::TextColored(ImVec4(1.0f, 0.95f, 0.2f, 1.0f), "  |   EPA POLLUTION PREVENTER    |");
        ImGui::TextColored(ImVec4(1.0f, 0.95f, 0.2f, 1.0f), "  |          ENERGY STAR         |");
        ImGui::TextColored(ImVec4(1.0f, 0.95f, 0.2f, 1.0f), "  '------------------------------'");

        ImGui::Columns(1);
        ImGui::Separator();
        ImGui::Spacing();

        // Memory check
        uint32_t memKB = ctx.getBiosMemoryKB();
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "Memory Test :  ");
        ImGui::SameLine();
        if (memKB >= 262144) {
            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "%u KB OK", memKB);
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.2f, 1.0f), "%u KB Testing...", memKB);
        }

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Award Plug and Play BIOS Extension v1.0A");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Initialize Plug and Play Cards...");
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "PNP Init Completed Successfully");

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "Detecting IDE Drives:");
        ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), "  Detecting IDE Primary Master   ... CSOPESY-VHD 40.0 GB (UDMA 66)");
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),     "  Detecting IDE Primary Slave    ... None");
        ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), "  Detecting IDE Secondary Master ... ATAPI CD-ROM 52X Mode 4");
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),     "  Detecting IDE Secondary Slave  ... None");

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Hardware Diagnostics:");
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "  [OK] PS/2 101/104-Key Standard Keyboard Detected");
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "  [OK] PS/2 Wheel Mouse Initialized (IRQ 12)");
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "  [OK] Floppy Controller 1.44M 3.5-in Verified");
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "  [OK] PCI VGA 3D Accelerator Card (Vendor 10DE / Dev 01D1)");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Footer prompt with blinking cursor
        bool showCursor = (static_cast<int>(ctx.getStateTimer() * 3.5f) % 2) == 0;
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.3f, 1.0f), 
            "Press DEL to enter SETUP , ESC or SPACE to skip POST%s", showCursor ? "_" : " ");

        // User skip button in bottom corner
        ImGui::SetCursorPosY(viewport->Size.y - 65.0f);
        if (ImGui::Button("Skip POST [Space]", ImVec2(160, 28))) {
            ctx.setState(BootState::BOOT_SPLASH);
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(Automatic transition in %.1f sec)", 
            std::max(0.0f, ctx.getPostDuration() - ctx.getStateTimer()));

        // Keyboard hotkeys to skip
        if (ImGui::IsKeyPressed(ImGuiKey_Escape) || 
            ImGui::IsKeyPressed(ImGuiKey_Space) || 
            ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            ctx.setState(BootState::BOOT_SPLASH);
        }
    }
    ImGui::End();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

void BootScreens::renderBootSplash(SystemContext& ctx, float deltaTime) {
    (void)deltaTime;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | 
                           ImGuiWindowFlags_NoMove | 
                           ImGuiWindowFlags_NoResize | 
                           ImGuiWindowFlags_NoSavedSettings | 
                           ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.07f, 0.12f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(32.0f, 32.0f));

    if (ImGui::Begin("##BOOT_SPLASH_SCREEN", nullptr, flags)) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        // Draw elegant radial / gradient backdrop
        drawList->AddRectFilledMultiColor(
            viewport->Pos, 
            ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y),
            IM_COL32(10, 15, 30, 255),
            IM_COL32(20, 35, 65, 255),
            IM_COL32(15, 20, 45, 255),
            IM_COL32(8, 12, 24, 255)
        );

        // Center calculation
        float centerX = viewport->Size.x * 0.5f;
        float startY = viewport->Size.y * 0.25f;

        ImGui::SetCursorPosY(startY);

        // Centered ASCII Logo: CSOPESY
        const char* asciiLogo[] = {
            "  ██████╗ ███████╗ ██████╗ ██████╗ ███████╗███████╗██╗   ██╗",
            " ██╔════╝ ██╔════╝██╔═══██╗██╔══██╗██╔════╝██╔════╝╚██╗ ██╔╝",
            " ██║      ███████╗██║   ██║██████╔╝█████╗  ███████╗ ╚████╔╝ ",
            " ██║      ╚════██║██║   ██║██╔═══╝ ██╔══╝  ╚════██║  ╚██╔╝  ",
            " ╚██████╗ ███████║╚██████╔╝██║     ███████╗███████║   ██║   ",
            "  ╚═════╝ ╚══════╝ ╚═════╝ ╚═╝     ╚══════╝╚══════╝   ╚═╝   "
        };

        for (int i = 0; i < 6; ++i) {
            float textWidth = ImGui::CalcTextSize(asciiLogo[i]).x;
            ImGui::SetCursorPosX(centerX - textWidth * 0.5f);
            ImGui::TextColored(ImVec4(0.35f, 0.75f, 1.0f, 1.0f), "%s", asciiLogo[i]);
        }

        ImGui::Spacing();
        ImGui::Spacing();

        // Subtitle & Version
        const char* title = "CSOPESY Workstation OS v2.4 (Enterprise Edition)";
        float titleW = ImGui::CalcTextSize(title).x;
        ImGui::SetCursorPosX(centerX - titleW * 0.5f);
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", title);

        const char* subtitle = "CSOPESY-x86 SMP Hybrid Microkernel 2.4.19  [Build 2026.10-release]";
        float subW = ImGui::CalcTextSize(subtitle).x;
        ImGui::SetCursorPosX(centerX - subW * 0.5f);
        ImGui::TextColored(ImVec4(0.6f, 0.75f, 0.9f, 0.8f), "%s", subtitle);

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        // Progress bar
        float progress = std::min(1.0f, ctx.getStateTimer() / ctx.getSplashDuration());
        float barWidth = std::min(480.0f, viewport->Size.x * 0.55f);
        ImGui::SetCursorPosX(centerX - barWidth * 0.5f);

        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.65f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.15f, 0.25f, 1.0f));
        ImGui::ProgressBar(progress, ImVec2(barWidth, 18.0f), "");
        ImGui::PopStyleColor(2);

        // Dynamic Loading Status Message
        const char* statusMsg = "Initializing system components...";
        if (progress < 0.22f) {
            statusMsg = "Initializing Hardware Abstraction Layer (HAL)...";
        } else if (progress < 0.45f) {
            statusMsg = "Mounting Virtual Root File System (/dev/vfs0)...";
        } else if (progress < 0.70f) {
            statusMsg = "Starting kernel thread scheduler & IPC bus...";
        } else if (progress < 0.92f) {
            statusMsg = "Loading OpenGL 3.3 Window Compositor...";
        } else {
            statusMsg = "Starting CSOPESY Desktop Shell...";
        }

        float statusW = ImGui::CalcTextSize(statusMsg).x;
        ImGui::SetCursorPosX(centerX - statusW * 0.5f);
        ImGui::TextColored(ImVec4(0.85f, 0.9f, 1.0f, 0.9f), "%s (%d%%)", statusMsg, static_cast<int>(progress * 100));

        // Skip button in bottom-right corner
        ImGui::SetCursorPosY(viewport->Size.y - 65.0f);
        ImGui::SetCursorPosX(centerX - 80.0f);
        if (ImGui::Button("Skip Loading [Space]", ImVec2(160, 28))) {
            ctx.setState(BootState::DESKTOP);
            ctx.showTaskManager = true;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Escape) || 
            ImGui::IsKeyPressed(ImGuiKey_Space) || 
            ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            ctx.setState(BootState::DESKTOP);
            ctx.showTaskManager = true;
        }
    }
    ImGui::End();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

} // namespace OS
