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

void read_standard_material(aiMaterial* assimp_material, Material* material, bool is_gltf, MeshImportOptions& options)
{
    // Base Color
    {
        aiString base_color_path("");
        aiReturn base_color_texture_found = assimp_material->GetTexture(aiTextureType_BASE_COLOR, 0, &base_color_path);

        if (base_color_texture_found == aiReturn_FAILURE)
            base_color_texture_found = assimp_material->GetTexture(aiTextureType_DIFFUSE, 0, &base_color_path);

        if (base_color_texture_found == aiReturn_SUCCESS)
        {
            set_texture_path(base_color_path, material->base_color_texture.path);
#if defined(MATERIAL_LOG)
            printf("Base Color Path: %s \n", material->base_color_texture.path.c_str());
#endif
            material->base_color_texture.srgb = true;

            aiUVTransform transform;

            if (assimp_material->Get(_AI_MATKEY_UVTRANSFORM_BASE, aiTextureType_BASE_COLOR, 0, transform) == aiReturn_SUCCESS)
            {
                material->base_color_texture.offset = glm::vec2(transform.mTranslation.x, transform.mTranslation.y);
                material->base_color_texture.scale  = glm::vec2(transform.mScaling.x, transform.mScaling.y);
            }
        }
        else
        {
            aiColor4D base_color = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);

            aiReturn base_color_factor_found = assimp_material->Get(AI_MATKEY_BASE_COLOR, base_color);

            if (base_color_factor_found == aiReturn_FAILURE)
                base_color_factor_found = assimp_material->Get(AI_MATKEY_COLOR_DIFFUSE, base_color);

            material->base_color = glm::vec4(base_color.r, base_color.g, base_color.b, base_color.a);
#if defined(MATERIAL_LOG)
            printf("Base Color Color: %f, %f, %f, %f \n", base_color.r, base_color.g, base_color.b, base_color.a);
#endif
        }
    }

    // Roughness/Metallic
    {
        if (is_gltf || options.is_orca_mesh)
        {
            aiTextureType target_texture_type = aiTextureType_DIFFUSE_ROUGHNESS;

            if (options.is_orca_mesh)
                target_texture_type = aiTextureType_SPECULAR;

            aiString roughness_metallic_path("");
            aiReturn roughness_metallic_texture_found = assimp_material->GetTexture(target_texture_type, 0, &roughness_metallic_path);

            if (roughness_metallic_texture_found == aiReturn_SUCCESS)
            {
                set_texture_path(roughness_metallic_path, material->roughness_texture.path);
                set_texture_path(roughness_metallic_path, material->metallic_texture.path);

                material->roughness_texture.channel_idx = 1;
                material->metallic_texture.channel_idx  = 2;
#if defined(MATERIAL_LOG)
                printf("Roughness Metallic Path: %s \n", material->roughness_texture.path.c_str());
#endif

                aiUVTransform transform;

                aiReturn transform_found = assimp_material->Get(_AI_MATKEY_UVTRANSFORM_BASE, target_texture_type, 0, transform);

                if (transform_found == aiReturn_SUCCESS)
                {
                    material->roughness_texture.offset = glm::vec2(transform.mTranslation.x, transform.mTranslation.y);
                    material->roughness_texture.scale  = glm::vec2(transform.mScaling.x, transform.mScaling.y);

                    material->metallic_texture.offset = glm::vec2(transform.mTranslation.x, transform.mTranslation.y);
                    material->metallic_texture.scale  = glm::vec2(transform.mScaling.x, transform.mScaling.y);
                }
            }
            else
            {
                aiReturn roughness_factor_found = assimp_material->Get(AI_MATKEY_ROUGHNESS_FACTOR, material->roughness);
                aiReturn metallic_factor_found  = assimp_material->Get(AI_MATKEY_METALLIC_FACTOR, material->metallic);

                if (roughness_factor_found == aiReturn_FAILURE)
                    material->roughness = 1.0f;

                if (metallic_factor_found == aiReturn_FAILURE)
                    material->metallic = 0.0f;
#if defined(MATERIAL_LOG)
                printf("Roughness: %f \n", material->roughness);
                printf("Metallic: %f \n", material->metallic);
#endif
            }
        }
        else
        {
            aiString roughness_path("");
            aiReturn roughness_texture_found = assimp_material->GetTexture(aiTextureType_SHININESS, 0, &roughness_path);

            if (roughness_texture_found == aiReturn_SUCCESS)
            {
                set_texture_path(roughness_path, material->roughness_texture.path);
#if defined(MATERIAL_LOG)
                printf("Roughness Path: %s \n", material->roughness_texture.path.c_str());
#endif
                aiUVTransform transform;

                if (assimp_material->Get(_AI_MATKEY_UVTRANSFORM_BASE, aiTextureType_SHININESS, 0, transform) == aiReturn_SUCCESS)
                {
                    material->roughness_texture.offset = glm::vec2(transform.mTranslation.x, transform.mTranslation.y);
                    material->roughness_texture.scale  = glm::vec2(transform.mScaling.x, transform.mScaling.y);
                }
            }
            else
                material->roughness = 1.0f;

            aiString metallic_path("");
            aiReturn metallic_texture_found = assimp_material->GetTexture(aiTextureType_AMBIENT, 0, &metallic_path);

            if (metallic_texture_found == aiReturn_SUCCESS)
            {
                set_texture_path(metallic_path, material->metallic_texture.path);
#if defined(MATERIAL_LOG)
                printf("Metallic Path: %s \n", material->metallic_texture.path.c_str());
#endif
                aiUVTransform transform;

                if (assimp_material->Get(_AI_MATKEY_UVTRANSFORM_BASE, aiTextureType_AMBIENT, 0, transform) == aiReturn_SUCCESS)
                {
                    material->metallic_texture.offset = glm::vec2(transform.mTranslation.x, transform.mTranslation.y);
                    material->metallic_texture.scale  = glm::vec2(transform.mScaling.x, transform.mScaling.y);
                }
            }
            else
                material->metallic = 0.0f;
        }
    }

    // Normal
    {
        // Try to find Normal texture
        aiTextureType target_texture_type = aiTextureType_NORMALS;

        if (options.displacement_as_normal)
            target_texture_type = aiTextureType_HEIGHT;

        aiString normal_path("");
        aiReturn normal_texture_found = assimp_material->GetTexture(target_texture_type, 0, &normal_path);

        if (normal_texture_found == aiReturn_SUCCESS)
        {
            set_texture_path(normal_path, material->normal_texture.path);
#if defined(MATERIAL_LOG)
            printf("Normal Path: %s \n", material->normal_texture.path.c_str());
#endif

            aiUVTransform transform;

            if (assimp_material->Get(_AI_MATKEY_UVTRANSFORM_BASE, target_texture_type, 0, transform) == aiReturn_SUCCESS)
            {
                material->normal_texture.offset = glm::vec2(transform.mTranslation.x, transform.mTranslation.y);
                material->normal_texture.scale  = glm::vec2(transform.mScaling.x, transform.mScaling.y);
            }
        }
    }

    // Displacement
    {
        // Try to find Displacement texture
        if (!options.displacement_as_normal)
        {
            aiTextureType target_texture_type = aiTextureType_DISPLACEMENT;

            aiString displacement_path("");
            aiReturn displacement_texture_found = assimp_material->GetTexture(target_texture_type, 0, &displacement_path);

            if (options.displacement_as_normal)
            {
                target_texture_type        = aiTextureType_HEIGHT;
                displacement_texture_found = assimp_material->GetTexture(target_texture_type, 0, &displacement_path);
            }

            if (displacement_texture_found == aiReturn_SUCCESS)
            {
                set_texture_path(displacement_path, material->displacement_texture.path);
#if defined(MATERIAL_LOG)
                printf("Normal Path: %s \n", material->displacement_texture.path.c_str());
#endif

                aiUVTransform transform;

                if (assimp_material->Get(_AI_MATKEY_UVTRANSFORM_BASE, target_texture_type, 0, transform) == aiReturn_SUCCESS)
                {
                    material->displacement_texture.offset = glm::vec2(transform.mTranslation.x, transform.mTranslation.y);
                    material->displacement_texture.scale  = glm::vec2(transform.mScaling.x, transform.mScaling.y);
                }
            }
        }
    }

    // Emissive
    {
        // Try to find Emissive texture
        aiTextureType target_texture_type = aiTextureType_EMISSION_COLOR;
        aiString      emissive_path("");
        aiReturn      emissive_texture_found = assimp_material->GetTexture(target_texture_type, 0, &emissive_path);

        if (emissive_texture_found != aiReturn_SUCCESS)
        {
            target_texture_type    = aiTextureType_EMISSIVE;
            emissive_texture_found = assimp_material->GetTexture(target_texture_type, 0, &emissive_path);
        }

        if (emissive_texture_found == aiReturn_SUCCESS)
        {
            set_texture_path(emissive_path, material->emissive_texture.path);
#if defined(MATERIAL_LOG)
            printf("Emissive Path: %s \n", material->emissive_texture.path.c_str());
#endif
            aiUVTransform transform;

            if (assimp_material->Get(_AI_MATKEY_UVTRANSFORM_BASE, emissive_texture_found, 0, transform) == aiReturn_SUCCESS)
            {
                material->emissive_texture.offset = glm::vec2(transform.mTranslation.x, transform.mTranslation.y);
                material->emissive_texture.scale  = glm::vec2(transform.mScaling.x, transform.mScaling.y);
            }
        }

        aiColor3D emissive;

        // Try loading in the Emissive Factor property
        if (assimp_material->Get(AI_MATKEY_EMISSIVE_INTENSITY, emissive) == aiReturn_SUCCESS)
        {
#if defined(MATERIAL_LOG)
            printf("Emissive Factor: %f, %f, %f \n", emissive.r, emissive.g, emissive.b);
#endif
            material->emissive_factor = glm::vec3(emissive.r, emissive.g, emissive.b);
        }
    }
}

