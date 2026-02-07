#include "audioManager.h"

#include <miniaudio.h>

#include <clogr.h>

namespace ion
{
    AudioManager::AudioManager()
    {
        ma_result result = ma_engine_init(nullptr, &m_engine);
        clogr::ensure(result == MA_SUCCESS, "Failed to init miniaufio.");
    }

    Sound AudioManager::createSound(const AudioClip &clip)
    {
        const ma_audio_buffer_config config = ma_audio_buffer_config_init(
            ma_format_f32,
            clip.channels,
            clip.totalFrames,
            clip.data,
            nullptr
        );

        auto* audioBuffer = new ma_audio_buffer();
        ma_result result = ma_audio_buffer_init(&config, audioBuffer);
        clogr::ensure(result == MA_SUCCESS, "Failed to init audio buffer.");

        auto* sound = new ma_sound();
        result = ma_sound_init_from_data_source(
            &m_engine,
            audioBuffer,
            0, // ASYNC?
            nullptr,
            sound
        );
        clogr::ensure(result == MA_SUCCESS, "Failed to init sound.");

        return {sound, audioBuffer};
    }

    void AudioManager::shutdown()
    {
        ma_engine_uninit(&m_engine);
    }
}
