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

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace ast
{
    const nvtt::Format kCompression[] =
    {
        nvtt::Format_RGB,
        nvtt::Format_BC1,
        nvtt::Format_BC1a,
        nvtt::Format_BC2,
        nvtt::Format_BC3,
        nvtt::Format_BC3n,
        nvtt::Format_BC4,
        nvtt::Format_BC5,
        nvtt::Format_BC6,
        nvtt::Format_BC7
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
    
    bool does_material_exist(std::vector<uint32_t> &materials, uint32_t &current_material)
    {
        for (auto it : materials)
        {
            if (it == current_material)
                return true;
        }
        
        return false;
    }
    
    bool import_mesh(const MeshImportDesc& desc, MeshDesc& mesh)
    {
        const aiScene* scene;
        Assimp::Importer importer;
        
        if (desc.source == IMPORT_SOURCE_FILE)
            scene = importer.ReadFile(desc.file.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        else if (desc.source == IMPORT_SOURCE_MEMORY)
            scene = importer.ReadFileFromMemory(desc.data, desc.size, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        else
            return false;
        
        if (scene)
        {
            mesh.name = filesystem::get_filename(desc.file);
            mesh.submeshes.resize(scene->mNumMeshes);
            
            uint32_t vertex_count = 0;
            uint32_t index_count = 0;
            aiMaterial* temp_material;
            std::vector<uint32_t> processed_mat_id;
            std::unordered_map<uint32_t, uint32_t> mat_id_mapping;
            uint32_t unnamed_mats = 1;
            
            for (int i = 0; i < scene->mNumMeshes; i++)
            {
                mesh.submeshes[i].index_count = scene->mMeshes[i]->mNumFaces * 3;
                mesh.submeshes[i].base_index = index_count;
                mesh.submeshes[i].base_vertex = vertex_count;
                
                vertex_count += scene->mMeshes[i]->mNumVertices;
                index_count += mesh.submeshes[i].index_count;
                
                if (!does_material_exist(processed_mat_id, scene->mMeshes[i]->mMaterialIndex))
                {
                    MaterialDesc mat;
                    
                    mat.name = scene->mMeshes[i]->mName.C_Str();
                    
                    if (mat.name.empty())
                    {
                        mat.name = mesh.name;
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
                        
                        MaterialTextureDesc mat_desc;
                        
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
                        
                        MaterialTextureDesc mat_desc;
                        
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
                        
                        MaterialTextureDesc mat_desc;
                        
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
                        
                        MaterialTextureDesc mat_desc;
                        
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
                        
                        MaterialTextureDesc mat_desc;
                        
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
                        
                        MaterialTextureDesc mat_desc;
                        
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
                        
                        MaterialTextureDesc mat_desc;
                        
                        mat_desc.srgb = true;
                        mat_desc.type = TEXTURE_DISPLACEMENT;
                        mat_desc.path = height_path;
                        
                        mat.textures.push_back(mat_desc);
                        
                        mat.displacement_type = DISPLACEMENT_PARALLAX_OCCLUSION;
                    }
                    
                    mat_id_mapping[scene->mMeshes[i]->mMaterialIndex] = mesh.materials.size();
                    
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
    
    struct TextureOutputHandler : public nvtt::OutputHandler
    {
        long offset;
        int mip_levels = 0;
        int mip_height;
        int compression_type;
        
        virtual void beginImage(int size, int width, int height, int depth, int face, int miplevel) override
        {
//            std::cout << "Beginning Image: Size = " << size << ", Mip = " << miplevel << ", Width = " << width << ", Height = " << height << std::endl;
//
//            MipSliceHeader mip0Header;
//
//            mip0Header.width = width;
//            mip0Header.height = height;
//            mip0Header.size = size;
//
//            stream->write((char*)&mip0Header, sizeof(MipSliceHeader));
//            offset += sizeof(MipSliceHeader);
//            stream->seekp(offset);
//
            mip_height = height;
            mip_levels++;
        }
        
        // Output data. Compressed data is output as soon as it's generated to minimize memory allocations.
        virtual bool writeData(const void * data, int size) override
        {
            offset += size;
            
            return true;
        }
        
        // Indicate the end of the compressed image. (New in NVTT 2.1)
        virtual void endImage() override
        {
        }
    };
    
    bool compress_array_item(TextureArrayItem& item, PixelType pixel_type, int pixel_size_bits, int channel_count, CompressionType compression, bool generate_mipmaps)
    {
        nvtt::CompressionOptions compression_options;
        nvtt::InputOptions input_options;
        nvtt::OutputOptions output_options;
        nvtt::Compressor compressor;
        TextureOutputHandler handler;
        
        compression_options.setFormat(kCompression[compression]);
        
        if (compression == COMPRESSION_NONE)
        {
            uint32_t pixel_size = pixel_size_bits;
            
            if (pixel_type == PIXEL_TYPE_UNORM8)
                compression_options.setPixelType(nvtt::PixelType_UnsignedNorm);
            else if (pixel_type == PIXEL_TYPE_FLOAT16 || pixel_type == PIXEL_TYPE_FLOAT32)
                compression_options.setPixelType(nvtt::PixelType_Float);
            
            if (channel_count == 4)
                compression_options.setPixelFormat(pixel_size, pixel_size, pixel_size, pixel_size);
            else if (channel_count == 3)
                compression_options.setPixelFormat(pixel_size, pixel_size, pixel_size, 0);
            else if (channel_count == 2)
                compression_options.setPixelFormat(pixel_size, pixel_size, 0, 0);
            else if (channel_count == 1)
                compression_options.setPixelFormat(pixel_size, 0, 0, 0);
        }
        
        if (pixel_type == PIXEL_TYPE_DEFAULT)
        {
            if (pixel_type == PIXEL_TYPE_UNORM8)
                input_options.setFormat(nvtt::InputFormat_BGRA_8UB);
            else if (pixel_type == PIXEL_TYPE_FLOAT16)
                input_options.setFormat(nvtt::InputFormat_RGBA_16F);
            else if (pixel_type == PIXEL_TYPE_FLOAT32)
                input_options.setFormat(nvtt::InputFormat_RGBA_32F);
        }
        else
        {
            if (pixel_type == PIXEL_TYPE_UNORM8)
                input_options.setFormat(nvtt::InputFormat_BGRA_8UB);
            else if (pixel_type == PIXEL_TYPE_FLOAT16)
                input_options.setFormat(nvtt::InputFormat_RGBA_16F);
            else if (pixel_type == PIXEL_TYPE_FLOAT32)
                input_options.setFormat(nvtt::InputFormat_RGBA_32F);
        }
        
        input_options.setNormalMap(false);
        input_options.setConvertToNormalMap(false);
        input_options.setNormalizeMipmaps(false);
        
        output_options.setOutputHeader(false);
        output_options.setOutputHandler(&handler);
        
        return false;
    }
    
    bool import_texture_2d(const Texture2DImportDesc& desc, TextureDesc& texture)
    {
        int x, y, n, bpp;
        void* data = nullptr;
        
        if (desc.source == IMPORT_SOURCE_FILE)
        {
            auto ext = filesystem::get_file_extention(desc.file);
            
            if (ext == "dds")
            {
                // Use NVTT for DDS files
                nv::DirectDrawSurface dds;
                
                if (dds.load(desc.file.c_str()))
                {
                    if (desc.pixel_type == PIXEL_TYPE_UNORM8)
                        bpp = 1;
                    else if (desc.pixel_type == PIXEL_TYPE_FLOAT16)
                        bpp = 2;
                    else if (desc.pixel_type == PIXEL_TYPE_FLOAT32)
                        bpp = 4;
                    else
                        return false;
                    
                    nv::Image img;
                    dds.mipmap(&img, 0, 0);
                
                    x = img.width();
                    y = img.height();
                    
                    auto fmt = img.format();
                
                    if (fmt == nv::Image::Format_RGB)
                        n = 3;
                    else if (fmt == nv::Image::Format_ARGB)
                        n = 4;
                    else
                        return false;
                    
                    size_t size = x * y * n * bpp;
                    data = malloc(size);
                    memcpy(data, img.pixels(), size);
                }
            }
            else
            {
                // Use STB for everything else
                if (desc.pixel_type == PIXEL_TYPE_UNORM8)
                {
                    data = stbi_load(desc.file.c_str(), &x, &y, &n, 0);
                    bpp = 1;
                }
                else if (desc.pixel_type == PIXEL_TYPE_FLOAT16)
                {
                    data = stbi_load_16(desc.file.c_str(), &x, &y, &n, 0);
                    bpp = 2;
                }
                else if (desc.pixel_type == PIXEL_TYPE_FLOAT32)
                {
                    data = stbi_loadf(desc.file.c_str(), &x, &y, &n, 0);
                    bpp = 4;
                }
                else
                    return false;
            }
        }
        else if (desc.source == IMPORT_SOURCE_MEMORY)
        {
            data = malloc(desc.size);
            memcpy(data, desc.data, desc.size);
            
            x = desc.width;
            y = desc.height;
            n = desc.channel_count;
            
            if (desc.pixel_type == PIXEL_TYPE_UNORM8)
                bpp = 1;
            else if (desc.pixel_type == PIXEL_TYPE_FLOAT16)
                bpp = 2;
            else if (desc.pixel_type == PIXEL_TYPE_FLOAT32)
                bpp = 4;
            else
                return false;
        }
        
        if (data)
        {
            texture.type = TEXTURE_2D;
            texture.name = filesystem::get_filename(desc.file);
            texture.channel_size = bpp;
            texture.mip_slice_count = 1;
            texture.channel_count = n;
            texture.compression = desc.options.compression;
            
            size_t num_pixels = x * y * n;
            size_t size_in_bytes = num_pixels * bpp;
            
            texture.array_items.resize(1);
            texture.array_items[0].mip_levels.resize(1);
            texture.array_items[0].mip_levels[0].width = x;
            texture.array_items[0].mip_levels[0].height = y;
            
            texture.array_items[0].mip_levels[0].pixels.copy_data(size_in_bytes, data);
            free(data);
            
            return true;
        }

        return false;
    }
    
    bool import_texture_cube(const TextureCubeImportDesc& desc, TextureDesc& texture)
    {
        if (desc.source == IMPORT_SOURCE_FILE)
        {
            
        }
        else if (desc.source == IMPORT_SOURCE_MEMORY)
        {
            
        }
        
        return false;
    }
}
