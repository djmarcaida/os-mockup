#pragma once

#include "../Core/SystemContext.h"

namespace OS {

class WindowCompositor {
public:
    static void render(SystemContext& ctx, float deltaTime);

private:
    static void handleShortcuts(SystemContext& ctx);
    static void renderPowerDialog(SystemContext& ctx);
};

} // namespace OS
