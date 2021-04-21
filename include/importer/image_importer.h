#pragma once

#include <common/image.h>

namespace ast
{
extern bool import_image(Image& img, const std::string& file, const PixelType& type = PIXEL_TYPE_UNORM8, int force_cmp = 0);
}
