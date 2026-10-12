#include "Desktop/WallpaperSlideshow.h"
#include "Desktop/DesktopBackground.h"
#include "Desktop/Taskbar.h"
#include "Desktop/UiHelpers.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
int checks = 0;
// The bundled GLAD loader only exposes functions used by the application.
// Resolve these additional read-only inspection calls for the native checks.
using GetTextureParameter = void (APIENTRY*)(GLenum, GLint, GLenum, GLint*);
using IsTexture = GLboolean (APIENTRY*)(GLuint);
using ReadPixels = void (APIENTRY*)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void*);
using Finish = void (APIENTRY*)();
GetTextureParameter getTextureParameter = nullptr;
IsTexture isTexture = nullptr;
ReadPixels readPixels = nullptr;
Finish finish = nullptr;

void check(bool condition, const std::string& message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}

bool unavailableNativeContext(int error) {
    return error == GLFW_API_UNAVAILABLE || error == GLFW_VERSION_UNAVAILABLE ||
           error == GLFW_FORMAT_UNAVAILABLE || error == GLFW_PLATFORM_ERROR ||
           error == GLFW_PLATFORM_UNAVAILABLE;
}

struct ImageDraw {
    ImTextureID texture;
    unsigned int alpha;
    ImVec2 minimum;
    ImVec2 maximum;
    ImVec2 uvMinimum;
    ImVec2 uvMaximum;
};

using Frame = std::vector<ImageDraw>;

Frame imageDraws(ImDrawList* drawList) {
    Frame result;
    for (const ImDrawCmd& command : drawList->CmdBuffer) {
        if (command.ElemCount == 0 || command.UserCallback) continue;
        // ImGui may batch opaque and translucent image quads into one command.
        // Inspect each triangle's vertices so batching cannot hide layer alpha.
        for (unsigned int triangle = 0; triangle < command.ElemCount; triangle += 3) {
            ImVec2 minimum(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
            ImVec2 maximum(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());
            ImVec2 uvMinimum(1, 1), uvMaximum(0, 0);
            const ImDrawVert& first = drawList->VtxBuffer[command.VtxOffset +
                drawList->IdxBuffer[command.IdxOffset + triangle]];
            for (unsigned int offset = triangle; offset < triangle + 3; ++offset) {
                const ImDrawVert& vertex = drawList->VtxBuffer[command.VtxOffset +
                    drawList->IdxBuffer[command.IdxOffset + offset]];
                minimum.x = std::min(minimum.x, vertex.pos.x);
                minimum.y = std::min(minimum.y, vertex.pos.y);
                maximum.x = std::max(maximum.x, vertex.pos.x);
                maximum.y = std::max(maximum.y, vertex.pos.y);
                uvMinimum.x = std::min(uvMinimum.x, vertex.uv.x);
                uvMinimum.y = std::min(uvMinimum.y, vertex.uv.y);
                uvMaximum.x = std::max(uvMaximum.x, vertex.uv.x);
                uvMaximum.y = std::max(uvMaximum.y, vertex.uv.y);
            }
            result.push_back({command.GetTexID(), (first.col >> IM_COL32_A_SHIFT) & 255U,
                              minimum, maximum, uvMinimum, uvMaximum});
        }
    }
    return result;
}

Frame frame(float deltaTime, ImVec2 size = ImVec2(1280, 800),
            ImVec2 position = ImVec2(0, 0),
            OS::WallpaperSizing sizing = OS::WallpaperSizing::FIT) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(1280, 800);
    io.DisplayFramebufferScale = ImVec2(1, 1);
    io.DeltaTime = 1.0f / 60.0f;
    ImGui::NewFrame();
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    OS::WallpaperSlideshow::render(drawList, position, size, deltaTime, sizing);
    ImGui::Render();
    return imageDraws(drawList);
}

Frame atTime(float elapsed, ImVec2 size = ImVec2(1280, 800),
             ImVec2 position = ImVec2(0, 0),
             OS::WallpaperSizing sizing = OS::WallpaperSizing::FIT) {
    OS::WallpaperSlideshow::reset();
    frame(elapsed, size, position, sizing);
    return frame(0, size, position, sizing);
}

std::set<ImTextureID> textures(const Frame& value) {
    std::set<ImTextureID> result;
    for (const ImageDraw& draw : value) result.insert(draw.texture);
    return result;
}

