cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project("AssetCore")

# Options
set(BUILD_ASSET_CORE_LOADER_LIBRARY true CACHE BOOL "Build loader library")
set(BUILD_ASSET_CORE_EXPORTER_LIBRARY true CACHE BOOL "Build exporter library")
set(BUILD_ASSET_CORE_IMPORTER_LIBRARY true CACHE BOOL "Build importer library")
set(BUILD_ASSET_CORE_TOOLS true CACHE BOOL "Build tools")
set(ENABLE_CLANG_FORMAT true CACHE BOOL "Enable code formatting")

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

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (BUILD_ASSET_CORE_IMPORTER_LIBRARY OR BUILD_ASSET_CORE_EXPORTER_LIBRARY)
	add_subdirectory("${PROJECT_SOURCE_DIR}/external/cmft")
	add_subdirectory("${PROJECT_SOURCE_DIR}/external/nvidia-texture-tools")
	add_subdirectory("${PROJECT_SOURCE_DIR}/external/assimp")
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/importer")

	# External
	set_target_properties (assimp PROPERTIES FOLDER external)
	set_target_properties (bc6h PROPERTIES FOLDER external)
	set_target_properties (bc7 PROPERTIES FOLDER external)
	set_target_properties (butteraugli PROPERTIES FOLDER external)
	set_target_properties (cmft PROPERTIES FOLDER external)
	set_target_properties (cubemaptest PROPERTIES FOLDER external)
	set_target_properties (EtcLib PROPERTIES FOLDER external)
	set_target_properties (filtertest PROPERTIES FOLDER external)
	set_target_properties (imperativeapi PROPERTIES FOLDER external)
	set_target_properties (nvassemble PROPERTIES FOLDER external)
	set_target_properties (nvcompress PROPERTIES FOLDER external)
	set_target_properties (nvcore PROPERTIES FOLDER external)
	set_target_properties (nvddsinfo PROPERTIES FOLDER external)
	set_target_properties (nvdecompress PROPERTIES FOLDER external)
	set_target_properties (nv-gnome-thumbnailer PROPERTIES FOLDER external)
	set_target_properties (nvhdrtest PROPERTIES FOLDER external)
	set_target_properties (nvimage PROPERTIES FOLDER external)
	set_target_properties (nvimgdiff PROPERTIES FOLDER external)
	set_target_properties (nvmath PROPERTIES FOLDER external)
	set_target_properties (nvtestsuite PROPERTIES FOLDER external)
	set_target_properties (nvthread PROPERTIES FOLDER external)
	set_target_properties (nvtt PROPERTIES FOLDER external)
	set_target_properties (nvzoom PROPERTIES FOLDER external)
	set_target_properties (posh PROPERTIES FOLDER external)
	set_target_properties (process_alpha_map PROPERTIES FOLDER external)
	set_target_properties (rg_etc1 PROPERTIES FOLDER external)
	set_target_properties (squish PROPERTIES FOLDER external)
	set_target_properties (UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES FOLDER external)
	set_target_properties (zlibstatic PROPERTIES FOLDER external)
	
	# Libraries
	set_target_properties (AssetCoreCommon PROPERTIES FOLDER libs)
	set_target_properties (AssetCoreImporter PROPERTIES FOLDER libs)
endif()

if (BUILD_ASSET_CORE_EXPORTER_LIBRARY)
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/exporter")
	set_target_properties (AssetCoreExporter PROPERTIES FOLDER libs)
endif()

if (BUILD_ASSET_CORE_LOADER_LIBRARY)
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/loader")
	set_target_properties (AssetCoreLoader PROPERTIES FOLDER libs)
endif()

if (BUILD_ASSET_CORE_TOOLS AND BUILD_ASSET_CORE_IMPORTER_LIBRARY AND BUILD_ASSET_CORE_EXPORTER_LIBRARY)
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/mesh_export")
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/image_export")
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/brdf_lut")
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/sh_project")

	# Tools
	set_target_properties (brdf_lut PROPERTIES FOLDER tools)
	set_target_properties (image_export PROPERTIES FOLDER tools)
	set_target_properties (mesh_export PROPERTIES FOLDER tools)
	set_target_properties (sh_project PROPERTIES FOLDER tools)
endif()

find_program(CLANG_FORMAT_EXE NAMES "clang-format" DOC "Path to clang-format executable")

file(GLOB_RECURSE FORMAT_HEADERS ${PROJECT_SOURCE_DIR}/src/*.h)

file(GLOB_RECURSE FORMAT_SOURCE ${PROJECT_SOURCE_DIR}/src/*.cpp)

if(CLANG_FORMAT_EXE AND ENABLE_CLANG_FORMAT)
    add_custom_target(clang-format-project-files COMMAND ${CLANG_FORMAT_EXE} -i -style=file ${FORMAT_HEADERS} ${FORMAT_SOURCE})
endif()