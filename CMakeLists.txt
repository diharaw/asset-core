cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project("AssetCore")

# Options
set(BUILD_ASSET_CORE_RUNTIME true CACHE BOOL "Build Runtime Asset Core Library")
set(BUILD_ASSET_CORE_OFFLINE true CACHE BOOL "Build Offline Asset Core Library")
set(BUILD_ASSET_CORE_TOOLS true CACHE BOOL "Build Asset Core tools")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(ASSIMP_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/external/assimp/include" "${CMAKE_CURRENT_BINARY_DIR}/external/assimp/include")
set(JSON_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/external/json/single_include/nlohmann")
set(GLM_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/external/glm/glm")
set(NVTT_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/external/nvidia-texture-tools/src" "${PROJECT_SOURCE_DIR}/external/nvidia-texture-tools/extern/poshlib" "${CMAKE_CURRENT_BINARY_DIR}/external/nvidia-texture-tools/src")
set(CMFT_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/external/cmft/include")
set(STB_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/external/stb")
set(AST_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include")

set(ASSET_CORE_INCLUDES "${ASSIMP_INCLUDE_DIRS}"
						"${JSON_INCLUDE_DIRS}"
						"${GLM_INCLUDE_DIRS}"
						"${AST_INCLUDE_DIRS}"
						"${NVTT_INCLUDE_DIRS}"
						"${CMFT_INCLUDE_DIRS}"
						"${STB_INCLUDE_DIRS}" 
						PARENT_SCOPE)

include_directories("${ASSIMP_INCLUDE_DIRS}"
					"${JSON_INCLUDE_DIRS}"
					"${GLM_INCLUDE_DIRS}"
					"${AST_INCLUDE_DIRS}"
					"${NVTT_INCLUDE_DIRS}"
					"${CMFT_INCLUDE_DIRS}"
					"${STB_INCLUDE_DIRS}")

add_subdirectory("${PROJECT_SOURCE_DIR}/src/common")

if (BUILD_ASSET_CORE_OFFLINE)
	add_subdirectory("${PROJECT_SOURCE_DIR}/external/cmft")
	add_subdirectory("${PROJECT_SOURCE_DIR}/external/nvidia-texture-tools")
	add_subdirectory("${PROJECT_SOURCE_DIR}/external/assimp")
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/offline")
endif()

if (BUILD_ASSET_CORE_RUNTIME)
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/runtime")
endif()

if (BUILD_ASSET_CORE_TOOLS)
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/mesh_export")
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/material_export")
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/image_export")
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/brdf_lut")
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/sh_project")
endif()
