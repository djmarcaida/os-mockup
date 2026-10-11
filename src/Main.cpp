#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "Core/SystemContext.h"
#include "Boot/BootScreens.h"
#include "Compositor/WindowCompositor.h"
#include "Apps/SystemInfoWindow.h"

#include <iostream>

static void glfwErrorCallback(int error, const char* description) {
    std::cerr << "[GLFW Error " << error << "]: " << description << std::endl;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    // 1. Setup GLFW error callback
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // 2. Configure OpenGL Context (3.3 Core Profile)
    const char* glsl_version = "#version 330 core";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // 3. Create GLFW Window
    const int initialWidth = 1280;
    const int initialHeight = 800;
    GLFWwindow* window = glfwCreateWindow(
        initialWidth, 
        initialHeight, 
        "CSOPESY Desktop OS Mock-up Emulator", 
        nullptr, 
        nullptr
    );

    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable V-Sync

    // 4. Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD OpenGL loader" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // 5. Initialize Dear ImGui Context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Apply default Cyber Dark theme
    OS::SystemInfoWindow::applyThemeToImGui(OS::ThemeMode::CYBER_DARK);

    // 6. Setup Dear ImGui Platform/Renderer Backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // 7. Initialize Operating System State Context
    OS::SystemContext sysContext;

    double lastTime = glfwGetTime();

    // 8. Main Emulation Loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Calculate delta time
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        // Check if OS requested clean shutdown
        if (sysContext.shouldCloseApp()) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        }

        // Update OS context & state machine
        sysContext.update(deltaTime);

        // Start Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Sequential State Machine Execution
        OS::BootState state = sysContext.getState();
        if (state == OS::BootState::BIOS_POST) {
            OS::BootScreens::renderBiosPost(sysContext, deltaTime);
        } else if (state == OS::BootState::BOOT_SPLASH) {
            OS::BootScreens::renderBootSplash(sysContext, deltaTime);
        } else if (state == OS::BootState::DESKTOP) {
            OS::WindowCompositor::render(sysContext, deltaTime);
        }

        // Rendering Pipeline
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.04f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // 9. Graceful Teardown & Resource Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
