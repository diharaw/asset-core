#pragma once 

#include "types.h"

struct TSM_Material
{
	char albedo[50];
	char normal[50];
	char roughness[50];
	char metalness[50];
	char displacement[50];
    //char emissive[50];
};

struct Assimp_Material
{
	char albedo[150];
	char normal[150];
	bool has_metalness;
	bool has_roughness;
	char metalness[150];
	char roughness[150];
    char displacement[150];
    char emissive[150];
	String mesh_name;
};

struct TSM_Material_Json
{
	char material[50];
};
