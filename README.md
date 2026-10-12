# CSOPESY Desktop OS Mock-up Emulator

A desktop-style operating system mock-up and graphics emulator written in **C++17** using **GLFW**, **OpenGL 3.3 Core**, **GLAD**, and **Dear ImGui**.

The emulator boots through a 1990s-style Megatrends BIOS/POST screen and animated splash before opening an interactive desktop with a looping portrait wallpaper slideshow. Its processes, virtual files, hardware specifications, and network state are simulated; the clock and OpenGL renderer information come from the host. The supplied startup MP3 plays through the host's audio output.

---

## Technical Stack & Architecture

- **Language**: C++17
- **Graphics API**: OpenGL 3.3 Core Profile
- **Windowing & Input**: GLFW 3.4 (fetched automatically via CMake `FetchContent`)
- **OpenGL Loader**: GLAD (OpenGL 3.3 Core loader bundled in `external/glad`)
- **Immediate-Mode UI & Compositor**: Dear ImGui v1.91.8 (using `imgui_impl_glfw` and `imgui_impl_opengl3` backends)
- **Image Loading**: stb_image (JPEG/PNG decoder bundled in `external/stb`)
- **Audio Playback**: miniaudio v0.11.21 (MP3 playback bundled in `external/miniaudio`)
- **Build System**: Cross-platform CMake (v3.20+)

---

## State Machine Architecture

The emulator implements a sequential boot/runtime state machine:

```
[ State 1: BIOS / POST Screen ]
             │  (3.2 seconds or Space/Esc skip)
             ▼
[ State 2: Boot Splash Screen ]
             │  (3.0 seconds, animated loading bar, or Space/Esc skip)
             ▼
[ State 3: Desktop Shell ] ───► Interactive Compositor (Taskbar, Windows, Task Manager)
             │
             ├──► [ Reboot ] ───► Loops back to State 1
             └──► [ PWR ]    ───► Clean exit (glfwSetWindowShouldClose)
```

### 1. State 1: BIOS / POST Screen
- Emulates a classic 1990s PC BIOS terminal screen (**CSOPESY Megatrends Modular BIOS v4.51PG**).
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
- Animated progress bar with illustrative initialization messages:
  - Hardware Abstraction Layer (HAL) initialization
  - Virtual File System mount (`/dev/vfs0`)
  - Simulated kernel threads and IPC scheduler startup
  - OpenGL 3.3 window compositor mount
- Smooth transition into the interactive desktop shell.
- Plays `assets/audio/boot.mp3` once on entering the splash, including after Restart. Skipping directly to the desktop still triggers it once. Playback runs without blocking the frame loop.

### 3. State 3: Desktop Shell
- **Desktop Background (Base Layer)**:
  - Fullscreen, borderless, non-collapsible ImGui background window.
  - The four supplied portraits loop in order: Bryan, Alberto, Duncan, Elkan. Each image holds for 4 seconds, then crossfades into the next over 1 second.
  - **System Info > Appearance > Wallpaper** offers **Fit** (default: the full image stays visible and proportional), **Crop to fill** (centered crop preserving proportions), and **Stretch to fill** (the full image stretches to fill). All modes use the desktop area above the taskbar. The selected theme fills any surrounding space; Restart resets the slideshow to the first image and retains the session's sizing choice.
  - Multi-color theme gradient rendered directly via ImGui DrawList with subtle cyber grid lines, also used as a fallback if images are unavailable.
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
    - Interactive volume/mute popup controlling the startup effect only. It does not change the host's system volume.
    - Virtual network connection status popup.
    - Live digital clock with date tooltip.
    - Dedicated quick `PWR` button.
