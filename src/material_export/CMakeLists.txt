cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libc++")
endif()

file(GLOB_RECURSE MATERIAL_EXPORT_SOURCE ${PROJECT_SOURCE_DIR}/src/material_export/*.cpp
								 	 	 ${PROJECT_SOURCE_DIR}/src/material_export/*.h)

add_executable(material_export ${MATERIAL_EXPORT_SOURCE})

target_link_libraries(material_export AssetCoreOffline)
target_link_libraries(material_export AssetCoreRuntime)