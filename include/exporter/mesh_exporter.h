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
        std::string texture_source_path = "";
        bool use_compression = true;
		bool normal_map_flip_green = false;
        bool        is_orca_mesh           = false;
        bool output_metadata = false;
    };
    
    extern bool export_mesh(const Mesh& desc, const MeshExportOption& options);
}
