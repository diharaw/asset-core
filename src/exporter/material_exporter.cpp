#include <exporter/material_exporter.h>
#include <exporter/image_exporter.h>
#include <common/filesystem.h>
#include <json.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>

namespace ast
{
nlohmann::json to_json(glm::vec2 v)
{
    nlohmann::json json;

    json["x"] = v.x;
    json["y"] = v.y;

    return json;
}

nlohmann::json to_json(glm::vec3 v)
{
    nlohmann::json json;

    json["x"] = v.x;
    json["y"] = v.y;
    json["z"] = v.z;

    return json;
}

nlohmann::json to_json(TextureInfo texture_info)
{
    nlohmann::json json;

    json["path"]        = texture_info.path;
    json["srgb"]        = texture_info.srgb;
    json["offset"]      = to_json(texture_info.offset);
    json["scale"]       = to_json(texture_info.scale);

    return json;
}

nlohmann::json to_json(TextureRef texture_ref)
{
    nlohmann::json json;

    json["texture_idx"] = texture_ref.texture_idx;
    json["channel_idx"] = texture_ref.channel_idx;

    return json;
}

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

    std::string           path_to_textures_folder_absolute_string      = options.output_root_folder_path_absolute + "/texture";
    std::string           path_to_materials_folder_absolute_string     = options.output_root_folder_path_absolute + "/material";
    std::filesystem::path path_to_textures_folder_absolute             = path_to_textures_folder_absolute_string;
    std::filesystem::path path_to_textures_folder_relative_to_material = std::filesystem::relative(path_to_textures_folder_absolute, path_to_materials_folder_absolute_string);
    std::string           absolute_path_to_textures_folder             = path_to_textures_folder_absolute.string();
    std::string           relative_path_to_textures_folder             = path_to_textures_folder_relative_to_material.string();

    // Common
    {
        doc["name"]            = desc.name;
        doc["surface_type"]    = kSurfaceType[desc.surface_type];
        doc["material_type"]   = kMaterialType[desc.material_type];
        doc["alpha_mode"]      = kAlphaMode[desc.alpha_mode];
        doc["is_double_sided"] = desc.is_double_sided;

        auto texture_array = doc.array();

        for (int i = 0; i < desc.textures.size(); i++)
        {
            auto src_texture_info = desc.textures[i];

            std::string source_texture_path = src_texture_info.path;

            std::string absolute_path_to_output_texture = path_to_textures_folder_absolute_string;
            absolute_path_to_output_texture += "/";
            absolute_path_to_output_texture += filesystem::get_filename(src_texture_info.path);
            absolute_path_to_output_texture += ".ast";

            std::filesystem::path output_texture_path_relative_to_material = std::filesystem::relative(absolute_path_to_output_texture, path_to_materials_folder_absolute_string);

            // Check if asset exists
            bool exists = filesystem::does_file_exist(absolute_path_to_output_texture);

            if (!exists)
            {
                bool is_normal_map = false;

                if (desc.normal_texture.texture_idx == i || desc.clear_coat_normal_texture.texture_idx == i)
                    is_normal_map = true;

                export_texture(source_texture_path, absolute_path_to_textures_folder, is_normal_map ? true : false, options.use_compression, is_normal_map ? options.normal_map_flip_green : false);
            }

            TextureInfo dst_texture_info;
                
            dst_texture_info.path = output_texture_path_relative_to_material.string();
            dst_texture_info.srgb = src_texture_info.srgb;
            dst_texture_info.offset = src_texture_info.offset;
            dst_texture_info.scale = src_texture_info.scale;

            texture_array.push_back(to_json(dst_texture_info));
        }

        doc["textures"] = texture_array;
    }

    // Standard
    {
        doc["base_color"]           = to_json(desc.base_color);
        doc["metallic"]             = desc.metallic;
        doc["roughness"]            = desc.roughness;
        doc["emissive_factor"]      = to_json(desc.emissive_factor);
        doc["base_color_texture"]   = to_json(desc.base_color_texture);
        doc["roughness_texture"]    = to_json(desc.roughness_texture);
        doc["metallic_texture"]     = to_json(desc.metallic_texture);
        doc["normal_texture"]       = to_json(desc.normal_texture);
        doc["displacement_texture"] = to_json(desc.displacement_texture);
        doc["emissive_texture"]     = to_json(desc.emissive_texture);
    }

    // Sheen
    {
        doc["sheen_color"]             = to_json(desc.sheen_color);
        doc["sheen_roughness"]         = desc.sheen_roughness;
        doc["sheen_color_texture"]     = to_json(desc.sheen_color_texture);
        doc["sheen_roughness_texture"] = to_json(desc.sheen_roughness_texture);
    }

    // Clear Coat
    {
        doc["clear_coat"]                   = desc.clear_coat;
        doc["clear_coat_roughness"]         = desc.clear_coat_roughness;
        doc["clear_coat_texture"]           = to_json(desc.clear_coat_texture);
        doc["clear_coat_roughness_texture"] = to_json(desc.clear_coat_roughness_texture);
        doc["clear_coat_normal_texture"]    = to_json(desc.clear_coat_normal_texture);
    }

    // Anisotropy
    {
        doc["anisotropy"]                    = desc.anisotropy;
        doc["anisotropy_texture"]            = to_json(desc.anisotropy_texture);
        doc["anisotropy_directions_texture"] = to_json(desc.anisotropy_directions_texture);
    }

    // Transmission
    {
        doc["transmission"]         = desc.transmission;
        doc["transmission_texture"] = to_json(desc.transmission_texture);
    }

    // IOR
    {
        doc["ior"] = desc.ior;
    }

    // Volume
    {
        doc["thickness_factor"]     = desc.thickness_factor;
        doc["attenuation_distance"] = desc.attenuation_distance;
        doc["attenuation_color"]    = to_json(desc.attenuation_color);
        doc["thickness_texture"]    = to_json(desc.thickness_texture);
    }

    std::string output_path = path_to_materials_folder_absolute_string;
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
