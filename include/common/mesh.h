#pragma once

#include <common/material.h>

namespace ast
{
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
    
    // --------------------------------------------------------------------------------
    // Binary Assets
    // --------------------------------------------------------------------------------
    
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
}
