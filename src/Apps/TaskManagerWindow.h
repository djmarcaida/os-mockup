#pragma once

#include "../Core/SystemContext.h"

namespace OS {

class TaskManagerWindow {
public:
    static void render(SystemContext& ctx, float deltaTime);

private:
    static int s_selectedPid;
    static char s_searchBuffer[64];
    static std::string s_statusMessage;
    static float s_statusTimer;
};

} // namespace OS
