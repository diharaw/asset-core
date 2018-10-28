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
        CompressionType compression = COMPRESSION_NONE;
    };
    
    extern bool export_material(const Material& desc, const MaterialExportOptions& options);
}
