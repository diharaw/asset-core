cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libc++")
endif()

file(GLOB_RECURSE IMAGE_EXPORT_SOURCE ${PROJECT_SOURCE_DIR}/src/image_export/*.cpp
								 	  ${PROJECT_SOURCE_DIR}/src/image_export/*.h)

add_executable(image_export ${IMAGE_EXPORT_SOURCE})

set_property(TARGET image_export PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$(Configuration)")

target_link_libraries(image_export AssetCoreImporter)
target_link_libraries(image_export AssetCoreExporter)
target_link_libraries(image_export AssetCoreLoader)