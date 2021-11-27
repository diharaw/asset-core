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
bool find_texture(aiMaterial* assimp_material, aiTextureType texture_type, uint32_t index, std::string base_path, Material* material, TextureRef& out_texture_ref, std::unordered_map<std::string, TextureRef>& texture_refs)
{
    aiString ai_path("");
    aiReturn texture_found = assimp_material->GetTexture(texture_type, index, &ai_path);

    if (texture_found == aiReturn_FAILURE)
        return false;

    std::string out_path = ai_path.C_Str();
    std::replace(out_path.begin(), out_path.end(), '\\', '/');

    if (out_path[0] == '.' && out_path[1] == '/')
        out_path = out_path.substr(2, out_path.length() - 1);

    out_path = base_path + out_path;

    if (texture_refs.find(out_path) != texture_refs.end())
        out_texture_ref = texture_refs[out_path];
    else
    {
        out_texture_ref.texture_idx = material->textures.size();
        
        aiUVTransform transform;

        if (assimp_material->Get(_AI_MATKEY_UVTRANSFORM_BASE, texture_type, index, transform) == aiReturn_SUCCESS)
        {
            out_texture_ref.offset = glm::vec2(transform.mTranslation.x, transform.mTranslation.y);
            out_texture_ref.scale  = glm::vec2(transform.mScaling.x, transform.mScaling.y);
        }

        texture_refs[out_path] = out_texture_ref;

        TextureInfo texture_info;

        texture_info.path = out_path;
        texture_info.srgb = (texture_type == aiTextureType_BASE_COLOR || texture_type == aiTextureType_DIFFUSE) ? true : false;

        material->textures.push_back(texture_info);
    }

    return true;
}

