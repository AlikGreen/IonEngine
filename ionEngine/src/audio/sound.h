#pragma once
#include "miniaudio.h"
#include "glm/vec3.hpp"

namespace ion
{
class Sound
{
public:
    void play() const;
    void stop() const;

    void setLoop(bool loop) const;
    void setPosition(glm::vec3 position) const;
    void setPitch(float pitch) const;
    void setVolume(float volume) const;
private:
    friend class AudioManager;
    Sound(ma_sound* sound, ma_audio_buffer* audioBuffer);
    ma_sound* m_sound;
    ma_audio_buffer*  m_audioBuffer;
};
}
