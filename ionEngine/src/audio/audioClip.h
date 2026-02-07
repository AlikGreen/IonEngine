#pragma once

namespace ion
{
struct AudioClip
{
   void* data{};
   uint64_t totalFrames = 0;
   uint32_t channels = 0;
   float sampleRate = 0;
};
}
