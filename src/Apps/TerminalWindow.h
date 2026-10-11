#pragma once

#include "../Core/SystemContext.h"

namespace OS {

class TerminalWindow {
public:
    static void render(SystemContext& ctx, float deltaTime);

private:
    static char s_inputBuffer[256];
    static bool s_scrollToBottom;
    static int s_historyIndex;
    static std::vector<std::string> s_localHistory;
};

} // namespace OS
