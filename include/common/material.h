#pragma once

#include <vector>
#include <string>
#include <stdint.h>
#include <glm.hpp>

namespace ast
{
static const std::string kSurfaceType[] = {
    "SURFACE_OPAQUE",
    "SURFACE_TRANSPARENT",
    "SURFACE_TRANSMISSIVE"
};

static const std::string kMaterialType[] = {
    "MATERIAL_UNLIT",
    "MATERIAL_STANDARD",
    "MATERIAL_CLOTH",
    "MATERIAL_CLEAR_COAT",
    "MATERIAL_TRANSLUCENT",
    "MATERIAL_SUBSURFACE"
};

static const std::string kAlphaMode[] = {
    "ALHPA_MODE_OPAQUE",
    "ALHPA_MODE_BLEND",
    "ALHPA_MODE_MASK"
};

enum SurfaceType
{
    SURFACE_OPAQUE,      // Surfaces with no alpha-blending.
    SURFACE_TRANSPARENT, // Alpha-blended transparency.
    SURFACE_TRANSMISSIVE // Transmissive surfaces with refraction and/or roughness.
};

enum MaterialType
{
    MATERIAL_UNLIT,
    MATERIAL_STANDARD,
    MATERIAL_CLOTH,
    MATERIAL_CLEAR_COAT,
    MATERIAL_TRANSLUCENT,
    MATERIAL_SUBSURFACE
};

enum AlphaMode
{
    ALPHA_MODE_OPAQUE,
    ALPHA_MODE_BLEND,
    ALPHA_MODE_MASK
};

struct TextureInfo
{
    bool      srgb        = false;
    int32_t   texture_idx = -1;
    uint32_t  channel_idx = 0;
    glm::vec2 offset      = glm::vec2(0.0f);
    glm::vec2 scale       = glm::vec2(1.0f);
};

struct Material
{
    std::string              name;
    SurfaceType              surface_type;
    MaterialType             material_type;
    AlphaMode                alpha_mode;
    bool                     is_double_sided;
    std::vector<std::string> textures;

    // Standard
    glm::vec3   base_color;
    float       metallic;
    float       roughness;
    glm::vec3   emissive_factor;
    TextureInfo base_color_texture;
    TextureInfo roughness_texture;
    TextureInfo metallic_texture;
    TextureInfo normal_texture;
    TextureInfo displacement_texture;
    TextureInfo emissive_texture;

    // Sheen
    glm::vec3   sheen_color;
    float       sheen_roughness;
    TextureInfo sheen_color_texture;
    TextureInfo sheen_roughness_texture;

    // Clear Coat
    float       clear_coat;
    float       clear_coat_roughness;
    TextureInfo clear_coat_texture;
    TextureInfo clear_coat_roughness_texture;
    TextureInfo clear_coat_normal_texture;

    // Anisotropy
    float       anisotropy;
    TextureInfo anisotropy_texture;
    TextureInfo anisotropy_directions_texture;

    // Transmission
    float       transmission;
    TextureInfo transmission_texture;

    // IOR
    float ior;

    // Volume
    float       thickness_factor;
    float       attenuation_distance;
    glm::vec3   attenuation_color;
    TextureInfo thickness_texture;
};
} // namespace ast
