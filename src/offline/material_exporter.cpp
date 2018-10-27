#include <offline/material_exporter.h>
#include <common/filesystem.h>
#include <json.hpp>
#include <iostream>
#include <fstream>

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
}
