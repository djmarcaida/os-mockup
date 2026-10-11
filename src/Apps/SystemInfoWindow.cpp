#include "SystemInfoWindow.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

namespace OS {

void SystemInfoWindow::applyThemeToImGui(ThemeMode mode) {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    if (mode == ThemeMode::CYBER_DARK) {
        style.WindowRounding = 6.0f;
        style.FrameRounding = 4.0f;
        style.PopupRounding = 6.0f;
        style.ScrollbarRounding = 6.0f;
        style.GrabRounding = 4.0f;
        style.TabRounding = 4.0f;
        style.WindowBorderSize = 1.0f;

        colors[ImGuiCol_Text]                  = ImVec4(0.92f, 0.94f, 0.96f, 1.00f);
        colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.55f, 0.65f, 1.00f);
        colors[ImGuiCol_WindowBg]              = ImVec4(0.09f, 0.12f, 0.18f, 0.96f);
        colors[ImGuiCol_ChildBg]               = ImVec4(0.07f, 0.09f, 0.14f, 1.00f);
        colors[ImGuiCol_PopupBg]               = ImVec4(0.10f, 0.13f, 0.20f, 0.98f);
        colors[ImGuiCol_Border]                = ImVec4(0.20f, 0.28f, 0.40f, 0.70f);
        colors[ImGuiCol_FrameBg]               = ImVec4(0.13f, 0.17f, 0.26f, 0.80f);
        colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.18f, 0.25f, 0.38f, 0.90f);
        colors[ImGuiCol_FrameBgActive]         = ImVec4(0.22f, 0.32f, 0.48f, 1.00f);
        colors[ImGuiCol_TitleBg]               = ImVec4(0.08f, 0.10f, 0.16f, 1.00f);
        colors[ImGuiCol_TitleBgActive]         = ImVec4(0.12f, 0.20f, 0.35f, 1.00f);
        colors[ImGuiCol_Button]                = ImVec4(0.15f, 0.24f, 0.38f, 0.80f);
        colors[ImGuiCol_ButtonHovered]         = ImVec4(0.22f, 0.38f, 0.60f, 1.00f);
        colors[ImGuiCol_ButtonActive]          = ImVec4(0.18f, 0.32f, 0.52f, 1.00f);
        colors[ImGuiCol_Header]                = ImVec4(0.18f, 0.28f, 0.44f, 0.70f);
        colors[ImGuiCol_HeaderHovered]         = ImVec4(0.25f, 0.40f, 0.62f, 0.85f);
        colors[ImGuiCol_HeaderActive]          = ImVec4(0.22f, 0.35f, 0.55f, 1.00f);
        colors[ImGuiCol_Tab]                   = ImVec4(0.11f, 0.15f, 0.24f, 0.85f);
        colors[ImGuiCol_TabHovered]            = ImVec4(0.22f, 0.38f, 0.60f, 1.00f);
        colors[ImGuiCol_TabActive]             = ImVec4(0.16f, 0.28f, 0.46f, 1.00f);
    } else if (mode == ThemeMode::CLASSIC_98) {
        // Windows 95 / 98 Classic look
        style.WindowRounding = 0.0f;
        style.FrameRounding = 0.0f;
        style.PopupRounding = 0.0f;
        style.ScrollbarRounding = 0.0f;
        style.GrabRounding = 0.0f;
        style.TabRounding = 0.0f;
        style.WindowBorderSize = 2.0f;

        colors[ImGuiCol_Text]                  = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg]              = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
        colors[ImGuiCol_ChildBg]               = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
        colors[ImGuiCol_PopupBg]               = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
        colors[ImGuiCol_Border]                = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        colors[ImGuiCol_FrameBg]               = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
        colors[ImGuiCol_FrameBgActive]         = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
        colors[ImGuiCol_TitleBg]               = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_TitleBgActive]         = ImVec4(0.00f, 0.00f, 0.50f, 1.00f);
        colors[ImGuiCol_Button]                = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
        colors[ImGuiCol_ButtonHovered]         = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
        colors[ImGuiCol_ButtonActive]          = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
        colors[ImGuiCol_Header]                = ImVec4(0.00f, 0.00f, 0.50f, 0.80f);
        colors[ImGuiCol_HeaderHovered]         = ImVec4(0.10f, 0.10f, 0.60f, 0.90f);
        colors[ImGuiCol_HeaderActive]          = ImVec4(0.00f, 0.00f, 0.40f, 1.00f);
        colors[ImGuiCol_Tab]                   = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
        colors[ImGuiCol_TabHovered]            = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
        colors[ImGuiCol_TabActive]             = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
    } else if (mode == ThemeMode::RETRO_AMBER) {
        // Monochrome Amber Phosphor
        style.WindowRounding = 4.0f;
        style.FrameRounding = 2.0f;
        style.WindowBorderSize = 1.0f;

        colors[ImGuiCol_Text]                  = ImVec4(1.00f, 0.72f, 0.10f, 1.00f);
        colors[ImGuiCol_TextDisabled]          = ImVec4(0.60f, 0.42f, 0.05f, 1.00f);
        colors[ImGuiCol_WindowBg]              = ImVec4(0.08f, 0.05f, 0.01f, 0.98f);
        colors[ImGuiCol_ChildBg]               = ImVec4(0.05f, 0.03f, 0.00f, 1.00f);
        colors[ImGuiCol_PopupBg]               = ImVec4(0.10f, 0.06f, 0.01f, 0.98f);
        colors[ImGuiCol_Border]                = ImVec4(0.60f, 0.40f, 0.05f, 0.80f);
        colors[ImGuiCol_FrameBg]               = ImVec4(0.16f, 0.10f, 0.02f, 0.85f);
        colors[ImGuiCol_TitleBg]               = ImVec4(0.10f, 0.06f, 0.01f, 1.00f);
        colors[ImGuiCol_TitleBgActive]         = ImVec4(0.35f, 0.22f, 0.04f, 1.00f);
        colors[ImGuiCol_Button]                = ImVec4(0.25f, 0.16f, 0.02f, 0.85f);
        colors[ImGuiCol_ButtonHovered]         = ImVec4(0.45f, 0.30f, 0.05f, 1.00f);
        colors[ImGuiCol_ButtonActive]          = ImVec4(0.35f, 0.22f, 0.04f, 1.00f);
    } else { // MATRIX_GREEN
        style.WindowRounding = 4.0f;
        style.FrameRounding = 2.0f;
        style.WindowBorderSize = 1.0f;

        colors[ImGuiCol_Text]                  = ImVec4(0.10f, 1.00f, 0.30f, 1.00f);
        colors[ImGuiCol_TextDisabled]          = ImVec4(0.05f, 0.50f, 0.15f, 1.00f);
        colors[ImGuiCol_WindowBg]              = ImVec4(0.02f, 0.06f, 0.03f, 0.98f);
        colors[ImGuiCol_ChildBg]               = ImVec4(0.01f, 0.04f, 0.02f, 1.00f);
        colors[ImGuiCol_PopupBg]               = ImVec4(0.03f, 0.08f, 0.04f, 0.98f);
        colors[ImGuiCol_Border]                = ImVec4(0.10f, 0.70f, 0.25f, 0.80f);
        colors[ImGuiCol_FrameBg]               = ImVec4(0.04f, 0.14f, 0.06f, 0.85f);
        colors[ImGuiCol_TitleBg]               = ImVec4(0.02f, 0.08f, 0.04f, 1.00f);
        colors[ImGuiCol_TitleBgActive]         = ImVec4(0.06f, 0.25f, 0.10f, 1.00f);
        colors[ImGuiCol_Button]                = ImVec4(0.05f, 0.20f, 0.08f, 0.85f);
        colors[ImGuiCol_ButtonHovered]         = ImVec4(0.10f, 0.40f, 0.15f, 1.00f);
        colors[ImGuiCol_ButtonActive]          = ImVec4(0.08f, 0.30f, 0.12f, 1.00f);
    }
}

