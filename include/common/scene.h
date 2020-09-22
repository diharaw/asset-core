#pragma once

#include <glm.hpp>
#include <string>
#include <vector>
#include <memory>
#include <json.hpp>

namespace ast
{
static const std::string kSceneNodeType[] = {
    "SCENE_NODE_MESH",
    "SCENE_NODE_CAMERA",
    "SCENE_NODE_DIRECTIONAL_LIGHT",
    "SCENE_NODE_SPOT_LIGHT",
    "SCENE_NODE_POINT_LIGHT",
    "SCENE_NODE_IBL",
    "SCENE_NODE_CUSTOM"
};

enum SceneNodeType
{
    SCENE_NODE_MESH,
    SCENE_NODE_CAMERA,
    SCENE_NODE_DIRECTIONAL_LIGHT,
    SCENE_NODE_SPOT_LIGHT,
    SCENE_NODE_POINT_LIGHT,
    SCENE_NODE_IBL,
    SCENE_NODE_CUSTOM,
    SCENE_NODE_COUNT
};

struct SceneNode
{
    SceneNodeType type;
    std::string                             name;
    nlohmann::json                          custom_data;
    std::vector<std::shared_ptr<SceneNode>> children;
};

struct EnvironmentMapNode : public SceneNode
{
    std::string environment_map;
};

struct TransformNode : public SceneNode
{
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

struct MeshNode : public TransformNode
{
    std::string mesh;
    std::string material_override;
};

struct DirectionalLightNode : public TransformNode
{
    glm::vec3 color;
    float     intensity;
};

struct SpotLightNode : public TransformNode
{
    glm::vec3 color;
    float     cone_angle;
    float     range;
    float     intensity;
};

struct PointLightNode : public TransformNode
{
    glm::vec3 color;
    float     range;
    float     intensity;
};

struct CameraNode : public TransformNode
{
    float near_plane;
    float far_plane;
    float fov;
};

struct IBLNode : public SceneNode
{
    std::string image;
};

struct Scene
{
    std::string                name;
    std::shared_ptr<SceneNode> scene_graph;
};
} // namespace ast
