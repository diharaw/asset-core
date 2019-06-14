#include <offline/scene_exporter.h>
#include <common/filesystem.h>
#include <json.hpp>
#include <iostream>
#include <fstream>

namespace ast
{
bool export_scene(const Scene& scene, const std::string& path)
{
    nlohmann::json doc;

    if (scene.name.size() == 0)
        doc["name"] = "untitled_scene";
    else
        doc["name"] = scene.name;

    // Camera Properties
    nlohmann::json camera;

    camera["type"]           = kCameraTypes[scene.camera.type];
    camera["rotation_speed"] = scene.camera.rotation_speed;
    camera["near_plane"]     = scene.camera.near_plane;
    camera["far_plane"]      = scene.camera.far_plane;

    if (scene.camera.type == CAMERA_ORBIT)
    {
        camera["orbit_boom_length"] = scene.camera.orbit_boom_length;

        auto orbit_center = doc.array();
        orbit_center.push_back(scene.camera.orbit_center[0]);
        orbit_center.push_back(scene.camera.orbit_center[1]);
        orbit_center.push_back(scene.camera.orbit_center[2]);

        camera["orbit_center"] = orbit_center;
    }
    else if (scene.camera.type == CAMERA_FLYTHROUGH)
    {
        camera["movement_speed"] = scene.camera.movement_speed;

        auto position = doc.array();
        position.push_back(scene.camera.position[0]);
        position.push_back(scene.camera.position[1]);
        position.push_back(scene.camera.position[2]);

        camera["position"] = position;

        auto rotation = doc.array();
        rotation.push_back(scene.camera.rotation[0]);
        rotation.push_back(scene.camera.rotation[1]);
        rotation.push_back(scene.camera.rotation[2]);

        camera["rotation"] = rotation;
    }

    doc["camera"] = camera;

    // Skybox
    nlohmann::json skybox;

    skybox["type"] = kSkyboxType[scene.skybox.type];

    if (scene.skybox.type == SKYBOX_STATIC)
    {
        skybox["environment_map"]     = scene.skybox.environment_map;
        skybox["diffuse_irradiance"]  = scene.skybox.diffuse_irradiance;
        skybox["specular_irradiance"] = scene.skybox.specular_irradiance;
    }

    doc["skybox"] = skybox;

    // Reflection Probes
    auto reflection_probes = doc.array();

    for (const auto& probe : scene.reflection_probes)
    {
        nlohmann::json json_probe;

        json_probe["path"] = probe.path;

        auto position = doc.array();
        position.push_back(probe.position[0]);
        position.push_back(probe.position[1]);
        position.push_back(probe.position[2]);

        json_probe["position"] = position;

        auto extents = doc.array();
        extents.push_back(probe.extents[0]);
        extents.push_back(probe.extents[1]);
        extents.push_back(probe.extents[2]);

        json_probe["extents"] = extents;

        reflection_probes.push_back(json_probe);
    }

    doc["reflection_probes"] = reflection_probes;

    // GI Probes
    auto gi_probes = doc.array();

    for (const auto& probe : scene.gi_probes)
    {
        nlohmann::json json_probe;

        json_probe["path"] = probe.path;

        auto position = doc.array();
        position.push_back(probe.position[0]);
        position.push_back(probe.position[1]);
        position.push_back(probe.position[2]);

        json_probe["position"] = position;

        gi_probes.push_back(json_probe);
    }

    doc["gi_probes"] = gi_probes;

    // Directional Lights
    auto directional_lights_array = doc.array();

    for (const auto& dir_light : scene.directional_lights)
    {
        nlohmann::json json_dir_light;

        auto color = doc.array();
        color.push_back(dir_light.color[0]);
        color.push_back(dir_light.color[1]);
        color.push_back(dir_light.color[2]);

        json_dir_light["color"] = color;

        auto rotation = doc.array();
        rotation.push_back(dir_light.rotation[0]);
        rotation.push_back(dir_light.rotation[1]);
        rotation.push_back(dir_light.rotation[2]);

        json_dir_light["rotation"] = rotation;

        json_dir_light["intensity"]    = dir_light.intensity;
        json_dir_light["cast_shadows"] = dir_light.casts_shadows;

        directional_lights_array.push_back(json_dir_light);
    }

    doc["directional_lights"] = directional_lights_array;

    // Point Lights
    auto point_lights_array = doc.array();

    for (const auto& point_light : scene.point_lights)
    {
        nlohmann::json json_point_light;

        auto color = doc.array();
        color.push_back(point_light.color[0]);
        color.push_back(point_light.color[1]);
        color.push_back(point_light.color[2]);

        json_point_light["color"] = color;

        auto position = doc.array();
        position.push_back(point_light.position[0]);
        position.push_back(point_light.position[1]);
        position.push_back(point_light.position[2]);

        json_point_light["position"] = position;

        json_point_light["range"]        = point_light.range;
        json_point_light["intensity"]    = point_light.intensity;
        json_point_light["cast_shadows"] = point_light.casts_shadows;

        point_lights_array.push_back(json_point_light);
    }

    doc["point_lights"] = point_lights_array;

    // Spot Lights
    auto spot_lights_array = doc.array();

    for (const auto& spot_light : scene.spot_lights)
    {
        nlohmann::json json_spot_light;

        auto color = doc.array();
        color.push_back(spot_light.color[0]);
        color.push_back(spot_light.color[1]);
        color.push_back(spot_light.color[2]);

        json_spot_light["color"] = color;

        auto position = doc.array();
        position.push_back(spot_light.position[0]);
        position.push_back(spot_light.position[1]);
        position.push_back(spot_light.position[2]);

        json_spot_light["position"] = position;

        auto rotation = doc.array();
        rotation.push_back(spot_light.rotation[0]);
        rotation.push_back(spot_light.rotation[1]);
        rotation.push_back(spot_light.rotation[2]);

        json_spot_light["rotation"] = rotation;

        json_spot_light["cone_angle"]   = spot_light.cone_angle;
        json_spot_light["range"]        = spot_light.range;
        json_spot_light["intensity"]    = spot_light.intensity;
        json_spot_light["cast_shadows"] = spot_light.casts_shadows;

        spot_lights_array.push_back(json_spot_light);
    }

    doc["spot_lights"] = spot_lights_array;

    // Entity
    auto entity_array = doc.array();

    for (const auto& entity : scene.entities)
    {
        nlohmann::json json_entity;

        json_entity["name"] = entity.name;
        json_entity["mesh"] = entity.mesh;

        if (entity.material_override.size() > 0)
            json_entity["material_override"] = entity.material_override;
        else
            json_entity["material_override"] = nullptr;

        auto position = doc.array();
        position.push_back(entity.position[0]);
        position.push_back(entity.position[1]);
        position.push_back(entity.position[2]);

        json_entity["position"] = position;

        auto rotation = doc.array();
        rotation.push_back(entity.rotation[0]);
        rotation.push_back(entity.rotation[1]);
        rotation.push_back(entity.rotation[2]);

        json_entity["rotation"] = rotation;

        auto scale = doc.array();
        scale.push_back(entity.scale[0]);
        scale.push_back(entity.scale[1]);
        scale.push_back(entity.scale[2]);

        json_entity["scale"] = scale;

        entity_array.push_back(json_entity);
    }

    doc["entities"] = entity_array;

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
