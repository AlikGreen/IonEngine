#include "sound.h"

#include <utility>

namespace ion
{
    void Sound::play() const
    {
        ma_sound_start(m_sound);
    }

    void Sound::stop() const
    {
        ma_sound_stop(m_sound);
    }

    void Sound::setLoop(const bool loop) const
    {
        ma_sound_set_looping(m_sound, loop);
    }

    void Sound::setPosition(const glm::vec3 position) const
    {
        ma_sound_set_position(m_sound, position.x, position.y, position.z);
    }

    void Sound::setPitch(const float pitch) const
    {
        ma_sound_set_pitch(m_sound, pitch);
    }

    void Sound::setVolume(const float volume) const
    {
        ma_sound_set_volume(m_sound, volume);
    }

    Sound::Sound(ma_sound* sound, ma_audio_buffer* audioBuffer)
        : m_sound(sound), m_audioBuffer(audioBuffer)
    {  }
}
