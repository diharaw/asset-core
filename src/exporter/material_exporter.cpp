#include <exporter/material_exporter.h>
#include <exporter/image_exporter.h>
#include <common/filesystem.h>
#include <json.hpp>
#include <iostream>
#include <fstream>

namespace ast
{
void export_texture(const std::string& src_path, const std::string& dst_path, bool normal_map, bool use_compression, bool normal_map_flip_green)
{
    Image img;

    if (import_image(img, src_path))
    {
        ImageExportOptions options;

        options.output_mips = -1;
        options.normal_map  = normal_map;
        options.path        = dst_path;
        options.compression = COMPRESSION_NONE;
        options.flip_green  = normal_map_flip_green;

        if (use_compression)
        {
            if (img.components == 1)
                options.compression = COMPRESSION_BC4;
            else if (img.components == 3)
                options.compression = COMPRESSION_BC1;
            else if (img.components == 4)
                options.compression = COMPRESSION_BC3;
        }

        export_image(img, options);

        img.deallocate();
    }
}

bool export_material(const Material& desc, const MaterialExportOptions& options)
{
    nlohmann::json doc;

    doc["name"]              = desc.name;
    doc["double_sided"]      = desc.double_sided;
    doc["alpha_mask"]        = desc.alpha_mask;
    doc["orca"]              = desc.orca;
    doc["metallic_workflow"] = desc.metallic_workflow;
    doc["material_type"]     = kMaterialType[desc.material_type];
    doc["shading_model"]     = kShadingModel[desc.shading_model];

    auto texture_array = doc.array();

    for (auto& texture_desc : desc.textures)
    {
        nlohmann::json texture;

        std::string path;

        if (options.relative_texture_path != "")
            path = options.relative_texture_path;
        else
        {
            path += filesystem::get_file_path(texture_desc.path);

            if (path.size() > 0)
                path += "/";

            path += "textures";
        }

        path += "/";
        path += filesystem::get_filename(texture_desc.path);
        path += ".ast";

        std::string src_path = options.texture_source_path;
        src_path += "/";
        src_path += texture_desc.path;

        std::string dst_path = options.dst_texture_path;

        std::string full_dst_path = dst_path;
        full_dst_path += "/";
        full_dst_path += filesystem::get_filename(texture_desc.path);
        full_dst_path += ".ast";

        bool exists = filesystem::does_file_exist(full_dst_path);

        if (options.texture_source_path != "" && options.dst_texture_path != "" && !exists)
            export_texture(src_path, dst_path, texture_desc.type == TEXTURE_NORMAL ? true : false, options.use_compression, texture_desc.type == TEXTURE_NORMAL ? options.normal_map_flip_green : false);

        texture["path"] = path;
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
        else if (property_desc.type == PROPERTY_EMISSIVE || (desc.metallic_workflow && property_desc.type == PROPERTY_METALNESS_SPECULAR))
        {
            auto float_array = doc.array();
            float_array.push_back(property_desc.vec3_value[0]);
            float_array.push_back(property_desc.vec3_value[1]);
            float_array.push_back(property_desc.vec3_value[2]);

            property["value"] = float_array;
        }
        else if ((!desc.metallic_workflow && property_desc.type == PROPERTY_METALNESS_SPECULAR) || property_desc.type == PROPERTY_ROUGHNESS_GLOSSINESS)
            property["value"] = property_desc.float_value;

        property_array.push_back(property);
    }

    doc["properties"] = property_array;

    std::string output_path = options.path;
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
} // namespace ast
