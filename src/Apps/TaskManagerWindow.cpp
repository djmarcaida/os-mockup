#include "TaskManagerWindow.h"
#include "../Desktop/UiHelpers.h"
#include <imgui.h>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace OS {

namespace {
std::string foldCase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}
}

int TaskManagerWindow::s_selectedPid = -1;
char TaskManagerWindow::s_searchBuffer[64] = "";
std::string TaskManagerWindow::s_statusMessage = "";
float TaskManagerWindow::s_statusTimer = 0.0f;

void TaskManagerWindow::render(SystemContext& ctx, float deltaTime) {
    if (!ctx.showTaskManager) return;

    if (s_statusTimer > 0.0f) {
        s_statusTimer -= deltaTime;
        if (s_statusTimer <= 0.0f) {
            s_statusMessage.clear();
        }
    }

    Ui::prepareAppWindow("Task Manager (CSOPESY Workstation)###TASK_MANAGER_WND", ImVec2(680, 520),
                         ImVec2(180, 100), ImVec2(560, 350));

    if (ImGui::Begin("Task Manager (CSOPESY Workstation)###TASK_MANAGER_WND", &ctx.showTaskManager)) {
        if (ImGui::BeginTabBar("TaskManagerTabBar")) {
            // TAB 1: PROCESSES
            if (ImGui::BeginTabItem("Processes")) {
                // Top control bar
                ImGui::PushItemWidth(220.0f);
                ImGui::InputTextWithHint("##proc_filter", "Filter process name...", s_searchBuffer, sizeof(s_searchBuffer));
                ImGui::PopItemWidth();

                ImGui::SameLine();

                bool canEnd = (s_selectedPid >= 0);
                if (!canEnd) ImGui::BeginDisabled();
                if (ImGui::Button("End Task", ImVec2(90.0f, 0.0f))) {
                    if (s_selectedPid >= 0) {
                        ctx.killProcess(s_selectedPid, s_statusMessage);
                        s_statusTimer = 4.0f;
                        s_selectedPid = -1;
                    }
                }
                if (!canEnd) ImGui::EndDisabled();

                ImGui::SameLine();
                if (ImGui::Button("New Task...", ImVec2(90.0f, 0.0f))) {
                    ImGui::OpenPopup("CreateNewTaskPopup");
                }

                // Popup for launching a new process
                if (ImGui::BeginPopup("CreateNewTaskPopup")) {
                    static char newProcName[64] = "notepad.exe";
                    ImGui::Text("Enter simulated process name:");
                    ImGui::InputText("##new_proc_name", newProcName, sizeof(newProcName));
                    if (ImGui::Button("Run", ImVec2(80, 0))) {
                        ctx.spawnProcess(newProcName, 2.5f, 24.0f);
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(80, 0))) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                // Status banner if any
                if (!s_statusMessage.empty()) {
                    ImGui::Spacing();
                    ImGui::TextWrapped("%s", s_statusMessage.c_str());
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Process Table
                const auto& processes = ctx.getProcesses();
                const std::string filter = foldCase(s_searchBuffer);
                std::vector<const ProcessInfo*> rows;
                for (const ProcessInfo& process : processes)
                    if (filter.empty() || foldCase(process.name).find(filter) != std::string::npos)
                        rows.push_back(&process);

                ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | 
                                            ImGuiTableFlags_RowBg | 
                                            ImGuiTableFlags_Resizable | 
                                            ImGuiTableFlags_ScrollY | 
                                            ImGuiTableFlags_Sortable |
                                            ImGuiTableFlags_ScrollX;

                if (ImGui::BeginTable("ProcessTable", 6, tableFlags, ImVec2(0.0f, -ImGui::GetFrameHeightWithSpacing() - 6.0f))) {
                    ImGui::TableSetupColumn("Process Name", ImGuiTableColumnFlags_WidthFixed, 210.0f);
                    ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                    ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    ImGui::TableSetupColumn("CPU %", ImGuiTableColumnFlags_WidthFixed, 75.0f);
                    ImGui::TableSetupColumn("Memory (MB)", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                    ImGui::TableSetupColumn("Disk (KB/s)", ImGuiTableColumnFlags_WidthFixed, 85.0f);
                    ImGui::TableSetupScrollFreeze(0, 1);
                    ImGui::TableHeadersRow();

                    // Sort a pointer view so terminal ps and backend order remain stable.
                    if (ImGuiTableSortSpecs* specs = ImGui::TableGetSortSpecs()) {
                        std::stable_sort(rows.begin(), rows.end(), [specs](const ProcessInfo* first, const ProcessInfo* second) {
                            for (int index = 0; index < specs->SpecsCount; ++index) {
                                const ImGuiTableColumnSortSpecs& spec = specs->Specs[index];
                                int order = 0;
                                const auto compare = [](const auto& a, const auto& b) { return a < b ? -1 : a > b ? 1 : 0; };
                                switch (spec.ColumnIndex) {
                                case 0: order = compare(foldCase(first->name), foldCase(second->name)); break;
                                case 1: order = compare(first->pid, second->pid); break;
                                case 2: order = compare(foldCase(first->status), foldCase(second->status)); break;
                                case 3: order = compare(first->cpuUsage, second->cpuUsage); break;
                                case 4: order = compare(first->memoryMB, second->memoryMB); break;
                                case 5: order = compare(first->diskIOKB, second->diskIOKB); break;
                                default: break;
                                }
                                if (order != 0) return spec.SortDirection == ImGuiSortDirection_Ascending ? order < 0 : order > 0;
                            }
                            return first->pid < second->pid;
                        });
                        specs->SpecsDirty = false;
                    }

                    for (const ProcessInfo* row : rows) {
                        const ProcessInfo& proc = *row;

                        ImGui::TableNextRow();

                        bool isSelected = (s_selectedPid == proc.pid);

                        // Column 0: Name (Selectable across entire row)
                        ImGui::TableSetColumnIndex(0);
                        char label[128];
                        snprintf(label, sizeof(label), "%s##pid_%d", proc.name.c_str(), proc.pid);
                        if (ImGui::Selectable(label, isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap)) {
                            s_selectedPid = proc.pid;
                        }

                        // Column 1: PID
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%d", proc.pid);

                        // Column 2: Status
                        ImGui::TableSetColumnIndex(2);
                        if (proc.status == "Running") {
                            ImGui::TextUnformatted(proc.status.c_str());
                        } else {
                            ImGui::TextDisabled("%s", proc.status.c_str());
                        }

                        // Column 3: CPU %
                        ImGui::TableSetColumnIndex(3);
                        if (proc.cpuUsage > 15.0f && proc.name != "idle_process") {
                            const ImVec4 warning = ctx.getTheme() == ThemeMode::CLASSIC_98
                                ? ImVec4(.55f, .15f, .03f, 1.0f) : ImVec4(1.0f, .6f, .2f, 1.0f);
                            ImGui::TextColored(warning, "%.1f %%", proc.cpuUsage);
                        } else {
                            ImGui::Text("%.1f %%", proc.cpuUsage);
                        }

                        // Column 4: Memory (MB)
                        ImGui::TableSetColumnIndex(4);
                        if (proc.memoryMB > 100.0f) {
                            ImGui::Text("%.1f MB", proc.memoryMB);
                        } else {
                            ImGui::Text("%.1f MB", proc.memoryMB);
                        }

                        // Column 5: Disk I/O
                        ImGui::TableSetColumnIndex(5);
                        ImGui::Text("%.1f", proc.diskIOKB);
                    }

                    ImGui::EndTable();
                }

                // Summary footer
                ImGui::Separator();
                ImGui::Text("Simulated: %zu processes | CPU %.1f%% | RAM %d / %d MB",
                    processes.size(), 
                    ctx.getTotalCpuUsage(), 
                    static_cast<int>(ctx.getTotalMemoryMB()), 
                    static_cast<int>(ctx.getMaxMemoryMB()));

                ImGui::EndTabItem();
            }

            // TAB 2: PERFORMANCE GRAPHS
            if (ImGui::BeginTabItem("Performance")) {
                ImGui::Spacing();

                // CPU Section
                ImGui::Text("Simulated CPU Utilization (60 Seconds)");
                ImGui::TextWrapped("Simulated processor: %s", ctx.cpuModel.c_str());
                ImGui::Text("Load: %.1f%% | Simulated tasks: %zu | Samples: 1/sec", ctx.getTotalCpuUsage(), ctx.getProcesses().size());

                ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.2f, 0.85f, 0.4f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.05f, 0.08f, 0.12f, 1.0f));
                ImGui::PlotLines("##CPU_Graph", ctx.getCpuHistory(), ctx.getHistoryCapacity(), 
                    0, nullptr, 0.0f, 100.0f, ImVec2(-1.0f, 110.0f));
                ImGui::PopStyleColor(2);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Memory Section
                ImGui::Text("Simulated Memory Usage History");
                ImGui::TextWrapped("Used: %d MB  |  Available: %d MB  |  Total Capacity: %d MB (%.1f%%)",
                    static_cast<int>(ctx.getTotalMemoryMB()),
                    static_cast<int>(ctx.getMaxMemoryMB() - ctx.getTotalMemoryMB()),
                    static_cast<int>(ctx.getMaxMemoryMB()),
                    (ctx.getTotalMemoryMB() / ctx.getMaxMemoryMB()) * 100.0f);

                ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.85f, 0.4f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.05f, 0.08f, 0.12f, 1.0f));
                ImGui::PlotLines("##RAM_Graph", ctx.getMemoryHistory(), ctx.getHistoryCapacity(), 
                    0, nullptr, 0.0f, ctx.getMaxMemoryMB(), ImVec2(-1.0f, 110.0f));
                ImGui::PopStyleColor(2);

                ImGui::EndTabItem();
            }

            // TAB 3: SYSTEM SUMMARY
            if (ImGui::BeginTabItem("System Summary")) {
                ImGui::Spacing();
                ImGui::Text("CSOPESY simulated kernel diagnostics");
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextWrapped("Operating System : %s", ctx.osVersion.c_str());
                ImGui::TextWrapped("Kernel Build     : %s", ctx.kernelVersion.c_str());
                ImGui::TextWrapped("Release Build    : %s", ctx.buildNumber.c_str());
                ImGui::TextWrapped("Scheduler model : MLFQ / 20 ms quantum (illustrative; no scheduler runs)");
                ImGui::TextWrapped("Memory model : Paging is illustrative; RAM statistics are simulated");
                ImGui::TextWrapped("IPC model : Shared memory and queues are illustrative; no IPC runs");
                ImGui::TextWrapped("Compositor : OpenGL 3.3 / Dear ImGui immediate-mode windows");
                ImGui::TextWrapped("Total Uptime : %s", ctx.getUptimeString().c_str());

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::Button("Spawn Simulated Task", ImVec2(200, 30))) {
                    ctx.spawnProcess("bench_worker.exe", 14.5f, 128.0f);
                }

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

} // namespace OS
