#include "BootAudio.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include <algorithm>
#include <iostream>
#include <system_error>

namespace OS {

struct BootAudio::Impl {
    ma_engine engine{};
    ma_sound sound{};
    bool engineReady = false;
    bool soundReady = false;

    ~Impl() {
        // Sounds refer to their engine, so release the sound first.
        if (soundReady) {
            ma_sound_uninit(&sound);
        }
        if (engineReady) {
            ma_engine_uninit(&engine);
        }
    }
};

BootAudio::BootAudio() = default;
BootAudio::~BootAudio() = default;

bool BootAudio::initialize(const std::filesystem::path& soundPath) {
    shutdown();

    std::error_code error;
    if (!std::filesystem::is_regular_file(soundPath, error)) {
        std::cerr << "[Boot audio] Startup sound is unavailable: "
                  << soundPath.u8string() << '\n';
        return false;
    }

    m_impl = std::make_unique<Impl>();
    ma_result result = ma_engine_init(nullptr, &m_impl->engine);
    if (result != MA_SUCCESS) {
        std::cerr << "[Boot audio] Cannot initialize the playback device: "
                  << ma_result_description(result) << '\n';
        m_impl.reset();
        return false;
    }
    m_impl->engineReady = true;

    // miniaudio can fall back to a silent backend when no hardware is usable.
    // Report that case too, while letting the desktop continue normally.
    ma_device* device = ma_engine_get_device(&m_impl->engine);
    if (!device || !device->pContext || device->pContext->backend == ma_backend_null) {
        std::cerr << "[Boot audio] No playback device is available.\n";
        m_impl.reset();
        return false;
    }

    // Decode this short effect once before the frame loop. Starting it later
    // only schedules playback and never waits for the clip to finish.
    const ma_uint32 flags = MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION;
#if defined(_WIN32)
    result = ma_sound_init_from_file_w(&m_impl->engine, soundPath.c_str(), flags,
                                       nullptr, nullptr, &m_impl->sound);
#else
    result = ma_sound_init_from_file(&m_impl->engine, soundPath.c_str(), flags,
                                     nullptr, nullptr, &m_impl->sound);
#endif
    if (result != MA_SUCCESS) {
        std::cerr << "[Boot audio] Cannot load the startup sound: "
                  << ma_result_description(result) << '\n';
        m_impl.reset();
        return false;
    }
    m_impl->soundReady = true;
    ma_sound_set_looping(&m_impl->sound, MA_FALSE);
    return true;
}

void BootAudio::update(BootState state, int volumeLevel, bool isMuted) {
    const bool rebootStarted = state == BootState::BIOS_POST &&
                               m_previousState != BootState::BIOS_POST;
    if (rebootStarted) {
        m_playedThisBoot = false;
        if (m_impl) {
            ma_sound_stop(&m_impl->sound);
            ma_sound_seek_to_pcm_frame(&m_impl->sound, 0);
        }
    }
    m_previousState = state;

    if (m_impl) {
        const float gain = isMuted ? 0.0f :
            static_cast<float>(std::clamp(volumeLevel, 0, 100)) / 100.0f;
        ma_sound_set_volume(&m_impl->sound, gain);
        if (state == BootState::SHUTTING_DOWN) {
            ma_sound_stop(&m_impl->sound);
        }
    }

    // DESKTOP covers skipping directly past boot screens. Returning from
    // the splash to the desktop must not start the same sound a second time.
    if (!m_playedThisBoot &&
        (state == BootState::BOOT_SPLASH || state == BootState::DESKTOP)) {
        m_playedThisBoot = true;
        if (m_impl) {
            ma_result result = ma_sound_seek_to_pcm_frame(&m_impl->sound, 0);
            if (result == MA_SUCCESS) {
                result = ma_sound_start(&m_impl->sound);
            }
            if (result != MA_SUCCESS) {
                std::cerr << "[Boot audio] Cannot play the startup sound: "
                          << ma_result_description(result) << '\n';
            }
        }
    }
}

void BootAudio::shutdown() {
    m_impl.reset();
    m_previousState = BootState::BIOS_POST;
    m_playedThisBoot = false;
}

} // namespace OS