- **Windows Task Manager Mock-up**:
  - Styled after Windows Task Manager with multiple tabs:
    - **Processes Tab**: ImGui table (`ImGui::BeginTable`) with columns: *Process Name*, *PID*, *Status*, *CPU %*, *Memory (MB)*, and *Disk (KB/s)*.
    - Populated with realistic OS processes (`csopesy_kernel.exe`, `compositor.srv`, `shell_ui.exe`, `idle_process`, `dwm_core.dll`, etc.) featuring live dynamic CPU/Memory jitter.
    - Process search filter and **"End Task"** action (with kernel safety protection).
    - **Performance Tab**: Real-time rolling 60-second line graphs for CPU load and RAM utilization (`ImGui::PlotLines`).
    - **System Summary Tab**: Illustrative kernel/scheduler information and a simulated task spawner. No MLFQ scheduler, paging system, or IPC mechanism runs.
- **Custom Screen 1: Virtual File Explorer**:
  - Address bar with path display and Up navigation.
  - Left navigation tree (`C:\CSOPESY`, `System32`, `Users\Administrator`, `Logs`).
  - Detailed file list table with double-click folder navigation and file preview in Notepad.
- **Custom Screen 1 (Alt): Terminal Console (`csopesy_sh`)**:
  - Retro command prompt with history recall (<kbd>↑</kbd> / <kbd>↓</kbd> arrows) and auto-scrolling output.
  - Built-in commands: `help`, `ver`, `ps`, `kill <pid>`, `mem`, `uptime`, `date`/`time`, `dir`/`ls`, `cat <file>`, `echo <text>`, `cls`/`clear`, `taskmgr`, `explorer`, `theme <name>`, `reboot`, `shutdown`/`exit`.
- **Custom Screen 2: System Settings & Info**:
  - Live OpenGL context introspection (`glGetString(GL_RENDERER)`, `GL_VENDOR`, `GL_VERSION`).
  - Live FPS and frame-time display with V-Sync toggle.
  - 4 selectable visual themes:
    1. *Cyber Dark* (Modern Slate & Cyan Glow)
    2. *Classic Windows 98 / 2000* (Retro Gray & Teal)
    3. *Retro CRT Amber* (Monochrome Phosphor)
    4. *Matrix Green* (Phosphor Cyberpunk)
- **Notepad / Text Viewer**:
  - Multiline text reader and editor with New/Save actions and Ctrl+N/Ctrl+S shortcuts while focused.
  - Save updates the opened file in the in-memory virtual filesystem; new documents receive a unique `Untitled-N.txt` name in the current virtual folder.
  - The initial welcome document is bound to `C:\CSOPESY\welcome.txt`. Opening another file or creating a new document while edits are unsaved offers Save, Discard, and Cancel.
  - Saved virtual files last for the current application session. Closing the application does not write them to the host disk.

---

## Directory Structure

```
os-mockup/
├── CMakeLists.txt
├── README.md
├── README.txt
├── .gitignore
├── imgui.ini
├── assets/
│   ├── audio/
│   │   └── boot.mp3
│   └── wallpapers/
│       ├── bryan2x2.jpg
│       ├── my2x2.jpg
│       ├── duncan2x2.jpg
│       └── kan2x2.png
├── tests/
│   ├── SystemContextTests.cpp
│   ├── UiInteractionTests.cpp
│   └── MediaTests.cpp
├── external/
│   ├── glad/
│   │   ├── include/
│   │   │   ├── KHR/
│   │   │   │   └── khrplatform.h
│   │   │   └── glad/
│   │   │       └── glad.h
│   │   └── src/
│   │       └── glad.c
│   ├── stb/
│   │   └── stb_image.h
│   └── miniaudio/
│       └── miniaudio.h
└── src/
    ├── Main.cpp
    ├── Core/
    │   ├── AssetPaths.h
    │   ├── AssetPaths.cpp
    │   ├── Types.h
    │   ├── SystemContext.h
    │   └── SystemContext.cpp
    ├── Boot/
    │   ├── BootAudio.h
    │   ├── BootAudio.cpp
    │   ├── BootScreens.h
    │   └── BootScreens.cpp
    ├── Desktop/
    │   ├── WallpaperSlideshow.h
    │   ├── WallpaperSlideshow.cpp
    │   ├── DesktopBackground.h
    │   ├── DesktopBackground.cpp
    │   ├── Taskbar.h
    │   ├── Taskbar.cpp
    │   └── UiHelpers.h
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

Dependencies (GLFW and Dear ImGui) are fetched automatically via CMake's `FetchContent`; GLAD, stb_image, and miniaudio are bundled directly in the repository. The first configuration needs network access; running the application needs a working OpenGL 3.3 driver. Startup audio uses an available playback device; missing audio hardware or media files are logged without preventing the desktop from opening.

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
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# 2. Compile the project
cmake --build build --config Release

# 3. Launch the application
# On Windows:
.\build\bin\Release\os_mockup.exe
# (or .\build\bin\os_mockup.exe depending on the generator)

# On Linux / macOS:
./build/bin/os_mockup
```

