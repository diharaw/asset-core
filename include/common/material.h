#pragma once

#include <vector>
#include <string>
#include <stdint.h>
#include <glm.hpp>
#include <json.hpp>

namespace ast
{
static const std::string kTextureType[] = {
    "TEXTURE_ALBEDO",
    "TEXTURE_EMISSIVE",
    "TEXTURE_DISPLACEMENT",
    "TEXTURE_NORMAL",
    "TEXTURE_METALNESS_SPECULAR",
    "TEXTURE_ROUGHNESS_GLOSSINESS",
    "TEXTURE_CUSTOM"
};

static const std::string kPropertyType[] = {
    "PROPERTY_ALBEDO",
    "PROPERTY_EMISSIVE",
    "PROPERTY_METALNESS_SPECULAR",
    "PROPERTY_ROUGHNESS_GLOSSINESS"
};

static const std::string kMaterialType[] = {
    "MATERIAL_TYPE_OPAQUE",
    "MATERIAL_TYPE_TRANSPARENT"
};

static const std::string kShadingModel[] = {
    "SHADING_MODEL_STANDARD",
    "SHADING_MODEL_CLOTH",
    "SHADING_MODEL_SUBSURFACE"
};

static const std::string kLightingModel[] = {
    "LIGHTING_MODEL_LIT",
    "LIGHTING_MODEL_UNLIT"
};

enum TextureType
{
    TEXTURE_ALBEDO = 0,
    TEXTURE_EMISSIVE,
    TEXTURE_DISPLACEMENT,
    TEXTURE_NORMAL,
    TEXTURE_METALNESS_SPECULAR,
    TEXTURE_ROUGHNESS_GLOSSINESS,
    TEXTURE_CUSTOM
};

enum MaterialPropertyType
{
    PROPERTY_ALBEDO = 0,
    PROPERTY_EMISSIVE,
    PROPERTY_METALNESS_SPECULAR,
    PROPERTY_ROUGHNESS_GLOSSINESS
};

enum MaterialType
{
    MATERIAL_TYPE_OPAQUE,
    MATERIAL_TYPE_TRANSPARENT
};

enum ShadingModel
{
    SHADING_MODEL_STANDARD = 0,
    SHADING_MODEL_CLOTH,
    SHADING_MODEL_SUBSURFACE
};

enum LightingModel
{
    LIGHTING_MODEL_LIT,
    LIGHTING_MODEL_UNLIT
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
    union {
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
    bool                          double_sided;
    bool                          alpha_mask;
    bool                          orca;
    bool                          metallic_workflow;
    MaterialType                  material_type;
    ShadingModel                  shading_model;
    LightingModel                 lighting_model;
    std::vector<Texture>          textures;
    std::vector<MaterialProperty> properties;
};
} // namespace ast
