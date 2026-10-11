#include "NotepadWindow.h"
#include <imgui.h>
#include <vector>

namespace OS {

void NotepadWindow::render(SystemContext& ctx, float deltaTime) {
    (void)deltaTime;
    if (!ctx.showNotepad) return;

    ImGui::SetNextWindowSize(ImVec2(540, 400), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(260, 180), ImGuiCond_FirstUseEver);

    std::string wndTitle = ctx.notepadTitle + "###NOTEPAD_WND";

    if (ImGui::Begin(wndTitle.c_str(), &ctx.showNotepad, ImGuiWindowFlags_MenuBar)) {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New", "Ctrl+N")) {
                    ctx.notepadTitle = "Untitled - Notepad";
                    ctx.notepadBuffer.clear();
                }
                if (ImGui::MenuItem("Save", "Ctrl+S")) {
                    // Saved in virtual memory
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) {
                    ctx.showNotepad = false;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About Notepad")) {
                    //
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // Multiline text edit
        std::vector<char> buffer(ctx.notepadBuffer.begin(), ctx.notepadBuffer.end());
        buffer.resize(std::max(buffer.size() + 1024, size_t(8192)));

        if (ImGui::InputTextMultiline("##notepad_text", buffer.data(), buffer.size(), 
            ImVec2(-1.0f, -1.0f), ImGuiInputTextFlags_AllowTabInput)) {
            ctx.notepadBuffer = buffer.data();
            ctx.notepadModified = true;
        }
    }
    ImGui::End();
}

} // namespace OS
