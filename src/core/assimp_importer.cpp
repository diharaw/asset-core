#include <assimp_importer.h>

#include <iostream>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <unordered_map>
#include <filesystem.h>

namespace assimp_importer
{
	static const aiTextureType kTextureTypes[] = 
    {
		aiTextureType_DIFFUSE,
		aiTextureType_SPECULAR,
		aiTextureType_AMBIENT,
		aiTextureType_EMISSIVE,
		aiTextureType_HEIGHT,
		aiTextureType_NORMALS,
		aiTextureType_SHININESS,
		aiTextureType_OPACITY,
		aiTextureType_DISPLACEMENT,
		aiTextureType_LIGHTMAP,
		aiTextureType_REFLECTION
	};

	static const char* kTextureTypeStrings[] = 
    {
		"aiTextureType_DIFFUSE",
		"aiTextureType_SPECULAR",
		"aiTextureType_AMBIENT",
		"aiTextureType_EMISSIVE",
		"aiTextureType_HEIGHT",
		"aiTextureType_NORMALS",
		"aiTextureType_SHININESS",
		"aiTextureType_OPACITY",
		"aiTextureType_DISPLACEMENT",
		"aiTextureType_LIGHTMAP",
		"aiTextureType_REFLECTION"
	};

    std::string get_texture_path(aiMaterial* material, aiTextureType texture_type)
    {
        aiString path;
        aiReturn result = material->GetTexture(texture_type, 0, &path);
        
        if (result == aiReturn_FAILURE)
            return "";
        else
        {
            std::string cppStr = std::string(path.C_Str());
            
            if (cppStr == "")
                return "";
            
            return cppStr;
        }
    }

	void dump_textures(aiMaterial* material)
	{
		std::string texture = "";

		for (uint32_t i = 0; i < 11; i++)
		{
			texture = get_texture_path(material, kTextureTypes[i]);

			if (texture != "")
				std::cout << kTextureTypeStrings[i] << " : " << texture << std::endl;
		}
	}
    
    bool does_material_exist(std::vector<unsigned int> &materials, unsigned int &current_material)
    {
        for (auto it : materials)
        {
            if (it == current_material)
                return true;
        }
        
        return false;
    }

