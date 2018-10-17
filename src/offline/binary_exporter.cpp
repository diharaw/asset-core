#include <binary_exporter.h>
#include <assimp_importer.h>
#include <filesystem.h>
#include <timer.h>
#include <iostream>

namespace binary_exporter
{
    void export_mesh(AssimpImportData* data, std::string output, Options options)
    {
        std::string meshOutputName = filesystem::get_filename(data->filename);
        
        auto mats = new TSM_Material_Json[data->header.material_count];
        
        for (int i = 0; i < data->header.material_count; i++)
        {
            Json doc;
            
            // Diffuse
            String map = std::string(data->materials[i].albedo);
            
            if (map != "")
            {
                String name = filesystem::get_file_name_and_extention(map);
                
                String path_in_mat = "texture/";
                path_in_mat += name;
                
                doc["diffuse_map"] = path_in_mat;
                
                String sourcePath = data->mesh_path;
                sourcePath += map;

                std::string dest_path = output;
                dest_path += "/";
                dest_path += path_in_mat;
                
                filesystem::copy_file(sourcePath, dest_path);
            }
            else
            {
                Json diffuse_color;
                
                diffuse_color["r"] = 0.0f;
                diffuse_color["g"] = 0.0f;
                diffuse_color["b"] = 0.0f;
                diffuse_color["a"] = 1.0f;
                
                doc["diffuse_value"] = diffuse_color;
            }
            
            // Normal
            map = std::string(data->materials[i].normal);
            
            if (map != "")
            {
                String name = filesystem::get_file_name_and_extention(map);
                
                String path_in_mat = "texture/";
                path_in_mat += name;
                
                doc["normal_map"] = path_in_mat;
                
                String sourcePath = data->mesh_path;
                sourcePath += map;
                
                std::string dest_path = output;
                dest_path += "/";
                dest_path += path_in_mat;
                
                filesystem::copy_file(sourcePath, dest_path);
            }
            
            // Metalness
            map = std::string(data->materials[i].metalness);
            
            if (data->materials[i].has_metalness)
            {
                String name = filesystem::get_file_name_and_extention(map);
                
                String path_in_mat = "texture/";
                path_in_mat += name;
                
                doc["metalness_map"] = path_in_mat;
                
                String sourcePath = data->mesh_path;
                sourcePath += map;
                
                std::string dest_path = output;
                dest_path += "/";
                dest_path += path_in_mat;
                
                filesystem::copy_file(sourcePath, dest_path);
            }
            else
            {
                // Metalness Value
                doc["metalness_value"] = 0.5f;
            }
            
            // Roughness
            map = std::string(data->materials[i].roughness);
            
            if (data->materials[i].has_roughness)
            {
                String name = filesystem::get_file_name_and_extention(map);
              
                String path_in_mat = "texture/";
                path_in_mat += name;
                
                doc["roughness_map"] = path_in_mat;
                
                String sourcePath = data->mesh_path;
                sourcePath += map;
                
                std::string dest_path = output;
                dest_path += "/";
                dest_path += path_in_mat;
                
                filesystem::copy_file(sourcePath, dest_path);
            }
            else
            {
                // Roughness Value
                doc["roughness_value"] = 0.5f;
            }
            
            // Backface Cull
            doc["backface_cull"] = true;
            
            std::string matPath =  output;
            matPath += "/material/mat_";
            
            matPath += data->materials[i].mesh_name;
            matPath += ".json";
            
            String output_str = doc.dump(4);
            
            if (filesystem::write_begin(matPath))
            {
                filesystem::write((void*)output_str.c_str(), output_str.size(), 1, 0);
                filesystem::write_end();
            }
            
            String formattedString = "material/mat_";
            formattedString += data->materials[i].mesh_name;
            formattedString += ".json\0";
            strncpy(mats[i].material, formattedString.c_str(),50);
        }
        
        for (int i = 0; i < data->header.mesh_count; i++)
        {
			uint32_t idx = data->meshes[i].material_index;
            
            if (options.verbose)
                std::cout << "Mat Idx : " << std::to_string(idx) << std::endl;
        }
        
        std::string meshPath = output;
        meshPath += "/mesh/";
        meshPath += meshOutputName;
        meshPath += ".";
        meshPath += ASSET_EXTENSION;
        
        if (filesystem::write_begin(meshPath))
        {
            long Offset = 0;
            // Write Header
            filesystem::write(&data->header, sizeof(TSM_FileHeader), 1, Offset);
            Offset += sizeof(TSM_FileHeader);
            // Write Vertices
            filesystem::write(data->vertices, sizeof(TSM_Vertex), data->header.vertex_count, Offset);
            Offset += sizeof(TSM_Vertex) * data->header.vertex_count;
            // Write Indices
            filesystem::write(data->indices, sizeof(uint32_t), data->header.index_count, Offset);
            Offset += sizeof(uint32_t) * data->header.index_count;
            // Write Mesh Headers
            filesystem::write(data->meshes, sizeof(TSM_MeshHeader), data->header.mesh_count, Offset);
            Offset += sizeof(TSM_MeshHeader) * data->header.mesh_count;
            // Write Materials
            filesystem::write(mats, sizeof(TSM_Material_Json), data->header.material_count, Offset);
            
            filesystem::write_end();
        }
        
        T_SAFE_DELETE_ARRAY(mats);
        T_SAFE_DELETE_ARRAY(data->vertices);
        T_SAFE_DELETE_ARRAY(data->indices);
        T_SAFE_DELETE_ARRAY(data->materials);
        T_SAFE_DELETE_ARRAY(data->meshes);
        T_SAFE_DELETE(data);
    }
    
    void export_mesh(std::string file, std::string output, Options options)
    {
        Timer timer;
        timer.start();
        
        filesystem::create_directory(output);
        filesystem::create_directory(output + "/mesh");
        filesystem::create_directory(output + "/material");
        filesystem::create_directory(output + "/texture");
        
        AssimpImportData* data = assimp_importer::import_mesh(file, options);
        export_mesh(data, output, options);
        
        timer.stop();
        
        std::cout << "Finished exporting mesh in " << timer.elapsed_time_sec() << " second(s)." << std::endl;
    }
}
