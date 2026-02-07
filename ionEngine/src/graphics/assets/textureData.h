#pragma once
#include <neonRHI/neonRHI.h>

namespace ion
{
class TextureData
{
public:
  uint32_t width, height;
  urhi::PixelLayout pixelLayout;
  urhi::PixelType pixelType;
  void* data;
};
}
