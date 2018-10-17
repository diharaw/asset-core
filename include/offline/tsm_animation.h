#pragma once 

#include "types.h"

struct TSM_Keyframe
{
	Quaternion rotation;
	Vector3    translation;
	Vector3    scale;
};

struct TSM_AnimationChannelHeader
{
	char joint_name[50];
};

struct TSM_AnimationHeader
{
	uint16 keyframe_count;
	uint16 animation_channel_count;
	char   name[50];
	double duration;
};