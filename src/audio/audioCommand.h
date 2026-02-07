#pragma once
#include "audioClip.h"
#include "soundHandle.h"

namespace ion
{
struct AudioCommand
{
    enum Type { Play, Stop } type;
    AudioClip* clip{};
    SoundHandle handle;
    float speed = 1.0f;
    float volume = 1.0f;
    float pitch = 1.0f;
    bool loop = true;
};
}