void read_sheen_material(aiMaterial* assimp_material, Material* material)
{
    // Sheen Color Texture
    {
        aiString path("");
        aiReturn texture_found = assimp_material->GetTexture(AI_MATKEY_SHEEN_COLOR_TEXTURE, &path);

        if (texture_found == aiReturn_SUCCESS)
        {
            set_texture_path(path, material->sheen_color_texture.path);
#if defined(MATERIAL_LOG)
            printf("Sheen Color Path: %s \n", material->sheen_color_texture.path.c_str());
#endif

            aiUVTransform transform;

            if (assimp_material->Get(_AI_MATKEY_UVTRANSFORM_BASE, AI_MATKEY_SHEEN_COLOR_TEXTURE, transform) == aiReturn_SUCCESS)
            {
                material->sheen_color_texture.offset = glm::vec2(transform.mTranslation.x, transform.mTranslation.y);
                material->sheen_color_texture.scale  = glm::vec2(transform.mScaling.x, transform.mScaling.y);
            }

            material->material_type = MATERIAL_CLOTH;
        }
    }

    // Sheen Roughness Texture
    {
        aiString path("");
        aiReturn texture_found = assimp_material->GetTexture(AI_MATKEY_SHEEN_ROUGHNESS_TEXTURE, &path);

        if (texture_found == aiReturn_SUCCESS)
        {
            set_texture_path(path, material->sheen_roughness_texture.path);
#if defined(MATERIAL_LOG)
            printf("Sheen Roughness Path: %s \n", material->sheen_roughness_texture.path.c_str());
#endif

            aiUVTransform transform;

            if (assimp_material->Get(_AI_MATKEY_UVTRANSFORM_BASE, AI_MATKEY_SHEEN_ROUGHNESS_TEXTURE, transform) == aiReturn_SUCCESS)
            {
                material->sheen_roughness_texture.offset = glm::vec2(transform.mTranslation.x, transform.mTranslation.y);
                material->sheen_roughness_texture.scale  = glm::vec2(transform.mScaling.x, transform.mScaling.y);
            }
        }
    }

    // Sheen Color Factor
    {
        aiColor3D sheen_color;
        aiReturn  property_found = assimp_material->Get(AI_MATKEY_SHEEN_COLOR_FACTOR, sheen_color);

        if (property_found == aiReturn_SUCCESS)
        {
            material->sheen_color   = glm::vec3(sheen_color.r, sheen_color.g, sheen_color.b);
            material->material_type = MATERIAL_CLOTH;
        }
    }

    // Sheen Roughness Factor
    {
        assimp_material->Get(AI_MATKEY_SHEEN_ROUGHNESS_FACTOR, material->sheen_roughness);
    }
}

