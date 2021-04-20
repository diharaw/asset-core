#pragma once

#include <vector>
#include <string>
#include <stdint.h>
#include <glm.hpp>
#include <json.hpp>

namespace ast
{
static const std::string kSurfaceType[] = {
    "SURFACE_OPAQUE",
    "SURFACE_TRANSPARENT"
};

static const std::string kMaterialType[] = {
    "MATERIAL_LAMBERT",
    "MATERIAL_GLASS",
    "MATERIAL_DISNEY",
    "MATERIAL_GLTF"
};

static const std::string kShadingModel[] = {
    "SHADING_MODEL_DEFAULT",
    "SHADING_MODEL_CLOTH",
    "SHADING_MODEL_CLEAR_COAT",
    "SHADING_MODEL_TRANSLUCENT",
    "SHADING_MODEL_SUBSURFACE"
};

enum SurfaceType
{
    SURFACE_OPAQUE,
    SURFACE_TRANSPARENT
};

enum MaterialType
{
    MATERIAL_MATTE,
    MATERIAL_MIRROR,
    MATERIAL_METAL,
    MATERIAL_GLASS,
    MATERIAL_DISNEY,
    MATERIAL_GLTF
};

enum ShadingModel
{
    SHADING_MODEL_DEFAULT,
    SHADING_MODEL_CLOTH,
    SHADING_MODEL_CLEAR_COAT,
    SHADING_MODEL_TRANSLUCENT,
    SHADING_MODEL_SUBSURFACE
};

struct Material
{
    std::string              name;
    SurfaceType              surface_type;
    MaterialType             material_type;
    ShadingModel             shading_model;
    bool                     alpha_tested;
    bool                     double_sided;
    bool                     thin;
    std::vector<std::string> images;
};

struct MatteMaterial : Material
{
    glm::vec3 base_color;
};

struct MirrorMaterial : Material
{
    
};

struct MetalMaterial : Material
{
};

struct GlassMaterial : Material
{
    float R;
    float T;
    float eta_a;
    float eta_b;
};

struct DisneyMaterial : Material
{
    glm::vec3 base_color;
    float     subsurface;
    float     metallic;
    float     specular;
    float     specular_tint;
    float     roughness;
    float     anisotropic;
    float     sheen;
    float     sheen_tint;
    float     clear_coat;
    float     clear_coat_gloss;
};

struct GLTFMaterial : Material
{

};
} // namespace ast
