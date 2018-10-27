#pragma once

#include <common/mesh.h>

namespace ast
{
    enum ImportSource
    {
        IMPORT_SOURCE_MEMORY,
        IMPORT_SOURCE_FILE
    };
    
    struct MeshImportDesc
    {
        // Common
        ImportSource source;
        
        // Memory Import
        void*  data;
        size_t size;
        
        // File Import
        std::string file;
    };
    
    extern bool import_mesh(const MeshImportDesc& desc, MeshDesc& mesh);
}