void checkOnly(const Frame& value, ImTextureID texture, const std::string& message) {
    check(!value.empty() && textures(value) == std::set<ImTextureID>{texture}, message);
    for (const ImageDraw& draw : value) check(draw.alpha == 255, message + " (not opaque)");
}

void checkIncoming(const Frame& value, ImTextureID outgoing, ImTextureID incoming,
                   unsigned int expectedAlpha, const std::string& message) {
    check(textures(value) == std::set<ImTextureID>{outgoing, incoming}, message + " (textures)");
    bool foundIncoming = false;
    bool foundOpaqueOutgoing = false;
    for (const ImageDraw& draw : value) {
        if (draw.texture == incoming) {
            foundIncoming = true;
            check(draw.alpha == expectedAlpha, message + " (incoming alpha)");
        }
        if (draw.texture == outgoing && draw.alpha == 255) foundOpaqueOutgoing = true;
    }
    check(foundIncoming && foundOpaqueOutgoing, message + " (crossfade base)");
}

void checkGeometry(ImTextureID texture, ImVec2 size, ImVec2 position) {
    const Frame value = atTime(0, size, position);
    checkOnly(value, texture, "Resizing must retain the current wallpaper");
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(texture));
    GLint width = 0, height = 0;
    getTextureParameter(GL_TEXTURE_2D, 0, 0x1000 /* GL_TEXTURE_WIDTH */, &width);
    getTextureParameter(GL_TEXTURE_2D, 0, 0x1001 /* GL_TEXTURE_HEIGHT */, &height);
    check(width > 0 && height > 0, "Wallpaper texture must contain decoded image data");
    const float scale = std::min(size.x / width, size.y / height);
    const float expectedWidth = width * scale;
    const float expectedHeight = height * scale;
    const ImageDraw& draw = value.front();
    check(std::fabs(draw.maximum.x - draw.minimum.x - expectedWidth) < 0.1f &&
          std::fabs(draw.maximum.y - draw.minimum.y - expectedHeight) < 0.1f,
          "Wallpaper must preserve its aspect ratio");
    check(std::fabs((draw.minimum.x + draw.maximum.x) * 0.5f - position.x - size.x * 0.5f) < 0.1f &&
          std::fabs((draw.minimum.y + draw.maximum.y) * 0.5f - position.y - size.y * 0.5f) < 0.1f,
          "Wallpaper must be centered in the requested viewport");
    check(draw.minimum.x >= position.x - 0.1f && draw.minimum.y >= position.y - 0.1f &&
          draw.maximum.x <= position.x + size.x + 0.1f &&
          draw.maximum.y <= position.y + size.y + 0.1f,
          "Portrait must fit fully inside the viewport");
}

bool near(float left, float right) { return std::fabs(left - right) < 0.001f; }

void checkSizing(const ImageDraw& draw, ImVec2 size, ImVec2 position,
                 OS::WallpaperSizing sizing) {
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(draw.texture));
    GLint width = 0, height = 0;
    getTextureParameter(GL_TEXTURE_2D, 0, 0x1000, &width);
    getTextureParameter(GL_TEXTURE_2D, 0, 0x1001, &height);
    const float drawnWidth = draw.maximum.x - draw.minimum.x;
    const float drawnHeight = draw.maximum.y - draw.minimum.y;
    const float uvWidth = draw.uvMaximum.x - draw.uvMinimum.x;
    const float uvHeight = draw.uvMaximum.y - draw.uvMinimum.y;
    check(width > 0 && height > 0 && uvWidth > 0 && uvHeight > 0,
          "Sizing must use a non-empty image region");
    check(draw.minimum.x >= position.x - 0.1f && draw.minimum.y >= position.y - 0.1f &&
          draw.maximum.x <= position.x + size.x + 0.1f &&
          draw.maximum.y <= position.y + size.y + 0.1f,
          "Sizing must not draw outside the visible desktop");
    check(near(draw.uvMinimum.x + draw.uvMaximum.x, 1) &&
          near(draw.uvMinimum.y + draw.uvMaximum.y, 1),
          "Image regions must remain centered");
    if (sizing != OS::WallpaperSizing::FIT) {
        check(near(draw.minimum.x, position.x) && near(draw.minimum.y, position.y) &&
              near(drawnWidth, size.x) && near(drawnHeight, size.y),
              "Crop and Stretch must fill the desktop without margins");
    }
    if (sizing == OS::WallpaperSizing::CROP) {
        check(draw.uvMinimum.x >= 0 && draw.uvMinimum.y >= 0 &&
              draw.uvMaximum.x <= 1 && draw.uvMaximum.y <= 1 &&
              (near(uvWidth, 1) || near(uvHeight, 1)),
              "Crop must retain a full source axis without repeating pixels");
        check(near(drawnWidth / (width * uvWidth), drawnHeight / (height * uvHeight)),
              "Crop must preserve the source image's aspect ratio");
        const bool wider = size.x / size.y > static_cast<float>(width) / height;
        check(wider ? near(uvWidth, 1) && uvHeight < 1 : near(uvHeight, 1) && uvWidth < 1,
              "Crop must trim the correct source axis for wide and tall desktops");
    } else {
        check(near(draw.uvMinimum.x, 0) && near(draw.uvMinimum.y, 0) &&
              near(draw.uvMaximum.x, 1) && near(draw.uvMaximum.y, 1),
              "Fit and Stretch must retain the whole source image");
        if (sizing == OS::WallpaperSizing::FIT) {
            check(near(drawnWidth / width, drawnHeight / height),
                  "Fit must preserve the source image's aspect ratio");
        }
    }
}

