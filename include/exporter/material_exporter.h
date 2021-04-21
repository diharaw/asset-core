#pragma once

#include <common/material.h>
#include <common/image.h>

namespace ast
{
struct MaterialExportOptions
{
    std::string output_root_folder_path_absolute;
    bool        use_compression       = true;
    bool        normal_map_flip_green = false;
};

extern bool export_material(const Material& desc, const MaterialExportOptions& options);
} // namespace ast
