cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libc++")

file(GLOB_RECURSE AST_LOADER_SOURCE ${PROJECT_SOURCE_DIR}/src/loader/*.cpp
									 ${PROJECT_SOURCE_DIR}/include/loader/*.h)

add_library(AssetCoreLoader ${AST_LOADER_SOURCE})

target_link_libraries(AssetCoreLoader AssetCoreCommon)