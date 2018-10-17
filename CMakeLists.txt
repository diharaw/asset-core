cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project("AssetCore")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(ASSIMP_INCLUDE_DIRS "external/assimp/include" "${CMAKE_CURRENT_BINARY_DIR}/external/assimp/include")
set(JSON_INCLUDE_DIRS "external/json/single_include/nlohmann")
set(GLM_INCLUDE_DIRS "external/glm/glm")
set(AST_INCLUDE_DIRS "include")

include_directories("${ASSIMP_INCLUDE_DIRS}"
					"${JSON_INCLUDE_DIRS}"
					"${GLM_INCLUDE_DIRS}"
					"${AST_INCLUDE_DIRS}")

add_subdirectory("external/assimp")
add_subdirectory("src/offline")
add_subdirectory("src/cli")
