#include "Core/SystemContext.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

struct TestRunner {
    int passed = 0;
    int failed = 0;

    void check(bool condition, const std::string& message) {
        if (!condition) throw std::runtime_error(message);
    }

    template <typename Test>
    void run(const char* name, Test test) {
        try {
            test();
            ++passed;
            std::cout << "[PASS] " << name << '\n';
        } catch (const std::exception& error) {
            ++failed;
            std::cerr << "[FAIL] " << name << ": " << error.what() << '\n';
        }
    }
};

bool near(float left, float right, float tolerance = 0.01f) {
    return std::fabs(left - right) <= tolerance;
}

void testVirtualFilePaths(TestRunner& tests) {
    OS::SystemContext context;
    const std::string documentsPath = "C:\\Users\\Administrator\\Documents";
    const OS::VirtualFile* documents = context.findFileByPath(documentsPath);
    tests.check(documents && documents->isDirectory, "absolute lookup should resolve a directory");
    tests.check(context.navigateToDirectory(documents), "any existing nested directory should be selectable");
    tests.check(context.getCurrentDirPath() == documentsPath, "directory path should be canonical and absolute");

    const OS::VirtualFile* notes = context.findFileByPath("notes.txt");
    tests.check(notes && !notes->isDirectory, "relative file lookup should use the current directory");
    tests.check(context.findFileByPath(".") == documents, "dot should resolve the current directory");
    tests.check(context.findFileByPath("../Desktop/readme.txt") != nullptr,
                "relative lookup should support parent traversal and slash normalization");
    tests.check(context.findFileByPath("c:/users/administrator/desktop/README.TXT") != nullptr,
                "absolute lookup should accept drive and component case differences");

    const std::string beforeInvalidNavigation = context.getCurrentDirPath();
    OS::VirtualFile foreignDirectory("Foreign", true, 0, "", "");
    OS::VirtualFile foreignFile("foreign.txt", false, 0, "", "");
    tests.check(!context.navigateToDirectory(&foreignDirectory), "foreign directory pointers must be rejected");
    tests.check(!context.navigateToDirectory(&foreignFile), "file pointers must not be navigable directories");
    tests.check(!context.navigateToDirectory(nullptr), "null directory pointers must be rejected");
    tests.check(context.getCurrentDirPath() == beforeInvalidNavigation,
                "failed navigation must leave the current directory unchanged");
    tests.check(context.findFileByPath("C:\\missing\\file.txt") == nullptr,
                "missing absolute paths should fail without changing context");
    tests.check(context.getCurrentDirPath() == beforeInvalidNavigation,
                "failed lookup must leave the current directory unchanged");

    tests.check(context.navigateUp(), "nested directory should navigate to its parent");
    tests.check(context.getCurrentDirPath() == "C:\\Users\\Administrator", "navigate up should rebuild the canonical parent path");
    tests.check(context.navigateTo("Desktop"), "legacy folder-name navigation should remain available");
    tests.check(context.getCurrentDirPath() == "C:\\Users\\Administrator\\Desktop", "legacy navigation should update the path");
    tests.check(context.navigateToDirectory(context.getRootFileSystem()), "root directory should be selectable");
    tests.check(context.getCurrentDirPath() == "C:\\", "root path should remain canonical");
    tests.check(!context.navigateUp(), "navigate up at the root should fail");
}