void read_standard_material(std::string mesh_path, aiMaterial* assimp_material, Material* material, std::unordered_map<std::string, TextureRef>& texture_refs, bool is_gltf, MeshImportOptions& options)
{
    // Base Color
    {
        bool base_color_texture_found = find_texture(assimp_material, aiTextureType_BASE_COLOR, 0, mesh_path, material, material->base_color_texture, texture_refs);

        if (!base_color_texture_found)
            base_color_texture_found = find_texture(assimp_material, aiTextureType_DIFFUSE, 0, mesh_path, material, material->base_color_texture, texture_refs);

#if defined(MATERIAL_LOG)
        if (base_color_texture_found)
            printf("Base Color Path: %s \n", material->textures[material, material->base_color_texture.texture_idx].c_str());
#endif

        if (!base_color_texture_found)
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

            bool roughness_metallic_texture_found = find_texture(assimp_material, target_texture_type, 0, mesh_path, material, material->roughness_texture, texture_refs);
            find_texture(assimp_material, target_texture_type, 0, mesh_path, material, material->metallic_texture, texture_refs);

#if defined(MATERIAL_LOG)
            if (roughness_metallic_texture_found)
                printf("Roughness Metallic Path: %s \n", material->textures[material, material->roughness_texture.texture_idx].c_str());
#endif

            if (!roughness_metallic_texture_found)
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
            bool     roughness_texture_found = find_texture(assimp_material, aiTextureType_SHININESS, 0, mesh_path, material, material->roughness_texture, texture_refs);

#if defined(MATERIAL_LOG)
            if (roughness_texture_found)
                printf("Roughness Path: %s \n", material->textures[material, material->roughness_texture.texture_idx].c_str());
#endif

            if (!roughness_texture_found)
                material->roughness = 1.0f;

            aiString metallic_path("");
            bool     metallic_texture_found = find_texture(assimp_material, aiTextureType_AMBIENT, 0, mesh_path, material, material->metallic_texture, texture_refs);

#if defined(MATERIAL_LOG)
            if (metallic_texture_found)
                printf("Metallic Path: %s \n", material->textures[material, material->metallic_texture.texture_idx].c_str());
#endif

            if (!metallic_texture_found)
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
        bool     normal_texture_found = find_texture(assimp_material, target_texture_type, 0, mesh_path, material, material->normal_texture, texture_refs);

#if defined(MATERIAL_LOG)
        if (normal_texture_found)
            printf("Normal Path: %s \n", material->textures[material, material->normal_texture.texture_idx].c_str());
#endif
    }

    // Displacement
    {
        // Try to find Displacement texture
        if (!options.displacement_as_normal)
        {
            aiTextureType target_texture_type = aiTextureType_DISPLACEMENT;

            aiString displacement_path("");
            bool     displacement_texture_found = find_texture(assimp_material, target_texture_type, 0, mesh_path, material, material->displacement_texture, texture_refs);

            if (options.displacement_as_normal)
            {
                target_texture_type        = aiTextureType_HEIGHT;
                displacement_texture_found = find_texture(assimp_material, target_texture_type, 0, mesh_path, material, material->displacement_texture, texture_refs);
            }

#if defined(MATERIAL_LOG)
            if (displacement_texture_found)
                printf("Displacement Path: %s \n", material->textures[material, material->displacement_texture.texture_idx].c_str());
#endif
        }
    }

    // Emissive
    {
        // Try to find Emissive texture
        aiTextureType target_texture_type = aiTextureType_EMISSION_COLOR;
        aiString      emissive_path("");
        bool          emissive_texture_found = find_texture(assimp_material, target_texture_type, 0, mesh_path, material, material->emissive_texture, texture_refs);

        if (emissive_texture_found != aiReturn_SUCCESS)
        {
            target_texture_type    = aiTextureType_EMISSIVE;
            emissive_texture_found = find_texture(assimp_material, target_texture_type, 0, mesh_path, material, material->emissive_texture, texture_refs);
        }

#if defined(MATERIAL_LOG)
        if (emissive_texture_found)
            printf("Emissive Path: %s \n", material->textures[material, material->emissive_texture.texture_idx].c_str());
#endif

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

void read_sheen_material(std::string mesh_path, aiMaterial* assimp_material, Material* material, std::unordered_map<std::string, TextureRef>& texture_refs)
{
    // Sheen Color Texture
    {
        aiString path("");
        bool     texture_found = find_texture(assimp_material, AI_MATKEY_SHEEN_COLOR_TEXTURE, mesh_path, material, material->sheen_color_texture, texture_refs);

        if (texture_found)
        {
#if defined(MATERIAL_LOG)
            printf("Sheen Color Path: %s \n", material->textures[material, material->sheen_color_texture.texture_idx].c_str());
#endif
            material->material_type = MATERIAL_CLOTH;
        }
    }

    // Sheen Roughness Texture
    {
        aiString path("");
        bool     texture_found = find_texture(assimp_material, AI_MATKEY_SHEEN_ROUGHNESS_TEXTURE, mesh_path, material, material->sheen_roughness_texture, texture_refs);

        if (texture_found)
        {
#if defined(MATERIAL_LOG)
            printf("Sheen Roughness Path: %s \n", material->textures[material, material->sheen_roughness_texture.texture_idx].c_str());
#endif
            material->material_type = MATERIAL_CLOTH;
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
        if (assimp_material->Get(AI_MATKEY_SHEEN_ROUGHNESS_FACTOR, material->sheen_roughness) == aiReturn_SUCCESS)
            material->material_type = MATERIAL_CLOTH;
    }
}

void read_clear_coat_material(std::string mesh_path, aiMaterial* assimp_material, Material* material, std::unordered_map<std::string, TextureRef>& texture_refs)
{
    // Clear Coat Texture
    {
        aiString path("");
        bool     texture_found = find_texture(assimp_material, AI_MATKEY_CLEARCOAT_TEXTURE, mesh_path, material, material->clear_coat_texture, texture_refs);

        if (texture_found)
        {
#if defined(MATERIAL_LOG)
            printf("Clear Coat Path: %s \n", material->textures[material, material->clear_coat_texture.texture_idx].c_str());
#endif
            material->material_type = MATERIAL_CLEAR_COAT;
        }
    }

    // Clear Coat Roughness Texture
    {
        aiString path("");
        bool     texture_found = find_texture(assimp_material, AI_MATKEY_CLEARCOAT_ROUGHNESS_TEXTURE, mesh_path, material, material->clear_coat_roughness_texture, texture_refs);

        if (texture_found)
        {
#if defined(MATERIAL_LOG)
            printf("Clear Coat Roughness Path: %s \n", material->textures[material, material->clear_coat_roughness_texture.texture_idx].c_str());
#endif
            material->material_type = MATERIAL_CLEAR_COAT;
        }
    }

    // Clear Coat Normal Texture
    {
        aiString path("");
        bool     texture_found = find_texture(assimp_material, AI_MATKEY_CLEARCOAT_NORMAL_TEXTURE, mesh_path, material, material->clear_coat_normal_texture, texture_refs);

        if (texture_found)
        {
#if defined(MATERIAL_LOG)
            printf("Clear Coat Normal Path: %s \n", material->textures[material, material->clear_coat_normal_texture.texture_idx].c_str());
#endif
            material->material_type = MATERIAL_CLEAR_COAT;
        }
    }

    // Clear Coat Factor
    {
        if (assimp_material->Get(AI_MATKEY_CLEARCOAT_FACTOR, material->clear_coat) == aiReturn_SUCCESS)
            material->material_type = MATERIAL_CLEAR_COAT;
    }

    // Clear Coat Roughness Factor
    {
        if (assimp_material->Get(AI_MATKEY_CLEARCOAT_ROUGHNESS_FACTOR, material->clear_coat_roughness) == aiReturn_SUCCESS)
            material->material_type = MATERIAL_CLEAR_COAT;
    }
}

void read_anisotropy_material(std::string mesh_path, aiMaterial* assimp_material, Material* material, std::unordered_map<std::string, TextureRef>& texture_refs)
{
    // TODO
}

void read_transmission_material(std::string mesh_path, aiMaterial* assimp_material, Material* material, std::unordered_map<std::string, TextureRef>& texture_refs)
{
    // Transmission Texture
    {
        aiString path("");
        bool     texture_found = find_texture(assimp_material, AI_MATKEY_TRANSMISSION_TEXTURE, mesh_path, material, material->transmission_texture, texture_refs);

        if (texture_found)
        {
#if defined(MATERIAL_LOG)
            printf("Transmission Path: %s \n", material->textures[material, material->transmission_texture.texture_idx].c_str());
#endif
            material->surface_type = SURFACE_TRANSMISSIVE;
        }
    }

    // Transmission Factor
    {
        if (assimp_material->Get(AI_MATKEY_TRANSMISSION_FACTOR, material->transmission) == aiReturn_SUCCESS)
            material->surface_type = SURFACE_TRANSMISSIVE;
    }
}

void read_volume_material(std::string mesh_path, aiMaterial* assimp_material, Material* material, std::unordered_map<std::string, TextureRef>& texture_refs)
{
    // Thickness Texture
    {
        aiString path("");
        bool     texture_found = find_texture(assimp_material, AI_MATKEY_VOLUME_THICKNESS_TEXTURE, mesh_path, material, material->thickness_texture, texture_refs);

        if (texture_found)
        {
#if defined(MATERIAL_LOG)
            printf("Thickness Path: %s \n", material->textures[material, material->thickness_texture.texture_idx].c_str());
#endif
            material->surface_type = SURFACE_TRANSMISSIVE;
        }
    }

    // Thickness Factor
    {
        if (assimp_material->Get(AI_MATKEY_VOLUME_THICKNESS_FACTOR, material->thickness_factor) == aiReturn_SUCCESS)
            material->surface_type = SURFACE_TRANSMISSIVE;
    }

    // Attenuation Distance
    {
        if (assimp_material->Get(AI_MATKEY_VOLUME_ATTENUATION_DISTANCE, material->attenuation_distance) == aiReturn_SUCCESS)
            material->surface_type = SURFACE_TRANSMISSIVE;
    }

    // Attenuation Color
    {
        aiColor3D attenuation_color;
        aiReturn  property_found = assimp_material->Get(AI_MATKEY_VOLUME_ATTENUATION_COLOR, attenuation_color);

        if (property_found == aiReturn_SUCCESS)
        {
            material->attenuation_color = glm::vec3(attenuation_color.r, attenuation_color.g, attenuation_color.b);
            material->surface_type      = SURFACE_TRANSMISSIVE;
        }
    }
}

void read_ior_material(aiMaterial* assimp_material, Material* material)
{
    assimp_material->Get(AI_MATKEY_REFRACTI, material->ior);
}

bool import_mesh(const std::string& file, MeshImportResult& import_result, MeshImportOptions options)
{
    bool        is_gltf   = false;
    std::string extension = filesystem::get_file_extention(file);

    if (extension == "gltf" || extension == "glb")
        is_gltf = true;

    std::filesystem::path absolute_file_path = std::filesystem::path(file);

    if (!absolute_file_path.is_absolute())
        absolute_file_path = std::filesystem::path(std::filesystem::current_path().string() + "/" + file);

    std::string path_to_mesh = filesystem::get_file_path(absolute_file_path.string());

    printf("Importing Mesh...\n\n");

    auto start = std::chrono::high_resolution_clock::now();

    const aiScene*   scene;
    Assimp::Importer importer;

    scene = importer.ReadFile(absolute_file_path.string().c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (scene)
    {
        import_result.name = filesystem::get_filename(file);

        import_result.name.erase(std::remove(import_result.name.begin(), import_result.name.end(), ':'), import_result.name.end());
        import_result.name.erase(std::remove(import_result.name.begin(), import_result.name.end(), '.'), import_result.name.end());

        import_result.submeshes.resize(scene->mNumMeshes);
        import_result.materials.resize(scene->mNumMaterials);

        uint32_t                                 vertex_count = 0;
        uint32_t                                 index_count  = 0;
        uint32_t                                 unnamed_mats = 1;
        std::vector<uint32_t>                    temp_indices;
        std::unordered_map<std::string, TextureRef> texture_refs;

        // Read materials.
        for (int i = 0; i < scene->mNumMaterials; i++)
        {
            import_result.materials[i] = std::unique_ptr<Material>(new Material());

            auto& material = import_result.materials[i];

            aiMaterial* assimp_material = scene->mMaterials[i];

            std::string mat_name = assimp_material->GetName().C_Str();

            mat_name.erase(std::remove(mat_name.begin(), mat_name.end(), ':'), mat_name.end());
            mat_name.erase(std::remove(mat_name.begin(), mat_name.end(), '.'), mat_name.end());

            // If material has no name, assign a name to it.
            if (mat_name.size() == 0 || mat_name == " ")
            {
                mat_name = import_result.name;
                mat_name += "_unnamed_material_";
                mat_name += std::to_string(unnamed_mats++);
            }

            material->name          = mat_name;
            material->surface_type  = SURFACE_OPAQUE;
            material->material_type = MATERIAL_STANDARD;
            material->alpha_mode    = ALPHA_MODE_OPAQUE;

            if (is_gltf)
            {
                aiString assimp_alpha_mode;

                if (assimp_material->Get(AI_MATKEY_GLTF_ALPHAMODE, assimp_alpha_mode) == aiReturn_SUCCESS)
                {
                    std::string alpha_mode = assimp_alpha_mode.C_Str();

                    if (alpha_mode == "OPAQUE")
                        material->alpha_mode = ALPHA_MODE_OPAQUE;
                    else if (alpha_mode == "BLEND")
                        material->alpha_mode = ALPHA_MODE_BLEND;
                    else if (alpha_mode == "MASK")
                        material->alpha_mode = ALPHA_MODE_MASK;
                }
            }

            assimp_material->Get(AI_MATKEY_TWOSIDED, material->is_double_sided);

            read_standard_material(path_to_mesh, assimp_material, material.get(), texture_refs, is_gltf, options);

            read_sheen_material(path_to_mesh, assimp_material, material.get(), texture_refs);

            read_clear_coat_material(path_to_mesh, assimp_material, material.get(), texture_refs);

            read_anisotropy_material(path_to_mesh, assimp_material, material.get(), texture_refs);

            read_transmission_material(path_to_mesh, assimp_material, material.get(), texture_refs);

            read_ior_material(assimp_material, material.get());

            read_volume_material(path_to_mesh, assimp_material, material.get(), texture_refs);
        }

        // Read submesh data.
        for (int i = 0; i < scene->mNumMeshes; i++)
        {
            std::string submesh_name = scene->mMeshes[i]->mName.C_Str();

            if (submesh_name.length() == 0)
                submesh_name = "submesh_" + std::to_string(i);

            strcpy(import_result.submeshes[i].name, submesh_name.c_str());
            import_result.submeshes[i].index_count  = scene->mMeshes[i]->mNumFaces * 3;
            import_result.submeshes[i].vertex_count = scene->mMeshes[i]->mNumVertices;
            import_result.submeshes[i].base_index   = index_count;
            import_result.submeshes[i].base_vertex  = vertex_count;

            vertex_count += scene->mMeshes[i]->mNumVertices;
            index_count += import_result.submeshes[i].index_count;

            import_result.submeshes[i].material_index = scene->mMeshes[i]->mMaterialIndex;
        }

        import_result.vertices.resize(vertex_count);
        import_result.indices.resize(index_count);
        temp_indices.resize(index_count);

        aiMesh* temp_mesh;
        int     idx          = 0;
        int     vertex_index = 0;

        // Read vertex data.
        for (int i = 0; i < scene->mNumMeshes; i++)
        {
            temp_mesh                              = scene->mMeshes[i];
            import_result.submeshes[i].max_extents = glm::vec3(temp_mesh->mVertices[0].x, temp_mesh->mVertices[0].y, temp_mesh->mVertices[0].z);
            import_result.submeshes[i].min_extents = glm::vec3(temp_mesh->mVertices[0].x, temp_mesh->mVertices[0].y, temp_mesh->mVertices[0].z);

            for (int k = 0; k < scene->mMeshes[i]->mNumVertices; k++)
            {
                import_result.vertices[vertex_index].position = glm::vec3(temp_mesh->mVertices[k].x, temp_mesh->mVertices[k].y, temp_mesh->mVertices[k].z);
                glm::vec3 n                                   = glm::vec3(temp_mesh->mNormals[k].x, temp_mesh->mNormals[k].y, temp_mesh->mNormals[k].z);
                import_result.vertices[vertex_index].normal   = n;

                if (temp_mesh->mTangents)
                {
                    glm::vec3 t = glm::vec3(temp_mesh->mTangents[k].x, temp_mesh->mTangents[k].y, temp_mesh->mTangents[k].z);
                    glm::vec3 b = glm::vec3(temp_mesh->mBitangents[k].x, temp_mesh->mBitangents[k].y, temp_mesh->mBitangents[k].z);

                    // @NOTE: Assuming right handed coordinate space
                    if (glm::dot(glm::cross(n, t), b) < 0.0f)
                        t *= -1.0f; // Flip tangent

                    import_result.vertices[vertex_index].tangent   = t;
                    import_result.vertices[vertex_index].bitangent = b;
                }

                if (temp_mesh->HasTextureCoords(0))
                    import_result.vertices[vertex_index].tex_coord = glm::vec2(temp_mesh->mTextureCoords[0][k].x, temp_mesh->mTextureCoords[0][k].y);

                if (import_result.vertices[vertex_index].position.x > import_result.submeshes[i].max_extents.x)
                    import_result.submeshes[i].max_extents.x = import_result.vertices[vertex_index].position.x;
                if (import_result.vertices[vertex_index].position.y > import_result.submeshes[i].max_extents.y)
                    import_result.submeshes[i].max_extents.y = import_result.vertices[vertex_index].position.y;
                if (import_result.vertices[vertex_index].position.z > import_result.submeshes[i].max_extents.z)
                    import_result.submeshes[i].max_extents.z = import_result.vertices[vertex_index].position.z;

                if (import_result.vertices[vertex_index].position.x < import_result.submeshes[i].min_extents.x)
                    import_result.submeshes[i].min_extents.x = import_result.vertices[vertex_index].position.x;
                if (import_result.vertices[vertex_index].position.y < import_result.submeshes[i].min_extents.y)
                    import_result.submeshes[i].min_extents.y = import_result.vertices[vertex_index].position.y;
                if (import_result.vertices[vertex_index].position.z < import_result.submeshes[i].min_extents.z)
                    import_result.submeshes[i].min_extents.z = import_result.vertices[vertex_index].position.z;

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
        for (int i = 0; i < import_result.submeshes.size(); i++)
        {
            SubMesh& submesh = import_result.submeshes[i];

            for (int idx = submesh.base_index; idx < (submesh.base_index + submesh.index_count); idx++)
                import_result.indices[count++] = submesh.base_vertex + temp_indices[idx];

            submesh.base_vertex = 0;
        }

        import_result.max_extents = import_result.submeshes[0].max_extents;
        import_result.min_extents = import_result.submeshes[0].min_extents;

        // Find AABB for entire import_result.
        for (int i = 0; i < import_result.submeshes.size(); i++)
        {
            if (import_result.submeshes[i].max_extents.x > import_result.max_extents.x)
                import_result.max_extents.x = import_result.submeshes[i].max_extents.x;
            if (import_result.submeshes[i].max_extents.y > import_result.max_extents.y)
                import_result.max_extents.y = import_result.submeshes[i].max_extents.y;
            if (import_result.submeshes[i].max_extents.z > import_result.max_extents.z)
                import_result.max_extents.z = import_result.submeshes[i].max_extents.z;

            if (import_result.submeshes[i].min_extents.x < import_result.min_extents.x)
                import_result.min_extents.x = import_result.submeshes[i].min_extents.x;
            if (import_result.submeshes[i].min_extents.y < import_result.min_extents.y)
                import_result.min_extents.y = import_result.submeshes[i].min_extents.y;
            if (import_result.submeshes[i].min_extents.z < import_result.min_extents.z)
                import_result.min_extents.z = import_result.submeshes[i].min_extents.z;
        }

        auto                          finish = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time   = finish - start;

        printf("Successfully imported mesh in %f seconds\n\n", time.count());

        return true;
    }

    return false;
}
} // namespace ast
