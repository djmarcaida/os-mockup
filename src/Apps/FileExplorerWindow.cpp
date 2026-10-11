#include "FileExplorerWindow.h"
#include "../Desktop/UiHelpers.h"
#include <imgui.h>
#include <cstdio>

namespace OS {

std::string FileExplorerWindow::s_selectedName;
std::string FileExplorerWindow::s_selectionDirectory;

void FileExplorerWindow::render(SystemContext& ctx, float deltaTime) {
    (void)deltaTime;
    if (!ctx.showFileExplorer) return;
    if (s_selectionDirectory != ctx.getCurrentDirPath()) s_selectedName.clear();
    s_selectionDirectory = ctx.getCurrentDirPath();
    Ui::prepareAppWindow("File Explorer - Virtual Disk (C:)###FILE_EXPLORER_WND",
                         ImVec2(660, 440), ImVec2(240, 140), ImVec2(520, 320));

    if (ImGui::Begin("File Explorer - Virtual Disk (C:)###FILE_EXPLORER_WND", &ctx.showFileExplorer)) {
        if (ImGui::Button("Up", ImVec2(44, 0))) {
            ctx.navigateUp();
            s_selectedName.clear();
        }
        ImGui::SameLine();
        char pathBuffer[512];
        std::snprintf(pathBuffer, sizeof(pathBuffer), "%s", ctx.getCurrentDirPath().c_str());
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputText("##address_bar", pathBuffer, sizeof(pathBuffer), ImGuiInputTextFlags_ReadOnly);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", ctx.getCurrentDirPath().c_str());
        ImGui::Separator();

        const float sidebarWidth = std::clamp(ImGui::GetContentRegionAvail().x * .28f, 132.0f, 180.0f);
        const float footerHeight = ImGui::GetFrameHeightWithSpacing() + ImGui::GetTextLineHeightWithSpacing() + 10.0f;
        ImGui::BeginChild("ExplorerSidebar", ImVec2(sidebarWidth, -footerHeight), ImGuiChildFlags_Borders);
        ImGui::Text("Virtual drives");
        ImGui::Separator();
        renderTreeView(ctx, ctx.getRootFileSystem());
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("ExplorerFileList", ImVec2(0.0f, -footerHeight), ImGuiChildFlags_Borders);
        renderFileList(ctx);
        ImGui::EndChild();

        ImGui::Separator();
        const VirtualFile* directory = ctx.getCurrentDirectory();
        ImGui::Text("%zu item(s) | Session-only virtual disk", directory ? directory->children.size() : 0);
        const VirtualFile* selected = nullptr;
        if (directory && s_selectionDirectory == ctx.getCurrentDirPath()) {
            for (const VirtualFile& item : directory->children)
                if (item.name == s_selectedName) { selected = &item; break; }
        }
        if (selected && !selected->isDirectory) {
            if (ImGui::Button("Open in Notepad")) ctx.openFileInNotepad(selected);
            ImGui::SameLine();
            ImGui::TextUnformatted(selected->name.c_str());
        } else {
            ImGui::TextDisabled("Double-click a folder or text file to open it.");
        }
    }
    ImGui::End();
}

void FileExplorerWindow::renderTreeView(SystemContext& ctx, const VirtualFile* directory) {
    if (!directory) return;
    ImGui::PushID(directory);
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (directory == ctx.getRootFileSystem()) flags |= ImGuiTreeNodeFlags_DefaultOpen;
    if (directory == ctx.getCurrentDirectory()) flags |= ImGuiTreeNodeFlags_Selected;
    const bool open = ImGui::TreeNodeEx(directory->name.c_str(), flags);
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        if (ctx.navigateToDirectory(directory)) {
            s_selectedName.clear();
            s_selectionDirectory = ctx.getCurrentDirPath();
        }
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Open %s", directory->name.c_str());
    if (open) {
        for (const VirtualFile& child : directory->children)
            if (child.isDirectory) renderTreeView(ctx, &child);
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void FileExplorerWindow::renderFileList(SystemContext& ctx) {
    const VirtualFile* directory = ctx.getCurrentDirectory();
    if (!directory) return;
    const ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX;
    if (ImGui::BeginTable("FilesTable", 4, flags)) {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 180.0f);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 82.0f);
        ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 68.0f);
        ImGui::TableSetupColumn("Modified", ImGuiTableColumnFlags_WidthFixed, 96.0f);
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();
        for (const VirtualFile& item : directory->children) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::PushID(item.name.c_str());
            const bool selected = s_selectionDirectory == ctx.getCurrentDirPath() && s_selectedName == item.name;
            if (ImGui::Selectable(item.name.c_str(), selected, ImGuiSelectableFlags_SpanAllColumns)) {
                s_selectedName = item.name;
                s_selectionDirectory = ctx.getCurrentDirPath();
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s%s", item.isDirectory ? "Folder: " : "File: ", item.name.c_str());
            const bool open = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
            ImGui::PopID();
            if (open) {
                if (item.isDirectory) {
                    ctx.navigateToDirectory(&item);
                    s_selectedName.clear();
                    s_selectionDirectory = ctx.getCurrentDirPath();
                    ImGui::EndTable();
                    return;
                }
                ctx.openFileInNotepad(&item);
            }
            ImGui::TableSetColumnIndex(1);
            if (item.isDirectory) ImGui::TextUnformatted("Folder");
            else {
                const size_t dot = item.name.find_last_of('.');
                const std::string type = dot == std::string::npos ? "File" : item.name.substr(dot + 1) + " file";
                ImGui::TextUnformatted(type.c_str());
            }
            ImGui::TableSetColumnIndex(2);
            if (item.isDirectory) ImGui::TextDisabled("-");
            else if (item.sizeBytes >= 1024) ImGui::Text("%.1f KB", static_cast<double>(item.sizeBytes) / 1024.0);
            else ImGui::Text("%zu B", item.sizeBytes);
            ImGui::TableSetColumnIndex(3);
            ImGui::TextUnformatted(item.modifiedDate.c_str());
        }
        ImGui::EndTable();
    }
}

} // namespace OS
