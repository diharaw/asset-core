cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libc++")

file(GLOB_RECURSE AST_IMPORTER_SOURCE ${PROJECT_SOURCE_DIR}/src/importer/*.cpp
									 ${PROJECT_SOURCE_DIR}/include/importer/*.h)

add_library(AssetCoreImporter ${AST_IMPORTER_SOURCE})

target_link_libraries(AssetCoreImporter AssetCoreCommon)
target_link_libraries(AssetCoreImporter nvtt)
target_link_libraries(AssetCoreImporter cmft)
target_link_libraries(AssetCoreImporter assimp)