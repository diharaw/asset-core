cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

file(GLOB_RECURSE MESH_EXPORT_SOURCE ${PROJECT_SOURCE_DIR}/src/mesh_export/*.cpp
								 	 ${PROJECT_SOURCE_DIR}/src/mesh_export/*.h)

add_executable(mesh_export ${MESH_EXPORT_SOURCE})

set_property(TARGET mesh_export PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$(Configuration)")

target_link_libraries(mesh_export AssetCoreImporter)
target_link_libraries(mesh_export AssetCoreExporter)