void sizingChecks(const std::array<ImTextureID, 4>& imageTextures) {
    const OS::WallpaperSizing modes[] = {OS::WallpaperSizing::FIT,
        OS::WallpaperSizing::CROP, OS::WallpaperSizing::STRETCH};
    for (OS::WallpaperSizing mode : modes) {
        for (ImVec2 size : {ImVec2(1280, 756), ImVec2(640, 436), ImVec2(300, 600)}) {
            const ImVec2 position(30, 20);
            const Frame value = atTime(0, size, position, mode);
            checkOnly(value, imageTextures[0], "Sizing must retain the current wallpaper");
            checkSizing(value.front(), size, position, mode);
        }
        checkIncoming(atTime(4.5f, ImVec2(1280, 756), ImVec2(0, 0), mode),
                      imageTextures[0], imageTextures[1], 128,
                      "Every sizing mode must crossfade with an opaque base");
        checkIncoming(atTime(19.5f, ImVec2(300, 600), ImVec2(0, 0), mode),
                      imageTextures[3], imageTextures[0], 128,
                      "Every sizing mode must crossfade the loop boundary");
    }
    atTime(7);
    checkOnly(frame(0, ImVec2(1280, 756), ImVec2(0, 0), OS::WallpaperSizing::CROP),
              imageTextures[1], "Changing to Crop must not restart the slideshow");
    checkOnly(frame(0, ImVec2(1280, 756), ImVec2(0, 0), OS::WallpaperSizing::STRETCH),
              imageTextures[1], "Changing to Stretch must not restart the slideshow");

    // Exercise the real desktop integration, including the taskbar's reserved area.
    OS::SystemContext context;
    context.setState(OS::BootState::DESKTOP);
    for (OS::WallpaperSizing mode : modes) {
        context.setWallpaperSizing(mode);
        for (ImVec2 size : {ImVec2(1280, 800), ImVec2(640, 480)}) {
            ImGuiIO& io = ImGui::GetIO();
            for (int pass = 0; pass < 2; ++pass) {
                OS::WallpaperSlideshow::reset();
                ImGui_ImplOpenGL3_NewFrame();
                io.DisplaySize = size;
                io.DeltaTime = 1.0f / 60.0f;
                ImGui::NewFrame();
                OS::DesktopBackground::render(context, 0);
                OS::Taskbar::render(context, 0);
                ImGui::Render();
            }
            bool found = false;
            const ImDrawData* data = ImGui::GetDrawData();
            for (int list = 0; list < data->CmdListsCount; ++list) {
                for (const ImageDraw& draw : imageDraws(data->CmdLists[list])) {
                    if (draw.texture != imageTextures[0]) continue;
                    found = true;
                    checkSizing(draw, ImVec2(size.x, size.y - OS::Ui::TaskbarHeight),
                                ImVec2(0, 0), mode);
                }
            }
            check(found, "Desktop must submit the selected sizing mode's wallpaper");
        }
    }
}

