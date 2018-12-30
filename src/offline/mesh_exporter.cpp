#include <offline/mesh_exporter.h>
#include <offline/material_exporter.h>
#include <common/filesystem.h>
#include <common/header.h>
#include <iostream>
#include <fstream>
#include <chrono>

#define WRITE_AND_OFFSET(stream, dest, size, offset) stream.write((char*)dest, size); offset += size; stream.seekg(offset);

namespace ast
{
    bool export_mesh(const Mesh& desc, const MeshExportOption& options)
    {
		auto start = std::chrono::high_resolution_clock::now();

        std::string material_path = options.path;
        
        if (options.relative_material_path != "")
        {
            material_path += "/";
            material_path += options.relative_material_path;
        }
        else
            material_path += "/materials";
        
        filesystem::create_directory(material_path);
        
        std::string texture_path = options.path;
        
        if (options.relative_texture_path != "")
        {
            texture_path += "/";
            texture_path += options.relative_texture_path;
        }
        else
            texture_path += "/textures";
        
        filesystem::create_directory(texture_path);
        
        std::string output_path = options.path;
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
                WRITE_AND_OFFSET(f, (char*)&desc.vertices[0], sizeof(Vertex) * desc.vertices.size(), offset);
            }
            
            // Write skeletal vertices
            if (desc.skeletal_vertices.size() > 0)
            {
                WRITE_AND_OFFSET(f, (char*)&desc.skeletal_vertices[0], sizeof(SkeletalVertex) * desc.skeletal_vertices.size(), offset);
            }
            
            // Write indices
            if (desc.indices.size() > 0)
            {
                WRITE_AND_OFFSET(f, (char*)&desc.indices[0], sizeof(uint32_t) * desc.indices.size(), offset);
            }
            
            // Write mesh headers
            if (desc.submeshes.size() > 0)
            {
                WRITE_AND_OFFSET(f, (char*)&desc.submeshes[0], sizeof(SubMesh) * desc.submeshes.size(), offset);
            }
            
            // Export materials
            std::vector<BINMeshMaterialJson> mats;
            
            for (const auto& material : desc.materials)
            {
                MaterialExportOptions mat_exp_options;
                
                mat_exp_options.path = material_path;
                mat_exp_options.relative_texture_path = options.relative_texture_path;
                mat_exp_options.texture_source_path = options.texture_source_path;
                mat_exp_options.dst_texture_path = texture_path;
                mat_exp_options.use_compression = options.use_compression;
                
                if (export_material(material, mat_exp_options))
                {
                    std::string mat_out_path = "materials/";
                    
                    if (options.relative_material_path != "")
                    {
                        mat_out_path = options.relative_material_path;
                        mat_out_path += "/";
                    }
                    
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

			auto finish = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> time = finish - start;

			printf("Successfully exported mesh in %f seconds\n\n", time.count());
            
            return true;
        }
        else
            std::cout << "Failed to write Mesh!" << std::endl;
        
        return false;
    }
}
