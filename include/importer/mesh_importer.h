#pragma once

#include <common/mesh.h>

namespace ast
{
struct MeshImportOptions
{
    bool displacement_as_normal = false;
    bool is_orca_mesh           = false;
};

extern bool import_mesh(const std::string& file, MeshImportResult& import_result, MeshImportOptions options = MeshImportOptions());
} // namespace ast