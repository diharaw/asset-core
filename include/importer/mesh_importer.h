#pragma once

#include <common/mesh.h>

namespace ast
{
struct MeshImportOptions
{
    bool displacement_as_normal = false;
    bool is_orca_mesh           = false;
    bool force_disney_materials = false;
};

extern bool import_mesh(const std::string& file, Mesh& mesh, MeshImportOptions options = MeshImportOptions());
} // namespace ast