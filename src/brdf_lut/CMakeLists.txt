cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

file(GLOB_RECURSE BRDF_LUT_SOURCE ${PROJECT_SOURCE_DIR}/src/brdf_lut/*.cpp
								  ${PROJECT_SOURCE_DIR}/src/brdf_lut/*.h)

add_executable(brdf_lut ${BRDF_LUT_SOURCE})

set_property(TARGET brdf_lut PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$(Configuration)")

target_link_libraries(brdf_lut AssetCoreImporter)
target_link_libraries(brdf_lut AssetCoreExporter)
target_link_libraries(brdf_lut AssetCoreLoader)