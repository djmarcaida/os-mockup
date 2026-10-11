# CSOPESY Desktop OS Mock-up Emulator

A desktop-style operating system mock-up and graphics emulator written in **modern C++ (C++17/C++20)** using **GLFW**, **OpenGL 3.3 Core**, **GLAD**, and **Dear ImGui**.

The emulator features an authentic retro boot pipeline that transitions smoothly from a 90s Megatrends BIOS/POST screen into a brand splash screen and finally an interactive windowed desktop environment.

---

## Technical Stack & Architecture

- **Language**: Modern C++17 / C++20
- **Graphics API**: OpenGL 3.3 Core Profile
- **Windowing & Input**: GLFW 3.4 (fetched automatically via CMake `FetchContent`)
- **OpenGL Loader**: GLAD (OpenGL 3.3 Core loader bundled in `external/glad`)
- **Immediate-Mode UI & Compositor**: Dear ImGui v1.91+ (using `imgui_impl_glfw` and `imgui_impl_opengl3` backends)
- **Build System**: Cross-platform CMake (v3.20+)

---

## State Machine Architecture

The emulator implements a sequential boot/runtime state machine:

```
[ State 1: BIOS / POST Screen ]
             │  (2-3 seconds or Space/Esc skip)
             ▼
[ State 2: Boot Splash Screen ]
             │  (Animated loading bar & kernel init messages)
             ▼
[ State 3: Desktop Shell ] ───► Interactive Compositor (Taskbar, Windows, Task Manager)
             │
             ├──► [ Reboot ] ───► Loops back to State 1
             └──► [ PWR ]    ───► Clean exit (glfwSetWindowShouldClose)
```

### 1. State 1: BIOS / POST Screen
- Emulates a classic 90s PC BIOS terminal screen (**CSOPESY Megatrends Modular BIOS v4.51PG**).
- Includes classic POST diagnostic output:
  - Energy Star ANSI header logo.
  - Dynamic ticking memory counter (`262,144 KB OK`).
  - Pentium III 733MHz processor specs and hardware caches.
  - IDE primary/secondary master drive detection (`CSOPESY-VHD 40.0 GB UDMA 66`).
  - PS/2 keyboard and mouse detection.
  - Blinking terminal prompt cursor.
  - Seamless timed transition (or press <kbd>Space</kbd> / <kbd>Esc</kbd> to skip immediately).

### 2. State 2: Boot Splash Screen
- Vignette gradient backdrop with centered ASCII art logo (**CSOPESY**).
- Kernel build and version metadata (`CSOPESY-x86 SMP Hybrid Kernel 2.4.19`).
- Animated progress bar with real-time initialization stages:
  - Hardware Abstraction Layer (HAL) initialization
  - Virtual File System mount (`/dev/vfs0`)
  - Kernel threads and IPC scheduler start
  - OpenGL 3.3 window compositor mount
- Smooth transition into the interactive desktop shell.

### 3. State 3: Desktop Shell
- **Desktop Background (Base Layer)**:
  - Fullscreen, borderless, non-collapsible ImGui background window.
  - Multi-color theme gradient rendered directly via ImGui DrawList with subtle cyber grid lines.
  - Desktop shortcut icons (`Task Manager`, `File Explorer`, `Terminal`, `System Info`, `Readme.txt`).
  - Real-time system clock and hardware monitor card anchored in the fixed top-right corner.
  - Prominent **PWR (Power)** button invoking a clean shutdown confirmation dialog rather than a hard crash/force-exit.
- **Fixed Taskbar Dock**:
  - Dock panel anchored along the bottom edge with system bezel.
  - **Start Menu**: user profile, quick application launcher, reboot and shutdown controls.
  - Clickable application toggles with active indicator states:
    - `Task Manager`
    - `File Explorer`
    - `Terminal (sh)`
    - `System Info`
    - `Notepad`
  - **System Tray**:
    - Interactive volume slider popup.
    - Virtual network connection status popup.
    - Live digital clock with date tooltip.
    - Dedicated quick `PWR` button.
- **Windows Task Manager Mock-up**:
  - Styled after Windows Task Manager with multiple tabs:
    - **Processes Tab**: ImGui table (`ImGui::BeginTable`) with columns: *Process Name*, *PID*, *Status*, *CPU %*, *Memory (MB)*, and *Disk (KB/s)*.
    - Populated with realistic OS processes (`csopesy_kernel.exe`, `compositor.srv`, `shell_ui.exe`, `idle_process`, `dwm_core.dll`, etc.) featuring live dynamic CPU/Memory jitter.
    - Process search filter and **"End Task"** action (with kernel safety protection).
    - **Performance Tab**: Real-time rolling 60-second line graphs for CPU load and RAM utilization (`ImGui::PlotLines`).
    - **System Summary Tab**: Kernel diagnostics, scheduler policy (MLFQ), and benchmark spawner.
- **Custom Screen 1: Virtual File Explorer**:
  - Address bar with path display and Up navigation.
  - Left navigation tree (`C:\CSOPESY`, `System32`, `Users\Administrator`, `Logs`).
  - Detailed file list table with double-click folder navigation and file preview in Notepad.
