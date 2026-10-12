#include "Core/SystemContext.h"
#include "Compositor/WindowCompositor.h"
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
int checks = 0;

void check(bool condition, const std::string& message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}

const GLubyte* APIENTRY testGlString(GLenum) {
    return reinterpret_cast<const GLubyte*>("Test context metadata");
}

void frame(OS::SystemContext& context, ImVec2 size) {
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = size;
    io.DeltaTime = 1.0f / 60.0f;
    ImGui::NewFrame();
    OS::WindowCompositor::render(context, io.DeltaTime);
    ImGui::Render();
}

ImGuiWindow* visibleWindow(const char* name) {
    ImGuiWindow* window = ImGui::FindWindowByName(name);
    check(window && window->Active, std::string("Missing visible window: ") + name);
    return window;
}

void checkAppBounds(const char* id, ImVec2 size) {
    ImGuiWindow* window = visibleWindow(id);
    const float tolerance = 1.0f;
    check(window->Pos.x >= -tolerance && window->Pos.y >= -tolerance,
          std::string("Window starts off-screen: ") + id);
    check(window->Pos.x + window->SizeFull.x <= size.x + tolerance,
          std::string("Window extends beyond right edge: ") + id);
    check(window->Pos.y + window->SizeFull.y <= size.y - 44.0f + tolerance,
          std::string("Window blocks taskbar: ") + id);
}

void click(OS::SystemContext& context, float x, float y, ImVec2 size) {
    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(x, y);
    frame(context, size);
    io.AddMouseButtonEvent(0, true);
    frame(context, size);
    io.AddMouseButtonEvent(0, false);
    frame(context, size);
    frame(context, size);
}

void activateModalButton(OS::SystemContext& context, const char* label, ImVec2 size) {
    ImGuiWindow* popup = visibleWindow("Unsaved edits##notepad_new");
    ImGui::ActivateItemByID(popup->GetID(label));
    frame(context, size);
    frame(context, size);
}

ImGuiID systemInfoTabId(const char* label) {
    ImGuiWindow* settings = visibleWindow("###SYSINFO_WND");
    const ImGuiID tabBarId = settings->GetID("SysInfoTabBar");
    ImGuiTabBar* tabBar = ImGui::GetCurrentContext()->TabBars.GetByKey(tabBarId);
    check(tabBar != nullptr, "System Info tab bar was not registered");

    const ImGuiID tabId = ImHashStr(label, 0, tabBarId);
    check(ImGui::TabBarFindTabByID(tabBar, tabId) != nullptr,
          std::string("Missing System Info tab: ") + label);
    return tabId;
}

void selectWallpaperSizing(OS::SystemContext& context, ImVec2 size,
                           int optionIndex, OS::WallpaperSizing expected,
                           const char* optionLabel) {
    context.setState(OS::BootState::DESKTOP);
    context.showTaskManager = context.showFileExplorer = context.showTerminal = false;
    context.showNotepad = false;
    context.showSystemInfo = true;
    frame(context, size);
    frame(context, size);

    checkAppBounds("###SYSINFO_WND", size);
    const ImGuiID appearanceTabId = systemInfoTabId("Appearance");
    ImGui::ActivateItemByID(appearanceTabId);
    frame(context, size);
    frame(context, size);

    ImGuiWindow* settings = visibleWindow("###SYSINFO_WND");
    const ImGuiID tabBarId = settings->GetID("SysInfoTabBar");
    ImGuiTabBar* tabBar = ImGui::GetCurrentContext()->TabBars.GetByKey(tabBarId);
    check(tabBar && tabBar->SelectedTabId == appearanceTabId,
          "Appearance tab could not be selected at the current viewport size");

    const ImGuiID comboId = ImHashStr("Sizing##wallpaper_sizing", 0, appearanceTabId);
    ImGui::ActivateItemByID(comboId);
    frame(context, size);
    frame(context, size);

    ImGuiWindow* popup = visibleWindow("##Combo_00");
    check(popup->Pos.x >= 0.0f && popup->Pos.y >= 0.0f &&
          popup->Pos.x + popup->SizeFull.x <= size.x + 1.0f &&
          popup->Pos.y + popup->SizeFull.y <= size.y + 1.0f,
          "Wallpaper sizing options are outside the viewport");

    const ImGuiID optionSeed = popup->GetID(optionIndex);
    const ImGuiID optionId = ImHashStr(optionLabel, 0, optionSeed);
    ImGui::ActivateItemByID(optionId);
    frame(context, size);
    frame(context, size);
    check(context.getWallpaperSizing() == expected,
          std::string("Selecting wallpaper sizing did not update context: ") + optionLabel);
}

