#pragma once

#include <common/asset_desc.h>

namespace ast
{
    enum ImportSource
    {
        IMPORT_SOURCE_MEMORY,
        IMPORT_SOURCE_FILE
    };
    
    struct TextureImportOptions
    {
        bool srgb;
        bool normal_map;
        bool generate_mip_chain;
    };
    
    struct Texture2DImportDesc
    {
        // Common
        ImportSource type;
        PixelType    pixel_type;
        
        // Memory Import
        void*  data;
        size_t size;
        
        // File Import
        std::string file;
        
        TextureImportOptions options;
    };
    
    struct TextureCubeImportDesc
    {
        // Common
        ImportSource  type;
        PixelType     pixel_type;
        CubeMapFormat format;
        
        // Memory Import (index 0 for cross, latlong and sphere. all others for face list)
        void*  data[6];
        size_t size[6];
        
        // File Import (index 0 for cross, latlong and sphere. all others for face list)
        std::string files[6];
        
        TextureImportOptions options;
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
    extern bool import_texture_2d(const Texture2DImportDesc& desc, TextureDesc& texture);
    extern bool import_texture_cube(const TextureCubeImportDesc& desc, TextureDesc& texture);
}
