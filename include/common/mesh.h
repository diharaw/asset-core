#pragma once

#include <common/material.h>

namespace ast
{
struct Vertex
{
    glm::vec3 position;
    glm::vec2 tex_coord;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

struct SkeletalVertex
{
    glm::vec3  position;
    glm::vec2  tex_coord;
    glm::vec3  normal;
    glm::vec3  tangent;
    glm::vec3  bitangent;
    glm::ivec4 bone_indices;
    glm::vec4  bone_weights;
};

struct SubMesh
{
    uint32_t  material_index;
    uint32_t  index_count;
    uint32_t  vertex_count;
    uint32_t  base_vertex;
    uint32_t  base_index;
    glm::vec3 max_extents;
    glm::vec3 min_extents;
    char      name[150];
};

struct Mesh
{
    std::string                            name;
    std::vector<Vertex>                    vertices;
    std::vector<SkeletalVertex>            skeletal_vertices;
    std::vector<uint32_t>                  indices;
    std::vector<SubMesh>                   submeshes;
    std::vector<std::unique_ptr<Material>> materials;
    std::vector<std::string>               material_paths;
    glm::vec3                              max_extents;
    glm::vec3                              min_extents;
};

// --------------------------------------------------------------------------------
// Binary Assets
// --------------------------------------------------------------------------------

struct BINMeshFileHeader
{
    uint32_t  mesh_count;
    uint32_t  material_count;
    uint32_t  vertex_count;
    uint32_t  skeletal_vertex_count;
    uint32_t  index_count;
    glm::vec3 max_extents;
    glm::vec3 min_extents;
    char      name[150];
};

struct BINMeshMaterialJson
{
    char material[150];
};
} // namespace ast
