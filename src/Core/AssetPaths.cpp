#include "AssetPaths.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <vector>
#endif

namespace OS {

std::filesystem::path getAssetDirectory(const char* executableArgument) {
    std::filesystem::path executable;
#if defined(_WIN32)
    wchar_t buffer[32768];
    const DWORD length = GetModuleFileNameW(nullptr, buffer, 32768);
    if (length > 0 && length < 32768) executable.assign(std::wstring(buffer, length));
#elif defined(__APPLE__)
    uint32_t length = 0;
    _NSGetExecutablePath(nullptr, &length);
    std::vector<char> buffer(length);
    if (_NSGetExecutablePath(buffer.data(), &length) == 0) executable.assign(buffer.data());
#elif defined(__linux__)
    std::error_code error;
    executable = std::filesystem::read_symlink("/proc/self/exe", error);
    if (error) executable.clear();
#endif
    if (executable.empty() && executableArgument && *executableArgument) {
        std::error_code error;
        executable = std::filesystem::absolute(executableArgument, error);
        if (error) executable.clear();
    }
    return executable.empty() ? std::filesystem::path("assets") : executable.parent_path() / "assets";
}

} // namespace OS
