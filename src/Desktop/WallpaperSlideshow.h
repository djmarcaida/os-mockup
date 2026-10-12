#pragma once

#include <imgui.h>
#include <cstddef>
#include <filesystem>
#include <vector>

namespace OS {

class WallpaperSlideshow {
public:
    static constexpr double HoldSeconds = 4.0;
    static constexpr double FadeSeconds = 1.0;

    // Texture lifetime must stay inside the lifetime of the OpenGL context.
    static void initialize(const std::filesystem::path& wallpaperDirectory);
    static void shutdown();
    static void reset();
    static void render(ImDrawList* drawList, ImVec2 position, ImVec2 size, float deltaTime);

    static std::size_t imageCount() { return s_images.size(); }

private:
    struct Image {
        unsigned int texture = 0;
        int width = 0;
        int height = 0;
    };
    static void drawImage(ImDrawList* drawList, const Image& image,
                          ImVec2 position, ImVec2 size, float opacity);
    static std::vector<Image> s_images;
    static double s_elapsed;
};

} // namespace OS