void testNotepadSourceSave(TestRunner& tests) {
    OS::SystemContext context;
    const std::string welcomePath = "C:\\CSOPESY\\welcome.txt";
    tests.check(context.getNotepadFilePath() == welcomePath, "initial editor should bind the displayed welcome file");
    tests.check(context.notepadBuffer == context.findFileByPath(welcomePath)->content,
                "initial editor content should match its virtual source");
    context.notepadBuffer = "welcome edited after taskbar launch";
    context.notepadModified = true;
    std::string defaultMessage;
    tests.check(context.saveNotepadDocument(defaultMessage), "initial document should save to its displayed source");
    tests.check(context.findFileByPath(welcomePath)->content == context.notepadBuffer &&
                !context.findFileByPath("C:\\Untitled-1.txt"), "initial save must not create an unrelated Untitled file");
    const std::string sourcePath = "C:\\Users\\Administrator\\Documents\\notes.txt";
    const OS::VirtualFile* source = context.findFileByPath(sourcePath);
    tests.check(source != nullptr, "source note should be addressable by an absolute path");
    const std::string originalContent = source->content;

    context.openFileInNotepad(source);
    tests.check(context.getNotepadFilePath() == sourcePath, "opening a VFS file should record its canonical path");
    tests.check(context.notepadBuffer == originalContent, "opening a VFS file should copy its content into the editor");
    tests.check(!context.notepadModified, "opening a VFS file should begin clean");

    context.notepadBuffer = "Updated from Notepad\nwith UTF-8: café\n";
    context.notepadModified = true;
    tests.check(context.navigateToDirectory(context.getRootFileSystem()), "current directory should be changeable before saving");

    std::string message;
    tests.check(context.saveNotepadDocument(message), "dirty source document should save successfully");
    const OS::VirtualFile* updated = context.findFileByPath(sourcePath);
    tests.check(updated && updated->content == context.notepadBuffer, "save should update the original VFS source file");
    tests.check(updated->sizeBytes == context.notepadBuffer.size(), "saved size should reflect the buffer byte count");
    tests.check(!updated->modifiedDate.empty(), "saving should refresh the source metadata date");
    tests.check(!context.notepadModified, "successful save should clear the dirty flag");
    tests.check(context.getNotepadFilePath() == sourcePath, "saving should retain the source's canonical path");

    const std::string savedContent = updated->content;
    const std::string savedDate = updated->modifiedDate;
    tests.check(context.saveNotepadDocument(message), "saving an unchanged source should be a successful no-op");
    updated = context.findFileByPath(sourcePath);
    tests.check(updated->content == savedContent && updated->modifiedDate == savedDate,
                "clean source save should not rewrite content or metadata");

    context.openFileInNotepad("scratch", "unsaved text");
    tests.check(context.getNotepadFilePath().empty(), "arbitrary content should remain an unsaved document");
    tests.check(context.notepadBuffer == "unsaved text" && !context.notepadModified,
                "legacy title/content opening should remain compatible");

    const std::string editorTitle = context.notepadTitle;
    const std::string editorBuffer = context.notepadBuffer;
    OS::VirtualFile foreignFile("foreign.txt", false, 0, "", "foreign");
    context.openFileInNotepad(&foreignFile);
    tests.check(context.notepadTitle == editorTitle && context.notepadBuffer == editorBuffer,
                "opening a file outside the VFS should leave the editor unchanged");
}

