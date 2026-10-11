#include "TaskManagerWindow.h"
#include <imgui.h>
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace OS {

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

    ImGui::SetNextWindowSize(ImVec2(680, 520), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(180, 100), ImGuiCond_FirstUseEver);

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
                    ImGui::Text("Enter process executable name:");
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
                    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", s_statusMessage.c_str());
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Process Table
                const auto& processes = ctx.getProcesses();
                std::string filter = s_searchBuffer;
                std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

                ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | 
                                            ImGuiTableFlags_RowBg | 
                                            ImGuiTableFlags_Resizable | 
                                            ImGuiTableFlags_ScrollY | 
                                            ImGuiTableFlags_Sortable;

                if (ImGui::BeginTable("ProcessTable", 6, tableFlags, ImVec2(0.0f, -ImGui::GetFrameHeightWithSpacing() - 6.0f))) {
                    ImGui::TableSetupColumn("Process Name", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                    ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    ImGui::TableSetupColumn("CPU %", ImGuiTableColumnFlags_WidthFixed, 75.0f);
                    ImGui::TableSetupColumn("Memory (MB)", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                    ImGui::TableSetupColumn("Disk (KB/s)", ImGuiTableColumnFlags_WidthFixed, 85.0f);
                    ImGui::TableHeadersRow();

                    for (const auto& proc : processes) {
                        // Apply filter
                        if (!filter.empty()) {
                            std::string procNameLower = proc.name;
                            std::transform(procNameLower.begin(), procNameLower.end(), procNameLower.begin(), ::tolower);
                            if (procNameLower.find(filter) == std::string::npos) {
                                continue;
                            }
                        }

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
                            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "%s", proc.status.c_str());
                        } else {
                            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", proc.status.c_str());
                        }

                        // Column 3: CPU %
                        ImGui::TableSetColumnIndex(3);
                        if (proc.cpuUsage > 15.0f && proc.name != "idle_process") {
                            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "%.1f %%", proc.cpuUsage);
                        } else {
                            ImGui::Text("%.1f %%", proc.cpuUsage);
                        }

                        // Column 4: Memory (MB)
                        ImGui::TableSetColumnIndex(4);
                        if (proc.memoryMB > 100.0f) {
                            ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "%.1f MB", proc.memoryMB);
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
                ImGui::Text("Processes: %zu  |  Overall CPU: %.1f%%  |  Physical Memory: %d MB / %d MB", 
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
                ImGui::TextColored(ImVec4(0.3f, 0.85f, 1.0f, 1.0f), "CPU Utilization History (60 Seconds)");
                ImGui::Text("Processor: %s", ctx.cpuModel.c_str());
                ImGui::Text("Current Load: %.1f%%  |  Speed: 733 MHz  |  Sockets: 1  |  Cores: 1  |  Threads: %zu", 
                    ctx.getTotalCpuUsage(), ctx.getProcesses().size() * 4);

                ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.2f, 0.85f, 0.4f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.05f, 0.08f, 0.12f, 1.0f));
                ImGui::PlotLines("##CPU_Graph", ctx.getCpuHistory(), ctx.getHistoryCapacity(), 
                    0, nullptr, 0.0f, 100.0f, ImVec2(-1.0f, 110.0f));
                ImGui::PopStyleColor(2);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Memory Section
                ImGui::TextColored(ImVec4(0.85f, 0.5f, 1.0f, 1.0f), "Memory Usage History");
                ImGui::Text("Used: %d MB  |  Available: %d MB  |  Total Capacity: %d MB (%.1f%%)",
                    static_cast<int>(ctx.getTotalMemoryMB()),
                    static_cast<int>(ctx.getMaxMemoryMB() - ctx.getTotalMemoryMB()),
                    static_cast<int>(ctx.getMaxMemoryMB()),
                    (ctx.getTotalMemoryMB() / ctx.getMaxMemoryMB()) * 100.0f);

                ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.85f, 0.4f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.05f, 0.08f, 0.12f, 1.0f));
                ImGui::PlotLines("##RAM_Graph", ctx.getMemoryHistory(), ctx.getHistoryCapacity(), 
                    0, nullptr, 0.0f, 4096.0f, ImVec2(-1.0f, 110.0f));
                ImGui::PopStyleColor(2);

                ImGui::EndTabItem();
            }

            // TAB 3: SYSTEM SUMMARY
            if (ImGui::BeginTabItem("System Summary")) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.35f, 0.85f, 1.0f, 1.0f), "CSOPESY OS Kernel Diagnostics");
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::Text("Operating System : %s", ctx.osVersion.c_str());
                ImGui::Text("Kernel Build     : %s", ctx.kernelVersion.c_str());
                ImGui::Text("Release Build    : %s", ctx.buildNumber.c_str());
                ImGui::Text("Scheduler Policy : Multi-Level Feedback Queue (MLFQ) - Quantum 20ms");
                ImGui::Text("Virtual Memory   : Two-level Paging with Demand Paging Enabled");
                ImGui::Text("IPC Subsystem    : Posix Shared Memory & Message Queues");
                ImGui::Text("Compositor       : OpenGL 3.3 Core Profile / Dear ImGui reactive pipeline");
                ImGui::Text("Total Uptime     : %s", ctx.getUptimeString().c_str());

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::Button("Spawn Benchmark Process", ImVec2(200, 30))) {
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
