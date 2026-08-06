#pragma once

namespace ion
{
class TextureData
{
public:
  uint32_t width, height;
  dg::TEXTURE_FORMAT pixelFormat;
  std::vector<uint8_t> data;
};
}
