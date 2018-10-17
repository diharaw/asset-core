#pragma once 

#include "types.h"

struct TSM_FileHeader
{
	uint8_t   mesh_type;
	uint16_t  mesh_count;
	uint16_t  material_count;
	uint32_t  vertex_count;
	uint32_t  index_count;
	glm::vec3 max_extents;
	glm::vec3 min_extents;
	char 	  name[50];
};

struct TSM_MeshHeader
{
	uint8_t   material_index;
	uint32_t  index_count;
	uint32_t  base_vertex;
	uint32_t  base_index;
	glm::vec3 max_extents;
	glm::vec3 min_extents;
};