#pragma once

#include <vector>
#include <string>
#include <stdint.h>
#include <glm.hpp>

namespace ast
{
    enum MaterialTextureType
    {
        TEXTURE_ALBEDO = 0,
        TEXTURE_EMISSIVE,
        TEXTURE_DISPLACEMENT,
        TEXTURE_NORMAL,
        TEXTURE_METALNESS,
        TEXTURE_ROUGHNESS,
        TEXTURE_SPECULAR,
        TEXTURE_GLOSSINESS,
        TEXTURE_CUSTOM
    };
    
    enum MaterialPropertyType
    {
        PROPERTY_ALBEDO = 0,
        PROPERTY_EMISSIVE,
        PROPERTY_METALNESS,
        PROPERTY_ROUGHNESS,
        PROPERTY_SPECULAR,
        PROPERTY_GLOSSINESS
    };
    
    enum ShadingModel
    {
        SHADING_MODEL_STANDARD = 0,
        SHADING_MODEL_CLOTH,
        SHADING_MODEL_SUBSURFACE
    };
    
    enum LightingModel
    {
        LIGHTING_MODEL_LIT = 0,
        LIGHTING_MODEL_UNLIT
    };
    
    enum DisplacementType
    {
        DISPLACEMENT_NONE = 0,
        DISPLACEMENT_PARALLAX_OCCLUSION,
        DISPLACEMENT_TESSELLATION
    };
    
    enum BlendMode
    {
        BLEND_MODE_OPAQUE = 0,
        BLEND_MODE_ADDITIVE,
        BLEND_MODE_MASKED,
        BLEND_MODE_TRANSLUCENT
    };
    
    enum CompressionType
    {
        COMPRESSION_NONE = 0,
        COMPRESSION_BC1  = 1,
        COMPRESSION_BC1a = 2,
        COMPRESSION_BC2  = 3,
        COMPRESSION_BC3  = 4,
        COMPRESSION_BC3n = 5,
        COMPRESSION_BC4 = 6,
        COMPRESSION_BC5 = 7,
        COMPRESSION_BC6 = 8,
        COMPRESSION_BC7 = 9,
        COMPRESSION_ETC1 = 10,
        COMPRESSION_ETC2 = 11,
        COMPRESSION_PVR = 12
    };
    
    enum PixelType
    {
        PIXEL_TYPE_DEFAULT = 0,
        PIXEL_TYPE_UNORM8 = 8,
        PIXEL_TYPE_FLOAT16 = 16,
        PIXEL_TYPE_FLOAT32 = 32
    };
    
    enum TextureType
    {
        TEXTURE_2D = 0,
        TEXTURE_CUBE
    };
    
    enum CubeMapFormat
    {
        CUBEMAP_FORMAT_VCROSS,
        CUBEMAP_FORMAT_HCROSS,
        CUBEMAP_FORMAT_FACE_LIST,
        CUBEMAP_FORMAT_VSTRIP,
        CUBEMAP_FORMAT_HSTRIP,
        CUBEMAP_FORMAT_LATLONG
    };
    
    struct VertexDesc
    {
        glm::vec3 position;
        glm::vec2 tex_coord;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec3 bitangent;
    };
    
    struct SkeletalVertexDesc
    {
        glm::vec3  position;
        glm::vec2  tex_coord;
        glm::vec3  normal;
        glm::vec3  tangent;
        glm::vec3  bitangent;
        glm::ivec4 bone_indices;
        glm::vec4  bone_weights;
    };
    
    struct MaterialTextureDesc
    {
        MaterialTextureType type;
        std::string         path;
        bool                srgb;
    };
    
    struct MaterialProperty
    {
        MaterialPropertyType type;
        union
        {
            bool  bool_value;
            int   int_value;
            float float_value;
            float vec2_value[2];
            float vec3_value[3];
            float vec4_value[4];
        };
    };
    
    struct MaterialDesc
    {
        std::string                      name;
        bool                             metallic_workflow;
        bool                             double_sided;
        std::string                      vertex_shader_func;
        std::string                      fragment_shader_func;
        BlendMode                        blend_mode;
        DisplacementType                 displacement_type;
        ShadingModel                     shading_model;
        LightingModel                    lighting_model;
        std::vector<MaterialTextureDesc> textures;
        std::vector<MaterialProperty>    properties;
    };
    
