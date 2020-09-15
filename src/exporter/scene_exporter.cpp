#include <exporter/scene_exporter.h>
#include <common/filesystem.h>
#include <json.hpp>
#include <iostream>
#include <fstream>
#include <memory>

namespace ast
{
nlohmann::json serialize_scene_node(std::shared_ptr<SceneNode> node);
void serialize_transform_node(std::shared_ptr<SceneNode> node, nlohmann::json& json);
void serialize_mesh_node(std::shared_ptr<SceneNode> node, nlohmann::json& json);
void serialize_directional_light_node(std::shared_ptr<SceneNode> node, nlohmann::json& json);
void serialize_spot_light_node(std::shared_ptr<SceneNode> node, nlohmann::json& json);
void serialize_point_light_node(std::shared_ptr<SceneNode> node, nlohmann::json& json);

nlohmann::json serialize_scene_node(std::shared_ptr<SceneNode> node)
{
    nlohmann::json json;

    json["type"]        = kSceneNodeType[node->type];
    json["name"]        = node->name;
    json["custom_data"] = node->custom_data;

    if (node->type == SCENE_NODE_MESH)
        serialize_mesh_node(node, json);
    else if (node->type == SCENE_NODE_DIRECTIONAL_LIGHT)
        serialize_directional_light_node(node, json);
    else if (node->type == SCENE_NODE_SPOT_LIGHT)
        serialize_spot_light_node(node, json);
    else if (node->type == SCENE_NODE_POINT_LIGHT)
        serialize_point_light_node(node, json);
  
    auto children = json.array();

    for (auto child : node->children)
        children.push_back(serialize_scene_node(child));

    json["children"] = children;

    return json;
}

void serialize_transform_node(std::shared_ptr<SceneNode> node, nlohmann::json& json)
{
    std::shared_ptr<TransformNode> transform_node = std::dynamic_pointer_cast<TransformNode>(node);

    auto position = json.array();
    position.push_back(transform_node->position[0]);
    position.push_back(transform_node->position[1]);
    position.push_back(transform_node->position[2]);

    json["position"] = position;

    auto rotation = json.array();
    rotation.push_back(transform_node->rotation[0]);
    rotation.push_back(transform_node->rotation[1]);
    rotation.push_back(transform_node->rotation[2]);

    json["rotation"] = rotation;

    auto scale = json.array();
    scale.push_back(transform_node->scale[0]);
    scale.push_back(transform_node->scale[1]);
    scale.push_back(transform_node->scale[2]);

    json["scale"] = scale;
}

void serialize_mesh_node(std::shared_ptr<SceneNode> node, nlohmann::json& json)
{
    std::shared_ptr<MeshNode> mesh_node = std::dynamic_pointer_cast<MeshNode>(node);

    json["mesh"] = mesh_node->mesh;

    if (mesh_node->material_override.size() > 0)
        json["material_override"] = mesh_node->material_override;
    else
        json["material_override"] = nullptr;
}

void serialize_directional_light_node(std::shared_ptr<SceneNode> node, nlohmann::json& json)
{
    std::shared_ptr<DirectionalLightNode> light_node = std::dynamic_pointer_cast<DirectionalLightNode>(node);

    auto color = json.array();
    color.push_back(light_node->color[0]);
    color.push_back(light_node->color[1]);
    color.push_back(light_node->color[2]);

    json["color"] = color;

    auto rotation = json.array();
    rotation.push_back(light_node->rotation[0]);
    rotation.push_back(light_node->rotation[1]);
    rotation.push_back(light_node->rotation[2]);

    json["rotation"] = rotation;
    json["intensity"] = light_node->intensity;
}

void serialize_spot_light_node(std::shared_ptr<SceneNode> node, nlohmann::json& json)
{
    std::shared_ptr<SpotLightNode> light_node = std::dynamic_pointer_cast<SpotLightNode>(node);

    auto color = json.array();
    color.push_back(light_node->color[0]);
    color.push_back(light_node->color[1]);
    color.push_back(light_node->color[2]);

    json["color"] = color;

    auto position = json.array();
    position.push_back(light_node->position[0]);
    position.push_back(light_node->position[1]);
    position.push_back(light_node->position[2]);

    json["position"] = position;

    auto rotation = json.array();
    rotation.push_back(light_node->rotation[0]);
    rotation.push_back(light_node->rotation[1]);
    rotation.push_back(light_node->rotation[2]);

    json["rotation"] = rotation;
    json["cone_angle"]            = light_node->cone_angle;
    json["range"]      = light_node->range;
    json["intensity"]             = light_node->intensity;
}

void serialize_point_light_node(std::shared_ptr<SceneNode> node, nlohmann::json& json)
{
    std::shared_ptr<PointLightNode> light_node = std::dynamic_pointer_cast<PointLightNode>(node);

    auto color = json.array();
    color.push_back(light_node->color[0]);
    color.push_back(light_node->color[1]);
    color.push_back(light_node->color[2]);

    json["color"] = color;

    auto position = json.array();
    position.push_back(light_node->position[0]);
    position.push_back(light_node->position[1]);
    position.push_back(light_node->position[2]);

    json["position"] = position;

    json["range"]     = light_node->range;
    json["intensity"] = light_node->intensity;
}

bool export_scene(const Scene& scene, const std::string& path)
{
    nlohmann::json doc;

    if (scene.name.size() == 0)
        doc["name"] = "untitled_scene";
    else
        doc["name"] = scene.name;

    if (scene.scene_graph)
        doc["scene_graph"] = serialize_scene_node(scene.scene_graph);

    std::string scene_name = scene.name;

    if (scene_name.size() == 0)
        scene_name = "untitled_scene";

    std::string output_path = path;
    output_path += "/";
    output_path += scene_name;
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
