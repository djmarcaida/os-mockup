#include "NotepadWindow.h"
#include "../Desktop/UiHelpers.h"
#include <imgui.h>
#include <algorithm>
#include <cstring>
#include <vector>

namespace OS {

namespace {
int resizeEditor(ImGuiInputTextCallbackData* data) {
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        auto& buffer = *static_cast<std::vector<char>*>(data->UserData);
        buffer.resize(static_cast<size_t>(data->BufSize));
        data->Buf = buffer.data();
    }
    return 0;
}
}

void NotepadWindow::render(SystemContext& ctx, float deltaTime) {
    if (!ctx.showNotepad) return;
    static std::vector<char> editor;
    static std::string lastModelText;
    static std::string lastDocument;
    static unsigned int editorRevision = 0;
    static std::string status;
    static float statusTimer = 0.0f;
    static bool pendingNew = false;
    static bool unsavedDialogOpen = false;
    static std::string decisionError;
    const char* unsavedPopup = "Unsaved edits##notepad_new";
    if (statusTimer > 0.0f) {
        statusTimer -= deltaTime;
        if (statusTimer <= 0.0f) status.clear();
    }
    const auto save = [&]() {
        const bool result = ctx.saveNotepadDocument(status);
        statusTimer = 6.0f;
        lastDocument = ctx.getNotepadFilePath() + ctx.notepadTitle;
        return result;
    };
    const auto newDocument = [&]() {
        ctx.newNotepadDocument();
        status.clear();
        statusTimer = 0.0f;
    };
    const std::string windowTitle = (ctx.notepadModified ? "* " : "") + ctx.notepadTitle + "###NOTEPAD_WND";
    Ui::prepareAppWindow(windowTitle.c_str(), ImVec2(540, 400), ImVec2(260, 180), ImVec2(400, 300));

    if (ImGui::Begin(windowTitle.c_str(), &ctx.showNotepad, ImGuiWindowFlags_MenuBar |
                     (ctx.notepadModified ? ImGuiWindowFlags_UnsavedDocument : 0))) {
        bool requestNew = false;
        bool requestSave = false;
        bool requestAbout = false;
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ctx.showPowerDialog) {
            const ImGuiIO& io = ImGui::GetIO();
            requestNew = io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_N, false);
            requestSave = io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false);
        }
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                requestNew |= ImGui::MenuItem("New", "Ctrl+N");
                requestSave |= ImGui::MenuItem("Save to virtual disk", "Ctrl+S");
                ImGui::Separator();
                if (ImGui::MenuItem("Close")) ctx.showNotepad = false;
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help")) {
                requestAbout = ImGui::MenuItem("About Notepad");
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        if (requestNew) {
            ctx.cancelNotepadOpen();
            if (ctx.notepadModified) pendingNew = true;
            else newDocument();
        }
        if (requestSave) save();
        if (requestAbout) ImGui::OpenPopup("About Notepad##notepad_about");
        if (ctx.hasPendingNotepadOpen()) pendingNew = false;
        if ((pendingNew || ctx.hasPendingNotepadOpen()) && !ImGui::IsPopupOpen(unsavedPopup)) {
            unsavedDialogOpen = true;
            decisionError.clear();
            ImGui::OpenPopup(unsavedPopup);
        }

        // Keep storage alive across frames. Resize only when a document grows or changes.
        const std::string document = ctx.getNotepadFilePath() + ctx.notepadTitle;
        if (editor.empty() || lastModelText != ctx.notepadBuffer || lastDocument != document) {
            editor.resize(std::max(size_t(8192), ctx.notepadBuffer.size() + 1));
            std::memcpy(editor.data(), ctx.notepadBuffer.c_str(), ctx.notepadBuffer.size() + 1);
            lastModelText = ctx.notepadBuffer;
            if (lastDocument != document) status.clear();
            lastDocument = document;
            ++editorRevision;
        }
        const char* message = status.empty() ? "Save keeps this document in the virtual disk for this session." : status.c_str();
        const float footerHeight = ImGui::GetTextLineHeightWithSpacing() +
            ImGui::CalcTextSize(message, nullptr, false, ImGui::GetContentRegionAvail().x).y + 10.0f;
        ImGui::PushID(static_cast<int>(editorRevision));
        if (ImGui::InputTextMultiline("##notepad_text", editor.data(), editor.size(),
                                     ImVec2(-1.0f, std::max(80.0f, ImGui::GetContentRegionAvail().y - footerHeight)),
                                     ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackResize,
                                     resizeEditor, &editor)) {
            ctx.notepadBuffer.assign(editor.data());
            lastModelText = ctx.notepadBuffer;
            ctx.notepadModified = true;
            status.clear();
        }
        ImGui::PopID();
        message = status.empty() ? "Save keeps this document in the virtual disk for this session." : status.c_str();
        ImGui::Text("%s | %zu characters | Virtual session", ctx.notepadModified ? "Modified" : "Saved / unchanged", ctx.notepadBuffer.size());
        if (ImGui::IsItemHovered()) {
            const std::string filePath = ctx.getNotepadFilePath();
            ImGui::SetTooltip("%s", filePath.empty() ? "New documents save into the current Explorer directory." : filePath.c_str());
        }
        ImGui::TextWrapped("%s", message);

        ImGui::SetNextWindowSize(ImVec2(440.0f, 230.0f), ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal(unsavedPopup, &unsavedDialogOpen, ImGuiWindowFlags_NoResize)) {
            const bool openingFile = ctx.hasPendingNotepadOpen();
            ImGui::TextWrapped(openingFile ? "Save your edits before opening another file?"
                                          : "Save your edits before creating a new document?");
            if (openingFile) ImGui::TextWrapped("Open: %s", ctx.getPendingNotepadFilePath().c_str());
            ImGui::Spacing();
            ImGui::TextWrapped("Saving updates the session-only virtual disk.");
            if (!decisionError.empty()) ImGui::TextWrapped("%s", decisionError.c_str());
            const float width = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2.0f) / 3.0f;
            const auto finishDecision = [&]() {
                if (openingFile) {
                    if (!ctx.confirmNotepadOpen()) {
                        status = "The requested file is no longer available.";
                        statusTimer = 6.0f;
                        ctx.cancelNotepadOpen();
                    }
                } else newDocument();
                pendingNew = false;
                unsavedDialogOpen = false;
                ImGui::CloseCurrentPopup();
            };
            if (ImGui::Button(openingFile ? "Save & open" : "Save & new", ImVec2(width, 30.0f))) {
                if (save()) finishDecision();
                else decisionError = status;
            }
            ImGui::SameLine();
            if (ImGui::Button("Discard", ImVec2(width, 30.0f))) finishDecision();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(width, 30.0f)) || ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
                ctx.cancelNotepadOpen();
                pendingNew = false;
                unsavedDialogOpen = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::EndPopup();
        }
        if (!unsavedDialogOpen || !ImGui::IsPopupOpen(unsavedPopup)) {
            ctx.cancelNotepadOpen();
            pendingNew = false;
            unsavedDialogOpen = false;
        }
        ImGui::SetNextWindowSize(ImVec2(390.0f, 205.0f), ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal("About Notepad##notepad_about", nullptr, ImGuiWindowFlags_NoResize)) {
            ImGui::Text("CSOPESY Notepad");
            ImGui::Separator();
            ImGui::TextWrapped("Edit text files from the virtual disk. Ctrl+N creates a document; Ctrl+S saves it. New documents become Untitled-N.txt in Explorer's current directory.");
            ImGui::TextWrapped("Virtual files stay in memory only for this session.");
            if (ImGui::Button("Close", ImVec2(100.0f, 30.0f)) || ImGui::IsKeyPressed(ImGuiKey_Escape, false))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

} // namespace OS
