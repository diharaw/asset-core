#include <offline/importer.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <unordered_map>
#include <common/filesystem.h>
#include <nvtt/nvtt.h>
#include <nvimage/Image.h>
#include <nvimage/ImageIO.h>
#include <nvimage/FloatImage.h>
#include <nvimage/DirectDrawSurface.h>
#include <nvcore/Ptr.h>
#include <nvcore/StrLib.h>
#include <nvcore/StdStream.h>
#include <nvcore/FileSystem.h>
#include <nvcore/Timer.h>
#include <cmft/image.h>
#include <cmft/cubemapfilter.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

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
    
    bool import_mesh(const std::string& path, MeshDesc& desc)
    {
        const aiScene* scene;
        Assimp::Importer importer;
        scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        
        if (scene)
        {
            desc.name = filesystem::get_filename(path);
            desc.submeshes.resize(scene->mNumMeshes);
            
            uint32_t vertex_count = 0;
            uint32_t index_count = 0;
            aiMaterial* temp_material;
            std::vector<uint32_t> processed_mat_id;
            std::unordered_map<uint32_t, uint32_t> mat_id_mapping;
            uint32_t unnamed_mats = 1;
            
            for (int i = 0; i < scene->mNumMeshes; i++)
            {
                desc.submeshes[i].index_count = scene->mMeshes[i]->mNumFaces * 3;
                desc.submeshes[i].base_index = index_count;
                desc.submeshes[i].base_vertex = vertex_count;
                
                vertex_count += scene->mMeshes[i]->mNumVertices;
                index_count += desc.submeshes[i].index_count;
                
                if (!does_material_exist(processed_mat_id, scene->mMeshes[i]->mMaterialIndex))
                {
                    MaterialDesc mat;
                    
                    mat.name = scene->mMeshes[i]->mName.C_Str();
                    
                    if (mat.name.empty())
                    {
                        mat.name = desc.name;
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
                    mat.fragment_shader_func = "";
                    mat.vertex_shader_func = "";
                    
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
                        
                        TextureDesc mat_desc;
                        
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
                        
                        TextureDesc mat_desc;
                        
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
                        
                        TextureDesc mat_desc;
                        
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
                        
                        TextureDesc mat_desc;
                        
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
                        
                        TextureDesc mat_desc;
                        
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
                        
                        TextureDesc mat_desc;
                        
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
                        
                        TextureDesc mat_desc;
                        
                        mat_desc.srgb = true;
                        mat_desc.type = TEXTURE_DISPLACEMENT;
                        mat_desc.path = height_path;
                        
                        mat.textures.push_back(mat_desc);
                        
                        mat.displacement_type = DISPLACEMENT_PARALLAX_OCCLUSION;
                    }
                    
                    mat_id_mapping[scene->mMeshes[i]->mMaterialIndex] = desc.materials.size();
                    
                    desc.materials.push_back(mat);
                }
                
                desc.submeshes[i].material_index = mat_id_mapping[scene->mMeshes[i]->mMaterialIndex];
            }
            
            desc.vertices.resize(vertex_count);
            desc.indices.resize(index_count);
            
            aiMesh* temp_mesh;
            int idx = 0;
            int vertex_index = 0;
            
            for (int i = 0; i < scene->mNumMeshes; i++)
            {
                temp_mesh = scene->mMeshes[i];
                desc.submeshes[i].max_extents = glm::vec3(temp_mesh->mVertices[0].x, temp_mesh->mVertices[0].y, temp_mesh->mVertices[0].z);
                desc.submeshes[i].min_extents = glm::vec3(temp_mesh->mVertices[0].x, temp_mesh->mVertices[0].y, temp_mesh->mVertices[0].z);
                
                for (int k = 0; k < scene->mMeshes[i]->mNumVertices; k++)
                {
                    desc.vertices[vertex_index].position = glm::vec3(temp_mesh->mVertices[k].x, temp_mesh->mVertices[k].y, temp_mesh->mVertices[k].z);
                    glm::vec3 n = glm::vec3(temp_mesh->mNormals[k].x, temp_mesh->mNormals[k].y, temp_mesh->mNormals[k].z);
                    desc.vertices[vertex_index].normal = n;
                    
                    if (temp_mesh->mTangents)
                    {
                        glm::vec3 t = glm::vec3(temp_mesh->mTangents[k].x, temp_mesh->mTangents[k].y, temp_mesh->mTangents[k].z);
                        glm::vec3 b = glm::vec3(temp_mesh->mBitangents[k].x, temp_mesh->mBitangents[k].y, temp_mesh->mBitangents[k].z);
                        
                        // @NOTE: Assuming right handed coordinate space
                        if (glm::dot(glm::cross(n, t), b) < 0.0f)
                            t *= -1.0f; // Flip tangent
                        
                        desc.vertices[vertex_index].tangent = t;
                        desc.vertices[vertex_index].bitangent = b;
                    }
                    
                    if (temp_mesh->HasTextureCoords(0))
                    {
                        desc.vertices[vertex_index].tex_coord = glm::vec2(temp_mesh->mTextureCoords[0][k].x, temp_mesh->mTextureCoords[0][k].y);
                    }
                    
                    if (desc.vertices[vertex_index].position.x > desc.submeshes[i].max_extents.x)
                        desc.submeshes[i].max_extents.x = desc.vertices[vertex_index].position.x;
                    if (desc.vertices[vertex_index].position.y > desc.submeshes[i].max_extents.y)
                        desc.submeshes[i].max_extents.y = desc.vertices[vertex_index].position.y;
                    if (desc.vertices[vertex_index].position.z > desc.submeshes[i].max_extents.z)
                        desc.submeshes[i].max_extents.z = desc.vertices[vertex_index].position.z;
                    
                    if (desc.vertices[vertex_index].position.x < desc.submeshes[i].min_extents.x)
                        desc.submeshes[i].min_extents.x = desc.vertices[vertex_index].position.x;
                    if (desc.vertices[vertex_index].position.y < desc.submeshes[i].min_extents.y)
                        desc.submeshes[i].min_extents.y = desc.vertices[vertex_index].position.y;
                    if (desc.vertices[vertex_index].position.z < desc.submeshes[i].min_extents.z)
                        desc.submeshes[i].min_extents.z = desc.vertices[vertex_index].position.z;
                    
                    vertex_index++;
                }
                
                for (int j = 0; j < temp_mesh->mNumFaces; j++)
                {
                    desc.indices[idx] = temp_mesh->mFaces[j].mIndices[0];
                    idx++;
                    desc.indices[idx] = temp_mesh->mFaces[j].mIndices[1];
                    idx++;
                    desc.indices[idx] = temp_mesh->mFaces[j].mIndices[2];
                    idx++;
                }
            }
            
            desc.max_extents = desc.submeshes[0].max_extents;
            desc.min_extents = desc.submeshes[0].min_extents;
            
            for (int i = 0; i < desc.submeshes.size(); i++)
            {
                if (desc.submeshes[i].max_extents.x > desc.max_extents.x)
                    desc.max_extents.x = desc.submeshes[i].max_extents.x;
                if (desc.submeshes[i].max_extents.y > desc.max_extents.y)
                    desc.max_extents.y = desc.submeshes[i].max_extents.y;
                if (desc.submeshes[i].max_extents.z > desc.max_extents.z)
                    desc.max_extents.z = desc.submeshes[i].max_extents.z;
                
                if (desc.submeshes[i].min_extents.x < desc.min_extents.x)
                    desc.min_extents.x = desc.submeshes[i].min_extents.x;
                if (desc.submeshes[i].min_extents.y < desc.min_extents.y)
                    desc.min_extents.y = desc.submeshes[i].min_extents.y;
                if (desc.submeshes[i].min_extents.z < desc.min_extents.z)
                    desc.min_extents.z = desc.submeshes[i].min_extents.z;
            }
            
            return true;
        }
    
        return false;
    }
    
    bool import_texture(const std::string& path, ImageDesc& desc)
    {
        auto ext = filesystem::get_file_extention(path);
        
        if (ext == "dds")
        {
            // Use NVTT for DDS files
        }
        else
        {
            // Use STB for everything else
        }
        
        return false;
    }
}