- **Custom Screen 1 (Alt): Terminal Console (`csopesy_sh`)**:
  - Retro command prompt with history recall (<kbd>↑</kbd> / <kbd>↓</kbd> arrows) and auto-scrolling output.
  - Built-in commands: `help`, `ver`, `ps`, `kill`, `mem`, `uptime`, `date`, `dir`, `cat`, `echo`, `cls`, `theme`, `reboot`, `shutdown`.
- **Custom Screen 2: System Settings & Info**:
  - Live OpenGL context introspection (`glGetString(GL_RENDERER)`, `GL_VENDOR`, `GL_VERSION`).
  - Live FPS and frame-time display with V-Sync toggle.
  - 4 selectable visual themes:
    1. *Cyber Dark* (Modern Slate & Cyan Glow)
    2. *Classic Windows 98 / 2000* (Retro Gray & Teal)
    3. *Retro CRT Amber* (Monochrome Phosphor)
    4. *Matrix Green* (Phosphor Cyberpunk)
- **Notepad / Text Viewer**:
  - Multiline text reader and editor for viewing system and user text files.

---

## Directory Structure

```
os-mockup/
├── CMakeLists.txt
├── README.md
├── .gitignore
├── external/
│   └── glad/
│       ├── include/
│       │   ├── KHR/
│       │   │   └── khrplatform.h
│       │   └── glad/
│       │       └── glad.h
│       └── src/
│           └── glad.c
└── src/
    ├── Main.cpp
    ├── Core/
    │   ├── Types.h
    │   ├── SystemContext.h
    │   └── SystemContext.cpp
    ├── Boot/
    │   ├── BootScreens.h
    │   └── BootScreens.cpp
    ├── Desktop/
    │   ├── DesktopBackground.h
    │   ├── DesktopBackground.cpp
    │   ├── Taskbar.h
    │   └── Taskbar.cpp
    ├── Apps/
    │   ├── TaskManagerWindow.h
    │   ├── TaskManagerWindow.cpp
    │   ├── FileExplorerWindow.h
    │   ├── FileExplorerWindow.cpp
    │   ├── TerminalWindow.h
    │   ├── TerminalWindow.cpp
    │   ├── SystemInfoWindow.h
    │   ├── SystemInfoWindow.cpp
    │   ├── NotepadWindow.h
    │   └── NotepadWindow.cpp
    └── Compositor/
        ├── WindowCompositor.h
        └── WindowCompositor.cpp
```

---

## Build & Run Instructions

The build configuration is **100% self-contained**. Dependencies (GLFW and Dear ImGui) are fetched automatically via CMake's `FetchContent`, and GLAD is bundled directly in the repository.

### Prerequisites
- A C++17 compatible compiler:
  - **Windows**: Visual Studio 2019/2022 (MSVC) or MinGW-w64 (GCC 9+)
  - **Linux**: GCC 9+ or Clang 10+
  - **macOS**: Xcode / Apple Clang
- **CMake 3.20+**
- Git (for fetching GLFW and ImGui during the first build)

---

### Method 1: Visual Studio 2019 / 2022 (Recommended for Windows)

1. Open **Visual Studio**.
2. Select **File > Open > Folder...** and choose the `os-mockup` directory.
3. Visual Studio will automatically detect `CMakeLists.txt` and configure the project.
4. In the startup item dropdown, select `os_mockup.exe`.
5. Press <kbd>F5</kbd> (Debug) or <kbd>Ctrl + F5</kbd> (Run without debugging).

---

### Method 2: VS Code

1. Open the `os-mockup` folder in **Visual Studio Code**.
2. Ensure the **CMake Tools** and **C/C++** extensions are installed.
3. Select your compiler kit (e.g. `Visual Studio Community 2022 Release - amd64` or `GCC`).
4. Press <kbd>F7</kbd> to build or click **Run / Debug** (`os_mockup`) on the status bar.

---

### Method 3: Command Line (CMake)

Open a terminal (PowerShell, Command Prompt, or bash) in the project directory:

```bash
# 1. Generate build files
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 2. Compile the project
cmake --build build --config Release

# 3. Launch the application
# On Windows:
.\build\bin\Release\os_mockup.exe
# (or .\build\bin\os_mockup.exe depending on the generator)

# On Linux / macOS:
./build/bin/os_mockup
```

---

## Keyboard Shortcuts & Quick Actions

| Shortcut | Action |
| :--- | :--- |
| <kbd>Space</kbd> / <kbd>Esc</kbd> | Skip BIOS POST or Splash Screen immediately |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>Esc</kbd> | Toggle Task Manager |
| <kbd>Alt</kbd> + <kbd>F4</kbd> | Open Power / Shutdown Confirmation Dialog |
| <kbd>F1</kbd> | Toggle System Settings & Info Window |
| <kbd>Ctrl</kbd> + <kbd>`</kbd> | Toggle Terminal Shell (`csopesy_sh`) |

---

## License & Credits

- Developed for Operating Systems Engineering (CSOPESY).
- Powered by [Dear ImGui](https://github.com/ocornut/imgui), [GLFW](https://www.glfw.org/), and [GLAD](https://glad.dav1d.de/).