void wallpaperSizingChecks(OS::SystemContext& context) {
    const ImVec2 size(640, 480);
    check(context.getWallpaperSizing() == OS::WallpaperSizing::FIT,
          "Wallpaper sizing should default to Fit");

    struct Selection {
        int index;
        OS::WallpaperSizing value;
        const char* label;
    };
    const Selection selections[] = {
        {1, OS::WallpaperSizing::CROP, "Crop to fill"},
        {0, OS::WallpaperSizing::FIT, "Fit"},
        {2, OS::WallpaperSizing::STRETCH, "Stretch to fill"}
    };

    for (const Selection& selection : selections) {
        selectWallpaperSizing(context, size, selection.index, selection.value, selection.label);
        context.reboot();
        check(context.getWallpaperSizing() == selection.value,
              std::string("Wallpaper sizing did not survive reboot: ") + selection.label);
    }

    // Leave the following editor checks in the desktop state, as before.
    context.setState(OS::BootState::DESKTOP);
    context.showSystemInfo = false;
    frame(context, size);
}

void viewportChecks(OS::SystemContext& context) {
    context.showTaskManager = context.showFileExplorer = context.showTerminal = true;
    context.showSystemInfo = context.showNotepad = true;
    const char* apps[] = {"###TASK_MANAGER_WND", "###FILE_EXPLORER_WND", "###TERMINAL_WND",
                          "###SYSINFO_WND", "###NOTEPAD_WND"};
    const ImVec2 sizes[] = {ImVec2(1920, 1080), ImVec2(1280, 800),
                           ImVec2(800, 600), ImVec2(640, 480)};
    for (ImVec2 size : sizes) {
        frame(context, size);
        frame(context, size);
        for (const char* app : apps) checkAppBounds(app, size);
        ImGuiWindow* desktop = visibleWindow("##DESKTOP_WALLPAPER_LAYER");
        check(desktop->Pos.x == 0 && desktop->Pos.y == 0 && desktop->Size.x == size.x && desktop->Size.y == size.y,
              "Wallpaper does not cover viewport");
        ImGuiWindow* taskbar = visibleWindow("##FIXED_TASKBAR_DOCK");
        check(taskbar->Pos.x == 0 && taskbar->Pos.y == size.y - 44 && taskbar->Size.x == size.x,
              "Taskbar is not anchored to bottom");
    }

    // Reproduce an old ini layout from a much larger display.
    for (const char* app : apps) {
        ImGui::SetWindowPos(app, ImVec2(2200, 1500));
        ImGui::SetWindowSize(app, ImVec2(1100, 900));
    }
    frame(context, ImVec2(640, 480));
    for (const char* app : apps) checkAppBounds(app, ImVec2(640, 480));
}

