#include "SystemInfoWindow.h"
#include "../Desktop/UiHelpers.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

namespace OS {

void SystemInfoWindow::applyThemeToImGui(ThemeMode mode) {
    ImGuiStyle& style = ImGui::GetStyle();
    // Start every selection from the same complete palette and geometry.
    style = ImGuiStyle();
    ImGui::StyleColorsDark(&style);
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
        colors[ImGuiCol_TextDisabled]          = ImVec4(0.64f, 0.69f, 0.77f, 1.00f);
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
        colors[ImGuiCol_TextDisabled]          = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
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
        colors[ImGuiCol_TitleBg]               = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
        colors[ImGuiCol_TitleBgActive]         = ImVec4(0.55f, 0.68f, 0.82f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]      = colors[ImGuiCol_TitleBg];
        colors[ImGuiCol_Header]                = ImVec4(0.55f, 0.65f, 0.80f, 1.00f);
        colors[ImGuiCol_HeaderHovered]         = ImVec4(0.65f, 0.75f, 0.90f, 1.00f);
        colors[ImGuiCol_HeaderActive]          = ImVec4(0.50f, 0.60f, 0.80f, 1.00f);
        colors[ImGuiCol_Tab]                   = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
        colors[ImGuiCol_TabHovered]            = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
        colors[ImGuiCol_TabActive]             = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
    } else if (mode == ThemeMode::RETRO_AMBER) {
        // Monochrome Amber Phosphor
        style.WindowRounding = 4.0f;
        style.FrameRounding = 2.0f;
        style.WindowBorderSize = 1.0f;

        colors[ImGuiCol_Text]                  = ImVec4(1.00f, 0.72f, 0.10f, 1.00f);
        colors[ImGuiCol_TextDisabled]          = ImVec4(0.78f, 0.57f, 0.16f, 1.00f);
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
        colors[ImGuiCol_TextDisabled]          = ImVec4(0.12f, 0.68f, 0.28f, 1.00f);
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

    if (mode == ThemeMode::RETRO_AMBER || mode == ThemeMode::MATRIX_GREEN) {
        style.PopupRounding = style.ScrollbarRounding = 4.0f;
        style.GrabRounding = style.TabRounding = 2.0f;
        colors[ImGuiCol_FrameBgHovered] = colors[ImGuiCol_ButtonHovered];
        colors[ImGuiCol_FrameBgActive] = colors[ImGuiCol_ButtonActive];
        colors[ImGuiCol_Header] = colors[ImGuiCol_Button];
        colors[ImGuiCol_HeaderHovered] = colors[ImGuiCol_ButtonHovered];
        colors[ImGuiCol_HeaderActive] = colors[ImGuiCol_ButtonActive];
        colors[ImGuiCol_Tab] = colors[ImGuiCol_TitleBg];
        colors[ImGuiCol_TabHovered] = colors[ImGuiCol_ButtonHovered];
        colors[ImGuiCol_TabSelected] = colors[ImGuiCol_ButtonActive];
    }
    const ImVec4 accent = mode == ThemeMode::CLASSIC_98 ? ImVec4(.1f, .25f, .55f, 1)
        : mode == ThemeMode::CYBER_DARK ? ImVec4(.35f, .85f, 1, 1) : colors[ImGuiCol_Text];
    colors[ImGuiCol_CheckMark] = colors[ImGuiCol_SliderGrab] = colors[ImGuiCol_NavCursor] = accent;
    colors[ImGuiCol_SliderGrabActive] = accent;
    colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered] = colors[ImGuiCol_SeparatorActive] = accent;
    colors[ImGuiCol_ResizeGripHovered] = colors[ImGuiCol_ResizeGripActive] = accent;
    colors[ImGuiCol_ScrollbarGrab] = colors[ImGuiCol_Button];
    colors[ImGuiCol_ScrollbarGrabHovered] = colors[ImGuiCol_ButtonHovered];
    colors[ImGuiCol_ScrollbarGrabActive] = colors[ImGuiCol_ButtonActive];
    colors[ImGuiCol_TabDimmed] = colors[ImGuiCol_Tab];
    colors[ImGuiCol_TabDimmedSelected] = colors[ImGuiCol_TabSelected];
    colors[ImGuiCol_TabSelectedOverline] = colors[ImGuiCol_TabDimmedSelectedOverline] = accent;
}

