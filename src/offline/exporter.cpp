#include <offline/exporter.h>
#include <common/filesystem.h>
#include <json.hpp>
#include <iostream>
#include <fstream>

#define WRITE_AND_OFFSET(stream, dest, size, offset) stream.write((char*)dest, size); offset += size; stream.seekg(offset);

namespace ast
{
    bool export_material(const std::string& path, const MaterialDesc& desc)
    {
        nlohmann::json doc;
        
        doc["name"] = desc.name;
        doc["metallic_workflow"] = desc.metallic_workflow;
        doc["double_sided"] = desc.double_sided;
        doc["vertex_shader_func"] = doc.array();
        doc["fragment_shader_func"] = doc.array();
        doc["blend_mode"] = kBlendMode[desc.blend_mode];
        doc["displacement_type"] = kDisplacementType[desc.displacement_type];
        doc["shading_model"] = kShadingModel[desc.shading_model];
        doc["lighting_model"] = kLightingModel[desc.lighting_model];
        
        auto texture_array = doc.array();
        
        for (auto& texture_desc : desc.textures)
        {
            nlohmann::json texture;
            
            texture["path"] = texture_desc.path;
            texture["srgb"] = texture_desc.srgb;
            texture["type"] = kTextureType[texture_desc.type];
            
            texture_array.push_back(texture);
        }
        
        doc["textures"] = texture_array;
        
        auto property_array = doc.array();
        
        for (auto& property_desc : desc.properties)
        {
            nlohmann::json property;
            
            property["type"] = kPropertyType[property_desc.type];
            
            if (property_desc.type == PROPERTY_ALBEDO)
            {
                auto float_array = doc.array();
                float_array.push_back(property_desc.vec4_value[0]);
                float_array.push_back(property_desc.vec4_value[1]);
                float_array.push_back(property_desc.vec4_value[2]);
                float_array.push_back(property_desc.vec4_value[3]);
                
                property["value"] = float_array;
            }
            else if (property_desc.type == PROPERTY_EMISSIVE ||
                     property_desc.type == PROPERTY_SPECULAR)
            {
                auto float_array = doc.array();
                float_array.push_back(property_desc.vec3_value[0]);
                float_array.push_back(property_desc.vec3_value[1]);
                float_array.push_back(property_desc.vec3_value[2]);
                
                property["value"] = float_array;
            }
            else if (property_desc.type == PROPERTY_METALNESS ||
                     property_desc.type == PROPERTY_ROUGHNESS ||
                     property_desc.type == PROPERTY_GLOSSINESS)
                property["value"] = property_desc.float_value;
            
            property_array.push_back(property);
        }
        
        doc["properties"] = property_array;
        
        std::string output_path = path;
        output_path += "/";
        output_path += desc.name;
        output_path += ".json";
        
        std::string output_str = doc.dump(4);
        
        std::fstream f(output_path, std::ios::out);
        
        if (f.is_open())
        {
            f.write(output_str.c_str(), output_str.size());
            f.close();
        
            return true;
        }
        else
            std::cout << "Failed to write Material JSON!" << std::endl;
        
        return false;
    }
    
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
    
    bool export_texture(const std::string& path, const TextureDesc& desc)
    {
        std::string output_path = path;
        output_path += "/";
        output_path += desc.name;
        output_path += ".ast";
        
        std::fstream f(output_path, std::ios::out | std::ios::binary);
        
        if (f.is_open())
        {
            BINImageHeader header;
            
            // Copy Name
            strcpy(&header.name[0], desc.name.c_str());
            header.name[desc.name.size()] = '\0';
            header.type = desc.type;
            header.compression = desc.compression;
            header.channel_size = desc.channel_size;
            header.channel_count = desc.channel_count;
            header.array_slice_count = desc.array_items.size();
            header.mip_slice_count = desc.mip_slice_count;
            
            size_t offset = 0;
            
            // Write header
            WRITE_AND_OFFSET(f, &header, sizeof(BINImageHeader), offset);
            
            for (const auto& array_item : desc.array_items)
            {
                for (const auto& mip_level : array_item.mip_levels)
                {
                    BINMipSliceHeader mip_header;
                    
                    mip_header.height = mip_level.height;
                    mip_header.width = mip_level.width;
                    mip_header.size = header.channel_count * header.channel_size * mip_header.height * mip_header.width;
                    
                    // Write mip header
                    WRITE_AND_OFFSET(f, &mip_header, sizeof(BINMipSliceHeader), offset);
                    
                    // Write pixels
                    WRITE_AND_OFFSET(f, mip_level.pixels.data, mip_header.size, offset);
                }
            }
            
            f.close();
            
            return true;
        }
        else
            std::cout << "Failed to write Image!" << std::endl;
        
        return false;
    }
}
