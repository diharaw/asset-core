#include <offline/mesh_exporter.h>
#include <offline/material_exporter.h>
#include <common/filesystem.h>
#include <common/header.h>
#include <iostream>
#include <fstream>

#define WRITE_AND_OFFSET(stream, dest, size, offset) stream.write((char*)dest, size); offset += size; stream.seekg(offset);

namespace ast
{
    bool export_mesh(const std::string& path, const MeshDesc& desc)
    {
        std::string material_path = path;
        material_path += "/materials";

        filesystem::create_directory(material_path);
        
        std::string texture_path = path;
        texture_path += "/textures";
        
        filesystem::create_directory(texture_path);
        
        std::string output_path = path;
        output_path += "/";
        output_path += desc.name;
        output_path += ".ast";
        
        std::fstream f(output_path, std::ios::out | std::ios::binary);
        
        if (f.is_open())
        {
            BINFileHeader fh;
            char* magic = (char*)&fh.magic;
            
            magic[0] = 'a';
            magic[1] = 's';
            magic[2] = 't';
            
            fh.version = AST_VERSION;
            fh.type = ASSET_MESH;
            
            BINMeshFileHeader header;
            
            // Copy Name
            strcpy(&header.name[0], desc.name.c_str());
            header.name[desc.name.size()] = '\0';
            
            header.index_count = desc.indices.size();
            header.vertex_count = desc.vertices.size();
            header.skeletal_vertex_count = desc.skeletal_vertices.size();
            header.material_count = desc.materials.size();
            header.mesh_count = desc.submeshes.size();
            header.max_extents = desc.max_extents;
            header.min_extents = desc.min_extents;
            
            size_t offset = 0;
            
            // Write mesh header
            WRITE_AND_OFFSET(f, (char*)&fh, sizeof(BINFileHeader), offset);
            
            // Write mesh header
            WRITE_AND_OFFSET(f, (char*)&header, sizeof(BINMeshFileHeader), offset);
     
            // Write vertices
            if (desc.vertices.size() > 0)
            {
                WRITE_AND_OFFSET(f, (char*)&desc.vertices[0], sizeof(VertexDesc) * desc.vertices.size(), offset);
            }
            
            // Write skeletal vertices
            if (desc.skeletal_vertices.size() > 0)
            {
                WRITE_AND_OFFSET(f, (char*)&desc.skeletal_vertices[0], sizeof(SkeletalVertexDesc) * desc.skeletal_vertices.size(), offset);
            }
            
            // Write indices
            if (desc.indices.size() > 0)
            {
                WRITE_AND_OFFSET(f, (char*)&desc.indices[0], sizeof(uint32_t) * desc.indices.size(), offset);
            }
            
            // Write mesh headers
            if (desc.submeshes.size() > 0)
            {
                WRITE_AND_OFFSET(f, (char*)&desc.submeshes[0], sizeof(SubMeshDesc) * desc.submeshes.size(), offset);
            }
            
            // Export materials
            std::vector<BINMeshMaterialJson> mats;
            
            for (const auto& material : desc.materials)
            {
                if (export_material(material_path, material))
                {
                    std::string mat_out_path = "materials/";
                    mat_out_path += material.name;
                    mat_out_path += ".json";
                    
                    BINMeshMaterialJson mat;
                    
                    strcpy(&mat.material[0], mat_out_path.c_str());
                    mat.material[mat_out_path.size()] = '\0';
                    
                    mats.push_back(mat);
                }
            }
            
            // Write material paths
            if (mats.size() > 0)
            {
                WRITE_AND_OFFSET(f, (char*)&mats[0], sizeof(BINMeshMaterialJson) * mats.size(), offset);
            }

            f.close();
            
            return true;
        }
        else
            std::cout << "Failed to write Mesh!" << std::endl;
        
        return false;
    }
}
