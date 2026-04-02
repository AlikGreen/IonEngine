#pragma once

#include <urhi/urhi.h>

namespace ion
{
class TextureData
{
public:
  uint32_t width, height;
  urhi::PixelFormat pixelFormat;
  std::vector<uint8_t> data;
};
}
