#pragma once

#include <common/asset_desc.h>

namespace ast
{
    extern bool import_mesh(const std::string& path, MeshDesc& desc);
    extern bool import_image(const std::string& path, ImageDesc& desc);
}
