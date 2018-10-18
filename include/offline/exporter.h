#pragma once

#include <common/asset_desc.h>

namespace ast
{
    extern bool export_material(const std::string& path, const MaterialDesc& desc);
    extern bool export_mesh(const std::string& path, const MeshDesc& desc);
    extern bool export_image(const std::string& path, const ImageDesc& desc);
}
