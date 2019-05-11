#pragma once

#include <vector>
#include <string>
#include <stdint.h>
#include <glm.hpp>

namespace ast
{
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
        "PROPERTY_GLOSSINESS",
		"PROPERTY_SHININESS",
		"PROPERTY_REFLECTIVITY"
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
        PROPERTY_GLOSSINESS,
		PROPERTY_SHININESS,
		PROPERTY_REFLECTIVITY
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
    
    struct Texture
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
            bool  bool_value;
            int   int_value;
            float float_value;
            float vec2_value[2];
            float vec3_value[3];
            float vec4_value[4];
        };
    };
    
    struct Material
    {
        std::string                   name;
        bool                          metallic_workflow;
        bool                          double_sided;
        std::string                   vertex_shader_func_path;
        std::string                   fragment_shader_func_path;
        BlendMode                     blend_mode;
        DisplacementType              displacement_type;
        ShadingModel                  shading_model;
        LightingModel                 lighting_model;
        std::vector<Texture>          textures;
        std::vector<MaterialProperty> properties;
    };
}
