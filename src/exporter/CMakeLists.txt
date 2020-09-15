cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libc++")

file(GLOB_RECURSE AST_EXPORTER_SOURCE ${PROJECT_SOURCE_DIR}/src/exporter/*.cpp
									 ${PROJECT_SOURCE_DIR}/include/exporter/*.h)

add_library(AssetCoreExporter ${AST_EXPORTER_SOURCE})

target_link_libraries(AssetCoreExporter AssetCoreCommon)
target_link_libraries(AssetCoreExporter nvtt)
target_link_libraries(AssetCoreExporter cmft)
target_link_libraries(AssetCoreExporter assimp)