void writePreview(const std::filesystem::path& path) {
    OS::WallpaperSlideshow::reset();
    OS::SystemContext context;
    context.setState(OS::BootState::DESKTOP);
    // Two frames settle the newly created desktop/taskbar layout without a
    // platform backend or any persisted imgui.ini file.
    for (int pass = 0; pass < 2; ++pass) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(1280, 800);
        io.DisplayFramebufferScale = ImVec2(1, 1);
        io.DeltaTime = 1.0f / 60.0f;
        ImGui::NewFrame();
        OS::DesktopBackground::render(context, 0);
        OS::Taskbar::render(context, 0);
        ImGui::Render();
    }
    glViewport(0, 0, 1280, 800);
    glClearColor(0.05f, 0.08f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    finish();
    const int width = 1280, height = 800;
    const std::uint32_t rowBytes = static_cast<std::uint32_t>((width * 3 + 3) & ~3);
    std::vector<unsigned char> pixels(rowBytes * height);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    readPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    check(glGetError() == GL_NO_ERROR, "Native wallpaper rendering must succeed");
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            std::swap(pixels[y * rowBytes + x * 3], pixels[y * rowBytes + x * 3 + 2]);
        }
    }
    std::ofstream output(path, std::ios::binary);
    check(output.is_open(), "Could not create native wallpaper preview");
    const auto word = [&output](std::uint16_t value) {
        output.put(static_cast<char>(value & 255));
        output.put(static_cast<char>((value >> 8) & 255));
    };
    const auto dword = [&output](std::uint32_t value) {
        for (int shift = 0; shift < 32; shift += 8) output.put(static_cast<char>((value >> shift) & 255));
    };
    output.put('B'); output.put('M');
    dword(54 + static_cast<std::uint32_t>(pixels.size()));
    word(0); word(0); dword(54); dword(40);
    dword(width); dword(height); word(1); word(24);
    dword(0); dword(static_cast<std::uint32_t>(pixels.size()));
    dword(2835); dword(2835); dword(0); dword(0);
    output.write(reinterpret_cast<const char*>(pixels.data()), static_cast<std::streamsize>(pixels.size()));
    check(output.good(), "Native wallpaper preview was not completely written");
}

void slideshowChecks(const std::filesystem::path& assets, const std::filesystem::path& preview) {
    OS::WallpaperSlideshow::initialize(assets / "wallpapers");
    check(OS::WallpaperSlideshow::imageCount() == 4, "All four supplied wallpapers must load");
    std::array<ImTextureID, 4> imageTextures{};
    for (std::size_t index = 0; index < imageTextures.size(); ++index) {
        const Frame value = atTime(static_cast<float>(index * 5));
        check(textures(value).size() == 1, "Each hold interval must use one image");
        imageTextures[index] = value.front().texture;
        check(isTexture(static_cast<GLuint>(imageTextures[index])) == GL_TRUE,
              "Each loaded image must have a live OpenGL texture");
    }
    check(std::set<ImTextureID>(imageTextures.begin(), imageTextures.end()).size() == 4,
          "Supplied wallpapers must use four distinct textures");
    for (float elapsed : {0.0f, 1.0f, 3.99f, 4.0f}) {
        checkOnly(atTime(elapsed), imageTextures[0], "First image must hold for four seconds");
    }
    checkIncoming(atTime(4.25f), imageTextures[0], imageTextures[1], 40, "Fade must ease in");
    checkIncoming(atTime(4.5f), imageTextures[0], imageTextures[1], 128, "Midpoint must blend images");
    checkIncoming(atTime(4.75f), imageTextures[0], imageTextures[1], 215, "Fade must ease out");
    checkOnly(atTime(5.0f), imageTextures[1], "Next image must become fully opaque");
    checkIncoming(atTime(19.5f), imageTextures[3], imageTextures[0], 128, "Last image must fade to first");
    checkOnly(atTime(20.0f), imageTextures[0], "Slideshow must loop after twenty seconds");

    checkGeometry(imageTextures[0], ImVec2(1280, 800), ImVec2(0, 0));
    checkGeometry(imageTextures[0], ImVec2(640, 480), ImVec2(30, 20));
    checkGeometry(imageTextures[0], ImVec2(300, 600), ImVec2(100, 50));
    sizingChecks(imageTextures);

    atTime(12.0f);
    OS::WallpaperSlideshow::reset();
    checkOnly(frame(0), imageTextures[0], "Reset must restart at the first image");
    atTime(7.0f);
    for (float delta : {-1.0f, std::numeric_limits<float>::infinity(),
                        std::numeric_limits<float>::quiet_NaN()}) {
        frame(delta);
        checkOnly(frame(0), imageTextures[1], "Invalid delta must not advance the slideshow");
    }
    checkOnly(atTime(200005.0f), imageTextures[1], "Large frame gaps must wrap correctly");
    const float largest = std::numeric_limits<float>::max();
    const double remainder = std::fmod(static_cast<double>(largest), 20.0);
    const Frame expected = atTime(static_cast<float>(remainder));
    const Frame actual = atTime(largest);
    check(textures(actual) == textures(expected), "Largest finite delta must remain in the cycle");
    check(frame(0, ImVec2(0, 0)).empty(), "Zero viewport size must not draw images");
    if (!preview.empty()) writePreview(preview);

    OS::WallpaperSlideshow::shutdown();
    check(OS::WallpaperSlideshow::imageCount() == 0, "Shutdown must release all wallpapers");
    for (ImTextureID texture : imageTextures) {
        check(isTexture(static_cast<GLuint>(texture)) == GL_FALSE, "Shutdown must delete each GL texture");
    }
    check(frame(0).empty(), "An uninitialized slideshow must not emit image draws");
    check(glGetError() == GL_NO_ERROR, "Slideshow checks must not produce OpenGL errors");
}
} // namespace