void SystemInfoWindow::render(SystemContext& ctx, float deltaTime) {
    (void)deltaTime;
    if (!ctx.showSystemInfo) return;

    ImGui::SetNextWindowSize(ImVec2(620, 480), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(220, 120), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("System Settings & Hardware Diagnostics###SYSINFO_WND", &ctx.showSystemInfo)) {
        if (ImGui::BeginTabBar("SysInfoTabBar")) {
            // TAB 1: SYSTEM & HARDWARE
            if (ImGui::BeginTabItem("System Overview")) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "CSOPESY Workstation OS v2.4");
                ImGui::TextDisabled("Designed for Operating Systems Architecture Simulation");
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::Text("Architecture : x86_64 Compatible Multi-Tasking OS");
                ImGui::Text("Microkernel  : CSOPESY-x86 SMP Hybrid Core (v2.4.19)");
                ImGui::Text("Build ID     : %s", ctx.buildNumber.c_str());
                ImGui::Text("System Uptime: %s", ctx.getUptimeString().c_str());

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "Processor & Memory Specifications:");
                ImGui::Text("CPU Model    : %s", ctx.cpuModel.c_str());
                ImGui::Text("Clock Speed  : 733 MHz FSB 133MHz");
                ImGui::Text("L1 Cache     : 32 KB Instruction + Data");
                ImGui::Text("L2 Cache     : 256 KB On-die ATC");
                ImGui::Text("Installed RAM: 8,192 MB Virtual RAM (256 MB Simulated Physical)");

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.5f, 1.0f), "Host Graphics Pipeline (OpenGL Context):");
                
                const char* glVendor = (const char*)glGetString(GL_VENDOR);
                const char* glRenderer = (const char*)glGetString(GL_RENDERER);
                const char* glVersion = (const char*)glGetString(GL_VERSION);
                const char* glSlVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

                ImGui::Text("GPU Renderer : %s", glRenderer ? glRenderer : "Unknown GPU");
                ImGui::Text("GPU Vendor   : %s", glVendor ? glVendor : "Unknown Vendor");
                ImGui::Text("OpenGL Vers. : %s", glVersion ? glVersion : "Unknown GL");
                ImGui::Text("GLSL Vers.   : %s", glSlVersion ? glSlVersion : "Unknown GLSL");
                ImGui::Text("Dear ImGui   : v%s", ImGui::GetVersion());
                ImGui::Text("GLFW Library : %s", glfwGetVersionString());

                ImGui::EndTabItem();
            }

            // TAB 2: APPEARANCE & THEMES
            if (ImGui::BeginTabItem("Appearance")) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "Desktop Shell Theme Manager");
                ImGui::TextDisabled("Select an operating system appearance theme:");
                ImGui::Separator();
                ImGui::Spacing();

                auto themeRadioButton = [&](const char* name, ThemeMode mode) {
                    if (ImGui::RadioButton(name, ctx.getTheme() == mode)) {
                        ctx.setTheme(mode);
                        applyThemeToImGui(mode);
                    }
                };

                themeRadioButton("Cyber Dark (Modern Slate & Cyan Glow)", ThemeMode::CYBER_DARK);
                themeRadioButton("Classic Windows 98 / 2000 (Retro Gray & Teal)", ThemeMode::CLASSIC_98);
                themeRadioButton("Retro CRT Amber (Monochrome Phosphor)", ThemeMode::RETRO_AMBER);
                themeRadioButton("Matrix Green (Hacker Cyberpunk)", ThemeMode::MATRIX_GREEN);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::Text("Performance & Rendering:");
                ImGui::Text("Current FPS : %.1f FPS (%.2f ms/frame)", 
                    ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);

                static bool vsync = true;
                if (ImGui::Checkbox("Enable V-Sync (Vertical Synchronization)", &vsync)) {
                    glfwSwapInterval(vsync ? 1 : 0);
                }

                ImGui::EndTabItem();
            }

            // TAB 3: ABOUT CSOPESY
            if (ImGui::BeginTabItem("About")) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "CSOPESY Desktop OS Mock-up Emulator");
                ImGui::Text("C++ Systems Programming & Graphics Final Project");
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::TextWrapped(
                    "This software simulates an operating system runtime environment, from the 90s BIOS / POST "
                    "terminal phase to an animated boot splash loader, into an interactive desktop compositor.\n\n"
                    "Architecture Stack:\n"
                    " - Language: C++17 / C++20\n"
                    " - Graphics: OpenGL 3.3 Core Profile + GLFW\n"
                    " - Extension Loader: GLAD\n"
                    " - GUI Compositor: Dear ImGui (Docking / Reactive UI pipeline)\n\n"
                    "Developed for Operating Systems Engineering (CSOPESY)."
                );
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

} // namespace OS
