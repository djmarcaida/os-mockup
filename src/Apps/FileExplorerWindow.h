#pragma once

#include "../Core/SystemContext.h"

namespace OS {

class FileExplorerWindow {
public:
    static void render(SystemContext& ctx, float deltaTime);

private:
    static void renderTreeView(SystemContext& ctx, const VirtualFile* dir);
    static void renderFileList(SystemContext& ctx);
    static const VirtualFile* s_selectedFile;
};

} // namespace OS
