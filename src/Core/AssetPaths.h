#pragma once

#include <filesystem>

namespace OS {

// CMake places assets beside the executable, independent of the launch directory.
std::filesystem::path getAssetDirectory(const char* executableArgument);

} // namespace OS