int main(int argc, char** argv) {
    GLFWwindow* window = nullptr;
    bool rendererReady = false;
    bool glReady = false;
    try {
        check(argc >= 2, "Expected project assets directory as the first argument");
        if (glfwInit() != GLFW_TRUE) {
            const char* description = nullptr;
            const int error = glfwGetError(&description);
            if (unavailableNativeContext(error)) {
                std::cout << "Native media checks skipped: GLFW/display unavailable: "
                          << (description ? description : "unknown native platform error") << '\n';
                glfwTerminate();
                return 77;
            }
            check(false, "Could not initialize GLFW for the native media checks");
        }
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(__APPLE__)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
        window = glfwCreateWindow(1280, 800, "Hidden wallpaper regression checks", nullptr, nullptr);
        if (!window) {
            const char* description = nullptr;
            const int error = glfwGetError(&description);
            if (unavailableNativeContext(error)) {
                std::cout << "Native media checks skipped: OpenGL 3.3/GPU unavailable: "
                          << (description ? description : "unknown native context error") << '\n';
                glfwTerminate();
                return 77;
            }
            check(false, "Could not create a hidden OpenGL 3.3 window");
        }
        glfwMakeContextCurrent(window);
        check(gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) != 0,
              "Could not initialize GLAD for the media checks");
        glReady = true;
        getTextureParameter = reinterpret_cast<GetTextureParameter>(glfwGetProcAddress("glGetTexLevelParameteriv"));
        isTexture = reinterpret_cast<IsTexture>(glfwGetProcAddress("glIsTexture"));
        readPixels = reinterpret_cast<ReadPixels>(glfwGetProcAddress("glReadPixels"));
        finish = reinterpret_cast<Finish>(glfwGetProcAddress("glFinish"));
        check(getTextureParameter && isTexture && readPixels && finish,
              "Could not load the native texture/framebuffer inspection functions");
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().IniFilename = nullptr;
        rendererReady = ImGui_ImplOpenGL3_Init("#version 330 core");
        check(rendererReady, "Could not initialize the ImGui native renderer");
        const std::filesystem::path preview = argc >= 3 ? std::filesystem::path(argv[2]) : std::filesystem::path{};
        slideshowChecks(std::filesystem::path(argv[1]), preview);
        ImGui_ImplOpenGL3_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        std::cout << checks << " native wallpaper loading/timing/geometry checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Media regression failure: " << error.what() << '\n';
        if (glReady) OS::WallpaperSlideshow::shutdown();
        if (rendererReady) ImGui_ImplOpenGL3_Shutdown();
        if (ImGui::GetCurrentContext()) ImGui::DestroyContext();
        if (window) glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
}
