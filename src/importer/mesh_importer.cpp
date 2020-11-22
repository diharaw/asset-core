#include <importer/mesh_importer.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>
#include <unordered_map>
#include <common/filesystem.h>
#include <chrono>
#include <filesystem>

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

std::string get_gltf_base_color_texture_path(aiMaterial* material)
{
    aiString path;
    aiReturn result = material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, &path);

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

    if (extension == "gltf")
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
        std::vector<uint32_t>                  temp_indices;
        std::unordered_map<uint32_t, uint32_t> mat_id_mapping;
        uint32_t                               unnamed_mats = 1;

        for (int i = 0; i < scene->mNumMeshes; i++)
        {
            std::string submesh_name = scene->mMeshes[i]->mName.C_Str();

            if (submesh_name.length() == 0)
                submesh_name = "submesh_" + std::to_string(i);

            strcpy(mesh.submeshes[i].name, submesh_name.c_str());
            mesh.submeshes[i].index_count = scene->mMeshes[i]->mNumFaces * 3;
            mesh.submeshes[i].vertex_count = scene->mMeshes[i]->mNumVertices;
            mesh.submeshes[i].base_index  = index_count;
            mesh.submeshes[i].base_vertex = vertex_count;

            vertex_count += scene->mMeshes[i]->mNumVertices;
            index_count += mesh.submeshes[i].index_count;

            if (!does_material_exist(processed_mat_ids, scene->mMeshes[i]->mMaterialIndex))
            {
                temp_material = scene->mMaterials[scene->mMeshes[i]->mMaterialIndex];

                Material mat;

                mat.name = mesh.name;
                mat.name += "_";
                mat.name += temp_material->GetName().C_Str();

                // Does a material with the same name exist?
                int32_t name_index = find_material_index(mesh.materials, mat.name);

                if (name_index != -1)
                {
                    mat_id_mapping[scene->mMeshes[i]->mMaterialIndex] = name_index;
                    processed_mat_ids.push_back(scene->mMeshes[i]->mMaterialIndex);
                }
                else
                {
#if defined(MATERIAL_LOG)
                    printf("------Material Start: %s------\n", temp_material->GetName().C_Str());
#endif

                    mat.name.erase(std::remove(mat.name.begin(), mat.name.end(), ':'), mat.name.end());
                    mat.name.erase(std::remove(mat.name.begin(), mat.name.end(), '.'), mat.name.end());

                    if (mat.name.empty())
                    {
                        mat.name = mesh.name;
                        mat.name += "_unnamed_material_";
                        mat.name += std::to_string(unnamed_mats++);
                    }

                    mat.double_sided  = false;
                    mat.alpha_mask    = false;
                    mat.material_type = MATERIAL_OPAQUE;
                    mat.shading_model = SHADING_MODEL_STANDARD;

                    std::string albedo_path = "";

                    // If this is a GLTF, try to find the base color texture path
                    if (is_gltf)
                        albedo_path = get_gltf_base_color_texture_path(temp_material);
                    else
                    {
                        // If not, try to find the Diffuse texture path
                        albedo_path = get_texture_path(temp_material, aiTextureType_DIFFUSE);

                        // If that doesn't exist, try to find Diffuse texture
                        if (albedo_path.empty())
                            albedo_path = get_texture_path(temp_material, aiTextureType_BASE_COLOR);
                    }

                    if (albedo_path.empty())
                    {
                        aiColor3D diffuse = aiColor3D(1.0f, 1.0f, 1.0f);
                        float     alpha   = 1.0f;

                        // Try loading in a Diffuse material property
                        if (temp_material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) != AI_SUCCESS)
                            temp_material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, diffuse);

                        temp_material->Get(AI_MATKEY_OPACITY, alpha);
#if defined(MATERIAL_LOG)
                        printf("Albedo Color: %f, %f, %f \n", diffuse.r, diffuse.g, diffuse.b);
#endif

                        MaterialProperty property;

                        property.type          = PROPERTY_ALBEDO;
                        property.vec4_value[0] = diffuse.r;
                        property.vec4_value[1] = diffuse.g;
                        property.vec4_value[2] = diffuse.b;
                        property.vec4_value[3] = alpha;

                        mat.properties.push_back(property);
                    }
                    else
                    {
#if defined(MATERIAL_LOG)
                        printf("Albedo Path: %s \n", albedo_path.c_str());
#endif
                        std::replace(albedo_path.begin(), albedo_path.end(), '\\', '/');

                        Texture tex_desc;

                        tex_desc.srgb = true;
                        tex_desc.type = TEXTURE_ALBEDO;
                        tex_desc.path = albedo_path;

                        mat.textures.push_back(tex_desc);
                    }

                    if (options.is_orca_mesh)
                    {
                        std::string roughness_metallic_path = get_texture_path(temp_material, aiTextureType_SPECULAR);

                        if (roughness_metallic_path.empty())
                        {
                            MaterialProperty roughness_property;

                            roughness_property.type        = PROPERTY_ROUGHNESS;
                            roughness_property.float_value = 1.0f;

                            mat.properties.push_back(roughness_property);

                            MaterialProperty metallic_property;

                            metallic_property.type        = PROPERTY_METALLIC;
                            metallic_property.float_value = 0.0f;

                            mat.properties.push_back(metallic_property);
                        }
                        else
                        {
#if defined(MATERIAL_LOG)
                            printf("Roughness Metallic Path: %s \n", roughness_metallic_path.c_str());
#endif
                            std::replace(roughness_metallic_path.begin(), roughness_metallic_path.end(), '\\', '/');

                            Texture roughness_tex_desc;

                            roughness_tex_desc.srgb          = false;
                            roughness_tex_desc.type          = TEXTURE_ROUGHNESS;
                            roughness_tex_desc.path          = roughness_metallic_path;
                            roughness_tex_desc.channel_index = 1;

                            mat.textures.push_back(roughness_tex_desc);

                            Texture metallic_tex_desc;

                            metallic_tex_desc.srgb          = false;
                            metallic_tex_desc.type          = TEXTURE_METALLIC;
                            metallic_tex_desc.path          = roughness_metallic_path;
                            metallic_tex_desc.channel_index = 2;

                            mat.textures.push_back(metallic_tex_desc);
                        }
                    }
                    else
                    {
                        // Try to find Roughness texture
                        std::string roughness_path = get_texture_path(temp_material, aiTextureType_SHININESS);

                        if (roughness_path.empty())
                            roughness_path = get_gltf_metallic_roughness_texture_path(temp_material);

                        if (roughness_path.empty())
                        {
                            float roughness = 0.0f;

                            // Try loading in a Diffuse material property
                            temp_material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, roughness);
#if defined(MATERIAL_LOG)
                            printf("Roughness Color: %f \n", roughness);
#endif
                            MaterialProperty property;

                            property.type        = PROPERTY_ROUGHNESS;
                            property.float_value = roughness;

                            mat.properties.push_back(property);
                        }
                        else
                        {
#if defined(MATERIAL_LOG)
                            printf("Roughness Path: %s \n", roughness_path.c_str());
#endif
                            std::replace(roughness_path.begin(), roughness_path.end(), '\\', '/');

                            Texture tex_desc;

                            tex_desc.srgb          = false;
                            tex_desc.type          = TEXTURE_ROUGHNESS;
                            tex_desc.path          = roughness_path;
                            tex_desc.channel_index = is_gltf ? 1 : 0;

                            mat.textures.push_back(tex_desc);
                        }

                        // Try to find Metallic texture
                        std::string metallic_path = get_texture_path(temp_material, aiTextureType_AMBIENT);

                        if (metallic_path.empty())
                            metallic_path = get_gltf_metallic_roughness_texture_path(temp_material);

                        if (metallic_path.empty())
                        {
                            float metallic = 0.0f;

                            // Try loading in a Diffuse material property
                            temp_material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, metallic);
#if defined(MATERIAL_LOG)
                            printf("Metallic Color: %f \n", metallic);
#endif
                            MaterialProperty property;

                            property.type        = PROPERTY_METALLIC;
                            property.float_value = metallic;

                            mat.properties.push_back(property);
                        }
                        else
                        {
#if defined(MATERIAL_LOG)
                            printf("Metallic Path: %s \n", metallic_path.c_str());
#endif
                            std::replace(metallic_path.begin(), metallic_path.end(), '\\', '/');

                            Texture tex_desc;

                            tex_desc.srgb          = false;
                            tex_desc.type          = TEXTURE_METALLIC;
                            tex_desc.path          = metallic_path;
                            tex_desc.channel_index = is_gltf ? 2 : 0;

                            mat.textures.push_back(tex_desc);
                        }
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