void testPendingNotepadOpen(TestRunner& tests) {
    OS::SystemContext context;
    const std::string sourcePath = context.getNotepadFilePath();
    const std::string targetPath = "C:\\Users\\Administrator\\Documents\\notes.txt";
    context.notepadBuffer = "keep my unsaved changes";
    context.notepadModified = true;
    context.openFileInNotepad(context.findFileByPath(sourcePath));
    tests.check(context.notepadModified && context.notepadBuffer == "keep my unsaved changes" &&
                !context.hasPendingNotepadOpen(), "reopening the same source must preserve unsaved edits");
    context.openFileInNotepad(context.findFileByPath(targetPath));
    tests.check(context.hasPendingNotepadOpen() && context.getPendingNotepadFilePath() == targetPath,
                "a different file should wait for an explicit decision");
    tests.check(context.getNotepadFilePath() == sourcePath && context.notepadModified &&
                context.notepadBuffer == "keep my unsaved changes", "pending open must preserve the dirty source");
    context.cancelNotepadOpen();
    tests.check(!context.hasPendingNotepadOpen() && context.notepadModified,
                "Cancel should keep the current document dirty");
    context.openFileInNotepad(context.findFileByPath(targetPath));
    std::string message;
    tests.check(context.saveNotepadDocument(message) && context.confirmNotepadOpen(),
                "Save then Open should persist the source before replacing it");
    tests.check(context.findFileByPath(sourcePath)->content == "keep my unsaved changes",
                "Save then Open should retain source changes");
    tests.check(context.getNotepadFilePath() == targetPath && !context.notepadModified &&
                !context.hasPendingNotepadOpen(), "confirmed open should show the requested clean document");
    context.notepadBuffer = "discard this edit";
    context.notepadModified = true;
    context.openFileInNotepad(context.findFileByPath(sourcePath));
    tests.check(context.confirmNotepadOpen() && context.notepadBuffer == "keep my unsaved changes",
                "explicit Discard should open the target without saving the dirty source");
    tests.check(context.findFileByPath(targetPath)->content != "discard this edit",
                "Discard must leave the original virtual file unchanged");

    // Saving a new file may invalidate a pointer to a sibling in the same folder.
    context.navigateToDirectory(context.findFileByPath("C:\\Users\\Administrator\\Documents"));
    context.newNotepadDocument();
    context.notepadBuffer = "new sibling";
    context.notepadModified = true;
    context.openFileInNotepad(context.findFileByPath(targetPath));
    tests.check(context.saveNotepadDocument(message) && context.confirmNotepadOpen(),
                "pending target should survive a new sibling allocation during Save");
    tests.check(context.getNotepadFilePath() == targetPath, "saved-new-file decision should retain the canonical target");
}

void testNewNotepadSaves(TestRunner& tests) {
    OS::SystemContext context;
    tests.check(context.navigateToDirectory(context.findFileByPath("C:\\Users\\Administrator\\Documents")),
                "documents directory should be selectable");

    context.newNotepadDocument();
    tests.check(context.notepadBuffer.empty() && !context.notepadModified,
                "new document should reset buffer and dirty state");
    tests.check(context.getNotepadFilePath().empty(), "new document should clear its source path");
    context.notepadBuffer = "first draft";
    context.notepadModified = true;
    std::string message;
    tests.check(context.saveNotepadDocument(message), "first untitled document should save");
    tests.check(context.getNotepadFilePath() == "C:\\Users\\Administrator\\Documents\\Untitled-1.txt",
                "first unsaved document should receive the first unique filename");
    tests.check(context.notepadTitle == "Untitled-1.txt - Notepad" && !context.notepadModified,
                "successful save should update the title and clear dirty state");

    context.newNotepadDocument();
    context.notepadBuffer = "second draft";
    context.notepadModified = true;
    tests.check(context.saveNotepadDocument(message), "second untitled document should save");
    tests.check(context.getNotepadFilePath() == "C:\\Users\\Administrator\\Documents\\Untitled-2.txt",
                "a name collision should advance to the next unique filename");

    const OS::VirtualFile* first = context.findFileByPath("Untitled-1.txt");
    const OS::VirtualFile* second = context.findFileByPath("Untitled-2.txt");
    tests.check(first && first->content == "first draft" && first->sizeBytes == std::string("first draft").size(),
                "first new document should be stored in the current virtual directory");
    tests.check(second && second->content == "second draft" && second->sizeBytes == std::string("second draft").size(),
                "second new document should be stored without overwriting the first");

    context.openFileInNotepad(first);
    context.notepadBuffer = "updated first draft";
    context.notepadModified = true;
    tests.check(context.navigateToDirectory(context.getRootFileSystem()), "directory may change while editing a source file");
    tests.check(context.saveNotepadDocument(message), "source save should re-find its file after directory changes");
    first = context.findFileByPath("C:\\Users\\Administrator\\Documents\\Untitled-1.txt");
    tests.check(first && first->content == "updated first draft",
                "saving after vector insertions and navigation should update the canonical source");
}

