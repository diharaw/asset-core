#include <loader/loader.h>
#include <common/header.h>
#include <fstream>
#include <common/filesystem.h>
#include <json.hpp>

#define READ_AND_OFFSET(stream, dest, size, offset) \
    stream.read((char*)dest, size);                 \
    offset += size;                                 \
    stream.seekg(offset);

#define PARSE_DEFAULT(json, structure, dst, default_value) \
    if (json.find(#dst) != json.end())                     \
    {                                                      \
        structure.dst = json[#dst];                        \
    }                                                      \
    else                                                   \
    {                                                      \
        structure.dst = default_value;                     \
    }

#define PARSE_CUSTOM(json, structure, dst, parse_func, default_value) \
    if (json.find(#dst) != json.end())                                \
    {                                                                 \
        structure.dst = parse_func(json[#dst]);                       \
    }                                                                 \
    else                                                              \
    {                                                                 \
        structure.dst = default_value;                                \
    }

#define PARSE_DEFAULT_PTR(json, structure, dst, default_value) \
    if (json.find(#dst) != json.end())                         \
    {                                                          \
        structure->dst = json[#dst];                           \
    }                                                          \
    else                                                       \
    {                                                          \
        structure->dst = default_value;                        \
    }

#define PARSE_CUSTOM_PTR(json, structure, dst, parse_func, default_value) \
    if (json.find(#dst) != json.end())                                    \
    {                                                                     \
        structure->dst = parse_func(json[#dst]);                          \
    }                                                                     \
    else                                                                  \
    {                                                                     \
        structure->dst = default_value;                                   \
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
glm::vec2                  deserialize_vec2(const nlohmann::json& json);
glm::vec3                  deserialize_vec3(const nlohmann::json& json);
TextureInfo                deserialize_texture_info(const nlohmann::json& json);
TextureRef                 deserialize_texture_ref(const nlohmann::json& json);

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

        mesh.materials.push_back(material_path);
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
    std::string surface_type;
    std::string alpha_mode;

    PARSE_DEFAULT(j, material, name, "untitled");

    if (j.find("surface_type") != j.end())
    {
        surface_type = j["surface_type"];

        for (int i = 0; i < 3; i++)
        {
            if (kSurfaceType[i] == surface_type)
            {
                material.surface_type = (SurfaceType)i;
                break;
            }
        }
    }
    else
        material.surface_type = SURFACE_OPAQUE;

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
        material.material_type = MATERIAL_STANDARD;

    if (j.find("alpha_mode") != j.end())
    {
        alpha_mode = j["alpha_mode"];

        for (int i = 0; i < 4; i++)
        {
            if (kAlphaMode[i] == alpha_mode)
            {
                material.alpha_mode = (AlphaMode)i;
                break;
            }
        }
    }
    else
        material.alpha_mode = ALPHA_MODE_OPAQUE;

    PARSE_DEFAULT(j, material, is_double_sided, false);

    if (j.find("textures") != j.end())
    {
        auto json_texture_infos = j["textures"];

        for (auto& json_texture_info : json_texture_infos)
            material.textures.push_back(deserialize_texture_info(json_texture_info));
    }

    // Standard
    {
        PARSE_CUSTOM(j, material, base_color, deserialize_vec3, glm::vec3(1.0f));
        PARSE_DEFAULT(j, material, metallic, 0.0f);
        PARSE_DEFAULT(j, material, roughness, 1.0f);
        PARSE_CUSTOM(j, material, emissive_factor, deserialize_vec3, glm::vec3(0.0f));
        PARSE_CUSTOM(j, material, base_color_texture, deserialize_texture_ref, TextureRef());
        PARSE_CUSTOM(j, material, roughness_texture, deserialize_texture_ref, TextureRef());
        PARSE_CUSTOM(j, material, metallic_texture, deserialize_texture_ref, TextureRef());
        PARSE_CUSTOM(j, material, normal_texture, deserialize_texture_ref, TextureRef());
        PARSE_CUSTOM(j, material, displacement_texture, deserialize_texture_ref, TextureRef());
        PARSE_CUSTOM(j, material, emissive_texture, deserialize_texture_ref, TextureRef());
    }

    // Sheen
    {
        PARSE_CUSTOM(j, material, sheen_color, deserialize_vec3, glm::vec3(0.0f));
        PARSE_DEFAULT(j, material, sheen_roughness, 0.0f);
        PARSE_CUSTOM(j, material, sheen_color_texture, deserialize_texture_ref, TextureRef());
        PARSE_CUSTOM(j, material, sheen_roughness_texture, deserialize_texture_ref, TextureRef());
    }

    // Clear Coat
    {
        PARSE_DEFAULT(j, material, clear_coat, 0.0f);
        PARSE_DEFAULT(j, material, clear_coat_roughness, 0.0f);
        PARSE_CUSTOM(j, material, clear_coat_texture, deserialize_texture_ref, TextureRef());
        PARSE_CUSTOM(j, material, clear_coat_roughness_texture, deserialize_texture_ref, TextureRef());
        PARSE_CUSTOM(j, material, clear_coat_normal_texture, deserialize_texture_ref, TextureRef());
    }

    // Anisotropy
    {
        PARSE_DEFAULT(j, material, anisotropy, 0.0f);
        PARSE_CUSTOM(j, material, anisotropy_texture, deserialize_texture_ref, TextureRef());
        PARSE_CUSTOM(j, material, anisotropy_directions_texture, deserialize_texture_ref, TextureRef());
    }

    // IOR
    {
        PARSE_DEFAULT(j, material, ior, 1.5f);
    }

    // Volume
    {
        PARSE_DEFAULT(j, material, thickness_factor, 0.0f);
        PARSE_DEFAULT(j, material, attenuation_distance, INFINITY);
        PARSE_CUSTOM(j, material, attenuation_color, deserialize_vec3, glm::vec3(0.0f));
        PARSE_CUSTOM(j, material, thickness_texture, deserialize_texture_ref, TextureRef());
    }

    return true;
}

void deserialize_transform_node(const nlohmann::json& json, std::shared_ptr<SceneNode> node)
{
    std::shared_ptr<TransformNode> transform_node = std::static_pointer_cast<TransformNode>(node);

    PARSE_CUSTOM_PTR(json, transform_node, position, deserialize_vec3, glm::vec3(0.0f));
    PARSE_CUSTOM_PTR(json, transform_node, rotation, deserialize_vec3, glm::vec3(0.0f));
    PARSE_CUSTOM_PTR(json, transform_node, scale, deserialize_vec3, glm::vec3(0.0f));
}

std::shared_ptr<TransformNode> deserialize_root_node(const nlohmann::json& json)
{
    std::shared_ptr<TransformNode> node = std::make_shared<TransformNode>();

    deserialize_transform_node(json, node);

    return node;
}

std::shared_ptr<SceneNode> deserialize_mesh_node(const nlohmann::json& json)
{
    std::shared_ptr<MeshNode> node = std::make_shared<MeshNode>();

    deserialize_transform_node(json, node);

    PARSE_DEFAULT_PTR(json, node, mesh, "");
    PARSE_DEFAULT_PTR(json, node, material_override, "");
    PARSE_DEFAULT_PTR(json, node, casts_shadow, true);

    return std::static_pointer_cast<SceneNode>(node);
}

std::shared_ptr<SceneNode> deserialize_directional_light_node(const nlohmann::json& json)
{
    std::shared_ptr<DirectionalLightNode> node = std::make_shared<DirectionalLightNode>();

    deserialize_transform_node(json, node);

    PARSE_DEFAULT_PTR(json, node, intensity, 0.0f);
    PARSE_DEFAULT_PTR(json, node, radius, 0.0f);
    PARSE_DEFAULT_PTR(json, node, casts_shadows, false);
    PARSE_CUSTOM_PTR(json, node, color, deserialize_vec3, glm::vec3(1.0f));
    PARSE_CUSTOM_PTR(json, node, rotation, deserialize_vec3, glm::vec3(0.0f));

    return std::static_pointer_cast<SceneNode>(node);
}

std::shared_ptr<SceneNode> deserialize_spot_light_node(const nlohmann::json& json)
{
    std::shared_ptr<SpotLightNode> node = std::make_shared<SpotLightNode>();

    deserialize_transform_node(json, node);

    PARSE_DEFAULT_PTR(json, node, inner_cone_angle, 0.0f);
    PARSE_DEFAULT_PTR(json, node, outer_cone_angle, 0.0f);
    PARSE_DEFAULT_PTR(json, node, radius, 0.0f);
    PARSE_DEFAULT_PTR(json, node, intensity, 0.0f);
    PARSE_DEFAULT_PTR(json, node, casts_shadows, false);
    PARSE_CUSTOM_PTR(json, node, color, deserialize_vec3, glm::vec3(1.0f));
    PARSE_CUSTOM_PTR(json, node, position, deserialize_vec3, glm::vec3(0.0f));
    PARSE_CUSTOM_PTR(json, node, rotation, deserialize_vec3, glm::vec3(0.0f));

    return std::static_pointer_cast<SceneNode>(node);
}

std::shared_ptr<SceneNode> deserialize_point_light_node(const nlohmann::json& json)
{
    std::shared_ptr<PointLightNode> node = std::make_shared<PointLightNode>();

    deserialize_transform_node(json, node);

    PARSE_DEFAULT_PTR(json, node, radius, 0.0f);
    PARSE_DEFAULT_PTR(json, node, intensity, 0.0f);
    PARSE_DEFAULT_PTR(json, node, casts_shadows, false);
    PARSE_CUSTOM_PTR(json, node, color, deserialize_vec3, glm::vec3(1.0f));
    PARSE_CUSTOM_PTR(json, node, position, deserialize_vec3, glm::vec3(0.0f));

    return std::static_pointer_cast<SceneNode>(node);
}

std::shared_ptr<SceneNode> deserialize_camera_node(const nlohmann::json& json)
{
    std::shared_ptr<CameraNode> node = std::make_shared<CameraNode>();

    deserialize_transform_node(json, node);

    PARSE_DEFAULT_PTR(json, node, near_plane, 1.0f);
    PARSE_DEFAULT_PTR(json, node, far_plane, 100.0f);
    PARSE_DEFAULT_PTR(json, node, fov, 60.0f);

    return std::static_pointer_cast<SceneNode>(node);
}

std::shared_ptr<SceneNode> deserialize_ibl_node(const nlohmann::json& json)
{
    std::shared_ptr<IBLNode> node = std::make_shared<IBLNode>();

    PARSE_DEFAULT_PTR(json, node, image, "");

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

    if (type == SCENE_NODE_MESH)
        node = deserialize_mesh_node(json);
    else if (type == SCENE_NODE_CAMERA)
        node = deserialize_camera_node(json);
    else if (type == SCENE_NODE_DIRECTIONAL_LIGHT)
        node = deserialize_directional_light_node(json);
    else if (type == SCENE_NODE_SPOT_LIGHT)
        node = deserialize_spot_light_node(json);
    else if (type == SCENE_NODE_POINT_LIGHT)
        node = deserialize_point_light_node(json);
    else if (type == SCENE_NODE_IBL)
        node = deserialize_ibl_node(json);
    else if (type == SCENE_NODE_ROOT)
        node = deserialize_root_node(json);
    else if (type == SCENE_NODE_CUSTOM)
        node = std::make_shared<SceneNode>();

    node->type = type;

    PARSE_DEFAULT_PTR(json, node, name, "");

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

glm::vec2 deserialize_vec2(const nlohmann::json& json)
{
    glm::vec2 vec;

    int i = 0;

    if (json.size() == 2)
    {
        for (auto& value : json)
        {
            vec[i] = value;
            i++;
        }
    }

    return vec;
}

glm::vec3 deserialize_vec3(const nlohmann::json& json)
{
    glm::vec3 vec;

    int i = 0;

    if (json.size() == 3)
    {
        for (auto& value : json)
        {
            vec[i] = value;
            i++;
        }
    }

    return vec;
}

TextureInfo deserialize_texture_info(const nlohmann::json& json)
{
    TextureInfo texture_info;

    PARSE_DEFAULT(json, texture_info, srgb, false);
    PARSE_CUSTOM(json, texture_info, offset, deserialize_vec2, glm::vec3(0.0f));
    PARSE_CUSTOM(json, texture_info, scale, deserialize_vec2, glm::vec3(1.0f));

    return texture_info;
}

TextureRef deserialize_texture_ref(const nlohmann::json& json)
{
    TextureRef texture_ref;

    PARSE_DEFAULT(json, texture_ref, texture_idx, -1);
    PARSE_DEFAULT(json, texture_ref, channel_idx, 0);

    return texture_ref;
}

bool load_scene(const std::string& path, Scene& scene)
{
    std::ifstream i(path);

    if (!i.is_open())
        return false;

    nlohmann::json j;
    i >> j;

    PARSE_DEFAULT(j, scene, name, "");
    PARSE_CUSTOM(j, scene, scene_graph, deserialize_scene_node, nullptr);

    return true;
}
} // namespace ast