void read_clear_coat_material(aiMaterial* assimp_material, Material* material)
{
    // Clear Coat Texture
    {
        aiString path("");
        aiReturn texture_found = assimp_material->GetTexture(AI_MATKEY_CLEARCOAT_TEXTURE, &path);

        if (texture_found == aiReturn_SUCCESS)
        {
            set_texture_path(path, material->clear_coat_texture.path);
#if defined(MATERIAL_LOG)
            printf("Clear Coat Path: %s \n", material->clear_coat_texture.path.c_str());
#endif

            aiUVTransform transform;

            if (assimp_material->Get(_AI_MATKEY_UVTRANSFORM_BASE, AI_MATKEY_CLEARCOAT_TEXTURE, transform) == aiReturn_SUCCESS)
            {
                material->clear_coat_texture.offset = glm::vec2(transform.mTranslation.x, transform.mTranslation.y);
                material->clear_coat_texture.scale  = glm::vec2(transform.mScaling.x, transform.mScaling.y);
            }

            material->material_type = MATERIAL_CLEAR_COAT;
        }
    }

    // Clear Coat Roughness
    {
        aiString path("");
        aiReturn texture_found = assimp_material->GetTexture(AI_MATKEY_CLEARCOAT_ROUGHNESS_TEXTURE, &path);

        if (texture_found == aiReturn_SUCCESS)
        {
            set_texture_path(path, material->clear_coat_roughness_texture.path);
#if defined(MATERIAL_LOG)
            printf("Clear Coat Roughness Path: %s \n", material->clear_coat_roughness_texture.path.c_str());
#endif

            aiUVTransform transform;

            if (assimp_material->Get(_AI_MATKEY_UVTRANSFORM_BASE, AI_MATKEY_CLEARCOAT_ROUGHNESS_TEXTURE, transform) == aiReturn_SUCCESS)
            {
                material->clear_coat_roughness_texture.offset = glm::vec2(transform.mTranslation.x, transform.mTranslation.y);
                material->clear_coat_roughness_texture.scale  = glm::vec2(transform.mScaling.x, transform.mScaling.y);
            }
        }
    }

    // Clear Coat Normal
    {
        aiString path("");
        aiReturn texture_found = assimp_material->GetTexture(AI_MATKEY_CLEARCOAT_NORMAL_TEXTURE, &path);

        if (texture_found == aiReturn_SUCCESS)
        {
            set_texture_path(path, material->clear_coat_normal_texture.path);
#if defined(MATERIAL_LOG)
            printf("Clear Coat Normal Path: %s \n", material->clear_coat_normal_texture.path.c_str());
#endif

            aiUVTransform transform;

            if (assimp_material->Get(_AI_MATKEY_UVTRANSFORM_BASE, AI_MATKEY_CLEARCOAT_NORMAL_TEXTURE, transform) == aiReturn_SUCCESS)
            {
                material->clear_coat_normal_texture.offset   = glm::vec2(transform.mTranslation.x, transform.mTranslation.y);
                material->clear_coat_normal_texture.scale  = glm::vec2(transform.mScaling.x, transform.mScaling.y);
            }
        }
    }

    // Clear Coat Factor
    {
        assimp_material->Get(AI_MATKEY_CLEARCOAT_FACTOR, material->clear_coat);
    }

    // Clear Coat Roughness Factor
    {
        assimp_material->Get(AI_MATKEY_CLEARCOAT_ROUGHNESS_FACTOR, material->clear_coat_roughness);
    }
}

