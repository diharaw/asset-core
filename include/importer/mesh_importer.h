#pragma once

#include <common/mesh.h>

namespace ast
{
	struct MeshImportOptions
	{
		bool displacement_as_normal = false;
	};

    extern bool import_mesh(const std::string& file, Mesh& mesh, MeshImportOptions options = MeshImportOptions());
        }