    struct SubMeshDesc
    {
        uint32_t  material_index;
        uint32_t  index_count;
        uint32_t  base_vertex;
        uint32_t  base_index;
        glm::vec3 max_extents;
        glm::vec3 min_extents;
    };
    
    struct MeshDesc
    {
        std::string                     name;
        std::vector<VertexDesc>         vertices;
        std::vector<SkeletalVertexDesc> skeletal_vertices;
        std::vector<uint32_t>           indices;
        std::vector<SubMeshDesc>        submeshes;
        std::vector<MaterialDesc>       materials;
        glm::vec3                       max_extents;
        glm::vec3                       min_extents;
    };
    
    struct TextureData
    {
        size_t size;
        void*  data;
        
        TextureData()
        {
            size = 0;
            data = nullptr;
        }
        
        ~TextureData()
        {
            size = 0;
            free(data);
        }
        
        void copy_data(size_t src_size, void* src_data)
        {
            data = malloc(src_size);
            memcpy(data, src_data, src_size);
            size = src_size;
        }
    };
    
    struct TextureMipSliceDesc
    {
        uint32_t    width;
        uint32_t    height;
        TextureData pixels;
    };
    
    struct TextureArrayItem
    {
        std::vector<TextureMipSliceDesc> mip_levels;
    };
    
    
    struct TextureDesc
    {
        std::string                   name;
        TextureType                   type;
        uint32_t                      channel_count;
        uint32_t                      mip_slice_count;
        uint32_t                      channel_size;
        CompressionType 	          compression;
        std::vector<TextureArrayItem> array_items;
    };
    
    // --------------------------------------------------------------------------------
    // Binary Assets
    // --------------------------------------------------------------------------------
    
    static const std::string kTextureType[] =
    {
        "TEXTURE_ALBEDO",
        "TEXTURE_EMISSIVE",
        "TEXTURE_DISPLACEMENT",
        "TEXTURE_NORMAL",
        "TEXTURE_METALNESS",
        "TEXTURE_ROUGHNESS",
        "TEXTURE_SPECULAR",
        "TEXTURE_GLOSSINESS",
        "TEXTURE_CUSTOM"
    };
    
    static const std::string kPropertyType[] =
    {
        "PROPERTY_ALBEDO",
        "PROPERTY_EMISSIVE",
        "PROPERTY_METALNESS",
        "PROPERTY_ROUGHNESS",
        "PROPERTY_SPECULAR",
        "PROPERTY_GLOSSINESS"
    };
    
    static const std::string kShadingModel[] =
    {
        "SHADING_MODEL_STANDARD",
        "SHADING_MODEL_CLOTH",
        "SHADING_MODEL_SUBSURFACE"
    };
    
    static const std::string kLightingModel[] =
    {
        "LIGHTING_MODEL_LIT",
        "LIGHTING_MODEL_UNLIT"
    };
    
    static const std::string kDisplacementType[] =
    {
        "DISPLACEMENT_NONE",
        "DISPLACEMENT_PARALLAX_OCCLUSION",
        "DISPLACEMENT_TESSELLATION"
    };
    
    static const std::string kBlendMode[] =
    {
        "BLEND_MODE_OPAQUE",
        "BLEND_MODE_ADDITIVE",
        "BLEND_MODE_MASKED",
        "BLEND_MODE_TRANSLUCENT"
    };
    
    struct BINMeshFileHeader
    {
        uint32_t   mesh_count;
        uint32_t   material_count;
        uint32_t   vertex_count;
        uint32_t   skeletal_vertex_count;
        uint32_t   index_count;
        glm::vec3  max_extents;
        glm::vec3  min_extents;
        char       name[50];
    };
    
    struct BINMeshMaterialJson
    {
        char material[50];
    };
    
    struct BINImageHeader
    {
        uint8_t  type;
        uint8_t  compression;
        uint8_t  channel_size;
        uint8_t  channel_count;
        uint16_t array_slice_count;
        uint8_t  mip_slice_count;
        char     name[50];
    };
    
    struct BINMipSliceHeader
    {
        uint16_t width;
        uint16_t height;
        uint32_t size;
    };
    
}
