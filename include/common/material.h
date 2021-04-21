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

struct TextureInfo
{
    int32_t  texture_idx = -1;
    uint32_t channel_idx = 0;
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

    int32_t base_color_texture   = -1;
    int32_t normal_texture       = -1;
    int32_t displacement_texture = -1;
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
    float     sheen;
    float     sheen_tint;
    float     clear_coat;
    float     clear_coat_gloss;
    float     anisotropic;

    TextureInfo base_color_texture;
    TextureInfo subsurface_texture;
    TextureInfo metallic_texture;
    TextureInfo specular_texture;
    TextureInfo specular_tint_texture;
    TextureInfo roughness_texture;
    TextureInfo sheen_texture;
    TextureInfo sheen_tint_texture;
    TextureInfo clear_coat_texture;
    TextureInfo clear_coat_gloss_texture;
    TextureInfo anisotropic_texture;
    TextureInfo anisotropy_directions_texture;
    TextureInfo normal_texture;
    TextureInfo displacement_texture;
};

struct GLTFMaterial : Material
{
    glm::vec3 base_color;
    float     metallic;
    float     roughness;
    float     refraction_thickness;
    float     transmission;
    float     ior;
    glm::vec3 sheen_color;
    float     sheen_roughness;
    float     clear_coat;
    float     clear_coat_roughness;
    float     anisotropy;

    TextureInfo base_color_texture;
    TextureInfo metallic_texture;
    TextureInfo roughness_texture;
    TextureInfo transmission_texture;
    TextureInfo clear_coat_texture;
    TextureInfo clear_coat_roughness_texture;
    TextureInfo clear_coat_normal_texture;
    TextureInfo sheen_color_texture;
    TextureInfo sheen_roughness_texture;
    TextureInfo anisotropy_texture;
    TextureInfo anisotropy_directions_texture;
    TextureInfo normal_texture;
    TextureInfo displacement_texture;
};
} // namespace ast
