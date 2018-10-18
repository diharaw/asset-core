#pragma once

#include <vector>
#include <string>
#include <stdint.h>
#include <glm.hpp>

namespace ast
{
    enum TextureType
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
    
    enum TextureCompression
    {
        COMPRESSION_NONE = 0,
        COMPREESION_DXT3,
        COMPREESION_DXT5
    };
    
    enum ImageType
    {
        IMAGE_2D = 0,
        IMAGE_CUBE
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
    
    struct TextureDesc
    {
        TextureType type;
        std::string path;
        bool        srgb;
    };
    
    struct MaterialProperty
    {
        MaterialPropertyType type;
        union
        {
            bool      bool_value;
            int       int_value;
            float     float_value;
            glm::vec2 vec2_value;
            glm::vec3 vec3_value;
            glm::vec4 vec4_value;
        };
    };
    
    struct MaterialDesc
    {
        std::string                   name;
        bool                          metallic_workflow;
        bool                          double_sided;
        std::string                   vertex_shader_func;
        std::string                   fragment_shader_func;
        BlendMode                     blend_mode;
        DisplacementType              displacement_type;
        ShadingModel                  shading_model;
        LightingModel                 lighting_model;
        std::vector<TextureDesc>      textures;
        std::vector<MaterialProperty> properties;
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
    
    struct ImageMipSliceDesc
    {
        uint32_t              width;
        uint32_t              height;
        std::vector<uint8_t>  pixels8;
        std::vector<uint16_t> pixels16;
        std::vector<uint32_t> pixels32;
    };
    
    struct ImageArraySliceDesc
    {
        std::vector<ImageMipSliceDesc> mip_slices;
    };
    
    struct ImageDesc
    {
        std::string                      name;
        ImageType                        type;
        uint32_t                         channel_count;
        uint32_t                         mip_slice_count;
        uint32_t                         channel_size;
        TextureCompression 	             compression;
        std::vector<ImageArraySliceDesc> array_slices;
    };
}
