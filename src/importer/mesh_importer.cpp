#include <importer/mesh_importer.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>
#include <unordered_map>
#include <unordered_set>
#include <common/filesystem.h>
#include <chrono>
#include <filesystem>

namespace ast
{
void set_texture_path(aiString in_string, std::string& out_string)
{
    out_string = in_string.C_Str();
    std::replace(out_string.begin(), out_string.end(), '\\', '/');
}

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

std::string get_gltf_metallic_roughness_texture_path(aiMaterial* material)
{
    aiString path;
    aiReturn result = material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &path);

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

bool does_material_exist(std::vector<uint32_t>& materials, uint32_t& current_material)
{
    for (auto it : materials)
    {
        if (it == current_material)
            return true;
    }

    return false;
}

int32_t find_material_index(std::vector<Material>& materials, std::string& current_material)
{
    for (int32_t i = 0; i < materials.size(); i++)
    {
        if (materials[i].name == current_material)
            return i;
    }

    return -1;
}

bool import_mesh(const std::string& file, Mesh& mesh, MeshImportOptions options)
{
    bool        is_gltf   = false;
    std::string extension = filesystem::get_file_extention(file);

    if (extension == "gltf" || extension == "glb")
        is_gltf = true;

    std::filesystem::path absolute_file_path = std::filesystem::path(file);

    if (!absolute_file_path.is_absolute())
        absolute_file_path = std::filesystem::path(std::filesystem::current_path().string() + "/" + file);

    printf("Importing Mesh...\n\n");

    auto start = std::chrono::high_resolution_clock::now();

    const aiScene*   scene;
    Assimp::Importer importer;

    scene = importer.ReadFile(absolute_file_path.string().c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (scene)
    {
        mesh.name = filesystem::get_filename(file);

        mesh.name.erase(std::remove(mesh.name.begin(), mesh.name.end(), ':'), mesh.name.end());
        mesh.name.erase(std::remove(mesh.name.begin(), mesh.name.end(), '.'), mesh.name.end());

        mesh.submeshes.resize(scene->mNumMeshes);

        uint32_t                               vertex_count = 0;
        uint32_t                               index_count  = 0;
        aiMaterial*                            temp_material;
        std::vector<uint32_t>                  processed_mat_ids;
        std::unordered_set<std::string>        processed_mat_names;
        std::vector<uint32_t>                  temp_indices;
        std::unordered_map<uint32_t, uint32_t> mat_id_mapping;
        uint32_t                               unnamed_mats = 1;

        for (int i = 0; i < scene->mNumMeshes; i++)
        {
            std::string submesh_name = scene->mMeshes[i]->mName.C_Str();

            if (submesh_name.length() == 0)
                submesh_name = "submesh_" + std::to_string(i);

            strcpy(mesh.submeshes[i].name, submesh_name.c_str());
            mesh.submeshes[i].index_count  = scene->mMeshes[i]->mNumFaces * 3;
            mesh.submeshes[i].vertex_count = scene->mMeshes[i]->mNumVertices;
            mesh.submeshes[i].base_index   = index_count;
            mesh.submeshes[i].base_vertex  = vertex_count;

            vertex_count += scene->mMeshes[i]->mNumVertices;
            index_count += mesh.submeshes[i].index_count;

            if (!does_material_exist(processed_mat_ids, scene->mMeshes[i]->mMaterialIndex))
            {
                temp_material = scene->mMaterials[scene->mMeshes[i]->mMaterialIndex];

                std::string mat_name = temp_material->GetName().C_Str();

                Material mat;

                mat.name = mat_name;
                //mat.name = mesh.name;
                //mat.name += "_";
                //mat.name += mat_name;

                //// Does a material with the same name exist?
                //int32_t name_index = find_material_index(mesh.materials, mat.name);

                //if (name_index != -1)
                //{
                //    mat_id_mapping[scene->mMeshes[i]->mMaterialIndex] = name_index;
                //    processed_mat_ids.push_back(scene->mMeshes[i]->mMaterialIndex);
                //}
                //else
                {
#if defined(MATERIAL_LOG)
                    printf("------Material Start: %s------\n", temp_material->GetName().C_Str());
#endif

                    mat.name.erase(std::remove(mat.name.begin(), mat.name.end(), ':'), mat.name.end());
                    mat.name.erase(std::remove(mat.name.begin(), mat.name.end(), '.'), mat.name.end());

                    if (mat.name.size() == 0 || mat.name == " ")
                    {
                        mat.name = mesh.name;
                        mat.name += "_unnamed_material_";
                        mat.name += std::to_string(unnamed_mats++);
                    }

                    mat.double_sided  = false;
                    mat.alpha_mask    = false;
                    mat.material_type = MATERIAL_OPAQUE;
                    mat.shading_model = SHADING_MODEL_STANDARD;

                    // Base Color 
                    {
                        aiString base_color_path("");
                        aiReturn base_color_texture_found = temp_material->GetTexture(aiTextureType_BASE_COLOR, 0, &base_color_path);

                        if (base_color_texture_found == aiReturn_FAILURE)
                            base_color_texture_found = temp_material->GetTexture(aiTextureType_DIFFUSE, 0, &base_color_path);

                        if (base_color_texture_found == aiReturn_SUCCESS)
                        {
                            set_texture_path(base_color_path, mat.base_color_texture.path);
#if defined(MATERIAL_LOG)
                            printf("Base Color Path: %s \n", mat.base_color_texture.path.c_str());
#endif
                            mat.base_color_texture.srgb = true;

                            aiUVTransform transform;

                            if (temp_material->Get(_AI_MATKEY_UVTRANSFORM_BASE, aiTextureType_BASE_COLOR, 0, transform) == aiReturn_SUCCESS)
                            {
                                mat.base_color_texture.offset = glm::vec2(transform.mTranslation.x, transform.mTranslation.y);
                                mat.base_color_texture.scale  = glm::vec2(transform.mScaling.x, transform.mScaling.y);
                            }
                        }
                        else
                        {
                            aiColor4D base_color = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
                            
                            aiReturn base_color_factor_found = temp_material->Get(AI_MATKEY_BASE_COLOR, base_color);
                            
                            if (base_color_factor_found == aiReturn_FAILURE)
                                base_color_factor_found = temp_material->Get(AI_MATKEY_COLOR_DIFFUSE, base_color);

                            mat.base_color = glm::vec4(base_color.r, base_color.g, base_color.b, base_color.a);
#if defined(MATERIAL_LOG)
                            printf("Base Color Color: %f, %f, %f, %f \n", base_color.r, base_color.g, base_color.b, base_color.a);
#endif
                        }
                    }

                    // Roughness/Metallic
                    {
                        if (is_gltf || options.is_orca_mesh)
                        {
                            aiString roughness_metallic_path("");
                            aiReturn roughness_metallic_texture_found = options.is_orca_mesh ? temp_material->GetTexture(aiTextureType_SPECULAR, 0, &roughness_metallic_path) : temp_material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &roughness_metallic_path);

                            if (roughness_metallic_texture_found == aiReturn_SUCCESS)
                            {
                                set_texture_path(roughness_metallic_path, mat.roughness_texture.path);
                                set_texture_path(roughness_metallic_path, mat.metallic_texture.path);
#if defined(MATERIAL_LOG)
                                printf("Roughness Metallic Path: %s \n", mat.roughness_texture.path.c_str());
#endif

                                aiUVTransform transform;

                                aiReturn transform_found = options.is_orca_mesh ? temp_material->Get(_AI_MATKEY_UVTRANSFORM_BASE, aiTextureType_SPECULAR, 0, transform) : temp_material->Get(_AI_MATKEY_UVTRANSFORM_BASE, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, transform);

                                if (transform_found == aiReturn_SUCCESS)
                                {
                                    mat.roughness_texture.offset = glm::vec2(transform.mTranslation.x, transform.mTranslation.y);
                                    mat.roughness_texture.scale  = glm::vec2(transform.mScaling.x, transform.mScaling.y);

                                    mat.metallic_texture.offset = glm::vec2(transform.mTranslation.x, transform.mTranslation.y);
                                    mat.metallic_texture.scale  = glm::vec2(transform.mScaling.x, transform.mScaling.y);
                                }
                            }
                            else
                            {

                                aiReturn roughness_factor_found = temp_material->Get(AI_MATKEY_ROUGHNESS_FACTOR, mat.roughness);
                                aiReturn metallic_factor_found = temp_material->Get(AI_MATKEY_METALLIC_FACTOR, mat.metallic);

                                if (roughness_factor_found == aiReturn_FAILURE)
                                    mat.roughness = 1.0f;

                                if (metallic_factor_found == aiReturn_FAILURE)
                                    mat.metallic = 0.0f;
#if defined(MATERIAL_LOG)
                                printf("Roughness: %f \n", mat.roughness);
                                printf("Metallic: %f \n", mat.metallic);
#endif
                            }
                        }
                        else
                        {
                            aiString roughness_path("");
                            aiReturn roughness_texture_found = temp_material->GetTexture(aiTextureType_SHININESS, 0, &roughness_path);

                            if (roughness_texture_found == aiReturn_SUCCESS)
                            {
                                set_texture_path(roughness_path, mat.roughness_texture.path);
#if defined(MATERIAL_LOG)
                                printf("Roughness Path: %s \n", mat.roughness_texture.path.c_str());
#endif
                                aiUVTransform transform;

                                if (temp_material->Get(_AI_MATKEY_UVTRANSFORM_BASE, aiTextureType_SHININESS, 0, transform) == aiReturn_SUCCESS)
                                {
                                    mat.roughness_texture.offset = glm::vec2(transform.mTranslation.x, transform.mTranslation.y);
                                    mat.roughness_texture.scale  = glm::vec2(transform.mScaling.x, transform.mScaling.y);
                                }
                            }
                            else
                                mat.roughness = 1.0f;

                            aiString metallic_path("");
                            aiReturn metallic_texture_found = temp_material->GetTexture(aiTextureType_AMBIENT, 0, &metallic_path);

                            if (metallic_texture_found == aiReturn_SUCCESS)
                            {
                                set_texture_path(metallic_path, mat.metallic_texture.path);
#if defined(MATERIAL_LOG)
                                printf("Metallic Path: %s \n", mat.metallic_texture.path.c_str());
#endif
                                aiUVTransform transform;

                                if (temp_material->Get(_AI_MATKEY_UVTRANSFORM_BASE, aiTextureType_AMBIENT, 0, transform) == aiReturn_SUCCESS)
                                {
                                    mat.metallic_texture.offset = glm::vec2(transform.mTranslation.x, transform.mTranslation.y);
                                    mat.metallic_texture.scale  = glm::vec2(transform.mScaling.x, transform.mScaling.y);
                                }
                            }
                            else
                                mat.metallic = 0.0f;
                        }
                    }

                    // Emissive 
                    {
                    }

                    // Normal
                    {
                    }

                    // Try to find Emissive texture
                    std::string emissive_path = get_texture_path(temp_material, aiTextureType_EMISSIVE);

                    if (emissive_path.empty())
                    {
                        aiColor3D emissive;

                        // Try loading in a Emissive material property
                        if (temp_material->Get(AI_MATKEY_COLOR_EMISSIVE, emissive))
                        {
#if defined(MATERIAL_LOG)
                            printf("Emissive Color: %f, %f, %f \n", emissive.r, emissive.g, emissive.b);
#endif
                            MaterialProperty property;

                            property.type          = PROPERTY_EMISSIVE;
                            property.vec4_value[0] = emissive.r;
                            property.vec4_value[1] = emissive.g;
                            property.vec4_value[2] = emissive.b;
                            property.vec4_value[3] = 1.0f;

                            mat.properties.push_back(property);
                        }
                    }
                    else
                    {
#if defined(MATERIAL_LOG)
                        printf("Emissive Path: %s \n", emissive_path.c_str());
#endif
                        std::replace(emissive_path.begin(), emissive_path.end(), '\\', '/');

                        Texture tex_desc;

                        tex_desc.srgb = false;
                        tex_desc.type = TEXTURE_EMISSIVE;
                        tex_desc.path = emissive_path;

                        mat.textures.push_back(tex_desc);
                    }

                    // Try to find Normal texture
                    std::string normal_path = get_texture_path(temp_material, aiTextureType_NORMALS);

                    if (options.displacement_as_normal)
                        normal_path = get_texture_path(temp_material, aiTextureType_HEIGHT);

                    if (!normal_path.empty())
                    {
#if defined(MATERIAL_LOG)
                        printf("Normal Path: %s \n", normal_path.c_str());
#endif
                        std::replace(normal_path.begin(), normal_path.end(), '\\', '/');

                        Texture mat_desc;

                        mat_desc.srgb = false;
                        mat_desc.type = TEXTURE_NORMAL;
                        mat_desc.path = normal_path;

                        mat.textures.push_back(mat_desc);
                    }

                    // Try to find Height texture
                    if (!options.displacement_as_normal)
                    {
                        std::string height_path = get_texture_path(temp_material, aiTextureType_HEIGHT);

                        if (!height_path.empty())
                        {
                            std::replace(height_path.begin(), height_path.end(), '\\', '/');

                            Texture tex_desc;

                            tex_desc.srgb = false;
                            tex_desc.type = TEXTURE_DISPLACEMENT;
                            tex_desc.path = height_path;

                            mat.textures.push_back(tex_desc);
                        }
                    }

                    mat_id_mapping[scene->mMeshes[i]->mMaterialIndex] = mesh.materials.size();
                    processed_mat_ids.push_back(scene->mMeshes[i]->mMaterialIndex);

                    mesh.materials.push_back(mat);
                }

#if defined(MATERIAL_LOG)
                printf("------Material End------\n\n");
#endif
            }

            mesh.submeshes[i].material_index = mat_id_mapping[scene->mMeshes[i]->mMaterialIndex];
        }

        mesh.vertices.resize(vertex_count);
        mesh.indices.resize(index_count);
        temp_indices.resize(index_count);

        aiMesh* temp_mesh;
        int     idx          = 0;
        int     vertex_index = 0;

        for (int i = 0; i < scene->mNumMeshes; i++)
        {
            temp_mesh                     = scene->mMeshes[i];
            mesh.submeshes[i].max_extents = glm::vec3(temp_mesh->mVertices[0].x, temp_mesh->mVertices[0].y, temp_mesh->mVertices[0].z);
            mesh.submeshes[i].min_extents = glm::vec3(temp_mesh->mVertices[0].x, temp_mesh->mVertices[0].y, temp_mesh->mVertices[0].z);

            for (int k = 0; k < scene->mMeshes[i]->mNumVertices; k++)
            {
                mesh.vertices[vertex_index].position = glm::vec3(temp_mesh->mVertices[k].x, temp_mesh->mVertices[k].y, temp_mesh->mVertices[k].z);
                glm::vec3 n                          = glm::vec3(temp_mesh->mNormals[k].x, temp_mesh->mNormals[k].y, temp_mesh->mNormals[k].z);
                mesh.vertices[vertex_index].normal   = n;

                if (temp_mesh->mTangents)
                {
                    glm::vec3 t = glm::vec3(temp_mesh->mTangents[k].x, temp_mesh->mTangents[k].y, temp_mesh->mTangents[k].z);
                    glm::vec3 b = glm::vec3(temp_mesh->mBitangents[k].x, temp_mesh->mBitangents[k].y, temp_mesh->mBitangents[k].z);

                    // @NOTE: Assuming right handed coordinate space
                    if (glm::dot(glm::cross(n, t), b) < 0.0f)
                        t *= -1.0f; // Flip tangent

                    mesh.vertices[vertex_index].tangent   = t;
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
                temp_indices[idx] = temp_mesh->mFaces[j].mIndices[0];
                idx++;
                temp_indices[idx] = temp_mesh->mFaces[j].mIndices[1];
                idx++;
                temp_indices[idx] = temp_mesh->mFaces[j].mIndices[2];
                idx++;
            }
        }

        int count = 0;

        for (int i = 0; i < mesh.submeshes.size(); i++)
        {
            SubMesh& submesh = mesh.submeshes[i];

            for (int idx = submesh.base_index; idx < (submesh.base_index + submesh.index_count); idx++)
                mesh.indices[count++] = submesh.base_vertex + temp_indices[idx];

            submesh.base_vertex = 0;
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

        auto                          finish = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time   = finish - start;

        printf("Successfully imported mesh in %f seconds\n\n", time.count());

        return true;
    }

    return false;
}
} // namespace ast