void testProcessMetricsAndTerminal(TestRunner& tests) {
    OS::SystemContext context;
    auto sumCpu = [&context]() {
        float total = 0.0f;
        for (const auto& process : context.getProcesses()) total += process.cpuUsage;
        return total;
    };
    auto sumMemory = [&context]() {
        float total = 0.0f;
        for (const auto& process : context.getProcesses()) total += process.memoryMB;
        return total;
    };

    tests.check(near(context.getTotalCpuUsage(), sumCpu() - context.getProcesses().front().cpuUsage),
                "initial CPU summary should reflect active table rows");
    tests.check(near(context.getTotalMemoryMB(), sumMemory()), "initial memory summary should reflect table rows");

    const size_t initialCount = context.getProcesses().size();
    const float initialCpu = context.getTotalCpuUsage();
    const float initialMemory = context.getTotalMemoryMB();
    context.spawnProcess("bad_values.exe", std::numeric_limits<float>::quiet_NaN(),
                         std::numeric_limits<float>::infinity());
    tests.check(context.getProcesses().size() == initialCount + 1, "spawn should append a process immediately");
    tests.check(context.getTotalCpuUsage() >= initialCpu && context.getTotalMemoryMB() >= initialMemory,
                "spawn should refresh CPU and memory totals immediately");
    tests.check(near(context.getTotalCpuUsage(), sumCpu() - context.getProcesses().front().cpuUsage),
                "CPU summary should match active process rows after spawn");
    tests.check(near(context.getTotalMemoryMB(), sumMemory()), "memory summary should match process rows after spawn");
    for (const auto& process : context.getProcesses()) {
        tests.check(std::isfinite(process.cpuUsage) && std::isfinite(process.memoryMB) && std::isfinite(process.diskIOKB),
                    "all process metrics should remain finite");
    }

    context.executeTerminalCommand("mem");
    const std::string expectedMemory = std::to_string(static_cast<int>(context.getTotalMemoryMB())) + " MB used";
    tests.check(context.getTerminalLines().back().text.find(expectedMemory) != std::string::npos,
                "terminal mem output should reflect the latest process table total");

    std::string message;
    const size_t countBeforeDenied = context.getProcesses().size();
    tests.check(!context.killProcess(4, message), "protected kernel process should not be terminable");
    tests.check(context.getProcesses().size() == countBeforeDenied, "protected process rejection should leave table unchanged");
    tests.check(near(context.getTotalMemoryMB(), sumMemory()), "rejected kill should leave totals consistent");
    tests.check(!context.killProcess(999999, message), "unknown process id should be rejected");
    tests.check(context.getProcesses().size() == countBeforeDenied, "unknown process rejection should leave table unchanged");

    const auto added = context.getProcesses().back();
    tests.check(context.killProcess(added.pid, message), "spawned user process should be terminable");
    tests.check(context.getProcesses().size() == initialCount, "successful kill should remove the selected process immediately");
    tests.check(near(context.getTotalMemoryMB(), initialMemory), "successful kill should refresh memory totals immediately");
    tests.check(near(context.getTotalCpuUsage(), sumCpu() - context.getProcesses().front().cpuUsage),
                "successful kill should refresh CPU totals immediately");
    tests.check(near(sumCpu(), 100.0f), "idle CPU should balance the active table rows after process mutations");

    const size_t terminalLineCount = context.getTerminalLines().size();
    context.executeTerminalCommand(" \t  \r\n");
    tests.check(context.getTerminalLines().size() == terminalLineCount,
                "whitespace-only terminal input should not emit an empty-command error");
    context.executeTerminalCommand("  HELP  ");
    tests.check(context.getTerminalLines().size() > terminalLineCount,
                "case-insensitive command parsing should still accept leading whitespace");
    context.executeTerminalCommand("cat Users");
    tests.check(context.getTerminalLines().back().text == "Cannot read a directory: Users",
                "cat must reject directories");
    context.executeTerminalCommand("cat C:\\CSOPESY\\welcome.txt");
    tests.check(context.getTerminalLines().back().text.find("Cannot read") == std::string::npos,
                "cat should still read absolute file paths");
    for (int index = 0; index < 80; ++index) context.spawnProcess("capacity_test.exe", 2.0f, 128.0f);
    tests.check(context.getTotalMemoryMB() <= context.getMaxMemoryMB() &&
                near(context.getTotalMemoryMB(), sumMemory(), 0.1f),
                "saturated memory must stay within capacity and equal table rows");
    context.setState(OS::BootState::DESKTOP);
    context.update(1.0f);
    tests.check(context.getTotalMemoryMB() <= context.getMaxMemoryMB() &&
                context.getMemoryHistory()[59] <= context.getMaxMemoryMB() &&
                near(context.getTotalMemoryMB(), sumMemory(), 0.1f),
                "process jitter and graphs must preserve saturated capacity");
}