void read_anisotropy_material(aiMaterial* assimp_material, Material* material)
{
    // TODO
}

void read_transmission_material(aiMaterial* assimp_material, Material* material)
{
    // Transmission Texture
    {
        aiString path("");
        aiReturn texture_found = assimp_material->GetTexture(AI_MATKEY_TRANSMISSION_TEXTURE, &path);

        if (texture_found == aiReturn_SUCCESS)
        {
            set_texture_path(path, material->transmission_texture.path);
#if defined(MATERIAL_LOG)
            printf("Transmission Path: %s \n", material->transmission_texture.path.c_str());
#endif

            aiUVTransform transform;

            if (assimp_material->Get(_AI_MATKEY_UVTRANSFORM_BASE, AI_MATKEY_TRANSMISSION_TEXTURE, transform) == aiReturn_SUCCESS)
            {
                material->transmission_texture.offset = glm::vec2(transform.mTranslation.x, transform.mTranslation.y);
                material->transmission_texture.scale  = glm::vec2(transform.mScaling.x, transform.mScaling.y);
            }

            material->surface_type = SURFACE_TRANSMISSIVE;
        }
    }

    // Transmission Factor
    {
        assimp_material->Get(AI_MATKEY_TRANSMISSION_FACTOR, material->transmission);
    }
}

