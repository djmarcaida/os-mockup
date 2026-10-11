#pragma once

#include "../Core/SystemContext.h"

namespace OS {

class Taskbar {
public:
    static void render(SystemContext& ctx, float deltaTime);

private:
    static void renderStartMenu(SystemContext& ctx);
};

} // namespace OS
