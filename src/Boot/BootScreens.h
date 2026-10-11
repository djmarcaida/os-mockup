#pragma once

#include "../Core/SystemContext.h"

namespace OS {

class BootScreens {
public:
    static void renderBiosPost(SystemContext& ctx, float deltaTime);
    static void renderBootSplash(SystemContext& ctx, float deltaTime);
};

} // namespace OS
