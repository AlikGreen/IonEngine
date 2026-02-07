#include "audioClipImporter.h"

#include "audioClip.h"
#include <clogr.h>
#include "miniaudio.h"

namespace ion
{
    void* AudioClipImporter::load(const std::string &filepath)
    {
        ma_decoder_config config = ma_decoder_config_init(
            ma_format_f32,
            2,
            44100
        );

        void* pData = nullptr;
        ma_uint64 frameCount = 0;

        const ma_result result = ma_decode_file(filepath.c_str(), &config, &frameCount, &pData);
        clogr::ensure(result == MA_SUCCESS, "Failed to load audio: {}", filepath);

        auto* clip = new AudioClip();
        clip->data = pData;
        clip->totalFrames = frameCount;
        clip->channels = 2;
        clip->sampleRate = 44100;

        return clip;
    }
}
