#include <offline/exporter.h>
#include <offline/filesystem.h>
#include <json.hpp>
#include <iostream>
#include <fstream>

namespace ast
{
    const std::string kTextureType[] =
    {
        "TEXTURE_ALBEDO",
        "TEXTURE_EMISSIVE",
        "TEXTURE_DISPLACEMENT",
        "TEXTURE_NORMAL",
        "TEXTURE_METALNESS",
        "TEXTURE_ROUGHNESS",
        "TEXTURE_SPECULAR",
        "TEXTURE_GLOSSINESS",
        "TEXTURE_CUSTOM"
    };
    
    const std::string kPropertyType[] =
    {
        "PROPERTY_ALBEDO",
        "PROPERTY_EMISSIVE",
        "PROPERTY_METALNESS",
        "PROPERTY_ROUGHNESS",
        "PROPERTY_SPECULAR",
        "PROPERTY_GLOSSINESS"
    };
    
    const std::string kShadingModel[] =
    {
        "SHADING_MODEL_STANDARD",
        "SHADING_MODEL_CLOTH",
        "SHADING_MODEL_SUBSURFACE"
    };
    
    const std::string kLightingModel[] =
    {
        "LIGHTING_MODEL_LIT",
        "LIGHTING_MODEL_UNLIT"
    };
    
    const std::string kDisplacementType[] =
    {
        "DISPLACEMENT_NONE",
        "DISPLACEMENT_PARALLAX_OCCLUSION",
        "DISPLACEMENT_TESSELLATION"
    };
    
    const std::string kBlendMode[] =
    {
        "BLEND_MODE_OPAQUE",
        "BLEND_MODE_ADDITIVE",
        "BLEND_MODE_MASKED",
        "BLEND_MODE_TRANSLUCENT"
    };
    
    struct BINMeshFileHeader
    {
        uint32_t   mesh_count;
        uint32_t   material_count;
        uint32_t   vertex_count;
        uint32_t   skeletal_vertex_count;
        uint32_t   index_count;
        glm::vec3  max_extents;
        glm::vec3  min_extents;
        char       name[50];
    };
    
    struct BINMeshMaterialJson
    {
        char material[50];
    };
    
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
                float_array.push_back(property_desc.vec4_value.x);
                float_array.push_back(property_desc.vec4_value.y);
                float_array.push_back(property_desc.vec4_value.z);
                float_array.push_back(property_desc.vec4_value.w);
                
                property["value"] = float_array;
            }
            else if (property_desc.type == PROPERTY_EMISSIVE ||
                     property_desc.type == PROPERTY_SPECULAR)
            {
                auto float_array = doc.array();
                float_array.push_back(property_desc.vec3_value.x);
                float_array.push_back(property_desc.vec3_value.y);
                float_array.push_back(property_desc.vec3_value.z);
                
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
        output_path += ".json";
        
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
            f.write((char*)&header, sizeof(BINMeshFileHeader));
            offset += sizeof(BINMeshFileHeader);
            f.seekg(offset);
            
            // Write vertices
            if (desc.vertices.size() > 0)
            {
                f.write((char*)&desc.vertices[0], sizeof(VertexDesc) * desc.vertices.size());
                offset += sizeof(VertexDesc) * desc.vertices.size();
                f.seekg(offset);
            }
            
            // Write skeletal vertices
            if (desc.skeletal_vertices.size() > 0)
            {
                f.write((char*)&desc.skeletal_vertices[0], sizeof(SkeletalVertexDesc) * desc.skeletal_vertices.size());
                offset += sizeof(SkeletalVertexDesc) * desc.skeletal_vertices.size();
                f.seekg(offset);
            }
            
            // Write indices
            if (desc.indices.size() > 0)
            {
                f.write((char*)&desc.indices[0], sizeof(uint32_t) * desc.indices.size());
                offset += sizeof(uint32_t) * desc.indices.size();
                f.seekg(offset);
            }
            
            // Write mesh headers
            if (desc.submeshes.size() > 0)
            {
                f.write((char*)&desc.submeshes[0], sizeof(SubMeshDesc) * desc.submeshes.size());
                offset += sizeof(SubMeshDesc) * desc.submeshes.size();
                f.seekg(offset);
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
                f.write((char*)&mats[0], sizeof(BINMeshMaterialJson) * mats.size());

            f.close();
            
            return true;
        }
        else
            std::cout << "Failed to write Mesh!" << std::endl;
        
        return false;
    }
}
