#pragma once

#include <common/mesh.h>
#include <common/image.h>

namespace ast
{
    struct MeshExportOption
    {
        std::string path;
        std::string relative_texture_path = "";
        std::string relative_material_path = "";
        CompressionType compression = COMPRESSION_NONE;
    };
    
    extern bool export_mesh(const Mesh& desc, const MeshExportOption& options);
}
