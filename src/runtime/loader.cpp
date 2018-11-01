#include <runtime/loader.h>
#include <common/header.h>
#include <fstream>

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
        return true;
    }
}
