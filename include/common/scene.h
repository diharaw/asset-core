#pragma once

#include <glm.hpp>
#include <string>
#include <vector>

namespace ast
{
    static const std::string kCameraTypes[] =
    {
        "CAMERA_FLYTHROUGH",
        "CAMERA_ORBIT"
    };
    
    static const std::string kSkyboxType[] =
    {
        "SKYBOX_PROCEDURAL",
        "SKYBOX_STATIC"
    };
    
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

		// General Properties
        float near_plane;
        float far_plane;
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
        std::string path;
        glm::vec3   position;
        glm::vec3   extents;
    };
    
    struct GIProbe
    {
        std::string path;
        glm::vec3   position;
    };

	struct PointLight
	{
		glm::vec3 color;
		glm::vec3 position;
		float range;
		float intensity;
		bool casts_shadows;
	};

	struct SpotLight
	{
		glm::vec3 color;
		glm::vec3 position;
		glm::vec3 rotation;
		float cone_angle;
		float range;
		float intensity;
		bool casts_shadows;
	};

	struct DirectionalLight
	{
		glm::vec3 color;
		glm::vec3 rotation;
		float intensity;
		bool casts_shadows;
	};
    
    struct Scene
    {
        std::string                   name;
        Camera                        camera;
        Skybox                        skybox;
        std::vector<ReflectionProbe>  reflection_probes;
        std::vector<GIProbe>          gi_probes;
        std::vector<Entity>           entities;
		std::vector<DirectionalLight> directional_lights;
		std::vector<SpotLight>		  spot_lights;
		std::vector<PointLight>		  point_lights;
    };
}
