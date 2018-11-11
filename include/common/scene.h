#pragma once

#include <glm.hpp>
#include <string>
#include <vector>

namespace ast
{
    enum CameraType
    {
        CAMERA_FLYTHROUGH,
        CAMERA_ORBIT
    };
    
    enum SkyboxType
    {
        SKYBOX_PROCEDURAL,
        SKYBOX_STATIC
    };
    
    struct Entity
    {
        std::string name;
        std::string mesh;
        std::string material_override;
        glm::vec3   position;
        glm::vec3   rotation;
        glm::vec3   scale;
    };
    
    struct Camera
    {
        CameraType type;
        float      rotation_speed;
        
        // Flythrough Properties
        glm::vec3  position;
        glm::vec3  rotation;
        float      movement_speed;
        
        // Orbit Properties
        glm::vec3  orbit_center;
        float      orbit_boom_length;
    };
    
    struct Skybox
    {
        SkyboxType  type;
        
        // Static Properties
        std::string environment_map;
        std::string diffuse_irradiance;
        std::string specular_irradiance;
    };
    
    struct ReflectionProbe
    {
        std::string diffuse_irradiance;
        std::string specular_irradiance;
        glm::vec3   position;
        glm::vec3   extents;
    };
    
    struct Scene
    {
        std::string                  name;
        Camera                       camera;
        Skybox                       skybox;
        std::vector<ReflectionProbe> reflection_probes;
        std::vector<Entity>          entities;
    };
}
