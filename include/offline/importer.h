#pragma once

#include <common/asset_desc.h>

namespace ast
{
    extern bool import_mesh(const std::string& path, MeshDesc& desc);
    extern bool import_texture(const std::string& path, const PixelType& pixel_type, ImageDesc& image);
}
