#pragma once
#include "asset/assetRef.h"
#include "audio/audioClip.h"

namespace ion
{
struct AudioSource
{
    AssetRef<AudioClip> clip;
    float pitch = 1.0f;
    float volume = 1.0f;
    bool loop = false;

    void play() { isPlaying = true; };
    void stop() { isPlaying  = false; };
private:
    friend class AudioSystem;
    bool isPlaying = false;
};
}