void SystemInfoWindow::render(SystemContext& ctx, float deltaTime) {
    (void)deltaTime;
    if (!ctx.showSystemInfo) return;

    Ui::prepareAppWindow("System Settings & Hardware Diagnostics###SYSINFO_WND", ImVec2(620, 480), ImVec2(220, 120));

    if (ImGui::Begin("System Settings & Hardware Diagnostics###SYSINFO_WND", &ctx.showSystemInfo)) {
        if (ImGui::BeginTabBar("SysInfoTabBar")) {
            // TAB 1: SYSTEM & HARDWARE
            if (ImGui::BeginTabItem("System Overview")) {
                ImGui::Spacing();
                ImGui::Text("CSOPESY Workstation OS v2.4");
                ImGui::TextWrapped("Simulated OS hardware; host graphics information below is live.");
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextWrapped("Architecture : Simulated x86 desktop environment");
                ImGui::TextWrapped("Kernel model : CSOPESY-x86 SMP Hybrid Core (v2.4.19), simulated");
                ImGui::TextWrapped("Build ID     : %s", ctx.buildNumber.c_str());
                ImGui::TextWrapped("System Uptime: %s", ctx.getUptimeString().c_str());

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::Text("Simulated processor & memory:");
                ImGui::TextWrapped("CPU Model    : %s", ctx.cpuModel.c_str());
                ImGui::TextWrapped("Clock Speed  : 733 MHz FSB 133MHz");
                ImGui::TextWrapped("L1 Cache     : 32 KB Instruction + Data");
                ImGui::TextWrapped("L2 Cache     : 256 KB On-die ATC");
                ImGui::TextWrapped("Memory model : 8,192 MB simulated RAM; BIOS display: 256 MB");

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::Text("Live host graphics (OpenGL context):");
                
                const char* glVendor = glGetString ? reinterpret_cast<const char*>(glGetString(GL_VENDOR)) : nullptr;
                const char* glRenderer = glGetString ? reinterpret_cast<const char*>(glGetString(GL_RENDERER)) : nullptr;
                const char* glVersion = glGetString ? reinterpret_cast<const char*>(glGetString(GL_VERSION)) : nullptr;
                const char* glSlVersion = glGetString ? reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)) : nullptr;

                ImGui::TextWrapped("GPU Renderer : %s", glRenderer ? glRenderer : "Unknown GPU");
                ImGui::TextWrapped("GPU Vendor   : %s", glVendor ? glVendor : "Unknown Vendor");
                ImGui::TextWrapped("OpenGL Vers. : %s", glVersion ? glVersion : "Unknown GL");
                ImGui::TextWrapped("GLSL Vers.   : %s", glSlVersion ? glSlVersion : "Unknown GLSL");
                ImGui::TextWrapped("Dear ImGui   : v%s", ImGui::GetVersion());
                ImGui::TextWrapped("GLFW Library : %s", glfwGetVersionString());

                ImGui::EndTabItem();
            }

            // TAB 2: APPEARANCE & THEMES
            if (ImGui::BeginTabItem("Appearance")) {
                ImGui::Spacing();
                ImGui::Text("Desktop Shell Theme Manager");
                ImGui::TextDisabled("Select an operating system appearance theme:");
                ImGui::Separator();
                ImGui::Spacing();

                auto themeRadioButton = [&](const char* name, ThemeMode mode) {
                    if (ImGui::RadioButton(name, ctx.getTheme() == mode)) {
                        ctx.setTheme(mode);
                    }
                };

                themeRadioButton("Cyber Dark (Modern Slate & Cyan Glow)", ThemeMode::CYBER_DARK);
                themeRadioButton("Classic Windows 98 / 2000 (Retro Gray & Teal)", ThemeMode::CLASSIC_98);
                themeRadioButton("Retro CRT Amber (Monochrome Phosphor)", ThemeMode::RETRO_AMBER);
                themeRadioButton("Matrix Green (Hacker Cyberpunk)", ThemeMode::MATRIX_GREEN);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::Text("Wallpaper");
                int wallpaperSizing = static_cast<int>(ctx.getWallpaperSizing());
                ImGui::SetNextItemWidth(std::min(220.0f, ImGui::GetContentRegionAvail().x - 50.0f));
                if (ImGui::Combo("Sizing##wallpaper_sizing", &wallpaperSizing,
                                 "Fit\0Crop to fill\0Stretch to fill\0")) {
                    ctx.setWallpaperSizing(static_cast<WallpaperSizing>(wallpaperSizing));
                }
                const char* sizingDescriptions[] = {
                    "Show the whole image at its original proportions.",
                    "Fill the desktop by cropping the edges.",
                    "Fill the desktop by stretching the image."
                };
                ImGui::TextDisabled("%s", sizingDescriptions[wallpaperSizing]);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextWrapped("Performance & Rendering:");
                ImGui::TextWrapped("Current FPS : %.1f FPS (%.2f ms/frame)",
                    ImGui::GetIO().Framerate, ImGui::GetIO().Framerate > 0.0f ? 1000.0f / ImGui::GetIO().Framerate : 0.0f);

                static bool vsync = true;
                if (ImGui::Checkbox("Enable V-Sync (Vertical Synchronization)", &vsync)) {
                    glfwSwapInterval(vsync ? 1 : 0);
                }

                ImGui::EndTabItem();
            }

            // TAB 3: ABOUT CSOPESY
            if (ImGui::BeginTabItem("About")) {
                ImGui::Spacing();
                ImGui::Text("CSOPESY Desktop OS Mock-up Emulator");
                ImGui::TextWrapped("C++ Systems Programming & Graphics Final Project");
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::TextWrapped(
                    "This software simulates an operating system runtime environment, from the 90s BIOS / POST "
                    "terminal phase to an animated boot splash loader, into an interactive desktop compositor.\n\n"
                    "Architecture Stack:\n"
                    " - Language: C++17 / C++20\n"
                    " - Graphics: OpenGL 3.3 Core Profile + GLFW\n"
                    " - Extension Loader: GLAD\n"
                    " - GUI Compositor: Dear ImGui immediate-mode windows\n\n"
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
