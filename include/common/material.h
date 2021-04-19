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
    "TEXTURE_METALLIC",
    "TEXTURE_ROUGHNESS",
    "TEXTURE_CUSTOM"
};

static const std::string kPropertyType[] = {
    "PROPERTY_ALBEDO",
    "PROPERTY_EMISSIVE",
    "PROPERTY_METALLIC",
    "PROPERTY_ROUGHNESS"
};

static const std::string kMaterialType[] = {
    "MATERIAL_OPAQUE",
    "MATERIAL_TRANSPARENT"
};

static const std::string kShadingModel[] = {
    "SHADING_MODEL_STANDARD",
    "SHADING_MODEL_CLOTH",
    "SHADING_MODEL_SUBSURFACE"
};

enum SurfaceType
{
    SURFACE_OPAQUE,
    SURFACE_TRANSPARENT
};

enum MaterialType
{
    MATERIAL_LAMBERT,
    MATERIAL_GLASS,
    MATERIAL_DISNEY,
    MATERIAL_GLTF
};
enum MaterialQueue
{
    MATERIAL_QUEUE_STANDARD,
    MATERIAL_QUEUE_CLOTH,
    MATERIAL_QUEUE_CLEAR_COAT,
    MATERIAL_QUEUE_TRANSLUCENT,
    MATERIAL_QUEUE_SUBSURFACE
};

enum MaterialFlags
{
    MATERIAL_FLAG_THIN,
    MATERIAL_FLAG_DOUBLE_SIDED,
    MATERIAL_FLAG_ALPHA_TESTED,
    MATERIAL_FLAG_ANISOTROPIC
};

struct Material
{
    std::string              name;
    MaterialType             type;
    MaterialQueue            queue;
    MaterialFlags            flags;
    std::vector<std::string> images;
};

struct LambertMaterial : Material
{

};

struct GlassMaterial : Material
{
};

struct DisneyMaterial : Material
{

};

struct GLTFMaterial : Material
{
};
} // namespace ast
