#pragma once 

#include "types.h"

struct TSM_Joint
{
	uint8_t index;
	uint8_t parent_index;
	Matrix4 offset_transform;
};

struct TSM_SkeletonHeader
{
	uint16_t joint_count;
	char     name[50];
};
