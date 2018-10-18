#include <offline/exporter.h>
#include <offline/filesystem.h>
#include <json.hpp>
#include <iostream>

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
        
        if (filesystem::write_begin(output_path))
        {
            filesystem::write((void*)output_str.c_str(), output_str.size(), 1, 0);
            filesystem::write_end();
            
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
        
        return true;
    }
}
