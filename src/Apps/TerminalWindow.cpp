#include "TerminalWindow.h"
#include "../Desktop/UiHelpers.h"
#include <imgui.h>
#include <cstring>

namespace OS {

char TerminalWindow::s_inputBuffer[256] = "";
bool TerminalWindow::s_scrollToBottom = true;
int TerminalWindow::s_historyIndex = -1;
std::vector<std::string> TerminalWindow::s_localHistory;

void TerminalWindow::render(SystemContext& ctx, float deltaTime) {
    (void)deltaTime;
    if (!ctx.showTerminal) return;

    Ui::prepareAppWindow("CSOPESY Command Prompt (csopesy_sh)###TERMINAL_WND", ImVec2(640, 420),
                         ImVec2(200, 160), ImVec2(460, 300));

    if (ImGui::Begin("CSOPESY Command Prompt (csopesy_sh)###TERMINAL_WND", &ctx.showTerminal)) {
        // Top help hint
        ImGui::TextWrapped("Simulated CSOPESY Shell | Type 'help' for commands.");
        ImGui::Separator();

        // Console text output region
        const float footerHeightToReserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing() + 4.0f;
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f, 0.05f, 0.07f, 1.0f));
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footerHeightToReserve), false, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::PopStyleColor();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 2));

        const auto& lines = ctx.getTerminalLines();
        for (const auto& line : lines) {
            // Unpack 0xAABBGGRR color
            float r = ((line.color >> 0) & 0xFF) / 255.0f;
            float g = ((line.color >> 8) & 0xFF) / 255.0f;
            float b = ((line.color >> 16) & 0xFF) / 255.0f;
            float a = ((line.color >> 24) & 0xFF) / 255.0f;
            if (a == 0.0f) a = 1.0f;

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(r, g, b, a));
            ImGui::TextUnformatted(line.text.c_str());
            ImGui::PopStyleColor();
        }

        if (s_scrollToBottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
        s_scrollToBottom = false;

        ImGui::PopStyleVar();
        ImGui::EndChild();

        ImGui::Separator();

        // Command line input prompt
        ImGui::TextUnformatted("admin@csopesy:~$");
        ImGui::SameLine();

        ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue | 
                                         ImGuiInputTextFlags_CallbackHistory;

        auto callback = [](ImGuiInputTextCallbackData* data) -> int {
            if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory) {
                if (s_localHistory.empty()) return 0;

                if (data->EventKey == ImGuiKey_UpArrow) {
                    if (s_historyIndex == -1) {
                        s_historyIndex = static_cast<int>(s_localHistory.size()) - 1;
                    } else if (s_historyIndex > 0) {
                        s_historyIndex--;
                    }
                } else if (data->EventKey == ImGuiKey_DownArrow) {
                    if (s_historyIndex != -1) {
                        if (++s_historyIndex >= static_cast<int>(s_localHistory.size())) {
                            s_historyIndex = -1;
                        }
                    }
                }

                if (s_historyIndex >= 0 && s_historyIndex < static_cast<int>(s_localHistory.size())) {
                    data->DeleteChars(0, data->BufTextLen);
                    data->InsertChars(0, s_localHistory[s_historyIndex].c_str());
                } else {
                    data->DeleteChars(0, data->BufTextLen);
                }
            }
            return 0;
        };

        ImGui::PushItemWidth(-78.0f);
        bool reclaimFocus = false;
        if (ImGui::InputText("##CommandLineInput", s_inputBuffer, sizeof(s_inputBuffer), inputFlags, callback)) {
            std::string cmd = s_inputBuffer;
            if (!cmd.empty()) {
                s_localHistory.push_back(cmd);
                s_historyIndex = -1;
                ctx.executeTerminalCommand(cmd);
                s_inputBuffer[0] = '\0';
                s_scrollToBottom = true;
            }
            reclaimFocus = true;
        }
        ImGui::PopItemWidth();

        // Keep auto-focus on text input
        ImGui::SetItemDefaultFocus();
        if (reclaimFocus) {
            ImGui::SetKeyboardFocusHere(-1);
        }

        ImGui::SameLine();
        if (ImGui::Button("Execute", ImVec2(70, 0))) {
            std::string cmd = s_inputBuffer;
            if (!cmd.empty()) {
                s_localHistory.push_back(cmd);
                s_historyIndex = -1;
                ctx.executeTerminalCommand(cmd);
                s_inputBuffer[0] = '\0';
                s_scrollToBottom = true;
            }
        }
    }
    ImGui::End();

}

} // namespace OS
