#pragma once 

#include "types.h"

struct TSM_Vertex
{
	glm::vec3 position;
	glm::vec2 tex_coord;
	glm::vec3 normal;
	glm::vec3 tangent;
    glm::vec3 bitangent;
};

struct TSM_SkeletalVertex
{
	glm::vec3  position;
	glm::vec2  tex_coord;
	glm::vec3  normal;
	glm::vec3  tangent;
	glm::ivec4 bone_indices;
	glm::vec4  bone_weights;
};
