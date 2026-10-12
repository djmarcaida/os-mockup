#include "WallpaperSlideshow.h"

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_WINDOWS_UTF8
#include <stb_image.h>

#include <algorithm>
#include <cmath>
#include <iostream>

namespace OS {

std::vector<WallpaperSlideshow::Image> WallpaperSlideshow::s_images;
double WallpaperSlideshow::s_elapsed = 0.0;

void WallpaperSlideshow::initialize(const std::filesystem::path& wallpaperDirectory) {
    shutdown();

    // Load once at startup, never during a frame. Missing images are skipped.
    const char* filenames[] = {"bryan2x2.jpg", "my2x2.jpg", "duncan2x2.jpg", "kan2x2.png"};
    GLint previousTexture = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);
    for (const char* filename : filenames) {
        const auto path = wallpaperDirectory / filename;
        Image image;
        int channels = 0;
        unsigned char* pixels = stbi_load(path.u8string().c_str(), &image.width, &image.height,
                                         &channels, STBI_rgb_alpha);
        if (!pixels) {
            std::cerr << "[Wallpaper] Could not load " << path << ": " << stbi_failure_reason() << '\n';
            continue;
        }

        glGenTextures(1, &image.texture);
        glBindTexture(GL_TEXTURE_2D, image.texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.width, image.height,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        stbi_image_free(pixels);
        s_images.push_back(image);
    }
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousTexture));
}

void WallpaperSlideshow::shutdown() {
    for (const Image& image : s_images) glDeleteTextures(1, &image.texture);
    s_images.clear();
    reset();
}

void WallpaperSlideshow::reset() {
    s_elapsed = 0.0;
}

void WallpaperSlideshow::drawImage(ImDrawList* drawList, const Image& image,
                                   ImVec2 position, ImVec2 size, float opacity) {
    if (opacity <= 0.0f) return;
    // Contain instead of stretching or cropping the portraits.
    const float scale = std::min(size.x / image.width, size.y / image.height);
    const ImVec2 imageSize(image.width * scale, image.height * scale);
    const ImVec2 minimum(position.x + (size.x - imageSize.x) * 0.5f,
                         position.y + (size.y - imageSize.y) * 0.5f);
    const ImVec2 maximum(minimum.x + imageSize.x, minimum.y + imageSize.y);
    drawList->AddImage(static_cast<ImTextureID>(image.texture), minimum, maximum,
                       ImVec2(0, 0), ImVec2(1, 1),
                       IM_COL32(255, 255, 255, static_cast<int>(255.0f * opacity + 0.5f)));
}

void WallpaperSlideshow::render(ImDrawList* drawList, ImVec2 position, ImVec2 size, float deltaTime) {
    if (s_images.empty() || size.x <= 0.0f || size.y <= 0.0f) return;
    if (s_images.size() == 1) {
        drawImage(drawList, s_images.front(), position, size, 1.0f);
        return;
    }
    constexpr double interval = HoldSeconds + FadeSeconds;
    const double cycle = interval * static_cast<double>(s_images.size());
    const std::size_t index = static_cast<std::size_t>(s_elapsed / interval);
    const double imageTime = std::fmod(s_elapsed, interval);
    float blend = static_cast<float>(std::max(0.0, (imageTime - HoldSeconds) / FadeSeconds));
    blend = blend * blend * (3.0f - 2.0f * blend); // Gentle start/end to the fade.

    drawImage(drawList, s_images[index], position, size, 1.0f - blend);
    if (blend > 0.0f) {
        const Image& next = s_images[(index + 1) % s_images.size()];
        // Keep the overlap opaque under the incoming layer for a true crossfade.
        // Only the outgoing image's non-overlapping edges fade toward the theme.
        const float nextScale = std::min(size.x / next.width, size.y / next.height);
        const ImVec2 nextSize(next.width * nextScale, next.height * nextScale);
        const ImVec2 nextMin(position.x + (size.x - nextSize.x) * 0.5f,
                             position.y + (size.y - nextSize.y) * 0.5f);
        drawList->PushClipRect(nextMin, ImVec2(nextMin.x + nextSize.x, nextMin.y + nextSize.y), true);
        drawImage(drawList, s_images[index], position, size, 1.0f);
        drawList->PopClipRect();
        drawImage(drawList, next, position, size, blend);
    }

    // Large frame gaps still advance/loop correctly.
    if (std::isfinite(deltaTime) && deltaTime > 0.0f) {
        s_elapsed = std::fmod(s_elapsed + static_cast<double>(deltaTime), cycle);
    }
}

} // namespace OS
