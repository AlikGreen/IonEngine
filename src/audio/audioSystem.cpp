#include "audioSystem.h"

#include "audioManager.h"
#include "components/audioSource.h"
#include "core/engine.h"
#include "core/sceneManager.h"
#include "core/components/transformComponent.h"
#include "entis/entis.h"

namespace ion
{
    void AudioSystem::postUpdate(Scene& scene)
    {
        auto& audioSources = scene.registry().view<AudioSource, Transform>();

        AudioManager& audioManager = Engine::audioManager();

        for (const auto& [entity, audioSource, transform] : audioSources)
        {
            const bool isPlaying = sounds.contains(audioSource.clip.id());
            if(audioSource.isPlaying && !isPlaying)
            {
                Sound sound = audioManager.createSound(*audioSource.clip);
                sound.play();
                sounds.emplace(audioSource.clip.id(), sound);
            }
            if(audioSource.isPlaying && isPlaying)
            {
                Sound& sound = sounds.at(audioSource.clip.id());
                sound.setLoop(audioSource.loop);
                sound.setVolume(audioSource.volume);
                sound.setPitch(audioSource.pitch);
                sound.setPosition(transform.position());
            }
            if(!audioSource.isPlaying && isPlaying)
            {
                Sound sound = audioManager.createSound(*audioSource.clip);
                sound.stop();
                sounds.erase(audioSource.clip.id());
            }
        }
    }
}
