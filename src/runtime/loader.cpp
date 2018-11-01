#include <runtime/loader.h>
#include <common/header.h>
#include <fstream>
#include <json.hpp>

#define READ_AND_OFFSET(stream, dest, size, offset) stream.read((char*)dest, size); offset += size; stream.seekg(offset);

namespace ast
{
    bool load_image(const std::string& path, Image& image)
    {
        std::fstream f(path, std::ios::in | std::ios::binary);
        
        if (!f.is_open())
            return false;
        
        BINFileHeader file_header;
        BINImageHeader image_header;
        uint16_t len = 0;
        
        size_t offset = 0;
        
        READ_AND_OFFSET(f, &file_header, sizeof(BINFileHeader), offset);
        
        READ_AND_OFFSET(f, &len, sizeof(uint16_t), offset);
        image.name.resize(len);
        
        READ_AND_OFFSET(f, image.name.c_str(), len, offset);
        
        READ_AND_OFFSET(f, &image_header, sizeof(BINImageHeader), offset);
        
        image.array_slices = image_header.num_array_slices;
        image.mip_slices = image_header.num_mip_slices;
        image.components = image_header.num_channels;
        image.type = image_header.channel_size;
        image.compression = image_header.compression;
        
        if (image_header.num_array_slices == 0)
            return false;
        
        for (int i = 0; i < image_header.num_array_slices; i++)
        {
            for (int j = 0; j < image_header.num_array_slices; j++)
            {
                BINMipSliceHeader mip_header;
                READ_AND_OFFSET(f, &mip_header, sizeof(BINMipSliceHeader), offset);
                
                image.data[i][j].width = mip_header.width;
                image.data[i][j].height = mip_header.height;
                image.data[i][j].data = malloc(mip_header.size);
                
                READ_AND_OFFSET(f, image.data[i][j].data, mip_header.size, offset);
            }
        }
        
        return true;
    }
    
    bool load_mesh(const std::string& path, Mesh& mesh)
    {
        std::fstream f(path, std::ios::in | std::ios::binary);
        
        if (!f.is_open())
            return false;
        
        BINFileHeader file_header;
        BINMeshFileHeader mesh_header;
        
        size_t offset = 0;
        
        READ_AND_OFFSET(f, &file_header, sizeof(BINFileHeader), offset);
        
        READ_AND_OFFSET(f, (char*)&mesh_header, sizeof(BINMeshFileHeader), offset);
        
        if (mesh_header.vertex_count > 0)
        {
            mesh.vertices.resize(mesh_header.vertex_count);
            READ_AND_OFFSET(f, (char*)&mesh.vertices[0], sizeof(Vertex) * mesh.vertices.size(), offset);
        }
        
        if (mesh_header.skeletal_vertex_count > 0)
        {
            mesh.skeletal_vertices.resize(mesh_header.skeletal_vertex_count);
            READ_AND_OFFSET(f, (char*)&mesh.skeletal_vertices[0], sizeof(SkeletalVertex) * mesh.skeletal_vertices.size(), offset);
        }
        
        if (mesh_header.index_count > 0)
        {
            mesh.indices.resize(mesh_header.index_count);
            READ_AND_OFFSET(f, (char*)&mesh.indices[0], sizeof(uint32_t) * mesh.indices.size(), offset);
        }
        
        if (mesh_header.mesh_count > 0)
        {
            mesh.submeshes.resize(mesh_header.mesh_count);
            READ_AND_OFFSET(f, (char*)&mesh.submeshes[0], sizeof(SubMesh) * mesh.submeshes.size(), offset);
        }
        
        std::vector<BINMeshMaterialJson> bin_materials;
        
        if (mesh_header.material_count > 0)
        {
            bin_materials.resize(mesh_header.material_count);
            mesh.materials.resize(mesh_header.material_count);
            READ_AND_OFFSET(f, (char*)&bin_materials[0], sizeof(BINMeshMaterialJson) * bin_materials.size(), offset);
        }
        
        for (int i = 0; i < mesh_header.material_count; i++)
        {
            if (!load_material(bin_materials[i].material, mesh.materials[i]))
            {
                std::cout << "Failed to load material: " << bin_materials[i].material << std::endl;
                return false;
            }
        }
        
        return true;
    }
    
    bool load_material(const std::string& path, Material& material)
    {
        std::ifstream i(path);
        
        nlohmann::json j;
        i >> j;

        std::string blend_mode;
        std::string displacement_type;
        std::string lighting_model;
        std::string shading_model;
        
        if (j.find("name") != j.end())
            material.name = j["name"];
        else
            material.name = "untitled";
        
        if (j.find("blend_mode") != j.end())
        {
            blend_mode = j["blend_mode"];
            
            for (int i = 0; i < 4; i++)
            {
                if (kBlendMode[i] == blend_mode)
                {
                    material.blend_mode = i;
                    break;
                }
            }
        }
        else
            material.blend_mode = BLEND_MODE_OPAQUE;
        
        if (j.find("displacement_type") != j.end())
        {
            displacement_type = j["displacement_type"];
            
            for (int i = 0; i < 3; i++)
            {
                if (kDisplacementType[i] == displacement_type)
                {
                    material.displacement_type = i;
                    break;
                }
            }
        }
        else
            material.displacement_type = DISPLACEMENT_NONE;
        
        if (j.find("lighting_model") != j.end())
        {
            lighting_model = j["lighting_model"];
            
            for (int i = 0; i < 2; i++)
            {
                if (kLightingModel[i] == lighting_model)
                {
                    material.lighting_model = i;
                    break;
                }
            }
        }
        else
            material.lighting_model = LIGHTING_MODEL_LIT;
        
        if (j.find("shading_model") != j.end())
        {
            shading_model = j["shading_model"];
            
            for (int i = 0; i < 3; i++)
            {
                if (kShadingModel[i] == shading_model)
                {
                    material.shading_model = i;
                    break;
                }
            }
        }
        else
            material.shading_model = SHADING_MODEL_STANDARD;
        
        if (j.find("double_sided") != j.end())
            material.double_sided = j["double_sided"];
        else
            material.double_sided = false;
        
        if (j.find("metallic_workflow") != j.end())
            material.metallic_workflow = j["metallic_workflow"];
        else
            material.metallic_workflow = true;
        
        return true;
    }
}
