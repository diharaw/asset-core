#include <runtime/loader.h>
#include <common/header.h>
#include <fstream>
#include <common/filesystem.h>
#include <json.hpp>

#define READ_AND_OFFSET(stream, dest, size, offset) stream.read((char*)dest, size); offset += size; stream.seekg(offset);

#define JSON_PARSE_VECTOR(src, dst, name, vec_size)    \
if (src.find(#name) != src.end())                     \
{                                                      \
auto vec = src[#name];                            \
int i = 0;                                         \
\
if (vec.size() == vec_size)                        \
{                                                  \
for (auto& value : vec)                        \
{                                              \
dst[i] = value;                            \
i++;                                        \
}                                              \
}                                                  \
}



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
        image.type = (PixelType)image_header.channel_size;
        image.compression = (CompressionType)image_header.compression;
        
        if (image_header.num_array_slices == 0)
            return false;
        
        for (int i = 0; i < image.array_slices; i++)
        {
            for (int j = 0; j < image.mip_slices; j++)
            {
                BINMipSliceHeader mip_header;
                READ_AND_OFFSET(f, &mip_header, sizeof(BINMipSliceHeader), offset);
                
                image.data[i][j].width = mip_header.width;
                image.data[i][j].height = mip_header.height;
                image.data[i][j].data = malloc(mip_header.size);
                image.data[i][j].size = mip_header.size;
                
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
        
        mesh.name = mesh_header.name;
        mesh.max_extents = mesh_header.max_extents;
        mesh.min_extents = mesh_header.min_extents;
        
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
            mesh.material_paths.resize(mesh_header.material_count);
            READ_AND_OFFSET(f, (char*)&bin_materials[0], sizeof(BINMeshMaterialJson) * bin_materials.size(), offset);
        }
        
        for (int i = 0; i < mesh_header.material_count; i++)
        {
            std::string relative_path = bin_materials[i].material;
            std::string parent_path = filesystem::get_file_path(path);
            std::string material_path = "";
            
            if (parent_path.length() == 0)
                material_path = relative_path;
            else
                material_path = parent_path + relative_path;
            
            mesh.material_paths[i] = material_path;
            
            if (!load_material(material_path, mesh.materials[i]))
            {
                std::cout << "Failed to load material: " << bin_materials[i].material << std::endl;
                return false;
            }
        }
        
        return true;
    }
    
    bool load_material(const std::string& path, Material& material)
    {
        std::ifstream i(path);
        
        nlohmann::json j;
        i >> j;

        std::string blend_mode;
        std::string displacement_type;
        std::string lighting_model;
        std::string shading_model;
        
        if (j.find("name") != j.end())
            material.name = j["name"];
        else
            material.name = "untitled";
        
        if (j.find("blend_mode") != j.end())
        {
            blend_mode = j["blend_mode"];
            
            for (int i = 0; i < 4; i++)
            {
                if (kBlendMode[i] == blend_mode)
                {
                    material.blend_mode = (BlendMode)i;
                    break;
                }
            }
        }
        else
            material.blend_mode = BLEND_MODE_OPAQUE;
        
        if (j.find("displacement_type") != j.end())
        {
            displacement_type = j["displacement_type"];
            
            for (int i = 0; i < 3; i++)
            {
                if (kDisplacementType[i] == displacement_type)
                {
                    material.displacement_type = (DisplacementType)i;
                    break;
                }
            }
        }
        else
            material.displacement_type = DISPLACEMENT_NONE;
        
        if (j.find("lighting_model") != j.end())
        {
            lighting_model = j["lighting_model"];
            
            for (int i = 0; i < 2; i++)
            {
                if (kLightingModel[i] == lighting_model)
                {
                    material.lighting_model = (LightingModel)i;
                    break;
                }
            }
        }
        else
            material.lighting_model = LIGHTING_MODEL_LIT;
        
        if (j.find("shading_model") != j.end())
        {
            shading_model = j["shading_model"];
            
            for (int i = 0; i < 3; i++)
            {
                if (kShadingModel[i] == shading_model)
                {
                    material.shading_model = (ShadingModel)i;
                    break;
                }
            }
        }
        else
            material.shading_model = SHADING_MODEL_STANDARD;
        
        if (j.find("double_sided") != j.end())
            material.double_sided = j["double_sided"];
        else
            material.double_sided = false;
        
        if (j.find("metallic_workflow") != j.end())
            material.metallic_workflow = j["metallic_workflow"];
        else
            material.metallic_workflow = true;
        
        if (j.find("fragment_shader_func") != j.end())
        {
            auto shader_lines = j["fragment_shader_func"];
            std::string source = "";
            
            for (auto& line : shader_lines)
            {
                std::string str_line = line;
                source += str_line;
                source += "\n";
            }
            
            material.fragment_shader_func = source;
        }
        
        if (j.find("vertex_shader_func") != j.end())
        {
            auto shader_lines = j["vertex_shader_func"];
            std::string source = "";
            
            for (auto& line : shader_lines)
            {
                std::string str_line = line;
                source += str_line;
                source += "\n";
            }
            
            material.vertex_shader_func = source;
        }
        
        if (j.find("textures") != j.end())
        {
            auto json_textures = j["textures"];
            
            for (auto& json_texture : json_textures)
            {
                Texture texture;
                
                if (json_texture.find("srgb") != json_texture.end())
                    texture.srgb = json_texture["srgb"];
                else
                    texture.srgb = true;
                
                if (json_texture.find("path") != json_texture.end())
                    texture.path = json_texture["path"];
                
                if (json_texture.find("type") != json_texture.end())
                {
                    std::string tex_type = json_texture["type"];
                    
                    for (int i = 0; i < 9; i++)
                    {
                        if (kTextureType[i] == tex_type)
                        {
                            texture.type = (TextureType)i;
                            break;
                        }
                    }
                }
                
                material.textures.push_back(texture);
            }
        }
        
        if (j.find("properties") != j.end())
        {
            auto json_properties = j["properties"];
            
            for (auto& json_property : json_properties)
            {
                MaterialProperty property;
                std::string type = "";
                bool found = false;
                
                if (json_property.find("type") != json_property.end())
                {
                    type = json_property["type"];
                    
                    if (type == kPropertyType[PROPERTY_ALBEDO])
                    {
                        property.type = PROPERTY_ALBEDO;
                        
                        if (json_property.find("value") != json_property.end())
                        {
                            auto vec = json_property["value"];
                            int i = 0;
                            
                            if (vec.size() != 4)
                                continue;
                            
                            for (auto& value : vec)
                            {
                                property.vec4_value[i] = value;
                                i++;
                            }
                            
                            found = true;
                        }
                    }
                    else if (type == kPropertyType[PROPERTY_EMISSIVE])
                    {
                        property.type = PROPERTY_EMISSIVE;
                        
                        if (json_property.find("value") != json_property.end())
                        {
                            auto vec = json_property["value"];
                            int i = 0;
                            
                            if (vec.size() != 4)
                                continue;
                            
                            for (auto& value : vec)
                            {
                                property.vec4_value[i] = value;
                                i++;
                            }
                            
                            found = true;
                        }
                    }
                    else if (type == kPropertyType[PROPERTY_METALNESS])
                    {
                        property.type = PROPERTY_METALNESS;
                        
                        if (json_property.find("value") != json_property.end())
                            property.float_value = json_property["value"];
                        
                        found = true;
                    }
                    else if (type == kPropertyType[PROPERTY_ROUGHNESS])
                    {
                        property.type = PROPERTY_ROUGHNESS;
                        
                        if (json_property.find("value") != json_property.end())
                            property.float_value = json_property["value"];
                        
                        found = true;
                    }
                    else if (type == kPropertyType[PROPERTY_SPECULAR])
                    {
                        property.type = PROPERTY_EMISSIVE;
                        
                        if (json_property.find("value") != json_property.end())
                        {
                            auto vec = json_property["value"];
                            int i = 0;
                            
                            if (vec.size() != 3)
                                continue;
                            
                            for (auto& value : vec)
                            {
                                property.vec3_value[i] = value;
                                i++;
                            }
                            
                            found = true;
                        }
                    }
                    else if (type == kPropertyType[PROPERTY_GLOSSINESS])
                    {
                        property.type = PROPERTY_GLOSSINESS;
                        
                        if (json_property.find("value") != json_property.end())
                            property.float_value = json_property["value"];
                        
                        found = true;
                    }
                }
                
                if (found)
                    material.properties.push_back(property);
            }
        }
        
        return true;
    }
    
    bool load_scene(const std::string& path, Scene& scene)
    {
        std::ifstream i(path);
        
        nlohmann::json j;
        i >> j;
        
        if (j.find("name") != j.end())
            scene.name = j["name"];
        
        if (j.find("camera") != j.end())
        {
            auto camera = j["camera"];
            
            if (camera.find("type") != camera.end())
            {
                std::string type = camera["type"];
                
                for (int i = 0; i < 2; i++)
                {
                    if (kCameraTypes[i] == type)
                    {
                        scene.camera.type = (CameraType)i;
                        break;
                    }
                }
            }
            
            if (camera.find("rotation_speed") != camera.end())
                scene.camera.rotation_speed = camera["rotation_speed"];
            
            if (scene.camera.type == CAMERA_ORBIT)
            {
                JSON_PARSE_VECTOR(camera, scene.camera.orbit_center, orbit_center, 3);
                
                if (camera.find("orbit_boom_length") != camera.end())
                    scene.camera.orbit_boom_length = camera["orbit_boom_length"];
            }
            else if (scene.camera.type == CAMERA_FLYTHROUGH)
            {
                JSON_PARSE_VECTOR(camera, scene.camera.position, position, 3);
                JSON_PARSE_VECTOR(camera, scene.camera.rotation, rotation, 3);
                
                if (camera.find("movement_speed") != camera.end())
                    scene.camera.movement_speed = camera["movement_speed"];
            }
        }
        
        if (j.find("skybox") != j.end())
        {
            auto skybox = j["skybox"];
            
            if (skybox.find("type") != skybox.end())
            {
                std::string type = skybox["type"];
                
                for (int i = 0; i < 2; i++)
                {
                    if (kSkyboxType[i] == type)
                    {
                        scene.skybox.type = (SkyboxType)i;
                        break;
                    }
                }
            }
            
            if (scene.skybox.type == SKYBOX_STATIC)
            {
                if (skybox.find("environment_map") != skybox.end())
                    scene.skybox.environment_map = skybox["environment_map"];
                
                if (skybox.find("diffuse_irradiance") != skybox.end())
                    scene.skybox.diffuse_irradiance = skybox["diffuse_irradiance"];
                
                if (skybox.find("specular_irradiance") != skybox.end())
                    scene.skybox.specular_irradiance = skybox["specular_irradiance"];
            }
        }
        
        if (j.find("reflection_probes") != j.end())
        {
            auto reflection_probes = j["reflection_probes"];
            
            for (auto probe : reflection_probes)
            {
                ReflectionProbe new_probe;
                
                if (probe.find("path") != probe.end())
                    new_probe.path = probe["path"];
               
                JSON_PARSE_VECTOR(probe, new_probe.position, position, 3);
                JSON_PARSE_VECTOR(probe, new_probe.extents, extents, 3);
                
                scene.reflection_probes.push_back(new_probe);
            }
        }
        
        if (j.find("gi_probes") != j.end())
        {
            auto gi_probes = j["gi_probes"];
            
            for (auto probe : gi_probes)
            {
                GIProbe new_probe;
                
                if (probe.find("path") != probe.end())
                    new_probe.path = probe["path"];
                
                JSON_PARSE_VECTOR(probe, new_probe.position, position, 3);
                
                scene.gi_probes.push_back(new_probe);
            }
        }
        
        if (j.find("entities") != j.end())
        {
            auto entities = j["entities"];
            
            for (auto entity : entities)
            {
                Entity new_entity;
                
                if (entity.find("name") != entity.end())
                    new_entity.name = entity["name"];
                
                if (entity.find("mesh") != entity.end())
                    new_entity.mesh = entity["mesh"];
                
                if (entity.find("material_override") != entity.end())
                {
                    auto material_override = entity["material_override"];
                    
                    if (!material_override.is_null())
                        new_entity.material_override = material_override;
                }
                
                JSON_PARSE_VECTOR(entity, new_entity.position, position, 3);
                JSON_PARSE_VECTOR(entity, new_entity.rotation, rotation, 3);
                JSON_PARSE_VECTOR(entity, new_entity.scale, scale, 3);
                
                scene.entities.push_back(new_entity);
            }
        }
        
        return true;
    }
}
