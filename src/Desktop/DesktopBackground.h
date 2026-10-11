#pragma once

#include "../Core/SystemContext.h"

namespace OS {

class DesktopBackground {
public:
    static void render(SystemContext& ctx, float deltaTime);
};

} // namespace OS