CMake copies `assets` beside the executable on every build. When sharing the executable, keep that `assets` folder beside it, including `audio/boot.mp3` and all four files in `wallpapers`. Media paths are resolved relative to the executable, so launching from an IDE or another working directory still loads them. To change the slideshow timing, adjust `HoldSeconds` and `FadeSeconds` in `src/Desktop/WallpaperSlideshow.h` and rebuild.

---

## Keyboard Shortcuts & Quick Actions

| Shortcut | Action |
| :--- | :--- |
| <kbd>Space</kbd> / <kbd>Esc</kbd> | Skip BIOS POST or Splash Screen immediately |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>Esc</kbd> | Toggle Task Manager |
| <kbd>Alt</kbd> + <kbd>F4</kbd> | Open Power / Shutdown Confirmation Dialog |
| <kbd>F1</kbd> | Toggle System Settings & Info Window |
| <kbd>Ctrl</kbd> + <kbd>`</kbd> | Toggle Terminal Shell (`csopesy_sh`) |
| <kbd>Ctrl</kbd> + <kbd>N</kbd> / <kbd>S</kbd> | New / Save document while Notepad is focused |

Native title-bar close also opens the cancelable power dialog. Use **PWR > Shut Down** to demonstrate the required shutdown path. The supported minimum window size is 640 x 480; desktop controls and application windows adapt when the window is resized.

## Regression Checks

On Windows with the Visual Studio 2022 C++ build tools installed:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
```

The context tests cover virtual-file navigation, source-aware saving and unsaved-open decisions, protected process behavior, bounded usage totals, boot/reboot state, and performance-history timing. CPU-side ImGui tests cover window bounds, compact taskbar app launches, power cancellation, theme changes, Ctrl+S, and the Save/Discard/Cancel decisions. Process usage values, CPU specifications, network state, and OS scheduler labels are simulated; the clock and OpenGL renderer information come from the host. Startup audio uses the host playback device, with application volume/mute controlling only this effect.

CTest registers `system_context`, `ui_interactions`, and `media_slideshow`. The tests use explicit checks in Release builds. The ImGui suite creates a CPU-side context and stubs OpenGL metadata; it does not exercise the GLFW close callback. The media slideshow test creates a hidden native GLFW/OpenGL context and skips when a usable GPU/display is unavailable. Listen for the startup effect and verify the title-bar close dialog in the running app. To build only the application, configure with `-DBUILD_TESTING=OFF`.

For coursework packaging, see [README.txt](README.txt) for the group members, entry point, and submission preparation. The technical presentation and assessment recording must be prepared separately.

## Architecture and Extension

The entry point is `int main(int argc, char** argv)` in `src/Main.cpp`. It creates a GLFW window and OpenGL context, loads GLAD, initializes the ImGui backends, constructs `OS::SystemContext`, and runs the frame loop. Each frame polls input, advances the context, renders the current boot/desktop state, submits ImGui draw data to OpenGL, and swaps buffers.

`AssetPaths` locates the bundled media beside the executable. `WallpaperSlideshow` loads the portraits into OpenGL textures once and blends them on the desktop draw list; its textures are released before the OpenGL context is destroyed. `BootAudio` decodes the short MP3 once, schedules playback once per boot, and applies the context's volume/mute settings to that sound. Restart resets both media features; missing images are skipped and unavailable audio is logged.

