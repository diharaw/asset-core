#pragma once

#include <common/material.h>
#include <common/image.h>

namespace ast
{
    struct MaterialExportOptions
    {
        std::string path;
        std::string texture_source_path = "";
        std::string relative_texture_path = "";
        std::string dst_texture_path = "";
        bool use_compression = true;
		bool normal_map_flip_green = false;
        std::string vertex_func_id;
        std::vector<std::string> vertex_func;
        std::string fragment_func_id;
        std::vector<std::string> fragment_func;
    };
    
    extern bool export_material(const Material& desc, const MaterialExportOptions& options);
}