void read_volume_material(aiMaterial* assimp_material, Material* material)
{
    // Thickness Texture
    {
        aiString path("");
        aiReturn texture_found = assimp_material->GetTexture(AI_MATKEY_VOLUME_THICKNESS_TEXTURE, &path);

        if (texture_found == aiReturn_SUCCESS)
        {
            set_texture_path(path, material->thickness_texture.path);
#if defined(MATERIAL_LOG)
            printf("Thickness Path: %s \n", material->thickness_texture.path.c_str());
#endif

            aiUVTransform transform;

            if (assimp_material->Get(_AI_MATKEY_UVTRANSFORM_BASE, AI_MATKEY_VOLUME_THICKNESS_TEXTURE, transform) == aiReturn_SUCCESS)
            {
                material->thickness_texture.offset   = glm::vec2(transform.mTranslation.x, transform.mTranslation.y);
                material->thickness_texture.scale  = glm::vec2(transform.mScaling.x, transform.mScaling.y);
            }

            material->surface_type = SURFACE_TRANSMISSIVE;
        }
    }

    // Thickness Factor
    {
        assimp_material->Get(AI_MATKEY_VOLUME_THICKNESS_FACTOR, material->thickness_factor);
    }

    // Attenuation Distance
    {
        assimp_material->Get(AI_MATKEY_VOLUME_ATTENUATION_DISTANCE, material->attenuation_distance);
    }

    // Attenuation Color
    {
        aiColor3D attenuation_color;
        aiReturn  property_found = assimp_material->Get(AI_MATKEY_VOLUME_ATTENUATION_COLOR, attenuation_color);

        if (property_found == aiReturn_SUCCESS)
            material->attenuation_color   = glm::vec3(attenuation_color.r, attenuation_color.g, attenuation_color.b);
    }
}

void read_ior_material(aiMaterial* assimp_material, Material* material)
{
    assimp_material->Get(AI_MATKEY_REFRACTI, material->ior);
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
        mesh.materials.resize(scene->mNumMaterials);

        uint32_t              vertex_count = 0;
        uint32_t              index_count  = 0;
        uint32_t              unnamed_mats = 1;
        std::vector<uint32_t> temp_indices;

        // Read materials.
        for (int i = 0; i < scene->mNumMaterials; i++)
        {
            mesh.materials[i] = std::unique_ptr<Material>(new Material());

            auto& material = mesh.materials[i];

            material->surface_type  = SURFACE_OPAQUE;
            material->material_type = MATERIAL_STANDARD;

            aiMaterial* assimp_material = scene->mMaterials[i];

            std::string mat_name = assimp_material->GetName().C_Str();

            mat_name.erase(std::remove(mat_name.begin(), mat_name.end(), ':'), mat_name.end());
            mat_name.erase(std::remove(mat_name.begin(), mat_name.end(), '.'), mat_name.end());

            // If material has no name, assign a name to it.
            if (mat_name.size() == 0 || mat_name == " ")
            {
                mat_name = mesh.name;
                mat_name += "_unnamed_material_";
                mat_name += std::to_string(unnamed_mats++);
            }

            read_standard_material(assimp_material, material.get(), is_gltf, options);

            read_sheen_material(assimp_material, material.get());

            read_clear_coat_material(assimp_material, material.get());

            read_anisotropy_material(assimp_material, material.get());

            read_transmission_material(assimp_material, material.get());

            read_ior_material(assimp_material, material.get());

            read_volume_material(assimp_material, material.get());
        }

        // Read submesh data.
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

            mesh.submeshes[i].material_index = scene->mMeshes[i]->mMaterialIndex;
        }

        mesh.vertices.resize(vertex_count);
        mesh.indices.resize(index_count);
        temp_indices.resize(index_count);

        aiMesh* temp_mesh;
        int     idx          = 0;
        int     vertex_index = 0;

        // Read vertex data.
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
                    mesh.vertices[vertex_index].tex_coord = glm::vec2(temp_mesh->mTextureCoords[0][k].x, temp_mesh->mTextureCoords[0][k].y);

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

        // Setup each submesh so that base vertex draws are not required.
        for (int i = 0; i < mesh.submeshes.size(); i++)
        {
            SubMesh& submesh = mesh.submeshes[i];

            for (int idx = submesh.base_index; idx < (submesh.base_index + submesh.index_count); idx++)
                mesh.indices[count++] = submesh.base_vertex + temp_indices[idx];

            submesh.base_vertex = 0;
        }

        mesh.max_extents = mesh.submeshes[0].max_extents;
        mesh.min_extents = mesh.submeshes[0].min_extents;

        // Find AABB for entire mesh.
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
