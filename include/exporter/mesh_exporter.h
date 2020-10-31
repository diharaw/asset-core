#pragma once

#include <common/mesh.h>
#include <common/image.h>

namespace ast
{
    struct MeshExportOption
    {
        std::string output_root_folder_path;
        bool use_compression = true;
		bool normal_map_flip_green = false;
        bool output_metadata = false;
    };
    
    extern bool export_mesh(const Mesh& desc, const MeshExportOption& options);
}
