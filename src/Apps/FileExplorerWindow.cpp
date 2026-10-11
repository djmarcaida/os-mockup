#include "FileExplorerWindow.h"
#include <imgui.h>
#include <iomanip>
#include <sstream>

namespace OS {

const VirtualFile* FileExplorerWindow::s_selectedFile = nullptr;

void FileExplorerWindow::render(SystemContext& ctx, float deltaTime) {
    (void)deltaTime;
    if (!ctx.showFileExplorer) return;

    ImGui::SetNextWindowSize(ImVec2(660, 440), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(240, 140), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("File Explorer - Virtual Disk (C:)###FILE_EXPLORER_WND", &ctx.showFileExplorer)) {
        // Navigation toolbar
        if (ImGui::Button(" [Up] ", ImVec2(50, 0))) {
            ctx.navigateUp();
            s_selectedFile = nullptr;
        }
        ImGui::SameLine();
        ImGui::Text("Address:");
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.14f, 0.22f, 1.0f));
        ImGui::PushItemWidth(-1.0f);
        char pathBuf[256];
        snprintf(pathBuf, sizeof(pathBuf), "%s", ctx.getCurrentDirPath().c_str());
        ImGui::InputText("##address_bar", pathBuf, sizeof(pathBuf), ImGuiInputTextFlags_ReadOnly);
        ImGui::PopItemWidth();
        ImGui::PopStyleColor();

        ImGui::Separator();

        // Split view: Left Sidebar (Tree) vs Right Pane (Files)
        float sidebarWidth = 180.0f;

        ImGui::BeginChild("ExplorerSidebar", ImVec2(sidebarWidth, -ImGui::GetFrameHeightWithSpacing() - 6.0f), true);
        ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "Virtual Drives");
        ImGui::Separator();
        renderTreeView(ctx, ctx.getRootFileSystem());
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("ExplorerFileList", ImVec2(0.0f, -ImGui::GetFrameHeightWithSpacing() - 6.0f), true);
        renderFileList(ctx);
        ImGui::EndChild();

        // Status bar footer
        ImGui::Separator();
        const auto* curDir = ctx.getCurrentDirectory();
        size_t count = curDir ? curDir->children.size() : 0;
        ImGui::Text("%zu item(s) in current directory  |  Virtual Disk (C:) 40.0 GB Total", count);

        if (s_selectedFile && !s_selectedFile->isDirectory) {
            ImGui::SameLine(ImGui::GetWindowWidth() - 180.0f);
            if (ImGui::Button("Open in Notepad", ImVec2(160, 0))) {
                ctx.openFileInNotepad(s_selectedFile->name, s_selectedFile->content);
            }
        }
    }
    ImGui::End();
}

void FileExplorerWindow::renderTreeView(SystemContext& ctx, const VirtualFile* dir) {
    if (!dir) return;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (dir->name == "C:") {
        flags |= ImGuiTreeNodeFlags_DefaultOpen;
    }

    bool nodeOpen = ImGui::TreeNodeEx(dir->name.c_str(), flags);

    if (ImGui::IsItemClicked()) {
        if (dir->name == "C:") {
            while (ctx.navigateUp());
            s_selectedFile = nullptr;
        }
    }

    if (nodeOpen) {
        for (const auto& child : dir->children) {
            if (child.isDirectory) {
                renderTreeView(ctx, &child);
            }
        }
        ImGui::TreePop();
    }
}

void FileExplorerWindow::renderFileList(SystemContext& ctx) {
    const auto* curDir = ctx.getCurrentDirectory();
    if (!curDir) return;

    ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | 
                                ImGuiTableFlags_RowBg | 
                                ImGuiTableFlags_Resizable | 
                                ImGuiTableFlags_ScrollY;

    if (ImGui::BeginTable("FilesTable", 4, tableFlags)) {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 90.0f);
        ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Modified", ImGuiTableColumnFlags_WidthFixed, 90.0f);
        ImGui::TableHeadersRow();

        for (const auto& item : curDir->children) {
            ImGui::TableNextRow();

            bool isSelected = (s_selectedFile == &item);

            // Column 0: Name
            ImGui::TableSetColumnIndex(0);
            std::string iconPrefix = item.isDirectory ? "[DIR]  " : "[FILE] ";
            std::string displayName = iconPrefix + item.name;

            if (ImGui::Selectable(displayName.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
                s_selectedFile = &item;
            }

            // Double click handler
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                if (item.isDirectory) {
                    ctx.navigateTo(item.name);
                    s_selectedFile = nullptr;
                    ImGui::EndTable();
                    return;
                } else {
                    ctx.openFileInNotepad(item.name, item.content);
                }
            }

            // Column 1: Type
            ImGui::TableSetColumnIndex(1);
            if (item.isDirectory) {
                ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "Folder");
            } else {
                std::string ext = "File";
                auto dotPos = item.name.find_last_of('.');
                if (dotPos != std::string::npos) ext = item.name.substr(dotPos + 1) + " file";
                ImGui::Text("%s", ext.c_str());
            }

            // Column 2: Size
            ImGui::TableSetColumnIndex(2);
            if (item.isDirectory) {
                ImGui::TextDisabled("-");
            } else {
                if (item.sizeBytes >= 1024) {
                    ImGui::Text("%.1f KB", item.sizeBytes / 1024.0f);
                } else {
                    ImGui::Text("%zu B", item.sizeBytes);
                }
            }

            // Column 3: Date
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", item.modifiedDate.c_str());
        }

        ImGui::EndTable();
    }
}

} // namespace OS