void testBootRebootAndHistory(TestRunner& tests) {
    OS::SystemContext context;
    context.update(context.getPostDuration());
    tests.check(context.getState() == OS::BootState::BOOT_SPLASH, "POST should transition to splash after its duration");
    context.update(context.getSplashDuration());
    tests.check(context.getState() == OS::BootState::DESKTOP, "splash should transition to desktop after its duration");
    tests.check(context.showTaskManager, "desktop transition should keep the default Task Manager behavior");

    context.setState(OS::BootState::DESKTOP);
    const float initialCpuSample = context.getCpuHistory()[59];
    const float initialMemorySample = context.getMemoryHistory()[59];
    context.update(0.75f);
    context.update(0.24f);
    tests.check(context.getCpuHistory()[59] == initialCpuSample && context.getMemoryHistory()[59] == initialMemorySample,
                "60-sample performance history should not advance before one second");
    context.update(0.02f);
    tests.check(near(context.getCpuHistory()[59], context.getTotalCpuUsage()),
                "CPU history should append after the one-second sample interval");
    tests.check(near(context.getMemoryHistory()[59], context.getTotalMemoryMB()),
                "memory history should append after the one-second sample interval");
    tests.check(context.getHistoryCapacity() == 60, "performance history should retain 60 samples");

    const std::string sourcePath = "C:\\CSOPESY\\welcome.txt";
    const OS::VirtualFile* source = context.findFileByPath(sourcePath);
    tests.check(source != nullptr, "welcome file should exist before reboot");
    context.openFileInNotepad(source);
    context.notepadBuffer = "preserve this unsaved edit";
    context.notepadModified = true;
    const size_t processCount = context.getProcesses().size();

    context.showTaskManager = context.showFileExplorer = context.showTerminal = true;
    context.showSystemInfo = context.showNotepad = context.showStartMenu = true;
    context.showPowerDialog = context.showVolumePopup = context.showNetworkPopup = true;
    context.reboot();
    tests.check(context.getState() == OS::BootState::BIOS_POST, "reboot should return to BIOS POST");
    tests.check(!context.showTaskManager && !context.showFileExplorer && !context.showTerminal &&
                !context.showSystemInfo && !context.showNotepad && !context.showStartMenu &&
                !context.showPowerDialog && !context.showVolumePopup && !context.showNetworkPopup,
                "reboot should hide every application, dialog, and popup");
    tests.check(context.getProcesses().size() == processCount, "reboot should preserve the process simulation");
    tests.check(context.notepadBuffer == "preserve this unsaved edit" && context.notepadModified,
                "reboot should preserve unsaved editor content");
    tests.check(context.findFileByPath(sourcePath) != nullptr, "reboot should preserve the virtual file system");
}

} // namespace

int main() {
    TestRunner tests;
    tests.run("virtual file paths", [&] { testVirtualFilePaths(tests); });
    tests.run("Notepad source save", [&] { testNotepadSourceSave(tests); });
    tests.run("pending Notepad open decisions", [&] { testPendingNotepadOpen(tests); });
    tests.run("new Notepad saves", [&] { testNewNotepadSaves(tests); });
    tests.run("process metrics and terminal", [&] { testProcessMetricsAndTerminal(tests); });
    tests.run("boot, reboot, and history", [&] { testBootRebootAndHistory(tests); });
    std::cout << tests.passed << " passed, " << tests.failed << " failed\n";
    return tests.failed == 0 ? 0 : 1;
}
