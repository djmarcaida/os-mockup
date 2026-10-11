#pragma once

#include "../Core/SystemContext.h"

namespace OS {

class SystemInfoWindow {
public:
    static void render(SystemContext& ctx, float deltaTime);

    // Apply color schemes to ImGui
    static void applyThemeToImGui(ThemeMode mode);
};

} // namespace OS
