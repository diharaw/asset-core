#include <offline/mesh_importer.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <unordered_map>
#include <common/filesystem.h>

#include <iostream>

namespace ast
{
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
    
    bool does_material_exist(std::vector<uint32_t> &materials, uint32_t &current_material)
    {
        for (auto it : materials)
        {
            if (it == current_material)
                return true;
        }
        
        return false;
    }
    
    bool import_mesh(const std::string& file, Mesh& mesh)
    {
        const aiScene* scene;
        Assimp::Importer importer;
        
        scene = importer.ReadFile(file.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        
        if (scene)
        {
            mesh.name = filesystem::get_filename(file);

			mesh.name.erase(std::remove(mesh.name.begin(), mesh.name.end(), ':'), mesh.name.end());
			mesh.name.erase(std::remove(mesh.name.begin(), mesh.name.end(), '.'), mesh.name.end());

            mesh.submeshes.resize(scene->mNumMeshes);
            
            uint32_t vertex_count = 0;
            uint32_t index_count = 0;
            aiMaterial* temp_material;
            std::vector<uint32_t> processed_mat_ids;
            std::unordered_map<uint32_t, uint32_t> mat_id_mapping;
            uint32_t unnamed_mats = 1;
            
            for (int i = 0; i < scene->mNumMeshes; i++)
            {
                mesh.submeshes[i].index_count = scene->mMeshes[i]->mNumFaces * 3;
                mesh.submeshes[i].base_index = index_count;
                mesh.submeshes[i].base_vertex = vertex_count;
                
                vertex_count += scene->mMeshes[i]->mNumVertices;
                index_count += mesh.submeshes[i].index_count;
                
                if (!does_material_exist(processed_mat_ids, scene->mMeshes[i]->mMaterialIndex))
                {
                    Material mat;
                    
                    mat.name = scene->mMeshes[i]->mName.C_Str();
                    
					mat.name.erase(std::remove(mat.name.begin(), mat.name.end(), ':'), mat.name.end());
					mat.name.erase(std::remove(mat.name.begin(), mat.name.end(), '.'), mat.name.end());

                    if (mat.name.empty())
                    {
                        mat.name =  mesh.name;
                        mat.name += "_unnamed_material_";
                        mat.name += std::to_string(unnamed_mats++);
                    }
                    
                    temp_material = scene->mMaterials[scene->mMeshes[i]->mMaterialIndex];
                    
                    int two_sided = 0;
                    temp_material->Get(AI_MATKEY_TWOSIDED, two_sided);
                    mat.double_sided = (bool)two_sided;
                    
                    mat.blend_mode = BLEND_MODE_OPAQUE;
                    mat.metallic_workflow = true;
                    mat.lighting_model = LIGHTING_MODEL_LIT;
                    mat.displacement_type = DISPLACEMENT_NONE;
                    mat.shading_model = SHADING_MODEL_STANDARD;
                    mat.fragment_shader_func_id = "";
                    mat.fragment_shader_func_src = "";
                    mat.vertex_shader_func_id = "";
                    mat.vertex_shader_func_src = "";
                    
                    // Try to find Diffuse texture
                    std::string albedo_path = get_texture_path(temp_material, aiTextureType_DIFFUSE);
                    
                    if (albedo_path.empty())
                    {
                        aiColor3D diffuse = aiColor3D(1.0f, 1.0f, 1.0f);
                        
                        // Try loading in a Diffuse material property
                        temp_material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
                        
                        MaterialProperty property;
                        
                        property.type = PROPERTY_ALBEDO;
                        property.vec4_value[0] = diffuse.r;
                        property.vec4_value[1] = diffuse.g;
                        property.vec4_value[2] = diffuse.b;
                        property.vec4_value[3] = 1.0f;
                        
                        mat.properties.push_back(property);
                    }
                    else
                    {
                        std::replace(albedo_path.begin(), albedo_path.end(), '\\', '/');
                        
                        Texture mat_desc;
                        
                        mat_desc.srgb = true;
                        mat_desc.type = TEXTURE_ALBEDO;
                        mat_desc.path = albedo_path;
                        
                        mat.textures.push_back(mat_desc);
                    }
                    
                    // Try to find Roughness texture
                    std::string roughness_path = get_texture_path(temp_material, aiTextureType_SHININESS);
                    
                    if (roughness_path.empty())
                    {
                        float roughness = 0.0f;
                        
                        MaterialProperty property;
                        
                        property.type = PROPERTY_ROUGHNESS;
                        property.float_value = roughness;
                        
                        mat.properties.push_back(property);
                    }
                    else
                    {
                        std::replace(roughness_path.begin(), roughness_path.end(), '\\', '/');
                        
                        Texture mat_desc;
                        
                        mat_desc.srgb = false;
                        mat_desc.type = TEXTURE_ROUGHNESS;
                        mat_desc.path = roughness_path;
                        
                        mat.textures.push_back(mat_desc);
                    }
                    
                    // Try to find Metalness texture
                    std::string metalness_path = get_texture_path(temp_material, aiTextureType_AMBIENT);
                    
                    if (metalness_path.empty())
                    {
                        float metalness = 0.0f;
                        
                        MaterialProperty property;
                        
                        property.type = PROPERTY_METALNESS;
                        property.float_value = metalness;
                        
                        mat.properties.push_back(property);
                    }
                    else
                    {
                        std::replace(metalness_path.begin(), metalness_path.end(), '\\', '/');
                        
                        Texture mat_desc;
                        
                        mat_desc.srgb = false;
                        mat_desc.type = TEXTURE_METALNESS;
                        mat_desc.path = metalness_path;
                        
                        mat.textures.push_back(mat_desc);
                    }
                    
                    // Try to find Emissive texture
                    std::string emissive_path = get_texture_path(temp_material, aiTextureType_EMISSIVE);
                    
                    if (emissive_path.empty())
                    {
                        aiColor3D emissive;
                        
                        // Try loading in a Emissive material property
                        if (temp_material->Get(AI_MATKEY_COLOR_EMISSIVE, emissive))
                        {
                            MaterialProperty property;
                            
                            property.type = PROPERTY_EMISSIVE;
                            property.vec4_value[0] = emissive.r;
                            property.vec4_value[1] = emissive.g;
                            property.vec4_value[2] = emissive.b;
                            property.vec4_value[3] = 1.0f;
                            
                            mat.properties.push_back(property);
                        }
                    }
                    else
                    {
                        std::replace(emissive_path.begin(), emissive_path.end(), '\\', '/');
                        
                        Texture mat_desc;
                        
                        mat_desc.srgb = false;
                        mat_desc.type = TEXTURE_EMISSIVE;
                        mat_desc.path = emissive_path;
                        
                        mat.textures.push_back(mat_desc);
                    }
                    
                    // Try to find Specular texture
                    std::string specular_path = get_texture_path(temp_material, aiTextureType_SPECULAR);
                    
                    if (specular_path.empty())
                    {
                        aiColor3D specular;
                        
                        // Try loading in a Specular material property
                        if (temp_material->Get(AI_MATKEY_COLOR_SPECULAR, specular))
                        {
                            MaterialProperty property;
                            
                            property.type = PROPERTY_SPECULAR;
                            property.vec4_value[0] = specular.r;
                            property.vec4_value[1] = specular.g;
                            property.vec4_value[2] = specular.b;
                            property.vec4_value[3] = 1.0f;
                            
                            mat.properties.push_back(property);
                        }
                    }
                    else
                    {
                        std::replace(specular_path.begin(), specular_path.end(), '\\', '/');
                        
                        Texture mat_desc;
                        
                        mat_desc.srgb = false;
                        mat_desc.type = TEXTURE_SPECULAR;
                        mat_desc.path = specular_path;
                        
                        mat.textures.push_back(mat_desc);
                    }
                    
                    // Try to find Normal texture
                    std::string normal_path = get_texture_path(temp_material, aiTextureType_NORMALS);
                    
                    if (!normal_path.empty())
                    {
                        std::replace(normal_path.begin(), normal_path.end(), '\\', '/');
                        
                        Texture mat_desc;
                        
                        mat_desc.srgb = false;
                        mat_desc.type = TEXTURE_NORMAL;
                        mat_desc.path = normal_path;
                        
                        mat.textures.push_back(mat_desc);
                    }
                    
                    // Try to find Height texture
                    std::string height_path = get_texture_path(temp_material, aiTextureType_HEIGHT);
                    
                    if (!height_path.empty())
                    {
                        std::replace(height_path.begin(), height_path.end(), '\\', '/');
                        
                        Texture mat_desc;
                        
                        mat_desc.srgb = true;
                        mat_desc.type = TEXTURE_DISPLACEMENT;
                        mat_desc.path = height_path;
                        
                        mat.textures.push_back(mat_desc);
                        
                        mat.displacement_type = DISPLACEMENT_PARALLAX_OCCLUSION;
                    }
                    
                    mat_id_mapping[scene->mMeshes[i]->mMaterialIndex] = mesh.materials.size();
                    processed_mat_ids.push_back(scene->mMeshes[i]->mMaterialIndex);
                    
                    mesh.materials.push_back(mat);
                }
                
                mesh.submeshes[i].material_index = mat_id_mapping[scene->mMeshes[i]->mMaterialIndex];
            }
            
            mesh.vertices.resize(vertex_count);
            mesh.indices.resize(index_count);
            
            aiMesh* temp_mesh;
            int idx = 0;
            int vertex_index = 0;
            
            for (int i = 0; i < scene->mNumMeshes; i++)
            {
                temp_mesh = scene->mMeshes[i];
                mesh.submeshes[i].max_extents = glm::vec3(temp_mesh->mVertices[0].x, temp_mesh->mVertices[0].y, temp_mesh->mVertices[0].z);
                mesh.submeshes[i].min_extents = glm::vec3(temp_mesh->mVertices[0].x, temp_mesh->mVertices[0].y, temp_mesh->mVertices[0].z);
                
                for (int k = 0; k < scene->mMeshes[i]->mNumVertices; k++)
                {
                    mesh.vertices[vertex_index].position = glm::vec3(temp_mesh->mVertices[k].x, temp_mesh->mVertices[k].y, temp_mesh->mVertices[k].z);
                    glm::vec3 n = glm::vec3(temp_mesh->mNormals[k].x, temp_mesh->mNormals[k].y, temp_mesh->mNormals[k].z);
                    mesh.vertices[vertex_index].normal = n;
                    
                    if (temp_mesh->mTangents)
                    {
                        glm::vec3 t = glm::vec3(temp_mesh->mTangents[k].x, temp_mesh->mTangents[k].y, temp_mesh->mTangents[k].z);
                        glm::vec3 b = glm::vec3(temp_mesh->mBitangents[k].x, temp_mesh->mBitangents[k].y, temp_mesh->mBitangents[k].z);
                        
                        // @NOTE: Assuming right handed coordinate space
                        if (glm::dot(glm::cross(n, t), b) < 0.0f)
                            t *= -1.0f; // Flip tangent
                        
                        mesh.vertices[vertex_index].tangent = t;
                        mesh.vertices[vertex_index].bitangent = b;
                    }
                    
                    if (temp_mesh->HasTextureCoords(0))
                    {
                        mesh.vertices[vertex_index].tex_coord = glm::vec2(temp_mesh->mTextureCoords[0][k].x, temp_mesh->mTextureCoords[0][k].y);
                    }
                    
                    if (mesh.vertices[vertex_index].position.x > mesh.submeshes[i].max_extents.x)
                        mesh.submeshes[i].max_extents.x = mesh.vertices[vertex_index].position.x;
                    if (mesh.vertices[vertex_index].position.y > mesh.submeshes[i].max_extents.y)
                        mesh.submeshes[i].max_extents.y = mesh.vertices[vertex_index].position.y;
                    if (mesh.vertices[vertex_index].position.z > mesh.submeshes[i].max_extents.z)
                        mesh.submeshes[i].max_extents.z = mesh.vertices[vertex_index].position.z;
                    
                    if (mesh.vertices[vertex_index].position.x < mesh.submeshes[i].min_extents.x)
                        mesh.submeshes[i].min_extents.x = mesh.vertices[vertex_index].position.x;
                    if (mesh.vertices[vertex_index].position.y < mesh.submeshes[i].min_extents.y)
                        mesh.submeshes[i].min_extents.y = mesh.vertices[vertex_index].position.y;
                    if (mesh.vertices[vertex_index].position.z < mesh.submeshes[i].min_extents.z)
                        mesh.submeshes[i].min_extents.z = mesh.vertices[vertex_index].position.z;
                    
                    vertex_index++;
                }
                
                for (int j = 0; j < temp_mesh->mNumFaces; j++)
                {
                    mesh.indices[idx] = temp_mesh->mFaces[j].mIndices[0];
                    idx++;
                    mesh.indices[idx] = temp_mesh->mFaces[j].mIndices[1];
                    idx++;
                    mesh.indices[idx] = temp_mesh->mFaces[j].mIndices[2];
                    idx++;
                }
            }
            
            mesh.max_extents = mesh.submeshes[0].max_extents;
            mesh.min_extents = mesh.submeshes[0].min_extents;
            
            for (int i = 0; i < mesh.submeshes.size(); i++)
            {
                if (mesh.submeshes[i].max_extents.x > mesh.max_extents.x)
                    mesh.max_extents.x = mesh.submeshes[i].max_extents.x;
                if (mesh.submeshes[i].max_extents.y > mesh.max_extents.y)
                    mesh.max_extents.y = mesh.submeshes[i].max_extents.y;
                if (mesh.submeshes[i].max_extents.z > mesh.max_extents.z)
                    mesh.max_extents.z = mesh.submeshes[i].max_extents.z;
                
                if (mesh.submeshes[i].min_extents.x < mesh.min_extents.x)
                    mesh.min_extents.x = mesh.submeshes[i].min_extents.x;
                if (mesh.submeshes[i].min_extents.y < mesh.min_extents.y)
                    mesh.min_extents.y = mesh.submeshes[i].min_extents.y;
                if (mesh.submeshes[i].min_extents.z < mesh.min_extents.z)
                    mesh.min_extents.z = mesh.submeshes[i].min_extents.z;
            }
            
            return true;
        }
    
        return false;
    }
}
