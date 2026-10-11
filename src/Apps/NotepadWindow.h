#pragma once

#include "../Core/SystemContext.h"

namespace OS {

class NotepadWindow {
public:
    static void render(SystemContext& ctx, float deltaTime);
};

} // namespace OS