    AssimpImportData* import_mesh(std::string file, Options options)
    {
        {
            std::string meshPath = filesystem::get_file_path(file);

            AssimpImportData* load_data = new AssimpImportData();
            
            load_data->mesh_path = meshPath;
            load_data->filename = file;
            
            const aiScene* Scene;
            Assimp::Importer Importer;
            Scene = Importer.ReadFile(file, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
            
            load_data->header.mesh_count = Scene->mNumMeshes;
            load_data->header.vertex_count = 0;
            load_data->header.index_count = 0;
			load_data->header.material_count = 0;
            load_data->meshes = new TSM_MeshHeader[Scene->mNumMeshes];
            
            // Temp
            load_data->skeletal = false;
            
            aiMaterial* temp_material;
            uint8_t materialIndex = 0;
            
            std::vector<Assimp_Material> temp_materials;
            std::vector<unsigned int> processedMatId;
            std::unordered_map<unsigned int, uint8_t> MatIDMapping;
			uint32_t unamedMats = 1;
            
            std::cout << Scene->mNumMaterials << std::endl;
            
            for (int i = 0; i < load_data->header.mesh_count; i++)
            {
				bool hasLeastOneTexture = false;

                load_data->meshes[i].index_count = Scene->mMeshes[i]->mNumFaces * 3;
                load_data->meshes[i].base_index = load_data->header.index_count;
                load_data->meshes[i].base_vertex = load_data->header.vertex_count;
                
                load_data->header.vertex_count += Scene->mMeshes[i]->mNumVertices;
                load_data->header.index_count += load_data->meshes[i].index_count;

                if (!does_material_exist(processedMatId, Scene->mMeshes[i]->mMaterialIndex))
                {
					Assimp_Material temp;

                    temp_material = Scene->mMaterials[Scene->mMeshes[i]->mMaterialIndex];
                    temp.mesh_name = std::string(Scene->mMeshes[i]->mName.C_Str());

                    if (options.verbose)
                    {
                        std::cout << "**** Dumping Textures for Material: " << temp.mesh_name << "\n" << std::endl;
                        dump_textures(temp_material);
                        std::cout << "\n************************************" << std::endl;
                    }

                    std::string albedo = get_texture_path(temp_material, aiTextureType_DIFFUSE);
                    
					temp.albedo[0] = '\0';
					if (albedo != "")
					{
						std::replace(albedo.begin(), albedo.end(), '\\', '/');

						if (albedo.length() > 4 && albedo[0] != ' ')
						{
							hasLeastOneTexture = true;
							strncpy(temp.albedo, albedo.c_str(), 150);
						}
					}

					std::string normal = get_texture_path(temp_material, aiTextureType_HEIGHT);

					temp.normal[0] = '\0';
					if (normal != "")
					{
						std::replace(normal.begin(), normal.end(), '\\', '/');

						if (normal.length() > 4 && normal[0] != ' ' && normal != "")
						{
							hasLeastOneTexture = true;
							strncpy(temp.normal, normal.c_str(), 150);
						}
					}
	
					std::string metalness = get_texture_path(temp_material, aiTextureType_AMBIENT);
                    std::string mtl_str = std::string(metalness);
                    
					temp.metalness[0] = '\0';
					if (metalness == "" || mtl_str == "")
                        temp.has_metalness = false;
					else
                    {
                        temp.has_metalness = true;
						std::replace(metalness.begin(), metalness.end(), '\\', '/');

						if (metalness.length() > 4 && metalness[0] != ' ' && metalness != "")
						{
							hasLeastOneTexture = true;
							strncpy(temp.metalness, metalness.c_str(), 150);
						}
                    }

					std::string roughness = get_texture_path(temp_material, aiTextureType_SHININESS);
                    std::string rgh_str = std::string(roughness);
                    
					temp.roughness[0] = '\0';
					if (roughness == "" || rgh_str == "")
                        temp.has_roughness = false;
					else
                    {
                        temp.has_roughness = true;
						std::replace(roughness.begin(), roughness.end(), '\\', '/');

						if (roughness.length() > 4 && roughness[0] != ' ' && roughness != "")
						{
							hasLeastOneTexture = true;
							strncpy(temp.roughness, roughness.c_str(), 150);
						}
                    }
                    
                    std::string displacement = get_texture_path(temp_material, aiTextureType_DISPLACEMENT);
                    std::string disp_str = std::string(displacement);
                    
                    temp.displacement[0] = '\0';
                    if (displacement != "" || disp_str != "")
                    {
                        std::replace(displacement.begin(), displacement.end(), '\\', '/');
                        
                        if (displacement.length() > 4 && displacement[0] != ' ' && displacement != "")
                        {
                            hasLeastOneTexture = true;
                            strncpy(temp.displacement, displacement.c_str(), 150);
                        }
                    }

					if (hasLeastOneTexture)
					{
						if (temp.mesh_name.length() == 0 || temp.mesh_name == "" || temp.mesh_name == " ")
							temp.mesh_name = "untitled_" + std::to_string(unamedMats++);

						load_data->header.material_count++;
						processedMatId.push_back(Scene->mMeshes[i]->mMaterialIndex);
						load_data->meshes[i].material_index = materialIndex;
						MatIDMapping[Scene->mMeshes[i]->mMaterialIndex] = materialIndex;

						temp_materials.push_back(temp);
						materialIndex++;
					}
					else
						load_data->meshes[i].material_index = 0;
                }
                else // if already exists, find the internal ID it maps to.
                    load_data->meshes[i].material_index = MatIDMapping[Scene->mMeshes[i]->mMaterialIndex];
            }

            load_data->header.material_count = processedMatId.size();
            
            if (load_data->header.material_count > 0)
                load_data->materials = new Assimp_Material[load_data->header.material_count];
            else
                load_data->materials = nullptr;
            
            for (int i = 0; i < temp_materials.size(); i++)
            {
                load_data->materials[i] = temp_materials[i];
            }
            
            load_data->vertices = new TSM_Vertex[load_data->header.vertex_count];
            load_data->indices = new uint32_t[load_data->header.index_count];
            
            aiMesh* TempMesh;
            int idx = 0;
            int vertexIndex = 0;
            
            for (int i = 0; i < load_data->header.mesh_count; i++)
            {
                TempMesh = Scene->mMeshes[i];
                load_data->meshes[i].max_extents = glm::vec3(TempMesh->mVertices[0].x, TempMesh->mVertices[0].y, TempMesh->mVertices[0].z);
                load_data->meshes[i].min_extents = glm::vec3(TempMesh->mVertices[0].x, TempMesh->mVertices[0].y, TempMesh->mVertices[0].z);
                
                for (int k = 0; k < Scene->mMeshes[i]->mNumVertices; k++)
                {
                    load_data->vertices[vertexIndex].position = glm::vec3(TempMesh->mVertices[k].x, TempMesh->mVertices[k].y, TempMesh->mVertices[k].z);
                    glm::vec3 n = glm::vec3(TempMesh->mNormals[k].x, TempMesh->mNormals[k].y, TempMesh->mNormals[k].z);
					load_data->vertices[vertexIndex].normal = n;

					if (TempMesh->mTangents)
					{
						glm::vec3 t = glm::vec3(TempMesh->mTangents[k].x, TempMesh->mTangents[k].y, TempMesh->mTangents[k].z);
						glm::vec3 b = glm::vec3(TempMesh->mBitangents[k].x, TempMesh->mBitangents[k].y, TempMesh->mBitangents[k].z);

						// @NOTE: Assuming right handed coordinate space
						if (glm::dot(glm::cross(n, t), b) < 0.0f)
							t *= -1.0f; // Flip tangent

						load_data->vertices[vertexIndex].tangent = t;
						load_data->vertices[vertexIndex].bitangent = b;
					}

					if (TempMesh->HasTextureCoords(0))
					{
						load_data->vertices[vertexIndex].tex_coord = glm::vec2(TempMesh->mTextureCoords[0][k].x, TempMesh->mTextureCoords[0][k].y);
					}

                    if (load_data->vertices[vertexIndex].position.x > load_data->meshes[i].max_extents.x)
                        load_data->meshes[i].max_extents.x = load_data->vertices[vertexIndex].position.x;
                    if (load_data->vertices[vertexIndex].position.y > load_data->meshes[i].max_extents.y)
                        load_data->meshes[i].max_extents.y = load_data->vertices[vertexIndex].position.y;
                    if (load_data->vertices[vertexIndex].position.z > load_data->meshes[i].max_extents.z)
                        load_data->meshes[i].max_extents.z = load_data->vertices[vertexIndex].position.z;
                    
                    if (load_data->vertices[vertexIndex].position.x < load_data->meshes[i].min_extents.x)
                        load_data->meshes[i].min_extents.x = load_data->vertices[vertexIndex].position.x;
                    if (load_data->vertices[vertexIndex].position.y < load_data->meshes[i].min_extents.y)
                        load_data->meshes[i].min_extents.y = load_data->vertices[vertexIndex].position.y;
                    if (load_data->vertices[vertexIndex].position.z < load_data->meshes[i].min_extents.z)
                        load_data->meshes[i].min_extents.z = load_data->vertices[vertexIndex].position.z;
                    
                    vertexIndex++;
                }
                
                for (int j = 0; j < TempMesh->mNumFaces; j++)
                {
                    load_data->indices[idx] = TempMesh->mFaces[j].mIndices[0];
                    idx++;
                    load_data->indices[idx] = TempMesh->mFaces[j].mIndices[1];
                    idx++;
                    load_data->indices[idx] = TempMesh->mFaces[j].mIndices[2];
                    idx++;
                }
            }
            
            load_data->header.max_extents = load_data->meshes[0].max_extents;
            load_data->header.min_extents = load_data->meshes[0].min_extents;
            
            for (int i = 0; i < load_data->header.mesh_count; i++)
            {
                if (load_data->meshes[i].max_extents.x > load_data->header.max_extents.x)
                    load_data->header.max_extents.x = load_data->meshes[i].max_extents.x;
                if (load_data->meshes[i].max_extents.y > load_data->header.max_extents.y)
                    load_data->header.max_extents.y = load_data->meshes[i].max_extents.y;
                if (load_data->meshes[i].max_extents.z > load_data->header.max_extents.z)
                    load_data->header.max_extents.z = load_data->meshes[i].max_extents.z;
                
                if (load_data->meshes[i].min_extents.x < load_data->header.min_extents.x)
                    load_data->header.min_extents.x = load_data->meshes[i].min_extents.x;
                if (load_data->meshes[i].min_extents.y < load_data->header.min_extents.y)
                    load_data->header.min_extents.y = load_data->meshes[i].min_extents.y;
                if (load_data->meshes[i].min_extents.z < load_data->header.min_extents.z)
                    load_data->header.min_extents.z = load_data->meshes[i].min_extents.z;
            }
            
            std::cout << std::endl;
            std::cout << "Vertex count: " << load_data->header.vertex_count << std::endl;
            std::cout << "Index count: " << load_data->header.index_count << std::endl;
            std::cout << "Mesh count: " << load_data->header.mesh_count << std::endl;
            std::cout << "Material count: " << load_data->header.material_count << std::endl;
            std::cout << std::endl;
            
            return load_data;
        }
        
        return nullptr;
    }
}