void interactionChecks(OS::SystemContext& context) {
    const ImVec2 size(640, 480);
    context.showTaskManager = context.showFileExplorer = context.showTerminal = false;
    context.showSystemInfo = context.showNotepad = false;
    frame(context, size);
    frame(context, size);

    // Click the actual compact taskbar, which must expose three distinct applications.
    click(context, 77, 458, size);
    check(context.showTaskManager, "Task Manager icon did not open its app");
    click(context, 125, 458, size);
    check(context.showFileExplorer, "Explorer icon did not open its app");
    click(context, 221, 458, size);
    check(context.showSystemInfo, "System Info icon did not open its app");

    click(context, 605, 458, size);
    check(context.showPowerDialog, "PWR did not open the power modal");
    ImGuiWindow* modal = visibleWindow("Power Options##shutdown_modal");
    check(modal->Pos.x >= 0 && modal->Pos.y >= 0 &&
          modal->Pos.x + modal->SizeFull.x <= size.x && modal->Pos.y + modal->SizeFull.y <= size.y,
          "Power modal extends outside viewport");
    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiKey_Escape, true);
    frame(context, size);
    io.AddKeyEvent(ImGuiKey_Escape, false);
    frame(context, size);
    check(!context.showPowerDialog && !context.shouldCloseApp(), "Cancel closed the application");

    // Terminal and Settings must share the same widget palette.
    context.executeTerminalCommand("theme amber");
    frame(context, size);
    const ImVec4 amber = ImGui::GetStyle().Colors[ImGuiCol_Text];
    context.executeTerminalCommand("theme green");
    frame(context, size);
    const ImVec4 green = ImGui::GetStyle().Colors[ImGuiCol_Text];
    check(amber.x > amber.y && green.y > green.x, "Terminal theme did not apply ImGui palette");

    wallpaperSizingChecks(context);

    context.newNotepadDocument();
    context.notepadBuffer = "Saved through the focused editor shortcut";
    context.notepadModified = true;
    frame(context, size);
    ImGui::SetWindowFocus("###NOTEPAD_WND");
    frame(context, size);
    io.AddKeyEvent(ImGuiMod_Ctrl, true);
    io.AddKeyEvent(ImGuiKey_S, true);
    frame(context, size);
    io.AddKeyEvent(ImGuiKey_S, false);
    io.AddKeyEvent(ImGuiMod_Ctrl, false);
    frame(context, size);
    check(!context.notepadModified && !context.getNotepadFilePath().empty(), "Focused Ctrl+S did not save");
    const OS::VirtualFile* saved = context.findFileByPath(context.getNotepadFilePath());
    check(saved && saved->content == context.notepadBuffer, "Ctrl+S did not update the virtual file");

    const std::string sourcePath = context.getNotepadFilePath();
    const std::string targetPath = "C:\\CSOPESY\\welcome.txt";
    context.notepadBuffer = "Dirty buffer before external open";
    context.notepadModified = true;
    context.openFileInNotepad(context.findFileByPath(targetPath));
    frame(context, size);
    frame(context, size);
    visibleWindow("Unsaved edits##notepad_new");
    check(context.getNotepadFilePath() == sourcePath && context.notepadModified,
          "External open discarded edits before a decision");
    io.AddKeyEvent(ImGuiKey_Escape, true);
    frame(context, size);
    io.AddKeyEvent(ImGuiKey_Escape, false);
    frame(context, size);
    check(!context.hasPendingNotepadOpen() && context.notepadModified &&
          context.notepadBuffer == "Dirty buffer before external open", "Escape did not cancel external open");

    context.openFileInNotepad(context.findFileByPath(targetPath));
    frame(context, size);
    frame(context, size);
    activateModalButton(context, "Save & open", size);
    check(context.getNotepadFilePath() == targetPath && !context.notepadModified &&
          !context.hasPendingNotepadOpen(), "Save & open did not finish the requested open");
    check(context.findFileByPath(sourcePath)->content == "Dirty buffer before external open",
          "Save & open lost source changes");
    context.notepadBuffer = "Discard this buffer";
    context.notepadModified = true;
    context.openFileInNotepad(context.findFileByPath(sourcePath));
    frame(context, size);
    frame(context, size);
    activateModalButton(context, "Discard", size);
    check(context.getNotepadFilePath() == sourcePath && !context.notepadModified &&
          context.notepadBuffer == "Dirty buffer before external open", "Discard did not open the requested source");
    check(context.findFileByPath(targetPath)->content != "Discard this buffer", "Discard saved changes unexpectedly");
}
} // namespace

int main() {
    try {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        unsigned char* pixels = nullptr;
        int width = 0, height = 0;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        // These CPU-side ImGui tests do not claim to validate a real GPU context.
        glad_glGetString = testGlString;
        OS::SystemContext context;
        context.setState(OS::BootState::DESKTOP);
        viewportChecks(context);
        interactionChecks(context);
        ImGui::DestroyContext();
        std::cout << checks << " UI interaction/bounds checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "UI regression failure: " << error.what() << '\n';
        if (ImGui::GetCurrentContext()) ImGui::DestroyContext();
        return 1;
    }
}
