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

struct TextureInfo
{
    std::string path;
    bool        srgb        = false;
    uint32_t    channel_idx = 0;
    glm::vec2   offset      = glm::vec2(0.0f);
    glm::vec2   scale       = glm::vec2(1.0f);
};

struct Material
{
    SurfaceType  surface_type;
    MaterialType material_type;
    bool         is_alpha_tested;
    bool         is_double_sided;

    // Standard
    glm::vec3   base_color;
    float       metallic;
    float       roughness;
    TextureInfo base_color_texture;
    TextureInfo roughness_texture;
    TextureInfo metallic_texture;
    TextureInfo normal_texture;
    TextureInfo displacement_texture;

    // Emissive
    glm::vec3   emissive_factor;
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
    TextureInfo thickness_texture;
    float       attenuation_distance;
    glm::vec3   attenuation_color;
};
} // namespace ast
