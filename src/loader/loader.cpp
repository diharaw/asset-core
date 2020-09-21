#include <loader/loader.h>
#include <common/header.h>
#include <fstream>
#include <common/filesystem.h>
#include <json.hpp>

#define READ_AND_OFFSET(stream, dest, size, offset) \
    stream.read((char*)dest, size);                 \
    offset += size;                                 \
    stream.seekg(offset);

#define JSON_PARSE_VECTOR(src, dst, name, vec_size) \
    if (src.find(#name) != src.end())               \
    {                                               \
        auto vec = src[#name];                      \
        int  i   = 0;                               \
                                                    \
        if (vec.size() == vec_size)                 \
        {                                           \
            for (auto& value : vec)                 \
            {                                       \
                dst[i] = value;                     \
                i++;                                \
            }                                       \
        }                                           \
    }

namespace ast
{
void                       deserialize_transform_node(const nlohmann::json& json, std::shared_ptr<SceneNode> node);
std::shared_ptr<SceneNode> deserialize_mesh_node(const nlohmann::json& json);
std::shared_ptr<SceneNode> deserialize_directional_light_node(const nlohmann::json& json);
std::shared_ptr<SceneNode> deserialize_spot_light_node(const nlohmann::json& json);
std::shared_ptr<SceneNode> deserialize_point_light_node(const nlohmann::json& json);
std::shared_ptr<SceneNode> deserialize_camera_node(const nlohmann::json& json);
std::shared_ptr<SceneNode> deserialize_ibl_node(const nlohmann::json& json);
std::shared_ptr<SceneNode> deserialize_scene_node(const nlohmann::json& json);

bool load_image(const std::string& path, Image& image)
{
    std::fstream f(path, std::ios::in | std::ios::binary);

    if (!f.is_open())
        return false;

    BINFileHeader  file_header;
    BINImageHeader image_header;
    uint16_t       len = 0;

    size_t offset = 0;

    READ_AND_OFFSET(f, &file_header, sizeof(BINFileHeader), offset);

    READ_AND_OFFSET(f, &len, sizeof(uint16_t), offset);
    image.name.resize(len);

    READ_AND_OFFSET(f, image.name.c_str(), len, offset);

    READ_AND_OFFSET(f, &image_header, sizeof(BINImageHeader), offset);

    image.array_slices = image_header.num_array_slices;
    image.mip_slices   = image_header.num_mip_slices;
    image.components   = image_header.num_channels;
    image.type         = (PixelType)image_header.channel_size;
    image.compression  = (CompressionType)image_header.compression;

    if (image_header.num_array_slices == 0)
        return false;

    for (int i = 0; i < image.array_slices; i++)
    {
        for (int j = 0; j < image.mip_slices; j++)
        {
            BINMipSliceHeader mip_header;
            READ_AND_OFFSET(f, &mip_header, sizeof(BINMipSliceHeader), offset);

            image.data[i][j].width  = mip_header.width;
            image.data[i][j].height = mip_header.height;
            image.data[i][j].data   = malloc(mip_header.size);
            image.data[i][j].size   = mip_header.size;

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

    BINFileHeader     file_header;
    BINMeshFileHeader mesh_header;

    size_t offset = 0;

    READ_AND_OFFSET(f, &file_header, sizeof(BINFileHeader), offset);

    READ_AND_OFFSET(f, (char*)&mesh_header, sizeof(BINMeshFileHeader), offset);

    mesh.name        = mesh_header.name;
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
        std::string parent_path   = filesystem::get_file_path(path);
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

    if (!i.is_open())
        return false;

    nlohmann::json j;
    i >> j;

    std::string material_type;
    std::string shading_model;

    if (j.find("name") != j.end())
        material.name = j["name"];
    else
        material.name = "untitled";

    if (j.find("double_sided") != j.end())
        material.double_sided = j["double_sided"];
    else
        material.double_sided = false;

    if (j.find("alpha_mask") != j.end())
        material.alpha_mask = j["alpha_mask"];
    else
        material.alpha_mask = false;

    if (j.find("orca") != j.end())
        material.orca = j["orca"];
    else
        material.orca = false;

    if (j.find("metallic_workflow") != j.end())
        material.metallic_workflow = j["metallic_workflow"];
    else
        material.metallic_workflow = true;

    if (j.find("material_type") != j.end())
    {
        material_type = j["material_type"];

        for (int i = 0; i < 4; i++)
        {
            if (kMaterialType[i] == material_type)
            {
                material.material_type = (MaterialType)i;
                break;
            }
        }
    }
    else
        material.material_type = MATERIAL_OPAQUE;

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
            {
                std::string relative_path = json_texture["path"];
                std::string parent_path   = filesystem::get_file_path(path);
                std::string material_path = "";

                if (parent_path.length() == 0)
                    texture.path = relative_path;
                else
                    texture.path = parent_path + relative_path;
            }

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
            std::string      type  = "";
            bool             found = false;

            if (json_property.find("type") != json_property.end())
            {
                type = json_property["type"];

                if (type == kPropertyType[PROPERTY_ALBEDO])
                {
                    property.type = PROPERTY_ALBEDO;

                    if (json_property.find("value") != json_property.end())
                    {
                        auto vec = json_property["value"];
                        int  i   = 0;

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
                        int  i   = 0;

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
                else if (type == kPropertyType[PROPERTY_METALNESS_SPECULAR])
                {
                    property.type = PROPERTY_METALNESS_SPECULAR;

                    if (json_property.find("value") != json_property.end())
                    {
                        if (material.metallic_workflow)
                        {
                            property.float_value = json_property["value"];
                            found                = true;
                        }
                        else
                        {
                            auto vec = json_property["value"];
                            int  i   = 0;

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
                }
                else if (type == kPropertyType[PROPERTY_ROUGHNESS_GLOSSINESS])
                {
                    property.type = PROPERTY_ROUGHNESS_GLOSSINESS;

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

void deserialize_transform_node(const nlohmann::json& json, std::shared_ptr<SceneNode> node)
{
    std::shared_ptr<TransformNode> transform_node = std::static_pointer_cast<TransformNode>(node);

    JSON_PARSE_VECTOR(json, transform_node->position, position, 3);
    JSON_PARSE_VECTOR(json, transform_node->rotation, rotation, 3);
    JSON_PARSE_VECTOR(json, transform_node->scale, scale, 3);
}

std::shared_ptr<SceneNode> deserialize_mesh_node(const nlohmann::json& json)
{
    std::shared_ptr<MeshNode> node = std::make_shared<MeshNode>();

    deserialize_transform_node(json, node);

    if (json.find("mesh") != json.end())
        node->mesh = json["mesh"];

    if (json.find("material_override") != json.end())
        node->material_override = json["material_override"];

    return std::static_pointer_cast<SceneNode>(node);
}

std::shared_ptr<SceneNode> deserialize_directional_light_node(const nlohmann::json& json)
{
    std::shared_ptr<DirectionalLightNode> node = std::make_shared<DirectionalLightNode>();

    deserialize_transform_node(json, node);

    if (json.find("intensity") != json.end())
        node->intensity = json["intensity"];

    JSON_PARSE_VECTOR(json, node->color, color, 3);
    JSON_PARSE_VECTOR(json, node->rotation, rotation, 3);

    return std::static_pointer_cast<SceneNode>(node);
}

std::shared_ptr<SceneNode> deserialize_spot_light_node(const nlohmann::json& json)
{
    std::shared_ptr<SpotLightNode> node = std::make_shared<SpotLightNode>();

    deserialize_transform_node(json, node);

    if (json.find("cone_angle") != json.end())
        node->cone_angle = json["cone_angle"];

    if (json.find("range") != json.end())
        node->range = json["range"];

    if (json.find("intensity") != json.end())
        node->intensity = json["intensity"];

    JSON_PARSE_VECTOR(json, node->color, color, 3);
    JSON_PARSE_VECTOR(json, node->position, position, 3);
    JSON_PARSE_VECTOR(json, node->rotation, rotation, 3);

    return std::static_pointer_cast<SceneNode>(node);
}

std::shared_ptr<SceneNode> deserialize_point_light_node(const nlohmann::json& json)
{
    std::shared_ptr<PointLightNode> node = std::make_shared<PointLightNode>();

    deserialize_transform_node(json, node);

    if (json.find("range") != json.end())
        node->range = json["range"];

    if (json.find("intensity") != json.end())
        node->intensity = json["intensity"];

    JSON_PARSE_VECTOR(json, node->color, color, 3);
    JSON_PARSE_VECTOR(json, node->position, position, 3);

    return std::static_pointer_cast<SceneNode>(node);
}

std::shared_ptr<SceneNode> deserialize_camera_node(const nlohmann::json& json)
{
    std::shared_ptr<CameraNode> node = std::make_shared<CameraNode>();

    deserialize_transform_node(json, node);

    if (json.find("near_plane") != json.end())
        node->near_plane = json["near_plane"];

    if (json.find("far_plane") != json.end())
        node->far_plane = json["far_plane"];

    if (json.find("fov") != json.end())
        node->fov = json["fov"];

    return std::static_pointer_cast<SceneNode>(node);
}

std::shared_ptr<SceneNode> deserialize_ibl_node(const nlohmann::json& json)
{
    std::shared_ptr<IBLNode> node = std::make_shared<IBLNode>();

    if (json.find("image") != json.end())
        node->image = json["image"];

    return std::static_pointer_cast<SceneNode>(node);
}

std::shared_ptr<SceneNode> deserialize_scene_node(const nlohmann::json& json)
{
    std::shared_ptr<SceneNode> node;

    SceneNodeType type;

    if (json.find("type") != json.end())
    {
        std::string node_type = json["type"];

        for (int i = 0; i < SCENE_NODE_COUNT; i++)
        {
            if (kSceneNodeType[i] == node_type)
            {
                type = (SceneNodeType)i;
                break;
            }
        }
    }
    else
        return nullptr;

    if (node->type == SCENE_NODE_MESH)
        node = deserialize_mesh_node(json);
    else if (node->type == SCENE_NODE_CAMERA)
        node = deserialize_camera_node(json);
    else if (node->type == SCENE_NODE_DIRECTIONAL_LIGHT)
        node = deserialize_directional_light_node(json);
    else if (node->type == SCENE_NODE_SPOT_LIGHT)
        node = deserialize_spot_light_node(json);
    else if (node->type == SCENE_NODE_POINT_LIGHT)
        node = deserialize_point_light_node(json);
    else if (node->type == SCENE_NODE_IBL)
        node = deserialize_ibl_node(json);
    else if (node->type == SCENE_NODE_CUSTOM)
        node = std::make_shared<SceneNode>();

    node->type = type;

    if (json.find("name") != json.end())
        node->name = json["name"];

    if (json.find("custom_data") != json.end())
        node->custom_data = json["custom_data"];

    if (json.find("children") != json.end())
    {
        auto children = json["children"];

        for (auto child : children)
            node->children.push_back(deserialize_scene_node(child));
    }

    return node;
}

bool load_scene(const std::string& path, Scene& scene)
{
    std::ifstream i(path);

    if (!i.is_open())
        return false;

    nlohmann::json j;
    i >> j;

    if (j.find("name") != j.end())
        scene.name = j["name"];

    if (j.find("scene_graph") != j.end())
        scene.scene_graph = deserialize_scene_node(j["scene_graph"]);

    return true;
}
} // namespace ast
