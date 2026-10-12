#pragma once

#include "../Core/Types.h"
#include <filesystem>
#include <memory>

namespace OS {

// Owns one startup sound; miniaudio plays it on its audio thread.
class BootAudio {
public:
    BootAudio();
    ~BootAudio();

    BootAudio(const BootAudio&) = delete;
    BootAudio& operator=(const BootAudio&) = delete;

    bool initialize(const std::filesystem::path& soundPath);
    void update(BootState state, int volumeLevel, bool isMuted);
    void shutdown();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    BootState m_previousState = BootState::BIOS_POST;
    bool m_playedThisBoot = false;
};

} // namespace OS
