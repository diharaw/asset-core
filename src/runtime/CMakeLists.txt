cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libc++")

file(GLOB_RECURSE AST_RUNTIME_SOURCE ${PROJECT_SOURCE_DIR}/src/runtime/*.cpp
									 ${PROJECT_SOURCE_DIR}/include/runtime/*.h)

add_library(AssetCoreRuntime ${AST_RUNTIME_SOURCE})

target_link_libraries(AssetCoreRuntime AssetCoreCommon)