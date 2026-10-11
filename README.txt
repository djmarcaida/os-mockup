CSOPESY Desktop OS Mock-up Emulator

CSOPESY GROUP 1
Group members:
  Choa, Bryan
  Descalzo, Alberto Miguel
  La Madrid, Elkan Ainer
  Marcaida, Duncan Joseph

SOURCE AND ENTRY POINT
Repository: https://github.com/djmarcaida/os-mockup
Entry file: src/Main.cpp
Entry function: int main(int argc, char** argv)
Build target: os_mockup
Language: C++17

PREREQUISITES
CMake 3.20+, Git, and a C++17 compiler (Visual Studio 2022 C++ tools on Windows).
Initial dependency setup needs Internet access; GLFW 3.4 and Dear ImGui v1.91.8
are fetched automatically. GLAD is bundled. A working OpenGL 3.3 driver is needed.

BUILD AND RUN (WINDOWS / VISUAL STUDIO 2022)
From the repository directory:
  cmake -S . -B build -G "Visual Studio 17 2022" -A x64
  cmake --build build --config Release --parallel 4
  .\build\bin\Release\os_mockup.exe

Alternatively open the repository folder in Visual Studio with C++/CMake support,
select the os_mockup target, and press Run/Debug. For the assessment video, show
launching with the IDE's Run/Debug control, as required by the supplied brief.

CHECKS
  ctest --test-dir build -C Release --output-on-failure
This runs system_context (backend regression cases) and ui_interactions
(CPU-side ImGui interactions and window bounds). It does not verify native GPU
rendering or the GLFW title-bar close callback; check those in the running app.
Use the taskbar icons to open Task Manager and two distinct custom screens, such
as File Explorer and System Info. Check the clock, resize the desktop, navigate
the virtual folders, edit/save a file in Notepad, and finish with PWR > Shut Down.

DATA AND CONTROLS
Processes, hardware specifications, audio/network state, and files are simulated.
Notepad Save changes the virtual file tree for the current session only. It does
not write user files to the host disk. ImGui window layout can persist in imgui.ini
in the launch working directory. PWR opens a dialog with shutdown/restart/cancel.
The title-bar close and Alt+F4 use the same cancelable power dialog. Restart
retains session data; Shut Down discards it. Notepad asks Save/Discard/Cancel
before replacing unsaved edits. See README.md for architecture and limitations.

SUBMISSION ITEMS TO PREPARE SEPARATELY
Prepare the technical PPT/PPTX with cover, walkthrough, architecture, code
snippets, and design discussion. Record a
seamless uncut MP4 at 480p-720p, maximum 1 GB, with readable IDE text. Show IDE
Run/Debug and do not access/modify/recompile code while performing the test case.
Embed the MP4 directly in the PPTX and include it in the presentation's file size.
Actual quiz inputs and exact deadlines must come from the course/AnimoSpace.