`SystemContext` owns the boot timers, simulated process table, performance history, virtual file tree, terminal output, editor document, theme, and visibility flags. App modules expose `static void render(SystemContext&, float)` and share this context. UI-specific state in static members means there is one instance of each app.

During the desktop state, the compositor applies the current theme and handles shortcuts, then submits the wallpaper, five application windows, fixed taskbar/tray, and power modal. The modal is submitted last. `UiHelpers.h` draws vector icons and constrains app bounds above the taskbar, including layouts restored from `imgui.ini`.

### Virtual files and editor behavior

- The VFS is rooted at `C:\`. Lookups accept absolute/relative paths, either slash separator, case differences, and `.`/`..`. Explorer navigation and terminal file lookup share the current virtual directory.
- Notepad records a canonical source path rather than retaining a pointer into a folder's child vector. Save re-resolves that source, updates content/size/date, and clears the dirty flag. New documents use unique names in Explorer's current folder.
- Opening a different file with dirty edits queues its canonical path. Save & open persists the current document before resolving the target; Discard opens it without saving; Cancel, Escape, or closing the prompt retains the edited document. Reopening the same source preserves its unsaved buffer.
- Restart returns to the boot sequence and hides applications/popups while retaining the session's virtual files, editor buffer, and simulated processes. Shutdown ends the session and discards this data. Layout alone can persist in `imgui.ini` in the launch working directory.

### Process metrics and themes

Process values fluctuate within simulated bounds. Spawn/kill operations refresh table totals immediately; protected system rows cannot be terminated. Active CPU usage is bounded to 100%, with idle usage balancing it after updates. When memory demand exceeds the simulated 8192 MB capacity, row values are scaled proportionally so their total and the graphs stay within capacity. History contains 60 samples collected once per second.

Both Settings and the terminal `theme` command select the same theme state. The compositor resets/applies a complete ImGui palette each desktop frame so changing themes through either route gives consistent results.

### Adding an application, command, or theme

1. For an application, add a renderer under `src/Apps`, a visibility flag in `SystemContext`, and its files to `OS_MOCKUP_SOURCES` in `CMakeLists.txt`. Render it in the compositor's application layer using `Ui::prepareAppWindow` and a stable ImGui window ID. Register its launcher in the Start menu and, where appropriate, the desktop/taskbar.
2. For a terminal command, extend `SystemContext::executeTerminalCommand` and the `help` text. Use context APIs so the shell and apps share the same simulated state.
3. For a theme, extend `ThemeMode`, the desktop background palette, `SystemInfoWindow::applyThemeToImGui`, the Settings selector, and the terminal alias mapping.
4. Extend context/ImGui regression checks for behavior changes and manually verify native rendering and shutdown when affected.

## Current Limitations

- File edits are session-only. Explorer/Terminal do not provide general create-folder, rename, move, delete, or permission operations; Notepad can create text files and save existing ones.
- The context remains centralized, and static UI state supports one window per app. Multiple independent instances would require separate instance state.
- The VFS uses nested child vectors and retains a current-directory pointer. Future structural mutations must preserve node identity or re-resolve directory/document paths safely.
- App visibility is independent of the simulated process table. Opening or hiding a window does not create or terminate a host or virtual process.
- The default ImGui font is used. The project provides no custom font loader, operating-system scheduler, host system-volume/network control, or persistent virtual filesystem. Audio playback is limited to the bundled startup effect.
- The supported minimum client size is 640 x 480. Narrow tables scroll horizontally. Committed `imgui.ini` supplies initial layout settings; removing the local layout file resets them.

---

## License & Credits

- Developed for Operating Systems Engineering (CSOPESY).
- Powered by [Dear ImGui](https://github.com/ocornut/imgui), [GLFW](https://www.glfw.org/), and [GLAD](https://glad.dav1d.de/).
- Media loading and playback use [stb_image](https://github.com/nothings/stb) and [miniaudio](https://github.com/mackron/miniaudio).
